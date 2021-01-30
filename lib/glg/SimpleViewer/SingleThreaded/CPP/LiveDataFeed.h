#pragma once

#include "DataFeed.h"
#include "GlgViewer.h"

class LiveDataFeed : public DataFeedC
{
 public:
   LiveDataFeed( GlgViewer * viewer );
   ~LiveDataFeed( void );

   GlgViewer * Viewer;

   // Virtual methods.
   virtual bool ReadDValue( TagRecordC * tag_record, double * d_value );
   virtual bool ReadSValue( TagRecordC * tag_record, CONST char ** s_value );
   virtual bool WriteDValue( CONST char * tag_source, double value );
   virtual bool WriteSValue( CONST char * tag_source, CONST char * str );

   LiveDataFeed& operator= ( const DataFeedC& object );
};
