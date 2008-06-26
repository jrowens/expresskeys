/*
 pen_mode.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
 
 Copyright (C) 2005 - Mats Johannesson
 
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
 Function opens the X input device (which stays open until program end)
 and toggles mode between Absolute and Relative based on a flag status.
 We make sure to only open the pen once by setting another flag.
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int toggle_pen_mode(Display *display, char *name)
{

	if (pen_mode == Absolute) {
		pen_mode = Relative;
	} else {
		pen_mode = Absolute;
	}

	if (be_verbose) {
		fprintf(stderr, "PEN MODE = %d\n", pen_mode);
	}

	if (!pen_open){
		pen_device = XOpenDevice(display, pen_info->id);
		pen_open = 1;
	}
	if ((pen_device) && (pen_open)) {
		XSetDeviceMode(display, pen_device, pen_mode);
		return 0;
	} else {
		return 1;
	}
}

/* End Code */

