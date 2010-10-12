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

#ifndef TABLET_H_
#define TABLET_H_

#include "defines.h"

/* Data structures (parts): */

extern struct common_data {
	const char* padname;
	unsigned long int* padid;
	const char* sty1name;
	unsigned long int* sty1id;
	unsigned char* sty1mode;
	const char* sty2name;
	unsigned long int* sty2id;
	unsigned char* sty2mode;
	const char* configfile;
	int userconfigversion;
	int num_record;
	int keycode_delay[2];
	char* class_name;
	char* stylus1_presscurve;
	char* stylus2_presscurve;
} common_data[];

extern struct touch_data {
	int handle_touch[1];
	int repeat_after[2];
	int repeat_delay[2];
	int repeat_left_up [1];
	int repeat_left_down [1];
	int repeat_right_up [1];
	int repeat_right_down [1];
	int left_touch_up[MAXKEYS];
	int left_touch_down[MAXKEYS];
	int right_touch_up[MAXKEYS];
	int right_touch_down[MAXKEYS];
} touch_data[];

extern struct wheel_data {
	int handle_wheel[1];
	int scroll_wheel_up[MAXKEYS];
	int scroll_wheel_down[MAXKEYS];
} wheel_data[];

extern struct button_data {
	int repeat_after[2];
	int repeat_delay[2];
	int repeat1 [1];
	int repeat2 [1];
	int repeat3 [1];
	int repeat4 [1];
	int repeat5 [1];
	int repeat6 [1];
	int repeat7 [1];
	int repeat8 [1];
	int repeat9 [1];
	int repeat10 [1];
	int repeat11 [1];
	int repeat12 [1];
	int repeat13 [1];
	int repeat14 [1];
	int repeat15 [1];
	int repeat16 [1];
	int repeat17 [1];
	int repeat18 [1];
	int button1[MAXKEYS];
	int button2[MAXKEYS];
	int button3[MAXKEYS];
	int button4[MAXKEYS];
	int button5[MAXKEYS];
	int button6[MAXKEYS];
	int button7[MAXKEYS];
	int button8[MAXKEYS];
	int button9[MAXKEYS];
	int button10[MAXKEYS];
	int button11[MAXKEYS];
	int button12[MAXKEYS];
	int button13[MAXKEYS];
	int button14[MAXKEYS];
	int button15[MAXKEYS];
	int button16[MAXKEYS];
	int button17[MAXKEYS];
	int button18[MAXKEYS];
} button_data[];


/* Data structures (consolidation): */
extern struct ux2_program {
	struct ux2_program* default_program;
	struct common_data common_data;
	struct touch_data touch_data;
	struct button_data button_data;
} ux2_internal_list[], ux2_1_external_list[], ux2_2_external_list[],
							ux2_3_external_list[];

extern struct bbo_program {
	struct bbo_program* default_program;
	struct common_data common_data;
	struct wheel_data wheel_data;
	struct touch_data touch_data;
	struct button_data button_data;
} bbo_internal_list[], bbo_1_external_list[], bbo_2_external_list[],
							bbo_3_external_list[];

extern struct bee_program {
	struct bee_program* default_program;
	struct common_data common_data;
	struct touch_data touch_data;
	struct button_data button_data;
} bee_internal_list[], bee_1_external_list[], bee_2_external_list[],
							bee_3_external_list[];

extern struct i3_program {
	struct i3_program* default_program;
	struct common_data common_data;
	struct touch_data touch_data;
	struct button_data button_data;
} i3_internal_list[], i3_1_external_list[], i3_2_external_list[],
							i3_3_external_list[];

extern struct i3s_program {
	struct i3s_program* default_program;
	struct common_data common_data;
	struct touch_data touch_data;
	struct button_data button_data;
} i3s_internal_list[], i3s_1_external_list[], i3s_2_external_list[],
							i3s_3_external_list[];

