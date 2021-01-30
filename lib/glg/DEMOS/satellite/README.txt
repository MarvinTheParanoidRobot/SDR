DATASET DESCRIPTION AND USAGE

This demo uses a map server dataset located in the map_data
subdirectory of the GLG installation directory. This is a small
dataset provided for demo purposes. More detailed datasets with higher
resolution data may be obtained.

RUNNING THE DEMO

To run the Satellite demo, change to the satellite directory and run

  ./satellite -satellite

To run the Trajectory demo, change to the satellite directory and run

  ./satellite -trajectory

or run the provided trajectory script.

MOVING THE DEMO OR DATASET DIRECTORIES

If the demo or map_data directories are moved to different
locations, the GISDataFile attribute of the GIS object has
to be adjusted using the GLG Builder or programmatically
(search for a "GISDataFile" comment in the source code).

MAKING DEMO SELF-CONTAINED

To make the demo self-contained, copy the GLG's map_data 
directory into satellite directory and set the GISDataPath
attributes of the GIS objects to

   map_data/sample.sdf

This will allow the satellite directory to be moved to any location
without adjusting the path to the map server data.
