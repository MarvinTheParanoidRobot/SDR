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
// The demo uses simulated data. However, provided with a data source in a
// real application, it will display live data. The GlgViewer example in the
// examples_html5 directory of the GLG installation provides an example of
// using JSON for receiving data from a server.
//////////////////////////////////////////////////////////////////////////////

// Get a handle to the GLG Toolkit library.
var GLG = new GlgToolkit();

// Debugging aid: uncomment the next line to throw an exception on a GLG error.
//GLG.ThrowExceptionOnError( true, true, true );

// Set initial size of the drawing.
SetDrawingSize( false );

// Increase canvas resoultion for mobile devices.
var CoordScale = SetCanvasResolution();

// Add event listener to detect a touch device.
document.addEventListener( "touchstart", DetectTouchDevice );

// Add event listener to detect ESC key.
document.addEventListener( "keydown", HandleKeyEvent );

/* Loads misc. assets used by the program and invokes the LoadDrawing function
   when done to load the first page.
*/
LoadAssets( ()=>{ LoadDrawing( 0, null ) } );

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

// PopupType contants.
const
  UNDEFINED_POPUP_TYPE = -1,
  EMBEDDED_POPUP_DIALOG = 0,
  CUSTOM_DIALOG = 1,
  EMBEDDED_POPUP_MENU = 2,
  CUSTOM_POPUP_MENU = 3;

// Predefined tables, can be extended by the application as needed.
var DialogTypeTable =
  [ new TypeRecord( "Popup", EMBEDDED_POPUP_DIALOG ),
    new TypeRecord( "CustomDialog", CUSTOM_DIALOG )
  ];
   
var PopupMenuTypeTable =
  [ new TypeRecord( "PopupMenu", EMBEDDED_POPUP_MENU ),
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
   RTCHART_WIDGET = 1,
   POPUP_MENU2_WIDGET = 2,
   POPUP_MENU3_WIDGET = 3;

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

var MainViewport;  /* GlgObject : Viewport of the loaded drawing */
var EmptyDrawing;  /* GlgObject */

var StartDragging = false;   /* boolean */

/* Name of a viewport name used to display popup dialogs and popup menus. 
   If it's not present in the drawing, it will be added to the drawing 
   when needed.
*/
var POPUP_VIEWPORT_NAME = "PopupViewport";

// A template for the PopupViewport loaded as an asset.
var PopupVPTemplate = null;

// Store active popup information.
var ActivePopup = null;   /* GlgActivePopup */

// Title variables used for status display;
var LoadingTitle = null;
var DisplayedTitle = null;

var UpdateInterval = 50;     /* int : Update rate in msec for drawing animation,
                                will be adjusted on per-drawing basis. */
var FirstDrawing = true;

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

var TouchDevice = false;

// Is used by DataFeed to return data values.
var data_point = new DataPoint();      /* DataPoint */

CreateDataFeed();

//////////////////////////////////////////////////////////////////////////////
// Load a requested page based on the page index in the page table.
//////////////////////////////////////////////////////////////////////////////
function LoadDrawing( /* int */ page_index, /* GlgPageRecord */ page_record )
{
   var title, filename;
   
   AbortPendingPageLoads();    // Cancel any pending page load requests. 
   
   /* Use either page_request or supplied page_index. */
   if( page_record == null )
   {
      if( !PageIndexValid( page_index ) )
        return;
      
      filename = PageTable[ page_index ].drawing_name;
      title = PageTable[ page_index ].drawing_title;   
   }
   else
   {
      filename = page_record.drawing_name;
      title = page_record.drawing_title;
   }
   
   if( DisplayedTitle == title )
     return;    // This page is already displayed.

   LoadingTitle = title;
   DisplayStatus();
   
   /* Store a new load request in a global variable to be able to abort it
      if needed.
   */
   PageLoadRequest = { enabled: true, filename: filename, title: title };
   
   // Request to load the new drawing and invoke the callback when it's ready.
   GLG.LoadWidgetFromURL( filename, null, LoadDrawingCB, PageLoadRequest,
                          AbortLoad );
}

//////////////////////////////////////////////////////////////////////////////
function LoadDrawingCB( /* GlgObject */ loaded_drawing, /* Object */ user_data,
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
      alert( "Drawing loading failed: " + LoadingTitle );
      LoadingTitle = null;
      DisplayStatus();            // Erase loading message.
      return;
   }

   LoadingTitle = null;
   DisplayedTitle = load_request.title;
   DisplayStatus();    // Display new drawing title.

   AbortPendingPopupLoads();   // Cancel any pending popup load requests.

   DestroyDrawing();    // Destroy the currently displayed drawing.

   StartPage( loaded_drawing );

   /* Obtain a list of tags in the loaded drawing and build TagRecords array
      to be used for animation.
   */
   QueryTags( loaded_drawing );

   // Perform page-specific initialization after remapping tags.
   HMIPage.Ready();

   // Display the drawing in a web page.
   MainViewport.Update();

   // Start periodic updates when the first drawing is loaded.
   if( FirstDrawing )
   {
      FirstDrawing = false;
      
      // Update drawing with data and start periodic data updates.
      UpdateDrawing();

      // Process alarms and start periodic alarm updates.
      GetAlarmData( true );
   }
}

