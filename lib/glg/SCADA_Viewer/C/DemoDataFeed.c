#include "DemoDataFeed.h"
#include "GlgSCADAViewer.h"

/* DemoDataFeed provides simulated data for demo, as well as for testing 
   with no LiveDataFeed.
   In an application, data will be coming from LiveDataFeed.
*/

/*----------------------------------------------------------------------*/
DataFeed * CreateDemoDataFeed()
{
   DemoDataFeed * ddf = GlgAllocStruct( sizeof( DemoDataFeed ) );
   ddf->DataFeed.type = DATA_FEED_TYPE;
   
   ddf->DataFeed.ReadDTagData  = dfReadDTagData;
   ddf->DataFeed.ReadSTagData  = dfReadSTagData;
   ddf->DataFeed.WriteDTagData = dfWriteDTagData;
   ddf->DataFeed.WriteSTagData = dfWriteSTagData;
   ddf->DataFeed.GetPlotData   = dfGetPlotData;
   ddf->DataFeed.FreePlotData  = dfFreePlotData;
   ddf->DataFeed.GetAlarms     = dfGetAlarms;
   ddf->DataFeed.FreeAlarms    = dfFreeAlarms;
   ddf->DataFeed.ACKAlarm      = dfACKAlarm;
   ddf->DataFeed.Destroy       = dfDestroy;

   ddf->ActiveAlarmList = NULL;

   return (DataFeed*) ddf;
}

/*----------------------------------------------------------------------*/
static void dfDestroy( DataFeed * datafeed )
{
   GlgFree( datafeed );
}

/*----------------------------------------------------------------------
| Returns a simulated D tag value for animation.
*/

static GlgBoolean dfReadDTagData( DataFeed * datafeed, TagRecord * tag_record,
                                  double * value, double * time_stamp )
{
   *time_stamp = GetCurrTime();   
   return GetDemoValue( tag_record->tag_source, value, GlgFalse );
}

/*----------------------------------------------------------------------
| Returns a simulated S tag value for animation.
*/
static GlgBoolean dfReadSTagData( DataFeed * datafeed, TagRecord * tag_record,
                                  char ** value )
{   
   return GetDemoString( tag_record->tag_source, value );
}

/*----------------------------------------------------------------------
| Sets new tag value. For demo purposes, set the tag value in the drawing. 
| In an application, the value should be written to the tag source in 
| the back-end system.
*/
static GlgBoolean dfWriteDTagData( DataFeed * datafeed, 
                                   char * tag_source, double value )
{
   GlgSetDTag( Viewer.MainViewport, tag_source, value, GlgTrue );
   GlgUpdate( Viewer.MainViewport );

   return GlgTrue;
}

/*----------------------------------------------------------------------
|
*/
static GlgBoolean dfWriteSTagData( DataFeed * datafeed, 
                                   char * tag_source, char * string )
{
   return GlgTrue;
}

/*----------------------------------------------------------------------
| Obtain plot data for the specified tag source and time interval.
| Data are held in a GLG array containing PlotDataPoint elements.
*/
static GlgObject dfGetPlotData( DataFeed * datafeed, char * tag_source, 
                                double start_time, double end_time,
                                int max_num_samples )
{
   GlgObject data_array;
   int i, num_samples;
   double interval;

   /* In a real application, the number of data points to be queried
      is determined by the start and end time. For the demo, return
      the requested max number of points.
   */
   if( max_num_samples < 1 )
     max_num_samples = 1;
   num_samples = max_num_samples;
   
   interval = ( end_time - start_time ) / max_num_samples;

   data_array = GlgArrayCreate( num_samples, 0 );
      
   for( i=0; i<num_samples; ++i )
   {
      /* Generate demo data. */
      PlotDataPoint * data_point = GlgAlloc( sizeof( PlotDataPoint ) );
      if( !GetDemoValue( tag_source, &data_point->value, GlgTrue ) )
        data_point->value = 0.;
      data_point->time_stamp = start_time + interval * i;
      data_point->value_valid = GlgTrue;
      GlgArrayAddToBottom( data_array, data_point );
   }
   return data_array;
}

