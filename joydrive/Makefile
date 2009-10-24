# include directories
INCLUDES  	= -Iinclude
INCLUDES	+= -I../common/include
INCLUDES	+= -I../planner/include
INCLUDES	+= -I../../libs/parser/include
INCLUDES	+= -I../../libs/microstrain/include
INCLUDES	+= -I../../libs/labjack/include
INCLUDES	+= -I../../libs/pololu/include
INCLUDES	+= -I../../libs/joy/include
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

# library directories
PARSEDIR	= ../../libs/parser/lib
JOYDIR		= ../../libs/joy/lib

# libraries
LIBPREFIX	= lib
PARSELIB	= $(LIBPREFIX)parser.a
JOYLIB		= $(LIBPREFIX)joy.a
LIBS		= $(PARSEDIR)/$(PARSELIB)
LIBS		+= $(JOYDIR)/$(JOYLIB)

# executables to build
EXE 		= joydrive

# object files
OBJS		= util.o
OBJS		+= network.o
OBJS		+= messages.o
OBJS		+= joydrive.o

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
