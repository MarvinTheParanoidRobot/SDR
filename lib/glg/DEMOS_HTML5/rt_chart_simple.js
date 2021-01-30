//////////////////////////////////////////////////////////////////////////////
// GLG Real Time Chart Example
//
// The demo is written in pure HTML5 and JavaScript. The source code of the
// demo uses the GLG Toolkit JavaScript Library supplied by the included
// Glg*.js and GlgToolkit*.js files.
//
// The library loads a GLG drawing containing a GLG Real Time Chart and
// renders it on a web page, providing an API to animate the chart with
// real-time data and handle user interaction.
//
// In addition to controlling the chart via the GLG API at run time,
// the GLG Graphics Builder can be used to set numerious parameters of
// the chart interactively, as well as to create panels containing
// multiple charts together with buttons that control charts' behavior.
//
// This source code demonstrates the basic features of the real-time chart,
// supplying real-time data using the GLG API.
//
// Check the GLG Real-Time Chart Demo
// (http://genlogic.com/html5_demos/rt_chart_demo.html) for more elaborate
// example that demonstrates integrated zooming and scrolling, Real-Time,
// Historical and Calendar chart modes, legend item selection and other
// GLG chart features.
//////////////////////////////////////////////////////////////////////////////

// Get a handle to the GLG Toolkit library.
var GLG = new GlgToolkit();

// Debugging aid: uncomment the next line to throw an exception on a GLG error.
//GLG.ThrowExceptionOnError( true, true, true );

// Set drawing height.
SetDrawingSize( false );

// Increase canvas resoultion for mobile devices.
var CoordScale = SetCanvasResolution();

/* Load any assets used by the program, and invoke the LoadDrawing function
   when done.
*/
LoadAssets( LoadDrawing );

//////////////////////////////////////////////////////////////////////////////
function LoadDrawing()
{
   /* Load a drawing from the stripchart3.g file. 
      The LoadCB callback will be invoked when the drawing has been loaded.
   */
   var drawing_file = "stripchart3.g";

   GLG.LoadWidgetFromURL( drawing_file, null, LoadCB, null );
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

   StartRealTimeChartExample( drawing );
}

var Drawing;

//////////////////////////////////////////////////////////////////////////////
function StartRealTimeChartExample( drawing )
{
   Drawing = drawing;
    
   Drawing.InitialDraw();
    
   InitChart();
    
   // Start periodic updates.
   UpdateTimer = setTimeout( UpdateChart, UPDATE_INTERVAL );
}

//////////////////////////////////////////////////////////////////////////////
// Control variables and constants
//////////////////////////////////////////////////////////////////////////////

const NUM_PLOTS  = 2;         /* Number of plot lines in the chart. */
const NUM_Y_AXES = NUM_PLOTS; /* One axis for each plot in this demo, 
                                 may be different. */
const UPDATE_INTERVAL = 30;   /* Update interval in msec */

const BUFFER_SIZE = 5000;    /* Number of samples to keep in the buffer for 
                                each line. */
var UpdateTimer = null; 

/* Variables used to keep current state. */
var Inversed = false;
var UpdatesOn = true;
var UseLinePlot = true;

/* Variables that keep state information used to generate simulated data 
   for the demo. */
var PlotCounter = null;
var Plot0Valid = true;
var SpanIndex = 0;
var RangeIndex = 0;
var DefaultColors = true;
var DefaultLineWidth = true;

var NumSamplesInBuffer = 0;

/* Is used to hold and pass around all information about one data point. */
var data_point =
{
  value : 0,
  value_valid : false,
  time_stamp : 0,
  has_time_stamp : false,
  has_marker : false
};

//////////////////////////////////////////////////////////////////////////
// Initializes the drawing and the chart.
//////////////////////////////////////////////////////////////////////////
function InitChart()
{
   Drawing.SetDResource( "$config/GlgMouseTooltipTimeout", 0.25 );

   /* Set the requested buffer size. */
   Drawing.SetDResource( "Chart/BufferSize", BUFFER_SIZE );

   /* Increase the number of plots and Y axes if the matching number of 
      them are not already defined in the chart's drawing. 
   */
   Drawing.SetDResource( "Chart/NumPlots", NUM_PLOTS );
   Drawing.SetDResource( "Chart/NumYAxes", NUM_Y_AXES );

   AdjustForMobileDevices();

   Drawing.SetupHierarchy();
}

