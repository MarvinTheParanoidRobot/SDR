import com.genlogic.*;
import java.util.ArrayList;

//////////////////////////////////////////////////////////////////////////
// Provide custom code to read and write real-time data values.
//////////////////////////////////////////////////////////////////////////

public class LiveDataFeed implements DataFeedInterface
{
   GlgSCADAViewer Viewer;

   public LiveDataFeed( GlgSCADAViewer viewer ) 
   {
      Viewer = viewer;

      // Place custom code here to initialize data feed.
      //
      // InitDataBase();
   }

   /////////////////////////////////////////////////////////////////////// 
   public boolean ReadDTag( GlgTagRecord tag_record, DataPoint data_point )
   {
      // Place code here to query and return double tag value.
      // The name of the tag is provided in tag_record.tag_source.

      data_point.d_value = 0.0;
      data_point.time_stamp = Viewer.GetCurrTime();
      return true;
   }

   /////////////////////////////////////////////////////////////////////// 
   public boolean ReadSTag( GlgTagRecord tag_record, DataPoint data_point )
   {
      // Place code here to query and return string tag value.
      // The name of the tag is provided in tag_record.tag_source.

      data_point.s_value = "no value";
      data_point.time_stamp = Viewer.GetCurrTime();
      return true;
   }

   /////////////////////////////////////////////////////////////////////// 
   public boolean WriteDTag( String tag_source, DataPoint data_point )
   {
      // Place code here to write double tag value.
      // The tag value is provided in data_point.d_value.
      
      return true;
   }

   /////////////////////////////////////////////////////////////////////// 
   public boolean WriteSTag( String tag_source, DataPoint data_point )
   {
      // Place code here to write double tag value.
      // The tag value is provided in data_point.s_value.
      
      return true;
   }

   //////////////////////////////////////////////////////////////////////////
   // Obtain an array of historical data for a given tag source.
   //////////////////////////////////////////////////////////////////////////
   public ArrayList<PlotDataPoint>
     GetPlotData( String tag_source, double start_time, double end_time,
                  int max_num_samples )
   {
      /* Place code here to query historical data for the plot specified by
         the provided tag_source.
         See the GetPlotData() method of DemoDataFeed for an example of 
         creating an array of plot data points.
      */
      return null;
   }

   //////////////////////////////////////////////////////////////////////////
   // Queries alarm data from the controlled process.
   // 
   //////////////////////////////////////////////////////////////////////////
   public ArrayList<AlarmRecord> GetAlarms()
   {
      /* Place code here to query a list of alarms.
         See the GetAlarms() method of DemoDataFeed for an example of 
         creating an array of alarm records.
      */
      return null;
   }

   //////////////////////////////////////////////////////////////////////////
   // Send alarm acknowledgement for the alarm associated with the specified 
   // tag.
   //////////////////////////////////////////////////////////////////////////
   public boolean ACKAlarm( String tag_source )
   {
      // Place code here to send alarm ACK for the specified tag.
      return true;
   }
}
