//////////////////////////////////////////////////////////////////////////////
// This example demonstrates how to animate a Glg drawing containing a
// panel (dashboard) of GLG controls and handle user interaction in a
// GLG widget, such as a button or a slider.
//
// GetData() method supplies simulated data for animation. An application 
// should provide a custom implementation of this method to supply real-time
// application data.
//////////////////////////////////////////////////////////////////////////////

// Get a handle to the GLG Toolkit library.
var GLG = new GlgToolkit();

// Debugging aid: uncomment the next line to throw an exception on a GLG error.
GLG.ThrowExceptionOnError( true, true, true );

// Enable debuginng/diagnostics information.
const DEBUG = false;

var DrawingName = "dashboard.g";         // GLG drawing filename. 

const UPDATE_INTERVAL = 100;             // Update rate in msec.
var UpdateTimer = null; 

/* If set to true, tags defined in the drawing are used for animation.
   Otherwise, object resources are used to push real-time values 
   into the drawing.
*/
var USE_TAGS = true;

// Top level viewport of the loaded drawing.   
var Viewport;

/* Coefficients for canvas resolution and text resolution. 
   These parameters will be adjusted for mobile devices with HiDPI displays
   in SetCanvasResolution().
*/
var CoordScale = 1;
var TextScale = 1;

var counter = 0;                       // Used for simulated data.

// Set initial size of the drawing.
SetDrawingSize( false );

/* Increase canvas resolution for mobile devices. Changes CoordScale and
   TextScale.
*/
SetCanvasResolution();

/* Load misc. assets such as GLG scrollbars. When assets are loaded, 
   LoadDrawing is invoked that loads a specified GLG drawing.
*/
LoadAssets( LoadDrawing, null );

function LoadDrawing()
{
    /* Load a drawing from the specified drawing file. 
       The LoadCB callback will be invoked when the drawing has been loaded.
    */
    GLG.LoadWidgetFromURL( DrawingName, null, LoadCB, /*user data*/ null );
}

//////////////////////////////////////////////////////////////////////////////
function LoadCB( /*GlgObject*/ drawing, /*Object*/ user_data, 
                 /*String*/ path )
{
    if( drawing == null )
    {
        AppAlert( "Can't load drawing, check console message for details." );
        return;
    }
    
    // Define the element in the HTML page to display the drawing.
    drawing.SetParentElement( "glg_area" );
    
    // Disable viewport border to use the border of the glg_area.
    drawing.SetDResource( "LineWidth", 0 );
    
    StartGlgDashboard( drawing );
}

