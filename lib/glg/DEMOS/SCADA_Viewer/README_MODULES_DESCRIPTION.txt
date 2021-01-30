MODULES AND INCLUDE FILES

scada_config_menu.txt 
   Configuration text file used to dynamically populate a navigation
   menu upon application start-up. The file uses a simple text format
   that can be edited with a text editor; each line of the file
   defines a drawing file associated with a navigation button, as well
   as well as other associated information to be used in the menu.
   The file format is just a sample, and can be modified by an
   application as needed.

GlgSCADAViewer.c, GlgSCADAViewer.h
   Main demo module. GlgSCADAViewer structure represents a class that
   encapsulates the viewer functionality, including page navigation,
   animation, user interaction, alarm display.

DataFeed.c, DataFeed.h 
   Encapsulation of a generic DataFeed interface, including Read,
   Write and other operations to interface between custom back-end
   data acquisition sytem and the graphics. The DataFeed structure
   contains a set of function pointer allowing an application to
   provide custom implementation of the datafeed interface, which can
   be either the same for all loaded pages, or different for
   individual pages.

   The Read functions are used for input, to push new values from the
   back-end system to the graphics, and Write functions are used for
   output, to write values from the graphics to the back-end system.

   GetAlarms function is used to obtain a list of system alarms. The
   alarms are displayed in the GUI using the Alarms dialog.

   In the demo, all HMI pages use the GLG tags mechanism for drawing
   animation. The TagSource parameter of each tag represents a name of
   the datasource variable used to obtain a new value from the
   real-time back-end system. The new value is obtained by the
   ReadDTag/ReadSTag method and gets pushed into the graphics using a
   corresponding tag source.

   Tag sources may be assigned at design time and stored in the .g
   file, or they may be assigned dynamically at run-time, using
   applicaion specific logic. RemapTags method of the GLGSCADAViewer
   class provides an example of tag sources assignment at run-time.

DemoDataFeed.c, DemoDataFeed.c

   Custom implementation of the DataFeed to supply simulated demo data
   for pages' animation. Used in random demo mode, which can be
   enabled either by setting Viewer.RANDOM_DATA to True, or via
   -random-data command line option.

LiveDataFeed.c, LiveDataFeed.h
   Application specific implementation of the data communication
   between the graphics and the back-end data acquisition
   system. LiveDataFeed contains function stubs for Read, Write and
   other operations. The application developer should provide
   application specific code for these functions to communicate with
   the data acquisition system.
 
   The Read function (ReadDTagData for numerical data or ReadSTagData
   for a string type data) obtains a new data value from the back-end
   system for a specified tag source, which represents a name of a
   real-time datasource variable. It is invoked periodically by
   GlgSCADAViewer for each tag defined in the loaded page. The new
   data value obtained by the Read function gets pushed into the
   graphics using a corresponding tag source.

   The Write functions (WriteDTagData and WriteSTagData) are used to
   write values to the back-end data acquisition system.

   The GetAlarms function obtains a list of system alarms.
   
   LiveDataFeed mode may be enabled either by setting
   Viewer.RANDOM_DATA to False, or via -live-data command line option.

HMIPage.c, HMIPage.h
   Encapsulation of a generic HMIPage interface, including handlers
   for pages' initialization, animation and user interaction.

   HMIPage structure includes are handlers invoked by the
   corresponding methods in the main GlgSCADAViewer class. If a
   handler in HMIPageBase returns true, the common code in the
   corresponding GlgSCADAViewer method will not be
   executed. Otherwise, the corresponding GlgSCADAViewer function will
   proceed executing code common for all HMI pages.

   This mechanism provides flexibility and scalability, allowing an
   application to treat an individual page either as a default page with
   common functionality, or extend HMIPageBase to provide special page
   handing as needed. 

   In the demo, DefaultHMIPage, ProcessPage and RTChartPage classes
   extend HMIPageBase class. DefaultHMIPage handles a default page,
   ProcessPage class handles Solvert Recovery page, and RTChartPage
   class handles RealTimeChart page.


DefaultHMIPage.c, DefaultHMIPage.h
   Implementation of the generic HMIPage a common (Default) page. In the
   demo, default pages are: Water Treatment, Electric Circuit, Test
   Commands. 

   The custom implementation of the HMIPage interface may implement
   all or some of the page's functionality. For example, a custom
   implementation may include page's initialization code, while
   animation and user interaction are handled in a generic way using
   common methods.
   
   PageTypeTable is defined in GlgSCADAViewer.c. Page type is
   identified by the PageType custom property stored in the page's
   drawing file. Upon page loading, the viewer checks PageType of the
   loaded viewport and handles the page accordingly. 

ProcessPage.c, ProcessPage.h 
   Handles Solvent Recovery page, providing custom implementation of
   the generic HMIPage. The page uses process.g drawing that has
   PageType property of the $Widget viewport set to "Process".

RTChartPage.c, RTChartPage.h 
   Handles RealTimeChart page, providing custom implementation of the
   generic HMIPage. The page uses scada_chart.h drawing that has
   PageType property of the $Widget viewport set to "RealTimeChart".

DataTypes.h        
   Defines data structures for GlgTagRecord, AlarmRecord, etc.

scada.h
   Includes enum definitions, structures, function prototypes, etc.
   
config.c
   Contains utility functions that parse configuration file and build
   an array of records used to populate the navigation menu.

util.c
   Defines tables used to process object commands. Each table is used
   to map command strings defined in the drawing to corresponding
   enums used in the code.
