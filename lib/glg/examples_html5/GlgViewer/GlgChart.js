
// Stores information about currently loaded chart page.
var ChartInfo = null;      /* ChartInfoObj */

// Update interval in msec.
const CHART_UPDATE_INTERVAL = 100; 

// Convenient time span constants.
const  ONE_MINUTE = 60;
const  ONE_HOUR   = 3600;
const  ONE_DAY    = 3600 * 24;
    
/* Prefill time interval, specifies amount of data to prefill in the 
   real time chart. 
*/
var PREFILL_SPAN = ONE_HOUR * 8;

/* Constants for scrolling the chart to the beginning or the end of 
   the time range.
*/
const DONT_CHANGE  = 0;
const MOST_RECENT  = 1;  /* Make the most recent data visible. */
const LEAST_RECENT = 2;  /* Make the least recent data visible.*/

// Index of the initial span to display.
var INIT_SPAN = 0;    

// Index of the currently displayed time span.
var SpanIndex = INIT_SPAN; 

var TimeSpan = 0;           // Time axis span in sec.

/* If set to true, the chart's data buffer is prefilled with historical data
   on start-up.
*/ 
var PrefillData = true;     

// int: Current auto-scroll state: enabled(1) or disabled(0).
var AutoScroll = 1;

// int: Stored AutoScroll state to be restored if ZoomTo is aborted.
var StoredScrollState; 

//////////////////////////////////////////////////////////////////////////////
function SetupRTChartPage( /*GlgObject*/ top_viewport )
{
    ChartInfo = new ChartInfoObj( top_viewport );

    // Set targeted update interval for the timer.
    UpdateInterval = CHART_UPDATE_INTERVAL;
    
    // Adjust chart parameters to make it look good on mobile devices.
    AdjustChartForMobileDevices();

    // Initialization before hierarchy setup.
    InitChartBeforeH();

    // Setup object hierarchy in the drawing.
    ChartInfo.MainViewport.SetupHierarchy();

    // Initialization after hierarchy setup.
    InitChartAfterH();
}

//////////////////////////////////////////////////////////////////////////////
// Initialization before hierarhy setup.
//////////////////////////////////////////////////////////////////////////////
function InitChartBeforeH()
{
    // Add Input and Trace callbacks.
    ChartInfo.MainViewport.AddListener( GLG.GlgCallbackType.INPUT_CB, 
                                        ChartInputCallback );

    ChartInfo.MainViewport.AddListener( GLG.GlgCallbackType.TRACE_CB, 
                                        ChartTraceCallback );

    // Retrieve object ID of the Toolbar viewport.
    ChartInfo.Toolbar = ChartInfo.MainViewport.GetResourceObject( "Toolbar" );

    // Retrieve ChartViewport and the Chart object.
    ChartInfo.ChartVP = 
        ChartInfo.MainViewport.GetResourceObject( "ChartViewport" );
    
    // Retrieve the Chart object.
    ChartInfo.Chart = ChartInfo.ChartVP.GetResourceObject( "Chart" );

    // Retrieve the number of plots defined in the drawing.
    ChartInfo.NumPlots = 
        Math.trunc( ChartInfo.Chart.GetDResource( "NumPlots" ) );
    
    // Retrieve the number of Y axes defined in the drawing.
    ChartInfo.NumYAxes = 
        Math.trunc( ChartInfo.Chart.GetDResource( "NumYAxes" ) );
    
    // Enable AutoScroll, both for the toggle button and the chart.
    ChangeAutoScroll( 1 );
    
    /* Set Chart Zoom mode. It was set and saved with the drawing, 
       but do it again programmatically just in case.
    */
    ChartInfo.ChartVP.SetZoomMode( null, ChartInfo.Chart, null,
                                     GLG.GlgZoomMode.CHART_ZOOM_MODE );
    
    /* Uncomment the line below to override XAxis label TimeFormat 
       defined in the drawing. "%T" displays time without a date.
    */
    //ChartInfo.Chart.SetSResource( "XAxis/TimeFormat", "%T" );
}

