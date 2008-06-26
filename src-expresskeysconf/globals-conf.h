/*
 globals-conf.h -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
 
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
/* For the mkdir call */
#include <sys/stat.h>
/* For the mkdir and kill calls */
#include <sys/types.h>
/* For the signal and kill calls */
#include <signal.h>

/* Standard X includes */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/* Our internal defines */
#define EXIT_OK 0
#define EXIT_KO 1
#define NON_VALID -1
#define MAXRECORDS 64	/* Max program definitions to handle (enough? ;-) */
#define MAXFIELDS 25	/* Max entries (minus program name) in each record */
#define MAXBUFFER 160	/* Scratch buffer. Two full terminal lines */
#define CONFIG_VERSION 2 /* Config file version - for future format changes */

/* Our global variables */
extern char *our_prog_name;	/* This program's file name */
extern char *our_prog_version; /* Our version. Change it in globals-conf.c! */
extern char *total_config_dir;	/* The "~/" of the user, plus our config_dir */
extern char *total_config_file;	/* total_config_dir plus config_file */
extern char *total_pid_file;	/* total_config_dir plus pid_file */
extern char *total_error_file;	/* total_config_dir plus error_file */
extern char *config_dir;	/* Set to a dot directory: .expresskeys */
extern char *config_file;	/* Set to expresskeys.conf */
extern char *pid_file;		/* Set to expresskeys.pid */
extern char *error_file;	/* Set to error_conf.log */

extern int screen;	/* Active screen. An X thing */
extern int num_list;	/* Number of programs we currently handle */

extern int be_verbose;	/* Flag to see if we should be spitting out info */

/* Global X functions */
extern Display *display;	/* Active display. An X thing */

/* Our global (internal) functions */
extern int read_file_config(int *ip, FILE *fp);
extern int use_events(Display *display);
extern void exit_on_error(FILE *fp, char *string1, char *string2, char *string3);
extern void clean_up_exit(int signum);

/* Our global structures */
/* The external_list is initialized from a file read */
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
}external_list [];

/* End Code */

