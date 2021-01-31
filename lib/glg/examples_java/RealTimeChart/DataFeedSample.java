
///////////////////////////////////////////////////////////////////////      
// Sample of a custom data feed class that implements DataFeedInterface.
// The class should implement GetPlotPoint() method that generates 
// custom data for each data point to be plotted in a chart.
// If the chart should be prefilled with historical data before
// it is drawn and starts receiving new data, the class should 
// implement  FillHistData method.
///////////////////////////////////////////////////////////////////////      
public class DataFeedSample implements DataFeedInterface
{
   GlgChart glg_chart;
   
   DataFeedSample( GlgChart chart ) 
   {
      glg_chart = chart;
   }
   
   /////////////////////////////////////////////////////////////////////// 
   // Invoked for each data point to be plotted in a chart.
   // plot_index indicates a zero based plot index for which
   // the data is being plotted.
   /////////////////////////////////////////////////////////////////////// 
   public void GetPlotPoint( int plot_index, DataPoint data_point )
   {
      // data_point.value = 
      // data_point.value_valid = 
      
      // data_point.has_time_stamp = //set to true or false
      // data_point.time_stamp =
   }
   
   /////////////////////////////////////////////////////////////////////// 
   // Pre-fills the chart's history buffer.
   /////////////////////////////////////////////////////////////////////// 
   public void FillHistData( int plot_index, double start_time, 
                             double end_time, DataPoint data_point )
   {
       
      // Define delta in sec. as needed.
      double dt = 1.;
      for( double time_stamp = start_time; 
           time_stamp < ( end_time != 0. ? end_time : glg_chart.GetCurrTime() );
           time_stamp += dt )
      {
         //data_point.value = ..
         //data_point.value_valid = ..
         //data_point.time_stamp = time_stamp;
         //data_point.has_time_stamp = true;
         
         glg_chart.PushPlotPoint( plot_index, data_point );
      }
   
   }
}
   
