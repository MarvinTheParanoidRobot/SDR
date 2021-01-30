#pragma once

#include "scada.h"

class GlgTagRecord;
class AlarmRecord;
class PlotDataPoint;
class GlgSCADAViewer;

class DataFeedBase
{
 public:
   DataFeedBase( GlgSCADAViewer * viewer );
   ~DataFeedBase( void );

 public:
   GlgSCADAViewer * Viewer;

 public:
   virtual GlgBoolean WriteDTag( SCONST char * tag_source, double value );
   virtual GlgBoolean WriteSTag( SCONST char * tag_source, SCONST char * str );
   virtual GlgBoolean ReadDTag( GlgTagRecord * tag_record, 
                                double * d_value, double * time_stamp );
   virtual GlgBoolean ReadSTag( GlgTagRecord * tag_record, 
                                SCONST char ** s_value );
   virtual GlgBoolean GetAlarms( AlarmRecordArrayType& alarm_list );
   virtual GlgBoolean GetPlotData( SCONST char * tag_source, 
                                   double start_time, double end_time,
                                   int max_num_samples,
                                   PlotDataArrayType& data_array );
   virtual void FreePlotData( PlotDataArrayType& data_array );
   virtual void FreeAlarms( AlarmRecordArrayType& alarm_list );

   // Acknowledge alarm associated with the tag source.
   virtual GlgBoolean ACKAlarm( SCONST char * tag_source );
};
