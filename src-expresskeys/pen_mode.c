/*
 pen_mode.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.

 Copyright (C) 2005-2006 - Mats Johannesson

 Based on setmode.c 1996 by Frederic Lepied (xinput-1.2)

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
 Function toggles mode between Absolute and Relative based on a flag status.
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int toggle_stylus1_mode(Display *display, char *name)
{

	if (stylus1_mode == Absolute) {
		stylus1_mode = Relative;
	} else {
		stylus1_mode = Absolute;
	}

	if (be_verbose) {
		fprintf(stderr, "ST1 MODE = %d\n", stylus1_mode);
	}

	if (stylus1_info) {
		XSetDeviceMode(display, stylus1_device, stylus1_mode);
		return 0;
	} else {
		return 1;
	}
}

/* End Code */

