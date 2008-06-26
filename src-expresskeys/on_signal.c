/*
 on_signal.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.

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

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Function acts as a signal handler replacement for SIGUSR1
 On this signal we read a possibly changed configuration file again
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void re_read_file_config(int signum)
{

	struct program *p;

	FILE *fp;
	FILE *errorfp;

/* Open (and truncate) an error log for future reference */

	if ((errorfp = fopen(total_error_file, "w")) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Reread - Can not open %s in write mode\n", our_prog_name, total_error_file);
	}

/* Read in an existing configuration file */

	p = external_list;
	if ((fp = fopen(total_config_file, "r")) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Reread - Can not open %s in read mode\n", our_prog_name, total_config_file);
	} else {
		switch (read_file_config((void *)&p, fp)){

			case 0: /* No errors */
			fclose(fp);
			break; /* OBS An identical list of error code returns exist in the
					main_setup.c file. So change both when altering. */

			case 1:
			fclose(fp);
			exit_on_error(errorfp, "%s ERROR: Reread - No complete record found in %s\n", our_prog_name, total_config_file);

			case 2:
			fclose(fp);
			exit_on_error(errorfp, "%s ERROR: Reread - Memory allocation error while parsing %s\n", our_prog_name, total_config_file);

			case 3:
			fclose(fp);
			exit_on_error(errorfp, "%s ERROR: Reread - Config File Version 3 or higher not found\n", our_prog_name, "");

			case 4:
			fclose(fp);
			exit_on_error(errorfp, "%s ERROR: Reread - A line was too long to handle in the Config File %s\n", our_prog_name, total_config_file);

			case 5:
			fclose(fp);
			exit_on_error(errorfp, "%s ERROR: Reread - A program record named \"default\" must exist in %s\n", our_prog_name, total_config_file);

			default:
			fclose(fp);
			exit_on_error(errorfp, "%s ERROR: Reread - Unknown error while parsing %s\n", our_prog_name, total_config_file);
		}
	}
	go_daemon = 0;
	status_report(0); /* Update our information file */
	go_daemon = 1;
	fclose(errorfp);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Function acts as a signal handler replacement for SIGUSR2. On this
 signal we print some choise information to a file and to the screen.
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void status_report(int signum)
{

	FILE *errorfp = NULL;
	FILE *statusfp = NULL;
	
	struct program *p;
	p = external_list;
	int i;

	errorfp = fopen(total_error_file, "w");

/* Open (and truncate) a status log and fill it with information */

	if ((statusfp = fopen(total_status_file, "w")) == NULL) {
		exit_on_error(errorfp, "%s ERROR: Can not open %s in write mode\n", our_prog_name, total_status_file);
	} else {
		fclose(errorfp);
		fprintf(statusfp, "PGR VERSION = %s\n", our_prog_version);
		fprintf(statusfp, "USR HOMEDIR = %s\n", getenv("HOME"));
		fprintf(statusfp, "OUR CNF-DIR = %s\n", total_config_dir);
		fprintf(statusfp, "OUR CNFFILE = %s\n", total_config_file);
		fprintf(statusfp, "OUR PIDFILE = %s\n", total_pid_file);
		fprintf(statusfp, "OUR INFFILE = %s\n", total_status_file);
		fprintf(statusfp, "OUR ERRFILE = %s\n", total_error_file);
		if (pad1_name) {
			fprintf(statusfp, "OUR PD1NAME = %s\n", pad1_name);
		}
		if (stylus1_name) {
			fprintf(statusfp, "OUR ST1NAME = %s\n", stylus1_name);
		}
		fprintf(statusfp, "%s-user = %d\n", configstring, userconfigversion);
		fprintf(statusfp, "%s-ours = %s\n", configstring, configversion);
		fprintf(statusfp, "ConfigHeaderFields = %d\n", configheaderfields);
		for (i = 0; i < num_list; i++, p++) {
			fprintf(statusfp, "PGR RECNAME = %s\n", p->class_name);
			fprintf(statusfp, "ST1 PRCURVE = \"%s\"\n", p->stylus1_presscurve);
		}
		fprintf(statusfp, "PGR RECORDS = %d\n", num_list);
		if (go_daemon) {
			fprintf(statusfp, "OUR RUN-PID = %d\n", getpid());
		}
		fclose(statusfp);
	}

/* Also print the info to sceen (only if we have been daemonised and act on a -s signal) */

	if (go_daemon) {
		p = external_list;
		fprintf(stderr, "PGR VERSION = %s\n", our_prog_version);
		fprintf(stderr, "USR HOMEDIR = %s\n", getenv("HOME"));
		fprintf(stderr, "OUR CNF-DIR = %s\n", total_config_dir);
		fprintf(stderr, "OUR CNFFILE = %s\n", total_config_file);
		fprintf(stderr, "OUR PIDFILE = %s\n", total_pid_file);
		fprintf(stderr, "OUR INFFILE = %s\n", total_status_file);
		fprintf(stderr, "OUR ERRFILE = %s\n", total_error_file);
		if (pad1_name) {
			fprintf(stderr, "OUR PD1NAME = %s\n", pad1_name);
		}
		if (stylus1_name) {
			fprintf(stderr, "OUR ST1NAME = %s\n", stylus1_name);
		}
		fprintf(stderr, "%s-user = %d\n", configstring, userconfigversion);
		fprintf(stderr, "%s-ours = %s\n", configstring, configversion);
		fprintf(stderr, "ConfigHeaderFields = %d\n", configheaderfields);
		for (i = 0; i < num_list; i++, p++) {
			fprintf(stderr, "PGR RECNAME = %s\n", p->class_name);
			fprintf(stderr, "ST1 PRCURVE = \"%s\"\n", p->stylus1_presscurve);
		}
		fprintf(stderr, "PGR RECORDS = %d\n", num_list);
		fprintf(stderr, "OUR RUN-PID = %d\n", getpid());
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Function acts as a signal handler replacement for SIGINT, SIGHUP and
 SIGTERM. All are normal exit signals. We want to trap them in order to
 perform some house keeping pre-exit. Delete a PID file and free memory
 Since it takes care of several signals, it could get invoked recursively
 if some other signal comes in. We use this "volatile" variable to track
 the case. At the end we restore the default signal handler and raise it
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

volatile sig_atomic_t clean_up_exit_in_progress = 0;
void clean_up_exit(int signum)
{
	int i;
	struct program *p;
	p = external_list;

	if (clean_up_exit_in_progress) {
		raise(signum);
	}
	clean_up_exit_in_progress = 1;

	if (go_daemon) {
		unlink(total_pid_file);
		unlink(total_status_file);
	}

	if (total_config_dir) {
		free(total_config_dir);
	}

	if (total_config_file) {
		free(total_config_file);
	}

	if (total_pid_file) {
		free(total_pid_file);
	}

	if (total_error_file) {
		free(total_error_file);
	}

	if (total_status_file) {
		free(total_status_file);
	}

	if (num_list) {
		for (i = 0; i < num_list; i++, p++) {
			free(p->stylus1_presscurve);
			free(p->class_name);
		}
	}

	if (pad1_info_block) {
		XFreeDeviceList(pad1_info_block);
	}

	if (stylus1_info_block) {
		XFreeDeviceList(stylus1_info_block);
	}

/* pad1_device and stylus1_device should not be explicitly closed by a
   call to XCloseDevice. It leaves a message from X (in the terminal
   where X is started from) saying: "ProcXCloseDevice to close or not ?"
   Instead we just free the XDevice structures created by XOpenDevice */

	if (pad1_device) {
		XFree(pad1_device);
	}

	if (stylus1_device) {
		XFree(stylus1_device);
	}

	if (display) {
		XCloseDisplay(display);
	}

	signal(signum, SIG_DFL);
	raise(signum);
}

/* End Code */

