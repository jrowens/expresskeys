/*
 reg_events.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.

 Copyright (C) 2005-2006 - Mats Johannesson

 Based on test.c 1996 by Frederic Lepied (xinput-1.2)

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
 and starts to look for supported event types. The scope should be the
 root window (ie everywhere) and we're only interested in motion events
 (touch strip action) and button press/release (for stylus only press and
 proximityin). Having found the info we ask the X server to keep us
 continuously notified about these events.
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int register_events(Display *display, XDeviceInfo *dev_info, char *name)
{

	int i;
	int count = 0;

	XInputClassInfo *ip;
	Window root_win;

	root_win = RootWindow(display, screen);

	if ((pad1_info) && (dev_info->id == pad1_info->id)) {
		XEventClass event_list[3];
		pad1_device = XOpenDevice(display, dev_info->id);
		if (!pad1_device) {
			return 0;
		}
		if (pad1_device->num_classes > 0) {
			for (ip = pad1_device->classes, i = 0; i < pad1_info->num_classes; ip++, i++) {
				switch (ip->input_class) {

				case ButtonClass:
				DeviceButtonPress(pad1_device, button_press_type, event_list[count]);
				count++;
				DeviceButtonRelease(pad1_device, button_release_type, event_list[count]);
				count++;
				break;

				case ValuatorClass:
				DeviceMotionNotify(pad1_device, motion_type, event_list[count]);
				count++;
				break;

				default:
				break;
				}
			}
		}
		if (XSelectExtensionEvent(display, root_win, event_list, count)) {
			return 0;
		}
		return count;
	}
	if ((stylus1_info) && (dev_info->id == stylus1_info->id)) {
		XEventClass event_list[2];
		stylus1_device = XOpenDevice(display, dev_info->id);
		if (!stylus1_device) {
			return 0;
		}
		if (stylus1_device->num_classes > 0) {
			for (ip = stylus1_device->classes, i = 0; i < stylus1_info->num_classes; ip++, i++) {
				switch (ip->input_class) {

				case ButtonClass:
				DeviceButtonPress(stylus1_device, button_press_type, event_list[count]);
				count++;
				break;

				case ValuatorClass:
				ProximityIn(stylus1_device, proximity_in_type, event_list[count]);
				count++;
				break;

				default:
				break;
				}
			}
		}
		if (XSelectExtensionEvent(display, root_win, event_list, count)) {
			return 0;
		}
		return count;
	}
	return 0;
}

/* End Code */

