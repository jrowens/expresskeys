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
#include <X11/extensions/XInput.h>

#include "tablet.h"

/* Globals: */

/* Strings to all (allowed) configuration files: */
const char* total_file1_padless;
const char* total_file2_padless;
const char* total_file3_padless;
const char* total_file1_graphire4bt;
const char* total_file2_graphire4bt;
const char* total_file3_graphire4bt;
const char* total_file1_graphire4;
const char* total_file2_graphire4;
const char* total_file3_graphire4;
const char* total_file1_intuos3s;
const char* total_file2_intuos3s;
const char* total_file3_intuos3s;
const char* total_file1_intuos3;
const char* total_file2_intuos3;
const char* total_file3_intuos3;
const char* total_file1_bee;
const char* total_file2_bee;
const char* total_file3_bee;

/* Externals: */

extern int have_pad;
extern int have_padless;

extern const int bee;
extern const int i3;
extern const int i3s;
extern const int g4;
extern const int g4b;
extern const int nop;

extern const char* pad_string;
extern const char* stylus_string;

extern const char* total_config_dir;

extern XDevice* pad_xdevice[MAXMODEL][MAXPAD];
extern XDevice* stylus_xdevice[MAXSTYLUS][MAXMODEL * MAXPAD];

extern unsigned long int* pad_id[MAXMODEL][MAXPAD];
extern unsigned long int* stylus_id[MAXSTYLUS][MAXMODEL * MAXPAD];

extern const char* pad_name[MAXMODEL][MAXPAD];
extern const char* stylus_name[MAXSTYLUS][MAXMODEL * MAXPAD];

extern unsigned char* stylus_mode[MAXSTYLUS][MAXMODEL * MAXPAD];

