CC = gcc
CFLAGS = -O2 -Wall
# Use -g3 to get full debug info

INCLUDES = -I/usr/X11R6/lib
LDFLAGS = -L/usr/X11R6/lib
LIBS = -lX11 -lXext -lXi -lXtst

TARGET = ekeys

SRCS = globals.c config_all.c signal_all.c pen_mode.c \
	get_device.c reg_events.c event_loop.c main_setup.c

OBJS = $(SRCS:.c=.o)
ALL =  $(SRCS)

all:	$(TARGET)

$(TARGET):	$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $*.c

clean:
	rm -f $(OBJS) $(TARGET)

