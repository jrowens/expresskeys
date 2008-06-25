/* Version 0.01 14 March 2005
 *
 * To compile (for example, 2 steps):
 * gcc -O2 -fomit-frame-pointer -c expresskeys.c
 * gcc -s -L/usr/X11R6/lib -o expresskeys expresskeys.o -lX11 -lXi -lXext -lXtst
 *
 * Run example: expresskeys pad &
 * Which will push it into the background. It is safe to close the terminal
 * afterwards. Oh, and X _must_ be running...
 * 
 * Key configuration is easy to change in the "#define KEY_xx yy" below.
 * Use the "xev" program to find keycodes or look them up somewhere...
 * I've set the Wacom Intuos3 defaults on both sides, which is:
 * Shift, Alt, Control and Space. Touch strips are not supported, yet.
 *
 * Have fun,
 * Mats
 */
/*
 * CopyLeft 2005 by Mats Johannesson aka Voluspa
 * Hacked code from xinput-1.2.tar.gz (the test.c and xinput.c)
 * Most are left as Frederic Lepied wrote it - I'm no coder!
 */
/*
 * Copyright 1996 by Frederic Lepied, France. <Frederic.Lepied@sugix.frmug.org>
 *                                                                            
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is  hereby granted without fee, provided that
 * the  above copyright   notice appear  in   all  copies and  that both  that
 * copyright  notice   and   this  permission   notice  appear  in  supporting
 * documentation, and that   the  name of  Frederic   Lepied not  be  used  in
 * advertising or publicity pertaining to distribution of the software without
 * specific,  written      prior  permission.     Frederic  Lepied   makes  no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.                   
 *                                                                            
 * FREDERIC  LEPIED DISCLAIMS ALL   WARRANTIES WITH REGARD  TO  THIS SOFTWARE,
 * INCLUDING ALL IMPLIED   WARRANTIES OF MERCHANTABILITY  AND   FITNESS, IN NO
 * EVENT  SHALL FREDERIC  LEPIED BE   LIABLE   FOR ANY  SPECIAL, INDIRECT   OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA  OR PROFITS, WHETHER  IN  AN ACTION OF  CONTRACT,  NEGLIGENCE OR OTHER
 * TORTIOUS  ACTION, ARISING    OUT OF OR   IN  CONNECTION  WITH THE USE    OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XTest.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define EXIT_OK 1
#define EXIT_KO 0
#define INVALID_EVENT_TYPE -1

/* Left ExpressKey Pad
------------ 
|  |   |   |
|  | 9 | T |
|11|---| O |
|  |10 | U |
|------| C |
|  12  | H |
------------
*/
#define KEY_9 50   /* Shift_L   */
#define KEY_10 64  /* Alt_L     */
#define KEY_11 37  /* Control_L */
#define KEY_12 65  /* Space     */

/* Right ExpressKey Pad
------------ 
|   |   |  |
| T |13 |  |
| O |---|15|
| U |14 |  |
| C |------|
| H |  16  |
------------
*/
#define KEY_13 50  /* Shift_L   */
#define KEY_14 64  /* Alt_L     */
#define KEY_15 37  /* Control_L */
#define KEY_16 65  /* Space     */

Bool check_xinput (Display *display);
int find_device_info(Display *display, char *name, Bool only_extended);
int register_events(Display	*display, XDeviceInfo *info, char *dev_name);
int use_events(Display *display);

int main (int argc, char *argv[])
{

Display *display = XOpenDisplay(NULL);

	if (display == NULL) {
		fprintf(stderr, "Can not connect to X-Server\n");
		return EXIT_KO;
	}

	if (!check_xinput(display)) {
		fprintf(stderr, "%s extension not present\n", INAME);
		XCloseDisplay(display);
		return EXIT_KO;
	}

	if (argc < 2) {
		fprintf(stderr, "Usage: expresskeys <device-name>\n");
		fprintf(stderr, "Example: expresskeys pad &\n");
		XCloseDisplay(display);
		return EXIT_KO;
	}

	XDeviceInfo	*info;
	int		idx = 1;

	info = find_device_info(display, argv[idx], True);

		if (!info) {
		fprintf(stderr, "Unable to find device %s\n", argv[idx]);
		XCloseDisplay(display);		
		return EXIT_KO;
	}

	if (register_events(display, info, argv[idx])) {
		use_events(display);
	} else {
		fprintf(stderr, "No event registered...\n");
		XCloseDisplay(display);
		return EXIT_KO;
		}

	XCloseDisplay(display);
	return EXIT_OK;
}

