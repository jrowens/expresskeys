CC = gcc
CFLAGS = -O2 -fomit-frame-pointer -Wall

INCLUDES = -I/usr/X11R6/lib
LDFLAGS = -s -L/usr/X11R6/lib
LIBS = -lX11 -lXi -lXext -lXtst

TARGET = expresskeys

SRCS = globals.c get_device.c reg_events.c pen_mode.c event_loop.c main_setup.c
OBJS = globals.o get_device.o reg_events.o pen_mode.o event_loop.o main_setup.o
ALL =  $(SRCS)

all:	$(TARGET)

$(TARGET):	$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $*.c

clean:
	rm -f $(OBJS) $(TARGET)

