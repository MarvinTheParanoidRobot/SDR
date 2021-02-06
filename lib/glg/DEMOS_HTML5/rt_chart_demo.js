//////////////////////////////////////////////////////////////////////////////
// GLG Real Time Chart Demo
//
// The demo is written in pure HTML5 and JavaScript. The source code of the
// demo uses the GLG Toolkit JavaScript Library supplied by the included
// Glg*.js and GlgToolkit*.js files.
//
// The library loads a GLG drawing containing a GLG Real Time Chart and
// renders it on a web page, providing an API to animate the chart with
// real-time data and handle user interaction, such as zooming or dragging
// the chart data with the mouse, or handling toolbar controls that modify
// the chart's behavior.
//
// In addition to controlling the chart via the GLG API at run time,
// the GLG Graphics Builder can be used to set numerious parameters of
// the chart interactively, as well as to create panels containing
// multiple charts together with buttons that control charts' behavior.
//
// This source code demonstrates how to use various features of the real-time
// chart. It includes examples of using the chart with both the real-time,
// historical and calendar data. In a real application, only a small fraction
// of the code will be used to dsiplay and update the chart in the selected
// usage mode.
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

/* Load any assets used by the program, and invoke the LoadDrawing function
   when done.
*/
LoadAssets( LoadDrawing );

//////////////////////////////////////////////////////////////////////////////
function LoadDrawing()
{
   /* Load a drawing from the stripchart.g file. 
      The LoadCB callback will be invoked when the drawing has been loaded.
   */
   var drawing_file = ( MobileVersion ? "stripchart2.g" : "stripchart.g" );

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

   StartRealTimeChartDemo( drawing );
}

var Drawing;
var ChartVP;
var Chart;

//////////////////////////////////////////////////////////////////////////////
function StartRealTimeChartDemo( drawing )
{
   Drawing = drawing;
   ChartVP = Drawing.GetResourceObject( "ChartViewport" );
   Chart = ChartVP.GetResourceObject( "Chart" );   
    
   AdjustForMobileDevices();
   
   /* Add Input callback used to handle user interaction. */
   Drawing.AddListener( GLG.GlgCallbackType.INPUT_CB, InputCallback );
    
   /* Add Trace callbacks used to start chart scrolling by dragging it with 
      the mouse. */
   Drawing.AddListener( GLG.GlgCallbackType.TRACE_CB, TraceCallback );
   Drawing.AddListener( GLG.GlgCallbackType.TRACE2_CB, Trace2Callback );
    
   // Display the number of data points per line and the total number of points.
   ChartVP.SetDResource( "NumDataPoints", BUFFER_SIZE );
   ChartVP.SetDResource( "NumDataPointsTotal", ( BUFFER_SIZE * NUM_PLOTS ) );
    
   Drawing.InitialDraw();
    
   InitChart();
    
   // Start periodic updates.
   StartUpdateTimer();
}

//////////////////////////////////////////////////////////////////////////////
// Control variables and constants
//////////////////////////////////////////////////////////////////////////////

/* Set to true to allow the user to ZoomTo using Button3-Drag-Release
   instead of the Button1-Drag-Release default.
*/
const ENABLE_ZOOM_TO_ON_BUTTON3 = true;

const DAY = 3600 * 24;   /* Number of seconds in a day */

const NUM_PLOTS  = 3;         /* Number of plot lines in the chart. */
const NUM_Y_AXES = NUM_PLOTS; /* One axis for each plot in this demo, 
                                 may be different. */
/* Sampling interval for historical data points in seconds. */
const HISTORICAL_DATA_INTERVAL = 60;   /* Once a minute */

/* Chart mode */
const REAL_TIME  = 0;         /* Real-time mode: updates graph with data using
                                 the current time for time stamps. */
const HISTORICAL = 1;         /* Historical mode: displays and scrolls through
                                 historical data. */
const CALENDAR   = 2;         /* Calendar mode: displays daily data. */

/* Constants for scrolling to the ends of the time range. */
const DONT_CHANGE  = 0;
const MOST_RECENT  = 1;       /* Make the most recent data visible. */
const LEAST_RECENT = 2;       /* Make the least recent data visible.*/

const UPDATE_INTERVAL = 30;   /* Update interval in msec */

const BUFFER_SIZE = 50000;    /* Number of samples to keep in the buffer for 
                                 each line. */
const PREFILL_DATA = true;    /* Setting to false suppresses pre-filling the 
                                 chart's buffer with data on start-up in the 
                                 REAL_TIME mode. */
var UpdateTimer = null; 

var Plot = new Array( NUM_PLOTS );
var YAxis = new Array( NUM_Y_AXES );

/* Variables used to keep current state. */
var TimeSpan = 0;          /* The currently displayed span in seconds. */
var StoredScrollState = 0; /* Stored AutoScroll state to be restored if ZoomTo
                              is aborted. */
var AutoScroll = 1;        /* Current auto-scroll state: enabled (1) or 
                              disabled (0). */
var Mode = REAL_TIME;      /* Current mode: real-time, historical or calendar. */
var SpanIndex = 1;         /* Index of the currently displayed time span.*/
var YAxisLabelType = 0;    /* Used to demonstrate diff. Y axis labels. */ 
var YAxisMobileOffset = 0; /* Adjustment offset for mobile devices. */

/* Variables that keep state information used to generate simulated data 
   for the demo. */
var PlotCounter = null;
var Plot0Valid = true;

/* Stores initial range values, used to restore after zooming. */
var Min = new Array( NUM_PLOTS );
var Max = new Array( NUM_PLOTS );

var SelectedPlot = null;
var StopAutoScroll = false;
var StartDragging = false;

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
   var i;
   
   Drawing.SetDResource( "$config/GlgMouseTooltipTimeout", 0.25 );

   /* Set the requested buffer size. */
   Chart.SetDResource( "BufferSize", BUFFER_SIZE );

   /* Increase the number of plots and Y axes if the matching number of 
      them are not already defined in the chart's drawing. 
   */
   Chart.SetDResource( "NumPlots", NUM_PLOTS );
   Chart.SetDResource( "NumYAxes", NUM_Y_AXES );

   Chart.SetupHierarchy();

   /* Using an Intermediate API to store plot IDs in an array for convenient
      access. To query plot IDs with the Standard API, use GetNamedPlot()
      in conjunction with CreateIndexedName().
   */
   var plot_array = Chart.GetResourceObject( "Plots" );
   for( i=0; i<NUM_PLOTS; ++i )
     Plot[i] = plot_array.GetElement( i );
   
   /* Store Y axes in an array for convenient access using an Intermediate 
      API. Alternatively, Y axes' resources can be accessed by their 
      resource names via the Standard API, for example: 
      "ChartVP/Chart/YAxisGroup/YAxis#0/Low"
   */
   var y_axis_array = Chart.GetResourceObject( "YAxisGroup" );
   for( i=0; i<NUM_Y_AXES; ++i )
     YAxis[i] = y_axis_array.GetElement( i );

   /* Set the Chart Zoom mode. It was set and saved with the drawing, 
      but do it again programmatically just in case.
   */
   ChartVP.SetZoomMode( null, Chart, null, GLG.GlgZoomMode.CHART_ZOOM_MODE );

   /* Set the initial Y axis label type. */
   ChangeYAxisLabelType( YAxisLabelType );

   /* Query the initial Y ranges defined in the drawing and store them
      for the Restore Ranges action.
   */
   StoreInitialYRanges();

   DisplaySelection( null, true );

   /* Sets initial mode: real-time, historical or calendar. */
   SetMode( Mode );
}

