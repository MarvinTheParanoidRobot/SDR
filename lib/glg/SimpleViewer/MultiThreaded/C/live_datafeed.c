#include "GlgViewer.h"

/* This module includes functions to perform data acquisition for
   live application data. Provide custom application code for these
   functions as needed.
*/

/*--------------------------------------------------------------------
| Get data from an application data source and send them to GUI via
| the provided sender socket.
*/
void ProcessRealData( void * sender_socket )
{
   /* Add code here to query application data and sned them to the
      provided sender socket. See ProcessDemoData() code in demo_datafeed.c
      for an example.
   */
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
