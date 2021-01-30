#pragma once

#include <stdio.h>

class PlotDataPoint
{
public:
   PlotDataPoint( void );
   PlotDataPoint( double _value, double _time_stamp, 
                  unsigned char _value_valid );
   
   ~PlotDataPoint( void ) {}
public:
   double value;
   double time_stamp;
   unsigned char value_valid;
};
