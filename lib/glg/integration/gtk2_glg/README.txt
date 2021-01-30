GTK-GLG INTEGRATION EXAMPLE

This example demonstrates how to use a GLG QT Widget for embedding GLG 
functionality in a GTK application. The widget loads a GLG drawings
containing a panel of interactive dials, gauges and custom toggles. 
GLG API is then used to update the objects in the drawing with live data 
and handle user interaction with the objects in the drawing.

The GLG drawing was created using the GLG Buidler and contains samples
of the available GLG dials and gauges.

OPENGL NOTES FOR UNIX/LINUX

If OpenGL support is not available, the GLG OpenGL driver can be disabled 
by using one of the following options:

   - using the -glg-disable-opengl command line option

   - setting the GlgOpenGLMode global configuration resource to 0 
     in the application code as follows:
       GlgSetDResource( NULL, "$config/GlgOpenGLMode", 0. );

   - setting GLG_OPENGL_MODE environment variable to False.
