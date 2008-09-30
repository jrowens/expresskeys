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

#include <stdio.h> /*NULL, FILE, fopen, rewind, fgetc, fprintf, ferror, fclose*/
#include "tablet.h"

/* Globals: */

/* Obligatory keyword in the config file: */
const char* configstring = "ConfigVersion";
/* Our version. Remember to change it if necessary: */
const char* configversion = "4";

/* Externals: */

extern const int bee;
extern const int i3;
extern const int i3s;
extern const int g4;
extern const int g4b;
extern const int nop;

extern const int bee_num_list;
extern const int i3_num_list;
extern const int i3s_num_list;
extern const int g4_num_list;
extern const int g4b_num_list;
extern const int nop_num_list;

extern const char* our_prog_name;

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Write a set of records tuned for a Cintiq 20wsx/'bee' tablet:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void write_bee(FILE* fp)
{
	int i;

	const char* new_record =
"%%			# <---  ******** BEGIN NEW PROGRAM RECORD ********\n\n";
	const char* name_tail = "# Name must be within double quotes \"\"";
	const char* cur_tail = "# PressCurve must be within double quotes \"\"";
	const char* sec_tail = "# Seconds (Max 10 - Min 0.01 - Or no delay)";
	const char* tch_onoff = "# Switch 1/0 (Enable/Disable Touch Strips)";
	const char* key_tail = "# Keycodes (Max number of keys to send is";
	const char* tht_tail = "# Switch 1/0 (Finger held at top repeat keys)";
	const char* thb_tail="# Switch 1/0 (Finger held at bottom repeat keys)";
	const char* but_tail="# Switch 1/0 (Press and hold button repeat keys)";

	struct bee_program* ip;
	ip = bee_internal_list;
	struct bee_configstrings* sp;
	sp = bee_configstrings;

	for (i = 0; i < bee_num_list; i++, ip++) {
		fprintf(fp, "%s", new_record);

		fprintf(fp, "%s		\"%s\" %s\n\n",
		sp->common_string.class_name, ip->common_data.class_name,
								name_tail);

		fprintf(fp, "%s \"%s\" %s\n",
		sp->common_string.stylus1_presscurve,
				ip->common_data.stylus1_presscurve, cur_tail);
		fprintf(fp, "%s \"%s\" %s\n\n",
		sp->common_string.stylus2_presscurve,
				ip->common_data.stylus2_presscurve, cur_tail);

		fprintf(fp, "%s	%i.%i	%s\n\n",
		sp->common_string.keycode_delay, *ip->common_data.keycode_delay,
				*(ip->common_data.keycode_delay+1), sec_tail);

		fprintf(fp, "%s	%i.%i	%s\n",
		sp->button_string.repeat_after, *ip->button_data.repeat_after,
				*(ip->button_data.repeat_after+1), sec_tail);
		fprintf(fp, "%s	%i.%i	%s\n\n",
		sp->button_string.repeat_delay, *ip->button_data.repeat_delay,
				*(ip->button_data.repeat_delay+1), sec_tail);

		fprintf(fp, "%s	%i.%i	%s\n",
		sp->touch_string.repeat_after, *ip->touch_data.repeat_after,
				*(ip->touch_data.repeat_after+1), sec_tail);
		fprintf(fp, "%s	%i.%i	%s\n\n",
		sp->touch_string.repeat_delay, *ip->touch_data.repeat_delay,
				*(ip->touch_data.repeat_delay+1), sec_tail);

		fprintf(fp, "%s	%i	%s\n\n",
		sp->touch_string.handle_touch,
				*ip->touch_data.handle_touch, tch_onoff);

		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->touch_string.left_touch_up,
			*ip->touch_data.left_touch_up,
			*(ip->touch_data.left_touch_up+1), key_tail, MAXKEYS);
		fprintf(fp, "%s	%i %i	%s %i)\n\n",
		sp->touch_string.left_touch_down,
			*ip->touch_data.left_touch_down,
			*(ip->touch_data.left_touch_down+1), key_tail, MAXKEYS);

		fprintf(fp, "%s		%i	%s\n",
		sp->touch_string.repeat_left_up,
				*ip->touch_data.repeat_left_up, tht_tail);
		fprintf(fp, "%s		%i	%s\n\n",
		sp->touch_string.repeat_left_down,
				*ip->touch_data.repeat_left_down, thb_tail);

		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->touch_string.right_touch_up,
			*ip->touch_data.right_touch_up,
			*(ip->touch_data.right_touch_up+1), key_tail, MAXKEYS);
		fprintf(fp, "%s	%i %i	%s %i)\n\n",
		sp->touch_string.right_touch_down,
			*ip->touch_data.right_touch_down,
			*(ip->touch_data.right_touch_down+1), key_tail,MAXKEYS);

		fprintf(fp, "%s		%i	%s\n",
		sp->touch_string.repeat_right_up,
				*ip->touch_data.repeat_right_up, tht_tail);
		fprintf(fp, "%s		%i	%s\n\n",
		sp->touch_string.repeat_right_down,
				*ip->touch_data.repeat_right_down, thb_tail);

		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->button_string.button9, *ip->button_data.button9,
				*(ip->button_data.button9+1), key_tail,MAXKEYS);
		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->button_string.button10, *ip->button_data.button10,
				*(ip->button_data.button10+1),key_tail,MAXKEYS);
		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->button_string.button11, *ip->button_data.button11,
				*(ip->button_data.button11+1),key_tail,MAXKEYS);
		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->button_string.button12, *ip->button_data.button12,
				*(ip->button_data.button12+1),key_tail,MAXKEYS);
		/* button 17 is on the left side */
		fprintf(fp, "%s		%i %i	%s %i)\n\n",
		sp->button_string.button17, *ip->button_data.button17,
				*(ip->button_data.button17+1),key_tail,MAXKEYS);

		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat9, *ip->button_data.repeat9, but_tail);
		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat10, *ip->button_data.repeat10,but_tail);
		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat11, *ip->button_data.repeat11,but_tail);
		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat12, *ip->button_data.repeat12,but_tail);
		fprintf(fp, "%s		%i	%s\n\n",
		sp->button_string.repeat17, *ip->button_data.repeat17,but_tail);

		fprintf(fp, "%s	%i %i	%s %i)\n",
		sp->button_string.button13, *ip->button_data.button13,
				*(ip->button_data.button13+1),key_tail,MAXKEYS);
		fprintf(fp, "%s	%i %i	%s %i)\n",
		sp->button_string.button14, *ip->button_data.button14,
				*(ip->button_data.button14+1),key_tail,MAXKEYS);
		fprintf(fp, "%s	%i %i	%s %i)\n",
		sp->button_string.button15, *ip->button_data.button15,
				*(ip->button_data.button15+1),key_tail,MAXKEYS);
		fprintf(fp, "%s	%i %i	%s %i)\n",
		sp->button_string.button16, *ip->button_data.button16,
				*(ip->button_data.button16+1),key_tail,MAXKEYS);
		fprintf(fp, "%s	%i %i	%s %i)\n\n",
		sp->button_string.button18, *ip->button_data.button18,
				*(ip->button_data.button18+1),key_tail,MAXKEYS);

		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat13, *ip->button_data.repeat13,but_tail);
		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat14, *ip->button_data.repeat14,but_tail);
		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat15, *ip->button_data.repeat15,but_tail);
		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat16, *ip->button_data.repeat16,but_tail);
		fprintf(fp, "%s		%i	%s\n\n",
		sp->button_string.repeat18, *ip->button_data.repeat18,but_tail);
	}
}




