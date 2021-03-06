/***************************************************************************
  GlgChart class is derived from GlgObjectC and encapsulates methods
  for initializing the chart drawing, updating the stripchart 
  with data and handling user interaction. The example uses the drawing 
  file stripchart_example.g, includes a stripchart widget
  as well as interface widgets allowing to scroll and zoom the graph. 

  For demo pourposes, the chart displays simulated data, using
  DemoDataFeed class, which is derived from DataFeedC defined 
  in DataFeed.h. The application may provide a custom implementation 
  of the DataFeedC, supplying real-time application specific data 
  to the chart.
***************************************************************************/

#include "GlgChart.h"
#include "DemoDataFeed.h"
#include "LiveDataFeed.h"

/* Convenient time span constants. */
#define ONE_MINUTE    60
#define ONE_HOUR      3600
#define ONE_DAY       ( 3600 * 24 )

#define TIME_SPAN     ONE_MINUTE     /* Time Span in sec. */
#define SCROLL_INCREMENT 10          /* Scroll increment in sec. */

/* Prefill time interval, specifies amount of data to prefill in the 
   real time chart. 
*/
#define PREFILL_SPAN  ONE_HOUR

/* Global functions. */
void UpdateChart( GlgChart * chart, GlgLong * timer_id );
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, 
                               GlgLong interval );

/* Used by DataFeed to return data values. */
DataPoint data_point;

/*----------------------------------------------------------------------
| Constructor. 
*/
GlgChart::GlgChart( void )
{
   /* Use simulated data by default. */
   RandomData = true;
   
   /* If set to false, pre-filling the chart's buffer 
      with data on start-up is turned off.
   */
   PrefillData = true;

   /* If set to true, the application supplies time stamp for each data 
      sample explicitly in DataFeed.GetPlotPoint(). Otherwise, time stamp 
      is automatically generated by the chart using current time.
   */
   SupplyTimeStamp = false;

   UpdateInterval = 100;  /* Update interval in msec */
   AutoScroll = true;     /* Auto-scroll state: enabled or disabled.*/
   TimeSpan = TIME_SPAN;  /* Currently displayed Time axis span in seconds */
   StoredScrollState;     /* Stored AutoScroll state to be restored if 
                             ZoomTo is aborted. */

   ChartOrientation = GLG_HORIZONTAL;

   /* Scroll increment for the X axis. */   
   ScrollFactor = SCROLL_INCREMENT / TIME_SPAN;

   TimerID = 0;
   DataFeed = NULL;
}

/*----------------------------------------------------------------------
| Destructor. 
*/
GlgChart::~GlgChart( void )
{
     delete Plots;
     delete Low;
     delete High;
     delete DataFeed;
}

/*----------------------------------------------------------------------
| Initialize chart parameters.
*/
void GlgChart::Init( void )
{
   EnableCallback( GLG_INPUT_CB, NULL );
   EnableCallback( GLG_TRACE_CB, NULL );

   /* Initialize Chart parameters before hierarchy is set up. */
   InitBeforeH();
   
   /* Set up object hierarchy. */ 
   SetupHierarchy();

   /* Initialize Chart parameters after hierarchy setup took place. */
   InitAfterH();

   // Add DataFeed object used to supply chart data.
   AddDataFeed();
   
   /* Prefill chart's history buffer with data. */
   if( PrefillData )
     FillChartHistory();
}
   
/*----------------------------------------------------------------------
| Add DataFeed object for supplying chart data. The example uses
| DemoDataFeed that supplies simulated demo data. To supply real-time
| data, an application should provide a custom implementation of the
| LiveDataFeed class.
*/
void GlgChart::AddDataFeed()
{
   if( DataFeed )
     delete DataFeed;

   if( RandomData )
   {
      DataFeed = new DemoDataFeed( this );
      error( "Using DemoDataFeed.", GlgFalse );
   }
   else
   {
      DataFeed = new LiveDataFeed( this );
      error( "Using LiveDataFeed.", GlgFalse );
   }
}

