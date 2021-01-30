#include "GlgViewer.h"

#define SLEEP_TIME  25    /* millisec */

#define RELATIVE_TO_NEW_RANGE( low, high, rel_value ) \
   ( (low) + ((high) - (low)) * rel_value )

void ProcessDemoData( void * sender_socket );
GlgBoolean GetGPSData( GPSData * data );
GlgBoolean GetTelemetryData( TelemetryData * data );
double GetDemoValue( double low, double high );
void GetDemoPosition( GlgPoint * latlon, double lon_min, double lon_max,
                      double lat_min, double lat_max,
                      double alt_low, double alt_high );

/*--------------------------------------------------------------------
| Get data from an application data source and send them to GUI via
| the provided sender socket.
*/
void ProcessDemoData( void * sender_socket )
{
   GPSData gps_data;
   TelemetryData telemetry_data;

   /* In the demo, use separate methods (GetGPSData and GetTelemetryData)
      to generate demo data for GPS and telemetry. In an application,
      a single method can be used to receive all data from either a single
      source/socket, or a multiple sources.
   */

   if( GetGPSData( &gps_data ) )
     SendDataToGUIThread( sender_socket, (BaseData*) &gps_data );

   if( GetTelemetryData( &telemetry_data ) )
     SendDataToGUIThread( sender_socket, (BaseData*) &telemetry_data );

   /* For a demo, put the thread to sleep after generateing data. 
      In an application, a single method that receives all data may simply 
      block if there are no incoming data.
   */
   SleepMS( SLEEP_TIME );
}

#define LOW 0
#define HIGH 100
#define MIN_LAT -90.
#define MAX_LAT 90.
#define MIN_LON -180.
#define MAX_LON +180.
#define ALT_LOW 0.
#define ALT_HIGH 10000.

/*--------------------------------------------------------------------
| Receive GPS data (using random values for demo) and store in the
| provided buffer. If no GPS data available, return False.
*/
GlgBoolean GetGPSData( GPSData * data )
{
   GlgPoint latlon;

   data->base.type = GPS;
   data->base.is_valid = GlgTrue;
   data->base.time_stamp = GetCurrTime();

   GetDemoPosition( &latlon, MIN_LON, MAX_LON,
                    MIN_LAT, MAX_LAT, ALT_LOW, ALT_HIGH );

   data->lat = latlon.y; 
   data->lon = latlon.x; 
   data->altitude = latlon.z;
   data->speed = GetDemoValue( LOW, HIGH ); 
   data->pitch = GetDemoValue( -30., 30. );
   data->roll = GetDemoValue( -30., 30. );
   data->yaw = GetDemoValue( -30., 30. );

   return True;
}

/*--------------------------------------------------------------------
| Receive TELEMETRY data (using random values for demo) and store in 
| the provided buffer. If no TELEMETRY data available, return False.
*/
GlgBoolean GetTelemetryData( TelemetryData * data )
{
   data->base.type = TELEMETRY;
   data->base.is_valid = GlgTrue;
   data->base.time_stamp = GetCurrTime();

   data->power = GetDemoValue( LOW, HIGH );  
   data->voltage = GetDemoValue( LOW, HIGH );  
   data->current = GetDemoValue( LOW, HIGH );  
   data->temperature = GetDemoValue( LOW, HIGH );  
   data->pressure = GetDemoValue( LOW, HIGH );  
   data->state_health = GetDemoValue( 0., 3. );

   return True;
}

/*----------------------------------------------------------------------*/
double GetDemoValue( double low, double high )
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
void GetDemoPosition( GlgPoint * latlon, 
                      double lon_min, double lon_max,
                      double lat_min, double lat_max,
                      double alt_low, double alt_high )
{
#define LAT_PERIOD     1000
#define LON_PERIOD     4000
#define OFFSET1     0
#define OFFSET2     0

   static long counter = 0;
   double
     angle1, angle2,
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

   /* Get a simulated altitude value. */
   latlon->z = GetDemoValue( alt_low, alt_high );
}

/*--------------------------------------------------------------------
| Simulte the "Write" opearion. For demo purposes, set the value
| of the specified tag source in the drawing. 
*/
GlgBoolean DemoWriteDValue( char * tag_source, double d_value )
{
   extern GlgObject Viewport;

   return GlgSetDTag( Viewport, tag_source, d_value, False );  
}

