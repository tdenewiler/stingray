LIB			= vision
INCLUDES	= -Iinclude
INCLUDES	+= -I/usr/include/opencv
CFLAGS		= -Wall
CFLAGS		+= -O2
CFLAGS		+= -g
CC			= g++
AR			= ar
ARFLAGS		= rs
LIBS		= $(LIB).a
LIBDIR		= lib
SRCDIR		= .

LIBOBJ = $(SRCDIR)/$(LIB).o

all: $(LIBS)

$(LIBS): $(LIBOBJ)
	$(AR) $(ARFLAGS) $(LIBDIR)/$@ $(LIBOBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

clean:
	rm -f *.o $(LIBDIR)/*.a