/*----------------------------------------------------------------------
| Initializes chart parameters before hierarchy setup occurred.
*/
void GlgChart::InitBeforeH( void )
{
   double major_interval, minor_interval;
   double d_value;

   ChartVP = GetResourceObject( "ChartViewport" );
   if( ChartVP.IsNull() )
     error( "Can't find ChartViewport", GlgTrue );
   
   Chart = ChartVP.GetResourceObject( "Chart" );
   if( Chart.IsNull() )
     error( "Can't find Chart object", GlgTrue );
   
   // Retrieve the chart orientation, i.e. vertical or horizontal.
   Chart.GetResource( "Orientation", &d_value );
   ChartOrientation = d_value;
   
   // Retrieve number of plots defined in .g file.
   Chart.GetResource( "NumPlots", &d_value );
   NumPlots = d_value;
   
   // Retrieve number of Y axes defined in .g file.
   Chart.GetResource( "NumYAxes", &d_value );
   NumYAxes = d_value;

   /* Set Time Span for the X axis. */
   Chart.SetResource( "XAxis/Span", TimeSpan );

   /* Set tick intervals for the Time axis.
      Use positive values for absolute time interval, for example
      set major_interval = 10 for a major tick every 10 sec.
   */
   major_interval = -6;      // 6 major intervals
   minor_interval = -5;      // 5 minor intervals
   Chart.SetResource( "XAxis/MajorInterval", major_interval );
   Chart.SetResource( "XAxis/MinorInterval", minor_interval );
   
   /* Enable AutoScroll, both for the toggle button and the chart. */
   ChangeAutoScroll( 1 );

   /* Set Chart Zoom mode. It was set and saved with the drawing, but do it 
      again programmatically just in case.
   */
   ChartVP.SetZoomMode( NULL, &Chart, NULL, GLG_CHART_ZOOM_MODE );
}

/*----------------------------------------------------------------------
| Initializes chart parameters after hierarchy setup has occurred.
*/
void GlgChart::InitAfterH( void )
{
   GlgLong i;
   GlgObjectC plot_array;
   GlgObjectC axis_array;
   GlgObjectC axis;

   /* Allocate an array to store plot object IDs. */
   Plots = new GlgObjectC[ NumPlots ];

   plot_array = Chart.GetResourceObject( "Plots" );
   
   /* Store objects IDs for each plot. */
   for( i=0; i<NumPlots; ++i )
   {
      Plots[ i ] = plot_array.GetElement( i ); 

      /* If needed, set EdgeColor and Annotation for each plot:
         Plots[i].SetResource( "EdgeColor", r, g, b );
         Plots[i].SetResource( "Annotation", annotation_string );
      */
   }

   /* Store initial range for each Y axis, used to restore ranges
      on zoom reset. Assumes that each plot is linked to the 
      corresponding axis in the drawing.
   */         
   Low = new double[ NumYAxes ];
   High = new double[ NumYAxes ];
   
   axis_array = Chart.GetResourceObject( "YAxisGroup" );
   for( i=0; i<NumYAxes; ++i )
   {
      axis = axis_array.GetElement( i );
      axis.GetResource( "Low", &Low[ i ] );
      axis.GetResource( "High", &High[ i ] );
   }
}

/*----------------------------------------------------------------------
|  Start dynamic updates.
*/
void GlgChart::StartUpdates()
{
   /* Start update timer. */
   TimerID = GlgAddTimeOut( AppContext, UpdateInterval, 
			    (GlgTimerProc) UpdateChart, this );
}

/*----------------------------------------------------------------------
| Stop dynamic updates.
*/
void GlgChart::StopUpdates()
{
   /* Stop update timers */
   if( TimerID )
   {
      GlgRemoveTimeOut( TimerID );
      TimerID = 0;
   }
}

/*----------------------------------------------------------------------
| Timer procedure to update the chart with new data.
*/
void UpdateChart( GlgChart * chart, GlgLong * timer_id )
{
   GlgULong sec, microsec;

   /* Start time for adjusting timer intervals. */
   GlgGetTime( &sec, &microsec );

   /* This example uses demo data. An application will provide a
      custom DataFeed object for supplying real-time chart data.
   */
   
   GlgLong num_plots = chart->NumPlots;

   // Update plot lines with new data supplied by the DataFeed object.
   for( int i=0; i<num_plots; ++i )
   {
      /* Use DataFeed to get new data value. The DataFeed object
	 fills the data_point object with value, time_stamp, etc.
      */
      if( chart->DataFeed->GetPlotPoint( i, data_point ) )
        /* Push a new data point into the chart's plot. */
        chart->PushPlotPoint( chart->Plots[ i ], data_point );
      else
        chart->error( "Error getting plot data sample.", GlgFalse );
   }

   chart->Update();

   /* Adjust timer intervals to have a constant update rate regardless
      of the time it takes to redraw the chart.
   */      
   GlgLong timer_interval = 
     GetAdjustedTimeout( sec, microsec, chart->UpdateInterval );

   chart->TimerID = 
     GlgAddTimeOut( chart->AppContext, timer_interval, 
		    (GlgTimerProc) UpdateChart, chart );
}