//////////////////////////////////////////////////////////////////////////////
// Initialization after hierarchy setup.
//////////////////////////////////////////////////////////////////////////////
function InitChartAfterH()
{
    var i;

    // Allocate PlotArray and store object IDs for each plot. 
    ChartInfo.PlotArray = new Array( ChartInfo.NumPlots );
      
    var plot_array = 
        ChartInfo.Chart.GetResourceObject( "Plots" );  /* GlgObject  */
    for( i=0; i < ChartInfo.NumPlots; ++i )
        ChartInfo.PlotArray[i] = plot_array.GetElement( i ); 
    
    /* Store initial range for each Y axis to restore on zoom reset. 
       Assumes that plots are linked with the corresponding axes in the 
       drawing.
    */         
    ChartInfo.Low = new Array( ChartInfo.NumYAxes );    /* double[] */
    ChartInfo.High = new Array( ChartInfo.NumYAxes );   /* double[] */
    
    var axis_array = 
        ChartInfo.Chart.GetResourceObject( "YAxisGroup" ); /* GlgObject */
    for( i=0; i < ChartInfo.NumYAxes; ++i )
    {
        var axis = axis_array.GetElement( i );   /* GlgObject */
        ChartInfo.Low[ i ] = axis.GetDResource( "Low" );
        ChartInfo.High[ i ] = axis.GetDResource( "High" );
    }
    
    // Set chart's time span on the X axis.
    SetChartSpan( SpanIndex );
    
    /* Prefill chart's history buffer with data for a specified number 
       of seconds.
    */
    if( PrefillData )
        FillChartHistory( ChartInfo.Chart, PREFILL_SPAN );
}

//////////////////////////////////////////////////////////////////////////////
// Reset chart-related global variables.
//////////////////////////////////////////////////////////////////////////////
function CleanupChartPage()
{
    ChartInfo = null;
    SpanIndex = INIT_SPAN; 
}

////////////////////////////////////////////////////////////////////////////// 
// Pre-fill the graph's history buffer with data. 
////////////////////////////////////////////////////////////////////////////// 
function FillChartHistory( /*GlgObject*/ chart, /*int*/ prefill_span )
{
    if( chart == null )
        return;

    var current_time = GetCurrTime();   /* double */
    
    /* Fill the amount of data requested by the PREFILL_SPAN, up to the 
       available chart's buffer size defined in the drawing.
       Add an extra second to avoid rounding errors.
    */
    var num_seconds = prefill_span + 1;  /* int */
    
    var buffer_size =     /* int */
       Math.trunc( chart.GetDResource( "BufferSize" ) );
    if( buffer_size < 1 )
        buffer_size = 1;
    
    var max_num_samples;   /* int */
    if( RANDOM_DATA )
    {
        // In random demo data mode, simulate data stored once per second.
        var samples_per_second = 1.0;   /* double */
        max_num_samples = Math.trunc( num_seconds * samples_per_second );
        
        if( max_num_samples > buffer_size )
            max_num_samples = buffer_size;
    }
    else
        max_num_samples = buffer_size;
    
    // Start and end time for the data query.
    var start_time = current_time - num_seconds;   /* double */
    var end_time = current_time;     /* double : Stop at the current time. */

    var num_plots = Math.trunc( chart.GetDResource( "NumPlots" ) ); /*int*/
    var plots = chart.GetResourceObject( "Plots" );

    var plot;               /* GlgObject */
    var tag_source;         /* String */
    for( var i=0; i<num_plots; ++i )
    {
        plot = plots.GetElement( i );     /* GlgObject */

        // Get tag source of the plot's ValueEntryPoint.
        tag_source = plot.GetSResource( "ValueEntryPoint/TagSource" );
        
        if( IsUndefined( tag_source ) )
            continue;
        
        // Obtain historical data for the plot.
        DataFeed.GetPlotData( tag_source, start_time, end_time, max_num_samples,
                              /*callback*/ PlotDataCB, 
                              /*user data*/ plot );
    }
}

