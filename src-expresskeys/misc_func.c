/*
 expresskeys - Support ExpressKeys, Touch Strips, Scroll Wheel on Wacom tablets.

 Copyright (C) 2005-2006 - Mats Johannesson

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

#include <stdio.h> /* NULL, FILE, sprintf, fopen, fclose, fgets, snprintf */
#include <stdlib.h> /* malloc, atoi */
#include <string.h> /* strlen, strstr, strchr */
#include <sys/types.h> /* kill */
#include <signal.h> /* kill */
#include <X11/extensions/XInput.h>
#include <X11/extensions/XTest.h>

#include "defines.h"

/* Globals: */

/* Active screen and display. The X server thing: */
int screen;
Display* display;

/* PID of a running daemon instance: */
int live_pid;

/* Externals: */

extern const char* our_prog_name;

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 This function should ensure X sanity at program start:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void check_x_sanity(FILE* errorfp)
{
	if ((display = XOpenDisplay(NULL)) == NULL) {
		exit_on_error(errorfp,
"\n%s ERROR: Can not connect to your X server\n", our_prog_name, "");
	}
	screen = DefaultScreen(display);

/* Can we use XTest to send fake key presses: */
	int event_base, error_base;
	int major_version, minor_version;
	if (!XTestQueryExtension(display, &event_base, &error_base,
					&major_version, &minor_version)) {
		exit_on_error(errorfp,
"\n%s ERROR: XTest extension not present in your X server\n",
							our_prog_name, "");
	}

/* Can we use XInput as a bridge to the tablet: */
	XExtensionVersion* xinputext;
	xinputext = XGetExtensionVersion(display, INAME);
	if (xinputext && (xinputext != (XExtensionVersion*) NoSuchExtension)) {
		XFree(xinputext);
	} else {
		exit_on_error(errorfp,
"\n%s ERROR: XInput extension not present in your X server\n",
							our_prog_name, "");
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Function allocates memory for and fills in strings to paths and files:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

char* path_malloc(char* path, char* file)
{
	FILE* errorfp = NULL;
	char* address = NULL;
	int len = 0;

	len = strlen(path) + strlen(file)+1;
	if ((address = malloc(len)) == NULL) {
		exit_on_error(errorfp,
"\n%s ERROR: Memory allocation trouble at path_malloc() for %s\n",
							our_prog_name, file);
	}
	sprintf(address, "%s%s", path, file);

	return address;

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 If a pid file exists it is a sign of either A) program already runs, or
 B) a crash/brutal kill not handled by our exit routine has occured previously.
 We therefore read in such a PID and perform a "fake" kill with it (signal
 number 0). If -1 (error) is returned we just carry on. Otherwise our kill test
 detected a process with that PID, and we commence with a comparison between
 the program name in /proc/%PID/status and the runtime program name stored in
 our own status log. If they are equal, a daemon instance indeed is present
 and we save the PID in the live_pid variable. There are circumstances when
 even this double test can fail its mission, but they should be rare:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void check_pid(const char* pidfile, const char* statusfile,
							const char* pgr_runline)
{
	int line_limit = 0;

	char pid_buffer[MAXBUFFER];
	char status_buffer[MAXBUFFER];
	char write_buffer[MAXBUFFER];

	FILE* fp = NULL;
	char* cp;

	pid_buffer[0] = '\0';
	status_buffer[0] = '\0';
	write_buffer[0] = '\0';

	if ((fp = fopen(pidfile, "r")) != NULL) {
		fgets(pid_buffer, MAXBUFFER, fp);
		fclose(fp);
	}

	if (pid_buffer[0]) {
		if ((fp = fopen(statusfile, "r")) != NULL) {
			while (((fgets(write_buffer, MAXBUFFER, fp)) != NULL)
				&& (line_limit < LINELIMIT)) {
				line_limit++;
				if ((strstr(write_buffer, pgr_runline))
								!=NULL) {
					cp = strchr(write_buffer, '=')+2;
					snprintf(status_buffer, MAXBUFFER, cp);
					status_buffer[strlen(status_buffer)-1]
									= '\0';
					break;
				}
			}
			fclose(fp);
		}
	}

	if ((pid_buffer[0]) && (status_buffer[0])
		&& ((kill(atoi(pid_buffer), 0)) != NON_VALID)) {

		snprintf(write_buffer, MAXBUFFER, "/proc/%s/status",
								pid_buffer);
		if ((fp = fopen(write_buffer, "r")) != NULL) {
			while ((fgets(write_buffer, MAXBUFFER, fp)) != NULL) {
				if ((strstr(write_buffer, status_buffer))
								!=NULL) {
					live_pid = atoi(pid_buffer);
					break;
				}
			}
			fclose(fp);
		}
	}

}

/* End Code */

