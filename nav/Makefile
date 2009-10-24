# include directories
INCLUDES    = -Iinclude
INCLUDES    += -I../common/include
INCLUDES    += -I../../libs/parser/include
INCLUDES    += -I../../libs/serial/include
INCLUDES    += -I../../libs/microstrain/include
INCLUDES    += -I../../libs/labjack/include
INCLUDES    += -I../../libs/pololu/include
INCLUDES    += -I../../libs/timing/include
INCLUDES    += -I../../libs/ssa/src
INCLUDES    += -I../labjackd/include
INCLUDES    += -I../planner/include/

# compiler and flags
CC          = g++
CFLAGS      = -Wall
CFLAGS      += -O2
CFLAGS      += -g
#DEFINES		= -DUSE_SSA

# library directories
PARSEDIR    = ../../libs/parser/lib
SERDIR      = ../../libs/serial/lib
MSDIR       = ../../libs/microstrain/lib
LJDIR       = ../../libs/labjack/lib
POLDIR      = ../../libs/pololu/lib
TIMINGDIR   = ../../libs/timing/lib
SSADIR      = ../../libs/ssa/lib

# libraries
LIBPREFIX	= lib
PARSELIB    = parser.a
SERLIB      = serial.a
MSLIB       = microstrain.a
LJLIB       = labjack.a
POLLIB      = pololu.a
TIMINGLIB   = timing.a
SSALIB      = ssa.a
STDLIBS     = -lm
STDLIBS		+= -lgnome-2
LIBS		= $(PARSEDIR)/$(PARSELIB)
LIBS		+= $(MSDIR)/$(MSLIB)
LIBS		+= $(LJDIR)/$(LJLIB)
LIBS		+= $(POLDIR)/$(POLLIB)
LIBS		+= $(SERDIR)/$(SERLIB)
LIBS		+= $(TIMINGDIR)/$(LIBPREFIX)$(TIMINGLIB)
LIBS		+= $(STDLIBS)

# executables to build
EXE         = nav

# object files
OBJS = util.o
OBJS += messages.o
OBJS += pid.o
OBJS += network.o
OBJS += nav.o

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
	
