#pragma once

#include "scada.h"

class GlgTagRecord
{
 public:
   GlgTagRecord( void );
   ~GlgTagRecord( void );

 public:
   int data_type;
   SCONST char * tag_source;
   GlgObjectC tag_obj;

   /* TimeEntryPoint for a plot in a RealTimeChart, if any.
      This object is valid if SUPPLY_PLOT_TIME_STAMP flag is true, and
      the drawing contains a chart where a plot's ValueEntryPoint has 
      a tag with TagSource=tag_source.
   */
   GlgObjectC plot_time_ep;   
};
