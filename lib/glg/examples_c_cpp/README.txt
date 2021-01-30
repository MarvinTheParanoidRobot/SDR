This directory contains various examples of using GLG library in C and
C++, including source code and (on Windows) project files.

Examples in the "animation", "controls" and "graphs" directory provide
several versions that use different GLG interfaces: generic C/C++
interface (source code file has a 'G' suffix), or platform-specific
(Motif or Windows) interface.

Examples in the realtime_chart and misc directories have only a
generic interface. They use a C version of the GLG Generic API.

To build examples on Unix/Linux, use generic makefiles located in the
src directory of the GLG installation:
   <glg_dir>/src/makefile    - for building in directories containing
                               a single demo or example.
   <glg_dir>/src/makefile2   - for building in directories containing
                               multiple examples.
Refer to the README_BUILD.txt file in the GLG installation directory
for more information on makefiles (Unix/Linux only).

On Windows, use the provided Vusial Studio project files.

To build C++ examples, GlgClass.cpp file from "<glg_dir>/src" directory 
should be compiled and linked with the example. Refer to the 
README_BUILD.txt file in each example's directory for information 
on additional preprocessor define directives for the C++ build.
