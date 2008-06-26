/*
 * config_write.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
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
/* Function writes out a short configuration file if none exists. It takes */
/* the info from a global memory structure whose only purpose is this initial */
/* write moment. The file should then be read back immediately to populate a */
/* memory structure that other functions rely on for their proper operation. */
/* Returns nothing useful. Write errors are checked in the calling function. */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int write_file_config(int *ip, FILE *fp)
{

	struct program *p;
	p = (void *)*ip;

	fprintf(fp, "------------------------------The field order is:-------------------------\n");
	fprintf(fp, "Programname	handle_touch \n");
	fprintf(fp, "l_touch_up	l_touch_up_plus	l_touch_down	l_touch_down_plus\n");
	fprintf(fp, "r_touch_up	r_touch_up_plus	r_touch_down	r_touch_down_plus\n");
	fprintf(fp, "key_9		key_9_plus	key_10		key_10_plus\n");
	fprintf(fp, "key_11		key_11_plus	key_12		key_12_plus\n");
	fprintf(fp, "key_13		key_13_plus	key_14		key_14_plus\n");
	fprintf(fp, "key_15		key_15_plus	key_16		key_16_plus\n");
	fprintf(fp, "--------------------------------------------------------------------------\n");
	fprintf(fp, "Please don't put any comment inside the definition below this text block.\n");
	fprintf(fp, "And please don't alter or remove the commas (,) after each field entry.\n");
	fprintf(fp, "\n");
	fprintf(fp, "{\"%s\",	%d,\n", p->class_name, p->handle_touch);
	fprintf(fp, "%d,		%d,		%d,		%d,\n", p->l_touch_up, p->l_touch_up_plus, p->l_touch_down, p->l_touch_down_plus);
	fprintf(fp, "%d,		%d,		%d,		%d,\n", p->r_touch_up, p->r_touch_up_plus, p->r_touch_down, p->r_touch_down_plus);
	fprintf(fp, "%d,		%d,		%d,		%d,\n", p->key_9, p->key_9_plus, p->key_10, p->key_10_plus);
	fprintf(fp, "%d,		%d,		%d,		%d,\n", p->key_11, p->key_11_plus, p->key_12, p->key_12_plus);
	fprintf(fp, "%d,		%d,		%d,		%d,\n", p->key_13, p->key_13_plus, p->key_14, p->key_14_plus);
	fprintf(fp, "%d,		%d,		%d,		%d,	}\n\n", p->key_15, p->key_15_plus, p->key_16, p->key_16_plus);

	return 0;
}

/* End Code */

