//////////////////////////////////////////////////////////////////////////////
// GLG Viewer example
//
// The demo is written in pure HTML5 and JavaScript. The source code of the
// demo uses the GLG Toolkit JavaScript Library supplied by the included
// Glg*.js and GlgToolkit*.js files.
//
// The library loads a GLG drawing and renders it on a web page, providing
// an API to animate the drawing with real-time data and handle user
// interaction with graphical objects in the drawing.
//
// The drawings are created using the GLG Graphics Builder, an interactive
// editor that allows to create grahical objects and define their dynamic
// behavior without any programming.
// 
// The viewer demonstrates how to:
// - animate a loaded drawing using tags defined in the drawing;
// - handle commands and custom events attached to objects in 
//   the GLG Builder.
//
// This example is written with GLG Intermediate API. 
//////////////////////////////////////////////////////////////////////////////

// Get a handle to the GLG Toolkit library.
var GLG = new GlgToolkit();

// Debugging aid: uncomment the next line to throw an exception on a GLG error.
GLG.ThrowExceptionOnError( true, true, true );

// Default drawing name and title.
const DEFAULT_DRAWING_NAME = "process_overview.g";
const DEFAULT_DRAWING_TITLE = "Process Overview";

// Stores the name of the currently loaded drawing.
var DrawingNameLoaded = null;

/* This object is created in LoadDrawing() to store information about 
   new drawing load request. Includes the following fields: 
   enabled, drawing_name, title.   
*/
var DrawingLoadRequest = null;

/* If set to true, simulated demo data will be used for animation.
   Set to false to enable live application data.
*/
const RANDOM_DATA = true;

// DataFeed object used for animation.
var DataFeed = null;  

// Top level viewport of the loaded drawing (GlgObject).
var MainViewport;

/* TouchDevice is set to true for a touch device, if a touchstart event is
   detected. Otherwise, it is set to false.
*/
var TouchDevice = false;

/* Coefficients for canvas resolution and text resolution. 
   These parameters will be adjusted for mobile devices with HiDPI displays
   in SetCanvasResolution() as needed.
*/
var CoordScale = 1;
var TextScale = 1;

// Size adjustment coefficients for mobile devices.
const MENU_COORD_SCALE = 2.0;      // Popup menu.
const DIALOG_COORD_SCALE = 1.2;    // Floating Popup dialog.

/* Set to true if ExtendedAPI is used for the project. It will allow to
   create PopupViewport on the fly withoutout a need to store it in
   each loaded drawing.
*/
var HAS_EXTENDED_API = true;

/* Popup viewport name stored in the drawing. If present, the viewport
   with this name will be used to display a popup menu or popup dialog 
   specified by the PopupMenu or PopupDialog command. The command may 
   override the popup name, using the command's resource
   MenuResource or DialogResource.
*/
var POPUP_VIEWPORT_NAME = "PopupViewport";

/* With the Intermediate API, the popup viewport is expected to be saved in the
   drawing. With the Extended API, the popup viewport may be added to the
   drawing on the fly at run-time, by loading the popup object from the GLG
   drawing file specified by POPUP_VIEWPORT_FILENAME.
*/
var POPUP_VIEWPORT_FILENAME = "popup_viewport.g";

/* A template for the PopupViewport loaded as an asset.
   The template is loaded from a file defined by POPUP_VIEWPORT_FILENAME
   and initialized in LoadAssets if HAS_EXTENDED_API=true;  
*/
var PopupVPTemplate = null;

// Add event listener to detect a touch device.
document.addEventListener( "touchstart", DetectTouchDevice );

// Add event listener to detect ESC key.
document.addEventListener( "keydown", HandleKeyEvent );

// Set initial size of the drawing.
SetDrawingSize( false );

// Increase canvas resolution for mobile devices.
SetCanvasResolution();

/* Load misc. assets such as GLG scrollbars. When assets are loaded, 
   LoadDrawing callback is invoked that loads a specified GLG drawing.
*/
LoadAssets( ()=>{ LoadDrawing( DEFAULT_DRAWING_NAME, DEFAULT_DRAWING_TITLE ) },
            null );

// Add DataFeed object which is used to animate the drawing with real-time data.
AddDataFeed();

////////////////////////////////////////////////////////////////////////////// 
// Load a GLG drawing from a file.
////////////////////////////////////////////////////////////////////////////// 
function LoadDrawing( /*String*/ filename, /*String*/ title )
{
    /* Don't reload the drawing if the filename is invalid or it matches 
       currently loaded drawing filename.
    */
    if( filename == null || filename == DrawingNameLoaded )
        return;

    // New drawing was requested, cancel any pending drawing load requests.
    AbortPendingLoadRequests();
    
    /* Create a new load request and store it in a global variable
       to be able to abort the request if needed.
       DrawingLoadRequest fields: enabled, drawing_name, title.
       The "enabled" field is used to cancel the current load request if 
       loading of another drawing was requested while the current request 
       has not been finished yet. This may happen if the user clicks on a 
       button to load one drawing and then clicks on another button to 
       load another drawing before the first drawing finished loading.
       The "drawing_name" ane the "title" fields are used to store 
       the drawing filename and the title of the load request.
    */
    DrawingLoadRequest =
        { enabled: true, drawing_name: filename, drawing_title: title };

    // Store title for the new drawing load request.
    LoadingTitle = title;
    
    // Display status info about the new drawing load request.
    DisplayStatus();

    /* Load a drawing from the specified drawing filename. 
       The LoadCB callback will be invoked when the drawing has been loaded.
    */
    GLG.LoadWidgetFromURL( /*String*/ filename, null, LoadCB, 
                           /*user data*/ DrawingLoadRequest );    
}

//////////////////////////////////////////////////////////////////////////////
// Load Callback, invoked after a GLG drawing finished loading.
// 'drawing' parameter provides an obejct ID of the loaded GLG viewport.
//////////////////////////////////////////////////////////////////////////////
function LoadCB( /*GlgObject*/ drawing, /*Object*/ user_data, 
                 /*String*/ path )
{
    var load_request = user_data;

    if( !load_request.enabled )
        /* This load request was aborted by requesting to load another drawing 
           before this load request has finished.
        */
     return;

    // Reset: we are done with this request.
    DrawingLoadRequest = null;
 
    if( drawing == null )
    {
       /* Stay on the previously loaded page, display status info and 
           generate an error.
        */
        AppAlert( "Drawing loading failed: " + LoadingTitle );
        LoadingTitle = null;
        DisplayStatus();
        return;
    }

    // Destroy currently loaded drawing, if any.
    if( MainViewport != null )
        DestroyDrawing();
    
    // Store drawing name of the currently loaded drawing.
    DrawingNameLoaded = load_request.drawing_name;

    // Define the element in the HTML page where to display the drawing.
    drawing.SetParentElement( "glg_area" );

    // Disable viewport border to use the border of the glg_area.
    drawing.SetDResource( "LineWidth", 0 );
    
    StartGlgViewer( drawing );

    // Update status info.
    LoadingTitle = null;
    DisplayedTitle = load_request.drawing_title;
    DisplayStatus();
}

/////////////////////////////////////////////////////////////////////////
// Variables and constants
/////////////////////////////////////////////////////////////////////////

// Enable/disable debuginng/diagnostics information.
const DEBUG = true;
const DEBUG_TIME_INTERVAL = false;

/* If set to false, TimerInterval used for updates will be adjused to obtain
   a targeted UPDATE_INTERVAL. Otherwise, a fixed timer interval is used for 
   updates.
*/
const USE_FIXED_TIMER_INTERVAL = true;

const UPDATE_INTERVAL = 100;              // Targeted data query interval, msec.
const MIN_IDLE_INTERVAL = 30;             // msec
const WAIT_INTERVAL = 30;                 // msec
const CHANGE_COEFF = 1/3;                 // Rate of time interval adjustment.

// Page type constants.
const
   UNDEFINED_PAGE_TYPE = -1,
   // Is used in the absence of the PageType property in the loaded drawing.
   DEFAULT_PAGE_TYPE = 0,
   RTCHART_PAGE = 1,
   TEST_COMMANDS_PAGE = 2;

/* Page type table, is used to determine the type of a loaded page based
   on the PageType property of the drawing. May be extended by an 
   application to add custom pages.
*/
var PageTypeTable = [
    new TypeRecord( "Default",              DEFAULT_PAGE_TYPE ),
    new TypeRecord( "RealTimeChart",        RTCHART_PAGE ),
    new TypeRecord( "TestCommands",         TEST_COMMANDS_PAGE )
];

// The type of the current page.
var PageType = UNDEFINED_PAGE_TYPE;   /* int */

// WidgetType constants.
const 
   UNDEFINED_WIDGET_TYPE = -1,
   DEFAULT_WIDGET_TYPE = 0,
   RTCHART_WIDGET = 1,
   POPUP_MENU2_WIDGET = 2;

/* Widget type table, is used to determine the type of a popup widget
   based on the "WidgetType" resource, if any.
*/
var WidgetTypeTable = [
    new TypeRecord( "RTChart",              RTCHART_WIDGET ),
    new TypeRecord( "PopupMenu2",           POPUP_MENU2_WIDGET )
];

