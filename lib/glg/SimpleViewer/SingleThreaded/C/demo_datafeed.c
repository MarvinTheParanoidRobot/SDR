#include "GlgViewer.h"

extern GlgObject Viewport;

/*--------------------------------------------------------------------
| Generate simulated D value in the range defined by Low and High variables.
| If RANDOM_DATA is False, ReadDValue from datafeed.c will be used
| to obtain real-time value.
*/
GlgBoolean DemoReadDValue( TagRecord * tag_record, double * d_value )
{

   /* Data range for simulated demo data. */
   double low = 0.;
   double high = 100.;
   
   double period = 100;
   double
     alpha, center, half_amplitude;

   static long counter = 0;

   period = 500.;
   half_amplitude = ( high - low ) / 2.0;
   center = low + half_amplitude;
   alpha = 2. * M_PI * counter / period;
   *d_value = center + half_amplitude * sin( alpha ) * sin( alpha / 30. );

   ++counter;
   return GlgTrue;
}

/*--------------------------------------------------------------------
| Generate simulated S value
*/
GlgBoolean DemoReadSValue( TagRecord * tag_record, char ** s_value )
{
   if( !s_value )
     return GlgFalse;
   
   *s_value = GlgRand( 0., 1. ) > 0.5 ? "On" : "Off";

   return GlgTrue;
}

/*--------------------------------------------------------------------
| Simulte the "Write" opearion. For demo purposes, set the value
| of the specified tag source in the drawing. 
*/
GlgBoolean DemoWriteDValue( char * tag_source, double d_value )
{
   return GlgSetDTag( Viewport, tag_source, d_value, False );  
}