/*----------------------------------------------------------------------
| Pushes the data_point's data into the plot.
*/
void GlgChart::PushPlotPoint( GlgObjectC& plot, DataPoint& data_point )
{
   /* Supply plot value for the chart via ValueEntryPoint. */
   plot.SetResource( "ValueEntryPoint", data_point.value );
   
   if( data_point.time_stamp )
   {
      /* Supply an optional time stamp. If not supplied, the chart will 
	 automatically generate a time stamp using current time. 
      */
      plot.SetResource( "TimeEntryPoint", data_point.time_stamp );
   }
   
   if( !data_point.value_valid )
   {	   
      /* If the data point is not valid, set ValidEntryPoint resource to 
	 display holes for invalid data points. If the point is valid,
	 it is automatically set to 1. by the chart.
      */
      plot.SetResource( "ValidEntryPoint", 0. );
   }
}

/*----------------------------------------------------------------------
| Fill the graph's history buffer with demo data. 
*/
void GlgChart::FillChartHistory()
{
   double current_time = GetCurrTime();
   
   /* Fill the amount of data requested by the PREFILL_SPAN, up to the 
      available chart's buffer size defined in the drawing.
      Add an extra second to avoid rounding errors.
   */
   int num_seconds = PREFILL_SPAN + 1;
   
   double d_value;
   Chart.GetResource( "BufferSize", &d_value );
   int buffer_size = d_value;

   if( buffer_size < 1 )
     buffer_size = 1;
   
   int max_num_samples;
   if( RandomData )
   {
      /* In random demo data mode, simulate data stored once per second. */
      double samples_per_second = 1.0;
      max_num_samples = (int) ( num_seconds * samples_per_second );
      
      if( max_num_samples > buffer_size )
        max_num_samples = buffer_size;
   }
   else
     max_num_samples = buffer_size;
   
   double start_time = current_time - num_seconds;
   double end_time = current_time;   /* Stop at the current time. */
   
   for( int i=0; i<NumPlots; ++i )
   { 
      DataArrayType data_array;
      if( !DataFeed->GetHistPlotData( i, start_time, end_time,
                                     max_num_samples, data_array ) 
          || data_array.empty() )
      {
         error( "No historical data for plot_index = " + i, GlgFalse );
         continue;
      }
      
      // Push data into the chart's plot with a given index.
      FillPlotData( Plots[ i ], data_array );

      // Free data_array.
      DataFeed->FreePlotData( data_array );
   }
}

/*----------------------------------------------------------------------
| Fills plot with data from the provided data array.
*/
void GlgChart::FillPlotData( GlgObjectC& plot,
                             DataArrayType& data_array )
{   
   DataArrayType::iterator it;
   for( it = data_array.begin(); it != data_array.end(); ++it )
   {
      DataPoint * data_point = *it;
      PushPlotPoint( plot, *data_point );
   }
}

