INCLUDES	= -Iinclude
INCLUDES	+= -I../include
INCLUDES	+= -I../../serial/include
CFLAGS		= -Wall
CFLAGS		+= -O2
CFLAGS		+= -g
CC			= g++
IMULIB		= microstrain.a
SERLIB		= serial.a
IMULIBDIR	= ../lib
SERLIBDIR	= ../../serial/lib
EXE			= imu_test

OBJS = imu_test.o

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $^ -o $@ $(IMULIBDIR)/$(IMULIB) $(SERLIBDIR)/$(SERLIB)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

clean:
	rm -f *.o $(EXE)
	
