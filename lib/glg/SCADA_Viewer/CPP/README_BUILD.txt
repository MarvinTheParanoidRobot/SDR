HOW TO BUILD AND RUN THE DEMO

Linux/Unix:

   A sample makefile may be found in the <glg_dir>/src/ directory. The
   makefile may be edited as needed to use different paths for the
   include files and libraries, as well as to modify other settings.

   The makefile grabs all source files in the directory and builds an
   executable with the filename defined by the TARGET variable (set to
   "demo" by default). The make file assumes that GLG is installed
   in /usr/local/glg by default.

   To build and run GLG SCADAViewer demo, copy the makefile into the
   SCADA demo directory, modify it as needed following the comments in
   the makefile. For C++, also change the default compiler from gcc to
   g++. Then build and run the demo:

      cd <scada_demo_dir>
      cp <glg_dir>/src/makefile .
      # Edit makefile as described above.
      make
      ./demo

   For the EVALUATION version of the GLG Toolkit, modify the GLG_LIB
   variable in the makefile as described in the comments.

   By default, the demo uses simulated data for animation. This mode
   may be also specified explicitly using -random-data command line
   option:

      ./demo -random-data

   To use live application data for animation, supply application
   specific code in LiveDataFeed and run the demo with -live-data
   command line option:

      ./demo -live-data

   Refer to README_MODULES_DESCRIPTION.txt for more details.

Windows:
   
   Use the provided Visual Studio project to build SCADAViewer.

   For the Production or Community Edition GLG installation, the project
   uses GLG libraries located in the <glg>\lib directory.

   To use the Evaluation version of the Toolkit, change the project 
   link settings to use libraries located in <glg>\eval\lib directory.

   To run the demo, start SCADAViewer.exe executable.

   Simulated demo data are used for animation by default. To enable
   this mode explicitly, specify -random-data command line option in
   the Visual Studio project, or run the demo from an MS DOS prompt:

      SCADAViewer.exe -random-data

   To use live application data for animation, supply application
   specific code in LiveDataFeed and run the demo with -live-data
   command line option. Specify this command line option to run the
   demo from the Visual Studio, or run the demo from an MSDOS prompt:

      SCADAViewer.exe -live-data
    
   Refer to README_MODULES_DESCRIPTION.txt for more details.
