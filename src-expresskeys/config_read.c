/*
 config_read.c -- Support ExpressKeys & Touch Strips on a Wacom Intuos3 tablet.

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
 Function reads a configuration file containing program names and definitions
 of which keys that should be mapped to pad buttons and touch strips. It takes
 a file pointer and two pointers to global structures as input (data and
 keywords). Returns nothing unless an error occured.
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int read_file_config(int *ip, FILE *fp)
{

	struct program *p;
	struct configstrings *hp;

	/* Convert to long for x86_64 systems */
	p = (void *)(long)*ip;

	if (is_graphire4) {
		hp = gr4_human_readable;
	} else {
		hp = human_readable;
	}

	int i;
	int j;
	int klen;
	int flen;
	int slen;
	int num_record = 0;
	int num_field = 0;
	int sane = 0;
	int programname_alloced = 0;
	int presscurve_alloced = 0;
	int re_aligned = 0;

	int *field_index = 0;
	const char *string_index = 0;

	int ifilter;
	unsigned char cfilter;

	char read_buffer [MAXBUFFER];
	char write_buffer [MAXBUFFER];

	const char *newline = 0;
	const char *comment = 0;
	const char *keyword = 0;
	const char *quote = 0;
	const char *record = 0;
	const char *field = 0;

/* Previously allocated memory for all the config record names and stylus
   PressCurve must be released on subsequent reads of the file. At program
   start the flag is 0 */

	if (reread_config) {
		for (i = 0; i < num_list; i++, p++) {
			free(p->stylus1_presscurve);
			free(p->class_name);
		}
		/* Convert to long for x86_64 systems */
		p = (void *)(long)*ip;
	}
	reread_config = 1;
	num_list = 0;

/* Begin by making sure that the Config File version number is present before
   a record begins. We exit if it's lacking or the number doesn't match up.
   Also exit if the line was too long to handle */

/* Since parsing of a file is a complex action, this comment is almost line by line:
 1) Read until end of file or until the first program record is encountered.
 2) No newline character means that the line extended beyond MAXBUFFER bytes - exit.
 3) The configstring must reside to the _left_ of an eventual comment.
 4) A pointer to the string is incremented by string length - to step past it.
 5) Next the pointer is stepped past any following whitespace, landing at the value.
 6) Whitespace and newline is computed away, and the value written to a null-terminated buffer.
 7) Compare... */

/* Version loop */
	while (((fgets(read_buffer, MAXBUFFER, fp)) != NULL) && ((strstr(read_buffer, "%%")) == NULL)) {
		if ((newline = (strchr(read_buffer, '\n'))) != NULL) {
			if ((keyword = (strstr(read_buffer, configstring))) != NULL) {
				if (((comment = (strchr(read_buffer, '#'))) != NULL) && (comment < keyword)) {
					keyword = 0;
				}
				if (keyword) {
					keyword = (keyword+(strlen(configstring)));
					keyword = (keyword+(strspn(keyword, " \t")));

					flen = strcspn(keyword, " \t\n#");
					strncpy(write_buffer, keyword, flen);
					strncpy(write_buffer + flen, "\0", 1);

					if (be_verbose) {
						fprintf(stderr, "%s-user = %s\n", configstring, write_buffer);
						fprintf(stderr, "%s-ours = %s\n", configstring, configversion);
					}
					if (*write_buffer != *configversion) {
						if (atoi(write_buffer) < 3) {
							return 3;	/* Config file version is too old to handle */
						}
					}
					userconfigversion = atoi(write_buffer);
					if (userconfigversion == 3) {
						if (stylus1_info) {
							configfields = 2; /* Users with only a 'stylus' device */
						}
						if (pad1_info) {
							configfields = config3fields; /* Full 'pad' and 'stylus' */
						}
						if (is_graphire4) {
							configfields = config3gr4fields; /* Tiny 'pad' and 'stylus' */
						}
						configheaderfields = config3headerfields;
					} else {
						return 3;		/* Add handling when next version exist! */
					}
					if (be_verbose) {
						fprintf(stderr, "ConfigHeaderFields = %d\n", configheaderfields);
					}
					break;				/* Found a compatible config file */
				}
			}
		} else {
			return 4;					/* Line was too long to parse */
		}
	}
