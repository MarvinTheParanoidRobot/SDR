#ifndef _DataTypes_h_
#define _DataTypes_h_

#include "GlgApi.h"

typedef struct _TagRecord 
{
   GlgDataType data_type;
   char * tag_source;
   GlgObject tag_obj;

   /* TimeEntryPoint for a plot in a RealTimeChart, if any.
      This object is valid if SUPPLY_PLOT_TIME_STAMP flag is true, and
      the drawing contains a chart where a plot's ValueEntryPoint 
      has a tag with TagSource=tag_source.
   */
   GlgObject plot_time_ep;   
} TagRecord;

typedef struct _AlarmRecord
{
   double time;    /* Epoch time in seconds. */
   char * tag_source;
   char * description;

   /* If string_value is set to null, double_value will be displayed as alarm 
      value; otherwise string_value will be displayed.
   */
   char * string_value;
   double double_value;

   int status;
   GlgBoolean ack;
   int age;       /* Used for demo alarm simulation only. */

} AlarmRecord;

typedef struct _PlotDataPoint
{
   double value;
   double time_stamp;
   unsigned char value_valid;

} PlotDataPoint;


#endif
