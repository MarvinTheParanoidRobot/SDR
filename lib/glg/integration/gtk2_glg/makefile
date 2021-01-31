# GMAKE makefile

CC = gcc

GLG_HOME=/usr/local/glg
GTK = gtk+-2.0

DEBUG_FLAGS = -g
#OPT_FLAGS = -O2

# Linking without the map server.
MAP_LIBS = -lglg_map_stub 

# Linking with the map server.
#MAP_LIBS = -lglg_map -ltiff -ltiff -lfreetype

# On Linux, uncomment the line below for additional libraries.
# Due to a known bug in ldd/dlopen, -lpthread is required to avoid a crash 
# when OpenGL is used.
EXTRA_LIBS = -lpthread

# On Solaris, uncomment the next line for additional libraries
#EXTRA_LIBS = -lnsl -lsocket

# On AIX, uncomment the next line for additional libraries
#EXTRA_LIBS = -liconv

########################################################################
GTK_CFLAGS = $(shell pkg-config $(GTK) --cflags)
GTK_LIBS = $(shell pkg-config $(GTK) --libs)

GLG_LIB = $(GLG_HOME)/lib
GLG_INCLUDES = -I$(GLG_HOME)/include

LIBS = $(GTK_LIBS) -L$(GLG_LIB) \
	-lglg_x11 $(MAP_LIBS) -ljpeg -lpng -lz -lXt -lX11 -lm -ldl

CFLAGS = $(DEBUG_FLAGS) $(OPT_FLAGS) -Wall $(GTK_CFLAGS) $(GLG_INCLUDES) 

INCLUDES := $(sort $(wildcard *.h))
SRCS_C	 := $(sort $(wildcard *.c))
OBJS 	 := $(sort $(SRCS_C:%.c=%.o))

TARGET  = glg_example
OBJECTS = $(OBJS)

%.o : %.c $(INCLUDES)
	$(RM) $@
	$(CC) -c $(CFLAGS)  \
	    $< -o $@ || ( rm $@ ; exit 1 )

$(TARGET): $(OBJECTS)
	$(CC) $(DEBUG_FLAGS) $(OBJECTS) $(LIBS) \
		-o $@ || (rm $(TARGET))

clean: 
	rm -f $(TARGET) $(OBJECTS)
