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

#include <stdio.h> /* NULL, FILE, fprintf, fopen, fclose */
#include <signal.h> /* SIGINT */
#include <X11/Xlib.h> /* XErrorEvent also pulls in X11/X.h with BadWindow def.*/
#include <X11/extensions/XIproto.h> /* Minor opcode error: X_OpenDevice */

#include "defines.h"

/* Externals: */

extern const char* our_prog_name;
extern const char* total_error_file;

extern void clean_up_exit(int signum);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 This function prints out the error strings from a caller and terminates:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void exit_on_error(FILE* errorfp, char* string1, const char* string2,
							const char* string3)
{
	if (errorfp) {
		fprintf(errorfp, string1, string2, string3);
		fclose(errorfp);
	}

	fprintf(stderr, string1, string2, string3);

/* We use SIGINT instead of SIGTERM to get rid of the "Terminated" message
 printed by eg. a bash shell */
	clean_up_exit(SIGINT);

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 This function is a rudimentary error handler for Xserver error returns:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int xerror_handler(Display* display, XErrorEvent* error_event)
{
/* There seems to be no standard way to catch an "BadDevice, invalid or
 uninitialized input device" error when trying to open devices which aren't
 plugged in. The extension error code varies (I've seen 169 and 170 myself)
 and the message string could be localized as well. Therefore we take a
 two-pronged approach here: Just return if the error code falls within the
 extension error range (>= 128) and at the same time is a X_OpenDevice
 "Minor opcode" as defined in X11/extensions/XIproto.h
   Also, in rare cases we might hit "BadWindow (invalid Window parameter)"
 (code 3) when looking for the present focus-window. I've only seen it with
 the "Firefox Preferences" window, but we better ignore any such case
 (standard error code from X11/X.h pulled in through X11/Xlib.h): */
	if (((error_event->error_code >= 128)
	&& (error_event->minor_code == X_OpenDevice))
	|| (error_event->error_code == BadWindow)) {
		return 0;
	}

/* Turning X behaviour into sync (from async) takes care of 'double-free' and
 other unpleasant conditions, should more than one X error surface here: */
	XSynchronize(display, True);

	FILE* errorfp = NULL;
	errorfp = fopen(total_error_file, "w");

	char error_buffer[MAXBUFFER];
	XGetErrorText(display, error_event->error_code, error_buffer,MAXBUFFER);
	fprintf(stderr, "\nX error message: %i %s\n", error_event->error_code,
								error_buffer);

	exit_on_error(errorfp,
	"\n%s ERROR: A fatal error was returned from the X server: %s\n",
						our_prog_name, error_buffer);

/* We will never reach this: */
	return 0;

}

/* End Code */

