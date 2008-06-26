/*
 globals.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
 
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

Display *display;
XDeviceInfo *pad_info_block;
XDeviceInfo *pen_info_block;
XDeviceInfo *pad_info;
XDeviceInfo *pen_info;
XDevice *pad_device;
XDevice *pen_device;

char *our_prog_name;
char *pen_name;
char *total_config_dir;
char *total_config_file;
char *total_pid_file;
char *total_error_file;
char *config_dir = "/.expresskeys";	/* This is where in the user home */
char *config_file = "/expresskeys.conf";/* directory we'd like to reside, */
char *pid_file = "/expresskeys.pid"; 	/* and what our different status */
char *error_file = "/error.log"; 	/* files should be called */

int screen;
int go_daemon = 0;	/* Do not become a daemon without a command */
int be_verbose = 0; /* Run silently per default */
int reread_config = 0;	/* No memory should be freed on the first read */
int handle_pen = 0;	/* Pen should not be handled per default */
int pen_mode = 1;	/* Assume pen is in Absolute mode initially */
int pen_open = 0;	/* Pen device hasn't been opened */

int motion_type = NON_VALID;
int button_press_type = NON_VALID;
int button_release_type = NON_VALID;

/* Allocate space for a list of program definitions (config file based)
   Also initialize a small list that is written out if no config file exists */

struct program external_list [MAXRECORDS];
struct program internal_list[] = {
/*	Name	handle_touch */
{"default",	0,
/*		l_touch_up	l_touch_up_plus	l_touch_down	l_touch_down_plus */
		98,		0,		104,		0,
/*		r_touch_up	r_touch_up_plus	r_touch_down	r_touch_down_plus */
		102,		0,		100,		0,
/*		key_9		key_9_plus	key_10		key_10_plus */
		50,		0,		64,		0,
/*		key_11		key_11_plus	key_12		key_12_plus */
		37,		0,		65,		0,
/*		key_13		key_13_plus	key_14		key_14_plus */
		50,		0,		64,		0,
/*		key_15		key_15_plus	key_16		key_16_plus */
		37,		0,		65,		0	},

/*	Name	handle_touch */
{"Gimp",	1,
/*		l_touch_up	l_touch_up_plus	l_touch_down	l_touch_down_plus */
		20,		0,		61,		0,
/*		r_touch_up	r_touch_up_plus	r_touch_down	r_touch_down_plus */
		20,		0,		61,		0,
/*		key_9		key_9_plus	key_10		key_10_plus */
		50,		0,		64,		0,
/*		key_11		key_11_plus	key_12		key_12_plus */
		37,		0,		65,		0,
/*		key_13		key_13_plus	key_14		key_14_plus */
		50,		0,		64,		0,
/*		key_15		key_15_plus	key_16		key_16_plus */
		37,		0,		65,		0	},

/*	Name	handle_touch */
{"Blender",	1,
/*		l_touch_up	l_touch_up_plus	l_touch_down	l_touch_down_plus */
		102,		0,		100,		0,
/*		r_touch_up	r_touch_up_plus	r_touch_down	r_touch_down_plus */
		98,		0,		104,		0,
/*		key_9		key_9_plus	key_10		key_10_plus */
		37,		0,		9,		0,
/*		key_11		key_11_plus	key_12		key_12_plus */
		50,		0,		23,		0,
/*		key_13		key_13_plus	key_14		key_14_plus */
		50,		30,		30,		0,
/*		key_15		key_15_plus	key_16		key_16_plus */
		999,		0,		65,		0	},

/*	Name	handle_touch */
{"XTerm",	0,
/*		l_touch_up	l_touch_up_plus	l_touch_down	l_touch_down_plus */
		0,		0,		0,		0,
/*		r_touch_up	r_touch_up_plus	r_touch_down	r_touch_down_plus */
		0,		0,		0,		0,
/*		key_9		key_9_plus	key_10		key_10_plus */
		0,		0,		0,		0,
/*		key_11		key_11_plus	key_12		key_12_plus */
		0,		0,		0,		0,
/*		key_13		key_13_plus	key_14		key_14_plus */
		0,		0,		0,		0,
/*		key_15		key_15_plus	key_16		key_16_plus */
		999,		0,		0,		0	},
};
/* The number of programs (num_list) must be computed here. After the struct */
int num_list = (sizeof internal_list / sizeof internal_list[0]);

/* End Code */

