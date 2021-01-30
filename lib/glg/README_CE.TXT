THE COMMUNITY EDITION OF THE GLG TOOLKIT README FILE

The Community Edition of the GLG Toolkit contains the Community Edition 
of the following components:

 - GLG Graphics Builder
 - GLG HMI Configurator
 - GLG Widgets (Dials, Meters, Charts, etc.)
 - GLG C/C++ Libraries
 - GLG Java Class Library
 - GLG Graphics Server for JSP / Mobile
 - GLG Map Server

GLG GRAPHICS BUILDER

The Community Edition of the GLG Graphics Builder is installed in the 
following location:

    <glg_dir>/bin/GlgBuilderCE            - uses OpenGL driver
    <glg_dir>/bin/GlgBuilderCE_no_opengl  - uses X driver

GLG HMI CONFIGURATOR

The Community Edition of the GLG HMI Configurator is installed in the 
following location:

    <glg_dir>/bin/GlgHMIConfiguratorCE            - uses OpenGL driver
    <glg_dir>/bin/GlgHMIConfiguratorCE_no_opengl  - uses X driver

GLG WIDGETS

The Community Edition of all GLG widget sets is included:

    Controls (Dials, Meters, Sliders, Buttons and Toggles)
    Avionics Dials
    Real Time Charts
    2D Graphs
    3D Graphs
    Process Control Symbols
    Special Widgets (menus, spinners, misc. input widgets)

The widgets are available in the Graphics Builder's and HMI Configurator's
widget palettes. The widgets' drawings are located in the 
<glg_dir>/DEMOS/widgets directory.

GLG C/C++ LIBRARIES

The Community Edition of the GLG C/C++ Libraries is installed in the 
following location:

 Standard API:
   <glg_dir>/lib/libglg.a            - Motif version
   <glg_dir>/lib/libglg_lesstif.a    - lesstiff version
   <glg_dir>/lib/libglg_xt.a         - Xt version
   <glg_dir>/lib/libglg_x11.a        - X11 version

 Intermediate and Extended APIs
   <glg_dir>/lib/libglg_int.a        - Intermediate API
   <glg_dir>/lib/libglg_ext.a        - Extended API


 Map Server Libraries:
   <glg_dir>/lib/libglg_map.a
   <glg_dir>/lib/libglg_map_stub.a   - stubs for linking withou the map server

 Utility libraries:
   <glg_dir>/lib/GlgGraphLayout.o    - the Graph Layout module
   <glg_dir>/lib/libz.a              - needed for the GLG libs.
   <glg_dir>/lib/libjpeg.a           - needed for the GLG libs.
   <glg_dir>/lib/libtiff.a           - needed for the map server libs.
   <glg_dir>/lib/libfreetype.a       - needed for the map server libs, provided
                                       for non-Linux systems.
   <glg_dir>/lib/libfreetype_stub.a  - for linking with map server libs without
                                       the freetype support.
   <glg_dir>/lib/libOSMesa16.so      - for using hardware AND software OpenGL
   <glg_dir>/lib/libtess_util.so     - for using software OpenGL with no 
                                       OpenGL drivers installed.
 Include files directory:
   <glg_dir>/include

 C/C++ bindings:
   <glg_dir>/src/GlgClass.cpp

 Sample makefile:
   <glg_dir>/src/makefile

The following directories contain C/C++ demos and examples with the source 
code:

   <glg_dir>/DEMOS                  - C/C++ demos
   <glg_dir>/examples_c_cpp         - C/C++ examples

GLG JAVA CLASS LIBRARY

The Community Edition of the GLG Java Class Library is installed in the 
following location:

   <glg_dir>/lib/GlgCE.jar

The following directories contain Java demos and examples with the source code:

   <glg_dir>/DEMOS/java_demos     - Java demos
   <glg_dir>/examples_java        - examples of various programming tasks
   <glg_dir>/src/Player.java      - source code of a viewer for GLG drawings

GLG GRAPHICS SERVER

The Community Edition of the GLG Graphics Server is installed in the following 
location:

   <glg_dir>/lib/GlgServerCE.jar

The Graphics Server demos and examples with the source code are located in 
the following directory:

   <glg_dir>/examples_jsp

GLG MAP SERVER

The Map Server executable for the Community Edition is installed in the 
following location:

    <glg_dir>/map_server/bin/GlmMap

The map server sample dataset is installed in the following directory:

    <glg_dir>/map_data


DOCUMENTATION

Online Documentation for the GLG Toolkit is located at:

   http://www.genlogic.com/doc_html/start.html

NON-COMMERCIAL LICENSE

The Community Edition Non-Commercial License is located at:

   <glg_dir>/LICENSE_CE.TXT

It is also available online:

   http://www.genlogic.com/LICENSE_CE.TXT
