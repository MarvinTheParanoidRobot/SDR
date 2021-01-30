#include "DemoDataFeed.h"
#include "GlgViewer.h"

#define LOW 0
#define HIGH 100
#define MIN_LAT -90.
#define MAX_LAT 90.
#define MIN_LON -180.
#define MAX_LON +180.
#define ALT_LOW 0.
#define ALT_HIGH 10000.

/*--------------------------------------------------------------------
| DemoDataFeed class is used to generate simulated demo data.
*/
DemoDataFeed::DemoDataFeed( GlgViewer * viewer ) : 
   DataFeedBase( viewer )
{
}

/*--------------------------------------------------------------------
| Invoked asynchronously from a thread. Accumulates received data 
| in AccumulatedData vector. 
|
| Thread locking is used inside DataFeedBase::StoreRawData() to ensure 
| synchronization for AccumulatedData object.
*/
#define SLEEP_MS 2 // sleep time in milliseconds
void DemoDataFeed::ProcessData( void )
{
   // Process GPS data (if any) and store them in AccumulatedData.
   GPSData * gps_data = GetGPSData();
   if( gps_data )
     StoreRawData( gps_data );
   
   // Process TELEMETRY data (if any) and store them in AccumulatedData.
   TelemetryData * telem_data = GetTelemetryData();
   if( telem_data )
     StoreRawData( telem_data );

   /* For a demo, put the data thread to sleep after processing data. 
      In an application, a method that receives data may simply 
      block if there are no incoming data.
   */
#ifndef _WINDOWS
   usleep( SLEEP_MS * 1000 );
#else
   Sleep( SLEEP_MS );
#endif
}

/*--------------------------------------------------------------------
| Receive GPS data (using random values for demo).
| If no GPS data available, return NULL.
*/
GPSData * DemoDataFeed::GetGPSData()
{
   GlgPoint latlon;
   GPSData * data = new GPSData();

   GetDemoPosition( &latlon, MIN_LON, MAX_LON,
                    MIN_LAT, MAX_LAT, ALT_LOW, ALT_HIGH );

   data->lat = latlon.y; 
   data->lon = latlon.x; 
   data->altitude = latlon.z;
   data->speed = GetDemoValue( LOW, HIGH ); 
   data->pitch = GetDemoValue( -30., 30. );
   data->roll = GetDemoValue( -30., 30. );
   data->yaw = GetDemoValue( -30., 30. );

   data->is_valid = GlgTrue;
   data->time_stamp = GetCurrTime();

   return data;
}

/*--------------------------------------------------------------------
| Receive TELEMETRY data (using random values for demo).
| If no TELEMETRY data available, return NULL.
*/
TelemetryData * DemoDataFeed::GetTelemetryData()
{
   TelemetryData * data = new TelemetryData();

   data->power = GetDemoValue( LOW, HIGH );  
   data->voltage = GetDemoValue( LOW, HIGH );  
   data->current = GetDemoValue( LOW, HIGH );  
   data->temperature = GetDemoValue( LOW, HIGH );  
   data->pressure = GetDemoValue( LOW, HIGH );  
   data->state_health = GetDemoValue( 0., 3. );

   data->is_valid = GlgTrue;
   data->time_stamp = GetCurrTime();

   return data;
}

/*--------------------------------------------------------------------
| Simulte the "Write" opearion for a D value. For demo purposes, 
| set the value of the specified tag source in the drawing. 
*/
GlgBoolean DemoDataFeed::WriteDValue( CONST char * tag_source, double d_value )
{
   return Viewer->SetTag( tag_source, d_value, GlgFalse );  
}


/*--------------------------------------------------------------------
| Simulte the "Write" opearion for S value. For demo purposes, set the value
| of the specified tag source in the drawing. 
*/
GlgBoolean DemoDataFeed::WriteSValue( CONST char * tag_source, CONST char * s_value )
{
   return Viewer->SetTag( tag_source, s_value, GlgFalse );  
}

/*----------------------------------------------------------------------*/
double DemoDataFeed::GetDemoValue( double low, double high )
{
#define PERIOD    5000
   static GlgLong counter = 0;
   double 
     value, 
     angle; 

   ++counter;
   angle = ( counter % PERIOD ) / (double) PERIOD;
   value = 0.5 + sin( 2. * M_PI * angle ) / 2.;      
   
   value = RELATIVE_TO_NEW_RANGE( low, high, value );
   
   return value;
}

/*----------------------------------------------------------------------*/
void DemoDataFeed::GetDemoPosition( GlgPoint * latlon, 
                                    double lon_min, double lon_max,
                                    double lat_min, double lat_max,
                                    double alt_low, double alt_high )
{
#define LAT_PERIOD     1000
#define LON_PERIOD     4000
#define OFFSET1     0
#define OFFSET2     0

   static long counter = 0;
   double angle1, angle2,
     lat, lon;

   ++counter;
   
   angle1 = ( counter % LAT_PERIOD ) / (double) LAT_PERIOD;
   angle2 = ( counter % LON_PERIOD ) / (double) LON_PERIOD;

   lat = 0.5 + sin( 2. * M_PI * angle1 + OFFSET1 ) / 2. * 0.9;
   lon = 0.5 + sin( 2. * M_PI * angle2 + OFFSET2 ) / 2. * 0.9;

   latlon->x = 
     RELATIVE_TO_NEW_RANGE( lon_min, lon_max, lon );

   latlon->y = 
     RELATIVE_TO_NEW_RANGE( lat_min, lat_max, lat );

   // Get a simulated altitude value.
   latlon->z = GetDemoValue( alt_low, alt_high );
}
