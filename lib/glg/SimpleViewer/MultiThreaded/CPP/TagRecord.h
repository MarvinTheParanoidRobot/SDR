#pragma once

#include "viewer.h"

class TagRecord
{
 public:
   // Default constructor
   TagRecord( void );

   // Destructor
   ~TagRecord( void );

 public:
   GlgLong data_type;
   GlgObjectC tag_obj;
   char * tag_source;

   /* May be used to further optimize performance. */
   GlgBoolean if_changed;

   TagRecord& operator= ( const TagRecord& record );
};

// A vector of pointers to tag data records.
typedef vector<TagRecord*> TagRecordsType;
