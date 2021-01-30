//////////////////////////////////////////////////////////////////////////////
// GLG Graph Demo
//
// The demo is written in pure HTML5 and JavaScript. The source code of the
// demo uses the GLG Toolkit JavaScript Library supplied by the included
// Glg*.js and GlgToolkit*.js files.
//
// The library load GLG drawings containing 2D and 3D Graphs and renders
// them on a web page, providing an API to animate the graphs with
// real-time data and modifying graphs' parameters.
//
// In addition to controlling the graphs via the GLG API at run time,
// the GLG Graphics Builder can be used to set parameters of the graphs
// interactively, as well as to create panels containing multiple graphs
// together with dials, toggles and other custom graphical objects.
//////////////////////////////////////////////////////////////////////////////

// Get a handle to the GLG Toolkit library.
var GLG = new GlgToolkit();

// Debugging aid: uncomment the next line to throw an exception on a GLG error.
//GLG.ThrowExceptionOnError( true, true, true );

// Set initial size of the drawing.
SetDrawingSize( false );

// Increase canvas resoultion for mobile devices.
SetCanvasResolution();

// Graph types
const BAR = 0;
const BAR_3D = 1;
const LINE = 2;
const RIBBON = 3;

const UPDATE_INTERVAL = 100;   // msec

var GraphType = -1;
var Graph;
var UpdateAfterEachSample = true;
var PerformUpdates = true;
var UpdateTimer;
var label_counter = 0;
var update_counter = 0;

// Load BAR graph.
LoadGraph( BAR );

//////////////////////////////////////////////////////////////////////////
function LoadGraph( type )
{
   if( GraphType == type )
     return;
   
   var graph_drawing;   
   switch( type )
   {
    case BAR:    graph_drawing = "bar1.g"; break;
    case BAR_3D: graph_drawing = "bar101.g"; break;
    case LINE:   graph_drawing = "line1.g"; break;
    case RIBBON: graph_drawing = "line101.g"; break;
    default: window.alert( "Invalid grapg type" );
   }

   /* Load a drawing from a requested file. 
      The LoadCB callback will be invoked when the drawing has been loaded.
   */
   GLG.LoadWidgetFromURL( graph_drawing, null, LoadCB, type );
}

//////////////////////////////////////////////////////////////////////////////
function LoadCB( drawing, new_type, path )
{
   if( drawing == null )
   {
      window.alert( "Can't load drawing, check console message for details." );
      return;
   }

   if( Graph != null )
     Graph.ResetHierarchy();   // Erase the previous graph, if any.
   
   GraphType = new_type;
   
   // Define the element in the HTML page to display the drawing in.
   drawing.SetParentElement( "glg_area" );

   // Disable viewport border to use the border of the glg_area.
   drawing.SetDResource( "LineWidth", 0 );

   EnableButtons();

   StartGraphDemo( drawing );
}

//////////////////////////////////////////////////////////////////////////
// Initializes the graph and starts updates.
//////////////////////////////////////////////////////////////////////////
function StartGraphDemo( drawing )
{
   Graph = drawing;

   // Set the initial number of samples, etc.
   Graph.SetDResource( "DataGroup/Factor", 20.0 );
   Graph.SetSResource( "XLabelGroup/XLabel/String", "" ); // Initial value
   Graph.SetDResource( "XLabelGroup/Factor", 5.0 );       // Num labels and ticks
   Graph.SetDResource( "XLabelGroup/MinorFactor", 4.0 );  // Num minor ticks
   Graph.SetDResource( "DataGroup/ScrollType", 0.0 );
   Graph.SetSResource( "XAxisLabel/String", "Sample" );
   Graph.SetSResource( "YAxisLabel/String", "Value" );

   // Don't set title: let the graph display the graph type defined in the
   // drawing as a title.
   // Graph.SetSResource( "Title/String", "Graph Example" );
   
   // Make the level line invisible
   if( GraphType == BAR || GraphType == LINE )
     Graph.SetDResource( "LevelObjectGroup/Visibility", 0.0 );

   Graph.InitialDraw();
   
   // Start periodic updates when the first graph is loaded.
   if( UpdateTimer == null )
     StartUpdateTimer();
}