////////////////////////////////////////////////////////////////////////////// 
// Fills plot with data from the provided data array.
// For increased performance of prefilling a chart with large quantities of
// data, the data are pushed into the plot using static low level API methods
// GLG.CreateDataSample and GLG.AddDataSample.
////////////////////////////////////////////////////////////////////////////// 
function PlotDataCB( /*PlotDataPoint[]*/ data_array, /*GlgObject*/ plot )
{
    if( data_array == null )
        return;

    var size = data_array.length;
    for( var i=0; i<size; ++i )
    {
        var data_point = data_array[i];   /* PlotDataPoint */
        var datasample = GLG.CreateDataSample( data_point.value,
                                               data_point.time_stamp,
                                               data_point.value_valid, 0 );
        GLG.AddDataSample( plot, datasample );
    }
}

//////////////////////////////////////////////////////////////////////////////
// Handle user interaction as needed.
//////////////////////////////////////////////////////////////////////////////
function ChartInputCallback( viewport, message_obj )
{
    if( ChartInfo == null || ChartInfo.ChartVP == null || 
        ChartInfo.Chart == null )
        return;

    var chart_vp = ChartInfo.ChartVP;
    var chart = ChartInfo.Chart;

    var origin = message_obj.GetSResource( "Origin" );
    var format = message_obj.GetSResource( "Format" );
    var action = message_obj.GetSResource( "Action" );
    var subaction = message_obj.GetSResource( "SubAction" );
    
    if( format == "Button" )
    {	 
        if( action !="Activate" &&         /* Not a push button */
            action != "ValueChanged" )     /* Not a toggle button */
            return;

        // Abort ZoomTo mode, if any.
        AbortZoomTo();
         
        if( origin == "ToggleAutoScroll" )
        {         
            /* Set Chart AutoScroll based on the ToggleAutoScroll toggle button 
               setting.
            */
            ChangeAutoScroll( -1 ); 
        }
        else if( origin == "ZoomTo" )
        {
            // Start ZoomTo operation.
            chart_vp.SetZoom( null, 't', 0.0 );  
        }
        else if( origin == "ZoomReset" )
        {         
            // Set initial time span and reset initial Y ranges.
            SetChartSpan( SpanIndex );  
            RestoreInitialYRanges();   
        }
        else if( origin == "ScrollBack" )
        {
            ChangeAutoScroll( 0 );
            
            // Scroll left by 1/3 of the span.
            chart_vp.SetZoom( null, 'l', 0.33 );
        }
        else if( origin == "ScrollForward" )
        {
            ChangeAutoScroll( 0 );
            
            // Scroll right by 1/3 of the span.
            chart_vp.SetZoom( null, 'r', 0.33 );
        }
        else if( origin == "ScrollBack2" )
        {
            ChangeAutoScroll( 0 );
            
            // Scroll left by a full span.
            chart_vp.SetZoom( null, 'l', 1.0 );
        }
        else if( origin == "ScrollForward2" )
        {
            ChangeAutoScroll( 0 );
            
            // Scroll right by a full span.
            chart_vp.SetZoom( null, 'r', 1.0 );
        }
        else if( origin == "ZoomIn" )
        {
            // Zoom in in Y direction.
            chart_vp.SetZoom( null, 'I', 1.5 );
        }
        else if( origin == "ZoomOut" )
        {
            // Zoom out in Y direction.
            chart_vp.SetZoom( null, 'O', 1.5 );
        }
        else if( origin == "ScrollToRecent" )
        {
            // Scroll to show most recent data.
            ScrollToDataEnd( MOST_RECENT, true );
        }
        
        viewport.Update();  //format = "Button"
    }

    else if( format == "Option" )
    {
        if( action != "Select" )
            return;

        // Abort ZoomTo mode, if any.
        AbortZoomTo();

        /* Handle events from the SpanSelector menu allowing to select time 
           interval for the X axis.
        */
        if( origin == "SpanSelector" )    /* Span change */
        { 
            SpanIndex =
                Math.trunc( message_obj.GetDResource( "SelectedIndex" ) );
            
            SetChartSpan( SpanIndex );
            RestoreInitialYRanges(); /* Restore in case the chart was zoomed.*/
            
            /* Scroll to show the recent data to avoid showing an empty chart
               if user scrolls too much into the future or into the past.
               
               Invoke ScrollToDataEnd() even if AutoScroll is True to 
               scroll ahead by a few extra seconds to show a few next updates
               without scrolling the chart.
            */
            var min_max =    /* GlgMinMax */
                chart.GetDataExtent( null, /* x extent */ true );  
            
            if( min_max != null )
            {
                var first_time_stamp = min_max.min;   /* double */
                var last_time_stamp = min_max.max;    /* double */
                var displayed_time_end =    /* double */
                    chart.GetDResource( "XAxis/EndValue" );
                
                if( AutoScroll != 0 )
                    ScrollToDataEnd( MOST_RECENT, true );
                
                else if( displayed_time_end >
                         last_time_stamp + GetExtraSeconds() )
                    ScrollToDataEnd( MOST_RECENT, true );
                
                else if( displayed_time_end - TimeSpan <= first_time_stamp )
                    ScrollToDataEnd( LEAST_RECENT, true );
                
                viewport.Update();
            }
        }
    }

    else if( format == "Chart" && action == "CrossHairUpdate" )
    {
        /* To avoid slowing down real-time chart updates, invoke Update() 
         to redraw cross-hair only if the chart is not updated fast 
         enough by the timer.
      */
      if( UpdateInterval > 100 )
          viewport.Update();         
    }          
  
    else if( action == "Zoom" )    // Zoom events
    {
        if( subaction == "ZoomRectangle" )
        {
            // Store AutoSCroll state to restore it if ZoomTo is aborted.
            StoredScrollState = AutoScroll;
            
            // Stop scrolling when ZoomTo action is started.
            ChangeAutoScroll( 0 );
        }
        else if( subaction == "End" )
        {
            /* No additional actions on finishing ZoomTo. The Y scrollbar 
               appears automatically if needed: it is set to GLG_PAN_Y_AUTO. 
               Don't resume scrolling: it'll scroll too fast since we zoomed 
               in. Keep it still to allow inspecting zoomed data.
            */
        }
        else if( subaction == "Abort" )
        {
            // Resume scrolling if it was on.
            ChangeAutoScroll( StoredScrollState ); 
        }
        
        viewport.Update();
    }
    else if( action == "Pan" )    // Pan events
    {
        // Place custom code to handle pan or drag events.
    }
}

