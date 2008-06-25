/* Version 0.06 29 March 2005
 *
 * To compile (example in 2 steps):
 * gcc -O2 -fomit-frame-pointer -c expresskeys.c
 * gcc -s -L/usr/X11R6/lib -o expresskeys expresskeys.o -lX11 -lXi -lXext -lXtst
 *
 * Run example: expresskeys pad &
 * Which will push it into the background. It is safe to close the terminal
 * afterwards. Oh, and X _must_ be running... The name, "pad" here, is
 * how it's called in xorg.conf (the "Identifier" option).
 *
 * Update example 17 March 2005: Myself I've put it in the .xinitrc as
 * "exec /usr/local/bin/expresskeys pad &" (without quotes) right before
 * the window manager is started.
 * 
 * Key configuration is easy to change in the "#define KEY_xx yy" below.
 * Use the "xev" program to find keycodes or look them up somewhere...
 * I've set the Wacom Intuos3 defaults on both sides, which is:
 * Shift, Alt, Control and Space. Touch strips are mildly supported.
 *
 * _Version 0.06 29 March 2005_
 * 
 * Touch Strip simple implementation. Default, if turned on, sends plus
 * (+) and minus (-) key presses based on finger/stylus up/down motion.
 * This was chosen for Gimp Zoom In/Out functionality. It must be turned
 * on by setting a value in the "user config area", just as for pen mode
 * handling. Default is off, don't handle the touch strips.
 * 
 * It turns out that with linuxwacom-0.6.7 (beta is out) this program
 * works better than ever! The blender "confusion" I talked about in
 * the previous version note has vanished completely. Also blender
 * zoom, translation and rotation work flawlessly with the pad buttons
 * and pen middle button (was half-working in linuxwacom-0.6.6). So the
 * XTestFakeKeyEvent was no bad choice at all. I'm very pleased :-)
 * 
 * _Version 0.05 16 March 2005_
 *
 * Bugfix. My key scan "case:, if, else, break" flow was somewhat borked.
 * Ugly function but does the right thing now. There are still issues
 * with (I believe) the timing of the XTestFakeKeyEvent of the XTest
 * extension. Using the "u" and "Shift-u" for undo and redo in blender
 * works, but sometimes blender gets confused. Waiting some seconds and
 * doing a "slow-push-release" of the key can fix the issue. Ah well,
 * this simulates keypresses, it's not the real thing... I'll look into
 * using another extension for the simulation.
 *
 * _Version 0.04 15 March 2005_
 *
 * Bugfix to handle certain key combinations better. Not perfect though.
 * Will debug further.
 *
 * _Version 0.03 15 March 2005_:
 * 
 * Handle a pen from the pad keys (toggle between absolute and relative mode).
 * See the new "user config area" for details. Observe: Whatever pen mode
 * you are in when exiting or killing this program, that's the pen mode
 * you have... So if wrong, fire up the program again and toggle until it's
 * the right mode. Default is off, don't handle a pen.
 * 
 * Clearly marked and cleaned up a "user config area" at the very beginning
 * of the code.
 * 
 * _Version 0.02 14 March 2005_:
 *
 * Added the option to specify an extra key for each pad key.
 * "#define KEY_xx_PLUS yy". By setting yy of KEY_11_PLUS to 57 you'd get
 * the famous Ctrl-n ;-). I needed this for undo and redo in blender.
 * 
 * _Version 0.01 14 March 2005_:
 *
 * Original release
 * 
 * Have fun,
 * Mats
 */
/*
 * CopyLeft 2005 by Mats Johannesson aka Voluspa
 * Hacked code from xinput-1.2.tar.gz (the test.c, setmode.c and xinput.c)
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

/* ++++++++++ Begin user config area ++++++++++ */

#define HANDLE_PEN 0	/* Main switch: 1 = yes handle a pen, 0 = no please */
#define PEN_NAME "stylus"   /* Identifier name as configured in xorg.conf */
#define PEN_MODE "Absolute" /* The mode we should expect the pen to be in */
			    /* when starting this program. Default usually */
			    /* is Absolute. The "mousy" feeling is Relative */