//////////////////////////////////////////////////////////////////////////
// Updates the chart with data.
// This demo uses simulated data provided by the GetDemoData() function.
// In an application, a custom GetData function may be used to obtain
// live data to be displayed in the chart.
//////////////////////////////////////////////////////////////////////////
function UpdateChart()
{
   if( UpdatesOn )
   {
      /* Supply demo data to update plot lines. */
      for( var i=0; i<NUM_PLOTS; ++i )
      {
         GetDemoData( i, data_point );
         PushPlotPoint( i, data_point );
      }

      /* Display the number of samples accumulated in the each plot's buffer. */
      if( NumSamplesInBuffer < BUFFER_SIZE )
      {
         ++NumSamplesInBuffer;
         Drawing.SetDResource( "StoredSamplesLabel/NumSamples",
                               NumSamplesInBuffer );
      }
   }
   
   Drawing.Update();    // Draw new data.

   // Restart the update timer.
   UpdateTimer = setTimeout( UpdateChart, UPDATE_INTERVAL );
}

//////////////////////////////////////////////////////////////////////////
// Pushes the data_point's data into the plot using resources.
//////////////////////////////////////////////////////////////////////////  
function PushPlotPoint( plot_index, data_point )
{
   /* This code uses the GLG Standard API to supply chart data. 
      To increase performance of charts with huge number of data points, 
      the GLG Intermediate API may be used to supply data directly to the plot,
      as shown in the GLG Real-Time Chart Demo.
   */
   
   /* Supply plot value for the chart via ValueEntryPoint.
      A time stamp (in seconds, double) may be supplied via TimeEntryPoint.      
   */
   Drawing.SetDResource( "Chart/Plots/Plot#" + plot_index + "/ValueEntryPoint",
                         data_point.value );
                 
   if( data_point.has_time_stamp )
   {
      /* Supply an optional time stamp. If not supplied, the chart will 
         automatically generate a time stamp using current time. 
      */
      Drawing.SetDResource( "Chart/Plots/Plot#" + plot_index + "/TimeEntryPoint",
                            data_point.time_stamp );
   }
      
   /* Using markers to annotate spikes on the first plot. The plot type
      was set to LINE & MARKERS in the drawing; marker's Visibility
      can be used as an entry point for marker visibility values.
   */
   if( plot_index == 0 )
     Drawing.SetDResource( "Chart/Plots/Plot#" + plot_index +
                           "/Marker/Visibility",
                           data_point.has_marker ? 1.0 : 0.0 );
   
   if( !data_point.value_valid )
   {	   
      /* If the data point is not valid, set ValidEntryPoint resource to 
         display holes for invalid data points. If the point is valid,
         it is automatically set to 1.0 by the chart.
      */
      Drawing.SetDResource( "Chart/Plots/Plot#" + plot_index +
                            "/ValidEntryPoint", 0.0 );
   }
}

//////////////////////////////////////////////////////////////////////////
// Inverses chart update direction.
//////////////////////////////////////////////////////////////////////////
function Inverse()
{
   Inversed = !Inversed;

   Drawing.SetDResource( "Chart/XAxis/Inversed", Inversed ? 1 : 0 );
   Drawing.Update();
}

//////////////////////////////////////////////////////////////////////////
function StartUpdate()
{
   UpdatesOn = true;
}

//////////////////////////////////////////////////////////////////////////
function StopUpdate()
{
   UpdatesOn = false;
}

