#pragma once

#include "DataFeed.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

class DemoDataFeed : public DataFeedC
{
 public:
   DemoDataFeed( GlgMapViewer * viewer );
   ~DemoDataFeed();

   virtual GlgBoolean GetIconData( IconData * icon );

   DemoDataFeed& operator= ( const DataFeedC& object );
};
