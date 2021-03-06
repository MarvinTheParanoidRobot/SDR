The following GLG Java examples are provided in the following 
subdirectories:

Controls
   Demonstrates how to create multiple instances of a GLG bean, 
   each displaying its own drawing. The drawings are control 
   widgets from <glg_dir>/widgets/controls directory, and are 
   used without modifications. It also shows how to handle user 
   interaction in the control widgets, for example reterieving  
   values from a slider or a knob based on the user actions.
   
Dashboard
   Demonstrates how to use a GLG bean to display and interact
   with a GLG drawing containing a panel of GLG controls. 
   The drawing has been composed in the GlgBuilder and contains 
   several GLG control widgets.

RealTimeChart
   Demonstrates how to display and update a GLG realtime
   stripchart widget and use its integrated interactive behavior.

RealTimeChartSimple
   A basic version of the RealTimeChart example.

RealTimeChartAlarm
   In addition to the functionality shown in the RealTimeChart example,
   this example also demonsrates how to handle GLG Alarms using a global
   Alarm handler. The alarms are added to the chart's plots in 
   the GLG Builder.
	
RTChartMarkerFeedback
   In addition to the functionality shown in the RealTimeChart example,
   this example also demonsrates how to show/hide a marker for a data sample
   selected with a double click. 

BarGraph2D
   Demonstrates how display and update a GLG graph in a GLG bean.
   The GLG bean is used as a component inside another applet.

LineGraph2D
   Demostrates the use of the GLG single line graph. Displays the
   current time stamps as the X axis labels.

ObjectSelection
   Demonstrates how to handle GLG object selection events using 
   GLG InputCallback. It shows how to process event types such as
   CustomEvent and ObjectSelection.

Subclassing
   Demonstrates how to subclass a GlgBean and use it as a custom 
   graphics component inside another applet. 

ConstrainAttributes
   Demonstrates how to constrain object attributes.

CustomData
   Demonstrates how to add custom properties to an object,
   such as custom MouseClickEvent and TooltipString, as well as custom
   DataValue property of the D (double) type.

ObjectCreateAndMove
   Demonstrates the following features:
     - the use of the Trace callback;
     - how to create a polygon object using the mouse;
     - how to add an object to the drawing;
     - how to traverse a container object, such as a polygon;
     - how to convert screen coordinates to world coordinates;
     - how to select and move objects with the mouse.

ObjectPosition
   Demonstrates the following features:
     - the use of the Trace callback;
     - drag&drop an icon from a palette to the Drawing Area;
     - create a polygon object with the mouse;
     - scale an object as it is placed in the drawing area;
     - move an object with the mouse;
     - highlight the selected object by displaying a rectangle surrounding 
       the object's bounding box; 
     - cut/paste operations;
     - save/load operations.

Blinking
   Demonstrates how to use integrated blinking feature of the 
   GLG Toolkit.

ListWidget
   Demonstrates how to use GLG List Widget, handle input events
   occured in the widget, as well as use the list widget's messages
   to query the list widget's settings and modify its items.

OptionMenu
   Demonstrates how to use GLG OptionMenu Widget and its messages.

TagsExample
   Demonstrates how to use GLG tags feature to query a list of tags
   defined in a drawing, and use them to animate the drawing with data.

SCADA_Viewer
   An example of a generic SCADA Viewer application that loads a GLG drawing
   and updates all tags defined in the drawing with realtime data.
   It also demonstrates how to navigate between several drawings.
   The example can be extended with custom application-specific 
   functionality.    

GIS
  GlgGISExample.java
    Demonstrates how to integrate GIS mapping functionality with the 
    dynamic features of the GLG Toolkit. It uses the GLG GIS object to
    display a map generated by the GLG Map Server, and shows how to 
    display a dynamic icon on top of the map.

    Since the Java version of the GLG GIS object uses the Map Server
    in the form of a cgi-bin web server process, the GLG Map Server
    needs to be set up on a local web server in order to run this
    example. Installation and setup instructions may be found in the
    Appendix A of the Map Server Reference Manual.

    The drawing used in this example, gis_example.g, also needs to be
    modified so that GISMapServerURL property of the GIS object points
    to the correct MapServer URL.

    This example uses the Extended API, but it can be rewritten to use 
    the Standard API only.

  GlgGISExample2.java
    Demonstrates how to integrate GIS mapping functionality with the 
    dynamic features of the GLG Toolkit. In addition to the previous 
    example, it also shows how to position an array of icons defined
    by their lat/lon coordinates, select them with the mouse and 
    calculate distances on the map, as well as how to toggle map layers
    based on user requests.

    Since the Java version of the GLG GIS object uses the Map Server
    in the form of a cgi-bin web server process, the GLG Map Server
    needs to be set up on a local web server in order to run this
    example. Installation and setup instructions may be found in the
    Appendix A of the Map Server Reference Manual.

    The drawing used in this example, gis_example2.g, also needs to be
    modified so that GISMapServerURL property of the GIS object points
    to the correct MapServer URL.

  GIS_PilotView
    Demonstrates how to integrate mapping functionality with the dynamic
    features of the GLG Toolkit in a Java application using asynchronous GIS
    map loading request. It also demonstrates "pilot view" functionality,
    with the map periodically adjusted to keep it centered on the moving
    aircraft icon.

All examples may be used either as applets in a browser or as standalone 
Java programs. The examples have to be built prior to running them.

WINDOWS NOTE: On Windows, replace "/" with "\" and use "copy" command 
              instead of "cp" in the compilation instructions below.

COMPILING THE EXAMPLES

  1. Change to the directory containing the example you want to build,
     for example <glg_dir>/examples_java/Graphs.

     cd <glg_dir>/examples_java/Graphs

  2. Copy GlgCE.jar from <glg_dir>/DEMOS/java_demos/java2
     to the directory of the example:
  
     cp <glg_dir>/DEMOS/java_demos/java2/GlgCE.jar .

  3. Set CLASSPATH environment variable to contain GlgCE.jar:
  
    on Windows in the MSDOS prompt 
      set CLASSPATH=.;.\GlgCE.jar

    on Unix
      export CLASSPATH=.:./GlgCE.jar

  4. Invoke Java compiler (javac) to compile a desired example. 
     For example, to compile GlgGraphComponent.java, execute
   
     javac GlgGraphComponent.java

   
RUNNING EXAMPLES AS APPLETS IN A BROWSER

Examples have to be built first. Refer to the compilation instructions
described above for details how to build the examples.

To run an example as an applet, load a corresponding .html file into a
browser.

RUNNING EXAMPLES AS STANDALONE JAVA PROGRAMS

  1. Examples have to be built first. Refer to the compilation instructions
     described above.

  2. Make sure CLASSPATH environment variable includes GlgCE.jar:

       on Windows in the MSDOS prompt 
         set CLASSPATH=.;.\GlgCE.jar

       on Unix
         export CLASSPATH=.:./GlgCE.jar

  3. Run a desired example by invoking JVM (java). For example, to
     run the GlgGraphComponent example, execute the following command
     in the Graphs subdirectory.

     java GlgGraphComponent

