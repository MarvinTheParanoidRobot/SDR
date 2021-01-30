//////////////////////////////////////////////////////////////////////////////
// GLG SimpleViewer example
//
// The example is written in pure HTML5 and JavaScript using GLG Standard API.
// The example source code uses the GLG Toolkit JavaScript Library 
// supplied by the included Glg*.js and GlgToolkit*.js files.
//
// The library loads a GLG drawing and renders it on a web page, providing
// an API to animate the drawing with real-time data and handle user
// interaction with graphical objects in the drawing.
//
// The drawings are created using the GLG Graphics Builder, an interactive
// editor that allows to create grahical objects and define their dynamic
// behavior without any programming.
//
// The viewer demonstrates how to animate a loaded drawing using tags 
// defined in the drawing.
//////////////////////////////////////////////////////////////////////////////

// Get a handle to the GLG Toolkit library.
var GLG = new GlgToolkit();

// Debugging aid: uncomment the next line to throw an exception on a GLG error.
GLG.ThrowExceptionOnError( true, true, true );

// Enable/disable debuginng/diagnostics information.
const DEBUG = false;
const DEBUG_TIME_INTERVAL = false;

/* If set to true, simulated demo data will be used for animation.
   Set to false to enable live application data.
*/
const RANDOM_DATA = true;

/* If set to false, TimerInterval used for updates will be adjused to obtain
   a targeted UPDATE_INTERVAL. Otherwise, a fixed timer interval is used for 
   updates.
*/
const USE_FIXED_TIMER_INTERVAL = false;

const DEFAULT_DRAWING_NAME = "tags_example.g";  // Default GLG drawing name.
const DEFAULT_DRAWING_TITLE = "Process Overview"; // Default description.

/* This object is used to pass data to the load callback. 
   DrawingLoadRequest fields: enabled, drawing_name, title.
   The "enabled" field is used to cancel the current load request if loading 
   of another drawing was requested while the current request has not finished. 
   This may happen if the user clicks on a button to load one drawing
   and then clicks on another button to load another drawing before
   the first drawing finished loading.
   The "drawing_name" field is used to store drawing name of the load request.
*/
var DrawingLoadRequest = 
    { enabled: true, 
      drawing_name: DEFAULT_DRAWING_NAME, 
      drawing_title: DEFAULT_DRAWING_TITLE 
    };

// Stores the name of the currently loaded drawing.
var DrawingNameLoaded = null;

const UPDATE_INTERVAL = 100;              // Targeted data query interval, msec.
const MIN_IDLE_INTERVAL = 30;             // msec
const WAIT_INTERVAL = 30;                 // msec
const CHANGE_COEFF = 1/3;                 // Rate of time interval adjustment.
var TimerInterval = UPDATE_INTERVAL;      // Initial data query interval.

// Start time for the data query, gets set in GetData().
var DataStartTime = 0;

var UpdateDuration;                   /* double */               
var FirstDataQuery = true;            /* boolean */
var FirstDrawing = true;              /* boolean */
var WaitForUpdate = false;            /* boolean */

// GLG viewport object of the currently loaded drawing.
var Viewport;            /* GlgObject */

// DataFeed object used for animation.
var DataFeed = null;    

// Dynamically created array of tag records of type GlgTagRecord.
var TagRecords = null;   /* GlgTagRecord[] */

/* Contains a list of tag sources defined in the drawing, used by the 
   DataFeed object to query real-time data for all tags defined in the drawing.
   Each element is an object with a property 'tag_source'.
*/
var DataTagList = null;  /* String[] */

// Title variables used for status display.
var LoadingTitle = null;       // Title of a drawing being loaded.
var DisplayedTitle = null;     // Title of currently displayed drawing.

/* Coefficients for canvas resolution and text resolution. 
   These parameters will be adjusted for mobile devices with HiDPI displays
   in SetCanvasResolution().
*/
var CoordScale = 1;
var TextScale = 1;

// Set initial size of the drawing.
SetDrawingSize( false );

/* Increase canvas resolution for mobile devices. Changes CoordScale and
   TextScale.
*/
SetCanvasResolution();

