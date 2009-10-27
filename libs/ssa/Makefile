LIB         = ssa
LIBPREFIX	= lib
SRCDIR		= ./src
GNOMEINC	= -I/usr/include/libgnome-2.0
GNOMEINC	+= -I/usr/include/glib-2.0
GNOMEINC	+= -I/usr/lib/glib-2.0/include
GNOMEINC	+= -I/usr/include/libbonobo-2.0
INCLUDES    = -I$(SRCDIR)
INCLUDES	+= $(GNOMEINC)
#DEBUG      = -DDEBUG -DDEBUGSCREEN
DEBUG       = -DDEBUG -DDEBUG2
LOG         = -DLOGFACILITY=LOG_LOCAL7
NETWORK     = -DUSE_NETWORK
COMPONENTS  = -DNEEDCMDFILETHREAD
SYSLIBS		= -lgnome-2
SYSLIBS		+= -lm
CFLAGS      = -Wall
CFLAGS      += $(DEBUG)
CFLAGS      += $(LOG)
CFLAGS      += $(NETWORK)
CFLAGS      += $(COMPONENTS)
CFLAGS      += -O2
#CFLAGS      += -g
CFLAGS		+= -fPIC
CC          = gcc $(CFLAGS) $(INCLUDES)
CPP         = g++ $(CFLAGS) $(INCLUDES)
AR          = ar
ARFLAGS     = rs
LIBNAME     = $(LIB).a
LIBDIR      = ./lib

OBJS  = command.o
OBJS += commandfile.o
OBJS += readconfigfile.o
OBJS += delay.o
OBJS += config.o
OBJS += waypoints.o
OBJS += logfile.o
OBJS += status.o
OBJS += telemfile.o
OBJS += socket.o

all: $(LIBNAME)

$(LIBNAME): $(OBJS)
	$(AR) $(ARFLAGS) $(LIBDIR)/$@ $(OBJS)

# Object files from this directory.
%.o: %.cpp
	$(CPP) -c $< $(SYSLIBS)

%.o: %.c
	$(CC) -c $< $(SYSLIBS)

# Object files from src directory.
%.o : $(SRCDIR)/%.c
	$(CPP) -c $< $(SYSLIBS)

%.o : $(SRCDIR)/%.c
	$(CC) -c $< $(SYSLIBS)

clean:
	rm -f *.o $(LIBDIR)/*.a
    