extern char* path_malloc(char* path, char* file);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 The long and boring information transfer from arrays to structures:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void mark_device(const char* device, int row, int column,
							const char* configfile)
{
	if (device == pad_string) {
		if (row == bee) {
			struct bee_program* p = NULL;
			if (column == 0) {
				p = bee_1_external_list;
			} else if (column == 1) {
				p = bee_2_external_list;
			} else if (column == 2) {
				p = bee_3_external_list;
			}
			p->common_data.padname = pad_name[row][column];
			p->common_data.padid = pad_id[row][column];
			p->common_data.configfile = configfile;
			return;
		}
		if (row == i3) {
			struct i3_program* p = NULL;
			if (column == 0) {
				p = i3_1_external_list;
			} else if (column == 1) {
				p = i3_2_external_list;
			} else if (column == 2) {
				p = i3_3_external_list;
			}
			p->common_data.padname = pad_name[row][column];
			p->common_data.padid = pad_id[row][column];
			p->common_data.configfile = configfile;
			return;
		}
		if (row == i3s) {
			struct i3s_program* p = NULL;
			if (column == 0) {
				p = i3s_1_external_list;
			} else if (column == 1) {
				p = i3s_2_external_list;
			} else if (column == 2) {
				p = i3s_3_external_list;
			}
			p->common_data.padname = pad_name[row][column];
			p->common_data.padid = pad_id[row][column];
			p->common_data.configfile = configfile;
			return;
		}
		if (row == g4) {
			struct g4_program* p = NULL;
			if (column == 0) {
				p = g4_1_external_list;
			} else if (column == 1) {
				p = g4_2_external_list;
			} else if (column == 2) {
				p = g4_3_external_list;
			}
			p->common_data.padname = pad_name[row][column];
			p->common_data.padid = pad_id[row][column];
			p->common_data.configfile = configfile;
			return;
		}
		if (row == g4b) {
			struct g4b_program* p = NULL;
			if (column == 0) {
				p = g4b_1_external_list;
			} else if (column == 1) {
				p = g4b_2_external_list;
			} else if (column == 2) {
				p = g4b_3_external_list;
			}
			p->common_data.padname = pad_name[row][column];
			p->common_data.padid = pad_id[row][column];
			p->common_data.configfile = configfile;
			return;
		}
		if (row == nop) {
			struct nop_program* p = NULL;
			if (column == 0) {
				p = nop_1_external_list;
			} else if (column == 1) {
				p = nop_2_external_list;
			} else if (column == 2) {
				p = nop_3_external_list;
			}
			p->common_data.configfile = configfile;
			return;
		}
	}

	if (device == stylus_string) {
		if ((column == MAXPAD*bee) || (column == (MAXPAD*bee)+1)
			|| (column == (MAXPAD*bee)+2)) {
			struct bee_program* p = NULL;
			if (column == MAXPAD*bee) {
				p = bee_1_external_list;
			} else if (column == (MAXPAD*bee)+1) {
				p = bee_2_external_list;
			} else if (column == (MAXPAD*bee)+2) {
				p = bee_3_external_list;
			}
			if (row == 0) {
				p->common_data.sty1name =
						stylus_name[row][column];
				p->common_data.sty1id = stylus_id[row][column];
				p->common_data.sty1mode =
						stylus_mode[row][column];
			} else if (row == 1) {
				p->common_data.sty2name =
						stylus_name[row][column];
				p->common_data.sty2id = stylus_id[row][column];
				p->common_data.sty2mode =
						stylus_mode[row][column];
			}
			return;
		}
		if ((column == MAXPAD*i3) || (column == (MAXPAD*i3)+1)
			|| (column == (MAXPAD*i3)+2)) {
			struct i3_program* p = NULL;
			if (column == MAXPAD*i3) {
				p = i3_1_external_list;
			} else if (column == (MAXPAD*i3)+1) {
				p = i3_2_external_list;
			} else if (column == (MAXPAD*i3)+2) {
				p = i3_3_external_list;
			}
			if (row == 0) {
				p->common_data.sty1name =
						stylus_name[row][column];
				p->common_data.sty1id = stylus_id[row][column];
				p->common_data.sty1mode =
						stylus_mode[row][column];
			} else if (row == 1) {
				p->common_data.sty2name =
						stylus_name[row][column];
				p->common_data.sty2id = stylus_id[row][column];
				p->common_data.sty2mode =
						stylus_mode[row][column];
			}
			return;
		}
		if ((column == MAXPAD*i3s) || (column == (MAXPAD*i3s)+1)
			|| (column == (MAXPAD*i3s)+2)) {
			struct i3s_program* p = NULL;
			if (column == MAXPAD*i3s) {
				p = i3s_1_external_list;
			} else if (column == (MAXPAD*i3s)+1) {
				p = i3s_2_external_list;
			} else if (column == (MAXPAD*i3s)+2) {
				p = i3s_3_external_list;
			}
			if (row == 0) {
				p->common_data.sty1name =
						stylus_name[row][column];
				p->common_data.sty1id = stylus_id[row][column];
				p->common_data.sty1mode =
						stylus_mode[row][column];
			} else if (row == 1) {
				p->common_data.sty2name =
						stylus_name[row][column];
				p->common_data.sty2id = stylus_id[row][column];
				p->common_data.sty2mode =
						stylus_mode[row][column];
			}
			return;
		}
		if ((column == MAXPAD*g4) || (column == (MAXPAD*g4)+1)
			|| (column == (MAXPAD*g4)+2)) {
			struct g4_program* p = NULL;
			if (column == MAXPAD*g4) {
				p = g4_1_external_list;
			} else if (column == (MAXPAD*g4)+1) {
				p = g4_2_external_list;
			} else if (column == (MAXPAD*g4)+2) {
				p = g4_3_external_list;
			}
			if (row == 0) {
				p->common_data.sty1name =
						stylus_name[row][column];
				p->common_data.sty1id = stylus_id[row][column];
				p->common_data.sty1mode =
						stylus_mode[row][column];
			} else if (row == 1) {
				p->common_data.sty2name =
						stylus_name[row][column];
				p->common_data.sty2id = stylus_id[row][column];
				p->common_data.sty2mode =
						stylus_mode[row][column];
			}
			return;
		}
		if ((column == MAXPAD*g4b) || (column == (MAXPAD*g4b)+1)
			|| (column == (MAXPAD*g4b)+2)) {
			struct g4b_program* p = NULL;
			if (column == MAXPAD*g4b) {
				p = g4b_1_external_list;
			} else if (column == (MAXPAD*g4b)+1) {
				p = g4b_2_external_list;
			} else if (column == (MAXPAD*g4b)+2) {
				p = g4b_3_external_list;
			}
			if (row == 0) {
				p->common_data.sty1name =
						stylus_name[row][column];
				p->common_data.sty1id = stylus_id[row][column];
				p->common_data.sty1mode =
						stylus_mode[row][column];
			} else if (row == 1) {
				p->common_data.sty2name =
						stylus_name[row][column];
				p->common_data.sty2id = stylus_id[row][column];
				p->common_data.sty2mode =
						stylus_mode[row][column];
			}
			return;
		}
		if ((column == MAXPAD*nop) || (column == (MAXPAD*nop)+1)
			|| (column == (MAXPAD*nop)+2)) {
			struct nop_program* p = NULL;
			if (column == MAXPAD*nop) {
				p = nop_1_external_list;
			} else if (column == (MAXPAD*nop)+1) {
				p = nop_2_external_list;
			} else if (column == (MAXPAD*nop)+2) {
				p = nop_3_external_list;
			}
			if (row == 0) {
				p->common_data.sty1name =
						stylus_name[row][column];
				p->common_data.sty1id = stylus_id[row][column];
				p->common_data.sty1mode =
						stylus_mode[row][column];
			} else if (row == 1) {
				p->common_data.sty2name =
						stylus_name[row][column];
				p->common_data.sty2id = stylus_id[row][column];
				p->common_data.sty2mode =
						stylus_mode[row][column];
			}
			return;
		}
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Malloc strings for the configuration file names, then jump to mark_device:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void mark_config(const char* device, int row, int column)
{
	const char* file1_padless = "/padless.conf1";
	const char* file2_padless = "/padless.conf2";
	const char* file3_padless = "/padless.conf3";
	const char* file1_graphire4bt = "/graphire4bt.conf1";
	const char* file2_graphire4bt = "/graphire4bt.conf2";
	const char* file3_graphire4bt = "/graphire4bt.conf3";
	const char* file1_graphire4 = "/graphire4.conf1";
	const char* file2_graphire4 = "/graphire4.conf2";
	const char* file3_graphire4 = "/graphire4.conf3";
	const char* file1_intuos3s = "/intuos3s.conf1";
	const char* file2_intuos3s = "/intuos3s.conf2";
	const char* file3_intuos3s = "/intuos3s.conf3";
	const char* file1_intuos3 = "/intuos3.conf1";
	const char* file2_intuos3 = "/intuos3.conf2";
	const char* file3_intuos3 = "/intuos3.conf3";
	const char* file1_bee = "/cintiq20.conf1";
	const char* file2_bee = "/cintiq20.conf2";
	const char* file3_bee = "/cintiq20.conf3";

	switch (row) {
/* pad Cintiq 20wsx */
	case 5:
		switch (column) {
		case 0:
			total_file1_bee = path_malloc
				((void*)total_config_dir, (void*)file1_bee);
			mark_device(device, row, column, total_file1_bee);
			return;

		case 1:
			total_file2_bee = path_malloc
				((void*)total_config_dir, (void*)file2_bee);
			mark_device(device, row, column, total_file2_bee);
			return;

		case 2:
			total_file3_bee = path_malloc
				((void*)total_config_dir, (void*)file3_bee);
			mark_device(device, row, column, total_file3_bee);
			return;

		default:
			return;
		}
/* pad Intuos3/Cintiq 21UX */
	case 4:
		switch (column) {
		case 0:
			total_file1_intuos3 = path_malloc
				((void*)total_config_dir, (void*)file1_intuos3);
			mark_device(device, row, column, total_file1_intuos3);
			return;

		case 1:
			total_file2_intuos3 = path_malloc
				((void*)total_config_dir, (void*)file2_intuos3);
			mark_device(device, row, column, total_file2_intuos3);
			return;

		case 2:
			total_file3_intuos3 = path_malloc
				((void*)total_config_dir, (void*)file3_intuos3);
			mark_device(device, row, column, total_file3_intuos3);
			return;

		default:
			return;
		}
/* pad Intuos3 small */
	case 3:
		switch (column) {
		case 0:
			total_file1_intuos3s = path_malloc
				((void*)total_config_dir,(void*)file1_intuos3s);
			mark_device(device, row, column, total_file1_intuos3s);
			return;

		case 1:
			total_file2_intuos3s = path_malloc
				((void*)total_config_dir,(void*)file2_intuos3s);
			mark_device(device, row, column, total_file2_intuos3s);
			return;

		case 2:
			total_file3_intuos3s = path_malloc
				((void*)total_config_dir,(void*)file3_intuos3s);
			mark_device(device, row, column, total_file3_intuos3s);
			return;

		default:
			return;
		}
/* pad Graphire4 */
	case 2:
		switch (column) {
		case 0:
			total_file1_graphire4 = path_malloc
			((void*)total_config_dir, (void*)file1_graphire4);
			mark_device(device, row, column, total_file1_graphire4);
			return;

		case 1:
			total_file2_graphire4 = path_malloc
			((void*)total_config_dir, (void*)file2_graphire4);
			mark_device(device, row, column, total_file2_graphire4);
			return;

		case 2:
			total_file3_graphire4 = path_malloc
			((void*)total_config_dir, (void*)file3_graphire4);
			mark_device(device, row, column, total_file3_graphire4);
			return;

		default:
			return;
		}
/* pad Graphire4 BlueTooth */
	case 1:
		switch (column) {
		case 0:
			total_file1_graphire4bt = path_malloc
			((void*)total_config_dir, (void*)file1_graphire4bt);
			mark_device(device, row, column,
						total_file1_graphire4bt);
			return;

		case 1:
			total_file2_graphire4bt = path_malloc
			((void*)total_config_dir, (void*)file2_graphire4bt);
			mark_device(device, row, column,
						total_file2_graphire4bt);
			return;

		case 2:
			total_file3_graphire4bt = path_malloc
			((void*)total_config_dir, (void*)file3_graphire4bt);
			mark_device(device, row, column,
						total_file3_graphire4bt);
			return;

		default:
			return;
		}
/* padless */
	case 0:
		switch (column) {
		case 0:
			total_file1_padless = path_malloc
				((void*)total_config_dir, (void*)file1_padless);
			mark_device(device, row, column, total_file1_padless);
			return;

		case 1:
			total_file2_padless = path_malloc
				((void*)total_config_dir, (void*)file2_padless);
			mark_device(device, row, column, total_file2_padless);
			return;

		case 2:
			total_file3_padless = path_malloc
				((void*)total_config_dir, (void*)file3_padless);
			mark_device(device, row, column, total_file3_padless);
			return;

		default:
			return;
		}

	default:
		return;
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Transfer obtained knowledge about the devices from arrays to structures:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void device_to_structure()
{
	int i, j;

	if (have_pad) {
		for (i = 0; i < MAXMODEL; i++) {
			for (j = 0; j < MAXPAD; j++) {
				if (pad_xdevice[i][j]) {
					mark_config(pad_string, i, j);
				}
			}
		}
	}

	if (have_padless) {
		for (i = 0; i < MAXSTYLUS; i++) {
			for (j = 0; j < MAXPAD; j++) {
				if (stylus_xdevice[i][j]) {
					mark_config(pad_string, i, j);
				}
			}
		}
	}

	for (i = 0; i < MAXSTYLUS; i++) {
		for (j = 0; j < MAXMODEL * MAXPAD; j++){
			if (stylus_xdevice[i][j]) {
				mark_device(stylus_string, i, j, "");
			}
		}
	}

}

/* End Code */

