//////////////////////////////////////////////////////////////////////////////
// GLG Controls Demo
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
//////////////////////////////////////////////////////////////////////////////

// Get a handle to the GLG Toolkit library.
var GLG = new GlgToolkit();

// Debugging aid: uncomment the next line to throw an exception on a GLG error.
//GLG.ThrowExceptionOnError( true, true, true );

/* Load a drawing from the dashboard.g file. 
   The LoadCB callback will be invoked when the drawing has been loaded.
*/
GLG.LoadWidgetFromURL( "dashboard.g", null, LoadCB, null );

// Set initial size of the drawing.
SetDrawingSize( false );

// Increase canvas resoultion for mobile devices.
var CoordScale = SetCanvasResolution();

//////////////////////////////////////////////////////////////////////////////
function LoadCB( drawing, data, path )
{
   if( drawing == null )
   {
      window.alert( "Can't load drawing, check console message for details." );
      return;
   }
   
   // Define the element in the HTML page to display the drawing in.
   drawing.SetParentElement( "glg_area" );
   
   // Disable viewport border to use the border of the glg_area.
   drawing.SetDResource( "LineWidth", 0 );

   StartDashboardDemo( drawing );
}

//////////////////////////////////////////////////////////////////////////////
// Control variables and constants
//////////////////////////////////////////////////////////////////////////////

// Graphics update interval.
const UPDATE_INTERVAL = 50;    // msec
var viewport;
var timer = null;
var animation_array = [];
var PerformUpdates = true;

//////////////////////////////////////////////////////////////////////////////
function StartDashboardDemo( drawing )
{
   viewport = drawing;

   /* Disable manual mode by default. Manual mode may be used to change
      dial values by dragging their needles with the mouse or touch.
   */
   viewport.SetDResource( "ManualModeButton/OnState", 0 );
   PerformUpdates = true;
   
   InitializeSimulationData();

   AdjustForMobileDevices();
    
   viewport.AddListener( GLG.GlgCallbackType.INPUT_CB, InputCallback );

   viewport.InitialDraw();

   // Start periodic updates.
   timer = setTimeout( UpdateControls, UPDATE_INTERVAL );
}
   
//////////////////////////////////////////////////////////////////////////////
function UpdateControls()
{
   if( PerformUpdates )
   {
      // Update all animation_values
      for( var i=0; i < animation_array.length; ++i )
        if( animation_array[ i ] != null )
          animation_array[ i ].Iterate();

      var temp = animation_array[ TEMP ].last_value;
      var fuel = animation_array[ FUEL ].last_value;
      var charge = animation_array[ BATT ].last_value;
      var rpm = animation_array[ RPM ].last_value;

      /* Set state of the System Status indicator. */
      var warning = ( temp >= 115 || fuel <= 10 || charge < 10 || rpm >= 8000 );
      viewport.SetDResourceIf( "SystemStatusIndicator/OnState",
                               warning ? 0 : 1, true );

      /* Push RPM and torque data into the history chart. */
      viewport.SetDResource( "HistoryChart/Chart/Plots/RPMEntryPoint", rpm );

      /* Simulate torque values. */
      var torque = rpm * rpm / 125000
      viewport.SetDResource( "HistoryChart/Chart/Plots/TorqueEntryPoint",
                             torque );
      viewport.Update();   // Show changes
   }
   
   // Restart update timer
   timer = setTimeout( UpdateControls, UPDATE_INTERVAL );
}

