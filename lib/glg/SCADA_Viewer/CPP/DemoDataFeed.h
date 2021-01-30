#pragma once

#include "DataFeedBase.h"

class DemoDataFeed : public DataFeedBase
{
 public: // Constructor/Destructor
   DemoDataFeed ( GlgSCADAViewer * viewer );
   ~DemoDataFeed( void );

   // Keeps a list of active alarms for simulation.
   AlarmRecordArrayType ActiveAlarmList;

 public:
   /* Methods defined as virtual in the base class DataFeedBase and 
      are expected to be overriden in a derived class.
   */
   GlgBoolean WriteDTag( SCONST char * tag_source, double value );
   GlgBoolean WriteSTag( SCONST char * tag_source, SCONST char * str );
   GlgBoolean ReadDTag( GlgTagRecord * tag_record, double * d_value,
                        double * time_stamp );
   GlgBoolean ReadSTag( GlgTagRecord * tag_record, SCONST char ** s_value  );
   GlgBoolean GetAlarms( AlarmRecordArrayType& );
   GlgBoolean GetPlotData( SCONST char * tag_source, 
                           double start_time, double end_time,
                           int max_num_samples,
                           PlotDataArrayType& data_array );
   void FreePlotData( PlotDataArrayType& data_array );
   void FreeAlarms( AlarmRecordArrayType& alarm_list );
   
   // Acknowledge alarm associated with the tag source.
   GlgBoolean ACKAlarm( SCONST char * tag_source );

 public:
   // Methods specific to the DemoDataFeed class.
   AlarmRecord * GetAlarmData( void );
   void AgeAlarms( AlarmRecordArrayType& alarm_list );   
   GlgBoolean GetDemoValue( SCONST char * tag_source, 
                            double * out_value, 
                            GlgBoolean historical_mode );
   GlgBoolean GetDemoString( SCONST char * tag_source, 
                             SCONST char ** our_value );
   SCONST char * GetTimeString( void );
};
