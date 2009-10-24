# include directories
INCLUDES	= -I../../libs/pololu/include
INCLUDES    += -I../../libs/serial/include
INCLUDES	+= -I../../libs/labjack/include

# compiler and flags
CC			= g++
CFLAGS		= -Wall
CFLAGS		+= -O2
CFLAGS		+= -g
CFLAGS		+= -Wno-write-strings

# library directories
POLDIR		= ../../libs/pololu/lib
SERDIR      = ../../libs/serial/lib
LJDIR		= ../../libs/labjack/lib

# libraries
LIBPREFIX	= lib
POLLIB		= $(LIBPREFIX)pololu.a
SERLIB      = $(LIBPREFIX)serial.a
LJLIB		= $(LIBPREFIX)labjack.a
LIBS		= $(POLDIR)/$(POLLIB)
LIBS		+= $(SERDIR)/$(SERLIB)
LIBS		+= $(LJDIR)/$(LJLIB)

# executables to build
EXE			= pololuCalibrate

# object files
OBJS		= pololuCalibrate.o

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

