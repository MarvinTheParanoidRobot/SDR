#include "DemoDataFeed.h"
#include "GlgChart.h"

static GlgBoolean IsInitialized = GlgFalse;

DemoDataFeed::DemoDataFeed( GlgChart * chart ) : DataFeedBase( chart )
{  
   Low = NULL;
   High = NULL;
   counter = 0;

   // Initialize data feed. Generate an error on failure and quit.
   if( !Initialize() )
     glg_chart->error( "DataFeed initialization failed.", GlgTrue );
}

DemoDataFeed::~DemoDataFeed( void )
{
   delete Low;
   delete High;
}

/*----------------------------------------------------------------------
| Initialize data feed as needed. An application should provide
| a custom implementation of this method in LiveDataFeed.
*/
GlgBoolean DemoDataFeed::Initialize( void )
{
   if( IsInitialized )
     return GlgTrue;    // Data feed has been already initialized.
   
   IsInitialized = GlgTrue;
   return GlgTrue;
}

/*----------------------------------------------------------------------
| Obtain one datasample for a given plot index.
*/
GlgBoolean DemoDataFeed::GetPlotPoint( GlgLong plot_index, 
                                       DataPoint& data_point )
{
   data_point.value = GetDemoValue( plot_index );
   data_point.value_valid = true;
   
   if( glg_chart->SupplyTimeStamp )
     // Supply time stamp explicitly.
     data_point.time_stamp = glg_chart->GetCurrTime();
   else
     // Chart will automatically supply time stamp using current time.
     data_point.time_stamp = 0.;  
   
   return GlgTrue;
}

/*----------------------------------------------------------------------
| Obtain a list of plot data points, used to prefill a GLG chart with
| historical data. For demo, return the requested max number of points.
*/
GlgBoolean DemoDataFeed::GetHistPlotData( GlgLong plot_index, 
                                          double start_time, double end_time,
                                          GlgLong max_num_samples,
                                          DataArrayType& data_array )
{
   if( max_num_samples < 1 )
     max_num_samples = 1;
   GlgLong num_samples = max_num_samples;
   
   double interval = ( end_time - start_time ) / max_num_samples;

   for( int i=0; i<num_samples; ++i )
   {
      // Generate demo data.
      DataPoint * data_point = new DataPoint();
      
      data_point->value = GetDemoValue( plot_index );
      data_point->time_stamp = start_time + interval * i;
      data_point->value_valid = true;

      data_array.push_back( data_point );
   }

   return GlgTrue;
}

/*----------------------------------------------------------------------
| Clear data_array -- free memory allocated for the data_array and its 
| elements.
*/
void DemoDataFeed::FreePlotData( DataArrayType& data_array )
{
   DataFeedBase::FreePlotData( data_array );
}

/*----------------------------------------------------------------------*/
double DemoDataFeed::GetDemoValue( GlgLong plot_index )
{
   double 
     value,
     period,
     alpha,
     center,
     half_amplitude;

   // Validate plot_index.
   if( plot_index < 0 || plot_index >= glg_chart->NumPlots )
   {
     glg_chart->error( "Invalid plot_index.", GlgFalse );
     return 0.;
   }

   // Store low/high ranges for the first time.
   if( !Low && !High )
     StorePlotRanges();
   
   double low = Low[ plot_index ];
   double high = High[ plot_index ];
   
   half_amplitude = ( high - low ) / 2.;
   center = low + half_amplitude;
   
   period = 50. * ( 1. + plot_index * 2. );
   alpha = 2. * M_PI * counter / period;
   
   value = center + 
     half_amplitude * sin( alpha ) * sin( alpha / 30. );
   
   ++counter;
   return value;
}

/*----------------------------------------------------------------------
|  Store low/high ranges for each plot. Used to generate simulated data.
*/
void DemoDataFeed::StorePlotRanges( void )
{
   Low = new double[ glg_chart->NumPlots ];
   High = new double[ glg_chart->NumPlots ];

   for( int i=0; i < glg_chart->NumPlots; ++i )
   {
      glg_chart->Plots[ i ].GetResource( "YLow", &Low[ i ] );
      glg_chart->Plots[ i ].GetResource( "YHigh", &High[ i ] );
   }
}
