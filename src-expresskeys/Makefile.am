## Process this file with automake to produce Makefile.in

AM_CFLAGS = -Wall
AM_LDFLAGS = @EXPRESSKEYS_LIBS@
AM_CPPFLAGS = @EXPRESSKEYS_INCL@

bin_PROGRAMS = expresskeys

expresskeys_SOURCES = main_setup.c defines.h tablet.c tablet.h command_line.c \
	config_read.c config_write.c event_loop.c get_device.c mark_info.c \
	misc_func.c on_error.c on_signal.c

