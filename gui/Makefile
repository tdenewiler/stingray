# include directories
INCLUDES  	= -Iinclude
INCLUDES	+= -I../common/include
INCLUDES	+= -I../planner/include
INCLUDES	+= -I../visiond/include
INCLUDES	+= -I../../libs/parser/include
INCLUDES	+= -I../../libs/microstrain/include
INCLUDES	+= -I../../libs/labjack/include
INCLUDES	+= -I../../libs/pololu/include
INCLUDES	+= -I../../libs/vision/include
INCLUDES	+= -I../../libs/serial/include
INCLUDES	+= -I../../libs/kalman/include
INCLUDES	+= -I../../libs/timing/include
INCLUDES	+= -I/usr/include/opencv

# compiler and flags
CC        	= g++
CFLAGS    	= -Wall
CFLAGS    	+= -O2
CFLAGS    	+= -g
CFLAGS		+= -Wno-write-strings
LINK		= -lcv

# Gtk+ flags
LLIBS = `pkg-config --libs gtk+-2.0`
GTKFLAGS = `pkg-config --cflags gtk+-2.0`

# library directories
PARSEDIR	= ../../libs/parser/lib

# libraries
LIBPREFIX	= lib
PARSELIB	= $(LIBPREFIX)parser.a
LIBS		= $(PARSEDIR)/$(PARSELIB)
LIBS		+= $(STDLIBS)
LIBS		+= $(LINK)

# executables to build
EXE 		= gui

# object files
OBJS += network.o
OBJS += messages.o
OBJS += gui.o
OBJS += client.o
OBJS += buttons.o
OBJS += events.o

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $(LLIBS) $(GTKFLAGS) \
	$(DEFINES) $^ -o $@ $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $(LLIBS) $(GTKFLAGS) $(DEFINES) -c $<
	
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $(LLIBS) $(GTKFLAGS) $(DEFINES) -c $<

%.o: ../common/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $(LLIBS) $(GTKFLAGS) $(DEFINES) -c $<

clean:
	rm -f *.o $(EXE)

