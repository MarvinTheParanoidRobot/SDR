/*****************************************************************************
  This example demonstrates the following features:
   - how to display and update a GLG realtime stripchart;
   - use the charts integrated interactive behavior, 
     such as scrolling and dragging;
   - show/hide a marker for a data sample sample selected with a double click.

  The program loads a GLG drawing stripchart.g, which includes a ChartViewport
  as well as interface widgets allowing to scroll and zoom the graph. 
  The graph is initilaized in InitChart() function and updated with 
  simulated data using a timer procedure UpdateChart().

  GetPlotValue() method, used in this example to generate demo data,
  may be replaced with a custom data feed.

  The X axis labels display current date and time using the time format
  defined in the drawing. By default, the labels are generated automatically 
  by the graph, however the program may supply a time stamp for 
  each data iteration, by setting use_current_time=False in UpdateChart()
  and replacing code in GetTimeStamp() method.

  This example is written using GLG Intermediate API, which allows
  easier and more convenient coding, as well as better chart performance.
  In case GLG Intermediate API is not availaible, GLG Standard API
  can be used to animate the chart, as shown in the example GlgRealTimeChartG.c.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "GlgApi.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

#define TIME_SPAN 60          /* Time Span in seconds */
#define SCROLL_INCREMENT 10   /* Scroll increment in seconds */
#define NUM_PLOTS 3           /* Number of plots */

typedef enum
{
   BUTTON_PRESS = 0,
   RESIZE,
   MOUSE_MOVE
} EventType;

/* Function prototypes */
void InitChartBeforeH( void );
void InitChartAfterH( void );
void UpdateChart( GlgAnyType data, GlgIntervalID * id );
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void Trace( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void ScrollToDataEnd();
void ChangeAutoScroll( GlgLong new_state );
void SetChartSpan( GlgLong span );
void RestoreInitialYRanges( void );
void MarkDataSample( double x, double y );
GlgDataSample * GetDataSample( double x, double y );
int GetClickCount( double x, double y );
GlgLong ZoomToMode( void );
void AbortZoomTo( void );
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, GlgLong interval );
void PreFillChartData( void );
void GetChartData( double time_stamp, GlgBoolean use_current_time );
double GetPlotValue( GlgLong plot_index );
double GetCurrTime( void );
double GetTimeStamp( void );
void error( char * string, GlgLong quit );
void dblclick_timer( GlgAnyType data, GlgIntervalID * id );

GlgObject
  Drawing, 
  ChartVP,
  Chart;

GlgObject Plots[ NUM_PLOTS ];

GlgLong
  BufferSize = 50000,    /* Number of samples in the history buffer per line. */ 
  PrefillData = True,    /* Setting to False supresses pre-filling the 
                            chart's buffer with data on start-up. */
  UpdateInterval = 100,  /* Update interval in msec */
  AutoScroll = True,     /* Current auto-scroll state: enabled or disabled. */
  TimeSpan = TIME_SPAN,  /* Currently displayed Time axis span in seconds */
  StoredScrollState;     /* Stored AutoScroll state to be restored if ZoomTo
                            is aborted. */

double
  /* Low and High range of the incoming data values. */
  Low = 0.,
  High = 10.;

double num_plots_drawing;  /* Number of plots as defined in .g file. */

double 
  /* Scroll increment for the chart X axis. */   
  ScrollFactor = SCROLL_INCREMENT / TIME_SPAN;


/* Flag to identify a double-click. */
GlgBoolean isSingleClick = False;

/* Selection delta in pixels. */
double 
  dx = 5., 
  dy = 5.; 

GlgAppContext AppContext;  /* Global, used to install a timeout. */

#include "GlgMain.h"       /* Cross-platform entry point. */

