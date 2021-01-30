OVERVIEW

This example demonstrates how to integrate mapping functionality with the
dynamic features of the GLG Toolkit in a Java application using asynchronous GIS
map loading request.

The program loads and animates a GLG drawing (gis_example.g) containing a GIS
Object and a dynamic symbol whose position gets dynamically updated with new
lat/lon data. A separate panel on the right displays telemetry information,
including lat/lon values, altitude, pitch, roll, yaw, etc. 

Tags defined in the drawing are used to push dynamic data values into graphics.

Simulated data values for animation are supplied by the GetDemoIconData
function. The application developer should provide custom data acquisition code
to communicate with the application specific data acquisition system.

The example demonstrates how to implement a Pilot View mode, where the map
is centered on the moving dynamic symbol (Aircraft). 

This example uses the GLG Map Server to display a map. The Map Server has to be
installed either on the local host or on a remote web server. After the Map
Server has been installed, modify the example code so that:

  - The statement that sets the GISDisable resource is commented out:
    //GISObject.SetDResource( "GISDisabled", 1.0 );
 
  - Set SuppliedMapServerURL in the source code to point to the Map Server
    location.

SUPPORTED FEATURES

  - Asynchronous map image loading on initial appearance. 
    Can be controlled by the application.

  - Map zooming and panning using Asynchronous GIS Request.

  - Map dragging with left+click+drag. 
    A choice of synchronous or asynchronous map image loading on dragging.

  - PilotView mode, so that the map is automatically recentered when the
    aircraft moved by a specified minimum distance on the screen (by 2 pixels,
    for example).
 
  - Icon selection with the left or right mouse button, as defined 
    in the drawing via a Custom Event action attached to the icon object.

This example is written with GLG Intermediate API.