//////////////////////////////////////////////////////////////////////////////
// A custom trace callback for the page; is used to obtain coordinates 
// of the mouse click.
//////////////////////////////////////////////////////////////////////////////
function ChartTraceCallback( /*GlgObject*/ viewport, 
                             /*GlgTraceData*/ trace_info )
{
    // Process only events that occur in ChartViewport.
    if( ChartInfo == null || 
        !trace_info.viewport.Equals( ChartInfo.ChartVP ) )
        return false;

    var x, y;        /* double */
    
    var event_type = trace_info.event_type;
    switch( event_type )
    {
    case GLG.GlgEventType.TOUCH_START:
        GLG.SetTouchMode();        /* Start dragging via touch events. */
        /* Fall through */
        
    case GLG.GlgEventType.TOUCH_MOVED:
        if( !GLG.GetTouchMode() )
            return;
    case GLG.GlgEventType.MOUSE_PRESSED:
    case GLG.GlgEventType.MOUSE_MOVED:
        x = trace_info.mouse_x * CoordScale;
        y = trace_info.mouse_y * CoordScale;
         
        /* COORD_MAPPING_ADJ is added to the cursor coordinates for precise
           pixel mapping.
        */
        x += GLG.COORD_MAPPING_ADJ;
        y += GLG.COORD_MAPPING_ADJ;
        break;

    default: return;
    }
   
    switch( event_type )
    {
    case GLG.GlgEventType.TOUCH_START:
    case GLG.GlgEventType.MOUSE_PRESSED:
        if( ZoomToMode() )
            return;  // ZoomTo or dragging mode in progress.
        
        /* Start dragging with the mouse on a mouse click. 
           If user clicked of an axis, the dragging will be activated in the
           direction of that axis. If the user clicked in the chart area,
           dragging in both the time and the Y direction will be activated. 
        */
        ChartInfo.ChartVP.SetZoom( null, 's', 0.0 );
        
        // Disable AutoScroll not to interfere with dragging.
        ChangeAutoScroll( 0 ); 
        break;
        
    default: return;
    }
}

