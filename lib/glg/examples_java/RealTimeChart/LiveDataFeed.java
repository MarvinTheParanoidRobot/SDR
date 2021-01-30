import java.util.ArrayList;

/////////////////////////////////////////////////////////////////////// 
// Application should provide custom implemnetation of LiveDataFeed.
/////////////////////////////////////////////////////////////////////// 

public class LiveDataFeed implements DataFeedInterface
{
   GlgChart glg_chart;
   static boolean IsInitialized = false;

   /////////////////////////////////////////////////////////////////////// 
   // Constructor.
   /////////////////////////////////////////////////////////////////////// 
   LiveDataFeed( GlgChart chart ) 
   {
      glg_chart = chart;

      // Initialize data feed. Generate an error on failure and quit.
      if( !Initialize() )
        glg_chart.error( "DataFeed initialization failed.", true );    
   }
   
   /////////////////////////////////////////////////////////////////////// 
   // Initiaze DataFeed. 
   /////////////////////////////////////////////////////////////////////// 
   public boolean Initialize()
   {
      if( IsInitialized )
        return true;    // Data feed has been already initialized.

      /* Place custom code here to initialize data feed.
         Return false on failure. 
      */

      LiveDataFeed.IsInitialized = true;

      return true;
   }

   ///////////////////////////////////////////////////////////////////////
   // Query data for an individual data sample for a given plot_index. 
   ///////////////////////////////////////////////////////////////////////
   public boolean GetPlotPoint( int plot_index, DataPoint data_point )
   {
      /* Provide custom code to fill in data_point fields. 
         If failed, return false.
      */

      data_point.value = 0.;
      data_point.value_valid = true;
      
      if( glg_chart.SUPPLY_TIME_STAMP )
        // Place custom code here to supply time stamp.
        data_point.time_stamp = glg_chart.GetCurrTime(); 
      else
        // Chart will automatically supply time stamp using current time.
        data_point.time_stamp = 0.;  

      return true;
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Query historical chart data for a provided time interval for a given
   // plot index. 
   /////////////////////////////////////////////////////////////////////// 
   public ArrayList<DataPoint> 
     GetHistPlotData( int plot_index, double start_time, double end_time,
                      int max_num_samples )
   {
      if( max_num_samples < 1 )
        max_num_samples = 1;
      int num_samples = max_num_samples;

      ArrayList<DataPoint> data_array = new ArrayList<DataPoint>();

      for( int i=0; i<num_samples; ++i )
      {
         DataPoint data_point = new DataPoint();

         /* Provide custom code to fill in data_point fields.
         data_point.value = 
         data_point.time_stamp = 
         data_point.value_valid = true;
         */

         data_array.add( data_point );
      }

      return data_array;
   }
}

