/*
 main_setup.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
 
 Copyright (C) 2005 - Mats Johannesson
 
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

	struct program *p;

	int i = 0;
	int len = 0;

	FILE *fp = NULL;
	FILE *errorfp = NULL;

/* Prelaunch sanity checks: See if X is OK */

	if ((display = XOpenDisplay(NULL)) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Can not connect to your X Server\n", our_prog_name, "");
		exit(EXIT_KO); 
	}
	screen = DefaultScreen(display);

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
		exit_on_error(errorfp, "%s ERROR: Memory allocation trouble at stage 2!\n", our_prog_name, "");
	}
	sprintf(total_config_dir_block, "%s%s", user_homedir, config_dir);
	total_config_dir = total_config_dir_block;

/* Concatenate the full path with the config file name. Store address */

	char *total_config_file_block;
	len = strlen(total_config_dir) + strlen(config_file) + 1;
	if ((total_config_file_block = (char *)malloc(len)) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Memory allocation trouble at stage 3!\n", our_prog_name, "");
	}
	sprintf(total_config_file_block, "%s%s", total_config_dir, config_file);
	total_config_file = total_config_file_block;

/* Concatenate the full path with the pid file name. Store address */

	char *total_pid_file_block;
	len = strlen(total_config_dir) + strlen(pid_file) + 1;
	if ((total_pid_file_block = (char *)malloc(len)) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Memory allocation trouble at stage 4!\n", our_prog_name, "");
	}
	sprintf(total_pid_file_block, "%s%s", total_config_dir, pid_file);
	total_pid_file = total_pid_file_block;

/* Concatenate the full path with the error file name. Store address */

	char *total_error_file_block;
	len = strlen(total_config_dir) + strlen(error_file) + 1;
	if ((total_error_file_block = (char *)malloc(len)) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Memory allocation trouble at stage 5!\n", our_prog_name, "");
	}
	sprintf(total_error_file_block, "%s%s", total_config_dir, error_file);
	total_error_file = total_error_file_block;

/* Prepare a scratch buffer for reading in an eventual PID */

	char pid_buffer [MAXBUFFER];

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

/* If a pid file exists it is a sign of either A) program already runs, or
   B) a crash/brutal kill not handled by our exit routine has occured
   previously. We therefore read in such a PID and perform a "fake" kill
   with it (signal number 0). If -1 (error) is returned we just carry on.
   Otherwise our kill test detected a process with that PID and we exit,
   based on the assumption that another instance is running */

	if ((fp = fopen(total_pid_file, "r")) != NULL) {
		fgets(pid_buffer, MAXBUFFER, fp);
		fclose(fp);
		if ((kill(atoi(pid_buffer), 0)) != NON_VALID) {
			exit_on_error(errorfp, "%s ERROR: Another instance of %s seems to be running!\n", our_prog_name, our_prog_name);
		}
	}

/* Open (and truncate) an error log for future reference */

	if ((errorfp = fopen(total_error_file, "w")) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Can not open %s in write mode\n", our_prog_name, total_error_file);
	}

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

/* We need at least the pad name specified... */

	if (argc < 2) {
		fprintf(stderr, "\n");
		fprintf(stderr, "%s Version: %s\n", our_prog_name, our_prog_version);
		fprintf(stderr, "\n");
		fprintf(stderr, "Usage: %s <pad-device-name> [<pen-device-name>] [-d] [-v]\n", our_prog_name);
		fprintf(stderr, "\n");
		fprintf(stderr, "Where the pad name is mandatory. Specify a pen name\n");
		fprintf(stderr, "if you want the program to handle pen mode switches.\n");
		fprintf(stderr, "Use -d to make the program a daemon (run in the background).\n");
		fprintf(stderr, "Use -v to print info to the screen at many execution points\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Example: %s pad stylus -d\n", our_prog_name);
		fprintf(stderr, "\n");
		exit_on_error(errorfp, "", "", "");
	}

/* See if the pad is for real, and if it is active */

	pad_info = (void *) get_device_info(display, pad_info_block, argv[1]);
	if (!pad_info) {
		exit_on_error(errorfp, "%s ERROR: Can not find pad device: %s\n", our_prog_name, argv[1]);
	}

