/*
 expresskeys - Support ExpressKeys, Touch Strips, Scroll Wheel on Wacom tablets.

 Copyright (C) 2005-2007 - Mats Johannesson

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

#include <stdio.h> /* NULL, FILE, sprintf, fgets, fopen, fclose, ftell */
#include <stdlib.h> /* malloc, free, atoi */
#include <string.h> /* strlen, strstr, strchr, strspn, strcspn */
#include <ctype.h> /* isdigit */

#include "tablet.h"

/* Globals: */

/* Flag if having at _at least_ one valid configuration file: */
int ok_config;

/* 'Internal' Globals: */

/* Flags used to keep track of state and reverting actions: */
static int revert_config;
static int pgr_recname_malloced;
static int st1_prcurve_malloced;
static int st2_prcurve_malloced;

/* Externals: */

extern int be_verbose;
extern int reread_config;

extern const int bee;
extern const int i3;
extern const int i3s;
extern const int g4;
extern const int g4b;
extern const int nop;

extern const char* padstr;
extern const char* sty1str;
extern const char* sty2str;

extern const char* our_prog_name;
extern const char* configstring;
extern const char* configversion;

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Try to find a boolean 1/0 switch and write it to a structure place:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void transfer_boolean(int address[], char* write_buffer)
{
	int k = 0;
	char* end_pos = write_buffer + strlen(write_buffer);

	if (reread_config || revert_config) {
		address[0] = 0;
	}

	for (; write_buffer < end_pos; write_buffer++) {
		if (isdigit(write_buffer[0])) {
			write_buffer[1] = '\0';
			k = atoi(write_buffer);
			if (k == 0 || k == 1) {
				address[0] = k;
			} else {
				address[0] = 0;
				if (be_verbose) {
					fprintf(stderr, "Only boolean 1 or 0 \
allowed as switch <-- RESETTING T0 ZERO\n");
				}
			}
			break;
		}
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Try to find digits for a time value and write it to a structure place.
 We allow a max of 10 in full seconds, and 99 as a part second (10.99).
 "Be liberal with what you accept but conservative with what you reject"
 has been the guiding thought here. Hence the extreme ugliness of the code:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void transfer_timevalue(int address[], char* write_buffer)
{
	int i;
	int k = 0;
	int m = 0;
	int min_digit = 0;
	int max_sec = 10;
	int max_nsec = 99;
	int max_val = 3;
/* Pad means 'padding' here: */
	int max_pad = 10;
	char val_buf[max_val];
	char pad_buf[max_pad];
	char* end_pos = write_buffer + strlen(write_buffer);

	if (reread_config || revert_config) {
		address[0] = 0;
		address[1] = 0;
	}

	for (i = 0; m < max_val && write_buffer < end_pos;) {
		if (i) {
			i--;
			for (k = 0; k < i; k++) {
				val_buf[k] = write_buffer[k];
			}
			val_buf[k] = '\0';
			if ((k = atoi(val_buf)) >= min_digit) {
				if (m == 0) {
					if (k <= max_sec) {
						address[m] = k;
					} else {
						address[m] = 0;
					}
					m++;
				} else if (m == 1) {
					if (k <= max_nsec) {
						address[m] = k;
						sprintf(pad_buf, "%c",
								*write_buffer);
					} else {
						address[m] = 0;
					}
					m++;
				}
			}
			write_buffer = write_buffer+i;
		}
		for (i = 0; i < max_val && write_buffer < end_pos; i++) {
			if (!isdigit(write_buffer[i])) {
				write_buffer[i] = '\0';
				if ((k = atoi(write_buffer)) >= min_digit) {
					if (m == 0) {
						if (k <= max_sec) {
							address[m] = k;
						} else {
							address[m] = 0;
						}
						m++;
					} else if (m == 1) {
						if (k <= max_nsec) {
							address[m] = k;
							sprintf(pad_buf, "%c",
								*write_buffer);
						} else {
							address[m] = 0;
						}
						m++;
					}
				}
				write_buffer = write_buffer+i+1;
				i = 0;
				break;
			}
		}
	}

/* Pad the nsec value with correct number of zeros for the nanosleep(2) call: */

	if (atoi(pad_buf) == 0 || address[1] >= 10) {
		sprintf(pad_buf, "%i%s", *(address+1), "0000000");
	} else {
		sprintf(pad_buf, "%i%s", *(address+1), "00000000");
	}
	address[1] = atoi(pad_buf);

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Try to find digits for keycodes and write them to a structure place. We allow
 a max of four digits per keycode (to accommodate our fake ones, eg 1009).
 "Be liberal with what you accept but conservative with what you reject"
 has been the guiding thought here. Hence the ugliness of the code:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void transfer_keycode(int address[], char* write_buffer)
{
	int i;
	int k = 0;
	int m = 0;
/* Filter out anything below keycode 9 (ESC): */
	int min_keycode = 9;
/* Filter out anything above keycode 1023. Other
 'out-of-bounds' areas are checked in the loops: */
	int max_keycode = 1023;
	int max_buf = 5;
	char buf[max_buf];
	char* end_pos = write_buffer + strlen(write_buffer);

	if (reread_config || revert_config) {
		for (i = 0; i < MAXKEYS; i++) {
			address[i] = 0;
		}
	}

	for (i = 0; m < MAXKEYS && write_buffer < end_pos;) {
		if (i) {
			i--;
			for (k = 0; k < i; k++) {
				buf[k] = write_buffer[k];
			}
			buf[k] = '\0';
			k = atoi(buf);
			if ((k >= min_keycode && k <= max_keycode)
				&& (k < 256 || k > 263)
				&& (k < 512 || k > 519)
				&& (k < 768 || k > 775)) {
				address[m] = k;
				m++;
			}
			write_buffer = write_buffer+i;
		}
		for (i = 0; i < max_buf && write_buffer < end_pos; i++) {
			if (!isdigit(write_buffer[i])) {
				write_buffer[i] = '\0';
				k = atoi(write_buffer);
				if ((k >= min_keycode && k <= max_keycode)
					&& (k < 256 || k > 263)
					&& (k < 512 || k > 519)
					&& (k < 768 || k > 775)) {
					if (m < MAXKEYS) {
						address[m] = k;
						m++;
					}
				}
				write_buffer = write_buffer+i+1;
				i = 0;
				break;
			}
		}
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Allocate memory for strings (ProgramName, Stylus1PressCurve, Stylus2PressCurve)
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static char* field_malloc(FILE* errorfp, char* write_buffer)
{
	char* address = NULL;
	int len = 0;

	len = strlen(write_buffer)+1;
	if ((address = malloc(len)) == NULL) {
		exit_on_error(errorfp,
"\n%s ERROR: Memory allocation trouble at field_malloc()\n", our_prog_name, "");
	}
	sprintf(address, "%s", write_buffer);

	return address;

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Chop away leading and trailing text in a record field:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void prune_field(char* field_begin, char* write_buffer)
{
	int flen = 0;
	char* quote = NULL;

	flen = strcspn(field_begin, " \t");
	field_begin = (field_begin+flen);
	field_begin = (field_begin+(strspn(field_begin, " \t\"")));
	if ((quote = (strchr(field_begin, '\"'))) != NULL) {
		flen = (quote - field_begin);
	} else {
		flen = strcspn(field_begin, "\n#");
	}
	strncpy(write_buffer, field_begin, flen);
	strncpy(write_buffer + flen, "\0", 1);

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Try to find a field keyword in the supplied string. If found, hand it over
 for pruning and transfer of data to the correct structure place:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void handle_field(FILE* errorfp, char* read_buffer, void* address,
								const int model)
{
	struct bee_program* pbee;
	struct i3_program* pi3;
	struct i3s_program* pi3s;
	struct g4_program* pg4;
	struct g4b_program* pg4b;
	struct nop_program* pnop;
	struct common_data* cdp = NULL;
	struct touch_data* tdp = NULL;
	struct wheel_data* wdp = NULL;
	struct button_data* bdp = NULL;
	struct bee_configstrings* cbee;
	struct i3_configstrings* ci3;
	struct i3s_configstrings* ci3s;
	struct g4_configstrings* cg4;
	struct g4b_configstrings* cg4b;
	struct nop_configstrings* cnop;
	struct common_string* csp = NULL;
	struct touch_string* tsp = NULL;
	struct wheel_string* wsp = NULL;
	struct button_string* bsp = NULL;

	if (model == bee) {
		pbee = address;
		cdp = &pbee->common_data;
		tdp = &pbee->touch_data;
		bdp = &pbee->button_data;
		cbee = bee_configstrings;
		csp = &cbee->common_string;
		tsp = &cbee->touch_string;
		bsp = &cbee->button_string;
	} else if (model == i3) {
		pi3 = address;
		cdp = &pi3->common_data;
		tdp = &pi3->touch_data;
		bdp = &pi3->button_data;
		ci3 = i3_configstrings;
		csp = &ci3->common_string;
		tsp = &ci3->touch_string;
		bsp = &ci3->button_string;
	} else if (model == i3s) {
		pi3s = address;
		cdp = &pi3s->common_data;
		tdp = &pi3s->touch_data;
		bdp = &pi3s->button_data;
		ci3s = i3s_configstrings;
		csp = &ci3s->common_string;
		tsp = &ci3s->touch_string;
		bsp = &ci3s->button_string;
	} else if (model == g4) {
		pg4 = address;
		cdp = &pg4->common_data;
		wdp = &pg4->wheel_data;
		bdp = &pg4->button_data;
		cg4 = g4_configstrings;
		csp = &cg4->common_string;
		wsp = &cg4->wheel_string;
		bsp = &cg4->button_string;
	} else if (model == g4b) {
		pg4b = address;
		cdp = &pg4b->common_data;
		bdp = &pg4b->button_data;
		cg4b = g4b_configstrings;
		csp = &cg4b->common_string;
		bsp = &cg4b->button_string;
	} else if (model == nop) {
		pnop = address;
		cdp = &pnop->common_data;
		cnop = nop_configstrings;
		csp = &cnop->common_string;
	}

	char write_buffer[MAXBUFFER];

	char* comment = NULL;
	char* field = NULL;

/* ProgramName */
	if (((field = (strstr(read_buffer, csp->class_name))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
		prune_field(field, write_buffer);
		if ((cdp->class_name) && (reread_config)
			&& (!pgr_recname_malloced)) {
			free(cdp->class_name);
			cdp->class_name = NULL;
		}
		if (!pgr_recname_malloced) {
			cdp->class_name = field_malloc(errorfp, write_buffer);
			pgr_recname_malloced = 1;
			if (be_verbose) {
				fprintf(stderr, "PGR RECNAME = \"%s\"\n",
							cdp->class_name);
			}
		}
		return;
	}
/* Stylus1PressCurve */
	if (((field = (strstr(read_buffer, csp->stylus1_presscurve))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
		prune_field(field, write_buffer);
		if ((cdp->stylus1_presscurve) && (reread_config)
			&& (!st1_prcurve_malloced)) {
			free(cdp->stylus1_presscurve);
			cdp->stylus1_presscurve = NULL;
		}
		if (!st1_prcurve_malloced) {
			cdp->stylus1_presscurve = field_malloc(errorfp,
								write_buffer);
			st1_prcurve_malloced = 1;
			if (be_verbose && cdp->class_name != NULL) {
				fprintf(stderr, "ST1 PRCURVE = \"%s\"\n",
						cdp->stylus1_presscurve);
			}
		}
		return;
	}
/* Stylus2PressCurve */
	if (((field = (strstr(read_buffer, csp->stylus2_presscurve))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
		prune_field(field, write_buffer);
		if ((cdp->stylus2_presscurve) && (reread_config)
			&& (!st2_prcurve_malloced)) {
			free(cdp->stylus2_presscurve);
			cdp->stylus2_presscurve = NULL;
		}
		if (!st2_prcurve_malloced) {
			cdp->stylus2_presscurve = field_malloc(errorfp,
								write_buffer);
			st2_prcurve_malloced = 1;
			if (be_verbose && cdp->class_name != NULL) {
				fprintf(stderr, "ST2 PRCURVE = \"%s\"\n",
						cdp->stylus2_presscurve);
			}
		}
		return;
	}
/* DelayEachKeycode */
	if (model != nop) {
		if (((field = (strstr(read_buffer, csp->keycode_delay)))!= NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_timevalue(cdp->keycode_delay, write_buffer);
			return;
		}
	}
/* HandleTouchStrips TouchRepeatAfter DelayTouchRepeat
 LeftPadTouchUp LeftPadTouchDown RepeatLeftUp RepeatLeftDown */
	if (model == i3s || model == i3 || model == bee) {
		if (((field = (strstr(read_buffer, tsp->handle_touch))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_boolean(tdp->handle_touch, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, tsp->repeat_after)))!= NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_timevalue(tdp->repeat_after, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, tsp->repeat_delay)))!= NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_timevalue(tdp->repeat_delay, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, tsp->left_touch_up)))
								!= NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_keycode(tdp->left_touch_up, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, tsp->left_touch_down)))
								!= NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_keycode(tdp->left_touch_down, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, tsp->repeat_left_up)))
								!= NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_boolean(tdp->repeat_left_up, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, tsp->repeat_left_down)))
								!= NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_boolean(tdp->repeat_left_down, write_buffer);
			return;
		}
	}
/* HandleScrollWheel ScrollWheelUp ScrollWheelDown */
	if (model == g4) {
		if (((field = (strstr(read_buffer, wsp->handle_wheel))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_boolean(wdp->handle_wheel, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, wsp->scroll_wheel_up)))
								!= NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_keycode(wdp->scroll_wheel_up, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, wsp->scroll_wheel_down)))
								!= NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_keycode(wdp->scroll_wheel_down, write_buffer);
			return;
		}
	}
/* ButtonRepeatAfter DelayButtonRepeat
 LeftPadButton9/LeftButton
 RepeatButton9/RepeatLeft */
	if (model == i3s || model == i3 || model == g4 || model == g4b ||
							  model == bee) {
		if (((field = (strstr(read_buffer, bsp->repeat_after)))!= NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_timevalue(bdp->repeat_after, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, bsp->repeat_delay)))!= NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_timevalue(bdp->repeat_delay, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, bsp->button9))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_keycode(bdp->button9, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, bsp->repeat9))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_boolean(bdp->repeat9, write_buffer);
			return;
		}
	}
