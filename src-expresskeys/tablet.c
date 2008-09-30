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

#include <stdio.h> /* NULL */
#include "tablet.h"

/* Globals: */

struct bee_program bee_internal_list[] = {
	{
	.common_data = {
		.class_name = "default",
		.stylus1_presscurve = "0 0 100 100",
		.stylus2_presscurve = "0 0 100 100",
		},
	.touch_data = {
		.handle_touch = { 1 },
		.repeat_after = { 0, 6 },
		.repeat_delay = { 0, 1 },
		.repeat_left_up = { 1 },
		.repeat_left_down = { 1 },
		.repeat_right_up = { 1 },
		.repeat_right_down = { 1 },
		.left_touch_up = { 994, 0 },
		.left_touch_down = { 995, 0 },
		.right_touch_up = { 98, 0 },
		.right_touch_down = { 104, 0 },
		},
	.button_data = {
		.repeat_after = { 0, 5 },
		.repeat_delay = { 0, 1 },
		.button9 = { 50, 0 },
		.button10 = { 64, 0 },
		.button11 = { 37, 0 },
		.button12 = { 65, 0 },
		.button13 = { 50, 0 },
		.button14 = { 64, 0 },
		.button15 = { 37, 0 },
		.button16 = { 65, 0 },
		.button17 = { 9, 0 },
		.button18 = { 36, 0 },
		},
	},
	{
	.common_data = {
		.class_name = "Gimp",
		.stylus1_presscurve = "0 0 100 100",
		.stylus2_presscurve = "0 0 100 100",
		},
	.touch_data = {
		.handle_touch = { 1 },
		.repeat_after = { 0, 6 },
		.repeat_delay = { 0, 1 },
		.left_touch_up = { 20, 0 },
		.left_touch_down = { 61, 0 },
		.right_touch_up = { 98, 20 },
		.right_touch_down = { 104, 61 },
		},
	.button_data = {
		.repeat_after = { 0, 5 },
		.repeat_delay = { 0, 1 },
		.button9 = { 50, 0 },
		.button10 = { 64, 0 },
		.button11 = { 37, 0 },
		.button12 = { 65, 0 },
		.button13 = { 37, 29 },
		.button14 = { 37, 52 },
		.button15 = { 999, 0 },
		.button16 = { 65, 0 },
		.button17 = { 9, 0 },
		.button18 = { 36, 0 },
		},
	},
	{
	.common_data = {
		.class_name = "Blender",
		.stylus1_presscurve = "0 0 100 100",
		.stylus2_presscurve = "0 0 100 100",
		},
	.touch_data = {
		.handle_touch = { 1 },
		.repeat_after = { 0, 6 },
		.repeat_delay = { 0, 1 },
		.repeat_left_up = { 1 },
		.repeat_left_down = { 1 },
		.repeat_right_up = { 1 },
		.repeat_right_down = { 1 },
		.left_touch_up = { 102, 0 },
		.left_touch_down = { 100, 0 },
		.right_touch_up = { 98, 0 },
		.right_touch_down = { 104, 0 },
		},
	.button_data = {
		.repeat_after = { 0, 5 },
		.repeat_delay = { 0, 1 },
		.button9 = { 37, 0 },
		.button10 = { 9, 0 },
		.button11 = { 50, 0 },
		.button12 = { 23, 0 },
		.button13 = { 37, 29 },
		.button14 = { 37, 52 },
		.button15 = { 999, 0 },
		.button16 = { 65, 0 },
		.button17 = { 9, 0 },
		.button18 = { 36, 0 },
		},
	},
	{
	.common_data = {
		.class_name = "XTerm",
		.stylus1_presscurve = "0 0 100 100",
		.stylus2_presscurve = "0 0 100 100",
		},
	.touch_data = {
		.repeat_after = { 0, 6 },
		.repeat_delay = { 0, 1 },
		},
	.button_data = {
		.repeat_after = { 0, 5 },
		.repeat_delay = { 0, 1 },
		.button15 = { 999, 0 },
		},
	},
};
const int bee_num_list = (sizeof bee_internal_list / sizeof bee_internal_list[0]);

