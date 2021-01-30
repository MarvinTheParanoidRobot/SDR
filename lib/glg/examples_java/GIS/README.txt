OVERVIEW

The GlgGISExample.java example demonstrates how to integrate mapping 
functionality with the dynamic features of the GLG Toolkit.

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

1. Provide custom implementation of GetLiveData method.
2. Set RANDOM_DATA flag in GlgGISExample.java to false.

This example is written with GLG Intermediate API.

ADDITIONAL EXAMPLES

The GlgGISExample2.java provides an additional functionality,
demonstrating how to add a list of target icons, such as airports, to
the map. A target may be selected with the mouse, making it currently
selected target. Once a target is selected, a DistancePopup overlay
is displayed, printing the name of the selected target and distance in
km between the target and moving aircraft. GetGlobeDistance() function
in this program is used to calculate distance in meters between two
points on the globe, defined in lat,lon coordinates.

The program also demonstrates how to turn ON/OFF map layers
dynamically at run-time, from a menu containing a list of available
layers. LayersDialog may be opened/closed using the ToggleMapLayers
toolbar button.