// CommandType constants.
const  
   UNDEFINED_COMMAND_TYPE = -1,
   GOTO = 0,
   POPUP_DIALOG = 1,
   POPUP_MENU = 2,
   CLOSE_POPUP_DIALOG = 3,
   CLOSE_POPUP_MENU = 4,
   WRITE_VALUE = 5,
   WRITE_VALUE_FROM_WIDGET = 6;

var CommandTypeTable = [
      new TypeRecord( "GoTo",                 GOTO ),
      new TypeRecord( "PopupDialog",          POPUP_DIALOG ),
      new TypeRecord( "PopupMenu",            POPUP_MENU ),
      new TypeRecord( "ClosePopupDialog",     CLOSE_POPUP_DIALOG ),
      new TypeRecord( "ClosePopupMenu",       CLOSE_POPUP_MENU ),
      new TypeRecord( "WriteValue",           WRITE_VALUE ),
      new TypeRecord( "WriteValueFromWidget", WRITE_VALUE_FROM_WIDGET )
];

// PopupType contants.
const
  UNDEFINED_POPUP_TYPE = -1,
  EMBEDDED_POPUP_DIALOG = 0,
  CUSTOM_DIALOG = 1,
  EMBEDDED_POPUP_MENU = 2,
  CUSTOM_POPUP_MENU = 3;
   
// Predefined tables, can be extended by the application as needed.
var DialogTypeTable = [
    new TypeRecord( "Popup",                EMBEDDED_POPUP_DIALOG ),
    new TypeRecord( "CustomDialog",         CUSTOM_DIALOG )
];

var PopupMenuTypeTable = [
    new TypeRecord( "PopupMenu",            EMBEDDED_POPUP_MENU ),
    new TypeRecord( "CustomMenu",           CUSTOM_POPUP_MENU )
];

/* Update interval for the loaded drawing, may vary depending on the loaded 
   drawing (loaded GLG page).
*/
var UpdateInterval = UPDATE_INTERVAL;

// Initial data query interval.
var TimerInterval = UpdateInterval;

// Store active popup information.
var ActivePopup = null;   /* GlgActivePopup */

// Dynamically created array of tag records of type GlgTagRecord.
var TagRecords = null;    /* GlgTagRecord[] */

/* Contains a list of tag sources defined in the drawing, used by the 
   DataFeed object to query real-time data for all tags defined in the drawing.
   Each element is an object with a property 'tag_source' and 'data_type'.
*/
var DataTagList = null;   /* [] */

// Title variables used for status display.
var LoadingTitle = null;       // Title of a drawing being loaded.
var DisplayedTitle = null;     // Title of currently displayed drawing.

/* Flag indicating how to supply a time stamp for a RealTime Chart that
   may be embedded into the loaded drawing: if set to 1, the application 
   will supply a time stamp explicitly. Otherwise, a time stamp will be 
   supplied automatically by chart using current time. 
*/
var SUPPLY_PLOT_TIME_STAMP = false;     /* boolean */

// Start time for the data query, gets set in GetData().
var DataStartTime = 0;                /* double */

var UpdateDuration;                   /* double */
var FirstDrawing = true;              /* boolean */
var FirstDataQuery = true;            /* boolean */
var WaitForUpdate = false;            /* boolean */

