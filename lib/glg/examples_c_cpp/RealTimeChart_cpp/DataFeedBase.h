#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>

#include "GlgClass.h"
#include "DataPoint.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

class GlgChart;

// An array of pointers to plot data points.
typedef std::vector<DataPoint*> DataArrayType;

// Base Data feed class, provides virtual methods.
class DataFeedBase
{
 public:
   DataFeedBase( GlgChart * chart );
   ~DataFeedBase();

 public:
   GlgChart * glg_chart;

 public:
   virtual GlgBoolean Initialize( void );

   // Obtain one datasample for a given plot index.
   virtual GlgBoolean GetPlotPoint( GlgLong plot_index, DataPoint& data_point );

   // Obtain an array of historical data for a given plot index.
   virtual GlgBoolean GetHistPlotData( GlgLong plot_index, 
                                       double start_time, double end_time,
                                       GlgLong max_num_samples, 
                                       DataArrayType& data_array );

   virtual void FreePlotData( DataArrayType& data_array );
};
