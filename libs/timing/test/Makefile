# Executable.
EXE         = test_timing

# Directories.
SRCDIR		= .
TIMINGDIR	= ../lib

# Includes.
INCLUDES	= -Iinclude
INCLUDES	+= -I../include

# Compiler and flags.
CFLAGS      = -Wall
CFLAGS      += -O2
CFLAGS      += -g
CC          = g++

# Standard libraries to include.
#STDLIBS     = -lm
LIBS		= $(STDLIBS)
LIBS		+= $(TIMINGDIR)/libtiming.a

# Glib flags for type definitions.
#LLIBS = `pkg-config --libs glib-2.0`
#GLIBFLAGS = `pkg-config --cflags glib-2.0`

# Object files.
OBJS = $(SRCDIR)/$(EXE).o


all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $^ -o $@ $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $< 
	
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $< 

clean:
	rm -f *.o $(EXE)

