MODULES AND CLASSES
   
scada_config_menu.txt 
   Configuration text file used to dynamically populate a navigation
   menu upon application start-up. The file uses a simple text format
   that can be edited with a text editor; each line of the file
   defines a drawing file associated with a navigation button, as well
   as well as other associated information to be used in the menu.
   The file format is just a sample, and can be modified by an
   application as needed.

GlgSCADAViewer
   Main demo class. Extends GlgJBean (Java), GlgControl (C#) or
   GlgObjectC (C++). Encapsulates the viewer functionality, including
   page navigation, animation, user interaction, alarm display.
   
   For Java and C#, also defines various tables, such as PageTypeTable
   to handle loaded HMI pages, CommandTypeTable to process various
   object commands, etc.  These tables are used to map command strings
   defined in the drawing to corresponding enums used in the code.

   For C++, PageTypeTable is defined in GlgSCADAViewer.cpp, while
   other tables are defined in scada.h.

DataFeedInterface (Java and C#)
DataFeedBase (C++)
   Encapsulation of a generic DataFeed interface, including Read,
   Write and other operations to interface between custom back-end
   data acquisition sytem and the graphics. Defines a set of methods
   allowing an application to provide custom implementation of the
   datafeed interface, which can be either the same for all loaded
   pages, or different for individual pages.

   The Read methods are used for input, to push new values from the
   back-end system to the graphics, and Write methods are used for
   output, to write values from the graphics to the back-end system.

   GetAlarms method is used to obtain a list of system alarms.
   The alarms are displayed in the GUI using the Alarms dialog.

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

DemoDataFeed
   Custom implementation of the DataFeed to supply simulated demo data
   for pages' animation. DemoDataFeed class implements
   DataFeedInterface in Java and C#, or extends DataFeedBase class in
   C++. Used in random demo mode, which can be enabled either by
   setting RANDOM_DATA to true, or via -random-data command line
   option.

LiveDataFeed
   Application specific implementation of the data communication
   interface between back-end data acquisition system and the
   graphics. LiveDataFeed class implements DataFeedInterface in Java
   and C#, or extends DataFeedBase class in C++. Contains stubs for
   Read, Write and other operations. The application developer should
   provide application specific code for these methods to communicate
   with the data acquisition system.
 
   Read method (ReadDTag for numerical data or ReadSTag for a string
   type data) obtains a new data value from the back-end system for a
   specified tag source, which represents a name of a real-time
   datasource variable. It is invoked periodically by GlgSCADAViewer
   for each tag defined in the loaded page. The new data value
   obtained by the Read method gets pushed into the graphics using a
   corresponding tag source.

   The WriteDTag and WriteSTag methods are used to write values to the
   back-end data acquisition system.

   The GetAlarms method obtains a list of system alarms.

   LiveDataFeed mode may be enabled either by setting RANDOM_DATA to
   false, or via -live-data command line option.

HMIPageBase
   Encapsulation of a generic HMIPage interface, including methods
   for pages' initialization, animation and user interaction. 

   HMIPageBase class methods are handlers invoked by the corresponding
   methods in the main GlgSCADAViewer class. If a handler in
   HMIPageBase returns true, the common code in the corresponding
   GlgSCADAViewer method will not be executed. Otherwise, the
   corresponding GlgSCADAViewer method will proceed executing code
   common for all HMI pages.

   This mechanism provides flexibility and scalability, allowing an
   application to treat an individual page either as a default page with
   common functionality, or extend HMIPageBase to provide special page
   handing as needed. 

   In the demo, DefaultHMIPage, ProcessPage and RTChartPage classes
   extend HMIPageBase class. DefaultHMIPage handles a default page,
   ProcessPage class handles Solvert Recovery page, and RTChartPage
   class handles RealTimeChart page.

DefaultHMIPage
   Extends HMIPageBase and provides implementation for handling a
   common (default) HMI page. 

   In the demo, default pages are: Water Treatment, Electric Circuit,
   Test Commands.

   Page type is identified by the PageType custom property stored in
   the page's drawing file. Upon page loading, the viewer checks
   PageType of the loaded viewport and handles the page
   accordingly. PageTypeTable is defined in GlgSCADAViewer class.

   The custom implementation of an individual page may implement all
   or some of the page's functionality. For example, a custom
   implementation may include page's initialization code, while
   animation and user interaction are handled in a generic way using
   common methods of a base class. 

ProcessPage
   Extends HMIPageBase and provides implementation for handling the
   Solvent Recovery page, The page uses process.g drawing that has
   PageType property of the $Widget viewport set to "Process".

RTChartPage
   Extends HMIPageBase and provides implementation for handling the
   RealTimeChart page. The page uses scada_chart.h drawing that has
   PageType property of the $Widget viewport set to "RealTimeChart".

EmptyPage 
   Extends HMIPageBase and creates an empty page.

GlgTagRecord 
   Contains tag information, including a tag source that represents
   a name of the datasource variable used to obtain a new value from
   the real-time back-end system. The new value gets pushed into the
   graphics using a corresponding tag source. 

AlarmRecord
   Contains alarm data for each alarm in the system. May be extended
   by the application developer as needed.

DataPoint (Java and C#)
   Contains information for an individual data point.

PlotDataPoint
   Contains information for an individual data sample of a chart widget.

ActiveDialogRecord
ActivePopupMenuRecord
   Contain information about active popup dialogs and popup menus.

GlgViewerControl (MFC only)
  Extends GlgControlC. Used in MFC framework to add a GLG user control
  to an MFC View. GlgViewerControl includes a handle to the Viewer
  object of type GlgSCADAViewer, which gets assigned to the viewport
  of the loaded top level drawing scada_main.g. Viewer object handles
  the functionality.

scada.h (C++ only)
   Includes enum definitions, structures, function prototypes, etc.
   
config.cpp (C++ only)
   Contains utility functions that parse configuration file and build
   an array of records used to populate the navigation menu.

util.cpp (C++ only)
   Defines tables used to process object commands. Each table is used
   to map command strings defined in the drawing to corresponding
   enums used in the code.
