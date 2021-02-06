JSP / TOMCAT SETUP

This directory contains JSP servlet examples of the GLG Graphics Server.

To deploy the examples, copy the "glg_demos" directory into the "webapps"
directory of the JavaEE server (Tomcat, etc.) and load any html file from 
the "webapps/glg_demos" directory into the web browser via a URL.

For example, for a typical Tomcat installation on a Linux-based local host, 
use the following URLs:

    http://localhost:8080/glg_demos/ajax_demos.html
    http://localhost:8080/glg_demos/process_demo.html

MAP SERVER SETUP FOR THE GIS DEMO

The GIS Demo uses the GLG Map Server to display a map. The GLG Map Server 
has to be installed either on the local host or on a remote web server.
Refer to the GLG Map Server documentation for setup instructions.

After the Map Server has been installed, enable the map display by modifying 
the source code of the GIS demo:

   <glg_dir>/examples_jps/glg_demos/WEB-INF/src/glg_demos/GlgGISServlet.java

1. Comment out the following two statements:

   GISObject.SetDResource( "GISDisabled", 1.0 );
   Map.SetDResource( "JavaSetupInfo/Visibility", 1.0 );

2. Initialize the SuppliedMapServerURL variable in the GlgGISServlet.java 
   source code to point to the GLG Map Server location, rebuild the demo 
   and restart the JSP server.

GLG JAVA SCRIPT LIBRARY

The common tasks for communicating with the GLG JSP servlet are encapsulated
in the GLG Script Object, which frees the application from writing low-level 
Java Script code. The glg_script.js file contains the source code of the 
GLG Script Object and is simply included in an HTML page. 

The Java Script code in the OnLoad() method of the HTML page sets
parameters of the GLG Script Object and provides methods for custom
event handling, after which the Script Object handles low-level details
of interaction with the JSP server.

GLG JAVA SCRIPT OBJECT: DOCUMENTATION AND EXAMPLES

The glg_script.txt file in the <glg>/examples_jsp/glg_demos directory
contains description of the public methods of the GLG Script Object.

The HTML pages of the demos in the <glg>/examples_jsp/glg_demos directory
provide examples of using the GLG Script Object for displaying updating
image and handling various user actions, such as tooltips or object selection.

COMPILING JSP EXAMPLES

To compile a JSP example, change directory to 
   <glg_dir>/examples_jsp/glg_demos/WEB-INF/src

Set CLASSPATH, replacing <servlet-api-path> with the path to the directory
containing the servlet-api.jar file:

  On Linux:
    export CLASSPATH=.:<servlet-api-path>/servlet-api.jar:../lib/GlgServerCE.jar

  On Windows:
    set CLASSPATH=.;<servlet-api-path>\servlet-api.jar;..\lib\GlgServerCE.jar

Compile a desired example, for example:

  javac -d ../classes glg_demos/GlgSimpleViewer.java

To deploy, copy the "glg_demos" directory into the "webapps" directory of the
JavaEE server as described at the beginning of this file.
