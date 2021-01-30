CUSTOM COLOR PALETTE TEMPLATE

The custom_color_palette.g drawing is a sample of a custom color
palette with predefined colors. It may be used by applications that
use a predefined list of colors to implement an company-wide
color scheme. It is commonly used in a process control applications
that use color coding.

The palette drawing can be modified using the GLG Graphics Builder.
The modified drawing may be used by simply copying it to the top 
level of the GLG's installation directory. Alternatively, it may also 
be used in the GLG Builder or HMI configurator via the 
-custom-color-palette command-line option or the GLG_CUSTOM_COLOR_PALETTE 
environment variable, or by setting the CustomColorPalette option in the
GLG configuration file. Refer to the GLG Builder documentation for more 
information.

The template drawing must contain a GLG color palette widget named
CustomColorPalette. The group (or series) named PaletteObject inside
the widget contains color cells whose FillColor attribute define the
selected color. The sample palette widget also contains optional labels.

Refer to the description of the GlgPalette interaction handler in the
Input Objects chapter of the GLG User's Guide and Builder Reference
Manual for more information.

INDEXED COLOR PALETTE TEMPLATE

The indexed_color_palette.g drawing is a sample of a custom color
palette that uses indexed colors. If an indexed color is changed in the 
palette, all colors in the drawing that use this indexed color will change
as well. The use of indexed colors allows an application to change colors 
used by application's drawings at run time.

An indexed color palette can be used in the GLG Graphics Builder and
HMI Configurator. The same file can be used at run time to define an 
indexed color palette used by an application, by setting either the 
GlgIndexedColorTable global configuration resource or the 
GLG_INDEXED_COLOR_TABLE environment variable. 

Alternatively, a simple text file containing a list of color values
can be used at run time to define an application's indexed color palette,
by setting either the GlgIndexedColorFile global configuration resource 
or the GLG_INDEXED_COLOR_FILE environment variable. The indexed_colors.txt
file provides examples of specifying indexed colors in various formats.
Refer to the GLG Builder documentation for more information.

CUSTOM TRANSFORMATIONS TEMPLATE

The custom_xform_template.g drawing contains custom transformation
templates for the GLG editors. The custom transformations are used by
both the GLG Graphics Builder and HMI Configurator to present the user
with simplified application-specific choices of dynamics.

The template drawing can be modified using the Enterprise version of the
GLG Graphics Builder in the OEM mode to add new custom transformations
or modify the existing ones. The modified template drawing may be used
via the Builder's -xform-templates command-line option or the
GLG_CUSTOM_XFORM_TEMPLATES environment variable. Refer to the GLG
Builder documentation for more information.

The template drawing must have a viewport named XformTemplates that
contains several rows of objects corresponding to a transformation
subtype as indicated by the row's name: ColorXform, VisibilityXform,
DXform, etc. Each row is a group containing objects that define custom
transformations in that group. The name of the object will be used as
the name of the transformation in the Builder's transformation
menu. Each object has a transformation named XformObject attached to
one of its attributes, as well as an XformLabel custom property of an
S type that defines the transformation type displayed in the Builder's
dynamic editing dialog.

A custom transformation is a transformation that has an export tag of 
the EXPORT_DYN type attached to some of its properties; the exported 
propertries will be visible as the public properties of the custom 
transform. A custom transform may contain an elaborate combination
of several transformations and constraints which will be presented 
to the user as a simplified list of public properties as defined by 
the attached export tags.

If a custom transformation is a list or threshold transformation with
a single item in the list, the Builder will ask the user for a number 
of items in the list when the custom transformation is selected and 
will create the requested number of the list items.

When the transformations from the template are used in the Builder,
the Builder scans objects in each row of the template and extract 
custom transformations defined using the convention described above.
The content of each row is used to create a menu of transformations
for one transformation subtype. The order of items in the menu is 
defined not by the visual order of the objects in the row, but by the
drawing object which can be cahnged using Arrange/Reorder buttons of 
the main menu or the Front/Back toolbar icons.
