/*
 expresskeys - Support ExpressKeys, Touch Strips, Scroll Wheel on Wacom tablets.

 Copyright (C) 2005-2007 - Mats Johannesson

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h> /* NULL, FILE, fopen, fclose, fprintf */
#include <stdlib.h> /* exit, getenv */
#include <libgen.h> /* basename */
#include <sys/stat.h> /* mkdir */
#include <sys/types.h> /* mkdir, getpid */
#include <unistd.h> /* getpid, daemon */
#include <signal.h> /* signal */
#include <X11/extensions/XInput.h>

#include "defines.h"

/* Globals: */

/* Flag to mark that a tablet with no 'pad' device is present: */
int have_padless;

/* The basename call will provide our runtime program name: */
const char* our_prog_name;

/* Path to the configuration directory and some information files: */
const char* total_config_dir;
const char* total_pid_file;
const char* total_error_file;
const char* total_status_file;

/* Externals */

extern int live_pid;
extern int have_pad;

extern int ok_xsetwacom;
extern int ok_config;

extern int go_daemon;
extern int be_verbose;
extern int just_exit;

extern int send_sigterm;
extern int send_sigusr1;
extern int send_sigusr2;

extern Display* display;

extern const char* user_pad;
extern const char* user_stylus1;
extern const char* user_stylus2;

extern XDevice* stylus_xdevice[MAXSTYLUS][MAXMODEL * MAXPAD];

extern char* path_malloc(char* path, char* file);
extern void parse_command(FILE* errorfp, int argc, char* argv[]);
extern void check_pid(const char* pidfile, const char* statusfile,
						const char* pgr_runline);
