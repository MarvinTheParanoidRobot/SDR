
/* Base class containing methods for data acquisition. An application
   can extend DataFeedBase class and provide a custom implementation
   of these methods.
*/

#include "DemoDataFeed.h"
#include "DataFeedBase.h"
#include "GlgTagRecord.h"
#include "AlarmRecord.h"
#include "PlotDataPoint.h"
#include "GlgSCADAViewer.h"

#define TEXT_BUFFER_LENGTH    1024

DemoDataFeed::DemoDataFeed( GlgSCADAViewer * viewer ) : DataFeedBase( viewer )
{
}

DemoDataFeed::~DemoDataFeed( void )
{
}

/*----------------------------------------------------------------------
| Queries new D tag value from the database.
*/
GlgBoolean DemoDataFeed::ReadDTag( GlgTagRecord * tag_record, 
                                   double * d_value, double * time_stamp )
{
   *time_stamp = GetCurrTime(); 
   return GetDemoValue( tag_record->tag_source, d_value, GlgFalse );
}

/*----------------------------------------------------------------------
| Queries new S tag value from the database and places it into str pointer. 
*/
GlgBoolean DemoDataFeed::ReadSTag( GlgTagRecord * tag_record, 
                                   SCONST char ** s_value  )
{
   return GetDemoString( tag_record->tag_source, s_value );
}

/*----------------------------------------------------------------------
| Writes a new numerical value into the specified tag_source.
| For demo purposes, set the tag value in the drawing. 
| In an application, the value should be written to the specified
| tag source in the back-end system.
*/
GlgBoolean DemoDataFeed::WriteDTag( SCONST char * tag_source, double value )
{
   /* Set new tag value of D-type. */

   Viewer->SetTag( tag_source, value, GlgTrue );
   Viewer->Update();

   return GlgTrue;
}

/*----------------------------------------------------------------------
| Writes a new string value into the specified tag_source.
*/
GlgBoolean DemoDataFeed::WriteSTag( SCONST char * tag_source, SCONST char * str )
{
   /* Set new tag value of S-type. */

   return GlgTrue;
}

/*----------------------------------------------------------------------
| Obtain a list of plot data points, used to prefill a GLG chart with
| historical data.
*/
GlgBoolean DemoDataFeed::GetPlotData( SCONST char * tag_source, 
                                      double start_time, double end_time,
                                      int max_num_samples,
                                      PlotDataArrayType& data_array )
{
   /* In a real application, the number of data points to be queried
      is determined by the start and end time. For the demo, return
      the requested max number of points.
   */
   if( max_num_samples < 1 )
     max_num_samples = 1;
   int num_samples = max_num_samples;
   
   double interval = ( end_time - start_time ) / max_num_samples;

   for( int i=0; i<num_samples; ++i )
   {
      double d_value;
      GlgBoolean is_valid;

      // Generate demo data.
      double time_stamp = start_time + interval * i;
      if( !GetDemoValue( tag_source, &d_value, GlgTrue ) )
      {
         d_value = 0.;
         is_valid = GlgFalse; 
      }
      else
        is_valid = GlgTrue;
      
      PlotDataPoint * data_point = 
        new PlotDataPoint( d_value, time_stamp, is_valid );
      data_array.push_back( data_point );
   }

   return GlgTrue;
}
  
/*----------------------------------------------------------------------
| Clear data_array -- free memory allocated for the data_array and its 
| elements.
*/
void DemoDataFeed::FreePlotData( PlotDataArrayType& data_array )
{
   DataFeedBase::FreePlotData( data_array );
}

