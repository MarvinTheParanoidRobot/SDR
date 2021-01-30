#pragma once

#include "DataFeedBase.h"

class DemoDataFeed : public DataFeedBase
{
 public:
   DemoDataFeed( GlgViewer * viewer );
   ~DemoDataFeed( void );

   // Virtual method of the base class that gets overriden by this class.
   void ProcessData( void );
   
   // Sample functions to fill sample data structures with simulated data. 
   GPSData * GetGPSData( void );
   TelemetryData * GetTelemetryData( void );

   GlgBoolean WriteDValue( CONST char * tag_source, double value );
   GlgBoolean WriteSValue( CONST char * tag_source, CONST char * str );
   double GetDemoValue( double low, double high );
   void GetDemoPosition( GlgPoint * latlon, 
                         double lon_min, double lon_max,
                         double lat_min, double lat_max,
                         double alt_low, double alt_high );
};