//////////////////////////////////////////////////////////////////////////////
function StartPage( /* GlgObject*/ drawing )
{
   MainViewport = drawing;   
   
   // Define the element in the HTML page to display the drawing in.
   drawing.SetParentElement( "glg_area" );

   // Disable viewport border to use the border of the glg_area.
   drawing.SetDResource( "LineWidth", 0 );
    
   /* Determines the type of the loaded page based on PageType property of the
      loaded drawing and sets HMIPage class that handles the page logic and user 
      interaction.
   */
   SetupHMIPage();

   // Perform before setup initialization common for all page types.
   InitBeforeSetup();
   
   // Perform page-specific initialization before setup.
   HMIPage.InitBeforeSetup();
   
   // Adjust the drawing for mobile devices if needed.
   HMIPage.AdjustForMobileDevices();

   // Setup object hierarchy in the drawing.
   MainViewport.SetupHierarchy();

   // Perform page-specific initialization after setup.
   HMIPage.InitAfterSetup();

   if( CoordScale != 1.0 )
     /* Increase pick resolution for the water treatment page to make it easier
        to select pump motors by touching on mobile devices.
     */
     MainViewport.SetDResource( "$config/GlgPickResolution",
                                PageType == AERATION_PAGE ? 30 : 5 );   
}  

//////////////////////////////////////////////////////////////////////////////
// Initialization before hierarchy setup.
//////////////////////////////////////////////////////////////////////////////
function InitBeforeSetup()
{
   // Add event listeners.
   MainViewport.AddListener( GLG.GlgCallbackType.INPUT_CB, InputCallback );
   MainViewport.AddListener( GLG.GlgCallbackType.TRACE_CB, TraceCallback );
   MainViewport.AddListener( GLG.GlgCallbackType.HIERARCHY_CB,
                             HierarchyCallback );

   /* Set "ProcessMouse" for the loaded viewport to enable custom commands,
      custom events and tooltips.
   */
   MainViewport.SetDResource( "ProcessMouse", 
                              ( GLG.GlgProcessMouseMask.MOUSE_CLICK | 
                                GLG.GlgProcessMouseMask.MOUSE_OVER_TOOLTIP ) );

   // If the drawing contains popup viewport, make it initially invisible.
   var popup_vp = MainViewport.GetResourceObject( POPUP_VIEWPORT_NAME );
   if( popup_vp != null )
   {
      // Reorder popup viewport to draw on top of other viewports.
      ReorderToFront( MainViewport, popup_vp );
      
      // Hide popup viewport.
      popup_vp.SetDResource( "Visibility", 0. );
   }

   // If the drawing contains a QuitButton, make it invisible.
   if( MainViewport.HasResourceObject( "QuitButton" ) )
     MainViewport.SetDResource( "QuitButton/Visibility", 0 );
}

