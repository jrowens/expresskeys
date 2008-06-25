/* Version 0.07 2 April 2005
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
 * Key configuration is easy to change in the "user config area" below.
 * Use the "xev" program to find keycodes or look them up somewhere...
 * I've set the Wacom Intuos3 defaults on both sides, which is:
 * Shift, Alt, Control and Space. Touch strips are mildly supported.
 *
 * Note 2 April 2005: Sometimes desktops or window managers "steal"
 * certain keypresses/combinations. If you experience that, look for
 * a way to change the keybindings of your environment.
 * 
 * _Version 0.07 2 April 2005_
 * 
 * Multiple configurations to rule them all... Yes, we now send
 * keypresses intelligently based on several configurations. I've
 * included a "default" catch all type, one for Gimp, for Blender
 * and for XTerm. Observe the spelling! It is case sensitive.
 * 
 * To create a new definition, just copy a full block and alter the
 * Name and the keycodes. To find the proper name of a program/window
 * fire up "xprop". It should be included with your X. xprop without
 * any arguments expects you to then click on the target window.
 * What comes out is a flood of information in the terminal window
 * you used to run xprop from. What we're after is something called
 * WM_CLASS. And within that, only one string. Let me show you:
 * 
 * $ xprop | grep WM_CLASS
 * WM_CLASS(STRING) = "<unknown>", "Eclipse"
 * 
 * It's the last string we would use, the "Eclipse" part. That is,
 * if we were doing a definition for this program, an IDE ;-)
 * 
 * You can see above why I use the last part. Program windows do not
 * always set their "name" (the first string). But they should
 * absolutely set the "class" they belong to, which often coincides
 * with the name.
 * 
 * So non-technically, this is how expresskeys works now:
 * 
 * 1) Pad button pressed or Touch strips touched.
 * 2) Examine which window is the current active one (has focus).
 * 3) Get the "class" name of the window.
 * 4) Compare that name with an internal list of program names.
 * 5) If a match is found, use those keydefinitions.
 * 6) If no match is found, use a "default" set of definitions.
 * 7) Send the keypress to the specified window.
 * 
 * In order to achieve this functionality I had to change the
 * "user config area" somewhat. I've done my very best to retain
 * a simple design, and at the same time keep it compact. But
 * success is in the eye of the beholder... Cut out example:
 * 
 *//* 	key_9	*//* <-- A visual reminder of which pad button it is. */
/*	50,		<-- The actual keycode and a COMMA (don't erase it).
 * 
 * Otherwise all the keys and options from past versions are, almost,
 * the same. End Version Note Rant.
 * 
 * _Version 0.06 29 March 2005_
 * 
 * Comment 2 April 2005: This is default only in Gimp. Basic defaults
 * are now Arrow-keys Up/Down/Right/Left. End comment.
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
 * Comment 2 April 2005: These instructions are dated. #define
 * for the keycodes are not used anymore. End comment.
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
 * CopyLeft 2005 by Mats Johannesson aka Voluspa. One working address
 * is devel with the ISP bredband which is a net domain.
 *
 * Hacked code from xinput-1.2.tar.gz (the test.c, setmode.c and xinput.c)
 * Most of the pure xinput is left as Frederic Lepied wrote it - I'm no coder!
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
#define PEN_NAME "stylus"	/* Identifier name as configured in xorg.conf */
#define PEN_MODE "Absolute"	/* The mode we should expect the pen to be in */
			/* when starting this program. Default usually */
			/* is Absolute. The "mousy" feeling is Relative */

