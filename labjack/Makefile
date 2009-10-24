LIB			= labjack
LIBPREFIX	= lib
INCLUDES	= -Iinclude
CFLAGS		= -Wall
CFLAGS		+= -O2
CFLAGS		+= -g
CFLAGS		+= -fPIC
CC			= g++
AR			= ar
ARFLAGS		= rs
SOFLAGS		= -shared
LIBSO		= $(LIBPREFIX)$(LIB).so
LIBA		= $(LIBPREFIX)$(LIB).a
LIBDIR		= lib
SRCDIR		= .
LIBS		= -lm

LIBOBJ = u3.o battery_sensor.o depth_sensor.o labjack.o log_labjack.o \
         water_sensor.o labjackusb.o


all: $(LIBSO) $(LIBA)

$(LIBSO): $(LIBOBJ)
	$(CC) $(SOFLAGS) $(CFLAGS) $(INCLUDES) $(DEFINES) -o $(LIBDIR)/$@ $(LIBOBJ)

$(LIBA): $(LIBOBJ)
	$(AR) $(ARFLAGS) $(LIBDIR)/$@ $(LIBOBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) $(OPTIM) $(INCLUDES) -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(OPTIM) $(INCLUDES) -c $<

clean:
	rm -f *.o $(LIBDIR)/*.a $(LIBDIR)/*.so
