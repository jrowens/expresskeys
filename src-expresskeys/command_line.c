/*
 expresskeys - Support ExpressKeys, Touch Strips, Scroll Wheel on Wacom tablets.

 Copyright (C) 2005-2006 - Mats Johannesson

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

#include <stdio.h> /* NULL, fprintf */
#include <string.h> /* strlen, strstr */
#include <ctype.h> /* tolower */
#include <sys/types.h> /* kill */
#include <signal.h> /* kill */

#include "defines.h"

/* Globals: */

/* Behaviour flags: */
int go_daemon;
int be_verbose;
int just_exit;
/* Kill a daemon instance: */
int send_sigterm;
/* Reread configuration files of a daemon instance: */
int send_sigusr1;
/* Report status of a daemon instance to file/screen: */
int send_sigusr2;

/* Our version. Remember to change it: */
const char* our_prog_version = "0.4.0";

/* Generic strings used in comparisons,
 and for flow control in some functions: */
const char* pad_string = "pad";
const char* stylus_string = "stylus";

/* Pad and styli named on the command line: */
const char* user_pad;
const char* user_stylus1;
const char* user_stylus2;

/* Externals: */

extern int live_pid;
extern const char* our_prog_name;

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Command line errors or an explicit -h prints a help screen:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void print_help(FILE* errorfp)
{
	const char* help_page =
"[OPTION]... [DEVICE]...\n"
"Any combination of pad and/or stylus can be specified.\n"
"An empty command line means automatic device search.\n"
"Options can be written like -dv or -d --v in any place.\n\n"

"  -d makes the program a daemon (runs in the background).\n"
"  -k terminates (kills) an already running daemon instance.\n"
"  -r re-reads the configuration files of a running daemon.\n"
"  -s tells a daemon instance to report status (file/screen).\n"
"  -v prints info to the screen at many execution points.\n"
"  -x sets -v and exits after some important info blocks.\n"
"  -h unconditionally brings up this help text.\n\n"

"Example1: expresskeys -d (All 'pad's and/or 'stylus'es found get used)\n"
"Example2: expresskeys 1stIntuos3Pad 1stIntuos3Stylus2 -d (named devices)\n"
"Example3: expresskeys -rv (visibly re-read the configuration file)\n\n"

"Please direct any bug reports or questions to the top address\n"
"in the AUTHORS file. This program is _not_ a linuxwacom project.\n";

	fprintf(stderr, "expresskeys Version: %s\n\n", our_prog_version);

	fprintf(stderr, "Usage: %s %s", our_prog_name, help_page);

	exit_on_error(errorfp, "", "", "");

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Parse command line for behaviour flags and named devices:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void parse_command(FILE* errorfp, int argc, char* argv[])
{
	char write_buffer[MAXBUFFER];

	int give_help = 0;
	int invalid = 0;
	int cosmetic;
	int i, c;

	while ((--argc > 0) && (**++argv == '-' || **argv != '-')) {
		if (**argv == '-') {
			cosmetic = 0;
			while (((c = *++argv[0])) && (argc != NON_VALID)) {
				switch (c) {
					case '-':
					cosmetic++;
					if (cosmetic > 1) {
						fprintf(stderr,
"\n%s ERROR: Only '-' or '--' before the switch(es), please.\n", our_prog_name);
						argc = NON_VALID;
						invalid = 1;
					}
					case 'd':
					go_daemon = 1;
					break;

					case 'k':
					send_sigterm = 1;
					break;

					case 'r':
					send_sigusr1 = 1;
					break;

					case 's':
					send_sigusr2 = 1;
					break;

					case 'v':
					be_verbose = 1;
					break;

					case 'x':
					just_exit = 1;
					be_verbose = 1;
					break;

					case 'h':
					give_help = 1;
					break;

					default:
					fprintf(stderr,
	"\n%s ERROR: Invalid switch '%c' on command line.\n", our_prog_name, c);
					argc = NON_VALID;
					invalid = 1;
					break;
				}
			}
		} else {
			if (strlen(argv[0]) < MAXBUFFER) {
				i = 0;
				do {
					write_buffer[i] = tolower(argv[0][i]);
				} while(argv[0][i++] != '\0');
				write_buffer[i-1] = '\0';
			} else {
				exit_on_error(errorfp,
"\n%s ERROR: A device name on the command line was too long to handle!\n",
							our_prog_name, "");
			}

			if (((strstr(write_buffer, pad_string)) !=NULL)
						&& (!user_pad)) {
				user_pad = *argv;
			} else if (((strstr(write_buffer, stylus_string))!=NULL)
						&& (!user_stylus1)) {
				user_stylus1 = *argv;
			} else if (((strstr(write_buffer, stylus_string))!=NULL)
						&& (!user_stylus2)) {
				user_stylus2 = *argv;
			} else {
				invalid = 1;
			}

			if (invalid) {
				fprintf(stderr,
		"\n%s ERROR: Invalid or extra device '%s' on command line.\n",
						our_prog_name, write_buffer);
				argc = NON_VALID;
			}
		}
	}

	if ((argc != 0) || (give_help == 1)) {
		if (!invalid) {
			fprintf(stderr, "\n");
		}
		print_help(errorfp);
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Act on user flags to kill, show status or reread configuration files of daemon:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void send_command(FILE* errorfp)
{
	if (send_sigusr1) {
		if ((kill(live_pid, SIGUSR1)) != NON_VALID) {
			if (be_verbose) {
				exit_on_error(errorfp,
"\n%s SUCCESS: The expresskeys configuration files has been re-read\n",
						our_prog_name, "");
			}
			exit_on_error(errorfp, "", "", "");
		} else {
			exit_on_error(errorfp,
"\n%s FAILURE: The expresskeys configuration files could not be re-read!\n",
						our_prog_name, "");
		}
	}
	if (send_sigusr2) {
		if ((kill(live_pid, SIGUSR2)) != NON_VALID) {
			if (be_verbose) {
				exit_on_error(errorfp,
"\n%s SUCCESS: The expresskeys status has been reported\n",
						our_prog_name, "");
			}
			exit_on_error(errorfp, "", "", "");
		} else {
			exit_on_error(errorfp,
"\n%s FAILURE: The expresskeys status could not be reported!\n",
						our_prog_name, "");
		}
	}
	if (send_sigterm) {
		if ((kill(live_pid, SIGTERM)) != NON_VALID) {
			if (be_verbose) {
				exit_on_error(errorfp,
"\n%s SUCCESS: The expresskeys daemon has been terminated\n",
						our_prog_name, "");
			}
			exit_on_error(errorfp, "", "", "");
		} else {
			exit_on_error(errorfp,
"\n%s FAILURE: The expresskeys daemon could not be terminated!\n",
						our_prog_name, "");
		}
	}
	exit_on_error(errorfp, "\n%s FAILURE: UNKNOWN COMMAND!\n",
						our_prog_name, "");

}

/* End Code */

