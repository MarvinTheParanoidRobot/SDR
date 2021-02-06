//////////////////////////////////////////////////////////////////////////////
// GLG SCADA Viewer Demo
//
// This demo shows an example of a SCADA application that demonstrates
// features used by typical SCADA, HMI, and process control and monitoring
// applications, including page navigation, handling user interaction,
// popup dialogs and alarms, executing user-defined commands and other
// related features.
//
// The demo is written in pure HTML5 and JavaScript. The source code of the
// demo uses the GLG Toolkit JavaScript Library supplied by the included
// Glg*.js and GlgToolkit*.js files.
//
// The library loads GLG drawings and renders them on a web page, providing
// an API to animate the drawing with real-time data and handle user
// interaction with graphical objects in the drawing.
//
// The drawings are created using the GLG Graphics Builder, an interactive
// editor that allows to create grahical objects and define their dynamic
// behavior without any programming.
//
// Except for the changes to comply with the JavaScript syntax, this source
// is identical to the source code of the corresponding C/C++, Java and C#
// versions of the demo.
//
// The demo uses simulated data for animation. An application will use
// live data obtained from the server via asynchronous HTTP requests.
// An example of using asynchronous HTTP requests to query live data
// from a server in a JSON format may be found in the GlgViewer example in
// the examples_html5 directory of the GLG installation.
//////////////////////////////////////////////////////////////////////////////

// Get a handle to the GLG Toolkit library.
var GLG = new GlgToolkit();

// Debugging aid: uncomment the next line to throw an exception on a GLG error.
//GLG.ThrowExceptionOnError( true, true, true );

// Filename of the menu configuration file.
var ConfigFile = "scada_config_menu.txt";
var ConfigFileData = null;    /* String : content of the configuration file */

// Set initial size of the drawing.
SetDrawingSize( false );

// Increase canvas resoultion for mobile devices.
var CoordScale = SetCanvasResolution();

// Add event listener to detect ESC key.
document.addEventListener( "keydown", HandleKeyEvent );

/* Loads misc. assets used by the program and invokes the LoadMainDrawing
   function when done.
*/
LoadAssets( LoadMainDrawing );

//////////////////////////////////////////////////////////////////////////////
function LoadMainDrawing()
{
    /* Load a drawing from the scada_main.g file. 
       The LoadCB callback will be invoked when the drawing has been loaded.
    */
   GLG.LoadWidgetFromURL( "scada_main.g", null, LoadCB, null );
}

//////////////////////////////////////////////////////////////////////////////
function LoadCB( drawing, data, path )
{
   if( drawing == null )
   {
      alert( "Can't load drawing, check console message for details." );
      return;
   }
   
   // Define the element in the HTML page where to display the drawing.
   drawing.SetParentElement( "glg_area" );
   
   // Disable viewport border to use the border of the glg_area.
   drawing.SetDResource( "LineWidth", 0 );

   StartSCADADemo( drawing );
}

//////////////////////////////////////////////////////////////////////////////
// Control variables and constants
//////////////////////////////////////////////////////////////////////////////

/* Set to true to demo with simulated data or to test without live data.
   Set to false or use -live-data command-line argument to use custom 
   live data feed.
*/
const RANDOM_DATA = true;   /* boolean */

/* Will be set to true if random data are used for the current page. */
var RandomData;             /* boolean */

// Page type constants.
const
   UNDEFINED_PAGE_TYPE = -1,
   // Is used in the absence of the PageType property in the loaded drawing.
   DEFAULT_PAGE_TYPE = 0,
   PROCESS_PAGE = 1,
   AERATION_PAGE = 2,
   CIRCUIT_PAGE = 3,
   RT_CHART_PAGE =4,
   TEST_COMMANDS_PAGE = 5;

/* Page type table, is used to determine the type of a loaded page based
   on the PageType property of the drawing. May be extended by an 
   application to add custom pages.
*/
var PageTypeTable =
  [ new TypeRecord( "Default",       DEFAULT_PAGE_TYPE ),
    new TypeRecord( "Process",       PROCESS_PAGE ),
    new TypeRecord( "Aeration",      AERATION_PAGE ),
    new TypeRecord( "Circuit",       CIRCUIT_PAGE ),
    new TypeRecord( "RealTimeChart", RT_CHART_PAGE ),
    new TypeRecord( "TestCommands",  TEST_COMMANDS_PAGE )
  ];

const NO_SCREEN = -1;    // Invalid menu screen index constant.

// CommandType contants.
const
   UNDEFINED_COMMAND_TYPE = -1,
   SHOW_ALARMS = 0,
   GOTO = 1,
   POPUP_DIALOG = 2,
   POPUP_MENU = 3,
   CLOSE_POPUP_DIALOG = 4,
   CLOSE_POPUP_MENU = 5,
   WRITE_VALUE = 6,
   WRITE_VALUE_FROM_WIDGET = 7,
   QUIT = 8;

// DialogType contants.
const
   UNDEFINED_DIALOG_TYPE = -1,
   GLOBAL_POPUP_DIALOG = 0,
   CUSTOM_DIALOG = 1,
   MAX_DIALOG_TYPE = 2;

const CLOSE_ALL = UNDEFINED_DIALOG_TYPE;

// PopupMenuType contants.
const
   UNDEFINED_POPUP_MENU_TYPE = -1,
   GLOBAL_POPUP_MENU = 0,
   CUSTOM_POPUP_MENU = 1;

// Predefined tables, can be extended by the application as needed.

var DialogTypeTable =
  [ new TypeRecord( "Popup", GLOBAL_POPUP_DIALOG ),
    new TypeRecord( "CustomDialog", CUSTOM_DIALOG )
  ];
   
var PopupMenuTypeTable =
  [ new TypeRecord( "PopupMenu", GLOBAL_POPUP_MENU ),
    new TypeRecord( "CustomMenu", CUSTOM_POPUP_MENU )
  ];
   
var CommandTypeTable =
  [ new TypeRecord( "ShowAlarms", SHOW_ALARMS ),
    new TypeRecord( "GoTo", GOTO ),
    new TypeRecord( "PopupDialog", POPUP_DIALOG ),
    new TypeRecord( "PopupMenu", POPUP_MENU ),
    new TypeRecord( "ClosePopupDialog", CLOSE_POPUP_DIALOG ),
    new TypeRecord( "ClosePopupMenu", CLOSE_POPUP_MENU ),
    new TypeRecord( "WriteValue", WRITE_VALUE ),
    new TypeRecord( "WriteValueFromWidget", WRITE_VALUE_FROM_WIDGET ),
    new TypeRecord( "Quit", QUIT )
  ];

// WidgetType constants.
const 
   UNDEFINED_WIDGET_TYPE = -1,
   DEFAULT_WIDGET_TYPE = 0,
   RTCHART_WIDGET = 1;

/* Widget type table, is used to determine the type of a popup widget
   based on the "WidgetType" resource, if any.
*/
var WidgetTypeTable =
  [ new TypeRecord( "RTChart", RTCHART_WIDGET ) ];

// The type of the current page.
var PageType = UNDEFINED_PAGE_TYPE;   /* int */

/* The object that implements functionality of an HMI page, it is a subclass
   of the HMIPageBase base object. A default HMIPageBase object is provided;
   custom HMI page subclasses may be used to define custom page logic, 
   as shown in the RealTimeChart and Process pages.
*/
var HMIPage;          /* HMIPageBase */

/* The object that is used for supplying data for animation, either random data 
   for a demo, or live data in a real application.
*/
var DataFeed;         /* DemoDataFeedObj or LiveDataFeedObj */

// Datafeed instances.
var
   DemoDataFeed,      /* DemoDataFeedObj */
   LiveDataFeed;      /* LiveDataFeedObj */

// Dynamically created array of tag records.
var TagRecordArray = [];  /* GlgTagRecord[] */

// Number of tags records in the TagRecordArray.
var NumTagRecords = 0;    /* int */

var MainViewport;  /* GlgObject : Viewport of the top level drawing */
var DrawingArea;   /* GlgObject : Subwindow object in top level drawing */
var DrawingAreaVP; /* GlgObject : Viewport loaded into the Subwindow */
var Menu;          /* GlgObject : Navigation menu viewport */
var AlarmDialog;   /* GlgObject : AlarmDialog viewport  */
var AlarmListVP;   /* GlgObject : Viewport containing the alarm list */
var NumAlarmRows;  /* int : Number of visible alarms on one alarm page. */
var AlarmRows;     /* GlgObject[] : Keeps object ID's of alarm rows for faster 
                      access. */
var EmptyDrawing;  /* GlgObject */

var AlarmStartRow = 0;  /* int: Scrolled state of the alarm list. */
var AlarmList;     /* AlarmRecord[] : List of alarms. */

var AlarmDialogVisible;      /* boolean */
var FirstAlarmDialog = true; /* boolean : Is used to show a help message when the
                                alarm dialog is shown the first time. */
var MessageDialog;           /* GlgObject : Popup dialog used for messages. */
var StartDragging = false;   /* boolean */

// Array of active dialogs. Is used to open/close active dialogs.
var ActiveDialogs;     /* GlgActiveDialogRecord[] */
   
// Active popup menu. It is assumed that only one popup menu is active at a time.
var ActivePopupMenu;   /* GlgActivePopupMenuRecord */
   
var UpdateInterval = 50;     /* int : Update rate in msec for drawing animation,
                                will be adjusted on per-drawing basis. */
var AlarmUpdateInterval = 1000; /* int : Alarm update interval in msec. */

/* Predefined menu table, is used if the configuration file is not supplied, 
   or if the config file cannot be successfully parsed. 
*/
var MenuTable = 
  [  /* label, drawing name, tooltip, title */
     new GlgMenuRecord( "Solvent\nRecovery", "process.g", 
                        "process.g", "Solvent Recovery System" ),

     new GlgMenuRecord( "Water\nTreatment", "scada_aeration.g", 
                        "scada_aeration.g", "Aeration Monitoring" ),
     
     new GlgMenuRecord( "Eectrical\nCircuit", "scada_electric.g", 
                        "scada_electric.g", "Electrical Circuit Monitoring" ),
     
     new GlgMenuRecord( "Real-Time\nStrip-Chart", "scada_chart.g", 
                        "scada_chart.g", "Real-Time Strip-Chart Sample" ),
     
     new GlgMenuRecord( "Test Object\nCommands", "scada_test_commands.g", 
                        "scada_test_commands.g", "Test Object Commands" )
  ];

/* MenuArray is built dynamically by reading a configuration file.
   If a configuration file is not supplied or cannot be successfully 
   parsed, a predefined MenuTable is used to populate MenuArray.
*/
var MenuArray;                /* GlgMenuRecord[] */
var NumMenuItems = 0;         /* int */

/* These objects are used to pass data to the load callback. They hava
   an "enabled" field that is used to cancel the current load request if 
   loading of another page (or another popup) was requested while the 
   current request has not finished. This may happen if the user clicks 
   on one page load button and then clicks on another page load button 
   before the first page finishes loading.
*/
var PageLoadRequest = null;
var PopupLoadRequest = null;

/* Flag indicating how to supply a time stamp for a RealTimeChart embedded 
   into an HMI page: if set to 1, the application will supply a time stamp   
   explicitly. Otherwise, a time stamp will be supplied automatically by
   chart using current time. 
*/
var SUPPLY_PLOT_TIME_STAMP = false;    /* boolean */

var TouchDevice = -1;

// Is used by DataFeed to return data values.
var data_point = new DataPoint();      /* DataPoint */

//////////////////////////////////////////////////////////////////////////////
function StartSCADADemo( drawing )
{
   MainViewport = drawing;

   InitBeforeSetup();

   MainViewport.SetupHierarchy();

   InitAfterSetup();
   
   MainViewport.Update();     /* Draw */

   // Update drawing with data and start periodic data updates.
   UpdateDrawing();

   // Process alarms and start periodic alarm updates.
   ProcessAlarms( true );
}

//////////////////////////////////////////////////////////////////////////////
function InitBeforeSetup()
{
   // Initialize MenuArray from the configuration file.
   FillMenuArray( ConfigFileData );

   /* Initialize ActiveDialogs array and ActivePopupMenu. */
   InitActivePopups();
      
   /* Store an object ID of the viewport named Menu. It contains navigation
      buttons for switching between drawings.
   */
   Menu = MainViewport.GetResourceObject( "Menu" );
      
   // Store an object ID of the Subwindow object named DrawingArea.
   DrawingArea = MainViewport.GetResourceObject( "DrawingArea" );
   if( DrawingArea == null ) // no DrawingArea found
   {
      alert( "Can't find DrawingArea Subwindow object." );
      return;
   }

   // Store an object ID of the AlarmDialog viewport.
   AlarmDialog = MainViewport.GetResourceObject( "AlarmDialog" );
   AlarmListVP = AlarmDialog.GetResourceObject( "Table" );

   /* Make AlarmDialog a free floating dialog. ShellType property
      can be also set at design time.
   */
   AlarmDialog.SetDResource( "ShellType", GLG.GlgShellType.DIALOG_SHELL );
   
   // Initialize alarm list (set initial values in the template).
   AlarmListVP.SetSResource( "Row/ID", "" );
   AlarmListVP.SetSResource( "Row/Description", "" );
   AlarmListVP.SetDResource( "Row/UseStringValue", 0.0 );
   AlarmListVP.SetDResource( "Row/DoubleValue", 0.0 );
   AlarmListVP.SetSResource( "Row/StringValue", "" );
   AlarmListVP.SetDResource( "Row/AlarmStatus", 0.0 );
   AlarmListVP.SetDResource( "Row/RowVisibility", 0.0 );
   
   // Make AlarmDialog initially invisible.
   AlarmDialog.SetDResource( "Visibility", 0.0 );
   AlarmDialogVisible = false;
   
   // Set title for the AlarmDialog.
   AlarmDialog.SetSResource( "ScreenName", "Alarm List" );

   // Set initial state of the alarm button.
   MainViewport.SetDResource( "AlarmButton/Blinking", 0.0 );

   /* Make Global PopupDialog a floating DialogShell type. */
   MainViewport.SetDResource( "PopupDialog/ShellType",
                              GLG.GlgShellType.DIALOG_SHELL );
   // Make Global PopupDialog initially invisible.
   MainViewport.SetDResource( "PopupDialog/Visibility", 0.0 );
   
   // Make Global PopupMenu initially invisible.
   MainViewport.SetDResource( "PopupMenu/Visibility", 0.0 );
   
   // Make message dialog invisible on startup.      
   MessageDialog = MainViewport.GetResourceObject( "MessageDialog" );
   MessageDialog.SetDResource( "Visibility", 0.0 );
   
   /* Make MessageDialog a free floating dialog. ShellType property
      can be also set at design time.
   */
   MessageDialog.SetDResource( "ShellType", GLG.GlgShellType.DIALOG_SHELL );

   // Delete the QuitButton used in a desktop version to exit the demo.
   var quit_button = MainViewport.GetResourceObject( "QuitButton" );
   var group = MainViewport.GetResourceObject( "ControlsGroup" );
   group.DeleteThisObject( quit_button );

   // Set the number of menu items. It must be done BEFORE hierarchy setup.
   Menu.SetDResource( "NumRows", NumMenuItems );

   AdjustForMobileDevices();
   
   NumAlarmRows = Math.trunc( AlarmListVP.GetDResource( "NumRows" ) );

   /* Add Hierarchy Listener to handle events when a new drawing is loaded
      into a Subwindow object.
   */
   MainViewport.AddListener( GLG.GlgCallbackType.HIERARCHY_CB,
                             HierarchyCallback );

   /* Add Input callback which is invoked when the user interacts with objects
      in a GLG drawing. It is used to handle events occurred in input objects,
      such as a menu, as well as Commands or Custom Mouse Events attached 
      to objects at design time.
   */
   MainViewport.AddListener( GLG.GlgCallbackType.INPUT_CB, InputCallback );

   /* Add Trace Listener used to handle low level events, such as obtaining
      coordinates of the mouse click, or keyboard events.
   */
   MainViewport.AddListener( GLG.GlgCallbackType.TRACE_CB, TraceCallback );

   HMIPage = new EmptyHMIPage();    // Initialize to empty page.

   CreateDataFeed();
}

