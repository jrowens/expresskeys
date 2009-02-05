/*
 expresskeys - Support ExpressKeys, Touch Strips, Scroll Wheel on Wacom tablets.

 Copyright (C) 2005-2007 - Mats Johannesson

 register_events() based on test.c 1996 by Frederic Lepied (xinput-1.2)

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

#include <stdio.h> /* NULL, FILE, fgets, popen, pclose, snprintf */
#include <string.h> /* strlen, strstr, strcspn, strncmp, strcmp */
#include <ctype.h> /* isdigit, tolower */
#include <stdlib.h> /* atoi */
#include <X11/extensions/XInput.h>

#include "defines.h"

/* Globals: */

/* State flags: */
int have_pad;
int ok_xsetwacom;
int xsetwacom_version;

/* Numbers we get from registering our interest in certain events: */
int button_press_type;
int button_release_type;
int motion_type;
int proximity_in_type;
int proximity_out_type;

/* Used in many situations as identification, for math and for flow control: */
const int bbo = 6;
const int bee = 5;
const int i3 = 4;
const int i3s = 3;
const int g4 = 2;
const int g4b = 1;
const int nop = 0;

/* The large structure emanating from XListInputDevices: */
XDeviceInfo* xdevice_list;

/* Arrays used as storage before transfer to structures, and for quick loops: */
XDevice* pad_xdevice[MAXMODEL][MAXPAD];
XDevice* stylus_xdevice[MAXSTYLUS][MAXMODEL * MAXPAD];

unsigned long int* pad_id[MAXMODEL][MAXPAD];
unsigned long int* stylus_id[MAXSTYLUS][MAXMODEL * MAXPAD];

const char* pad_name[MAXMODEL][MAXPAD];
const char* stylus_name[MAXSTYLUS][MAXMODEL * MAXPAD];

unsigned char* stylus_mode[MAXSTYLUS][MAXMODEL * MAXPAD];

/* 'Internal' Globals: */

/* Counting to keep track, and store in the right array element: */
static int pad_num;
static int stylus_num;

/* Recycled placeholder: */
static XDevice* tmp_device;

/* Externals: */

extern int screen;
extern Display* display;

extern const char* our_prog_name;

extern const char* pad_string;
extern const char* stylus_string;

extern const char* user_pad;
extern const char* user_stylus1;
extern const char* user_stylus2;

static int get_xsetwacom_version(void)
{
	FILE* execfp = NULL;
	int major, minor, release, ret;
	char read_buffer[MAXBUFFER];

	major = minor = release = 0;

	if ((execfp = popen("xsetwacom -V", "r")) != NULL) {
		fgets(read_buffer, MAXBUFFER, execfp);
		if (pclose(execfp) != NON_VALID) {
			ret = sscanf(read_buffer, "%i.%i.%i", &major, &minor,
				     &release);
			if (ret == 3)
				return major * 100 + minor * 10 + release;
		}
	}

	return 0;
}