//////////////////////////////////////////////////////////////////////////
// Updates the chart with data.
// This demo uses simulated data provided by the GetDemoData() function.
// In an application, a custom GetData function may be used to obtain
// live data to be displayed in the chart.
//////////////////////////////////////////////////////////////////////////
function UpdateChart()
{
   /* Supply demo data to update plot lines. */
   for( var i=0; i<NUM_PLOTS; ++i )
   {
      GetDemoData( i, data_point );
      PushPlotPoint( i, data_point );
   }
   
   Drawing.Update();    // Draw new data.

   // Restart the update timer.
   StartUpdateTimer();
}

//////////////////////////////////////////////////////////////////////////
// Pushes the data_point's data into the plot using resources.
//////////////////////////////////////////////////////////////////////////  
function PushPlotPoint( plot_index, data_point )
{
   var plot = Plot[ plot_index ];

   /* Supply plot value for the chart via ValueEntryPoint. */
   plot.SetDResource( "ValueEntryPoint", data_point.value );
                 
   if( data_point.has_time_stamp )
   {
      /* Supply an optional time stamp. If not supplied, the chart will 
         automatically generate a time stamp using current time. 
      */
      plot.SetDResource( "TimeEntryPoint", data_point.time_stamp );
   }
      
   /* Using markers to annotate spikes on the first plot. The plot type
      was set to LINE & MARKERS in the drawing; marker's Visibility
      can be used as an entry point for marker visibility values.
   */
   if( plot_index == 0 )
     plot.SetDResource( "Marker/Visibility", data_point.has_marker ? 1.0 : 0.0 );
   
   if( !data_point.value_valid )
   {	   
      /* If the data point is not valid, set ValidEntryPoint resource to 
         display holes for invalid data points. If the point is valid,
         it is automatically set to 1.0 by the chart.
      */
      plot.SetDResource( "ValidEntryPoint", 0.0 );
   }
}

//////////////////////////////////////////////////////////////////////////
// Pushes the data_point's data into the plot using low level API methods
// for increased performance. It is used to prefill a chart with large
// quantities of data.
//////////////////////////////////////////////////////////////////////////  
function PushPlotPointDirect( plot_index, data_point )
{
   /* Supply an optional time stamp. Use the current time if the time stamp
      is not supplied.
   */
   var time_stamp =
     ( data_point.has_time_stamp ? data_point.time_stamp : GetCurrTime() );

   var marker_visibility =
     ( plot_index == 0 && data_point.has_marker ? 1.0 : 0.0 );

   var datasample =
     GLG.CreateDataSample( data_point.value, time_stamp, data_point.value_valid,
                           marker_visibility );
   GLG.AddDataSample( Plot[ plot_index ], datasample );
}

//////////////////////////////////////////////////////////////////////////
// Handle user interaction.
//////////////////////////////////////////////////////////////////////////
function InputCallback( viewport, message_obj )
{
   var
     origin,
     format,
     action,
     subaction;

   origin = message_obj.GetSResource( "Origin" );
   format = message_obj.GetSResource( "Format" );
   action = message_obj.GetSResource( "Action" );
   subaction = message_obj.GetSResource( "SubAction" );

   if( format == "Button" )         /* Handle button clicks */
   {
      if( action != "Activate" &&      /* Not a push button */
          action != "ValueChanged" )   /* Not a toggle button */
        return;
      
      AbortZoomTo();
      
      if( origin == "ScrollToRecent" )
      {         
         /* Set time axis's end to current time. */
         ScrollToDataEnd( MOST_RECENT, true ); 
      }
      else if( origin == "ToggleAutoScroll" )
      {         
         ChangeAutoScroll( -1 );   /* Toggle curr. value between 0 and 1.0 */
      }
      else if( origin == "ZoomTo" )
      {
          if( ENABLE_ZOOM_TO_ON_BUTTON3 )
            /* Allow the ZoomTo toolbar button to use the left mouse button. */
            Drawing.SetDResource( "$config/GlgZoomToButton", 1.0 );

         ChartVP.SetZoom( null, 't', 0.0 );  /* Start ZoomTo op */

         /* Temporarily enable touch events to make it possible to define
            the ZoomTo rectangle by touch dragging on mobile devices.
         */
         StartDragging = true;
      }
      else if( origin == "ResetZoom" )
      {         
         SetChartSpan( SpanIndex );
         RestoreInitialYRanges();
      }
      else if( origin == "ScrollBack" )
      {
         ChangeAutoScroll( 0 );
         
         /* Scroll left by 1/3 of the span. */
         ChartVP.SetZoom( null, 'l', 0.33 );
      }
      else if( origin == "ScrollBack2" )
      {
         ChangeAutoScroll( 0 );
         
         /* Scroll left by a full span. */
         ChartVP.SetZoom( null, 'l', 1.0 );
      }
      else if( origin == "ScrollForward" )
      {
         ChangeAutoScroll( 0 );
         
         /* Scroll right by 1/3 of the span. */
         ChartVP.SetZoom( null, 'r', 0.33 );
      }
      else if( origin == "ScrollForward2" )
      {
         ChangeAutoScroll( 0 );
         
         /* Scroll right by a full span. */
         ChartVP.SetZoom( null, 'r', 1.0 );
      }
      else if( origin == "ToggleLabels" )
      {
         ChangeYAxisLabelType( -1 );   /* Change to the next type. */
      }
      else if( origin == "DemoMode" )
      {
         // Toggle the current mode between REAL_TIME, HISTORICAL and CALENDAR.
         SetMode( -1 );
      }

      Drawing.Update();
   }
   else if( format == "Menu" )
   {
      if( action != "Activate" )
        return;
      
      AbortZoomTo();
      
      if( origin == "SpanSelector" )    /* Span change */
      {         
         SpanIndex = message_obj.GetDResource( "SelectedIndex" );
         SpanIndex = Math.round( SpanIndex );   // Make sure it's an int.
         
         SetChartSpan( SpanIndex );
         RestoreInitialYRanges(); /* Restore in case the chart was zoomed.*/
         
         /* Scroll to show the recent data to avoid showing an empty chart
            if user scrolls too much into the future or into the past.
            
            In the real-time mode, invoke ScrollToDataEnd() even if 
            AutoScroll is True to scroll ahead by a few extra seconds to 
            show a few next updates without scrolling the chart.
         */
         var min_max = Chart.GetDataExtent( null, /* x extent */ true );

         if( min_max != null )
         {
            var first_time_stamp = min_max.min;
            var last_time_stamp = min_max.max;
            var displayed_time_end = Chart.GetDResource( "XAxis/EndValue" );

            if( Mode == REAL_TIME && AutoScroll != 0 )
              ScrollToDataEnd( MOST_RECENT, true );
            else if( displayed_time_end > last_time_stamp + GetExtraSeconds() )
              ScrollToDataEnd( MOST_RECENT, true );
            else if( displayed_time_end - TimeSpan <= first_time_stamp )
              ScrollToDataEnd( LEAST_RECENT, true );
         }
         Drawing.Update();
      }
   }
   else if( format == "Zoom" )
   {
      if( action == "Zoom" )
      {
         if( subaction == "ZoomRectangle" )
         {
            /* Store the current AutoScroll state to restore it if ZoomTo is
               aborted. */
            StoredScrollState = AutoScroll;
            
            /* Stop scrolling: ZoomTo action is being started. */
            ChangeAutoScroll( 0 );
         }
         else if( subaction == "End" )
         {
            /* No addtional actions on finishing ZoomTo. The Y scrollbar
               appears automatically if needed: it is set to PAN_Y_AUTO. 
               Don't resume scrolling: it'll scroll too fast since we zoomed 
               in. Keep it still to allow inspecting zoomed data.
            */
         }
         else if( subaction == "Abort" )
         {
            /* Resume scrolling if it was on. */
            ChangeAutoScroll( StoredScrollState );         
         }
         
         Drawing.Update();
      }
   }
   else if( format == "Pan" )
   {
      if( action == "Pan" )
      {
         /* This code may be used to perform custom actions when dragging 
            the chart's data with the mouse. 
         */
         if( subaction == "Start" )   /* Chart dragging start */
         {
         }
         else if( subaction == "Drag" )    /* Dragging */
         {
         }
         else if( subaction == "ValueChanged" )   /* Scrollbars */
         {
         }
         /* Dragging ended or aborted. */
         else if( subaction == "End" ||
                  subaction == "Abort" )
         {
         }
      }
   }
   else if( format == "Tooltip" )
   {
      if( action == "SpecialTooltip" )
      {
         /* When the chart tooltip appears, erase selection text, but
            keep selection marker from the tooltip. 
         */
         DisplaySelection( null, false );
      }
   }
   else if( format == "Chart" )
   {
      if( action == "CrossHairUpdate" )
      {
         /* No need to invoke Update() to redraw the new position of the 
            chart's cross hair cursor: the drawing will be redrawn in one
            batch by either the update timer or DisplaySelection().
         */
      }
   }
   else if( format == "CustomEvent" )
   {
      var event_label = message_obj.GetSResource( "EventLabel" );
      if( event_label != null )
        if( event_label == "LegendSelect" )
        {
           SelectPlot( GLG.GetSelectedPlot() );   /* Select plot. */
           /* Don't stop auto-scroll if legend was clicked on. */
           StopAutoScroll = false;
        }
        else if( event_label == "LegendUnselect" )
        {
           SelectPlot( null );                   /* Unselect plot. */
           StopAutoScroll = false;
        }
   }
}