/*----------------------------------------------------------------------*/
static void dfFreePlotData( DataFeed * datafeed, GlgObject data_array )
{
   int i, size;
   PlotDataPoint * data_point;

   if( !data_array )
     return;

   size = GlgArrayGetSize( data_array );
   for( i=0; i<size; ++i )
   {
      data_point = (PlotDataPoint*) GlgArrayGetElement( data_array, i );
      GlgFree( data_point );
   }
   GlgDropObject( data_array );
}

/*----------------------------------------------------------------------*/
static GlgBoolean GetDemoValue( char * tag_source, double * out_value, 
                                GlgBoolean historical_mode )
{
   static GlgLong counter = 0;
   static GlgLong tag_index = 0;
   double 
     value,
     period,
     alpha,
     center,
     amplitude;
   
   switch( Viewer.PageType )
   {
    case AERATION_PAGE:
      *out_value = GlgRand( 0., 5. );
      return GlgTrue;

    case CIRCUIT_PAGE:
      if( strstr( tag_source, "State" ) )
        /* tag_source contains string "State" */
        *out_value = GlgRand( 0., 1.3 );
      else
        *out_value = GlgRand( 0., 1000. );
      return GlgTrue;

    case RT_CHART_PAGE:
      if( strstr( tag_source, "Volts" ) )
      {
         /* tag_source contains string "Volts" */
         center = 380.;
         amplitude = 40.;
         period = 300.;
      }
      else   /* "Amps" */
      {
         center = 30.;
         amplitude = 15.;
         period = 1000.;
      }
      
      alpha = 2. * M_PI * counter / period;        
      value = center + amplitude * sin( alpha ) * sin( alpha / 30. );
         
      if( historical_mode )
        counter += 10;   /* Historical data, data were saved once per second. */
      else
        ++counter;       /* Real-time mode: updates 10 times per second. */
      *out_value = value;
      return GlgTrue;
      
    case TEST_COMMANDS_PAGE:
      /* Generate demo data in the range [10,90] for the real-time chart
         in the popup dialog.
      */
      period = 100. * ( 1. + tag_index * 2. );
      alpha = 2. * M_PI * counter / period;
      value = 50. + 40. * sin( alpha ) * sin( alpha / 30. );

      ++tag_index;
      if( tag_index >= Viewer.NumTagRecords )
	tag_index = 0;

      ++counter;
      *out_value = value;
      return GlgTrue;

    default: 
      *out_value = GlgRand( 0., 100. );
      return GlgTrue;
   }
}

/*----------------------------------------------------------------------*/
static GlgBoolean GetDemoString( char * tag_source, char ** value )
{
   *value = ( GlgRand( 0., 1. ) > 0.5 ? "On" : "Off" );
   return GlgTrue;
}

enum AlarmSetType
{
   RESET_ALARM = 0,
   RAISE_ALARM,
};

#define FIRST_ALARM_RAISE_THRESHOLD   0.6
#define ALARM_RAISE_THRESHOLD         0.8
      
#define OLD_ACK_ALARM_AGE       15
#define OLD_NON_ACK_ALARM_AGE   30
#define NUM_SIMULATED_ALARMS    30

