/*
 * event_loop -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
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
#include "UserConfig-Pad"

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

		if ((!class_hint->res_class) && (parent) && (focus_window != root)){
			XFree(class_hint->res_class);
			XFree(class_hint->res_name);
			XGetClassHint(display, parent, class_hint);
		}

		if ((focus_window == root) || (class_hint->res_class == NULL)){
			p = prog_list;
		} else {
			for (p = prog_list; p < prog_list + NUM_LIST; p++)
				if (strcmp (class_hint->res_class, p->class_name) == 0){
					in_list = 1;
					break;
				}
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
						if (p->l_touch_up){
							XTestFakeKeyEvent(display, p->l_touch_up, True, CurrentTime);
							if (p->l_touch_up_plus){
								XTestFakeKeyEvent(display, p->l_touch_up_plus, True, CurrentTime);
								XTestFakeKeyEvent(display, p->l_touch_up_plus, False, CurrentTime);
							}
							XTestFakeKeyEvent(display, p->l_touch_up, False, CurrentTime);
						}
					} else if ((rotation > old_rotation) && (old_rotation >= elder_rotation)){
						if (p->l_touch_down){
							XTestFakeKeyEvent(display, p->l_touch_down, True, CurrentTime);
							if (p->l_touch_down_plus){
								XTestFakeKeyEvent(display, p->l_touch_down_plus, True, CurrentTime);
								XTestFakeKeyEvent(display, p->l_touch_down_plus, False, CurrentTime);
							}
							XTestFakeKeyEvent(display, p->l_touch_down, False, CurrentTime);
						}
					}
				elder_rotation = old_rotation;
				old_rotation = rotation;
				}

				if (throttle > 1){
					if ((throttle < old_throttle) && (old_throttle <= elder_throttle)){
						if (p->r_touch_up){
							XTestFakeKeyEvent(display, p->r_touch_up, True, CurrentTime);
							if (p->r_touch_up_plus){
								XTestFakeKeyEvent(display, p->r_touch_up_plus, True, CurrentTime);
								XTestFakeKeyEvent(display, p->r_touch_up_plus, False, CurrentTime);
							}
							XTestFakeKeyEvent(display, p->r_touch_up, False, CurrentTime);
						}
					} else if ((throttle > old_throttle) && (old_throttle >= elder_throttle)){
						if (p->r_touch_down){
							XTestFakeKeyEvent(display, p->r_touch_down, True, CurrentTime);
							if (p->r_touch_down_plus){
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
	return EXIT_OK;
}

/* End code */

