GTKMM-GLG INTEGRATION EXAMPLE

OVERVIEW

This example demonstrates how to use a GLG GTKMM Widget for embedding GLG 
functionality in a GTKMM application. The widget loads a GLG drawings
containing a panel of interactive dials, gauges and custom toggles. 
GLG API is then used to update the objects in the drawing with live data 
and handle user interaction with the objects in the drawing.

The GLG drawing was created using the GLG Buidler and contains samples
of the available GLG dials and gauges.

USING GLG C OR C++ API TO ACCESS OBJECTS IN THE DRAWING

The example uses a GLG C API to access the objects inside the GLG drawing. 
The viewport variable of the GtkmmGlgWidget class holds the object ID of the
loaded drawing. The GLG API provides a way to access any graphical object
defined in the drawing, change its attributes, supply dynamic data and
handle user interaction with the objects in the drawing (selection, user input,
mouse over, etc.).

Alternatively, a GLG C++ API can be used, in which case the type of the 
viewport variable in the GtkmmGlgWidget class will be of a GlgObjectC, instead 
of GlgObject. The GlgObjectC class is defined in the GlgClass.cpp file 
that provides GLG C++ bindings and located in the cpp directory of the GLG
installation.

OPENGL NOTES FOR UNIX/LINUX

If OpenGL warnings are generated by GLG at run-time due to not found or
missing libGLU library, the GLG OpenGL driver can be disabled by using 
one of the following options:

   - using the -glg-disable-opengl command line option

   - setting the GlgOpenGLMode global configuration resource to 0 
     in the application code after instantiating the first QGlgWidget 
     as follows:
       GlgSetDResource( NULL, "$config/GlgOpenGLMode", 0. );

   - setting GLG_OPENGL_MODE environment variable to False.