/* LeftPadButton10/RightButton RepeatButton10/RepeatRight */
	if (model == i3s || model == i3 || model == g4b || model == bee) {
		if (((field = (strstr(read_buffer, bsp->button10))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_keycode(bdp->button10, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, bsp->repeat10))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_boolean(bdp->repeat10, write_buffer);
			return;
		}
	}

/* LeftPadButton11 LeftPadButton12 RepeatButton11 RepeatButton12 */
	if (model == i3s || model == i3 || model == bee) {
		if (((field = (strstr(read_buffer, bsp->button11))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_keycode(bdp->button11, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, bsp->button12))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_keycode(bdp->button12, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, bsp->repeat11))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_boolean(bdp->repeat11, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, bsp->repeat12))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_boolean(bdp->repeat12, write_buffer);
			return;
		}
	}
/* RightPadButton13 RepeatButton13 */
	if (model == i3 || model == g4 || model == bee) {
		if (((field = (strstr(read_buffer, bsp->button13))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_keycode(bdp->button13, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, bsp->repeat13))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_boolean(bdp->repeat13, write_buffer);
			return;
		}
	}

/* RightPadTouchUp RightPadTouchDown RepeatRightUp RepeatRightDown
 RightPadButton13 RightPadButton14 RightPadButton15 RightPadButton16
 RepeatButton13 RepeatButton14 RepeatButton15 RepeatButton16 */
	if (model == i3 || model == bee) {
		if (((field = (strstr(read_buffer, tsp->right_touch_up)))
								!= NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_keycode(tdp->right_touch_up, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, tsp->right_touch_down)))
								!= NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_keycode(tdp->right_touch_down, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, tsp->repeat_right_up)))
								!= NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_boolean(tdp->repeat_right_up, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, tsp->repeat_right_down)))
								!= NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_boolean(tdp->repeat_right_down, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, bsp->button14))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_keycode(bdp->button14, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, bsp->button15))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_keycode(bdp->button15, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, bsp->button16))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_keycode(bdp->button16, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, bsp->repeat14))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_boolean(bdp->repeat14, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, bsp->repeat15))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_boolean(bdp->repeat15, write_buffer);
			return;
		}
		if (((field = (strstr(read_buffer, bsp->repeat16))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			transfer_boolean(bdp->repeat16, write_buffer);
			return;
		}

		if (model == bee) {
			if (((field = (strstr(read_buffer, bsp->button17))) != NULL)
				&& (((comment = (strchr(read_buffer, '#'))) == NULL)
							|| (field < comment))) {
				prune_field(field, write_buffer);
				transfer_keycode(bdp->button17, write_buffer);
				return;
			}
			if (((field = (strstr(read_buffer, bsp->button18))) != NULL)
				&& (((comment = (strchr(read_buffer, '#'))) == NULL)
							|| (field < comment))) {
				prune_field(field, write_buffer);
				transfer_keycode(bdp->button18, write_buffer);
				return;
			}
			if (((field = (strstr(read_buffer, bsp->repeat17))) != NULL)
				&& (((comment = (strchr(read_buffer, '#'))) == NULL)
							|| (field < comment))) {
				prune_field(field, write_buffer);
				transfer_boolean(bdp->repeat17, write_buffer);
				return;
			}
			if (((field = (strstr(read_buffer, bsp->repeat18))) != NULL)
				&& (((comment = (strchr(read_buffer, '#'))) == NULL)
							|| (field < comment))) {
				prune_field(field, write_buffer);
				transfer_boolean(bdp->repeat18, write_buffer);
				return;
			}
		}
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 This is primarily a twofold loop that tries to find all the records in a
 configuration file and hand over each non-empty line to functions further
 up in the food chain. The reason for it being so long, is that important
 error handling must be performed right at the end of a record, such as
 freeing memory allocated to strings. We allow, at a minimum, a configuration
 file to consist of 1 record seperator %% and 1 keyword ProgramName using
 the string "default". The program will not accomplish one iota with such
 a minimalistic file, but we pass it as valid:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void read_body(FILE* errorfp, void* address, const int model,
							const char* configfile)
{
	const char* def_rec = "default";

/* Reset status. Better safe than sorry: */
	revert_config = 0;
	pgr_recname_malloced = 0;
	st1_prcurve_malloced = 0;
	st2_prcurve_malloced = 0;

	struct bee_program* pbee = NULL;
	struct i3_program* pi3 = NULL;
	struct i3s_program* pi3s = NULL;
	struct g4_program* pg4 = NULL;
	struct g4b_program* pg4b = NULL;
	struct nop_program* pnop = NULL;
	struct bee_program* pbee_base = NULL;
	struct i3_program* pi3_base = NULL;
	struct i3s_program* pi3s_base = NULL;
	struct g4_program* pg4_base = NULL;
	struct g4b_program* pg4b_base = NULL;
	struct nop_program* pnop_base = NULL;

	if (model == bee) {
		pbee = address;
		pbee_base = pbee;
		if (reread_config) {
			pbee->default_program = NULL;
			pbee->common_data.num_record = 0;
		}
	} else if (model == i3) {
		pi3 = address;
		pi3_base = pi3;
		if (reread_config) {
			pi3->default_program = NULL;
			pi3->common_data.num_record = 0;
		}
	} else if (model == i3s) {
		pi3s = address;
		pi3s_base = pi3s;
		if (reread_config) {
			pi3s->default_program = NULL;
			pi3s->common_data.num_record = 0;
		}
	} else if (model == g4) {
		pg4 = address;
		pg4_base = pg4;
		if (reread_config) {
			pg4->default_program = NULL;
			pg4->common_data.num_record = 0;
		}
	} else if (model == g4b) {
		pg4b = address;
		pg4b_base = pg4b;
		if (reread_config) {
			pg4b->default_program = NULL;
			pg4b->common_data.num_record = 0;
		}
	} else if (model == nop) {
		pnop = address;
		pnop_base = pnop;
		if (reread_config) {
			pnop->default_program = NULL;
			pnop->common_data.num_record = 0;
		}
	}

	FILE* fp = NULL;

/* Endless loop protection variable: */
	long last_position = 0;

	int num_record = 0;
	int no_default = 0;
	int line_limit = 0;

	char read_buffer[MAXBUFFER];

	char* newline = NULL;
	char* comment = NULL;
	char* record = NULL;

	if ((fp = fopen(configfile, "r")) == NULL) {
		exit_on_error(errorfp,
				"\n%s ERROR: Can not open %s in read mode\n",
						our_prog_name, configfile);
	}

/* Find the very first record separator %% outside a comment # */

	while ((fgets(read_buffer, MAXBUFFER, fp) != NULL)
		&& (line_limit < MAXRECORDS * LINELIMIT)) {

		line_limit++;

		if ((newline = strchr(read_buffer, '\n')) == NULL) {
			fclose(fp);
			exit_on_error(errorfp,
	"\n%s ERROR: A line was too long to handle in the Config File %s\n",
						our_prog_name, configfile);
		}

		if (newline == read_buffer) {
			continue;
		}

		if (((record = (strstr(read_buffer, "%%"))) != NULL)
			&& (((comment = (strchr(read_buffer, '#'))) == NULL)
						|| (record < comment))) {
			break;
		}
	}

/* Begin record loop */

	while ((num_record < MAXRECORDS)
		&& (line_limit < MAXRECORDS * LINELIMIT)
		&& ((record = (strstr(read_buffer, "%%"))) != NULL)
		&& (((comment = (strchr(read_buffer, '#'))) == NULL)
					|| (record < comment))) {

/* Protect us from a really broken configuration file (endless loop): */
		if (last_position == ftell(fp)) {
			break;
		}
		last_position = ftell(fp);

/* Begin field loop*/

		while ((fgets(read_buffer, MAXBUFFER, fp) != NULL)
			&& (((record = (strstr(read_buffer, "%%"))) == NULL)
			|| (((comment = (strchr(read_buffer, '#'))) != NULL)
						&& (comment < record)))) {

			line_limit++;
			if (line_limit >= MAXRECORDS * LINELIMIT) {
				break;
			}

			if ((newline = strchr(read_buffer, '\n')) == NULL) {
				fclose(fp);
				exit_on_error(errorfp,
	"\n%s ERROR: A line was too long to handle in the Config File %s\n",
						our_prog_name, configfile);
			}

			if (newline == read_buffer) {
				continue;
			}

			if (pbee) {
				handle_field(errorfp, read_buffer, pbee, model);
			} else if (pi3) {
				handle_field(errorfp, read_buffer, pi3, model);
			} else if (pi3s) {
				handle_field(errorfp, read_buffer, pi3s, model);
			} else if (pg4) {
				handle_field(errorfp, read_buffer, pg4, model);
			} else if (pg4b) {
				handle_field(errorfp, read_buffer, pg4b, model);
			} else if (pnop) {
				handle_field(errorfp, read_buffer, pnop, model);
			}
		}

/* End field loop*/

		if (pbee) {
			if (pbee->common_data.class_name == NULL) {
				revert_config = 1;
				if (pbee->common_data.stylus1_presscurve
								!= NULL) {
				free(pbee->common_data.stylus1_presscurve);
				st1_prcurve_malloced = 0;
				pbee->common_data.stylus1_presscurve = NULL;
				}
				if (pbee->common_data.stylus2_presscurve
								!= NULL) {
				free(pbee->common_data.stylus2_presscurve);
				st2_prcurve_malloced = 0;
				pbee->common_data.stylus2_presscurve = NULL;
				}
			}
			if ((pbee->common_data.class_name != NULL)
				&& (strcmp(pbee->common_data.class_name,
							def_rec) == 0)) {
				pbee_base->default_program = pbee;
			}
			if (pbee->common_data.class_name != NULL) {
				pgr_recname_malloced = 0;
				revert_config = 0;
				num_record++;
				if (pbee->common_data.stylus1_presscurve
								!= NULL) {
					st1_prcurve_malloced = 0;
				}
				if (pbee->common_data.stylus2_presscurve
								!= NULL) {
					st2_prcurve_malloced = 0;
				}
				pbee++;
			}
		} else if (pi3) {
			if (pi3->common_data.class_name == NULL) {
				revert_config = 1;
				if (pi3->common_data.stylus1_presscurve
								!= NULL) {
				free(pi3->common_data.stylus1_presscurve);
				st1_prcurve_malloced = 0;
				pi3->common_data.stylus1_presscurve = NULL;
				}
				if (pi3->common_data.stylus2_presscurve
								!= NULL) {
				free(pi3->common_data.stylus2_presscurve);
				st2_prcurve_malloced = 0;
				pi3->common_data.stylus2_presscurve = NULL;
				}
			}
			if ((pi3->common_data.class_name != NULL)
				&& (strcmp(pi3->common_data.class_name,
							def_rec) == 0)) {
				pi3_base->default_program = pi3;
			}
			if (pi3->common_data.class_name != NULL) {
				pgr_recname_malloced = 0;
				revert_config = 0;
				num_record++;
				if (pi3->common_data.stylus1_presscurve
								!= NULL) {
					st1_prcurve_malloced = 0;
				}
				if (pi3->common_data.stylus2_presscurve
								!= NULL) {
					st2_prcurve_malloced = 0;
				}
				pi3++;
			}
		} else if (pi3s) {
			if (pi3s->common_data.class_name == NULL) {
				revert_config = 1;
				if (pi3s->common_data.stylus1_presscurve
								!= NULL) {
				free(pi3s->common_data.stylus1_presscurve);
				st1_prcurve_malloced = 0;
				pi3s->common_data.stylus1_presscurve = NULL;
				}
				if (pi3s->common_data.stylus2_presscurve
								!= NULL) {
				free(pi3s->common_data.stylus2_presscurve);
				st2_prcurve_malloced = 0;
				pi3s->common_data.stylus2_presscurve = NULL;
				}
			}
			if ((pi3s->common_data.class_name != NULL)
				&& (strcmp(pi3s->common_data.class_name,
							def_rec) == 0)) {
				pi3s_base->default_program = pi3s;
			}
			if (pi3s->common_data.class_name != NULL) {
				pgr_recname_malloced = 0;
				revert_config = 0;
				num_record++;
				if (pi3s->common_data.stylus1_presscurve
								!= NULL) {
					st1_prcurve_malloced = 0;
				}
				if (pi3s->common_data.stylus2_presscurve
								!= NULL) {
					st2_prcurve_malloced = 0;
				}
				pi3s++;
			}
		} else if (pg4) {
			if (pg4->common_data.class_name == NULL) {
				revert_config = 1;
				if (pg4->common_data.stylus1_presscurve
								!= NULL) {
				free(pg4->common_data.stylus1_presscurve);
				st1_prcurve_malloced = 0;
				pg4->common_data.stylus1_presscurve = NULL;
				}
				if (pg4->common_data.stylus2_presscurve
								!= NULL) {
				free(pg4->common_data.stylus2_presscurve);
				st2_prcurve_malloced = 0;
				pg4->common_data.stylus2_presscurve = NULL;
				}
			}
			if ((pg4->common_data.class_name != NULL)
				&& (strcmp(pg4->common_data.class_name,
							def_rec) == 0)) {
				pg4_base->default_program = pg4;
			}
			if (pg4->common_data.class_name != NULL) {
				pgr_recname_malloced = 0;
				revert_config = 0;
				num_record++;
				if (pg4->common_data.stylus1_presscurve
								!= NULL) {
					st1_prcurve_malloced = 0;
				}
				if (pg4->common_data.stylus2_presscurve
								!= NULL) {
					st2_prcurve_malloced = 0;
				}
				pg4++;
			}
		} else if (pg4b) {
			if (pg4b->common_data.class_name == NULL) {
				revert_config = 1;
				if (pg4b->common_data.stylus1_presscurve
								!= NULL) {
				free(pg4b->common_data.stylus1_presscurve);
				st1_prcurve_malloced = 0;
				pg4b->common_data.stylus1_presscurve = NULL;
				}
				if (pg4b->common_data.stylus2_presscurve
								!= NULL) {
				free(pg4b->common_data.stylus2_presscurve);
				st2_prcurve_malloced = 0;
				pg4b->common_data.stylus2_presscurve = NULL;
				}
			}
			if ((pg4b->common_data.class_name != NULL)
				&& (strcmp(pg4b->common_data.class_name,
							def_rec) == 0)) {
				pg4b_base->default_program = pg4b;
			}
			if (pg4b->common_data.class_name != NULL) {
				pgr_recname_malloced = 0;
				revert_config = 0;
				num_record++;
				if (pg4b->common_data.stylus1_presscurve
								!= NULL) {
					st1_prcurve_malloced = 0;
				}
				if (pg4b->common_data.stylus2_presscurve
								!= NULL) {
					st2_prcurve_malloced = 0;
				}
				pg4b++;
			}
		} else if (pnop) {
			if (pnop->common_data.class_name == NULL) {
				revert_config = 1;
				if (pnop->common_data.stylus1_presscurve
								!= NULL) {
				free(pnop->common_data.stylus1_presscurve);
				st1_prcurve_malloced = 0;
				pnop->common_data.stylus1_presscurve = NULL;
				}
				if (pnop->common_data.stylus2_presscurve
								!= NULL) {
				free(pnop->common_data.stylus2_presscurve);
				st2_prcurve_malloced = 0;
				pnop->common_data.stylus2_presscurve = NULL;
				}
			}
			if ((pnop->common_data.class_name != NULL)
				&& (strcmp(pnop->common_data.class_name,
							def_rec) == 0)) {
				pnop_base->default_program = pnop;
			}
			if (pnop->common_data.class_name != NULL) {
				pgr_recname_malloced = 0;
				revert_config = 0;
				num_record++;
				if (pnop->common_data.stylus1_presscurve
								!= NULL) {
					st1_prcurve_malloced = 0;
				}
				if (pnop->common_data.stylus2_presscurve
								!= NULL) {
					st2_prcurve_malloced = 0;
				}
				pnop++;
			}
		}
	}

/* End record loop */

	fclose(fp);

	if (model == bee) {
		pbee = address;
		if (pbee->default_program == NULL) {
			no_default = 1;
		}
		pbee->common_data.num_record = num_record;
	} else if (model == i3) {
		pi3 = address;
		if (pi3->default_program == NULL) {
			no_default = 1;
		}
		pi3->common_data.num_record = num_record;
	} else if (model == i3s) {
		pi3s = address;
		if (pi3s->default_program == NULL) {
			no_default = 1;
		}
		pi3s->common_data.num_record = num_record;
	} else if (model == g4) {
		pg4 = address;
		if (pg4->default_program == NULL) {
			no_default = 1;
		}
		pg4->common_data.num_record = num_record;
	} else if (model == g4b) {
		pg4b = address;
		if (pg4b->default_program == NULL) {
			no_default = 1;
		}
		pg4b->common_data.num_record = num_record;
	} else if (model == nop) {
		pnop = address;
		if (pnop->default_program == NULL) {
			no_default = 1;
		}
		pnop->common_data.num_record = num_record;
	}

	if (no_default) {
exit_on_error(errorfp, "\n%s ERROR: A program record named \"default\" must \
exist in\n%s\n", our_prog_name, configfile);
	}

	if (be_verbose) {
		fprintf(stderr, "PGR RECORDS = %i (of max %i)\n\n",
							num_record, MAXRECORDS);
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 If running in verbose mode we print out the names of all devices:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void print_device(FILE* statusfp, const char* padname,
				const char* sty1name, const char* sty2name)
{
	fprintf(statusfp, "%s  = %s\n", padstr, (padname?padname :"LACKING"));
	fprintf(statusfp, "%s = %s\n", sty1str, (sty1name?sty1name :"LACKING"));
	fprintf(statusfp, "%s = %s\n", sty2str, (sty2name?sty2name :"LACKING"));

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Try to find a digit representing the ConfigVersion in a configuration file:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static int read_header(FILE* errorfp, char* write_buffer,const char* configfile)
{
	FILE* fp = NULL;

	char read_buffer[MAXBUFFER];

	int k = 0;
	int line_limit = 0;

	char* end_pos = NULL;
	char* newline = NULL;
	char* comment = NULL;
	char* record = NULL;
	char* field = NULL;

	if ((fp = fopen(configfile, "r")) == NULL) {
		exit_on_error(errorfp,
				"\n%s ERROR: Can not open %s in read mode\n",
						our_prog_name, configfile);
	}

	while ((fgets(read_buffer, MAXBUFFER, fp) != NULL)
		&& (!k)
		&& (line_limit < MAXRECORDS * LINELIMIT)
		&& (((record = (strstr(read_buffer, "%%"))) == NULL)
		|| (((comment = (strchr(read_buffer, '#'))) != NULL)
					&& (comment < record)))) {

		line_limit++;

		if ((newline = strchr(read_buffer, '\n')) == NULL) {
			fclose(fp);
			exit_on_error(errorfp,
	"\n%s ERROR: A line was too long to handle in the Config File %s\n",
						our_prog_name, configfile);
		}

		if (newline == read_buffer) {
			continue;
		}

		if (((field = (strstr(read_buffer, configstring)))
								!= NULL)
			&& (((comment = (strchr(read_buffer, '#')))
								== NULL)
						|| (field < comment))) {
			prune_field(field, write_buffer);
			end_pos = write_buffer + strlen(write_buffer);
			for (; write_buffer < end_pos; write_buffer++) {
				if (isdigit(write_buffer[0])) {
					write_buffer[1] = '\0';
					k = atoi(write_buffer);
					break;
				}
			}
		}
	}

	fclose(fp);

	if (k && be_verbose) {
		fprintf(stderr, "%s-user = %s\n", configstring, write_buffer);
		fprintf(stderr, "%s-ours = %s\n", configstring, configversion);
	}

	if (!k) {
		fprintf(stderr, "%s-user = %i\n", configstring, k);
		fprintf(stderr, "%s-ours = %s\n", configstring, configversion);
		fprintf(stderr,"ConfigVersion not present. Skipping file:\n");
		fprintf(stderr, "%s\n\n", configfile);
		return 0;
	}

	if (k != atoi(configversion)) {
	fprintf(stderr,"ConfigVersion %i is unsupported. Skipping file:\n",k);
		fprintf(stderr, "%s\n\n", configfile);
		return 0;
	}

	ok_config = 1;
	return k;

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Hand over all the associated configuration files for further processing:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void read_config(FILE* errorfp)
{
	int i, k;
	const char* our_cnffile = "OUR CNFFILE =";
	char write_buffer[MAXBUFFER];

	ok_config = 0;

	struct model_index* mip;
	mip = model_list;

	for (i = 0; i < MAXPAD; i++, mip++) {
		if (mip->bee->common_data.configfile) {
			if (be_verbose) {
				fprintf(stderr, "%s %s\n", our_cnffile,
				mip->bee->common_data.configfile);
			}

			if ((k = read_header(errorfp, write_buffer,
					mip->bee->common_data.configfile))) {
				mip->bee->common_data.userconfigversion = k;
				if (be_verbose) {
					print_device(stderr,
						mip->bee->common_data.padname,
						mip->bee->common_data.sty1name,
						mip->bee->common_data.sty2name);
				}
				read_body(errorfp, mip->bee, bee,
					mip->bee->common_data.configfile);
			}
		}
		if (mip->i3->common_data.configfile) {
			if (be_verbose) {
				fprintf(stderr, "%s %s\n", our_cnffile,
				mip->i3->common_data.configfile);
			}

			if ((k = read_header(errorfp, write_buffer,
					mip->i3->common_data.configfile))) {
				mip->i3->common_data.userconfigversion = k;
				if (be_verbose) {
					print_device(stderr,
						mip->i3->common_data.padname,
						mip->i3->common_data.sty1name,
						mip->i3->common_data.sty2name);
				}
				read_body(errorfp, mip->i3, i3,
					mip->i3->common_data.configfile);
			}
		}
		if (mip->i3s->common_data.configfile) {
			if (be_verbose) {
				fprintf(stderr, "%s %s\n", our_cnffile,
				mip->i3s->common_data.configfile);
			}
			if ((k = read_header(errorfp, write_buffer,
					mip->i3s->common_data.configfile))) {
				mip->i3s->common_data.userconfigversion = k;
				if (be_verbose) {
					print_device(stderr,
						mip->i3s->common_data.padname,
						mip->i3s->common_data.sty1name,
						mip->i3s->common_data.sty2name);
				}
				read_body(errorfp, mip->i3s, i3s,
					mip->i3s->common_data.configfile);
			}
		}
		if (mip->g4->common_data.configfile) {
			if (be_verbose) {
				fprintf(stderr, "%s %s\n", our_cnffile,
				mip->g4->common_data.configfile);
			}
			if ((k = read_header(errorfp, write_buffer,
					mip->g4->common_data.configfile))) {
				mip->g4->common_data.userconfigversion = k;
				if (be_verbose) {
					print_device(stderr,
						mip->g4->common_data.padname,
						mip->g4->common_data.sty1name,
						mip->g4->common_data.sty2name);
				}
				read_body(errorfp, mip->g4, g4,
					mip->g4->common_data.configfile);
			}
		}
		if (mip->g4b->common_data.configfile) {
			if (be_verbose) {
				fprintf(stderr, "%s %s\n", our_cnffile,
				mip->g4b->common_data.configfile);
			}
			if ((k = read_header(errorfp, write_buffer,
					mip->g4b->common_data.configfile))) {
				mip->g4b->common_data.userconfigversion = k;
				if (be_verbose) {
					print_device(stderr,
						mip->g4b->common_data.padname,
						mip->g4b->common_data.sty1name,
						mip->g4b->common_data.sty2name);
				}
				read_body(errorfp, mip->g4b, g4b,
					mip->g4b->common_data.configfile);
			}
		}
		if (mip->nop->common_data.configfile) {
			if (be_verbose) {
				fprintf(stderr, "%s %s\n", our_cnffile,
				mip->nop->common_data.configfile);
			}
			if ((k = read_header(errorfp, write_buffer,
					mip->nop->common_data.configfile))) {
				mip->nop->common_data.userconfigversion = k;
				if (be_verbose) {
					print_device(stderr, NULL,
						mip->nop->common_data.sty1name,
						mip->nop->common_data.sty2name);
				}
				read_body(errorfp, mip->nop, nop,
					mip->nop->common_data.configfile);
			}
		}
	}

}

/* End Code */

