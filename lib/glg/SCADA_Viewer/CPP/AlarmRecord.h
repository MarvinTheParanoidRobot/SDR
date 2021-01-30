#pragma once

#include "scada.h"

class AlarmRecord
{
 public:
   AlarmRecord( void );
   AlarmRecord( double _time, SCONST char * _tag_source, 
                SCONST char * _description, SCONST char * _s_value,
                double _d_value, int _status, GlgBoolean _sck );

   ~AlarmRecord( void );
   
 public:
   double time;    // Epoch time in seconds.
   SCONST char * tag_source;
   SCONST char * description;

   /* If string_value is set to null, double_value will be displayed as alarm 
      value; otherwise string_value will be displayed.
   */
   SCONST char * string_value;
   double double_value;

   int status;
   GlgBoolean ack;
   int age;       // Used for demo alarm simulation only.
};