struct bee_program bee_1_external_list[MAXRECORDS];
struct bee_program bee_2_external_list[MAXRECORDS];
struct bee_program bee_3_external_list[MAXRECORDS];

struct bee_configstrings bee_configstrings[] = {
{{"DelayEachKeycode", "ProgramName", "Stylus1PressCurve", "Stylus2PressCurve"},
{"HandleTouchStrips", "TouchRepeatAfter", "DelayTouchRepeat",
"RepeatLeftUp", "RepeatLeftDown", "RepeatRightUp", "RepeatRightDown",
"LeftPadTouchUp", "LeftPadTouchDown", "RightPadTouchUp", "RightPadTouchDown"},
{"ButtonRepeatAfter", "DelayButtonRepeat",
"RepeatButton9", "RepeatButton10", "RepeatButton11", "RepeatButton12",
"RepeatButton13", "RepeatButton14", "RepeatButton15", "RepeatButton16",
"RepeatButton17", "RepeatButton18",
"LeftPadButton9", "LeftPadButton10", "LeftPadButton11", "LeftPadButton12",
"RightPadButton13", "RightPadButton14", "RightPadButton15", "RightPadButton16",
"LeftPadButton17", "RightPadButton18"}}
};

struct i3_program i3_internal_list[] = {
	{NULL,{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,
	{0,0},"default", "0 0 100 100", "0 0 100 100"},
	{{1},{0,6},{0,1},{1},{1},{1},{1},
	{994,0},{995,0},{98,0},{104,0}},
	{{0,5},{0,1},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
	{50,0},{64,0},{37,0},{65,0},
	{50,0},{64,0},{37,0},{65,0},}},
	{NULL,{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,
	{0,0},"Gimp", "0 0 100 100", "0 0 100 100"},
	{{1},{0,6},{0,1},{0},{0},{0},{0},
	{20,0},{61,0},{64,20},{64,61}},
	{{0,5},{0,1},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
	{50,0},{64,0},{37,0},{65,0},
	{37,29},{37,52},{999,0},{65,0},}},
	{NULL,{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,
	{0,0},"Blender", "0 0 100 100", "0 0 100 100"},
	{{1},{0,6},{0,1},{1},{1},{1},{1},
	{102,0},{100,0},{98,0},{104,0}},
	{{0,5},{0,1},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
	{37,0},{9,0},{50,0},{23,0},
	{37,29},{37,52},{999,0},{65,0},}},
	{NULL,{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,
	{0,0},"XTerm", "0 0 100 100", "0 0 100 100"},
	{{0},{0,6},{0,1},{0},{0},{0},{0},
	{0,0},{0,0},{0,0},{0,0}},
	{{0,5},{0,1},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
	{0,0},{0,0},{0,0},{0,0},
	{0,0},{0,0},{999,0},{0,0},}}
};
const int i3_num_list = (sizeof i3_internal_list / sizeof i3_internal_list[0]);

struct i3_program i3_1_external_list[MAXRECORDS];
struct i3_program i3_2_external_list[MAXRECORDS];
struct i3_program i3_3_external_list[MAXRECORDS];

struct i3_configstrings i3_configstrings[] = {
{{"DelayEachKeycode", "ProgramName", "Stylus1PressCurve", "Stylus2PressCurve"},
{"HandleTouchStrips", "TouchRepeatAfter", "DelayTouchRepeat",
"RepeatLeftUp", "RepeatLeftDown", "RepeatRightUp", "RepeatRightDown",
"LeftPadTouchUp", "LeftPadTouchDown", "RightPadTouchUp", "RightPadTouchDown"},
{"ButtonRepeatAfter", "DelayButtonRepeat",
"RepeatButton9", "RepeatButton10", "RepeatButton11", "RepeatButton12",
"RepeatButton13", "RepeatButton14", "RepeatButton15", "RepeatButton16",
NULL,NULL,
"LeftPadButton9", "LeftPadButton10", "LeftPadButton11", "LeftPadButton12",
"RightPadButton13", "RightPadButton14", "RightPadButton15", "RightPadButton16",
NULL, NULL}}
};