/* Load misc. assets such as GLG scrollbars. When assets are loaded, 
   LoadDrawing is invoked that loads a specified GLG drawing.
*/
LoadAssets( ()=>{ LoadDrawing( DEFAULT_DRAWING_NAME, DEFAULT_DRAWING_TITLE ) },
            null );

// Add DataFeed object.
AddDataFeed();

////////////////////////////////////////////////////////////////////////////// 
// Load a GLG drawing from a file.
////////////////////////////////////////////////////////////////////////////// 
function LoadDrawing( /*String*/ filename, /*String*/ title )
{
    /* Don't reload the drawing if the filename is invalid or it matches 
       currently loaded drawing name.
    */
    if( filename == null || filename == DrawingNameLoaded )
        return;

    // New drawing was requested, cancel any pending drawing load requests.
    AbortPendingLoadRequests();    

    // Create a new load request.
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
    if( Viewport != null )
        DestroyDrawing();

    // Store drawing name of the currently loaded drawing.
    DrawingNameLoaded = load_request.drawing_name;
        
    // Define the element in the HTML page to display the drawing.
    drawing.SetParentElement( "glg_area" );
    
    // Disable viewport border to use the border of the glg_area.
    drawing.SetDResource( "LineWidth", 0 );

    StartGlgViewer( drawing );

    // Update status info.
    LoadingTitle = null;
    DisplayedTitle = load_request.drawing_title;
    DisplayStatus();
}

//////////////////////////////////////////////////////////////////////////////
function StartGlgViewer( /*GlgObject*/ drawing )
{
    Viewport = drawing;

    // Adjust the drawing for mobile devices if needed.
    AdjustForMobileDevices( Viewport );
    
    // Initialization before hierarchy setup.
    InitBeforeH();

    // Setup object hierarchy in the drawing.
    Viewport.SetupHierarchy();

    // Initialization after hierarchy setup.
    InitAfterH();

    // Flag to indicate first data query for the loaded page.
    FirstDataQuery = true;
    
    // Start data update timer.
    if( FirstDrawing )
    {
        FirstDrawing = false;
        GetData();
    }

    // Display the drawing in a web page.
    Viewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
// Destroy currently loaded drawing, if any.
//////////////////////////////////////////////////////////////////////////////
function DestroyDrawing()
{
    if( Viewport == null )
        return;
    
    DeleteTagRecords();
    Viewport.ResetHierarchy();
    Viewport = null;
}

//////////////////////////////////////////////////////////////////////////////
// Initialization before hierarchy setup.
//////////////////////////////////////////////////////////////////////////////
function InitBeforeH()
{
    Viewport.AddListener( GLG.GlgCallbackType.INPUT_CB, InputCallback );
    
    // If the drawing contains a QuitButton, make it invisible.
    if( Viewport.HasResourceObject( "QuitButton" ) )
        Viewport.SetDResource( "QuitButton/Visibility", 0 );
}

//////////////////////////////////////////////////////////////////////////////
// Initialization after hierarchy setup.
//////////////////////////////////////////////////////////////////////////////
function InitAfterH()
{
    // Build TagRecords array, a list of GLG tag records.
    if( !CreateTagRecords( Viewport ) )
        return;
    
    /* Build DataTagList to be used by the DataFeed object to query real-time
       data from the server for all tags defined in the drawing.
    */
    CreateDataTagList();
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
// Create and populate TagRecords array. Each item has the following
// properties: tag_obj, data_type, tag_name, tag_source, if_changed.
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
        
        tag_access_type = Math.trunc( tag_obj.GetDResource( "TagAccessType" ) );
        
        // Handle special tags according to their tag access type.
        switch( tag_access_type )
        {
         case GLG.GlgTagAccessType.OUTPUT_TAG: continue;   // Skip OUTPUT tags.

         case GLG.GlgTagAccessType.INIT_ONLY_TAG:
         case GLG.GlgTagAccessType.INPUT_TAG:
         default: break;
        }
        
        /* Add a valid tag record to the list. "value" property will be assigned
           in StoreTagValue(), when a new data value is received. 
        */
        var tag_record = new GlgTagRecord( data_type, tag_name, tag_source, 
                                           tag_obj, tag_access_type );

        TagRecords.push( tag_record );
    }

    Debug( "TagRecords array size: " + TagRecords.length ); 

    if( TagRecords.length == 0 )
    {
        TagRecords = null;
        return false;
    }
    
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
function CreateDataTagList()
{
    if( TagRecords == null )
    {
        DatatagList = null;
        return;
    }

    DataTagList = [];
    for( var i=0; i<TagRecords.length ; ++i )
        DataTagList.push( { tag_source: TagRecords[i].tag_source } );
}

//////////////////////////////////////////////////////////////////////////////
// Obtain real-time data for all tags defined in the drawing.
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
function GetDataCB( /*Array*/ new_data, /*String*/ drawing_name )
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

   if( USE_FIXED_TIMER_INTERVAL )  // Use fixed targeted UPDATE_INTERVAL
    {
         // Push new data to the graphics.
        PushData( new_data );

        // Send new data query request.
        setTimeout( GetData, UPDATE_INTERVAL );
    }
    else   // Adjust TimerInterval to try obtain a targeted UPDATE_INTERVAL
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
            /* Data query took longer than targeted UPDATE_INTERVAL: process
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
        AppError( "No new data received." );
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
        tag_record = StoreTagValue( new_data[i].tag_source, new_data[i].value );

        // Tag record not found, or the value is invalid.
        if( tag_record == null )
            continue;

        // Push new data value into graphics.
        switch( tag_record.data_type )
        {
         case GLG.GlgDataType.D: // D-type tag
            /* For performance optimization, pass 'true' for the if_changed 
               flag, so that the graphics is updated only if the value has 
               changed. if_changed flag is ignored for a real-time chart,
               so that the chart scrolls even if the value is the same.
            */
            Viewport.SetDTag( tag_record.tag_source, tag_record.value, 
                              /*if_changed*/ true );
            break;
                
         case GLG.GlgDataType.S:
            // Pass 'true" for the if_changed flag.
            Viewport.SetSTag( tag_record.tag_source, tag_record.value, 
                              /*if_changed*/ true );
            break;
             
         case GLG.GlgDataType.G:      // Not used in this example.
         default: break;
        }
    }

    // Refresh display.
    Viewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
