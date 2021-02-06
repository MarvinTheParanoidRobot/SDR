//////////////////////////////////////////////////////////////////////////////
// GLG Process Control Demo
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
// Except for the changes to comply with the JavaScript syntax, this source
// is identical to the source code of the corresponding C/C++, Java and C#
// versions of the demo.
//////////////////////////////////////////////////////////////////////////////

// Get a handle to the GLG Toolkit library.
var GLG = new GlgToolkit();

// Debugging aid: uncomment the next line to throw an exception on a GLG error.
//GLG.ThrowExceptionOnError( true, true, true );

// Set initial size of the drawing.
SetDrawingSize( false );

// Increase canvas resoultion for mobile devices.
var CoordScale = SetCanvasResolution();

/* Loads misc. assets used by the program and invokes the LoadDrawing function
   when done.
*/
LoadAssets( LoadDrawing );

//////////////////////////////////////////////////////////////////////////////
function LoadDrawing()
{
    /* Load a drawing from the process.g file. 
       The LoadCB callback will be invoked when the drawing has been loaded.
    */
    GLG.LoadWidgetFromURL( "process.g", null, LoadCB, null );
}

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

   StartProcessDemo( drawing );
}

//////////////////////////////////////////////////////////////////////////////
// Control variables and constants
//////////////////////////////////////////////////////////////////////////////

/* Demonstrates updating the drawing using either tags (true) or 
   resources (false).
*/
const USE_TAGS = true;

/* Demonstrates two ways of processing user clicks on objects:
   - processing actions attached to objects in the input callback (true), or 
   - using simple selection via object names in the selection callback (false).
*/
const USE_ACTIONS = false;

// Graphics update interval.
const UPDATE_INTERVAL = 50;    // msec

var viewport;
var timer = null;
var timer_3D = null;
   
//////////////////////////////////////////////////////////////////////////////
function StartProcessDemo( drawing )
{
   viewport = drawing;

   InitDrawing();
   viewport.InitialDraw();

   // Start periodic updates.
   timer = setTimeout( IterateProcess, UPDATE_INTERVAL );

   // Show 3D pipes after a few seconds.
   timer_3D = setTimeout( Toggle3D, 3000 );
}
   
//////////////////////////////////////////////////////////////////////////////
function InitDrawing()
{
   viewport.SetDResource( "3DPipesToggle/ShadowWidth", 0.0 );
   viewport.SetDResource( "3DPipesToggle/LineWidth", 1.0 );
   viewport.SetGResource( "3DPipesToggle/EdgeColor", 0.7, 0.7, 0.7 );
   
   viewport.SetDResource( "FlowToggle/ShadowWidth", 0.0 );
   viewport.SetDResource( "FlowToggle/LineWidth", 1.0 );
   viewport.SetGResource( "FlowToggle/EdgeColor", 0.7, 0.7, 0.7 );

   AdjustForMobileDevices();

   viewport.AddListener( GLG.GlgCallbackType.INPUT_CB, InputCallback );
   viewport.AddListener( GLG.GlgCallbackType.SELECT_CB, SelectCallback );

   if( FullScreen )
   {
      /* Maintain the aspect ratio regardless of the screen dimensions
         (or orientation on mobile devices).
      */
      viewport.SetDResource( "Stretch", 0 );
      viewport.SetDResource( "PushIn", 1 ); 
      viewport.SetDResource( "YScale", 0.92 );
      viewport.SetDResource( "XScale", 1.08 );
   }
}

//////////////////////////////////////////////////////////////////////////////
function AdjustForMobileDevices()
{
   if( screen.width < 600 )
     // Increase button lengths for small screens to fit labels.
     viewport.SetDResource( "FlowToggle/XScale", 1.2 );

   /* Adjust chart offsets to fit chart labels on mobile devices with 
      canvas scaling.
   */
   if( CoordScale != 1.0 )
   {
      var chart = viewport.GetResourceObject( "ChartVP/Chart" );  /* GlgObject */
      AdjustOffset( chart, "OffsetTop", 10. );
      AdjustOffset( chart, "OffsetLeft", 10. );
      AdjustOffset( chart, "OffsetBottom", -5. );
   }
}

