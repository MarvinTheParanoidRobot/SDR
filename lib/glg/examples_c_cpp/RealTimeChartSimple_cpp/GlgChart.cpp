/***************************************************************************
  GlgChart class is derived from GlgObjectC and encapsulates methods
  for initializing the chart drawing and updating the stripchart 
  with data. 

  For demo pourposes, the chart displays simulated data, using
  GetDemoValue() method. The application may replace GetDemoValue()
  and supply real-time application specific data.

  This example is written using GLG Standard API. 
***************************************************************************/

#include "GlgChart.h"

#define TIME_SPAN 60            /* Time Span in sec. */
#define DEF_NUM_PLOTS 3         /* Number of plots in a chart */

/* Global functions. */
void UpdateChart( GlgChart * chart, GlgLong * timer_id );
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, 
			    GlgLong interval );

/* Used to return data values. */
DataPoint data_point;

/*----------------------------------------------------------------------
| Constructor. 
*/
GlgChart::GlgChart()
{
   /* Set default number of plots in a chart. */
   NumPlots = DEF_NUM_PLOTS;

   UpdateInterval = 100;  /* Update interval in msec */
   TimeSpan = TIME_SPAN;  /* Currently displayed Time axis span in seconds */
   
   /* Default Low and High range of the incoming data values. */
   Low = 0.;
   High = 10.;

   TimerID = 0;
}

/*----------------------------------------------------------------------
| Destructor.
*/
GlgChart::~GlgChart( void )
{
   if( Plots )
     delete Plots;
}
   
/*----------------------------------------------------------------------
| Initialize chart parameters.
*/
void GlgChart::Init( void )
{
   EnableCallback( GLG_INPUT_CB, NULL );

   /* Initialize Chart parameters before hierarchy is set up. */
   InitBeforeH();
   
   /* Set up object hierarchy. */ 
   SetupHierarchy();

   /* Initialize Chart parameters after hierarchy setup took place. */
   InitAfterH();
}

/*----------------------------------------------------------------------
| Initializes chart parameters before hierarchy setup occurred.
*/
void GlgChart::InitBeforeH( void )
{
   double 
     num_plots_d,
     major_interval, 
     minor_interval;

   /* Retrieve number of plots defined in .g file. */
   GetResource( "Chart/NumPlots", &num_plots_d );
   num_plots_drawing = num_plots_d;
   
   /* Set new number of plots as needed. */
   SetResource( "Chart/NumPlots", NumPlots );
   
   /* Set Time Span for the X axis. */
   SetResource( "Chart/XAxis/Span", TimeSpan );

   /* Set tick intervals for the Time axis. Positive value sets the
      exact interval in sec, while negative value sets the number of ticks
      regardless of the time span. 
   */
   major_interval = -6; /* 6 major intervals */
   minor_interval = -5; /* 6 minor ticks (or 5 intervals) */
   
   SetResource( "Chart/XAxis/MajorInterval", major_interval );
   SetResource( "Chart/XAxis/MinorInterval", minor_interval );

   /* Set data value range. Since the graph has one Y axis and
      common data range for the plots, Low/High data range is
      set on the YAxis level.
     */
   SetResource( "Chart/YAxis/Low", Low );
   SetResource( "Chart/YAxis/High", High );
}

/*----------------------------------------------------------------------
| Initializes chart parameters after hierarchy setup occurred.
*/
void GlgChart::InitAfterH( void )
{
   GlgLong i;
   CONST char * res_name;

   /* Allocate an array to store plot object IDs. */
   Plots = new GlgObjectC [ NumPlots ];

   /* Store objects IDs for each plot. */
   for( i=0; i<NumPlots; ++i )
   {
      res_name = GlgCreateIndexedName( (char*) "Plot#%", i );
      Plots[i] = GetNamedPlot( "Chart", res_name ); 
      GlgFree( (char*) res_name );
   }
      
   /* For the existing plots, use color and line annotation setting 
      from the drawing; initialize new plots using random colors and strings
      for demo purposes. 
   */
   if( num_plots_drawing < NumPlots )
     for( i=num_plots_drawing; i < NumPlots; ++i )
     {
	/* Using a random color for a demo. */
	Plots[i].SetResource( "EdgeColor", GlgRand(0., 1.), 
			      GlgRand(0., 1.), GlgRand(0., 1.) );

	res_name = GlgCreateIndexedName( (char*) "Var%", i );
	Plots[i].SetResource( "Annotation", res_name );
	GlgFree( (char*) res_name );
     }
}

/*----------------------------------------------------------------------
| Start dynamic updates.
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
   GlgLong i, timer_interval;

   /* Start time for adjusting timer intervals. */
   GlgGetTime( &sec, &microsec );

   /* This example uses demo data. An application will provide a
      custom DataFeed object for supplying real-time chart data.
   */
   
   GlgLong num_plots = chart->NumPlots;

   /* Update plot lines with new data. */
   for( i=0; i<num_plots; ++i )
   {
      chart->GetPlotPoint( i, data_point );
      chart->PushPlotPoint( i, data_point );
   }

   chart->Update();

   /* Adjust timer intervals to have a constant update rate regardless
      of the time it takes to redraw the chart.
   */      
   timer_interval = 
     GetAdjustedTimeout( sec, microsec, chart->UpdateInterval );

   chart->TimerID = 
     GlgAddTimeOut( chart->AppContext, timer_interval, 
		    (GlgTimerProc) UpdateChart, chart );
}

/*----------------------------------------------------------------------
| Generate a new plot point. The example uses simulated data
| generated by GetDemoValue(). The time stamp is automatically
| generated by the chart, but the application may supply a time
| stamp as needed.
*/
void GlgChart::GetPlotPoint( int plot_index, DataPoint& data_point )
{
   data_point.value = GetDemoValue( plot_index );
   data_point.value_valid = true;
   
   data_point.has_time_stamp = false; // Use current time stamp for demo
   data_point.time_stamp = 0.;
}

/*----------------------------------------------------------------------
| Pushes the data_point's data into the plot.
*/
void GlgChart::PushPlotPoint( int plot_index, DataPoint& data_point )
{
   /* Supply plot value for the chart via ValueEntryPoint. */
   Plots[ plot_index ].SetResource( "ValueEntryPoint", data_point.value );
   
   if( data_point.has_time_stamp )
   {
      /* Supply an optional time stamp. If not supplied, the chart will 
	 automatically generate a time stamp using current time. 
      */
      Plots[ plot_index ].SetResource( "TimeEntryPoint", 
				      data_point.time_stamp );
   }
   
   if( !data_point.value_valid )
   {	   
      /* If the data point is not valid, set ValidEntryPoint resource to 
	 display holes for invalid data points. If the point is valid,
	 it is automatically set to 1. by the chart.
      */
      Plots[ plot_index ].SetResource( "ValidEntryPoint", 0. );
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
}

/*----------------------------------------------------------------------
| Generates demo data value.
*/
double GlgChart::GetDemoValue( int plot_index )
{
   static double counter = 0;
   double 
     half_amplitude, center,
     period,
     value,
     alpha;
   
   half_amplitude = ( High - Low ) / 2.;
   center = Low + half_amplitude;
   
   period = 100. * ( 1. + plot_index * 2. );
   alpha = 2. * M_PI * counter / period;
   
   value = center + 
     half_amplitude * sin( alpha ) * sin( alpha / 30. );
   
   ++counter;
   return value;
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