//////////////////////////////////////////////////////////////////////////
// Changes line width of the selected plot.
//////////////////////////////////////////////////////////////////////////
function SelectPlot( plot )
{
   if( plot == SelectedPlot )
     return;
   
   if( SelectedPlot != null )
   {
      /* Unselect the previously selected plot. */
      SelectedPlot.SetDResource( "Selected", 0.0 );
      SelectedPlot = null;
   }
   
   if( plot != null )
   {
      /* Select a new plot. "Selected" resource controls transformation 
         attached to the plot's line width. When the Selected resource 
         is set to 1, the plot's LineWidth changes to 2.
      */
      plot.SetDResource( "Selected", 1.0 );
      SelectedPlot = plot;
   }
   
   Drawing.Update();
}

//////////////////////////////////////////////////////////////////////////
function EnableTouchDragging()
{
   if( EnableTouchDragging.first_time == undefined )   // first time
   {
       alert( "Scrolling the chart by touch-dragging is disabled on this page by default to avoid interfering with page scrolling and zooming on mobile devices. Use the Mobile Version button to see the mobile version with touch scrolling enabled by default.\n\nTo scroll the data in the chart on mobile devices by touch on this page, press the Enable Touch Dragging button, then touch and drag the chart. To repeat, press the button again." );
      EnableTouchDragging.first_time = false;
   }
   
   StartDragging = true;
}

//////////////////////////////////////////////////////////////////////////
// Used to start scrolling the chart by dragging it with the mouse.
//////////////////////////////////////////////////////////////////////////
function TraceCallback( viewport, trace_info )
{
   var x, y;
   var display_selection = false;

   /* Use ChartViewport's events only. */
   if( !trace_info.viewport.Equals( ChartVP ) )
     return;

   var event_type = trace_info.event_type;
   switch( event_type )
   {
    case GLG.GlgEventType.TOUCH_START:
      /* Enable touch dragging only for the mobile version to avoid interfering
         with page scrolling and zooming.
      */
      if( !MobileVersion && !StartDragging )
        return;
      
      GLG.SetTouchMode();        /* Start dragging via touch events. */
      StartDragging = false;     /* Reset for the next time. */
      /* Fall through */

    case GLG.GlgEventType.TOUCH_MOVED:
      if( !GLG.GetTouchMode() )
        return;
    case GLG.GlgEventType.MOUSE_PRESSED:
    case GLG.GlgEventType.MOUSE_MOVED:
      x = trace_info.mouse_x * CoordScale;
      y = trace_info.mouse_y * CoordScale;
      display_selection = true;
         
      /* COORD_MAPPING_ADJ is added to the cursor coordinates for precise
         pixel mapping.
      */
      x += GLG.COORD_MAPPING_ADJ;
      y += GLG.COORD_MAPPING_ADJ;
      break;
        
    case GLG.GlgEventType.MOUSE_EXITED: 
      /* Erase last selection when cursor leaves the window. */
      DisplaySelection( null, true );
      return;
      
    default: return;
   }
   
   switch( event_type )
   {
    case GLG.GlgEventType.TOUCH_START:
    case GLG.GlgEventType.MOUSE_PRESSED:
      if( ZoomToMode() )
        return; /* ZoomTo or dragging mode in progress. */
      
      /* Start dragging with the mouse on a mouse click on Button1,
         or start ZoomTo on Button3.
         
         For Button1, if user clicked on an axis, the dragging will
         be activated in the direction of that axis. If the user
         clicked on the chart area, dragging in both the time and
         the Y direction will be activated.
         
         To allow dragging just in one direction, use '>' instead of 's' 
         for horizontal scrolling and '^' for vertical.
      */
      switch( trace_info.button )
      {
       case 1:    // Left button
         ChartVP.SetZoom( null, 's', 0.0 );     /* Start dragging */
         break;
       case 3:   // Right button
         if( ENABLE_ZOOM_TO_ON_BUTTON3 )
         {
            /* Change ZoomTo button from 1 to 3. */
            Drawing.SetDResource( "$config/GlgZoomToButton", 3.0 );

            ChartVP.SetZoom( null, 't', 0.0 );   /* Start ZoomTo */
         }
         break;
      }
      
      /* Disable AutoScroll not to interfere with dragging - but do it later
         in the Trace2 callback, only if legend was not clicked on.
      */
      StopAutoScroll = true;
      break;
   }

   /* In addition to a tooltip appearing after a timeout when the mouse 
      stops, display selection information when the mouse moves over a 
      chart or axis. The selection is displayed using the same format as 
      the tooltip, which is configured via the TooltipFormat attribute 
      of the chart. Alternatively, an application can invoke 
      GlgCreateChartSelection() and display the returned data in a 
      custom format.
   */
   if( display_selection && !ZoomToMode() )
   {
      var selection_string =
        Chart.CreateTooltipString( x, y, 10.0, 10.0, "<single_line>" );

      /* Display new selection or erase last selection if no selection
         (when string is null). 
      */
      DisplaySelection( selection_string, true );
   }
}

