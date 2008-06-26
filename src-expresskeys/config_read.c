/*
 config_read.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.
 
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

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Function reads a configuration file containing program names and
 definitions of which keys that should be mapped to pad buttons and
 touch strips. It takes a file pointer and a pointer to a global
 structure as input. Returns nothing unless an error occured.
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int read_file_config(int *ip, FILE *fp)
{

	struct program *p;
	p = (void *)*ip;

	int i;
	int num_record = 0;
	int num_field = 0;
	
	int *field_index = 0;

	char buffer [MAXBUFFER];
	char line_buffer [MAXBUFFER];
	const char ignore[] = " \t\n";
	const char *delimiter_first, *delimiter_last;
	char *token;

/* Previously allocated memory for the program names must be released
   on subsequent reads of the config file. At program start the flag is 0 */

	if (reread_config) {
		for (i = 0; i < num_list; i++, p++) {
			free(p->class_name);
		}
		p = (void *)*ip;
	}
	reread_config = 1;
	num_list = 0;

/* Begin by making sure that the Config File Version number is present before
   a record begins. We exit if it's lacking or the number doesn't match up */

/* FIXME Unpredictable behaviour on lines longer than MAXBUFFER etc. See BUGS */

	while ((fgets(line_buffer, MAXBUFFER, fp)) != NULL) {
		if ((delimiter_first = (strchr(line_buffer, '#'))) != NULL) {
			strncpy(buffer, line_buffer, ((delimiter_first) - (line_buffer)));
			strncpy(buffer + ((delimiter_first) - (line_buffer)), "\0", 1);
			strcpy(line_buffer, buffer);
		}
		if ((delimiter_first = (strchr(line_buffer, '%'))) != NULL &&
		(delimiter_last = (strrchr(line_buffer, '%'))) != NULL &&
		(delimiter_last != delimiter_first)) {
			return 3;
		}
		if ((delimiter_first = (strchr(line_buffer, ':'))) != NULL) {
			strcpy(buffer, line_buffer);
			token = strtok(buffer, ":");
			token = strtok(NULL, ignore);
			strcpy(buffer, token);
			if (be_verbose) {
				fprintf(stderr, "Config File Version on disk = %s Expected = %d\n", buffer, CONFIG_VERSION);
			}
			if ((atoi(buffer)) != CONFIG_VERSION) {
				return 3;
			}
			break;
		}
	}

/* Read the config file in one go from top to bottom. Parse out the info
   between record start "%%" and record end "%%". Recognize field beginnings
   by a colon ":". Each full record is written to a global memory structure,
   while partial records are discarded and excessive fields are truncated.
   No sanity check on program names or keycode functionality is performed
   A global counter variable of full record instances is updated before the
   function exits to reflect the new state. */

/* FIXME Unpredictable behaviour on lines longer than MAXBUFFER etc. See BUGS */

	while ((fgets(line_buffer, MAXBUFFER, fp)) != NULL) {
		if (num_record < MAXRECORDS) {
			if ((delimiter_first = (strchr(line_buffer, '#'))) != NULL) {
				strncpy(buffer, line_buffer, ((delimiter_first) - (line_buffer)));
				strncpy(buffer + ((delimiter_first) - (line_buffer)), "\0", 1);
				strcpy(line_buffer, buffer);
			}
			if ((delimiter_first = (strchr(line_buffer, '%'))) == NULL &&
			(delimiter_last = (strrchr(line_buffer, '%'))) == NULL &&
			(delimiter_last == delimiter_first)) {
				if ((delimiter_first = (strchr(line_buffer, ':'))) != NULL) {
					if ((delimiter_first = (strchr(line_buffer, '"'))) != NULL) {
						strcpy(buffer, line_buffer);
						token = strtok(buffer, "\t\n");
						while ((delimiter_first = (strchr(token, '"'))) == NULL) {
							token = strtok(NULL, "\t\n");
						}
						if (((delimiter_last = (strrchr(token, '"'))) != NULL) &&
						(delimiter_last != delimiter_first)) {
							strncpy(buffer, delimiter_first+1, ((delimiter_last) - (delimiter_first + 1)));
							strncpy(buffer + ((delimiter_last) - (delimiter_first + 1)), "\0", 1);
							if ((p->class_name = (char *)malloc(strlen(buffer)+1)) == NULL) {
								return 2;
							}
							sprintf(p->class_name, "%s", buffer);
							if (be_verbose) {
								fprintf(stderr, "PGR RECNAME = %s\n", buffer);
							}
							field_index = &p->handle_touch;

/* FIXME Unpredictable behaviour on lines longer than MAXBUFFER etc. See BUGS */

							while ((fgets(line_buffer, MAXBUFFER, fp)) != NULL) {
								if ((delimiter_first = (strchr(line_buffer, '%'))) != NULL &&
								(delimiter_last = (strrchr(line_buffer, '%'))) != NULL &&
								(delimiter_last != delimiter_first)) {
									break;
								}
								if (num_field < MAXFIELDS) {
									if ((delimiter_first = (strchr(line_buffer, '#'))) != NULL) {
										strncpy(buffer, line_buffer, ((delimiter_first) - (line_buffer)));
										strncpy(buffer + ((delimiter_first) - (line_buffer)), "\0", 1);
										strcpy(line_buffer, buffer);
									}
									if ((delimiter_first = (strchr(line_buffer, ':'))) != NULL) {
										strcpy(buffer, line_buffer);
										token = strtok(buffer, ":");
										token = strtok(NULL, ignore);
										strcpy(buffer, token);
										*field_index = atoi(buffer);
										field_index++;
										num_field++;
									}
								}
							}
							if (num_field == MAXFIELDS) {
								num_record++;
								p++;
							} else {
								if (be_verbose) {
									fprintf(stderr, "%s skipped! (fields were too few)\n", p->class_name);
								}
								free(p->class_name);
							}
							num_field = 0;
						}
					}
				}
			}
		}
	}
	if (!num_record) {
		return 1;
	}
	num_list = num_record;

	if (be_verbose) {
		fprintf(stderr, "PGR RECORDS = %d\n", num_list);
	}

	return 0;
}

/* End Code */