//////////////////////////////////////////////////////////////////////////////
function IterateProcess()
{
   GetProcessData();          // Get new process data.

   UpdateDrawingWithData();   // Update the drawing with the new data.

   // Restart update timer.
   timer = setTimeout( IterateProcess, UPDATE_INTERVAL );
}

//////////////////////////////////////////////////////////////////////////////
// In a real application, this function will query live process data.
//
// This demo uses simulated data to animate the drawing.
//////////////////////////////////////////////////////////////////////////////
function GetProcessData()
{   
   GetSimulatedData();
}

//////////////////////////////////////////////////////////////////////////////
// Updates the drawing with the new data.
//////////////////////////////////////////////////////////////////////////////
function UpdateDrawingWithData()
{
   // The drawing can be updated using either tags or resources.
   if( USE_TAGS )
     UpdateProcessTags();
   else
     UpdateProcessResources();      
   
   viewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
// Updates drawing using tags. Each tag is referenced by name and is set
// to a value of the corresponding variable.
//////////////////////////////////////////////////////////////////////////////
function UpdateProcessTags()
{
   viewport.SetDTag( "SolventValveValue", SolventValve, true );
   viewport.SetDTag( "SteamValveValue", SteamValve, true );
   viewport.SetDTag( "CoolingValveValue", CoolingValve, true );
   viewport.SetDTag( "WaterValveValue", WaterValve, true );
   
   viewport.SetDTag( "SolventFlow", GetFlow( SOLVENT_FLOW ), true );
   viewport.SetDTag( "SteamFlow", GetFlow( STEAM_FLOW ), true );
   viewport.SetDTag( "CoolingFlow", GetFlow( COOLING_FLOW ), true );
   viewport.SetDTag( "WaterFlow", GetFlow( WATER_FLOW ), true );
   
   viewport.SetDTag( "OutFlow", OutFlow, true );

   viewport.SetDTag( "SteamTemperature", SteamTemperature, true );
   viewport.SetDTag( "HeaterTemperature", HeaterTemperature, true );
   viewport.SetDTag( "BeforePreHeaterTemperature",
                     BeforePreHeaterTemperature, true );
   viewport.SetDTag( "PreHeaterTemperature", PreHeaterTemperature, true );
   viewport.SetDTag( "AfterPreHeaterTemperature",
                     AfterPreHeaterTemperature, true );
   viewport.SetDTag( "CoolingTemperature", CoolingTemperature, true );
   
   viewport.SetDTag( "HeaterLevel", HeaterLevel, true );
   viewport.SetDTag( "WaterLevel", WaterLevel, true );
   
   viewport.SetDTag( "HeaterAlarm", HeaterAlarm ? 1.0 : 0.0, true );
   viewport.SetDTag( "WaterAlarm", WaterAlarm ? 1.0 : 0.0, true );
   
   /* Pass if_changed=false to move the chart even if the value did not 
      change. The rest of resources use true to update them only if their 
      values changed.
   */
   viewport.SetDTag( "PlotValueEntryPoint", HeaterTemperature, false );
   
   viewport.SetDTag( "PressureValue", 5.0 * HeaterPressure, true );
}   

//////////////////////////////////////////////////////////////////////////////
// Updates drawing using heirarchical resources. Each resource is
// referenced by a resource path and is set to a value of the
// corresponding variable.
//////////////////////////////////////////////////////////////////////////////
function UpdateProcessResources()
{
   viewport.SetDResourceIf( "SolventValve/Value", SolventValve, true );
   viewport.SetDResourceIf( "SteamValve/Value", SteamValve, true );
   viewport.SetDResourceIf( "CoolingValve/Value", CoolingValve, true );
   viewport.SetDResourceIf( "WaterValve/Value", WaterValve, true );
   
   viewport.SetDResourceIf( "SolventFlow", GetFlow( SOLVENT_FLOW ), true );
   viewport.SetDResourceIf( "SteamFlow", GetFlow( STEAM_FLOW ), true );
   viewport.SetDResourceIf( "CoolingFlow", GetFlow( COOLING_FLOW ), true );
   viewport.SetDResourceIf( "WaterFlow", GetFlow( WATER_FLOW ), true );
   
   viewport.SetDResourceIf( "OutFlow", OutFlow, true );
   
   viewport.SetDResourceIf( "Heater/SteamTemperature", SteamTemperature, true );
   viewport.SetDResourceIf( "Heater/HeaterTemperature",
                            HeaterTemperature, true );
   viewport.SetDResourceIf( "BeforePreHeaterTemperature",
                            BeforePreHeaterTemperature, true );
   viewport.SetDResourceIf( "PreHeaterTemperature", PreHeaterTemperature, true );
   viewport.SetDResourceIf( "AfterPreHeaterTemperature",
                            AfterPreHeaterTemperature, true );
   viewport.SetDResourceIf( "CoolingTemperature", CoolingTemperature, true );
      
   viewport.SetDResourceIf( "Heater/HeaterLevel", HeaterLevel, true );
   viewport.SetDResourceIf( "WaterSeparator/WaterLevel", WaterLevel, true );
   
   viewport.SetDResourceIf( "HeaterAlarm", HeaterAlarm ? 1.0 : 0.0, true );
   viewport.SetDResourceIf( "WaterAlarm", WaterAlarm ? 1.0 : 0.0, true );
   
   /* Pass if_changed=false to move the chart even if the value did not 
      change. The rest of resources use true to update them only if their 
      values changed.
   */
   viewport.SetDResourceIf( "ChartVP/Chart/Plots/Plot#0/ValueEntryPoint", 
                            HeaterTemperature, false );
   
   viewport.SetDResourceIf( "PressureGauge/Value", 5.0 * HeaterPressure, true );
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
   // var subaction = message_obj.GetSResource( "SubAction" );

   if( format == "Button" )
   {	 
      if( action != "ValueChanged" )
        return;

      var _3D_pipes_visible =
        ( viewport.GetDResource( "3DPipesToggle/OnState" ) != 0.0 );
      var flow_lines_visible =
        ( viewport.GetDResource( "FlowToggle/OnState" ) != 0.0 );

      if( origin == "3DPipesToggle" )
      {
         /* Make flow lines visible if both 3D pipes and flow lines were 
            turned off.
         */
         if( !_3D_pipes_visible && !flow_lines_visible )
         {
            viewport.SetDResource( "FlowToggle/OnState", 1.0 );
            viewport.Update();
         }

         Clear3DTimer();
      }
      else if( origin == "FlowToggle" )
      {
         /* Make 3D pipes visible if both flow lines and 3D pipes were
            turned off.
         */
         if( !_3D_pipes_visible && !flow_lines_visible )
         {
            viewport.SetDResource( "3DPipesToggle/OnState", 1.0 );
            viewport.Update();
         }

         Clear3DTimer();
      }
      else if( origin == "ToggleAutoScroll" )
      {
         /* Activate chart's X pan slider when AutoScroll=OFF.
            The toggle is connected to the chart's AutoScroll and controls 
            it. The X pan slider is activated here.
         */
         var auto_scroll =
           ( viewport.GetDResource( "ChartVP/Chart/AutoScroll" ) != 0.0 );
         viewport.SetDResource( "ChartVP/Pan",
                                ( auto_scroll ? GLG.GlgPanType.PAN_Y_AUTO :
                                  GLG.GlgPanType.PAN_X | GLG.GlgPanType.PAN_Y ) );

         if( auto_scroll )
         {
            /* Reset the chart's ranges when returning to auto-scroll. */
            viewport.SetDResource( "ChartVP/Chart/Plots/Plot#0/YLow", 50.0 );
            viewport.SetDResource( "ChartVP/Chart/Plots/Plot#0/YHigh", 150.0 );
         }
         viewport.Update();
      }
   }
   else if( format == "CustomEvent" )
   {
      /* If USE_ACTIONS=false, user clicks on objects are processed using
         simple selection via object names in the Select callback.
      */
      if( !USE_ACTIONS )
        return;
         
      /* Handle custom event actions attached to valves to open or close 
         them when the user clicks on them with the left or right mouse 
         button.
      */
      var event_label = message_obj.GetSResource( "EventLabel" );
      var button = message_obj.GetDResource( "ButtonIndex" );
         
      var increment;
      if( button == 1 )
        increment = 1.0;
      else
        increment = -1.0;
      
      if( event_label == "SolventValveClick" )
      { 
         SolventValve += 0.2 * increment;
         SolventValve = PutInRange( SolventValve, 0.0, 1.0 );
         viewport.SetDResource( "SolventValve/Value", SolventValve );
      }
      else if( event_label == "SteamValveClick" )
      {
         SteamValve += 0.2 * increment;
         SteamValve = PutInRange( SteamValve, 0.0, 1.0 );
         viewport.SetDResource( "SteamValve/Value", SteamValve );
      }
      else if( event_label == "CoolingValveClick" )
      {
         CoolingValve += 0.2 * increment;
         CoolingValve = PutInRange( CoolingValve, 0.0, 1.0 );
         viewport.SetDResource( "CoolingValve/Value", CoolingValve );
      }
      else if( event_label == "WaterValveClick" )
      {
         WaterValve += 0.2 * increment;
         WaterValve = PutInRange( WaterValve, 0.0, 1.0 );
         viewport.SetDResource( "WaterValve/Value", WaterValve );
      }
      /* Erase or display the pressure gauge when the gauge or the heater
         are clicked on.
      */
      else if( event_label == "HeaterClick" ||
               event_label == "PressureGaugeClick" )
      {
         var visibility = viewport.GetDResource( "PressureGauge/Visibility" );
         viewport.SetDResource( "PressureGauge/Visibility",
                                visibility == 0 ? 1.0 : 0.0 );
      }
      viewport.Update();
   }
   else if( format == "Timer" )   // Handles timer transformations.
     viewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
// Selection callback that may be used as an alternative way to handle
// mouse selection, such open or close valves on a mouse click, by
// processing names of objects selected by the mouse click.
//
// InputCallback uses a more elaborate alternative that handles custom
// actions attached to objects in the drawing, which does not rely
// on object names.
//////////////////////////////////////////////////////////////////////////////
function SelectCallback( vp, name_array, button )
{
   var
     visibility,
     increment;
   var name;

   /* If USE_ACTIONS=true, Input callback is used to process user clicks
      via actions attached to objects, instead of using the Select callback.
   */
   if( USE_ACTIONS )
     return;
   
   // Process user clicks on objects using simple selection via object names.
   if( name_array == null )
     return;
   
   if( button == 1 )
     increment = 1.0;
   else
     increment = -1.0;
   
   for( var i=0; ( name = name_array[i] ) != null; ++i )
   {
      if( name == "SolventValve" )
      { 
         SolventValve += 0.2 * increment;
         SolventValve = PutInRange( SolventValve, 0.0, 1.0 );
         viewport.SetDResource( "SolventValve/Value", SolventValve );
         break;
      }
      else if( name == "SteamValve" )
      {
         SteamValve += 0.2 * increment;
         SteamValve = PutInRange( SteamValve, 0.0, 1.0 );
         viewport.SetDResource( "SteamValve/Value", SteamValve );
         break;
      }
      else if( name == "CoolingValve" )
      {
         CoolingValve += 0.2 * increment;
         CoolingValve = PutInRange( CoolingValve, 0.0, 1.0 );
         viewport.SetDResource( "CoolingValve/Value", CoolingValve );
         break;
      }
      else if( name == "WaterValve" )
      {
         WaterValve += 0.2 * increment;
         WaterValve = PutInRange( WaterValve, 0.0, 1.0 );
         viewport.SetDResource( "WaterValve/Value", WaterValve );
         break;
      }
      else if( name.indexOf( "Heater" ) == 0 ||
               name.indexOf( "PressureGauge" ) == 0 )
      {
         visibility = viewport.GetDResource( "PressureGauge/Visibility" );
         viewport.SetDResource( "PressureGauge/Visibility",
                                visibility == 0.0 ? 1.0 : 0.0 );
         break;
      }
   }
   viewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
// Toggles 3D pipes when the "Toggle 3D" HTML button is pressed.
//////////////////////////////////////////////////////////////////////////////
function Toggle3D()
{      
   var _3D_pipes_visible =
     ( viewport.GetDResource( "3DPipesToggle/OnState" ) != 0.0 );
   var flow_lines_visible =
     ( viewport.GetDResource( "FlowToggle/OnState" ) != 0.0 );

   // Toggle 3D pipes display.
   _3D_pipes_visible = ( _3D_pipes_visible == 1 ? 0 : 1 );
   viewport.SetDResource( "3DPipesToggle/OnState", _3D_pipes_visible );
   
   // Make flow lines visible if both 3D pipes and flow lines were turned off.
   if( !_3D_pipes_visible && !flow_lines_visible )
     viewport.SetDResource( "FlowToggle/OnState", 1.0 );
   
   viewport.Update();

   Clear3DTimer();
}

//////////////////////////////////////////////////////////////////////////////
// Clears the timer used to demo 3D pipes.
//////////////////////////////////////////////////////////////////////////////
function Clear3DTimer()
{
   if( timer_3D == null )
     return;

   clearTimeout( timer_3D );
   timer_3D = null;
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
   const ASPECT_RATIO = 700 / 600;
   
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
   GLG.SetCanvasScale( coord_scale, 1.5, 0.6 );
   
   // Mobile devices use fixed device-width: disable Change Drawing Size button.
   var change_size_button = document.getElementById( "change_size" );
   if( change_size_button != null )
     change_size_button.parentNode.removeChild( change_size_button );

   return coord_scale;      // Chosen coord scale for mobile devices.
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

   // Invoke the callback after all assets have been loaded.
   if( AssetLoaded.num_loaded == 2 )
     data.callback();
}

//////////////////////////////////////////////////////////////////////////////
// SIMULATION ONLY
// All code below is used only to animate the demo with simulated data.
// In a real application, live process data will be queried and used
// to update the drawing.
//////////////////////////////////////////////////////////////////////////////

// Constants that define simulation parameters.
const
   PROCESS_SPEED = 0.05,
   HEATER_LEVEL_SPEED = 0.05,
   WATER_LEVEL_SPEED = 0.02,
   VALVE_CHANGE_SPEED = 0.05,
   STEAM_VALVE_CHANGE_SPEED = 0.05;

const
   SOLVENT_FLOW = 0,
   STEAM_FLOW = 1,
   COOLING_FLOW = 2,
   WATER_FLOW = 3;
   
// Variables used in the process simulation.
var
   WaterAlarm = false,
   HeaterAlarm = false;
   
var
   ProcessCounter = 0,
   heater_high = 0,
   heater_low = 0,
   water_high = 0,
   water_low = 0,
   steam_high = 0,
   steam_low = 0,
   cooling_high = 0,
   cooling_low = 0;
   
var
   SolventValve = 0.85,
   SteamValve = 1.0,
   CoolingValve = 0.8,
   WaterValve = 0.4,
   SolventFlow = 0.0,
   SteamFlow = 0.0,
   CoolingFlow = 0.0,
   WaterFlow = 0.0,
   OutFlow = 3495.0,
   SteamTemperature = 0.0,
   HeaterTemperature = 0.0,
   BeforePreHeaterTemperature = 0.0,
   PreHeaterTemperature = 0.0,
   AfterPreHeaterTemperature = 0.0,
   CoolingTemperature = 0.0,
   HeaterPressure = 0.0,
   HeaterLevel = 0.5,
   WaterLevel = 0.1;
   
//////////////////////////////////////////////////////////////////////////////
// SIMULATION ONLY
// Recalculate simulated values used to animate the demo.
//////////////////////////////////////////////////////////////////////////////
function GetProcessData()
{
   ++ProcessCounter;
   if( ProcessCounter == 0x7fffffff )
     ProcessCounter = 0;
   
   SteamTemperature += ( SteamValve - 0.6 ) * 2 * PROCESS_SPEED;
   SteamTemperature = PutInRange( SteamTemperature, 0.0, 1.0 );
   
   HeaterTemperature +=
     ( SteamTemperature - HeaterTemperature * HeaterLevel ) * PROCESS_SPEED;
   HeaterTemperature = PutInRange( HeaterTemperature, 0.0, 1.5 );
   
   BeforePreHeaterTemperature +=
     ( 1.5 * HeaterTemperature - BeforePreHeaterTemperature ) * PROCESS_SPEED;
   BeforePreHeaterTemperature =
     PutInRange( BeforePreHeaterTemperature, 0.0, 1.0 );
         
   PreHeaterTemperature +=
     ( BeforePreHeaterTemperature - PreHeaterTemperature ) * PROCESS_SPEED;
   PreHeaterTemperature = PutInRange( PreHeaterTemperature, 0.0, 1.0 );
   
   AfterPreHeaterTemperature +=
     ( 0.9 * HeaterTemperature - AfterPreHeaterTemperature ) *
     PROCESS_SPEED ;
   AfterPreHeaterTemperature =
     PutInRange( AfterPreHeaterTemperature, 0.0, 1.0 );
         
   CoolingTemperature +=
     ( AfterPreHeaterTemperature - CoolingTemperature - CoolingValve )
     * PROCESS_SPEED;
   CoolingTemperature = PutInRange( CoolingTemperature, 0.0, 1.0 );
         
   OutFlow = SolventValve * 3495.0;
         
   HeaterLevel += ( SolventValve - 0.75 ) * HEATER_LEVEL_SPEED;
   HeaterLevel = PutInRange( HeaterLevel, 0.0, 1.0 );
         
   // Inversed
   WaterLevel += ( 0.5 - WaterValve ) * WATER_LEVEL_SPEED;
   WaterLevel = PutInRange( WaterLevel, 0.0, 1.0 );
         
   if( HeaterLevel > 0.9 || heater_high != 0 )
   {
      heater_high = LagVar( heater_high, 10 );
      SolventValve -= VALVE_CHANGE_SPEED;
   }
   else if( HeaterLevel < 0.45 || heater_low != 0 )
   {
      heater_low = LagVar( heater_low, 10 );
      SolventValve += VALVE_CHANGE_SPEED;
   }
   SolventValve = PutInRange( SolventValve, 0.0, 1.0 );
         
   // Inversed
   if( WaterLevel > 0.2 || water_high != 0 )
   {
      water_high = LagVar( water_high, 10 );
      WaterValve += VALVE_CHANGE_SPEED;
   }
   else if( WaterLevel < 0.05 || water_low != 0 )
   {
      water_low = LagVar( water_low, 10 );
      WaterValve -= VALVE_CHANGE_SPEED;
   }
   WaterValve = PutInRange( WaterValve, 0.0, 1.0 );
         
   if( SteamTemperature > 0.9 || steam_high != 0 )
   {
      LagVar( steam_high, 20 );
      SteamValve -= STEAM_VALVE_CHANGE_SPEED;
   }
   else if( SteamTemperature < 0.2 || steam_low != 0 )
   {
      LagVar( steam_low, 20 );
      SteamValve += STEAM_VALVE_CHANGE_SPEED;
   }
   SteamValve = PutInRange( SteamValve, 0.0, 1.0 );
         
   if( CoolingTemperature > 0.7 || cooling_high != 0 )
   {
      LagVar( cooling_high, 10 );
      CoolingValve += VALVE_CHANGE_SPEED;
   }
   else if( CoolingTemperature < 0.3 || cooling_low != 0 )
   {
      LagVar( cooling_low, 10 );
      CoolingValve -= VALVE_CHANGE_SPEED;
   }
   CoolingValve = PutInRange( CoolingValve, 0.0, 1.0 );
         
   HeaterPressure = HeaterLevel * ( HeaterTemperature + 1.0 ) / 2.0;
   HeaterPressure = PutInRange( HeaterPressure, 0.0, 1.0 );
         
   HeaterAlarm = ( HeaterLevel < 0.45 || HeaterLevel > 0.9 );
   WaterAlarm = ( WaterLevel > 0.2 || WaterLevel < 0.05 );
}

//////////////////////////////////////////////////////////////////////////////
// SIMULATION ONLY
// Returns the flow value, which is later used as a line type value used 
// to simulate liquid flow.
//////////////////////////////////////////////////////////////////////////////
function GetFlow( type )
{
   if( type == SOLVENT_FLOW )
     return SolventFlow = GetFlowValue( SolventFlow, SolventValve );
   else if( type == STEAM_FLOW )
     return SteamFlow = GetFlowValue( SteamFlow, SteamValve );
   else if( type == COOLING_FLOW )
     return CoolingFlow = GetFlowValue( CoolingFlow, CoolingValve );
   else if( type == WATER_FLOW )
     return WaterFlow = GetFlowValue( WaterFlow, WaterValve );
   else
     return 0.0;
}
   
//////////////////////////////////////////////////////////////////////////////
// SIMULATION ONLY
// Recalculates the line type values used to simulate liquid flow based
// on the previous line type value and a flow speed defined by the valve
// opening.
// Parameters:
//     state - last value of the line type
//     valve - current valve opening
//
// Shifting the line type pattern's offset is achieved by increasing the
// line type value by 32.0 Refer to the documentation of the polygon's 
// LineType resource for more details.   
// Alternatively, the flow line widget from the Custom Object palette
// may be used for integrated flow line functionality, in which case
// this code is not needed. 
//////////////////////////////////////////////////////////////////////////////
function GetFlowValue( state, valve )
{
   var
     value,
     update_interval;
   const
     FLOW_LINE_TYPE = 24,
     NO_FLOW_LINE_TYPE = 0,
     MAX_FLOW = 3;
      
   if( valve == 0 )
     value = NO_FLOW_LINE_TYPE;     // Valve is closed - no flow.
   else
   {
      if( state == 0.0 )
        value = FLOW_LINE_TYPE;    // First time: init to FLOW_LINE_TYPE.
      else
      {
         // Skip a few intervals to represent variable flow speed.
         update_interval = MAX_FLOW - Math.trunc( ( valve + 0.1 ) * MAX_FLOW );
         update_interval = Math.min( 0, update_interval );
         update_interval = Math.max( MAX_FLOW, update_interval );
         if( update_interval == 0 ||
             ( ProcessCounter % update_interval ) == 0 )
         {
            /* Add 32 to the line type value to increase the line type 
               pattern's offset by 1.
            */
            value = state + 32;
            
            /* Reset periodically at the end of the pattern to prevent 
               overflow. Since the length of the GDI pattern is 24 and 
               the length of the OpenGL pattern is 16, reset after 24 * 16 
               iterations to handle both.
            */
            if( value == FLOW_LINE_TYPE + 32 * 24 * 16 )
              value = FLOW_LINE_TYPE;
         }
         else
           // No change: skipping a few intervals to show a slow speed.
           value = state;
      }
   }
   
   return value;
}
   
//////////////////////////////////////////////////////////////////////////////
// SIMULATION ONLY
// Helps to implement lag behavior
//////////////////////////////////////////////////////////////////////////////
function LagVar( variable, lag )
{
   if( variable != 0 )
     return --variable;
   else
     return lag;
}

//////////////////////////////////////////////////////////////////////////////
// SIMULATION ONLY
//////////////////////////////////////////////////////////////////////////////
function PutInRange( variable, low, high )
{
   if( variable < low )
     return low;
   else if( variable > high )
     return high;
   else
     return variable;
}
