#pragma once

#include "DataFeedBase.h"

class LiveDataFeed : public DataFeedBase
{
 public:
   LiveDataFeed( GlgChart * chart );
   ~LiveDataFeed( void );

   // Virtual methods to supply custom data feed interface.
   GlgBoolean Initialize( void );
   GlgBoolean GetPlotPoint( GlgLong plot_index, DataPoint& data_point );
   GlgBoolean GetHistPlotData( GlgLong plot_index, 
                               double start_time, double end_time,
                               GlgLong max_num_samples, 
                               DataArrayType& data_array );
   void FreePlotData( DataArrayType& data_array );
}; 
