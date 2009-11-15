# include directories
INCLUDES  	= -Iinclude
INCLUDES	+= -I../common/include
INCLUDES	+= -I../../libs/parser/include
INCLUDES	+= -I../../libs/timing/include
INCLUDES	+= -I../../libs/sysid/include

# compiler and flags
CC        	= g++
CFLAGS    	= -Wall
CFLAGS    	+= -O2
CFLAGS    	+= -g
CFLAGS		+= -Wno-write-strings

# library directories
PARSEDIR	= ../../libs/parser/lib
TIMINGDIR	= ../../libs/timing/lib
SYSIDDIR	= ../../libs/sysid/lib

# libraries
LIBPREFIX	= lib
PARSELIB	= $(LIBPREFIX)parser.a
TIMINGLIB	= $(LIBPREFIX)timing.a
SYSIDLIB	= $(LIBPREFIX)sysid.a
STDLIBS		= -lm
LIBS		= $(PARSEDIR)/$(PARSELIB)
LIBS		+= $(TIMINGDIR)/$(TIMINGLIB)
LIBS		+= $(SYSIDDIR)/$(SYSIDLIB)
LIBS		+= $(LINK)
LIBS		+= $(STDLIBS)

# executables to build
EXE 		= estimate

# object files
OBJS		= messages.o
OBJS		+= network.o
OBJS		+= estimate.o

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

