//////////////////////////////////////////////////////////////////////////////
// GLG Robot Arm Demo
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

/* Load a drawing from the robot_arm.g file. 
   The LoadCB callback will be invoked when the drawing has been loaded.
*/
GLG.LoadWidgetFromURL( "robot_arm.g", null, LoadCB, null );

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

   StartRobotArmDemo( drawing );
}

//////////////////////////////////////////////////////////////////////////////
// Control variables and constants
//////////////////////////////////////////////////////////////////////////////

// Graphics update interval.
const UPDATE_INTERVAL = 50;    // msec
var viewport;
var timer = null;
var elbow_array = [];
var AutoUpdate = true;

//////////////////////////////////////////////////////////////////////////////
function StartRobotArmDemo( drawing )
{
   viewport = drawing;

   InitializeSimulationData();

   viewport.AddListener( GLG.GlgCallbackType.INPUT_CB, InputCallback );

   AdjustForMobileDevices();
   
   viewport.InitialDraw();

   // Start periodic updates.
   timer = setTimeout( UpdateRobotArm, UPDATE_INTERVAL );
   StartUpdate();
}
   
//////////////////////////////////////////////////////////////////////////////
function UpdateRobotArm()
{
   if( AutoUpdate )
   {
      /* Calculate and set new resource values
         Different elbows of the arm are named "s0" - "s6" in the drawing.
         The controlling parameters of elbows' rotation transformations
         are named "Value". 
      */
      
      // Update all seven elbows of the robot arm.
      for( var i=0; i < elbow_array.length; ++i )
        elbow_array[ i ].Iterate();

      viewport.Update();   // Show changes
   }
   
   // Restart update timer
   timer = setTimeout( UpdateRobotArm, UPDATE_INTERVAL );
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

   if( format== "Button" )         /* Handle button clicks */
   {
      if( action != "Activate" &&      /* Not a push button */
          action != "ValueChanged" )   /* Not a toggle button */
        return;

      if( origin == "Updates" )
      {
         AutoUpdate = ( message_obj.GetDResource( "OnState" ) == 0 ?
                        false : true );
      }
      else if( origin == "WireFrame" )
      {
         SetWireFrame( message_obj.GetDResource( "OnState" ) == 0 ?
                       false : true );
      }
   } 
   else if( format == "Slider" && action == "ValueChanged" )
   {
      /* Slider was moved: stop updates to allow the user to control
         the arm with sliders.
      */
      AutoUpdate = false;
      viewport.SetDResource( "Updates/OnState", 0 );
   }

   if( format != "Window" )
     viewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
function SetWireFrame( wireframe )
{
   viewport.SetDResource( "robot_area/robot_arm/ZSort", wireframe ? 0 : 1 );
   viewport.SetDResource( "robot_area/fill_type",
                          ( wireframe ?
                            GLG.GlgFillType.EDGE : GLG.GlgFillType.FILL_EDGE ) );

   // Update toggle button with the new value.
   viewport.SetDResource( "WireFrame/OnState", wireframe ? 1 : 0 );
   viewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
function StartUpdate()
{
   AutoUpdate = true;      
   viewport.SetDResource( "Updates/OnState", 1 );   // Update the toggle button.
   viewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
function  StopUpdate()
{
   AutoUpdate = false;
   viewport.SetDResource( "Updates/OnState", 0 );   // Update the toggle button.
   viewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
function AdjustForMobileDevices()
{
   if( CoordScale == 1.0 )
     return;   /* Desktop version. */

   /* Increases the width of the panel with slider controls for mobile devices
      with small screens.
   */
   viewport.SetDResource( "CoordScale", 1.5 );
}

//////////////////////////////////////////////////////////////////////////////
// Changes drawing size while maintaining width/height aspect ratio.
//////////////////////////////////////////////////////////////////////////////
function SetDrawingSize( next_size )
{
   const ASPECT_RATIO = 600 / 500;

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
   GLG.SetCanvasScale( coord_scale, 1.5, 1. );
   
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
   // Initialize simulation controlling parameters

   elbow_array[ 0 ] =
     new GlgAnimationValue( viewport, SIN, 0,  70, -0.75, 0.75, "s0/Value" );
   elbow_array[ 1 ] =
     new GlgAnimationValue( viewport, SIN, 0,  60,  -0.75, 0.75, "s1/Value" );
   elbow_array[ 2 ] =
     new GlgAnimationValue( viewport, SIN, 0, 200,  0.0,   1.0,   "s2/Value" );
   elbow_array[ 3 ] =
     new GlgAnimationValue( viewport, SIN, 0, 150,  0.0,   1.0,   "s3/Value" );
   elbow_array[ 4 ] =
     new GlgAnimationValue( viewport, SIN, 0,  30,  0.0,   1.0,   "s4/Value" );
   elbow_array[ 5 ] =
     new GlgAnimationValue( viewport, SIN, 0, 100,  0.0,   1.0,   "s5/Value" );
   elbow_array[ 6 ] =
     new GlgAnimationValue( viewport, SIN, 0,  20,  0.0,   1.0,   "s6/Value" );
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

