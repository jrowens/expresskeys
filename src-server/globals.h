/*
 globals.h -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
 
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
#define PEN_CURVE_DOWNWARD 1001	/* Pen Sensitivity. Sample xsetwacom usage */
#define PEN_CURVE_DEFAULT 1002
#define PEN_CURVE_UPWARD 1003
#define TOGGLE_PEN 999	/* String signaling the key's use as a mode toggle */
#define MAXRECORDS 64	/* Max program definitions to handle (enough? ;-) */
#define MAXFIELDS 25	/* Max entries (minus program name) in each record */
#define MAXBUFFER 160	/* Scratch buffer. Two full terminal lines */
#define CONFIG_VERSION 2 /* Config file version - for future format changes */

/* Our global variables */
extern char *our_prog_name;	/* This program's file name */
extern char *pen_name;		/* Identifier as set in the X config file */
extern char *total_config_dir;	/* The "~/" of the user, plus our config_dir */
extern char *total_config_file;	/* total_config_dir plus config_file */
extern char *total_pid_file;	/* total_config_dir plus pid_file */
extern char *total_error_file;	/* total_config_dir plus error_file */
extern char *config_dir;	/* Set to a dot directory: .expresskeys */
extern char *config_file;	/* Set to expresskeys.conf */
extern char *pid_file;		/* Set to expresskeys.pid */
extern char *error_file;	/* Set to error.log */

extern int screen;	/* Active screen. An X thing */
extern int num_list;	/* Number of programs we currently handle */

extern int go_daemon;	/* Flag to see if this program is in daemon mode */
extern int be_verbose;	/* Flag to see if we should be spitting out info */
extern int reread_config; /* Flag for memory release if redoing the config */
extern int handle_pen;	/* Flag (main switch) to see if a pen is handled */
extern int pen_mode;	/* Flag to keep track of the pen mode we are in */
extern int pen_open;	/* Flag to make sure we only open the pen once */
extern int pen_presscurve; /* Pen Sensitivity. Sample xsetwacom usage */

extern int motion_type;		/* Event type to keep track of - Touch Strips */
extern int button_press_type;	/* Event type to keep track of - Pad Buttons */
extern int button_release_type;	/* Event type to keep track of - Pad Buttons */

/* Global X functions */
extern Display *display;	/* Active display. An X thing */
extern XDeviceInfo *pad_info_block;	/* Base address - searching pad info */
extern XDeviceInfo *pen_info_block; /* Base address - searching pen info */
extern XDeviceInfo *pad_info;	/* Keeps all X knowledge of the pad device */
extern XDeviceInfo *pen_info;	/* Keeps all X knowledge of the pen device */
extern XDevice *pad_device;	/* The actual pointer to the pad device */
extern XDevice *pen_device;	/* The actual pointer to the pen device */

/* Our global (internal) functions */
extern int write_file_config_header(FILE *fp);
extern int write_file_config(int *ip, FILE *fp);
extern int read_file_config(int *ip, FILE *fp);
extern int get_device_info(Display *display, XDeviceInfo *info, char *name);
extern int register_events(Display *display, XDeviceInfo *pad_info, char *name);
extern int toggle_pen_mode(Display *display, char *name);
extern int use_events(Display *display);
extern int call_xsetwacom(int num);
extern void exit_on_error(FILE *fp, char *string1, char *string2, char *string3);
extern void re_read_file_config(int signum);
extern void clean_up_exit(int signum);

/* Our global structures */
/* The internal_list is initialized in globals.c
   The external_list is initialized from file reads */
extern struct program {
	char *class_name;
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

/* End Code */