extern void send_command(FILE* errorfp);
extern void check_x_sanity(FILE* errorfp);
extern int xerror_handler(Display* display, XErrorEvent* error_event);
extern void get_device_info();
extern void check_user(FILE* errorfp);
extern void shuffle_styli();
extern void device_to_structure();
extern void report_header(FILE* reportfp);
extern void write_config(FILE* errorfp);
extern void read_config(FILE* errorfp);
extern void clean_up_exit(int signum);
extern void re_read_config(int signum);
extern void status_report(int signum);
extern void use_events();

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Like most main() functions, this is primarily a place to call other functions
 from and to check some important return results (through flags and arrays):
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int main(int argc, char* argv[])
{
	int i, j;

	our_prog_name = basename(argv[0]);

	const char* warn_xsetwacom =
"\nOBSERVE: xsetwacom 0.0.7 or greater was not detected on this machine\n"
"(linuxwacom-0.7.5-2 or higher is required). Since we need at least that\n"
"version for identification and other functionality, all tablets will be\n"
"treated as having no 'pad' (being 'padless'). A very limited set of\n"
"features, or none at all, will be available.\n\n";

	const char* no_tablet =
"ERROR: Tablet not attached, OR (in case of Cintiq 21UX, Intuos3\n"
"and Graphire4) the 'pad' device has not been specified in xorg.conf, OR is\n"
"lacking on the command line when using \"named devices\".\n";

	const char* config_dir = "/.expresskeys";
	const char* pid_file = "/expresskeys.pid";
	const char* error_file = "/error.log";
	const char* status_file = "/status.log";

/* String (and name) read from the status log while searching for a daemon: */
	const char* pgr_runline = "PGR RUNNAME =";

	FILE* fp = NULL;
	FILE* errorfp = NULL;

	char pid_buffer[MAXBUFFER];
	const char* user_homedir;

	if ((user_homedir = getenv("HOME")) == NULL) {
		exit_on_error(errorfp,
	"\n%s ERROR: Can not find your HOME directory!\n", our_prog_name, "");
	}

/* Malloc strings for the configuration directory and information files: */
	total_config_dir = path_malloc((void*)user_homedir, (void*)config_dir);
	total_pid_file = path_malloc((void*)total_config_dir, (void*)pid_file);
	total_error_file = path_malloc((void*)total_config_dir,
							(void*)error_file);
	total_status_file = path_malloc((void*)total_config_dir,
							(void*)status_file);

/* Try to open the the configuration directory for reading, just as a test
 to see if it exists. A failure here can mean many things, but we then try to
 create it as a means to rule out a true lack of existence: */
	if ((fp = fopen(total_config_dir, "r")) == NULL) {
		if ((mkdir(total_config_dir, 0777)) == NON_VALID) {
			exit_on_error(errorfp,
"\n%s ERROR: Can not read or create %s\n", our_prog_name, total_config_dir);
		}
	} else {
		fclose(fp);
	}

/* Open (and truncate) an error log for future reference: */
	if ((errorfp = fopen(total_error_file, "w")) == NULL) {
		exit_on_error(errorfp,
"\n%s ERROR: Can not open %s in write mode\n", our_prog_name, total_error_file);
	}

/* Check for flags and named devices on the command line: */
	parse_command(errorfp, argc, argv);

/* Look for a daemon instance of the program: */
	check_pid(total_pid_file, total_status_file, pgr_runline);

/* Act on signal flags from the user: */
	if ((live_pid) && ((send_sigusr1) || (send_sigusr2) ||
						(send_sigterm))) {
		send_command(errorfp);
	} else if ((!live_pid) && ((send_sigusr1) || (send_sigusr2) ||
						(send_sigterm))) {
		exit_on_error(errorfp,
"\n%s ERROR: No daemon instance of expresskeys found to send a -r -s or -k\n",
							our_prog_name, "");
	}

/* Try to hinder several instances of the program to run simultaneously: */
	if (live_pid) {
		exit_on_error(errorfp,
"\n%s ERROR: A daemon instance of expresskeys seems to be running!\n",
							our_prog_name, "");
	}

/* See if X is running (open a connection to the X display) and that both
 XInput and XTest extensions are present. The tests should not be performed
 prior to this point, since we both want to be able to send signals from a
 client running in a pure text terminal, _and_ prevent the same client from
 deleting a live PID-file (test of opened X display is used as trigger): */
	check_x_sanity(errorfp);

/* Register an error handler for X server error returns: */
	XSetErrorHandler(xerror_handler);

/* Search for and open appropriate devices. Register events and fill arrays: */
	get_device_info();

/* Compare device names specified on the command line with found devices: */
	if ((user_pad) || (user_stylus1) || (user_stylus2)) {
		check_user(errorfp);
	}

/* See if a tablet with no 'pad' device has been detected: */
	for (i = 0; i < MAXSTYLUS && have_padless == 0; i++) {
		for (j = 0; j < MAXPAD; j++) {
			if (stylus_xdevice[i][j]) {
				have_padless = 1;
				break;
			}
		}
	}

/* Exit if we are totally void of tablets: */
	if ((!have_pad) && (!have_padless)) {
		fp = stderr;
		for (i = 0; i < 2; i++) {
			fprintf(fp, "\n%s %s", our_prog_name, no_tablet);
			fp = errorfp;
		}
		fclose(errorfp);
		errorfp = NULL;
		exit_on_error(errorfp, "", "", "");
	}

/* Adjust possible misappropriations when filling in the stylus arrays: */
	shuffle_styli();

/* Transfer obtained knowledge about the devices from arrays to structures: */
	device_to_structure();

/* Aid in debugging/tuning or just satisfy curiosity if asked to: */
	if (be_verbose) {
		report_header(stderr);
	}

/* Write out a minimal configuration file for any tablet that is missing one: */
	write_config(errorfp);

/* Read in existing configuration files for currently connected tablets: */
	read_config(errorfp);

/* Sanity verifications: */
	if (!ok_config) {
		exit_on_error(errorfp,
"\n%s ERROR: No active file with a valid ConfigVersion found!\n",
							our_prog_name, "");
	}

	if (!ok_xsetwacom) {
		fp = stderr;
		for (i = 0; i < 2; i++) {
			fprintf(fp, "%s", warn_xsetwacom);
			fp = errorfp;
		}
	}

/* Exit if all the user wanted to see was the main 'debugging' info block: */
	if (just_exit) {
		exit_on_error(errorfp, "", "", "");
	}

/* Replace some of the normal signal handlers with our own functions. We want
 SIGUSR1 to read in all the configuration files after a modification, SIGUSR2
 to print status (the information we would get from -s) to file/screen, and
 all the normal program exits should first clean up a bit: */
	if ((signal(SIGUSR1, re_read_config) == SIG_ERR)
		|| (signal(SIGUSR2, status_report) == SIG_ERR)
		|| (signal(SIGINT, clean_up_exit) == SIG_ERR)
		|| (signal(SIGHUP, clean_up_exit) == SIG_ERR)
		|| (signal(SIGTERM, clean_up_exit) == SIG_ERR)) {
		exit_on_error(errorfp,
"\n%s ERROR: Failed to modify signal handling!\n", our_prog_name, "");
	}

/* Store information we have in the status file (silent use of the -s switch):*/
	if (go_daemon) {
		go_daemon = 0;
		status_report(0);
		go_daemon = 1;
	}

/* Ready to launch in the foreground or as a daemon. In daemon mode we also
 take care of storing our PID in the config dir. Observe that with a (0, 0)
 standard input/output/error goes to /dev/null. I've found it better to use
 (0, 1) and see errors while writing the code. It also comes in handy when
 running in -v (verbose) mode: */
	if (go_daemon) {
		if ((daemon(0, 1)) == NON_VALID) {
			exit_on_error(errorfp,
"\n%s ERROR: Failed to fork into daemon mode! EXITING!\n", our_prog_name, "");
		} else {
			sprintf(pid_buffer, "%i", getpid());
			if ((fp = fopen(total_pid_file, "w")) == NULL) {
				exit_on_error(errorfp,
"\n%s ERROR: Can not open %s in write mode\n", our_prog_name, total_pid_file);
			} else {
				if (be_verbose) {
					fprintf(stderr,
					"OUR RUN-PID = %s\n", pid_buffer);
				}
				fprintf(fp, "%s", pid_buffer);
				if (ferror(fp)) {
					fclose(fp);
					exit_on_error(errorfp,
"\n%s ERROR: Write error in %s\n", our_prog_name, total_pid_file);
				} else {
					fclose(fp);
				}
			}
		}
	}
	fclose(errorfp);

/* Our true launch, the event loop: */
	use_events();

/* We should never reach this: */
	exit(0);

}

/* End Code */