/*----------------------------------------------------------------------
| GLG Input callback.
*/
void GlgChart::Input(GlgObjectC& viewport, GlgObjectC& message)
{
   CONST char
     * format,
     * action,
     * origin,
     * subaction;
   
   /* Get the message's format, action and origin. */
   message.GetResource( "Format", &format );
   message.GetResource( "Action", &action );
   message.GetResource( "Origin", &origin );
   message.GetResource( "SubAction", &subaction );
      
   /* Handle window closing. May use viewport's name.*/
   if( strcmp( format, "Window" ) == 0 &&
       strcmp( action, "DeleteWindow" ) == 0 )
   {
      /* Closing main window: exit. */
      exit( GLG_EXIT_OK );
   }

   if( strcmp( format, "Button" ) == 0 )         /* Handle button clicks */
   {
      if( strcmp( action, "Activate" ) != 0 &&     /* Push button */
          strcmp( action, "ValueChanged" ) != 0 )   /* Toggle button */
	return;

      AbortZoomTo();

      if( strcmp( origin, "ToggleAutoScroll" ) == 0 )
      {         
         ChangeAutoScroll( -1 ); /* Set Chart AutoScroll based on 
				    ToggleAuScroll toggle setting. */ 
      }
      else if( strcmp( origin, "ZoomTo" ) == 0 )
      {
	 /* Start ZoomTo operation */
         ChartVP.SetZoom( NULL, 't', 0. );  
      }
      else if( strcmp( origin, "ResetZoom" ) == 0 )
      {         
	 /* Set initial time span and reset initial Y ranges. */
         SetChartSpan( TimeSpan );  
	 RestoreInitialYRanges();   
      }
      else if( strcmp( origin, "ScrollBack" ) == 0 )
      {
         ChangeAutoScroll( 0 );

         /* Scroll left. Scrolling can be done by either setting chart's 
	    XAxis/EndValue resource or by using GlgSetZoom().
	 */
	 /*
	 double end_value;
         GetResource( "ChartViewport/Chart/XAxis/EndValue", 
	               &end_value );
	 end_value -= SCROLL_INCREMENT;
	 SetResource( "ChartViewport/Chart/XAxis/EndValue", 
	               end_value );
	 */

         switch( ChartOrientation )
         {
          case GLG_HORIZONTAL:
          default:
            ChartVP.SetZoom( NULL, 'l', ScrollFactor );
            break;
          case GLG_VERTICAL:
            ChartVP.SetZoom( NULL, 'd', ScrollFactor );
            break;
         }

      }
      else if( strcmp( origin, "ScrollForward" ) == 0 )
      {
         ChangeAutoScroll( 0 );

         switch( ChartOrientation )
         {
          case GLG_HORIZONTAL:
          default:
            // Scroll right.
            ChartVP.SetZoom( NULL, 'r', ScrollFactor );
            break;
          case GLG_VERTICAL:
            // Scroll up.
            ChartVP.SetZoom( NULL, 'u', ScrollFactor );
            break;
         }
      }
      else if( strcmp( origin, "ScrollToRecent" ) == 0 )
      {
	 /* Scroll to show most recent data. */
	 ScrollToDataEnd();
      }
 
      Update();
   }
   else if( strcmp( format, "Chart" ) == 0 && 
            strcmp( action, "CrossHairUpdate" ) == 0 )
   {
      /* To avoid slowing down real-time chart updates, invoke Update() 
         to redraw cross-hair only if the chart is not updated fast 
         enough by the timer.
      */
      if( UpdateInterval > 100 )
        Update();
   }            

   /* Handle Zoom/Pan events */
   else if( strcmp( action, "Zoom" ) == 0 )
   {
      if( strcmp( subaction, "ZoomRectangle" ) == 0 )
      {
         /* Store AutoSCroll state to restore it if ZoomTo is aborted. */
         StoredScrollState = AutoScroll;

         /* Stop scrolling when ZoomTo action is started. */
         ChangeAutoScroll( 0 );
      }
      else if( strcmp( subaction, "End" ) == 0 )
      {
         /* No additional actions on finishing ZoomTo. The Value axis 
            scrollbar appears automatically if needed: 
            it is set to PAN_Y_AUTO for a horizontal chart
            and PAN_X_AUTO for a vertical chart.
            Don't resume scrolling: it'll scroll too fast since we zoomed 
            in. Keep it still to allow inspecting zoomed data.
         */
      }
      else if( strcmp( subaction, "Abort" ) == 0 )
      {
         /* Resume scrolling if it was on. */
         ChangeAutoScroll( StoredScrollState );         
      }

      Update();
   }
   else if( strcmp( action, "Pan" ) == 0 )
   {
      /* This code may be used to perform custom action when dragging the 
         chart's data with the mouse. 
      */
      if( strcmp( subaction, "Start" ) == 0 )   /* Chart dragging start */
      {
      }
      else if( strcmp( subaction, "Drag" ) == 0 )    /* Dragging */
      {
      }
      else if( strcmp( subaction, "ValueChanged" ) == 0 )   /* Scrollbars */
      {
      }
      /* Dragging ended or aborted. */
      else if( strcmp( subaction, "End" ) == 0 || 
               strcmp( subaction, "Abort" ) == 0 )
      {
      }     
   }   
}