//////////////////////////////////////////////////////////////////////////////
function AdjustForMobileDevices()
{
   if( CoordScale == 1.0 )
     return;  // Desktop
   
   // Increases size of the menu area for mobile devices.
   MainViewport.SetDResource( "CoordScale", 1.4 );

   // Increase the size of the alarm dialog.
   AlarmDialog.SetDResource( "Height", 700 );
   AlarmDialog.SetDResource( "Width", 800 );
   AlarmDialog.SetGResource( "Point1", -750, 400, 0 );

   /* Increase height of alarm rows to make it easier to acknowledge an alarm
      bu touching the alarm row on mobile devices.
   */
   AlarmListVP.SetDResource( "NumRows", 7 );
   AlarmListVP.SetDResource( "NumVisibleRows", 7 );
   
   // Increase the size of the scroll controls in the alarm dialog.
   ScaleParameter( AlarmDialog, "ScrollWidth", 2.0 );
   ScaleParameter( AlarmDialog, "TopScrollHeight", 3.0 );
   ScaleParameter( AlarmDialog, "BottomScrollHeight", 3.0 );
   ScaleParameter( AlarmDialog, "HeaderHeight", 1.4 );
   
   // Adjust initial position of the popup dialog.
   MainViewport.SetGResource( "PopupDialog/Point1", -400, 500, 0 );

   // Increase the size of the message dialog.
   MessageDialog.SetDResource( "CoordScale", 1.5 );
}

//////////////////////////////////////////////////////////////////////////////
function InitAfterSetup()
{
   // Initialize the navigation menu.
   InitMenu();
   
   // Store object ID's of alarm rows for faster access.
   AlarmRows = new Array( NumAlarmRows );
   for( var i=0; i<NumAlarmRows; ++i )
     AlarmRows[i] = AlarmListVP.GetResourceObject( "Row" + i );
   
   // Load drawing corresponding to the first menu item.
   LoadDrawingFromMenu( 0 );
}

//////////////////////////////////////////////////////////////////////////////
// Data feeds are reused for different pages - create just once.
//////////////////////////////////////////////////////////////////////////////
function CreateDataFeed()
{      
   DemoDataFeed = new DemoDataFeedObj();
   
   LiveDataFeed = new LiveDataFeedObj();
}

//////////////////////////////////////////////////////////////////////////////
// Read MenuArray from a configuration file. If configuration file 
// is not supplied, use predefined MenuTable array.
//////////////////////////////////////////////////////////////////////////////
function FillMenuArray( /* String */ config_data )
{
   MenuArray = ReadMenuConfig( config_data );
   if( MenuArray == null )
   {
      console.log( "Can't read config file: using predefined Menu Table." );
         
      // Fill MenuArry from a predefined array MenuTable.
      var num_items = MenuTable.length;
      MenuArray = new Array( num_items );
      for( var i=0; i < num_items; ++i )
        MenuArray.push( MenuTable[ i ] );
   }
      
   NumMenuItems = MenuArray.length;
}
   
//////////////////////////////////////////////////////////////////////////////
// Initialize the navigation menu, a viewport named "Menu", based on
// the menu records from the supplied configuration file.
//////////////////////////////////////////////////////////////////////////////
function InitMenu()
{
   var button;       /* GlgObject */
   var menu_record;  /* GlgMenuRecord */

   // Populate menu buttons based on the MenuArray.
   for( var i=0; i<NumMenuItems; ++i )
   {
      button = Menu.GetResourceObject( "Button" + i );
      
      menu_record = MenuArray[ i ];
      button.SetSResource( "LabelString", menu_record.label_string );
      button.SetSResource( "TooltipString", menu_record.tooltip_string );
   }
   
   SelectMainMenuItem( NO_SCREEN, true );
}
   
//////////////////////////////////////////////////////////////////////////////
function UpdateDrawing()
{
   UpdateData();

   // Restart update timer.
   timer = setTimeout( UpdateDrawing, UpdateInterval );
}

//////////////////////////////////////////////////////////////////////////////
// Traverses an array of tag records, gets new data for each tag and 
// updates the drawing with new values.
//////////////////////////////////////////////////////////////////////////////
function UpdateData()
{
   /* Invoke a page's custom update method, if implemented.
      Don't update any tags if the custom method returns true.
   */         
   if( !HMIPage.UpdateData() && DataFeed != null )
   {
      /* Always update all tags defined in the current page, as well as 
         any additional tags defined in popup dialogs in the main drawing, 
         outside of the current page.
      */
      var num_tag_records = TagRecordArray.length;   /* int */
      for( var i=0; i<num_tag_records; ++i )
      {
         var tag_record = TagRecordArray[i];  /* GlgTagRecord */
         
         switch( tag_record.data_type )
         {
          case GLG.GlgDataType.D:
            // Obtain a new numerical data value for a given tag. 
            if( DataFeed.ReadDTag( tag_record, data_point ) )
            {
               /* Push a new data value into a given tag. If the last argument 
                  (if_changed flag) is true, the value is pushed into graphics
                  only if it changed. Otherwise, a new value is always 
                  pushed into graphics. The if_changed flag is ignored for tags
                  attached to the plots in a real time chart, and the new value
                  is always pushed to the chart even if it is the same.
               */
               MainViewport.SetDTag( tag_record.tag_source,
                                     data_point.d_value, true );
               
               /* Push a time stamp to the TimeEntryPoint of a plot in 
                  a real-time chart, if found.
               */ 
               if( tag_record.plot_time_ep != null )
                 tag_record.plot_time_ep.SetDResource( null,
                                                       data_point.time_stamp );
            }
            break;
            
          case GLG.GlgDataType.S:    
            // Obtain a new string data value for a given tag. 
            if( DataFeed.ReadSTag( tag_record, s_data_point ) )            
            {
               // Push new data.
               MainViewport.SetSTag( tag_record.tag_source,
                                     s_data_point.s_value, true );
            }
            break;
         }
      }
      
      // Update the drawing with new data.
      MainViewport.Update();
   }
}   

//////////////////////////////////////////////////////////////////////////////
// Fills AlarmList dialog with received alarm data.
//////////////////////////////////////////////////////////////////////////////
function ProcessAlarms( /* boolean */ query_new_list )
{
   if( query_new_list && DataFeed != null )
     // Get a new alarm list.
     AlarmList = DataFeed.GetAlarms();
   
   var num_alarms;   /* int */
   if( AlarmList == null )
     num_alarms = 0;
   else
     num_alarms = AlarmList.length;
   
   // Activate Alarms button's blinking if there are unacknowledged alarms.
   var has_active_alarms = false;   /* boolean  */
   for( var i=0; i<num_alarms; ++i )
   {
      var alarm = AlarmList[i];   /* AlarmRecord */
      if( !alarm.ack )
      {
         has_active_alarms = true;
         break;
      }
   }

   MainViewport.SetDResourceIf( "AlarmButton/Blinking",
                                has_active_alarms ? 1.0 : 0.0, true );

   if( AlarmDialogVisible )
   {
      // Fill alarm rows starting with the AlarmStartRow that controls scrolling.
      var num_visible = num_alarms - AlarmStartRow;   /* int */
      if( num_visible < 0 )
        num_visible = 0;
      else if( num_visible > NumAlarmRows )
        num_visible = NumAlarmRows;
      
      // Fill alarm rows.
      var alarm_row;   /* GlgObject */
      for( var i=0; i<num_visible; ++i )
      {         
         var alarm = AlarmList[i];   /* AlarmRecord */
         alarm_row = AlarmRows[i];
         
         alarm_row.SetDResourceIf( "AlarmIndex", AlarmStartRow + i + 1, true  );
         alarm_row.SetDResourceIf( "TimeInput", alarm.time, true  );
         alarm_row.SetSResourceIf( "ID", alarm.tag_source, true );
         alarm_row.SetSResourceIf( "Description", alarm.description, true );
         
         // Set to 1 to supply string value via the StringValue resource.
         // Set to 0 to supply double value via the DoubleValue resource.
         alarm_row.SetDResourceIf( "UseStringValue", 
                                   alarm.string_value == null ? 0.0 : 1.0, true );
         if( alarm.string_value == null )
           alarm_row.SetDResourceIf( "DoubleValue", alarm.double_value, true );
         else
           alarm_row.SetSResourceIf( "StringValue", alarm.string_value, true );
         
         alarm_row.SetDResourceIf( "RowVisibility", 1.0, true  );
         alarm_row.SetDResourceIf( "AlarmStatus", alarm.status, true );
         
         /* Enable blinking: will be disabled when alarm is ACK'ed. */
         alarm_row.SetDResourceIf( "BlinkingEnabled", alarm.ack ? 0.0 : 1.0, 
                                   true );
      }
      
      /* Empty the rest of the rows. Use true as the last parameter to update
         only if the value changes.
      */
      for( var i=num_visible; i<NumAlarmRows; ++i )
      {
         alarm_row = AlarmRows[i];
         
         alarm_row.SetDResourceIf( "AlarmIndex", AlarmStartRow + i + 1, true );
         alarm_row.SetSResourceIf( "ID", "", true );
         
         // Set status to normal to unhighlight the rightmost alarm field.
         alarm_row.SetDResourceIf( "AlarmStatus", 0.0, true );
         
         // Make all text labels invisible.
         alarm_row.SetDResourceIf( "RowVisibility", 0.0, true );
         
         alarm_row.SetDResourceIf( "BlinkingEnabled", 0.0, true );
      }
      
      AlarmDialog.Update();
   }  

   // Restart alarm update timer.
   if( query_new_list )
     alarm_timer =
       setTimeout( () => ProcessAlarms( true ), AlarmUpdateInterval );
}

//////////////////////////////////////////////////////////////////////////////
// Input callback is invoked when the user interacts with objects in a
// GLG drawing. It is used to handle events occurred in input objects,
// such as a menu, as well as Commands or Custom Mouse Events attached 
// to objects at design time.
//////////////////////////////////////////////////////////////////////////////
function InputCallback( /* GlgObject */ viewport, /* GlgObject */ message_obj )
{
   var
     origin,   /* String */
     format,   /* String */
     action;   /* String */

   var menu_index;     /* int */
   var selected_obj;   /* GlgObject */
   var action_obj;     /* GlgObject */

   /* Return if the page's custom input handler processed the input.
      Otherwise, continue to process common events and commands.
   */
   if( HMIPage.InputCallback( viewport, message_obj ) )
     return;

   origin = message_obj.GetSResource( "Origin" );
   format = message_obj.GetSResource( "Format" );
   action = message_obj.GetSResource( "Action" );
   
   // Retrieve selected object.
   selected_obj = message_obj.GetResourceObject( "Object" );
      
   // Handle events from the screen navigation menu, named "Menu".
   if( format == "Menu" )
   {
      if( action != "Activate" || origin != "Menu" )
        return;
      
      /* User selected a button from the menu object named Menu. 
         Load a new drawing associated with the selected button.
      */
      menu_index = Math.trunc( message_obj.GetDResource( "SelectedIndex" ) );
      
      /* Load the drawing associated with the selected menu button and
         display it in the DrawingArea.
      */
      LoadDrawingFromMenu( menu_index );

      viewport.Update();
   }
      
   /* Handle custom commands attached to objects in the drawing at 
      design time.
   */
   else if( format == "Command" )
   {
      action_obj = message_obj.GetResourceObject( "ActionObject" ); 
      ProcessObjectCommand( viewport, selected_obj, action_obj );
      viewport.Update();
   }

   // Handle zoom controls on the left.
   else if( format == "Button" )
   {
      if( origin == null || 
          action != "Activate" &&      /* Not a push button */
          action != "ValueChanged" )   /* Not a toggle button */
        return;
      
      if( origin == "MessageDialogOK" )
      {
         // Close message dialog.
         MessageDialog.SetDResource( "Visibility", 0.0 );
         MessageDialog.Update();
      }
     
      /* Zoom and pan buttons. */
      else if( origin == "Left" )
        Zoom( DrawingAreaVP, 'l', 0.1 );
      else if( origin == "Right" )
        Zoom( DrawingAreaVP, 'r', 0.1 );
      else if( origin == "Up" )
        Zoom( DrawingAreaVP, 'u', 0.1 );
      else if( origin == "Down" )
        Zoom( DrawingAreaVP, 'd', 0.1 );
      else if( origin == "ZoomIn" )
        Zoom( DrawingAreaVP, 'i', 1.5 );
      else if( origin == "ZoomOut" )
        Zoom( DrawingAreaVP, 'o', 1.5 );
      else if( origin == "ZoomTo" )
      {
         StartDragging = true;
         Zoom( DrawingAreaVP, 't', 0.0 );
      }
      else if( origin == "ZoomReset" )
        Zoom( DrawingAreaVP, 'n', 0.0 );
      
      /* Alarm scrolling buttons. */
      else if( origin == "ScrollToTop" )
      {
         AlarmStartRow = 0;
         ProcessAlarms( false );
      }
      else if( origin == "ScrollUp" )
      {
         --AlarmStartRow;
         if( AlarmStartRow < 0 )
           AlarmStartRow = 0;
         ProcessAlarms( false );
      }
      else if( origin == "ScrollUp2" )
      {
         AlarmStartRow -= NumAlarmRows;
         if( AlarmStartRow < 0 )
           AlarmStartRow = 0;
         ProcessAlarms( false );
      }
      else if( origin == "ScrollDown" )
      {
         ++AlarmStartRow;
         ProcessAlarms( false );
      }
      else if( origin == "ScrollDown2" )
      {
         AlarmStartRow += NumAlarmRows;
         ProcessAlarms( false );
      }
      else
        return;
      
      viewport.Update();
   }
   
   // Handle custom events.
   else if( format == "CustomEvent" )
   {
      var event_label = message_obj.GetSResource( "EventLabel" );   /* String */
      var action_data = null;   /* GlgObject */
      
      if( event_label == null || event_label.length == 0 )
        return;    // don't process events with empty EventLabel.

      /* Retrieve action object. It will be null for custom events 
         added prior to GLG v.3.5. 
         If action object is present, retrieve its ActionData object.
         If Action Data object present, use its properties for custom event 
         processing as needed.
      */
      action_obj = message_obj.GetResourceObject( "ActionObject" ); 
      if( action_obj != null )
        action_data = action_obj.GetResourceObject( "ActionData" );

      if( event_label == "AlarmRowACK" )
      {
         // The object ID of the alarm row selected by Ctrl-click.
         var alarm_row = selected_obj;   /* GlgObject */

         // Retrieve the tag source.
         var tag_source = alarm_row.GetSResource( "ID" );   /* String */
            
         if( DataFeed != null )
           DataFeed.ACKAlarm( tag_source );
      }
      else
      {
         /* Place custom code here to handle custom events as needed. */
      }
      
      viewport.Update();
   }
            
   /* Handle events from a Real-Time Chart. */
   else if( format == "Chart" && action == "CrossHairUpdate" )
   {
      /* To avoid slowing down real-time chart updates, invoke Update() 
         to redraw cross-hair only if the chart is not updated fast 
         enough by the timer.
      */
      if( UpdateInterval > 100 )
        viewport.Update();
   }
      
   // Handle Timer events, generated by objects with blinking dynamics.
   else if( format == "Timer" )  
     viewport.Update();
   
   // Handle window closing events.
   else if( format == "Window" && action == "DeleteWindow" )
   {
      if( selected_obj == null )
        return;

      if( selected_obj.Equals( MessageDialog ) )
      {
         MessageDialog.SetDResource( "Visibility", 0.0 );
         MessageDialog.Update();
      }
      else if( selected_obj.Equals( AlarmDialog ) )
      {
         CloseAlarmDialog();
      }
      else
      {
         /* If the closing window is found in the ActiveDialogs array, 
            close the active dialog. 
         */
         for( var i=0; i<MAX_DIALOG_TYPE; ++i )
           if( selected_obj.Equals( ActiveDialogs[ i ].dialog ) )
           {
              ClosePopupDialog( i );
              break;
           }
      }

      viewport.Update();
   }
}

