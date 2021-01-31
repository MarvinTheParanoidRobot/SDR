import com.genlogic.*;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Iterator;
import java.text.SimpleDateFormat;

//////////////////////////////////////////////////////////////////////////
// DemoDataFeed provides simulated data for demo, as well as for testing 
// with no LiveDataFeed.
// In an application, data will be coming from LiveDataFeed.
//////////////////////////////////////////////////////////////////////////

public class DemoDataFeed implements DataFeedInterface
{
   GlgSCADAViewer Viewer;

   long counter = 0;
   long tag_index = 0;

   static final double FIRST_ALARM_RAISE_THRESHOLD = 0.6;
   static final double ALARM_RAISE_THRESHOLD = 0.8;
      
   static final int OLD_ACK_ALARM_AGE = 15;
   static final int OLD_NON_ACK_ALARM_AGE = 30;
   static final int NUM_SIMULATED_ALARMS = 30;

   // Keeps a list of active alarms for simulation.
   ArrayList<AlarmRecord> ActiveAlarmList = new ArrayList<AlarmRecord>();
      
   /////////////////////////////////////////////////////////////////////// 
   public DemoDataFeed( GlgSCADAViewer viewer ) 
   {
      Viewer = viewer;
   }

   /////////////////////////////////////////////////////////////////////// 
   public boolean ReadDTag( GlgTagRecord tag_record, DataPoint data_point )
   {
      data_point.d_value = GetDemoValue( tag_record.tag_source, false );
      data_point.time_stamp = Viewer.GetCurrTime();
      return true;
   }

   /////////////////////////////////////////////////////////////////////// 
   public boolean ReadSTag( GlgTagRecord tag_record, DataPoint data_point )
   {
      data_point.s_value = GetDemoString( tag_record.tag_source );
      data_point.time_stamp = Viewer.GetCurrTime();
      return true;
   }

   /////////////////////////////////////////////////////////////////////// 
   public boolean WriteDTag( String tag_source, DataPoint data_point )
   {
      /* In demo mode, set the tag in the drawing to simulate round trip
         after the write operation.
      */
      return Viewer.SetDTag( tag_source, data_point.d_value, false );
   }

   public boolean WriteSTag( String tag_source, DataPoint data_point )
   {
      /* In demo mode, set the tag in the drawing to simulate round trip
         after the write operation.
      */
      return Viewer.SetSTag( tag_source, data_point.s_value, false );
   }

   //////////////////////////////////////////////////////////////////////////
   // Queries alarm data from the controlled process, uses simulated alarms
   // for the demo.
   //////////////////////////////////////////////////////////////////////////
   public ArrayList<AlarmRecord> GetAlarms()
   {
      AlarmRecord alarm;

      /* Simulate alarms. */

      /* Ages alarms and removes old alarms from the list for contineous
         simulation.
      */
      AgeAlarms( ActiveAlarmList );
      int num_active_alarms = ActiveAlarmList.size();

      double alarm_raise_threshold = 
        ( num_active_alarms == 0 ? FIRST_ALARM_RAISE_THRESHOLD : 
          ALARM_RAISE_THRESHOLD );

      // Add new simulated alarm (conditionally).
      if( num_active_alarms < NUM_SIMULATED_ALARMS &&          
          GlgObject.Rand( 0.0, 1.0 ) > alarm_raise_threshold )
      {
         alarm = GetAlarmData(); 
         ActiveAlarmList.add( alarm );
         num_active_alarms = ActiveAlarmList.size();
      }
      
      if( num_active_alarms == 0 )
        return null;   // No alarms.

      // Create a new list of alarms to be returned.
      ArrayList<AlarmRecord> alarm_list = new ArrayList<AlarmRecord>();
     
      /* For simulating alarms in the demo, populate the list with alarms from 
         ActiveAlarmList.
      */
      for( int i=0; i<num_active_alarms; ++i )
      {
         alarm = ActiveAlarmList.get( i );
         alarm_list.add( alarm );
      }

      return alarm_list;
   }
      
   /////////////////////////////////////////////////////////////////////// 
   // Ages alarms and removes old alarms from the list for continuous
   // simulation.
   ///////////////////////////////////////////////////////////////////////
   void AgeAlarms( ArrayList<AlarmRecord> alarm_list )
   {
      /* Age all alarms and remove all old acknowledged alarms.
         For continuous simulation in the demo, also remove the first 
         old non-acknowledged alarm.
      */
      Iterator<AlarmRecord> iterator = alarm_list.iterator();
      while( iterator.hasNext() )
      {
         AlarmRecord alarm = iterator.next();
         ++alarm.age;
         if( alarm.ack && alarm.age > OLD_ACK_ALARM_AGE ||
             !alarm.ack && alarm.age > OLD_NON_ACK_ALARM_AGE )
         {
            iterator.remove();
            return;
         }
      }
   }

