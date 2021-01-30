#include "stdafx.h"
#include "TagRecord.h"

TagRecordC::TagRecordC( void )
{
   tag_source = NULL;
}

TagRecordC::~TagRecordC( void )
{
   GlgFree( tag_source ); // it checks for NULL automatically
}
