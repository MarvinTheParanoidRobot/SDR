import java.util.ArrayList;

// Data feed interface.
public interface DataFeedInterface
{
   boolean Initialize();

   // Obtain data for an individual data sample for a given plot index. 
   boolean GetPlotPoint( int plot_index, DataPoint data_point );

   // Obtain an array of historical data for a given plot index.
   ArrayList<DataPoint> 
     GetHistPlotData( int plot_index, double start_time, double end_time,
                      int max_num_samples );
}