//////////////////////////////////////////////////////////////////////////////
// Destroy currently loaded drawing, if any.
//////////////////////////////////////////////////////////////////////////////
function DestroyDrawing()
{
   if( MainViewport == null )
     return;
    
   // Close active popup, if any, and reset its viewport.
   CloseActivePopup();
   
   // Clear TagRecords array.
   DeleteTagRecords();
   
   // Destroy loaded drawing.
   MainViewport.ResetHierarchy();
   MainViewport = null;
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
// Displays the title of the current drawing and the one being loaded (if any).
//////////////////////////////////////////////////////////////////////////////
function DisplayStatus()
{
   var message;

   if( DisplayedTitle == null && LoadingTitle == null )
     message = "<br>";
   else
   {
      if( DisplayedTitle )
        message = "Displayed: <b>" + DisplayedTitle + "</b>";
      
      if( LoadingTitle )
      {
         if( DisplayedTitle )
           message += "&nbsp;&nbsp;&nbsp;&nbsp;";     
         else
           message = "";
         
         message += "Loading: <b>" + LoadingTitle + "</b>";
      }
   }

   document.getElementById( "status_div" ).innerHTML = message;
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
      
   /* Handle custom commands attached to objects in the drawing at 
      design time.
   */
   if( format == "Command" )
   {
      action_obj = message_obj.GetResourceObject( "ActionObject" ); 
      ProcessObjectCommand( viewport, selected_obj, action_obj );
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

      // Place custom code here to handle custom events based on event label. */
      //if( event_label == "MyEvent" )
      // ...
      // viewport.Update();
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
      
   // Handle window closing events.
   else if( format == "Window" && action == "DeleteWindow" )
   {
      if( selected_obj == null )
        return;

      // If the closing window is an active popup dialog, close active popup. 
      if( ActivePopup != null &&
          selected_obj.Equals( ActivePopup.popup_vp ) )
      {
         ClosePopupDialog( ActivePopup.popup_type );
         viewport.Update();
      }
   }

   // Handle Timer events, generated by objects with blinking dynamics.
   else if( format == "Timer" )  
     viewport.Update();
   
   /* Update drawing when a subdrawing, a subwindow or an image loads a new
      drawing or a new image.
   */
   else if( format == "TemplateLoad" || format == "ImageLoad" )
     MainViewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
function PerformZoom( /* String */ zoom_type )
{
   switch( zoom_type )
   {
    case 'i':
    case 'o':
      Zoom( MainViewport, zoom_type, 1.5 );
      MainViewport.Update();
      break;

    case 'l':
    case 'r':
    case 'u':
    case 'd':
      Zoom( MainViewport, zoom_type, 0.1 );
      MainViewport.Update();
      break;

    case 'n':
      Zoom( MainViewport, 'n', 0 );
      MainViewport.Update();
      break;

    case 't':
      StartDragging = true;
      Zoom( MainViewport, 't', 0.0 );
      break;
   }
}

//////////////////////////////////////////////////////////////////////////////
// Is used to handle low level events, such as obtaining coordinates of 
// the mouse click, or keyboard events. 
//////////////////////////////////////////////////////////////////////////////
function TraceCallback( /* GlgObject */ viewport, /* GlgTraceData */ trace_info )
{      
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
      break;
         
    case GLG.GlgHierarchyCallbackType.AFTER_SETUP_CB:
      var subwindow = info_data.object; /* GlgObject */
      var drawing_vp = info_data.subobject;   /* GlgObject */   
      
      /* Store the loaded drawing viewport in the ActivePopup, if any.
         Use GetReference API method to obtain a GlgObject instance
         that can be stored in a global variable.
         Finalize initialization of active popup after object hierachy
         is set up for the loaded popup drawing.
      */
      if( ActivePopup != null && ActivePopup.subwindow.Equals( subwindow ) &&
          ActivePopup.drawing_vp == null )
      {
         ActivePopup.drawing_vp = GLG.GetReference( drawing_vp );

         /* Finalize popup when the subwindow finishes initialization and
            is ready.
         */
         setTimeout( FinalizeActivePopup, 1 );
      }
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
   PageType = GetPageType( MainViewport );     // Get page type.

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
      HMIPage = new DefaultHMIPage( MainViewport );
      break;

    case PROCESS_PAGE:
      HMIPage = new ProcessPage( MainViewport );
      break;

    case RT_CHART_PAGE:
      HMIPage = new RTChartPage( MainViewport );
      break;

    case TEST_COMMANDS_PAGE:
      // Test page: always use demo data.
      DataFeed = DemoDataFeed;
                                            
      HMIPage = new DefaultHMIPage( MainViewport );
      break;

    default:
      /* New custom page: use live data if requested with the -live-data 
         command-line option, otherwise use simulated random data for 
         testing. RANDOM_DATA may be set to false to use live data by 
         default.
      */
      HMIPage = new DefaultHMIPage( MainViewport );
      break;
   }

   UpdateInterval = HMIPage.GetUpdateInterval();
   
   // True if DemoDataFeed is used for the current page.
   RandomData = ( DataFeed == DemoDataFeed );
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
      ShowAlarms( "System Alarms" );
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

//////////////////////////////////////////////////////////////////////   
// Process command "PopupDialog". The requested dialog is expected to be
// embedded into the currently loaded drawing.
//////////////////////////////////////////////////////////////////////////////
function DisplayPopupDialog( /* GlgObject */ command_vp, 
                             /* GlgObject */ selected_obj, 
                             /* GlgObject */ command_obj )
{ 
   /* Obtain DialogType. This example handles dialogs embedded in the
      loaded drawing. The application may extend the code 
      to handle various application specific dialog types.
   */
   var dialog_type = GetDialogType( command_obj );
    
   // Close currently active popup dialog, if any.
   ClosePopupDialog( dialog_type );

   switch( dialog_type )
   {
    case EMBEDDED_POPUP_DIALOG:
      // Retrieve dialog name.
      var dialog_res = command_obj.GetSResource( "DialogResource" );

      /* If undefined, use default popup viewport name to load and display
         a popup dialog drawing specified by the command.
      */
      if( IsUndefined( dialog_res ) ||
          !MainViewport.HasResourceObject( dialog_res ) )
        dialog_res = POPUP_VIEWPORT_NAME;

      DisplayEmbeddedPopup( EMBEDDED_POPUP_DIALOG, dialog_res, 
                            selected_obj, command_obj );
      break;
        
    case CUSTOM_DIALOG:
      // Add custom code here to handle custom dialog types.
      break;
    case UNDEFINED_POPUP_TYPE:
    default:
      break;
   }
    
   /* Get new data to fill the popup dialog with current values,
      without waiting for the timer to perform next data query.
   */ 
   UpdateData();
}
    
//////////////////////////////////////////////////////////////////////////////
// This method may be extended to handle closing of different dialog types. 
//////////////////////////////////////////////////////////////////////////////
function ClosePopupDialog( /* int */ dialog_type )
{
   if( ActivePopup == null )
     return; // Nothing to do.
    
   /* Close currently active popup dialog. It clears the current popup
      and rebuilds TagRecords array.
   */
   CloseActivePopup();
}

//////////////////////////////////////////////////////////////////////////////
function DisplayPopupMenu( /* GlgObject */ command_vp,
                           /* GlgObject */ selected_obj,
                           /* GlgObject */ command_obj )
{
   /* Obtain MenuType. This example handles only one menu type, where the menu
      is embedded in the loaded drawing. The application may extend the code 
      to handle various application specific dialog types.
   */
   var menu_type = GetPopupMenuType( command_obj );

   // Close currently active popup menu, if any.
   ClosePopupMenu( menu_type );

   switch( menu_type )
   {
    case EMBEDDED_POPUP_MENU:
      // Retrieve menu name.
      var menu_res = command_obj.GetSResource( "MenuResource" );

      /* If undefined, use default popup viewport name to load display
         a popup menu drawing specified by the command.
      */
      if( IsUndefined( menu_res ) ||
          !MainViewport.HasResourceObject( menu_res ) )
        menu_res = POPUP_VIEWPORT_NAME;

      DisplayEmbeddedPopup( EMBEDDED_POPUP_MENU, menu_res, selected_obj,
                            command_obj );
      break;

    case CUSTOM_POPUP_MENU:
      // Add code here to handle custom popup meny type.
      break;
       
    case UNDEFINED_POPUP_TYPE:
    default:
      break;
   }
}

//////////////////////////////////////////////////////////////////////////////
// This method camn be extended to handle closing if different menu types.
//////////////////////////////////////////////////////////////////////////////
function ClosePopupMenu( /* int */ menu_type )
{
   if( ActivePopup == null )
     return; // Nothing to do.
    
   /* Close currently active popup dialog. It clears the current popup
      and rebuilds TagRecords array.
   */
   CloseActivePopup();
}

//////////////////////////////////////////////////////////////////////////////
function CloseActivePopup()
{
   if( ActivePopup == null )
     return;

   if( ActivePopup.popup_vp == null ) // Shouldn't happen.
   {
      alert( "Null popup viewport." );
      ActivePopup = null;
      return;
   }

   switch( ActivePopup.popup_type )
   {
    case EMBEDDED_POPUP_DIALOG:
    case EMBEDDED_POPUP_MENU:
      // Destroy currently loaded popup drawing and load empty drawing.
      if( ActivePopup.subwindow != null )
        ActivePopup.subwindow.SetSResource( "SourcePath",
                                            "empty_drawing.g" );
      // Make popup invisible.
      ActivePopup.popup_vp.SetDResource( "Visibility", 0 );  
      ActivePopup.popup_vp.Update();
      break;

    case CUSTOM_DIALOG:
      // Add custom code here to handle custom dialog.
      break;

    case CUSTOM_POPUP_MENU:
      // Add custom code here to handle custom menu types.
      break;

    case UNDEFINED_POPUP_TYPE:
    default:
       alert( "Popup closing failed." );
       break;
   }

   // Reset ActivePopup object.
   ActivePopup = null;

   // Rebuild TagRecords array.
   QueryTags();
}

//////////////////////////////////////////////////////////////////////////////
// Handle popups embedded in the loaded drawing.
// popup_name specifies the viewport name in the drawing to be used to
// display a popu menu ot popup dialog drawing specified by the command.
// 
// With the Intermediate API, a popup viewport is expected to be saved 
// in each drawing that supports popup commands. 
// With the Extended API, the popup viewport can be added to the
// drawing on the fly at run-time.
//////////////////////////////////////////////////////////////////////////////
function DisplayEmbeddedPopup( /* int */ popup_type, /* String */ popup_name, 
                               /* GlgObject */ selected_obj, 
                               /* GlgObject */ command_obj )
{
   /* Retrieve DrawingFile from the command, which specifies the 
      GLG drawing to be displayed in the popup viewport.
   */
   var drawing_file = command_obj.GetSResource("DrawingFile");
   if( IsUndefined( drawing_file ) )
   {
      alert("Invalid DrawingFile, popup command failed.");
      return;
   }
    
   // Extract Title string from the command.
   var title = command_obj.GetSResource("Title" );

   /* Extract  ParamS and ParamD parameters from the command, which may
      define extra custom data.
   */
   var paramS = command_obj.GetSResource( "ParamS" );
   var paramD = command_obj.GetDResource( "ParamD" );

   // Store information for the active popup.
   ActivePopup = CreateActivePopup( popup_type, popup_name, drawing_file,
                                    selected_obj, title, paramS, paramD );    
   if( ActivePopup == null )
   {
      alert( "Popup command failed." );
      return;
   }

   // Load GLG drawing into the popup viewport.
   ActivePopup.subwindow.SetSResource( "SourcePath", drawing_file );

   // Make a popup dialog a floating dialog shell.
   var shell_type = ( ActivePopup.popup_type == EMBEDDED_POPUP_DIALOG ?
                      GLG.GlgShellType.DIALOG_SHELL :
                      GLG.GlgShellType.NO_TOP_SHELL );
   ActivePopup.popup_vp.SetDResourceIf( "ShellType", shell_type, true ); 
}

//////////////////////////////////////////////////////////////////////////////
function FinalizeActivePopup()
{ 
   if( ActivePopup == null || ActivePopup.drawing_vp == null ) 
   {
      alert( "Popup command failed." );
      CloseActivePopup();
      return;
   }

   /* Initialize tags for the popup based on the selected object.
      Store the number of remapped tags in ActivePopup for information
      and testing if needed. 
   */   
   ActivePopup.num_remapped_tags = 
     InitializePopup( ActivePopup.selected_obj, ActivePopup.drawing_vp );

   // Adjust popup parameters for mobile devices as needed.
   DialogAdjustForMobileDevices();

   /* Set popup viewport size, based on the size of the loaded popup drawing,
      and position it next to the selected object.
   */
   SetPopupSizeAndPosition();

   /* Rebuild TagRecords array, in case there are INPUT tags in the loaded 
      popup drawing.
   */
   QueryTags();

   // Display the popup.
   ActivePopup.popup_vp.SetDResource( "Visibility", 1 );
}

//////////////////////////////////////////////////////////////////////////////
// Initialize a popup object based on its WidgetType property, if any.
//////////////////////////////////////////////////////////////////////////////
function InitializePopup( /* GlgObject */ selected_obj,
                          /* GlgObject */ popup_obj )    /* int */
{
   if( selected_obj == null || popup_obj == null )
     return 0;

   // If title string is valid, display the title in the popup.
   if( !IsUndefined( ActivePopup.title ) )
   {
      if( ActivePopup.drawing_vp.HasResourceObject( "Title" ) )
        ActivePopup.drawing_vp.SetSResource( "Title", ActivePopup.title );
      
      if( IsFloatingDialog( ActivePopup.popup_vp ) )
        ActivePopup.popup_vp.SetSResource( "ScreenName", ActivePopup.title );
    }
 
   /* Set extra parameters ParamS and ParamD in the loaded popup drawing
      if these parameters are specified by the popup command, and if
      ParamS and ParamD resources are present in the popup drawing.
      In this example, ParamS is ued to specify a YAxis label in the 
      popup chart (rtchart_popup.g).
   */
   if( !IsUndefined( ActivePopup.paramS ) && 
       ActivePopup.drawing_vp.HasResourceObject( "ParamS" ) )
     ActivePopup.drawing_vp.SetSResource( "ParamS", ActivePopup.paramS );
   
   if( ActivePopup.paramD != null && 
       ActivePopup.drawing_vp.HasResourceObject( "ParamD" ) )
     ActivePopup.drawing_vp.SetDResource( "ParamD", ActivePopup.paramD );

   var num_remapped_tags = TransferTags( selected_obj, popup_obj, false );

   // Initialize active popup dialog based on WidgetType, if any.
   var widget_type = GetWidgetType( popup_obj ); /* String */
   switch( widget_type )
   {
      // Add custom cases here as needed.
    default: break;
   }

   return num_remapped_tags;
}

//////////////////////////////////////////////////////////////////////////////
// Create GlgActivePopup object and store information about the currently
// active popup.
//////////////////////////////////////////////////////////////////////////////
function CreateActivePopup( /* int */ popup_type, /* String */ popup_name, 
                            /* String */ drawing_file,
                            /* GlgObject */ selected_obj,
                            /* String */ title, /* String */ paramS, 
                            /* double */ paramD )          /* GlgActivePopup */
{
   if( MainViewport == null )
     return null;

   // Retrieve popup viewport, if any.
   var popup_vp = MainViewport.GetResourceObject( popup_name );
    
   if( popup_vp == null )
   {
      /* If popup viewport is not found in the loaded drawing, it can
         be added on the fly from PopupVPTemplate loaded as an asset.
         GLg Extended API is required to execute this functionality.
      */
      if( PopupVPTemplate == null )
      {
         alert( "Null popup viewport template." );
         return null;
      }
       
      /* Create a new PopupViewport by making a copy of the popup 
         tempate, and add the popup viewport to the drawing.
      */
      popup_vp = PopupVPTemplate.CloneObject( GLG.GlgCloneType.STRONG_CLONE );
      MainViewport.AddObjectToBottom( popup_vp );
   }

   // Retrieve DrawingArea subwindow from the popup viewport.
   var subwindow = popup_vp.GetResourceObject( "DrawingArea" );
   if( subwindow == null ) 
   {
      alert( "Can't find DrawingArea in the popup viewport." );
      return null;
   }

   var active_popup = new GlgActivePopup( popup_type, popup_vp, drawing_file,
                                          subwindow, selected_obj, title,
                                          paramS, paramD );
   return active_popup;
}

//////////////////////////////////////////////////////////////////////////////
// Returns true if the object is a viewport and it is a floating dialog.
//////////////////////////////////////////////////////////////////////////////
function IsFloatingDialog( /* GlgObject */ glg_obj )   /* boolean */
{
   var type = Math.trunc( glg_obj.GetDResource( "Type" ) ); /* GlgObjectType */
   if( type == GLG.GlgObjectType.VIEWPORT )
   {
      var shell_type = Math.trunc( glg_obj.GetDResource( "ShellType" ) ); 
      if( shell_type != GLG.GlgShellType.NO_TOP_SHELL ) 
        return true; // Floating dialog.
   }
   return false;
}

//////////////////////////////////////////////////////////////////////////////
// Set popup viewport size, based on the size of the loaded popup drawing,
// and position it next to the selected object.
//////////////////////////////////////////////////////////////////////////////
function SetPopupSizeAndPosition()
{
   if( ActivePopup == null || ActivePopup.drawing_vp == null ||
       ActivePopup.popup_vp == null || ActivePopup.selected_obj == null )
     return;

   var floating = IsFloatingDialog( ActivePopup.popup_vp );
   if( floating )
   {
      // Unset control points to use hints in screen coordinates.
      ActivePopup.popup_vp.SetGResource( "Point1", 0, 0, 0 );
      ActivePopup.popup_vp.SetGResource( "Point2", 0, 0, 0 );

      // Unset point transformations as well.
      ActivePopup.popup_vp.SetDResource( "Width", 0 );
      ActivePopup.popup_vp.SetDResource( "Height", 0 );
   }
   
   // Set popup viewport size, based on the size of the loaded popup drawing.
   SetPopupSize( floating );
    
   // Position the popup viewport next to the selected object.
   PositionPopup( floating );
}

//////////////////////////////////////////////////////////////////////////////
function SetPopupSize( /* boolean */ floating )
{
   // Get width and height from the drawing if defined, or use default values.
   if( ActivePopup.drawing_vp.HasResourceObject( "Width" ) )
     ActivePopup.width = ActivePopup.drawing_vp.GetDResource( "Width" );
    
   if( ActivePopup.drawing_vp.HasResourceObject( "Height" ) )
     ActivePopup.height = ActivePopup.drawing_vp.GetDResource( "Height" );
   

   if( CoordScale != 1.0 )
   {
      // Increase dialog and menu sizes for mobile devices with coord. scaling.
      if( ActivePopup.popup_type >= EMBEDDED_POPUP_MENU )
      {
         ActivePopup.width *= 3.0;
         ActivePopup.height *= 3.0;

      }
      else   // Dialog
      {
         ActivePopup.width *= 2.0;
         ActivePopup.height *= 2.0;
      }
   }
   
   if( floating )
   {
      ActivePopup.popup_vp.SetDResource( "Screen/WidthHint", ActivePopup.width );
      ActivePopup.popup_vp.SetDResource( "Screen/HeightHint", ActivePopup.height );
   }
   else
   {
      ActivePopup.popup_vp.SetDResource( "Width", ActivePopup.width );
      ActivePopup.popup_vp.SetDResource( "Height", ActivePopup.height );
   }
}

//////////////////////////////////////////////////////////////////////////////
// Position embedded poppup object next to the selected object.
//////////////////////////////////////////////////////////////////////////////
function PositionPopup( /* boolean */ floating )
{ 
   var x, y;                             /* double */
   var x_anchor, y_anchor;               /* int */
   var offset = 5.0 * CoordScale;        /* offset in pixels */
    
   // Obtain a parent viewport of the selected object.
   var selected_obj_vp =
     ActivePopup.selected_obj.GetParentViewport( true ); /* GlgObject */

   // Obtain a parent viewport of the popup viewport.
   var popup_parent_vp =
     ActivePopup.popup_vp.GetParentViewport( true );     /* GlgObject */
    
   // Obtain the object's bounding box in screen coordinates.
   var selected_obj_box = ActivePopup.selected_obj.GetBox();   /* GlgCube */
    
   var converted_box = /* GlgCube */
     GLG.CopyGlgCube( /*GlgCube*/ selected_obj_box );
    
   /* If the ActivePopup.popup_vp is located in a different viewport from the
      viewport of the selected object, convert screen coordinates of the 
      selected object box from the viewport of the selected object to the 
      viewport that contains the popup.
   */
   if( !selected_obj_vp.Equals( popup_parent_vp ) )
   {
      GLG.TranslatePointOrigin( selected_obj_vp, popup_parent_vp, 
                                converted_box.p1 );
      GLG.TranslatePointOrigin( selected_obj_vp, popup_parent_vp, 
                                converted_box.p2 );
   }

   // Obtain width and height in pixels of the popup parent viewport.
   var popup_parent_width = popup_parent_vp.GetDResource( "Screen/Width" );
   var popup_parent_height = popup_parent_vp.GetDResource( "Screen/Height" );
    
   var popup_width = ActivePopup.width;
   var popup_height = ActivePopup.height;
    
   /* Position the popup at the upper right or lower left corner of 
      the selected object, if possible. Otherwise (viewport is too small), 
      position it in the center of the viewport.
   */  
   if( converted_box.p2.x + popup_width + offset > popup_parent_width )
   {
      /* Outside of window right edge. Position right edge of the popup 
         to the left of the selected object. Always use HLEFT anchor 
         to simplify out-of-the-window check.
      */
      x =  converted_box.p1.x - offset - popup_width;
      x_anchor = GLG.GlgAnchoringType.HLEFT;
   }
   else 
   {
      // Position left edge of the popup to the right of the selected object. 
      x = converted_box.p2.x + offset; 
      x_anchor = GLG.GlgAnchoringType.HLEFT;
   }
    
   // Anchor is always HLEFT here to make checks simpler.
   if( x < 0 || x + popup_width > popup_parent_width )
   {
      // Not enough space: place in the center.
      x = popup_parent_width / 2.0;
      x_anchor = GLG.GlgAnchoringType.HCENTER;
   }
    
   if( converted_box.p1.y - popup_height - offset < 0.0 ) 
   {
      /* Outside of window top edge. Position the top edge
         of the popup below the selected object.
      */
      y =  converted_box.p2.y + offset;
      y_anchor = GLG.GlgAnchoringType.VTOP;
   }
   else 
   {
      y = converted_box.p1.y - offset - popup_height; 
      y_anchor = GLG.GlgAnchoringType.VTOP;
   }
    
   // Anchor is always GLG_VTOP here to make checks simpler.
   if( y < 0 || y + popup_height > popup_parent_height )
   {
      // Not enough space: place in the center.
      y = popup_parent_height / 2.0;
      y_anchor = GLG.GlgAnchoringType.HCENTER;
   }

   /* The check for floating dialog is needed only if there is a custom
      dialog inside a drawing, other than PopupViewport.
      Default EMDEDDED_DIALOG is positioned first, and then it is set to
      be floating dialog.
   */
   if( floating )
   {
      if( x_anchor == GLG.GlgAnchoringType.HCENTER )
        x -= popup_width / 2.;

      if( y_anchor == GLG.GlgAnchoringType.HCENTER )
        y -= popup_height / 2.;

      var glg_area = document.getElementById( "glg_area" );
      x += ( glg_area.offsetLeft * CoordScale ); 
      y += ( glg_area.offsetTop * CoordScale );

      ActivePopup.popup_vp.SetDResource( "Screen/XHint", x );
      ActivePopup.popup_vp.SetDResource( "Screen/YHint", y );
   }
   else    
   {
      /* Popup viewport size was changed by SetPopupSize().
         Make sure the popup viewport is set up before positioning it.
      */
      ActivePopup.popup_vp.SetupHierarchy();

     ActivePopup.popup_vp.PositionObject( GLG.GlgCoordType.SCREEN_COORD, 
                                          x_anchor | y_anchor, x, y, 0.0 );
   }
}

//////////////////////////////////////////////////////////////////////////////
// Process "GoTo" command. The command loads a new drawing specified
// by the DrawingFile parameter and replaces current drawing.
////////////////////////////////////////////////////////////////////// 
function GoTo( command_vp, selected_obj, command_obj )
{
    // Retrieve command parameters. 
    var drawing_file = command_obj.GetSResource( "DrawingFile" );
    var title = command_obj.GetSResource( "Title" );

    // If DrawingFile is not valid, abort the command.
    if( IsUndefined( drawing_file ) )
    {
        AppError( "GoTo Command failed: Invalid DrawingFile." );
        return;
    }

    // Load requested drawing, replacing current drawing.
    var new_page = new GlgPageRecord( drawing_file, title );
    LoadDrawing( 0, new_page ); 
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
// Returns index of the PageTable item with a matching drawing_name,
// if any. If not found, returns NO_SCREEN.
//////////////////////////////////////////////////////////////////////////////
function LookUpPageIndex( /* String */ drawing_name )   /* int */
{
   var page_record;   /* GlgPageRecord */

   for( var i=0; i<NumPages; ++i )
   {
      page_record = PageArray[i];
      if( drawing_name == page_record.drawing_name )
        return i;
   }
   
   return NO_SCREEN;
}
   
//////////////////////////////////////////////////////////////////////////////
function PageIndexValid( /* int */ page_index )
{
   if( page_index < NO_SCREEN || page_index >= NumPages )
   {
      alert( "Invalid page index." );
      return false;
   }
   return true;
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
                               EMBEDDED_POPUP_MENU, UNDEFINED_POPUP_TYPE );
}
   
//////////////////////////////////////////////////////////////////////////////
// Returns DialogType integer constant using DialogTypeTable.
//////////////////////////////////////////////////////////////////////////////
function GetDialogType( /* GlgObject */ command_obj )   /* int */
{
   var dialog_type_str = command_obj.GetSResource( "DialogType" );  /* String */
      
   return ConvertStringToType( DialogTypeTable, dialog_type_str,
                               EMBEDDED_POPUP_DIALOG, UNDEFINED_POPUP_TYPE );
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
     return DEFAULT_WIDGET_TYPE;

   return widget_type;
}

//////////////////////////////////////////////////////////////////////////////
function DialogAdjustForMobileDevices()
{
   if( CoordScale == 1.0 )
     return;   // Desktop

   // Adjust dialog for mobile devices with coord. scaling.
   if( ActivePopup.drawing_vp.HasResourceObject( "CoordScale" ) )
     ActivePopup.drawing_vp.SetDResource( "CoordScale", 2.0 );
   
   var chart = /* GlgObject */
     ActivePopup.drawing_vp.GetResourceObject( "Chart" );
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
   const ASPECT_RATIO = 900 / 750;

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

    /* Load a popup viewport object that will be used to display popup drawings
       for the PopupDialog or PopupMenu commands that may be attached to objects.
    */
    GLG.LoadWidgetFromURL( "popup_viewport.g", null, AssetLoaded,
                           { name: "popup_viewport", callback: callback } );
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
   else if( data.name == "popup_viewport" )
   {
      if( loaded_obj != null )
      {
         PopupVPTemplate = loaded_obj;
         PopupVPTemplate.SetSResource( "Name", POPUP_VIEWPORT_NAME );
         PopupVPTemplate.SetDResource( "Visibility", 0. );
      }
   }
   else
     console.error( "Unexpected asset name" );

   /* Define an internal variable to keep the number of loaded assets. */
   if( AssetLoaded.num_loaded == undefined )
     AssetLoaded.num_loaded = 1;
   else
     ++AssetLoaded.num_loaded;

   // Invoke the callback after all assets have been loaded.
   if( AssetLoaded.num_loaded == 3 )
     data.callback();
}

//////////////////////////////////////////////////////////////////////////
// Detect a touch device if it hasn't been detected yet.
//////////////////////////////////////////////////////////////////////////
function DetectTouchDevice()
{
   if( !TouchDevice && MainViewport != null ) 
   {
      TouchDevice = true;
      
      /* For touch devices, disable mouse button check and Ctrl key check 
         for Commands and Custom MouseClick events.
      */
      MainViewport.SetDResource( "$config/GlgDisableMouseButtonCheck", 1 );
      MainViewport.SetDResource( "$config/GlgDisableControlKeyCheck", 1 );

      document.removeEventListener( "touchstart", DetectTouchDevice );
   }
}

//////////////////////////////////////////////////////////////////////////////
// Closes active popup dialog and Alarm dialog (if any) when the Esc key
// is pressed.
//////////////////////////////////////////////////////////////////////////////
function HandleKeyEvent( event )
{
   if( event.key == "Escape" )
   {
      CloseActivePopup();
      CloseAlarmDialog();
   }
}

//////////////////////////////////////////////////////////////////////////
function GetCurrTime()   /* double */
{
   return Date.now() / 1000;    // seconds
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

//////////////////////////////////////////////////////////////////////////
// Is used to store information for the active popup.
//////////////////////////////////////////////////////////////////////////
function GlgActivePopup( /* int */ popup_type, /* GlgObject */ popup_vp,
                         /* String */ drawing_file,
                         /* GlgObject */ subwindow,
                         /* GlgObject */ selected_obj,
                         /* String */ title,
                         /* String */ paramS,
                         /* double */ paramD )
{
    this.popup_type = popup_type;
    this.popup_vp = popup_vp;
    this.drawing_file = drawing_file;
    this.subwindow = subwindow;
    this.selected_obj = selected_obj;
    this.title = title;
    this.paramS = paramS;
    this.paramD = paramD;

    this.drawing_vp = null;
    this.num_remapped_tags = 0;
    this.width = 300;   // Default width
    this.height = 300;  // Default height
}
