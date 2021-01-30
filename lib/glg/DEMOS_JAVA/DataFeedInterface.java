import com.genlogic.*;
import java.util.ArrayList;

// Data feed interface.
public interface DataFeedInterface
{
   // Query new D (double) tag value from the database.
   boolean ReadDTag( GlgTagRecord tag_record, DataPoint data_point );

   // Query new S (string) tag value from the database.
   boolean ReadSTag( GlgTagRecord tag_record, DataPoint data_point );

   // Write new numerical value into the provided database tag. 
   boolean WriteDTag( String tag_source, DataPoint data_point );
 
   // Write new string value into the provided database tag. 
   boolean WriteSTag( String tag_source, DataPoint data_point );

   // Obtain an array of historical data for a given tag source.
   ArrayList<PlotDataPoint> 
     GetPlotData( String tag_source, double start_time, double end_time,
                  int max_num_samples );

   // Obtain a list of process alarms.
   ArrayList<AlarmRecord> GetAlarms();

   // Acknowledge alarm associated with the tag source.
   boolean ACKAlarm( String tag_source );
}