//////////////////////////////////////////////////////////////////////////
// Updates the graph with random data
//////////////////////////////////////////////////////////////////////////
function UpdateGraph()
{
   if( PerformUpdates )
   {
      var num_samples;
      if( UpdateAfterEachSample )
        num_samples = 1;    // Update after each new datasample.
      else                  // Update after filling the whole graph.
        num_samples = Math.floor( Graph.GetDResource( "DataGroup/Factor" ) );

      for( var i=0; i<num_samples; ++i )
      {
         ++label_counter;
         if( label_counter >= 10000 )
           label_counter = 0;
         
         // Push next data value
         Graph.SetDResource( "DataGroup/EntryPoint", GLG.Rand( 0.0, 1.0 ) );
         
         // Push next label
         Graph.SetSResource( "XLabelGroup/EntryPoint", "#" + label_counter );
      }

      // Slow update rates if filling the whole graph.
      if( num_samples == 1 || ( update_counter % 4 ) == 0 )
        Graph.Update();

      ++update_counter;
      if( update_counter >= 100 )
        update_counter = 0;
   }

   // Restart the update timer.
   StartUpdateTimer();
}

//////////////////////////////////////////////////////////////////////////////
// Enable / disable buttons depending on the graph type.
//////////////////////////////////////////////////////////////////////////////
function EnableButtons()
{
   // Disable Reverse button for a ribbon graph.
   document.getElementById("reverse_button").disabled =
     ( GraphType == RIBBON ? true : false );

   // Disable grid buttons for 3D graphs.
   document.getElementById("x_grid_button").disabled =
     ( GraphType == RIBBON || GraphType == BAR_3D ? true : false );
   document.getElementById("y_grid_button").disabled =
     ( GraphType == RIBBON || GraphType == BAR_3D ? true : false );
}

//////////////////////////////////////////////////////////////////////////
// Changes number of samples
//////////////////////////////////////////////////////////////////////////
function NumberOfSamples()
{
   var num_samples = Math.floor( Graph.GetDResource( "DataGroup/Factor" ) );
   
   num_samples += 10;
   if( num_samples >= 60 )
     num_samples = 20;
   
   Graph.SetDResource( "DataGroup/Factor", num_samples );
   Graph.Update();
   
   // Adjust number of labes to a new number of samples
   ChangeXLabels( 0 ); 
}

//////////////////////////////////////////////////////////////////////////
// Changes scroll type
//////////////////////////////////////////////////////////////////////////
function ScrollType()
{
   var scroll_type = Math.floor( Graph.GetDResource( "DataGroup/ScrollType" ) );

   // Toggle between WRAP and SCROLL
   if( scroll_type == GLG.GlgHistoryScrollType.WRAPPED )
     scroll_type = GLG.GlgHistoryScrollType.SCROLLED; 
   else
     scroll_type = GLG.GlgHistoryScrollType.WRAPPED;

   Graph.SetDResource( "DataGroup/ScrollType", scroll_type );
   Graph.Update();
}

//////////////////////////////////////////////////////////////////////////
// Changes scroll direction
//////////////////////////////////////////////////////////////////////////
function Reverse()
{
   if( GraphType != RIBBON )    // No reversing for 3D line
   {
      var inversed = Math.floor( Graph.GetDResource( "DataGroup/Inversed" ) );

      // Toggle between 0 and 1
      if( inversed == 0 )
        inversed = 1;
      else
        inversed = 0;

      Graph.SetDResource( "DataGroup/Inversed", inversed );
      Graph.Update();
   }
}

//////////////////////////////////////////////////////////////////////////
// Changes range
//////////////////////////////////////////////////////////////////////////
function ChangeRange()
{
   var high_range =
     Math.floor( Graph.GetDResource( "YLabelGroup/YLabel/High" ) );

   if( high_range == 1.0 )
     high_range = 5;
   else
     high_range = 1.0;

   Graph.SetDResource( "YLabelGroup/YLabel/High", high_range );
   Graph.Update();
}

//////////////////////////////////////////////////////////////////////////
// Changes the number of digits after the decimal point for Y labels
//////////////////////////////////////////////////////////////////////////
function ChangeYFormat()
{
   var format = Graph.GetSResource( "YLabelGroup/YLabel0/Format" );
   if( format == "%.1lf" )
     format = "%.2lf";
   else
     format = "%.1lf";

   Graph.SetSResource( "YLabelGroup/YLabel0/Format", format );
   Graph.Update();
}