//////////////////////////////////////////////////////////////////////////
// Trace2 callback is invoked after the Input callback.
//////////////////////////////////////////////////////////////////////////
function Trace2Callback( viewport, trace_info )
{
   /* Use ChartViewport's events only. */
   if( !trace_info.viewport.Equals( ChartVP ) )
     return;

   /* Stop auto-scroll on a click in the chart, but not if the legend was 
      clicked.
   */
   switch( trace_info.event_type )
   {
    case GLG.GlgEventType.TOUCH_START:
    case GLG.GlgEventType.MOUSE_PRESSED:
      if( StopAutoScroll )
      {
         StopAutoScroll = false;
         ChangeAutoScroll( 0 );
      }
      break;
   }
}

//////////////////////////////////////////////////////////////////////////
// Display information about the selected point. It is used on a mouse move 
// in addition to a tooltip.
//////////////////////////////////////////////////////////////////////////
function DisplaySelection( selection_string, erase_selection_marker )
{   
   if( selection_string == null )
   {
      selection_string = "";

      // No selection: erase selection highlight marker if requested.
      if( erase_selection_marker )
        Chart.SetDResource( "DrawSelected", 0.0, /* if changed */ true );
   }
   
   ChartVP.SetSResource( "SelectionLabel/String", selection_string, true );

   /* In the real-time mode the drawing is updated on a timer, otherwise update
      it here.
   */
   if( Mode != REAL_TIME )
     ChartVP.Update();
}

//////////////////////////////////////////////////////////////////////////
// Scrolls the graph to the minimum or maximum time stamp to show the 
// most recent or the least recent data. If show_extra is True, adds a 
// few extra seconds in the real-time mode to show a few next updates
// without scrolling the chart.
//
// Enabling AutoScroll automatically scrolls to show current data points 
// when the new time stamp is more recent then the EndValue of the axis, 
// but it is not the case when the chart is scrolled into the future 
// (to the right) - still need to invoke this method.
//////////////////////////////////////////////////////////////////////////
function ScrollToDataEnd( data_end, show_extra )
{
   var end_value, extra_sec;
      
   if( data_end == DONT_CHANGE )
     return;

   /* Get the min and max time stamp. */
   var min_max = Chart.GetDataExtent( null, /* x extent */ true );
   if( min_max == null )
     return;

   if( show_extra )   
     extra_sec = GetExtraSeconds();
   else
     extra_sec = 0.0;
   
   if( data_end == MOST_RECENT )
     end_value = min_max.max + extra_sec;
   else   /* LEAST_RECENT */
     end_value = min_max.min - extra_sec + TimeSpan ;
   
   Chart.SetDResource( "XAxis/EndValue", end_value );
}

//////////////////////////////////////////////////////////////////////////
// Determines a good number of extra seconds to be added at the end in
// the real-time mode to show a few next updates without scrolling the
// chart.
//////////////////////////////////////////////////////////////////////////
function GetExtraSeconds()
{
   var extra_sec, max_extra_sec;
      
   if( Mode != REAL_TIME )
     return 0.0;
   
   extra_sec = TimeSpan * 0.1;
   switch( SpanIndex )
   {
    default:
    case 0:
    case 1: 
    case 2: max_extra_sec = 3.0; break;
    case 3: max_extra_sec = 5.0; break;
   }
   
   if( extra_sec > max_extra_sec )
     extra_sec = max_extra_sec;
   
   return extra_sec;
}

//////////////////////////////////////////////////////////////////////////
function ChangeAutoScroll( new_value )
{
   var pan_x;

   if( new_value == -1 ) /* Use the state of the ToggleAutoScroll button. */
   {
      AutoScroll = Drawing.GetDResource( "ToggleAutoScroll/OnState" );
      AutoScroll = Math.round( AutoScroll );  // Make sure it's an int.
   }
   else    /* Set to the supplied value. */
   {
      AutoScroll = new_value;
      
      /* Update the AutoScroll toggle with the new value. */
      Drawing.SetDResource( "ToggleAutoScroll/OnState", AutoScroll );
   }

   /* Set chart's auto-scroll. */
   Chart.SetDResource( "AutoScroll", AutoScroll );

   /* Activate time scrollbar if AutoScroll is Off. The Y value scrollbar 
      uses PAN_Y_AUTO and appears automatically as needed.
   */
   pan_x = ( AutoScroll != 0 ? GLG.GlgPanType.NO_PAN : GLG.GlgPanType.PAN_X );
   ChartVP.SetDResource( "Pan", ( pan_x | GLG.GlgPanType.PAN_Y_AUTO ) );
}

