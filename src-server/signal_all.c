/*
 * signal_all.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
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

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Function acts as a signal handler replacement for SIGUSR1 and SIGUSR2 */
/* On these signals we read a possibly changed configuration file again */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void re_read_file_config(int signum)
{

	struct program *p;

	FILE *fp;
	FILE *errorfp;

/* Open (and truncate) an error log for future reference */
	
	if ((errorfp = fopen(total_error_file, "w")) == NULL) {
		fprintf(stderr, "%s ERROR: Reread - Can not open %s in write mode\n", our_prog_name, total_error_file);
		XCloseDisplay(display);
		exit(EXIT_KO);
	}

/* Read in an existing configuration file */

	p = external_list;
	if ((fp = fopen(total_config_file, "r")) == NULL) {
		fprintf(stderr, "%s ERROR: Reread - Can not open %s in read mode\n", our_prog_name, total_config_file);
		fprintf(errorfp, "%s ERROR: Reread - Can not open %s in read mode\n", our_prog_name, total_config_file);
		XCloseDisplay(display);
		exit(EXIT_KO);
	} else {
		switch (read_file_config((void *)&p, fp)){
			
			case 0:
			fclose(fp);
			break;

			case 1:
			fprintf(stderr, "%s ERROR: Reread - No complete record found in %s\n", our_prog_name, total_config_file);
			fprintf(errorfp, "%s ERROR: Reread - No complete record found in %s\n", our_prog_name, total_config_file);
			XCloseDisplay(display);
			exit(EXIT_KO);
			
			case 2:
			fprintf(stderr, "%s ERROR: Reread - Memory allocation error while parsing %s\n", our_prog_name, total_config_file);
			fprintf(errorfp, "%s ERROR: Reread - Memory allocation error while parsing %s\n", our_prog_name, total_config_file);
			XCloseDisplay(display);
			exit(EXIT_KO);
			
			default:
			fprintf(stderr, "%s ERROR: Reread - Unknown error while parsing %s\n", our_prog_name, total_config_file);
			fprintf(errorfp, "%s ERROR: Reread - Unknown error while parsing %s\n", our_prog_name, total_config_file);
			XCloseDisplay(display);
			exit(EXIT_KO);
		}
	}
	
	fclose(errorfp);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Function acts as a signal handler replacement for SIGINT, SIGHUP and */
/* SIGTERM. All are normal exit signals. We want to trap them in order to */
/* perform some house keeping pre-exit. Here it deletes a possible PID file */
/* Since it takes care of several signals, it could get invoked recursively */
/* if some other signal comes in. We use this "volatile" variable to track */
/* the case. At the end we restore the default signal handler and raise it */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

volatile sig_atomic_t clean_up_exit_in_progress = 0;
void clean_up_exit(int signum)
{
	if (clean_up_exit_in_progress) {
		raise(signum);
	}
	clean_up_exit_in_progress = 1;
	
	if (go_daemon) {
		unlink(total_pid_file);
	}
	
	signal(signum, SIG_DFL);
	raise(signum);
}

