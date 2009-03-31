# include directories
INCLUDES  	= -Iinclude
INCLUDES	+= -I../common/include
INCLUDES	+= -I../../libs/parser/include
INCLUDES	+= -I../../libs/microstrain/include
INCLUDES	+= -I../../libs/labjack/include
INCLUDES	+= -I../../libs/pololu/include

# defines
DEFINES		= -DUSE_CLIENT

# compiler and flags
CC        	= g++
CFLAGS    	= -Wall
CFLAGS    	+= -O2
CFLAGS    	+= -g

# Gtk+ flags
LLIBS = `pkg-config --libs gtk+-2.0`
GTKFLAGS = `pkg-config --cflags gtk+-2.0`

# libraries
PARSELIB	= parser.a

# library directories
PARSEDIR	= ../../libs/parser/lib

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
	$(DEFINES) $^ -o $@ $(STDLIBS) $(PARSEDIR)/$(PARSELIB)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $(LLIBS) $(GTKFLAGS) $(DEFINES) -c $<
	
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $(LLIBS) $(GTKFLAGS) $(DEFINES) -c $<

%.o: ../common/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $(LLIBS) $(GTKFLAGS) $(DEFINES) -c $<

clean:
	rm -f *.o $(EXE)

