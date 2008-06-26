## Process this file with automake to produce Makefile.in

AM_CFLAGS = -Wall

bin_PROGRAMS = expresskeysconf

expresskeysconf_SOURCES = main_setup-conf.c globals-conf.c globals-conf.h \
	config_read-conf.c on_error-conf.c on_signal-conf.c event_loop-conf.c

expresskeysconf_LDFLAGS = @EXPRESSKEYSCONF_LIBS@

