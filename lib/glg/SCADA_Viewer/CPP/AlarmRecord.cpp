
#include "AlarmRecord.h"

// Constructor
AlarmRecord::AlarmRecord( void )
{
   tag_source = NULL;
   description = NULL;
   string_value = NULL;
   ack = GlgFalse;

   double_value = -1.0; 
   status = -1;
   time = 0.;
   age = 0;
}

// Constructor
AlarmRecord::AlarmRecord( double _time, SCONST char * _tag_source, 
                          SCONST char * _description, SCONST char * _s_value,
                          double _d_value, int _status, GlgBoolean _ack )
{
   time = _time;
   tag_source = GlgStrClone( _tag_source );
   description = GlgStrClone( _description );
   string_value = GlgStrClone( _s_value );
   double_value = _d_value;
   status = _status;
   ack = _ack;
   age = 0;
}

// Destructor
AlarmRecord::~AlarmRecord( void )
{
   GlgFree( (void*) tag_source ); // it checks for NULL automatically
   GlgFree( (void*) description );
   GlgFree( (void*) string_value );
}

