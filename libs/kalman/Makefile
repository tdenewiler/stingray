LIB			= kalman
LIBPREFIX	= lib
INCLUDES	= -Iinclude
INCLUDES	+= -I/usr/include/opencv
INCLUDES	+= -I../common/include
CFLAGS		= -Wall
CFLAGS		+= -O2
CFLAGS		+= -g
CFLAGS		+= -fPIC
CFLAGS		+= -Wno-write-strings
CC			= g++
AR			= ar
ARFLAGS		= rs
SOFLAGS		= -shared
LIBSO		= $(LIBPREFIX)$(LIB).so
LIBA		= $(LIBPREFIX)$(LIB).a
LIBDIR		= lib
SRCDIR		= .

LIBOBJ 		= $(SRCDIR)/$(LIB).o

all: $(LIBSO) $(LIBA)

$(LIBSO): $(LIBOBJ)
	$(CC) $(SOFLAGS) $(CFLAGS) $(INCLUDES) $(DEFINES) -o $(LIBDIR)/$@ $(LIBOBJ)

$(LIBA): $(LIBOBJ)
	$(AR) $(ARFLAGS) $(LIBDIR)/$@ $(LIBOBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(LINK) $(DEFINES) -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(LINK) $(DEFINES) -c $<

clean:
	rm -f *.o $(LIBDIR)/*.a $(LIBDIR)/*.so
