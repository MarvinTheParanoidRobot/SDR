GLG JAVA DEMOS

This directory contains the Java version of the GLG demos shown on the
GLG web site (http://www.genlogic.com/demos.html). While the online demos
use the GLG JavaScript library, the Java demos use the GLG Java Class Library
and may be run as stand-alone Java programs as described below.

LOCATION OF THE DEMO FILES

The Java demo files are located in the <glg_dir>/DEMOS_JAVA directory and
contain the following files:

         *.java         - Java source code files for the demos
         *.g            - GLG drawing files created with the GLG Graphics Builder
         GlgCE.jar      - GLG class library
         GlgDemo.jar    - JAR file containing class files of all demos

RUNNING THE GLG JAVA DEMOS

To run the Java version of the demos as standalone java programs,
change to the Java demos directory and set the CLASSPATH environment
variable to include the GlgCE.jar and GlgDemo.jar files:

	".:./GlgCE.jar:./GlgDemo.jar"

on UNIX, or:

	".;.\GlgCE.jar;.\GlgDemo.jar"

on Windows. 

Then start the demo of your choice by using one the following commands:

	java GlgSCADAViewer        // SCADA Viewer example
	java GlgAircombatDemo      // Aircombat Simulation
	java GlgGISDemo            // GIS AirTraffic Demo
	java GlgDiagramDemo        // Diagram Demo
	java GlgDiagramDemo -process-diagram     // Process Diagram Demo
	java GlgAirTrafficDemo     // AirTraffic Demo
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

To use the compiled class files, remove the GlgDemo.jar file from the CLASSPATH.
