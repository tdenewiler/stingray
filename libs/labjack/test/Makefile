#CC = g++
#CFLAGS = -g -pedantic -ansi -Wall -fPIC
#OPTIM = -O3 -funroll-loops -fstrength-reduce -fomit-frame-pointer -ffast-math \
#        -falign-functions=2 -falign-jumps=2 -falign-loops=2
CFLAGS	= -Wall
CFLAGS	+= -O2
CFLAGS	+= -g
CC		= gcc


INCLUDES = -I../include
STDLIBS = -lm
LIBS = ../lib/labjack.a

EXE = labjack_test
OBJS = labjack_test.o

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(OBJS) -o $@ $(STDLIBS) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(OPTIM) $(INCLUDES) -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(OPTIM) $(INCLUDES) -c $<

clean:
	rm -f $(OBJS) $(EXE)