//////////////////////////////////////////////////////////////////////////////
function StartGlgDashboard( /*GlgObject*/ drawing )
{
    Viewport = drawing;
    
    // Initialization before hierarchy setup.
    InitBeforeH();

    // Setup object hierarchy in the drawing.
    Viewport.SetupHierarchy();

    // Initialization after hierarchy setup.
    InitAfterH();

    // Start dynamic updates.
    StartUpdateTimer();

    // Display the drawing in a web page.
    Viewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
// Initialization before hierarchy setup.
//////////////////////////////////////////////////////////////////////////////
function InitBeforeH()
{
    // add Input callback to hadle user interaction.
    Viewport.AddListener( GLG.GlgCallbackType.INPUT_CB, InputCallback );

    // Set initial patameters as needed.
    Viewport.SetDResource( "DialPressure/Low", 0.0 );
    Viewport.SetDResource( "DialVoltage/Low", 0.0 );
    Viewport.SetDResource( "DialAmps/Low", 0.0 );
    Viewport.SetDResource( "SliderPressure/Low", 0.0 );
    
    Viewport.SetDResource( "DialPressure/High", 50.0 );
    Viewport.SetDResource( "DialVoltage/High", 120.0 );
    Viewport.SetDResource( "DialAmps/High", 10.0 );
    Viewport.SetDResource( "SliderPressure/High", 50.0 );

    // If the drawing contains a QuitButton, make it invisible.
    if( Viewport.HasResourceObject( "QuitButton" ) )
        Viewport.SetDResource( "QuitButton/Visibility", 0 );
}

//////////////////////////////////////////////////////////////////////////////
// Initialization after hierarchy setup.
//////////////////////////////////////////////////////////////////////////////
function InitAfterH()
{
    // Place application specific code here as needed.
}

//////////////////////////////////////////////////////////////////////////
function StartUpdateTimer()
{
   UpdateTimer = setTimeout( UpdateDrawing, UPDATE_INTERVAL );
}

//////////////////////////////////////////////////////////////////////////
function StopUpdateTimer()
{
   if( UpdateTimer != null )
   {
      clearTimeout( UpdateTimer );
      UpdateTimer = null;
   }
}

//////////////////////////////////////////////////////////////////////////////
// Animation: obtain new data and push the new values to graphics.
//////////////////////////////////////////////////////////////////////////////
function UpdateDrawing()
{  
    /* Obtain simulated demo data values in a specified range.
       The application should provide a custom implementation
       of the data acquisition interface to obtain real-time
       data values.
    */
    var voltage = GetData( 0.0, 120.0 );
    var current = GetData( 0.0, 10.0 );
    
    if( USE_TAGS ) // Use tags for animation.
    {
        // Push values to the objects using tags defined in the drawing.
        Viewport.SetDTag( "Voltage", voltage, /*if_changed*/ true );
        Viewport.SetDTag( "Current", current, /*if_changed*/ true );
    }
    else // Use resources for animation.
    {
        // Push values to the objects using resource paths.
        Viewport.SetDResourceIf("DialVoltage/Value", voltage, 
                                /*if_changed*/ true );
        Viewport.SetDResourceIf("DialAmps/Value", current, 
                                /*if_changed*/ true );
    }

    // Refresh display.
    Viewport.Update();

    // Restart the update timer.
    StartUpdateTimer();
}

//////////////////////////////////////////////////////////////////////////////
// Handle user interaction as needed.
//////////////////////////////////////////////////////////////////////////////
function InputCallback( /*GlgObject*/ viewport, /*GlgObject*/ message_obj )
{
    var origin = message_obj.GetSResource( "Origin" );
    var format = message_obj.GetSResource( "Format" );
    var action = message_obj.GetSResource( "Action" );
    
    // Handle events from a GLG Button widget.
    if( format == "Button" )
    {
        if( action == "Activate" )  //Push button events.
        {
            // Place code here to handle push button events.
        }
        else if( action == "ValueChanged" ) //Toggle button events.
        {
            if( origin == "StartButton" )
            {
                var value = message_obj.GetDResource( "OnState" );
                  switch (value)
                  {
                  case 0:
                     StopUpdateTimer();
                      break;
                  case 1:
                      StartUpdateTimer();
                      break;
                  default: break;
                  }
               }
        }
        
        // Refresh display.
        viewport.Update();
    }
    
    // Input occurred in a slider named SliderPressure.
    else if( format == "Slider" && origin == "SliderPressure" )
    {
        // Retrieve current slider value from the message object.
        var slider_value = 
            message_obj.GetDResource( "ValueY" );
        
        // Set a data value for a dial control DialPressure.
        viewport.SetDResource( "DialPressure/Value", slider_value);
        viewport.Update();
    }
}

//////////////////////////////////////////////////////////////////////////////
function GetData( /*double*/ low, /*double*/ high )
{
    var
     half_amplitude, center,
     period,
     value,
     alpha;
      
    half_amplitude = ( high - low ) / 2.0;
    center = low + half_amplitude;
    
    period = 100.0;
    alpha = 2.0 * Math.PI * counter / period;
      
    value = center +
        half_amplitude * Math.sin( alpha ) * Math.sin( alpha / 30.0 );
    
    ++counter;
    return value;
}

//////////////////////////////////////////////////////////////////////////////
// Change drawing size while maintaining width/height aspect ratio.
//////////////////////////////////////////////////////////////////////////////
function SetDrawingSize( next_size )
{
    const ASPECT_RATIO = 700 / 700;
    
    const MIN_WIDTH = 400;
    const MAX_WIDTH = 700;
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
       used for integrated GLG scrolling, such as chart scrolling or
       panning of a drawing with the enabled Pan property. 
       For each loaded scrollbar, the AssetLoaded callback is invoked with 
       the supplied data array parameter.
    */    
    GLG.LoadWidgetFromURL( "scrollbar_h.g", null, AssetLoaded,
                           { name: "scrollbar_h", callback: callback,
                             user_data: user_data } );
    GLG.LoadWidgetFromURL( "scrollbar_v.g", null, AssetLoaded,
                           { name: "scrollbar_v", callback: callback,
                             user_data: user_data } );
}

//////////////////////////////////////////////////////////////////////////////
function AssetLoaded( glg_object, data, path )
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
function GetCurrTime()
{
   return Date.now() / 1000;    // seconds
}   

//////////////////////////////////////////////////////////////////////////////
// Returns simulated data value.
//////////////////////////////////////////////////////////////////////////////
function GetDemoValue( plot_index )
{
    var half_amplitude = ( High - Low ) / 2.0;
    var center = Low + half_amplitude;
    var period = 100.0 * ( 1.0 + plot_index * 2.0 );
    var alpha = 2.0 * Math.PI * counter / period;
    
    var value = center + 
        half_amplitude * Math.sin( alpha ) * Math.sin( alpha / 30.0 );
    
    counter++;
    return value;
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


