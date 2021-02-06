#pragma once

#include "DataFeed.h"

class LiveDataFeed : public DataFeedC
{
 public:
   LiveDataFeed( GlgMapViewer * viewer );
   ~LiveDataFeed( void );

   virtual GlgBoolean GetIconData( IconData * icon );

   LiveDataFeed& operator= ( const DataFeedC& object );
};
