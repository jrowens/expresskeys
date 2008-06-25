/*
 * pen_mode.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
 *
 * Copyright (C) 2005 - Mats Johannesson
 *
 * Based on setmode.c 1996 by Frederic Lepied
 *
 * This code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this code; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "globals.h"

int toggle_pen_mode(Display *display, char *name)
{

	if (pen_mode == Absolute) {
		pen_mode = Relative;
	} else {
		pen_mode = Absolute;
	}
	if (!pen_open){
		pen_device = XOpenDevice(display, pen_info->id);
		pen_open = 1;
	}
	if ((pen_device) && (pen_open)) {
		XSetDeviceMode(display, pen_device, pen_mode);
		return 0;
	} else {
		fprintf(stderr, "ERROR: Can not open pen device: %s\n", PEN_NAME);
		return 0;
	}
}

/* End Code */

