#pragma once

class GlgChart;

class DataPoint
{
 public:
   double value;
   bool value_valid;
   bool has_time_stamp;
   double time_stamp;
		    
 public:
   DataPoint( void ) {};
   ~DataPoint( void ) {};
};

// Base Data feed class, provides virtual methods.
class DataFeedC
{
 public:
   DataFeedC() {};
   ~DataFeedC() {};

   virtual void GetPlotPoint( int plot_index, DataPoint& data_point ) {};

   // This method may be used to pre-fill data in the real-time mode.
   // It may do nothing if pre-filling data is not needed.
   virtual void FillHistData( int plot_index, 
			     double start_time, double end_time, 
			      DataPoint& data_point ) {};
};
