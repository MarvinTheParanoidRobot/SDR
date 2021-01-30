The Community Edition of the GLG Map Server is a fully functional
version of the GLG Map Server for the CGI and Fast-CGI deployment. It
includes all features that are available in the commercial version and
is provided free of charge for non-commercial use. It is provided as a
service to the software development community for developing
non-commercial projects that do not have resources to purchase a
commercial license.

The download contains the map server executable for the selected
platform, a sample GIS dataset and map server documentation.

MAP SERVER FILES

The "map_server" directory of the download contains the following 
subdirectories:

 - bin directory contains the map server executable. On Linux, it also 
   contains the gis_image_import utility.

 - cgi-bin directory contains setup files for the web server setup

 - convert directory contains the source code of sample filters for 
   importing vector GIS data

 - src directory contains the source code of utilities for writing vector
   data in the GVF format

 - include directory contains the Gvf.h file for compiling the source code
   samples.

SAMPLE DATASET

The sample GIS dataset is located in the "map_data" directory and
contains the earth image, world political boundaries, and a few other
GIS layers. 

The map_data directory also contains scripts named request_*
that contain sample commands for generating map images using
command-line options. Both the Unix/Linux and Windows versions
are provided.

To use the scripts, edit the GLG_DIR and MAP_SERVER variables at
the beginning of the scripts to point to the location where the
downloaded files and map server executable are installed.

GLG MAP SERVER DOCUMENTATION

Online Documentation for the GLG Map Server is located at: 

   http://www.genlogic.com/doc_html/start.html#Map_Server_Reference

The map server FAQs with examples of GIS query requests are available 
online at:

   http://www.genlogic.com/map_server_faq.html

When the community edition of the GLG Map Server is downloaded as
a separate download, copies of the GLG Map Server Tutorial and 
GLG Map Server Reference Manual are also provided in the "doc" 
directory. 

GLG TOOLKIT DOCUMENTATION 

Online Documentation for the GLG Toolkit is located at:

   http://www.genlogic.com/doc_html/start.html

Documentation in the PDF format may be downloaded from:

   http://www.genlogic2.com/download/glg_doc_pdf.zip

WEB SERVER SETUP INSTRUCTIONS

The "Appendix A: Web Server Installation Notes" of the GLG Map Server
Reference Manual contains instructions for setting up the map server
as a CGI or FastCGI process on a web server, for both Unix/Linux and
Windows platforms.

FAST PROTOTYPING TOOLS

The GLG Graphics Builder provided with the Community Edition of 
the GLG Toolkit may be used as a fast prototyping tool for the 
GLG Map Server. 

You may download the Community Edition of the GLG Toolkit from 
the Generic Logic's web site (www.genlogic.com) and use its 
Graphics Builder to test and debug the map server setup interactively.

The first chapter of the GLG Map Server Tutorial provides detailed
instructions on prototyping the map server with the GLG Builder.
