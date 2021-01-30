#include <stdio.h>
#include "GlgApi.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

#define RadToDeg( angle )   ( ( angle ) / M_PI * 180. )
#define DegToRad( angle )   ( ( angle ) /180. * M_PI )

void DisplayUsage( void );
int SatelliteMain( int argc, char * argv[], GlgAppContext app_context );
int TrajectoryMain( int argc, char * argv[], GlgAppContext app_context );
void SetPolygonPoint( GlgObject polygon, GlgLong point_index,
                             double lon, double lat, double elev );
void AddPolygonPoint( GlgObject polygon, double lon, double lat, double elev,
                      GlgBoolean add_at_the_end );
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, 
                            GlgLong interval );

