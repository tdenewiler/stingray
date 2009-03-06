#CC = g++
#CFLAGS = -g -pedantic -ansi -Wall -fPIC
#OPTIM = -O3 -funroll-loops -fstrength-reduce -fomit-frame-pointer -ffast-math \
#        -falign-functions=2 -falign-jumps=2 -falign-loops=2

CFLAGS	= -Wall
CFLAGS	+= -O2
CFLAGS	+= -g
CC		= g++

INCLUDES = -Iinclude
LIBS = -lm

AR = ar
ARFLAGS = rs

LIB = labjack.a
LIBDIR = lib
LIBOBJ = u3.o battery_sensor.o depth_sensor.o labjack.o log_labjack.o \
         water_sensor.o labjackusb.o

all: $(LIB)

$(LIB): $(LIBOBJ)
	$(AR) $(ARFLAGS) $(LIBDIR)/$@ $(LIBOBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) $(OPTIM) $(INCLUDES) -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(OPTIM) $(INCLUDES) -c $<

clean:
	rm -f $(LIBOBJ) $(LIBDIR)/$(LIB)