Bool check_xinput (Display *display)
{
	XExtensionVersion *version;
	Bool present;
	
	version = XGetExtensionVersion (display, INAME);
	
	if (version && (version != (XExtensionVersion*) NoSuchExtension)) {
		present = version->present;
		XFree(version);
		return present;
	} else {
		return False;
	}
}	

int find_device_info(Display *display, char *name, Bool only_extended)
{
	XDeviceInfo	*devices; 
	int		loop;
	int		num_devices;
	int		len = strlen(name);
	Bool		is_id = True;
	XID		id = 0;
    
	for(loop=0; loop<len; loop++) {
	if (!isdigit(name[loop])) {
		is_id = False;
		break;
	}
	}

	if (is_id) {
	id = atoi(name);
	}
    
	devices = XListInputDevices(display, &num_devices);

	for(loop=0; loop<num_devices; loop++) {
	if ((!only_extended || (devices[loop].use == IsXExtensionDevice)) &&
		((!is_id && strcmp(devices[loop].name, name) == 0) ||
		(is_id && devices[loop].id == id))) {
	     	return &devices[loop];
		}
	}
	return NULL;
}

static int button_press_type = INVALID_EVENT_TYPE;
static int button_release_type = INVALID_EVENT_TYPE;
int register_events(Display	*display, XDeviceInfo *info, char *dev_name)
{
	int number = 0;
	XEventClass event_list[2];
	int i;
	XDevice *device;
	Window root_win;
	unsigned long screen;
	XInputClassInfo *ip;

	screen = DefaultScreen(display);
	root_win = RootWindow(display, screen);

	device = XOpenDevice(display, info->id);

	if (!device) {
	fprintf(stderr, "Unable to open device %s\n", dev_name);
	return 0;
	}

	if (device->num_classes > 0) {
	for (ip = device->classes, i=0; i<info->num_classes; ip++, i++) {
		switch (ip->input_class) {
		case ButtonClass:
		DeviceButtonPress(device, button_press_type, event_list[number]); number++;
		DeviceButtonRelease(device, button_release_type, event_list[number]); number++;
		break;
		default:
		break;
	    }
		}
	if (XSelectExtensionEvent(display, root_win, event_list, number)) {
		fprintf(stderr, "Error selecting extended events\n");
		return 0;		
		}
	}
	return number;	
}

int use_events(Display *display)
{
	XEvent Event;
	while(1) {
	XNextEvent(display, &Event);

	if (Event.type == button_press_type) {

		XDeviceButtonEvent *button = (XDeviceButtonEvent *) &Event;

		switch (button->button) {
			case 9:
XTestFakeKeyEvent(display, KEY_9, True, CurrentTime );
			break;
			case 10:
XTestFakeKeyEvent(display, KEY_10, True, CurrentTime );
			break;
			case 11:
XTestFakeKeyEvent(display, KEY_11, True, CurrentTime );
			break;
			case 12:
XTestFakeKeyEvent(display, KEY_12, True, CurrentTime );
			break;
			case 13:
XTestFakeKeyEvent(display, KEY_13, True, CurrentTime );
			break;
			case 14:
XTestFakeKeyEvent(display, KEY_14, True, CurrentTime );
			break;
			case 15:
XTestFakeKeyEvent(display, KEY_15, True, CurrentTime );
			break;
			case 16:
XTestFakeKeyEvent(display, KEY_16, True, CurrentTime );
			break;			
			default:
			break;
		}
	}

	if (Event.type == button_release_type) {
		XDeviceButtonEvent *button = (XDeviceButtonEvent *) &Event;

        switch (button->button) {
			case 9:
XTestFakeKeyEvent(display, KEY_9, False, CurrentTime );
			break;
			case 10:
XTestFakeKeyEvent(display, KEY_10, False, CurrentTime );
			break;
			case 11:
XTestFakeKeyEvent(display, KEY_11, False, CurrentTime );
			break;
			case 12:
XTestFakeKeyEvent(display, KEY_12, False, CurrentTime );
			break;
			case 13:
XTestFakeKeyEvent(display, KEY_13, False, CurrentTime );
			break;
			case 14:
XTestFakeKeyEvent(display, KEY_14, False, CurrentTime );
			break;
			case 15:
XTestFakeKeyEvent(display, KEY_15, False, CurrentTime );
			break;
			case 16:
XTestFakeKeyEvent(display, KEY_16, False, CurrentTime );
			break;
			default:
			break;
			}
		}
	}
}

/* End code */

