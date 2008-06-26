CC = gcc
CFLAGS = -O2 -Wall
# Use -g3 to get full debug info

INCLUDES = -I/usr/X11R6/lib
LDFLAGS = -L/usr/X11R6/lib
LIBS = -lX11 -lXi -lXext -lXtst

TARGET = cekeys

SRCS = main_dummy.c

OBJS = $(SRCS:.c=.o)
ALL =  $(SRCS)

all:	$(TARGET)

$(TARGET):	$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $*.c

clean:
	rm -f $(OBJS) $(TARGET)