//////////////////////////////////////////////////////////////////////////
// Changes the time span shown in the graph, adjusts major and minor tick 
// intervals to match the time span.
//////////////////////////////////////////////////////////////////////////
function SetChartSpan( span_index )
{
   var span, major_interval, minor_interval, time_offset;
   var num_vis_points, sampling_interval;
   
   var in_the_middle = false;
   var fix_leap_years = false;

   /* Change chart's time span, as well as major and minor tick intervals.*/
   switch( Mode )
   {
    case REAL_TIME:
      switch( span_index )
      {
       case 0:         
         span = 10;            /* 10 sec. */
         major_interval = 3;   /* major tick every 3 sec. */
         minor_interval = 1;   /* minor tick every sec. */
         break;
         
       case 1:
         span = 60;            /* 1 min. */
         major_interval = 15;  /* major tick every 15 sec. */
         minor_interval = 1;   /* minor tick every sec. */
         break;
         
       case 2:
         span = 600;           /* 10 min. */
         major_interval = 180; /* major tick every 3 min. */
         minor_interval = 60;  /* minor tick every min. */
         break;
         
       case 3:
         span = -1;             /* Show all data */
         major_interval = -4;   /* 4 major ticks */
         minor_interval = -5;   /* 5 minor ticks */
         break;
         
       default: console.error( "Invalid span index" ); return;
      }
      time_offset = 0;
      sampling_interval = UPDATE_INTERVAL / 1000.0;
      break;
      
    case HISTORICAL:   
      switch( span_index )
      {
       case 0:
         span = 3600;                /* 1 hour */
         major_interval = 60 * 10;   /* major ticks every 10 min. */
         minor_interval = 60;        /* minor ticks every min. */
         break;
         
       case 1:
         span = 3600 * 8;            /* 8 hours */
         major_interval = 3600 * 2;  /* major tick every 2 hours */
         minor_interval = 60 * 15;   /* minor tick every 15 minutes */
         break;
         
       case 2:
         span = DAY;                 /* 24 hours */
         major_interval = 3600 * 6;  /* major tick every 6 hours */
         minor_interval = 3600;      /* minor tick every hour */
         break;
         
       case 3:
         span = DAY * 10;            /* 10 days */
         major_interval = DAY * 2;   /* major tick every 2 days */
         minor_interval = 3600 * 12; /* minor tick every 12 hours */
         break;
         
       default: console.error( "Invalid span index" ); return;
      }
      
      /* Positions major ticks and labels at 8 AM instead of midnight
         when the major tick interval is set to one day or a whole number 
         of days.
      */
      time_offset = 3600 * 8;
      sampling_interval = HISTORICAL_DATA_INTERVAL;
      break;
      
    case CALENDAR: 
      switch( span_index )
      {
       case 0:
         span = DAY * 31;             /* 1 month */
         major_interval = DAY * 5 ;   /* major ticks every 5 days */
         minor_interval = DAY;        /* minor ticks every day. */
         
         /* Positions ticks and labels at noon instead of midnight. */
         time_offset = DAY / 2;
         break;
         
       case 1:
         span = DAY * 31 * 3;         /* 1 quarter. */
         major_interval = DAY * 14;   /* major tick every 2 weeks */
         minor_interval = DAY;        /* minor tick every day */
            
         /* Positions ticks and labels at noon instead of midnight. */
         time_offset = DAY / 2;
         break;
         
       case 2:
         /* Display labels in the middle of each month's interval. */
         in_the_middle = true; 
         
         /* Offsets month labels by 1/2 month to position them in the 
            midddle of the month's interval.
         */
         time_offset = DAY * 15;
         
         span = DAY * 365;          /* 1 year */
         major_interval = -12;      /* major tick every month (12 ticks) */
         minor_interval = -2;       /* minor tick in the middle (2 ticks)
                                       to show the extent of the month. */
         break;
         
       case 3:
         span = DAY * 365 * 10;      /* 10 years */
         major_interval = DAY * 365; /* major tick every year */
         minor_interval = -12;       /* minor tick every month (12 ticks) */
         
         /* Time labels display only the year for this time scale.
            Position major ticks and labels a bit past the 1st the month 
            to avoid any rounding errors. The tooltips display the exact
            date regardless.
         */
         time_offset = 3600;
         break;
         
       default: console.error( "Invalid span index" ); return;
      }
      
      if( span_index >= 2 )     /* 1 year or 10 years */
        /* The major tick is positioned at the start of the month or the 
           start of the year. Tell the chart to properly calculate the label
           position by adjusting by the number of accumulated leap days.
           It matters only in the calendar mode when the major tick interval
           is greater then a day.
        */
        fix_leap_years = true;
      
      sampling_interval = DAY / 2;
      break;
      
    default: console.error( "Invalid mode" ); return;
   }
   
   /* Update the menu in the drawing with the initial value if different. */
   if( !MobileVersion )
     Drawing.SetDResource( "SpanSelector/SelectedIndex", span_index, true );
   
   /* Set intervals before SetZoom() below to avoid redrawing huge number 
      of labels. */
   Chart.SetDResource( "XAxis/MajorInterval", major_interval );
   Chart.SetDResource( "XAxis/MinorInterval", minor_interval );
   
   Chart.SetDResource( "XAxis/MajorOffset", time_offset );
   Chart.SetDResource( "XAxis/FixLeapYears", ( fix_leap_years ? 1.0 : 0.0 ) );
   
   // Set the X axis span which controls how much data is displayed in the chart.
   if( span > 0 )
   {
      TimeSpan = span;
      Chart.SetDResource( "XAxis/Span", TimeSpan );
   }
   else   /* span == -1 : show all accumulated data. */
   {
      /* 'N' resets span to show all data accumulated in the buffer. */
      ChartVP.SetZoom( null, 'N', 0.0 );
      
      /* Query the actual time span: set it to the extent of the data 
         accumulated in the chart's buffer, plus a few extra seconds
         at the end to show a few updates without scrolling the chart.
      */
      var min_max = Chart.GetDataExtent( null, /* x extent */ true );
      TimeSpan = ( min_max.max - min_max.min + GetExtraSeconds() );
      TimeSpan = Math.floor( TimeSpan );   // Make it an int.
   }
   
   /* Turn on data filtering for large spans. FilterType and FilterPrecision
      attributes of all plots are constrained, so that they may be set in one
      place, on one plot.
   */
   if( span_index > 1 )
   {
      /* Agregate multiple data samples to minimize a number of data points 
         drawn per each horizontal FilterPrecision interval.
         Show only one set of MIN/MAX values per each pixel interval. 
         An averaging data filter is also available.
      */
      Plot[0].SetDResource( "FilterType",
                            GLG.GlgChartFilterType.MIN_MAX_FILTER );
      Plot[0].SetDResource( "FilterPrecision", 1.0 );
   }
   else
     Plot[0].SetDResource( "FilterType", GLG.GlgChartFilterType.NULL_FILTER );

   /* Display the filter state in the drawing. */
   ChartVP.SetSResource( "DataFilterState", span_index > 1 ? "ON" : "OFF" );
   
   /* Erase major ticks if showing month labels in the middle of the month 
      interval in the CALENDAR mode. */
   Chart.SetDResource( "XAxis/MajorTickSize", ( in_the_middle ? 0.0 : 10.0 ) );
   Chart.SetDResource( "XAxis/LabelOffset", ( in_the_middle ? 10.0 : 0.0 ) );
   
   /* Display the number of data points visible in all three lines in the 
      current time span.
   */
   num_vis_points = TimeSpan / sampling_interval * NUM_PLOTS;
   num_vis_points = Math.floor( num_vis_points );   // Make it an int.
   
   /* Must be divisible by NUM_PLOTS */
   num_vis_points = Math.floor( num_vis_points / NUM_PLOTS ) * NUM_PLOTS;
   num_vis_points = Math.min( num_vis_points, BUFFER_SIZE * NUM_PLOTS );
   ChartVP.SetDResource( "NumDataPointsVisible", num_vis_points );
   
   /* Change time and tooltip formatting to match the demo mode and the 
      selected span. */
   SetTimeFormats();
   
   SetMarkerSize();   /* Decrease marker size for large spans. */
}

//////////////////////////////////////////////////////////////////////////
// Changes labels in the span selection buttons when switching between 
// the REAL_TIME, HISTORICAL and CALENDAR modes.
//////////////////////////////////////////////////////////////////////////
function SetSelectorLabels()
{
   const NUM_SPAN_OPTIONS = 4;

   var label;
   for( var i=0; i<NUM_SPAN_OPTIONS; ++i )
   {
      switch( Mode )
      {
       case REAL_TIME:
         switch( i )
         {
          case 0: label = "10 sec"; break;
          case 1: label = "1 min";  break;
          case 2: label = "10 min"; break;
          case 3: label = "All";    break;           
          default: console.error( "Invalid span index" ); return;
         }
         break;
         
       case HISTORICAL:
         switch( i )
         {
          case 0: label = "1 hour"; break;
          case 1: label = "8 hours";  break;
          case 2: label = "24 hours"; break;
          case 3: label = "1 week";    break;           
          default: console.error( "Invalid span index" ); return;
         }
         break;
         
       case CALENDAR:
         switch( i )
         {
          case 0: label = "1 month"; break;
          case 1: label = "1 quarter";  break;
          case 2: label = "1 year"; break;
          case 3: label = "10 years";    break;           
          default: console.error( "Invalid span index" ); return;
         }
         break;
         
       default: console.error( "Invalid mode" ); return;
      }

      if( MobileVersion )
      {
         // Set button labels in HTML.
         var button = document.getElementById( "SpanButton" + i );
         button.innerHTML = label;
      }
      else        
      {
         // Set button labels in the drawing.
         var res_name = "SpanSelector/Button" +  i;
         var button = Drawing.GetResourceObject( res_name );      
         button.SetSResource( "LabelString", label );
      }
   }
}

//////////////////////////////////////////////////////////////////////////
function StoreInitialYRanges()
{
   /* In this demo, each plot is associated with the corresponding axis by
      setting the plot's LinkedAxis property in the drawing file. When a 
      plot is linked to an axis, the plot and the axis use the same Y range, 
      
      We are using an Intermediate API to access Y axes here for convenience.
      Alternatively, Y axes' resources can be accessed by their resource 
      names via the Standard API, for example:
      "ChartVP/Chart/YAxisGroup/YAxis#0/Low"
   */
   for( var i=0; i<NUM_Y_AXES; ++i )
   {
      Min[i] = YAxis[i].GetDResource( "Low" );
      Max[i] = YAxis[i].GetDResource( "High" );
   }
}

