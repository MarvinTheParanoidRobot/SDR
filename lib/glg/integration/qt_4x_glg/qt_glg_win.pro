######################################################################

TEMPLATE = app
TARGET = glg_example
DEPENDPATH += .
DEFINES = _WINDOWS
INCLUDEPATH += . C:\Glg\include
LIBS = C:\Glg\lib\GlgEx.lib

# Input
HEADERS += MainWindow.h QGlgApplication.h QGlgWidget.h
SOURCES += main.cpp MainWindow.cpp QGlgApplication.cpp QGlgWidget.cpp CustomWidget.cpp animation.c