/*----------------------------------------------------------------------
| Scroll to the end of the data history buffer.
*/
void GlgChart::ScrollToDataEnd()
{
   GlgMinMax min_max;

   /* Get min/max time stamp. */
   if( !Chart.GetDataExtent( NULL, &min_max, true /*X extent*/ ) )
     return;

   Chart.SetResource( "XAxis/EndValue", min_max.max );
}

/*----------------------------------------------------------------------*/
void GlgChart::ChangeAutoScroll( GlgLong new_value )
{
   double auto_scroll;

   if( new_value == -1 )  /* Use the state of the ToggleAutoScroll button. */
   {
      GetResource( "Toolbar/ToggleAutoScroll/OnState", 
		   &auto_scroll );
      AutoScroll = auto_scroll;
   }
   else    /* Set to the supplied value. */
   {
      AutoScroll = new_value;
      SetResource( "Toolbar/ToggleAutoScroll/OnState", (double) AutoScroll );
   }

   /* Set chart's auto-scroll. */
   Chart.SetResource( "AutoScroll", (double) AutoScroll );

   /* Activate time scrollbar if AutoScroll is Off. The Value scrollbar 
      uses GLG_PAN_Y_AUTO (horizontal chart) or 
      GLG_PAN_X_AUTO (vertical chart) and appears automatically as needed.
   */
   int pan_time_axis, pan_value_axis;
   switch( ChartOrientation )
   {
    case GLG_HORIZONTAL:
    default:
      pan_time_axis = GLG_PAN_X;
      pan_value_axis = GLG_PAN_Y_AUTO;
      break;
    case GLG_VERTICAL:
      pan_time_axis = GLG_PAN_Y;
      pan_value_axis = GLG_PAN_X_AUTO;
      break;
   }
   
   pan_time_axis = ( AutoScroll != 0 ? GLG_NO_PAN : pan_time_axis );
   ChartVP.SetResource( "Pan", (double) ( pan_time_axis | pan_value_axis ) );
}

/*----------------------------------------------------------------------
| Changes the time span shown in the graph.
*/
void GlgChart::SetChartSpan( GlgLong span )
{
   if( span > 0 )
     Chart.SetResource( "XAxis/Span", (double) span );
   else  /* Reset span to show all data accumulated in the buffer. */
     ChartVP.SetZoom( NULL, 'N', 0. );
}

/*----------------------------------------------------------------------*/
void GlgChart::RestoreInitialYRanges()
{
   GlgObjectC axis_array;
   GlgObjectC axis;

   axis_array = Chart.GetResourceObject( "YAxisGroup" );
   for( int i=0; i<NumYAxes; ++i )
   {
      axis = axis_array.GetElement( i ); 
      axis.SetResource( "Low", Low[ i ] );
      axis.SetResource( "High", High[ i ] );
   }
}

/*----------------------------------------------------------------------
| Returns true if the chart's viewport is in ZoomToMode.
| ZoomToMode is activated on Dragging and ZoomTo operations.
*/
GlgLong GlgChart::ZoomToMode()
{
   double zoom_mode;

   ChartVP.GetResource( "ZoomToMode", &zoom_mode );
   if( zoom_mode )
     return (GlgLong) zoom_mode;

   return false;
}

/*----------------------------------------------------------------------*/
void GlgChart::AbortZoomTo()
{
   if( ZoomToMode() )
   {
      /* Abort zoom mode in progress. */
      ChartVP.SetZoom( NULL, 'e', 0. ); 
      Update();
   }
}

