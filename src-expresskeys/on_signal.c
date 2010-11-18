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

#include <stdio.h> /* NULL, FILE, fprintf, fopen, fclose */
#include <stdlib.h> /* free, getenv */
#include <signal.h> /* signal, raise */
#include <sys/types.h> /* getpid */
#include <unistd.h> /* getpid */
#include <X11/extensions/XInput.h>

#include "tablet.h"

/* Globals: */

/* Flag to show that it's another pass through the configuration files: */
int reread_config;

/* Strings prefacing the device names. We write this out in the status log,
 under the config-file string, so that an external program can use the info: */
const char* padstr = "CurrentPad";
const char* sty1str = "CurrentSty1";
const char* sty2str = "CurrentSty2";

/* Externals: */

extern int ok_config;
extern int go_daemon;

extern const int ux;
extern const int ux2;
extern const int bbo;
extern const int bee;
extern const int i3;
extern const int i3s;
extern const int g4;
extern const int g4b;
extern const int nop;

extern Display* display;

extern XDeviceInfo* xdevice_list;
extern XDevice* pad_xdevice[MAXMODEL][MAXPAD];
extern XDevice* stylus_xdevice[MAXSTYLUS][MAXMODEL * MAXPAD];

extern const char* pad_name[MAXMODEL][MAXPAD];
extern const char* stylus_name[MAXSTYLUS][MAXMODEL * MAXPAD];

extern const char* our_prog_name;
extern const char* our_prog_version;

extern const char* configstring;
extern const char* configversion;

extern char* total_config_dir;
extern char* total_pid_file;
extern char* total_error_file;
extern char* total_status_file;
extern char* total_file1_padless;
extern char* total_file2_padless;
extern char* total_file3_padless;
extern char* total_file1_graphire4bt;
extern char* total_file2_graphire4bt;
extern char* total_file3_graphire4bt;
extern char* total_file1_graphire4;
extern char* total_file2_graphire4;
extern char* total_file3_graphire4;
extern char* total_file1_intuos3s;
extern char* total_file2_intuos3s;
extern char* total_file3_intuos3s;
extern char* total_file1_intuos3;
extern char* total_file2_intuos3;
extern char* total_file3_intuos3;
extern char* total_file1_bee;
extern char* total_file2_bee;
extern char* total_file3_bee;
extern char* total_file1_bbo;
extern char* total_file2_bbo;
extern char* total_file3_bbo;
extern char* total_file1_ux2;
extern char* total_file2_ux2;
extern char* total_file3_ux2;
extern char* total_file1_ux;
extern char* total_file2_ux;
extern char* total_file3_ux;