/*----------------------------------------------------------------------
*/
int GlgMain( int argc, char **argv, GlgAppContext app_context )
{   
   char * full_path;

   /* Picks up the current locale settings that define the preferred time 
      label format. */
   GlgInitLocale( NULL );

   /* Initialize GLG */
   AppContext = GlgInit( False, app_context, argc, argv );

   GlgSetDResource( (GlgObject)0, "$config/GlgMouseTooltipTimeout", 0.25 );

   full_path = GlgGetRelativePath( argv[0], "stripchart_markers.g" );
   Drawing = GlgLoadWidgetFromFile( full_path );
   if( !Drawing )
     error( "Can't load drawing file.", True );

   GlgFree( full_path );

   ChartVP = GlgGetResourceObject( Drawing, "ChartViewport" );
   Chart = GlgGetResourceObject( ChartVP, "Chart" );

   /* Setting widget dimensions in screen pixels. If not set, default 
      dimensions will be used as set in the drawing file by
      Point1/Point2 control points of the $Widget viewport.
      */
   GlgSetGResource( Drawing, "Point1", 0., 0., 0. );
   GlgSetGResource( Drawing, "Point2", 0., 0., 0. );
   GlgSetDResource( Drawing, "Screen/WidthHint",  800. );
   GlgSetDResource( Drawing, "Screen/HeightHint", 600. );

   /* Setting the window title. */
   GlgSetSResource( Drawing, "ScreenName", "GLG RealTime Stripchart Example" );

   GlgAddCallback( Drawing, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );
   GlgAddCallback( Drawing, GLG_TRACE_CB, (GlgCallbackProc)Trace, NULL );

   /* Initialize Chart parameters. Invoked before hierarchy setup. */
   InitChartBeforeH(); 

   /* Set up object hierarchy. */ 
   GlgSetupHierarchy( Drawing );

   /* Initialize Chart parameters after hierarchy setup took place. */
   InitChartAfterH(); 

   /* Display the graph. */
   GlgUpdate( Drawing );
   
   /* Start the timer to update the chart with data. */
   GlgAddTimeOut( AppContext, UpdateInterval, (GlgTimerProc)UpdateChart, 
                  NULL );

   return (int) GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| Initializes chart parameters before hierarchy setup occurred.
*/
void InitChartBeforeH()
{
   /* Retrieve number of plots defined in .g file. */
   GlgGetDResource( Chart, "NumPlots", &num_plots_drawing );

   /* Set new number of plots as needed. */
   GlgSetDResource( Chart, "NumPlots", NUM_PLOTS );

   /* Set Time Span for the X axis. */
   GlgSetDResource( Chart, "XAxis/Span", TimeSpan );

   /* Set data value range. Since the graph has one Y axis and
      common data range for the plots, Low/High data range is
      set on the YAxis level.
     */
   GlgSetDResource( Chart, "YAxis/Low", Low );
   GlgSetDResource( Chart, "YAxis/High", High );

   /* Enable AutoScroll, both for the toggle button and the chart. */
   ChangeAutoScroll( 1 );

   /* Set Chart Zoom mode. It was set and saved with the drawing, but do it 
      again programmatically just in case.
      */
   GlgSetZoomMode( ChartVP, NULL, Chart, NULL, GLG_CHART_ZOOM_MODE );

   /* Set Chart TooltipMode to XY, to match the data sample selection
      mode on a doble-click in GetDataSample().
   */
   GlgSetDResource( Chart, "TooltipMode", 3. );
}

/*----------------------------------------------------------------------
| Initializes chart parameters after hierarchy setup has occurred.
*/
void InitChartAfterH()
{
   GlgLong i;
   GlgObject plot_array;
   char * res_name;

   plot_array = GlgGetResourceObject( Chart, "Plots" );
 
   /* Store object IDs for individual plot objects. 
      Make markers invisible for each plot.
   */
   for( i=0; i<NUM_PLOTS; ++i )
   {
      Plots[i] = GlgGetElement( plot_array, i );
      GlgSetDResource( Plots[i], "Marker/Visibility", 0. );
   }
   
   /* For the existing plots, use color and line annotation setting 
      from the drawing; initialize new plots using random colors and strings
      for demo purposes. 
   */
   if( num_plots_drawing < NUM_PLOTS )
     for( i=num_plots_drawing; i < NUM_PLOTS; ++i )
     {
	/* Using a random color for a demo. */
	GlgSetGResource( Plots[i], "EdgeColor", 
			 GlgRand(0., 1.), GlgRand(0., 1.), GlgRand(0., 1.) );

	res_name = GlgCreateIndexedName( "Var%", i );
	GlgSetSResource( Plots[i], "Annotation", res_name );
	GlgFree( res_name );
     }
   
   /* Prefill chart's history bufer with data. */
   if( PrefillData )
     PreFillChartData();
}

#define DEBUG_TIMER False
/*----------------------------------------------------------------------
| Timer procedure to update the chart with new data.
*/
void UpdateChart( GlgAnyType data, GlgIntervalID * id )
{
   GlgULong sec1, microsec1;
   GlgLong timer_interval;
   GlgBoolean use_current_time = True;

   /* Start time for adjusting timer intervals. */
   GlgGetTime( &sec1, &microsec1 );

  /* Supply demo data to update plot lines. 
      In this example, current time is automatically supplied
      by the chart. The application may supply a time stamp instead,
      in which case code needs to be added to GetTimeStamp()
      to return a time stamp as needed. For demo purposes,
      GetTimeStamp() returns current time in sec.
   */
   GetChartData( use_current_time ? 0. : GetTimeStamp(), 
		 use_current_time);

   GlgUpdate( Drawing );
   GlgSync( Drawing );    /* Improves interactive response */

   /* Adjust timer intervals to have a constant update rate regardless
      of the time it takes to redraw the chart.
   */      
   timer_interval = GetAdjustedTimeout( sec1, microsec1, UpdateInterval );

   GlgAddTimeOut( AppContext, timer_interval, (GlgTimerProc)UpdateChart, 
                  NULL );
}

/*----------------------------------------------------------------------
| Supplies chart data for each plot.
*/
void GetChartData( double time_stamp, GlgBoolean use_current_time )
{
   GlgLong i;
   double value;

   for( i=0; i<NUM_PLOTS; ++i )
   {
      /* Get new data value. The example uses simulated data, while
	 an application will replace code in GetPlotValue() to
	 supply application specific data for a given plot index.
        */
      value = GetPlotValue( i );

      /* Supply plot value for the chart via ValueEntryPoint. */
      GlgSetDResource( Plots[i], "ValueEntryPoint", value );

      /* Supply an optional time stamp. If not supplied, the chart will 
         automatically fill the time stamp with the current time. 
      */
      if( !use_current_time )  
	 GlgSetDResource( Plots[i], "TimeEntryPoint", time_stamp );

      /* Set ValidEntryPoint resource only if a graph needs to display
	 holes for invalid data points.
      */
      GlgSetDResource( Plots[i], "ValidEntryPoint", 1. /*valid*/ );
   }
}


/*----------------------------------------------------------------------
| Handle user interaction.
*/
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{
   GlgObject message_obj;
   char
     * origin,
     * format,
     * action,
     * subaction;
   
   message_obj = (GlgObject) call_data;

   GlgGetSResource( message_obj, "Origin", &origin );
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "SubAction", &subaction );

   /* Handle window closing. */
   if( strcmp( format, "Window" ) == 0 )
   {
      if( strcmp( action, "DeleteWindow" ) == 0 )
      {
	 /* Closing main window: exit. */
	 exit( GLG_EXIT_OK );
      }
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
         GlgSetZoom( ChartVP, NULL, 't', 0. );  /* Start ZoomTo operation */
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

         /* Scroll left.
	    Note: Scrolling can be done by either setting chart's 
	    XAxis/EndValue resource or by using GlgSetZoom().
	 */
	 /* 
	 double end_value;
         GlgGetDResource( Chart, "XAxis/EndValue", &end_value );
	 end_value -= SCROLL_INCREMENT;
	 printf( "new end value = %lf\n", end_value );
	 GlgSetDResource( Chart, "XAxis/EndValue", end_value );
	 */

         GlgSetZoom( ChartVP, NULL, 'l', ScrollFactor );
      }
      else if( strcmp( origin, "ScrollForward" ) == 0 )
      {
         ChangeAutoScroll( 0 );

         /* Scroll right */
         GlgSetZoom( ChartVP, NULL, 'r', ScrollFactor );
      }
 
      GlgUpdate( Drawing );
   }
   else if( strcmp( origin, "ScrollToRecent" ) == 0 )
   {
      /* Scroll to show most recent data. */
      ScrollToDataEnd();
   }
   
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
         /* No addtional actions on finishing ZoomTo. The Y scrollbar 
            appears automatically if needed: it is set to GLG_PAN_Y_AUTO. 
            Don't resume scrolling: it'll scroll too fast since we zoomed in.
            Keep it still to allow inspecting zoomed data.
         */
      }
      else if( strcmp( subaction, "Abort" ) == 0 )
      {
         /* Resume scrolling if it was on. */
         ChangeAutoScroll( StoredScrollState );         
      }

      GlgUpdate( Drawing );
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
void ScrollToDataEnd( )
{
   GlgMinMax min_max;

   /* Get min/max time stamp. */
   if( !GlgGetDataExtent( Chart, NULL, &min_max, True /*X extent*/ ) )
     return;
   
   GlgSetDResource( Chart, "XAxis/EndValue", min_max.max );
}