#define HANDLE_TOUCH 0	/* Main switch: 1 = yes handle touch strips, 0 = no */
#define L_TOUCH_UP 20	/* Default 20 = "+" = Gimp Zoom In. Left up motion*/
#define L_TOUCH_DOWN 61 /* Default 61 = "-" = Gimp Zoom Out. Left down motion */
#define R_TOUCH_UP 20	/* Default 20 = "+" = Gimp Zoom In. Right up motion */
#define R_TOUCH_DOWN 61 /* Default 61 = "-" = Gimp Zoom Out. Right down motion*/
			/* Change direction by switching _UP and _DOWN values */

/* Left ExpressKey Pad
------------ 
|  |   |   |		Wacom Intuos3 defaults are:
|  | 9 | T |
|11|---| O |		Key 9  = (left) Shift	= keycode 50
|  |10 | U |		Key 10 = (left) Alt	= keycode 64
|------| C |		Key 11 = (left) Control	= keycode 37
|  12  | H |		Key 12 = Space		= keycode 65
------------
Use the string TOGGLE_PEN if you want a key to do pen mode changes.
Eg: "#define KEY_11 TOGGLE_PEN" (the above HANDLE_PEN must also be set to 1)
*/
#define KEY_9 50
#define KEY_9_PLUS 0	/* extra key */
#define KEY_10 64
#define KEY_10_PLUS 0	/* extra key */
#define KEY_11 37
#define KEY_11_PLUS 0	/* extra key */
#define KEY_12 65
#define KEY_12_PLUS 0	/* extra key */

/* Right ExpressKey Pad
------------ 
|   |   |  |		Wacom Intuos3 defaults are:
| T |13 |  |
| O |---|15|		Key 13 = (left) Shift	= keycode 50
| U |14 |  |		Key 14 = (left) Alt	= keycode 64
| C |------|		Key 15 = (left) Control	= keycode 37
| H |  16  |		Key 16 = Space		= keycode 65
------------
Use the string TOGGLE_PEN if you want a key to do pen mode changes.
Eg: "#define KEY_15 TOGGLE_PEN" (the above HANDLE_PEN must also be set to 1)
*/
#define KEY_13 50
#define KEY_13_PLUS 0	/* extra key */
#define KEY_14 64
#define KEY_14_PLUS 0	/* extra key */
#define KEY_15 37
#define KEY_15_PLUS 0	/* extra key */
#define KEY_16 65
#define KEY_16_PLUS 0	/* extra key */

/* ++++++++++ End user config area ++++++++++ */

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
#define TOGGLE_PEN 999

Bool check_xinput (Display *display);
int find_device_info(Display *display, char *name, Bool only_extended);
int register_events(Display	*display, XDeviceInfo *info, char *dev_name);
int use_events(Display *display);
int toggle_pen_mode(Display *display, char *name);

int pen_mode = 1;
int elder_rotation = 4097;
int old_rotation = 4097;
int elder_throttle = 4097;
int old_throttle = 4097;

int main (int argc, char *argv[])
{

		if (PEN_MODE == "Absolute") {
		pen_mode = Absolute;
		} else {
		pen_mode = Relative;
		}

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
	return 0;
}