//////////////////////////////////////////////////////////////////////////////
// Change chart's AutoScroll mode.
//////////////////////////////////////////////////////////////////////////////
function ChangeAutoScroll( /*int*/ new_value )
{
    if( ChartInfo == null || ChartInfo.Toolbar == null )
        return;

    if( new_value == -1 )  // Use the state of the ToggleAutoScroll button.
    {
        AutoScroll = 
            Math.trunc( ChartInfo.Toolbar.GetDResource( "ToggleAutoScroll/OnState" ) );
    }
    else    // Set to the supplied value. 
    {
        AutoScroll = new_value;
        ChartInfo.Toolbar.SetDResource( "ToggleAutoScroll/OnState", 
                                          AutoScroll );
    }
    
    // Set chart's auto-scroll.
    ChartInfo.Chart.SetDResource( "AutoScroll", AutoScroll );
    
    /* Activate time scrollbar if AutoScroll is Off. The Y value scrollbar 
       uses GLG_PAN_Y_AUTO and appears automatically as needed.
    */
    var pan_x =    /* int */
      ( AutoScroll != 0 ? GLG.GlgPanType.NO_PAN : GLG.GlgPanType.PAN_X );
    
    ChartInfo.ChartVP.SetDResource( "Pan", ( pan_x | GLG.GlgPanType.PAN_Y_AUTO ) );
}

//////////////////////////////////////////////////////////////////////////////
// Changes the time span shown in the graph, adjusts major and minor tick 
// intervals to match the time span.
//////////////////////////////////////////////////////////////////////////////
function SetChartSpan( /*int*/ span_index )
{
    if( ChartInfo == null )
        return;

    var span, major_interval, minor_interval;   /* int */
    
    /* Change chart's time span, as well as major and minor tick intervals.*/
    switch( span_index )
    {
    default:
    case 0:
        span = ONE_MINUTE;
        major_interval = 10;  /* major tick every 10 sec. */
        minor_interval = 1;   /* minor tick every sec. */
        break;
        
    case 1:
        span = 10 * ONE_MINUTE;
        major_interval = ONE_MINUTE * 2; /* major tick every tow minutes. */
        minor_interval = 30;             /* minor tick every 30 sec. */
        break;
        
    case 2:
        span = ONE_HOUR;
        major_interval = ONE_MINUTE * 10; /* major tick every 10 min. */
        minor_interval = ONE_MINUTE;      /* minor tick every min. */
        break;

    case 3:
        span = ONE_HOUR * 8;
        major_interval = ONE_HOUR;        /* major tick every hour. */
        minor_interval = ONE_MINUTE * 15; /* minor tick every 15 minutes. */
        break;
    }
    
    /* Update the menu in the drawing with the initial value if different. */
    ChartInfo.Toolbar.SetDResourceIf( "SpanSelector/SelectedIndex", 
                                      span_index, true );
        
    /* Set intervals before SetZoom() below to avoid redrawing huge number 
       of labels. 
    */
    ChartInfo.Chart.SetDResource( "XAxis/MajorInterval", major_interval );
    ChartInfo.Chart.SetDResource( "XAxis/MinorInterval", minor_interval );
    
    /* Set the X axis span which controls how much data is displayed in the 
       chart. 
    */
    TimeSpan = span;
    ChartInfo.Chart.SetDResource( "XAxis/Span", TimeSpan );
    
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
        ChartInfo.PlotArray[0].SetDResource( "FilterType", 
                                    GLG.GlgChartFilterType.MIN_MAX_FILTER );
        ChartInfo.PlotArray[0].SetDResource( "FilterPrecision", 1.0 );
    }
    else
        ChartInfo.PlotArray[0].SetDResource( "FilterType", 
                                   GLG.GlgChartFilterType.NULL_FILTER );
    
    /* Change time and tooltip formatting to match the selected span. */
    SetTimeFormats();
}

