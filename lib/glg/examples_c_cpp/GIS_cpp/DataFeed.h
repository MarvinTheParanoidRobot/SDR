#pragma once

#include "IconData.h"

class GlgMapViewer;

// Base Datafeed class, provides virtual methods that can be overriden
// by derived classes.
//
class DataFeedC
{
 public:
   DataFeedC( GlgMapViewer * viewer );
   ~DataFeedC() {};

   GlgMapViewer * Viewer;
   
   virtual GlgBoolean GetIconData( IconData * icon ) { return GlgTrue; };
};
