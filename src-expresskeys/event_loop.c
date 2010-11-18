/*
 expresskeys - Support ExpressKeys, Touch Strips, Scroll Wheel on Wacom tablets.

 Copyright (C) 2005-2007 - Mats Johannesson, Denis DerSarkisian

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

#include <stdio.h> /* NULL, fprintf, sprintf, snprintf */
#include <string.h> /* strcmp */
#include <stdlib.h> /* system */
#include <time.h> /* nanosleep */
#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XTest.h>

#include "tablet.h"

/* 'Internal' Globals: */

/* At certain points it is necessary to know which device ID that belongs
 to the event we process (toggle_stylus and prepare_action). The pad ID is
 not used anywhere, but we save it anyway (the future could find it useful): */
static unsigned long int* id_pad;
static unsigned long int* id_sty1;
static unsigned long int* id_sty2;

/* Crucial flags to keep track of the event state
 (except for the is_pad. But the future...): */
static int is_ux;
static int is_ux2;
static int is_bbo;
static int is_bee;
static int is_i3;
static int is_i3s;
static int is_g4;
static int is_g4b;
static int is_nop;
static int is_pad;
static int is_sty1;
static int is_sty2;

/* Used while we try to match the focus window with known program names: */
static int record_num;

/* What DelayEachKeycode the user has specified for the present record: */
static int key_sec;
static int key_nsec;

/* Used in change_curve (set to the default "0 0 100 100" through value 4):*/
static int stylus_presscurve = 4;
/* Used in change_rotate (set to the default "NONE" through value 0): */
static int tablet_rotation = 0;

/* Used as 'history-buffers' by do_stylus to set an automatic PressCurve: */
static int name_change;
static char curvebuffer[CURVEMAX];
static char namebuffer[MAXBUFFER];
/* Used by do_stylus/change_curve/change_rotate to communicate with xsetwacom:*/
static char execbuffer[MAXBUFFER];

/* Used by find_focus and send_keys (the latter as a 'safety' measure): */
static Window focus_window = None;
static int focus_state;

/* The actual 'safety' flags: */
static int abort_repeat;
static int key_repeat;

/* Flags for special actions (stylus Mode, stylus PressCurve, tablet Rotate): */
static const int mode_action = 0;
static const int curve_action = 1;
static const int rotate_action = 2;

/* Externals: */

extern Display* display;
extern int be_verbose;

