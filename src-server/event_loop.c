/*
 * event_loop.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
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

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Function waits perpetually for the X server to deliver information */
/* about events from the input device. Receipt of an event that we've */
/* registered for (button press/release and motion) triggers a good deal */
/* of activity in a setup phase, after which we send the fake key press */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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

	XDeviceMotionEvent *motion;
	XDeviceButtonEvent *button;

	XEvent Event;

	while(1) {

		XNextEvent(display, &Event);

		class_hint = XAllocClassHint();	/* Memory allocation. Free it later! */

		focus_window = None;
		in_list = 0;

/* Locate which window that currently has the focus and get its list of */
/* related windows. Then pull its ClassHint into our allocated structure */

		XGetInputFocus(display, &focus_window, &focus_state);
		XQueryTree(display, focus_window, &root, &parent, &children, &num_children);
		XGetClassHint(display, focus_window, class_hint);

/* If the class hint (aka WM_CLASS) contains no class string we free the */
/* allocated memory for each structure member and get the ClassHint of the */
/* window parent, if it has one. Observe that we must skip the root window */

		if ((!class_hint->res_class) && (parent) && (focus_window != root)) {
			XFree(class_hint->res_class);
			XFree(class_hint->res_name);
			XGetClassHint(display, parent, class_hint);
		}

/* If the root window had the focus, or if the active window or its parent */
/* had no class string at all, we use the top ("default") program definition */
/* from the read in configuration file when evaluating the event. Otherwise */
/* we start scanning for a match between the class strings in our list and */
/* the found window class. Set a flag if a match is encountered */

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

/* The allocated memory for the ClassHint structure, and each of its */
/* members, must be freed here. Also, the call to XQueryTree to get a */
/* list of related windows might have allocated memory for child entries. */
/* It must be released as well */
		
		XFree(class_hint->res_class);
		XFree(class_hint->res_name);
		XFree(class_hint);
		if (children) XFree((char *)children);

/* Finally start to look at the actual event. Touch Strips come first */

		if (Event.type == motion_type) {

			if (p->handle_touch) {

    			motion = (XDeviceMotionEvent *) &Event;

				rotation = motion->axis_data[3];
				throttle = motion->axis_data[4];

/* As can be analyzed with Frederic Lepied's excellent xinput-1.2 program */
/* the touch strip data comes in on axis 3 and 4 (left and right strips). */
/* The pad device never uses x-axis [0], y-axis [1] or wheel [5]. The value */
/* is always 0 there. The pressure [2], rotation [3] and throttle [4] all */
/* rest with a value of 1. Touch strips send data about the finger position */
/* in 13 steps. Furthest down is 4097 while the top is 2. Template: */
/* 4097, 2049, 1025, 513, 257, 129, 65, 33, 17, 9, 5, 3, 2. We don't care */
/* about those numbers per se (for now at least ;-), but just stick them */
/* in history buffers. The present value is then compared to an old and */
/* an even older one to determine direction of the finger movement. Observe */
/* that when we finally send a fake keypress, it has two parts - true and */
/* false. It corresponds with a key press and a key release. Order is key ;-) */

/* Left Touch Strip */

				if (rotation > 1) {
					if ((rotation < old_rotation) && (old_rotation <= elder_rotation)) {
						if (p->l_touch_up) {
							XTestFakeKeyEvent(display, p->l_touch_up, True, CurrentTime);
							if (p->l_touch_up_plus) {
								XTestFakeKeyEvent(display, p->l_touch_up_plus, True, CurrentTime);
								XTestFakeKeyEvent(display, p->l_touch_up_plus, False, CurrentTime);
							}
							XTestFakeKeyEvent(display, p->l_touch_up, False, CurrentTime);
						}
					} else if ((rotation > old_rotation) && (old_rotation >= elder_rotation)) {
						if (p->l_touch_down) {
							XTestFakeKeyEvent(display, p->l_touch_down, True, CurrentTime);
							if (p->l_touch_down_plus) {
								XTestFakeKeyEvent(display, p->l_touch_down_plus, True, CurrentTime);
								XTestFakeKeyEvent(display, p->l_touch_down_plus, False, CurrentTime);
							}
							XTestFakeKeyEvent(display, p->l_touch_down, False, CurrentTime);
						}
					}
				elder_rotation = old_rotation;
				old_rotation = rotation;
				}

/* Right Touch Strip */

				if (throttle > 1) {
					if ((throttle < old_throttle) && (old_throttle <= elder_throttle)) {
						if (p->r_touch_up) {
							XTestFakeKeyEvent(display, p->r_touch_up, True, CurrentTime);
							if (p->r_touch_up_plus) {
								XTestFakeKeyEvent(display, p->r_touch_up_plus, True, CurrentTime);
								XTestFakeKeyEvent(display, p->r_touch_up_plus, False, CurrentTime);
							}
							XTestFakeKeyEvent(display, p->r_touch_up, False, CurrentTime);
						}
					} else if ((throttle > old_throttle) && (old_throttle >= elder_throttle)) {
						if (p->r_touch_down) {
							XTestFakeKeyEvent(display, p->r_touch_down, True, CurrentTime);
							if (p->r_touch_down_plus) {
								XTestFakeKeyEvent(display, p->r_touch_down_plus, True, CurrentTime);
								XTestFakeKeyEvent(display, p->r_touch_down_plus, False, CurrentTime);
							}
							XTestFakeKeyEvent(display, p->r_touch_down, False, CurrentTime);
						}
					}
				elder_throttle = old_throttle;
				old_throttle = throttle;
				}
			}
		}

/* Now see if the event concerned the pad buttons. Not much to talk about. */
/* We follow the configuration definitions, and handle a pen if requested */
/* to do so. The switch routine is ugly and easy to get wrong, but it */
/* works! Ah yes, the xinput-1.2 program reveals Wacom to have numbered the */
/* buttons 9, 10, 11, 12 on the left side and 13, 14, 15, 16 on the right. */
/* Template: */
/*
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

			switch (button->button) {

				case 9:
				if (p->key_9 == TOGGLE_PEN)
					if (handle_pen)
						toggle_pen_mode(display, pen_name);
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
					if (handle_pen)
						toggle_pen_mode(display, pen_name);
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
					if (handle_pen)
						toggle_pen_mode(display, pen_name);
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
					if (handle_pen)
						toggle_pen_mode(display, pen_name);
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
					if (handle_pen)
						toggle_pen_mode(display, pen_name);
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
					if (handle_pen)
						toggle_pen_mode(display, pen_name);
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
					if (handle_pen)
						toggle_pen_mode(display, pen_name);
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
					if (handle_pen)
						toggle_pen_mode(display, pen_name);
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

/* Pad Button Release */

/* There is just an exit point below */
/* this switch routine by the way */

		if (Event.type == button_release_type) {

			button = (XDeviceButtonEvent *) &Event;

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
	exit(EXIT_OK);
}

/* End code */