extern struct g4_program {
	struct g4_program* default_program;
	struct common_data common_data;
	struct wheel_data wheel_data;
	struct button_data button_data;
} g4_internal_list[], g4_1_external_list[], g4_2_external_list[],
							g4_3_external_list[];

extern struct g4b_program {
	struct g4b_program* default_program;
	struct common_data common_data;
	struct button_data button_data;
} g4b_internal_list[], g4b_1_external_list[], g4b_2_external_list[],
							g4b_3_external_list[];

extern struct nop_program {
	struct nop_program* default_program;
	struct common_data common_data;
} nop_internal_list[], nop_1_external_list[], nop_2_external_list[],
							nop_3_external_list[];


/* String structures (parts): */

extern struct common_string {
	const char* keycode_delay;
	const char* class_name;
	const char* stylus1_presscurve;
	const char* stylus2_presscurve;
} common_string[];

extern struct touch_string {
	const char* handle_touch;
	const char* repeat_after;
	const char* repeat_delay;
	const char* repeat_left_up;
	const char* repeat_left_down;
	const char* repeat_right_up;
	const char* repeat_right_down;
	const char* left_touch_up;
	const char* left_touch_down;
	const char* right_touch_up;
	const char* right_touch_down;
} touch_string[];

extern struct wheel_string {
	const char* handle_wheel;
	const char* scroll_wheel_up;
	const char* scroll_wheel_down;
} wheel_string[];

extern struct button_string {
	const char* repeat_after;
	const char* repeat_delay;
	const char* repeat1;
	const char* repeat2;
	const char* repeat3;
	const char* repeat4;
	const char* repeat5;
	const char* repeat6;
	const char* repeat7;
	const char* repeat8;
	const char* repeat9;
	const char* repeat10;
	const char* repeat11;
	const char* repeat12;
	const char* repeat13;
	const char* repeat14;
	const char* repeat15;
	const char* repeat16;
	const char* repeat17;
	const char* repeat18;
	const char* button1;
	const char* button2;
	const char* button3;
	const char* button4;
	const char* button5;
	const char* button6;
	const char* button7;
	const char* button8;
	const char* button9;
	const char* button10;
	const char* button11;
	const char* button12;
	const char* button13;
	const char* button14;
	const char* button15;
	const char* button16;
	const char* button17;
	const char* button18;
} button_string[];


/* String structures (consolidation): */

extern struct ux2_configstrings {
	struct common_string common_string;
	struct touch_string touch_string;
	struct button_string button_string;
} ux2_configstrings[];

extern struct bbo_configstrings {
	struct common_string common_string;
	struct wheel_string wheel_string;
	struct touch_string touch_string;
	struct button_string button_string;
} bbo_configstrings[];

extern struct bee_configstrings {
	struct common_string common_string;
	struct touch_string touch_string;
	struct button_string button_string;
} bee_configstrings[];

extern struct i3_configstrings {
	struct common_string common_string;
	struct touch_string touch_string;
	struct button_string button_string;
} i3_configstrings[];

extern struct i3s_configstrings {
	struct common_string common_string;
	struct touch_string touch_string;
	struct button_string button_string;
} i3s_configstrings[];

extern struct g4_configstrings {
	struct common_string common_string;
	struct wheel_string wheel_string;
	struct button_string button_string;
} g4_configstrings[];

extern struct g4b_configstrings {
	struct common_string common_string;
	struct button_string button_string;
} g4b_configstrings[];

extern struct nop_configstrings {
	struct common_string common_string;
} nop_configstrings[];


/* Data structure (final consolidation): */

extern struct model_index {
	struct ux2_program* ux2;
	struct bbo_program* bbo;
	struct bee_program* bee;
	struct i3_program* i3;
	struct i3s_program* i3s;
	struct g4_program* g4;
	struct g4b_program* g4b;
	struct nop_program* nop;
} model_list[];

#endif /*TABLET_H_*/

/* End Code */

