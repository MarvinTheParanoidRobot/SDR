DATASET DESCRIPTION AND USAGE

This demo uses a map server dataset located in the map_data
subdirectory of the GLG installation directory. This is a small
dataset provided for demo purposes. More detailed datasets with higher
resolution data may be obtained.

The following vector layers of the demo dataset are used in the demo,
as defined in the sample_dcw.sdf file:

  political_filled  - world political boundaries (filled polygons)
  states            - US states (edges)
  states_fill       - US states (filled polygons)
  us_cities         - US cities
  grid              - grid and grid labels
  outline           - globe outline for the orthographic projection

The path to the dataset is encoded in the GISDataFile attribute of the
GIS objects inside of the airtraffic.g drawing.

The "political_filled" layer contains political boundaries data from
the Digital Chart of the World, in the form of filled polygons which
can be used without the earth layer in the background. This is a simple
top-level dataset, more detailed data are also available.

RUNNING THE DEMO

To run the demo, change to the airtraffic directory and run

  ./airtraffic

MOVING THE DEMO OR DATASET DIRECTORIES

If the demo or map_data directories are moved to different
locations, the GISDataFile attribute of the GIS object has
to be adjusted using the GLG Builder or programmatically
(search for a "GISDataFile" comment in the source code).

MAKING DEMO SELF-CONTAINED

To make the demo self-contained, copy the GLG's map_data 
directory into airtraffic directory and set the GISDataPath
attributes of the GIS objects to

   map_data/sample.sdf

This will allow the airtraffic directory to be moved to any location
without adjusting the path to the map server data.