/* Now, on to the next area. Please walk this way madam!		*/
/*----------------------------------------------.			*/
struct program {		/*		|			*/
	char *class_name;	/*		|			*/
	int handle_touch;	/*		|			*/
	int l_touch_up;		/*		|			*/
	int l_touch_down;	/*		|			*/
	int r_touch_up;		/*		|			*/
	int r_touch_down;	/*		|			*/
	int key_9;		/*		|			*/
	int key_9_plus;		/*		|			*/
	int key_10;		/*		|			*/
	int key_10_plus;	/*		|			*/
	int key_11;		/*	Nothing to see here		*/
	int key_11_plus;	/*	madam... Please move		*/
	int key_12;		/*	along.				*/
	int key_12_plus;	/*		|			*/
	int key_13;		/*		|			*/
	int key_13_plus;	/*		|			*/
	int key_14;		/*		|			*/
	int key_14_plus;	/*		|			*/
	int key_15;		/*		|			*/
	int key_15_plus;	/*		|			*/
	int key_16;		/*		|			*/
	int key_16_plus;	/*		|			*/
} prog_list[] = {		/*		|			*/
				/*		|			*/
/*-------------------------------.		V			*/
/*					Go further down, past
 * 					these comments for the
 * 					real configuration area.
 */
/* Left ExpressKey Pad
------------ 
|  |   |   |		Wacom Intuos3 defaults are:
|  | 9 | T |
|11|---| O |		Key 9  = (left) Shift	= keycode 50
|  |10 | U |		Key 10 = (left) Alt	= keycode 64
|------| C |		Key 11 = (left) Control	= keycode 37
|  12  | H |		Key 12 = Space		= keycode 65
------------
*/
/* Right ExpressKey Pad
------------ 
|   |   |  |		Wacom Intuos3 defaults are:
| T |13 |  |
| O |---|15|		Key 13 = (left) Shift	= keycode 50
| U |14 |  |		Key 14 = (left) Alt	= keycode 64
| C |------|		Key 15 = (left) Control	= keycode 37
| H |  16  |		Key 16 = Space		= keycode 65
------------
*/

/* The top configuration (named "default") will be used with
 * all programs and their windows that are not specified in a
 * separate configuration below this one. Default keycodes are
 * the now famous Wacom Intuos3 ones. I've also set totally
 * non-destructive touch strip keys (only used if handle_touch
 * is altered to a "1" - without quotes) which are: Arrow-keys
 * Up/Down on the left strip and Arrow-keys Right/Left on the
 * right strip. Change direction of the movement by switching
 * the _up and _down values.
 * 
 * If you want a key to do pen mode changes, use the value 999
 * under the corresponding keylabel. To be able to switch mode
 * anywhere, each configuration must contain one 999 definition.
 * And, of course, the main HANDLE_PEN definition above must
 * first be set to "1" - without quotes.
 * 
 * Please don't alter or remove the commas (,) after the keycodes.
 * They _must_ be there just as written.
 */
/*	Name	handle_touch */
{"default",	0,
/*		l_touch_up	l_touch_down	r_touch_up	r_touch_down */
		98,		104,		102,		100,
/*		key_9		key_9_plus	key_10		key_10_plus */
		50,		0,		64,		0,
/*		key_11		key_11_plus	key_12		key_12_plus */
		37,		0,		65,		0,
/*		key_13		key_13_plus	key_14		key_14_plus */
		50,		0,		64,		0,
/*		key_15		key_15_plus	key_16		key_16_plus */
		37,		0,		65,		0	},

/*
 * Gimp has the touch strips turned on by default. The keycodes are:
 * 20 = "+" = Gimp Zoom In. Left/Right touch strip up motion
 * 61 = "-" = Gimp Zoom Out. Left/Right touch strip down motion
 * Change direction of the movement by switching _up and _down values.
 */
/*	Name	handle_touch */
{"Gimp",	1,
/*		l_touch_up	l_touch_down	r_touch_up	r_touch_down */
		20,		61,		20,		61,
/*		key_9		key_9_plus	key_10		key_10_plus */
		50,		0,		64,		0,
/*		key_11		key_11_plus	key_12		key_12_plus */
		37,		0,		65,		0,
/*		key_13		key_13_plus	key_14		key_14_plus */
		50,		0,		64,		0,
/*		key_15		key_15_plus	key_16		key_16_plus */
		37,		0,		65,		0	},

/* This is my private definition for the 3D program blender...
 */
/*	Name	handle_touch */
{"Blender",	1,
/*		l_touch_up	l_touch_down	r_touch_up	r_touch_down */
		102,		100,		98,		104,
/*		key_9		key_9_plus	key_10		key_10_plus */
		37,		0,		9,		0,
/*		key_11		key_11_plus	key_12		key_12_plus */
		50,		0,		23,		0,
/*		key_13		key_13_plus	key_14		key_14_plus */
		50,		30,		30,		0,
/*		key_15		key_15_plus	key_16		key_16_plus */
		999,		0,		65,		0	},

/* I feel that an xterm is too important a window to have _any_
 * interference from the pad. But observe that I want to be able
 * to switch pen mode even with such a window in focus.
 */
/*	Name	handle_touch */
{"XTerm",	0,
/*		l_touch_up	l_touch_down	r_touch_up	r_touch_down */
		0,		0,		0,		0,
/*		key_9		key_9_plus	key_10		key_10_plus */
		0,		0,		0,		0,
/*		key_11		key_11_plus	key_12		key_12_plus */
		0,		0,		0,		0,
/*		key_13		key_13_plus	key_14		key_14_plus */
		0,		0,		0,		0,
/*		key_15		key_15_plus	key_16		key_16_plus */
		999,		0,		0,		0	},

/* And that's how it's done. Just copy a section and tweak the
 * settings for a new program. Your machine speed and memory
 * sets the limit ;-)
 */
/* ++++++++++ End user config area ++++++++++ */

};
#define NUM_LIST (sizeof prog_list / sizeof prog_list[0])