function StoreTagValue( tag_source, value )
{
    // Find a tag record with a matching tag_source.
    var tag_record = LookupTagRecords( tag_source );

    if( tag_record == null ) // Not found.
        return null;
    
    // Store new value in the found tag_record.
    tag_record.value = value;

    // Return a valid tag record.
    return tag_record;
} 

//////////////////////////////////////////////////////////////////////////////
// Find a tag record in TagRecords array with a tag source matching
// the specified tag_source.
//////////////////////////////////////////////////////////////////////////////
function LookupTagRecords( tag_source )
{
    for( var i=0; i<TagRecords.length; ++i )
    {
        if( TagRecords[i].tag_source == tag_source )
            return TagRecords[i];
    }

    return null; // not found.
}

//////////////////////////////////////////////////////////////////////////////
// Handle user interaction with the buttons, as well as process custom
// actions attached to objects in the drawing.
//////////////////////////////////////////////////////////////////////////////
function InputCallback( vp, message_obj )
{
    var origin = message_obj.GetSResource( "Origin" );
    var format = message_obj.GetSResource( "Format" );
    var action = message_obj.GetSResource( "Action" );
    
    if( format == "Button" )
    {	 
        /* Neither a push button or a toggle button. */
        if( action !="Activate" && action != "ValueChanged" )
            return;
        
        if( action == "Activate" )  // Push button event.
        {
            if( origin == "StartButton" )
            {
                if ( RANDOM_DATA && Viewport.HasTagSource( "State" ) )
                    DataFeed.WriteDValue( "State", 1.0 );
                else
                    // Place custom code here as needed.
                    ;
            }
            else if( origin == "StopButton" )
            {
                if ( RANDOM_DATA && Viewport.HasTagSource( "State" ) )
                    DataFeed.WriteDValue( "State", 0.0 );
                else
                    // Place custom code here as needed.
                    ; 
            }
        }
        else if( action == "ValueChanged" ) // Toggle button event.
        {
            var state = message_obj.GetDResource( "OnState" );
            
            // Place code here to handle events from a toggle button
            // and write a new value to a given tag_source.
            // DataFeed.WriteDValue( tag_source, state );
            
            if ( RANDOM_DATA && Viewport.HasTagSource( "State" ) )
                DataFeed.WriteDValue( "State", state );
            else // Place custom code here
                ;
        }
        
        Viewport.Update();  //format = "Button"
    }
    else if( format == "Timer" )   // Handles timer transformations.
        Viewport.Update();
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

//////////////////////////////////////////////////////////////////////////////
// Adjust GLG object geometry for mobile devices if needed, using 
// special properties defined in the object.
//////////////////////////////////////////////////////////////////////////////
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
}

