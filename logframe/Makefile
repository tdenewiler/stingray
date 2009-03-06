LIB       = logframe
INCLUDES  = -Iinclude
CFLAGS    = -Wall
CFLAGS    += -O2
CFLAGS    += -g
CC        = gcc
AR        = ar
ARFLAGS   = rs
LIBS      = $(LIB).a
LIBDIR    = lib
SRCDIR    = .

OTHERS    = $(SRCDIR)/hashtable.o $(SRCDIR)/hashtable_itr.o


LIBOBJ = $(SRCDIR)/$(LIB).o $(OTHERS)

all: $(LIBS)

$(LIBS): $(LIBOBJ)
	$(AR) $(ARFLAGS) $(LIBDIR)/$@ $(LIBOBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<
	
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

test: all
	$(CC) $(CFLAGS) -I.. -o test test.c $(LIBDIR)/$(LIB).a -lm
clean:
	rm -f *.o $(LIBDIR)/*.a

