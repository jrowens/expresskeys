## Process this file with automake to produce Makefile.in

AM_CFLAGS = -Wall
AM_LDFLAGS = @EXPRESSKEYS_LIBS@
AM_CPPFLAGS = @EXPRESSKEYS_INCL@

bin_PROGRAMS = expresskeys

expresskeys_SOURCES = main_setup.c globals.c globals.h config_read.c \
	config_write.c event_loop.c exec_shell.c get_device.c on_error.c \
	on_signal.c pen_mode.c reg_events.c

