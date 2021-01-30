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

/* Load a drawing from the controls.g file. 
   The LoadCB callback will be invoked when the drawing has been loaded.
*/
GLG.LoadWidgetFromURL( "controls.g", null, LoadCB, null );

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

   StartControlsDemo( drawing );
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
function StartControlsDemo( drawing )
{
   viewport = drawing;

   /* Enable touch dragging for sliders and knobs only for the mobile version
      to avoid interfering with page scrolling and zooming.
      It is enabled by default.
   */
   viewport.SetDResource( "$config/GlgSliderTouchDrag",
                          ( MobileVersion ? 1 : 0 ) );

   InitializeSimulationData();

   AdjustForMobileDevices();
   
   viewport.AddListener( GLG.GlgCallbackType.INPUT_CB, InputCallback );

   viewport.InitialDraw();

   // Start periodic updates.
   timer = setTimeout( UpdateControls, UPDATE_INTERVAL );
   StartUpdate();
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

   if( action == "ValueChanged" )
   {
      /* Stop automatic updates to let the user play with controls
         when a control's value is adjusted with the mouse.
      */
      StopUpdate();
   }
   else if( format == "Button" )
   {
      if( action != "Activate" )
        return;
         
      if( origin == "Start" )
        StartUpdate();
      else if( origin == "Stop" )
        StopUpdate();
   }

   if( format != "Window" )
     viewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
function StartUpdate()
{
   PerformUpdates = true;
   viewport.SetDResource( "ManualMode", 0 );
}

//////////////////////////////////////////////////////////////////////////////
function StopUpdate()
{
   PerformUpdates = false;
   viewport.SetDResource( "ManualMode", 1 );
}

//////////////////////////////////////////////////////////////////////////////
function AdjustForMobileDevices()
{
   if( CoordScale == 1.0 )
     return;   /* Desktop version. */
   
   /* Adjust joystick offsets to make more space for the moving X/Y labels
      on mobile devices with canvas scaling.
   */
   AdjustOffset( viewport, "Joystick/OffsetLeft", 10. );
   AdjustOffset( viewport, "Joystick/OffsetBottom", -5. );   
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
// Changes drawing size while maintaining width/height aspect ratio.
//////////////////////////////////////////////////////////////////////////////
function SetDrawingSize( next_size )
{
   const ASPECT_RATIO = 750 / 600;

   // Settings for desktop displays.
   const MIN_WIDTH = 600;
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
// SIMULATION ONLY
// All code below is used only to animate the demo with simulated data.
// In a real application, live data will be queried and used to update
// the drawing.
//////////////////////////////////////////////////////////////////////////////

const INCR = 0;
const SIN = 1;
const RANDOM = 2;
const RANDOM_INT = 3;

//////////////////////////////////////////////////////////////////////////////
function InitializeSimulationData()
{
    const k = 3;   // Speed factor

    // Initialize simulation controlling parameters
   
   /* 3 top gages */
   animation_array[ 0 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 47 * k, 50.0, 180.0, "Dial1/Value" );
   animation_array[ 1 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 63 * k, 30.0, 90.0, "Dial2/Value" );
   animation_array[ 2 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 91 * k, 1.2, 3.5, "Dial3/Value" );

   /* Two small gauges in the middle. */
   animation_array[ 3 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 36 * k, 2.0, 8.0, "Dial4/Value" );
   animation_array[ 4 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 59 * k, 2.0, 8.0, "Dial5/Value" );

   /* Sliders */
   animation_array[ 5 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 75 * k, 0.2, 0.8, "Slider1/ValueY" );
   animation_array[ 6 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 45 * k, 0.2, 0.8, "Slider2/ValueY" );
   animation_array[ 7 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 84 * k, 0.2, 0.8, "Slider3/ValueY" );
   animation_array[ 8 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 65 * k, 0.2, 0.8, "Slider4/ValueY" );
   animation_array[ 9 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 32 * k, 0.2, 0.8, "Slider5/ValueY" );
   animation_array[ 10 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 58 * k, 0.2, 0.8, "Slider6/ValueY" );

   /* Thermometer */
   animation_array[ 11 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 80 * k, -20.0, 105.0, "Thermometer/ValueY" );

   /* 3 knobs to the right of the thermometer */
   animation_array[ 12 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 46 * k, 5.0, 80.0, "Knob1/Value" );
   animation_array[ 13 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 65 * k, 1.0, 9.0, "Knob2/Value" );
   animation_array[ 14 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 38 * k, 1.0, 9.0, "Knob3/Value" );

   /* 2 switches */
   animation_array[ 15 ] =
     new GlgAnimationValue( viewport, RANDOM_INT,
                            0, 5, 0.0, 3.0, "Switch1/ValueX" );
   animation_array[ 16 ] =
     new GlgAnimationValue( viewport, RANDOM_INT,
                            0, 5, 0.0, 2.0, "Switch2/OnState" );

   /* Lights */
   animation_array[ 17 ] =
     new GlgAnimationValue( viewport, RANDOM,
                            0, 5, 0.0, 3.0, "Light1/Value" );
   animation_array[ 18 ] =
     new GlgAnimationValue( viewport, RANDOM,
                            0, 5, 0.0, 3.0, "Light2/Value" );
   animation_array[ 19 ] =
     new GlgAnimationValue( viewport, RANDOM,
                            0, 5, 0.0, 3.0, "Light3/Value" );

   /* Toggle switches */
   animation_array[ 20 ] =
     new GlgAnimationValue( viewport, RANDOM_INT,
                            0, 5, 0.0, 1.99, "Toggle1/OnState" );
   animation_array[ 21 ] =
     new GlgAnimationValue( viewport, RANDOM_INT,
                            0, 5, 0.0, 1.99, "Toggle2/OnState" );
   animation_array[ 22 ] =
     new GlgAnimationValue( viewport, RANDOM_INT,
                            0, 5, 0.0, 1.99, "Toggle3/OnState" );

   /* Joystick */
   animation_array[ 23 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 23 * k, 0.2, 0.8, "Joystick/ValueX" );
   animation_array[ 24 ] =
     new GlgAnimationValue( viewport, SIN,
                            0, 19 * k, 0.2, 0.8, "Joystick/ValueY" );
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
      break;

    case INCR:
      value = this.min +
         ( this.counter / ( this.period - 1 ) ) * ( this.max - this.min );
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
      break;
   }

   viewport.SetDResource( this.name, value );

   // Increment counter
   ++this.counter;
   if( this.counter >= this.period )
       this.counter = 0;    // Reset counter
}