/*----------------------------------------------------------------------
| Obtain a list of alarms using simulated data and return it as a 
| GLG array containing alarm records.
*/
static GlgObject dfGetAlarms( DataFeed * datafeed_p )
{
   DemoDataFeed * datafeed = (DemoDataFeed*) datafeed_p;
   GlgObject alarm_list;
   AlarmRecord * alarm;
   double alarm_raise_threshold;
   int i, num_active_alarms;

   /* Simulate alarms. */

   /* Create alarm list first time. */
   if( !datafeed->ActiveAlarmList )
     datafeed->ActiveAlarmList = GlgArrayCreate( 0, 0 );

   /* Ages alarms and removes old alarms from the list for continuous
      simulation.
   */
   AgeAlarms( datafeed, datafeed->ActiveAlarmList );
   num_active_alarms = GlgArrayGetSize( datafeed->ActiveAlarmList );

   alarm_raise_threshold = 
     ( num_active_alarms == 0 ? FIRST_ALARM_RAISE_THRESHOLD : 
       ALARM_RAISE_THRESHOLD );

   /* Add new simulated alarm (conditionally). */
   if( num_active_alarms < NUM_SIMULATED_ALARMS &&
       GlgRand( 0., 1. ) > alarm_raise_threshold )
   {
      alarm = dfGetAlarmData( datafeed ); 
      GlgArrayAddToBottom( datafeed->ActiveAlarmList, alarm );
      num_active_alarms = GlgArrayGetSize( datafeed->ActiveAlarmList );
   }
      
   if( !num_active_alarms )
     return NULL;   /* No alarms. */

   /* Create a new list of alarms to be returned. */
   alarm_list = GlgArrayCreate( 0, 0 );
     
   /* For simulating alarms in the demo, populate the list with alarms from 
      ActiveAlarmList. Since active alarms are kept in the ActiveAlarmList
      are are persistent, store pointers to alarm structures and do not free
      alarms when freeing the returned alarm list in dfFreeAlarms(). Instead, 
      alarms will be freed when they are deleted from the active alarm list.

      The LiveDataFeed used in a real application may create or copy alarms
      added to the alarm list returned by lfGetAlarms(), and then free alarm
      structures when the list is destroyed by lfFreeAlarms().
   */
   for( i=0; i<num_active_alarms; ++i )
   {
      alarm = (AlarmRecord*) GlgArrayGetElement( datafeed->ActiveAlarmList, i );
      GlgArrayAddToBottom( alarm_list, alarm );
   }
   
   return alarm_list;
}

/*----------------------------------------------------------------------
| Free the alarm list data.
*/
static void dfFreeAlarms( DataFeed * datafeed_p, GlgObject alarm_list )
{
   DemoDataFeed * datafeed = (DemoDataFeed*) datafeed_p;
#if 0
   int i, size;
   AlarmRecord * alarm;
#endif

   if( !alarm_list )
     return;

   /* Don't free alarm records in the demo mode: they are just pointers to
      the alarm records in ActiveAlarmList. Alarm records are freed
      when they are deleted from ActiveAlarmList.
   */
#if 0
   size = GlgArrayGetSize( alarm_list );
   for( i=0; i<size; ++i )
   {
      alarm = (AlarmRecord*) GlgArrayGetElement( datafeed->ActiveAlarmList, i );
      FreeAlarmData( alarm );
   }
#endif

   GlgDropObject( alarm_list );
}

/*-------------------------------------------------------------------------
| Send alarm acknowledgement for the alarm associated with the specified 
| tag.
*/
static GlgBoolean dfACKAlarm( DataFeed * datafeed_p, char * tag_source )
{
   DemoDataFeed * datafeed = (DemoDataFeed*) datafeed_p;
   AlarmRecord * alarm;
   int i, num_alarms;

   /* Simulate alarm ACK in the demo mode. In a real application, 
      LiveDataFeed will send alarm ACK to the process data server.
   */
   
   num_alarms = GlgArrayGetSize( datafeed->ActiveAlarmList );   
     
   /* Find the alarm in the active alarm list and reset it's ACK flag. */
   for( i=0; i<num_alarms; ++i )
   {
      alarm = (AlarmRecord*) GlgGetElement( datafeed->ActiveAlarmList, i );
      if( strcmp( alarm->tag_source, tag_source ) == 0 )
      {
         alarm->ack = GlgTrue;
         printf( "Acknowledging alarm: %s\n", tag_source );
         return GlgTrue;
      }
   }
   return GlgFalse;
}

