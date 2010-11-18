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

#ifndef DEFINES_H_
#define DEFINES_H_

/* Declare exit_on_error here since it is used almost everywhere: */
extern void exit_on_error(FILE* errorfp, char* string1, const char* string2,
							const char* string3);

/* Obvious: */
#define NON_VALID -1
/* A bit more than two full terminal lines for buffers. It should also handle
the longest keyword (currently RightPadTouchDown + a space = 18) + 32 keycodes
of max 4 digits length + a space between each of them (sum 178): */
#define MAXBUFFER 180
/* 'Models': padless, Graphire4 BlueTooth, Graphire4, Intuos3 small, Intuos3, Bee, Bamboo, Cintiq 21UX, Cintiq 21UX2 */
#define MAXMODEL 9
/* Max number of tablets per 'model': */
#define MAXPAD 3
/* Max number of styli per tablet: */
#define MAXSTYLUS 2
/* PressCurve max number length, eg "100 100 100 100" + \0 */
#define CURVEMAX 16
/* Rotate max string length, eg NONE + \0 */
#define ROTATEMAX 5
/* Max program definitions to handle for each tablet: */
#define MAXRECORDS 32
/* Prevent too easy a DOS of the system by creating humongous configuration
 files. Each file read is limited to MAXRECORDS * LINELIMIT lines. Since a
 normal I3 record sports around 50 lines, 20 times that much should suffice: */
#define LINELIMIT 1000
/* Max number of keys to simulate for each button/touchstrip/scrollwheel: */
#define MAXKEYS 32

/* OBSERVE: I've set a minimum keycode of 9 (ESC) and an upper maximum for
 keycode values as 1023 in transfer_keycode(). There are other 'out-of-bounds'
 areas that we also filter away: 256-263, 512-519, 768-775. X would crash
 if they were used: 2 BadValue (integer parameter out of range for operation) */

/* The fake keycodes below will never reach X, but they are still filtered */

/* Values signaling button's use as a mouse button: */
enum {
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
/* Value signaling button's use as a stylus Mode toggle (Absolute/Relative): */
#define TOGGLE_STYLUS 999
/* Values signaling button's use as a stylus PressCurve changer: */
#define STYLUS_CURVE_DOWNWARD 1001
#define STYLUS_CURVE_DEFAULT 1002
#define STYLUS_CURVE_UPWARD 1003
/* Values signaling button's use as a tablet Rotate changer: */
#define TABLET_ROTATE_NONE 1004
#define TABLET_ROTATE_HALF 1005
#define TABLET_ROTATE_CW 1006
#define TABLET_ROTATE_CCW 1007
#define TABLET_ROTATE_CW_AGAIN 1008
#define TABLET_ROTATE_CCW_AGAIN 1009

#endif /*DEFINES_H_*/

/* End Code */