/*----------------------------------------------------------------------
| Used to obtain coordinates of the mouse click.
*/
void GlgChart::Trace(  GlgObjectC& callback_viewport, 
		       GlgTraceCBStruct * trace_data )
{      
   int
     event_type = 0,
     x, y,
     width, height;

   GlgObjectC event_vp;
   event_vp = trace_data->viewport;

   /* Process only events that occur in ChartViewport */ 
   if( !event_vp.Same( ChartVP ) )
     return;

   /* Platform-specific code to extract event information. */
#ifndef _WINDOWS
   switch( trace_data->event->type )
   {
    case ButtonPress:
      if( trace_data->event->xbutton.button != 1 )
        return;   /* Use only the left button clicks. */

      x = trace_data->event->xbutton.x;
      y = trace_data->event->xbutton.y;
      event_type = BUTTON_PRESS;
      break;
      
    case MotionNotify:
      x = trace_data->event->xmotion.x;
      y = trace_data->event->xmotion.y;
      event_type = MOUSE_MOVE;
      break;

    case ConfigureNotify:
      width = trace_data->event->xconfigure.width;
      height = trace_data->event->xconfigure.height;
      event_type = RESIZE;
      break;
      
    default: return;
   }
#else
   switch( trace_data->event->message )
   {
    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
      x = GET_X_LPARAM( trace_data->event->lParam );
      y = GET_Y_LPARAM( trace_data->event->lParam );

      switch( trace_data->event->message )
      {
       case WM_LBUTTONDOWN:   
         event_type = BUTTON_PRESS; 
         break;
       case WM_MOUSEMOVE:
         event_type = MOUSE_MOVE;
         break;
      }
      break;

    case WM_SIZE:
      width = LOWORD( trace_data->event->lParam );
      height = HIWORD( trace_data->event->lParam );
      event_type = RESIZE;
      break;
      
    default: return;
   }
#endif
   
   switch( event_type )
   {
    case RESIZE:
      break;

    case BUTTON_PRESS:
      if( ZoomToMode() )
        return; /* ZoomTo or dragging mode in progress. */
      
      /* Start dragging with the mouse on a mouse click. 
         If user clicked on an axis, the dragging will be activated in the
         direction of that axis. If the user clicked in the chart area,
         dragging in both the time and the Y direction will be activated.
      */
      SetZoom( "ChartViewport", 's', 0. );

      /* Disable AutoScroll not to interfere with dragging */
      ChangeAutoScroll( 0 );        
      break;

    case MOUSE_MOVE:
      break;

    default: return;
   } 
}

/*----------------------------------------------------------------------
| Set window size in screen cooridnates. 
*/
void GlgChart::SetSize( GlgLong x, GlgLong y, 
                        GlgLong width, GlgLong height )
{
   SetResource( "Point1", 0., 0., 0. );
   SetResource( "Point2", 0., 0., 0. );
   
   SetResource( "Screen/XHint", (double) x );
   SetResource( "Screen/YHint", (double) y );
   SetResource( "Screen/WidthHint", (double) width );
   SetResource( "Screen/HeightHint", (double) height );
}

/*----------------------------------------------------------------------
| Return exact time including fractions of seconds.
*/
double GlgChart::GetCurrTime()
{
   GlgULong sec, microsec;

   GlgGetTime( &sec, &microsec );
   return sec + microsec / 1000000.;
}

/*----------------------------------------------------------------------*/
void GlgChart::error( CONST char * str, GlgBoolean quit )
{
   GlgError( GLG_USER_ERROR, (char*) str );
   if( quit )
     exit( GLG_EXIT_ERROR );
}


/*----------------------------------------------------------------------*/
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, 
			   GlgLong interval )
{
   GlgULong sec2, microsec2;
   GlgLong elapsed_time, adj_interval;

   GlgGetTime( &sec2, &microsec2 );  /* End time */
   
   /* Elapsed time in millisec */
   elapsed_time = 
     ( sec2 - sec1 ) * 1000 + (GlgLong) ( microsec2 - microsec1 ) / 1000;

   /* Maintain constant update interval regardless of the system speed. */
   if( elapsed_time + 20 >= interval )
      /* Slow system: update as fast as we can, but allow a small interval 
         for handling input events. */
     adj_interval = 20;
   else
     /* Fast system: keep constant update interval. */
     adj_interval = interval - elapsed_time;

#if DEBUG_TIMER
   printf( "sec= %ld, msec= %ld\n", sec2 - sec1, microsec2 - microsec1 );
   printf( "*** elapsed= %ld, requested= %ld, adjusted= %ld\n",
	  (long) elapsed_time, (long) interval, (long) adj_interval );
#endif

   return adj_interval;
}


