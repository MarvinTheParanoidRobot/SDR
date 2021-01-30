This directory contains a sample dataset for the GLG Map Server.

The following layers are defined in the sample.sdf files:

  earth  	    earth image (on by default in sample.sdf)
  shadow            shaded relief image for overlaying elevation data
  political   	    political boundaries (edges)
  political_filled  political boundary areas (on by default in sample_dcw.sdf)
  states	    US States, outlines
  states_dcw	    same data as the states layer, but different edge color.
  states_fill       US States, filled
  us_cities         US Cities
  grid              grid, gray color
  grid30            grid, gray30 color
  grid50            grid, gray50 color
  grid70            grid, gray70 color
  outline           globe outline for the orthographic projection
  default_gis       an alias containing earth and states layers
  default_air       an alias containing political_filled, states_dcw, grid50
                    and outline layers.

Layer and alias names can be used in a comma separated list that specifies
layers to be displayed on the map, for example:

   earth,political,grid

DATASET LAYER DETAILS

The "earth" layer is turned on by default in sample.sdf server dataset
file and uses a high-resolution 8192x4096 earth image provided by
NASA. The image is tiled to optimize performance of zoomed queries,
and a smaller image in the earth_fallback layer is used as a fallback 
for high-level views.

The "political" layer contains political boundaries data in the form of
unfilled polygons that can be overlayed on top of the earth image.

The "political_filled" layer contains political boundaries data from
the Digital Chart of the World, in the form of filled polygons which
can be used without the earth layer in the background. This is a simple
top-level dataset, more detailed data are also available.

The "us_cites" dataset contains US cities data.

The states datasets contains US States data that could be used with
other datasets.

DATASET DISCLAIMERS

The demo dataset supplied with the map server uses some publicly
available image and vector data. 

The earth image dataset is provided by NASA. The following describes
the image license for the earth image:

The imagery is free for use.  The only restrictions are:

(1) NASA requires that they be given a credit, as owners of the imagery

(2) Visible Earth requests that you provide a credit (with URL if
possible) for them (http://visibleearth.nasa.gov/), since you found
the imagery with us.

(3) VE requests that you provide a credit for the Earth Observatory
team (http://earthobservatory.nasa.gov/) for generating the imagery.

Of course, only (1) is required, (2) and (3) are entirely optional and
have no effect on your permission to use the imagery.

The shaded relief image is derived from the Global Land One-kilometer Base 
Elevation (GLOBE) Digital Elevation Model, Version 1.0., by:

GLOBE Task Team and others (Hastings, David A., Paula K. Dunbar,
Gerald M. Elphingstone, Mark Bootz, Hiroshi Murakami, Hiroshi
Maruyama, Hiroshi Masaharu, Peter Holland, John Payne, Nevin
A. Bryant, Thomas L. Logan, J.-P. Muller, Gunter Schreier, and John
S. MacDonald), eds., 1999. The Global Land One-kilometer Base
Elevation (GLOBE) Digital Elevation Model, Version 1.0. National
Oceanic and Atmospheric Administration, National Geophysical Data
Center, 325 Broadway, Boulder, Colorado 80303, U.S.A. Digital data
base on the World Wide Web 
(URL: http://www.ngdc.noaa.gov/mgg/topo/globe.html) and CD-ROMs.