//////////////////////////////////////////////////////////////////////////////
function AssetLoaded( /*GlgObject*/ glg_object, /*Object*/ data, 
                      /*String*/ path )
{
    if( data.name == "scrollbar_h" )
    {
        if( glg_object != null )
            glg_object.SetResourceObject( "$config/GlgHScrollbar", glg_object );
    }
    else if( data.name == "scrollbar_v" )
    {
        if( glg_object != null )
            glg_object.SetResourceObject( "$config/GlgVScrollbar", glg_object );
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
    if( AssetLoaded.num_loaded == 2 )
        data.callback( data.user_data );
}

//////////////////////////////////////////////////////////////////////////
// Sets a D parameter of the specified object to a specified value.
// Returns true on success. Returns false if the specified resource 
// is not present.
//////////////////////////////////////////////////////////////////////////
function SetParameter( /*GlgObject*/ object, 
                       /*String*/ res_name, /*double*/ value ) /*boolean*/
{
    if( !object.HasResourceObject( res_name ) )
        return false;
    
    return object.SetDResourceIf( res_name, value, true );
}


//////////////////////////////////////////////////////////////////////////////
function IsUndefined( /*String*/ str ) /* boolean */
{
    return ( str == null || str.length == 0 || 
             str == "unset" || str == "$unnamed" );
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
    window.alert( message );
}

//////////////////////////////////////////////////////////////////////////
// Used only if USE_FIXED_TIMER_INTERVAL = false.
//////////////////////////////////////////////////////////////////////////
function AdjustTimerInterval()
{
    if( FirstDataQuery )
    {
        FirstDataQuery = false;
        TimerInterval = UPDATE_INTERVAL;
        return;
    }

    /* If updates and data queries are fast compared to the UPDATE_INTERVAL, 
       a simple timer with a fixed UPDATE_INTERVAL can be used, as shown in 
       the case of USE_FIXED_TIMER_INTERVAL=true, and the logic below 
       would not be necessary.

       However, if a timer with a fixed interval is started before rendering 
       is completed by PushData(), it may overload the browser and cause 
       sluggish response if rendering takes longer than the requested 
       UPDATE_INTERVAL. If a timer with a fixed interval is started after 
       PushData() is called, the actual update interval will be slower than 
       the requested interval if either rendering or data query takes longer.

       The logic below uses a dynamic timeout that attempts to maintain the 
       requested UPDATE_INTERVAL regardless of the fluctuations in the duration 
       of the data requests and drawing updates.
       
       The data query is asynchronous. If the data query takes a long time, 
       we want to issue the next data query right away, so that the new data 
       are loaded while the drawing is being updated with the data we received. 
       If data queries and drawing updates are fast, we want to use a timeout 
       that would ensure a requested UPDATE_INTERVAL. 
       
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

    if( elapsed_time < UPDATE_INTERVAL )
    {
        /* Data request + update was too fast, increase timer interval.
           Increase gradually using CHANGE_COEFF to avoid rapid jumps on a 
           single fast iteration that might have little data to update.
        */
        TimerInterval += ( UPDATE_INTERVAL - elapsed_time ) * CHANGE_COEFF;
    }
    else if( elapsed_time > UPDATE_INTERVAL )
    {
        // The data query took longer, decrease timer interval if possible.
        var delta = elapsed_time - UPDATE_INTERVAL;
        
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
    // else : elapsed_time == UPDATE_INTERVAL, no change.
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

    // The value type is double for data_type=D, or String for data_type=S.
    this.value = null;         
}
