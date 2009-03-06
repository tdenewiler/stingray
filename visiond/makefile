# include directories
INCLUDES  	= -Iinclude
INCLUDES	+= -I../common/include
INCLUDES	+= -I../../libs/parser/include
INCLUDES	+= -I../../libs/microstrain/include
INCLUDES	+= -I../../libs/labjack/include
INCLUDES	+= -I../../libs/pololu/include
INCLUDES	+= -I../../libs/vision/include
INCLUDES	+= -I/usr/include/opencv

# compiler and flags
CC        	= g++
CFLAGS    	= -Wall
CFLAGS    	+= -O2
CFLAGS    	+= -g
LINK		= -L/usr/X11R6/lib
LINK		+= -lcv
LINK		+= -lX11
LINK		+= -lhighgui
#LINK		+= -lstdc++
#LINK		+= -lm

# libraries
PARSELIB	= parser.a
VISIONLIB	= vision.a
STDLIBS		= -lm

# library directories
PARSEDIR	= ../../libs/parser/lib
VISIONDIR	= ../../libs/vision/lib

# executables to build
EXE 		= visiond

# object files
OBJS		= util.o
OBJS		+= network.o
OBJS		+= messages.o
OBJS		+= visiond.o

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $^ -o $@ \
												$(PARSEDIR)/$(PARSELIB) \
												$(VISIONDIR)/$(VISIONLIB) \
												$(LINK) \
												$(STDLIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

%.o: ../common/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

clean:
	rm -f *.o $(EXE)
