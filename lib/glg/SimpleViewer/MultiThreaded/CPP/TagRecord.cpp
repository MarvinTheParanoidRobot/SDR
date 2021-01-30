#include "TagRecord.h"

TagRecord::TagRecord( void )
{
   tag_source = NULL;
   if_changed = GlgFalse;
}

TagRecord::~TagRecord( void )
{ 
   GlgFree( tag_source ); 
   tag_source = NULL;
}

TagRecord& TagRecord::operator= ( const TagRecord& record )
{
   tag_source = GlgStrClone( record.tag_source );
   data_type = record.data_type;
   tag_obj = record.tag_obj;        // GlgObjectC instance
   if_changed = record.if_changed;

   return *this;
}