static int motion_type = INVALID_EVENT_TYPE;
static int button_press_type = INVALID_EVENT_TYPE;
static int button_release_type = INVALID_EVENT_TYPE;
int register_events(Display	*display, XDeviceInfo *info, char *dev_name)
{
	int number = 0;
	XEventClass event_list[3];
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

		case ValuatorClass:
		DeviceMotionNotify(device, motion_type, event_list[number]); number++;
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

	if (Event.type == motion_type) {

		if (HANDLE_TOUCH){
			int rotation;
			int throttle;
	
	    	XDeviceMotionEvent *motion = (XDeviceMotionEvent *) &Event;

			rotation = motion->axis_data[3];
			throttle = motion->axis_data[4];

			if (rotation > 1){
				if ((rotation < old_rotation) && (old_rotation <= elder_rotation)){
					XTestFakeKeyEvent(display, L_TOUCH_UP, True, CurrentTime);
					XTestFakeKeyEvent(display, L_TOUCH_UP, False, CurrentTime);
				}
				else if ((rotation > old_rotation) && (old_rotation >= elder_rotation)){
					XTestFakeKeyEvent(display, L_TOUCH_DOWN, True, CurrentTime);
					XTestFakeKeyEvent(display, L_TOUCH_DOWN, False, CurrentTime);
				}
			elder_rotation = old_rotation;
			old_rotation = rotation;
			}

			if (throttle > 1){
				if ((throttle < old_throttle) && (old_throttle <= elder_throttle)){
					XTestFakeKeyEvent(display, R_TOUCH_UP, True, CurrentTime);
					XTestFakeKeyEvent(display, R_TOUCH_UP, False, CurrentTime);
				}
				else if ((throttle > old_throttle) && (old_throttle >= elder_throttle)){
					XTestFakeKeyEvent(display, R_TOUCH_DOWN, True, CurrentTime);
					XTestFakeKeyEvent(display, R_TOUCH_DOWN, False, CurrentTime);
				}
			elder_throttle = old_throttle;
			old_throttle = throttle;
			}
		}
	}

	if (Event.type == button_press_type) {

		XDeviceButtonEvent *button = (XDeviceButtonEvent *) &Event;

		switch (button->button) {
			case 9:
		if (KEY_9 == TOGGLE_PEN)
			if (HANDLE_PEN)
				toggle_pen_mode(display, PEN_NAME);
			else
				break;
		else
		if (KEY_9)
XTestFakeKeyEvent(display, KEY_9, True, CurrentTime );
		if (KEY_9_PLUS)
XTestFakeKeyEvent(display, KEY_9_PLUS, True, CurrentTime );
		else
			break;
			break;
			case 10:
		if (KEY_10 == TOGGLE_PEN)
			if (HANDLE_PEN)
				toggle_pen_mode(display, PEN_NAME);
			else
				break;
		else
		if (KEY_10)
XTestFakeKeyEvent(display, KEY_10, True, CurrentTime );
		if (KEY_10_PLUS)
XTestFakeKeyEvent(display, KEY_10_PLUS, True, CurrentTime );
		else
			break;
			break;
			case 11:
		if (KEY_11 == TOGGLE_PEN)
			if (HANDLE_PEN)
				toggle_pen_mode(display, PEN_NAME);
			else
				break;
		else
		if (KEY_11)
XTestFakeKeyEvent(display, KEY_11, True, CurrentTime );
		if (KEY_11_PLUS)
XTestFakeKeyEvent(display, KEY_11_PLUS, True, CurrentTime );
		else
			break;
			break;
			case 12:
		if (KEY_12 == TOGGLE_PEN)
			if (HANDLE_PEN)
				toggle_pen_mode(display, PEN_NAME);
			else
				break;
		else
		if (KEY_12)
XTestFakeKeyEvent(display, KEY_12, True, CurrentTime );
		if (KEY_12_PLUS)
XTestFakeKeyEvent(display, KEY_12_PLUS, True, CurrentTime );
		else
			break;
			break;
			case 13:
		if (KEY_13 == TOGGLE_PEN)
			if (HANDLE_PEN)
				toggle_pen_mode(display, PEN_NAME);
			else
				break;
		else
		if (KEY_13)
XTestFakeKeyEvent(display, KEY_13, True, CurrentTime );
		if (KEY_13_PLUS)
XTestFakeKeyEvent(display, KEY_13_PLUS, True, CurrentTime );
		else
			break;
			break;
			case 14:
		if (KEY_14 == TOGGLE_PEN)
			if (HANDLE_PEN)
				toggle_pen_mode(display, PEN_NAME);
			else
				break;
		else
		if (KEY_14)
XTestFakeKeyEvent(display, KEY_14, True, CurrentTime );
		if (KEY_14_PLUS)
XTestFakeKeyEvent(display, KEY_14_PLUS, True, CurrentTime );
		else
			break;
			break;
			case 15:
		if (KEY_15 == TOGGLE_PEN)
			if (HANDLE_PEN)
				toggle_pen_mode(display, PEN_NAME);
			else
				break;
		else
		if (KEY_15)
XTestFakeKeyEvent(display, KEY_15, True, CurrentTime );
		if (KEY_15_PLUS)
XTestFakeKeyEvent(display, KEY_15_PLUS, True, CurrentTime );
		else
			break;
			break;
			case 16:
		if (KEY_16 == TOGGLE_PEN)
			if (HANDLE_PEN)
				toggle_pen_mode(display, PEN_NAME);
			else
				break;
		else
		if (KEY_16)
XTestFakeKeyEvent(display, KEY_16, True, CurrentTime );
		if (KEY_16_PLUS)
XTestFakeKeyEvent(display, KEY_16_PLUS, True, CurrentTime );
		else
			break;
			break;			
			default:
			break;
		}
	}

	if (Event.type == button_release_type) {
		XDeviceButtonEvent *button = (XDeviceButtonEvent *) &Event;

        switch (button->button) {
			case 9:
		if (KEY_9 == TOGGLE_PEN)
			break;
		else
		if (KEY_9_PLUS)
XTestFakeKeyEvent(display, KEY_9_PLUS, False, CurrentTime );
		if (KEY_9)
XTestFakeKeyEvent(display, KEY_9, False, CurrentTime );
		else
			break;
			break;
			case 10:
		if (KEY_10 == TOGGLE_PEN)		
			break;
		else
		if (KEY_10_PLUS)
XTestFakeKeyEvent(display, KEY_10_PLUS, False, CurrentTime );
		if (KEY_10)
XTestFakeKeyEvent(display, KEY_10, False, CurrentTime );
		else
			break;
			break;
			case 11:
		if (KEY_11 == TOGGLE_PEN)
			break;
		else
		if (KEY_11_PLUS)
XTestFakeKeyEvent(display, KEY_11_PLUS, False, CurrentTime );
		if (KEY_11)
XTestFakeKeyEvent(display, KEY_11, False, CurrentTime );
		else
			break;
			break;
			case 12:
		if (KEY_12 == TOGGLE_PEN)
			break;
		else
		if (KEY_12_PLUS)
XTestFakeKeyEvent(display, KEY_12_PLUS, False, CurrentTime );
		if (KEY_12)
XTestFakeKeyEvent(display, KEY_12, False, CurrentTime );
		else
			break;
			break;
			case 13:
		if (KEY_13 == TOGGLE_PEN)
			break;
		else
		if (KEY_13_PLUS)		
XTestFakeKeyEvent(display, KEY_13_PLUS, False, CurrentTime );
		if (KEY_13)
XTestFakeKeyEvent(display, KEY_13, False, CurrentTime );
		else
			break;
			break;
			case 14:
		if (KEY_14 == TOGGLE_PEN)
			break;
		else
		if (KEY_14_PLUS)
XTestFakeKeyEvent(display, KEY_14_PLUS, False, CurrentTime );
		if (KEY_14)
XTestFakeKeyEvent(display, KEY_14, False, CurrentTime );
		else
			break;
			break;
			case 15:
		if (KEY_15 == TOGGLE_PEN)
			break;
		else
		if (KEY_15_PLUS)
XTestFakeKeyEvent(display, KEY_15_PLUS, False, CurrentTime );
		if (KEY_15)
XTestFakeKeyEvent(display, KEY_15, False, CurrentTime );
		else
			break;
			break;
			case 16:
		if (KEY_16 == TOGGLE_PEN)
			break;
		else
		if (KEY_16_PLUS)
XTestFakeKeyEvent(display, KEY_16_PLUS, False, CurrentTime );
		if (KEY_16)
XTestFakeKeyEvent(display, KEY_16, False, CurrentTime );
		else
			break;
			break;
			default:
			break;
			}
		}
	}
}

int toggle_pen_mode(Display *display, char *name)
{

	XDeviceInfo	*info;
	XDevice		*device;

	info = find_device_info(display, PEN_NAME, True);

	if (!info) {
		fprintf(stderr, "unable to find device %s\n", PEN_NAME);
		return 0;
	}

	if (pen_mode == Absolute) {
	pen_mode = Relative;
	} else {
	pen_mode = Absolute;
	}

	device = XOpenDevice(display, info->id);

	if (device) {
	XSetDeviceMode(display, device, pen_mode);
	return 0;
	} else {
	fprintf(stderr, "Unable to open device %s\n", PEN_NAME);
	return 0;
	}
}

/* End code */

