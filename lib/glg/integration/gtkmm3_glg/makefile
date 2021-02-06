# GMAKE makefile

CC=g++

GLG_HOME = /usr/local/glg
GTKMM = gtkmm-3.0

DEBUG_FLAGS = -g
#OPT_FLAGS = -O2

# Linking without the map server.
MAP_LIBS = -lglg_map_stub 

# When linking with the map server on Linux, uncomment one of the lines
#   below depending on the libtiff configuration. In Debian Buster,
#   static libtiff needs additional -lwebp and -lzstd libraries.
# On non-Linux Unix platforms, comment out both lines.
#EXTRA_TIFF_LIBS = -llzma -ljbig
#EXTRA_TIFF_LIBS = -llzma -ljbig -lwebp -lzstd

# Linking with the map server.
#MAP_LIBS = -lglg_map -ltiff $(EXTRA_TIFF_LIBS)

# On Linux, uncomment the line below for additional libraries.
# Due to a known bug in ldd/dlopen, -lpthread is required to avoid a crash 
# when OpenGL is used.
EXTRA_LIBS = -lpthread

# On Solaris, uncomment the next line for additional libraries
#EXTRA_LIBS = -lnsl -lsocket

# On AIX, uncomment the next line for additional libraries
#EXTRA_LIBS = -liconv

########################################################################
GTKMM_CFLAGS := $(shell pkg-config $(GTKMM) --cflags)
GTKMM_LIBS := $(shell pkg-config $(GTKMM) --libs)

GLG_INCLUDES = -I$(GLG_HOME)/include
GLG_LIB = $(GLG_HOME)/lib

LIBS= $(GTKMM_LIBS) -L$(GLG_LIB) \
	-lglg_x11 $(MAP_LIBS) -lXt -lX11 -lXft -lfontconfig -lfreetype \
	-ljpeg -lpng -lz -lm -ldl

INCLUDES := $(sort $(wildcard *.h))
SRCS_C	 := $(sort $(wildcard *.c))
SRCS_CPP := $(sort $(wildcard *.cpp))
SRCS	 := $(SRCS_C) $(SRCS_CPP)
OBJS 	 := $(sort $(SRCS_C:%.c=%.o)) $(sort $(SRCS_CPP:%.cpp=%.o))

TARGET  = glg_example
OBJECTS = $(OBJS)

CFLAGS  = $(DEBUG_FLAGS) $(OPT_FLAGS) $(GTKMM_CFLAGS) $(GLG_INCLUDES)

%.o : %.cpp $(INCLUDES)
	$(RM) $@
	$(CC) -c $(CFLAGS) \
	    $< -o $@ || ( rm $@ ; exit 1 )

%.o : %.c $(INCLUDES)
	$(RM) $@
	$(CC) -c $(CFLAGS) \
	    $< -o $@ || ( rm $@ ; exit 1 )

$(TARGET): $(OBJECTS)
	$(CC) $(DEBUG_FLAGS) $(OBJECTS) $(LIBS) \
		-o $@ || (rm $(TARGET))

clean:
	rm -f $(TARGET) $(OBJECTS)

