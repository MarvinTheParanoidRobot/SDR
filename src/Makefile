
IDIR =../include
CC=gcc

GLG_HOME=/home/pi/SDR/lib/glg

GLG_LIB = $(GLG_HOME)/lib
GLG_INCLUDES = -I$(GLG_HOME)/include

DEBUG_FLAGS = -g

CFLAGS=-I$(IDIR) $(DEBUG_FLAGS) $(OPT_FLAGS) -Wall  $(GLG_INCLUDES) `pkg-config --libs gtk+-3.0` `pkg-config --cflags gtk+-3.0` -export-dynamic 

MAP_LIBS = -lglg_map_stub 

EXTRA_LIBS = -lpthread

ODIR=obj
LDIR =../lib
LIBS=-lm -lfftw3f $(GTK_LIBS) -L$(GLG_LIB) \
	-lglg_x11 $(MAP_LIBS) -lXt -lX11 -lXft -lfontconfig -lfreetype \
	-ljpeg -lpng -lz -lm -ldl

_DEPS = Test_Data.h SDR.h tinywav.h gtkglg.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = SDR.o tinywav.o Test_Data.o Visual.o gtkglg.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

visual: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) 

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

run:
	./visual

