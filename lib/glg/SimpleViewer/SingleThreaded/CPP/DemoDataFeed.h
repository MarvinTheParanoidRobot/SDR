#pragma once

#include "DataFeed.h"
#include "GlgViewer.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

class DemoDataFeed : public DataFeedC
{
 public:
   DemoDataFeed( GlgViewer * viewer );
   ~DemoDataFeed( void );

   GlgViewer * Viewer;

   // Virtual methods.
   virtual bool ReadDValue( TagRecordC * tag_record, double * d_value );
   virtual bool ReadSValue( TagRecordC * tag_record, CONST char ** s_value );
   virtual bool WriteDValue( CONST char * tag_source, double value );
   virtual bool WriteSValue( CONST char * tag_source, CONST char * str );

   DemoDataFeed& operator= ( const DataFeedC& object );
};