//////////////////////////////////////////////////////////////////////////
function RestoreInitialYRanges()
{
   /* In this demo, each plot is associated with the corresponding axis by
      setting the plot's LinkedAxis property in the drawing file. When a plot
      is linked to an axis, changing the plot's and axis' ranges may be done
      by changing ranges of just one object: either a plot or its linked 
      axis. If a plot is not linked to an axis, its range may be different.
   */
   for( var i=0; i<NUM_Y_AXES; ++i )
   {
      YAxis[i].SetDResource( "Low",  Min[i] );
      YAxis[i].SetDResource( "High", Max[i] );
   }
}

//////////////////////////////////////////////////////////////////////////
function ZoomToMode()
{
   var zoom_mode = ChartVP.GetDResource( "ZoomToMode" );
   return ( zoom_mode != 0 );
}

//////////////////////////////////////////////////////////////////////////
function AbortZoomTo()
{
   if( ZoomToMode() )
   {
      /* Abort zoom mode in progress. */
      ChartVP.SetZoom( null, 'e', 0.0 ); 
      ChartVP.Update();
   }
}

//////////////////////////////////////////////////////////////////////////
// Demonstrates different styles of Y axis label positioning.
//
// This is usually done by configuring the chart in the GlgBuilder.
// This code just toggles through a few options via an API.
//////////////////////////////////////////////////////////////////////////
function ChangeYAxisLabelType( new_type )
{
   const NUM_LABEL_TYPES = 4;

   var label0, label1, label2;
   var offset_labels;
   var text_direction;      
   var label_anchoring; /* Label anchoring relatively to its control point. */
   var label_position;  /* Label position relatively to its axis. */

   if( new_type < 0 )              /* Toggle through the values. */   
   {
      ++YAxisLabelType;
      if( YAxisLabelType >= NUM_LABEL_TYPES )
        YAxisLabelType = 0;
   }
   else
     YAxisLabelType = new_type;    /* Use the supplied value. */
   
   switch( YAxisLabelType )
   {
    case 0:
      label0 = "Var1"; 
      label1 = "Var2"; 
      label2 = "Var3"; 
      text_direction = GLG.GlgTextDirection.HORIZONTAL_TEXT;

      /* Position and anchor axis labels at the center of each axis in the
         horizontal direction. */
      label_position =
        ( GLG.GlgAnchoringType.HCENTER | GLG.GlgAnchoringType.VTOP );
      label_anchoring =
        ( GLG.GlgAnchoringType.HCENTER | GLG.GlgAnchoringType.VBOTTOM );
      offset_labels = false;
      break;

    case 1:
      label0 = "Var1"; 
      label1 = "Var2"; 
      label2 = "Var3"; 
      text_direction = GLG.GlgTextDirection.VERTICAL_ROTATED_LEFT;
      
      /* Position and anchor axis labels at the center of each axis in the
         horizontal direction. */
      label_position =
        ( GLG.GlgAnchoringType.HCENTER | GLG.GlgAnchoringType.VTOP );
      label_anchoring =
        ( GLG.GlgAnchoringType.HCENTER | GLG.GlgAnchoringType.VBOTTOM );
      offset_labels = false;
      break;

    case 2:
      label0 = "Variable 1"; 
      label1 = "Variable 2"; 
      label2 = "Variable 3"; 
      text_direction = GLG.GlgTextDirection.HORIZONTAL_TEXT;

      /* Position and anchor axis labels on the left edge of each axis in 
         the horizontal direction. */
      label_position =
        ( GLG.GlgAnchoringType.HLEFT | GLG.GlgAnchoringType.VTOP );
      label_anchoring =
        ( GLG.GlgAnchoringType.HLEFT | GLG.GlgAnchoringType.VBOTTOM );
      offset_labels = true;
      break;
      
    case 3:
      label0 = "Var1"; 
      label1 = "Var2"; 
      label2 = "Var3"; 
      text_direction = GLG.GlgTextDirection.VERTICAL_ROTATED_LEFT;
      /* Position and anchor axis labels at the center of each axis in the
         vertical direction. */      
      label_position =
        ( GLG.GlgAnchoringType.HLEFT | GLG.GlgAnchoringType.VCENTER );
      label_anchoring =
        ( GLG.GlgAnchoringType.HRIGHT | GLG.GlgAnchoringType.VCENTER );
      offset_labels = false;
      break;
      
    default:
      console.error( "Wrong type" ); 
      YAxisLabelType = 0; 
      return;
   }
   
   YAxis[0].SetSResource( "AxisLabel/String", label0 );
   YAxis[1].SetSResource( "AxisLabel/String", label1 );
   YAxis[2].SetSResource( "AxisLabel/String", label2 );

   /* Set text direction for all labels using the % wildcard. */
   Chart.SetDResource( "YAxisGroup/YAxis#%/AxisLabel/TextDirection",
                       text_direction );

   if( offset_labels )
   {
      for( var i=0; i<3; ++i )
        /* Set increasing Y offsets. */
        YAxis[i].SetGResource( "AxisLabelOffset", 0.0, 32.0 - i * 13, 0.0 );
   }
   else    /* Set all Y offsets = 10 using the % wildcard. */
     Chart.SetGResource( "YAxisGroup/YAxis#%/AxisLabelOffset", 0.0, 10.0, 0.0 );
   
   /* Set position and anchoring of all labels using the % wildcard. */
   Chart.SetDResource( "YAxisGroup/YAxis#%/AxisLabelPosition", label_position );
   Chart.SetDResource( "YAxisGroup/YAxis#%/AxisLabelAnchoring",
                       label_anchoring );

   /* Adjusts the space taken by the Y axes to accomodate different axis 
      label layouts.
   */
   AdjustYAxisSpace();
}

//////////////////////////////////////////////////////////////////////////
// Returns data sample querying interval (in sec.) depending on the demo 
// mode.
//////////////////////////////////////////////////////////////////////////
function GetPointInterval()
{
   switch( Mode )
   {
    case REAL_TIME:
      return UPDATE_INTERVAL / 1000.0;  /* Update interval is in millisec. */
      
    case HISTORICAL:
      /* Historical data are sampled once per minute. */
      return HISTORICAL_DATA_INTERVAL;
      
    case CALENDAR:
      /* Sample calendar data twice per day. 
         libc can not go back beyond 1900 - only ~40K days.
         If sampling once per day, limit BufferSize to 40K.
      */
      return DAY / 2;
      
    default: console.error( "Invalid mode" ); return 100.0;
   }
}

//////////////////////////////////////////////////////////////////////////
function PreFillChartData()
{
   var
     current_time, start_time, end_time,
     num_seconds, dt;

   current_time = GetCurrTime();
   
   /* Roll back by the amount corresponding to the buffer size. */
   dt = GetPointInterval();
   num_seconds = BUFFER_SIZE * dt;
      
   if( Mode == REAL_TIME )
     num_seconds += 1.0;  /* Add an extra second to avoid rounding errors. */
   
   start_time = current_time - num_seconds;
   end_time = 0.0;        /* Stop at the current time. */
   
   for( var i=0; i<NUM_PLOTS; ++i )
     FillHistData( i, start_time, end_time );
   
   /* Remove the message. */
   ChartVP.SetDResource( "PreFillMessage/Visibility", 0.0 );

   ScrollToDataEnd( MOST_RECENT, false );

   if( MobileVersion )
     // Erase the into text at the top of the chart.
     Drawing.SetDResource( "ChartViewport/InfoLabel/Visibility", 0 );

   Drawing.Update();
}

