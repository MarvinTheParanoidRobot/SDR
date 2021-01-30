######################################################################

TEMPLATE = app
TARGET = glg_example
DEPENDPATH += .
INCLUDEPATH += . /usr/local/glg/include
LIBS = -L/usr/local/glg/lib -lglg_x11 -lglg_map_stub \
       -lXt -lX11 -lXft -lfontconfig -lfreetype -ljpeg -lpng -lz -ldl

# Uncomment next line if the GLG OpenGL driver will be used,
#   in which case QGLWidget is used as a base class of QGlgWidget.
# The line may be commented out if the GLG X11 driver is used,
#   to use QWidget as a base class. This may cause Qt to select a parent
#   widget's XVisual that doesn't work with OpenGL in some system setups.
#GLG_OPENGL = true

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

contains( GLG_OPENGL, true ) {
DEFINES += GLG_OPENGL
QT += opengl
}

# Input
HEADERS += MainWindow.h QGlgApplication.h QGlgWidget.h 
SOURCES += main.cpp MainWindow.cpp QGlgApplication.cpp QGlgWidget.cpp \
           CustomWidget.cpp animation.c
