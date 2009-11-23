# include directories
INCLUDES  	= -Iinclude
INCLUDES	+= -Ilibs/timing/include

# compiler and flags
CC        	= g++
CFLAGS    	= -Wall
CFLAGS    	+= -O2
CFLAGS    	+= -g

# library directories
TIMINGDIR	= libs/timing/lib

# libraries
LIBPREFIX	= lib
TIMINGLIB	= $(LIBPREFIX)timing.a
LIBS		+= $(TIMINGDIR)/$(TIMINGLIB)
LIBS		+= $(LINK)
LIBS		+= $(STDLIBS)

# executables to build
EXE1		= server
EXE2		= client
EXE			= $(EXE1)
EXE			+= $(EXE2)

# object files
OBJS		= messages.o
OBJS		+= network.o
EXE1OBJS	= $(OBJS)
EXE1OBJS	+= server.o
EXE2OBJS	= $(OBJS)
EXE2OBJS	+= client.o

all: $(EXE)

$(EXE1): $(EXE1OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $^ -o $@ $(LIBS)

$(EXE2): $(EXE2OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $^ -o $@ $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

%.o: ../common/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

%.o: ../common/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

clean:
	rm -f *.o $(EXE)

