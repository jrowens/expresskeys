/*
 * main_setup.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
 *
 * Copyright (C) 2005 - Mats Johannesson
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "globals.h"

int main (int argc, char *argv[])
{

	our_prog_name = argv[0];

	struct program *p;

	int i;
	int len;

	FILE *fp;
	FILE *errorfp;

/* Prelaunch sanity checks: See if X is OK */

	if ((display = XOpenDisplay(NULL)) == NULL) {
		fprintf(stderr, "%s ERROR: Can not connect to your X Server\n", our_prog_name);
		return EXIT_KO; 
	}
	screen = DefaultScreen(display);

/* Locate the home directory of the user running this program */

	char *user_homedir;
	if ((user_homedir = getenv("HOME")) == NULL) {
		fprintf(stderr, "%s ERROR: Can not find your HOME directory!\n", our_prog_name);
		XCloseDisplay(display);
		return EXIT_KO;
	}

/* This is silly, but in order to keep the heap from being stomped on */
/* by malloc and even automatic allocations we first need to allocate */
/* a slight bit of memory. Here I use 1024 bytes, which seems enough. */
/* Not doing this results in random errors when changing the code. */
/* The "heap_protect" memory is freed before we leave the function */
/* Another solution is to allocate a fair bit of memory in the actual */
/* malloc that stomps on the heap. But it seems even sillier to use */
/* a 1024 byte string when only needing 41, than this workaround is*/

	char *heap_protect;
	if ((heap_protect = (char *)malloc(1024)) == NULL) {
		fprintf(stderr, "%s ERROR: Memory allocation trouble at stage 1!\n", our_prog_name);
		return EXIT_KO; 
	}

/* Concatenate the home directory string with the string of our preferred*/
/* configuration file directory. The address to the whole string is then */
/* copied to a global pointer, so we won't have to perform this part again */

	char *total_config_dir_block;
	len = strlen(user_homedir) + strlen(config_dir) + 1;
	if ((total_config_dir_block = (char *)malloc(len)) == NULL) {
		fprintf(stderr, "%s ERROR: Memory allocation trouble at stage 2!\n", our_prog_name);
		XCloseDisplay(display);
		return EXIT_KO; 
	}
	sprintf(total_config_dir_block, "%s%s", user_homedir, config_dir);
	total_config_dir = total_config_dir_block;

/* Concatenate the full path with the config file name. Store address */

	char *total_config_file_block;
	len = strlen(total_config_dir) + strlen(config_file) + 1;
	if ((total_config_file_block = (char *)malloc(len)) == NULL) {
		fprintf(stderr, "%s ERROR: Memory allocation trouble at stage 3!\n", our_prog_name);
		XCloseDisplay(display);
		return EXIT_KO; 
	}
	sprintf(total_config_file_block, "%s%s", total_config_dir, config_file);
	total_config_file = total_config_file_block;

/* Concatenate the full path with the pid file name. Store address */

	char *total_pid_file_block;
	len = strlen(total_config_dir) + strlen(pid_file) + 1;
	if ((total_pid_file_block = (char *)malloc(len)) == NULL) {
		fprintf(stderr, "%s ERROR: Memory allocation trouble at stage 4!\n", our_prog_name);
		XCloseDisplay(display);
		return EXIT_KO; 
	}
	sprintf(total_pid_file_block, "%s%s", total_config_dir, pid_file);
	total_pid_file = total_pid_file_block;

/* Concatenate the full path with the error file name. Store address */

	char *total_error_file_block;
	len = strlen(total_config_dir) + strlen(error_file) + 1;
	if ((total_error_file_block = (char *)malloc(len)) == NULL) {
		fprintf(stderr, "%s ERROR: Memory allocation trouble at stage 5!\n", our_prog_name);
		XCloseDisplay(display);
		return EXIT_KO; 
	}
	sprintf(total_error_file_block, "%s%s", total_config_dir, error_file);
	total_error_file = total_error_file_block;

/* Prepare a scratch buffer for reading in an eventual PID */

	char pid_buffer [MAXBUFFER];

/* Try to open the the configuration directory for */
/* reading, just as a test to see if it exists. A failure */
/* here can mean many things, but we then try to create */
/* it as a means to rule out a true lack of existence */

	if ((fp = fopen(total_config_dir, "r")) == NULL) {
		if ((mkdir(total_config_dir, 0777)) == NON_VALID) {
			fprintf(stderr, "%s ERROR: Can not read or create %s\n", our_prog_name, total_config_dir);
			XCloseDisplay(display);
			return EXIT_KO;
		}
	} else {
		fclose(fp);
	}