//////////////////////////////////////////////////////////////////////////////
// Is used to handle low level events, such as obtaining coordinates of 
// the mouse click, or keyboard events. 
//////////////////////////////////////////////////////////////////////////////
function TraceCallback( /* GlgObject */ viewport, /* GlgTraceData */ trace_info )
{      
   // Detect touch device if it hasn't been detected yet.
   if( TouchDevice == -1 )
     if( trace_info.event_type == GLG.GlgEventType.TOUCH_START )
     {
        TouchDevice = 1;

        /* Allow touch actions regardless of the mouse button and the control key
           state, which are not present on mobile devices.
        */
        viewport.SetDResource( "$config/GlgDisableMouseButtonCheck", 1 );
        viewport.SetDResource( "$config/GlgDisableControlKeyCheck", 1 );
     }
     else if( trace_info.event_type == GLG.GlgEventType.MOUSE_PRESSED )
       TouchDevice = 0;
   
   /* Return if the page's custom trace callback processed the event.
      Otherwise, continue to process common events.
   */
   if( HMIPage.TraceCallback( viewport, trace_info ) )
     return;

   var x, y;   /* double */
   var event_vp = trace_info.viewport;   /* GlgObject */
   
   var event_type = trace_info.event_type;
   switch( event_type )
   {
    case GLG.GlgEventType.TOUCH_START:
      // On mobile devices, enable touch dragging for defining ZoomTo region.
      if( !StartDragging )
        return;
      
      GLG.SetTouchMode();        /* Start dragging via touch events. */
      StartDragging = false;     /* Reset for the next time. */
      /* Fall through */

    case GLG.GlgEventType.TOUCH_MOVED:
      if( !GLG.GetTouchMode() )
        return;
    case GLG.GlgEventType.MOUSE_PRESSED:
    case GLG.GlgEventType.MOUSE_MOVED:
      // Obtain mouse coordinates. 
      x = trace_info.mouse_x * CoordScale;
      y = trace_info.mouse_y * CoordScale;
      
      /* COORD_MAPPING_ADJ is added to the cursor coordinates for precise
         pixel mapping.
      */
      x += GLG.COORD_MAPPING_ADJ;
      y += GLG.COORD_MAPPING_ADJ;
      break;
         
    case GLG.GlgEventType.KEY_DOWN:
      // Handle key presses when input focus is in the drawing.
      switch( trace_info.event.keyCode )
      {
       case 27: // ESC key
         break;
       default: break;
      }
      break;
   }
}

//////////////////////////////////////////////////////////////////////////////
// Hierarchy callback, added to the top level viewport and invoked when 
// a new drawing is loaded into any subwindow or subdrawing object. 
// In this demo, this callback processes events only from the reference
// of type "Subwindow" (such as DrawingArea).
//////////////////////////////////////////////////////////////////////////////
function HierarchyCallback( /* GlgObject */ viewport, 
                            /* GlgHierarchyData */ info_data )
{
   // Handle events only from Subwindow-type reference objects.
   var obj_type = /* int */
     Math.trunc( info_data.object.GetDResource( "ReferenceType" ) );

   if( obj_type != GLG.GlgReferenceType.SUBWINDOW_REF )
     return;
      
   /* This callback is invoked twice: one time before hierarchy setup
      for the new drawing, and second time after hierarchy setup.
      Drawing initialization can be done here if needed.
   */
   switch( info_data.condition )
   {
    case GLG.GlgHierarchyCallbackType.BEFORE_SETUP_CB:
      var drawing_vp = info_data.subobject;   /* GlgObject */
      if( drawing_vp == null )
      {
         alert( "Drawing loading failed" ); 
         if( info_data.object.Equals( DrawingArea ) )
           DrawingAreaVP = null;
         return;
      }

      /* Set "ProcessMouse" attribute for the loaded viewport, to process
         custom events and tooltips.
      */
      drawing_vp.SetDResource( "ProcessMouse",
                               ( GLG.GlgProcessMouseMask.MOUSE_CLICK | 
                                 GLG.GlgProcessMouseMask.MOUSE_OVER_TOOLTIP ) );
         
      /* Set "OwnsInputCB" attribute for the loaded viewport,
         so that Input callback is invoked with this viewport ID.
      */
      drawing_vp.SetDResource( "OwnsInputCB", 1.0 ); 

      if( info_data.object.Equals( DrawingArea ) )
      {
         DrawingAreaVP = GLG.GetReference( drawing_vp );
         SetupHMIPage();

         /* Initialize loaded drawing before setup. */
         HMIPage.InitBeforeSetup();

         HMIPage.AdjustForMobileDevices();
      }
      break;
         
    case GLG.GlgHierarchyCallbackType.AFTER_SETUP_CB:
       /* Initialize loaded drawing after setup. */
       if( info_data.object.Equals( DrawingArea ) )
           HMIPage.InitAfterSetup();
       break;
       
       default: break;
   }
}

//////////////////////////////////////////////////////////////////////////////
// Assigns a new page to HMIPage variable, either as a default or 
// custom HMI page, based on the PageType property of the loaded 
// drawing. The HMI page class handles the page logic and user 
// interaction.
// 
// If the PageType property doesn't exist in the drawing or is set to 
// "Default", DefaultHMIPage class is used for the new page.
// Otherwise, a page class type corresponding to the PageType property 
// is assigned to HMIPage variable. For example, RTChartPage class is 
// used if PageType property of the loaded drawing is set to 
// "RealTimeChart".
// 
// The method also assigns DataFeed for the new page.
//////////////////////////////////////////////////////////////////////////////
function SetupHMIPage()
{ 
   PageType = GetPageType( DrawingAreaVP );     // Get page type.

   /* Use live data if requested with the -live-data command-line option, 
      otherwise use simulated random data for testing. 
      RANDOM_DATA may be set to false to use live data by default.
   */
   if( RANDOM_DATA )
     DataFeed = DemoDataFeed;
   else
     DataFeed = LiveDataFeed;
    
   switch( PageType )
   {
    case UNDEFINED_PAGE_TYPE:
      HMIPage = new EmptyHMIPage();
      break;
        
    case DEFAULT_PAGE_TYPE:
    case AERATION_PAGE:
    case CIRCUIT_PAGE:
      HMIPage = new DefaultHMIPage( DrawingAreaVP );
      break;

    case PROCESS_PAGE:
      HMIPage = new ProcessPage( DrawingAreaVP );
      break;

    case RT_CHART_PAGE:
      HMIPage = new RTChartPage( DrawingAreaVP );
      break;

    case TEST_COMMANDS_PAGE:
      // Test page: always use demo data.
      DataFeed = DemoDataFeed;
        
      HMIPage = new DefaultHMIPage( DrawingAreaVP );
      break;

    default:
      /* New custom page: use live data if requested with the -live-data 
         command-line option, otherwise use simulated random data for 
         testing. RANDOM_DATA may be set to false to use live data by 
         default.
      */
      HMIPage = new DefaultHMIPage( DrawingAreaVP );
      break;
   }

   UpdateInterval = HMIPage.GetUpdateInterval();
    
   // True if DemoDataFeed is used for the current page.
   RandomData = ( DataFeed == DemoDataFeed );
}

//////////////////////////////////////////////////////////////////////////////
// Load a new drawing into the DrawingArea when the user selects an item
// from the navigation menu object (Menu object).
//////////////////////////////////////////////////////////////////////////////
function LoadDrawingFromMenu( /* int */ page_index )
{
   if( !PageIndexValid( page_index ) )
     return;   // Invalid page index - don't load.

   // Close active popup dialogs and popup menu, if any.
   CloseActivePopups( CLOSE_ALL );
    
   var menu_record = MenuArray[ page_index ];   /* GlgMenuRecord */

   if( menu_record.drawing_name == null || menu_record.drawing_name.length == 0 )
   {
      alert( "Can't load drawing - missing filename." );
      return;
   }
    
   AbortPendingPageLoads();    // Cancel any pending page load requests. 

   SetLoadingMessage( menu_record.drawing_title );

   /* Store a new load request in a global variable to be able to abort it
      if needed.
   */
   PageLoadRequest = { enabled : true, page_index : page_index };
    
   // Request to load the new drawing and invoke the callback when it's ready.
   GLG.LoadObjectFromURL( menu_record.drawing_name, null,
                          LoadDrawingFromMenuCB, PageLoadRequest, AbortLoad,
                          /* subwindow to handle bindings */ DrawingArea );
}

//////////////////////////////////////////////////////////////////////////////
// This function is invoked after the drawing's raw data has been downloaded
// and before loading the drawing from raw data. If the function returns true,
// loading the drawing from raw data is aborted.
//////////////////////////////////////////////////////////////////////////////
function AbortLoad( load_request )   /* boolean */
{
   // Return true to abort if the load request was cancelled.
   return !load_request.enabled;
}

////////////////////////////////////////////////////////////////////////////// 
// Cancels any pending page load requests.
////////////////////////////////////////////////////////////////////////////// 
function AbortPendingPageLoads()
{
   if( PageLoadRequest != null )
   {
      PageLoadRequest.enabled = false;
      PageLoadRequest = null;
   }
}

////////////////////////////////////////////////////////////////////////////// 
// Cancels any pending popup load requests.
////////////////////////////////////////////////////////////////////////////// 
function AbortPendingPopupLoads()
{
   if( PopupLoadRequest != null )
   {
      PopupLoadRequest.enabled = false;
      PopupLoadRequest = null;
   }
}

