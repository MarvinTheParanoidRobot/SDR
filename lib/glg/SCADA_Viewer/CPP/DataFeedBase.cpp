
/* Base class containing methods for data acquisition. An application
   can extend DataFeedBase class and provide a custom implementation
   of these methods.
*/

#include "DataFeedBase.h"
#include "PlotDataPoint.h"
#include "GlgTagRecord.h"
#include "AlarmRecord.h"
#include "GlgSCADAViewer.h"

#define TEXT_BUFFER_LENGTH    1024

DataFeedBase::DataFeedBase( GlgSCADAViewer * viewer )
{
   Viewer = viewer;
}

DataFeedBase::~DataFeedBase( void )
{
}

/*----------------------------------------------------------------------
| Queries new D tag value from the database.
*/
GlgBoolean DataFeedBase::ReadDTag( GlgTagRecord * tag_record, 
                                   double * d_value, double * time_stamp )
{
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
GlgBoolean DataFeedBase::ReadSTag( GlgTagRecord * tag_record, 
                                   SCONST char ** s_value  )
{   
   static char buffer[ TEXT_BUFFER_LENGTH ];
   
   // Place the returned value into a static buffer, making sure to check 
   // the length of the string against TEXT_BUFFER_LENGTH to avoid 
   // buffer overruns. This avoids requiring the caller to free the returned
   // string.

   strcpy( buffer, "unset" );
   *s_value = buffer;

   /* A subclass of DataFeedBase, such as LiveDataFeed, should return 
      GlgTrue on success, and GlgFalse on failure.
   */
   return GlgFalse;
}

/*----------------------------------------------------------------------
| Writes a new numerical value into the specified tag_source.
*/
GlgBoolean DataFeedBase::WriteDTag( SCONST char *tag_source, double value )
{
   /* Set new tag value of D-type. */

   return GlgFalse;
}

/*----------------------------------------------------------------------
| Writes a new string value into the specified tag_source.
*/
GlgBoolean DataFeedBase::WriteSTag( SCONST char *tag_source, SCONST char * str )
{
   /* Set new tag value of S-type. */

   return GlgFalse;
}


/*----------------------------------------------------------------------
| Obtain a list of alarms.
*/
GlgBoolean DataFeedBase::GetAlarms( AlarmRecordArrayType& alarm_list )
{
   return GlgFalse;
}

/*----------------------------------------------------------------------
| Obtain a list of plot data points, used to prefill a GLG chart with
| historical data, and store it in the data_array.
*/
GlgBoolean DataFeedBase::GetPlotData( SCONST char * tag_source, 
                                      double start_time, double end_time,
                                      int max_num_samples,
                                      PlotDataArrayType& data_array )
{
   return GlgFalse;
}
  
/*----------------------------------------------------------------------
 */
void DataFeedBase::FreePlotData( PlotDataArrayType& data_array )
{
   if( data_array.empty() )
     return;
 
   // Purge the content of vector elements.
   PlotDataArrayType::iterator it;
   for( it = data_array.begin(); it != data_array.end(); ++it )
      delete *it;

   data_array.clear();
}

/*----------------------------------------------------------------------
*/
void DataFeedBase::FreeAlarms( AlarmRecordArrayType& alarm_list )
{
   
   if( alarm_list.empty() )
     return;
   
   // Purge the content of vector elements.
   AlarmRecordArrayType::iterator it;
   for( it = alarm_list.begin(); it != alarm_list.end(); ++it )
      delete *it;

   alarm_list.clear();
}

GlgBoolean DataFeedBase::ACKAlarm( SCONST char * tag_source )
{
   return GlgFalse;
}
