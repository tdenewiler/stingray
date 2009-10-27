INCLUDES  = -Iinclude
INCLUDES 	+= -I../include
CFLAGS    = -Wall
CFLAGS    += -O2
CFLAGS    += -g
CC        = g++
LIB		  = joy.a
LIBDIR    = ../lib
EXE 	  = joy_test

OBJS = joy_test.o

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $^ -o $@ $(LIBDIR)/$(LIB)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

clean:
	rm -f *.o $(EXE)

