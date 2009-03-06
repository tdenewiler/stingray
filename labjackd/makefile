# include directories
INCLUDES  	= -Iinclude
INCLUDES	+= -I../common/include
INCLUDES	+= -I../../libs/microstrain/include
INCLUDES	+= -I../../libs/pololu/include
INCLUDES	+= -I../../libs/labjack/include

# compiler and flags
CC        	= g++
CFLAGS    	= -Wall
CFLAGS    	+= -O2
CFLAGS    	+= -g

# libraries
LJLIB		= labjack.a

# library directories
LJDIR		= ../../libs/labjack/lib

# executables to build
EXE 		= labjackd

# object files
OBJS += messages.o
OBJS += network.o
OBJS += labjackd.o

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $^ -o $@ $(LJDIR)/$(LJLIB)

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

