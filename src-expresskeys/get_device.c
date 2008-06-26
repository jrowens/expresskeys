/*
 get_device.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.

 Copyright (C) 2005-2006 - Mats Johannesson

 Based on xinput.c 1996 by Frederic Lepied (xinput-1.2)

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
 Function queries the X server for input devices. It only cares about non
 core ones, and compares them with what was specified on the command line.
 If no device was specified we do an automatic search for 'pad' and 'stylus'.
 The "info" is a XDeviceInfo structure whos address is transferred to a
 permanent global copy for freeing at program exit
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* get_device_info returns a long to satisfy x86_64 systems */
long get_device_info(Display *display, XDeviceInfo *info, char *name)
{
	int	i, j, k, c;
	int	nr_devices;
	int len = 0;

	char read_buffer [MAXBUFFER];
	char write_buffer [MAXBUFFER];

	XValuatorInfoPtr valuator;
	XAnyClassPtr anyclass;

	info = XListInputDevices(display, &nr_devices);

	if (!pad1_info_block) {
		pad1_info_block = info;
	} else {
		if (!stylus1_info_block) {
			stylus1_info_block = info;
		}
	}

/* Device specified on the command line */

	if ((pad1_name) || (stylus1_name)) {
		for(i = 0; i < nr_devices; i++) {
			if ((info[i].use == IsXExtensionDevice) &&
			(strcmp (info[i].name, name) == 0)) {
				anyclass = (XAnyClassPtr)(info[i].inputclassinfo);
				for (k = 0; k < info[i].num_classes; k++) {
					if (anyclass->class == ValuatorClass) {
						valuator = (XValuatorInfoPtr) anyclass;
						if (stylus1_info_block == info) {
							stylus1_mode = valuator->mode;
						}
					}
					anyclass = (XAnyClassPtr)((char *)anyclass + anyclass->length);
				}
				/* Convert to long for x86_64 systems */
				return (long) &info[i];
			}
		}
		return 0;
	} else {

/* No device specified on the command line */

		for(i = 0; i < nr_devices; i++) {
			if (info[i].use == IsXExtensionDevice) {
				len = strlen(info[i].name);
				if (len < MAXBUFFER) {
					sprintf(read_buffer, "%s", info[i].name);
					for (j = 0; j < len; j++) {
						c = tolower(read_buffer[j]); /* Turn everything into lower case */
						sprintf(write_buffer+j, "%c", c);
						++read_buffer[j];
					}
					strncpy(write_buffer + len, "\0", 1);
					if ((strstr(write_buffer, name)) !=NULL) {
						anyclass = (XAnyClassPtr)(info[i].inputclassinfo);
						for (k = 0; k < info[i].num_classes; k++) {
							if (anyclass->class == ValuatorClass) {
								valuator = (XValuatorInfoPtr) anyclass;
								if ((strstr(write_buffer, "stylus"))) {
									stylus1_mode = valuator->mode;
								}
							}
							anyclass = (XAnyClassPtr)((char *)anyclass + anyclass->length);
						}
						/* Convert to long for x86_64 systems */
						return (long) &info[i];
					}
				}
			}
		}
	return 0;
	}
}

/* End Code */

