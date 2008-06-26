/*
 config_write.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.

 Copyright (C) 2005-2006 - Mats Johannesson

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
 Function writes a header in a new file, including config file version
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int write_file_config_header(FILE *fp)
{

	fprintf(fp, "\n%s %s\n", configstring, configversion);
	if (pad1_name) {
		fprintf(fp, "%s %s\n", pad1idstring, pad1_name);
	} else {
		fprintf(fp, "%s %s\n", pad1idstring, "LACKING");
	}
	if (stylus1_name) {
		fprintf(fp, "%s %s\n", stylus1idstring, stylus1_name);
	} else {
		fprintf(fp, "%s %s\n", stylus1idstring, "LACKING");
	}
	fprintf(fp, "\n# ***************** Please do not remove the above header! ****************\n\n");
	fprintf(fp, "# Blank lines and everything following a comment \"#\" sign are ignored.\n\n");
	fprintf(fp, "# This file will be re-created whenever missing, but the name and layout depend\n");
	fprintf(fp, "# on which devices that are detected at the very moment of program execution.\n\n");
	if ((pad1_info) && (!is_graphire4)) {
		fprintf(fp, "# Some ASCII art showing the \"default\" program record:\n");
		fprintf(fp, "#\n");
		fprintf(fp, "# Left ExpressKey Pad\n");
		fprintf(fp, "# ------------\n");
		fprintf(fp, "# |  |   |   |		Wacom Intuos3 defaults are:\n");
		fprintf(fp, "# |  | 9 | T |\n");
		fprintf(fp, "# |11|---| O |		Button 9  = (left) Shift	= keycode 50\n");
		fprintf(fp, "# |  |10 | U |		Button 10 = (left) Alt		= keycode 64\n");
		fprintf(fp, "# |------| C |		Button 11 = (left) Control	= keycode 37\n");
		fprintf(fp, "# |  12  | H |		Button 12 = Space		= keycode 65\n");
		fprintf(fp, "# ------------\n");
		fprintf(fp, "#\n");
		fprintf(fp, "# Right ExpressKey Pad\n");
		fprintf(fp, "# ------------\n");
		fprintf(fp, "# |   |   |  |		Wacom Intuos3 defaults are:\n");
		fprintf(fp, "# | T |13 |  |\n");
		fprintf(fp, "# | O |---|15|		Button 13 = (left) Shift	= keycode 50\n");
		fprintf(fp, "# | U |14 |  |		Button 14 = (left) Alt		= keycode 64\n");
		fprintf(fp, "# | C |------|		Button 15 = (left) Control	= keycode 37\n");
		fprintf(fp, "# | H |  16  |		Button 16 = Space		= keycode 65\n");
		fprintf(fp, "# ------------\n");
		fprintf(fp, "#\n");
	}
	if ((pad1_info) || (is_graphire4)) {
		fprintf(fp, "# Use the value 999 as a keycode to enable pen mode toggling. To be able\n");
		fprintf(fp, "# to switch mode anywhere each program block must then contain one 999\n");
		fprintf(fp, "# definition.\n\n");
		fprintf(fp, "# Use the values 991 to 997 for simulating mouse buttons 1 to 7. Only existing\n");
		fprintf(fp, "# mouse buttons, defined through the driver of the active core pointer, can be\n");
		fprintf(fp, "# simulated.\n\n");
	}
	fprintf(fp, "# Values for the stylus 'PressCurve' ie 'sensitivity' can be chosen the easy\n");
	fprintf(fp, "# way - copying from how the wacomcpl program writes them, or the hard way -\n");
	fprintf(fp, "# by experimenting... The seven curves picked by wacomcpl are (Soft to Firm):\n");
	fprintf(fp, "# 1               2               3               4 (default)\n");
	fprintf(fp, "# \"0 75 25 100\" | \"0 50 50 100\" | \"0 25 75 100\" | \"0 0 100 100\"\n");
	fprintf(fp, "# 5               6               7\n");
	fprintf(fp, "# \"25 0 100 75\" | \"50 0 100 50\" | \"75 0 100 25\"\n\n");
	fprintf(fp, "# OBSERVE: The \"default\" program record must exist _somewhere_ in the file!\n");
	fprintf(fp, "# Each program record is separated by one set of double percentage signs: %s\n", "%%");
	fprintf(fp, "# Each program field begins after a case sensitive keyword, eg: ProgramName\n");
	fprintf(fp, "# Whitespace (tabs and spaces) are ignored in the keycode fields, but spaces\n");
	fprintf(fp, "# are recorded when in a ProgramName or Stylus1PressCurve field (between the\n");
	fprintf(fp, "# double quotes).\n\n");
	if (is_graphire4) {
		fprintf(fp, "# Graphire4 user: Sorry about the automatic keycodes written here. They are\n");
		fprintf(fp, "# not optimal, to say the least, but picked from a larger set intended for\n");
		fprintf(fp, "# an Intuos3 (with its different button order). Perhaps we can write out a\n");
		fprintf(fp, "# more correct initial configuration file in a later version... Also, to you\n");
		fprintf(fp, "# BlueTooth users, the Scroll Wheel entries will disappear when a finer\n");
		fprintf(fp, "# grained configuration file can be written.\n\n");
	}

	return 0;

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Function writes out a short configuration file if none exists. It takes
 the info from a global memory structure whose only purpose is this initial
 write moment. The file should then be read back immediately to populate a
 memory structure that other functions rely on for their proper operation.
 Returns nothing useful. Write errors are checked in the calling function.
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int write_file_config(int *ip, FILE *fp)
{

	struct program *p;
	struct configstrings *hp;
/* Convert to long for x86_64 systems */
	p = (void *)(long)*ip;

	if (is_graphire4) {
		hp = gr4_human_readable;
	} else {
		hp = human_readable;
	}

	fprintf(fp, "%s			# <---  ********** BEGIN NEW PROGRAM RECORD **********\n\n", "%%");
	if (!is_graphire4) {
		fprintf(fp, "%s		\"%s\" # Name must be within double quotes \"\"\n\n", hp->h_class_name, p->class_name);
		fprintf(fp, "%s \"%s\" # PressCurve must be within double quotes \"\"\n\n", hp->h_stylus1_presscurve, p->stylus1_presscurve);
		if (pad1_info) {
			fprintf(fp, "%s	%d	# Main switch. Use 1 to enable the Touch Strips\n\n", hp->h_handle_touch, p->handle_touch);
			fprintf(fp, "%s		%d	# First keycode\n", hp->h_l_touch_up, p->l_touch_up);
			fprintf(fp, "%s	%d	# Extra keycode\n\n", hp->h_l_touch_up_plus, p->l_touch_up_plus);
			fprintf(fp, "%s	%d	# First keycode\n", hp->h_l_touch_down, p->l_touch_down);
			fprintf(fp, "%s	%d	# Extra keycode\n\n", hp->h_l_touch_down_plus, p->l_touch_down_plus);
			fprintf(fp, "%s		%d	# First keycode\n", hp->h_r_touch_up, p->r_touch_up);
			fprintf(fp, "%s	%d	# Extra keycode\n\n", hp->h_r_touch_up_plus, p->r_touch_up_plus);
			fprintf(fp, "%s	%d	# First keycode\n", hp->h_r_touch_down, p->r_touch_down);
			fprintf(fp, "%s	%d	# Extra keycode\n\n", hp->h_r_touch_down_plus, p->r_touch_down_plus);
			fprintf(fp, "%s		%d	# First keycode\n", hp->h_key_9, p->key_9);
			fprintf(fp, "%s	%d	# Extra keycode\n\n", hp->h_key_9_plus, p->key_9_plus);
			fprintf(fp, "%s		%d	# First keycode\n", hp->h_key_10, p->key_10);
			fprintf(fp, "%s	%d	# Extra keycode\n\n", hp->h_key_10_plus, p->key_10_plus);
			fprintf(fp, "%s		%d	# First keycode\n", hp->h_key_11, p->key_11);
			fprintf(fp, "%s	%d	# Extra keycode\n\n", hp->h_key_11_plus, p->key_11_plus);
			fprintf(fp, "%s		%d	# First keycode\n", hp->h_key_12, p->key_12);
			fprintf(fp, "%s	%d	# Extra keycode\n\n", hp->h_key_12_plus, p->key_12_plus);
			fprintf(fp, "%s	%d	# First keycode\n", hp->h_key_13, p->key_13);
			fprintf(fp, "%s	%d	# Extra keycode\n\n", hp->h_key_13_plus, p->key_13_plus);
			fprintf(fp, "%s	%d	# First keycode\n", hp->h_key_14, p->key_14);
			fprintf(fp, "%s	%d	# Extra keycode\n\n", hp->h_key_14_plus, p->key_14_plus);
			fprintf(fp, "%s	%d	# First keycode\n", hp->h_key_15, p->key_15);
			fprintf(fp, "%s	%d	# Extra keycode\n\n", hp->h_key_15_plus, p->key_15_plus);
			fprintf(fp, "%s	%d	# First keycode\n", hp->h_key_16, p->key_16);
			fprintf(fp, "%s	%d	# Extra keycode\n\n", hp->h_key_16_plus, p->key_16_plus);
		}
	} else {
		fprintf(fp, "%s		\"%s\" # Name must be within double quotes \"\"\n\n", hp->h_class_name, p->class_name);
		fprintf(fp, "%s \"%s\" # PressCurve must be within double quotes \"\"\n\n", hp->h_stylus1_presscurve, p->stylus1_presscurve);
		fprintf(fp, "%s	%d	# Main switch. Use 1 to enable the Scroll Wheel\n\n", hp->h_handle_touch, p->handle_touch);
		fprintf(fp, "%s		%d	# First keycode\n", hp->h_l_touch_up, p->l_touch_up);
		fprintf(fp, "%s	%d	# Extra keycode\n\n", hp->h_l_touch_up_plus, p->l_touch_up_plus);
		fprintf(fp, "%s		%d	# First keycode\n", hp->h_l_touch_down, p->l_touch_down);
		fprintf(fp, "%s	%d	# Extra keycode\n\n", hp->h_l_touch_down_plus, p->l_touch_down_plus);
		fprintf(fp, "%s		%d	# First keycode\n", hp->h_key_9, p->key_9);
		fprintf(fp, "%s		%d	# Extra keycode\n\n", hp->h_key_9_plus, p->key_9_plus);
		fprintf(fp, "%s		%d	# First keycode\n", hp->h_key_13, p->key_13);
		fprintf(fp, "%s		%d	# Extra keycode\n\n", hp->h_key_13_plus, p->key_13_plus);
	}
	return 0;
}

/* End Code */