extern int button_press_type;
extern int button_release_type;
extern int motion_type;
extern int proximity_in_type;
extern int proximity_out_type;

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 We use xsetwacom to rotate the tablet. Internally, value to string is 0 = NONE,
 1 = CW (Clock-Wise 90 degrees), 2 = HALF (Half-turn 180 degrees), 3 = CCW
 (Counter-Clock-Wise 90 degrees). Looking down on the tablet and tilting the
 head '90' degrees to the right would simulate a CW operation. Both
 TABLET_ROTATE_CW_AGAIN and TABLET_ROTATE_CCW_AGAIN flip over in a carousel
 fashion at the top and bottom values. Observe that all attached tablets share
 the same state-variable. There was little point in differentiating them.
 Also, with a max of 15 tablets it would become messy:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void change_rotate(int fake_key, struct common_data* cdp)
{
	char new_rotate[ROTATEMAX];

	if (fake_key == TABLET_ROTATE_CW_AGAIN) {
		if (tablet_rotation < 3) {
			tablet_rotation++;
		} else {
			tablet_rotation = 0;
		}
	} else if (fake_key == TABLET_ROTATE_CCW_AGAIN) {
		if (tablet_rotation > 0) {
			tablet_rotation--;
		} else {
			tablet_rotation = 3;
		}
	} else if (fake_key == TABLET_ROTATE_NONE) {
		tablet_rotation = 0;
	} else if (fake_key == TABLET_ROTATE_CW) {
		tablet_rotation = 1;
	} else if (fake_key == TABLET_ROTATE_HALF) {
		tablet_rotation = 2;
	} else if (fake_key == TABLET_ROTATE_CCW) {
		tablet_rotation = 3;
	}

	switch (tablet_rotation) {

		case 0:
		sprintf(new_rotate, "NONE");
		break;

		case 1:
		sprintf(new_rotate, "CW");
		break;

		case 2:
		sprintf(new_rotate, "HALF");
		break;

		case 3:
		sprintf(new_rotate, "CCW");
		break;

		default:
		sprintf(new_rotate, "NONE");
		break;
	}

	snprintf(execbuffer, MAXBUFFER, "xsetwacom set %s Rotate %s",
						cdp->sty1name, new_rotate);

/* FIXME This does not catch return errors from the system call: */
	if ((system(execbuffer)) == NON_VALID) {
		fprintf(stderr, "Failed to fork a shell for \
xsetwacom to set PressCurve!\n");
	} else {
		if (be_verbose) {
			fprintf(stderr, "(Now set to \"%s\") ", new_rotate);
		}
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 We use xsetwacom to alter the pen sensitivity to pressure, just like setting
 the Feel -> Sensitivity in wacomcpl (the tcl utility program) between
 Soft (lower) and Firm (higher) values (1 to 7, 4 is default). Both
 STYLUS_CURVE_DOWNWARD and STYLUS_CURVE_UPWARD flip over in a carousel fashion
 at the top and bottom values. Observe that all styli from all attached
 tablets share the same state-variable. There was little point in
 differentiating them. Also, with a max of 30 styli it would become messy:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void change_curve(int fake_key, struct common_data* cdp)
{
	char new_curve[CURVEMAX];

	if (fake_key == STYLUS_CURVE_DOWNWARD) {
		if (stylus_presscurve > 1) {
			stylus_presscurve--;
		} else {
			stylus_presscurve = 7;
		}
	} else if (fake_key == STYLUS_CURVE_DEFAULT) {
		stylus_presscurve = 4;
	} else if (fake_key == STYLUS_CURVE_UPWARD) {
		if (stylus_presscurve < 7) {
			stylus_presscurve++;
		} else {
			stylus_presscurve = 1;
		}
	}

	switch (stylus_presscurve) {

		case 1:
		sprintf(new_curve, "0 75 25 100");
		break;

		case 2:
		sprintf(new_curve, "0 50 50 100");
		break;

		case 3:
		sprintf(new_curve, "0 25 75 100");
		break;

		case 4:
		sprintf(new_curve, "0 0 100 100");
		break;

		case 5:
		sprintf(new_curve, "25 0 100 75");
		break;

		case 6:
		sprintf(new_curve, "50 0 100 50");
		break;

		case 7:
		sprintf(new_curve, "75 0 100 25");
		break;

		default:
		sprintf(new_curve, "0 0 100 100");
		break;
	}

	snprintf(execbuffer, MAXBUFFER, "xsetwacom set %s PressCurve %s",
						cdp->sty1name, new_curve);

/* FIXME This does not catch return errors from the system call: */
	if ((system(execbuffer)) == NON_VALID) {
		fprintf(stderr, "Failed to fork a shell for \
xsetwacom to set PressCurve!\n");
	} else {
		if (be_verbose) {
			fprintf(stderr,
				"(%s Sensitivity = %i PressCurve = \"%s\") ",
				cdp->sty1name, stylus_presscurve, new_curve);
		}
	}

	if (id_sty2 != NULL) {
		snprintf(execbuffer, MAXBUFFER,"xsetwacom set %s PressCurve %s",
						cdp->sty2name, new_curve);

/* FIXME This does not catch return errors from the system call: */
		if ((system(execbuffer)) == NON_VALID) {
			fprintf(stderr, "Failed to fork a shell for \
xsetwacom to set PressCurve!\n");
		} else {
			if (be_verbose) {
				fprintf(stderr,
				"\n(%s Sensitivity = %i PressCurve = \"%s\") ",
				cdp->sty2name, stylus_presscurve, new_curve);
			}
		}
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Here we toggle the stylus/styli mode directly (without using xsetwacom):
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void toggle_stylus(struct common_data* cdp)
{
	unsigned char mode;

	mode = *cdp->sty1mode;
	mode = mode ? Relative : Absolute;

	XSetDeviceMode(display, (XDevice*)id_sty1, mode);
	*cdp->sty1mode = mode;
	if (be_verbose) {
		fprintf(stderr,"(Mode of %s now is %s) ",
			cdp->sty1name, (mode?"Absolute":"Relative"));
	}

	if (id_sty2 != NULL) {
		mode = *cdp->sty2mode;
		mode = mode ? Relative : Absolute;

		XSetDeviceMode(display, (XDevice*)id_sty2, mode);
		*cdp->sty2mode = mode;
		if (be_verbose) {
			fprintf(stderr,"\n(Mode of %s now is %s) ",
			cdp->sty2name, (mode?"Absolute":"Relative"));
		}
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Staging area before toggle_stylus, change_curve or change_rotate:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void prepare_action(int fake_key, int which_action)
{
	int i;

	struct model_index* mip;
	struct common_data* cdp = NULL;

	mip = model_list;
	for (i = 0; i < MAXPAD; i++, mip++) {
		if ((mip->ux->common_data.sty1id != NULL)
		&& (mip->ux->common_data.sty1id == id_sty1)) {
			cdp = &mip->ux->common_data;
			break;
		}
		if ((mip->ux2->common_data.sty1id != NULL)
		&& (mip->ux2->common_data.sty1id == id_sty1)) {
			cdp = &mip->ux2->common_data;
			break;
		}
		if ((mip->bbo->common_data.sty1id != NULL)
		&& (mip->bbo->common_data.sty1id == id_sty1)) {
			cdp = &mip->bbo->common_data;
			break;
		}
		if ((mip->bee->common_data.sty1id != NULL)
		&& (mip->bee->common_data.sty1id == id_sty1)) {
			cdp = &mip->bee->common_data;
			break;
		}
		if ((mip->i3->common_data.sty1id != NULL)
		&& (mip->i3->common_data.sty1id == id_sty1)) {
			cdp = &mip->i3->common_data;
			break;
		}
		if ((mip->i3s->common_data.sty1id != NULL)
		&& (mip->i3s->common_data.sty1id == id_sty1)) {
			cdp = &mip->i3s->common_data;
			break;
		}
		if ((mip->g4->common_data.sty1id != NULL)
		&& (mip->g4->common_data.sty1id == id_sty1)) {
			cdp = &mip->g4->common_data;
			break;
		}
		if ((mip->g4b->common_data.sty1id != NULL)
		&& (mip->g4b->common_data.sty1id == id_sty1)) {
			cdp = &mip->g4b->common_data;
			break;
		}
	}

	if (cdp) {
		if (which_action == mode_action) {
			toggle_stylus(cdp);
		} else if (which_action == curve_action) {
			change_curve(fake_key, cdp);
		} else if (which_action == rotate_action) {
			change_rotate(fake_key, cdp);
		}
		return;
	}

	fprintf(stderr,"(No stylus seen!) ");

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 This function acts like a traffic cop for keycodes. Sends real keycodes to
 XTestFakeKeyEvent. Interprets fake keycodes (currently keycodes standing
 in for mouse button presses) and sends them to the appropriate function.
 Only existing mouse buttons, defined through the driver of the active core
 pointer, can be simulated. If you run this function in a graphical debugger,
 the mouse _will_ be effected:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static int fake_event(Display *display, unsigned int keycode, Bool is_press,
							unsigned long delay)
{
	if ((MOUSE_BUTTON_MIN < keycode) && (keycode < MOUSE_BUTTON_MAX)) {
/* Mouse button code */
		if (be_verbose) {
			fprintf(stderr, "(MOUSE ");
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
				fprintf(stderr, "real button %d found) ",
									button);
			}
			return XTestFakeButtonEvent(display, button, is_press,
									delay);
		} else {
			if (be_verbose) {
				fprintf(stderr, "no real button %d found!) ",
									button);
			}
			return 0;
		}
	} else {
/* Keycode */
		if (be_verbose) {
			fprintf(stderr, "(KEYBD) ");
		}
		return XTestFakeKeyEvent(display, keycode, is_press, delay);
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Staging area before sending any actual keycodes:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void send_keys(int keys[], int button_state)
{
/* Pull in a possible DelayEachKeycode to use as pause: */
	static struct timespec add_time;
	add_time.tv_sec = key_sec;
	add_time.tv_nsec = key_nsec;

	Window present_window = None;
	int present_state;

	int i;
	int m = 0;
	long j = (long)keys;

/* Find out how many keycodes to send: */
	while ((keys[m]) && (m < MAXKEYS)) {
		m++;
	}
	m--;

/* Prevent key-repeat from running absolutely wild in unintended windows. But
 only abort the sending of keycodes if we are in a repeat mode. Otherwise
 mouse/keyboard can become a bit out of sync with the button/key-state. For
 example, if we simulate a mouse scroll-wheel and have configured the WM to
 switch virtual desktops on root window 'scrolls', then the next desktop would
 be another focus-window, and the mouse scroll wheel remain in a 'pressed'
 state because we didn't send the 'release' keycode:*/
	if (key_repeat) {
		XGetInputFocus(display, &present_window, &present_state);
		if (focus_window != present_window) {
			if (be_verbose) {
				fprintf(stderr,"(none) Focus window changed\n");
			}
			abort_repeat = 1;
			return;
		}
	}

	if (button_state) {
		if (be_verbose) {
		fprintf(stderr,"down\nDelay is %i sec %i nsec\nSent keycodes: ",
							key_sec, key_nsec);
		}
		for (i = 0; i <= m; i++) {
			if (key_sec || key_nsec) {
				nanosleep(&add_time, NULL);
			}
			if (keys[i] == TOGGLE_STYLUS) {
				if (be_verbose) {
					fprintf(stderr,"(Stylus Mode toggle) ");
				}
				prepare_action(0, mode_action);
			} else if ((keys[i] >= STYLUS_CURVE_DOWNWARD)
				&& (keys[i] <= STYLUS_CURVE_UPWARD)) {
				if (be_verbose) {
					fprintf(stderr,"(Stylus PressCurve)\n");
				}
				prepare_action(keys[i], curve_action);
			} else if ((keys[i] >= TABLET_ROTATE_NONE)
				&& (keys[i] <= TABLET_ROTATE_CCW_AGAIN)) {
				if (be_verbose) {
					fprintf(stderr,"(Tablet Rotate) ");
				}
				prepare_action(keys[i], rotate_action);
			} else {
				fake_event(display, keys[i], True, CurrentTime);
			}
			if (be_verbose) {
				fprintf(stderr, "%i ", keys[i]);
			}
		}
		if (be_verbose) {
			fprintf(stderr, "\n");
		}
	} else {
		if (be_verbose) {
		fprintf(stderr,"up\nDelay is %i sec %i nsec\nSent keycodes: ",
							key_sec, key_nsec);
		}
		for (i = j+m; i >= j; i--, m--) {
			if (key_sec || key_nsec) {
				nanosleep(&add_time, NULL);
			}
			if (keys[m] == TOGGLE_STYLUS) {
				if (be_verbose) {
					fprintf(stderr,
					"(Stylus Mode toggle - skipped) ");
				}
			} else if ((keys[m] >= STYLUS_CURVE_DOWNWARD)
				&& (keys[m] <= STYLUS_CURVE_UPWARD)) {
				if (be_verbose) {
					fprintf(stderr,
					"(Stylus PressCurve - skipped) ");
				}
			} else if ((keys[m] >= TABLET_ROTATE_NONE)
				&& (keys[m] <= TABLET_ROTATE_CCW_AGAIN)) {
				if (be_verbose) {
					fprintf(stderr,
					"(Tablet Rotate - skipped) ");
				}
			} else {
				fake_event(display, keys[m], False,CurrentTime);
			}
			if (be_verbose) {
				fprintf(stderr, "%i ", keys[m]);
			}
		}
		if (be_verbose) {
			fprintf(stderr, "\n");
		}
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Here we use two double-loops to handle button/touch strip repeats. The first
 set gives the user a 'grace period' where nothing happens (defined through
 ButtonRepeatAfter/TouchRepeatAfter in each program record). Once we enter the
 second set, the repeat of sending keycodes begins (each iteration delayed by
 what is defined through DelayButtonRepeat/DelayTouchRepeat).

 A multi-threaded approach would be better and more clean, but who will code it?
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void do_repeat(void* base_address, void* record_address,
	XDeviceButtonEvent* button, XDeviceMotionEvent* motion, int strip)
{
	const char* faking_button = "Faking button press ";

	static struct timespec nap_time;

	int count = 1;
	int eat_event = 1;
	int button_state = 1;

	XEvent Event;
	XDeviceButtonEvent* tmp_button;
	XDeviceMotionEvent* tmp_motion;

	struct ux_program* pux;
	struct ux2_program* pux2;
	struct bbo_program* pbbo;
	struct bee_program* pbee;
	struct i3_program* pi3;
	struct i3s_program* pi3s;
	struct g4_program* pg4;
	struct g4b_program* pg4b;

	struct common_data* base_cdp = NULL;
	struct common_data* cdp = NULL;
	struct button_data* bdp = NULL;
	struct touch_data* tdp = NULL;

	if (is_ux) {
		pux = base_address;
		base_cdp = &pux->common_data;
		pux = record_address;
		cdp = &pux->common_data;
		bdp = &pux->button_data;
		tdp = &pux->touch_data;
	} else if (is_ux2) {
		pux2 = base_address;
		base_cdp = &pux2->common_data;
		pux2 = record_address;
		cdp = &pux2->common_data;
		bdp = &pux2->button_data;
		tdp = &pux2->touch_data;
	} else if (is_bbo) {
		pbbo = base_address;
		base_cdp = &pbbo->common_data;
		pbbo = record_address;
		cdp = &pbbo->common_data;
		bdp = &pbbo->button_data;
		tdp = &pbbo->touch_data;
	} else if (is_bee) {
		pbee = base_address;
		base_cdp = &pbee->common_data;
		pbee = record_address;
		cdp = &pbee->common_data;
		bdp = &pbee->button_data;
		tdp = &pbee->touch_data;
	} else if (is_i3) {
		pi3 = base_address;
		base_cdp = &pi3->common_data;
		pi3 = record_address;
		cdp = &pi3->common_data;
		bdp = &pi3->button_data;
		tdp = &pi3->touch_data;
	} else if (is_i3s) {
		pi3s = base_address;
		base_cdp = &pi3s->common_data;
		pi3s = record_address;
		cdp = &pi3s->common_data;
		bdp = &pi3s->button_data;
		tdp = &pi3s->touch_data;
	} else if (is_g4) {
		pg4 = base_address;
		base_cdp = &pg4->common_data;
		pg4 = record_address;
		cdp = &pg4->common_data;
		bdp = &pg4->button_data;
	} else if (is_g4b) {
		pg4b = base_address;
		base_cdp = &pg4b->common_data;
		pg4b = record_address;
		cdp = &pg4b->common_data;
		bdp = &pg4b->button_data;
	}

/* The value of the nanoseconds field must be in the range 0 to 999999999

999999999 =~ 1 second
100000000 = 0.1         = 1/10           = 1 deci-
 10000000 = 0.01        = 1/100          = 1 centi-
  1000000 = 0.001       = 1/1000         = 1 milli-
   100000 = 0.0001      = 1/10.000
    10000 = 0.00001     = 1/100.000
     1000 = 0.000001    = 1/1000.000     = 1 micro-
      100 = 0.0000001   = 1/10.000.000
       10 = 0.00000001  = 1/100.000.000
        1 = 0.000000001 = 1/1000.000.000 = 1 nano-
*/

/* Incoming events need a bit of time to become 'visible': */
	nap_time.tv_sec = 0; /* 0s */
	nap_time.tv_nsec = 200000000; /* 0.2s */
	nanosleep(&nap_time, NULL);

/* ButtonRepeatAfter/TouchRepeatAfter: */
	if (strip) {
		nap_time.tv_sec = tdp->repeat_after[0];
		nap_time.tv_nsec = tdp->repeat_after[1];
		if (be_verbose) {
			fprintf(stderr,"TouchRepeatAfter is %i sec %i nsec\n",
				tdp->repeat_after[0], tdp->repeat_after[1]);
		}
	} else {
		nap_time.tv_sec = bdp->repeat_after[0];
		nap_time.tv_nsec = bdp->repeat_after[1];
		if (be_verbose) {
			fprintf(stderr,"ButtonRepeatAfter is %i sec %i nsec\n",
				bdp->repeat_after[0], bdp->repeat_after[1]);
		}
	}

/* OBSERVE: The order in which we "eat" events (XCheckTypedEvent) _matter_
 since the purpose of the operation is to discard anything not coming from
 the active button or touch strip: */
	while (eat_event) {
		while (eat_event) {
			eat_event = 0;

			if (XCheckTypedEvent(display, proximity_in_type,
								&Event)) {
				eat_event = 1;
			}
			if (strip) {
				if (XCheckTypedEvent(display,
						button_press_type, &Event)) {
					eat_event = 1;
				}
				if (XCheckTypedEvent(display,
						button_release_type, &Event)) {
					eat_event = 1;
				}
				if (XCheckTypedEvent(display, motion_type,
								&Event)) {
					eat_event = 1;
					tmp_motion = (XDeviceMotionEvent*)
									&Event;
					if (tmp_motion->deviceid ==
							*base_cdp->padid) {
						XPutBackEvent(display, &Event);
						return;
					}
				}
				if (XCheckTypedEvent(display,
						proximity_out_type, &Event)) {
					eat_event = 1;
					tmp_motion = (XDeviceMotionEvent*)
									&Event;
					if (tmp_motion->deviceid ==
							*base_cdp->padid) {
						return;
					}
				}
			} else {
				if (XCheckTypedEvent(display,
						proximity_out_type, &Event)) {
					eat_event = 1;
				}
				if (XCheckTypedEvent(display, motion_type,
								&Event)) {
					eat_event = 1;
				}
				if (XCheckTypedEvent(display,
						button_press_type, &Event)) {
					eat_event = 1;
					tmp_button = (XDeviceButtonEvent*)
									&Event;
					if ((tmp_button->button ==
								button->button)
					&& (tmp_button->deviceid ==
							*base_cdp->padid)) {
						XPutBackEvent(display, &Event);
					}
				}
				if (XCheckTypedEvent(display,
						button_release_type, &Event)) {
					eat_event = 1;
					tmp_button = (XDeviceButtonEvent*)
									&Event;
					if ((tmp_button->button ==
								button->button)
					&& (tmp_button->deviceid ==
							*base_cdp->padid)) {
						XPutBackEvent(display, &Event);
						return;
					}
				}
			}
		}

		if (count) {
			nanosleep(&nap_time, NULL);
			count = 0;
			eat_event = 1;
		} else {
			break;
		}
	}

/* The button/touch strip is still pressed/held. Adjust global flags:*/
	key_repeat = 1;
	abort_repeat = 0;

/* DelayButtonRepeat/DelayTouchRepeat: */
	if (strip) {
		nap_time.tv_sec = tdp->repeat_delay[0];
		nap_time.tv_nsec = tdp->repeat_delay[1];
		if (be_verbose) {
			fprintf(stderr,"DelayTouchRepeat is %i sec %i nsec\n",
				tdp->repeat_delay[0], tdp->repeat_delay[1]);
		}
	} else {
		nap_time.tv_sec = bdp->repeat_delay[0];
		nap_time.tv_nsec = bdp->repeat_delay[1];
		if (be_verbose) {
			fprintf(stderr,"DelayButtonRepeat is %i sec %i nsec\n",
				bdp->repeat_delay[0], bdp->repeat_delay[1]);
		}
	}

	while (1) {
		eat_event = 1;

/* It is absolutely CRUCIAL to toggle button_state HERE for the XPutBackEvent
 to work correctly when dealing with button-repeat. Plug in two tablets, hold
 one button, then press the same button on the other tablet. You'll get a
 truly confused keyboard unless the state is toggled HERE ---> */
		if (!strip) {
			button_state = button_state ? 0 : 1;
		}

		while (eat_event) {
			eat_event = 0;

			if (XCheckTypedEvent(display, proximity_in_type,
								&Event)) {
				eat_event = 1;
			}
			if (strip) {
				if (XCheckTypedEvent(display,
						button_press_type, &Event)) {
					eat_event = 1;
				}
				if (XCheckTypedEvent(display,
						button_release_type, &Event)) {
					eat_event = 1;
				}
				if (XCheckTypedEvent(display, motion_type,
								&Event)) {
					eat_event = 1;
					tmp_motion = (XDeviceMotionEvent*)
									&Event;
					if (tmp_motion->deviceid ==
							*base_cdp->padid) {
						XPutBackEvent(display, &Event);
						return;
					}
				}
				if (XCheckTypedEvent(display,
						proximity_out_type, &Event)) {
					eat_event = 1;
					tmp_motion = (XDeviceMotionEvent*)
									&Event;
					if (tmp_motion->deviceid ==
							*base_cdp->padid) {
						return;
					}
				}
			} else {
				if (XCheckTypedEvent(display,
						proximity_out_type, &Event)) {
					eat_event = 1;
				}
				if (XCheckTypedEvent(display, motion_type,
								&Event)) {
					eat_event = 1;
				}
				if (XCheckTypedEvent(display,
						button_press_type, &Event)) {
					eat_event = 1;
				}
				if (XCheckTypedEvent(display,
						button_release_type, &Event)) {
					eat_event = 1;
					tmp_button = (XDeviceButtonEvent*)
									&Event;
					if ((tmp_button->button ==
								button->button)
					&& (tmp_button->deviceid ==
							*base_cdp->padid)) {
						if (!button_state) {
							XPutBackEvent(display,
									&Event);
						}
						return;
					}
				}
			}
		}

		if (strip) {
			button_state = 1;
			if (be_verbose) {
				fprintf(stderr, "%s", faking_button);
			}

			switch (strip) {
				case 1:
					send_keys(tdp->left_touch_up,
								button_state);
					button_state = 0;
					if (be_verbose) {
						fprintf(stderr, "%s",
								faking_button);
					}
					send_keys(tdp->left_touch_up,
								button_state);
					break;
				case 2:
					send_keys(tdp->left_touch_down,
								button_state);
					button_state = 0;
					if (be_verbose) {
						fprintf(stderr, "%s",
								faking_button);
					}
					send_keys(tdp->left_touch_down,
								button_state);
					break;
				case 3:
					send_keys(tdp->right_touch_up,
								button_state);
					button_state = 0;
					if (be_verbose) {
						fprintf(stderr, "%s",
								faking_button);
					}
					send_keys(tdp->right_touch_up,
								button_state);
					break;
				case 4:
					send_keys(tdp->right_touch_down,
								button_state);
					button_state = 0;
					if (be_verbose) {
						fprintf(stderr, "%s",
								faking_button);
					}
					send_keys(tdp->right_touch_down,
								button_state);
					break;
				default:
					return;
			}
		} else {
			if (be_verbose) {
				fprintf(stderr, "Button %i ", button->button);
			}

			switch (button->button) {
				case 1:
					send_keys(bdp->button1, button_state);
					break;

				case 2:
					send_keys(bdp->button2, button_state);
					break;

				case 3:
					send_keys(bdp->button3, button_state);
					break;

				case 4:
					send_keys(bdp->button4, button_state);
					break;

				case 5:
					send_keys(bdp->button5, button_state);
					break;

				case 6:
					send_keys(bdp->button6, button_state);
					break;

				case 7:
					send_keys(bdp->button7, button_state);
					break;

				case 8:
					send_keys(bdp->button8, button_state);
					break;

				case 9:
					send_keys(bdp->button9, button_state);
					break;

				case 10:
					send_keys(bdp->button10, button_state);
					break;

				case 11:
					send_keys(bdp->button11, button_state);
					break;

				case 12:
					send_keys(bdp->button12, button_state);
					break;

				case 13:
					send_keys(bdp->button13, button_state);
					break;

				case 14:
					send_keys(bdp->button14, button_state);
					break;

				case 15:
					send_keys(bdp->button15, button_state);
					break;

				case 16:
					send_keys(bdp->button16, button_state);
					break;
				case 17:
					send_keys(bdp->button17, button_state);
					break;
				case 18:
					send_keys(bdp->button18, button_state);
					break;

				default:
					return;
			}
		}
/* Stop repeating if the focus window has changed: */
		if (abort_repeat) {
			return;
		}
		nanosleep(&nap_time, NULL);
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Set things up for a 'pad' Touch Strip motion. Check for a repeat at the end:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void do_motion(void* base_address, void* record_address,
				XDeviceMotionEvent* motion, XEvent Event)
{
/* Reset a global flag: */
	key_repeat = 0;

	const char* faking_button = "Faking button press ";

	const char* key_zero = "is zero!\n"
"0 or below keycode 9, above keycode 1023, or in other out-of-bounds areas.\n"
"Perhaps it is not defined at all in the configuration file.";

/* Set the Touch Strip state histories to 'smart' values. This is to prevent
 action on the very first touch, before a real history is recorded. Top is 1,
 Bottom is 4096 (previous x*2, ie 13 finger/stylus positions):*/
	static int elder_rotation = 1;
	static int old_rotation = 1;
	static int elder_throttle = 1;
	static int old_throttle = 1;

/* This might be unnecessary, but the following variables are targets of
 right/left shifted unsigned values. Make them unsigned as well...:*/
	unsigned static int more_rotation = 1;
	unsigned static int less_rotation = 1;
	unsigned static int more_throttle = 1;
	unsigned static int less_throttle = 1;

/* We right/left shift these values. Keep them unsigned so that vacant bits
 become zero no matter which size machine we run on: */
	unsigned int rotation = 0;
	unsigned int throttle = 0;

/* We always begin by faking a button "down": */
	int button_state = 1;

	int repeat_left_up = 0;
	int repeat_left_down = 0;
	int repeat_right_up = 0;
	int repeat_right_down = 0;

	if (motion->axes_count >= 5) {
		rotation = motion->axis_data[3];
		throttle = motion->axis_data[4];
/* Future proofing for when/if linuxwacom is adjusted to a 'real' axes count: */
	} else if (motion->axes_count == 2) {
/* Intuos3/Cintiq: */
		rotation = motion->axis_data[0];
		throttle = motion->axis_data[1];
	} else if (motion->axes_count == 1) {
/* Intuos3 'small': */
		rotation = motion->axis_data[0];
	}

	struct ux_program* pux;
	struct ux2_program* pux2;
	struct bbo_program* pbbo;
	struct bee_program* pbee;
	struct i3_program* pi3;
	struct i3s_program* pi3s;
	struct common_data* cdp = NULL;
	struct touch_data* tdp = NULL;

	if (is_ux) {
		pux = record_address;
		cdp = &pux->common_data;
		tdp = &pux->touch_data;
	} else if (is_ux2) {
		pux2 = record_address;
		cdp = &pux2->common_data;
		tdp = &pux2->touch_data;
	} else if (is_bbo) {
		pbbo = record_address;
		cdp = &pbbo->common_data;
		tdp = &pbbo->touch_data;
	} else if (is_bee) {
		pbee = record_address;
		cdp = &pbee->common_data;
		tdp = &pbee->touch_data;
	} else if (is_i3) {
		pi3 = record_address;
		cdp = &pi3->common_data;
		tdp = &pi3->touch_data;
	} else if (is_i3s) {
		pi3s = record_address;
		cdp = &pi3s->common_data;
		tdp = &pi3s->touch_data;
	}

	if (!*tdp->handle_touch) {
		if (be_verbose) {
			fprintf(stderr, "* Touch Strips are not \
defined/enabled in configuration file *\n");
		}
		return;
	}

/* DelayEachKeycode: */
	key_sec = cdp->keycode_delay[0];
	key_nsec = cdp->keycode_delay[1];

/* Left Touch Strip: */
	if (rotation) {
		if (be_verbose) {
			fprintf(stderr,
			"* Left Touch Strip * value %i\n", rotation);
		}
		if (*tdp->left_touch_up) {
			if (*tdp->repeat_left_up) {
				repeat_left_up = 1;
			}
			if ((rotation == less_rotation)
			&& (rotation < old_rotation)
			&& (rotation != elder_rotation)) {
				if (be_verbose) {
					fprintf(stderr,
					"(Left Touch Strip Up) value %i\n",
								rotation);
					fprintf(stderr,
					"Old value %i and Elder value %i\n",
						old_rotation, elder_rotation);
					fprintf(stderr, "%s", faking_button);
				}
				send_keys(tdp->left_touch_up, button_state);
				button_state = 0;
				if (be_verbose) {
					fprintf(stderr, "%s", faking_button);
				}
				send_keys(tdp->left_touch_up, button_state);
			}
		} else {
			if (be_verbose) {
				fprintf(stderr, "Up %s\n", key_zero);
			}
		}
		if (*tdp->left_touch_down) {
			if (*tdp->repeat_left_down) {
				repeat_left_down = 1;
			}
			if ((rotation == more_rotation)
			&& (rotation > old_rotation)
			&& (rotation != elder_rotation)) {
				if (be_verbose) {
					fprintf(stderr,
					"(Left Touch Strip Down) value %i\n",
								rotation);
					fprintf(stderr,
					"Old value %i and Elder value %i\n",
						old_rotation, elder_rotation);
					fprintf(stderr, "%s", faking_button);
				}
				send_keys(tdp->left_touch_down, button_state);
				button_state = 0;
				if (be_verbose) {
					fprintf(stderr, "%s", faking_button);
				}
				send_keys(tdp->left_touch_down, button_state);
			}
		} else {
			if (be_verbose) {
				fprintf(stderr, "Down %s\n", key_zero);
			}
		}
		elder_rotation = old_rotation;
		old_rotation = rotation;
		more_rotation = rotation << 1;
		less_rotation = rotation >> 1;
	}

/* Right Touch Strip: */
	if (throttle) {
		if (be_verbose) {
			fprintf(stderr,
			"* Right Touch Strip * value %i\n", throttle);
		}
		if (*tdp->right_touch_up) {
			if (*tdp->repeat_right_up) {
				repeat_right_up = 1;
			}
			if ((throttle == less_throttle)
			&& (throttle < old_throttle)
			&& (throttle != elder_throttle)) {
				if (be_verbose) {
					fprintf(stderr,
					"(Right Touch Strip Up) value %i\n",
								throttle);
					fprintf(stderr,
					"Old value %i and Elder value %i\n",
						old_throttle, elder_throttle);
					fprintf(stderr, "%s", faking_button);
				}
				send_keys(tdp->right_touch_up, button_state);
				button_state = 0;
				if (be_verbose) {
					fprintf(stderr, "%s", faking_button);
				}
				send_keys(tdp->right_touch_up, button_state);
			}
		} else {
			if (be_verbose) {
				fprintf(stderr, "Up %s\n", key_zero);
			}
		}
		if (*tdp->right_touch_down) {
			if (*tdp->repeat_right_down) {
				repeat_right_down = 1;
			}
			if ((throttle == more_throttle)
			&& (throttle > old_throttle)
			&& (throttle != elder_throttle)) {
				if (be_verbose) {
					fprintf(stderr,
					"(Right Touch Strip Down) value %i\n",
								throttle);
					fprintf(stderr,
					"Old value %i and Elder value %i\n",
						old_throttle, elder_throttle);
					fprintf(stderr, "%s", faking_button);
				}
				send_keys(tdp->right_touch_down, button_state);
				button_state = 0;
				if (be_verbose) {
					fprintf(stderr, "%s", faking_button);
				}
				send_keys(tdp->right_touch_down, button_state);
			}
		} else {
			if (be_verbose) {
				fprintf(stderr, "Down %s\n", key_zero);
			}
		}
		elder_throttle = old_throttle;
		old_throttle = throttle;
		more_throttle = throttle << 1;
		less_throttle = throttle >> 1;
	}

	if (repeat_left_up && rotation == 1) {
		do_repeat(base_address, record_address, NULL, motion, 1);
	}
	if (repeat_left_down && rotation == 4096) {
		do_repeat(base_address, record_address, NULL, motion, 2);
	}
	if (repeat_right_up && throttle == 1) {
		do_repeat(base_address, record_address, NULL, motion, 3);
	}
	if (repeat_right_down && throttle == 4096){
		do_repeat(base_address, record_address, NULL, motion, 4);
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Set things up for a 'pad' button press/release. Check for a repeat at the end:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void do_button(void* base_address, void* record_address,
				XDeviceButtonEvent* button, XEvent Event)
{
/* Reset a global flag: */
	key_repeat = 0;

	const char* key_zero = "is zero!\n"
"0 or below keycode 9, above keycode 1023, or in other out-of-bounds areas.\n"
"Perhaps it is not defined at all in the configuration file.";

	int button_state = (Event.type == button_press_type) ? 1 : 0;
	int repeat_button = 0;
	int ok_button = 0;

	struct ux_program* pux;
	struct ux2_program* pux2;
	struct bbo_program* pbbo;
	struct bee_program* pbee;
	struct i3_program* pi3;
	struct i3s_program* pi3s;
	struct g4_program* pg4;
	struct g4b_program* pg4b;

	struct common_data* cdp = NULL;
	struct button_data* bdp = NULL;
	struct wheel_data* wdp = NULL;

	if (is_ux) {
		pux = record_address;
		cdp = &pux->common_data;
		bdp = &pux->button_data;
	} else if (is_ux2) {
		pux2 = record_address;
		cdp = &pux2->common_data;
		bdp = &pux2->button_data;
	} else if (is_bbo) {
		pbbo = record_address;
		cdp = &pbbo->common_data;
		bdp = &pbbo->button_data;
		wdp = &pbbo->wheel_data;
	} else if (is_bee) {
		pbee = record_address;
		cdp = &pbee->common_data;
		bdp = &pbee->button_data;
	} else if (is_i3) {
		pi3 = record_address;
		cdp = &pi3->common_data;
		bdp = &pi3->button_data;
	} else if (is_i3s) {
		pi3s = record_address;
		cdp = &pi3s->common_data;
		bdp = &pi3s->button_data;
	} else if (is_g4) {
		pg4 = record_address;
		cdp = &pg4->common_data;
		bdp = &pg4->button_data;
		wdp = &pg4->wheel_data;
	} else if (is_g4b) {
		pg4b = record_address;
		cdp = &pg4b->common_data;
		bdp = &pg4b->button_data;
	}

/* DelayEachKeycode: */
	key_sec = cdp->keycode_delay[0];
	key_nsec = cdp->keycode_delay[1];

	if (be_verbose && button->button > 5) {
		fprintf(stderr, "Button %i ", button->button);
	}

/* Disable keyboard auto-repeat (whole system) while our button is processed: */
	if (button->button > 5) {
		XAutoRepeatOff(display);
	}

	switch (button->button) {
		case 1:
			if (*bdp->button1) {
				ok_button = 1;
				send_keys(bdp->button1, button_state);
				if (*bdp->repeat1) {
					repeat_button = 1;
				}
			}
			break;
		case 2:
			if (*bdp->button2) {
				ok_button = 1;
				send_keys(bdp->button2, button_state);
				if (*bdp->repeat2) {
					repeat_button = 2;
				}
			}
			break;
		case 3:
			if (*bdp->button3) {
				ok_button = 1;
				send_keys(bdp->button3, button_state);
				if (*bdp->repeat3) {
					repeat_button = 1;
				}
			}
			break;
		case 4:
			if (is_ux || is_ux2) {
				if (*bdp->button4) {
					ok_button = 1;
					send_keys(bdp->button4, button_state);
					if (*bdp->repeat4) {
						repeat_button = 1;
					}
				}
				break;
			}
			if (!wdp || !*wdp->handle_wheel) {
				if (be_verbose) {
					fprintf(stderr,
				"* Scroll Wheel is not defined/enabled in \
configuration file *\n");
				}
				return;
			}
			if (*wdp->scroll_wheel_up) {
				if (be_verbose) {
					fprintf(stderr,
						"(Scroll Wheel Up) Button %i ",
								button->button);
				}
				send_keys(wdp->scroll_wheel_up, button_state);
			} else {
				if (be_verbose) {
					fprintf(stderr,
					"(Scroll Wheel Up) Button %i %s\n",
						button->button, key_zero);
				}
			}
			return;
		case 5:
			if (is_ux || is_ux2) {
				if (*bdp->button5) {
					ok_button = 1;
					send_keys(bdp->button5, button_state);
					if (*bdp->repeat5) {
						repeat_button = 1;
					}
				}
				break;
			}
			if (!wdp && !*wdp->handle_wheel) {
				if (be_verbose) {
					fprintf(stderr,
				"* Scroll Wheel is not defined/enabled in \
configuration file *\n");
				}
				return;
			}
			if (*wdp->scroll_wheel_down) {
				if (be_verbose) {
					fprintf(stderr,
					"(Scroll Wheel Down) Button %i ",
								button->button);
				}
				send_keys(wdp->scroll_wheel_down, button_state);
			} else {
				if (be_verbose) {
					fprintf(stderr,
					"(Scroll Wheel Down) Button %i %s\n",
						button->button, key_zero);
				}
			}
			return;
		case 6:
			if (*bdp->button6) {
				ok_button = 1;
				send_keys(bdp->button6, button_state);
				if (*bdp->repeat6) {
					repeat_button = 1;
				}
			}
			break;
		case 7:
			if (*bdp->button7) {
				ok_button = 1;
				send_keys(bdp->button7, button_state);
				if (*bdp->repeat7) {
					repeat_button = 1;
				}
			}
			break;
		case 8:
			if (*bdp->button8) {
				ok_button = 1;
				send_keys(bdp->button8, button_state);
				if (*bdp->repeat8) {
					repeat_button = 1;
				}
			}
			break;
		case 9:
			if (*bdp->button9) {
				ok_button = 1;
				send_keys(bdp->button9, button_state);
				if (*bdp->repeat9) {
					repeat_button = 1;
				}
			}
			break;
		case 10:
			if (*bdp->button10) {
				ok_button = 1;
				send_keys(bdp->button10, button_state);
				if (*bdp->repeat10) {
					repeat_button = 1;
				}
			}
			break;
		case 11:
			if (*bdp->button11) {
				ok_button = 1;
				send_keys(bdp->button11, button_state);
				if (*bdp->repeat11) {
					repeat_button = 1;
				}
			}
			break;
		case 12:
			if (*bdp->button12) {
				ok_button = 1;
				send_keys(bdp->button12, button_state);
				if (*bdp->repeat12) {
					repeat_button = 1;
				}
			}
			break;
		case 13:
			if (*bdp->button13) {
				ok_button = 1;
				send_keys(bdp->button13, button_state);
				if (*bdp->repeat13) {
					repeat_button = 1;
				}
			}
			break;
		case 14:
			if (*bdp->button14) {
				ok_button = 1;
				send_keys(bdp->button14, button_state);
				if (*bdp->repeat14) {
					repeat_button = 1;
				}
			}
			break;
		case 15:
			if (*bdp->button15) {
				ok_button = 1;
				send_keys(bdp->button15, button_state);
				if (*bdp->repeat15) {
					repeat_button = 1;
				}
			}
			break;
		case 16:
			if (*bdp->button16) {
				ok_button = 1;
				send_keys(bdp->button16, button_state);
				if (*bdp->repeat16) {
					repeat_button = 1;
				}
			}
			break;
		case 17:
			if (*bdp->button17) {
				ok_button = 1;
				send_keys(bdp->button17, button_state);
				if (*bdp->repeat17) {
					repeat_button = 1;
				}
			}
			break;
		case 18:
			if (*bdp->button18) {
				ok_button = 1;
				send_keys(bdp->button18, button_state);
				if (*bdp->repeat18) {
					repeat_button = 1;
				}
			}
			break;
		default:
			return;

	}

	if (be_verbose && !ok_button) {
		fprintf(stderr, "%s\n", key_zero);
	}

	if (ok_button && button_state && repeat_button) {
		do_repeat(base_address, record_address, button, NULL, 0);
	}

/* Enable keyboard auto-repeat (whole system). Also done at program exit: */
	XAutoRepeatOn(display);

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Automatically change stylus PressCurve if the current record structure value
 differs from what the 'history-buffer' holds. Observe that all styli from
 all attached tablets share the same history-buffer. There was little point in
 differentiating them. Also, with a max of 30 styli it would become messy:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void do_stylus(void* base_address, void* record_address)
{
	struct ux_program* pux;
	struct ux2_program* pux2;
	struct bbo_program* pbbo;
	struct bee_program* pbee;
	struct i3_program* pi3;
	struct i3s_program* pi3s;
	struct g4_program* pg4;
	struct g4b_program* pg4b;
	struct nop_program* pnop;

	struct common_data* cdp = NULL;
	struct common_data* base_cdp = NULL;

	if (is_ux) {
		pux = base_address;
		base_cdp = &pux->common_data;
		pux = record_address;
		cdp = &pux->common_data;
	} else if (is_ux2) {
		pux2 = base_address;
		base_cdp = &pux2->common_data;
		pux2 = record_address;
		cdp = &pux2->common_data;
	} else if (is_bbo) {
		pbbo = base_address;
		base_cdp = &pbbo->common_data;
		pbbo = record_address;
		cdp = &pbbo->common_data;
	} else if (is_bee) {
		pbee = base_address;
		base_cdp = &pbee->common_data;
		pbee = record_address;
		cdp = &pbee->common_data;
	} else if (is_i3) {
		pi3 = base_address;
		base_cdp = &pi3->common_data;
		pi3 = record_address;
		cdp = &pi3->common_data;
	} else if (is_i3s) {
		pi3s = base_address;
		base_cdp = &pi3s->common_data;
		pi3s = record_address;
		cdp = &pi3s->common_data;
	} else if (is_g4) {
		pg4 = base_address;
		base_cdp = &pg4->common_data;
		pg4 = record_address;
		cdp = &pg4->common_data;
	} else if (is_g4b) {
		pg4b = base_address;
		base_cdp = &pg4b->common_data;
		pg4b = record_address;
		cdp = &pg4b->common_data;
	} else if (is_nop) {
		pnop = base_address;
		base_cdp = &pnop->common_data;
		pnop = record_address;
		cdp = &pnop->common_data;
	}

	if (is_sty1) {
		if (cdp->stylus1_presscurve == NULL) {
			return;
		}
		if (strcmp(curvebuffer, cdp->stylus1_presscurve) != 0) {
			snprintf(curvebuffer, CURVEMAX, "%s",
						cdp->stylus1_presscurve);
			snprintf(execbuffer, MAXBUFFER,
					"xsetwacom set %s PressCurve %s",
					base_cdp->sty1name, curvebuffer);

/* FIXME This does not catch return errors from the system call: */
			if ((system(execbuffer)) == NON_VALID) {
				fprintf(stderr, "Failed to fork a shell for \
xsetwacom to set PressCurve!\n");
			} else {
				if (be_verbose) {
					fprintf(stderr,
				"Stylus 1 (%s) PressCurve set to: \"%s\"\n",
			 		base_cdp->sty1name, curvebuffer);
				}
			}
		}
		return;
	}

	if (is_sty2) {
		if (cdp->stylus2_presscurve == NULL) {
			return;
		}
		if (strcmp(curvebuffer, cdp->stylus2_presscurve) != 0) {
			snprintf(curvebuffer, CURVEMAX, "%s",
   						cdp->stylus2_presscurve);
			snprintf(execbuffer, MAXBUFFER,
					"xsetwacom set %s PressCurve %s",
					base_cdp->sty2name, curvebuffer);

/* FIXME This does not catch return errors from the system call: */
			if ((system(execbuffer)) == NON_VALID) {
				fprintf(stderr, "Failed to fork a shell for \
xsetwacom to set PressCurve!\n");
			} else {
				if (be_verbose) {
					fprintf(stderr,
				"Stylus 2 (%s) PressCurve set to: \"%s\"\n",
			 	base_cdp->sty2name, curvebuffer);
				}
			}
		}
		return;
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Find current window focus and return the record structure address to use:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void* find_focus(void* record_address, XEvent Event)
{
	const char* prog_focus = "Program window focus =";

	focus_window = None;
	name_change = 0;

	struct ux_program* pux;
	struct ux2_program* pux2;
	struct bbo_program* pbbo;
	struct bee_program* pbee;
	struct i3_program* pi3;
	struct i3s_program* pi3s;
	struct g4_program* pg4;
	struct g4b_program* pg4b;
	struct nop_program* pnop;

	XClassHint* class_hint;

	Window root, parent;
	Window* children;
	unsigned int num_children;

	int i;
	int in_list = 0;

/* Memory allocation. Free it later! */
	class_hint = XAllocClassHint();

/* Locate which window that currently has the focus and get its list of related
 windows. Then pull its ClassHint into our allocated structure: */
	XGetInputFocus(display, &focus_window, &focus_state);
	XQueryTree(display, focus_window, &root, &parent, &children,
								&num_children);
	XGetClassHint(display, focus_window, class_hint);

/* If the class hint (aka WM_CLASS) contains no class string we free the
 allocated memory for each structure member and get the ClassHint of the window
 parent, if it has one. Observe that we must skip the root window: */
	if ((!class_hint->res_class) && (parent) && (focus_window != root)) {
		XFree(class_hint->res_class);
		XFree(class_hint->res_name);
		XGetClassHint(display, parent, class_hint);
	}

/* If the root window had the focus, or if the active window or its parent had
 no class string at all, we use the "default" program definition from the read
 in configuration file when evaluating the event. Otherwise we start scanning
 for a match between the class strings in our list and the found window class:*/

	if (be_verbose) {
		fprintf(stderr,
"--------------------------------------------------------------------------\n");
	}

	if (is_ux) {
		pux = record_address;
		if ((focus_window == root) || (class_hint->res_class == NULL)) {
			record_address = pux->default_program;
			pux = record_address;
			if (be_verbose) {
				fprintf(stderr, "%s %s\n", prog_focus,
						pux->common_data.class_name);
			}
			if (strcmp(namebuffer, pux->common_data.class_name)
									!= 0) {
				snprintf(namebuffer, MAXBUFFER, "%s",
						pux->common_data.class_name);
				name_change = 1;
			}
			goto clean_up;
		}
		for (i = 0; i < record_num; i++, pux++) {
			if (strcmp(class_hint->res_class,
					pux->common_data.class_name) == 0) {
				in_list = 1;
				break;
			}
		}
		if (!in_list) {
			pux = record_address;
			record_address = pux->default_program;
			pux = record_address;
		} else {
			record_address = pux;
		}
		if (be_verbose) {
			fprintf(stderr, "%s %s\n", prog_focus,
						pux->common_data.class_name);
		}
		if (strcmp(namebuffer, pux->common_data.class_name) != 0) {
			snprintf(namebuffer, MAXBUFFER, "%s",
						pux->common_data.class_name);
			name_change = 1;
		}
		goto clean_up;
	} else if (is_ux2) {
		pux2 = record_address;
		if ((focus_window == root) || (class_hint->res_class == NULL)) {
			record_address = pux2->default_program;
			pux2 = record_address;
			if (be_verbose) {
				fprintf(stderr, "%s %s\n", prog_focus,
						pux2->common_data.class_name);
			}
			if (strcmp(namebuffer, pux2->common_data.class_name)
									!= 0) {
				snprintf(namebuffer, MAXBUFFER, "%s",
						pux2->common_data.class_name);
				name_change = 1;
			}
			goto clean_up;
		}
		for (i = 0; i < record_num; i++, pux2++) {
			if (strcmp(class_hint->res_class,
					pux2->common_data.class_name) == 0) {
				in_list = 1;
				break;
			}
		}
		if (!in_list) {
			pux2 = record_address;
			record_address = pux2->default_program;
			pux2 = record_address;
		} else {
			record_address = pux2;
		}
		if (be_verbose) {
			fprintf(stderr, "%s %s\n", prog_focus,
						pux2->common_data.class_name);
		}
		if (strcmp(namebuffer, pux2->common_data.class_name) != 0) {
			snprintf(namebuffer, MAXBUFFER, "%s",
						pux2->common_data.class_name);
			name_change = 1;
		}
		goto clean_up;
	} else if (is_bbo) {
		pbbo = record_address;
		if ((focus_window == root) || (class_hint->res_class == NULL)) {
			record_address = pbbo->default_program;
			pbbo = record_address;
			if (be_verbose) {
				fprintf(stderr, "%s %s\n", prog_focus,
						pbbo->common_data.class_name);
			}
			if (strcmp(namebuffer, pbbo->common_data.class_name)
									!= 0) {
				snprintf(namebuffer, MAXBUFFER, "%s",
						pbbo->common_data.class_name);
				name_change = 1;
			}
			goto clean_up;
		}
		for (i = 0; i < record_num; i++, pbbo++) {
			if (strcmp(class_hint->res_class,
					pbbo->common_data.class_name) == 0) {
				in_list = 1;
				break;
			}
		}
		if (!in_list) {
			pbbo = record_address;
			record_address = pbbo->default_program;
			pbbo = record_address;
		} else {
			record_address = pbbo;
		}
		if (be_verbose) {
			fprintf(stderr, "%s %s\n", prog_focus,
						pbbo->common_data.class_name);
		}
		if (strcmp(namebuffer, pbbo->common_data.class_name) != 0) {
			snprintf(namebuffer, MAXBUFFER, "%s",
						pbbo->common_data.class_name);
			name_change = 1;
		}
		goto clean_up;
	} else if (is_bee) {
		pbee = record_address;
		if ((focus_window == root) || (class_hint->res_class == NULL)) {
			record_address = pbee->default_program;
			pbee = record_address;
			if (be_verbose) {
				fprintf(stderr, "%s %s\n", prog_focus,
						pbee->common_data.class_name);
			}
			if (strcmp(namebuffer, pbee->common_data.class_name)
									!= 0) {
				snprintf(namebuffer, MAXBUFFER, "%s",
						pbee->common_data.class_name);
				name_change = 1;
			}
			goto clean_up;
		}
		for (i = 0; i < record_num; i++, pbee++) {
			if (strcmp(class_hint->res_class,
					pbee->common_data.class_name) == 0) {
				in_list = 1;
				break;
			}
		}
		if (!in_list) {
			pbee = record_address;
			record_address = pbee->default_program;
			pbee = record_address;
		} else {
			record_address = pbee;
		}
		if (be_verbose) {
			fprintf(stderr, "%s %s\n", prog_focus,
						pbee->common_data.class_name);
		}
		if (strcmp(namebuffer, pbee->common_data.class_name) != 0) {
			snprintf(namebuffer, MAXBUFFER, "%s",
						pbee->common_data.class_name);
			name_change = 1;
		}
		goto clean_up;
	} else if (is_i3) {
		pi3 = record_address;
		if ((focus_window == root) || (class_hint->res_class == NULL)) {
			record_address = pi3->default_program;
			pi3 = record_address;
			if (be_verbose) {
				fprintf(stderr, "%s %s\n", prog_focus,
						pi3->common_data.class_name);
			}
			if (strcmp(namebuffer, pi3->common_data.class_name)
									!= 0) {
				snprintf(namebuffer, MAXBUFFER, "%s",
						pi3->common_data.class_name);
				name_change = 1;
			}
			goto clean_up;
		}
		for (i = 0; i < record_num; i++, pi3++) {
			if (strcmp(class_hint->res_class,
					pi3->common_data.class_name) == 0) {
				in_list = 1;
				break;
			}
		}
		if (!in_list) {
			pi3 = record_address;
			record_address = pi3->default_program;
			pi3 = record_address;
		} else {
			record_address = pi3;
		}
		if (be_verbose) {
			fprintf(stderr, "%s %s\n", prog_focus,
						pi3->common_data.class_name);
		}
		if (strcmp(namebuffer, pi3->common_data.class_name) != 0) {
			snprintf(namebuffer, MAXBUFFER, "%s",
						pi3->common_data.class_name);
			name_change = 1;
		}
		goto clean_up;
	} else if (is_i3s) {
		pi3s = record_address;
		if ((focus_window == root) || (class_hint->res_class == NULL)) {
			record_address = pi3s->default_program;
			pi3s = record_address;
			if (be_verbose) {
				fprintf(stderr, "%s %s\n", prog_focus,
						pi3s->common_data.class_name);
			}
			if (strcmp(namebuffer, pi3s->common_data.class_name)
									!= 0) {
				snprintf(namebuffer, MAXBUFFER, "%s",
						pi3s->common_data.class_name);
				name_change = 1;
			}
			goto clean_up;
		}
		for (i = 0; i < record_num; i++, pi3s++) {
			if (strcmp(class_hint->res_class,
					pi3s->common_data.class_name) == 0) {
				in_list = 1;
				break;
			}
		}
		if (!in_list) {
			pi3s = record_address;
			record_address = pi3s->default_program;
			pi3s = record_address;
		} else {
			record_address = pi3s;
		}
		if (be_verbose) {
			fprintf(stderr, "%s %s\n", prog_focus,
						pi3s->common_data.class_name);
		}
		if (strcmp(namebuffer, pi3s->common_data.class_name) != 0) {
			snprintf(namebuffer, MAXBUFFER, "%s",
						pi3s->common_data.class_name);
			name_change = 1;
		}
		goto clean_up;
	} else if (is_g4) {
		pg4 = record_address;
		if ((focus_window == root) || (class_hint->res_class == NULL)) {
			record_address = pg4->default_program;
			pg4 = record_address;
			if (be_verbose) {
				fprintf(stderr, "%s %s\n", prog_focus,
						pg4->common_data.class_name);
			}
			if (strcmp(namebuffer, pg4->common_data.class_name)
									!= 0) {
				snprintf(namebuffer, MAXBUFFER, "%s",
						pg4->common_data.class_name);
				name_change = 1;
			}
			goto clean_up;
		}
		for (i = 0; i < record_num; i++, pg4++) {
			if (strcmp(class_hint->res_class,
					pg4->common_data.class_name) == 0) {
				in_list = 1;
				break;
			}
		}
		if (!in_list) {
			pg4 = record_address;
			record_address = pg4->default_program;
			pg4 = record_address;
		} else {
			record_address = pg4;
		}
		if (be_verbose) {
			fprintf(stderr, "%s %s\n", prog_focus,
						pg4->common_data.class_name);
		}
		if (strcmp(namebuffer, pg4->common_data.class_name) != 0) {
			snprintf(namebuffer, MAXBUFFER, "%s",
						pg4->common_data.class_name);
			name_change = 1;
		}
		goto clean_up;
	} else if (is_g4b) {
		pg4b = record_address;
		if ((focus_window == root) || (class_hint->res_class == NULL)) {
			record_address = pg4b->default_program;
			pg4b = record_address;
			if (be_verbose) {
				fprintf(stderr, "%s %s\n", prog_focus,
						pg4b->common_data.class_name);
			}
			if (strcmp(namebuffer, pg4b->common_data.class_name)
									!= 0) {
				snprintf(namebuffer, MAXBUFFER, "%s",
						pg4b->common_data.class_name);
				name_change = 1;
			}
			goto clean_up;
		}
		for (i = 0; i < record_num; i++, pg4b++) {
			if (strcmp(class_hint->res_class,
					pg4b->common_data.class_name) == 0) {
				in_list = 1;
				break;
			}
		}
		if (!in_list) {
			pg4b = record_address;
			record_address = pg4b->default_program;
			pg4b = record_address;
		} else {
			record_address = pg4b;
		}
		if (be_verbose) {
			fprintf(stderr, "%s %s\n", prog_focus,
						pg4b->common_data.class_name);
		}
		if (strcmp(namebuffer, pg4b->common_data.class_name) != 0) {
			snprintf(namebuffer, MAXBUFFER, "%s",
						pg4b->common_data.class_name);
			name_change = 1;
		}
		goto clean_up;
	} else if (is_nop) {
		pnop = record_address;
		if ((focus_window == root) || (class_hint->res_class == NULL)) {
			record_address = pnop->default_program;
			pnop = record_address;
			if (be_verbose) {
				fprintf(stderr, "%s %s\n", prog_focus,
						pnop->common_data.class_name);
			}
			if (strcmp(namebuffer, pnop->common_data.class_name)
									!= 0) {
				snprintf(namebuffer, MAXBUFFER, "%s",
						pnop->common_data.class_name);
				name_change = 1;
			}
			goto clean_up;
		}
		for (i = 0; i < record_num; i++, pnop++) {
			if (strcmp(class_hint->res_class,
					pnop->common_data.class_name) == 0) {
				in_list = 1;
				break;
			}
		}
		if (!in_list) {
			pnop = record_address;
			record_address = pnop->default_program;
			pnop = record_address;
		} else {
			record_address = pnop;
		}
		if (be_verbose) {
			fprintf(stderr, "%s %s\n", prog_focus,
						pnop->common_data.class_name);
		}
		if (strcmp(namebuffer, pnop->common_data.class_name) != 0) {
			snprintf(namebuffer, MAXBUFFER, "%s",
						pnop->common_data.class_name);
			name_change = 1;
		}
		goto clean_up;
	}

clean_up:

/* The allocated memory for the ClassHint structure, and each of its members,
 must be freed here. Also, the call to XQueryTree to get a list of related
 windows might have allocated memory for child entries. It must be released as
 well: */
	XFree(class_hint->res_class);
	XFree(class_hint->res_name);
	XFree(class_hint);
	if (children) XFree((char*)children);

	return record_address;

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Match the device ID which triggered an event with an attached device.
 Set flags and save all related ID's. Return the base structure address:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void* match_id(XDeviceButtonEvent* button, XDeviceMotionEvent* motion)
{
	int i;
	struct model_index* mip;

	if (button) {
		mip = model_list;
		for (i = 0; i < MAXPAD; i++, mip++) {
			if ((mip->ux->common_data.padid != NULL
			&& *mip->ux->common_data.padid ==
					button->deviceid)
			|| (mip->ux->common_data.sty1id != NULL
			&& *mip->ux->common_data.sty1id ==
					button->deviceid)
			|| (mip->ux->common_data.sty2id != NULL
			&& *mip->ux->common_data.sty2id ==
					button->deviceid)) {
				if (mip->ux->default_program == NULL) {
					return NULL;
				}
				if (mip->ux->common_data.padid != NULL
				&& *mip->ux->common_data.padid ==
							button->deviceid) {
					is_pad = 1;
					id_pad = mip->ux->common_data.padid;
				} else if (mip->ux->common_data.sty1id != NULL
				&& *mip->ux->common_data.sty1id ==
							button->deviceid) {
					is_sty1 = 1;
				} else if (mip->ux->common_data.sty2id != NULL
				&& *mip->ux->common_data.sty2id ==
							button->deviceid) {
					is_sty2 = 1;
				}
				is_ux = 1;
				record_num = mip->ux->common_data.num_record;
				if (mip->ux->common_data.sty1id != NULL) {
					id_sty1 = mip->ux->common_data.sty1id;
				}
				if (mip->ux->common_data.sty2id != NULL) {
					id_sty2 = mip->ux->common_data.sty2id;
				}
				return mip->ux;
			}
			if ((mip->ux2->common_data.padid != NULL
			&& *mip->ux2->common_data.padid ==
					button->deviceid)
			|| (mip->ux2->common_data.sty1id != NULL
			&& *mip->ux2->common_data.sty1id ==
					button->deviceid)
			|| (mip->ux2->common_data.sty2id != NULL
			&& *mip->ux2->common_data.sty2id ==
					button->deviceid)) {
				if (mip->ux2->default_program == NULL) {
					return NULL;
				}
				if (mip->ux2->common_data.padid != NULL
				&& *mip->ux2->common_data.padid ==
							button->deviceid) {
					is_pad = 1;
					id_pad = mip->ux2->common_data.padid;
				} else if (mip->ux2->common_data.sty1id != NULL
				&& *mip->ux2->common_data.sty1id ==
							button->deviceid) {
					is_sty1 = 1;
				} else if (mip->ux2->common_data.sty2id != NULL
				&& *mip->ux2->common_data.sty2id ==
							button->deviceid) {
					is_sty2 = 1;
				}
				is_ux2 = 1;
				record_num = mip->ux2->common_data.num_record;
				if (mip->ux2->common_data.sty1id != NULL) {
					id_sty1 = mip->ux2->common_data.sty1id;
				}
				if (mip->ux2->common_data.sty2id != NULL) {
					id_sty2 = mip->ux2->common_data.sty2id;
				}
				return mip->ux2;
			}
			if ((mip->bbo->common_data.padid != NULL
			&& *mip->bbo->common_data.padid ==
					button->deviceid)
			|| (mip->bbo->common_data.sty1id != NULL
			&& *mip->bbo->common_data.sty1id ==
					button->deviceid)
			|| (mip->bbo->common_data.sty2id != NULL
			&& *mip->bbo->common_data.sty2id ==
					button->deviceid)) {
				if (mip->bbo->default_program == NULL) {
					return NULL;
				}
				if (mip->bbo->common_data.padid != NULL
				&& *mip->bbo->common_data.padid ==
							button->deviceid) {
					is_pad = 1;
					id_pad = mip->bbo->common_data.padid;
				} else if (mip->bbo->common_data.sty1id != NULL
				&& *mip->bbo->common_data.sty1id ==
							button->deviceid) {
					is_sty1 = 1;
				} else if (mip->bbo->common_data.sty2id != NULL
				&& *mip->bbo->common_data.sty2id ==
							button->deviceid) {
					is_sty2 = 1;
				}
				is_bbo = 1;
				record_num = mip->bbo->common_data.num_record;
				if (mip->bbo->common_data.sty1id != NULL) {
					id_sty1 = mip->bbo->common_data.sty1id;
				}
				if (mip->bbo->common_data.sty2id != NULL) {
					id_sty2 = mip->bbo->common_data.sty2id;
				}
				return mip->bbo;
			}
			if ((mip->bee->common_data.padid != NULL
			&& *mip->bee->common_data.padid ==
					button->deviceid)
			|| (mip->bee->common_data.sty1id != NULL
			&& *mip->bee->common_data.sty1id ==
					button->deviceid)
			|| (mip->bee->common_data.sty2id != NULL
			&& *mip->bee->common_data.sty2id ==
					button->deviceid)) {
				if (mip->bee->default_program == NULL) {
					return NULL;
				}
				if (mip->bee->common_data.padid != NULL
				&& *mip->bee->common_data.padid ==
							button->deviceid) {
					is_pad = 1;
					id_pad = mip->bee->common_data.padid;
				} else if (mip->bee->common_data.sty1id != NULL
				&& *mip->bee->common_data.sty1id ==
							button->deviceid) {
					is_sty1 = 1;
				} else if (mip->bee->common_data.sty2id != NULL
				&& *mip->bee->common_data.sty2id ==
							button->deviceid) {
					is_sty2 = 1;
				}
				is_bee = 1;
				record_num = mip->bee->common_data.num_record;
				if (mip->bee->common_data.sty1id != NULL) {
					id_sty1 = mip->bee->common_data.sty1id;
				}
				if (mip->bee->common_data.sty2id != NULL) {
					id_sty2 = mip->bee->common_data.sty2id;
				}
				return mip->bee;
			}
			if ((mip->i3->common_data.padid != NULL
			&& *mip->i3->common_data.padid ==
					button->deviceid)
			|| (mip->i3->common_data.sty1id != NULL
			&& *mip->i3->common_data.sty1id ==
					button->deviceid)
			|| (mip->i3->common_data.sty2id != NULL
			&& *mip->i3->common_data.sty2id ==
					button->deviceid)) {
/* We need to check for the default program here as well, in case the
 configuration file never got parsed (like when a ConfigVersion mis-match
 occured). Then we'd have some registered hardware but no config info, which
 would segfault other functions later on. Invalidating the address with NULL: */
				if (mip->i3->default_program == NULL) {
					return NULL;
				}
				if (mip->i3->common_data.padid != NULL
				&& *mip->i3->common_data.padid ==
							button->deviceid) {
					is_pad = 1;
					id_pad = mip->i3->common_data.padid;
				} else if (mip->i3->common_data.sty1id != NULL
				&& *mip->i3->common_data.sty1id ==
							button->deviceid) {
					is_sty1 = 1;
				} else if (mip->i3->common_data.sty2id != NULL
				&& *mip->i3->common_data.sty2id ==
							button->deviceid) {
					is_sty2 = 1;
				}
				is_i3 = 1;
				record_num = mip->i3->common_data.num_record;
				if (mip->i3->common_data.sty1id != NULL) {
					id_sty1 = mip->i3->common_data.sty1id;
				}
				if (mip->i3->common_data.sty2id != NULL) {
					id_sty2 = mip->i3->common_data.sty2id;
				}
				return mip->i3;
			}
			if ((mip->i3s->common_data.padid != NULL
			&& *mip->i3s->common_data.padid ==
					button->deviceid)
			|| (mip->i3s->common_data.sty1id != NULL
			&& *mip->i3s->common_data.sty1id ==
					button->deviceid)
			|| (mip->i3s->common_data.sty2id != NULL
			&& *mip->i3s->common_data.sty2id ==
					button->deviceid)) {
				if (mip->i3s->default_program == NULL) {
					return NULL;
				}
				if (mip->i3s->common_data.padid != NULL
				&& *mip->i3s->common_data.padid ==
							button->deviceid) {
					is_pad = 1;
					id_pad = mip->i3s->common_data.padid;
				} else if (mip->i3s->common_data.sty1id != NULL
				&& *mip->i3s->common_data.sty1id ==
							button->deviceid) {
					is_sty1 = 1;
				} else if (mip->i3s->common_data.sty2id != NULL
				&& *mip->i3s->common_data.sty2id ==
							button->deviceid) {
					is_sty2 = 1;
				}
				is_i3s = 1;
				record_num = mip->i3s->common_data.num_record;
				if (mip->i3s->common_data.sty1id != NULL) {
					id_sty1 = mip->i3s->common_data.sty1id;
				}
				if (mip->i3s->common_data.sty2id != NULL) {
					id_sty2 = mip->i3s->common_data.sty2id;
				}
				return mip->i3s;
			}
			if ((mip->g4->common_data.padid != NULL
			&& *mip->g4->common_data.padid ==
					button->deviceid)
			|| (mip->g4->common_data.sty1id != NULL
			&& *mip->g4->common_data.sty1id ==
					button->deviceid)
			|| (mip->g4->common_data.sty2id != NULL
			&& *mip->g4->common_data.sty2id ==
					button->deviceid)) {
				if (mip->g4->default_program == NULL) {
					return NULL;
				}
				if (mip->g4->common_data.padid != NULL
				&& *mip->g4->common_data.padid ==
							button->deviceid) {
					is_pad = 1;
					id_pad = mip->g4->common_data.padid;
				} else if (mip->g4->common_data.sty1id != NULL
				&& *mip->g4->common_data.sty1id ==
							button->deviceid) {
					is_sty1 = 1;
				} else if (mip->g4->common_data.sty2id != NULL
				&& *mip->g4->common_data.sty2id ==
							button->deviceid) {
					is_sty2 = 1;
				}
				is_g4 = 1;
				record_num = mip->g4->common_data.num_record;
				if (mip->g4->common_data.sty1id != NULL) {
					id_sty1 = mip->g4->common_data.sty1id;
				}
				if (mip->g4->common_data.sty2id != NULL) {
					id_sty2 = mip->g4->common_data.sty2id;
				}
				return mip->g4;
			}
			if ((mip->g4b->common_data.padid != NULL
			&& *mip->g4b->common_data.padid ==
					button->deviceid)
			|| (mip->g4b->common_data.sty1id != NULL
			&& *mip->g4b->common_data.sty1id ==
					button->deviceid)
			|| (mip->g4b->common_data.sty2id != NULL
			&& *mip->g4b->common_data.sty2id ==
					button->deviceid)) {
				if (mip->g4b->default_program == NULL) {
					return NULL;
				}
				if (mip->g4b->common_data.padid != NULL
				&& *mip->g4b->common_data.padid ==
							button->deviceid) {
					is_pad = 1;
					id_pad = mip->g4b->common_data.padid;
				} else if (mip->g4b->common_data.sty1id != NULL
				&& *mip->g4b->common_data.sty1id ==
							button->deviceid) {
					is_sty1 = 1;
				} else if (mip->g4b->common_data.sty2id != NULL
				&& *mip->g4b->common_data.sty2id ==
							button->deviceid) {
					is_sty2 = 1;
				}
				is_g4b = 1;
				record_num = mip->g4b->common_data.num_record;
				if (mip->g4b->common_data.sty1id != NULL) {
					id_sty1 = mip->g4b->common_data.sty1id;
				}
				if (mip->g4b->common_data.sty2id != NULL) {
					id_sty2 = mip->g4b->common_data.sty2id;
				}
				return mip->g4b;
			}
			if((mip->nop->common_data.sty1id != NULL
			&& *mip->nop->common_data.sty1id ==
					button->deviceid)
			|| (mip->nop->common_data.sty2id != NULL
			&& *mip->nop->common_data.sty2id ==
					button->deviceid)) {
				if (mip->nop->default_program == NULL) {
					return NULL;
				}
				if (mip->nop->common_data.sty1id != NULL
				&& *mip->nop->common_data.sty1id ==
							button->deviceid) {
					is_sty1 = 1;
				} else if (mip->nop->common_data.sty2id != NULL
				&& *mip->nop->common_data.sty2id ==
							button->deviceid) {
					is_sty2 = 1;
				}
				is_nop = 1;
				record_num = mip->nop->common_data.num_record;
				return mip->nop;
			}
		}
	} else if (motion) {
		mip = model_list;
		for (i = 0; i < MAXPAD; i++, mip++) {
			if ((mip->ux->common_data.padid != NULL)
			&& (*mip->ux->common_data.padid
					== motion->deviceid)) {
				if (mip->ux->default_program == NULL) {
					return NULL;
				}
				is_pad = 1;
				is_ux = 1;
				id_pad = mip->ux->common_data.padid;
				record_num = mip->ux->common_data.num_record;
				if (mip->ux->common_data.sty1id != NULL) {
					id_sty1 = mip->ux->common_data.sty1id;
				}
				if (mip->ux->common_data.sty2id != NULL) {
					id_sty2 = mip->ux->common_data.sty2id;
				}
				return mip->ux;
			}
			if ((mip->ux2->common_data.padid != NULL)
			&& (*mip->ux2->common_data.padid
					== motion->deviceid)) {
				if (mip->ux2->default_program == NULL) {
					return NULL;
				}
				is_pad = 1;
				is_ux2 = 1;
				id_pad = mip->ux2->common_data.padid;
				record_num = mip->ux2->common_data.num_record;
				if (mip->ux2->common_data.sty1id != NULL) {
					id_sty1 = mip->ux2->common_data.sty1id;
				}
				if (mip->ux2->common_data.sty2id != NULL) {
					id_sty2 = mip->ux2->common_data.sty2id;
				}
				return mip->ux2;
			}
			if ((mip->bbo->common_data.padid != NULL)
			&& (*mip->bbo->common_data.padid
					== motion->deviceid)) {
				if (mip->bbo->default_program == NULL) {
					return NULL;
				}
				is_pad = 1;
				is_bbo = 1;
				id_pad = mip->bbo->common_data.padid;
				record_num = mip->bbo->common_data.num_record;
				if (mip->bbo->common_data.sty1id != NULL) {
					id_sty1 = mip->bbo->common_data.sty1id;
				}
				if (mip->bbo->common_data.sty2id != NULL) {
					id_sty2 = mip->bbo->common_data.sty2id;
				}
				return mip->bbo;
			}
			if ((mip->bee->common_data.padid != NULL)
			&& (*mip->bee->common_data.padid
					== motion->deviceid)) {
				if (mip->bee->default_program == NULL) {
					return NULL;
				}
				is_pad = 1;
				is_bee = 1;
				id_pad = mip->bee->common_data.padid;
				record_num = mip->bee->common_data.num_record;
				if (mip->bee->common_data.sty1id != NULL) {
					id_sty1 = mip->bee->common_data.sty1id;
				}
				if (mip->bee->common_data.sty2id != NULL) {
					id_sty2 = mip->bee->common_data.sty2id;
				}
				return mip->bee;
			}
			if ((mip->i3->common_data.padid != NULL)
			&& (*mip->i3->common_data.padid
					== motion->deviceid)) {
				if (mip->i3->default_program == NULL) {
					return NULL;
				}
				is_pad = 1;
				is_i3 = 1;
				id_pad = mip->i3->common_data.padid;
				record_num = mip->i3->common_data.num_record;
				if (mip->i3->common_data.sty1id != NULL) {
					id_sty1 = mip->i3->common_data.sty1id;
				}
				if (mip->i3->common_data.sty2id != NULL) {
					id_sty2 = mip->i3->common_data.sty2id;
				}
				return mip->i3;
			}
			if((mip->i3s->common_data.padid != NULL)
			&& (*mip->i3s->common_data.padid
					== motion->deviceid)) {
				if (mip->i3s->default_program == NULL) {
					return NULL;
				}
				is_pad = 1;
				is_i3s = 1;
				id_pad = mip->i3s->common_data.padid;
				record_num = mip->i3s->common_data.num_record;
				if (mip->i3s->common_data.sty1id != NULL) {
					id_sty1 = mip->i3s->common_data.sty1id;
				}
				if (mip->i3s->common_data.sty2id != NULL) {
					id_sty2 = mip->i3s->common_data.sty2id;
				}
				return mip->i3s;
			}
		}
	}
	return NULL;

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Endless launchpad loop as each event we've registered for comes in:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void use_events()
{
	XEvent Event;
	XDeviceButtonEvent* button = NULL;
	XDeviceMotionEvent* motion = NULL;

	void* record_address = NULL;
	void* base_address = NULL;

/*Set the stylus PressCurve histories to a default string, and a program name:*/
	snprintf(curvebuffer, CURVEMAX, "0 0 100 100");
	snprintf(namebuffer, MAXBUFFER, "dummy");

	while(1) {

/* The Event.type checks here are a bit excessive, but left in place to ensure
 complete control. It is so easy to register for other events and forget... */
		if ((is_sty1 || is_sty2)
		&& (Event.type == button_press_type
		|| Event.type == proximity_in_type)) {
			if (record_address != NULL) {
				base_address = record_address;
				record_address = find_focus(record_address,
									Event);
				if (name_change) {
					do_stylus(base_address, record_address);
				}
			}
		} else if ((button)
			&& (Event.type == button_press_type
			||  Event.type == button_release_type)) {
			if (record_address != NULL) {
				base_address = record_address;
				record_address = find_focus(record_address,
									Event);
				do_button(base_address, record_address, button,
									Event);
			}

		} else if (motion) {
			if (record_address != NULL) {
				base_address = record_address;
				record_address = find_focus(record_address,
									Event);
				do_motion(base_address, record_address, motion,
									Event);
			}
		}

		is_ux = 0;
		is_ux2 = 0;
		is_bbo = 0;
		is_bee = 0;
		is_i3 = 0;
		is_i3s = 0;
		is_g4 = 0;
		is_g4b = 0;
		is_nop = 0;
		is_pad = 0;
		is_sty1 = 0;
		is_sty2 = 0;

		id_pad = NULL;
		id_sty1 = NULL;
		id_sty2 = NULL;

		button = NULL;
		motion = NULL;

		XNextEvent(display, &Event);

		if (Event.type == proximity_in_type
		|| Event.type == button_press_type
		|| Event.type == button_release_type) {
			button = (XDeviceButtonEvent*) &Event;
		} else if (Event.type == motion_type) {
			motion = (XDeviceMotionEvent*) &Event;
		}

		if (button || motion) {
			record_address = match_id(button, motion);
		}
	}

}

/* End Code */

