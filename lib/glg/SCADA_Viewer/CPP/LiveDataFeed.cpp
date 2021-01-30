
/* Base class containing methods for data acquisition. An application
   can extend LiveDataFeed class and provide a custom implementation
   of these methods.
*/

#include "LiveDataFeed.h"
#include "GlgTagRecord.h"
#include "AlarmRecord.h"
#include "PlotDataPoint.h"
#include "GlgSCADAViewer.h"

#define TEXT_BUFFER_LENGTH    1024

/*----------------------------------------------------------------------
| Constructor.
*/
LiveDataFeed::LiveDataFeed( GlgSCADAViewer * viewer ) : DataFeedBase( viewer )
{
   InitDataBase();
}

/*----------------------------------------------------------------------
| Destructor.
*/
LiveDataFeed::~LiveDataFeed()
{
}

/*----------------------------------------------------------------------
| Provide custom code to initialize data communication.
*/
GlgBoolean LiveDataFeed::InitDataBase()
{
   return GlgTrue;
}

/*----------------------------------------------------------------------
| Queries new D tag value from the database. Provide application specific
| code to query a double value and a time stamp.
*/
GlgBoolean LiveDataFeed::ReadDTag( GlgTagRecord * tag_record, 
                                   double * d_value, double * time_stamp )
{
   SCONST char * tag_source = tag_record->tag_source;

   /* Insert code to query value and time stamp for a given tag_source.
   *d_value = 0.;
   *time_stamp = GetCurrTime(); 
     
   /* An implementation of the datafeed class, such as LiveDataFeed that
      performs real-time data acquisition, should return GlgTrue on success,
      and GlgFalse on failure.
   */
   return GlgFalse; 
}

/*----------------------------------------------------------------------
| Queries new S tag value from the database and places it into str pointer. 
*/
GlgBoolean LiveDataFeed::ReadSTag( GlgTagRecord * tag_record, 
                                   SCONST char ** str  )
{   
   SCONST char * tag_source = tag_record->tag_source;

   static char buffer[ TEXT_BUFFER_LENGTH ];
   
   // Place the returned value into a static buffer, making sure to check 
   // the length of the string against TEXT_BUFFER_LENGTH to avoid 
   // buffer overruns. This avoids requiring the caller to free the returned
   // string.

   strcpy( buffer, "unset" );
   *str = buffer;

   /* A subclass of DataFeedBase, such as LiveDataFeed, should return 
      GlgTrue on success, and GlgFalse on failure.
   */
   return GlgFalse;
}

/*----------------------------------------------------------------------
| Writes a new numerical value into the specified tag_source.
*/
GlgBoolean LiveDataFeed::WriteDTag( SCONST char * tag_source, double value )
{
   /* Set new tag value of D-type. */

   return GlgTrue;
}

/*----------------------------------------------------------------------
| Writes a new string value into the specified tag_source.
*/
GlgBoolean LiveDataFeed::WriteSTag( SCONST char *tag_source, SCONST char * str )
{
   /* Set new tag value of S-type. */

   return GlgTrue;
}


/*----------------------------------------------------------------------
| Obtain a list of alarms.
*/
GlgBoolean LiveDataFeed::GetAlarms( AlarmRecordArrayType& alarm_list )
{
   /* Place custom code here to create and return an array of
      application alarms. Refer to GetAlarms() method of
      DemoDataFeed for an example of creating an array of alarms.
   */

   /******Example:
   int num_active_alarms = 
          
   for( int i=0; i<num_active_alarms; ++i )
   {
      AlarmRecord * alarm = new AlarmRecord();
      alarm->tag_source = GlgStrClone( ... );
      alarm->description = GlgStrClone( ... );
      alarm->string_value = GlgStrClone( ... );
      alarm->ack = GlgFalse; 
      alarm->double_value = 
      alarm->status = 
      alarm->time = 
      alarm_list.push_back( alarm );
   }
   
   return GlgTrue;
   ******/

   return GlgFalse; // return false of failure to obtain alarms.
}

/*----------------------------------------------------------------------
| Obtain a list of plot data points, used to prefill a GLG chart with
| historical data.
*/
GlgBoolean LiveDataFeed::GetPlotData( SCONST char * tag_source, 
                                      double start_time, double end_time,
                                      int max_num_samples,
                                      PlotDataArrayType& data_array )
{
   /* Place code here to query historical data for the plot specified by
      the provided tag_source. Refer to the GetPlotData() method of 
      DemoDataFeed for an example of creating an array of plot data points.
   */

   /**** Example:
   int num_data_samples = 
   for( int i=0; i<num_data_samples; ++i )
   {
      double d_value = 
      double time_stamp =
      unsigned char is_valid =  
      PlotDataPoint * data_point = new PlotDataPoint( d_value, time_stamp, is_valid );
      data_array.push_back( data_point ); 
   }
   return GlgTrue;
   *****/

   return GlgFalse;  // return false on failure.
}
  
/*----------------------------------------------------------------------
| Clear data_array -- free memory allocated for the data_array and its 
| elements.
*/
void LiveDataFeed::FreePlotData( PlotDataArrayType& data_array )
{
   DataFeedBase::FreePlotData( data_array );
} 

/*----------------------------------------------------------------------
| Free the alarm list data.
*/
void LiveDataFeed::FreeAlarms( AlarmRecordArrayType& alarm_list )
{
   DataFeedBase::FreeAlarms( alarm_list );
}

GlgBoolean LiveDataFeed::ACKAlarm( SCONST char * tag_source )
{
   /* Place custom code here to acknowledge an alarm for a given
      tag_source.
   */
   return GlgTrue;
}
