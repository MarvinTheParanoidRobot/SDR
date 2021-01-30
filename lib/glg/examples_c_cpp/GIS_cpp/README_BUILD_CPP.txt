BUILD NOTES

The example is written using GLG Intermediate API.

Define the GLG_CPP_INTERMEDIATE_API preprocessor definition
to compile the example.

On Linux/Unix:

    Use compilation option: -D GLG_CPP_INTERMEDIATE_API

On Windows: 

    Add GLG_CPP_INTERMEDIATE_API to the Preprocessor Definitions in the 
    Visual Studio project.

ADDING ICONS AT RUN TIME

To add new icons dynamically at run time, GLG Extended API has to be used 
instead of the Intermediate API.

Define the GLG_CPP_EXTENDED_API preprocessor definition
to compile the example.

On Linux/Unix:

    Use compilation option: -D GLG_CPP_EXTENDED_API

On Windows: 

    Add GLG_CPP_EXTENDED_API to the Preprocessor Definitions in the 
    Visual Studio project and change the GLG library from GlgIn.lib to
    GlgEx.lib.