#include <X11/Xlib.h>
#include <X11/Xutil.h>
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

	int event_base, error_base;
	int major_version, minor_version;
	if (!XTestQueryExtension (display, &event_base, &error_base,
	&major_version, &minor_version)) {
		fprintf (stderr, "XTest extension not supported on server\n");
		XCloseDisplay(display);
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

		XClassHint *class_hint;
		class_hint = XAllocClassHint();

		Window focus_window = None;
		int focus_state;

		Window root, parent;
		Window *children;
		unsigned int num_children;

		struct program *p;
		int in_list = 0;

		XGetInputFocus(display, &focus_window, &focus_state);
		XQueryTree(display, focus_window, &root, &parent, &children, &num_children);
		XGetClassHint(display, focus_window, class_hint);
		if (!class_hint->res_class){
			XFree(class_hint->res_class);
			XFree(class_hint->res_name);
			XGetClassHint(display, parent, class_hint);
		}

		for (p = prog_list; p < prog_list + NUM_LIST; p++)
			if (strcmp (class_hint->res_class, p->class_name) == 0){
				in_list = 1;
				break;
			}
		
		XFree(class_hint->res_class);
		XFree(class_hint->res_name);
		if (children) XFree((char *)children);

		if (!in_list){
			p = prog_list;
		}

	if (Event.type == motion_type) {

		if (p->handle_touch){
			int rotation;
			int throttle;
	
	    	XDeviceMotionEvent *motion = (XDeviceMotionEvent *) &Event;

			rotation = motion->axis_data[3];
			throttle = motion->axis_data[4];

			if (rotation > 1){
				if ((rotation < old_rotation) && (old_rotation <= elder_rotation)){
					XTestFakeKeyEvent(display, p->l_touch_up, True, CurrentTime);
					XTestFakeKeyEvent(display, p->l_touch_up, False, CurrentTime);
				}
				else if ((rotation > old_rotation) && (old_rotation >= elder_rotation)){
					XTestFakeKeyEvent(display, p->l_touch_down, True, CurrentTime);
					XTestFakeKeyEvent(display, p->l_touch_down, False, CurrentTime);
				}
			elder_rotation = old_rotation;
			old_rotation = rotation;
			}

			if (throttle > 1){
				if ((throttle < old_throttle) && (old_throttle <= elder_throttle)){
					XTestFakeKeyEvent(display, p->r_touch_up, True, CurrentTime);
					XTestFakeKeyEvent(display, p->r_touch_up, False, CurrentTime);
				}
				else if ((throttle > old_throttle) && (old_throttle >= elder_throttle)){
					XTestFakeKeyEvent(display, p->r_touch_down, True, CurrentTime);
					XTestFakeKeyEvent(display, p->r_touch_down, False, CurrentTime);
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
		if (p->key_9 == TOGGLE_PEN)
			if (HANDLE_PEN)
				toggle_pen_mode(display, PEN_NAME);
			else
				break;
		else
		if (p->key_9)
XTestFakeKeyEvent(display, p->key_9, True, CurrentTime );
		if (p->key_9_plus)
XTestFakeKeyEvent(display, p->key_9_plus, True, CurrentTime );
		else
			break;
			break;
			case 10:
		if (p->key_10 == TOGGLE_PEN)
			if (HANDLE_PEN)
				toggle_pen_mode(display, PEN_NAME);
			else
				break;
		else
		if (p->key_10)
XTestFakeKeyEvent(display, p->key_10, True, CurrentTime );
		if (p->key_10_plus)
XTestFakeKeyEvent(display, p->key_10_plus, True, CurrentTime );
		else
			break;
			break;
			case 11:
		if (p->key_11 == TOGGLE_PEN)
			if (HANDLE_PEN)
				toggle_pen_mode(display, PEN_NAME);
			else
				break;
		else
		if (p->key_11)
XTestFakeKeyEvent(display, p->key_11, True, CurrentTime );
		if (p->key_11_plus)
XTestFakeKeyEvent(display, p->key_11_plus, True, CurrentTime );
		else
			break;
			break;
			case 12:
		if (p->key_12 == TOGGLE_PEN)
			if (HANDLE_PEN)
				toggle_pen_mode(display, PEN_NAME);
			else
				break;
		else
		if (p->key_12)
XTestFakeKeyEvent(display, p->key_12, True, CurrentTime );
		if (p->key_12_plus)
XTestFakeKeyEvent(display, p->key_12_plus, True, CurrentTime );
		else
			break;
			break;
			case 13:
		if (p->key_13 == TOGGLE_PEN)
			if (HANDLE_PEN)
				toggle_pen_mode(display, PEN_NAME);
			else
				break;
		else
		if (p->key_13)
XTestFakeKeyEvent(display, p->key_13, True, CurrentTime );
		if (p->key_13_plus)
XTestFakeKeyEvent(display, p->key_13_plus, True, CurrentTime );
		else
			break;
			break;
			case 14:
		if (p->key_14 == TOGGLE_PEN)
			if (HANDLE_PEN)
				toggle_pen_mode(display, PEN_NAME);
			else
				break;
		else
		if (p->key_14)
XTestFakeKeyEvent(display, p->key_14, True, CurrentTime );
		if (p->key_14_plus)
XTestFakeKeyEvent(display, p->key_14_plus, True, CurrentTime );
		else
			break;
			break;
			case 15:
		if (p->key_15 == TOGGLE_PEN)
			if (HANDLE_PEN)
				toggle_pen_mode(display, PEN_NAME);
			else
				break;
		else
		if (p->key_15)
XTestFakeKeyEvent(display, p->key_15, True, CurrentTime );
		if (p->key_15_plus)
XTestFakeKeyEvent(display, p->key_15_plus, True, CurrentTime );
		else
			break;
			break;
			case 16:
		if (p->key_16 == TOGGLE_PEN)
			if (HANDLE_PEN)
				toggle_pen_mode(display, PEN_NAME);
			else
				break;
		else
		if (p->key_16)
XTestFakeKeyEvent(display, p->key_16, True, CurrentTime );
		if (p->key_16_plus)
XTestFakeKeyEvent(display, p->key_16_plus, True, CurrentTime );
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
		if (p->key_9 == TOGGLE_PEN)
			break;
		else
		if (p->key_9_plus)
XTestFakeKeyEvent(display, p->key_9_plus, False, CurrentTime );
		if (p->key_9)
XTestFakeKeyEvent(display, p->key_9, False, CurrentTime );
		else
			break;
			break;
			case 10:
		if (p->key_10 == TOGGLE_PEN)		
			break;
		else
		if (p->key_10_plus)
XTestFakeKeyEvent(display, p->key_10_plus, False, CurrentTime );
		if (p->key_10)
XTestFakeKeyEvent(display, p->key_10, False, CurrentTime );
		else
			break;
			break;
			case 11:
		if (p->key_11 == TOGGLE_PEN)
			break;
		else
		if (p->key_11_plus)
XTestFakeKeyEvent(display, p->key_11_plus, False, CurrentTime );
		if (p->key_11)
XTestFakeKeyEvent(display, p->key_11, False, CurrentTime );
		else
			break;
			break;
			case 12:
		if (p->key_12 == TOGGLE_PEN)
			break;
		else
		if (p->key_12_plus)
XTestFakeKeyEvent(display, p->key_12_plus, False, CurrentTime );
		if (p->key_12)
XTestFakeKeyEvent(display, p->key_12, False, CurrentTime );
		else
			break;
			break;
			case 13:
		if (p->key_13 == TOGGLE_PEN)
			break;
		else
		if (p->key_13_plus)		
XTestFakeKeyEvent(display, p->key_13_plus, False, CurrentTime );
		if (p->key_13)
XTestFakeKeyEvent(display, p->key_13, False, CurrentTime );
		else
			break;
			break;
			case 14:
		if (p->key_14 == TOGGLE_PEN)
			break;
		else
		if (p->key_14_plus)
XTestFakeKeyEvent(display, p->key_14_plus, False, CurrentTime );
		if (p->key_14)
XTestFakeKeyEvent(display, p->key_14, False, CurrentTime );
		else
			break;
			break;
			case 15:
		if (p->key_15 == TOGGLE_PEN)
			break;
		else
		if (p->key_15_plus)
XTestFakeKeyEvent(display, p->key_15_plus, False, CurrentTime );
		if (p->key_15)
XTestFakeKeyEvent(display, p->key_15, False, CurrentTime );
		else
			break;
			break;
			case 16:
		if (p->key_16 == TOGGLE_PEN)
			break;
		else
		if (p->key_16_plus)
XTestFakeKeyEvent(display, p->key_16_plus, False, CurrentTime );
		if (p->key_16)
XTestFakeKeyEvent(display, p->key_16, False, CurrentTime );
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

