#pragma once

#include "GlgClass.h"

class TagRecordC
{
public:
   TagRecordC( void );
   ~TagRecordC( void );
public:
   GlgObjectC tag_obj;
   int data_type;
   char * tag_source;

   /* May be used to further optimize performance. */
   bool if_changed;
};
