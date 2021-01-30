#pragma once

#include "DataFeedBase.h"
#include "GlgViewer.h"

class LiveDataFeed : public DataFeedBase
{
 public:
   LiveDataFeed( GlgViewer * viewer );
   ~LiveDataFeed( void );

   // Virtual methods.
   void Init( void );
   void Terminate( void );
   void ProcessData( void );
   GlgBoolean WriteDValue( CONST char * tag_source, double value );
   GlgBoolean WriteSValue( CONST char * tag_source, CONST char * str );
};