//////////////////////////////////////////////////////////////////////////////
// Sets the formats of time labels and tooltips depending on the selected 
// time span.
//////////////////////////////////////////////////////////////////////////////
function SetTimeFormats()
{
    if( ChartInfo == null )
        return;

    var 
      time_label_format,           /* String */
      time_tooltip_format,         /* String */
      chart_tooltip_format;        /* String */

    /* No additional code is required to use the default settings defined 
       in the drawing. The code below illustrates advanced options for 
       customizing label and tooltip formatting when switching between 
       time spans and data display modes.
       
       For an even greater control over labels and tooltips, an application 
       can define custom Label and Tooltip formatters that will supply 
       custom strings for axis labels and tooltips.
    */
    
    /* Different time formats are used depending on the selected
       time span. See strftime() for all time format options.
    */
    switch( SpanIndex )
    {
    default:  /* 1 minute and 10 minutes spans */
        /* Use the preferred time and date display format for the current 
           locale. 
        */
        time_label_format = "%X%n%x";
        break;
        
    case 2: /* 1 hour span */
        /* Use the 12 hour time display with no seconds, and the default 
         date display format for the current locale.
        */
        time_label_format = "%I:%M %p%n%x";
        break;
        
    case 3: /* 1 hour and 8 hour spans */
        /* Use 24 hour notation and don't display seconds. */
        time_label_format = "%H:%M%n%x";
        break;
    }
    
    ChartInfo.Chart.SetSResource( "XAxis/TimeFormat", time_label_format );
    
    /* Specify axis and chart tooltip format, if different from default 
       formats defined in the drawing.
   */
    time_tooltip_format = 
        "Time: <axis_time:%X> +0.<axis_time_ms:%03.0lf> sec.\nDate: <axis_time:%x>";
    
    /* <sample_time:%s> inherits time format from the X axis. */
    chart_tooltip_format = 
        "Plot <plot_string:%s> value= <sample_y:%.2lf>\n<sample_time:%s>";
    
    /* Set time label and tooltip formats. */
    ChartInfo.Chart.SetSResource( "XAxis/TooltipFormat", time_tooltip_format );
    ChartInfo.Chart.SetSResource( "TooltipFormat", chart_tooltip_format );
}

//////////////////////////////////////////////////////////////////////////////
// Scrolls the chart to the minimum or maximum time stamp to show the 
// most recent or the least recent data. If show_extra is True, adds a 
// few extra seconds in the real-time mode to show a few next updates
// without scrolling the chart.
//
// Enabling AutoScroll automatically scrolls the chart to show current 
// data points when the new time stamp is more recent then the EndValue 
// of the axis, but it is not the case when the chart is scrolled into 
// the future (to the right) - still need to invoke this method.
//////////////////////////////////////////////////////////////////////////////
function ScrollToDataEnd( /*int*/ data_end, /*boolean*/ show_extra_sec )
{
    if( ChartInfo == null )
        return;

    var end_value, extra_sec;   /* double */
    
    if( data_end == this.DONT_CHANGE )
        return;
    
    // Get the min and max time stamp.
    var min_max =   /* GlgMinMax */
        ChartInfo.Chart.GetDataExtent( null, /* x extent */ true );
    if( min_max == null )
        return;
    
    if( show_extra_sec )   
        extra_sec = GetExtraSeconds();
    else
        extra_sec = 0.0;
    
    if( data_end == this.MOST_RECENT )
        end_value = min_max.max + extra_sec;
    else   /* LEAST_RECENT */
        end_value = min_max.min - extra_sec + this.TimeSpan ;
    
    ChartInfo.Chart.SetDResource( "XAxis/EndValue", end_value );
}
   
