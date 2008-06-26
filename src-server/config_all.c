/*
 * config_all.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
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
/* Function reads a configuration file containing program names and */
/* definitions of which keys that should be mapped to pad buttons and */
/* touch strips. It takes a file pointer and a pointer to a global */
/* structure as input. Returns nothing unless an error occured.*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int read_file_config(int *ip, FILE *fp)
{

	struct program *p;
	p = (void *)*ip;

	int i = 0;
	int j = 0;
	char c;
	
	int num_record = 0;
	int num_field = 0;

	char buffer [MAXBUFFER];
	char *heap_protect;

/* Put a dummy heap protection in place... */

	if ((heap_protect = (char *)malloc(1024)) == NULL) {
		return 2; 
	}

/* Previously allocated memory for the program names must be released */
/* on subsequent reads of the config file. At program start the flag is 0 */

	if (reread_config) {
		for (i = 0; i < num_list; i++, p++) {
			free(p->class_name);
		}
		p = (void *)*ip;
	}
	reread_config = 1;

/* Read the config file in one go from top to bottom. Parse out the info */
/* between record start "{" and record end "}". Recognize field ends by a */
/* comma ",".Each full record is written to a global memory structure, */
/* while partial records are discarded and excessive fields are truncated. */
/* No sanity check on program names or keycode functionality is performed */
/* A global counter variable of full record instances is updated before */
/* the function exits to reflect the new state. */

	while ((c = fgetc(fp)) != EOF) {
		if (num_record < MAXRECORDS) {
			if (c == '{') {
				while ((c = fgetc(fp)) != EOF && (c != ',')) {
						if ((j < MAXBUFFER - 1) && (c != '"') && (isprint(c))) {
							buffer [j] = c;
							j++;
						}
				}
				buffer [j] = '\0';
				i = 0;
				j = 0;
				if ((p->class_name = (char *)malloc(strlen(buffer)+1)) == NULL) {
					return 2;
				}
				sprintf(p->class_name, "%s", buffer);
				while ((c = fgetc(fp)) != EOF && (c != '}')) {
					if (num_field < MAXFIELDS) {
						if ((c != ',') && (isdigit(c))) {
							if (i < MAXDIGITS) {
								buffer [i] = c;
								i++;
							}
						}
						if (c == ',') {
							buffer [i] = '\0';
							i=0;

							switch (num_field) {
								case 0:
								p->handle_touch = atoi(buffer);
								break;
								case 1:
								p->l_touch_up = atoi(buffer);
								break;
								case 2:
								p->l_touch_up_plus = atoi(buffer);
								break;
								case 3:
								p->l_touch_down = atoi(buffer);
								break;
								case 4:
								p->l_touch_down_plus = atoi(buffer);
								break;
								case 5:
								p->r_touch_up = atoi(buffer);
								break;
								case 6:
								p->r_touch_up_plus = atoi(buffer);
								break;
								case 7:
								p->r_touch_down = atoi(buffer);
								break;
								case 8:
								p->r_touch_down_plus = atoi(buffer);
								break;
								case 9:
								p->key_9 = atoi(buffer);
								break;
								case 10:
								p->key_9_plus = atoi(buffer);
								break;
								case 11:
								p->key_10 = atoi(buffer);
								break;
								case 12:
								p->key_10_plus = atoi(buffer);
								break;
								case 13:
								p->key_11 = atoi(buffer);
								break;
								case 14:
								p->key_11_plus = atoi(buffer);
								break;
								case 15:
								p->key_12 = atoi(buffer);
								break;
								case 16:
								p->key_12_plus = atoi(buffer);
								break;
								case 17:
								p->key_13 = atoi(buffer);
								break;
								case 18:
								p->key_13_plus = atoi(buffer);
								break;
								case 19:
								p->key_14 = atoi(buffer);
								break;
								case 20:
								p->key_14_plus = atoi(buffer);
								break;
								case 21:
								p->key_15 = atoi(buffer);
								break;
								case 22:
								p->key_15_plus = atoi(buffer);
								break;
								case 23:
								p->key_16 = atoi(buffer);
								break;
								case 24:
								p->key_16_plus = atoi(buffer);
								break;
								default:
								break;
							}
							num_field++;
						}
					}
				}
				if (num_field == MAXFIELDS) {
					num_record++;
					p++;
				} else {
					free(p->class_name);
				}
				num_field = 0;
			}
		}
	}
	if (!num_record) {
		return 1;
	}
	free(heap_protect);
	num_list = num_record;
	return 0;
}

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

