/***************************************************************************
  This example demonstrates how to display and update a GLG realtime
  stripchart widget and use its integrated interactive behavior.
  
  The program loads a GLG drawing stripchart_example.g, which 
  includes a ChartViewport as well as interface widgets allowing 
  to scroll and zoom the graph. 
  
  The graph is initilaized in InitChart() function and updated with 
  simulated data using a timer procedure UpdateChart().

  GetPlotValue() method, used in this example to generate demo data,
  may be replaced with a custom data feed.

  The X axis labels display current date and time using the time format
  defined in the drawing. By default, the labels are generated automatically 
  by the graph, however the program may supply a time stamp for 
  each data iteration, by setting use_current_time=False in UpdateChart()
  and replacing code in GetTimeStamp() method.

  This example is written using GLG Standard API. For coding conveninece 
  as well as better performance, use GLG Intermediate API as shown in 
  the example GlgRealTimeChart_IntAPI.java.
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "GlgApi.h"

#define TIME_SPAN 60          /* Time Span in sec. */
#define SCROLL_INCREMENT 10   /* Scroll increment in sec. */
#define NUM_PLOTS 3           /* Number of lines in a chart. */

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
GlgLong ZoomToMode( void );
void AbortZoomTo( void );
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, 
			    GlgLong interval );
void PreFillChartData( void );
void GetChartData( double time_stamp, GlgBoolean use_current_time );
double GetPlotValue( GlgLong plot_index );
double GetCurrTime( void );
double GetTimeStamp( void );
void error( char * string, GlgLong quit );


GlgObject Drawing;             /* Top level viewport. */
GlgObject Plots[ NUM_PLOTS ];  /* Array of the plot object IDs. */

GlgLong
  BufferSize = 5000,     /* Number of samples in the history buffer per line. */
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
  /* Scroll increment for the X axis. */   
  ScrollFactor = SCROLL_INCREMENT / TIME_SPAN;

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

   full_path = GlgGetRelativePath( argv[0], "stripchart_example.g" );
   Drawing = GlgLoadWidgetFromFile( full_path );
   if( !Drawing )
     error( "Can't load drawing file.", True );

   GlgFree( full_path );

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
   GlgGetDResource( Drawing, "ChartViewport/Chart/NumPlots", 
		    &num_plots_drawing );

   /* Set new number of plots as needed. */
   GlgSetDResource( Drawing, "ChartViewport/Chart/NumPlots", NUM_PLOTS );

   /* Set Time Span for the X axis. */
   GlgSetDResource(  Drawing, "ChartViewport/Chart/XAxis/Span", TimeSpan );

   /* Set data value range. Since the graph has one Y axis and
      common data range for the plots, Low/High data range is
      set on the YAxis level.
     */
   GlgSetDResource( Drawing, "ChartViewport/Chart/YAxis/Low", Low );
   GlgSetDResource( Drawing, "ChartViewport/Chart/YAxis/High", High );

   /* Enable AutoScroll, both for the toggle button and the chart. */
   ChangeAutoScroll( 1 );

   /* Set Chart Zoom mode. It was set and saved with the drawing, but do it 
      again programmatically just in case.
      */
   GlgSetZoomMode( Drawing, "ChartViewport", NULL, 
		   "ChartViewport/Chart", GLG_CHART_ZOOM_MODE );
}