//////////////////////////////////////////////////////////////////////////////
// Restore Y axis range to the initial Low/High values.
//////////////////////////////////////////////////////////////////////////////
function RestoreInitialYRanges()
{
    if( ChartInfo == null )
        return;

    var axis_array = 
        ChartInfo.Chart.GetResourceObject( "YAxisGroup" ); /* GlgObject */
    
    for( var i=0; i < ChartInfo.NumYAxes; ++i )
    {
        var axis = axis_array.GetElement( i );   /* GlgObject */
        axis.SetDResource( "Low", ChartInfo.Low[ i ] );
        axis.SetDResource( "High", ChartInfo.High[ i ] );
    }
}
   
//////////////////////////////////////////////////////////////////////////////
// Returns true if the chart's viewport is in ZoomToMode.
// ZoomToMode is activated on Dragging and ZoomTo operations.
//////////////////////////////////////////////////////////////////////////////
function ZoomToMode()   /* boolean */
{
    var zoom_mode =    /* int */
       Math.trunc( ChartInfo.ChartVP.GetDResource( "ZoomToMode" ) );
    return ( zoom_mode != 0 );
}

//////////////////////////////////////////////////////////////////////////////
// Abort ZoomTo mode.
//////////////////////////////////////////////////////////////////////////////
function AbortZoomTo()
{
    if( ZoomToMode() )
    {
        // Abort zoom mode in progress.
        ChartInfo.ChartVP.SetZoom( null, 'e', 0.0 ); 
        ChartInfo.ChartVP.Update();
    }
}

//////////////////////////////////////////////////////////////////////////////
// Scale chart parameters for mobile devices.
//////////////////////////////////////////////////////////////////////////////
function AdjustChartForMobileDevices()
{
    if( CoordScale == 1.0 )   // Desktop, no adjustments needed.
        return;

    var result = false;
    result = SetParameter( ChartInfo.MainViewport, "Toolbar/XScale", 1.5 );
    result = SetParameter( ChartInfo.MainViewport, "Toolbar/YScale", 1.5 ); 
    result = SetParameter( ChartInfo.MainViewport, "Toolbar/HeightScale", 1.5 ); 
    result = SetParameter( ChartInfo.MainViewport, 
                           "ChartViewport/OffsetCoeffForMobile", 1.2 ); 
}

//////////////////////////////////////////////////////////////////////////////
// Determines a good number of extra seconds to be added at the end in
// the real-time mode to show a few next updates without scrolling the
// chart.
//////////////////////////////////////////////////////////////////////////////
function GetExtraSeconds()   /* double */
{
   var extra_sec, max_extra_sec;   /* double */

   extra_sec = TimeSpan * 0.1;
   max_extra_sec = ( TimeSpan > ONE_HOUR ? 5.0 : 3.0 );

   if( extra_sec > max_extra_sec )
     extra_sec = max_extra_sec;
   
   return extra_sec;
}

//////////////////////////////////////////////////////////////////////////////
// Used to store and pass information about one data sample.
// Set has_time_stamp=true to supply time_stamp explicitly. 
// Otherwise, the chart will automatically display a time stamp 
// using current time.
//////////////////////////////////////////////////////////////////////////////
function PlotDataPoint( /*double*/ value, /*double*/ time_stamp,
                        /*boolean*/ value_valid )
{
    this.value = value;
    this.time_stamp = time_stamp;
    this.value_valid = value_valid;
};

//////////////////////////////////////////////////////////////////////////////
function ChartInfoObj( /*GlgObject*/ top_viewport )
{
    this.MainViewport = top_viewport;    /* Top viewport */

    // Viewport containing the Chart oject.
    this.ChartVP = null;          /* GlgObject */

    // An array of plot objects.
    this.PlotArray = null;        /* GlgObject[] */

    // Number of plots.
    this.NumPlots = 0;            /* int */
    
    // Number of Y axes.
    this.NumYAxes = 0;            /* int */

    // Arrays storing low and high ranges for all Y axes.
    this.Low = null;             /* double[] */
    this.High = null;            /* double[] */

    // Toolbar object inside the chart drawing.
    this.Toolbar = null;         /* GlgObject */
}