struct i3s_program i3s_internal_list[] = {
	{NULL,{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,
	{0,0},"default", "0 0 100 100", "0 0 100 100"},
	{{1},{0,6},{0,1},{1},{1},{0},{0},
	{994,0},{995,0}},
	{{0,5},{0,1},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
	{50,0},{64,0},{37,0},{65,0},}},
	{NULL,{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,
	{0,0},"Gimp", "0 0 100 100", "0 0 100 100"},
	{{1},{0,6},{0,1},{0},{0},{0},{0},
	{20,0},{61,0}},
	{{0,5},{0,1},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
	{37,29},{37,52},{999,0},{65,0},}},
	{NULL,{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,
	{0,0},"Blender", "0 0 100 100", "0 0 100 100"},
	{{1},{0,6},{0,1},{1},{1},{0},{0},
	{102,0},{100,0}},
	{{0,5},{0,1},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
	{37,29},{37,52},{999,0},{65,0},}},
	{NULL,{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,
	{0,0},"XTerm", "0 0 100 100", "0 0 100 100"},
	{{0},{0,6},{0,1},{0},{0},{0},{0},
	{0,0},{0,0}},
	{{0,5},{0,1},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
	{0,0},{0,0},{999,0},{0,0},}}
};
const int i3s_num_list = (sizeof i3s_internal_list / sizeof
							i3s_internal_list[0]);

struct i3s_program i3s_1_external_list[MAXRECORDS];
struct i3s_program i3s_2_external_list[MAXRECORDS];
struct i3s_program i3s_3_external_list[MAXRECORDS];

struct i3s_configstrings i3s_configstrings[] = {
{{"DelayEachKeycode", "ProgramName", "Stylus1PressCurve", "Stylus2PressCurve"},
{"HandleTouchStrips", "TouchRepeatAfter", "DelayTouchRepeat",
"RepeatLeftUp","RepeatLeftDown",NULL,NULL,"LeftPadTouchUp", "LeftPadTouchDown"},
{"ButtonRepeatAfter", "DelayButtonRepeat",
"RepeatButton9", "RepeatButton10", "RepeatButton11", "RepeatButton12",
NULL,NULL,NULL,NULL,NULL,NULL,
"LeftPadButton9", "LeftPadButton10", "LeftPadButton11", "LeftPadButton12"}}
};

struct g4_program g4_internal_list[] = {
	{
	.common_data = {
		.class_name = "default",
		.stylus1_presscurve = "0 0 100 100",
		.stylus2_presscurve = "0 0 100 100",
		},
	.wheel_data = {
		.handle_wheel = { 1 },
		.scroll_wheel_up = { 994, 0 },
		.scroll_wheel_down = { 995, 0 },
		},
	.button_data = {
		.repeat_after = { 0, 5 },
		.repeat_delay = { 0, 1 },
		.button9 = { 0, 5 },
		.button11 = { 50, 0 },
		.button12 = { 64, 0 },
		.button13 = { 0, 1 },
		},
	},
	{
	.common_data = {
		.class_name = "Gimp",
		.stylus1_presscurve = "0 0 100 100",
		.stylus2_presscurve = "0 0 100 100",
		},
	.wheel_data = {
		.handle_wheel = { 1 },
		.scroll_wheel_up = { 20, 0 },
		.scroll_wheel_down = { 61, 0 },
		},
	.button_data = {
		.repeat_after = { 0, 5 },
		.repeat_delay = { 0, 1 },
		.button9 = { 37, 29 },
		.button13 = { 37, 52 },
		},
	},
	{
	.common_data = {
		.class_name = "Blender",
		.stylus1_presscurve = "0 0 100 100",
		.stylus2_presscurve = "0 0 100 100",
		},
	.wheel_data = {
		.handle_wheel = { 1 },
		.scroll_wheel_up = { 102, 0 },
		.scroll_wheel_down = { 100, 0 },
		},
	.button_data = {
		.repeat_after = { 0, 5 },
		.repeat_delay = { 0, 1 },
		.button9 = { 37, 29 },
		.button13 = { 37, 52 },
		},
	},
	{
	.common_data = {
		.class_name = "XTerm",
		.stylus1_presscurve = "0 0 100 100",
		.stylus2_presscurve = "0 0 100 100",
		},
	.button_data = {
		.repeat_after = { 0, 5 },
		.repeat_delay = { 0, 1 },
		.button9 = { 999, 0 },
		.button13 = { 999, 0 },
		},
	},
};