//////////////////////////////////////////////////////////////////////////////
// Handles user input: in this demo, it handles clicks on the start and stop 
// buttons.
//////////////////////////////////////////////////////////////////////////////
function InputCallback( vp, message_obj )
{
   var origin = message_obj.GetSResource( "Origin" );
   var format = message_obj.GetSResource( "Format" );
   var action = message_obj.GetSResource( "Action" );
   // var subaction = message_obj.GetSResource( "SubAction" );

   if( format == "Button" )
   {
      if( action != "Activate" && action != "ValueChanged" )
          return;

      if( origin == "ManualModeButton" )
      {
         var manual_mode = vp.GetDResource( "ManualModeButton/OnState" );
         PerformUpdates = !manual_mode;
      }
   }
   
   if( format != "Window" )
     viewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
// Changes drawing size while maintaining width/height aspect ratio.
//////////////////////////////////////////////////////////////////////////////
function SetDrawingSize( next_size )
{
   const ASPECT_RATIO = 1 / 1;

   // Settings for desktop displays.
   const MIN_WIDTH = 400;
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
   var coord_scale = 2.0;
   GLG.SetCanvasScale( coord_scale, 1.75, 0.6 );
   
   // Mobile devices use fixed device-width: disable Change Drawing Size button.
   var change_size_button = document.getElementById( "change_size" );
   if( change_size_button != null )
     change_size_button.parentNode.removeChild( change_size_button );

   return coord_scale;      // Chosen coord scale for mobile devices.
}

//////////////////////////////////////////////////////////////////////////////
function AdjustForMobileDevices()
{
   if( CoordScale == 1.0 )
     return;   /* Desktop version. */

   /* Adjust chart offsets to make more space for the title and axes labels
      on mobile devices with canvas scaling.
   */
   AdjustOffset( viewport, "HistoryChart/Chart/OffsetTop", 5. );
   AdjustOffset( viewport, "HistoryChart/Chart/OffsetLeft", 15. );
   AdjustOffset( viewport, "HistoryChart/Chart/OffsetRight", -15. );
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
// SIMULATION ONLY
// All code below is used only to animate the demo with simulated data.
// In a real application, live data will be queried and used to update
// the drawing.
//////////////////////////////////////////////////////////////////////////////

const INCR = 0;
const SIN = 1;
const RANDOM = 2;
const RANDOM_INT = 3;

const RPM  = 1;
const FUEL = 2;
const TEMP = 3;
const BATT = 4;

//////////////////////////////////////////////////////////////////////////////
function InitializeSimulationData()
{
    const k = 1;   // Speed factor

    // Initialize simulation controlling parameters
   
   animation_array[ 0 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 47 * k, 20.0, 90.0, "SpeedDial/Value" );
   animation_array[ 1 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 63 * k, 1000.0, 5000.0, "RPMDial/Value" );
   /* Inversed : decrese with time */
   animation_array[ 2 ] =
     new GlgAnimationValue( viewport, INCR,
                            0, 30 * k, 100, 0, "SpeedDial/FuelLevel" );
   animation_array[ 3 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 91 * k, 90, 122, "RPMDial/Temperature" );
   animation_array[ 4 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 59 * k, 30, 80, "BatteryChargeGauge/Value" );   
}

//////////////////////////////////////////////////////////////////////////////
function GlgAnimationValue( viewport, type, counter, period, min, max, name )
{
   this.viewport = viewport;
   this.type = type;
   this.counter = counter;
   this.period = period;
   this.min = min;
   this.max = max;
   this.name = name;
   this.last_value = null; 
}

//////////////////////////////////////////////////////////////////////////////
GlgAnimationValue.prototype.Iterate = function ()
{
   var angle, value;
    
   if( this.period < 1 )
   {
      GLG.Error( GLG.GlgErrorType.USER_ERROR, "Invalid this.period.", null );
      return;
   }

   switch( this.type )
   {
    case SIN:
      angle = Math.PI * this.counter / this.period;	 
      value = this.min + ( this.max - this.min ) * Math.sin( angle );
      this.last_value = value;
    break;

    case INCR:
      value = this.min +
         ( this.counter / ( this.period - 1 ) ) * ( this.max - this.min );
      this.last_value = value;
      break;

    case RANDOM:
      if( this.last_value == null || this.counter == this.period - 1 )
      {
         value = GLG.Rand( this.min, this.max );
         this.last_value = value;
      }
      else
        value = this.last_value;
      break;

    case RANDOM_INT:
      if( this.last_value == null || this.counter == this.period - 1 )
      {
         value = Math.floor( GLG.Rand( this.min, this.max ) );
         this.last_value = value;
      }
      else
        value = this.last_value;
      break;

    default:
      GLG.Error( GLG.GlgErrorType.USER_ERROR, "Invalid animation type." );
      value = 0.0;
      this.last_value = value;
      break;
   }

   viewport.SetDResource( this.name, value );

   // Increment counter
   ++this.counter;
   if( this.counter >= this.period )
       this.counter = 0;    // Reset counter
}