/* If a pid file exists it is a sign of either A) program already runs, or */
/* B) a crash/brutal kill not handled by our exit routine has occured */
/* previously. We therefore read in such a PID and perform a "fake" kill */
/* with it (signal number 0). If -1 (error) is returned we just carry on. */
/* Otherwise our kill test detected a process with that PID and we exit, */
/* based on the assumption that another instance is running */

	if ((fp = fopen(total_pid_file, "r")) != NULL) {
		fgets(pid_buffer, MAXBUFFER, fp);
		fclose(fp);
		if ((kill(atoi(pid_buffer), 0)) != NON_VALID) {
			fprintf(stderr, "%s ERROR: Another instance of %s seems to be running!\n", our_prog_name, our_prog_name);
			XCloseDisplay(display);
			return EXIT_KO;
		}
	}

/* Open (and truncate) an error log for future reference */

	if ((errorfp = fopen(total_error_file, "w")) == NULL) {
		fprintf(stderr, "%s ERROR: Can not open %s in write mode\n", our_prog_name, total_error_file);
		XCloseDisplay(display);
		return EXIT_KO;
	}
	
/* Can we use XTest to send fake key presses */

	int event_base, error_base;
	int major_version, minor_version;
	if (!XTestQueryExtension(display, &event_base, &error_base,
		&major_version, &minor_version)) {
		fprintf(stderr, "%s ERROR: XTest extension not present on your X server\n", our_prog_name);
		fprintf(errorfp, "%s ERROR: XTest extension not present on your X server\n", our_prog_name);
		XCloseDisplay(display);
		return EXIT_KO;
	}

/* Can we use XInput to talk with the tablet */

	XExtensionVersion *xinputext;
	xinputext = XGetExtensionVersion(display, INAME);
	if (xinputext && (xinputext != (XExtensionVersion*) NoSuchExtension)) {
		XFree(xinputext);
	} else {
		fprintf(stderr, "%s ERROR: XInput extension not present on your X server\n", our_prog_name);
		fprintf(errorfp, "%s ERROR: XInput extension not present on your X server\n", our_prog_name);
		XCloseDisplay(display);
		return EXIT_KO;
	}

/* We need at least the pad name specified... */

	if (argc < 2) {
		fprintf(stderr, "\n");
		fprintf(stderr, "Usage: %s <pad-device-name> [<pen-device-name>] [-d]\n", our_prog_name);
		fprintf(stderr, "\n");
		fprintf(stderr, "Where the pad name is mandatory. Specify a pen name\n");
		fprintf(stderr, "if you want the program to handle pen mode switches.\n");
		fprintf(stderr, "Use -d to make the program a daemon (run in the background).\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Example: %s pad stylus -d\n", our_prog_name);
		fprintf(stderr, "\n");
		XCloseDisplay(display);
		return EXIT_KO;
	}

/* See if the pad is for real, and if it is active */

	pad_info = (void *) get_device_info(display, argv[1]);
	XFreeDeviceList(info);
	if (!pad_info) {
		fprintf(stderr, "%s ERROR: Can not find pad device: %s\n", our_prog_name, argv[1]);
		fprintf(errorfp, "%s ERROR: Can not find pad device: %s\n", our_prog_name, argv[1]);
		XCloseDisplay(display);
		return EXIT_KO;
	}

/* Set a flag if we should run as a daemon. Also register */
/* and check a pen device, should such an action be requested */

	if (argc > 2) {
		for (i = 2; i < argc; i++) {
			if (strcmp(argv[i], "-d") == 0) {
				go_daemon = 1;
				break;
			}
		}
		for (i = 2; i < argc; i++) {
			if (strcmp(argv[i], "-d") != 0) {
				pen_name = argv[i];
				handle_pen = 1;
				pen_info = (void *) get_device_info(display, argv[i]);
				XFreeDeviceList(info);
				if (!pen_info) {
					fprintf(stderr, "%s ERROR: Can not find pen device: %s\n", our_prog_name, pen_name);
					fprintf(errorfp, "%s ERROR: Can not find pen device: %s\n", our_prog_name, pen_name);
					XCloseDisplay(display);
					return EXIT_KO;
				}
				break;
			}
		}
	}

/* Make sure we can open a requested pen */

	if (handle_pen) {
		pen_mode = Relative;
		if (toggle_pen_mode(display, pen_name)) {
			fprintf(stderr, "%s ERROR: Can not open pen device: %s\n", our_prog_name, pen_name);
			fprintf(errorfp, "%s ERROR: Can not open pen device: %s\n", our_prog_name, pen_name);
			XCloseDisplay(display);
			return EXIT_KO;
		}
	}

/* If no configuration file exists, write out a */
/* short one from an internal memory structure */

	if ((fp = fopen(total_config_file, "a+")) == NULL) {
		fprintf(stderr, "%s ERROR: Can not open %s in read/write mode\n", our_prog_name, total_config_file);
		fprintf(errorfp, "%s ERROR: Can not open %s in read/write mode\n", our_prog_name, total_config_file);
		XCloseDisplay(display);
		return EXIT_KO;
	} else {
		rewind(fp);
		if (fgetc(fp) == EOF) {
			for (p = internal_list; p < internal_list + num_list; p++) {
				write_file_config((void *)&p, fp);
				if (ferror(fp)) {
					fprintf(stderr, "%s ERROR: Write error in %s\n", our_prog_name, total_config_file);
					fprintf(errorfp, "%s ERROR: Write error in %s\n", our_prog_name, total_config_file);
					XCloseDisplay(display);
					return EXIT_KO;
				}
			}
		}
	}
	fclose(fp);