/*----------------------------------------------------------------------
| Initializes chart parameters after hierarchy setup has occurred.
*/
void InitChartAfterH()
{
   GlgLong i;
   char *res_name;

   /* Store objects IDs for each plot. */
   for( i=0; i<NUM_PLOTS; ++i )
   {
      res_name = GlgCreateIndexedName( "Plot#%", i );
      Plots[i] = GlgGetNamedPlot( Drawing, "ChartViewport/Chart", res_name ); 
      GlgFree( res_name );
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
   GlgULong sec, microsec;
   GlgLong i, timer_interval;
   GlgBoolean use_current_time = True; /* automatic time stamp */

   /* Start time for adjusting timer intervals. */
   GlgGetTime( &sec, &microsec );

   /* Supply demo data to update plot lines. 
      In this example, current time is automatically supplied
      by the chart. The application may supply a time stamp instead,
      by replacing code in GetTimeStamp() method.
   */
   GetChartData( use_current_time ? 0. : GetTimeStamp(), 
		 use_current_time);

   GlgUpdate( Drawing );
   GlgSync( Drawing );    /* Improves interactive response */

   /* Adjust timer intervals to have a constant update rate regardless
      of the time it takes to redraw the chart.
   */      
   timer_interval = GetAdjustedTimeout( sec, microsec, UpdateInterval );

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
	 /* Start ZoomTo operation */
         GlgSetZoom( Drawing, "ChartViewport", 't', 0. );  
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
         GlgGetDResource( Drawing, "ChartViewport/Chart/XAxis/EndValue", 
	                  &end_value );
	 end_value -= SCROLL_INCREMENT;
	 GlgSetDResource( Drawing, "ChartViewport/Chart/XAxis/EndValue", 
	                  end_value );
	 */

         GlgSetZoom( Drawing, "ChartViewport", 'l', ScrollFactor );
      }
      else if( strcmp( origin, "ScrollForward" ) == 0 )
      {
         ChangeAutoScroll( 0 );

         /* Scroll right */
         GlgSetZoom( Drawing, "ChartViewport", 'r', ScrollFactor );
      }
      else if( strcmp( origin, "ScrollToRecent" ) == 0 )
      {
	 /* Scroll to show most recent data. */
	 ScrollToDataEnd();
      }
 
      GlgUpdate( Drawing );
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
void ScrollToDataEnd()
{
   GlgMinMax min_max;

   /* Get min/max time stamp. */
   if( !GlgGetDataExtent( Drawing, "ChartViewport/Chart", 
			  &min_max, True /*X extent*/ ) )
     return;

   GlgSetDResource( Drawing, "ChartViewport/Chart/XAxis/EndValue", 
		    min_max.max );
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
   GlgSetDResource(  Drawing, "ChartViewport/Chart/AutoScroll", 
		     (double) AutoScroll );

   /* Activate time scrollbar if AutoScroll is Off. The Y value scrollbar 
      uses GLG_PAN_Y_AUTO and appears automatically as needed.
   */
   pan_x = ( AutoScroll ? GLG_NO_PAN : GLG_PAN_X );
   GlgSetDResource( Drawing, "ChartViewport/Pan", 
		    (double) ( pan_x | GLG_PAN_Y_AUTO ) );
}

/*----------------------------------------------------------------------
| Changes the time span shown in the graph.
*/
void SetChartSpan( GlgLong span )
{
   if( span > 0 )
     GlgSetDResource( Drawing, "ChartViewport/Chart/XAxis/Span", (double) span );
   else  /* Reset span to show all data accumulated in the buffer. */
     GlgSetZoom( Drawing, "ChartViewport", 'n', 0. );
}

/*----------------------------------------------------------------------
|
*/
void RestoreInitialYRanges()
{
   GlgSetDResource( Drawing, "ChartViewport/Chart/YAxis/Low",  Low );
   GlgSetDResource( Drawing, "ChartViewport/Chart/YAxis/High", High );
}

/*----------------------------------------------------------------------
| Returns True if the chart's viewport is in ZoomToMode.
| ZoomToMode is activated on Dragging and ZoomTo operations.
*/
GlgLong ZoomToMode()
{
   double zoom_mode;

   GlgGetDResource( Drawing, "ChartViewport/ZoomToMode", &zoom_mode );
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
      GlgSetZoom( Drawing, "ChartViewport", 'e', 0. ); 
      GlgUpdate( Drawing );
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
     width, height;
   char * event_vp_name;

   trace_data = (GlgTraceCBStruct*) call_data;

   /* Process only events that occur in ChartViewport */ 
   GlgGetSResource( trace_data->viewport, "Name", &event_vp_name );
   if( !event_vp_name || strcmp( event_vp_name, "ChartViewport" ) !=0 )
     return;

   /* Platform-specific code to extract event information. */
#ifndef _WINDOWS
   switch( trace_data->event->type )
   {
    case ButtonPress:
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
      x = GET_X_LPARAM( trace_data->event->lParam );
      y = GET_Y_LPARAM( trace_data->event->lParam );
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
      
      /* Start dragging with the mouse on a mouse click. 
         If user clicked on an axis, the dragging will be activated in the
         direction of that axis. If the user clicked in the chart area,
         dragging in both the time and the Y direction will be activated.
      */
      GlgSetZoom( Drawing, "ChartViewport", 's', 0. );

      /* Disable AutoScroll not to interfere with dragging */
      ChangeAutoScroll( 0 );        
      break;

    case MOUSE_MOVE:
      break;

    default: return;
   } 
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
| on a given plot index.

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