const int g4_num_list = (sizeof g4_internal_list / sizeof g4_internal_list[0]);

struct g4_program g4_1_external_list[MAXRECORDS];
struct g4_program g4_2_external_list[MAXRECORDS];
struct g4_program g4_3_external_list[MAXRECORDS];

struct g4_configstrings g4_configstrings[] = {
{{"DelayEachKeycode", "ProgramName", "Stylus1PressCurve", "Stylus2PressCurve"},
{"HandleScrollWheel", "ScrollWheelUp", "ScrollWheelDown"},
{"ButtonRepeatAfter", "DelayButtonRepeat",
"RepeatLeft", NULL, NULL, NULL, "RepeatRight", NULL, NULL, NULL, NULL, NULL,
"LeftButton", NULL, NULL, NULL, "RightButton"}}
};

struct g4b_program g4b_internal_list[] = {
	{NULL,{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,
	{0,0},"default", "0 0 100 100", "0 0 100 100"},
	{{0,5},{0,1},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
	{50,0},{64,0},}},
	{NULL,{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,
	{0,0},"Gimp", "0 0 100 100", "0 0 100 100"},
	{{0,5},{0,1},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
	{37,29},{37,52},}},
	{NULL,{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,
	{0,0},"Blender", "0 0 100 100", "0 0 100 100"},
	{{0,5},{0,1},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
	{37,29},{37,52},}},
	{NULL,{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,
	{0,0},"XTerm", "0 0 100 100", "0 0 100 100"},
	{{0,5},{0,1},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
	{0,0},{999,0},}}
};
const int g4b_num_list = (sizeof g4b_internal_list / sizeof
							g4b_internal_list[0]);

struct g4b_program g4b_1_external_list[MAXRECORDS];
struct g4b_program g4b_2_external_list[MAXRECORDS];
struct g4b_program g4b_3_external_list[MAXRECORDS];

struct g4b_configstrings g4b_configstrings[] = {
{{"DelayEachKeycode", "ProgramName", "Stylus1PressCurve", "Stylus2PressCurve"},
{"ButtonRepeatAfter", "DelayButtonRepeat",
"RepeatLeft", "RepeatRight",NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
"LeftButton", "RightButton"}}
};

struct nop_program nop_internal_list[] = {
	{NULL,{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,
	{0,0},"default", "0 0 100 100", "0 0 100 100"}},
	{NULL,{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,
	{0,0},"Gimp", "0 0 100 100", "0 0 100 100"}},
	{NULL,{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,
	{0,0},"Blender", "0 0 100 100", "0 0 100 100"}},
	{NULL,{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,
	{0,0},"XTerm", "0 0 100 100", "0 0 100 100"}}
};
const int nop_num_list = (sizeof nop_internal_list / sizeof
							nop_internal_list[0]);

struct nop_program nop_1_external_list[MAXRECORDS];
struct nop_program nop_2_external_list[MAXRECORDS];
struct nop_program nop_3_external_list[MAXRECORDS];

struct nop_configstrings nop_configstrings[] = {
{{NULL, "ProgramName", "Stylus1PressCurve", "Stylus2PressCurve"}}
};

struct model_index model_list[] = {
	{bee_1_external_list, i3_1_external_list, i3s_1_external_list,
	 g4_1_external_list, g4b_1_external_list, nop_1_external_list},
	{bee_2_external_list, i3_2_external_list, i3s_2_external_list,
	 g4_2_external_list, g4b_2_external_list, nop_2_external_list},
	{bee_3_external_list, i3_3_external_list, i3s_3_external_list,
	 g4_3_external_list, g4b_3_external_list, nop_3_external_list}
};

/* End Code */