   /////////////////////////////////////////////////////////////////////// 
   // Generates demo data value.
   ///////////////////////////////////////////////////////////////////////
   public double GetDemoValue( String tag_source, boolean historical_mode )
   {
      switch( Viewer.PageType )
      {
       case GlgSCADAViewer.AERATION_PAGE: 
         return GlgObject.Rand( 0.0, 10.0 );

       case GlgSCADAViewer.CIRCUIT_PAGE:
         if( tag_source.endsWith( "State" ) )
           return GlgObject.Rand( 0.0, 1.3 );
         else
           return GlgObject.Rand( 0.0, 1000.0 );
            
       case GlgSCADAViewer.RT_CHART_PAGE:
         double 
           center,
           amplitude,
           period, alpha,
           value;

         if( tag_source.startsWith( "Volts" ) )
         {
            center = 380.0;
            amplitude = 40.0;
            period = 300.0;
         }
         else
         {
            center = 30.0;
            amplitude = 15.0;
            period = 1000.0;
         }

         alpha = 2.0 * Math.PI * counter / period;        
         value = center + 
           amplitude * Math.sin( alpha ) * Math.sin( alpha / 30.0 );
         
         if( historical_mode )
           counter += 10;   // Historical data, data were saved once per second.
         else
           ++counter;       // Real-time mode: updates 10 times per second.
         return value;

       case GlgSCADAViewer.TEST_COMMANDS_PAGE:
         /* Generate demo data in the range [10,90] for the real-time chart
            in the popup dialog.
         */
         period = 100.0 * ( 1.0 + tag_index * 2.0 );
         alpha = 2.0 * Math.PI * counter / period;
         value = 50.0 + 40.0 * Math.sin( alpha ) * Math.sin( alpha / 30.0 );
            
         ++tag_index;
         if( tag_index >= Viewer.NumTagRecords )
           tag_index = 0;
            
         ++counter;
         return value;
            
       default: return GlgObject.Rand( 0.0, 100.0 );
      }
   }

   /////////////////////////////////////////////////////////////////////// 
   // Obtain an array of historical data for a given tag source.
   /////////////////////////////////////////////////////////////////////// 
   public ArrayList<PlotDataPoint> 
     GetPlotData( String tag_source, double start_time, double end_time,
                  int max_num_samples )
   {
      /* In a real application, the number of data points to be queried
         is determined by the start and end time. For the demo, return
         the requested max number of points.
      */
      if( max_num_samples < 1 )
        max_num_samples = 1;
      int num_samples = max_num_samples;
      
      double interval = ( end_time - start_time ) / max_num_samples;

      ArrayList<PlotDataPoint> data_array = 
        new ArrayList<PlotDataPoint>();
      
      for( int i=0; i<num_samples; ++i )
      {
         /* Generate demo data. */
         PlotDataPoint data_point = new PlotDataPoint();
         data_point.value = GetDemoValue( tag_source, true );
         data_point.time_stamp = start_time + interval * i;
         data_point.value_valid = true;
         data_array.add( data_point );
      }
      return data_array;
   }

   /////////////////////////////////////////////////////////////////////// 
   public String GetDemoString( String tag_source )
   {
      return ( GlgObject.Rand( 0.0, 1.0 ) > 0.5 ? "On" : "Off" );
   }

   //////////////////////////////////////////////////////////////////////////
   public AlarmRecord GetAlarmData()
   {
      /* Simulate alarms for the demo. In a real application, LiveDataSource
         will query the list of alarms from the process data server.
      */

      int alarm_status = (int) GlgObject.Rand( 1.0, 3.99 );

      char ch = (char) ( 'A' + (int) GlgObject.Rand( 0.0, 26.9 ) );
      String alarm_source = "" + ch + (int) GlgObject.Rand( 100.0, 999.0 );

      String description;      
      String string_value = null;
      double double_value = 0.0;
      int random_message;

      if( alarm_status < 3 )
      {
         random_message = ( GlgObject.Rand( 0.0, 10.0 ) < 5.0 ? 0 : 1 );

         switch( random_message )
         {
          default:
          case 0:
            description = "Tank #% low";
            double_value = GlgObject.Rand( 100.0, 150.0 ); 
            break;

          case 1: 
            description = "Tank #% high";
            double_value = GlgObject.Rand( 600.0, 900.0 ); 
            break;
         }
      }
      else
      {
         random_message = (int) GlgObject.Rand( 0.0, 2.99 );
         switch( random_message )
         {
          default:
          case 0: description = "Breaker #%"; string_value = "TRIPPED"; break;
          case 1: description = "Fuse #%"; string_value = "BLOWN"; break;
          case 2: description = "Tank #%"; string_value = "OVERFLOW"; break;
         }         
      }   

      int random_number = (int) GlgObject.Rand( 1.0, 100.0 );
      description = GlgObject.CreateIndexedName( description, random_number );

      double alarm_time = Viewer.GetCurrTime();

      AlarmRecord alarm = new AlarmRecord();
      alarm.time = alarm_time;
      alarm.tag_source = alarm_source;
      alarm.description = description;
      alarm.string_value = string_value;
      alarm.double_value = double_value;
      alarm.status = alarm_status;
      alarm.ack = false;
      return alarm;
   }

   //////////////////////////////////////////////////////////////////////////
   public boolean ACKAlarm( String tag_source )
   {
      /* Simulate alarm ACK in the demo mode. In a real application, 
         LiveDataFeed will send alarm ACK to the process data server.
      */

      int num_alarms = ActiveAlarmList.size();   
     
      // Find the alarm in the active alarm list and reset it's ACK flag.
      for( int i=0; i<num_alarms; ++i )
      {
         AlarmRecord alarm = ActiveAlarmList.get( i );
         if( alarm.tag_source.equals( tag_source ) )
         {
            alarm.ack = true;
            System.out.println( "Acknowledging alarm: " + tag_source );
            return true;
         }
      }
      return false;
   }
}