/*----------------------------------------------------------------------
| 
*/
static AlarmRecord * dfGetAlarmData( DemoDataFeed * datafeed )
{
   AlarmRecord * alarm;
   int 
     alarm_status,
     random_message,
     random_number;
   double 
     alarm_time,
     double_value = 0.;
   char
     * description,
     * string_value = NULL,
     alarm_source[ 128 ];
  
   /* Simulate alarms for the demo. In a real application, LiveDataSource
      will query the list of alarms from the process data server.
   */
   alarm_status = GlgRand( 1., 3.99 );

   sprintf( alarm_source, "%c%d", 'A' + (int) GlgRand( 0., 26.9 ),
            (int) GlgRand( 100., 999. ) );

   if( alarm_status < 3 )
   {
      random_message = ( GlgRand( 0., 10. ) < 5. ? 0 : 1 );
      
      switch( random_message )
      {
       default:
       case 0:
         description = "Tank #% low";
         double_value = GlgRand( 100., 150. ); 
         break;
         
       case 1: 
         description = "Tank #% high";
         double_value = GlgRand( 600., 900. ); 
         break;
      }
   }
   else
   {
      random_message = GlgRand( 0., 2.99 );
      switch( random_message )
      {
       default:
       case 0: description = "Breaker #%"; string_value = "TRIPPED"; break;
       case 1: description = "Fuse #%"; string_value = "BLOWN"; break;
       case 2: description = "Tank #%"; string_value = "OVERFLOW"; break;
      }         
   }   

   random_number = GlgRand( 1., 100. );
   description = GlgCreateIndexedName( description, random_number );
   
   alarm_time = GetCurrTime();

   alarm = GlgAllocStruct( sizeof( AlarmRecord ) );   /* Initialized with 0s. */
   alarm->time = alarm_time;
   alarm->tag_source = GlgStrClone( alarm_source );
   alarm->description = description;    /* Was created above, don't clone. */
   alarm->string_value = GlgStrClone( string_value );
   alarm->double_value = double_value;
   alarm->status = alarm_status;
   alarm->ack = GlgFalse;
   return alarm;
}

/*----------------------------------------------------------------------
| Ages alarms and removes old alarms from the list for continuous
| simulation.
*/
static void AgeAlarms( DemoDataFeed * datafeed, GlgObject alarm_list )
{
   int i, size;
   AlarmRecord * alarm;

   /* Age all alarms and remove all old acknowledged alarms.
      For continuous simulation in the demo, also remove the first 
      old non-acknowledged alarm.
   */

   size = GlgArrayGetSize( alarm_list );
   for( i=0; i<size; ++i )
   {
      alarm = (AlarmRecord*) GlgArrayGetElement( datafeed->ActiveAlarmList, i );
      ++alarm->age;

      if( alarm->ack && alarm->age > OLD_ACK_ALARM_AGE ||
          !alarm->ack && alarm->age > OLD_NON_ACK_ALARM_AGE )
      {
         FreeAlarmData( alarm );
         GlgArrayDeleteAt( datafeed->ActiveAlarmList, i );
         return;
      }
   }
}

/*----------------------------------------------------------------------
| Returns an allocated alarm time stamp string.
*/
static char * GetTimeString()
{
#define BUFFER_SIZE  256   
   char buffer[ BUFFER_SIZE ];
   
   GlgULong sec, msec;
   struct tm * time_struct;
   time_t time;
   
   GlgGetTime( &sec, &msec );
   time = sec;
   time_struct = localtime( &time );
   if( !time_struct )
     return NULL;
   
   /* Format a label string */
   strftime( buffer, BUFFER_SIZE, "%H:%M:%S", time_struct );

   return GlgStrClone( buffer );
}