#define setwacom_ignore_result(buffer, buff_size, device, prop, value) \
		{\
			FILE *pfd; \
			snprintf(buffer, buff_size, "xsetwacom set %s "prop " " value, \
								device); \
			pfd = popen(buffer, "r"); \
			if (pclose(pfd) == NON_VALID) \
				fprintf(stderr, "Error setting " prop " as " value "\n"); \
		}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 This function uses the "popen" command which creates a pipe, forks and
 invokes a shell where xsetwacom can be run. First action is to ensure that
 version 0.0.7 or greater of xsetwacom is present (ie linuxwacom-0.7.5-2
 where the option GetTabletID was introduced). Thereafter we match the tablet
 decimal number string against known tablet numbers. A full table can be
 found in src/xdrv/wcmUSB.c of the linuxwacom sources (Hex numbers).
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static int identify_device(char* device_name)
{
	const char* cintiq_20wsx = "197"; /* 0xC5 */
	const char* cintiq_21UX = "63"; /* 0x3F */
	const char* i3_6x8 = "177"; /* 0xB1 */
	const char* i3_9x12 = "178"; /* 0xB2 */
	const char* i3_12x12 = "179"; /* 0xB3 */
	const char* i3_12x19 = "180"; /* 0xB4 */
	const char* i3_6x11 = "181"; /* 0xB5 */
	const char* i3s_4x5 = "176"; /* 0xB0 */
	const char* i3s_4x6 = "183"; /* 0xB7 */
	const char* g4_4x5 = "21"; /* 0x15 */
	const char* g4_6x8 = "22"; /* 0x16 */
	const char* g4b_6x8 = "129"; /* 0x81 */
	const char* bamboo = "101"; /* 0x65 */

/* Minimum xsetwacom version we can use is 0.0.7 */
	const int min_xsetwacom = 7;

	char read_buffer[MAXBUFFER];
	char write_buffer[MAXBUFFER];

	int len = 0;
	int ok_value = 0;

	FILE* execfp = NULL;
	read_buffer[0] = '\0';

	ok_xsetwacom = 0;
	xsetwacom_version = get_xsetwacom_version();
	if (xsetwacom_version > min_xsetwacom) {
		ok_xsetwacom = 1;

		/*
		 * linuxwacom >= 0.8.0 (xsetwacom 0.1.0) handles the
		 * touchstrips and expresskeys by default. let's make
		 * sure those settings are cleared.
		 */
		if (xsetwacom_version >= 10) {
			printf("Info: linuxwacom 0.8.0 or newer detected, the "
			       "TouchStrips and ExpressKeys settings will "
			       "be reset.\n");
			setwacom_ignore_result(write_buffer, MAXBUFFER, device_name, "RelWUp", "0");
			setwacom_ignore_result(write_buffer, MAXBUFFER, device_name, "RelWDn", "0");
			setwacom_ignore_result(write_buffer, MAXBUFFER, device_name, "AbsWUp", "0");
			setwacom_ignore_result(write_buffer, MAXBUFFER, device_name, "AbsWDn", "0");

			setwacom_ignore_result(write_buffer, MAXBUFFER, device_name, "StripRUp", "0");
			setwacom_ignore_result(write_buffer, MAXBUFFER, device_name, "StripRDn", "0");
			setwacom_ignore_result(write_buffer, MAXBUFFER, device_name, "StripLUp", "0");
			setwacom_ignore_result(write_buffer, MAXBUFFER, device_name, "StripLDn", "0");
		}
	}

/* linuxwacom-0.7.7-3 changed GetTabletID to plain TabletID. Later, support
 for both strings were introduced. We follow the same pattern here, defaulting
 to the new way, should the old format disappear in a linuxwacom future: */
	if (ok_xsetwacom) {
		read_buffer[0] = '\0';
		snprintf(write_buffer,MAXBUFFER, "xsetwacom get %s TabletID",
								device_name);
		if ((execfp = popen(write_buffer, "r")) != NULL) {
			fgets(read_buffer, MAXBUFFER, execfp);
			if (((pclose(execfp)) != NON_VALID)
			&& (isdigit(read_buffer[0]))) {
				ok_value = 1;
			} else {
				read_buffer[0] = '\0';
				snprintf(write_buffer,MAXBUFFER,
				"xsetwacom get %s GetTabletID", device_name);
				if ((execfp = popen(write_buffer, "r"))
								!= NULL) {
					fgets(read_buffer, MAXBUFFER, execfp);
					if (((pclose(execfp)) != NON_VALID)
					&& (isdigit(read_buffer[0]))) {
						ok_value = 1;
					}
				}
			}
		}

		if (ok_value) {
			len = strcspn(read_buffer, " \t\n");
			if ((strncmp(read_buffer, bamboo, len)) == 0) {
				return bbo;
			}
			if ((strncmp(read_buffer, cintiq_20wsx, len)) == 0) {
				return bee;
			}
			if ((strncmp(read_buffer, cintiq_21UX, len)) == 0) {
				return i3;
			}
			if (((strncmp(read_buffer, i3_6x8, len)) == 0)
				|| ((strncmp(read_buffer, i3_9x12, len)) == 0)
				|| ((strncmp(read_buffer, i3_12x12, len)) == 0)
				|| ((strncmp(read_buffer, i3_12x19, len)) == 0)
				|| ((strncmp(read_buffer, i3_6x11, len)) == 0)){
				return i3;
			}
			if (((strncmp(read_buffer, i3s_4x5, len)) == 0)
				|| ((strncmp(read_buffer, i3s_4x6, len)) == 0)){
				return i3s;
			}
			if (((strncmp(read_buffer, g4_4x5, len)) == 0)
				|| ((strncmp(read_buffer, g4_6x8, len)) == 0)) {
				return g4;
			}
			if ((strncmp(read_buffer, g4b_6x8, len)) == 0) {
				return g4b;
			}
		}
	}

/* We must treat all tablets as 'padless' if xsetwacom is missing or too old: */
	if (!ok_xsetwacom) {
		have_pad = 0;
	}
	return nop;

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Start looking for supported event types. The scope should be the root window
 (ie everywhere). We're interested in motion events and proximity in/out for
 the touch strips, and button press/release for the pad buttons. For the
 styli we ask about button press and proximity in. Having found the info
 we ask the X server to keep us continuously notified about these events:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static int register_events(const char* name)
{
	int i;
	int count = 0;

	XInputClassInfo* ip;
	Window root_win;

	root_win = RootWindow(display, screen);

	if (name == pad_string) {
		XEventClass event_list[5];
		if (tmp_device->num_classes > 0) {
			for (ip = tmp_device->classes,
				i = 0; i < tmp_device->num_classes; ip++, i++) {
				switch (ip->input_class) {

				case ButtonClass:
				DeviceButtonPress(tmp_device,
					button_press_type, event_list[count]);
				count++;
				DeviceButtonRelease(tmp_device,
					button_release_type, event_list[count]);
				count++;
				break;

				case ValuatorClass:
				DeviceMotionNotify(tmp_device,
					motion_type, event_list[count]);
				count++;
				ProximityIn(tmp_device,
					proximity_in_type, event_list[count]);
				count++;
				ProximityOut(tmp_device,
					proximity_out_type, event_list[count]);
				count++;
				break;

				default:
				break;
				}
			}
		}
		if (XSelectExtensionEvent(display, root_win,
					event_list, count)) {
			return 0;
		}
		return count;
	}

	if (name == stylus_string) {
		XEventClass event_list[2];
		if (tmp_device->num_classes > 0) {
			for (ip = tmp_device->classes,
				i = 0; i < tmp_device->num_classes; ip++, i++) {
				switch (ip->input_class) {

				case ButtonClass:
				DeviceButtonPress(tmp_device,
					button_press_type, event_list[count]);
				count++;
				break;

				case ValuatorClass:
				ProximityIn(tmp_device,
					proximity_in_type, event_list[count]);
				count++;
				break;

				default:
				break;
				}
			}
		}
		if (XSelectExtensionEvent(display, root_win,
					event_list, count)) {
			return 0;
		}
		return count;
	}
	return 0;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Find next empty element in the pad-array:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static int count_pad(int model)
{
	int i;
	for (i = 0; i < MAXPAD; i++) {
		if (!pad_xdevice[model][i]) {
			break;
		}
	}
	return i;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Find next empty element in the stylus-array:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static int count_stylus(int model)
{
	int i, j;
	pad_num = 0;

	for (j = 0; j < MAXPAD && pad_num == 0; j++) {
		for (i = 0; i < MAXSTYLUS; i++) {
			if (!stylus_xdevice[i][model * MAXPAD + j]) {
				pad_num = 1;
				break;
			}
		}
	}
	pad_num = model * MAXPAD + --j;
	return i;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Open a pad-device and start collecting data:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void follow_pad(XDeviceInfo* xdevice_list, int number)
{
	int model;

	if ((tmp_device = XOpenDevice(display, xdevice_list[number].id))) {
		if (register_events(pad_string)) {
			model = identify_device(xdevice_list[number].name);
			pad_num = count_pad(model);
			if (pad_num < MAXPAD) {
				pad_xdevice[model][pad_num] = tmp_device;
				pad_id[model][pad_num] = &tmp_device->device_id;
				pad_name[model][pad_num] = xdevice_list[number]
									.name;
				have_pad = 1;
			} else {
				XFree(tmp_device);
			}
		}
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Open a stylus-device and start collecting data:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void follow_stylus(XDeviceInfo* xdevice_list, int number)
{
	int i;
	int model;

	XValuatorInfoPtr valuator;
	XAnyClassPtr anyclass;

	if ((tmp_device = XOpenDevice(display, xdevice_list[number].id))) {
		if (register_events(stylus_string)) {
			model = identify_device(xdevice_list[number].name);
			stylus_num = count_stylus(model);
			if ((pad_num < model * MAXPAD + MAXPAD)
			&& (stylus_num < MAXSTYLUS)) {
				stylus_xdevice[stylus_num][pad_num]= tmp_device;
				stylus_id[stylus_num][pad_num] =
							&tmp_device->device_id;
				stylus_name[stylus_num][pad_num] =
						xdevice_list[number].name;
				anyclass = (XAnyClassPtr)
					(xdevice_list[number].inputclassinfo);
				for (i = 0; i < xdevice_list[number]
							.num_classes; i++) {
					if (anyclass->class == ValuatorClass) {
						valuator =
						(XValuatorInfoPtr)anyclass;
						stylus_mode[stylus_num][pad_num]
							= &valuator->mode;
					}
					anyclass = (XAnyClassPtr)
					((char*)anyclass + anyclass->length);
				}
			} else {
				XFree(tmp_device);
			}
		}
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 First processing of an extension-device's name-string:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static int check_name(char* read_buffer, char* write_buffer, int len)
{
	int i;

	if (len >= MAXBUFFER) {
		len = MAXBUFFER - 1;
	}

	for (i = 0; i < len; i++) {
		write_buffer[i] = tolower(read_buffer[i]);
	}
	write_buffer[i] = '\0';

	if ((strstr(write_buffer, pad_string) !=NULL)
		|| (strstr(write_buffer, stylus_string) !=NULL)) {
		return 1;
	}
	return 0;

}

/* newer Xorg servers changed the "use" field on the list of input devices */
#ifdef IsXExtensionKeyboard
#define EXTENSION_DEVICE IsXExtensionKeyboard
#else
#define EXTENSION_DEVICE IsXExtensionDevice
#endif

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Find all extension-devices containing the strings 'pad'/'stylus'. If the
 user has specified which pad/stylus(es) to use on the command line, we
 ignore all others:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void get_device_info()
{
	int i;
	int len;
	int found;
	int nr_devices;

	char read_buffer[MAXBUFFER];
	char write_buffer[MAXBUFFER];

	const char* user_dummy = "not_specified";
	if ((user_pad) || (user_stylus1)) {
		if (user_pad) {
			pad_string = user_pad;
		} else {
			pad_string = user_dummy;
		}
		if (user_stylus1) {
			stylus_string = user_stylus1;
		} else {
			stylus_string = user_dummy;
		}
	}

	xdevice_list = XListInputDevices(display, &nr_devices);

	for(i = 0; i < nr_devices; i++) {
		if (xdevice_list[i].use == EXTENSION_DEVICE) {
			len = strlen(xdevice_list[i].name);
			snprintf(read_buffer, MAXBUFFER, "%s", xdevice_list[i]
									.name);
			if (check_name(read_buffer, write_buffer, len)) {
				found = 0;
				if (user_pad) {
					if (strcmp(read_buffer, pad_string)
									== 0) {
						found = 1;
					}
				} else if (strstr(write_buffer, pad_string)) {
					found = 1;
				}
				if ((strstr(write_buffer, pad_string))
								&& (found)) {
					follow_pad(xdevice_list, i);
				}
				if (user_stylus1) {
					if (strcmp(read_buffer, stylus_string)
									== 0) {
						found = 1;
					}
				} else if (strstr(write_buffer,
							stylus_string)) {
					found = 1;
				}
				if ((strstr(write_buffer, stylus_string))
								&& (found)) {
					follow_stylus(xdevice_list, i);
					if (user_stylus2) {
						stylus_string = user_stylus2;
					}
				}
			}
		}
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Compare device names specified on the command line with found devices:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void check_user(FILE* errorfp)
{
	int userpad = 0;
	int userstylus1 = 0;
	int userstylus2 = 0;

	int i, j;

	if (user_pad) {
		for (i = 0; i < MAXMODEL && userpad == 0; i++) {
			for (j = 0; j < MAXPAD; j++) {
				if (pad_name[i][j]) {
					if (strcmp(user_pad,
							pad_name[i][j]) == 0) {
						userpad = 1;
						break;
					}
				}
			}
		}
		if (!userpad) {
			exit_on_error(errorfp,
			"\n%s ERROR: User named device \"%s\" not found!\n",
						our_prog_name, user_pad);
		}
	}

	if (user_stylus1) {
		for (i = 0; i < MAXSTYLUS && userstylus1 == 0; i++) {
			for (j = 0; j < MAXMODEL * MAXPAD; j++){
				if (stylus_name[i][j]) {
					if (strcmp(user_stylus1,
						stylus_name[i][j]) == 0) {
						userstylus1 = 1;
						break;
					}
				}
			}
		}
		if (!userstylus1) {
			exit_on_error(errorfp,
			"\n%s ERROR: User named device \"%s\" not found!\n",
						our_prog_name, user_stylus1);
		}
	}

	if (user_stylus2) {
		for (i = 0; i < MAXSTYLUS && userstylus2 == 0; i++) {
			for (j = 0; j < MAXMODEL * MAXPAD; j++){
				if (stylus_name[i][j]) {
					if (strcmp(user_stylus2,
						stylus_name[i][j]) == 0) {
						userstylus2 = 1;
						break;
					}
				}
			}
		}
		if (!userstylus2) {
			exit_on_error(errorfp,
			"\n%s ERROR: User named device \"%s\" not found!\n",
						our_prog_name, user_stylus2);
		}
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 XListInputDevices doesn't provide the devices in a predictable order, so the
 stylus array could be out of 'sync' with reality. For example, one tablet
 might get two styli, while another tablet of the same model gets none.
 The crude heuristics used below is: "If the next tablet of this model has
 an empty 'slot' zero, and the previous tablet more than 'slot' zero occupied,
 transfer the last stylus from the previous tablet to the next tablet". The
 user can also list the styli differently in xorg.conf to correct errors:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void shuffle_styli()
{
	int i, j;

	for (i = MAXSTYLUS - 1; i < MAXSTYLUS; i++) {
		for (j = 0; j < MAXMODEL * MAXPAD; j++){
			if (stylus_xdevice[i][j]) {
				if (((j+1)-((j/MAXPAD)*MAXPAD)) < MAXPAD) {
					if ((pad_xdevice[j/MAXPAD][(j+1)-
							((j/MAXPAD)*MAXPAD)])
						&& (!stylus_xdevice[0][j+1])) {
						stylus_xdevice[0][j+1]
							= stylus_xdevice[i][j];
						stylus_xdevice[i][j] = NULL;
						stylus_id[0][j+1]
							= stylus_id[i][j];
						stylus_id[i][j] = NULL;
						stylus_name[0][j+1]
							= stylus_name[i][j];
						stylus_name[i][j] = NULL;
						stylus_mode[0][j+1]
							= stylus_mode[i][j];
						stylus_mode[i][j] = NULL;
					}
				}
			}
		}
	}

}

/* End Code */