/* End version loop */
	if (keyword == NULL) {
		return 3;						/* End of file or a record begins */
	}

/* Read the config file in one go from top to bottom. Parse out the info
   between one record separator "%%" and the next. Recognize field beginnings
   by a keyword, for example HandleTouchStrips. Each full record is written
   to a global memory structure, while partial records are discarded and
   excessive fields are truncated. No sanity check on program names is
   performed (except for the mandatory "default") and only a basic control
   of the keycodes is in place - it filters out anything below 9 [Esc].
   A global counter variable of full record instances is updated before the
   function exits to reflect the new state. */

/* FIXME Doesn't discard duplicate keywords and is thus prone to become out of sync in such cases */

/* Fake that we've already read a line with a record separator, so that the while loop can begin */

	sprintf(read_buffer, "%s", "%%\n");

/* Record loop */
	while ((num_record < MAXRECORDS) && ((record = (strstr(read_buffer, "%%"))) != NULL)) {
		if ((newline = (strchr(read_buffer, '\n'))) != NULL) {
			if (((comment = (strchr(read_buffer, '#'))) != NULL) && (comment < record)) {
				record = 0;
				sprintf(read_buffer, "%s", "%%\n");	/* <-- Since no new reads from the file */
			}										/* is performed in this part of the loop, */
			if (record) {							/* we break out by erasing an encountered */
				num_field = 0;                      /* commented record separator. We must */
				programname_alloced = 0;					/* write an actual separator here! */
				presscurve_alloced = 0;

/* Field loop*/	while ((((fgets(read_buffer, MAXBUFFER, fp)) != NULL) && ((record = (strstr(read_buffer, "%%"))) == NULL))
						|| (((comment = (strchr(read_buffer, '#'))) != NULL) && (comment < record))) {
					if ((newline = (strchr(read_buffer, '\n'))) != NULL) {

						keyword = hp->h_class_name;
						re_aligned = 0;

/* Keyword loop */		for (i = 0; i < configfields; i++) {

							klen = strlen(keyword);
							if ((field = (strstr(read_buffer, keyword))) != NULL) {
								flen = strcspn(field, " \t");
								if (flen == klen) {
									if (((comment = (strchr(read_buffer, '#'))) != NULL) && (comment < field)) {
										field = 0;
									}
									if ((field) && (num_field < configfields)) {

										field = (field+flen);
										field = (field+(strspn(field, " \t\"")));
										if ((quote = (strchr(field, '\"'))) != NULL) {
											flen = (quote - field);
										} else {
											flen = strcspn(field, " \t\n#");
										}
										strncpy(write_buffer, field, flen);
										strncpy(write_buffer + flen, "\0", 1);
/* Begin basic sanity checks */
										ifilter = atoi(write_buffer);
										cfilter = *write_buffer;

/* Users of boolean 0 or 1 */			if (keyword == hp->h_handle_touch) {
											if ((flen == 1) && (isdigit(cfilter))) {
												if ((ifilter != 0) && (ifilter != 1)) {
													sprintf(write_buffer, "%s", "0");
													if (be_verbose) {
														fprintf(stderr, "Only boolean 1 or 0 allowed in %s field <-- RESETTING T0 ZERO\n", keyword);
													}
												}
											} else {
												sprintf(write_buffer, "%s", "0");
												if (be_verbose) {
													fprintf(stderr, "Only boolean 1 or 0 allowed in %s field <-- RESETTING T0 ZERO\n", keyword);
												}
											}
/* FIXME Only a basic control of the keycodes is in place - it filters out anything below 9 [Esc] and above 0 */
										} else {
/* Users not in this if-condition */		if ((keyword != hp->h_class_name)
												&& (keyword != hp->h_stylus1_presscurve)) {
												if ((ifilter != 0) && (ifilter < 9)) {
													sprintf(write_buffer, "%s", "0");
													if (be_verbose) {
														fprintf(stderr, "Only a 9 [Esc] or above allowed in %s field <-- RESETTING T0 ZERO\n", keyword);
													}
												}
											}
										}
/* End basic sanity checks */
										if (keyword == hp->h_class_name) {
/* Memory allocation. Freed on reread */	if ((p->class_name = (char *)malloc(flen+1)) == NULL) {
												return 2;
											}
/* Write the field to memory structure */	sprintf(p->class_name, "%s", write_buffer);
											num_record++; /* Only count record if program name is present */
											num_field++;
											programname_alloced = 1;
										} else if (keyword == hp->h_stylus1_presscurve) {
/* Memory allocation. Freed on reread */		if ((p->stylus1_presscurve = (char *)malloc(flen+1)) == NULL) {
													return 2;
												}
/* Write the field to memory structure */		sprintf(p->stylus1_presscurve, "%s", write_buffer);
												num_field++;
												presscurve_alloced = 1;
										} else {
/* Walking through members of a structure only using pointers, when the member name
 in effect is unknown, still eludes me. This hack must suffice until the fog clears.
 What I _do_ know is that the data structure begins with two char members, so
 subtract them from the configfields number loop. They are already handled further
 up in the code. The rest of the data structure consists of int members, where
 a simple increment will walk correctly (while keeping in sync with the string
 structure through string length+1). */
											field_index = &p->handle_touch;
											string_index = hp->h_handle_touch;
											for (j = 0; j <= (configfields - 2); j++) {
												slen = strlen(string_index);
												if ((strcmp(string_index, keyword)) == 0) {
/* Time for another ugly hack...*/					if (is_graphire4) {
														if ((strcmp(keyword, "LeftButton")) == 0) {
															field_index = &p->key_9;
														}
														if ((strcmp(keyword, "LeftButtonPlus")) == 0) {
															field_index = &p->key_9_plus;
														}
														if ((strcmp(keyword, "RightButton")) == 0) {
															field_index = &p->key_13;
														}
														if ((strcmp(keyword, "RightButtonPlus")) == 0) {
															field_index = &p->key_13_plus;
														}
													}
/* Just until I understand!*/
/* Write the field to memory structure */			*field_index = atoi(write_buffer);
													num_field++;
													break;
												}
												field_index++;
												string_index = string_index+slen+1;
											}
										}
										break;
									}
								}
							}
							keyword = keyword+klen+1; /* Steps to the next string member in the struct */
/* This is such a HACK*/	if (!re_aligned) {
/* Because I don't know*/		if ((programname_alloced) && (presscurve_alloced)) {
/* How to traverse structs*/		keyword = hp->h_handle_touch;
/* Refactor after graphire4!*/		re_aligned = 1;
								}
							}
						}
/* End keyword loop */
					} else {
						return 4;	/* Line was too long to parse */
					}
				}
/* End field loop */
			}
			if ((num_field < configfields) && (num_field > 0)) {
				if (be_verbose) {
					fprintf(stderr, "Check configfile! A record is incomplete or has a premature record separator\n");
				}
				if (presscurve_alloced) {
					free(p->stylus1_presscurve);
				}
				if ((num_record) && (programname_alloced)) {
					if (be_verbose) {
						fprintf(stderr, "PGR RECNAME = %s skipped! (too few fields or a misplaced record separator)\n", p->class_name);
					}
					num_record--;
					free(p->class_name);
				}
			} else {
				if (num_field == configfields) {
					if (be_verbose) {
						fprintf(stderr, "PGR RECNAME = %s\n", p->class_name);
						fprintf(stderr, "ST1 PRCURVE = \"%s\"\n", p->stylus1_presscurve);
					}
					p++;
				}
			}
		} else {
			return 4;					/* Line was too long to parse */
		}
	}
/* End record loop */
	if (!num_record) {
		return 1;						/* Could not find a complete record */
	}

	sane = 0;
	p = p - num_record;
	for (i = 0; i < num_record; i++, p++) {
		if ((strcmp(p->class_name, "default")) == 0) {
			default_program = p;
			sane = 1;
			break;
		}
	}
	if (!sane) {
		return 5;						/* No "default" program record. Exit */
	}

	num_list = num_record;
	if (be_verbose) {
		fprintf(stderr, "PGR RECORDS = %d\n", num_list);
	}
	return 0;							/* Return without errors */
}

/* End Code */

