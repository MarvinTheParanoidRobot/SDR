#include "LiveDataFeed.h"
#include "GlgChart.h"

static GlgBoolean IsInitialized = GlgFalse;

/////////////////////////////////////////////////////////////////////// 
// Application should provide custom implemnetation of LiveDataFeed.
/////////////////////////////////////////////////////////////////////// 

LiveDataFeed::LiveDataFeed( GlgChart * chart ) : DataFeedBase( chart )
{  
   // Initialize data feed. Generate an error on failure and quit.
   if( !Initialize() )
     glg_chart->error( "DataFeed initialization failed.", GlgTrue );
}

LiveDataFeed::~LiveDataFeed( void )
{
}

/*----------------------------------------------------------------------
| Initialize data feed as needed. An application should provide
| a custom implementation of this method.
*/
GlgBoolean LiveDataFeed::Initialize( void )
{
   if( IsInitialized )
     return GlgTrue;    // Data feed has been already initialized.

   /* Place custom code here to initialize data feed.
      If initialization failed, return GlgFalse.
   */

   IsInitialized = GlgTrue;
   
   return GlgTrue;
}

/*----------------------------------------------------------------------
| Obtain one datasample for a given plot index.
*/
GlgBoolean LiveDataFeed::GetPlotPoint( GlgLong plot_index, 
                                       DataPoint& data_point )
{
   /* Provide custom code to fill in data_point fields. 
      If data query failed, return GlgFalse.
   */
   
   data_point.value = 0.;
   data_point.value_valid = true;
   
   if( glg_chart->SupplyTimeStamp )
     // Place custom code here to supply time stamp.
     data_point.time_stamp = glg_chart->GetCurrTime();
   else
     // Chart will automatically supply time stamp using current time.
     data_point.time_stamp = 0.;  
   
   return GlgTrue;
}

/*----------------------------------------------------------------------
| Obtain a list of plot data points, used to prefill a GLG chart with
| historical data. If failed, return GlgFalse.
*/
GlgBoolean LiveDataFeed::GetHistPlotData( GlgLong plot_index, 
                                          double start_time, double end_time,
                                          GlgLong max_num_samples,
                                          DataArrayType& data_array )
{
   if( max_num_samples < 1 )
     max_num_samples = 1;
   int num_samples = max_num_samples;
   
   for( int i=0; i<num_samples; ++i )
   {
      DataPoint * data_point = new DataPoint();
      
      /* Provide custom code to fill in data_point fields.
         data_point->value = 
         data_point->time_stamp = 
         data_point->value_valid = true;
      */
      
      data_array.push_back( data_point );
   }

   return GlgTrue;
}

/*----------------------------------------------------------------------
| Clear data_array -- free memory allocated for the data_array and its 
| elements.
*/
void LiveDataFeed::FreePlotData( DataArrayType& data_array )
{
   DataFeedBase::FreePlotData( data_array );
}


