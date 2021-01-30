HOW TO BUILD AND RUN THE DEMO

JAVA

   For the Evaluation or Community Edition GLG installation, set the 
   CLASSPATH environment variable to include GlgCE.jar located in the 
   project directory:
  
    on Windows in the MSDOS prompt
      set CLASSPATH=.;.\GlgCE.jar

    on Unix/Linux
      export CLASSPATH=.:./GlgCE.jar

   For the Production GLG installation, copy <glg_dir>/lib/GlgInt2.jar
   to the project directory and set the CLASSPATH environment variable 
   to include GlgInt2.jar:

    on Windows in the MSDOS prompt
      set CLASSPATH=.;.\GlgInt2.jar

    on Unix/Linux
      export CLASSPATH=.:./GlgInt2.jar

   Invoke Java compiler (javac) to compile GlgSCADAViewer:
   
      javac GlgSCADAViewer.java

   To run GlgSCADAViewer as a stand-alone Java program:

      java GlgSCADAViewer
   
   By default, the demo uses simulated data for animation. This mode
   may be also specified explicitly using -random-data command line
   option:

      java GlgSCADAViewer -random-data

   To use live application data for animation, supply application
   specific code in LiveDataFeed and run the demo with -live-data
   command line option:

      java GlgSCADAViewer -live-data

   Refer to README_MODULES_DESCRIPTION.txt for more details.

   To run GlgSCADAViewer as an applet in a browser, copy the project 
   directory to a web server and load the scada_viewer.html file into
   a web browser. For the Production installation, change scada_viewer.html
   to use GlgInt2.jar instead of GlgCE.jar.

C#/.NET
   Use the provided Visual Studio project to build the Viewer. 

   For the Evaluation or Community Edition GLG installation, the project
   uses the Community Edition version of the GLG C# class library 
   (Glg.NetCE.dll).

   For the Production GLG installation, copy the <glg_dir>\lib\Glg.NetInt.dll
   library to the project directory and change the Reference section of the 
   project to use Glg.NetInt.dll instead of Glg.NetCE.dll.

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
