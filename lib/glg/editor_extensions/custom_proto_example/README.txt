CUSTOM RUN MODULE DLL EXAMPLE

This directory contains a sample code of a custom prototyping module
used to animate the drawing with real data in the Run Mode, as well as
handle user interaction, object selections and custom runtime dialogs
with an application-specific runtime logic.

The custom prototyping module is supplied in the form of a shared
library (DLL on Windows) and can be used with both the Graphics
Builder and HMI Configurator.

This sample implementation queries the list of tags defined in the 
drawing and animates them with random data. It also demonstrates the
use of resources as well as the use of a custom run-time popup dialog.

USING THE MODULE TO IMPLEMENT AN INTERGRATED APPLICATION

The module has an access to a complete GLG API, both the Standard and
Extended, making it possible to implement a complete application
integrated with a GLG editor. The application will function in the
editor's Run mode, while the Edit mode may be used for editing the
application's drawing.

For even further customization, the -run command-line option or the
StartRun configuration file variable can be used to start the GLG
editor in the Run mode. The -run-window command-line option or the
RunWindow configuration file variable can be used to start the Run
mode in a separate window, hiding the GLG editor's menus and toolbars.
The custom option DLL may be used to add custom OEM menu options for
the Run mode.

Since the module uses the GLG APIs supplied by the GLG editor's
executable, it may use both the GLG Standard and the Extended API with
no additional GLG libraries required. When the module is used with the
GLG Graphics Builder or HMI Configurator, the editors provide the
module with the Run-Time license for the Extended API.

RUNNING THE EXAMPLE

To run the example, run the following script in the example's directory:

   Linux/Unix:
      run_proto_example
   
   Windows:
      run_proto_example.bat

The script starts a Demo version of the GLG Builder by default.
To use a production version of the GLG Builder or GLG HMI Configurator,
edit the script to set the GLG_EXE variable to a desired executable.

When the Builder starts, start the Run mode of the Builder. The DLL will 
load and display the popup dialog from the dialog.g file and update it 
with the status information using resources. The DLL receives and processes 
all input events. When the user presses the Stop button, the DLL stops 
the Run mode of the Builder.

EDITING AND COMPILING

To provide a custom version of the module that connects to real data
and uses them to animate the drawing, modify the sample.c file and
rebuild the library using the provided glg_custom_dll.o file
(glg_custom_dll.obj on Windows).

In addition to the cross-platform GLG-based dialogs, the module may
also use native dialogs, based on Windows' Win32 API or Xt/Motif on
Linux/Unix.

NOTE: DON'T REMOVE glg_custom_dll.o (.obj)  FILE WHEN REBUILDING THE MODULE.

To build the shared library on Unix, use a sample makefile in this 
directory. To build a proto module DLL on Windows, load the enclosed 
project file into Visual Studio and build the DLL.

DEPLOYMENT

The -proto-lib command line option may be used either the GLG
Graphics Builder or HMI Configurator to deploy the proto module from
any location:

  Linux/Unix:
    <glg_dir>/bin/GlgBuilder -proto-lib <glg_dir>/editor_extensions/custom_proto_example/libglg_custom_proto.so

  Windows:
    <glg_dir>\bin\GlgBuilder -proto-lib <glg_dir>\editor_extensions\custom_proto_example\glg_custom_proto.dll

The CustomProtoLib variable of the glg_config and glg_hmi_config
configuration files may also be used to specify the location of the DLL.

Alternatively, you may copy the shared library / DLL and any associated .g 
files to the <glg_dir>/bin directory where GlgBuilder resides. 
On Linux/UNIX, the LD_LIBRARY_PATH environment variable (or LIBPATH
environment variable on AIX) will also need to be defined to include the 
<glg_dir>/bin directory.

   Linux/Unix:
      cp libglg_custom_proto.so /usr/local/glg/bin 
      cp *.g /usr/local/glg/bin 
      export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/glg/bin

   Windows:
      cp glg_custom_proto.dll <glg_dir>\bin
      cp *.g <glg_dir>\bin
