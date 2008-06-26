/*
 globals-conf.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
 
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

#include "globals-conf.h"

Display *display;

char *our_prog_name;
char *our_prog_version = "0.2.4"; /* Our version. Remember to change it! */
char *total_config_dir;
char *total_config_file;
char *total_pid_file;
char *total_error_file;
char *config_dir = "/.expresskeys";	/* This is where in the user home */
char *config_file = "/expresskeys.conf";/* directory we will look, */
char *pid_file = "/expresskeys.pid"; 	/* and what the different status */
char *error_file = "/error_conf.log"; 	/* files are called */

int screen;
int be_verbose = 0; /* Run silently per default */
int num_list = 0;

/* Allocate space for a list of program definitions (config file based) */

struct program external_list [MAXRECORDS];

/* End Code */

