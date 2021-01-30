import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
// DemoDataFeed provides simulated data for demo, as well as for testing 
// with no LiveDataFeed.
// In an application, data will be coming from LiveDataFeed.
//////////////////////////////////////////////////////////////////////////

public class DemoDataFeed extends DataFeedBase
{
   // Low/High range for simulated data. 
   double HIGH = 100.;
   double LOW = 0.;
   double MIN_LAT = -90.;
   double MAX_LAT = 90.;
   double MIN_LON = -180.;
   double MAX_LON = +180.;
   double ALT_LOW = 0.;
   double ALT_HIGH = 10000.;
   
   // Used to generate simulated data.
   long counter1 = 0;
   long counter2 = 0;

   // Sleep time in milliseconds.
   static final int SLEEP_MS = 2; 

   /////////////////////////////////////////////////////////////////////// 
   public DemoDataFeed( GlgViewer viewer ) 
   {
      super( viewer );
   }

   /////////////////////////////////////////////////////////////////////// 
   // Invoked asynchronously from a thread. Accumulates received data 
   // in AccumulatedData vector. 
   // 
   // Thread synchronization is used inside DataFeedBase.StoreRawData() 
   // to ensure synchronization for AccumulatedData object.
   /////////////////////////////////////////////////////////////////////// 
   @Override
   void ProcessData()
   {
      // Process GPS data (if any) and store them in AccumulatedData.
      GPSData gps_data = GetGPSData();
      if( gps_data != null )
        StoreRawData( gps_data );
      
      // Process TELEMETRY data (if any) and store them in AccumulatedData.
      TelemetryData telem_data = GetTelemetryData();
      if( telem_data != null )
        StoreRawData( telem_data );
      
      /* For a demo, put the data thread to sleep after processing data. 
         In an application, a method that receives data may simply 
         block if there are no incoming data.
      */
       try 
       {
          DataThread.sleep( SLEEP_MS );
       } 
       catch (InterruptedException e) 
       {
          System.out.println( "DataThread sleep interrupted." );
       }
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Receive GPS data (using random values for demo).
   // If no GPS data available, return null.
   ///////////////////////////////////////////////////////////////////////
   GPSData GetGPSData()
   {
      GPSData data = new GPSData();
      
      GlgPoint latlon = 
        GetDemoPosition( MIN_LON, MAX_LON, MIN_LAT, MAX_LAT, 
                         ALT_LOW, ALT_HIGH );
      
      data.lat = latlon.y; 
      data.lon = latlon.x; 
      data.altitude = latlon.z;
      data.speed = GetDemoValue( LOW, HIGH ); 
      data.pitch = GetDemoValue( -30., 30. );
      data.roll = GetDemoValue( -30., 30. );
      data.yaw = GetDemoValue( -30., 30. );
      
      data.is_valid = true;
      data.time_stamp = GetCurrTime();
      
      return data;
   }

   ///////////////////////////////////////////////////////////////////////
   // Receive TELEMETRY data (using random values for demo).
   // If no TELEMETRY data available, return NULL.
   ///////////////////////////////////////////////////////////////////////
   TelemetryData GetTelemetryData()
   {
      TelemetryData data = new TelemetryData();
      
      data.power = GetDemoValue( LOW, HIGH );  
      data.voltage = GetDemoValue( LOW, HIGH );  
      data.current = GetDemoValue( LOW, HIGH );  
      data.temperature = GetDemoValue( LOW, HIGH );  
      data.pressure = GetDemoValue( LOW, HIGH );  
      data.state_health = GetDemoValue( 0., 3. );
      
      data.is_valid = true;
      data.time_stamp = GetCurrTime();
      
      return data;
   }
   
   /////////////////////////////////////////////////////////////////////// 
   @Override
   boolean WriteDValue( String tag_source, double d_value )
   {
      /* In the demo mode, set the tag in the drawing to simulate round trip
         after the write operation.
      */
      return Viewer.SetDTag( tag_source, d_value, false );
   }

   /////////////////////////////////////////////////////////////////////// 
   @Override
   boolean WriteSValue( String tag_source, String s_value )
   {
      /* In the demo mode, set the tag in the drawing to simulate round trip
         after the write operation.
      */
      return Viewer.SetSTag( tag_source, s_value, false );
   }
   
   /////////////////////////////////////////////////////////////////////// 
   double GetDemoValue( double low, double high )
   {
      int PERIOD = 5000;
      double 
        value, 
        angle; 

      ++counter1;
      angle = ( counter1 % PERIOD ) / (double) PERIOD;
      value = 0.5 + Math.sin( 2. * Math.PI * angle ) / 2.;      
      
      return ( value * ( high - low ) + low );
   }
   
   /////////////////////////////////////////////////////////////////////// 
   GlgPoint GetDemoPosition( double lon_min, double lon_max,
                                    double lat_min, double lat_max,
                                    double alt_low, double alt_high )
   {
      int LAT_PERIOD = 1000;
      int LON_PERIOD = 4000;
      int OFFSET1 = 0;
      int OFFSET2 = 0;

      double
        angle1, angle2,
        lat, lon;
      
      ++counter2;
   
      angle1 = ( counter2 % LAT_PERIOD ) / (double) LAT_PERIOD;
      angle2 = ( counter2 % LON_PERIOD ) / (double) LON_PERIOD;
      
      lat = 0.5 + Math.sin( 2. * Math.PI * angle1 + OFFSET1 ) / 2. * 0.9;
      lon = 0.5 + Math.sin( 2. * Math.PI * angle2 + OFFSET2 ) / 2. * 0.9;
      
      GlgPoint latlon = new GlgPoint();
      latlon.x = lon * ( lon_max - lon_min ) + lon_min;
      latlon.y = lat * ( lat_max - lat_min ) + lat_min;
            
      // Get a simulated altitude value.
      latlon.z = GetDemoValue( alt_low, alt_high );

      return latlon;
   }
}
   
   