/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Write a set of records tuned for a Intuos3/Cintiq tablet:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void write_i3(FILE* fp)
{
	int i;

	const char* new_record =
"%%			# <---  ******** BEGIN NEW PROGRAM RECORD ********\n\n";
	const char* name_tail = "# Name must be within double quotes \"\"";
	const char* cur_tail = "# PressCurve must be within double quotes \"\"";
	const char* sec_tail = "# Seconds (Max 10 - Min 0.01 - Or no delay)";
	const char* tch_onoff = "# Switch 1/0 (Enable/Disable Touch Strips)";
	const char* key_tail = "# Keycodes (Max number of keys to send is";
	const char* tht_tail = "# Switch 1/0 (Finger held at top repeat keys)";
	const char* thb_tail="# Switch 1/0 (Finger held at bottom repeat keys)";
	const char* but_tail="# Switch 1/0 (Press and hold button repeat keys)";

	struct i3_program* ip;
	ip = i3_internal_list;
	struct i3_configstrings* sp;
	sp = i3_configstrings;

	for (i = 0; i < i3_num_list; i++, ip++) {

		fprintf(fp, "%s", new_record);

		fprintf(fp, "%s		\"%s\" %s\n\n",
		sp->common_string.class_name, ip->common_data.class_name,
								name_tail);

		fprintf(fp, "%s \"%s\" %s\n",
		sp->common_string.stylus1_presscurve,
				ip->common_data.stylus1_presscurve, cur_tail);
		fprintf(fp, "%s \"%s\" %s\n\n",
		sp->common_string.stylus2_presscurve,
				ip->common_data.stylus2_presscurve, cur_tail);

		fprintf(fp, "%s	%i.%i	%s\n\n",
		sp->common_string.keycode_delay, *ip->common_data.keycode_delay,
				*(ip->common_data.keycode_delay+1), sec_tail);

		fprintf(fp, "%s	%i.%i	%s\n",
		sp->button_string.repeat_after, *ip->button_data.repeat_after,
				*(ip->button_data.repeat_after+1), sec_tail);
		fprintf(fp, "%s	%i.%i	%s\n\n",
		sp->button_string.repeat_delay, *ip->button_data.repeat_delay,
				*(ip->button_data.repeat_delay+1), sec_tail);

		fprintf(fp, "%s	%i.%i	%s\n",
		sp->touch_string.repeat_after, *ip->touch_data.repeat_after,
				*(ip->touch_data.repeat_after+1), sec_tail);
		fprintf(fp, "%s	%i.%i	%s\n\n",
		sp->touch_string.repeat_delay, *ip->touch_data.repeat_delay,
				*(ip->touch_data.repeat_delay+1), sec_tail);

		fprintf(fp, "%s	%i	%s\n\n",
		sp->touch_string.handle_touch,
				*ip->touch_data.handle_touch, tch_onoff);

		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->touch_string.left_touch_up,
			*ip->touch_data.left_touch_up,
			*(ip->touch_data.left_touch_up+1), key_tail, MAXKEYS);
		fprintf(fp, "%s	%i %i	%s %i)\n\n",
		sp->touch_string.left_touch_down,
			*ip->touch_data.left_touch_down,
			*(ip->touch_data.left_touch_down+1), key_tail, MAXKEYS);

		fprintf(fp, "%s		%i	%s\n",
		sp->touch_string.repeat_left_up,
				*ip->touch_data.repeat_left_up, tht_tail);
		fprintf(fp, "%s		%i	%s\n\n",
		sp->touch_string.repeat_left_down,
				*ip->touch_data.repeat_left_down, thb_tail);

		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->touch_string.right_touch_up,
			*ip->touch_data.right_touch_up,
			*(ip->touch_data.right_touch_up+1), key_tail, MAXKEYS);
		fprintf(fp, "%s	%i %i	%s %i)\n\n",
		sp->touch_string.right_touch_down,
			*ip->touch_data.right_touch_down,
			*(ip->touch_data.right_touch_down+1), key_tail,MAXKEYS);

		fprintf(fp, "%s		%i	%s\n",
		sp->touch_string.repeat_right_up,
				*ip->touch_data.repeat_right_up, tht_tail);
		fprintf(fp, "%s		%i	%s\n\n",
		sp->touch_string.repeat_right_down,
				*ip->touch_data.repeat_right_down, thb_tail);

		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->button_string.button9, *ip->button_data.button9,
				*(ip->button_data.button9+1), key_tail,MAXKEYS);
		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->button_string.button10, *ip->button_data.button10,
				*(ip->button_data.button10+1),key_tail,MAXKEYS);
		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->button_string.button11, *ip->button_data.button11,
				*(ip->button_data.button11+1),key_tail,MAXKEYS);
		fprintf(fp, "%s		%i %i	%s %i)\n\n",
		sp->button_string.button12, *ip->button_data.button12,
				*(ip->button_data.button12+1),key_tail,MAXKEYS);

		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat9, *ip->button_data.repeat9, but_tail);
		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat10, *ip->button_data.repeat10,but_tail);
		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat11, *ip->button_data.repeat11,but_tail);
		fprintf(fp, "%s		%i	%s\n\n",
		sp->button_string.repeat12, *ip->button_data.repeat12,but_tail);

		fprintf(fp, "%s	%i %i	%s %i)\n",
		sp->button_string.button13, *ip->button_data.button13,
				*(ip->button_data.button13+1),key_tail,MAXKEYS);
		fprintf(fp, "%s	%i %i	%s %i)\n",
		sp->button_string.button14, *ip->button_data.button14,
				*(ip->button_data.button14+1),key_tail,MAXKEYS);
		fprintf(fp, "%s	%i %i	%s %i)\n",
		sp->button_string.button15, *ip->button_data.button15,
				*(ip->button_data.button15+1),key_tail,MAXKEYS);
		fprintf(fp, "%s	%i %i	%s %i)\n\n",
		sp->button_string.button16, *ip->button_data.button16,
				*(ip->button_data.button16+1),key_tail,MAXKEYS);

		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat13, *ip->button_data.repeat13,but_tail);
		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat14, *ip->button_data.repeat14,but_tail);
		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat15, *ip->button_data.repeat15,but_tail);
		fprintf(fp, "%s		%i	%s\n\n",
		sp->button_string.repeat16, *ip->button_data.repeat16,but_tail);

	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Write a set of records tuned for a Intuos3 'small' tablet:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void write_i3s(FILE* fp)
{
	int i;

	const char* new_record =
"%%			# <---  ******** BEGIN NEW PROGRAM RECORD ********\n\n";
	const char* name_tail = "# Name must be within double quotes \"\"";
	const char* cur_tail = "# PressCurve must be within double quotes \"\"";
	const char* sec_tail = "# Seconds (Max 10 - Min 0.01 - Or no delay)";
	const char* tch_onoff = "# Switch 1/0 (Enable/Disable Touch Strips)";
	const char* key_tail = "# Keycodes (Max number of keys to send is";
	const char* tht_tail = "# Switch 1/0 (Finger held at top repeat keys)";
	const char* thb_tail="# Switch 1/0 (Finger held at bottom repeat keys)";
	const char* but_tail="# Switch 1/0 (Press and hold button repeat keys)";

	struct i3s_program* ip;
	ip = i3s_internal_list;
	struct i3s_configstrings* sp;
	sp = i3s_configstrings;

	for (i = 0; i < i3s_num_list; i++, ip++) {

		fprintf(fp, "%s", new_record);

		fprintf(fp, "%s		\"%s\" %s\n\n",
		sp->common_string.class_name, ip->common_data.class_name,
								name_tail);

		fprintf(fp, "%s \"%s\" %s\n",
		sp->common_string.stylus1_presscurve,
				ip->common_data.stylus1_presscurve, cur_tail);
		fprintf(fp, "%s \"%s\" %s\n\n",
		sp->common_string.stylus2_presscurve,
				ip->common_data.stylus2_presscurve, cur_tail);

		fprintf(fp, "%s	%i.%i	%s\n\n",
		sp->common_string.keycode_delay, *ip->common_data.keycode_delay,
				*(ip->common_data.keycode_delay+1), sec_tail);

		fprintf(fp, "%s	%i.%i	%s\n",
		sp->button_string.repeat_after, *ip->button_data.repeat_after,
				*(ip->button_data.repeat_after+1), sec_tail);
		fprintf(fp, "%s	%i.%i	%s\n\n",
		sp->button_string.repeat_delay, *ip->button_data.repeat_delay,
				*(ip->button_data.repeat_delay+1), sec_tail);


		fprintf(fp, "%s	%i.%i	%s\n",
		sp->touch_string.repeat_after, *ip->touch_data.repeat_after,
				*(ip->touch_data.repeat_after+1), sec_tail);
		fprintf(fp, "%s	%i.%i	%s\n\n",
		sp->touch_string.repeat_delay, *ip->touch_data.repeat_delay,
				*(ip->touch_data.repeat_delay+1), sec_tail);

		fprintf(fp, "%s	%i	%s\n\n",
		sp->touch_string.handle_touch,
				*ip->touch_data.handle_touch, tch_onoff);

		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->touch_string.left_touch_up,
			*ip->touch_data.left_touch_up,
			*(ip->touch_data.left_touch_up+1), key_tail, MAXKEYS);
		fprintf(fp, "%s	%i %i	%s %i)\n\n",
		sp->touch_string.left_touch_down,
			*ip->touch_data.left_touch_down,
			*(ip->touch_data.left_touch_down+1), key_tail, MAXKEYS);

		fprintf(fp, "%s		%i	%s\n",
		sp->touch_string.repeat_left_up,
				*ip->touch_data.repeat_left_up, tht_tail);
		fprintf(fp, "%s		%i	%s\n\n",
		sp->touch_string.repeat_left_down,
				*ip->touch_data.repeat_left_down, thb_tail);

		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->button_string.button9, *ip->button_data.button9,
				*(ip->button_data.button9+1), key_tail,MAXKEYS);
		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->button_string.button10, *ip->button_data.button10,
				*(ip->button_data.button10+1),key_tail,MAXKEYS);
		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->button_string.button11, *ip->button_data.button11,
				*(ip->button_data.button11+1),key_tail,MAXKEYS);
		fprintf(fp, "%s		%i %i	%s %i)\n\n",
		sp->button_string.button12, *ip->button_data.button12,
				*(ip->button_data.button12+1),key_tail,MAXKEYS);

		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat9, *ip->button_data.repeat9, but_tail);
		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat10, *ip->button_data.repeat10,but_tail);
		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat11, *ip->button_data.repeat11,but_tail);
		fprintf(fp, "%s		%i	%s\n\n",
		sp->button_string.repeat12, *ip->button_data.repeat12,but_tail);

	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Write a set of records tuned for a Graphire4 tablet:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void write_g4(FILE* fp)
{
	int i;

	const char* new_record =
"%%			# <---  ******** BEGIN NEW PROGRAM RECORD ********\n\n";
	const char* name_tail = "# Name must be within double quotes \"\"";
	const char* cur_tail = "# PressCurve must be within double quotes \"\"";
	const char* sec_tail = "# Seconds (Max 10 - Min 0.01 - Or no delay)";
	const char* whl_onoff = "# Switch 1/0 (Enable/Disable Scroll Wheel)";
	const char* key_tail = "# Keycodes (Max number of keys to send is";
	const char* but_tail="# Switch 1/0 (Press and hold button repeat keys)";

	struct g4_program* ip;
	ip = g4_internal_list;
	struct g4_configstrings* sp;
	sp = g4_configstrings;

	for (i = 0; i < g4_num_list; i++, ip++) {

		fprintf(fp, "%s", new_record);

		fprintf(fp, "%s		\"%s\" %s\n\n",
		sp->common_string.class_name, ip->common_data.class_name,
								name_tail);

		fprintf(fp, "%s \"%s\" %s\n",
		sp->common_string.stylus1_presscurve,
				ip->common_data.stylus1_presscurve, cur_tail);
		fprintf(fp, "%s \"%s\" %s\n\n",
		sp->common_string.stylus2_presscurve,
				ip->common_data.stylus2_presscurve, cur_tail);

		fprintf(fp, "%s	%i.%i	%s\n\n",
		sp->common_string.keycode_delay, *ip->common_data.keycode_delay,
				*(ip->common_data.keycode_delay+1), sec_tail);

		fprintf(fp, "%s	%i.%i	%s\n",
		sp->button_string.repeat_after, *ip->button_data.repeat_after,
				*(ip->button_data.repeat_after+1), sec_tail);
		fprintf(fp, "%s	%i.%i	%s\n\n",
		sp->button_string.repeat_delay, *ip->button_data.repeat_delay,
				*(ip->button_data.repeat_delay+1), sec_tail);

		fprintf(fp, "%s	%i	%s\n\n",
		sp->wheel_string.handle_wheel,
				*ip->wheel_data.handle_wheel, whl_onoff);

		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->wheel_string.scroll_wheel_up,
			*ip->wheel_data.scroll_wheel_up,
			*(ip->wheel_data.scroll_wheel_up+1), key_tail, MAXKEYS);
		fprintf(fp, "%s		%i %i	%s %i)\n\n",
		sp->wheel_string.scroll_wheel_down,
			*ip->wheel_data.scroll_wheel_down,
			*(ip->wheel_data.scroll_wheel_down+1),key_tail,MAXKEYS);

		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->button_string.button9, *ip->button_data.button9,
				*(ip->button_data.button9+1), key_tail,MAXKEYS);
		fprintf(fp, "%s		%i %i	%s %i)\n\n",
		sp->button_string.button13, *ip->button_data.button13,
				*(ip->button_data.button13+1),key_tail,MAXKEYS);

		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat9, *ip->button_data.repeat9, but_tail);
		fprintf(fp, "%s		%i	%s\n\n",
		sp->button_string.repeat13, *ip->button_data.repeat13,but_tail);

	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Write a set of records tuned for a Graphire4 BlueTooth tablet:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void write_g4b(FILE* fp)
{
	int i;

	const char* new_record =
"%%			# <---  ******** BEGIN NEW PROGRAM RECORD ********\n\n";
	const char* name_tail = "# Name must be within double quotes \"\"";
	const char* cur_tail = "# PressCurve must be within double quotes \"\"";
	const char* sec_tail = "# Seconds (Max 10 - Min 0.01 - Or no delay)";
	const char* key_tail = "# Keycodes (Max number of keys to send is";
	const char* but_tail="# Switch 1/0 (Press and hold button repeat keys)";

	struct g4b_program* ip;
	ip = g4b_internal_list;
	struct g4b_configstrings* sp;
	sp = g4b_configstrings;

	for (i = 0; i < g4b_num_list; i++, ip++) {

		fprintf(fp, "%s", new_record);

		fprintf(fp, "%s		\"%s\" %s\n\n",
		sp->common_string.class_name, ip->common_data.class_name,
								name_tail);

		fprintf(fp, "%s \"%s\" %s\n",
		sp->common_string.stylus1_presscurve,
				ip->common_data.stylus1_presscurve, cur_tail);
		fprintf(fp, "%s \"%s\" %s\n\n",
		sp->common_string.stylus2_presscurve,
				ip->common_data.stylus2_presscurve, cur_tail);

		fprintf(fp, "%s	%i.%i	%s\n\n",
		sp->common_string.keycode_delay, *ip->common_data.keycode_delay,
				*(ip->common_data.keycode_delay+1), sec_tail);

		fprintf(fp, "%s	%i.%i	%s\n",
		sp->button_string.repeat_after, *ip->button_data.repeat_after,
				*(ip->button_data.repeat_after+1), sec_tail);
		fprintf(fp, "%s	%i.%i	%s\n\n",
		sp->button_string.repeat_delay, *ip->button_data.repeat_delay,
				*(ip->button_data.repeat_delay+1), sec_tail);


		fprintf(fp, "%s		%i %i	%s %i)\n",
		sp->button_string.button9, *ip->button_data.button9,
				*(ip->button_data.button9+1), key_tail,MAXKEYS);
		fprintf(fp, "%s		%i %i	%s %i)\n\n",
		sp->button_string.button10, *ip->button_data.button10,
				*(ip->button_data.button10+1),key_tail,MAXKEYS);

		fprintf(fp, "%s		%i	%s\n",
		sp->button_string.repeat9, *ip->button_data.repeat9, but_tail);
		fprintf(fp, "%s		%i	%s\n\n",
		sp->button_string.repeat10, *ip->button_data.repeat10,but_tail);

	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Write a set of records tuned for a 'padless' tablet:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void write_nop(FILE* fp)
{
	int i;

	const char* new_record =
"%%			# <---  ******** BEGIN NEW PROGRAM RECORD ********\n\n";
	const char* name_tail = "# Name must be within double quotes \"\"";
	const char* cur_tail = "# PressCurve must be within double quotes \"\"";

	struct nop_program* ip;
	ip = nop_internal_list;
	struct nop_configstrings* sp;
	sp = nop_configstrings;

	for (i = 0; i < nop_num_list; i++, ip++) {

		fprintf(fp, "%s", new_record);

		fprintf(fp, "%s		\"%s\" %s\n\n",
		sp->common_string.class_name, ip->common_data.class_name,
								name_tail);

		fprintf(fp, "%s \"%s\" %s\n",
		sp->common_string.stylus1_presscurve,
				ip->common_data.stylus1_presscurve, cur_tail);
		fprintf(fp, "%s \"%s\" %s\n\n",
		sp->common_string.stylus2_presscurve,
				ip->common_data.stylus2_presscurve, cur_tail);

	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Write some handy documentation at the top of each configuration file:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void write_preface(FILE* fp, const int model)
{
	const char* common_preface1 =
"\n# *************** Please do not remove the above header! ***************\n\n"

"# Blank lines and everything following a comment \"#\" sign are ignored.\n\n"

"# This file will be re-created whenever missing, but the name and layout\n"
"# depend on which devices that are detected at the very moment of program\n"
"# execution.\n\n";

	const char* model_notnop1 =
"# Some ASCII art showing the \"default\" program record:\n"
"#\n";

	const char *model_bee =
"# Cintiq 20 ExpressKeys Pad\n"
"#             Left         Right\n"
"#           +---+---+    +---+---+\n"
"#           |N/A|N/A|    |N/A|N/A|\n"
"#           +---+---+    +---+---+\n"
"# Back Left                         Back Right\n"
"#  +----+   +---+---+    +---+---+   +----+\n"
"#  |    |   |   17  |    |   18  |   |    |\n"
"#  | T  |   +---+---+    +---+---+   | T  |\n"
"#  | O  |   |   | 9 |    | 13|   |   | O  |\n"
"#  | U  |   |11 |---|    +---+ 15|   | U  |\n"
"#  | C  |   |   |10 |    | 14|   |   | C  |\n"
"#  | H  |   +---+---+    +---+---+   | H  |\n"
"#  |    |   |   12  |    |   16  |   |    |\n"
"#  +----+   +---+---+    +---+---+   +----+\n"
"# The N/A keys aren't supported yet\n";

	const char* model_i3si3 =
"# Left ExpressKeys Pad\n"
"# ------------\n"
"# |  |   |   |		Wacom Intuos3 defaults are:\n"
"# |  | 9 | T |\n"
"# |11|---| O |		Button 9  = (left) Shift	= keycode 50\n"
"# |  |10 | U |		Button 10 = (left) Alt		= keycode 64\n"
"# |------| C |		Button 11 = (left) Control	= keycode 37\n"
"# |  12  | H |		Button 12 = Space		= keycode 65\n"
"# ------------\n";

	const char* model_i3 =
"#\n"
"# Right ExpressKeys Pad\n"
"# ------------\n"
"# |   |   |  |		Wacom Intuos3 defaults are:\n"
"# | T |13 |  |\n"
"# | O |---|15|		Button 13 = (left) Shift	= keycode 50\n"
"# | U |14 |  |		Button 14 = (left) Alt		= keycode 64\n"
"# | C |------|		Button 15 = (left) Control	= keycode 37\n"
"# | H |  16  |		Button 16 = Space		= keycode 65\n"
"# ------------\n";

	const char* model_g4 =
"#            ExpressKeys Pad\n"
"# --------------------------------------\n"
"# | Left Button | Wheel | Right Button | \n"
"# --------------------------------------\n"
"#\n"
"#     Wacom Graphire4 defaults are:\n"
"#\n"
"# Left Button  = (left) Shift	= keycode 50\n"
"# Right Button = (left) Alt	= keycode 64\n";

	const char* model_g4b =
"#        ExpressKeys Pad\n"
"# -------------------------------\n"
"# | Left Button || Right Button | \n"
"# -------------------------------\n"
"#\n"
"# Wacom Graphire4 BT defaults are:\n"
"#\n"
"# Left Button  = (left) Shift	= keycode 50\n"
"# Right Button = (left) Alt	= keycode 64\n";

	const char* model_notnop2 =
"\n# Use the X program 'xev' to find keycodes when changing the key fields.\n"
"# Use the X program 'xprop' to find the name string to use for each record.\n"
"# A nice output can be obtained from: xprop | grep WM_CLASS | cut -d ',' -f2\n"
"# Read more about those two programs in the USAGE file.\n\n"

"# * Fake keycode: 999 *\n"
"# Use the value 999 as a keycode to enable stylus mode toggling between\n"
"# Absolute/Relative. To be able to switch mode anywhere each program block\n"
"# must then contain one 999 definition.\n\n"

"# * Fake keycodes: 991, 992, 993, 994, 995, 996, 997 *\n"
"# Use the values 991 to 997 for simulating mouse buttons 1 to 7. Only\n"
"# existing mouse buttons, defined through the driver of the active core\n"
"# pointer, can be simulated.\n\n"

"# * Fake keycodes: 1004, 1005, 1006, 1007, 1008, 1009 *\n"
"# Use the value 1004 to return from a tablet rotation (NONE), 1005 to flip a\n"
"# tablet 180 degrees (HALF), 1006 to rotate 90 degrees clock-wise (CW) and\n"
"# 1007 to rotate 90 degrees counter-clock-wise (CCW). By using 1008 you can\n"
"# rotate the tablet endlessly in a clock-wise manner (CW-HALF-CCW-NONE-CW-)\n"
"# and 1009 does the same motion counter-clock-wise (CCW-HALF-CW-NONE-CCW-).\n"
"# Looking down on the tablet and tilting the head '90' degrees to the right\n"
"# would simulate a CW operation.\n\n"

"# * Fake keycodes: 1001, 1002, 1003 *\n"
"# Instead of defining a fixed stylus PressCurve for different program\n"
"# blocks, you can use three values as keycodes to alter the curve inter-\n"
"# actively. 1001 will decrease the PressCurve, while 1003 will increase it.\n"
"# 1002 restores the curve to its default state: \"0 0 100 100\". Both the\n"
"# upward and downward curve changes flip over in a carousel fashion at the\n"
"# top and bottom values.\n\n";

	const char* common_preface2 =
"# Values for the stylus 'PressCurve' ie 'sensitivity' can be chosen the easy\n"
"# way - copying from how the wacomcpl program writes them, or the hard way -\n"
"# by experimenting.. The seven curves picked by wacomcpl are (Soft to Firm):\n"
"# 1               2               3               4 (default)\n"
"# \"0 75 25 100\" | \"0 50 50 100\" | \"0 25 75 100\" | \"0 0 100 100\"\n"
"# 5               6               7\n"
"# \"25 0 100 75\" | \"50 0 100 50\" | \"75 0 100 25\"\n\n"

"# OBSERVE: The \"default\" program record must exist SOMEWHERE in the file!\n"
"# Each program record is separated by one set of double percentage signs: %%\n"
"# Each program field begins after a case sensitive keyword, eg: ProgramName\n"
"# Whitespace (tabs and spaces) are ignored in the keycode fields, but spaces\n"
"# are recorded when in a ProgramName or StylusXPressCurve field (between the\n"
"# double quotes).\n\n";

	fprintf(fp, "%s", common_preface1);

	if (model != nop) {
		fprintf(fp, "%s", model_notnop1);
	}

	if (model == bee)
		fprintf(fp, "%s", model_bee);

	if ((model == i3s) || (model == i3)) {
		fprintf(fp, "%s", model_i3si3);
	}

	if (model == i3) {
		fprintf(fp, "%s", model_i3);
	}

	if (model == g4) {
		fprintf(fp, "%s", model_g4);
	}

	if (model == g4b) {
		fprintf(fp, "%s", model_g4b);
	}

	if (model != nop) {
		fprintf(fp, "%s", model_notnop2);
	}

	fprintf(fp, "%s", common_preface2);

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 If a configuration file doesn't exist we write a minimal one:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void write_if_lacking(FILE* errorfp, const char* configfile, int model)
{
	FILE* fp = NULL;

	if ((fp = fopen(configfile, "a+")) == NULL) {
		exit_on_error(errorfp,
"\n%s ERROR: Can not open %s in read/write mode\n", our_prog_name, configfile);
	}

	rewind(fp);

	if (fgetc(fp) == EOF) {

		fprintf(fp, "\n%s %s\n", configstring, configversion);

		write_preface(fp, model);

		if (model == bee) {
			write_bee(fp);
		} else if (model == i3) {
			write_i3(fp);
		} else if (model == i3s) {
			write_i3s(fp);
		} else if (model == g4) {
			write_g4(fp);
		} else if (model == g4b) {
			write_g4b(fp);
		} else if (model == nop) {
			write_nop(fp);
		}
	}

	if (ferror(fp)) {
		fclose(fp);
		exit_on_error(errorfp, "\n%s ERROR: Write error in %s\n",
						our_prog_name, configfile);
	}

	fclose(fp);

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Hand over all the associated configuration files for further processing:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void write_config(FILE* errorfp)
{
	int i;

	struct model_index* mip;
	mip = model_list;

	for (i = 0; i < MAXPAD; i++, mip++) {
		if (mip->bee->common_data.configfile)
			write_if_lacking(errorfp,
					mip->bee->common_data.configfile, bee);
		if (mip->i3->common_data.configfile) {
			write_if_lacking(errorfp,
					mip->i3->common_data.configfile, i3);
		}
		if (mip->i3s->common_data.configfile) {
			write_if_lacking(errorfp,
					mip->i3s->common_data.configfile, i3s);
		}
		if (mip->g4->common_data.configfile) {
			write_if_lacking(errorfp,
					mip->g4->common_data.configfile, g4);
		}
		if (mip->g4b->common_data.configfile) {
			write_if_lacking(errorfp,
					mip->g4b->common_data.configfile, g4b);
		}
		if (mip->nop->common_data.configfile) {
			write_if_lacking(errorfp,
					mip->nop->common_data.configfile, nop);
		}
	}

}

/* End Code */

