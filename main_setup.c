/*
 * main_setup.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
 *
 * Copyright (C) 2005 - Mats Johannesson
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

int main (int argc, char *argv[])
{

	if (PEN_MODE == "Absolute") {
		pen_mode = Absolute;
	} else {
		pen_mode = Relative;
	}

	if ((display = XOpenDisplay(NULL)) == NULL) {
		fprintf(stderr, "ERROR: Can not connect to your X Server\n");
		return EXIT_KO; 
	}

	screen = DefaultScreen(display);

	int event_base, error_base;
	int major_version, minor_version;
	if (!XTestQueryExtension (display, &event_base, &error_base,
		&major_version, &minor_version)) {
		fprintf (stderr, "ERROR: XTest extension not present on your X server\n");
		XCloseDisplay(display);
		return EXIT_KO;
	}

	XExtensionVersion *xinputext;
	xinputext = XGetExtensionVersion (display, INAME);
	if (xinputext && (xinputext != (XExtensionVersion*) NoSuchExtension)) {
		XFree(xinputext);
	} else {
		fprintf (stderr, "ERROR: XInput extension not present on your X server\n");
		XCloseDisplay(display);
		return EXIT_KO;
	}

	if (argc < 2) {
		fprintf(stderr, "Usage: expresskeys <device-name>\n");
		fprintf(stderr, "Example: expresskeys pad &\n");
		XCloseDisplay(display);
		return EXIT_KO;
	}

	pad_info = (void *) get_device_info(display, argv[1]);
	XFreeDeviceList (info);
	if (!pad_info) {
		fprintf(stderr, "ERROR: Can not find pad device: %s\n", argv[1]);
		XCloseDisplay(display);
		return EXIT_KO;
	}

	if (HANDLE_PEN){
		pen_info = (void *) get_device_info(display, PEN_NAME);
		XFreeDeviceList (info);
		if (!pen_info) {
			fprintf(stderr, "ERROR: Can not find pen device: %s\n", PEN_NAME);
			XCloseDisplay(display);
			return EXIT_KO;
		}
	}

	if (register_events(display, pad_info, argv[1])) {
		use_events(display);
	} else {
		fprintf(stderr, "ERROR: Could not register any events!\n");
		XCloseDisplay(display);
		return EXIT_KO;
	}

	XCloseDisplay(display);
	return EXIT_OK;
}

/* End Code */

