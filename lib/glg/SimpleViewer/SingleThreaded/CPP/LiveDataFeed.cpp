#include "stdafx.h"
#include "LiveDataFeed.h"

#ifdef _WINDOWS
# pragma warning( disable : 4996 )    /* Allow cross-platform strcpy */
#endif

/*--------------------------------------------------------------------
| LiveDataFeed class. The application should provide custom implementation
| for the Read and Write methods to communicate with real-time data
| acquisition system.
*/
LiveDataFeed::LiveDataFeed( GlgViewer * viewer )
{
   Viewer = viewer;
}

LiveDataFeed::~LiveDataFeed()
{
}

/*--------------------------------------------------------------------
| Read real-time data value of D-type.
*/
bool LiveDataFeed::ReadDValue( TagRecordC * tag_record, double * d_value )
{
   /* Place your code here to query the value of the tag specified by 
      tag_record->tag_source.
   */
   *d_value = 0.;

   return True;
}

#define TEXT_BUFFER_LENGTH 1024

/*--------------------------------------------------------------------
| Read real-time data value of S-type.
*/
bool LiveDataFeed::ReadSValue( TagRecordC * tag_record, CONST char ** s_value )
{
   if( !s_value )
     return False;
   
   static char buffer[ TEXT_BUFFER_LENGTH ];
   
   /* Place your code here to query the value of the tag specified by 
      tag_record->tag_source.
      Place the returned value into a static buffer, making sure to check 
      the length of the string against TEXT_BUFFER_LENGTH to avoid 
      buffer overruns. This avoids requiring the caller to free the returned
      string.
   */
   strcpy( buffer, "unset" );  // Replace with custom code.
   *s_value = buffer;
   return True;
}

/*--------------------------------------------------------------------
| Simulte the "Write" opearion for a D value. For demo purposes, 
| set the value of the specified tag source in the drawing. 
*/
bool LiveDataFeed::WriteDValue( CONST char * tag_source, double d_value )
{
   /* Set new tag value of D-type. */
   return True;
}


/*--------------------------------------------------------------------
| Simulte the "Write" opearion for S value. For demo purposes, set the value
| of the specified tag source in the drawing. 
*/
bool LiveDataFeed::WriteSValue( CONST char * tag_source, CONST char * s_value )
{
   /* Set new tag value of S-type. */
   return True;
}

