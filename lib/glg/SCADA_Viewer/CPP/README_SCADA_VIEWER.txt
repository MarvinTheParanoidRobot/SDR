GLG SCADA VIEWER DEMO 

The demo demonsrates a design of a generic SCADA Viewer application. 

The Viewer is meant to be designed by a System Integrator in such a
way that it can display and animate arbitrary graphical pages created
by End Users using the GLG HMI Configurator (or the GLG Builder).

The Viewer handles both Read and Write operations defined in graphical
pages by the End User via tags and user commands, as well as navigation 
and drill-down functionality defined by the user in the page.

The Viewer is written using the GLG Intermediate API which is royalty-free.

BUILDING AND RUNNING THE DEMO

Refer to the README_SCADA_VIEWER_BUILD.txt file for information on how to
build and run the demo.

Refer to the README_MODULES_DESCRIPTION.txt file for information about
source code modules.

SUPPORTED COMMAND LINE OPTIONS
  -random-data
       use simulated demo data for animation, using DemoDataFeed
  -live-data
       use live real-time application data for animation, using LiveDataFeed

DESCRIPTION OF FEATURES

GlgSCADAViewer demo demonsrates a design of a generic SCADA Viewer
application and includes the following functionality:
 
- Navigation between various drawings (pages) using navigation menu buttons. 

- Animating loaded drawings using a list of tags defined in each
  drawing. Dynamic data values for animation are supplied either using
  simulated data in DemoDataFeed module, or live application data
  in LiveDataFeed module. The application developer should provide
  custom data acquisition code for Read/Write functions in
  LiveDataFeed to communicate with the application specific data
  acquisition system.

  To use live data for animation: 
  
  1) Provide custom implementation of the Read/Write methods in
     LiveDataFeed module.  
  2) Set RANDOM_DATA flag in GlgSCADAViewer module to False, 
     or use -live-data command line option.

   Page animation is implemented via the use of tags added to the
   objects' data driven parameters. The TagSource parameter of each
   tag represents a name of the datasource variable used to obtain a
   new value from the real-time back-end system. The new value gets
   pushed into the graphics using a corresponding tag source.

   Tag sources may be assigned at design time and stored in the .g
   file, or they may be assigned dynamically at run-time, using
   applicaion specific logic. RemapTags method of the GLGSCADAViewer
   class provides an example of tag sources assignment at run-time.

- Handling of a loaded page using either a common mechanism
  (DefaultHMIPage), or provide special handling for those pages that
  need custom functionality not present in a default page, such as
  RealTimeChart page (RTChartPage) or SolventRecovery page
  (ProcessPage). Special handling may include page initialization,
  animation, user interaction, etc.

  Pages that need special handling are identified by the PageType
  custom property added to the $Widget viewport and stored in the .g
  file. At run-time, the viewer checks PageType of the loaded viewport
  and handles the page accordingly. A table of known page types is
  defined in GlgSCADAViewer module, PageTypeTable array.

  For example, Water Treatment page is a default page and is handled
  by DefaultHMIPage class. The Solvent Recovery page loads a drawing
  file process.g that has PageType="Process". The page's animation
  using simulated data and user interaction are handled by the
  ProcessPage class. The RealTimeChart page loads scada_chart.g
  drawing with PageTYpe="RealTimeChart. Page's initialialization,
  animation and user interaction are handled by the RTChartPage class.
 
- Populating and configuring a navigation menu by reading a configuration
  file. The demo uses configuration file scada_config_menu.txt by
  default; a custom configuration file can be specified as a command
  line argument via the -config option:

     <viewer_exe> -config <config_filename> 

  If configurarion file cannot be parsed, a hard-coded array
  MenuTable[] is used as a fallback to populate the menu.

- Displaying a dialog with a scrolling list of system alarms.

- Handling object selection events in the loaded drawing.

- Handle commands attached to objects at design time, such as
  PopupDialog, PopupMenu, GoTo, WriteValue, WriteValueFromWidget,
  etc. When the command is triggered, it is processed in the Input
  callback by invoking the ProcessCommands() function.

- Test Commands button of the navigation menu displays the
  scada_test_commands.g test page with examples of various object
  commands. The commands include activation of either global or
  embedded popup menus and popup dialogs, navigating to a different
  page via a GoTo command, as well as executing Write commands to
  control the system based on user input.

- Resizing behavior: if the Viewer's window is resized, the displayed
  page is resized as well, while the interface controls on the left
  and the status area at the bottom remain fixed size.
   
GUI LAYOUT AND DESIGN

The top level drawing (scada_main.g) is loaded upon application
start-up and represents a graphical interface for the viewer layout.
It contains the following elements:

