# include directories
INCLUDES  	= -Iinclude
INCLUDES	+= -I../common/include
INCLUDES	+= -I../../libs/parser/include
INCLUDES	+= -I../../libs/serial/include
INCLUDES	+= -I../../libs/microstrain/include
INCLUDES	+= -I../../libs/labjack/include
INCLUDES	+= -I../../libs/pololu/include
INCLUDES	+= -I../../libs/kalman/include
INCLUDES	+= -I../../libs/timing/include
INCLUDES	+= -I/usr/include/opencv

# compiler and flags
CC        	= g++
CFLAGS    	= -Wall
CFLAGS    	+= -O2
CFLAGS    	+= -g
LINK		= -lcv

# library directories
PARSEDIR	= ../../libs/parser/lib
SERDIR		= ../../libs/serial/lib
MSDIR		= ../../libs/microstrain/lib
LJDIR		= ../../libs/labjack/lib
POLDIR		= ../../libs/pololu/lib
KALDIR		= ../../libs/kalman/lib
TIMINGDIR	= ../../libs/timing/lib

# libraries
LIBPREFIX	= lib
PARSELIB	= $(LIBPREFIX)parser.a
SERLIB		= $(LIBPREFIX)serial.a
MSLIB		= $(LIBPREFIX)microstrain.a
LJLIB		= $(LIBPREFIX)labjack.a
POLLIB		= $(LIBPREFIX)pololu.a
KALLIB		= $(LIBPREFIX)kalman.a
TIMINGLIB	= $(LIBPREFIX)timing.a
STDLIBS		= -lm
LIBS		= $(PARSEDIR)/$(PARSELIB)
LIBS		+= $(MSDIR)/$(MSLIB)
LIBS		+= $(LJDIR)/$(LJLIB)
LIBS		+= $(POLDIR)/$(POLLIB)
LIBS		+= $(SERDIR)/$(SERLIB)
LIBS		+= $(KALDIR)/$(KALLIB)
LIBS		+= $(TIMINGDIR)/$(TIMINGLIB)
LIBS		+= $(LINK)
LIBS		+= $(STDLIBS)

# executables to build
EXE 		= planner

# object files
OBJS = util.o
OBJS += messages.o
OBJS += pid.o
OBJS += network.o
OBJS += task.o
OBJS += planner.o

all: $(EXE)

$(EXE): $(OBJS)
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

