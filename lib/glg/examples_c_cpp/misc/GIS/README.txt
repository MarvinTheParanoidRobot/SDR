.


SUPPORTED COMMAND LINE OPTIONS

  -random-data  
        uses simulated demo data for animation

  -live-data
        uses live application data for animation
 
  <filename>
         specifies GLG drawing filename to be loaded and animated;
         if not defined, DefaultDrawingName is used.

OVERVIEW

This example demonstrates how to integrate mapping functionality with
the dynamic features of the GLG Toolkit.

The program loads and animates a GLG drawing containing a GIS Object
and an icon whose position gets dynamically updated with new lat/lon
data.

The drawing filename may be supplied as a command line argument.
If not supplied, a default drawing name defined in the application code
is loaded. In this example, the default drawing name is "gis_example.g".

Dynamic data values for animation are supplied by the GetDemoData
function if random data are used for animation, and GetLiveData
function if live data are used. The application developer should
provide custom data acquisition code in GetLiveData to communicate
with the application specific data acquisition system.

The example provides a choice of 2 icon symbols, a triangle or an
aircraft. An icon with a specified name will be displayed on the map,
and the other icon will be invisible. Icon names are specified as
ICON_NAME1 and ICON_NAME2.

SUPPORTED FEATURES

  - Map zooming and panning.
  - Map dragging with left+click+drag.
  - Icon selection with the left or right mouse button, as defined 
    in the drawing via a Custom Event action attached to the icon object.
  - Icon tooltip.
  - Right click on the map displays lat/lon coordinates at the bottom 
    of the drawing, using the InfoObject.

HOW TO ENABLE LIVE DATA FOR ANIMATION

1. Provide custom implementation of the GetLiveData() function.
2. Set RANDOM_DATA flag in GlgGISExampleG.c to False, 
   or use -live-data command line option. 

This example is written with GLG Intermediate API.


