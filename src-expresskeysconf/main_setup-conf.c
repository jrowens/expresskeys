/*
 main_setup-conf.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
 
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

#include "globals-conf.h"

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
	}
	screen = DefaultScreen(display);

/* Locate the home directory of the user running this program */

	char *user_homedir;
	if ((user_homedir = getenv("HOME")) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Can not find your HOME directory!\n", our_prog_name, "");
	}

/* Concatenate the home directory string with the string of our expected
   configuration file directory. The address to the whole string is then
   copied to a global pointer, so we won't have to perform this part again */

	char *total_config_dir_block;
	len = strlen(user_homedir) + strlen(config_dir) + 1;
	if ((total_config_dir_block = (char *)malloc(len)) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Memory allocation trouble at stage 1!\n", our_prog_name, "");
	}
	sprintf(total_config_dir_block, "%s%s", user_homedir, config_dir);
	total_config_dir = total_config_dir_block;

/* Concatenate the full path with the config file name. Store address */

	char *total_config_file_block;
	len = strlen(total_config_dir) + strlen(config_file) + 1;
	if ((total_config_file_block = (char *)malloc(len)) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Memory allocation trouble at stage 2!\n", our_prog_name, "");
	}
	sprintf(total_config_file_block, "%s%s", total_config_dir, config_file);
	total_config_file = total_config_file_block;

/* Concatenate the full path with the pid file name. Store address */

	char *total_pid_file_block;
	len = strlen(total_config_dir) + strlen(pid_file) + 1;
	if ((total_pid_file_block = (char *)malloc(len)) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Memory allocation trouble at stage 3!\n", our_prog_name, "");
	}
	sprintf(total_pid_file_block, "%s%s", total_config_dir, pid_file);
	total_pid_file = total_pid_file_block;

/* Concatenate the full path with the error file name. Store address */

	char *total_error_file_block;
	len = strlen(total_config_dir) + strlen(error_file) + 1;
	if ((total_error_file_block = (char *)malloc(len)) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Memory allocation trouble at stage 4!\n", our_prog_name, "");
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

/* Set a flag if we should run in verbose mode */

	if (argc > 1) {
		for (i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-v") == 0) {
				be_verbose = 1;
				break;
			}
		}
	}

	if (be_verbose) {
		fprintf(stderr, "PGR VERSION = %s\n", our_prog_version);
		fprintf(stderr, "USR HOMEDIR = %s\n", user_homedir);
		fprintf(stderr, "THE CNF-DIR = %s\n", total_config_dir);
		fprintf(stderr, "THE CNFFILE = %s\n", total_config_file);
		fprintf(stderr, "THE PIDFILE = %s\n", total_pid_file);
		fprintf(stderr, "OUR LOGFILE = %s\n", total_error_file);
	}

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

/* Replace some of the normal signal handlers with our own functions.
   All the normal program exits should first clean up a bit */

	if ((signal(SIGINT, clean_up_exit) == SIG_ERR)
		|| (signal(SIGHUP, clean_up_exit) == SIG_ERR)
		|| (signal(SIGTERM, clean_up_exit) == SIG_ERR)) {
		exit_on_error(errorfp, "%s ERROR: Failed to modify signal handling!\n", our_prog_name, "");
	}

	use_events(display); /* <-- Our true launch! The event loop */
	exit_on_error(errorfp, "", "", "");

	exit(EXIT_OK); /* We should never reach this */

}

/* End Code */

