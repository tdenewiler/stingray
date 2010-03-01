INCLUDES	= -Iinclude
INCLUDES	+= -I../libs/labjack/include
INCLUDES	+= -I../libs/logframe/include
INCLUDES	+= -I../libs/serial/include
INCLUDES	+= -I../libs/microstrain/include
INCLUDES	+= -I../libs/pololu/include

CFLAGS	= -Wall
CFLAGS	+= -O2
CFLAGS	+= -g
STDLIBS	= -lm
CC		= g++

# these are the names of the libraries
IMULIB		= microstrain.a
SERLIB		= serial.a
POLOLULIB	= pololu.a
LABJACKLIB	= labjack.a
LOGFRAMELIB	= logframe.a

# these are the paths for each library
IMULIBDIR		= ../libs/microstrain/lib
SERLIBDIR		= ../libs/serial/lib
POLOLULIBDIR	= ../libs/pololu/lib
LABJACKLIBDIR	= ../libs/labjack/lib
LOGFRAMELIBDIR	= ../libs/logframe/lib

EXE		= main_test
OBJS	= main_test.o

all: $(EXE)

# couldn't find an order that works
$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $^ -o $@ \
	$(STDLIBS) \
	$(POLOLULIBDIR)/$(POLOLULIB) \
	$(IMULIBDIR)/$(IMULIB) \
	$(SERLIBDIR)/$(SERLIB) \
	$(LABJACKLIBDIR)/$(LABJACKLIB) \
	$(LOGFRAMELIBDIR)/$(LOGFRAMELIB)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

clean:
	rm -f *.o $(EXE)

