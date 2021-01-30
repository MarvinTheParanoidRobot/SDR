#include "PlotDataPoint.h"

PlotDataPoint::PlotDataPoint( void ) {}

PlotDataPoint::PlotDataPoint( double _value, double _time_stamp, 
                              unsigned char _value_valid ) 
{
   value = _value;
   time_stamp = _time_stamp;
   value_valid = _value_valid;
}
