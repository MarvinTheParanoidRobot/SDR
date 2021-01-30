#include <stdio.h>
#include <stdlib.h>
#include "GlgApi.h"
#include "DataFeed.h"

/* Run-time pointer check. */
#define DATA_FEED_TYPE_CHECK( df_ptr ) \
   ( df_ptr->type != DATA_FEED_TYPE ? df_type_error() : 1 )

/*-------------------------------------------------------------------------*/
static int df_type_error( void )
{
   GlgError( GLG_USER_ERROR, "Invalid DataFeed pointer" );
   exit( GLG_EXIT_ERROR );
}

/*-------------------------------------------------------------------------*/
void DF_Destroy( void * df )
{
   DataFeed * data_feed = (DataFeed*) df;
   DATA_FEED_TYPE_CHECK( data_feed );

   data_feed->Destroy( data_feed );
}

/*-------------------------------------------------------------------------*/
GlgBoolean DF_ReadDTagData( void * df, TagRecord * tag_record, 
                            double * value, double * time_stamp )
{
   DataFeed * data_feed = (DataFeed*) df;
   DATA_FEED_TYPE_CHECK( data_feed );
   
   return data_feed->ReadDTagData( data_feed, tag_record, value, time_stamp );
}

/*-------------------------------------------------------------------------*/
GlgBoolean DF_ReadSTagData( void * df, TagRecord * tag_record, char ** value )
{
   DataFeed * data_feed = (DataFeed*) df;
   DATA_FEED_TYPE_CHECK( data_feed );

   return data_feed->ReadSTagData( data_feed, tag_record, value );
}

/*-------------------------------------------------------------------------*/
GlgBoolean DF_WriteDTagData( void * df, char * tag_source, 
                            double value )
{
   DataFeed * data_feed = (DataFeed*) df;
   DATA_FEED_TYPE_CHECK( data_feed );

   return data_feed->WriteDTagData( data_feed, tag_source, value );
}

/*-------------------------------------------------------------------------*/
GlgBoolean DF_WriteSTagData( void * df, char * tag_source, 
                            char * string )
{
   DataFeed * data_feed = (DataFeed*) df;
   DATA_FEED_TYPE_CHECK( data_feed );

   return data_feed->WriteSTagData( data_feed, tag_source, string );
}

/*-------------------------------------------------------------------------*/
GlgObject DF_GetPlotData( void * df, char * tag_source, 
                          double start_time, double end_time, 
                          int max_num_samples )
{
   DataFeed * data_feed = (DataFeed*) df;
   DATA_FEED_TYPE_CHECK( data_feed );

   return data_feed->GetPlotData( data_feed, tag_source, start_time, end_time, 
                                  max_num_samples );
}

/*-------------------------------------------------------------------------*/
void DF_FreePlotData( void * df, GlgObject data_array )
{
   DataFeed * data_feed = (DataFeed*) df;
   DATA_FEED_TYPE_CHECK( data_feed );

   data_feed->FreePlotData( data_feed, data_array );
}

/*-------------------------------------------------------------------------*/
GlgObject DF_GetAlarms( void * df )
{
   DataFeed * data_feed = (DataFeed*) df;
   DATA_FEED_TYPE_CHECK( data_feed );

   return data_feed->GetAlarms( data_feed );
}

/*-------------------------------------------------------------------------*/
void DF_FreeAlarms( void * df, GlgObject alarm_list )
{
   DataFeed * data_feed = (DataFeed*) df;
   DATA_FEED_TYPE_CHECK( data_feed );

   data_feed->FreeAlarms( data_feed, alarm_list );
}

/*-------------------------------------------------------------------------*/
GlgBoolean DF_ACKAlarm( void * df, char * tag_source )
{
   DataFeed * data_feed = (DataFeed*) df;
   DATA_FEED_TYPE_CHECK( data_feed );

   return data_feed->ACKAlarm( data_feed, tag_source );
}
