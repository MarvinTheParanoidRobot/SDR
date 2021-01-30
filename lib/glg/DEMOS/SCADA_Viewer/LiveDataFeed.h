#ifndef _LiveDataFeed_h_
#define _LiveDataFeed_h_

#include "GlgApi.h"
#include "DataFeed.h"
#include "GlgSCADAViewer.h"

typedef struct _LiveDataFeed
{
   DataFeed DataFeed;
   
   /* Additional fields may be added as needed. */

} LiveDataFeed;

DataFeed * CreateLiveDataFeed( void );
static void lfDestroy( DataFeed * datafeed );
static GlgBoolean lfReadDTagData( DataFeed * datafeed, TagRecord * tag_record,
                                  double * value, double * time_stamp );
static GlgBoolean lfReadSTagData( DataFeed * datafeed, TagRecord * tag_record,
                                  char ** value );
static GlgBoolean lfWriteDTagData( DataFeed * datafeed, 
                                   char *tag_source, double value );
static GlgBoolean lfWriteSTagData( DataFeed * datafeed, 
                                   char *tag_source, char * string );
static GlgBoolean lfGetAlarmData( DataFeed * datafeed, 
                                  AlarmRecord * alarm_record );
static GlgObject lfGetPlotData( DataFeed * datafeed, char * tag_source, 
                                double start_time, double end_time, 
                                int max_num_samples );
static void lfFreePlotData( DataFeed * datafeed, GlgObject data_array );
static GlgObject lfGetAlarms( DataFeed * datafeed );
static void lfFreeAlarms( DataFeed * datafeed, GlgObject alarm_list );
static GlgBoolean lfACKAlarm( DataFeed * datafeed, char * tag_source );

#endif
