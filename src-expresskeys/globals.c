/*
 globals.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.

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

Display *display;
XDeviceInfo *pad1_info_block;
XDeviceInfo *stylus1_info_block;
XDeviceInfo *pad1_info;
XDeviceInfo *stylus1_info;
XDevice *pad1_device;
XDevice *stylus1_device;

char *configstring = "ConfigVersion"; /* Searchable string in the config file */
char *configversion = "3"; /* Our version. Remember to change it if necessary! */
char *pad1idstring = "Identifier1Pad";		/* Marker to bind tablet and config file */
char *stylus1idstring = "Identifier1Sty";	/* Marker that can't bind... */
char *our_prog_version = "0.3.2"; /* Our version. Remember to change it! */
char *our_prog_name;
char *pad1_name = 0;
char *pad1_autoname = "pad";
char *stylus1_name = 0;
char *stylus1_autoname = "stylus";
char *total_config_dir;
char *total_config_file;
char *total_pid_file;
char *total_error_file;
char *total_status_file;
char *config_dir = "/.expresskeys";	/* This is where in the user home */
char *config_file = 0;				/* directory we'd like to reside, */
char *pid_file = "/expresskeys.pid"; 	/* and what our different status */
char *error_file = "/error.log"; 	/* files should be called */
char *status_file = "/status.log";
char *config_file_padless = "/padless.conf1";
char *config_file_intuos3 = "/intuos3.conf1";
char *config_file_graphire4 = "/graphire4.conf1";

int userconfigversion = 0; /* Keep track of which format the user has */
int config3headerfields = 3; /* Number of fields in the header of a configversion 3 file */
int configheaderfields = 0; /* How many header fields to actually parse in each file */
int config3fields = 27; /* Number of fields in a configversion 3 record */
int config3gr4fields = 11; /* Number of fields if tablet is a Graphire4 */
int configfields = 0; /* How many fields to actually parse in each record */
int screen;			/* Active screen. An X thing */
int is_graphire4 = 0;	/* Only flag if it is so */
int just_exit = 0;	/* Do not terminate the program immediately */
int go_daemon = 0;	/* Do not become a daemon without a command */
int be_verbose = 0; /* Run silently per default */
int reread_config = 0;	/* No memory should be freed on the first read */
int stylus1_mode = 1;	/* Pen initially in Absolute mode. Real value discovered later */
int padstylus1_presscurve = 4; /* Stylus default Sensitivity. Sample xsetwacom usage */

int motion_type = NON_VALID;
int button_press_type = NON_VALID;
int button_release_type = NON_VALID;
int proximity_in_type = NON_VALID;

struct program *default_program; /* Pointer to the "default" program record */

/* Allocate space for a list of program definitions (config file based)
   Also initialize a small list that is written out if no config file exists */

struct program external_list [MAXRECORDS];
struct program internal_list[] = {
/*	Name	stylus1_presscurve	handle_touch */
{"default",	"0 0 100 100",	1,
/*		l_touch_up	l_touch_up_plus	l_touch_down	l_touch_down_plus */
		994,		0,		995,		0,
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

/*	Name	stylus1_presscurve	handle_touch */
{"Gimp", "0 0 100 100",	1,
/*		l_touch_up	l_touch_up_plus	l_touch_down	l_touch_down_plus */
		20,		0,		61,		0,
/*		r_touch_up	r_touch_up_plus	r_touch_down	r_touch_down_plus */
		64,		20,		64,		61,
/*		key_9		key_9_plus	key_10		key_10_plus */
		50,		0,		64,		0,
/*		key_11		key_11_plus	key_12		key_12_plus */
		37,		0,		65,		0,
/*		key_13		key_13_plus	key_14		key_14_plus */
		37,		29,		37,		52,
/*		key_15		key_15_plus	key_16		key_16_plus */
		999,		0,		65,		0	},

/*	Name	stylus1_presscurve	handle_touch */
{"Blender",	"0 0 100 100",	1,
/*		l_touch_up	l_touch_up_plus	l_touch_down	l_touch_down_plus */
		102,		0,		100,		0,
/*		r_touch_up	r_touch_up_plus	r_touch_down	r_touch_down_plus */
		98,		0,		104,		0,
/*		key_9		key_9_plus	key_10		key_10_plus */
		37,		0,		9,		0,
/*		key_11		key_11_plus	key_12		key_12_plus */
		50,		0,		23,		0,
/*		key_13		key_13_plus	key_14		key_14_plus */
		37,		29,		37,		52,
/*		key_15		key_15_plus	key_16		key_16_plus */
		999,		0,		65,		0	},

/*	Name	stylus1_presscurve	handle_touch */
{"XTerm",	"0 0 100 100",	0,
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
		999,		0,		0,		0	}
};
/* The number of programs (num_list) must be computed here. After the struct */
int num_list = (sizeof internal_list / sizeof internal_list[0]);

/* These are the keywords prefacing a value in the Intuos3 config file */
struct configstrings human_readable[] = {
{"ProgramName", "Stylus1PressCurve", "HandleTouchStrips",
"LeftPadTouchUp", "LeftPadTouchUpPlus",
"LeftPadTouchDown", "LeftPadTouchDownPlus",
"RightPadTouchUp", "RightPadTouchUpPlus",
"RightPadTouchDown", "RightPadTouchDownPlus",
"LeftPadButton9", "LeftPadButton9Plus",
"LeftPadButton10", "LeftPadButton10Plus",
"LeftPadButton11", "LeftPadButton11Plus",
"LeftPadButton12", "LeftPadButton12Plus",
"RightPadButton13", "RightPadButton13Plus",
"RightPadButton14", "RightPadButton14Plus",
"RightPadButton15", "RightPadButton15Plus",
"RightPadButton16", "RightPadButton16Plus" }
};

/* These are the keywords prefacing a value in the Graphire4 config file */
struct configstrings gr4_human_readable[] = {
{"ProgramName", "Stylus1PressCurve", "HandleScrollWheel",
"ScrollWheelUp", "ScrollWheelUpPlus",
"ScrollWheelDown", "ScrollWheelDownPlus",
"dummy", "dummy", "dummy", "dummy", "LeftButton", "LeftButtonPlus",
"RightButton", "RightButtonPlus", "dummy", "dummy", "dummy", "dummy",
"dummy", "dummy", "dummy", "dummy", "dummy", "dummy", "dummy", "dummy" }
};

/* End Code */

