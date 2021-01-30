
/* Base class containing methods for data acquisition. An application
   can extend DataFeedBase class and provide a custom implementation
   of these methods.
*/

#include "DataFeedBase.h"

// Constructor
DataFeedBase::DataFeedBase( GlgChart * chart )
{
   glg_chart = chart;
}

// Destructor
DataFeedBase::~DataFeedBase( void )
{
}

/*----------------------------------------------------------------------
| Initialize data feed.
*/
GlgBoolean DataFeedBase::Initialize( void ) 
{
   return GlgFalse;
}

// Get datasample for a given plot index. 
GlgBoolean DataFeedBase::GetPlotPoint( GlgLong plot_index, 
                                       DataPoint& data_point )
{
   return GlgFalse;
}

/*----------------------------------------------------------------------
| Obtain a list of plot data points, used to prefill a GLG chart with
| historical data, and store it in the data_array.
*/
GlgBoolean DataFeedBase::GetHistPlotData( GlgLong plot_index, 
                                          double start_time, double end_time,
                                          GlgLong max_num_samples,
                                          DataArrayType& data_array )
{
   return GlgFalse;
}
  
/*----------------------------------------------------------------------
| Free data_array containing plot points.
*/
void DataFeedBase::FreePlotData( DataArrayType& data_array )
{
   if( data_array.empty() )
     return;
   
   // Purge the content of vector elements.
   DataArrayType::iterator it;
   for( it = data_array.begin(); it != data_array.end(); ++it )
     delete *it;
   
   data_array.clear();
}
