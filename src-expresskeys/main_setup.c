/*
 main_setup.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.

 Copyright (C) 2005-2006 - Mats Johannesson

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
*/

#include "globals.h"

int main (int argc, char *argv[])
{

	our_prog_name = basename(argv[0]);

	int len = 0;

	struct program *p;
	struct configstrings *hp;

	FILE *fp = NULL;
	FILE *errorfp = NULL;

/* Locate the home directory of the user running this program */

	char *user_homedir;
	if ((user_homedir = getenv("HOME")) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Can not find your HOME directory!\n", our_prog_name, "");
	}

/* Concatenate the home directory string with the string of our preferred
   configuration file directory. The address to the whole string is then
   copied to a global pointer, so we won't have to perform this part again */

	char *total_config_dir_block;
	len = strlen(user_homedir) + strlen(config_dir) + 1;
	if ((total_config_dir_block = (char *)malloc(len)) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Memory allocation trouble at stage 1!\n", our_prog_name, "");
	}
	sprintf(total_config_dir_block, "%s%s", user_homedir, config_dir);
	total_config_dir = total_config_dir_block;

/* Concatenate the full path with the pid file name. Store address */

	char *total_pid_file_block;
	len = strlen(total_config_dir) + strlen(pid_file) + 1;
	if ((total_pid_file_block = (char *)malloc(len)) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Memory allocation trouble at stage 2!\n", our_prog_name, "");
	}
	sprintf(total_pid_file_block, "%s%s", total_config_dir, pid_file);
	total_pid_file = total_pid_file_block;

/* Concatenate the full path with the error file name. Store address */

	char *total_error_file_block;
	len = strlen(total_config_dir) + strlen(error_file) + 1;
	if ((total_error_file_block = (char *)malloc(len)) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Memory allocation trouble at stage 3!\n", our_prog_name, "");
	}
	sprintf(total_error_file_block, "%s%s", total_config_dir, error_file);
	total_error_file = total_error_file_block;

/* Try to open the the configuration directory for reading, just as a
   test to see if it exists. A failure here can mean many things, but we
   then try to create it as a means to rule out a true lack of existence */

	if ((fp = fopen(total_config_dir, "r")) == NULL) {
		if ((mkdir(total_config_dir, 0777)) == NON_VALID) {
			exit_on_error(errorfp, "%s ERROR: Can not read or create %s\n", our_prog_name, total_config_dir);
		}
	} else {
		fclose(fp);
	}

/* Open (and truncate) an error log for future reference */

	if ((errorfp = fopen(total_error_file, "w")) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Can not open %s in write mode\n", our_prog_name, total_error_file);
	}

/* Prelaunch sanity checks: See if X is OK */

	if ((display = XOpenDisplay(NULL)) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Can not connect to your X Server\n", our_prog_name, "");
	}
	screen = DefaultScreen(display);

/* Can we use XTest to send fake key presses */

	int event_base, error_base;
	int major_version, minor_version;
	if (!XTestQueryExtension(display, &event_base, &error_base,
		&major_version, &minor_version)) {
		exit_on_error(errorfp, "%s ERROR: XTest extension not present on your X server\n", our_prog_name, "");
	}

/* Can we use XInput to talk with the tablet */

	XExtensionVersion *xinputext;
	xinputext = XGetExtensionVersion(display, INAME);
	if (xinputext && (xinputext != (XExtensionVersion*) NoSuchExtension)) {
		XFree(xinputext);
	} else {
		exit_on_error(errorfp, "%s ERROR: XInput extension not present on your X server\n", our_prog_name, "");
	}

/* Automatically discover, open and register events with the first devices
   that contain the string 'pad' and/or 'stylus' in their xorg.conf "Identifier"
   entries (case insensitive). These devices will be invalidated if the user
   specifies a named device on the command line (at least for now, since we only
   allow one device of each kind to be registered) */

	pad1_info = (void *) get_device_info(display, pad1_info_block, pad1_autoname);
	if (pad1_info) {
		if (register_events(display, pad1_info, pad1_info->name)) {
			pad1_autoname = pad1_info->name;
		} else {
			pad1_autoname = 0;
		}
	} else {
		pad1_autoname = 0;
	}

	stylus1_info = (void *) get_device_info(display, stylus1_info_block, stylus1_autoname);
	if (stylus1_info) {
		if (register_events(display, stylus1_info, stylus1_info->name)) {
			stylus1_autoname = stylus1_info->name;
		} else {
			stylus1_autoname = 0;
		}
	} else {
		stylus1_autoname = 0;
	}

/* Free X memory allocations for missing devices */

	if (!pad1_autoname) {
		XFreeDeviceList(pad1_info_block);
		pad1_info_block = 0;
	}
	if (!stylus1_autoname) {
		XFreeDeviceList(stylus1_info_block);
		stylus1_info_block = 0;
	}

/* If NO device can be found or have its events registered, we exit. Fair? */

	if ((!pad1_autoname) && (!stylus1_autoname)) {
		exit_on_error(errorfp, "%s ERROR: Absolutely NO 'pad' or 'stylus' device seem to be connected. Check your system (eg xorg.conf)! Verify this lack by running \"xsetwacom list\" or \"xidump -l\"\n", our_prog_name, "");
	}

/* Command line handling */

	char read_buffer [MAXBUFFER];
	char write_buffer [MAXBUFFER];

	int send_sigterm = 0;
	int send_sigusr1 = 0;
	int send_sigusr2 = 0;
	int give_help = 0;

	int i, c, d;

	while ((--argc > 0) && (**++argv == '-' || **argv != '-')) {
		if (**argv == '-') {
			while (((c = *++argv[0])) && (argc != NON_VALID)) {
				switch (c) {

					case 'd':
					go_daemon = 1;
					break;

					case 'k':
					send_sigterm = 1;
					break;

					case 'r':
					send_sigusr1 = 1;
					break;

					case 'v':
					be_verbose = 1;
					break;

					case 'x':
					just_exit = 1;
					be_verbose = 1;
					break;

					case 's':
					send_sigusr2 = 1;
					break;

					case 'h':
					give_help = 1;
					break;

					default:
					fprintf(stderr, "\n%s ERROR: Invalid switch '-%c' on command line.\n", our_prog_name, c);
					argc = NON_VALID;
					break;
				}
			}
		} else {
			len = strlen(*argv);
			if (len < MAXBUFFER) {
				sprintf(read_buffer, "%s", *argv);
				for (i = 0; i < len; i++) {
					d = tolower(read_buffer[i]); /* Turn everything into lower case */
					sprintf(write_buffer+i, "%c", d);
					++read_buffer[i];
				}
				strncpy(write_buffer + len, "\0", 1);
			} else {
				exit_on_error(errorfp, "%s ERROR: A device name on the command line was too long to handle!\n", our_prog_name, "");
			}
			if (!pad1_name) {
				if ((strstr(write_buffer, "pad")) !=NULL) {
					pad1_name = *argv;
				}
			}
			if (!stylus1_name) {
				if ((strstr(write_buffer, "stylus")) !=NULL) {
					stylus1_name = *argv;
				}
			}
			if ((!pad1_name) && (!stylus1_name)) {
				argc = NON_VALID;
			}
		}
	}
	if ((argc != 0) || (give_help == 1)) {
		fprintf(stderr, "\n");
		fprintf(stderr, "%s Version: %s\n", our_prog_name, our_prog_version);
		fprintf(stderr, "\n");
		fprintf(stderr, "Usage: %s [OPTION]... [DEVICE]...\n", our_prog_name);
		fprintf(stderr, "Any combination of pad and/or stylus can be specified.\n");
		fprintf(stderr, "An empty command line means automatic device search.\n");
		fprintf(stderr, "Options can be written like -dv or -d -v in any place.\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "  -d makes the program a daemon (runs in the background).\n");
		fprintf(stderr, "  -k terminates (kills) an already running daemon instance.\n");
		fprintf(stderr, "  -r re-reads the configuration file of a running daemon.\n");
		fprintf(stderr, "  -v prints info to the screen at many execution points.\n");
		fprintf(stderr, "  -x sets -v and exits after some important info blocks.\n");
		fprintf(stderr, "  -s tells a daemon instance to report status (info block).\n");
		fprintf(stderr, "  -h unconditionally brings up this help text.\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Example1: %s -d (first 'pad' and/or 'stylus' found get used)\n", our_prog_name);
		fprintf(stderr, "Example2: %s 1stIntuos3Pad 1stIntuos3Stylus2 -d (named devices)\n", our_prog_name);
		fprintf(stderr, "Example3: %s -rv (visibly re-read the configuration file)\n", our_prog_name);
		fprintf(stderr, "\n");
		fprintf(stderr, "Please direct any bug reports or questions to the top address\n");
		fprintf(stderr, "in the AUTHORS file. This program is _not_ a linuxwacom project.\n");
		exit_on_error(errorfp, "", "", "");
	}

/* Prepare a scratch buffer for reading in an eventual PID */

	char pid_buffer [MAXBUFFER];

/* The following routine handles both intentional re-runs of the program
   (configuration file re-read requests or a terminate to a daemon), and
   unintentional attempts to start another daemon. The latter goes like:

   If a pid file exists it is a sign of either A) program already runs, or
   B) a crash/brutal kill not handled by our exit routine has occured
   previously. We therefore read in such a PID and perform a "fake" kill
   with it (signal number 0). If -1 (error) is returned we just carry on.
   Otherwise our kill test detected a process with that PID and we exit,
   based on the assumption that another instance is running */

	if ((fp = fopen(total_pid_file, "r")) != NULL) { /* File exists */
		fgets(pid_buffer, MAXBUFFER, fp);
		fclose(fp);
		if (((kill(atoi(pid_buffer), 0)) != NON_VALID)) { /* Live pid-file */
			if ((send_sigusr1) || (send_sigusr2) || (send_sigterm)) {
				if (send_sigusr1) {
					if ((kill(atoi(pid_buffer), SIGUSR1)) != NON_VALID) {
						if (be_verbose) {
							exit_on_error(errorfp, "%s SUCCESS: The %s configuration file has been re-read\n", our_prog_name, our_prog_name);
						}
						exit_on_error(errorfp, "", "", "");
					} else {
						exit_on_error(errorfp, "%s FAILURE: The %s configuration file could not be re-read!\n", our_prog_name, our_prog_name);
					}
				}
				if (send_sigusr2) {
					if ((kill(atoi(pid_buffer), SIGUSR2)) != NON_VALID) {
						if (be_verbose) {
							exit_on_error(errorfp, "%s SUCCESS: The %s status has been reported\n", our_prog_name, our_prog_name);
						}
						exit_on_error(errorfp, "", "", "");
					} else {
						exit_on_error(errorfp, "%s FAILURE: The %s status could not be reported!\n", our_prog_name, our_prog_name);
					}
				}
				if (send_sigterm) {
					if ((kill(atoi(pid_buffer), SIGTERM)) != NON_VALID) {
						if (be_verbose) {
							exit_on_error(errorfp, "%s SUCCESS: The %s daemon has been terminated\n", our_prog_name, our_prog_name);
						}
						exit_on_error(errorfp, "", "", "");
					} else {
						exit_on_error(errorfp, "%s FAILURE: The %s daemon could not be terminated!\n", our_prog_name, our_prog_name);
					}
				}
			} else {
				second_instance = 1; /* Flag prevents a PID-file deletion */
				exit_on_error(errorfp, "%s ERROR: Another instance of %s seems to be running!\n", our_prog_name, our_prog_name);
			}
		} else { /* Dead pid-file */
			if ((send_sigusr1) || (send_sigusr2) || (send_sigterm)) {
				exit_on_error(errorfp, "%s ERROR: No daemon instance of %s found to send a -r -s or -k\n", our_prog_name, our_prog_name);
			}
		}
	} else { /* No file at all */
		if ((send_sigusr1) || (send_sigusr2) || (send_sigterm)) {
			exit_on_error(errorfp, "%s ERROR: No daemon instance of %s found to send a -r -s or -k\n", our_prog_name, our_prog_name);
		}
	}

/* See if a requested pad is for real, and if events can be registered with it */

	if (pad1_name) {
		if ((pad1_autoname) && ((strlen(pad1_autoname) != (strlen(pad1_name))))) {
			XFreeDeviceList(pad1_info_block);
			pad1_info_block = 0;
			pad1_autoname = 0;
		} else {
			if ((pad1_autoname) && ((strcmp(pad1_autoname, pad1_name)) != 0)) {
				XFreeDeviceList(pad1_info_block);
				pad1_info_block = 0;
				pad1_autoname = 0;
			}
		}
		if (!pad1_info_block) {
			pad1_info = (void *) get_device_info(display, pad1_info_block, pad1_name);
			if (!pad1_info) {
				exit_on_error(errorfp, "%s ERROR: Can not find pad device: %s\n", our_prog_name, pad1_name);
			}
			if (!register_events(display, pad1_info, pad1_name)) {
				exit_on_error(errorfp, "%s ERROR: Could not register any pad events with %s\n", our_prog_name, pad1_name);
			}
		} else {
			pad1_name = pad1_autoname;
		}
	} else {
		pad1_name = pad1_autoname;
	}

/* See if a requested stylus is for real, and if events can be registered with it */

	if (stylus1_name) {
		if ((stylus1_autoname) && ((strlen(stylus1_autoname) != (strlen(stylus1_name))))) {
			XFreeDeviceList(stylus1_info_block);
			stylus1_info_block = 0;
			stylus1_autoname = 0;
		} else {
			if ((stylus1_autoname) && ((strcmp(stylus1_autoname, stylus1_name)) != 0)) {
				XFreeDeviceList(stylus1_info_block);
				stylus1_info_block = 0;
				stylus1_autoname = 0;
			}
		}
		if (!stylus1_info_block) {
			stylus1_info = (void *) get_device_info(display, stylus1_info_block, stylus1_name);
			if (!stylus1_info) {
				exit_on_error(errorfp, "%s ERROR: Can not find stylus device: %s\n", our_prog_name, stylus1_name);
			}
			if (!register_events(display, stylus1_info, stylus1_name)) {
				exit_on_error(errorfp, "%s ERROR: Could not register any stylus events with %s\n", our_prog_name, stylus1_name);
			}
		} else {
			stylus1_name = stylus1_autoname;
		}
	} else {
		stylus1_name = stylus1_autoname;
	}

/* Now determine what the config file will be named as */

	if ((pad1_name) && (stylus1_name)) {
		config_file = config_file_intuos3;
	} else {
		if (stylus1_name) {
			config_file = config_file_padless;
		} else {
			config_file = config_file_intuos3; /* User error? 'pad' without 'stylus'! */
		}
	}

/* Concatenate the full path with the config file name. Store address */

	char *total_config_file_block;
	len = strlen(total_config_dir) + strlen(config_file) + 1;
	if ((total_config_file_block = (char *)malloc(len)) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Memory allocation trouble at stage 4!\n", our_prog_name, "");
	}
	sprintf(total_config_file_block, "%s%s", total_config_dir, config_file);
	total_config_file = total_config_file_block;

	if (be_verbose) {
		fprintf(stderr, "PGR VERSION = %s\n", our_prog_version);
		fprintf(stderr, "USR HOMEDIR = %s\n", user_homedir);
		fprintf(stderr, "OUR CNF-DIR = %s\n", total_config_dir);
		fprintf(stderr, "OUR CNFFILE = %s\n", total_config_file);
		fprintf(stderr, "OUR PIDFILE = %s\n", total_pid_file);
		fprintf(stderr, "OUR LOGFILE = %s\n", total_error_file);
		if (pad1_name) {
			fprintf(stderr, "OUR PD1NAME = %s\n", pad1_name);
		}
		if (stylus1_name) {
			fprintf(stderr, "OUR ST1NAME = %s\n", stylus1_name);
		}
	}

/* If no configuration file exists, write out a short one from an internal
   memory structure. Also tag it with a Config File Version number */

	if ((fp = fopen(total_config_file, "a+")) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Can not open %s in read/write mode\n", our_prog_name, total_config_file);
	} else {
		rewind(fp);
		if (fgetc(fp) == EOF) {
			write_file_config_header(fp);
			for (p = internal_list; p < internal_list + num_list; p++) {
				write_file_config((void *)&p, fp);
				if (ferror(fp)) {
					exit_on_error(errorfp, "%s ERROR: Write error in %s\n", our_prog_name, total_config_file);
				}
			}
		}
	}
	fclose(fp);

/* Read in an existing configuration file */

	p = external_list;
	hp = human_readable;
	if ((fp = fopen(total_config_file, "r")) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Can not open %s in read mode\n", our_prog_name, total_config_file);
	} else {
		switch (read_file_config((void *)&p, (void *)&hp, fp)){

			case 0: /* No errors */
			fclose(fp);
			break; /* OBS An identical list of error code returns exist in the on_signal.c
					file (the re_read_file_config function). So change both when altering. */

			case 1:
			fclose(fp);
			exit_on_error(errorfp, "%s ERROR: No complete record found in %s\n", our_prog_name, total_config_file);

			case 2:
			fclose(fp);
			exit_on_error(errorfp, "%s ERROR: Memory allocation error while parsing %s\n", our_prog_name, total_config_file);

			case 3:
			fclose(fp);
			exit_on_error(errorfp, "%s ERROR: Config File Version 3 or higher not found\n", our_prog_name, "");

			case 4:
			fclose(fp);
			exit_on_error(errorfp, "%s ERROR: A line was too long to handle in the Config File %s\n", our_prog_name, total_config_file);

			case 5:
			fclose(fp);
			exit_on_error(errorfp, "%s ERROR: A program record named \"default\" must exist in %s\n", our_prog_name, total_config_file);

			default:
			fclose(fp);
			exit_on_error(errorfp, "%s ERROR: Unknown error while parsing %s\n", our_prog_name, total_config_file);
		}
	}

/* Exit if all we wanted to see was the main 'debugging' info block */

	if (just_exit) {
		exit_on_error(errorfp, "", "", "");
	}

/* Replace some of the normal signal handlers with our own functions. We
   want SIGUSR1 to read in the config file after a modification, SIGUSR2
   to print status (the information we would get from -x) to the screen,
   and all the normal program exits should first clean up a bit */

	if ((signal(SIGUSR1, re_read_file_config) == SIG_ERR)
		|| (signal(SIGUSR2, status_report) == SIG_ERR)
		|| (signal(SIGINT, clean_up_exit) == SIG_ERR)
		|| (signal(SIGHUP, clean_up_exit) == SIG_ERR)
		|| (signal(SIGTERM, clean_up_exit) == SIG_ERR)) {
		exit_on_error(errorfp, "%s ERROR: Failed to modify signal handling!\n", our_prog_name, "");
	}

/* Ready to launch in the foreground or as a daemon.
   In daemon mode we also take care of storing our PID in the config dir
   Observe that with a (0, 0) standard input/output/error goes to /dev/null
   I've found it better to use (0, 1) and see errors while writing the code
   It also comes in handy when running in (-v) verbose mode */

		if (go_daemon) {
			if ((daemon(0, 1)) == NON_VALID) {
				exit_on_error(errorfp, "%s ERROR: Failed to fork into daemon mode! EXITING!\n", our_prog_name, "");
			} else {
				sprintf(pid_buffer, "%d", getpid());
				if ((fp = fopen(total_pid_file, "w")) == NULL) {
					exit_on_error(errorfp, "%s ERROR: Can not open %s in write mode\n", our_prog_name, total_pid_file);
				} else {
					if (be_verbose) {
						fprintf(stderr, "OUR RUN-PID = %s\n", pid_buffer);
					}
					fprintf(fp, "%s", pid_buffer);
					if (ferror(fp)) {
						exit_on_error(errorfp, "%s ERROR: Write error in %s\n", our_prog_name, total_pid_file);
					} else {
						fclose(fp);
					}
				}
			}
		}
		fclose(errorfp);
		use_events(display); /* <-- Our true launch! The event loop */

	exit(EXIT_OK); /* We should never reach this */

}

/* End Code */