//////////////////////////////////////////////////////////////////////////
// Changes the number of Y labels
//////////////////////////////////////////////////////////////////////////
function ChangeYLabels()
{
   var num_minor_ticks;

   var num_labels = Math.floor( Graph.GetDResource( "YLabelGroup/Factor" ) );

   if( num_labels == 5 )
   {
      num_labels = 4;
      num_minor_ticks = 3;
   }
   else
   {
      num_labels = 5;
      num_minor_ticks = 2;
   }
      
   Graph.SetDResource( "YLabelGroup/Factor", num_labels );
   Graph.SetDResource( "YLabelGroup/MinorFactor", num_minor_ticks );
   Graph.Update();
}

//////////////////////////////////////////////////////////////////////////
// Changes the number of X labels
//////////////////////////////////////////////////////////////////////////
function ChangeXLabels( change )
{
   var num_minor_ticks;

   var num_samples = Math.floor( Graph.GetDResource( "DataGroup/Factor" ) );
   var num_labels = Math.floor( Graph.GetDResource( "XLabelGroup/Factor" ) );

   if( change != 0 )
     if( num_labels == 5 )
       num_labels = 10;
     else
       num_labels = 5;
      
   if( num_labels == num_samples )
     // No minor ticks: we have a lable for each sample
     num_minor_ticks = 1;
   else
     num_minor_ticks = num_samples / num_labels;

   Graph.SetDResource( "XLabelGroup/Factor", num_labels );
   Graph.SetDResource( "XLabelGroup/MinorFactor", num_minor_ticks );
   Graph.Update();
}

//////////////////////////////////////////////////////////////////////////
function OneSample()
{
   UpdateAfterEachSample = true;

   // Enable Change Scroll Type and Reverse buttons
   document.getElementById("scroll_type_button").disabled = false;
   if( GraphType != RIBBON )
     document.getElementById("reverse_button").disabled = false;
}

//////////////////////////////////////////////////////////////////////////
function WholeFrame()
{
   UpdateAfterEachSample = false;

   // Disable Change Scroll Type and Reverse buttons
   document.getElementById("scroll_type_button").disabled = true;
   document.getElementById("reverse_button").disabled = true;
}

//////////////////////////////////////////////////////////////////////////
function StopUpdate()
{
   PerformUpdates = false;
}

//////////////////////////////////////////////////////////////////////////
function StartUpdate()
{
   PerformUpdates = true;
}

//////////////////////////////////////////////////////////////////////////
function SetTitles( title, titleX, titleY )
{
   Graph.SetSResource( "Title/String", title );
   Graph.SetSResource( "XAxisLabel/String", titleX );
   Graph.SetSResource( "YAxisLabel/String", titleY );
   Graph.Update();
}

//////////////////////////////////////////////////////////////////////////
function Grid( x_grid )
{
   var resource_name;

   if( x_grid != 0 )
     resource_name = "XGridGroup/Visibility";
   else
     resource_name = "YGridGroup/Visibility";

   var grid_on = Math.floor( Graph.GetDResource( resource_name ) );

   // Toggle between 0 and 1
   if( grid_on == 0 )
     grid_on = 1;
   else
     grid_on = 0;
        
   Graph.SetDResource( resource_name, grid_on );
   Graph.Update();
}

//////////////////////////////////////////////////////////////////////////////
// Changes drawing size while maintaining width/height aspect ratio.
//////////////////////////////////////////////////////////////////////////////
function SetDrawingSize( next_size )
{
   const ASPECT_RATIO = 700 / 600;
   
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
   GLG.SetCanvasScale( coord_scale, 2, 0.6 );
   
   // Mobile devices use fixed device-width: disable Change Drawing Size button.
   var change_size_button = document.getElementById( "change_size" );
   if( change_size_button != null )
     change_size_button.parentNode.removeChild( change_size_button );

   return coord_scale;      // Chosen coord scale for mobile devices.
}

//////////////////////////////////////////////////////////////////////////
function StartUpdateTimer()
{
   UpdateTimer = setTimeout( UpdateGraph, UPDATE_INTERVAL );
}

function StopUpdateTimer()
{
   if( UpdateTimer != null )
   {
      clearTimeout( UpdateTimer );
      UpdateTimer = null;
   }
}
