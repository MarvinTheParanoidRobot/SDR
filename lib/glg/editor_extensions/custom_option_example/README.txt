CUSTOM EDITOR OPTIONS DLL EXAMPLE

This directory contains a sample code of a custom editor customization
module. The module may be used to perform an OEM customization of the
GLG editor by adding custom icons, menu options and dialogs with
application-specific logic. The module may also be used to verify the
drawing against a custom set of rules before saving it into a file, as
well as remove unwanted editor icons and menu options.

The custom prototyping module is supplied in the form of a shared
library (DLL on Windows) and can be used with both the Graphics
Builder and HMI Configurator.

This sample implementation demonstrates how to add a custom OEM menus
and toolbar icons to a GLG editor. The code provides examples of
implementing both push and toggle buttons, as well as cascading
sub-menus. It also shows how to change sensitivity of the menu options
depending on an object selection and the GLG editors mode: Edit or
Run.

One of the OEM menu options demonstrates how to implement a custom OEM
dialog that performs a custom OEM action in the editor's Edit mode. The
example also has code samples showing how to customize the GLG editor
by removing unwanted icons and menu options.

RUNNING THE EXAMPLE

To run the example, run the following script in the example's directory:

   Linux/Unix:
      run_option_example
   
   Windows:
      run_option_example.bat

The script starts a Demo version of the GLG Builder by default.
To use a production version of the GLG Builder or GLG HMI Configurator,
edit the script to set the GLG_EXE variable to a desired executable.

When the Builder starts, the OEM Sample Menu appears in the main menu
after the Edit menu and also in the popup menu. A custom OEM icon 
(a red square with the OEM label) apears near the right side of the
editor's toolbar. The OEM icon and menu entries become active when an
object is selected.

Create an object, select it and try the OEM menu options. The Add/Edit
Custom Value menu option and the OEM toolbar icon activate an OEM
dialog that adds an OEM property to the selected object and allows the
user to edit its value. The property is vsible in the resource Browser
as OEMProperty. The sample code also checks and sets the objects
HasResource flag if necessary. The code demonstrates how to implement
both modal and non-modal custom dialogs.

The OEM menu contains options for both the Edit and Run modes. Starting 
the Run mode disables edit options and enables runtime options of the
OEM menu. The custom editor options DLLs may also implement the
functionality of the custom run mode DLL described in the previous
section, making it possible to provide a single DLL that handles both
the OEM editor options and the OEM runtime mode.

EDITING AND COMPILING

To provide a custom version of the module that implements
application-specific functionality modify the sample.c file and rebuild
the library using the provided glg_custom_dll.o and glg_custom_editor_dll.o 
files (glg_custom_dll.obj and glg_custom_editor_dll.obj on Windows).

NOTE: DON'T REMOVE glg_custom_dll.o (.obj) AND glg_custom_editor_dll.o (.obj) 
      FILES WHEN REBUILDING THE MODULE.

Since the module uses the GLG APIs of the GLG editor, it may use both
the GLG Standard and the Extended API with no additional libraries
required. When the module is used with the GLG Graphics Builder or HMI
Configurator, the editors provide the module with the Run-Time license
for the Extended API, allowing the developer to create elaborate
custom applications integrated with the GLG editors.

To build the shared library on Unix, use a sample makefile in this
directory. To build the customization module DLL on Windows, load the
enclosed project file into Visual Studio and build the dll.

DEPLOYMENT

The -option-lib command line option may be used either the GLG
Graphics Builder or HMI Configurator to deploy the customization
module from any location:

  Linux/Unix:
    <glg_dir>/bin/GlgBuilder -option-lib <glg_dir>/editor_extensions/custom_option_example/libglg_custom_option.so

  Windows:
    <glg_dir>\bin\GlgBuilder -option-lib <glg_dir>\editor_extensions\custom_option_example\glg_custom_option.dll

The CustomOptionLib variable of the glg_config and glg_hmi_config
configuration files may also be used to specify the location of the DLL.

Alternatively, you may copy the shared library / DLL and any associated .g 
files to the <glg_dir>/bin directory where GlgBuilder resides. 
On Linux/UNIX, the LD_LIBRARY_PATH environment variable (or LIBPATH
environment variable on AIX) will also need to be defined to include the 
<glg_dir>/bin directory.

   Linux/Unix:
      cp libglg_custom_option.so /usr/local/glg/bin 
      cp *.g /usr/local/glg/bin 
      export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/glg/bin

   Windows:
      cp glg_custom_option.dll <glg_dir>\bin
      cp *.g <glg_dir>\bin

ADDING CUSTOM ICONS

The -editor-icons command line option or the GLG_EDITOR_ICONS
environment variable can be used to supply a custom icons template
drawing for the Buildewr or HMI Configurator. The custom_editor_icons.g 
and custom_hmi_icons.g drawings provide samples of icon templates for
both the Builder and HMI Configurator that contain a custom icon. The
custom icon has an "OEM" label and is highlighted with a red
background. The custom DLL code provides an example of managing 
the custom icon.

The template drawing may contain either one or two viewports named
ToolbarPalette and IconPalette, which define icons for the Toolbar
Palette and Object Palette. When adding custom icons to the template,
one has to follow GLG Editor conventions. The icons in the template
are located in viewport objects that are named "Icon<N>", where <N> is
a sequential icon index that controls the order of icons. The name of
the viewport's Visibility attribute is used a place holder that keeps
the icon's label, which is passed the the DLL's ManageIcon()
method. The name of the viewport's EdgeColor attribute is a place
holder for the icon's tooltip.

The icon's graphics is named "$Drawing" and is placed inside the
viewport.  The $Drawing object may contain an ActiveState resource of
D type that changes the icon's appearance when the icon is disabled. 
If the $Drawing object contains the ActiveState resource, the $Drawing's
HasResources should be set to NO in order for the ActiveState resource
to be visible at the level of the icon's viewport.

The easiest way to add a new custom icon is by using the following steps:
- copy an existing icon viewport
- change the viewport's name index and adjust indexes of other icon 
  viewports to define the position of the new icon
- change the name of the viewport's Visibility attribute to define a unique 
  icon label for the new icon
- change the name of the viewport's EdgeColor attribute to define the icon's
  tooltip
- don't delete the $Drawing group as it contains ActiveState and related 
  transformation for changing icon's appearance when disabled -  they can 
  be reused
- edit or replace objects inside the $Drawing group inside the viewport
  to define the icon's graphical appearance
- modify the custom option DLL to manage the custom icon and build the DLL
- test the new custom icon using the -editor-icons option to specify
  the new icon template file and the -option-lib option to specify
  the modified custom option DLL.
