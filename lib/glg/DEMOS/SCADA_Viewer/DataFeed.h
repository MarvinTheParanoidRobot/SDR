#ifndef _DataFeed_h_
#define _DataFeed_h_

#include "GlgApi.h"
#include "DataTypes.h"

/* Used for run-time pointer check. */
#define DATA_FEED_TYPE   ( 'd' + 'f' )

typedef struct _DataFeed DataFeed;

struct _DataFeed
{
   int type;
   void (*Destroy) ( DataFeed * df );
   GlgBoolean (*ReadDTagData)( DataFeed * df, TagRecord * tag_record, 
                               double * value, double * time_stamp );
   GlgBoolean (*ReadSTagData)( DataFeed * df, TagRecord * tag_record,
                               char ** value );
   GlgBoolean (*WriteDTagData)( DataFeed * df, char *tag_source, double value );
   GlgBoolean (*WriteSTagData)( DataFeed * df, char *tag_source, char * string );
   GlgObject (*GetPlotData)( DataFeed * df, char * tag_source, 
                             double start_time, double end_time, 
                             int max_num_samples );
   void (*FreePlotData)( DataFeed * df, GlgObject data_array );
   GlgObject (*GetAlarms)( DataFeed * df );
   void (*FreeAlarms)( DataFeed * df, GlgObject alarm_list );
   GlgBoolean (*ACKAlarm)( DataFeed * df, char * tag_source );
};

static int df_type_error( void );
void DF_Destroy( void * df );
GlgBoolean DF_ReadDTagData( void * df, TagRecord * tag_record, 
                            double * value, double * time_stamp );
GlgBoolean DF_ReadSTagData( void * df, TagRecord * tag_record, char ** value );
GlgBoolean DF_WriteDTagData( void * df, char *tag_source, double value );
GlgBoolean DF_WriteSTagData( void * df, char *tag_source, char * string );
GlgObject DF_GetPlotData( void * df, char * tag_source, 
                          double start_time, double end_time, 
                          int max_num_samples );
void DF_FreePlotData( void * df, GlgObject data_array );
GlgObject DF_GetAlarms( void * df );
void DF_FreeAlarms( void * df, GlgObject alarm_list );
GlgBoolean DF_ACKAlarm( void * df, char * tag_source );

#endif
