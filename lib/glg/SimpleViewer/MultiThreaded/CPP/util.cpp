#include "viewer.h"

/*----------------------------------------------------------------------
| Set viewport size in screen cooridnates. 
*/
void SetSize( GlgObjectC& viewport,
              GlgLong x, GlgLong y, GlgLong width, GlgLong height )
{
   if( viewport.IsNull() )
     return;

   viewport.SetResource( "Point1", 0., 0., 0. );
   viewport.SetResource( "Point2", 0., 0., 0. );

   viewport.SetResource( "Screen/XHint", (double) x );
   viewport.SetResource( "Screen/YHint", (double) y );
   viewport.SetResource( "Screen/WidthHint", (double) width );
   viewport.SetResource( "Screen/HeightHint", (double) height );
}

/*----------------------------------------------------------------------
| Return exact time including fractions of seconds.
*/
double GetCurrTime()
{
   GlgULong sec, microsec;
   
   if( !GlgGetTime( &sec, &microsec ) )
     return 0.;
     
   return sec + microsec / 1000000.;
}

/*----------------------------------------------------------------------
| Utility function to validate the string.
*/
GlgBoolean IsUndefined( SCONST char * str )
{
   if( !str || !*str || strcmp( str, "unset" ) == 0 ||
       strcmp( str, "$unnamed" ) == 0 )
     return GlgTrue;

   return GlgFalse;
}

/*----------------------------------------------------------------------
| Utility function to check the string for the suffix.
| Returns true of string1 ends with string2 (suffix). 
| use_case=true checks for case, otherwise case is ignored.
*/
GlgBoolean EndsWith( SCONST char * string1, SCONST char * string2, 
                     GlgBoolean use_case )
{
   GlgLong length1 = strlen( string1 );
   GlgLong length2 = strlen( string2 );

   if( length1 < length2 )
     return GlgFalse;

   if( use_case )
     return ( strcmp( string1 + length1 - length2, string2 ) == 0 );
   else 
     return ( strcasecmp( string1 + length1 - length2, string2 ) == 0 );
}

/*----------------------------------------------------------------------
| Cuts off the tail string from str and returns a copy of the substring.
| The returned string must be freed in the invoking function. 
*/
char * RemoveTail( SCONST char * str, SCONST char * tail )
{
   if( !EndsWith( str, tail, GlgTrue ) )
     return NULL;

   // EndsWith returned true str includes tail
   GlgLong str_len = strlen( str );
   GlgLong tail_len = strlen( tail );
   GlgLong tail_index = str_len - tail_len;

   char * new_str = (char * ) GlgAlloc( tail_index + 1 );
   strncpy( new_str, str, tail_index );
   new_str[ tail_index ] = '\0';

   return new_str;
} 

/*----------------------------------------------------------------------
| 
*/
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, GlgLong interval )
{
   GlgULong sec2, microsec2;
   GlgLong elapsed_time, adj_interval;

   GlgGetTime( &sec2, &microsec2 );  /* End time */
   
   /* Elapsed time in millisec */
   elapsed_time = 
     ( sec2 - sec1 ) * 1000 + (long) ( microsec2 - microsec1 ) / 1000;

   /* Maintain constant update interval regardless of the system speed. */
   if( elapsed_time + 20 >= interval )
      /* Slow system: update as fast as we can, but allow a small interval 
         for handling input events. */
     adj_interval = 20;
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
