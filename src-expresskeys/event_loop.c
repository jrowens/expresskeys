/*
 event_loop.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
 
 Copyright (C) 2005-2006 - Mats Johannesson, Denis DerSarkisian
 
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
 Function acts like a traffic cop for keycodes. Sends real keycodes to
 XTestFakeKeyEvent. Interprets fake keycodes (currently keycodes standing
 in for mouse button presses) and sends them to the appropriate function.
 Only existing mouse buttons, defined through the driver of the active core
 pointer, can be simulated. If you run this function in a graphical debugger,
 the mouse _will_ be effected.
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int fake_event(Display *display, unsigned int keycode, Bool is_press,
               unsigned long delay)
{
	if ((MOUSE_BUTTON_MIN < keycode) && (keycode < MOUSE_BUTTON_MAX)) {
		/* mouse button code */
		if (be_verbose) {
			fprintf(stderr, "MOUSE: ");
		}

		int i;
		unsigned char button_map[MOUSE_BUTTON_MAX - MOUSE_BUTTON_MIN];
		for (i = 0; i < (MOUSE_BUTTON_MAX - MOUSE_BUTTON_MIN); i++) {
			button_map[i] = '\0';
		}

		XGetPointerMapping(display, button_map, sizeof(button_map));
		unsigned int button = keycode - MOUSE_BUTTON_MIN;
		unsigned int real_button = button_map[button - 1];

		if (real_button) {
			if (be_verbose) {
				fprintf(stderr, "Real button %d found: ", button);
			}
			return XTestFakeButtonEvent(display, button, is_press, delay);
		} else {
			if (be_verbose) {
				fprintf(stderr, "No real button %d found! ", button);
			}
			return 0;
		}
	} else {
		/* keycode */
		if (be_verbose) {
			fprintf(stderr, "KEYBD: ");
		}
		return XTestFakeKeyEvent(display, keycode, is_press, delay);
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Function waits perpetually for the X server to deliver information
 about events from the input device. Receipt of an event that we've
 registered for (button press/release and motion) triggers a good deal
 of activity in a setup phase, after which we send the fake key press.
 A button press or release will always cause first a motion and then 
 a button event. That's why you'll see the focus window name being
 printed twice if running with the verbose flag set.
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int use_events(Display *display)
{

	XClassHint *class_hint;

	Window focus_window;
	int focus_state;

	Window root, parent;
	Window *children;
	unsigned int num_children;

	struct program *p;
	int in_list;

	int elder_rotation = 4097; /* Set the Touch Strip state histories to */
	int old_rotation = 4097;   /* their max values. This is to ensure */
	int elder_throttle = 4097; /* that no action is taken on the very */
	int old_throttle = 4097;   /* first touch before a real history is */
	int rotation;              /* recorded. Bottom is 4097. Top is 2 */
	int throttle;

	int i = 0;
	int *button_index = 0;

	XDeviceMotionEvent *motion;
	XDeviceButtonEvent *button;

	XEvent Event;

	while(1) {

		XNextEvent(display, &Event);

		class_hint = XAllocClassHint();	/* Memory allocation. Free it later! */

		focus_window = None;
		in_list = 0;

/* Locate which window that currently has the focus and get its list of
   related windows. Then pull its ClassHint into our allocated structure */

		XGetInputFocus(display, &focus_window, &focus_state);
		XQueryTree(display, focus_window, &root, &parent, &children, &num_children);
		XGetClassHint(display, focus_window, class_hint);

/* If the class hint (aka WM_CLASS) contains no class string we free the
   allocated memory for each structure member and get the ClassHint of the
   window parent, if it has one. Observe that we must skip the root window */

		if ((!class_hint->res_class) && (parent) && (focus_window != root)) {
			XFree(class_hint->res_class);
			XFree(class_hint->res_name);
			XGetClassHint(display, parent, class_hint);
		}

/* If the root window had the focus, or if the active window or its parent
   had no class string at all, we use the top ("default") program definition
   from the read in configuration file when evaluating the event. Otherwise
   we start scanning for a match between the class strings in our list and
   the found window class. Set a flag if a match is encountered */

		if ((focus_window == root) || (class_hint->res_class == NULL)) {
			p = external_list;
		} else {
			for (p = external_list; p < external_list + num_list; p++)
				if (strcmp (class_hint->res_class, p->class_name) == 0) {
					in_list = 1;
					break;
				}
			}

/* Any program not found in our configuration gets the "default" treatment */

		if (!in_list) {
			p = external_list;
		}

/* The allocated memory for the ClassHint structure, and each of its
   members, must be freed here. Also, the call to XQueryTree to get a
   list of related windows might have allocated memory for child entries.
   It must be released as well */
		
		XFree(class_hint->res_class);
		XFree(class_hint->res_name);
		XFree(class_hint);
		if (children) XFree((char *)children);

		if (be_verbose) {
			fprintf(stderr, "PGR FOCUS = %s\n", p->class_name);
		}

/* Finally start to look at the actual event. Touch Strips come first */

		if (Event.type == motion_type) {

			if (p->handle_touch) {

    			motion = (XDeviceMotionEvent *) &Event;

				rotation = motion->axis_data[3];
				throttle = motion->axis_data[4];

/* As can be analyzed with Frederic Lepied's excellent xinput-1.2 program
   the touch strip data comes in on axis 3 and 4 (left and right strips).
   The pad device never uses x-axis [0], y-axis [1] or wheel [5]. The value
   is always 0 there. The pressure [2], rotation [3] and throttle [4] all
   rest with a value of 1. Touch strips send data about the finger position
   in 13 steps. Furthest down is 4097 while the top is 2. Template:
   4097, 2049, 1025, 513, 257, 129, 65, 33, 17, 9, 5, 3, 2. We don't care
   about those numbers per se (for now at least ;-), but just stick them
   in history buffers. The present value is then compared to an old and
   an even older one to determine direction of the finger movement. Observe
   that when we finally send a fake keypress, it has two parts - true and
   false. It corresponds with a key press and a key release. Order is key ;-) */

/* Left Touch Strip */

				if (rotation > 1) {
					if ((rotation < old_rotation) && (old_rotation <= elder_rotation)) {
						if (p->l_touch_up) {
							fake_event(display, p->l_touch_up, True, CurrentTime);
							if (be_verbose) {
								fprintf(stderr, "LTCHUP = %d dn\n", p->l_touch_up);
							}
							if (p->l_touch_up_plus) {
								fake_event(display, p->l_touch_up_plus, True, CurrentTime);
								if (be_verbose) {
									fprintf(stderr, "LTCHUP+ = %d dn\n", p->l_touch_up_plus);
								}
								fake_event(display, p->l_touch_up_plus, False, CurrentTime);
								if (be_verbose) {
									fprintf(stderr, "LTCHUP+ = %d up\n", p->l_touch_up_plus);
								}
							}
							fake_event(display, p->l_touch_up, False, CurrentTime);
							if (be_verbose) {
								fprintf(stderr, "LTCHUP = %d up\n", p->l_touch_up);
							}
						}
					} else if ((rotation > old_rotation) && (old_rotation >= elder_rotation)) {
						if (p->l_touch_down) {
							fake_event(display, p->l_touch_down, True, CurrentTime);
							if (be_verbose) {
								fprintf(stderr, "LTCHDN = %d dn\n", p->l_touch_down);
							}
							if (p->l_touch_down_plus) {
								fake_event(display, p->l_touch_down_plus, True, CurrentTime);
								if (be_verbose) {
									fprintf(stderr, "LTCHDN+ = %d dn\n", p->l_touch_down_plus);
								}
								fake_event(display, p->l_touch_down_plus, False, CurrentTime);
								if (be_verbose) {
									fprintf(stderr, "LTCHDN+ = %d up\n", p->l_touch_down_plus);
								}
							}
							fake_event(display, p->l_touch_down, False, CurrentTime);
							if (be_verbose) {
								fprintf(stderr, "LTCHDN = %d up\n", p->l_touch_down);
							}
						}
					}
				elder_rotation = old_rotation;
				old_rotation = rotation;
				}

/* Right Touch Strip */

				if (throttle > 1) {
					if ((throttle < old_throttle) && (old_throttle <= elder_throttle)) {
						if (p->r_touch_up) {
							fake_event(display, p->r_touch_up, True, CurrentTime);
							if (be_verbose) {
								fprintf(stderr, "RTCHUP = %d dn\n", p->r_touch_up);
							}
							if (p->r_touch_up_plus) {
								fake_event(display, p->r_touch_up_plus, True, CurrentTime);
								if (be_verbose) {
									fprintf(stderr, "RTCHUP+ = %d dn\n", p->r_touch_up_plus);
								}
								fake_event(display, p->r_touch_up_plus, False, CurrentTime);
								if (be_verbose) {
									fprintf(stderr, "RTCHUP+ = %d up\n", p->r_touch_up_plus);
								}
							}
							fake_event(display, p->r_touch_up, False, CurrentTime);
							if (be_verbose) {
								fprintf(stderr, "RTCHUP = %d up\n", p->r_touch_up);
							}
						}
					} else if ((throttle > old_throttle) && (old_throttle >= elder_throttle)) {
						if (p->r_touch_down) {
							fake_event(display, p->r_touch_down, True, CurrentTime);
							if (be_verbose) {
								fprintf(stderr, "RTCHDN = %d dn\n", p->r_touch_down);
							}
							if (p->r_touch_down_plus) {
								fake_event(display, p->r_touch_down_plus, True, CurrentTime);
								if (be_verbose) {
									fprintf(stderr, "RTCHDN+ = %d dn\n", p->r_touch_down_plus);
								}
								fake_event(display, p->r_touch_down_plus, False, CurrentTime);
								if (be_verbose) {
									fprintf(stderr, "RTCHDN+ = %d up\n", p->r_touch_down_plus);
								}
							}
							fake_event(display, p->r_touch_down, False, CurrentTime);
							if (be_verbose) {
								fprintf(stderr, "RTCHDN = %d up\n", p->r_touch_down);
							}
						}
					}
				elder_throttle = old_throttle;
				old_throttle = throttle;
				}
			}
		}

/* Now see if the event concerned the pad buttons. Not much to talk about.
   We follow the configuration definitions, and handle a pen if requested
   to do so. Wacom have numbered the buttons 9, 10, 11, 12 on the left side
   and 13, 14, 15, 16 on the right. Template:

Left ExpressKey Pad
------------ 
|  |   |   |		Wacom Intuos3 defaults are:
|  | 9 | T |
|11|---| O |		Key 9  = (left) Shift	= keycode 50
|  |10 | U |		Key 10 = (left) Alt	= keycode 64
|------| C |		Key 11 = (left) Control	= keycode 37
|  12  | H |		Key 12 = Space		= keycode 65
------------

Right ExpressKey Pad
------------ 
|   |   |  |		Wacom Intuos3 defaults are:
| T |13 |  |
| O |---|15|		Key 13 = (left) Shift	= keycode 50
| U |14 |  |		Key 14 = (left) Alt	= keycode 64
| C |------|		Key 15 = (left) Control	= keycode 37
| H |  16  |		Key 16 = Space		= keycode 65
------------
*/

/* Pad Button Press */

		if (Event.type == button_press_type) {

			button = (XDeviceButtonEvent *) &Event;

			button_index = &p->key_9;

			for (i = 9; i < button->button; i++) {
				button_index++;
				button_index++;
			}

			if (*button_index == TOGGLE_PEN) {
				if (handle_pen) {
					if (be_verbose) {
						fprintf(stderr, "BTN %d = %d dn\n", button->button, *button_index);
					}
					toggle_pen_mode(display, pen_name);
				}
			} else if ((*button_index >= PEN_CURVE_DOWNWARD) &&
						(*button_index <= PEN_CURVE_UPWARD)) {
				if (be_verbose) {
					fprintf(stderr, "BTN %d = %d dn\n", button->button, *button_index);
				}
				call_xsetwacom(*button_index);
			} else {
				if (*button_index) {
					fake_event(display, *button_index, True, CurrentTime );
					if (be_verbose) {
						fprintf(stderr, "BTN %d = %d dn\n", button->button, *button_index);
					}
				}
				button_index++;
				if (*button_index) {
					fake_event(display, *button_index, True, CurrentTime );
					if (be_verbose) {
						fprintf(stderr, "BTN+ %d = %d dn\n", button->button, *button_index);
					}
				}
			}
		}

/* Pad Button Release */

		if (Event.type == button_release_type) {

			button = (XDeviceButtonEvent *) &Event;

			button_index = &p->key_9;

			for (i = 9; i < button->button; i++) {
				button_index++;
				button_index++;
			}

			if (*button_index == TOGGLE_PEN) {
				if (be_verbose) {
					fprintf(stderr, "BTN %d = %d up\n", button->button, *button_index);
				}
			} else if ((*button_index >= PEN_CURVE_DOWNWARD) &&
						(*button_index <= PEN_CURVE_UPWARD)) {
				if (be_verbose) {
					fprintf(stderr, "BTN %d = %d up\n", button->button, *button_index);
				}
			} else {
				button_index++;
				if (*button_index) {
					fake_event(display, *button_index, False, CurrentTime );
					if (be_verbose) {
						fprintf(stderr, "BTN+ %d = %d up\n", button->button, *button_index);
					}
				}
				button_index--;
				if (*button_index) {
					fake_event(display, *button_index, False, CurrentTime );
					if (be_verbose) {
						fprintf(stderr, "BTN %d = %d up\n", button->button, *button_index);
					}
				}
			}
		}
	}
	exit(EXIT_OK);
}

/* End code */