//////////////////////////////////////////////////////////////////////////
// Changes plot type.
//////////////////////////////////////////////////////////////////////////
function ChangeType()
{
   UseLinePlot = !UseLinePlot;

   var plot_type0, plot_type1;
   var filter_type, filter_precision0, filter_precision1;
   
   if( UseLinePlot )
   {
      plot_type0 = GLG.GlgPlotType.LINE_AND_MARKERS_PLOT;
      plot_type1 = GLG.GlgPlotType.LINE_PLOT;

      /* Filter out samples if the number of them exceeds the number of 
         available pixels.
      */
      filter_type = GLG.GlgChartFilterType.MIN_MAX_FILTER;
      filter_precision0 = 2;
      filter_precision1 = 2;
   }
   else
   {
      plot_type0 = GLG.GlgPlotType.STEP_AND_MARKERS_PLOT;
      plot_type1 = GLG.GlgPlotType.FLOATING_BAR_PLOT;

      /* Filter out excessive samples to have some space between bars. */
      filter_type = GLG.GlgChartFilterType.DISCARD_FILTER;
      filter_precision0 = 40;
      filter_precision1 = 10;
   }
      

   Drawing.SetDResource( "Chart/Plots/Plot#0/PlotType", plot_type0 )
   Drawing.SetDResource( "Chart/Plots/Plot#1/PlotType", plot_type1 )

   Drawing.SetDResource( "Chart/Plots/Plot#0/FilterType", filter_type )
   Drawing.SetDResource( "Chart/Plots/Plot#1/FilterType", filter_type )

   Drawing.SetDResource( "Chart/Plots/Plot#0/FilterPrecision",
                         filter_precision0 )
   Drawing.SetDResource( "Chart/Plots/Plot#1/FilterPrecision",
                         filter_precision1 )

   Drawing.Update();
}

//////////////////////////////////////////////////////////////////////////
// Changes the time span shown in the graph, adjusts major and minor tick 
// intervals to match the time span.
//////////////////////////////////////////////////////////////////////////
function ChangeSpan()
{
   ++SpanIndex;
   SpanIndex %= 4;

   var span, major_interval, minor_interval;
   var time_format, ms_format;
   
   /* Change chart's time span, as well as major and minor tick intervals.*/
   switch( SpanIndex )
   {
    case 0:         
      span = 10;              /* 10 sec. */
      major_interval = 3;     /* major tick every 3 sec. */
      minor_interval = 1;     /* minor tick every sec. */
      time_format = "%r%n%x"; /* Show time and date */  
      ms_format = "";         /* Don't show fractions of a sec. */
      break;
      
    case 1:
      span = 5;               /* 5 sec. */
      major_interval = 1;     /* major tick every sec. */
      minor_interval = 0.5;   /* minor tick every 1/2 sec. */
      time_format = "%T";     /* Show time in 24h notation */  
      ms_format = "";         /* Don't show fractions of a sec. */
      break;
      
    case 2:
      span = 1;               /* 1 sec. */
      major_interval = 0.2;   /* major tick every 2/10 sec. */
      minor_interval = 0.1;   /* minor tick every 1/10 sec. */
      time_format = "%T";     /* Show time in 24h notation. */
      ms_format = "\n%03.0f msec"; /* Show fractions of a sec on a new line:
                                       to avoid clutter on mobile devices. */
      break;
      
    case 3:
      span = -1;              /* Show all data */
      major_interval = -4;    /* 4 major ticks */
      minor_interval = -5;    /* 5 minor ticks */
      time_format = "%r%n%x"; /* Show time and date */  
      ms_format = "";         /* Don't show fractions of a sec. */
      break;
      
    default: console.error( "Invalid span index" ); return;
   }

   Drawing.SetDResource( "Chart/XAxis/MajorInterval", major_interval );
   Drawing.SetDResource( "Chart/XAxis/MinorInterval", minor_interval );
   Drawing.SetSResource( "Chart/XAxis/TimeFormat", time_format );
   Drawing.SetSResource( "Chart/XAxis/TimeFormat", time_format );
   Drawing.SetSResource( "Chart/XAxis/MilliSecFormat", ms_format );

   
   // Set the X axis span which controls how much data is displayed in the chart.
   if( span > 0 )
     Drawing.SetDResource( "Chart/XAxis/Span", span );
   else
     /* span == -1 : show all accumulated data. 'N' resets span to show all 
        data accumulated in the buffer.
     */
     Drawing.SetZoom( null, 'N', 0.0 );

   Drawing.Update();
}

//////////////////////////////////////////////////////////////////////////
// Changes the Y ranges.
//////////////////////////////////////////////////////////////////////////
function ChangeRange()
{
   ++RangeIndex;
   RangeIndex %= 3;

   var high_coeff, low_coeff;
   switch( RangeIndex )
   {
    default:
    case 0: high_coeff = 1.0; low_coeff = 0.0; break;
    case 1: high_coeff = 0.7; low_coeff = 0.3; break;
    case 2: high_coeff = 2.0; low_coeff = 0.0; break;
   }

   Drawing.SetDResource( "Chart/Plots/Plot#0/YHigh", 10. * high_coeff );
   Drawing.SetDResource( "Chart/Plots/Plot#1/YHigh", 100. * high_coeff );

   Drawing.SetDResource( "Chart/Plots/Plot#0/YLow", 10. * low_coeff );
   Drawing.SetDResource( "Chart/Plots/Plot#1/YLow", 100. * low_coeff );
   
   Drawing.Update();
} 

