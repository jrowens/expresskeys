/*
 * reg_events.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
 *
 * Copyright (C) 2005 - Mats Johannesson
 *
 * Based on test.c 1996 by Frederic Lepied (xinput-1.2)
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
/* Function opens the X input device (which stays open until program end) */
/* and starts to look for supported event types. The scope should be the */
/* root window (ie everywhere) and we're only interested in motion events */
/* (touch strip action) and button press/release. Having found the info */
/* we ask the X server to keep us continuously notified about these events */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int register_events(Display *display, XDeviceInfo *pad_info, char *name)
{
	int i;
	int count = 0;
	XEventClass event_list[3];
	XInputClassInfo *ip;
	Window root_win;

	root_win = RootWindow(display, screen);

	pad_device = XOpenDevice(display, pad_info->id);
	if (!pad_device) {
		return 0;
	}

	if (pad_device->num_classes > 0) {
		for (ip = pad_device->classes, i = 0; i < pad_info->num_classes; ip++, i++) {
			switch (ip->input_class) {

			case ButtonClass:
			DeviceButtonPress(pad_device, button_press_type, event_list[count]);
			count++;
			DeviceButtonRelease(pad_device, button_release_type, event_list[count]);
			count++;
			break;

			case ValuatorClass:
			DeviceMotionNotify(pad_device, motion_type, event_list[count]);
			count++;
			break;

			default:
			break;
			}
		}

	if (XSelectExtensionEvent(display, root_win, event_list, count)) {
		return 0;
		}
	}
	return count;
}

/* End Code */

