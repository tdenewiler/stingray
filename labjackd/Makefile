# include directories
INCLUDES  	= -Iinclude
INCLUDES	+= -I../common/include
INCLUDES	+= -I../planner/include
INCLUDES	+= -I../../libs/microstrain/include
INCLUDES	+= -I../../libs/pololu/include
INCLUDES	+= -I../../libs/labjack/include
INCLUDES	+= -I../../libs/parser/include
INCLUDES	+= -I../../libs/serial/include
INCLUDES	+= -I../../libs/kalman/include
INCLUDES	+= -I../../libs/timing/include
INCLUDES	+= -I/usr/include/opencv

# compiler and flags
CC        	= g++
CFLAGS    	= -Wall
CFLAGS    	+= -O2
CFLAGS    	+= -g

# library directories
LJDIR		= ../../libs/labjack/lib
PARSEDIR	= ../../libs/parser/lib

# libraries
LIBPREFIX	= lib
LJLIB		= $(LIBPREFIX)labjack.a
PARSELIB	= $(LIBPREFIX)parser.a
LIBS		= $(LJDIR)/$(LJLIB)
LIBS		+= $(PARSEDIR)/$(PARSELIB)

# executables to build
EXE 		= labjackd

# object files
OBJS = util.o
OBJS += messages.o
OBJS += network.o
OBJS += labjackd.o

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