/* Read in an existing configuration file */

	p = external_list;
	if ((fp = fopen(total_config_file, "r")) == NULL) {
		fprintf(stderr, "%s ERROR: Can not open %s in read mode\n", our_prog_name, total_config_file);
		fprintf(errorfp, "%s ERROR: Can not open %s in read mode\n", our_prog_name, total_config_file);
		XCloseDisplay(display);
		return EXIT_KO;
	} else {
		switch (read_file_config((void *)&p, fp)){
			
			case 0:
			fclose(fp);
			break;

			case 1:
			fprintf(stderr, "%s ERROR: No complete record found in %s\n", our_prog_name, total_config_file);
			fprintf(errorfp, "%s ERROR: No complete record found in %s\n", our_prog_name, total_config_file);
			XCloseDisplay(display);
			return EXIT_KO;
			
			case 2:
			fprintf(stderr, "%s ERROR: Memory allocation error while parsing %s\n", our_prog_name, total_config_file);
			fprintf(errorfp, "%s ERROR: Memory allocation error while parsing %s\n", our_prog_name, total_config_file);
			XCloseDisplay(display);
			return EXIT_KO;
			
			default:
			fprintf(stderr, "%s ERROR: Unknown error while parsing %s\n", our_prog_name, total_config_file);
			fprintf(errorfp, "%s ERROR: Unknown error while parsing %s\n", our_prog_name, total_config_file);
			XCloseDisplay(display);
			return EXIT_KO;
		}
	}

/* Replace some of the normal signal handlers with our own functions. We */
/* want SIGUSR1 and SIGUSR2 to read in the config file after a modification, */
/* and all the normal program exits should first clean up a bit. The exit */
/* could have been handled by a call to "atexit" but this was more fun :-) */

	if ((signal(SIGUSR1, re_read_file_config) == SIG_ERR)
		|| (signal(SIGUSR2, re_read_file_config) == SIG_ERR)
		|| (signal(SIGINT, clean_up_exit) == SIG_ERR)
		|| (signal(SIGHUP, clean_up_exit) == SIG_ERR)
		|| (signal(SIGTERM, clean_up_exit) == SIG_ERR)) {
		fprintf(stderr, "%s ERROR: Failed to modify signal handling!\n", our_prog_name);
		fprintf(errorfp, "%s ERROR: Failed to modify signal handling!\n", our_prog_name);
		XCloseDisplay(display);
		return EXIT_KO;
	}

/* Ready to launch in the foreground or as a daemon after one last check. */
/* In daemon mode we also take care of storing our PID in the config dir */
/* Observe that with a (0, 0) standard input/output/error goes to /dev/null */
/* I've found it better to use (0, 1) and see errors while writing the code */

	if (register_events(display, pad_info, argv[1])) {
		if (go_daemon) {
			if ((daemon(0, 1)) == NON_VALID) {
				fprintf(stderr, "%s ERROR: Failed to fork into daemon mode! EXITING!\n", our_prog_name);
				fprintf(errorfp, "%s ERROR: Failed to fork into daemon mode! EXITING!\n", our_prog_name);
				XCloseDisplay(display);
				return EXIT_KO;
			} else {
				sprintf(pid_buffer, "%d", getpid());
				if ((fp = fopen(total_pid_file, "w")) == NULL) {
					fprintf(stderr, "%s ERROR: Can not open %s in write mode\n", our_prog_name, total_pid_file);
					fprintf(errorfp, "%s ERROR: Can not open %s in write mode\n", our_prog_name, total_pid_file);
					XCloseDisplay(display);
					exit(EXIT_KO);
				} else {
					fprintf(fp, "%s", pid_buffer);
					if (ferror(fp)) {
						fprintf(stderr, "%s ERROR: Write error in %s\n", our_prog_name, total_pid_file);
						fprintf(errorfp, "%s ERROR: Write error in %s\n", our_prog_name, total_pid_file);
						XCloseDisplay(display);
						exit(EXIT_KO);
					} else {
						fclose(fp);
					}
				}
			}
		}
		fclose(errorfp); /* Close error log */
		free(heap_protect); /* free the dummy heap protect */
		use_events(display); /* <-- Our true launch! The event loop */
	} else {
		fprintf(stderr, "%s ERROR: Could not register any events!\n", our_prog_name);
		fprintf(errorfp, "%s ERROR: Could not register any events!\n", our_prog_name);
		XCloseDisplay(display);
		return EXIT_KO;
	}

	XCloseDisplay(display);
	return EXIT_OK;
}

/* End Code */

