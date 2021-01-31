######################################################################

TEMPLATE = app
TARGET = glg_example
DEPENDPATH += .
INCLUDEPATH += . /usr/local/glg/include
LIBS = -L/usr/local/glg/lib -lglg_x11 -lglg_map_stub \
       -lXt -lX11 -ljpeg -lpng -lz -ldl

# On Linux, uncomment the line below for additional libraries.
# Due to a known bug in ldd/dlopen, -lpthread is required to avoid a crash 
# when OpenGL is used.
EXTRA_LIBS = -lpthread

# On Solaris, uncomment the next line for additional libraries
#EXTRA_LIBS = -lnsl -lsocket

# On AIX, uncomment the next line for additional libraries
#EXTRA_LIBS = -liconv

LIBS += $(EXTRA_LIBS)

QT += widgets
QT += x11extras

# Input
HEADERS += MainWindow.h QGlgApplication.h QGlgWidget.h 
SOURCES += main.cpp MainWindow.cpp QGlgApplication.cpp QGlgWidget.cpp \
           CustomWidget.cpp animation.c