//////////////////////////////////////////////////////////////////////////////
function LoadDrawingFromMenuCB( /* GlgObject */ loaded_drawing,
                                /* Object */ user_data, /* String */ path )
{
   var load_request = user_data;

   if( !load_request.enabled )
     /* This load request was aborted by requesting to load another page before
        this load request has finished.
     */
     return;
    
   PageLoadRequest = null;   // Reset: we are done with this request.
    
   if( loaded_drawing == null )
   {
      alert( "Drawing loading failed" );
      return;
   }

   /* Close active popup dialogs and popup menu again: they might have been
      activated while waiting for the new page being loaded.
   */
   CloseActivePopups( CLOSE_ALL );
    
   AbortPendingPopupLoads();   // Cancel any pending popup load requests.

   /* Loads a new drawing into the DrawingArea subwindow object.
      DrawingAreaVP is assigned in the Hierarchy callback to the 
      ID of the $Widget viewport of the loaded drawing.
   */
   SetDrawing( DrawingArea, loaded_drawing );
   if( DrawingAreaVP == null )
   {
      DeleteTagRecords();
      PageType = UNDEFINED_PAGE_TYPE;
      HMIPage = new EmptyHMIPage();
      return;
   }

   /* Query a list of tags from the loaded drawing and build a new
      TagRecordArray.
   */
   QueryTags( DrawingAreaVP );
    
   var page_index = load_request.page_index;    /* int */
   SelectMainMenuItem( page_index );

   // Set title and background color.
   var title = MenuArray[ page_index ].drawing_title;   /* String */
   SetupLoadedPage( title );

   if( CoordScale != 1.0 )
     /* Increase pick resolution for the water treatment page to make it easier
        to select pump motors by touching on mobile devices.
     */
     MainViewport.SetDResource( "$config/GlgPickResolution",
                                PageType == AERATION_PAGE ? 30 : 5 );
    
   HMIPage.Ready();

   MainViewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
function SetLoadingMessage( /* String */ title )
{
   MainViewport.SetSResource( "Title", "Loading " + title + " page..." );
   MainViewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
function SetupLoadedPage( /* String */ title )
{
   // Set new title. 
   MainViewport.SetSResource( "Title", title );
    
   /* Set the color of the top level window and menus could
      to match the color of the loaded drawing. 
   */
   var color = DrawingAreaVP.GetGResource( "FillColor" );    /* GlgPoint */
   MainViewport.SetGResourceFromPoint( "FillColor", color );      
}

//////////////////////////////////////////////////////////////////////////////
// Sets a new drawing as a template of the specified subwindow object
// and returns a viewport displayed in the subwindow.
//////////////////////////////////////////////////////////////////////////////
function SetDrawing( /* GlgObject */ subwindow,
                     /* GlgObject */ drawing )    /* GlgObject */
{
   if( subwindow == null || drawing == null )
   {
      alert( "Drawing loading failed" );
      return null;
   }

   /* Set the new drawing as a template of the subwindow.
      The new drawing will be setup, and HierarchyCallback will be invoked 
      before and after hierarchy setup for new drawing. This callback can be
      used to invoke code for initializing the new drawing using the same
      code logic as in the desktop version of the application, making it 
      easier to port and maintain several versions of the application.
   */
   subwindow.SetTemplate( drawing );
    
   // Return the viewport displayed in the subwindow.
   return( subwindow.GetResourceObject( "Instance" ) );
}

//////////////////////////////////////////////////////////////////////////////
// Query tags for a given viewport and rebuild TagRecordArray. 
// The new_drawing parameter is the viewport of the new drawing. 
// TagRecordArray will include tags for the top level viewport 
// of the viewer, including tags for the loaded page, as well as
// tags for the popup dialogs, if any.
//////////////////////////////////////////////////////////////////////////////
function QueryTags( /* GlgObject */ new_drawing )
{
   // Delete existing tag records from TagRecordArray.
   DeleteTagRecords();
    
   /* Remap tags in the loaded drawing if needed.
      Will invoke HMIPage's RemapTagObject() for each tag.
   */
   if( new_drawing != null && HMIPage.NeedTagRemapping() )
     RemapTags( new_drawing );
    
   /* Build an array of tag records containing tags information and
      store it in TagRecordArray. TagRecordArray will be used for 
      unimating the drawing with data.
      Always create data for the top level viewport (MainViewport),
      to keep a global list of tags that include tags in any dynamically
      loaded dialogs.
   */
   CreateTagRecords( MainViewport );
}

//////////////////////////////////////////////////////////////////////////////
// Create an array of tag records containing tag information.
//////////////////////////////////////////////////////////////////////////////
function CreateTagRecords( /* GlgObject */ drawing_vp )
{
   var tag_obj;     /* GlgObject */
   var
     tag_source,    /* String  */
     tag_name,      /* String  */
     tag_comment;   /* String  */
   var
     data_type,     /* int */
     access_type;   /* int */
    
   // Obtain a list of tags with unique tag sources.
   var tag_list =    /* GlgObject  */
     drawing_vp.CreateTagList( /* List each tag source only once */ true );
   if( tag_list == null )
     return;   
    
   var size = tag_list.GetSize();
   if( size == 0 )
     return; // no tags found 
    
   for( var i=0; i<size; ++i )
   {
      tag_obj = tag_list.GetElement( i );
        
      // Skip OUTPUT tags.
      access_type = Math.trunc( tag_obj.GetDResource( "TagAccessType" ) );
      if( access_type == GLG.GlgTagAccessType.OUTPUT_TAG )
        continue;

      /* Retrieve TagSource, the name of the database field used as a 
         data source. */
      tag_source = tag_obj.GetSResource( "TagSource" );

      // Skip undefined tag sources, such as "" or "unset".
      if( IsUndefined( tag_source ) )
        continue; 
        
      if( RandomData )
      {
         /* For demo purposes only, skip tags that have:
            - TagName contains "Speed" or "Test";
            - TagSource contains "Test";
            - TagComment contains "Test".
            Such tags are present in motor_info.g displayed in the 
            PopupDialog, as well as scada_test_commands.g. 
            The demo shows how to set the value for these tags 
            using commands WriteValue or WriteValueFromWidget. 
         */
         tag_name = tag_obj.GetSResource( "TagName" );
         tag_comment = tag_obj.GetSResource( "TagComment" );
         if( !IsUndefined( tag_name ) && 
             ( tag_name.includes( "Speed" ) || tag_name.includes( "Test" ) ) )
           continue;
         if( tag_source.includes( "Test" ) )
           continue;
         if( !IsUndefined( tag_comment ) && tag_comment.includes( "Test" ) )
           continue;
      }

      // Get tag object's data type: GLG_D, GLG_S or GLG_G
      data_type = Math.trunc( tag_obj.GetDResource( "DataType" ) );
        
      /* Create a new tag record. plot_time_ep will be assigned in
         SetPlotTimeEP() if needed.
      */
      var tag_record = new GlgTagRecord( data_type, tag_source, tag_obj );
        
      // Add a new tag record to TagRecordArray
      TagRecordArray.push( tag_record );
   }
    
   // Store number of tag records.
   NumTagRecords = TagRecordArray.length;

   /* If a drawing contains a chart, ValueEntryPoint of each plot 
      may have a tag to push data values using the common tag mechanism. 
      The time stamp for the corresponding TimeEntryPoint of the plot
      may be supplied either automatically by the chart 
      (SUPPLY_PLOT_TIME_STAMP=false), or the application may supply the
      time stamp explicitly for each data sample 
      (SUPPLY_PLOT_TIME_STAMP=true).
       
      If SUPPLY_PLOT_TIME_STAMP=1, each tag record in TagRecordArray
      should store corresponding plot's TimeEntryPoint (plot_time_ep).
      If not found, plot_time_ep will be null.
   */
   if( SUPPLY_PLOT_TIME_STAMP )
     SetPlotTimeEP( drawing_vp );
}

//////////////////////////////////////////////////////////////////////////////
// Delete tag records frogm TagRecordArray.
//////////////////////////////////////////////////////////////////////////////
function DeleteTagRecords()
{
   if( TagRecordArray.length > 0 )
     TagRecordArray.length = 0;
    
   NumTagRecords = 0;      
}

//////////////////////////////////////////////////////////////////////////////
// Store TimeEntryPoint (plot_time_ep) in each tag record, if found.
//////////////////////////////////////////////////////////////////////////////
function SetPlotTimeEP( /* GlgObject */ drawing_vp )
{
   if( NumTagRecords == 0 )
     return;
    
   // Obtain a list of all tags, including non-unique tag sources.
   var tag_list =    /* GlgObject */
     drawing_vp.CreateTagList( /* List all tags */ false ); 
    
   if( tag_list == null )
     return;
    
   var size = tag_list.GetSize();
   if( size == 0 )
     return; /* no tags found */
    
   /* For each tag in the list, check if there is a plot object in the
      drawing with a matching TagSource for the plot's ValueEntryPoint.
      If found, obtain plot's TimeEntryPoint and store it in the
      TagRecordArray for a tag record with a matching tag_source.
   */
   for( var i=0; i<size; ++i )
   {
      var tag_obj = tag_list.GetElement( i );   /* GlgObject */
        
      /* Check if the tag belongs to a chart's Entry Point.
         If yes, proceed with finding the PLOT object the tag belongs to.
         Otherwise, skip this tag object.
      */
      if( tag_obj.GetDResource( "AlwaysChanged" ) == 0 )
        continue;
        
      /* Retrieve TagSource and TagComment. In the demo, TagComment is
         not used, but the application may use it as needed.
      */
      var tag_comment = tag_obj.GetSResource( "TagComment" );   /* String */
      var tag_source = tag_obj.GetSResource( "TagSource" );     /* String */
        
      if( IsUndefined( tag_source ) )
        return;
        
      /* Find a TimeEntryPoint of a plot in a RealTimeChart with
         a matching TagSource assigned for the plot's ValueEntryPoint.
         It is assumed that there is ONLY ONE plot in the drawing 
         with a given TagSource. 
      */
      var plot_time_ep = FindMatchingTimeEP( tag_obj );   /* GlgObject */
        
      if( plot_time_ep == null )
        continue;   /* There is no plot for this tag source. */
        
      /* Find a tag record in TagRecordArray with a matching tag_source.
         If found, assign plot_time_ep. Otherwise, generate an error.
      */
      var tag_record = LookupTagRecords( tag_source );
        
      if( tag_record != null )
        tag_record.plot_time_ep = plot_time_ep;
      else /* shouldn't happen */
        console.error( "No matching tag record, TimeEntryPoint not stored." );
   }
}

//////////////////////////////////////////////////////////////////////////////
// For a given tag object, find a parent plot object (PLOT object type). 
// If found, return the plot's TimeEntryPoint.
// It is assumed that there is ONLY ONE plot in the drawing with a given
// TagSource. 
//////////////////////////////////////////////////////////////////////////////
function FindMatchingTimeEP( /* GlgObject */ tag_obj )   /* */
{
   /* Search for a plot object type, which is a parent of the tag_obj
      (ValueEntryPoint).
   */
   var match_type = GLG.GlgObjectMatchType.OBJECT_TYPE_MATCH;
   var find_parents = true;
   var find_first_match = true;
   var search_inside = false;
   var search_drawable_only = false;
   var object_type = GLG.GlgObjectType.PLOT;
   // null parameters at the end: may be omitted.
   // var object_name = null;
   // var resource_name = null;
   // var object_id = null;
   // var custom_match = null;
    
   var rval =
     tag_obj.FindMatchingObjects( match_type, find_parents, find_first_match,
                                  search_inside, search_drawable_only,
                                  object_type
                                  /* omitting trailing null parameters */ );
   if( rval == null || rval.found_object == null )
     return null; /* matching object not found */
    
   var plot = rval.found_object;   /* GlgObject */
   return plot.GetResourceObject( "TimeEntryPoint" );
}

//////////////////////////////////////////////////////////////////////////////
// Lookup TagRecordArray and return a matching tag record with 
// tag_source=match_tag_source.
//////////////////////////////////////////////////////////////////////////////
function LookupTagRecords( /* String */ match_tag_source )   /* TagRecord */
{
   if( IsUndefined( match_tag_source ) )
     return null;

   var num_tag_records = TagRecordArray.length;
   for( var i=0; i<num_tag_records; ++i )
   {
      var tag_record = TagRecordArray[i];

      if( tag_record.tag_source == match_tag_source )
        return tag_record;
   }
    
   return null; // not found
}

//////////////////////////////////////////////////////////////////////////////
// Close all active popups, including popup dialogs and popup menus.
//////////////////////////////////////////////////////////////////////////////
function CloseActivePopups( /* int */ allow_dialog )
{
   /* Close all active dialogs, except the ones which may remain
      visible until closed by the user, as specified by the allow_dialog 
      parameter.
   */
   for( var i=0; i<MAX_DIALOG_TYPE; ++i )
   {
      if( i == allow_dialog )
        continue; 
        
      ClosePopupDialog( i );
   }
    
   /* Close Global PopupMenu, if any. */
   ClosePopupMenu( GLOBAL_POPUP_MENU );
}

//////////////////////////////////////////////////////////////////////////////
// Process commands attached to objects at design time.
// Command types are defined in CommandTypeTable.
// CommandType strings in the table must match the CommandType strings 
// defined in the drawing.
//////////////////////////////////////////////////////////////////////////////
function ProcessObjectCommand( /* GlgObject */ command_vp, 
                               /* GlgObject */ selected_obj, 
                               /* GlgObject */ action_obj )
{
   var command_obj;   /* GlgObject */
   var
     command_type,    /* int */
     dialog_type,     /* int */
     menu_type;       /* int */
    
   if( selected_obj == null || action_obj == null )
     return;
    
   /* Retrieve Command object. */
   command_obj = action_obj.GetResourceObject( "Command" );
   if( command_obj == null )
     return;
    
   command_type = GetCommandType( command_obj );
   switch( command_type )
   {
    case SHOW_ALARMS:
      ShowAlarms();
      break;
    case POPUP_DIALOG:
      DisplayPopupDialog( command_vp, selected_obj, command_obj );
      break;
    case CLOSE_POPUP_DIALOG:
      dialog_type = GetDialogType( command_obj );
      ClosePopupDialog( dialog_type );
      break;
    case POPUP_MENU:
      DisplayPopupMenu( command_vp, selected_obj, command_obj );
      break;
    case CLOSE_POPUP_MENU:
      menu_type = GetPopupMenuType( command_obj );
      ClosePopupMenu( menu_type );
      break;
    case GOTO:
      GoTo( command_vp, selected_obj, command_obj );
      break;
    case WRITE_VALUE:
      WriteValue( command_vp, selected_obj, command_obj );
      break;
    case WRITE_VALUE_FROM_WIDGET:
      WriteValueFromInputWidget( command_vp, selected_obj, command_obj );
      break;
    default: 
      console.log( "Command failed: Undefined CommandType." );
      break;
   }
}

//////////////////////////////////////////////////////////////////////////////   
// Opens or closes the alarm window.
//////////////////////////////////////////////////////////////////////////////   
function ShowAlarms()
{
   AlarmDialogVisible = ToggleResource( "AlarmDialog/Visibility" );
    
   /* If the alarm dialog is becoming visible, fill alarms to show them
      right away.
   */
   if( FirstAlarmDialog )   // Show the help message the first time only.
   {
      FirstAlarmDialog = false;

      ShowMessageDialog( ( TouchDevice == 1 ? "Click" : "Ctrl-click" ) +
                         " on the alarm row to acknowledge an alarm.", false );
   }
   MainViewport.Update();
}

//////////////////////////////////////////////////////////////////////////////   
function CloseAlarmDialog()
{
   if( AlarmDialogVisible )
     ShowAlarms();   // Toggle the state of alarm dialog to erase it.
}

//////////////////////////////////////////////////////////////////////////////
function ShowMessageDialog( /* String */ message, /* boolean */ error )
{
   MessageDialog.SetSResource( "MessageString", message );
    
   /* Set to 1. to highlight the message in red. */
   MessageDialog.SetDResource( "ShowErrorColor", error ? 1.0 : 0.0 );
    
   MessageDialog.SetDResource( "Visibility", 1.0 );
}

//////////////////////////////////////////////////////////////////////////////
// Toggles resource value between 0 and 1 and returns the new value
// as a boolean.
//////////////////////////////////////////////////////////////////////////////
function ToggleResource( /* String */ resource_name )   /* boolean */
{
   var current_value = MainViewport.GetDResource( resource_name );  /* double */

   current_value = ( current_value == 0.0 ? 1.0 : 0.0 );
    
   MainViewport.SetDResource( resource_name, current_value );
   return ( current_value == 1.0 );      
}

//////////////////////////////////////////////////////////////////////////////   
// Process command "PopupDialog". The requested dialog can be embedded 
// into the currently loaded drawing, or a global PopupDialog can be used 
// as a popup. Command parameters DialogType and DialogResource are
// mandatory. DialogResource specifies the resource path of the dialog 
// to be displayed. If DialogResource starts with a forward slash ('/'), 
// the dialog name is relative to the top level viewport MainViewport; 
// otherwise, it is relative to the currently loaded viewport where the 
// command occurred, i.e. command_vp.
//
// The popup dialog is initialized as needed, using parameters of the 
// selected object. In this demo, the tag sources are transfered from the 
// selected object to the dialog viewport for the tags with a matching
// TagName.
//////////////////////////////////////////////////////////////////////////////   
function DisplayPopupDialog( /* GlgObject */ command_vp,
                             /* GlgObject */ selected_obj, 
                             /* GlgObject */ command_obj )     
{
   var
     subwindow,   /* GlgObject */
     dialog;      /* GlgObject */
    
   /* Retrieve command parameters (strings). */
   var dialog_res = command_obj.GetSResource( "DialogResource" );
   var drawing_file = command_obj.GetSResource( "DrawingFile" );
   var destination_res = command_obj.GetSResource( "Destination" );

   /* Obtain DialogType. */
   var dialog_type = GetDialogType( command_obj );   /* int */
   if( dialog_type == UNDEFINED_DIALOG_TYPE )
   {
      console.error( "PopupDialog Command failed: Unknown DialogType." );
      return;
   }
    
   /* Close active popups, if any. To avoid flickering, do not close the 
      dialog with the same dialog type that is requested by this command.
   */
   CloseActivePopups( dialog_type );

   AbortPendingPopupLoads();   // Cancel any pending popup load requests.
    
   /* DialogResource specifies resource path of the dialog to be displayed.
      If the path starts with '/', it is relative to the top level 
      $Widget viewport. Otherwise, the path is relative to the 
      viewport of the Input callback (command_vp).
   */
   if( IsUndefined( dialog_res ) )
   {
      console.error( "PopupDialog Command failed: Invalid DialogResource." );
      ClosePopupDialog( dialog_type );
      return;
   }
    
   /* Obtain an object ID of the requested popup dialog. 
      If invalid, abort the command. 
   */
   if( dialog_res.startsWith("/") )
     /* skip '/' */
     dialog = MainViewport.GetResourceObject( dialog_res.substring( 1 ) );
   else 
     dialog = command_vp.GetResourceObject( dialog_res );
    
   if( dialog == null )
   {
      console.error( "PopupDialog Command failed: Dialog not found." );
      ClosePopupDialog( dialog_type );
      return;
   }

   if( IsUndefined( drawing_file ) )
   {
      /* DrawingFile is not defined, use dialog as a popup viewport. */
      var popup_vp = dialog;     /* GlgObject */
      FinishDisplayPopupDialog( selected_obj, command_obj, dialog, dialog_type,
                                null, popup_vp, popup_vp );
   }
   else      /* If DrawingFile is present, load the corresponding drawing into 
                the subwindow object defined by the Destination parameter. */
   {      
      /* Use Destination resource, if any, to display the specified drawing. 
         If omitted, use default name "DrawingArea". It is assumed that 
         Destination points to the subwindow object inside a dialog.
      */
      var subwindow_name;   /* String */
      if( IsUndefined( destination_res ) )
        subwindow_name = "DrawingArea"; /* Use default name. */
      else
        subwindow_name = destination_res;
        
      /* Obtain an object ID of the subwindow inside a dialog. */
      subwindow = dialog.GetResourceObject( subwindow_name );
      if( subwindow == null )
      {
         console.error( "PopupDialog Command failed: Destination object not found." );
         ClosePopupDialog( dialog_type );
         return;
      }

      /* Store a new load request in a global variable to be able to abort it
         if needed.
      */
      PopupLoadRequest = { enabled : true,
                           command_obj : command_obj,
                           selected_obj : selected_obj,
                           dialog : dialog,
                           dialog_type : dialog_type,
                           subwindow : subwindow };
        
      // Request to load the new drawing and invoke the callback when it's ready.
      GLG.LoadObjectFromURL( drawing_file, null,
                             DisplayPopupDialogCB, PopupLoadRequest, AbortLoad,
                             /* subwindow to handle bindings */ subwindow );
   }
}

////////////////////////////////////////////////////////////////////////////// 
function DisplayPopupDialogCB( /* GlgObject */ loaded_drawing,
                               /* Object */ user_data, /* String */ path )
{
   var load_request = user_data;

   if( !load_request.enabled )
     /* This load request was aborted by requesting to load another page before
        this load request has finished.
     */
     return;
    
   PopupLoadRequest = null;   // Reset: we are done with this request.

   var dialog_type = load_request.dialog_type;
   var subwindow = load_request.subwindow;
    
   /* Close active popups again: they might have been activated while the
      dialog was being loaded. To avoid flickering, do not close the 
      dialog with the same dialog type that is requested by this command.
   */
   CloseActivePopups( dialog_type );

   if( loaded_drawing == null )
   {
      alert( "Dialog loading failed" );
      ClosePopupDialog( dialog_type );
      return;
   }
    
   // Load new popup drawing and obtain an object id of its viewport.
   var popup_vp = SetDrawing( subwindow, loaded_drawing );    /* GlgObject */
   if( popup_vp == null )
   {
      console.error( "PopupDialog Command failed." );
      ClosePopupDialog( dialog_type );
      return;
   }

   var command_obj = load_request.command_obj;   /* GlgObject */ 
   var selected_obj = load_request.selected_obj;  /* GlgObject */ 
   var dialog = load_request.dialog;             /* GlgObject */ 

   FinishDisplayPopupDialog( selected_obj, command_obj, dialog, dialog_type,
                             subwindow, popup_vp, loaded_drawing );
}

////////////////////////////////////////////////////////////////////////////// 
function FinishDisplayPopupDialog( /* GlgObject */ selected_obj,
                                   /* GlgObject */ command_obj,
                                   /* GlgObject */ dialog,
                                   /* int */ dialog_type,
                                   /* GlgObject */ subwindow,
                                   /* GlgObject */ popup_vp,
                                   /* GlgObject */ popup_drawing )
{
   /* Increase dialog size for mobile devices with coord. scaling. */
   if( CoordScale != 1.0 && dialog.HasResourceObject( "CoordScale" ) )
     dialog.SetDResource( "CoordScale", 2.0 );

   DialogAdjustForMobileDevices( popup_vp );
    
   var title = command_obj.GetSResource( "Title" );   /* String */
   
   /* For the tags with matching TagName, transfer tag sources from the 
      selected object to the loaded popup viewport.
   */
   TransferTags( selected_obj, popup_vp, false );

   // Initialize active popup dialog based on WidgetType, if any.
   var widget_type = GetWidgetType( popup_drawing ); /* String */
   switch( widget_type )
   {
      // Add custom cases here as needed.
    default: break;
   }
    
   /* If a new dialog drawing was loaded, rebuild TagRecordArray to 
      include tags both for the drawing displayed in the main drawing area 
      and drawing displayed in the popup dialog.
   */
   if( !popup_vp.Equals( dialog ) )
     QueryTags( popup_vp );
    
   /* Poll new data to fill the popup dialog with current values. */
   UpdateData();
    
   /* Display title in the loaded viewport, if Title resource is found. */
   if( popup_vp.HasResourceObject( "Title" ) )
     popup_vp.SetSResource( "Title", title );
    
   /* Display title as the dialog caption. */
   dialog.SetSResource( "ScreenName", title );
    
   /* Display the dialog if it is not up already. */
   dialog.SetDResource( "Visibility", 1.0 );
    
   /* Store dialog information in ActiveDialogs array */
   ActiveDialogs[ dialog_type ].dialog_type = dialog_type;
   ActiveDialogs[ dialog_type ].dialog = dialog;
   ActiveDialogs[ dialog_type ].subwindow = subwindow;
   ActiveDialogs[ dialog_type ].popup_vp = popup_vp;
   ActiveDialogs[ dialog_type ].isVisible = true;

   dialog.Update();
}

////////////////////////////////////////////////////////////////////////////// 
// Close active popup dialog of a given type.
//////////////////////////////////////////////////////////////////////////////
function ClosePopupDialog( /* int */ dialog_type )
{
   if( dialog_type == UNDEFINED_DIALOG_TYPE || 
       dialog_type >= MAX_DIALOG_TYPE )
   {
      console.error( "Dialog closing failed." ); 
      return;
   }
    
   if( ActiveDialogs[ dialog_type ].dialog == null )
     return; /* nothing to do. */
    
   if( ActiveDialogs[ dialog_type ].subwindow != null && 
       ActiveDialogs[ dialog_type ].popup_vp != null )
   {
      /* Destroy currently loaded popup drawing and load empty drawing.
         EmptyDrawing is preloaded as an asset to be able to close dialogs
         without waiting for the load to finish.
      */
      SetDrawing( ActiveDialogs[ dialog_type ].subwindow,
                  EmptyDrawing.CopyObject() );
        
      /* Rebuild a list of tags to exclude the tags from the previously
         loaded popup viewport.
      */
      QueryTags( null );
   }
    
   /* Hide the dialog */
   ActiveDialogs[ dialog_type ].dialog.SetDResource( "Visibility", 0.0 );
    
   /* Clear a dialog record with a specified index (dialog_type)
      in the ActiveDialogs array.
   */
   ActiveDialogs[ dialog_type ].dialog_type = UNDEFINED_DIALOG_TYPE;
   ActiveDialogs[ dialog_type ].dialog = null;
   ActiveDialogs[ dialog_type ].subwindow = null;
   ActiveDialogs[ dialog_type ].popup_vp = null;
   ActiveDialogs[ dialog_type ].isVisible = false;
}

////////////////////////////////////////////////////////////////////////////// 
// Transfer tag sources for tags with a matching TagName from the
// selected object to the specified viewport.
////////////////////////////////////////////////////////////////////////////// 
function TransferTags( /* GlgObject */ selected_obj, /* GlgObject */ viewport, 
                       /* boolean */ unset_tags )
{
   var
     tag_list,             /* GlgObject */
     tag_obj;              /* GlgObject */
   var
     tag_source,           /* String */
     tag_name;             /* String */
   var num_remapped_tags;  /* int */

   /* Obtain a list of tags defined in the selected object. */
   tag_list = selected_obj.CreateTagList( /* List all tags */ false );        
   if( tag_list == null )
     return;
    
   var size = tag_list.GetSize();   /* int */
   if( size == 0 )
     return; /* no tags found */
    
   /* Traverse the tag list. For each tag, transfer the TagSource
      defined in the selected object to the tag in the loaded 
      popup drawing that has a matching TagName.
   */
   for( var i=0; i<size; ++i )
   {
      tag_obj = tag_list.GetElement( i );
        
      /* Obtain TagName. */
      tag_name = tag_obj.GetSResource( "TagName" );
        
      /* Skip tags with undefined TagName */
      if( IsUndefined( tag_name ) )
        continue;
        
      /* Obtain TagSource. */
      tag_source = tag_obj.GetSResource( "TagSource" );
        
      /* Skip tags with undefined TagSource. */
      if( IsUndefined( tag_source ) )
        continue;
        
      /* Remap all tags with the specified tag name (tag_name)
         to use a new tag source (tag_source).
      */
      if( unset_tags )
        num_remapped_tags = RemapNamedTags( viewport, tag_name, "unset" );
      else
        num_remapped_tags = RemapNamedTags( viewport, tag_name, tag_source );
   }
}

////////////////////////////////////////////////////////////////////////////// 
// Process command "PopupMenu". The requested popup menu can be embedded 
// in the currently loaded drawing, or a global PopupMenu can be used 
// as a popup. Command parameters MenuType and MenuResource are
// mandatory. MenuResource specifies the resource path of the menu object 
// to be displayed. If MenuResource starts with a forward slash ('/'), 
// the menu name is relative to the top level viewport MainViewport; 
// otherwise, it is relative to the currently loaded viewport where the 
// command occurred, i.e. command_vp.
//
// The popup menu is initialized as needed, using parameters of the 
// selected object. In this demo, the tag sources are transfered from the 
// selected object to the menu viewport for the tags with a matching
// TagName.
////////////////////////////////////////////////////////////////////////////// 
function DisplayPopupMenu( /* GlgObject */ command_vp,
                           /* GlgObject */ selected_obj, 
                           /* GlgObject */ command_obj )
{
   var
     subwindow,   /* GlgObject */
     menu_obj;    /* GlgObject */
    
   /* Retrieve command parameters (strings). */
   var menu_res = command_obj.GetSResource( "MenuResource" );
   var drawing_file = command_obj.GetSResource( "DrawingFile" );
   var destination_res = command_obj.GetSResource( "Destination" );
    
   /* Obtain MenuType. */
   var menu_type = GetPopupMenuType( command_obj );   /* int */
   if( menu_type == UNDEFINED_POPUP_MENU_TYPE )
   {
      console.error( "PopupMenu Command failed: Unknown MenuType." );
      return;
   }
    
   /* Close active popups, if any. */
   CloseActivePopups( CLOSE_ALL );

   AbortPendingPopupLoads();   // Cancel any pending popup load requests.
    
   /* MenuResource specifies resource path of the menu object to be 
      displayed. If the path starts with '/', it is relative to the 
      top level $Widget viewport. Otherwise, the path is relative to 
      the viewport of the Input callback (command_vp).
   */
   if( IsUndefined( menu_res ) )
   {
      console.error( "PopupMenu Command failed: Invalid MenuResource." );
      return;
   }
    
   /* Obtain an object ID of the requested popup menu. 
      If invalid, abort the command. 
   */
   if( menu_res.startsWith("/") )
     /* skip '/'*/
     menu_obj = MainViewport.GetResourceObject( menu_res.substring( 1 ) );
   else 
     menu_obj = command_vp.GetResourceObject( menu_res );
    
   if( menu_obj == null )
   {
      console.error( "PopupMenu Command failed: Menu object not found." );
      return;
   }
    
   if( IsUndefined( drawing_file ) )
   {
      /* DrawingFile is not defined, use menu_obj as popup viewport. */
      var popup_vp = menu_obj;   /* GlgObject */
      FinishDisplayPopupMenu( selected_obj, command_obj, menu_obj, menu_type,
                              null, popup_vp );
   }
   else
   {
      /* DrawingFile is defined, use it to load the corresponding 
         drawing into the subwindow object defined by the Destination 
         parameter. It is assumed that Destination points to the 
         subwindow object inside the menu object.
      */
      var subwindow_name;   /* String */
      if( IsUndefined( destination_res ) ) 
        subwindow_name = "DrawingArea"; /* Use default name. */
      else
        subwindow_name = destination_res;
        
      /* Obtain an object ID of the subwindow inside the menu object. */
      subwindow = menu_obj.GetResourceObject( subwindow_name );
      if( subwindow == null )
      {
         console.error( "PopupDialog Command failed: Destination object not found." );
         return;
      }
        
      /* Store a new load request in a global variable to be able to abort it
         if needed.
      */
      PopupLoadRequest = { enabled : true,
                           command_obj : command_obj,
                           selected_obj : selected_obj,
                           menu_obj : menu_obj,
                           menu_type : menu_type,
                           subwindow : subwindow };
        
      // Request to load the new drawing and invoke the callback when it's ready.
      GLG.LoadObjectFromURL( drawing_file, null,
                             DisplayPopupMenuCB, PopupLoadRequest, AbortLoad,
                             /* subwindow to handle bindings */ subwindow );
   }
}

////////////////////////////////////////////////////////////////////////////// 
function DisplayPopupMenuCB( /* GlgObject */ loaded_drawing,
                             /* Object */ user_data, /* String */ path )
{
   var load_request = user_data;

   if( !load_request.enabled )
     /* This load request was aborted by requesting to load another page before
        this load request has finished.
     */
     return;
    
   PopupLoadRequest = null;   // Reset: we are done with this request.
    
   /* Close active popups again: they might have been activated while the
      menu was being loaded. 
   */
   CloseActivePopups( CLOSE_ALL );

   if( loaded_drawing == null )
   {
      alert( "Menu loading failed" );
      return;
   }
    
   // Load new menu drawing and obtain an object id of its viewport.
   var subwindow = load_request.subwindow;
   var popup_vp = SetDrawing( subwindow, loaded_drawing );    /* GlgObject */
   if( popup_vp == null )
   {
      console.error( "PopupMenu Command failed." );
      return;
   }

   var command_obj = load_request.command_obj;    /* GlgObject */ 
   var selected_obj = load_request.selected_obj;  /* GlgObject */ 
   var menu_obj = load_request.menu_obj;          /* GlgObject */ 
   var menu_type = load_request.menu_type;          /* GlgObject */ 

   /* If the viewport has Width and Height resources that define
      its size in pixels, adjust the size of the menu object 
      to match the size of the loaded viewport.
   */
   if( popup_vp.HasResourceObject( "Width" ) && 
       menu_obj.HasResourceObject( "Width" ) )
   {
      var menu_width;   /* double */
      menu_width = popup_vp.GetDResource( "Width" );
      menu_obj.SetDResource( "Width", menu_width );
   }
    
   if( popup_vp.HasResourceObject( "Height" ) &&
       menu_obj.HasResourceObject( "Height" ) )
   {
      var menu_height;   /* double */
      menu_height =  popup_vp.GetDResource( "Height" );
      menu_obj.SetDResource( "Height", menu_height );
   }

   FinishDisplayPopupMenu( selected_obj, command_obj, menu_obj, menu_type,
                           subwindow, popup_vp );
}

////////////////////////////////////////////////////////////////////////////// 
function FinishDisplayPopupMenu( /* GlgObject */ selected_obj,
                                 /* GlgObject */ command_obj,
                                 /* GlgObject */ menu_obj,
                                 /* int */ menu_type,
                                 /* GlgObject */ subwindow,
                                 /* GlgObject */ popup_vp )
{
   /* Increase the size of the menu and its close icon for mobile devices
      with coord. scaling.
   */
   if( CoordScale != 1.0 )
   {
      if( menu_obj.HasResourceObject( "CoordScale" ) )
        menu_obj.SetDResource( "CoordScale", 3. );

      if( popup_vp.HasResourceObject( "CoordScale" ) )
        popup_vp.SetDResource( "CoordScale", 3. );
   }
    
   /* Transfer tag sources from the selected object to the loaded 
      popup viewport, using tags with a matching TagName.
   */
   TransferTags( selected_obj, popup_vp, false );
    
   /* Display title in the loaded viewport, if Title resource is found. */
   if( popup_vp.HasResourceObject( "Title" ) )
   {
      var title = command_obj.GetSResource("Title" );   /* String */      
      popup_vp.SetSResource( "Title", title );
   }
    
   /* Show the menu. */
   menu_obj.SetDResource( "Visibility", 1.0 );
    
   /* Store menu information in the global ActivePopupMenu structure, 
      used to close the active popup menu.
   */
   ActivePopupMenu.menu_type = menu_type;
   ActivePopupMenu.menu_obj = menu_obj;
   ActivePopupMenu.subwindow = subwindow;
   ActivePopupMenu.menu_vp = popup_vp;
   ActivePopupMenu.selected_obj = selected_obj; 
   ActivePopupMenu.isVisible = true; 
    
   /* Position the menu next to the selected object. */
   PositionPopupMenu();
    
   menu_obj.Update();
}

////////////////////////////////////////////////////////////////////////////// 
// Position ActivePopupMenu at the upper right corner of the selected object,
// if possible. Otherwise, position the menu close to the selected object
// such that it is displayed within the current viewport.
////////////////////////////////////////////////////////////////////////////// 
function PositionPopupMenu()
{
   var   /* GlgObject */
     selected_obj_vp,  // Viewport that contains selected object.
     menu_parent_vp;   // Parent viewport that contains the popup menu.
   var  /* double */
     x, y,
     offset = 5.0,     // offset in pixels.
     menu_width, menu_height,
     parent_width, parent_height; 
   var  /* int */
     x_anchor,
     y_anchor; 
    
   if( ActivePopupMenu.selected_obj == null || 
       ActivePopupMenu.menu_obj == null )
     return;
    
   selected_obj_vp = ActivePopupMenu.selected_obj.GetParentViewport( true );
   menu_parent_vp = ActivePopupMenu.menu_obj.GetParentViewport( true );
    
   /* Obtain the object's bounding box in screen coordinates. */
   var sel_obj_box = ActivePopupMenu.selected_obj.GetBox();   /* GlgCube */
   var converted_box = GLG.CopyGlgCube( sel_obj_box );        /* GlgCube */
    
   /* If the menu is located in a different viewport from the viewport
      of the selected object, convert screen coordinates of the 
      selected object box from the viewport of the selected object to the 
      viewport that contains the popup menu.
   */
   if( !selected_obj_vp.Equals( menu_parent_vp ) )
   {
      GLG.TranslatePointOrigin( selected_obj_vp, menu_parent_vp,
                                converted_box.p1 );
      GLG.TranslatePointOrigin( selected_obj_vp, menu_parent_vp,
                                converted_box.p2 );
   }
    
   /* Obtain width and height in pixels of the parent viewport 
      of the menu. 
   */
   parent_width = menu_parent_vp.GetDResource( "Screen/Width" );
   parent_height = menu_parent_vp.GetDResource( "Screen/Height" );
    
   /* Obtain width and height of the menu object. */
   var menu_obj_box = ActivePopupMenu.menu_obj.GetBox();   /* GlgCube */
   menu_width = menu_obj_box.p2.x - menu_obj_box.p1.x;
   menu_height = menu_obj_box.p2.y - menu_obj_box.p1.y;
    
   /* Position the popup at the upper right or lower left corner of 
      the selected object, if possible. Otherwise (viewport is too small), 
      position it in the center of the viewport.
   */   
   if( converted_box.p2.x + menu_width + offset > parent_width )
   {
      /* Outside of the window right edge. 
         Position the right edge of the popup to the left of the selected object.
         Always use HLEFT anchor to simplify out-of-the-window check.
      */
      x =  converted_box.p1.x - offset - menu_width;
      x_anchor = GLG.GlgAnchoringType.HLEFT;
   }
   else 
   {
      // Position the left edge of the popup to the right of the selected object.
      x = converted_box.p2.x + offset; 
      x_anchor = GLG.GlgAnchoringType.HLEFT;
   }

   /* Anchor is always HLEFT here to make checks simpler. */
   if( x < 0 || x + menu_width > parent_width )
   {
      /* Not enough space: place in the center. */
      x = parent_width / 2.0;
      x_anchor = GLG.GlgAnchoringType.HCENTER;
   }
    
   if( converted_box.p1.y - menu_height - offset < 0.0 ) 
   {
      /* Outside of window top edge.
         Position the top edge of the popup below the selected object.
      */
      y =  converted_box.p2.y + offset;
      y_anchor = GLG.GlgAnchoringType.VTOP;
   }
   else 
   {
      /* Position the bottom edge of the popup above the selected object.
         Always use GLG_VTOP anchor to simplify out-of-the-window check.
      */
      y = converted_box.p1.y - offset - menu_height; 
      y_anchor = GLG.GlgAnchoringType.VTOP;
   }
    
   /* Anchor is always GLG_VTOP here to make checks simpler. */
   if( y < 0 || y + menu_height > parent_height )
   {
      /* Not enough space: place in the center. */
      y = parent_height / 2.0;
      y_anchor = GLG.GlgAnchoringType.HCENTER;
   }
    
   ActivePopupMenu.menu_obj.PositionObject( GLG.GlgCoordType.SCREEN_COORD, 
                                            x_anchor | y_anchor, x, y, 0.0 );
}

////////////////////////////////////////////////////////////////////////////// 
// Close active popup menu. In this demo, menu_type is not used, since
// there is only a single ActivePopupMenu object. The code can be 
// extended by the application developer as needed.
////////////////////////////////////////////////////////////////////////////// 
function ClosePopupMenu( /* int */ menu_type )
{
   if( ActivePopupMenu.menu_obj == null )
     return; /* Nothing to do. */
    
   /* Hide active popup. */
   ActivePopupMenu.menu_obj.SetDResource( "Visibility", 0.0 );
   MainViewport.Update();
    
   if( ActivePopupMenu.subwindow != null )
   {
      /* Destroy currently loaded popup drawing and load empty drawing.
         EmptyDrawing is preloaded as an asset to be able to close dialogs
         without waiting for the load to finish.
      */
      SetDrawing( ActivePopupMenu.subwindow, EmptyDrawing.CopyObject() );
   }
   else
   {
      /* Unset tags in the menu object, which were previously
         transfered and assigned from the selected object. 
      */ 
      if( ActivePopupMenu.selected_obj != null )
        TransferTags( ActivePopupMenu.selected_obj,
                      ActivePopupMenu.menu_obj, true );
   }
    
   /* Clear menu record. */
   ActivePopupMenu.menu_type = UNDEFINED_POPUP_MENU_TYPE;
   ActivePopupMenu.menu_obj = null;
   ActivePopupMenu.subwindow = null;
   ActivePopupMenu.menu_vp = null;
   ActivePopupMenu.selected_obj = null;
   ActivePopupMenu.isVisible = false;
}

////////////////////////////////////////////////////////////////////////////// 
// Process "GoTo" command. The command loads a new drawing specified
// by the DrawingFile parameter into the subwindow object specified
// by the Destination parameter. If Destination is omitted, uses
// main DrawingArea subwindow object.  
////////////////////////////////////////////////////////////////////////////// 
function GoTo( /* GlgObject */ command_vp, /* GlgObject */ selected_obj,
               /* GlgObject */ command_obj )
{
   var subwindow;      /* GlgObject */
    
   /* Close active popup dialogs and popup menu. */
   CloseActivePopups( CLOSE_ALL );

   AbortPendingPopupLoads();   // Cancel any pending popup load requests.

   /* Retrieve command parameters (strings). */
   var drawing_file = command_obj.GetSResource( "DrawingFile" );
   var destination_res = command_obj.GetSResource( "Destination" );
    
   /* If DrawingFile is not valid, abort the command. */
   if( IsUndefined( drawing_file ) )
   {
      console.error( "GoTo Command failed: Invalid DrawingFile." );
      return;
   }

   /* Use Destination resource, if any, to display the specified drawing. 
      It is assumed that Destination points to the subwindow object.
      If not defined, use top level DrawingArea subwindow by default.
   */
   if( IsUndefined( destination_res ) ) 
     subwindow = DrawingArea;
   else
   {
      if( destination_res.startsWith("/") )
        /* Destination is relative to the top level viewport (MainViewport).
           Omit the first '/' when using the resource path to obtain the 
           subwindow object.
        */
        subwindow =
          MainViewport.GetResourceObject( destination_res.substring( 1 ) );
      else 
        /* Destination is relative to the current viewport, where the
           command occurred.
        */
        subwindow = command_vp.GetResourceObject( destination_res );
        
      if( subwindow == null )
      {
         console.error( "GoTo Command failed: Invalid Destination." );
         return;
      }
   }
    
   AbortPendingPageLoads();    // Cancel any pending page load requests. 

   // Get title from the command.
   var title = command_obj.GetSResource( "Title" );   /* String */
   SetLoadingMessage( title );

   /* Store a new load request in a global variable to be able to abort it
      if needed.
   */
   PageLoadRequest = { enabled: true,
                       command_obj : command_obj,
                       drawing_file : drawing_file,
                       subwindow : subwindow };

   // Request to load the new drawing and invoke the callback when it's ready.
   GLG.LoadObjectFromURL( drawing_file, null,
                          GoToCB, PageLoadRequest, AbortLoad,
                          /* subwindow to handle bindings */ subwindow );
}

//////////////////////////////////////////////////////////////////////////////
function GoToCB( /* GlgObject */ loaded_drawing, /* Object */ user_data,
                 /* String */ path )
{
   var load_request = user_data;

   if( !load_request.enabled )
     /* This load request was aborted by requesting to load another page before
        this load request has finished.
     */
     return;
    
   PageLoadRequest = null;   // Reset: we are done with this request.
    
   if( loaded_drawing == null )
   {
      alert( "Drawing loading failed" );
      return;
   }

   /* Close active popup dialogs and popup menu again: they might be activated
      while waiting for the new page being loaded.
   */
   CloseActivePopups( CLOSE_ALL );
    
   AbortPendingPopupLoads();   // Cancel any pending popup load requests.
    
   var subwindow = load_request.subwindow;

   // Load new drawing and obtain an object id of its viewport. 
   var drawing_vp = SetDrawing( subwindow, loaded_drawing );   /* GlgObject */
   if( drawing_vp == null )
   {
      /* If drawing loading fails, it will be reported in HierarchyCallback. 
         Generate an additional error indicating command failing.
      */
      console.error( "GoTo Command failed." );
      return;
   }      
    
   /* Rebuild TagRecordArray for the newly loaded drawing. */
   QueryTags( drawing_vp );
    
   if( subwindow.Equals( DrawingArea ) )
   {
      /* Reset main menu selection. If the new drawing matches one of 
         the drawings defined in the MenuArray, update 
         main menu with the corresponding index.
      */
      var drawing_file = load_request.drawing_file;         /* String */
      var screen_index = LookUpMenuArray( drawing_file );   /* int */
      SelectMainMenuItem( screen_index, /* update menu */ true );
        
      var command_obj = load_request.command_obj;           /* GlgObject */
      var title = command_obj.GetSResource( "Title" );      /* String */
      SetupLoadedPage( title );      // Use title from the command.
        
      HMIPage.Ready();
   }
}

//////////////////////////////////////////////////////////////////////////////
// Process command "WriteValue". The command writes a new value specified
// by the Value parameter into the tag in the back-end system
// specfied by the OutputTagHolder.
////////////////////////////////////////////////////////////////////////////// 
function WriteValue( /* GlgObject */ command_vp, /* GlgObject */ selected_obj, 
                     /* GlgObject */ command_obj )
{
   /* Retrieve tag source to write data to. */
   var tag_source =      /* String */
     command_obj.GetSResource( "OutputTagHolder/TagSource" );
    
   /* Validate. */
   if( IsUndefined( tag_source ) )
   {
      console.error( "WriteValue Command failed: Invalid TagSource." );
      return;
   }
    
   /* Retrieve the value to be written to the tag source. */
   var value = command_obj.GetDResource( "Value" );   /* double */
    
   /* Place custom code here as needed, to validate the value specified
      in the command.
   */
    
   /* Write new value to the specified tag source. */
   data_point.d_value = value;
   DataFeed.WriteDTag( tag_source, data_point );
    
   if( RandomData )
   {
      /* For demo purposes, update the tag value in the drawing. 
         In an application, the input tag will be updated 
         from the back-end system.
      */
      MainViewport.SetDTag( tag_source, value, true );
      MainViewport.Update();
   }
}

////////////////////////////////////////////////////////////////////////////// 
// Process command "WriteValueFromWidget". The command allows writing
// a new value into the tag in the back-end system using an input
// widget, such as a toggle or a spinner.
////////////////////////////////////////////////////////////////////////////// 
function WriteValueFromInputWidget( /* GlgObject */ command_vp, 
                                    /* GlgObject */ selected_obj, 
                                    /* GlgObject */ command_obj )
{
   var write_tag_obj, read_tag_obj;   /* GlgObject */

   var value_res = command_obj.GetSResource( "ValueResource" );  /* String */
    
   /* Obtain object ID of the read tag object. FOr example, in case of a
      spinner widget, it will be "Value" resource.
   */
   read_tag_obj = selected_obj.GetResourceObject( value_res );
   if( read_tag_obj == null )
     return;
    
   /* Obtain object ID of the write tag object. */
   write_tag_obj = command_obj.GetResourceObject( "OutputTagHolder" ); 
   if( write_tag_obj == null )
     return;
    
   /* Obtain TagSource from the write tag. */
   var output_tag_source = write_tag_obj.GetSResource( "TagSource" );  // String
    
   /* Validate. */
   if( IsUndefined( output_tag_source ) )
   {
      console.error( "Write Command failed: Invalid Output TagSource." );
      return;
   }
    
   /* Retrieve new value from the input widget. */
   var value = read_tag_obj.GetDResource( null );   /* double */
    
   /* Write the obtained value from the widget to the output tag. */
   data_point.d_value = value;
   DataFeed.WriteDTag( output_tag_source, data_point );
    
   if( RandomData )
   {
      /* Update the tag value in the drawing. In an application,
         the read tag will be updated from the back-end system.
      */
      var read_tag_source = read_tag_obj.GetSResource( "TagSource" );  // String
      if( !IsUndefined( read_tag_source ) )
      {
         MainViewport.SetDTag( read_tag_source, value, true );
         MainViewport.Update();
      }
   }
}

////////////////////////////////////////////////////////////////////////////// 
// Performs zoom/pan operations of the specified type.
////////////////////////////////////////////////////////////////////////////// 
function Zoom( /* GlgObject */ viewport, /* String */ zoom_type,
               /* double */ scale )
{
   var zoom_reset_type = 'n';
    
   if( viewport == null )
     return;
    
   switch( zoom_type )
   {
    default: 
      viewport.SetZoom( null, zoom_type, scale );
      break;
      
    case 'n':
      /* If a viewport is a chart with the chart zoom mode, use 'N'
         to reset both Time and Y ranges. For a chart, 'n' would reset 
         only the Time range.
      */
      var zoom_mode_obj =   /* GlgObject */
        viewport.GetResourceObject( "ZoomModeObject" );
      if( zoom_mode_obj != null )
      {
         var object_type =      /* int */
           Math.trunc( zoom_mode_obj.GetDResource( "Type" ) );
         if( object_type == GLG.GlgObjectType.CHART )
           zoom_reset_type = 'N';
      }
      
      viewport.SetZoom( null, zoom_reset_type, 0.0 );
      break;
   }
}

//////////////////////////////////////////////////////////////////////////////
// Initialize ActiveDialogs array and ActivePopupMenu.
//////////////////////////////////////////////////////////////////////////////
function InitActivePopups()
{
   ActiveDialogs = new Array( MAX_DIALOG_TYPE );

   /* Initialize ActiveDialogs array. */
   for( var i=0; i<MAX_DIALOG_TYPE; ++i )
     ActiveDialogs[ i ] =
       new GlgActiveDialogRecord( UNDEFINED_DIALOG_TYPE,
                                  null, null, null, false );
       
   /* Initialize ActivePopupMenu. */
   ActivePopupMenu =
     new GlgActivePopupMenuRecord( UNDEFINED_POPUP_MENU_TYPE,
                                   null, null, null, null, false );
}

//////////////////////////////////////////////////////////////////////////////
// Returns index of the MenuArray item with a matching drawing_name,
// if any. If not found, returns NO_SCREEN.
//////////////////////////////////////////////////////////////////////////////
function LookUpMenuArray( /* String */ drawing_name )   /* int */
{
   var menu_record;   /* GlgMenuRecord */

   for( var i=0; i<NumMenuItems; ++i )
   {
      menu_record = MenuArray[i];
      if( drawing_name == menu_record.drawing_name )
        return i;
   }
   
   return NO_SCREEN;
}
   
//////////////////////////////////////////////////////////////////////////////
function PageIndexValid( /* int */ page_index )
{
   if( page_index < NO_SCREEN || page_index >= NumMenuItems )
   {
      console.error( "Invalid main menu index." );
      return false;
   }
   return true;
}

//////////////////////////////////////////////////////////////////////////////
// Select MainMenu item with a specified index.
// NO_SCREEN value (-1) unselects a previously selected menu item, if any.
//////////////////////////////////////////////////////////////////////////////
function SelectMainMenuItem( /* int */ menu_index )
{
   if( PageIndexValid( menu_index ) )
     Menu.SetDResource( "SelectedIndex", menu_index );
}
   
//////////////////////////////////////////////////////////////////////////////
// Utility function, queries a PageType property of the drawing and 
// converts it to a PageType integer constant using PageTypeTable.
//////////////////////////////////////////////////////////////////////////////
function GetPageType( /* GlgObject */ drawing )   /* int */
{
   if( drawing == null )
     return DEFAULT_PAGE_TYPE;
   
   var type_obj = drawing.GetResourceObject( "PageType" );   /* GlgObject */
   if( type_obj == null )
     return DEFAULT_PAGE_TYPE;
   
   var type_str = type_obj.GetSResource( null );   /* String */
   
   var page_type =   /* int */
     ConvertStringToType( PageTypeTable, type_str, UNDEFINED_PAGE_TYPE,
                          UNDEFINED_PAGE_TYPE );
   if( page_type == UNDEFINED_PAGE_TYPE )
   {
      console.error( "Invalid PageType." );
      return DEFAULT_PAGE_TYPE;
   }
   return page_type;
}

//////////////////////////////////////////////////////////////////////////////
// Returns CommandType integer constant using CommandTypeTable.
//////////////////////////////////////////////////////////////////////////////
function GetCommandType( /* GlgObject */ command_obj )   /* int */
{
   var command_type_str = command_obj.GetSResource( "CommandType" ); /* String */
 
   return ConvertStringToType( CommandTypeTable, command_type_str,
                               UNDEFINED_COMMAND_TYPE, UNDEFINED_COMMAND_TYPE );
}

//////////////////////////////////////////////////////////////////////////////
// Returns PopupMenuType integer constant using PopupMenuTypeTable.
//////////////////////////////////////////////////////////////////////////////
function GetPopupMenuType( /* GlgObject */ command_obj )   /* int */
{
   var menu_type_str = command_obj.GetSResource( "MenuType" );  /* String */
      
   return ConvertStringToType( PopupMenuTypeTable, menu_type_str,
                               GLOBAL_POPUP_MENU, UNDEFINED_POPUP_MENU_TYPE );
}
   
//////////////////////////////////////////////////////////////////////////////
// Returns DialogType integer constant using DialogTypeTable.
//////////////////////////////////////////////////////////////////////////////
function GetDialogType( /* GlgObject */ command_obj )   /* int */
{
   var dialog_type_str = command_obj.GetSResource( "DialogType" );  /* String */
      
   return ConvertStringToType( DialogTypeTable, dialog_type_str,
                               GLOBAL_POPUP_DIALOG, UNDEFINED_DIALOG_TYPE );
}

//////////////////////////////////////////////////////////////////////////
function GetWidgetType( /* GlgObject */ glg_obj ) /* int */
{
   if( glg_obj == null )
   {
      alert( "Null widget" );
      return UNDEFINED_WIDGET_TYPE;
   }
   
   var widget_type_obj = glg_obj.GetResourceObject( "WidgetType" );
   if( widget_type_obj == null )
     return DEFAULT_WIDGET_TYPE;
   
   var widget_type_str = widget_type_obj.GetSResource( null );  /* String */
   var widget_type = ConvertStringToType( WidgetTypeTable, widget_type_str,
                                          UNDEFINED_WIDGET_TYPE, 
                                          UNDEFINED_WIDGET_TYPE );     
   if( widget_type == UNDEFINED_WIDGET_TYPE )
   {
      console.warn( "Undefined WidgetType, using default WidgetType." );
      return DEFAULT_WIDGET_TYPE;
   }
   return widget_type;
}

//////////////////////////////////////////////////////////////////////////////
function DialogAdjustForMobileDevices( /* GlgObject */ popup_vp )
{
   if( CoordScale == 1.0 )
     return;   // Desktop

   var chart = popup_vp.GetResourceObject( "Chart" );       /* GlgObject */
   if( chart != null )
   {
      /* Adjust chart offsets to fit chart labels on mobile devices with 
         canvas scaling.
      */      
      AdjustOffset( chart, "OffsetTop", 10. );
      AdjustOffset( chart, "OffsetLeft", 15. );
      AdjustOffset( chart, "OffsetBottom", -10. );

      chart.SetDResource( "XAxis/MajorInterval", -4 );
   }
}

//////////////////////////////////////////////////////////////////////////////
// Remap all object tags with the specified tag_name to use a new 
// tag_source. 
//////////////////////////////////////////////////////////////////////////////
function RemapNamedTags( /* GlgObject */ glg_object, /* String */ tag_name,
                         /* String */ tag_source )      /* int */
{
   var tag_obj;   /* GlgObject */
   var size;      /* int */

   /* Obtain a list of tags with TagName attribute matching the 
      specified tag_name.
   */
   var tag_list =    /* GlgObject */
     glg_object.GetTagObject( tag_name, /* by name */ true, 
                              /* list all tags */ false, 
                              /* multiple tags mode */ false, 
                              GLG.GlgTagType.DATA_TAG );
   if( tag_list == null )
     size = 0;
   else
     size = tag_list.GetSize();
   
   for( var i=0; i<size; ++i )
   {
      tag_obj = tag_list.GetElement( i );
         
      /* In simulation demo mode, assign new tag source unconditionally,
         including INIT ONLY tags. In live data mode, handle INIT ONLY tags 
         using real-time data as shown in the code below. 
      */
      if( RandomData )
      {
         AssignTagSource( tag_obj, tag_source );
         
         /* For demo purposes, initialize tags with TagName="Speed" 
            using a random data value. These tags are present for motor 
            ebjects in scada_aeration.g, as well as a gauge and slider 
            in scada_motor_info.g. 
         */
         if( tag_name == "Speed" )
           MainViewport.SetDTag( tag_source, GLG.Rand( 300.0, 1500.0 ), true );
            
         continue;
      }

      /* If tag is INIT_ONLY, initialize its value based on the current 
         data value for the given tag_source. Don't reassign TagSource 
         for this tag_obj, it is initilaized only once and will not be 
         subject to periodic updates.
      */
      var access_type =       /* int */
        Math.trunc( tag_obj.GetDResource( "TagAccessType" ) );

      if( access_type == GLG.GlgTagAccessType.INIT_ONLY_TAG )
      {
         var data_type =   /* int */
           Math.trunc( tag_obj.GetDResource( "DataType" ) );
         
         if( data_type == GLG.GlgDataType.D )
         {
            var d_value = MainViewport.GetDTag( tag_source );   /* double  */
            tag_obj.SetDResource( null, d_value );
         }
      }
      else
        AssignTagSource( tag_obj, tag_source );
   }
   
   return size;
}

//////////////////////////////////////////////////////////////////////////////
// Remap tags in the loaded drawing if needed.
// In demo mode, it assigns unset tag sources to be the same as 
// tag names. 
//////////////////////////////////////////////////////////////////////////////
function RemapTags( /* GlgObject */ drawing_vp )
{
   var tag_obj;    /* GlgObject */
   var
     tag_source,   /* String  */
     tag_name;     /* String  */

   /* Obtain a list of all tags defined in the drawing and remap them
      as needed.
   */
   var tag_list =   /* GlgObject */
     drawing_vp.CreateTagList( /* List all tags */ false );  
   if( tag_list == null )
     return;
   
   var size = tag_list.GetSize();
   if( size == 0 )
     return; // no tags found
   
   // Traverse the tag list and remap each tag as needed.
   for( var i=0; i<size; ++i )
   {
      tag_obj = tag_list.GetElement( i );
         
      /* Retrieve TagName and TagSource attributes from the
         tag object. TagSource represents the data source variable
         used to supply real-time data. This function demonstrates
         how to reassign the TagSource at run-time.
      */
      tag_name = tag_obj.GetSResource( "TagName" );
      tag_source = tag_obj.GetSResource( "TagSource" );
      
      HMIPage.RemapTagObject( tag_obj, tag_name, tag_source );
   }
}

//////////////////////////////////////////////////////////////////////////////
// Assigns new TagSource to the given tag object.
//////////////////////////////////////////////////////////////////////////////
function AssignTagSource( /* GlgObject */ tag_obj, /* String */ new_tag_source )
{
   tag_obj.SetSResource( "TagSource", new_tag_source );
} 

//////////////////////////////////////////////////////////////////////////////
// Utility function to validate the string. Returns true if the string
// is undefined (invalid).
//////////////////////////////////////////////////////////////////////////////
function IsUndefined( /* String */ str )   /* boolean */
{
   if( str == null || str.length == 0 || str == "unset" || str == "$unnamed" )
     return true;
   
   return false;
}      

//////////////////////////////////////////////////////////////////////////////
// Utility function to convert a string to a corresponding int value
// using a provided table.
//////////////////////////////////////////////////////////////////////////////
function ConvertStringToType( /* TypeRecord[] */ table, /* String */ type_str, 
                              /* int  */ empty_type,
                              /* int */ undefined_type )  /* int */
{
   if( type_str == null || type_str.length == 0 )
     return empty_type;
   
   for( var i=0; i < table.length; ++i )
     if( type_str == table[i].type_str )
       return table[i].type_int;
      
   return undefined_type;
}

//////////////////////////////////////////////////////////////////////////////
// Is used to populate predefined tables, such as CommandTypeTable,
// DialogTypeTable, etc.
//////////////////////////////////////////////////////////////////////////////
function TypeRecord( /* String */ str, /* int */ value )
{
   this.type_str = str;
   this.type_int = value;
}
   
//////////////////////////////////////////////////////////////////////////////
// ReadMenuConfig is used by GlgSCADAViewer, to read and parse a
// configuration file. The method stores information in the
// MenuArray, which is an array of classes of type GlgMenuRecord. 
// Return a number of read records. 
//////////////////////////////////////////////////////////////////////////////
function ReadMenuConfig( /* String */ config_data )  /* GlgMenuRecord[] */
{
   if( config_data == null || config_data.length == 0 )
     return null;
   
   var menu_array = [];   /* GlgMenuRecord[] */

   /* Split into text lines. */
   var line_array = config_data.match( /[^\r\n]+/g );   /* String[] */
   
   var num_read_records = 0;
   var num_lines = line_array.length;
   for( var i=0; i<num_lines; ++i )
   {
      var line = line_array[i];
      if( line.length == 0 || line.startsWith( '#' ) )
        continue;   // Skip comments and empty lines.
         
      // Split on commas to get an array of entries.
      var entries = line.split( ',' );
      var num_entries = entries.length;

      // 5th entry after comma may be "".
      if( num_entries != 4 && num_entries != 5 )
      {         
         console.error( "Missing entries, line: " + line );
         continue;
      }
         
      var   /* String */
        label_string, drawing_name, tooltip_string, drawing_title;
      
      for( var j=0; j<4; ++j )
      {
         // Remove heading and trailing spaces
         var entry = entries[j].trim();
            
         entry = HandleLineFeedChar( entry );
            
         // Fill table elements.
         switch( j )
         {
          case 0: 
            label_string = entry;
            break;
          case 1: 
            drawing_name = entry;
            break;
          case 2: 
            tooltip_string = entry;
            break;
          case 3: 
            drawing_title = entry;
            break;
         }
      }

      var menu_item = new GlgMenuRecord( label_string, drawing_name,
                                         tooltip_string, drawing_title );      
      menu_array.push( menu_item );
      ++num_read_records;	       
   }
      
   return menu_array;
}
   
//////////////////////////////////////////////////////////////////////////////
// Replace all occurences of "\n" with \n symbol.
// "\n" is used in the config file to create multi-line strings.
//////////////////////////////////////////////////////////////////////////////
function HandleLineFeedChar( /* String */ string )   /* String */
{
   return string.replace( /\\n/g, "\n" );
}

//////////////////////////////////////////////////////////////////////////////
function EmptyHMIPage()    // extends HMIPageBase
{
}

EmptyHMIPage.prototype = Object.create( HMIPageBase.prototype );
EmptyHMIPage.prototype.constructor = EmptyHMIPage;
EmptyHMIPage.prototype.GetUpdateInterval = function()
{
   /* Idle, but invoke periodically to check for a new update interval due to
      a page change.
   */
   return 250;
}

EmptyHMIPage.prototype.UpdateData = function()
{
   return true;  // Return true to disable tag updates for an empty page.
}

//////////////////////////////////////////////////////////////////////////////
// Changes drawing size while maintaining width/height aspect ratio.
//////////////////////////////////////////////////////////////////////////////
function SetDrawingSize( next_size )
{
   const ASPECT_RATIO = 900 / 700;

   // Settings for desktop displays.
   const MIN_WIDTH = 800;
   const MAX_WIDTH = 1000;
   const SCROLLBAR_WIDTH = 15;
   
   if( SetDrawingSize.size_index == undefined )   // first time
   {
      SetDrawingSize.size_index = 0;

      SetDrawingSize.small_sizes       = [ 1, 1.5,  2.,   2.5 ];
      SetDrawingSize.medium_sizes      = [ 1, 0.75, 1.25, 1.5 ];
      SetDrawingSize.large_sizes       = [ 1, 0.6,  1.25, 1.5 ];
      SetDrawingSize.num_sizes = SetDrawingSize.small_sizes.length;
      SetDrawingSize.is_mobile = ( screen.width <= 760 );

      window.addEventListener( "resize", ()=>{ SetDrawingSize( false ) } );
   }
   else if( next_size )
   {
      ++SetDrawingSize.size_index;
      SetDrawingSize.size_index %= SetDrawingSize.num_sizes;
   }

   var drawing_area = document.getElementById( "glg_area" );
   if( SetDrawingSize.is_mobile )
   {
      /* Mobile devices use constant device-width, adjust only the height 
         of the drawing to keep the aspect ratio.
      */
      drawing_area.style.height =
        "" + Math.trunc( drawing_area.clientWidth / ASPECT_RATIO ) + "px";
   }
   else   /* Desktop */
   {
      var span = document.body.clientWidth; 
      if( !SetDrawingSize.is_mobile )
        span -= SCROLLBAR_WIDTH;

      var start_width;
      if( span < MIN_WIDTH )
        start_width = MIN_WIDTH;
      else if( span > MAX_WIDTH )
        start_width = MAX_WIDTH;
      else
        start_width = span;

      var size_array;
      if( span < 600 )
        size_array = SetDrawingSize.small_sizes;
      else if( span < 800 )
        size_array = SetDrawingSize.medium_sizes;
      else
        size_array = SetDrawingSize.large_sizes;

      var size_coeff = size_array[ SetDrawingSize.size_index ];
      var width = Math.trunc( Math.max( start_width * size_coeff, MIN_WIDTH ) );
   
      drawing_area.style.width = "" + width + "px";
      drawing_area.style.height = "" + Math.trunc( width / ASPECT_RATIO ) + "px";
   }
}

//////////////////////////////////////////////////////////////////////////////
// Increases canvas resolution for mobile devices with HiDPI displays.
// Returns chosen coordinate scale factor.
//////////////////////////////////////////////////////////////////////////////
function SetCanvasResolution()
{
   // Set canvas resolution only for mobile devices with devicePixelRatio != 1.
   if( window.devicePixelRatio == 1. || !SetDrawingSize.is_mobile )
     return 1.0;   // Use coord scale = 1.0 for desktop.
   
   /* The first parameter defines canvas coordinate scaling with values 
      between 1 and devicePixelRatio. Values greater than 1 increase 
      canvas resolution and result in sharper rendering. The value of 
      devicePixelRatio may be used for very crisp rendering with very thin lines.

      Canvas scale > 1 makes text smaller, and the second parameter defines
      the text scaling factor used to increase text size.

      The third parameter defines the scaling factor that is used to
      scale down text in native widgets (such as native buttons, toggles, etc.)
      to match the scale of the drawing.
   */
   var coord_scale = 3.0;
   GLG.SetCanvasScale( coord_scale, 1.5, 0.5 );
   
   // Mobile devices use fixed device-width: disable Change Drawing Size button.
   var change_size_button = document.getElementById( "change_size" );
   if( change_size_button != null )
     change_size_button.parentNode.removeChild( change_size_button );

   return coord_scale;      // Chosen coord scale for mobile devices.
}

//////////////////////////////////////////////////////////////////////////////
// Adjusts the specified offset by a requested amount.
//////////////////////////////////////////////////////////////////////////////
function AdjustOffset( /* GlgObject */ object, /* String */ offset_name,
                       /* double */ adjustment )
{
   var value = object.GetDResource( offset_name );   /* double */
   value += adjustment;
   object.SetDResource( offset_name, value );
}

//////////////////////////////////////////////////////////////////////////////
// Scales the specified offset by a requested scale factor.
//////////////////////////////////////////////////////////////////////////////
function ScaleParameter( /* GlgObject */ object, /* String */ offset_name,
                         /* double */ scale )
{
   var value = object.GetDResource( offset_name );   /* double */
   value *= scale;
   object.SetDResource( offset_name, value );
}

//////////////////////////////////////////////////////////////////////////////
// Loads any assets required by the application and invokes the specified
// callback when done.
// Alternatively, the application's drawing can be loaded as an asset here
// as well, so that it starts loading without waiting for other assets to
// finish loading.
//////////////////////////////////////////////////////////////////////////////
function LoadAssets( callback )
{
   /* HTML5 doesn't provide a scrollbar input element (only a range input 
      html element is available). This application needs to load GLG scrollbars
      used for integrated chart scrolling. For each loaded scrollbar, the 
      AssetLoaded callback is invoked with the supplied data.
   */
   GLG.LoadWidgetFromURL( "scrollbar_h.g", null, AssetLoaded,
                          { name: "scrollbar_h", callback: callback } );
   GLG.LoadWidgetFromURL( "scrollbar_v.g", null, AssetLoaded,
                          { name: "scrollbar_v", callback: callback } );
   GLG.LoadAsset( ConfigFile, GLG.GlgHTTPRequestResponseType.TEXT,
                  AssetLoaded, { name: "config_file", callback: callback } );
   GLG.LoadObjectFromURL( "empty_drawing.g", null, AssetLoaded,
                          { name: "empty_drawing", callback: callback } );
}

//////////////////////////////////////////////////////////////////////////////
function AssetLoaded( loaded_obj, data, path )
{
   if( data.name == "scrollbar_h" )
   {
      if( loaded_obj != null )
        loaded_obj.SetResourceObject( "$config/GlgHScrollbar", loaded_obj );
   }
   else if( data.name == "scrollbar_v" )
   {
      if( loaded_obj != null )
        loaded_obj.SetResourceObject( "$config/GlgVScrollbar", loaded_obj );
   }
   else if( data.name == "config_file" )
   {
      ConfigFileData = loaded_obj;
   }
   else if( data.name == "empty_drawing" )
   {
      EmptyDrawing = loaded_obj;
   }
   else
     console.error( "Unexpected asset name" );

   /* Define an internal variable to keep the number of loaded assets. */
   if( AssetLoaded.num_loaded == undefined )
     AssetLoaded.num_loaded = 1;
   else
     ++AssetLoaded.num_loaded;

   // Invoke the callback after all assets have been loaded.
   if( AssetLoaded.num_loaded == 4 )
     data.callback();
}

//////////////////////////////////////////////////////////////////////////////
// Closes active popup dialog and Alarm dialog (if any) when the Esc key
// is pressed.
//////////////////////////////////////////////////////////////////////////////
function HandleKeyEvent( event )
{
   if( event.key == "Escape" )
   {
      CloseActivePopups( CLOSE_ALL );
      CloseAlarmDialog();
   }
}

//////////////////////////////////////////////////////////////////////////
function GetCurrTime()   /* double */
{
   return Date.now() / 1000;    // seconds
}   

//////////////////////////////////////////////////////////////////////////
// Is used to store information for an individual menu item,
// can be extended as needed.
//////////////////////////////////////////////////////////////////////////
function GlgMenuRecord( /* String */ label_string, /* String */ drawing_name,
                        /* String */ tooltip_string, /* String */ drawing_title )
{
   this.label_string = label_string;
   this.drawing_name = drawing_name;
   this.tooltip_string = tooltip_string;
   this.drawing_title = drawing_title;
}
   
//////////////////////////////////////////////////////////////////////////
// Is used to store information for a given GLG tag object,
// can be extended as needed.
//////////////////////////////////////////////////////////////////////
function GlgTagRecord( /* int */ data_type, /* String */ tag_source,
                       /* GlgObject */ tag_obj )
{
   this.data_type = data_type;
   this.tag_source = tag_source;
   this.tag_obj = tag_obj;
   
   /* plot_time_ep is TimeEntryPoint for a plot in a RealTimeChart, if any.
      If SUPPLY_PLOT_TIME_STAMP flag is true and the drawing contains charts,
      this object is non-null for tags attached to charts' ValueEntryPoint.
      It is set to null by default. It will be assigned in SetPlotTimeEP() 
      if needed.
   */
   this.plot_time_ep = null;   /* GlgObject */
}

//////////////////////////////////////////////////////////////////////////////
// Is used to store plot data points.
//////////////////////////////////////////////////////////////////////////////
function PlotDataPoint( /* double */ value, /* double */ time_stamp,
                        /* boolean */ value_valid )
{
   this.value = value;
   this.time_stamp = time_stamp;
   this.value_valid = value_valid;
}

///////////////////////////////////////////////////////////////////////      
// Is used to store data for animating the drawing.
///////////////////////////////////////////////////////////////////////      
function DataPoint()
{
   var value = null;        /* double or string */
   var time_stamp = null;   /* double */
}

//////////////////////////////////////////////////////////////////////////
// Used to store information for an individual alarm, can be extended
// as needed.
//////////////////////////////////////////////////////////////////////
function AlarmRecord()
{
   this.time = null;          /* double : Epoch time in seconds. */
   this.tag_source = null;    /* String */
   this.description = null;   /* String */

   /* If string_value is set to null, double_value will be displayed as alarm 
      value; otherwise string_value will be displayed.
   */
   this.string_value = null;  /* String  */
   this.double_value = 0;     /* double */

   this.status = 0;           /* int */   
   this.ack = false;          /* boolean */

   this.age = 0;             /* int: Used for demo alarm simulation only. */
}

//////////////////////////////////////////////////////////////////////////
// Is used to store information for each active popup dialog.
//////////////////////////////////////////////////////////////////////
function GlgActiveDialogRecord( /* int */ dialog_type, /* GlgObject */ dialog, 
                                /* GlgObject */ subwindow,
                                /* GlgObject */ popup_vp,
                                /* boolean */ isVisible )
{
   this.dialog_type = dialog_type;
   this.dialog = dialog;         // dialog object ID 
   this.subwindow = subwindow;   // Subwindow object inside a dialog.
   this.popup_vp = popup_vp;     // Viewport loaded into subwindow's DrawingArea.
   this.isVisible = isVisible;
}

///////////////////////////////////////////////////////////////////////
// Is used to store information for the active popup menu.
//////////////////////////////////////////////////////////////////////
function GlgActivePopupMenuRecord( /* int */ menu_type, /* GlgObject */ menu_obj,
                                   /* GlgObject */ subwindow,
                                   /* GlgObject */ menu_vp,
                                   /* GlgObject */ selected_obj,
                                   /* boolean */ isVisible )
{
   this.menu_type = menu_type;
   this.menu_obj = menu_obj;   // menu object ID
   this.subwindow = subwindow; // Subwindow object inside a dialog.
   this.menu_vp = menu_vp;     // Viewport loaded into subwindow's drawing_area.
   this.selected_obj = selected_obj;  // Symbol that trigerred popup menu.
   this.isVisible = isVisible;
}
