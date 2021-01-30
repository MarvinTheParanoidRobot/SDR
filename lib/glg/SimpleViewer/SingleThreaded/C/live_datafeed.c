#include "GlgViewer.h"

#ifdef _WINDOWS
# pragma warning( disable : 4996 )
#endif

/* This module includes functions to perform data acquisition for
   live application data. Provide custom application code for these
   functions as needed.
*/

#define TEXT_BUFFER_LENGTH    1024

/*----------------------------------------------------------------------
| Queries new D tag value from the database.
*/
GlgBoolean ReadDValue( TagRecord * tag_record, double * d_value )
{ 
   /* Place your code here to query the value of the tag specified by 
      tag_record->tag_source.
   */
   *d_value = 0.;
   return GlgTrue;
}

/*----------------------------------------------------------------------
| Queries new S tag value from the database.
*/
GlgBoolean ReadSValue( TagRecord * tag_record, char ** s_value )
{   
   static char buffer[ TEXT_BUFFER_LENGTH ];
   
   /* Place your code here to query the value of the tag specified by 
      tag_record->tag_source.
      Place the returned value into a static buffer, making sure to check 
      the length of the string against TEXT_BUFFER_LENGTH to avoid 
      buffer overruns. This avoids requiring the caller to free the returned
      string.
   */

   strcpy( buffer, "unset" );
   *s_value = buffer;
   return GlgTrue;
}

/*----------------------------------------------------------------------
|
*/
GlgBoolean WriteDValue( char *tag_source, double value )
{
   /* Set new tag value of D-type. */

   return GlgTrue;
}

/*----------------------------------------------------------------------
|
*/
GlgBoolean WriteSValue( char *tag_source, char * str )
{
   /* Set new tag value of S-type. */

   return GlgTrue;
}


