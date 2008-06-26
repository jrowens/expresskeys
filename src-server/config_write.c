/*
 config_write.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
 
 Copyright (C) 2005 - Mats Johannesson
 
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

	fprintf(fp, "\nVersion: %d		# Config File Version. Please don't remove.\n\n", CONFIG_VERSION);
	fprintf(fp, "# Blank lines and everything following a comment \"#\" sign are ignored.\n\n");
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
	fprintf(fp, "# The \"default\" program record must be the very first record, at the top.\n");
	fprintf(fp, "# Use the value 999 as a keycode to enable pen mode toggling. To be able\n");
	fprintf(fp, "# to switch mode anywhere each program block must then contain one 999\n");
	fprintf(fp, "# definition.\n\n");
	fprintf(fp, "# Each program record is enclosed between two sets of double percentage signs.\n");
	fprintf(fp, "# Each program field begins after a colon \":\". Whitespace (tabs and spaces)\n");
	fprintf(fp, "# are ignored in the keycode fields, but spaces are recorded when in a\n");
	fprintf(fp, "# Program Name field (between the double quotes).\n\n\n");

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
	p = (void *)*ip;

	fprintf(fp, "%s				# <--- Begin New Program Record\n\n", "%%");
	fprintf(fp, "00 Program Name: \"%s\"	# Name must be within double quotes \"\"\n", p->class_name);
	fprintf(fp, "01 Handle Touch Strips: %d	# Main switch. Use 1 to enable the Touch Strips\n\n", p->handle_touch);
	fprintf(fp, "02 Left Pad - Touch Up:		%d	# Left Touch Up\n", p->l_touch_up);
	fprintf(fp, "03 Left Pad - Touch Up Plus:	%d	# Extra key\n\n", p->l_touch_up_plus);
	fprintf(fp, "04 Left Pad - Touch Down:	%d	# Left Touch Down\n", p->l_touch_down);
	fprintf(fp, "05 Left Pad - Touch Down Plus:	%d	# Extra key\n\n", p->l_touch_down_plus);
	fprintf(fp, "06 Right Pad - Touch Up:	%d	# Right Touch Up\n", p->r_touch_up);
	fprintf(fp, "07 Right Pad - Touch Up Plus:	%d	# Extra key\n\n", p->r_touch_up_plus);
	fprintf(fp, "08 Right Pad - Touch Down:	%d	# Right Touch Down\n", p->r_touch_down);
	fprintf(fp, "09 Right Pad - Touch Down Plus:	%d	# Extra key\n\n", p->r_touch_down_plus);
	fprintf(fp, "10 Left Pad - Button 9:		%d	# Button 9\n", p->key_9);
	fprintf(fp, "11 Left Pad - Button 9 Plus:	%d	# Extra key\n\n", p->key_9_plus);
	fprintf(fp, "12 Left Pad - Button 10:	%d	# Button 10\n", p->key_10);
	fprintf(fp, "13 Left Pad - Button 10 Plus:	%d	# Extra key\n\n", p->key_10_plus);
	fprintf(fp, "14 Left Pad - Button 11:	%d	# Button 11\n", p->key_11);
	fprintf(fp, "15 Left Pad - Button 11 Plus:	%d	# Extra key\n\n", p->key_11_plus);
	fprintf(fp, "16 Left Pad - Button 12:	%d	# Button 12\n", p->key_12);
	fprintf(fp, "17 Left Pad - Button 12 Plus:	%d	# Extra key\n\n", p->key_12_plus);
	fprintf(fp, "18 Right Pad - Button 13:	%d	# Button 13\n", p->key_13);
	fprintf(fp, "19 Right Pad - Button 13 Plus:	%d	# Extra key\n\n", p->key_13_plus);
	fprintf(fp, "20 Right Pad - Button 14:	%d	# Button 14\n", p->key_14);
	fprintf(fp, "21 Right Pad - Button 14 Plus:	%d	# Extra key\n\n", p->key_14_plus);
	fprintf(fp, "22 Right Pad - Button 15:	%d	# Button 15\n", p->key_15);
	fprintf(fp, "23 Right Pad - Button 15 Plus:	%d	# Extra key\n\n", p->key_15_plus);
	fprintf(fp, "24 Right Pad - Button 16:	%d	# Button 16\n", p->key_16);
	fprintf(fp, "25 Right Pad - Button 16 Plus:	%d	# Extra key\n\n", p->key_16_plus);
	fprintf(fp, "%s				# <--- End Program Record\n\n", "%%");

	return 0;
}

/* End Code */