extern void read_config(FILE* errorfp);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Aid in debugging or to satisfy curiosity. This is part of status reporting:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void report_header(FILE* statusfp)
{
	int i, j;

	fprintf(statusfp,
"\n'Models': nopad, Graphire4 BlueTooth, Graphire4, Intuos3 small, \
Intuos3/Cintiq, Cintiq 20wsx, Bamboo, Cintiq 21UX2\n");
	fprintf(statusfp, "\n* Pad-index *\n");
	fprintf(statusfp, "%i (nop): 0 1 2\n", nop);
	fprintf(statusfp, "%i (g4b): 0 1 2\n", g4b);
	fprintf(statusfp, "%i (g4 ): 0 1 2\n", g4);
	fprintf(statusfp, "%i (i3s): 0 1 2\n", i3s);
	fprintf(statusfp, "%i (i3 ): 0 1 2\n", i3);
	fprintf(statusfp, "%i (bee): 0 1 2\n", bee);
	fprintf(statusfp, "%i (bbo): 0 1 2\n", bbo);
	fprintf(statusfp, "%i (ux2): 0 1 2\n\n", ux2);
	fprintf(statusfp, "%i (ux): 0 1 2\n\n", ux);
	fprintf(statusfp, "* Stylus-index *\n");
	fprintf(statusfp, "0 st1 (nop): 0 1 2 (g4b): 3 4 5 (g4): 6 7 8 \
(i3s): 9 10 11 (i3): 12 13 14 (bee): 15 16 (bbo): 17 18 (ux2): 19 20 (ux): 21 22\n");
	fprintf(statusfp, "1 st2 (nop): 0 1 2 (g4b): 3 4 5 (g4): 6 7 8 \
(i3s): 9 10 11 (i3): 12 13 14 (bee): 15 16 (bbo): 17 18 (ux2): 19 20 (ux): 21 22\n\n");
	fprintf(statusfp, "PGR RUNNAME = %s\n", our_prog_name);
	fprintf(statusfp, "PGR VERSION = %s\n", our_prog_version);
	fprintf(statusfp, "USR HOMEDIR = %s\n", getenv("HOME"));
	fprintf(statusfp, "OUR CNF-DIR = %s\n", total_config_dir);
	fprintf(statusfp, "OUR PIDFILE = %s\n", total_pid_file);
	fprintf(statusfp, "OUR INFFILE = %s\n", total_status_file);
	fprintf(statusfp, "OUR ERRFILE = %s\n", total_error_file);
	for (i = 0; i < MAXMODEL; i++) {
		for (j = 0; j < MAXPAD; j++) {
			if (pad_name[i][j]) {
				fprintf(statusfp,
		"OUR PDxNAME = pad_name[%i][%i] = %s\n", i, j, pad_name[i][j]);
			}
		}
	}
	for (i = 0; i < MAXSTYLUS; i++) {
		for (j = 0; j < MAXMODEL * MAXPAD; j++){
			if (stylus_name[i][j]) {
				fprintf(statusfp,
	"OUR STxNAME = stylus_name[%i][%i] = %s\n", i, j, stylus_name[i][j]);
			}
		}
	}
	fprintf(statusfp, "\n");

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Aid in debugging or to satisfy curiosity. This is part of status reporting:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void report_record(FILE* statusfp, void* address, const int model)
{
	struct ux_program* pux;
	struct ux2_program* pux2;
	struct bbo_program* pbbo;
	struct bee_program* pbee;
	struct i3_program* pi3;
	struct i3s_program* pi3s;
	struct g4_program* pg4;
	struct g4b_program* pg4b;
	struct nop_program* pnop;
	struct common_data* cdp = NULL;

	if (model == ux) {
		pux = address;
		cdp = &pux->common_data;
	} else if (model == ux2) {
		pux2 = address;
		cdp = &pux2->common_data;
	} else if (model == bbo) {
		pbbo = address;
		cdp = &pbbo->common_data;
	} else if (model == bee) {
		pbee = address;
		cdp = &pbee->common_data;
	} else if (model == i3) {
		pi3 = address;
		cdp = &pi3->common_data;
	} else if (model == i3s) {
		pi3s = address;
		cdp = &pi3s->common_data;
	} else if (model == g4) {
		pg4 = address;
		cdp = &pg4->common_data;
	} else if (model == g4b) {
		pg4b = address;
		cdp = &pg4b->common_data;
	} else if (model == nop) {
		pnop = address;
		cdp = &pnop->common_data;
	}

	if (cdp->class_name != NULL) {
		fprintf(statusfp, "PGR RECNAME = \"%s\"\n", cdp->class_name);
	}
	if (cdp->stylus1_presscurve != NULL) {
		fprintf(statusfp, "ST1 PRCURVE = \"%s\"\n",
						cdp->stylus1_presscurve);
	}
	if (cdp->stylus2_presscurve != NULL) {
		fprintf(statusfp, "ST2 PRCURVE = \"%s\"\n",
						cdp->stylus2_presscurve);
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Aid in debugging or to satisfy curiosity. This is part of status reporting:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void print_common(FILE* statusfp, const char* configfile,
				int userconfigversion, const char* padname,
				const char* sty1name, const char* sty2name)
{
	fprintf(statusfp, "OUR CNFFILE = %s\n", configfile);
	fprintf(statusfp, "%s-user = %i\n", configstring, userconfigversion);
	fprintf(statusfp, "%s-ours = %s\n", configstring, configversion);
	fprintf(statusfp, "%s  = %s\n", padstr, (padname?padname :"LACKING"));
	fprintf(statusfp, "%s = %s\n", sty1str, (sty1name?sty1name :"LACKING"));
	fprintf(statusfp, "%s = %s\n", sty2str, (sty2name?sty2name :"LACKING"));

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Aid in debugging or to satisfy curiosity. This is part of status reporting:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void report_common(FILE* statusfp)
{
	int i, j;
	int num_record;
	const char* pgr_records = "PGR RECORDS =";

	struct ux_program* pux;
	struct ux2_program* pux2;
	struct bbo_program* pbbo;
	struct bee_program* pbee;
	struct i3_program* pi3;
	struct i3s_program* pi3s;
	struct g4_program* pg4;
	struct g4b_program* pg4b;
	struct nop_program* pnop;

	struct model_index* mip;
	mip = model_list;

	for (i = 0; i < MAXPAD; i++, mip++) {
		if (mip->ux->common_data.num_record) {
			print_common(statusfp, mip->ux->common_data.configfile,
					mip->ux->common_data.userconfigversion,
						mip->ux->common_data.padname,
						mip->ux->common_data.sty1name,
						mip->ux->common_data.sty2name);
			num_record = mip->ux->common_data.num_record;
			pux = mip->ux;
			for (j = 0; j < num_record; j++, pux++) {
				report_record(statusfp, pux, ux);
			}
			fprintf(statusfp, "%s %i (of max %i)\n\n", pgr_records,
							num_record, MAXRECORDS);
		}
		if (mip->ux2->common_data.num_record) {
			print_common(statusfp, mip->ux2->common_data.configfile,
					mip->ux2->common_data.userconfigversion,
						mip->ux2->common_data.padname,
						mip->ux2->common_data.sty1name,
						mip->ux2->common_data.sty2name);
			num_record = mip->ux2->common_data.num_record;
			pux2 = mip->ux2;
			for (j = 0; j < num_record; j++, pux2++) {
				report_record(statusfp, pux2, ux2);
			}
			fprintf(statusfp, "%s %i (of max %i)\n\n", pgr_records,
							num_record, MAXRECORDS);
		}
		if (mip->bbo->common_data.num_record) {
			print_common(statusfp, mip->bbo->common_data.configfile,
					mip->bbo->common_data.userconfigversion,
						mip->bbo->common_data.padname,
						mip->bbo->common_data.sty1name,
						mip->bbo->common_data.sty2name);
			num_record = mip->bbo->common_data.num_record;
			pbbo = mip->bbo;
			for (j = 0; j < num_record; j++, pbbo++) {
				report_record(statusfp, pbbo, bbo);
			}
			fprintf(statusfp, "%s %i (of max %i)\n\n", pgr_records,
							num_record, MAXRECORDS);
		}
		if (mip->bee->common_data.num_record) {
			print_common(statusfp, mip->bee->common_data.configfile,
					mip->bee->common_data.userconfigversion,
						mip->bee->common_data.padname,
						mip->bee->common_data.sty1name,
						mip->bee->common_data.sty2name);
			num_record = mip->bee->common_data.num_record;
			pbee = mip->bee;
			for (j = 0; j < num_record; j++, pbee++) {
				report_record(statusfp, pbee, bee);
			}
			fprintf(statusfp, "%s %i (of max %i)\n\n", pgr_records,
							num_record, MAXRECORDS);
		}
		if (mip->i3->common_data.num_record) {
			print_common(statusfp, mip->i3->common_data.configfile,
					mip->i3->common_data.userconfigversion,
						mip->i3->common_data.padname,
						mip->i3->common_data.sty1name,
						mip->i3->common_data.sty2name);
			num_record = mip->i3->common_data.num_record;
			pi3 = mip->i3;
			for (j = 0; j < num_record; j++, pi3++) {
				report_record(statusfp, pi3, i3);
			}
			fprintf(statusfp, "%s %i (of max %i)\n\n", pgr_records,
							num_record, MAXRECORDS);
		}
		if (mip->i3s->common_data.num_record) {
			print_common(statusfp, mip->i3s->common_data.configfile,
					mip->i3s->common_data.userconfigversion,
						mip->i3s->common_data.padname,
						mip->i3s->common_data.sty1name,
						mip->i3s->common_data.sty2name);
			num_record = mip->i3s->common_data.num_record;
			pi3s = mip->i3s;
			for (j = 0; j < num_record; j++, pi3s++) {
				report_record(statusfp, pi3s, i3s);
			}
			fprintf(statusfp, "%s %i (of max %i)\n\n", pgr_records,
							num_record, MAXRECORDS);
		}
		if (mip->g4->common_data.num_record) {
			print_common(statusfp, mip->g4->common_data.configfile,
					mip->g4->common_data.userconfigversion,
						mip->g4->common_data.padname,
						mip->g4->common_data.sty1name,
						mip->g4->common_data.sty2name);
			num_record = mip->g4->common_data.num_record;
			pg4 = mip->g4;
			for (j = 0; j < num_record; j++, pg4++) {
				report_record(statusfp, pg4, g4);
			}
			fprintf(statusfp, "%s %i (of max %i)\n\n", pgr_records,
							num_record, MAXRECORDS);
		}
		if (mip->g4b->common_data.num_record) {
			print_common(statusfp, mip->g4b->common_data.configfile,
					mip->g4b->common_data.userconfigversion,
						mip->g4b->common_data.padname,
						mip->g4b->common_data.sty1name,
						mip->g4b->common_data.sty2name);
			num_record = mip->g4b->common_data.num_record;
			pg4b = mip->g4b;
			for (j = 0; j < num_record; j++, pg4b++) {
				report_record(statusfp, pg4b, g4b);
			}
			fprintf(statusfp, "%s %i (of max %i)\n\n", pgr_records,
							num_record, MAXRECORDS);
		}
		if (mip->nop->common_data.num_record) {
			print_common(statusfp, mip->nop->common_data.configfile,
					mip->nop->common_data.userconfigversion,
									NULL,
						mip->nop->common_data.sty1name,
						mip->nop->common_data.sty2name);
			num_record = mip->nop->common_data.num_record;
			pnop = mip->nop;
			for (j = 0; j < num_record; j++, pnop++) {
				report_record(statusfp, pnop, nop);
			}
			fprintf(statusfp, "%s %i (of max %i)\n\n", pgr_records,
							num_record, MAXRECORDS);
		}
	}
	if (go_daemon) {
		fprintf(statusfp, "OUR RUN-PID = %i\n\n", getpid());
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 This function acts as a signal handler replacement for SIGUSR2. On that
 signal we print some choise information to a file and to the screen:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void status_report(int signum)
{
	FILE* errorfp = NULL;
	FILE* statusfp = NULL;

	errorfp = fopen(total_error_file, "w");

/* Open (and truncate) a status log and fill it with information. Don't exit
 on pure write errors though. It is better to keep on running: */
	if ((statusfp = fopen(total_status_file, "w")) == NULL) {
		exit_on_error(errorfp,
"%s ERROR: Can not open %s in write mode\n", our_prog_name, total_status_file);
	}

	fclose(errorfp);

	report_header(statusfp);
	report_common(statusfp);

	fclose(statusfp);

/* Also print the information to sceen, but only if we have been daemonised
 and act on a -s signal: */
	if (go_daemon) {
		report_header(stderr);
		report_common(stderr);
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 This function acts as a signal handler replacement for SIGUSR1. On that
 signal we read in all active configuration files again:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void re_read_config(int signum)
{
	FILE* errorfp = NULL;
	reread_config = 1;

	if ((errorfp = fopen(total_error_file, "w")) == NULL) {
		exit_on_error(errorfp,
"%s ERROR: Reread - Can not open %s in write mode\n", our_prog_name,
							total_error_file);
	}

	read_config(errorfp);

	if (!ok_config) {
		exit_on_error(errorfp,
"\n%s ERROR: No active file with a valid ConfigVersion found!\n",
							our_prog_name, "");
	}

/* Update our information file */
	go_daemon = 0;
	status_report(0);
	go_daemon = 1;
	fclose(errorfp);

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Free malloced strings (ProgramName, Stylus1PressCurve, Stylus2PressCurve):
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void free_common(void* address, const int model)
{
	struct ux_program* pux;
	struct ux2_program* pux2;
	struct bbo_program* pbbo;
	struct bee_program* pbee;
	struct i3_program* pi3;
	struct i3s_program* pi3s;
	struct g4_program* pg4;
	struct g4b_program* pg4b;
	struct nop_program* pnop;
	struct common_data* cdp = NULL;

	if (model == ux) {
		pux = address;
		cdp = &pux->common_data;
	} else if (model == ux2) {
		pux2 = address;
		cdp = &pux2->common_data;
	} else if (model == bbo) {
		pbbo = address;
		cdp = &pbbo->common_data;
	} else if (model == bee) {
		pbee = address;
		cdp = &pbee->common_data;
	} else if (model == i3) {
		pi3 = address;
		cdp = &pi3->common_data;
	} else if (model == i3s) {
		pi3s = address;
		cdp = &pi3s->common_data;
	} else if (model == g4) {
		pg4 = address;
		cdp = &pg4->common_data;
	} else if (model == g4b) {
		pg4b = address;
		cdp = &pg4b->common_data;
	} else if (model == nop) {
		pnop = address;
		cdp = &pnop->common_data;
	}

	if (cdp->class_name != NULL) {
		free(cdp->class_name);
	}
	if (cdp->stylus1_presscurve != NULL) {
		free(cdp->stylus1_presscurve);
	}
	if (cdp->stylus2_presscurve != NULL) {
		free(cdp->stylus2_presscurve);
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 This function acts as a signal handler replacement for SIGINT, SIGHUP and
 SIGTERM. All are normal exit signals. We want to trap them in order to
 perform some house keeping pre-exit, such as deleting a PID file, a status
 file and free various memory. Since it takes care of several signals, it
 could get invoked recursively if some other signal comes in. We use this
 "volatile" variable to track the case. At the end we restore the default
 signal handler and raise it:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

volatile sig_atomic_t clean_up_exit_in_progress = 0;
void clean_up_exit(int signum)
{
	int i, j;

	struct ux_program* pux;
	struct ux2_program* pux2;
	struct bbo_program* pbbo;
	struct bee_program* pbee;
	struct i3_program* pi3;
	struct i3s_program* pi3s;
	struct g4_program* pg4;
	struct g4b_program* pg4b;
	struct nop_program* pnop;

	struct model_index* mip;

	if (clean_up_exit_in_progress) {
		raise(signum);
	}
	clean_up_exit_in_progress = 1;

/* Make sure that we don't leave the system with keyboard auto-repeat off,
 should some X error force us to quit in the middle of that state. But don't
 do this if acting from another client sending signals to a daemon instance.
 Segmentation fault _will_ ensue because no X display has been opened by it: */
	if (display) {
		XAutoRepeatOn(display);
	}

	mip = model_list;
	for (i = 0; i < MAXPAD; i++, mip++) {
		if (mip->ux->common_data.configfile) {
			pux = mip->ux;
			for (j = 0; j < MAXRECORDS; j++, pux++) {
				free_common(pux, ux);
			}
		}
		if (mip->ux2->common_data.configfile) {
			pux2 = mip->ux2;
			for (j = 0; j < MAXRECORDS; j++, pux2++) {
				free_common(pux2, ux2);
			}
		}
		if (mip->bbo->common_data.configfile) {
			pbbo = mip->bbo;
			for (j = 0; j < MAXRECORDS; j++, pbbo++) {
				free_common(pbbo, bbo);
			}
		}
		if (mip->bee->common_data.configfile) {
			pbee = mip->bee;
			for (j = 0; j < MAXRECORDS; j++, pbee++) {
				free_common(pbee, bee);
			}
		}
		if (mip->i3->common_data.configfile) {
			pi3 = mip->i3;
			for (j = 0; j < MAXRECORDS; j++, pi3++) {
				free_common(pi3, i3);
			}
		}
		if (mip->i3s->common_data.configfile) {
			pi3s = mip->i3s;
			for (j = 0; j < MAXRECORDS; j++, pi3s++) {
				free_common(pi3s, i3s);
			}
		}
		if (mip->g4->common_data.configfile) {
			pg4 = mip->g4;
			for (j = 0; j < MAXRECORDS; j++, pg4++) {
				free_common(pg4, g4);
			}
		}
		if (mip->g4b->common_data.configfile) {
			pg4b = mip->g4b;
			for (j = 0; j < MAXRECORDS; j++, pg4b++) {
				free_common(pg4b, g4b);
			}
		}
		if (mip->nop->common_data.configfile) {
			pnop = mip->nop;
			for (j = 0; j < MAXRECORDS; j++, pnop++) {
				free_common(pnop, nop);
			}
		}
	}

/* Prevent a live PID-file deletion when sending signals (no display open): */
	if (display) {
		if (total_pid_file) {
			unlink(total_pid_file);
		}
		if (total_status_file) {
			unlink(total_status_file);
		}
	}

	if (total_config_dir) {
		free(total_config_dir);
	}
	if (total_pid_file) {
		free(total_pid_file);
	}
	if (total_error_file) {
		free(total_error_file);
	}
	if (total_status_file) {
		free(total_status_file);
	}

/*---*/

	if (total_file1_padless) {
		free(total_file1_padless);
	}
	if (total_file2_padless) {
		free(total_file2_padless);
	}
	if (total_file3_padless) {
		free(total_file3_padless);
	}

/*---*/

	if (total_file1_graphire4bt) {
		free(total_file1_graphire4bt);
	}
	if (total_file2_graphire4bt) {
		free(total_file2_graphire4bt);
	}
	if (total_file3_graphire4bt) {
		free(total_file3_graphire4bt);
	}

/*---*/

	if (total_file1_graphire4) {
		free(total_file1_graphire4);
	}
	if (total_file2_graphire4) {
		free(total_file2_graphire4);
	}
	if (total_file3_graphire4) {
		free(total_file3_graphire4);
	}

/*---*/

	if (total_file1_intuos3s) {
		free(total_file1_intuos3s);
	}
	if (total_file2_intuos3s) {
		free(total_file2_intuos3s);
	}
	if (total_file3_intuos3s) {
		free(total_file3_intuos3s);
	}

/*---*/

	if (total_file1_intuos3) {
		free(total_file1_intuos3);
	}
	if (total_file2_intuos3) {
		free(total_file2_intuos3);
	}
	if (total_file3_intuos3) {
		free(total_file3_intuos3);
	}

/*---*/

	if (total_file1_bee) {
		free(total_file1_bee);
	}
	if (total_file2_bee) {
		free(total_file2_bee);
	}
	if (total_file3_bee) {
		free(total_file3_bee);
	}

/*---*/

	if (total_file1_bbo) {
		free(total_file1_bbo);
	}
	if (total_file2_bbo) {
		free(total_file2_bbo);
	}
	if (total_file3_bbo) {
		free(total_file3_bbo);
	}

/*---*/

	if (total_file1_ux2) {
		free(total_file1_ux2);
	}
	if (total_file2_ux2) {
		free(total_file2_ux2);
	}
	if (total_file3_ux2) {
		free(total_file3_ux2);
	}

/*---*/

	if (total_file1_ux) {
		free(total_file1_ux);
	}
	if (total_file2_ux) {
		free(total_file2_ux);
	}
	if (total_file3_ux) {
		free(total_file3_ux);
	}

/* The pad devices and styli devices should not be explicitly closed by a
 call to XCloseDevice. It leaves a message from X (in the terminal where X
 is started from) saying: "ProcXCloseDevice to close or not ?" Instead we
 just free the XDevice structures created by XOpenDevice: */
	for (i = 0; i < MAXMODEL; i++) {
		for (j = 0; j < MAXPAD; j++) {
			if (pad_xdevice[i][j]) {
				XFree(pad_xdevice[i][j]);
			}
		}
	}

	for (i = 0; i < MAXSTYLUS; i++) {
		for (j = 0; j < MAXMODEL * MAXPAD; j++){
			if (stylus_xdevice[i][j]) {
				XFree(stylus_xdevice[i][j]);
			}
		}
	}

/*---*/

	if (xdevice_list) {
		XFreeDeviceList(xdevice_list);
	}

	if (display) {
		XCloseDisplay(display);
	}

	signal(signum, SIG_DFL);
	raise(signum);

}

/* End Code */