- A menu object named "Menu", used to navigate between graphical pages.
  It is initialzed based on the configuration file on application start-up.

- DrawingArea subwindow object, used to load and display a new
  page when the user selects a navigation menu button, or clicks
  on a symbol or a button that has an attached GoTo command.

- AlarmDialog, displayed when the user clicks on the AlarmList button
  on the left. The button has an attached "PopupDialog" command with
  DialogType="AlarmDialog"; the command is processed in the Input
  callback.
   
  AlarmDialog displays a scrlolling table containing a list of
  application alarms. The table layout is created in the GLG Builder
  and can be customized as needed.

- PopupDialog, a global popup dialog viewport displayed when a user
  clicks on an object that has a "PopupDialog" command with
  DialogType="Popup". The dialog contains the DrawingArea subwindow
  object that displays the drawing specified by the command's
  DrawingFile parameter.

  To view the PopupDialog in the demo, select the Water Treatment
  navigation menu button and click on a motor symbol represented by a
  colored square. The dialog loads and displays the scada_motor_info.g
  drawing specified by the command attached to the symbol.

  The spinner inside the dialog drawing allows the user to write a new
  value to the back-end system via the WriteValueFromWidget command
  attached to the spinner. In the demo, the spinner is used to write a
  new value to the tag that represents the motor speed.

  The popup dialog is context-sensitive: when the dialog is activated,
  its tags are initialized to display data related to the selected
  object. It is done by the TransferTags function that transfers tags
  with matching tag names from the selected object to the dialog
  drawing.

  Once the dialog tags are initialized, they are included in the
  global list of tag records (TagRecordsArray) that is used to animate
  the drawing. Each time the PopupDialog is opened, closed or its
  drawing is switched, TagRecordsArray gets rebuilt using the
  QueryTags function.
  
- A global PopupMenu viewport displayed when a user clicks on an
  object that has a "PopupMenu" command with MenuType="PopupMenu". The
  viewport contains the DrawingArea subwindow object that displays the
  drawing specified by the command's DrawingFile parameter.  The
  global popup menu is shared by all graphical pages, but displays
  different menu draings depending on the selected symbol.

  To view the global popup menu in the demo, select the Test Commands
  navigation menu button and right-click on the motor symbols in the
  upper right. The popup menu is displayed with the popup_menu3.g
  drawing that contains the STOP, STANDBY and RUNNING buttons for
  changing the motor state.

  The buttons inside a popup menu have a "WriteValue" command that
  writes a new value to the tag in the back-end system when the user
  clicks on a button. Both the value and the output tag are defined by
  the command parameters. For example, clicking on the STOP button
  writes the value of 0 to the tag specified by the button's command,
  while clicking on the STANDBY button writes the value of 1. the
  WriteValue command is processed in the code by the WriteValue()
  function.

- Embebded popup menu and embedded popup dialog that are included in
  the current page may be used instead of the global ones described
  above. The embedded versions may be used in cases when a page needs
  a special popup dialog or menu that is specific to this page.

  To view embedded popups, use the Test Commands navigation menu
  button and left-click on a valve symbol in the upper left. It will
  activate a popup menu with two buttons for opening or closing the
  selected valve using a "WriteValue" command attached to each button.

GLG DRAWINGS USED IN THE DEMO
    
scada_main.g
   Contains the top level GUI used by the viewer.

process.g, scada_aeration.g, scada_electric.g, scada_chart.g, 
scada_test_commands.g
   Sample graphical pages that are loaded at run-time when the user
   selects a corresponding navigation menu button. These drawings are
   listed in the "scada_config_menu.txt" configuration file.
   New drawings can be created in the GLG HMI Configurator and added to 
   the application by editing the configuration file.

scada_motor_info.g 
   Sample graphical page to be displayed in a popup dialog. The popup
   dialog is displayed when the user clicks on one of the motor
   symbols in the Water Treatment page (scada_aeration.g) and shows
   detailed information about a motor. The drawing is shared by all
   motor symbols, and configured dynamically to display status
   information for the selected motor. The drawing also includes a
   spinner for changing the motor speed, which has a Write command
   that writes a value entered by the user to the back-end system.
   The popup dialog can display different drawings depending on the
   type of the selected object.

popup_menu2.g, popup_menu3.g
   Sample drawings for 2-state and 3-state popup menus used in the
   TestCommands page (scada_test_commands.g). A popup menu is
   activated when a user selects a motor symbol with the right mouse
   button or a valve with the left mouse button. The menu is
   configured and positioned dynamically based on the selected object.
   Clicking on a popup menu button executes a Write command to open
   or close a valve, or change the state of a motor.