//////////////////////////////////////////////////////////////////////////
// Changes plot colors.
//////////////////////////////////////////////////////////////////////////
function ChangeColors()
{
   DefaultColors = !DefaultColors;

   var color0, color1;
   
   if( DefaultColors )
   {
      Drawing.SetGResource( "Chart/Plots/Plot#0/EdgeColor", 0., 0., 1. );
      Drawing.SetGResource( "Chart/Plots/Plot#0/FillColor", 0., 0., 1. );

      Drawing.SetGResource( "Chart/Plots/Plot#1/EdgeColor", 0., 0.6, 0. );
      Drawing.SetGResource( "Chart/Plots/Plot#1/FillColor", 0., 0.6, 0. );
   }
   else
   {
      Drawing.SetGResource( "Chart/Plots/Plot#0/EdgeColor", 0., 0.6, 0.6 );
      Drawing.SetGResource( "Chart/Plots/Plot#0/FillColor", 0., 0.6, 0.6 );

      Drawing.SetGResource( "Chart/Plots/Plot#1/EdgeColor", 0.65, 0., 0.65 );
      Drawing.SetGResource( "Chart/Plots/Plot#1/FillColor", 0.65, 0., 0.65 );
   }

   Drawing.Update();
}

//////////////////////////////////////////////////////////////////////////
// Changes plot line widths.
//////////////////////////////////////////////////////////////////////////
function ChangeLineWidth()
{
   DefaultLineWidth = !DefaultLineWidth;

   var line_width = ( DefaultLineWidth ? 1 : 3 );

   Drawing.SetDResource( "Chart/Plots/Plot#0/LineWidth", line_width );
   Drawing.SetDResource( "Chart/Plots/Plot#1/LineWidth", line_width );

   Drawing.Update();
}

//////////////////////////////////////////////////////////////////////////
// Supplies demo data, including the plot's value, an optional time stamp
// and an optional sample_valid flag, as well as visibility of a mraker used
// to annotate some data pooints. 
//
// In a real application, data will be coming from an application-specific 
// data source.
//////////////////////////////////////////////////////////////////////////
function GetDemoData( plot_index, data_point )
{
   GetDemoPlotValue( plot_index, data_point );  /* Fills a value to plot. */
   
   /* Let the chart use current time as a time stamp. Optionally,
      an application can provide a time stamp in data_point.time_stamp
      and set data_point.has_time_stamp = true.
   */
   data_point.has_time_stamp = false;
   
   /* Set an optional ValidEntryPoint to make some samples invalid for
      Plot0.0 It is optional: default=True is used for the rest of the plots 
      when ValidEntryPoint is not supplied.
   */   
   if( plot_index == 0 )
   {
      if( Plot0Valid ) 
        /* Make samples invalid occasionally. */
        Plot0Valid = ( GLG.Rand( 0.0, 100.0 ) > 2.0 );
      else
        /* Make it valid again after a while. */
        Plot0Valid= ( GLG.Rand( 0.0, 100.0 ) > 30.0 );
      
      data_point.value_valid = Plot0Valid;
   }
   else
     data_point.value_valid = true;
}

/* DATA SIMULATION: These variables used for simulating data displayed 
   in the chart. In a real application, data will be coming from a 
   real data source.
*/
const MAX_COUNTER = 50000;
const PERIOD = 1000;
const SPIKE_DURATION = 25;

var first_error = true;
var spike_counter = 1000;

var max_spike_height = 0.0;

