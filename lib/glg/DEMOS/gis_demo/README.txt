DEMO VERSIONS
 
There are two source code files:
 
   gis_demo.c        - a complete demo with animated plane simulation.
   gis_demo_simple.c - a basic demo, shows how to place GLG icons on the map.  

DATASET DESCRIPTION AND USAGE

This demo uses a map server dataset located in the map_data
subdirectory of the GLG installation directory. This is a small
dataset provided for demo purposes. More detailed datasets with higher
resolution data may be obtained.

The following image and vector layers of the demo dataset (defined in
the sample.sdf) are used in the demo:

  earth       - earth image
  political   - political boundaries
  states      - US states (edges)
  states_fill - US states (filled polygons)
  us_cities   - US cities
  grid        - grid and grid labels

The path to the dataset is encoded in the GISDataFile attribute of the
GIS objects inside of the gis_demo.g drawing.

The "earth" layer is turned on by default in sample.sdf server dataset
file and uses a high-resolution 4Mx3M earth image provided by
NASA. The image is tiled to optimize performance of zoomed queries,
and a smaller image is used as a fallback for high-level views.

RUNNING THE DEMO

To run the demo, change to the gis_demo directory and run

  ./gis_demo

MOVING THE DEMO OR DATASET DIRECTORIES

If the demo or map_data directories are moved to different
locations, the GISDataFile attribute of the GIS object has
to be adjusted using the GLG Builder or programmatically
(search for a "GISDataFile" comment in the source code).

MAKING DEMO SELF-CONTAINED

To make the demo self-contained, copy the GLG's map_data 
directory into gis_demo directory and set the GISDataPath
attributes of the GIS objects to

   map_data/sample.sdf

This will allow the gis_demo directory to be moved to any location
without adjusting the path to the map server data.

DATASET DISCLAIMERS

The demo dataset supplied with the map server uses some publicly
available image and vector data. The earth image dataset is provided
by NASA. The following describes the image license for the earth
image:

The imagery is free for use.  The only restrictions are:

(1) NASA requires that they be given a credit, as owners of the imagery

(2) Visible Earth requests that you provide a credit (with URL if
possible) for them (http://visibleearth.nasa.gov/), since you found
the imagery with us.

(3) VE requests that you provide a credit for the Earth Observatory
team (http://earthobservatory.nasa.gov/) for generating the imagery.

Of course, only (1) is required, (2) and (3) are entirely optional and
have no effect on your permission to use the imagery.

