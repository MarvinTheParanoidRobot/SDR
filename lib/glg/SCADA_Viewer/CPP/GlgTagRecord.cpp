
#include "GlgTagRecord.h"

GlgTagRecord::GlgTagRecord( void )
{
   tag_source = NULL;
}

GlgTagRecord::~GlgTagRecord( void )
{
   GlgFree( (void*) tag_source ); // it checks for NULL automatically
}
