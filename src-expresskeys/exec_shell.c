/*
 exec_shell.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.

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

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Function uses the simple "system" command to execute another program in
 a shell, waiting for it to return. Here it calls the "xsetwacom" from
 the linuxwacom.sourceforge.net driver package. Observe that it doesn't
 check whether the program gets executed or not, just that the shell gets
 forked without an error.

 This sample utilization of xsetwacom alters the pen sensitivity to pressure
 just like setting the Feel -> Sensitivity in wacomcpl (the tcl utility
 program) between Soft (lower) and Firm (higher) values (1 to 7, 4 is
 default). Both STYLUS1_CURVE_DOWNWARD and STYLUS1_CURVE_UPWARD flip over in a
 carousel fashion at the top and bottom values.
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int call_xsetwacom(int num)
{

	char buffer [MAXBUFFER];
	char curve [12];

	if ((num >= STYLUS1_CURVE_DOWNWARD) &&
		(num <= STYLUS1_CURVE_UPWARD)) {
		if (stylus1_info) {
			if (num == STYLUS1_CURVE_DOWNWARD) {
				if (padstylus1_presscurve > 1) {
					padstylus1_presscurve--;
				} else {
					padstylus1_presscurve = 7;
				}
			}
			if (num == STYLUS1_CURVE_DEFAULT) {
				padstylus1_presscurve = 4;
			}
			if (num == STYLUS1_CURVE_UPWARD) {
				if (padstylus1_presscurve < 7) {
					padstylus1_presscurve++;
				} else {
					padstylus1_presscurve = 1;
				}
			}
			switch (padstylus1_presscurve) {

				case 1:
				sprintf(curve, "0 75 25 100");
				break;

				case 2:
				sprintf(curve, "0 50 50 100");
				break;

				case 3:
				sprintf(curve, "0 25 75 100");
				break;

				case 4:
				sprintf(curve, "0 0 100 100");
				break;

				case 5:
				sprintf(curve, "25 0 100 75");
				break;

				case 6:
				sprintf(curve, "50 0 100 50");
				break;

				case 7:
				sprintf(curve, "75 0 100 25");
				break;

				default:
				sprintf(curve, "0 0 100 100");
				break;

			}
			snprintf(buffer, MAXBUFFER, "xsetwacom set %s PressCurve %s", stylus1_name, curve);
			if ((system(buffer)) == NON_VALID) {
				fprintf(stderr, "Failed to fork a shell for xsetwacom!\n");
			} else {
				if (be_verbose) {
					fprintf(stderr, "%s Sensitivity = %d PressCurve = %s\n", stylus1_name, padstylus1_presscurve, curve);
				}
			}
		}
	}
	return 0;

}

/* End Code */

