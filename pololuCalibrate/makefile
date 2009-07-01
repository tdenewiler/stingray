# include directories
INCLUDES	= -I../../libs/pololu/include
INCLUDES    += -I../../libs/serial/include
INCLUDES	+= -I../../libs/labjack/include

# compiler and flags
CC			= g++
CFLAGS		= -Wall
CFLAGS		+= -O2
CFLAGS		+= -g

# libraries
POLLIB		= pololu.a
SERLIB      = serial.a
LJLIB		= labjack.a

# library directories
POLDIR		= ../../libs/pololu/lib
SERDIR      = ../../libs/serial/lib
LJDIR		= ../../libs/labjack/lib

# executables to build
EXE			= pololuCalibrate

# object files
OBJS		= pololuCalibrate.o

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $^ -o $@ \
                                                $(POLDIR)/$(POLLIB) \
                                                $(SERDIR)/$(SERLIB) \
                                                $(LJDIR)/$(LJLIB)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

%.o: ../common/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

clean:
	rm -f *.o $(EXE)

