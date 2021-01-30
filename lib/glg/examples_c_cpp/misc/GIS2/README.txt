The drawing file used for this example, gis_example2.g, may need to be
modified to use the correct path for the GISDataFile property of the
GIS object. 

By default, GISDataFile property is set to
"/usr/local/glg/map_data/sample.sdf". If the GLG Toolkit is
installed in the directory other than "/usr/local/glg", this path
should be changed to point to <glg_dir>/map_data/sample.sdf, where
<glg_dir> is a GLG installation directory.

To set GISDataFile property of the GIS object:

  Load gis_example2.g into the GlgBuidler 
  Select $Widget viewport 
  Click on ObjectResources toolbar button to bring resources 
    of the $Widget viewport (selected object) 
  In the Resource browser: 
    Doubleclick on MapVp 
    Doubleclick on GISObject 
    Select GISDataFile resource 
    In the ResourceObject dialog, set the correct path to the sample.sdf file
   
  
  