/*----------------------------------------------------------------------
|
*/
void ChangeAutoScroll( GlgLong new_value )
{
   double auto_scroll;
   GlgLong pan_x;

   if( new_value == -1 )  /* Use the state of the ToggleAutoScroll button. */
   {
      GlgGetDResource( Drawing, "Toolbar/ToggleAutoScroll/OnState", 
		       &auto_scroll );
      AutoScroll = auto_scroll;
   }
   else    /* Set to the supplied value. */
   {
      AutoScroll = new_value;
      GlgSetDResource( Drawing, "Toolbar/ToggleAutoScroll/OnState", 
		       AutoScroll );
   }

   /* Set chart's auto-scroll. */
   GlgSetDResource( Chart, "AutoScroll", (double) AutoScroll );

   /* Activate time scrollbar if AutoScroll is Off. The Y value scrollbar 
      uses GLG_PAN_Y_AUTO and appears automatically as needed.
   */
   pan_x = ( AutoScroll ? GLG_NO_PAN : GLG_PAN_X );
   GlgSetDResource( ChartVP, "Pan", (double) ( pan_x | GLG_PAN_Y_AUTO ) );
}

/*----------------------------------------------------------------------
| Changes the time span shown in the graph.
*/
void SetChartSpan( GlgLong span )
{
   if( span > 0 )
     GlgSetDResource( Drawing, "ChartViewport/Chart/XAxis/Span", 
		      (double) span );
   else  /* Reset span to show all data accumulated in the buffer. */
     GlgSetZoom( Drawing, "ChartViewport", 'n', 0. );
}

