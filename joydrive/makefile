# include directories
INCLUDES  	= -Iinclude
INCLUDES	+= -I../common/include
INCLUDES	+= -I../../libs/parser/include
INCLUDES	+= -I../../libs/microstrain/include
INCLUDES	+= -I../../libs/labjack/include
INCLUDES	+= -I../../libs/pololu/include
INCLUDES	+= -I../../libs/joy/include

# compiler and flags
CC        	= g++
CFLAGS    	= -Wall
CFLAGS    	+= -O2
CFLAGS    	+= -g

# libraries
PARSELIB	= parser.a
JOYLIB		= joy.a

# library directories
PARSEDIR	= ../../libs/parser/lib
JOYDIR		= ../../libs/joy/lib

# executables to build
EXE 		= joydrive

# object files
OBJS		= util.o
OBJS		+= network.o
OBJS		+= messages.o
OBJS		+= joydrive.o

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $^ -o $@ \
												$(PARSEDIR)/$(PARSELIB) \
												$(JOYDIR)/$(JOYLIB) \

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

%.o: ../common/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

clean:
	rm -f *.o $(EXE)
