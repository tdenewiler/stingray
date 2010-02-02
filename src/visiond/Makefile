# include directories
INCLUDES  	= -Iinclude
INCLUDES	+= -I../common/include
INCLUDES	+= -I../planner/include
INCLUDES	+= -I../../libs/parser/include
INCLUDES	+= -I../../libs/microstrain/include
INCLUDES	+= -I../../libs/labjack/include
INCLUDES	+= -I../../libs/pololu/include
INCLUDES	+= -I../../libs/vision/include
INCLUDES	+= -I../../libs/timing/include
INCLUDES	+= -I../../libs/serial/include
INCLUDES	+= -I../../libs/kalman/include
INCLUDES	+= -I/usr/include/opencv

# compiler and flags
CC        	= g++
CFLAGS    	= -Wall
CFLAGS    	+= -O2
CFLAGS    	+= -g
CFLAGS		+= -Wno-write-strings
LINK		= -L/usr/X11R6/lib
LINK		+= -lcv
LINK		+= -lX11
LINK		+= -lhighgui

# library directories
PARSEDIR	= ../../libs/parser/lib
VISIONDIR	= ../../libs/vision/lib
TIMINGDIR	= ../../libs/timing/lib

# libraries
LIBPREFIX	= lib
PARSELIB	= $(LIBPREFIX)parser.a
VISIONLIB	= $(LIBPREFIX)vision.a
TIMINGLIB	= $(LIBPREFIX)timing.a
STDLIBS		= -lm
LIBS		= $(PARSEDIR)/$(PARSELIB)
LIBS		+= $(TIMINGDIR)/$(TIMINGLIB)
LIBS		+= $(VISIONDIR)/$(VISIONLIB)
LIBS		+= $(LINK)
LIBS		+= $(STDLIBS)

# executables to build
EXE 		= visiond

# object files
OBJS		= network.o
OBJS		+= messages.o
OBJS		+= visiond.o

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $^ -o $@ $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

%.o: ../common/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

clean:
	rm -f *.o $(EXE)
