/*
 on_error.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.

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
 Function prints out the error strings from a caller and terminates
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void exit_on_error(FILE *fp, char *string1, char *string2, char *string3)
{

	if (fp) {
		fprintf(fp, string1, string2, string3);
		fclose(fp);
	}

	fprintf(stderr, string1, string2, string3);
	clean_up_exit(SIGINT); /* We use SIGINT here instead of SIGTERM to get rid */
                            /* of the "Terminated" message printed by the shell */

/* FIXME Drops the state EXIT_KO when calling clean_up_exit, becomes EXIT_OK */

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Function is a rudimentary error handler for Xserver error returns
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int xerror_handler(Display *display, XErrorEvent *error_event)
{
	FILE *errorfp = NULL;
	errorfp = fopen(total_error_file, "w");

	char error_buffer [MAXBUFFER];
	XGetErrorText(display, error_event->error_code, error_buffer, MAXBUFFER);
	fprintf(stderr, "X error message: %d %s\n", error_event->error_code, error_buffer);
	exit_on_error(errorfp, "%s ERROR: A fatal error was returned from the X server: %s\n", our_prog_name, error_buffer);
	return 0;
}

/* End Code */

