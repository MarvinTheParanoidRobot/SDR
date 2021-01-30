#pragma once

#include "TagRecord.h"

// Base Data feed class, provides virtual methods.
class DataFeedC
{
 public:
   DataFeedC( void ) {};
   ~DataFeedC() {};

   // Virtual methods.
   virtual bool ReadDValue( TagRecordC * tag_record, double *d_value )
   {
      return True;
   }

   virtual bool ReadSValue( TagRecordC * tag_record, char ** s_value )
   {
      return True;
   }

   virtual bool WriteDValue( CONST char *tag_source, double value )
   {
      return True;
   }

   virtual bool WriteSValue( CONST char *tag_source, CONST char * str )
   {
      return True;
   }
};
