/*
 * globals.h -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
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

#include "UserConfig-Pen"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XTest.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define EXIT_OK 1
#define EXIT_KO 0
#define NON_VALID -1
#define TOGGLE_PEN 999
#define NUM_LIST (sizeof prog_list / sizeof prog_list[0])

extern Display *display;
extern int screen;

extern XDeviceInfo *info;
extern XDeviceInfo *pad_info;
extern XDeviceInfo *pen_info;
extern XDevice *pad_device;
extern XDevice *pen_device;

extern int pen_mode;
extern int pen_open;
extern int elder_rotation;
extern int old_rotation;
extern int elder_throttle;
extern int old_throttle;

extern int motion_type;
extern int button_press_type;
extern int button_release_type;

extern int get_device_info(Display *display, char *name);
extern int register_events(Display *display, XDeviceInfo *pad_info, char *name);
extern int toggle_pen_mode(Display *display, char *name);
extern int use_events(Display *display);

/* End Code */

