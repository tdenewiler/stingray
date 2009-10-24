# include directories
INCLUDES  	= -Iinclude
INCLUDES	+= -I../common/include
INCLUDES	+= -I../../libs/microstrain/include
INCLUDES	+= -I../../libs/pololu/include
INCLUDES	+= -I../../libs/labjack/include
INCLUDES	+= -I../../libs/parser/include

# compiler and flags
CC        	= g++
CFLAGS    	= -Wall
CFLAGS    	+= -O2
CFLAGS    	+= -g

# library directories
LJDIR		= ../../libs/labjack/lib
PARSEDIR	= ../../libs/parser/lib

# libraries
LJLIB		= labjack.a
PARSELIB	= parser.a
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

