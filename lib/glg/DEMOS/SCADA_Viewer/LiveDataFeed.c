#include "LiveDataFeed.h"
#include "GlgSCADAViewer.h"

/* This module includes functions to perform data acquisition for
   live application data. Provide custom application code for these
   functions as needed.

   The data in GlgSCADAViewer structure are accessible via the global Viewer 
   variable.
*/

#define TEXT_BUFFER_LENGTH    1024

/*----------------------------------------------------------------------*/
DataFeed * CreateLiveDataFeed()
{
   LiveDataFeed * ldf = GlgAllocStruct( sizeof( LiveDataFeed ) );
   ldf->DataFeed.type = DATA_FEED_TYPE;
   
   ldf->DataFeed.ReadDTagData  = lfReadDTagData;
   ldf->DataFeed.ReadSTagData  = lfReadSTagData;
   ldf->DataFeed.WriteDTagData = lfWriteDTagData;
   ldf->DataFeed.WriteSTagData = lfWriteSTagData;
   ldf->DataFeed.GetPlotData   = lfGetPlotData;
   ldf->DataFeed.FreePlotData  = lfFreePlotData;
   ldf->DataFeed.GetAlarms     = lfGetAlarms;
   ldf->DataFeed.FreeAlarms    = lfFreeAlarms;
   ldf->DataFeed.ACKAlarm      = lfACKAlarm;
   ldf->DataFeed.Destroy       = lfDestroy;

   return (DataFeed*) ldf;
}

/*----------------------------------------------------------------------*/
static void lfDestroy( DataFeed * datafeed )
{
   GlgFree( datafeed );
}

/*----------------------------------------------------------------------
| Queries new D tag value from the database.
*/
static GlgBoolean lfReadDTagData( DataFeed * datafeed, TagRecord * tag_record,
                                  double * value, double * time_stamp )
{
   char * tag_source;

   /* Insert code here for live data value and time stamp for a 
      given tag source.
   */
   tag_source = tag_record->tag_source; 
   *value = 0.;
   *time_stamp = GetCurrTime();
   return GlgTrue;
}

/*----------------------------------------------------------------------
| Queries new S tag value from the database.
*/
static GlgBoolean lfReadSTagData( DataFeed * datafeed, TagRecord * tag_record,
                                  char ** value )
{   
   static char buffer[ TEXT_BUFFER_LENGTH ];
   
   /* Place your code here to query the value of the tag specified by 
      tag_record->tag_source.
   
      For performace reasons, this function uses a static buffer to return
      the string instead of returning an allocated string. This eliminates 
      the need to free the returned string by the caller and limits the
      number of malloc/free calls.

      The use of a static buffer here is OK because the returned value is 
      used right away and is not stored.

      When copying the returned value in the static buffer, make sure to check 
      the length of the string against TEXT_BUFFER_LENGTH to avoid 
      buffer overruns.
   */

   strcpy( buffer, "unset" );
   *value = buffer;
   return GlgTrue;
}

/*----------------------------------------------------------------------
|
*/
static GlgBoolean lfWriteDTagData( DataFeed * datafeed, 
                                   char *tag_source, double value )
{
   /* Set new tag value of D-type. */

   return GlgTrue;
}

/*----------------------------------------------------------------------
|
*/
static GlgBoolean lfWriteSTagData( DataFeed * datafeed, 
                                   char *tag_source, char * string )
{
   /* Set new tag value of S-type. */

   return GlgTrue;
}

/*----------------------------------------------------------------------
| Obtain plot data for the specified tag source and time interval.
| Data are held in a GLG array containing PlotDataPoint elements.
| See DemoDataFeed's GetPlotData() for an example.
*/
static GlgObject lfGetPlotData( DataFeed * datafeed, char * tag_source, 
                                double start_time, double end_time, 
                                int max_num_samples )
{
   return NULL;
}

/*----------------------------------------------------------------------
| Frees plot data (GLG Array containing PlotDataPoint elements).
| See DemoDataFeed's FreePlotData() for an example.
*/
static void lfFreePlotData( DataFeed * datafeed, GlgObject data_array )
{
}

/*----------------------------------------------------------------------
| Obtain alarm data. 
*/
static GlgBoolean lfGetAlarmData( DataFeed * datafeed, 
                                  AlarmRecord * alarm_record )
{
   return GlgFalse;
}

/*----------------------------------------------------------------------
| Obtain a list of process alarms and return it as a GLG array 
| containing alarm records.
*/
static GlgObject lfGetAlarms( DataFeed * datafeed )
{
   /* Place custom code here as needed to populate AlarmDialog with
      alarms information.
   */

   /*
   AlarmRecord alarm_record;

   if( GetAlarmData( &alarm_record )
   {
      PushAlarm( &alarm_record );   
      FreeAlarmData( &alarm_record );
   }
   */

   return NULL;
}

/*----------------------------------------------------------------------
| Frees the alarm list data.
*/
static void lfFreeAlarms( DataFeed * datafeed, GlgObject alarm_list )
{
   /* Place custom code to free alarm data. */
}

/*-------------------------------------------------------------------------
| Send alarm acknowledgement for the specified tag.
*/
static GlgBoolean lfACKAlarm( DataFeed * datafeed, char * tag_source )
{
   return GlgTrue;
}
