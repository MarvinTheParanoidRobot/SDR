
IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR)  -Wall `pkg-config --libs gtk+-3.0` `pkg-config --cflags gtk+-3.0` -export-dynamic

ODIR=obj
LDIR =../lib
LIBS=-lm -lfftw3f 

_DEPS = Test_Data.h SDR.h tinywav.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = SDR.o tinywav.o Test_Data.o Visual.o
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