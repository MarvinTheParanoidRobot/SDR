/***************************************************************************
  This example demonstrates how to display and update a GLG realtime
  stripchart widget.
  
  The program loads a GLG widget drawing chart2.g and updates the chart 
  with simulated data using a timer procedure UpdateChart().

  GetPlotValue() method, used in this example to generate demo data,
  may be replaced with a custom data feed.

  The X axis labels display current date and time using the time format
  defined in the drawing. By default, the labels are generated automatically 
  by the graph, however the program may supply a time stamp for 
  each data iteration, by setting use_current_time=False in UpdateChart()
  and replacing code in GetTimeStamp() method.

  This example is written using GLG Standard API. 
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "GlgApi.h"
#ifdef _WINDOWS
# pragma warning( disable : 4244 )
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

#define TIME_SPAN 60          /* Time Span in sec. */
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
void GetChartData( double time_stamp, GlgBoolean use_current_time );
double GetPlotValue( GlgLong plot_index );
double GetCurrTime( void );
double GetTimeStamp( void );
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, 
			    GlgLong interval );
void error( char * string, GlgLong quit );


GlgObject Drawing;             /* Top level viewport. */
GlgObject Plots[ NUM_PLOTS ];  /* Array of the plot object IDs. */

GlgLong
  UpdateInterval = 100, /* Update interval in msec */
  TimeSpan = TIME_SPAN; /* Currently displayed Time axis span in seconds */

double
  /* Low and High range of the incoming data values. */
  Low = 0.,
  High = 10.;

GlgLong num_plots_in_drawing;  /* Number of plots as defined in .g file. */

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

   full_path = GlgGetRelativePath( argv[0], "chart2.g" );
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
   double 
     num_plots_d,
     major_interval, 
     minor_interval;

   /* Retrieve number of plots defined in .g file. */
   GlgGetDResource( Drawing, "Chart/NumPlots", &num_plots_d );
   num_plots_in_drawing = num_plots_d;

   /* Set new number of plots as needed. */
   GlgSetDResource( Drawing, "Chart/NumPlots", NUM_PLOTS );

   /* Set Time Span for the X axis. */
   GlgSetDResource(  Drawing, "Chart/XAxis/Span", TimeSpan );

   /* Set tick intervals for the Time axis. Positive value sets the
      exact interval in sec, while negative value sets the number of ticks
      regardless of the time span. 
   */
   major_interval = -6; /* 6 major intervals */
   minor_interval = -5; /* 6 minor ticks (or 5 intervals) */

   GlgSetDResource( Drawing, "Chart/XAxis/MajorInterval", major_interval );
   GlgSetDResource( Drawing, "Chart/XAxis/MinorInterval", minor_interval );

   /* Set data value range. Since the graph has one Y axis and
      common data range for the plots, Low/High data range is
      set on the YAxis level.
     */
   GlgSetDResource( Drawing, "Chart/YAxis/Low", Low );
   GlgSetDResource( Drawing, "Chart/YAxis/High", High );
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
      Plots[i] = GlgGetNamedPlot( Drawing, "Chart", res_name ); 
      GlgFree( res_name );
   }
      
   /* For the existing plots, use color and line annotation setting 
      from the drawing; initialize new plots using random colors and strings
      for demo purposes. 
   */
   if( num_plots_in_drawing < NUM_PLOTS )
     for( i=num_plots_in_drawing; i < NUM_PLOTS; ++i )
     {
	/* Using a random color for a demo. */
	GlgSetGResource( Plots[i], "EdgeColor", 
			 GlgRand(0., 1.), GlgRand(0., 1.), GlgRand(0., 1.) );

	res_name = GlgCreateIndexedName( "Var%", i );
	GlgSetSResource( Plots[i], "Annotation", res_name );
	GlgFree( res_name );
     }
}

#define DEBUG_TIMER False

/*----------------------------------------------------------------------
| Timer procedure to update the chart with new data.
*/
void UpdateChart( GlgAnyType data, GlgIntervalID * id )
{
   GlgULong sec, microsec;
   GlgLong timer_interval;
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
     * action;

   message_obj = (GlgObject) call_data;

   GlgGetSResource( message_obj, "Origin", &origin );
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );

   /* Handle window closing. */
   if( strcmp( format, "Window" ) == 0 )
   {
      if( strcmp( action, "DeleteWindow" ) == 0 )
	 /* Closing main window: exit. */
	 exit( GLG_EXIT_OK );
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