//////////////////////////////////////////////////////////////////////////
// Prefills the chart with data using simulated data. In a real application,
// data will be coming from an application-specific data source.
//////////////////////////////////////////////////////////////////////////
function FillHistData( plot_index, start_time, end_time )
{
   var check_curr_time;

   /* Demo: generate demo pre-fill data with the same frequency as the 
      UPDATE_INTERVAL (in millisec). In an application, data will be queried
      from a real data source, returning an array of data points.
   */
   var dt = GetPointInterval();

   if( end_time == 0.0 )
   {
      check_curr_time = true;
      end_time = GetCurrTime();
   }
   else
     check_curr_time = false;
   
   /* When prefilling up to the current time, use the result of 
      GetCurrTime() as the loop's end condition and check it after
      each iteration to account for the time it takes to prefill 
      the chart.
   */
   for( time_stamp = start_time; 
        time_stamp < end_time && ( !check_curr_time ||
                                   time_stamp < GetCurrTime() );
        time_stamp += dt )
   {
      GetDemoData( plot_index, data_point );
      GetDemoData( plot_index, data_point );
      
      /* Set the time stamp. */
      data_point.time_stamp = time_stamp;
      data_point.has_time_stamp = true;
      
      PushPlotPointDirect( plot_index, data_point );
   }
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
const SPIKE_DURATION_RT = 25;
const SPIKE_DURATION_HS = 8;
const APPROX_PERIOD = 100;

var first_error = true;
var state = 0;
var change_counter = 10;
var spike_counter = 1000;
var approx_counter = 0;
var last_direction = 1;

var max_spike_height = 0.0;
var last_value = 5.0;
var increment_sign = 1.0;
var last_value2 = 0.0; 
var increment_sign2 = 1.0;         
var last_value3 = 70.0;

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
      if( Mode == REAL_TIME )           
        value = 5.0 + 1.5 * Math.sin( alpha / 5.0 ) + Math.sin( 2.0 * alpha );
      else               
      {
         last_value += GLG.Rand( 0.0, 0.01 ) * increment_sign;
         last_value2 += GLG.Rand( 0.0, 0.03 ) * increment_sign2;
         
         value = last_value + last_value2;
         
         if( GLG.Rand( 0.0, 1000.0 ) > 995.0 )
           increment_sign *= -1;

         if( GLG.Rand( 0.0, 1000.0 ) > 750.0 )
           increment_sign2 *= -1;
         
         if( value > 6.2 )
           increment_sign2 = -1.0;
         else if( value < 3.8 )
           increment_sign2 = 1.0;
      }
      
      /* Add a spike */
      spike_height = 0;
      spike_duration =
        ( Mode == REAL_TIME ? SPIKE_DURATION_RT : SPIKE_DURATION_HS );

      if( spike_counter >= spike_duration * 3 )
      {
         if( GLG.Rand( 0.0, 1000.0 ) > 990.0 ) 
         {
            /* Start a spike */
            spike_counter = 0;
            spike_sign = ( GLG.Rand( 0.0, 10.0 ) > 4.0 ? 1.0 : -1.0 );
            max_spike_height =
              spike_sign * GLG.Rand( 0.0, Mode == REAL_TIME ? 1.0 : 0.5 );
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
      if( change_counter != 0 )
      {
         --change_counter;
         
         if( change_counter == 0 )
         {            
            state = ( state == 0 ? 1 : 0 );   /* Change the state */
            
            /* Time of the next change */
            change_counter = GLG.Rand( 10.0, 100.0 );
            change_counter = Math.floor( change_counter ); // Make it an int.
         }
      }
      
      value = state;
      break;
      
    case 2:
      if( Mode == REAL_TIME )
      {
         period = ( 0.95 + 0.05 * Math.abs( Math.sin( alpha / 10.0 ) ) ); 
         value = 8.3 + Math.sin( 30.0 * period * alpha ) * 
           Math.sin( Math.PI / 8.0 + alpha );
         value *= 10.0;
      }
      else
      {
         value = last_value3 + last_direction * 0.1 * 
           ( 1.0 - Math.cos( 2.0 * Math.PI * approx_counter / APPROX_PERIOD ) );
            
         last_value3 = value;
         if( Mode == HISTORICAL )
           approx_counter += 3;
         else
           approx_counter += 1;
         
         if( last_direction < 0.0 && value < 0.6  * Max[ plot_index ] ||
             last_direction > 0.0 && value > 0.95 * Max[ plot_index ] ||
             GLG.Rand( 0.0, 1000.0 ) > 900.0 )
         {
            last_direction *= -1;
            approx_counter = 0;
         }
      }
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

//////////////////////////////////////////////////////////////////////////
// Sets the display mode: REAL_TIME, HISTORICAL or CALENDAR.
// If invoked with mode=-1, switch the mode between all three demo mode.
//////////////////////////////////////////////////////////////////////////
function SetMode( mode )
{
   SelectPlot( null );   /* Unselect a previously selected plot, if any. */
   
   if( mode >= 0 )
     Mode = mode;   /* Set to the specified mode. */
   else    /* Negative value: switch between all modes. */
   {
      ++Mode;
      if( Mode > CALENDAR )
        Mode = REAL_TIME;
   }
   
   switch( Mode )
   {
    case REAL_TIME:      
      SpanIndex = 1;
      AutoScroll = 1; 

      StartUpdateTimer();    // Start timer to update data in REAL_TIME mode.
      break;

    case HISTORICAL:
    case CALENDAR:      
      SpanIndex = ( Mode == HISTORICAL ? 2 : 3 );
      AutoScroll = 0;
      StopUpdateTimer(); // No data updates in HISTORICAL and CALENDAR modes.
      break;
      
    default: console.error( "Invalid mode" ); return;
   }
   
   /* Disable "Toggle AutoScroll" button in HISTORICAL and CALENDAR modes.*/
   Drawing.SetDResource( "ToggleAutoScroll/HandlerDisabled",
                         ( Mode != REAL_TIME ? 1.0 : 0.0 ) );

   /* Disable AutoScroll in non-real-time modes. */
   ChangeAutoScroll( AutoScroll );
   
   SetSelectorLabels();
   SetChartSpan( SpanIndex );
   RestoreInitialYRanges();
   
   /* Erase the step plot and its axis in the CALENDAR mode. */
   var enabled = ( Mode == CALENDAR ? 0.0 : 1.0 );
   YAxis[1].SetDResource( "Visibility", enabled );
   Plot[1].SetDResource( "Enabled", enabled );
   Chart.SetDResource( "Levels/Level#0/Enabled", enabled );
   Chart.SetDResource( "Levels/Level#1/Enabled", enabled );
   AdjustYAxisSpace();

   /* Clear all accumulated data samples: the data will be refilled 
      according to the new display mode.
   */
   Chart.ClearDataBuffer( null );
   
   /* Switch label in the DemoMode button. */
   if( MobileVersion )
   {
      // Set button label in HTML.
      var label;
      switch( Mode )
      {
       case REAL_TIME:  label = "Change to Historical"; break;
       case HISTORICAL: label = "Change to Calendar"; break;
       case CALENDAR:   label = "Change to Real-Time"; break;
       default:         label = "Unknown demo mode"; break;
      }
      var button = document.getElementById( "DemoMode" );
      button.innerHTML = label;
   }
   else
     // Set button label in the drawing.
     Drawing.SetDResource( "DemoMode/Mode", Mode );
     
   /* In the real-time mode pre-fill chart data only if PREFILL_DATA=true.
      Always prefill in the historical and calendar mode.
   */
   if( Mode != REAL_TIME || PREFILL_DATA )
   {
      /* Display "Pre-filling the chart" message. */
      ChartVP.SetDResource( "PreFillMessage/Visibility", 1.0 );
      ChartVP.Update();
      
      /* Invoke PreFillChartData on a timer, to let the browser to show the 
         chart first.
      */
      setTimeout( PreFillChartData, 50 );
   }
   else
   {
      ScrollToDataEnd( MOST_RECENT, false );

      /* Erase prefill message. */
      ChartVP.SetDResource( "PreFillMessage/Visibility", 0.0 );
   }
}

//////////////////////////////////////////////////////////////////////////
// Sets the formats of time labels and tooltips depending on the demo mode
// and the selected time span.
//////////////////////////////////////////////////////////////////////////
function SetTimeFormats()
{
   var time_label_format, time_tooltip_format, chart_tooltip_format;

   /* No additional code is required to use the defaults defined in the 
      drawing. This elaborate example illustrates advanced options for 
      customizing label and tooltip formatting when switching between 
      time spans and data display modes modes.
      
      For an even greater control over labels and tooltips, an application 
      can define custom Label and Tooltip formatters that will supply 
      custom strings for axis labels and tooltips.
   */
   
   switch( Mode )
   {
    case REAL_TIME:
      /* See strftime() for all time format options. */
      time_label_format = "%X%n%x"; 

      time_tooltip_format = "Time: <axis_time:%X> +0.<axis_time_ms:%03.0lf> sec.\nDate: <axis_time:%x>";
      
      /* <sample_time:%s> inherits time format from the X axis. */
      chart_tooltip_format = "Plot <plot_string:%s> value= <sample_y:%.2lf>\n<sample_time:%s>";
      break;
      
    case HISTORICAL:
      /* See strftime() for all time format options. */
      time_label_format = "%R%n%e %b %Y";
      
      time_tooltip_format = "Time: <axis_time:%c>";
      chart_tooltip_format = "Plot <plot_string:%s> value= <sample_y:%.2lf>\nTime: <sample_x_time:%c>";
      break;
      
    case CALENDAR:
      /* See strftime() for all time format options. */
      if( SpanIndex == 0 || SpanIndex == 1 )   /* 1 month or 1 quarter */
      {
         /* Include day of month. */
         time_label_format = "%e %b\n%Y";
         time_tooltip_format = "Date: <axis_time:%a> <axis_time:%d> <axis_time:%b> <axis_time:%Y>";
         chart_tooltip_format = "<plot_string:%s> value= <sample_y:%.2lf>\nDate: <sample_x_time:%a> <sample_x_time:%d> <sample_x_time:%b> <sample_x_time:%Y>";
      }
      else    /* SpanIndex == 2 or 3 :   1 year or 10 years */
      {
         /* Exclude day of month. */
         time_tooltip_format = "Date: <axis_time:%d> <axis_time:%b> <axis_time:%Y>";
         chart_tooltip_format = "<plot_string:%s> value= <sample_y:%.2lf>\nDate: <sample_x_time:%d> <sample_x_time:%b> <sample_x_time:%Y>";
         
         if( SpanIndex == 2 )      /* 1 year */
         {
            /* Display only month + short year in time labels. */
            time_label_format = "%b\n%Y";
         }
         else    /* SpanIndex == 3 : 10 years */
         {
            /* Display only year in labels. */
            time_label_format = "%Y";
         }
      } 
      break;
      
    default: console.error( "Invalid mode" ); return;
   }
   
   /* Set time label and tooltip formats. */
   Chart.SetSResource( "XAxis/TimeFormat", time_label_format );
   Chart.SetSResource( "XAxis/TooltipFormat", time_tooltip_format );
   Chart.SetSResource( "TooltipFormat", chart_tooltip_format );
}

//////////////////////////////////////////////////////////////////////////
// The chart layout and Y axis space may be configured interactively in
// the Graphics Builder.
//
// This function adjusts the space taken by the Y axes at run time when a 
// number of displayed Y axes and/or their label layout changes. 
//////////////////////////////////////////////////////////////////////////
function AdjustYAxisSpace()
{
   var axis_offset, label_offset;
   
   if( Mode == CALENDAR )
   {
      /* Only two axes are displayed in non-CALENDAR modes. */     
      axis_offset = -25.0;
      
      /* YAxisLabelType == 3 needs extra space to position labels for 
         two axes.
      */
      label_offset = ( ( YAxisLabelType == 3 ) ? 30.0 : 0.0 );
   }
   else
   {
      /* All three axes are displayed in non-CALENDAR modes. */
      axis_offset = 0.0;
      
      /* YAxisLabelType == 3 needs extra space to position labels for three
         axes. */
      label_offset = ( ( YAxisLabelType == 3 ) ? 45.0 : 0.0 );
   }
   
   ChartVP.SetDResource( "OffsetLeft",
                         axis_offset + label_offset + YAxisMobileOffset );
}

//////////////////////////////////////////////////////////////////////////
// Decreases marker size for large spans.
//////////////////////////////////////////////////////////////////////////
function SetMarkerSize()
{
   Plot[0].SetDResource( "Marker/MarkerSize", SpanIndex < 2 ? 7.0 : 5.0 );
   
   /* Enable smoother sub-pixel scrolling of markers. */
   Plot[0].SetDResource( "Marker/AntiAliasing", 
                         GLG.GlgAntiAliasingType.ANTI_ALIASING_DBL );
}

//////////////////////////////////////////////////////////////////////////////
function AdjustForMobileDevices()
{
   if( CoordScale == 1.0 )
     return;   /* Desktop version. */

   /* Increase chart offsets to have more space for scaled text labels
      in canvas with increased resolution on mobile devices.
   */
   AdjustOffset( Chart, "OffsetTop", 10 );
   AdjustOffset( Chart, "OffsetBottom", -20 );
   YAxisMobileOffset = 30;

   /* Increase size of the ToggleLabels button. */
   AdjustOffset( ChartVP, "ToggleLabels/Width", 10 );
   AdjustOffset( ChartVP, "ToggleLabels/Height", -10 );
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
   const MIN_WIDTH = 500;
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
   GLG.SetCanvasScale( coord_scale, 1.5, 0.6 );
   
   // Mobile devices use fixed device-width: disable Change Drawing Size button.
   var change_size_button = document.getElementById( "change_size" );
   if( change_size_button != null )
     change_size_button.parentNode.removeChild( change_size_button );

   return coord_scale;      // Chosen coord scale for mobile devices.
}

//////////////////////////////////////////////////////////////////////////
function GetCurrTime()
{
   return Date.now() / 1000;    // seconds
}   

//////////////////////////////////////////////////////////////////////////
function StartUpdateTimer()
{
   UpdateTimer = setTimeout( UpdateChart, UPDATE_INTERVAL );
}

function StopUpdateTimer()
{
   if( UpdateTimer != null )
   {
      clearTimeout( UpdateTimer );
      UpdateTimer = null;
   }
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