/*----------------------------------------------------------------------
|
*/
void RestoreInitialYRanges()
{
   GlgSetDResource( Chart, "YAxis/Low",  Low );
   GlgSetDResource( Chart, "YAxis/High", High );
}

/*----------------------------------------------------------------------
| Returns True if the chart's viewport is in ZoomToMode.
| ZoomToMode is activated on Dragging and ZoomTo operations.
*/
GlgLong ZoomToMode()
{
   double zoom_mode;

   GlgGetDResource( ChartVP, "ZoomToMode", &zoom_mode );
   if( zoom_mode )
     return (GlgLong) zoom_mode;

   return False;
}

/*----------------------------------------------------------------------
|
*/
void AbortZoomTo()
{
   if( ZoomToMode() )
   {
      /* Abort zoom mode in progress. */
      GlgSetZoom( ChartVP, NULL, 'e', 0. ); 
      GlgUpdate( ChartVP );
   }
}

/*----------------------------------------------------------------------
| Used to obtain coordinates of the mouse click.
*/
void Trace( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{      
   GlgTraceCBStruct * trace_data;
   int
     event_type = 0,
     x, y,
     button_index,
     width, height;

   trace_data = (GlgTraceCBStruct*) call_data;

   if( trace_data->viewport != ChartVP )
     return;

   /* Platform-specific code to extract event information. */
#ifndef _WINDOWS
   switch( trace_data->event->type )
   {
    case ButtonPress:
      x = trace_data->event->xbutton.x;
      y = trace_data->event->xbutton.y;
      button_index = trace_data->event->xbutton.button;
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
    case WM_LBUTTONDBLCLK:
      x = GET_X_LPARAM( trace_data->event->lParam );
      y = GET_Y_LPARAM( trace_data->event->lParam );
      button_index = 1;
      event_type = BUTTON_PRESS;
      break;
      
    case WM_MOUSEMOVE:
      x = GET_X_LPARAM( trace_data->event->lParam );
      y = GET_Y_LPARAM( trace_data->event->lParam );
      event_type = MOUSE_MOVE;
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

      /* Disable AutoScroll not to interfere with dragging  or
	 marking data samples.
      */
      ChangeAutoScroll( 0 );        

      /* Handle left mouse clicks. */
      if( button_index == 1 )
      {
	 /* Get click count to differentiate between double-click and 
	    single click. 
	 */
	 if( GetClickCount( x, y ) == 2 )
	 {
	    /* Double-click: show/hide a marker for a selected data sample, 
	       if any.
	    */
	    MarkDataSample( x, y );
	 }
	 else /* Single click. */
	 {
	    /* Start dragging with the mouse on a mouse click. 
	       If the user clicked on an axis, the dragging will 
	       be activated in the direction of that axis. 
	       If the user clicked in the chart area, dragging 
	       in both the time and the Y direction will be activated.
	    */
	    GlgSetZoom( ChartVP, NULL, 's', 0. );
	 }
      }
      break;

    case MOUSE_MOVE:
      break;

    default: return;
   } 
}


/*----------------------------------------------------------------------
| Show/Hide a marker for the selected data sample at the cursor position.
*/
void MarkDataSample( double x, double y )
{
   /* Obtain the closest data sample at the cursor position. */
   GlgDataSample * data_sample = GetDataSample( x, y );
   
   if( !data_sample )
     return;
   
   data_sample->marker_vis = 
     ( data_sample->marker_vis == 0. ? /*show*/ 1.f : /*hide*/ 0.f );
   
   GlgChangeObject( Chart, NULL );
   GlgUpdate( ChartVP );
}

/*----------------------------------------------------------------------
| Obtain a chart data sample at the cursor position. 
*/
GlgDataSample * GetDataSample( double x, double y )
{
   GlgObject 
     plot,
     data_array;
   GlgDataSample * data_sample;
   int 
     num_samples, 
     i;
   char 
     * name,
     * sample_x_string;
   double 
     sample_x_value, 
     sample_y_value;

   /* Query chart selection at the cursor position. */
   GlgObject selection = 
     GlgCreateChartSelection( Chart, NULL /* query all plots */, 
			      x, y, dx, dy,
			      True   /* x/y in screen coord */, 
			      False  /* don't include invalid points */, 
			      False  /* a point with smallest xy distance */ );
   
   if( !selection )
   {
      error( "No data sample at cursor.", False );
      return NULL;  /* No valid data sample is selected. */
   }
   
   /* Query X/Y values of the selected data sample. */
   plot = GlgGetResourceObject( selection, "SelectedPlot" );
   GlgGetSResource( plot, "Name", &name );
   GlgGetDResource( selection, "SampleX", &sample_x_value );
   GlgGetDResource( selection, "SampleY", &sample_y_value );

   /* Debugging info. */
   GlgGetSResource( selection, "SampleXString", &sample_x_string );
   printf( "Selection: %s time=%s value=%lf\n",
	   name, sample_x_string, sample_y_value );

   /* Dereference selection object, to prevent memory leak. */
   GlgDropObject( selection );

   /* Obtain a list of data samples from the selected plot. */
   data_array = GlgGetResourceObject( plot, "Array" );
   
   if( data_array && ( num_samples = GlgGetSize( data_array ) ) !=0 )
   {
      /* Traverse the array to find a data sample with a matching x/y value.
	 data_array is a linked list and should be traversed
	 using SetStart() and Iterate() for efficiency, as opposed to
	 using indexes via GetElement(i);
      */
      GlgSetStart( data_array );
      for( i=0; i<num_samples; ++i )
      {
	 data_sample = (GlgDataSample*) GlgIterate( data_array );
	 
	 /* Debugging info. */
	 /*
	   printf( "DataSample info: time = %lf value = %lf\n",
	            data_sample->time, data_sample->value );
	 */
	 
	 if( data_sample->valid &&
	     data_sample->time == sample_x_value &&
	     data_sample->value == sample_y_value  )
	   /* found matching data sample */
	   return data_sample;
      }
   } 
   
   return NULL;     /* No matching samples found. */
}

/*----------------------------------------------------------------------
| For a double-click, return 2; otherwise, return 1 (single click).
*/
int GetClickCount( double x, double y )
{
   if( isSingleClick )
   { 
      isSingleClick = False; /* Reset flag. */
      return 2; /* Double click occurred. */
   }
   else
   {
      isSingleClick = True;

      /* Start a timer to identify a double click. */
      GlgAddTimeOut( AppContext, 500, (GlgTimerProc)dblclick_timer, NULL );
   }
   
   return 1;   /* Single click. */
}

/*----------------------------------------------------------------------
| Timer function used to identify a double click.
*/
void dblclick_timer( GlgAnyType data, GlgIntervalID * id )
{
   /* Reset flag. */
   isSingleClick = False; 
}

/*----------------------------------------------------------------------
| Fill the graph's history buffer with demo data. 
*/
void PreFillChartData()
{
   GlgLong i;
   GlgULong num_seconds;   
   double current_time, start_time;
   double dt, time_stamp;

   current_time = GetCurrTime();
   
   /* Roll back by the amount corresponding to the buffer size. */
   dt = UpdateInterval / 1000.;     /* Update interval is in millisec. */
   
   /* Add an extra second to avoid rounding errors. */
   num_seconds = BufferSize * dt + 1;

   start_time = current_time - num_seconds;
   time_stamp = start_time;

   for( i=0; i<BufferSize || time_stamp < GetCurrTime(); ++i )
   {      
      time_stamp = start_time + i * dt;
      GetChartData( time_stamp, False );
   }
}

/*----------------------------------------------------------------------
| Supplies plot values for the demo. In a real application, these data 
| will be coming from an application-specific data source. 
| time_stamp parameter is not used for demo data, but in a real
| application the plot's data value may be retrieved based
| on a given 

*/
double GetPlotValue( GlgLong plot_index )
{
   static GlgLong counter = 0;
   double 
     half_amplitude, center,
     period,
     value,
     alpha;

   half_amplitude = ( High - Low ) / 2.;
   center = Low + half_amplitude;

   period = 100. * ( 1. + plot_index * 2. );
   alpha = 2. * M_PI * counter / period;

   value = center + half_amplitude * sin( alpha ) *  sin( alpha / 30. );

   ++counter;
   return value;
}

/*----------------------------------------------------------------------
| For demo purposes, returns current time in seconds. 
| Place application specific code here to return a time stamp as needed.
*/
double GetTimeStamp( )
{
   return GetCurrTime(); /* for demo purposes */
}


/*----------------------------------------------------------------------
| Return exact time including fractions of seconds.
*/
double GetCurrTime()
{
   GlgULong sec, microsec;

   GlgGetTime( &sec, &microsec );
   return sec + microsec / 1000000.;
}

/*----------------------------------------------------------------------
|
*/
void error( char * string, GlgBoolean quit )
{
   GlgError( GLG_USER_ERROR, string );
   if( quit )
     exit( GLG_EXIT_ERROR );
}

/*----------------------------------------------------------------------
| 
*/
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

