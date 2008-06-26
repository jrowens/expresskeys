/*
 globals.h -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.

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

/* Standard Program includes */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
/* For the basename call */
#include <libgen.h>
/* For the daemon, getpid and unlink calls */
#include <unistd.h>
/* For the mkdir call */
#include <sys/stat.h>
/* For the mkdir, getpid and kill calls */
#include <sys/types.h>
/* For the signal and kill calls */
#include <signal.h>

/* Standard X includes */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/* Special X extensions we use */
#include <X11/extensions/XInput.h>
#include <X11/extensions/XTest.h>

/* Our internal defines */
#define EXIT_OK 0
#define EXIT_KO 1
#define NON_VALID -1
enum {                 /* Strings signaling key's use as a mouse button */
	MOUSE_BUTTON_MIN = 990,
	MOUSE_1,
	MOUSE_2,
	MOUSE_3,
	MOUSE_4,
	MOUSE_5,
	MOUSE_6,
	MOUSE_7,
	MOUSE_BUTTON_MAX
};
#define STYLUS1_CURVE_DOWNWARD 1001	/* Pen Sensitivity. Sample xsetwacom usage */
#define STYLUS1_CURVE_DEFAULT 1002
#define STYLUS1_CURVE_UPWARD 1003
#define TOGGLE_STYLUS1 999	/* String signaling the key's use as a mode toggle */
#define MAXRECORDS 64	/* Max program definitions to handle (enough? ;-) */
#define MAXBUFFER 160	/* Scratch buffer. Two full terminal lines */
#define CURVEMAX 16	/* PressCurve max number length, eg "100 100 100 100" + NULL */

/* Our global variables */
extern char *configstring; /* Config file string preceding config file version nr. */
extern char *configversion; /* For future format changes. Define it in globals.c! */
extern char *pad1idstring;		/* Marker preceding the xorg.conf device Identifier */
extern char *stylus1idstring;	/* Marker preceding the xorg.conf device Identifier */
extern char *our_prog_name;	/* This program's file name */
extern char *our_prog_version; /* Our program version. Change it in globals.c! */
extern char *pad1_name;			/* Identifier as set in the X config file */
extern char *pad1_autoname;		/* Picking the name directly from X */
extern char *stylus1_name;		/* Identifier as set in the X config file */
extern char *stylus1_autoname;	/*  Picking the name directly from X */
extern char *total_config_dir;	/* The "~/" of the user, plus our config_dir */
extern char *total_config_file;	/* total_config_dir plus config_file */
extern char *total_pid_file;	/* total_config_dir plus pid_file */
extern char *total_error_file;	/* total_config_dir plus error_file */
extern char *config_dir;	/* Set to a dot directory: .expresskeys */
extern char *config_file;	/* Set to NULL */
extern char *pid_file;		/* Set to expresskeys.pid */
extern char *error_file;	/* Set to error.log */
extern char *config_file_padless;
extern char *config_file_intuos3;
extern char *config_file_graphire4;


extern int userconfigversion; /* Keep track of which format to parse */
extern int config3headerfields; /* Nr. of fields in the header of a configversion 3 file */
extern int configheaderfields; /* The decided number of fields to parse in each file */
extern int config3fields; /* Nr. of fields in a configversion 3 record: globals.c! */
extern int configfields; /* The decided number of fields to parse in each record */
extern int screen;	/* Active screen. An X thing */
extern int num_list;	/* Number of programs we currently handle */

extern int just_exit;	/* Flag to see if we should exit the program immediately */
extern int go_daemon;	/* Flag to see if this program is in daemon mode */
extern int second_instance; /* Flag it if a second instance of a daemon run is tried */
extern int be_verbose;	/* Flag to see if we should be spitting out info */
extern int reread_config; /* Flag for memory release if re-reading the config */
extern int stylus1_mode;	/* Flag to keep track of the stylus mode we are in */
extern int padstylus1_presscurve; /* Pen Sensitivity. Sample xsetwacom usage */

extern int motion_type;		/* Event type to keep track of - Touch Strips */
extern int button_press_type;	/* Event type to keep track of - Pad/Stylus Buttons */
extern int button_release_type;	/* Event type to keep track of - Pad Buttons */
extern int proximity_in_type;	/* Event type to keep track of - Stylus In */

/* Our global (internal) functions */
extern int write_file_config_header(FILE *fp);
extern int write_file_config(int *ip, FILE *fp);
extern int read_file_config(int *ip, int *ihp, FILE *fp);
/* get_device_info returns a long to satisfy x86_64 systems */
extern long get_device_info(Display *display, XDeviceInfo *info, char *name);
extern int register_events(Display *display, XDeviceInfo *dev_info, char *name);
extern int toggle_stylus1_mode(Display *display, char *name);
extern int use_events(Display *display);
extern int call_xsetwacom(int num);
extern void exit_on_error(FILE *fp, char *string1, char *string2, char *string3);
extern void re_read_file_config(int signum);
extern void status_report(int signum);
extern void clean_up_exit(int signum);

/* Global X structures */
extern Display *display;	/* Active display. An X thing */
extern XDeviceInfo *pad1_info_block;	/* Base address - searching pad info */
extern XDeviceInfo *stylus1_info_block; /* Base address - searching pen info */
extern XDeviceInfo *pad1_info;	/* Keeps all X knowledge of the pad device */
extern XDeviceInfo *stylus1_info;	/* Keeps all X knowledge of the pen device */
extern XDevice *pad1_device;	/* The actual pointer to the pad device */
extern XDevice *stylus1_device;	/* The actual pointer to the pen device */

/* Our global structures */
extern struct program *default_program; /* Pointer to the "default" program record */

/* The internal_list is initialized in globals.c
   The external_list is initialized from file reads */
extern struct program {
	char *class_name;
	char *stylus1_presscurve;
	int handle_touch;
	int l_touch_up;
	int l_touch_up_plus;
	int l_touch_down;
	int l_touch_down_plus;
	int r_touch_up;
	int r_touch_up_plus;
	int r_touch_down;
	int r_touch_down_plus;
	int key_9;
	int key_9_plus;
	int key_10;
	int key_10_plus;
	int key_11;
	int key_11_plus;
	int key_12;
	int key_12_plus;
	int key_13;
	int key_13_plus;
	int key_14;
	int key_14_plus;
	int key_15;
	int key_15_plus;
	int key_16;
	int key_16_plus;
}internal_list [], external_list [];

/* This list is initialized in globals.c */
extern struct configstrings {
	char *h_class_name;
	char *h_stylus1_presscurve;
	char *h_handle_touch;
	char *h_l_touch_up;
	char *h_l_touch_up_plus;
	char *h_l_touch_down;
	char *h_l_touch_down_plus;
	char *h_r_touch_up;
	char *h_r_touch_up_plus;
	char *h_r_touch_down;
	char *h_r_touch_down_plus;
	char *h_key_9;
	char *h_key_9_plus;
	char *h_key_10;
	char *h_key_10_plus;
	char *h_key_11;
	char *h_key_11_plus;
	char *h_key_12;
	char *h_key_12_plus;
	char *h_key_13;
	char *h_key_13_plus;
	char *h_key_14;
	char *h_key_14_plus;
	char *h_key_15;
	char *h_key_15_plus;
	char *h_key_16;
	char *h_key_16_plus;
}human_readable [];

/* End Code */

