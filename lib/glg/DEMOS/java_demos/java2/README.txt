GLG JAVA DEMOS

The GLG Java demos may be run from the www.genlogic.com website, 
or from a local filesystem without an internet access.

When the demos are run from a local file system, they may be run 
either in a web browser, or as stand-alone Java programs.

LOCATION OF THE DEMO FILES

The Java demo files are located in the <glg_dir>/DEMOS/java_demos
directory of the GLG Demo Download:

   java_demos          - the top-level demo directory

      index.html       - the starting point for running the demos in a browser
      demos.html       - another starting point for running the demos 
      *.gif, *.jpg      - image files used by index.html

      java2            - Java demo directory
         *.html           - demo html files 
         *.java           - Java source code files for the demos
         *.glg, *.g       - GLG drawing files
         GlgCE.jar        - GLG class library
         GlgDemo.jar     - demo class files

RUNNING THE GLG JAVA DEMOS IN A BROWSER

You need a Java2-enabled browser to run Java and Swing demos. 
To enable Java in the browser, Sun's Java2 plug-in needs to be installed.

To run the demos, start your Web browser, load the java_demos/index.html file,
then click on the Java demo you want to run.

RUNNING THE GLG JAVA DEMOS AS STAND-ALONE JAVA PROGRAMS

You can run the demos by either browsing the demos in a web browser
(starting with index.html), or starting the demos as stand-alone Java
programs.

To run the Java version of the demos as standalone java programs,
change to the "java_demos/java2" directory and set the CLASSPATH
environment variable to include the GlgCE.jar and GlgDemo.jar files:

	".:./GlgCE.jar:./GlgDemo.jar"

on UNIX, or:

	".;.\GlgCE.jar;.\GlgDemo.jar"

on Windows. 

Then start the demo of your choice by using one the following
commands:

	java GlgSCADAViewer        // SCADA Viewer example
	java GlgAircombatDemo      // Aircombat Simulation
	java GlgGISDemo            // GIS AirTraffic Demo
	java GlgDiagramDemo        // Diagram Demo
	java GlgDiagramDemo -process-diagram     // Process Diagram Demo
	java GlgAirTraffiocDemo    // AirTraffic Demo
	java GlgMapDemo            // Supply Chain Visualization
	java GlgProcessDemo	   // Process Control Demo
	java GlgRobotArmDemo	   // 3D Simulation Demo
	java GlgAnimationDemo      // Simple game animation
	java GlgAvionicsDemo       // Avionics Dashboard
	java GlgGraphDemo	   // GLG graph samples
	java GlgGraphCrossWireDemo // Custom graph with a cross-hair cursor
	java GlgGraphLayoutDemo	   // Graph layout example
	java GlgNetworkDemo	   // Network Monitoring Demo

To run the Graph Layout Demo, the GlgGraphLayout.jar must also be
added to the CLASSPATH.

On both Windows and Unix, you can use JRE to run the demos:

	jre -cp .;.\GlgCE.jar;.\GlgDemo.jar <demo_name>   (on Windows)

	jre -cp .:./GlgCE.jar:./GlgDemo.jar <demo_name>   (on Unix)

To recompile the demo source code using the same class path, use the
following command:

	javac <demo_name>.java
