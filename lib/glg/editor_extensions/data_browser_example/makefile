# Uncomment the next line for gcc
#CC = gcc

#DEBUG_OPTS = -g
LD = ld

# Location of the GLG installation
GLG_DIR = /usr/local/glg

######################################################################
# Uncomment ONE set of appropriate platform variables below.
######################################################################

# Uncomment the next line for Linux
#LINUX = 1

# Uncomment the next two lines for 64-bit Linux 
#LINUX = 1
#LINUX_64_BIT = 1

# Uncomment the next line for Solaris
#SOLARIS = 1

# Uncomment the next line for HPUX
#HP = 1

# For HPUX on Itanium, also uncomment the next line.
#ITANIUM = 1

# Uncomment the next line for IRIX
#SGI = 1

# Uncomment the next line for AIX
#IBM = 1

######################################################################

# Name of the shared library to build.
TARGET = libglg_custom_data.so

# Custom object files that provide application-specific functionality.
OBJECTS = sample.o

# A supplied object file that provides GLG API.
GLG_OBJECTS = glg_custom_dll.o

######################################################################

# Solaris and Linux default
SYS_CFLAGS =
SYS_LD_FLAGS = -G
SYS_LIBS = 

ifdef LINUX
  SYS_CFLAGS = -Wmissing-prototypes -Wmissing-declarations
  DLL_PIC_FLAGS = -fPIC
endif

#ifdef LINUX_64_BIT
#  DLL_PIC_FLAGS = -fPIC
#endif
#
#ifdef LINUX_BB
#  DLL_PIC_FLAGS = -fPIC
#endif

ifdef HP
  ifndef ITANIUM
    SYS_FLAGS_PA = +DAportable
  endif

  SYS_CFLAGS = +Z $(SYS_FLAGS_PA)
  SYS_LD_FLAGS = -b
  SYS_LIBS = 
endif

ifdef SGI
  SYS_CFLAGS = -n32
  SYS_LD_FLAGS = -shared -n32
  SYS_LIBS = /usr/lib32/libcvt.so
endif

ifdef IBM
  SYS_CFLAGS = 
  SYS_LD_FLAGS = -G -bexpall -bnoentry
  SYS_LIBS = -lc
endif

######################################################################

ifndef GLG_INTERNAL
   GLG_INCL = $(GLG_DIR)/include
else
   GLG_INCL = ..  
endif

CFLAGS = $(SYS_CFLAGS) $(DEBUG_OPTS) $(DLL_PIC_FLAGS) -I$(GLG_INCL) 
LD_FLAGS = $(SYS_LD_FLAGS)

all : $(TARGET)
 
.SUFFIXES: .o .c

.c.o : 
	$(CC) -c $(CFLAGS) \
		$< -o $@;

$(TARGET) : $(OBJECTS) $(GLG_OBJECTS)
	( $(LD) $(LD_FLAGS) -o $@ $(OBJECTS) $(GLG_OBJECTS) $(SYS_LIBS) \
		|| ( rm $@ ; exit 1 ) )

clean :
	rm $(OBJECTS) ; :

sample.o : sample.c glg_custom_data.h glg_custom_dll.h 
