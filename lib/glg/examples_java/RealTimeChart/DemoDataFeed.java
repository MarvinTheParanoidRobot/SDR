import java.util.ArrayList;

/////////////////////////////////////////////////////////////////////// 
// Sample implementation of DataFeedInterface, provides simulated chart
// data. In a real application, data will be coming from LiveDataFeed,
// a custom implementation of DataFeedInterface.
/////////////////////////////////////////////////////////////////////// 

public class DemoDataFeed implements DataFeedInterface
{
   GlgChart glg_chart;
   static boolean IsInitialized = false;

   /* Stores low/high data range for each plot. Used to generate simulated
      data in GetDemoValue().
   */
   double [][] LowHigh = null;
   long counter = 0;

   /////////////////////////////////////////////////////////////////////// 
   // Constructor.
   /////////////////////////////////////////////////////////////////////// 
   DemoDataFeed( GlgChart chart ) 
   {
      glg_chart = chart;

      // Initialize data feed. Generate an error on failure and quit.
      if( !Initialize() )
        glg_chart.error( "DataFeed initialization failed.", true );
   }
   
   /////////////////////////////////////////////////////////////////////// 
   // Initiaze DataFeed. Application will provide custom code for 
   // data feed initialization.
   /////////////////////////////////////////////////////////////////////// 
   public boolean Initialize()
   {
      if( IsInitialized )
        return true;    // Data feed has been already initialized.
      
      DemoDataFeed.IsInitialized = true;
      return true; 
   }

   ///////////////////////////////////////////////////////////////////////
   // Obtain data for an individual data sample for a given
   // plot_index. Uses simulated data for demo.
   ///////////////////////////////////////////////////////////////////////
   public boolean GetPlotPoint( int plot_index, DataPoint data_point )
   {
      data_point.value = GetDemoValue( plot_index );
      data_point.value_valid = true;
      
      if( glg_chart.SUPPLY_TIME_STAMP )
        // Supply time stamp explicitly.
        data_point.time_stamp = glg_chart.GetCurrTime();
      else
        // Chart will automatically supply time stamp using current time.
        data_point.time_stamp = 0.;  

      return true;
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Obtain historical chart data for a provided time interval for a given
   // plot index. For demo, return the requested max number of points.
   /////////////////////////////////////////////////////////////////////// 
   public ArrayList<DataPoint> 
     GetHistPlotData( int plot_index, double start_time, double end_time,
                      int max_num_samples )
   {
      if( max_num_samples < 1 )
        max_num_samples = 1;
      int num_samples = max_num_samples;
      
      double interval = ( end_time - start_time ) / max_num_samples;

      ArrayList<DataPoint> data_array = new ArrayList<DataPoint>();

      for( int i=0; i<num_samples; ++i )
      {
         /* Generate demo data. */
         DataPoint data_point = new DataPoint();

         data_point.value = GetDemoValue( plot_index );
         data_point.time_stamp = start_time + interval * i;
         data_point.value_valid = true;
         data_array.add( data_point );
      }

      return data_array;
   }
   
   /////////////////////////////////////////////////////////////////////// 
   // Generates demo data value using low/high data range of a plot
   // with a specified plot_index.
   ///////////////////////////////////////////////////////////////////////
   double GetDemoValue( int plot_index )
   {
      double 
        half_amplitude, center,
        period,
        value,
        alpha;

      // Store low/high ranges for the first time.
      if( LowHigh == null )
        StorePlotRanges();

      double low = LowHigh[ plot_index ][ 0 ];
      double high = LowHigh[ plot_index ][ 1 ];

      half_amplitude = ( high - low ) / 2.;
      center = low + half_amplitude;
      
      period = 50. * ( 1. + plot_index * 2. );
      alpha = 2. * Math.PI * counter / period;
      
      value = center + 
        half_amplitude * Math.sin( alpha ) * Math.sin( alpha / 30. );
      
      ++counter;
      return value;
   }

   /////////////////////////////////////////////////////////////////////// 
   // Store low/high ranges for each plot. Used to generate simulated data.
   /////////////////////////////////////////////////////////////////////// 
   public void StorePlotRanges()
   {
      LowHigh = new double[ glg_chart.NumPlots ][ 2 ];

      for( int i=0; i < glg_chart.NumPlots; ++i )
      {
         LowHigh[ i ][ 0 ] = glg_chart.Plots[ i ].GetDResource( "YLow" );
         LowHigh[ i ][ 1 ] = glg_chart.Plots[ i ].GetDResource( "YHigh" );
      }
   }
}

