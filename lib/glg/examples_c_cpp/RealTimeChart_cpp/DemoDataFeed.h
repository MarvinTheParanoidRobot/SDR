#pragma once

#include "DataFeedBase.h"

class DemoDataFeed : public DataFeedBase
{
 public:
   DemoDataFeed( GlgChart * chart );
   ~DemoDataFeed( void );

   /* Stores low/high data range for each plot. Used to generate simulated
      data in GetDemoValue().
   */
   double * Low;
   double * High;

   // Used in GetDemoValue() to generate demo data.
   GlgLong counter;

   // Virtual methods to supply custom data feed interface.
   GlgBoolean Initialize( void );
   GlgBoolean GetPlotPoint( GlgLong plot_index, DataPoint& data_point );
   GlgBoolean GetHistPlotData( GlgLong plot_index, 
                               double start_time, double end_time,
                               GlgLong max_num_samples, 
                               DataArrayType& data_array );
   void FreePlotData( DataArrayType& data_array );
   void StorePlotRanges( void );
   double GetDemoValue( GlgLong plot_index );
}; 
