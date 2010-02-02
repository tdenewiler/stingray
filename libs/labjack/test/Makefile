# include directories
INCLUDES  	= -I../include

# compiler and flags
CC        	= g++
CFLAGS    	= -Wall
CFLAGS    	+= -O2
CFLAGS    	+= -g

# library directories
LJDIR		= ../lib

# libraries
LIBPREFIX	= lib
LJLIB		= $(LIBPREFIX)labjack.a
LIBS		= $(LJDIR)/$(LJLIB)

# executables to build
EXE 		= labjack_test

# object files
OBJS = labjack_test.o

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $^ -o $@ $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

clean:
	rm -f $(OBJS) $(EXE)

