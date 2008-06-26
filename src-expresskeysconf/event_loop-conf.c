/*
 event_loop-conf.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
 
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

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Function waits perpetually for the X server
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int use_events(Display *display)
{

	return 0;
	/*exit(EXIT_OK);*/
}

/* End code */

