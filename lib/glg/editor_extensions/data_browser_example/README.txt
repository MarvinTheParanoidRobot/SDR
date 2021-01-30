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

There are several ways to deploy the custom data browser library.

1. The -data-lib command line option may be used with either the GLG
   Graphics Builder or the HMI Configurator to deploy the data browser module
   from a specified location:

   Linux/Unix:
     <glg_dir>/bin/GlgBuilder -data-lib <path>/libglg_custom_data.so

  Windows:
    <glg_dir>\bin\GlgBuilder -data-lib <path>\glg_custom_data.dll

2. The CustomDataLib variable of the glg_config and glg_hmi_config
   configuration files may also be used to specify the location of the DLL.

3. Alternatively, the shared library / DLL may be copied to the
   directory where GlgBuilder or GlgHMIEditor resides, which depends on the
   version of the Toolkit:

     - Production Version:        data_browser_dir = <glg_dir>/bin
     - Evaluation Version:        data_browser_dir = <glg_dir>/eval/bin
     - Community Edition Version: data_browser_dir = <glg_dir>/DEMOS/main_demo

   Linux/Unix:
      cp libglg_custom_data.so <data_browser_dir>

   Windows:
      cp glg_custom_dala.dll <data_browser_dir>
