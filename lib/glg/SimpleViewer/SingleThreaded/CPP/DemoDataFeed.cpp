#include <math.h>
#include "DemoDataFeed.h"
#include "stdafx.h"

#ifdef _WINDOWS
# pragma warning( disable : 4800 )
#endif

/*--------------------------------------------------------------------
| DemoDataFeed class is used to generate simulated demo data.
*/
DemoDataFeed::DemoDataFeed( GlgViewer * viewer )
{
   Viewer = viewer;
}

DemoDataFeed::~DemoDataFeed()
{
}

bool DemoDataFeed::ReadDValue( TagRecordC * tag_record, double * d_value )
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
   return True;
}

/*--------------------------------------------------------------------
| Generate simulated S value
*/
bool DemoDataFeed::ReadSValue( TagRecordC * tag_record, CONST char ** s_value )
{
   if( !s_value )
     return False;
   
   *s_value = ( GlgRand( 0., 1. ) > 0.5 ? "On" : "Off" );

   return True;
}

/*--------------------------------------------------------------------
| Simulte the "Write" opearion for a D value. For demo purposes, 
| set the value of the specified tag source in the drawing. 
*/
bool DemoDataFeed::WriteDValue( CONST char * tag_source, double d_value )
{
   return Viewer->SetTag( tag_source, d_value, False );  
}


/*--------------------------------------------------------------------
| Simulte the "Write" opearion for S value. For demo purposes, set the value
| of the specified tag source in the drawing. 
*/
bool DemoDataFeed::WriteSValue( CONST char * tag_source, CONST char * s_value )
{
   return Viewer->SetTag( tag_source, s_value, False );  
}