//////////////////////////////////////////////////////////////////////////
// Supplies plot values for the demo; also sets data_point.has_marker field
// to annotate some data points with a marker.
//
// In a real application, data will be coming from an application-specific 
// data source.
//////////////////////////////////////////////////////////////////////////
function GetDemoPlotValue( plot_index, data_point )
{
   var
     value, alpha, period, 
     spike_sign, spike_height;   
   var spike_duration;
   
   /* First time: init plot's state counters used to simulate data. */
   if( PlotCounter == null )
   {
      PlotCounter = new Array( NUM_PLOTS );
      for( var i=0; i<NUM_PLOTS; ++i )
        PlotCounter[ i ] = 0;
   }

   alpha = 2.0 * Math.PI * PlotCounter[ plot_index ] / PERIOD;
   switch( plot_index )
   {
    case 0:      
      value = 3.0 + 1.5 * Math.sin( alpha / 5.0 ) + Math.sin( 2.0 * alpha );
      
      /* Add a spike */
      spike_height = 0;
      spike_duration = SPIKE_DURATION;

      if( spike_counter >= spike_duration * 3 )
      {
         if( GLG.Rand( 0.0, 1000.0 ) > 990.0 ) 
         {
            /* Start a spike */
            spike_counter = 0;
            spike_sign = ( GLG.Rand( 0.0, 10.0 ) > 4.0 ? 1.0 : -1.0 );
            max_spike_height = spike_sign * GLG.Rand( 0.0, 1.0 );
         }
      }

      /* Annotate spikes with a marker. */
      data_point.has_marker = ( spike_counter == 0 );
      
      if( spike_counter <= spike_duration )
      {
         var spike_coeff;
         
         spike_coeff = 1.0 - spike_counter / spike_duration;
         spike_height = 
           0.3 * max_spike_height * spike_coeff  * spike_coeff * 
           ( 1.0 + Math.cos( 2.0 * Math.PI * spike_counter / 12.0 ) );
      }
      
      ++spike_counter;
      value += spike_height; 
      break;
      
    case 1:
      period = ( 0.95 + 0.05 * Math.abs( Math.sin( alpha / 10.0 ) ) ); 
      value = 7.0 + Math.sin( 30.0 * period * alpha ) * 
        Math.sin( Math.PI / 8.0 + alpha );
      value *= 10.0;
      break;
      
    default:
      if( first_error )
      {
         first_error = false;
         console.error( "Add a case to provide demo data for added plots." );
      }
      value = 62.0;
      break;
   }
   
   /* Increase the plot's state counter used to simulate demo data. */
   ++PlotCounter[ plot_index ];
   PlotCounter[ plot_index ] = ( PlotCounter[ plot_index ] % MAX_COUNTER );
   
   data_point.value = value;   /* Returned simulated value. */
}

//////////////////////////////////////////////////////////////////////////////
function AdjustForMobileDevices()
{
   if( CoordScale == 1.0 )
     return;   /* Desktop version. */

   /* Increase chart offsets to have more space for scaled text labels
      in canvas with increased resolution on mobile devices.
   */
   AdjustOffset( Drawing, "Chart/OffsetLeft", 30 );
   AdjustOffset( Drawing, "Chart/OffsetTop", 20 );
   AdjustOffset( Drawing, "Chart/OffsetBottom", -20 );
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
// Maintains width/height aspect ratio.
//////////////////////////////////////////////////////////////////////////////
function SetDrawingSize( next_size )
{
   // Settings for desktop displays.
   const MIN_HEIGHT = 250;
   const MAX_HEIGHT = 500;

   if( SetDrawingSize.size_index == undefined )   // first time
   {
      SetDrawingSize.size_index = 0;
      SetDrawingSize.is_mobile = ( screen.width <= 760 );
      window.addEventListener( "resize", ()=>{ SetDrawingSize( false ) } );
   }
   else if( next_size )
   {
      ++SetDrawingSize.size_index;
      SetDrawingSize.size_index %= 3;
   }

   var drawing_area = document.getElementById( "glg_area" );
   if( SetDrawingSize.is_mobile )
   {
      /* Mobile devices use constant device-width. */
       drawing_area.style.height =
           "" + Math.trunc( drawing_area.clientWidth * 0.8 ) + "px";
   }
   else   /* Desktop */
   {
      var coeff;
      switch( SetDrawingSize.size_index )
      {
       default:
       case 0: coeff = 0.6; break;
       case 1: coeff = 0.8; break;
       case 2: coeff = 0.4; break;
      }
   
      var width = document.body.clientWidth;
      if( width < 300 )
        coeff *= 1.2;

      var height = width * coeff;

      if( height < MIN_HEIGHT )
        height = MIN_HEIGHT;
      else if( height > MAX_HEIGHT )
        height = MAX_HEIGHT;

      drawing_area.style.height = "" + Math.trunc( height ) + "px";
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