/*----------------------------------------------------------------------*/
GlgBoolean DemoDataFeed::GetDemoValue( SCONST char * tag_source, 
                                       double * out_value, 
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
   
   switch( Viewer->PageType )
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
      if( tag_index >= Viewer->NumTagRecords )
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
GlgBoolean DemoDataFeed::GetDemoString( SCONST char * tag_source, 
                                        SCONST char ** out_value )
{
   *out_value = ( GlgRand( 0., 1. ) > 0.5 ? "On" : "Off" );
   return True;
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
| Obtain a list of alarms using simulated data.
*/
GlgBoolean DemoDataFeed::GetAlarms( AlarmRecordArrayType& alarm_list )
{
   AlarmRecord * alarm;
   double alarm_raise_threshold;
 
   /* Simulate alarms. */

   /* Ages alarms and removes old alarms from the list for continuous
      simulation.
   */
   AgeAlarms( ActiveAlarmList );
   GlgLong num_active_alarms = ActiveAlarmList.size();

   alarm_raise_threshold = 
     ( num_active_alarms == 0 ? FIRST_ALARM_RAISE_THRESHOLD : 
       ALARM_RAISE_THRESHOLD );

   /* Add new simulated alarm (conditionally). */
   if( num_active_alarms < NUM_SIMULATED_ALARMS &&
       GlgRand( 0., 1. ) > alarm_raise_threshold )
   {
      alarm = GetAlarmData(); 
      ActiveAlarmList.push_back( alarm );
   }
      
   if( ActiveAlarmList.empty() )
     return GlgFalse;   /* No alarms. */

   /* Create a new list of alarms to be returned. For simulating alarms 
      in the demo, populate the list with alarms from ActiveAlarmList.
   */
   AlarmRecordArrayType::iterator it;
   for( it=ActiveAlarmList.begin(); it != ActiveAlarmList.end(); ++it )
      alarm_list.push_back( *it );

   return GlgTrue;
}

/*----------------------------------------------------------------------
| Free the alarm list data.
*/
void DemoDataFeed::FreeAlarms( AlarmRecordArrayType& alarm_list )
{
   /* In demo mode, clear the alarm_list, but don't free content of 
      alarm_list elements: it contains pointers to alarm records 
      in ActiveAlarmList, which get freed when alarms are removed 
      from ActiveAlarmList. 

      In live data mode, free alarm_list properly, using FreeAlarms() 
      of the base class, where the content of alarm_list gets freed. 
   */
#if 0
   DataFeedBase::FreeAlarms( alarm_list );
#else
   alarm_list.clear();
#endif
}

/*-------------------------------------------------------------------------
| Send alarm acknowledgement for the alarm associated with the specified 
| tag.
*/
GlgBoolean DemoDataFeed::ACKAlarm( SCONST char * tag_source )
{
   /* Simulate alarm ACK in demo mode. In a real application, 
      LiveDataFeed will send alarm ACK to the process data server.
   */
   
   AlarmRecordArrayType::iterator it;
   for( it = ActiveAlarmList.begin(); it != ActiveAlarmList.end(); ++it )
   {
      AlarmRecord * alarm = *it;
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
| Simulate alarms for the demo. In a real application, LiveDataSource
| will query the list of alarms from the process data server.
*/
AlarmRecord * DemoDataFeed::GetAlarmData( void )
{
   double double_value = 0.;
   char  alarm_source[ 128 ];
   SCONST char
     * description,
     * string_value = NULL;
   int random_message;

   /* Simulate alarms for the demo. In a real application, LiveDataSource
      will query the list of alarms from the process data server.
   */
   int alarm_status = GlgRand( 1., 3.99 );

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

   int random_number = GlgRand( 1., 100. );
   description = GlgCreateIndexedName( description, random_number );
   
   double alarm_time = GetCurrTime();

   AlarmRecord * alarm = new AlarmRecord();
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
void DemoDataFeed::AgeAlarms( AlarmRecordArrayType& alarm_list )
{
   if( alarm_list.empty() )
     return;

   /* Age all alarms and remove all old acknowledged alarms.
      For continuous simulation in the demo, also remove the first 
      old non-acknowledged alarm.
   */
   AlarmRecordArrayType::iterator it;
   for( it = alarm_list.begin(); it != alarm_list.end(); ++it )
   {
      AlarmRecord * alarm = *it;
      ++alarm->age;

      if( alarm->ack && alarm->age > OLD_ACK_ALARM_AGE ||
          !alarm->ack && alarm->age > OLD_NON_ACK_ALARM_AGE )
      {
         delete (*it);            // free memory allocated for the element.
         alarm_list.erase( it );  //delete element from the alarm_list.
         return;
      }
   }
}

/*----------------------------------------------------------------------
| Returns an allocated alarm time stamp string.
*/
SCONST char * DemoDataFeed::GetTimeString()
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
