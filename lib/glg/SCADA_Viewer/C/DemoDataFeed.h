#ifndef _DemoDataFeed_h_
#define _DemoDataFeed_h_

#include "GlgApi.h"
#include "DataFeed.h"
#include "GlgSCADAViewer.h"

typedef struct _DemoDataFeed
{
   DataFeed DataFeed;

   /* Keeps a list of active alarms for simulation. */
   GlgObject ActiveAlarmList;
 
} DemoDataFeed;

DataFeed * CreateDemoDataFeed( void );
static void dfDestroy( DataFeed * datafeed );
static GlgBoolean dfReadDTagData( DataFeed * datafeed, TagRecord * tag_record,
                                  double * value, double * time_stamp );
static GlgBoolean dfReadSTagData( DataFeed * datafeed, TagRecord * tag_record,
                                  char ** value );
static GlgBoolean dfWriteDTagData( DataFeed * datafeed, 
                                   char *tag_source, double value );
static GlgBoolean dfWriteSTagData( DataFeed * datafeed, 
                                   char *tag_source, char * string );
static AlarmRecord * dfGetAlarmData( DemoDataFeed * datafeed );
static GlgObject dfGetPlotData( DataFeed * datafeed, char * tag_source, 
                                double start_time, double end_time,
                                int max_num_samples );
static void dfFreePlotData( DataFeed * datafeed, GlgObject data_array );
static GlgObject dfGetAlarms( DataFeed * datafeed );
static void dfFreeAlarms( DataFeed * datafeed, GlgObject alarm_list );
static GlgBoolean dfACKAlarm( DataFeed * datafeed, char * tag_source );
static GlgBoolean GetDemoValue( char * tag_source, double * value, 
                                GlgBoolean historical_mode );
static GlgBoolean GetDemoString( char * tag_source, char ** value );
static void AgeAlarms( DemoDataFeed * datafeed, GlgObject alarm_list );
static char * GetTimeString( void );

#endif