//////////////////////////////////////////////////////////////////////////////
function StartGlgViewer( /*GlgObject*/ drawing )
{
    // Store loaded drawing.
    MainViewport = drawing;

    // Obtain PageType of the loaded drawing.
    PageType = GetPageType( MainViewport );     // Get page type.

    // Initialize the page based on PageType.
    switch( PageType )
    {
    case DEFAULT_PAGE_TYPE:
    case TEST_COMMANDS_PAGE:
        SetupDefaultPage();
        break;
    case RTCHART_PAGE:
        SetupRTChartPage( MainViewport );
        break;
    default: return;
    }
    
    /* Obtain a list of tags in the loaded drawing and build TagRecords array
       to be used for animation.
    */
    QueryTags();

    // Flag to indicate first data query for the loaded page.
    FirstDataQuery = true;

    /* Start an update timer for real-time data and a separate timer
       to query alarm data.
    */  
    if( FirstDrawing )
    {
        FirstDrawing = false;
        GetData();
    
        /* Get alarm data and highlight the Alarms button if there are any 
           unacknowledged alarms.
        */
        GetAlarmData();
    }

    // Display the drawing in a web page.
    MainViewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
function SetupDefaultPage()
{
    UpdateInterval = UPDATE_INTERVAL;

    // Adjust the drawing for mobile devices if needed.
    AdjustForMobileDevices( MainViewport );

    // Initialization before hierarchy setup.
    InitBeforeSetup();

    // Setup object hierarchy in the drawing.
    MainViewport.SetupHierarchy();

    // Initialization after hierarchy setup.
    InitAfterSetup();
}

//////////////////////////////////////////////////////////////////////////////
// Destroy currently loaded drawing, if any.
//////////////////////////////////////////////////////////////////////////////
function DestroyDrawing()
{
    if( MainViewport == null )
        return;
    
    // Close active popup, if any, and unload the popup's drawing.
    CloseActivePopup();

    // Clear TagRecords array.
    DeleteTagRecords();

    // Perform page specific cleanup based on PageType.
    switch( PageType )
    {
    case DEFAULT_PAGE_TYPE:
    case TEST_COMMANDS_PAGE:
        break;
    case RTCHART_PAGE:
        CleanupChartPage();
        break;
    default: return;
    }

    // Destroy loaded drawing.
    MainViewport.ResetHierarchy();
    MainViewport = null;
}

////////////////////////////////////////////////////////////////////////////// 
// Cancels any pending drawing load requests.
////////////////////////////////////////////////////////////////////////////// 
function AbortPendingLoadRequests()
{
   if( DrawingLoadRequest != null )
   {
      DrawingLoadRequest.enabled = false;
      DrawingLoadRequest = null;
   }
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
// Initialization after hierarchy setup.
//////////////////////////////////////////////////////////////////////////////
function InitAfterSetup()
{
    // Place custom application code as needed.
}

//////////////////////////////////////////////////////////////////////////////
function QueryTags()
{
    // Delete existing tag records from TagRecords array.
    DeleteTagRecords();

    // Build TagRecords array, a list of GLG tag records.
    if( !CreateTagRecords( MainViewport ) )
        return;
    
    /* Build DataTagList to be used by the DataFeed object to query real-time
       data from the server for all tags defined in the drawing.
       Include all tags, including INIT_ONLY and INPUT tags.
    */
    CreateDataTagList( /*include all tags*/ true );
}

//////////////////////////////////////////////////////////////////////////////
function AddDataFeed()
{
    if( RANDOM_DATA )
    {
        DataFeed = new DemoDataFeed();
        console.log( "Using random DemoDataFeed." );
    }
    else
    {
        DataFeed = new LiveDataFeed();
        console.log( "Using LiveDataFeed." );
    }
}

//////////////////////////////////////////////////////////////////////////////
// Create and populate TagRecords array, with elements of type
// GlgTagRecord.
//////////////////////////////////////////////////////////////////////////////
function CreateTagRecords( viewport )
{
    /* Retrieve a tag list from the drawing. Include tags with unique
       tag sources.
    */
    var tag_list = viewport.CreateTagList( /*unique tag sources*/ true );
    if( tag_list == null )
        return false;  // no tags found.
    
    var size = tag_list.GetSize();
    if( size == 0 )
        return false; // no tags found 
    
    /* Create an array of tag records by traversing the tag list and retrieving 
       information from each tag object in the list.
    */
    var 
        tag_obj,
        tag_source, 
        tag_name, 
        tag_comment, 
        data_type, 
        tag_access_type;

    TagRecords = [];
    for( var i=0; i<size; ++i )
    {
        tag_obj = tag_list.GetElement( i );
        tag_source = tag_obj.GetSResource( "TagSource" );
        tag_name = tag_obj.GetSResource( "TagName" );
        tag_comment = tag_obj.GetSResource( "TagComment" );
        data_type = Math.trunc( tag_obj.GetDResource( "DataType" ) );

        // Skip undefined tags.
        if( IsUndefined( tag_source ) )
            continue;
        
        if( RANDOM_DATA )
        {
            /* For demo purposes only, skip tags that have:
               - TagName contains "Test";
               - TagSource contains "Test";
               - TagComment contains "Test".
               Such tags may be present in the objects allowing to test commands
               such as WriteValue or WriteValueFromWidget.
            */
            if( !IsUndefined( tag_name ) && tag_name.indexOf( "Test" ) >= 0 )
              continue;
            if( tag_source.indexOf( "Test" ) >= 0 )
              continue;
            if( !IsUndefined( tag_comment ) && 
                tag_comment.indexOf( "Test" ) >= 0 )
              continue;
        }

        // Obtain tag access type.
        tag_access_type = Math.trunc( tag_obj.GetDResource( "TagAccessType" ) );

        switch( tag_access_type )
        {
         case GLG.GlgTagAccessType.OUTPUT_TAG:
             // Skip OUTPUT tags.
             continue;
             
         case GLG.GlgTagAccessType.INIT_ONLY_TAG:
         case GLG.GlgTagAccessType.INPUT_TAG:
         default: break;
        }

        /* Add a valid tag record to the list. "value" property will be assigned
           in StoreTagValue(), when a new data value is received. 
        */
        var tag_record = new GlgTagRecord( data_type, tag_name, tag_source, 
                                           tag_obj, tag_access_type );

        /* Set if_changed flag to true, so that the new value
           will be pushed into the graphics only if the value has changed.
           The flag will be ignored if a tag is attached to a plot entry point
           in a real-time chart, so that the chart will scroll even if the
           value hasn't changed.
        */
        tag_record.if_changed = true;

        TagRecords.push( tag_record );
    }

    Debug( "TagRecords array size: " + TagRecords.length ); 

    if( TagRecords.length == 0 )
    {
        TagRecords = null;
        return false;
    }

    /* If a drawing contains a chart, ValueEntryPoint of each plot 
       may have a tag to push data values using the common tag mechanism. 
       The time stamp for the corresponding TimeEntryPoint of the plot
       may be supplied either automatically by the chart 
       (SUPPLY_PLOT_TIME_STAMP=false), or the application may supply the
       time stamp explicitly for each data sample 
       (SUPPLY_PLOT_TIME_STAMP=true).
       
       If SUPPLY_PLOT_TIME_STAMP=true, a tag record in TagRecords array
       should store TimeEntryPoint (plot_time_ep) of a plot with a 
       corresponding tag source, so that the application can supply
       a time stamp to the plot's data sample. 

       If not found, plot_time_ep=null.

       Each tag record also stores ValueEntryPoint and ValidEntryPoint 
       for a plot in a chart. 
    */
    if( SUPPLY_PLOT_TIME_STAMP )
        StoreTimeEntryPoints( MainViewport );
    
    return true;
}

//////////////////////////////////////////////////////////////////////////////
function DeleteTagRecords()
{
    // Drop existing tag records.
    TagRecords = null;
    
    // Drop existing list of tags.
    DataTagList = null;
}

//////////////////////////////////////////////////////////////////////////////
function CreateDataTagList( /*boolean*/ all_tags )
{
    if( TagRecords == null )
    {
        DatatagList = null;
        return;
    }

    DataTagList = [];
    for( var i=0; i<TagRecords.length ; ++i )
    {
        if( all_tags ||
            TagRecords[i].tag_access_type == GLG.GlgTagAccessType.INPUT_TAG )
        {
            DataTagList.push( { tag_source: TagRecords[i].tag_source,
                                data_type:  TagRecords[i].data_type } );
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// Obtains real-time data for all tags defined in the drawing.
// A list of tags is stored in DataTagList array.
//////////////////////////////////////////////////////////////////////////////
function GetData()
{
    if( DataFeed == null || DataTagList == null )
        return;

    DataStartTime = new Date().getTime();

    /* Obtain new real-time data values for all tags in the DataTagList
       and invoke GetDataCB callback when done. Pass currently loaded
       drawing name to the callback.
    */
    DataFeed.ReadData( DataTagList, GetDataCB /*callback*/, 
                       DrawingNameLoaded /*user data*/ );
}

//////////////////////////////////////////////////////////////////////////
// Data query callback. It is invoked by the DataFeed after the new data 
// are received from the server.
//////////////////////////////////////////////////////////////////////////
function GetDataCB( new_data, drawing_name )
{
    /* Ignore new data if the drawing name has changed and a new drawing 
       has been loaded. This will stop further queries for the old drawing.
       The queries for the new drawing were started in StartGlgViewer
       when the new drawing has been loaded.
    */ 
    if( drawing_name != DrawingNameLoaded )
    {
        /* When a new drawing is loaded, restart the timer with a shorter
           time interval.
        */
        setTimeout( GetData, 30 );   
        return;
    }

    /* Query new data even if the previous query failed (new_data is null),
       to continue data updates even if there were intermittent network errors.
    */
    if( USE_FIXED_TIMER_INTERVAL )  // Use fixed targeted UpdateInterval
    {
        // Push new data to the graphics.
        PushData( new_data );

        /* Reset the FirstDataQuery flag. 
           Optional: For performance optimization, rebuild DataTagList 
           to exclude INIT_ONLY tags that have been intialized in the
           drawing by the first data query and will not be subject to
           subsequent data queries for the currently loaded drawing.
        */
        if( FirstDataQuery )
        {
            FirstDataQuery = false;

            /* Rebuild DataTagList to include only INPUT tags, and exclude
               INIT_ONLY tags.
            */
            CreateDataTagList( false );
        }

        // Send new data query request.
        setTimeout( GetData, UpdateInterval );
    }
    else   // Adjust TimerInterval to try obtain a targeted UpdateInterval
    {
        /* If next data is received and GetDataCB is invoked before updates have
           finished, set a timer to wait for the updates to finish before 
           sending new data query request. 
        */
        if( WaitForUpdate )
        {
            setTimeout( function(){ GetDataCB( new_data, drawing_name ); },
                        WAIT_INTERVAL );
            return;
        }

        // If data query finished before the update, wait for update to finish.
        WaitForUpdate = true;

        AdjustTimerInterval();

        if( DEBUG_TIME_INTERVAL )
            console.log( "   Adjusted time interval=" + Math.trunc( TimerInterval ) );
        
        /* Send new data query request right away to get new data asynchronously
           while the current data is being pushed to the graphics, without 
           waiting for the rendering to finish.
        */
        GetData();

        if( TimerInterval == 0 )
            /* Data query took longer than targeted UpdateInterval: process
               with no delay.
            */
            ProcessData( new_data );
        else
            // Delay next iteration to maintain requested update rate.
            setTimeout( function(){ ProcessData( new_data ); }, TimerInterval );
    }
}

//////////////////////////////////////////////////////////////////////////
// Used only if USE_FIXED_TIMER_INTERVAL = false.
//////////////////////////////////////////////////////////////////////////
function ProcessData( new_data )
{
    var update_start_time = new Date().getTime();

    // Push new data to the graphics.
    PushData( new_data );
        
    var update_finish_time = new Date().getTime();

    UpdateDuration = update_finish_time - update_start_time;
    WaitForUpdate = false;   // Update finished.
}

//////////////////////////////////////////////////////////////////////////////
// Push new data into graphics. For each tag in new_data array, find a
// tag record in TagRecords array with a matching tag_source, store
// the new value in the found tag record, and push new value into graphics.  
//////////////////////////////////////////////////////////////////////////////
function PushData( new_data )
{
    if( new_data == null || new_data.length == 0 )
    {
        AppInfo( "No new data received." );
        return;
    }

    if( TagRecords == null )
        return;

    var tag_record = null;
    for( var i=0; i<new_data.length; ++i )
    {
        /* Store a new value in the tag record with a matching tag source,
           if found.
        */
        tag_record = StoreTagValue( new_data[i].tag_source, 
                                    new_data[i].value,
                                    new_data[i].time_stamp );

        // Tag record not found.
        if( tag_record == null )
            continue;

        // Push new data value into graphics.
        switch( tag_record.data_type )
        {
         case GLG.GlgDataType.D: // D-type tag
            MainViewport.SetDTag( tag_record.tag_source, tag_record.value, 
                                  tag_record.if_changed );

            /* Push a time stamp to the TimeEntryPoint of a plot in 
               a real-time chart, if found.
            */ 
            if( tag_record.plot_time_ep != null && 
                tag_record.time_stamp != null )
                tag_record.plot_time_ep.SetDResource( null, 
                                                      tag_record.time_stamp );

            if( tag_record.plot_valid_ep != null )
                tag_record.plot_valid_ep.SetDResource( null,
                                           tag_record.value_valid ? 1. : 0. );

            break;
            
         case GLG.GlgDataType.S:
            MainViewport.SetSTag( tag_record.tag_source, tag_record.value, 
                                  tag_record.if_changed );
            break;
             
         case GLG.GlgDataType.G:      // Not used in this example.
         default: break;
        }
    }

    // Refresh display.
    MainViewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
function StoreTagValue( /*String*/ tag_source, /*double or String*/ value, 
                        /*double*/ time_stamp )
{
    // Find a tag record with a matching tag_source.
    var tag_record = LookupTagRecords( tag_source );

    if( tag_record == null ) // Not found.
        return null;

    /* If the drawing contains a chart and tag_record contains entry point 
       plot_valid_ep, check if the incoming value is valid.
    */
    var value_valid = true;
    if( tag_record.plot_valid_ep != null )
        value_valid = 
            DataFeed.IsValid( tag_source, tag_record.data_type, value );
        
    // Store new value, time stamp and the valid flag in the found tag_record.
    tag_record.value = value;
    tag_record.time_stamp = time_stamp;
    tag_record.value_valid = value_valid;

    // Return a valid tag record.
    return tag_record;
} 

//////////////////////////////////////////////////////////////////////////////
// Handle user interaction with the buttons, as well as process custom
// actions attached to objects in the drawing.
//////////////////////////////////////////////////////////////////////////////
function InputCallback( /*GlgObject*/ viewport, /*GlgObject*/ message_obj )
{
    var origin = message_obj.GetSResource( "Origin" );   /*String*/
    var format = message_obj.GetSResource( "Format" );   /*String*/
    var action = message_obj.GetSResource( "Action" );   /*String*/

    // Retrieve selected object ID from the message object.
    var selected_obj = message_obj.GetResourceObject( "Object" );

    // Handle custom commands attached to objects at design time.
    if( format == "Command" )
    {
        var action_obj = message_obj.GetResourceObject( "ActionObject" ); 
        ProcessObjectCommand( viewport, selected_obj, action_obj );
        viewport.Update();
    }
    
    // Handle custom events.
    else if( format == "CustomEvent" )
    {
        var event_label = message_obj.GetSResource( "EventLabel" );
        var action_data = null;
        
        if( event_label == null || event_label == "" )
            return;    // don't process events with empty EventLabel.
        
        var action_obj = message_obj.GetResourceObject( "ActionObject" ); 
        if( action_obj != null )
            action_data = action_obj.GetResourceObject( "ActionData" );
        
        /* Place custom code here to handle custom events as needed. */
        
        viewport.Update();
    }
    
    if( format == "Button" )
    {	 
        // Neither a push button or a toggle button.
        if( action !="Activate" && action != "ValueChanged" )
            return;
        
        if( action == "Activate" )  // Push button event.
        {
            // Place custom code here to handle push button events as needed.
        }
        else if( action == "ValueChanged" ) // Toggle button event.
        {
            var state = message_obj.GetDResource( "OnState" );
            
            // Place code here to handle events from a toggle button
            // and write a new value to a given tag_source.
            // DataFeed.WriteDValue( tag_source, state );
            
            if ( RANDOM_DATA && MainViewport.HasTagSource( "State" ) )
                DataFeed.WriteDValue( "State", state );
            else // Place custom code here
                ;
        }
        
        viewport.Update();  //format = "Button"
    }
    
    else if( format == "Timer" )   // Handles timer transformations.
        viewport.Update();

    // Window closing.
    else if( format == "Window" && action == "DeleteWindow" )
    {
        if( selected_obj == null )
            return;
        
        // If the closing window is an active popup dialog, close active popup. 
        if( ActivePopup != null && selected_obj.Equals( ActivePopup.popup_vp ) )
        {
            ClosePopupDialog( ActivePopup.popup_type );
            viewport.Update();
        }
    }

    /* Update drawing when a subdrawing, a subwindow or an image loads a new
       drawing or a new image.
    */
    else if( format == "TemplateLoad" || format == "ImageLoad" )
        MainViewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
// Trace callback is used to process native events of interest.
//////////////////////////////////////////////////////////////////////////////
function TraceCallback( /*GlgObject*/ viewport, /*GlgTaceData*/ trace_info )
{   
    var x, y; //Cursor position.
    var event_type = trace_info.event_type;

    switch( event_type )
    {
     case GLG.GlgEventType.MOUSE_PRESSED:
     case GLG.GlgEventType.MOUSE_MOVED:
        x = trace_info.mouse_x * CoordScale;
        y = trace_info.mouse_y * CoordScale;
       break;
        
     case GLG.GlgEventType.KEY_DOWN:
        /* Add custom code to handle various keys as needed when the
           input focus is in the GLG drawing.
        */
        var keyCode = trace_info.event.keyCode; /* int */
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////
// Hierarchy callback, added to the top level viewport and invoked when 
// when a new drawing is loaded into a Subwindow object. 
// For example, PopupDialog and PopupMenu viewports contain a Subwdinow 
// named DrawingArea where a popup drawing is loaded.
//////////////////////////////////////////////////////////////////////////////
function HierarchyCallback( /*GlgObject*/ viewport, 
                            /*GlgHierarchyData*/ info_data )
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
            AppAlert( "Drawing loading failed." ); 
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
        
        /* Store the loaded drawing viewport in the ActivePopup, if any, and
           finalize initialization of active popup.
        */
        if( ActivePopup != null && ActivePopup.subwindow.Equals( subwindow ) &&
            ActivePopup.drawing_vp == null )
        {
            /* Use GetReference API method to obtain a GlgObject instance
               that can be stored in a global variable.
            */
            ActivePopup.drawing_vp = GLG.GetReference( drawing_vp );

            /* Finalize popup initialization after subwindow initialization 
               is completed. It has to be done on a timer due to asynchronous
               nature of drawing loading.
            */
            setTimeout( FinalizeActivePopup, 1 );
        }
        break;
        
     default: break;
    }
}

//////////////////////////////////////////////////////////////////////////////
function ProcessObjectCommand( /*GlgObject*/ command_vp, 
                               /*GlgObject*/ selected_obj,
                               /*GlgObject*/ action_obj )
{
    if( selected_obj == null || action_obj == null )
        return;
    
    // Retrieve Command object.
    var command_obj = action_obj.GetResourceObject( "Command" ); /*GlgObject*/
    if( command_obj == null )
        return;
    
    /* Retrieve EventLabel. Add custom application code to handle application
       specific event labels.
    */
    var event_label = action_obj.GetSResource( "EventLabel" ); /*String*/
 
    var command_type = GetCommandType( command_obj );
    switch( command_type )
    {
    case POPUP_DIALOG:
        DisplayPopupDialog( command_vp, selected_obj, command_obj, event_label );
        break;
    case CLOSE_POPUP_DIALOG:
        var dialog_type = GetDialogType( command_obj );
        ClosePopupDialog( dialog_type );
        break;
    case POPUP_MENU:
        DisplayPopupMenu( command_vp, selected_obj, command_obj, event_label );
        break;
    case CLOSE_POPUP_MENU:
        var menu_type = GetPopupMenuType( command_obj );
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
        AppAlert( "Command failed: Undefined CommandType." );
        break;
    }
}

//////////////////////////////////////////////////////////////////////   
// Process command "PopupDialog". The requested dialog is expected to be
// embedded into the currently loaded drawing.
//////////////////////////////////////////////////////////////////////////////
function DisplayPopupDialog( /*GlgObject*/ command_vp, 
                             /*GlgObject*/ selected_obj, 
                             /*GlgObject*/ command_obj,
                             /*String*/ event_label )
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
        if( IsUndefined( dialog_res ) )
            dialog_res = POPUP_VIEWPORT_NAME;
        
        DisplayEmbeddedPopup( EMBEDDED_POPUP_DIALOG, dialog_res, 
                              selected_obj, command_obj, event_label );
        break;
        
     case CUSTOM_DIALOG:
        // Add custom code here to handle custom dialog types.
        break;
     case UNDEFINED_POPUP_TYPE:
     default:
        break;
    }
}
    
//////////////////////////////////////////////////////////////////////////////
// This method may be extended to handle closing of different dialog types. 
//////////////////////////////////////////////////////////////////////////////
function ClosePopupDialog( /*int*/ dialog_type )
{
    if( ActivePopup == null )
        return; // Nothing to do.
    
    /* Close currently active popup dialog. It clears the current popup
       and rebuilds TagRecords array.
    */
    CloseActivePopup();
}

//////////////////////////////////////////////////////////////////////////////
function DisplayPopupMenu( /*GlgObject*/ command_vp, 
                           /*GlgObject*/ selected_obj,
                           /*GlgObject*/ command_obj,
                           /*String*/    event_label )
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
        if( IsUndefined( menu_res ) )
            menu_res = POPUP_VIEWPORT_NAME;

        DisplayEmbeddedPopup( EMBEDDED_POPUP_MENU, menu_res, 
                              selected_obj, command_obj, event_label );
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
function ClosePopupMenu( /*int*/ menu_type )
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
        AppAlert( "Null popup viewport." );
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
        AppAlert( "Popup closing failed." );
        break;
    }

    // Reset ActivePopup object.
    ActivePopup = null;

    // Rebuild TagRecords array.
    QueryTags();
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
        AppAlert( "GoTo Command failed: Invalid DrawingFile." );
        return;
    }

    // Load requested drawing, replacing current drawing.
    LoadDrawing( drawing_file, title );
}

//////////////////////////////////////////////////////////////////////    
// Process command "WriteValue". The command writes a new value specified
// by the Value parameter into the tag in the back-end system
// specified by the OutputTagHolder.
//////////////////////////////////////////////////////////////////////////////
function WriteValue( command_vp, selected_obj, command_obj )
{
    // Retrieve tag source to write data to.
    var tag_source =  command_obj.GetSResource( "OutputTagHolder/TagSource" );
    
    // Validate.
    if( IsUndefined( tag_source ) )
    {
        AppAlert( "WriteValue Command failed: Invalid TagSource." );
        return;
    }
      
    // Retrieve the new value to be written to the specified output tag.
    var value = command_obj.GetDResource( "Value" );
    
    /* Place custom code here as needed, to validate the value specified
       in the command.
    */
    
    /* Write new value to the specified tag source. */
    DataFeed.WriteDValue( tag_source, value );
}

//////////////////////////////////////////////////////////////////////////////
// Process command "WriteValueFromWidget". The command allows writing
// a new value into the tag in the back-end system using an input
// widget, such as a toggle or a spinner.
//////////////////////////////////////////////////////////////////////////////
function WriteValueFromInputWidget( command_vp, widget, command_obj )
{
    /* Retrieve input widget's resource name that stores the new value 
       when the user uses the input widget to change the value.
       For a spinner, the resource name is "Value"; for a toggle button, 
       it is "OnState".
    */
    var widget_value_res = command_obj.GetSResource( "ValueResource" );
    
    /* Obtain object ID of the input resource/tag object we read the 
       new value from.
    */
    var input_tag_obj = widget.GetResourceObject( widget_value_res );
    if( input_tag_obj == null )
        return;
      
    // Obtain object ID of the write tag object (output tag).
    var output_tag_obj = command_obj.GetResourceObject( "OutputTagHolder" ); 
    if( output_tag_obj == null )
        return;
    
    /* Obtain TagSource from the write tag. */
    var output_tag_source = output_tag_obj.GetSResource( "TagSource" );
    
    /* Validate. */
    if( IsUndefined( output_tag_source ) )
    {
        AppAlert( "Write Command failed: Invalid Output TagSource." );
        return;
    }
    
    // Retrieve new value from the input widget.
    var value = input_tag_obj.GetDResource( null );
    
    // Write the new value retrieved from the widget to the output tag.
    DataFeed.WriteDValue( output_tag_source, value );
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
function DisplayEmbeddedPopup( /*int*/ popup_type, /*String*/ popup_name, 
                               /*GlgObject*/ selected_obj, 
                               /*GlgObject*/ command_obj,
                               /*String*/ event_label )
{
    /* Retrieve DrawingFile from the command, which specifies the 
       GLG drawing to be displayed in the popup viewport.
    */
    var drawing_file = command_obj.GetSResource( "DrawingFile" )
    if( IsUndefined( drawing_file ) )
    {
        AppAlert( "Invalid DrawingFile, popup command failed." );
        return;
    }
    
    // Extract Title string from the command.
    var title = command_obj.GetSResource( "Title" );

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
        AppAlert( "Popup command failed." );
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
// Finish popup initialization. This method is invoked in HierarchyCallback
// after hierarchy is set up for the loaded popup drawing.
//////////////////////////////////////////////////////////////////////////////
function FinalizeActivePopup()
{ 
    if( ActivePopup == null || ActivePopup.drawing_vp == null ) 
    {
        AppAlert( "Popup command failed." );
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
    AdjustForMobileDevices( ActivePopup.drawing_vp );
    
    /* Set popup viewport size, based on the size of the loaded popup drawing,
       and position it next to the selected object.
    */
    SetPopupSizeAndPosition();

    /* Rebuild TagRecords array, in case there are INPUT tags in the loaded 
       popup drawing.
    */
    QueryTags();

    /* Make the new popup visible. Update() call to refresh display will be 
       invoked in the InputCallback on TemplateLoad message generated 
       after the new drawing is loaded into a subwindow and is set up.
    */
    ActivePopup.popup_vp.SetDResource( "Visibility", 1 );
}

//////////////////////////////////////////////////////////////////////////////
// Create GlgActivePopup object and store information about the currently
// active popup.
//////////////////////////////////////////////////////////////////////////////
function CreateActivePopup( /*int*/ popup_type, /*String*/ popup_name, 
                            /*String*/ drawing_file, /*GlgObject*/ selected_obj,
                            /*String*/ title, /*String*/ paramS, 
                            /*double*/ paramD )           /* GlgActivePopup */
{
    if( MainViewport == null )
        return null;

    // Retrieve popup viewport, if any.
    var popup_vp = MainViewport.GetResourceObject( popup_name );

    if( popup_vp == null )
    {
        /* A popup viewport has a custom name different from default, but
           an object with this name is not found in the loaded drawing: 
           generate an error and return.
        */
        if( popup_name != POPUP_VIEWPORT_NAME )
        {
            AppAlert( "Can't find embedded dialog or menu " + popup_name );
            return null;
        }
        
        /* A popup viewport with a default name is not present in the loaded 
           drawing: add it on the fly from PopupVPTemplate. 
           Extended API is required to execute this functionality.
        */
        if( HAS_EXTENDED_API )
        {
            if( PopupVPTemplate == null )
            {
                AppAlert( "Null popup viewport template." );
                return null;
            }
            
            /* Create a new PopupViewport by making a copy of the popup 
               tempate, and add the popup viewport to the drawing.
            */
            popup_vp = 
                PopupVPTemplate.CloneObject( GLG.GlgCloneType.STRONG_CLONE );
            MainViewport.AddObjectToBottom( popup_vp );
        }
        else
        {
            AppAlert( "Can't find popup viewport " + popup_name );
            return null;
        }
    }

    // Retrieve DrawingArea subwindow from the popup viewport.
    var subwindow = popup_vp.GetResourceObject( "DrawingArea" );
    if( subwindow == null ) 
    {
        AppAlert( "Can't find DrawingArea in the popup viewport." );
        return null;
    }

    // Create a new ActivePopup object.
    var active_popup = new GlgActivePopup( popup_type, popup_vp, drawing_file, 
                                           subwindow, selected_obj, title,
                                           paramS, paramD );
    return active_popup;
}

//////////////////////////////////////////////////////////////////////////////
// If there is a Chart object in the drawing, and a plot in the chart
// has a valid tag for the ValueEnterPoint, the tag record for that tag
// should store object IDs for the plot's TimeEntryPoint and ValidEntryPoint.
// It enables the application to push a time stamp and validity flag
// for each data point in a chart, if needed.
//////////////////////////////////////////////////////////////////////////////
function StoreTimeEntryPoints( /*GlgObject*/ viewport )
{
    if( TagRecords == null )
        return;
    
    // Obtain a list of all tags, including non-unique tag sources.
    var tag_list =    /* GlgObject */
        viewport.CreateTagList( /* List all tags */ false ); 
    
    if( tag_list == null )
        return;
    
    var size = tag_list.GetSize();
    if( size == 0 )
     return; /* no tags found */
    
    /* For each tag in the list, check if there is a chart object in the
       drawing that has a plot with a matching TagSource assigned to the 
       plot's ValueEntryPoint. If found, obtain plot's TimeEntryPoint, 
       ValidEntryPoint and ValueEntryPoint, and store their objects IDs 
       in the tag record with a matching tag_source. Process only tags
       of type INPUT_TAG.
    */
    for( var i=0; i<size; ++i )
    {
        var tag_obj = tag_list.GetElement( i );   /* GlgObject */
        
        /* Check if the tag belongs to a chart's Entry Point.
           If yes, proceed with finding the PLOT object the tag belongs to.
           Otherwise, skip this tag objectpwd
.
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
        
        /* We are interested only in INPUT tags that are subject
           to periodic data updates.
        */
        var access_type = Math.trunc( tag_obj.GetDResource( "TagAccessType" ) );
        if( access_type != GLG.GlgTagAccessType.INPUT_TAG )
            continue;

        /* Find a plot object in a RealTimeChart (if any) with a matching 
           TagSource assigned for the plot's ValueEntryPoint.
           It is assumed that there is ONLY ONE plot in the drawing 
           with a given TagSource. 
        */
        var plot = FindMatchingPlot( tag_obj );   /* GlgObject */
        if( plot == null )
            continue;   /* There is no plot for this tag source. */
        
        StorePlotEntryPoints( plot, tag_source, false );
    }
}

//////////////////////////////////////////////////////////////////////////////
// For a given tag object, find a parent plot object (PLOT object type). 
// If found, return the plot object. It is assumed that there is ONLY ONE 
// plot in the drawing with a given TagSource. 
//////////////////////////////////////////////////////////////////////////////
function FindMatchingPlot( /*GlgObject*/ tag_obj )
{
    /* Traverse only if the tag is set up. It will not be set up
       a tag is connected to a subdrawing which has not been loaded yet.
       It is assumed that if a drawing contains a chart, the chart is
       not inside a subdrawing. Otherwise, a special handling would be
       required.
    */
    if( tag_obj.GetDResource( "HISetup" ) == 0 )
        return null;

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
        return null;
    else
        return rval.found_object;
}

//////////////////////////////////////////////////////////////////////////////
// Store object IDs for the plot's ValueEntryPoint, TimeEntryPoint and
// ValueEntryPoint in a tag record that has a specified tag_source.
//////////////////////////////////////////////////////////////////////////////
function StorePlotEntryPoints( /*GlgObject*/ plot, /*String*/ tag_source )
{
    if( plot == null || IsUndefined( tag_source ) )
        return;

    // Find a tag record in TagRecordArray with a matching tag_source.
    var tag_record = LookupTagRecords( tag_source );
    if( tag_record == null ) /* shouldn't happen */
    {
        AppAlert( "No matching tag record, TimeEntryPoint not stored." ); 
        return;
    }
    
    // Found matching tag record, store plot's entry points in the tag record. 
    tag_record.plot_value_ep = plot.GetResourceObject( "ValueEntryPoint" );
    tag_record.plot_time_ep = plot.GetResourceObject( "TimeEntryPoint" );
    tag_record.plot_valid_ep = plot.GetResourceObject( "ValidEntryPoint" );
}

//////////////////////////////////////////////////////////////////////////////
// Find a tag record in TagRecords array with a tag source matching
// the specified tag_source.
//////////////////////////////////////////////////////////////////////////////
function LookupTagRecords( /*String*/ tag_source )
{
    for( var i=0; i<TagRecords.length; ++i )
    {
        if( TagRecords[i].tag_source == tag_source )
            return TagRecords[i];
    }

    return null; // not found.
}

//////////////////////////////////////////////////////////////////////////////
// Initialize a popup object based on its WidgetType property, if any.
//////////////////////////////////////////////////////////////////////////////
function InitializePopup( /*GlgObject*/ selected_obj, /*GlgObject*/ popup_obj )
                        /* int */
{
    if( selected_obj == null || popup_obj == null )
        return 0;

    // If title string is valid, display the title in the popup.
    if( !IsUndefined( ActivePopup.title ) )
    {
        if( ActivePopup.drawing_vp.HasResourceObject( "TitleString" ) )
            ActivePopup.drawing_vp.SetSResource( "TitleString", 
                                                 ActivePopup.title );
        
        if( IsFloatingDialog( ActivePopup.popup_vp ) )
            ActivePopup.popup_vp.SetSResource( "ScreenName", 
                                               ActivePopup.title );
    }
 
    /* Set extra parameters ParamS and ParamD in the loaded popup drawing
       if these parameters are specified by the popup command, and if
       ParamS and ParamD resources are present in the popup drawing.
       In this example, ParamS is used to specify YAxis label in the 
       popup chart (rtchart_popup.g).
    */
    if( !IsUndefined( ActivePopup.paramS ) && 
        ActivePopup.drawing_vp.HasResourceObject( "ParamS" ) )
        ActivePopup.drawing_vp.SetSResource( "ParamS", ActivePopup.paramS );

    if( ActivePopup.paramD != null && 
        ActivePopup.drawing_vp.HasResourceObject( "ParamD" ) )
        ActivePopup.drawing_vp.SetDResource( "ParamD", ActivePopup.paramD );

    /* Transfer tags from the selected object to the popup object,
       (unset_tags parameter = false).
    */
    var num_remapped_tags = TransferTags( selected_obj, popup_obj, false );                
    // Initialize active popup dialog based on WidgetType, if any.
    var widget_type = GetWidgetType( popup_obj ); /*String*/
    switch( widget_type )
    {
    case RTCHART_WIDGET:
        var chart = popup_obj.GetResourceObject( "Chart" );    /* GlgObject */
        if( chart != null )
            InitChartWidget( selected_obj, chart );  
        else
            AppAlert( "Cant' find Chart object, chart initialization failed." );
        break;

    case POPUP_MENU2_WIDGET:
        // Add custom code here as needed.
        break;
    default:
        break;
    }

    return num_remapped_tags;
}

//////////////////////////////////////////////////////////////////////////////
// Initilizes the chart widget. Assign chart's tags by transferring 
// tag sources from the selected object to the chart object for those 
// tags that have a matching TagName in the selected object and 
// the chart's plots. If the chart has valid tag sources already assigned
// in the chart drawing, leave them intact. 
//////////////////////////////////////////////////////////////////////////////
function InitChartWidget( /*GlgObject*/ selected_obj, /*GlgObject*/ chart )
{ 
    var PREFILL_CHART_POPUP = true;

    /* Obtain historical data for the number of seconds defined by the
       chart Span.
    */
    if( PREFILL_CHART_POPUP )
    {
        var prefill_span =  /* int */
        Math.trunc( chart.GetDResource( "XAxis/Span" ) );
        
        FillChartHistory( chart, prefill_span * 10 );
    } 
}

//////////////////////////////////////////////////////////////////////////////
// Transfer tag sources for tags with a matching TagName from the
// selected object to the specified viewport. Returns a total number of 
// remapped tags. If unset_tags=true, set tag sources to "unset".
//////////////////////////////////////////////////////////////////////////////
function TransferTags( /*GlgObject*/ selected_obj, /*GlgObject*/ viewport, 
                       /*boolean*/ unset_tags )     /* int */
{
    // Obtain a list of tags defined in the selected object.
    var tag_list = selected_obj.CreateTagList( /*List all tags*/ false );        
    if( tag_list == null )
        return;
    
    var size = tag_list.GetSize();
    if( size == 0 )
        return 0; /* no tags found */
    
    /* Traverse the tag list. For each tag, transfer the TagSource
       defined in the selected object to the tag in the loaded 
       popup drawing that has a matching TagName.
    */
    var tag_obj, tag_name, tag_source;
    var num_remapped_tags = 0;  /* int */
    var total_remapped = 0;     /* int */
    for( var i=0; i<size; ++i )
    {
        tag_obj = tag_list.GetElement( i );
        
        // Obtain TagName.
        tag_name = tag_obj.GetSResource( "TagName" );
        
        // Skip tags with undefined TagName.
        if( IsUndefined( tag_name ) )
            continue;
        
        // Obtain TagSource.
        tag_source = tag_obj.GetSResource( "TagSource" );
         
        // Skip tags with undefined TagSource.
        if( IsUndefined( tag_source ) )
            continue;
        
        /* Remap all tags with the specified tag name (tag_name)
           to use a new tag source (tag_source).
        */
        if( unset_tags )
            num_remapped_tags = RemapNamedTags( viewport, tag_name, "unset" );
        else
            num_remapped_tags = RemapNamedTags( viewport, tag_name, tag_source );
        
        total_remapped += num_remapped_tags;
    }

    return total_remapped;
}

//////////////////////////////////////////////////////////////////////
// Remap all object tags with the specified tag_name to use a new 
// tag_source. 
//////////////////////////////////////////////////////////////////////
function RemapNamedTags( /*GlgObject*/ glg_obj, /*String*/ tag_name, 
                        /*String*/ tag_source )              /*int*/
{
    /* Obtain a list of tags with TagName attribute matching 
       the specified tag_name.
    */
    var tag_list = 
        glg_obj.GetTagObject( tag_name, /*by name*/ true, 
                              /*list all tags*/ false, 
                              /*multiple tags mode*/ false, 
                              GLG.GlgTagType.DATA_TAG );

    if( tag_list == null )
        return 0;
    
    var size = tag_list.GetSize();
    if( size == 0 )
        return 0;

    var tag_obj, access_type, data_type, d_value, s_value;
    for( var i=0; i<size; ++i )
    {
        tag_obj = tag_list.GetElement( i );
        
        /* If tag is INIT ONLY, initialize its value based on the current 
           data value for the given tag_source. Don't reassign TagSource 
           for this tag_obj, it is initialized only once and will not be 
           subject to periodic updates.
        */
        access_type = Math.trunc( tag_obj.GetDResource( "TagAccessType" ) );
        if( access_type == GLG.GlgTagAccessType.INIT_ONLY_TAG )
        {
            data_type = Math.trunc( tag_obj.GetDResource( "DataType" ) );
            switch( data_type )
            { 
            case GLG.GlgDataType.D:
                d_value = MainViewport.GetDTag( tag_source );
                tag_obj.SetDResource( null, d_value );
                break;
            case GLG.GlgDataType.S:
                s_value = MainViewport.GetSTag( tag_source );
                tag_obj.SetSResource( null, s_value );
                break;
            }
        }
        else
            AssignTagSource( tag_obj, tag_source );
    }
    
    return size;
}

//////////////////////////////////////////////////////////////////////
// Assigns new TagSource to the given tag object.
//////////////////////////////////////////////////////////////////////
function AssignTagSource( /*GlgObject*/ tag_obj, /*String*/ new_tag_source )
{
    tag_obj.SetSResource( "TagSource", new_tag_source );
} 

//////////////////////////////////////////////////////////////////////////////
// Set popup viewport size, based on the size of the loaded popup drawing,
// and position the popup next to the selected object.
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
// Set popup viewport size, based on the size of the loaded popup drawing,
// and position it next to the selected object.
//////////////////////////////////////////////////////////////////////////////
function SetPopupSize( /*boolean*/ floating )
{
    /* Use "Width" and "Height" resources from the loaded popup drawing, if any.
       Otherwise, use default width and height defined in GlgActivePopup.
    */
    if( ActivePopup.drawing_vp.HasResourceObject( "Width" ) )
        ActivePopup.width = ActivePopup.drawing_vp.GetDResource( "Width" );
    
    if( ActivePopup.drawing_vp.HasResourceObject( "Height" ) )
        ActivePopup.height = ActivePopup.drawing_vp.GetDResource( "Height" );

    // Adjust popup width and height for mobile devices.
    if( CoordScale != 1.0 )
    {
        if( ActivePopup.popup_type >= EMBEDDED_POPUP_MENU )
        {
            ActivePopup.width *= MENU_COORD_SCALE;
            ActivePopup.height *= MENU_COORD_SCALE;
        }
        else   // Dialog
        {
            ActivePopup.width *= DIALOG_COORD_SCALE;
            ActivePopup.height *= DIALOG_COORD_SCALE;
        }
    }

    if( floating )
    {
        // For a floating dialog, use Screen hints to set dialog size.
        ActivePopup.popup_vp.SetDResource( "Screen/WidthHint", 
                                           ActivePopup.width );
        ActivePopup.popup_vp.SetDResource( "Screen/HeightHint", 
                                           ActivePopup.height );
    }
    else
    {
        /* For a non-floating dialog, set popup size by setting 
           viewport's Width/Height resources.
        */ 
        ActivePopup.popup_vp.SetDResource( "Width", ActivePopup.width );
        ActivePopup.popup_vp.SetDResource( "Height", ActivePopup.height );
    }
}

//////////////////////////////////////////////////////////////////////////////
// Position embedded poppup object next to the selected object.
//////////////////////////////////////////////////////////////////////////////
function PositionPopup( /*boolean*/ floating )
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
    
    // Width and height of the popup object.
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

    /* Use hints (XHint and YHint) to position a floating the dialog.
       Position a non-floating viewport using PositionObject() API method.
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
        // Make sure the popup viewport is set up before positioning it.
        ActivePopup.popup_vp.SetupHierarchy();
        
        ActivePopup.popup_vp.PositionObject( GLG.GlgCoordType.SCREEN_COORD, 
                                             x_anchor | y_anchor, x, y, 0.0 );
    }
}

//////////////////////////////////////////////////////////////////////////////
// Returns true if the object is a viewport and is a floating dialog.
//////////////////////////////////////////////////////////////////////////////
function IsFloatingDialog( /*GlgObject*/ glg_obj )   /* boolean */
{
    var type = Math.trunc( glg_obj.GetDResource( "Type" ) ); /*GlgObjectType*/
    if( type == GLG.GlgObjectType.VIEWPORT )
    {
        var shell_type = Math.trunc( glg_obj.GetDResource( "ShellType" ) ); 
        if( shell_type != GLG.GlgShellType.NO_TOP_SHELL ) 
            return true; // Floating dialog.
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////
// Move the object to the top of the object hierarchy inside a container.
//////////////////////////////////////////////////////////////////////////////
function ReorderToFront( /*GlgObject*/ container, /*GlgObject*/ glg_obj )
{
    // Get container size.
    var size = container.GetSize();
    if( size == 0 )
        return;

    // Get current index position.
    var obj_pos = container.GetIndex( glg_obj );

    // Bring the object to front.
    container.ReorderElement( obj_pos, size - 1 );
}
 
//////////////////////////////////////////////////////////////////////////////
// Change drawing size while maintaining width/height aspect ratio.
//////////////////////////////////////////////////////////////////////////////
function SetDrawingSize( next_size )
{
    const ASPECT_RATIO = 700 / 540;
    
    const MIN_WIDTH = 500;
    const MAX_WIDTH = 900;
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
       drawing_area.style.height = 
           "" + Math.trunc( width / ASPECT_RATIO ) + "px";
   }
}

//////////////////////////////////////////////////////////////////////////////
// Increases canvas resolution for mobile devices with HiDPI displays.
// Sets CoordScale and TextScale global variables for mobile devices.
//////////////////////////////////////////////////////////////////////////////
function SetCanvasResolution()
{
    // Set canvas resolution only for mobile devices with devicePixelRatio != 1.
    if( window.devicePixelRatio == 1. || !SetDrawingSize.is_mobile )
        return;   // Use coord scale = 1.0 for desktop.

    /* The first parameter defines canvas coordinate scaling with values 
       between 1 and devicePixelRatio. Values greater than 1 increase 
       canvas resolution and result in sharper rendering. The value of 
       devicePixelRatio may be used for very crisp rendering with very thin 
       lines.
       
       Canvas scale > 1 makes text smaller, and the second parameter defines
       the text scaling factor used to increase text size.
       
       The third parameter defines the scaling factor that is used to
       scale down text in native widgets (such as native buttons, toggles, etc.)
       to match the scale of the drawing.
    */
    CoordScale = 2.0;
    TextScale = 1.5;
    var native_widget_text_scale = 0.6;
    GLG.SetCanvasScale( CoordScale, TextScale, native_widget_text_scale );
    
    // Mobile devices use fixed device-width: disable Change Drawing Size button.
    var change_size_button = document.getElementById( "change_size" );
    if( change_size_button != null )
        change_size_button.parentNode.removeChild( change_size_button );
}

//////////////////////////////////////////////////////////////////////////
// Adjust GLG object geometry for mobile devices if needed, using
// special properties defined in the object.
//////////////////////////////////////////////////////////////////////////
function AdjustForMobileDevices( /*GlgObject*/ glg_obj )
{
    if( CoordScale == 1.0 ) // Desktop, no adjustements needed.
        return;

    SetParameter( glg_obj, "CoordScale", CoordScale );
    SetParameter( glg_obj, "OffsetCoeffForMobile", TextScale );
}

//////////////////////////////////////////////////////////////////////////////
// Loads any assets required by the application and invokes the specified
// callback when done.
// Alternatively, the application drawing can be loaded as an asset here
// as well, so that it starts loading without waiting for the other assets 
// to finish loading.
//////////////////////////////////////////////////////////////////////////////
function LoadAssets( callback, user_data )
{
    /* HTML5 doesn't provide a scrollbar input element (only a range input 
       html element is available). This application needs to load GLG scrollbars
       used for integrated chart scrolling. For each loaded scrollbar, the 
       AssetLoaded callback is invoked with the supplied data array parameter.
    */    
    GLG.LoadWidgetFromURL( "scrollbar_h.g", null, AssetLoaded,
                           { name: "scrollbar_h", callback: callback,
                             user_data: user_data } );
    GLG.LoadWidgetFromURL( "scrollbar_v.g", null, AssetLoaded,
                           { name: "scrollbar_v", callback: callback,
                             user_data: user_data } );
    
    /* Load a popup viewport object that will be used to display popup drawings
       for the PopupDialog or PopupMenu commands that may be attached to objects.
    */
    if( HAS_EXTENDED_API )
    {
        GLG.LoadWidgetFromURL( POPUP_VIEWPORT_FILENAME, null, AssetLoaded,
                               { name: "popup_viewport", callback: callback,
                                 user_data: user_data } );
    }   
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
    
    /* Invoke the callback (the second parameter of the data array) after all
       assets have been loaded.
    */
    if( AssetLoaded.num_loaded == ( HAS_EXTENDED_API ? 3 : 2 ) )
        data.callback( data.user_data );
}

//////////////////////////////////////////////////////////////////////////////
// Status display: 
// Display the title of the currently displayed drawing, as well as
// the title of the drawing which is in the process of being loaded (if any).
//////////////////////////////////////////////////////////////////////////////
function DisplayStatus( message )
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
                // Add spaces after the displayed drawing title.
                message += "&nbsp;&nbsp;&nbsp;&nbsp;";   
            else
                message = "";
            
            message += "Loading: <b>" + LoadingTitle + "</b>";
        }
    }

    document.getElementById( "status_div" ).innerHTML = message;
}

//////////////////////////////////////////////////////////////////////////
// Returns PageType integer constant using PageTypeTable.
//////////////////////////////////////////////////////////////////////////
function GetPageType( /*GlgObject*/ drawing )  /*int*/
{
    if( drawing == null )
        return UNDEFINED_PAGE_TYPE;

    var type_obj = drawing.GetResourceObject( "PageType" );   /* GlgObject */
    if( type_obj == null ) // PageType resource is not present.
        return DEFAULT_PAGE_TYPE;
    
    var type_str = type_obj.GetSResource( null );   /* String */
   
    var page_type =   /* int */
        ConvertStringToType( PageTypeTable, type_str, UNDEFINED_PAGE_TYPE,
                             UNDEFINED_PAGE_TYPE );

    if( page_type == UNDEFINED_PAGE_TYPE )
    {
        AppInfo( "Undefined PageType, using Default PageType." );
        return DEFAULT_PAGE_TYPE;
    }

    return page_type;
}

//////////////////////////////////////////////////////////////////////////
// Returns CommandType integer constant using CommandTypeTable.
//////////////////////////////////////////////////////////////////////////
function GetCommandType( /*GlgObject*/ command_obj ) /*int*/
{
    var command_type_str = command_obj.GetSResource( "CommandType" );
    
    return ConvertStringToType( CommandTypeTable, command_type_str,
                                UNDEFINED_COMMAND_TYPE, UNDEFINED_COMMAND_TYPE );
}

//////////////////////////////////////////////////////////////////////////
// Returns PopupMenuType integer constant using PopupMenuTypeTable.
//////////////////////////////////////////////////////////////////////////
function GetPopupMenuType( /*GlgObject*/ command_obj )
{
    var menu_type_str = command_obj.GetSResource( "MenuType" );
    
    return ConvertStringToType( PopupMenuTypeTable, menu_type_str,
                                EMBEDDED_POPUP_MENU, UNDEFINED_POPUP_TYPE );
}

//////////////////////////////////////////////////////////////////////////
// Returns DialogType integer constant using DialogTypeTable.
//////////////////////////////////////////////////////////////////////////
function GetDialogType( /*GlgObject*/ command_obj )
{
    var dialog_type_str = command_obj.GetSResource( "DialogType" );
    
    return ConvertStringToType( DialogTypeTable, dialog_type_str,
                                EMBEDDED_POPUP_DIALOG, UNDEFINED_POPUP_TYPE );
}

//////////////////////////////////////////////////////////////////////////
function GetWidgetType( /*GlgObject*/ glg_obj ) /* int */
{
    if( glg_obj == null )
    {
        AppAlert( "Null widget." );
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
        AppInfo( "Undefined WidgetType, using default WidgetType." );
        return DEFAULT_WIDGET_TYPE;
    }   

    return widget_type;
}

//////////////////////////////////////////////////////////////////////////
// Utility function to convert a string to a corresponding int value
// using a provided table.
//////////////////////////////////////////////////////////////////////////
function ConvertStringToType( table, type_str, empty_type, undefined_type )
{
    if( type_str == null || type_str.length == 0 )
        return empty_type;
      
    for( var i=0; i<table.length; ++i )
    {
        if( type_str == table[i].type_str )
          return table[i].type_int;
    }
    
    return undefined_type;
}

//////////////////////////////////////////////////////////////////////////////
function IsUndefined( /*String*/ str )  /* boolean */
{
    return ( str == null || str.length == 0 || 
             str == "unset" || str == "$unnamed" );
}

//////////////////////////////////////////////////////////////////////////////
function Debug( message )
{
    if( DEBUG )
        console.log( message );
}

//////////////////////////////////////////////////////////////////////////////
function AppError( message )
{
    console.error( message );
}

//////////////////////////////////////////////////////////////////////////////
function AppAlert( message )
{
    alert( message );
}

//////////////////////////////////////////////////////////////////////////////
function AppInfo( message )
{
    console.info( message );
}

//////////////////////////////////////////////////////////////////////////
function GetCurrTime()
{
   return Date.now() / 1000;    // seconds
}   

//////////////////////////////////////////////////////////////////////////
// Used only if USE_FIXED_TIMER_INTERVAL = false.
//////////////////////////////////////////////////////////////////////////
function AdjustTimerInterval()
{
    if( FirstDataQuery )
    {
        FirstDataQuery = false;
        TimerInterval = UpdateInterval;
        return;
    }

    /* If updates and data queries are fast compared to the UpdateInterval, 
       a simple timer with a fixed UpdateInterval can be used, as shown in 
       the case of USE_FIXED_TIMER_INTERVAL=true, and the logic below 
       would not be necessary.

       However, if a timer with a fixed interval is started before rendering 
       is completed by PushData(), it may overload the browser and cause 
       sluggish response if rendering takes longer than the requested 
       UpdateInterval. If a timer with a fixed interval is started after 
       PushData() is called, the actual update interval will be slower than 
       the requested interval if either rendering or data query takes longer.

       The logic below uses a dynamic timeout that attempts to maintain the 
       requested UpdateInterval regardless of the fluctuations in the duration 
       of the data requests and drawing updates.
       
       The data query is asynchronous. If the data query takes a long time, 
       we want to issue the next data query right away, so that the new data 
       are loaded while the drawing is being updated with the data we received. 
       If data queries and drawing updates are fast, we want to use a timeout 
       that would ensure a requested UpdateInterval. 
       
       To determine an appropriate timeout value, we would need to know how 
       long it took to query data, and how long it took to update the drawing.
       If the drawing rendering (refresh) by PushData() takes a long time, 
       there is no way to determine the time it took to load data, since the 
       GetDataCB data callback is delayed until the rendering is complete. 

       The code below uses iterative approach to dynamically adjust to the 
       fluctuations of the time required to render/refresh graphics and the time
       of each data query.
    */
    var current_time = new Date().getTime();
    
    var elapsed_time = current_time - DataStartTime;
    var idle_time    = elapsed_time - UpdateDuration;
    
    if( DEBUG_TIME_INTERVAL )
        console.log( "Elapsed time=" + elapsed_time + 
           " update duration=" + UpdateDuration + 
           " idle time= " + idle_time );

    if( idle_time < MIN_IDLE_INTERVAL )
    {
        /* Rendering was too slow (idle time too small): increase timer interval
           to let the browser handle UI events.
        */
        TimerInterval += ( MIN_IDLE_INTERVAL - idle_time );
        if( DEBUG_TIME_INTERVAL )
            console.log( "  Adding " +  ( MIN_IDLE_INTERVAL - idle_time ) );
        return;
    }

    if( elapsed_time < UpdateInterval )
    {
        /* Data request + update was too fast, increase timer interval.
           Increase gradually using CHANGE_COEFF to avoid rapid jumps on a 
           single fast iteration that might have little data to update.
        */
        TimerInterval += ( UpdateInterval - elapsed_time ) * CHANGE_COEFF;
    }
    else if( elapsed_time > UpdateInterval )
    {
        // The data query took longer, decrease timer interval if possible.
        var delta = elapsed_time - UpdateInterval;
        
        /* Can't adjust by more than max_allowed: need to maintain 
           MIN_IDLE_INTERVAL.
        */
        var max_allowed = idle_time - MIN_IDLE_INTERVAL;
        
        if( delta > max_allowed )
            delta = max_allowed;
        
        /* Decrease gradually using CHANGE_COEFF to avoid rapid jumps on a 
           single delayed data request.
        */
        TimerInterval -= delta * CHANGE_COEFF;
        
        if( TimerInterval < 0 )
            TimerInterval = 0;  // Data request is slow, use no delay.
    }
    // else : elapsed_time == UpdateInterval, no change.
}

//////////////////////////////////////////////////////////////////////////
// Sets a D parameter of the specified object to the specified value.
// Returns false if the specified resource is not present.
// Returns true on success.
//////////////////////////////////////////////////////////////////////////
function SetParameter( /*GlgObject*/ object, 
                       /*String*/ res_name, /*double*/ value ) /* boolean */
{
    var res_obj = object.GetResourceObject( res_name );
    if( res_obj == null )
        return false;

    return res_obj.SetDResourceIf( null, value, /*if_changed*/ true );
}

//////////////////////////////////////////////////////////////////////////
// Scale a D parameter of the specified object by the specified scale
// factor.
//////////////////////////////////////////////////////////////////////////
function ScaleParameter( /*GlgObject*/ object, 
                         /*String*/ res_name, /*double*/ scale ) /* boolean */
{
    var res_obj = object.GetResourceObject( res_name );
    if( res_obj == null )
        return false;

    var value = res_obj.GetDResource( null );
    return res_obj.SetDResource( null, value * scale );
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

        // Remove touch listener once a touch device has been detected.
        document.removeEventListener( "touchstart", DetectTouchDevice );
    }
}

//////////////////////////////////////////////////////////////////////////////
// "keydown" event listener added to the document to handle key down events.
// In this example, ESC key closes active popup dialog and Alarm dialog,
// if any.
//////////////////////////////////////////////////////////////////////////////
function HandleKeyEvent( event )
{
    if( event.key == "Escape" )      // ESC key
    {
        // Close active popup, if any.
        CloseActivePopup();
        
        // Close Alarm dialog, if any.
        CloseAlarmDialog();
    }
}

//////////////////////////////////////////////////////////////////////////////
// Is used to populate predefined tables, such as CommandTypeTable,
// PageTypeTable, DialogTypeTable, etc.
//////////////////////////////////////////////////////////////////////////////
function TypeRecord( /*String*/ str, /*int*/ value )
{
   this.type_str = str;
   this.type_int = value;
}

//////////////////////////////////////////////////////////////////////////
// GlgTagRecord object is used to store information for a given GLG tag. 
// It can be extended by the application as needed.
//////////////////////////////////////////////////////////////////////////
function GlgTagRecord( /*int*/ data_type, /*String*/ tag_name,
                       /*String*/ tag_source, /*GlgObject*/ tag_obj,
                       /*int*/ tag_access_type )
{
    this.data_type = data_type;              /* int */
    this.tag_name = tag_name;                /* String */
    this.tag_source = tag_source;            /* String */
    this.tag_obj = tag_obj;                  /* GlgObject */
    this.tag_access_type = tag_access_type;  /* int */ 

    /* Object IDs for ValueEntryPoint, TimeEntryPoint and ValidEntryPoint
       for a plot in a RealTimeChart, if any. These objects will be valid if:
        - SUPPLY_PLOT_TIME_STAMP=true,
        - the drawing contains a chart,
        - the chart's plot has a valid TagSource assigned to its 
          ValueEntryPoint.
    */
    this.plot_value_ep = null;     /* GlgObject* /
    this.plot_time_ep = null;      /* GlgObject */
    this.plot_valid_ep= null;      /* GlgObject */

    // The value type is double for data_type=D, or String for data_type=S.
    this.value = null;         

    this.time_stamp = null;        /* double */
    this.value_valid = false;      /* boolean */
    
    /* if_changed flag will be set to false if there is a chart in the drawing
       with a matching TagSource. Otherwise, it will be set to true for
       performance optimization to push a new value to the graphics only
       if the value has changed.
    */
    this.if_changed = false;
}

//////////////////////////////////////////////////////////////////////////
function GlgActivePopup( /*int*/ popup_type, /*GlgObject*/ popup_vp,
                         /*String*/ drawing_file, /*GlgObject*/ subwindow,
                         /*GlgObject*/ selected_obj, /*String*/ title,
                         /*String*/ paramS, /*double*/ paramD )
{
    this.popup_type = popup_type;              /* int */
    this.popup_vp = popup_vp;                  /* GlgObject */  
    this.drawing_file = drawing_file;          /* String */
    this.subwindow = subwindow;                /* GlgObject */
    this.selected_obj = selected_obj;          /* GlgObject */
    this.title = title;                        /* String */
    this.paramS = paramS;                      /* String */
    this.paramD = paramD;                      /* double */

    this.drawing_vp = null;                    /* GlgObject */
    this.num_remapped_tags = 0;                /* int */
    this.width = 300;                          /* Default popup width */
    this.height = 300;                         /* Default popup height */
}
