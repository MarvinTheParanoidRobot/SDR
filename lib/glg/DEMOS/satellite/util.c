#include <stdio.h>
#include "GlgApi.h"
#include "util.h"

#define DEBUG_TIMER          0   /* Set to 1 to debug timer intervals */

/*----------------------------------------------------------------------
| 
*/
void SetPolygonPoint( GlgObject polygon, GlgLong point_index,
                      double lon, double lat, double elev )
{
   GlgObject point;

   point = GlgGetElement( polygon, point_index );
   GlgSetGResource( point, NULL, lon, lat, elev );
}

/*----------------------------------------------------------------------
| 
*/
void AddPolygonPoint( GlgObject polygon, double lon, double lat, double elev,
                      GlgBoolean add_at_the_end )
{
   GlgObject point;

   /* Copy the first point and add it at the end of the polygon. */
   point = GlgGetElement( polygon, 0 );
   point = GlgCopyObject( point );

   GlgSetGResource( point, NULL, lon, lat, elev );
   if( add_at_the_end )
     GlgAddObjectToBottom( polygon, point );
   else                /* add at the beginning of the polygon */
     GlgAddObjectToTop( polygon, point );

   /* Dereference the copy to let polygon manage it. */
   GlgDropObject( point );
}

/*----------------------------------------------------------------------
| 
*/
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, 
			   GlgLong interval )
{
   GlgULong sec2, microsec2;
   GlgLong elapsed_time, adj_interval;

   GlgGetTime( &sec2, &microsec2 );  /* End time */
   
   /* Elapsed time in millisec */
   elapsed_time = 
     ( sec2 - sec1 ) * 1000 + (long) ( microsec2 - microsec1 ) / 1000;

   /* Maintain constant update interval regardless of the system speed. */
   if( elapsed_time + 10 >= interval )
      /* Slow system: update as fast as we can, but allow a small interval 
         for handling input events. */
     adj_interval = 10;
   else
     /* Fast system: keep constant update interval. */
     adj_interval = interval - elapsed_time;

#if DEBUG_TIMER
   printf( "sec= %ld, msec= %ld\n", 
           (long)( sec2 - sec1 ), (long)( microsec2 - microsec1 ) );
   printf( "*** elapsed= %ld, requested= %ld, adjusted= %ld\n",
           (long) elapsed_time, (long) interval, (long) adj_interval );
#endif

   return adj_interval;
}
