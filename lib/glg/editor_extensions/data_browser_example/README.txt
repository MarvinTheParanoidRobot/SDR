CUSTOM DATA BROWSER DLL EXAMPLE

This directory contains a sample code of a custom data browser module.
A data browser is used to browse available tags to be asigned to the
dynamic object attributes in the GLG Graphics Builder or HMI
Configurator. The data browser can be customized by an application
developer to present a user with a list of tag names associated with
an application-specific data source, such as a process database. A
custom data browser module is supplied in a form of a shared library
(DLL on Windows).

The custom data brwoser module is supplied in the form of a shared
library (DLL on Windows) and can be used with both the Graphics
Builder and HMI Configurator.

This sample uses a dummy data source just to illustrate the technique
for creating a custom data browser module.

RUNNING THE EXAMPLE

To run the example, run the following script in the example's directory:

   Linux/Unix:
      run_data_example
   
   Windows:
      run_data_example.bat

The script starts a Demo version of the GLG Builder by default.
To use a production version of the GLG Builder or GLG HMI Configurator,
edit the script to set the GLG_EXE variable to a desired executable.

When the Builder starts, add a tag to an object's attribute and click on 
the Browse button of the Data Tag dialog to start the tag browser. Select 
a controller, tag group and tag, then press Select to insert selected tag 
into the TagSource field.

EDITING AND COMPILING

To provide a custom version of the module that connects to a real data 
source, modify the sample.c file and rebuild the library using provided
glg_custom_dll.o file (glg_custom_dll.obj on Windows). 

NOTE: DON'T REMOVE glg_custom_dll.o (.obj)  FILE WHEN REBUILDING THE MODULE.

To build the shared library on Unix, use a sample makefile in this 
directory. To build a custom data DLL on Windows, load the enclosed 
project file into Visual Studio and build the DLL.

DEPLOYMENT

The -data-lib command line option may be used either the GLG
Graphics Builder or HMI Configurator to deploy the data browser module
from any location:

  Linux/Unix:
    <glg_dir>/bin/GlgBuilder -data-lib <glg_dir>/editor_extensions/data_browser_example/libglg_custom_data.so

  Windows:
    <glg_dir>\bin\GlgBuilder -data-lib <glg_dir>\editor_extensions\data_browser_example\glg_custom_data.dll

The CustomDataLib variable of the glg_config and glg_hmi_config
configuration files may also be used to specify the location of the DLL.

Alternatively, you may copy the shared library / DLL to the
<glg_dir>/bin directory where GlgBuilder resides. On Linux/UNIX, the
LD_LIBRARY_PATH environment variable (or LIBPATH environment variable on AIX) 
will also need to be defined to include the <glg_dir>/bin directory.

   Linux/Unix:
      cp libglg_custom_data.so /usr/local/glg/bin 
      export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/glg/bin

   Windows:
      cp glg_custom_dala.dll <glg_dir>\bin