/* Set a flag if we should run as a daemon or/and in verbose mode. Also
   register and check a pen device, should such an action be requested */

	if (argc > 2) {
		for (i = 2; i < argc; i++) {
			if (strcmp(argv[i], "-d") == 0) {
				go_daemon = 1;
				break;
			}
		}
		for (i = 2; i < argc; i++) {
			if (strcmp(argv[i], "-v") == 0) {
				be_verbose = 1;
				break;
			}
		}
		for (i = 2; i < argc; i++) {
			if ((strcmp(argv[i], "-d")) != 0 &&
			((strcmp(argv[i], "-v")) != 0)) {
				pen_name = argv[i];
				handle_pen = 1;
				pen_info = (void *) get_device_info(display, pen_info_block, argv[i]);
				if (!pen_info) {
					exit_on_error(errorfp, "%s ERROR: Can not find pen device: %s\n", our_prog_name, pen_name);
				}
				break;
			}
		}
	}

	if (be_verbose) {
		fprintf(stderr, "USR HOMEDIR = %s\n", user_homedir);
		fprintf(stderr, "OUR CNF-DIR = %s\n", total_config_dir);
		fprintf(stderr, "OUR CNFFILE = %s\n", total_config_file);
		fprintf(stderr, "OUR PIDFILE = %s\n", total_pid_file);
		fprintf(stderr, "OUR LOGFILE = %s\n", total_error_file);
		fprintf(stderr, "OUR PADNAME = %s\n", argv[1]);
		if (pen_name) {
			fprintf(stderr, "OUR PENNAME = %s\n", pen_name);
		}
	}

/* Make sure we can open a requested pen */

	if (handle_pen) {
		pen_mode = Relative;
		if (toggle_pen_mode(display, pen_name)) {
			exit_on_error(errorfp, "%s ERROR: Can not open pen device: %s\n", our_prog_name, pen_name);
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
	if ((fp = fopen(total_config_file, "r")) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Can not open %s in read mode\n", our_prog_name, total_config_file);
	} else {
		switch (read_file_config((void *)&p, fp)){
			
			case 0:
			fclose(fp);
			break;

			case 1:
			fclose(fp);
			exit_on_error(errorfp, "%s ERROR: No complete record found in %s\n", our_prog_name, total_config_file);
			
			case 2:
			fclose(fp);
			exit_on_error(errorfp, "%s ERROR: Memory allocation error while parsing %s\n", our_prog_name, total_config_file);

			case 3:
			fclose(fp);
			exit_on_error(errorfp, "%s ERROR: Config File Version %d not found\n", our_prog_name, (void *)CONFIG_VERSION);

			default:
			fclose(fp);
			exit_on_error(errorfp, "%s ERROR: Unknown error while parsing %s\n", our_prog_name, total_config_file);
		}
	}

/* Replace some of the normal signal handlers with our own functions. We
   want SIGUSR1 and SIGUSR2 to read in the config file after a modification,
   and all the normal program exits should first clean up a bit */

	if ((signal(SIGUSR1, re_read_file_config) == SIG_ERR)
		|| (signal(SIGUSR2, re_read_file_config) == SIG_ERR)
		|| (signal(SIGINT, clean_up_exit) == SIG_ERR)
		|| (signal(SIGHUP, clean_up_exit) == SIG_ERR)
		|| (signal(SIGTERM, clean_up_exit) == SIG_ERR)) {
		exit_on_error(errorfp, "%s ERROR: Failed to modify signal handling!\n", our_prog_name, "");
	}

/* Ready to launch in the foreground or as a daemon after one last check.
   In daemon mode we also take care of storing our PID in the config dir
   Observe that with a (0, 0) standard input/output/error goes to /dev/null
   I've found it better to use (0, 1) and see errors while writing the code
   It also comes in handy when running in (-v) verbose mode */

	if (register_events(display, pad_info, argv[1])) {
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
	} else {
		exit_on_error(errorfp, "%s ERROR: Could not register any events!\n", our_prog_name, "");
	}

	exit(EXIT_OK);

}

/* End Code */

