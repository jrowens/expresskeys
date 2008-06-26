/*
 on_signal-conf.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
 
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

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Function acts as a signal handler replacement for SIGINT, SIGHUP and
 SIGTERM. All are normal exit signals. We want to trap them in order to
 perform some house keeping pre-exit. Mostly to free memory.
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

	if (num_list) {
		for (i = 0; i < num_list; i++, p++) {
			free(p->class_name);
		}
	}

	if (display) {
		XCloseDisplay(display);
	}

	signal(signum, SIG_DFL);
	raise(signum);
}

/* End Code */

