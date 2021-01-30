#include "stdafx.h"
#include <math.h>
#include "DemoDataFeed.h"
#include "GlgMapViewer.h"

#ifdef _WINDOWS
# pragma warning( disable : 4800 )
#endif

/*--------------------------------------------------------------------
| DemoDataFeed class is used to generate simulated demo data.
*/
DemoDataFeed::DemoDataFeed( GlgMapViewer * viewer ) : 
   DataFeedC( viewer )
{
}

DemoDataFeed::~DemoDataFeed()
{
}

/*--------------------------------------------------------------------
| Generate simulated data for animation.
*/
GlgBoolean DemoDataFeed::GetIconData( IconData * icon )
{
   // A counter used to calculate icon position in GetIconPosition().
   static int RotationState = 0;   

    double radius = Viewer->InitExtent.x / 5.0;
    
    ++RotationState;
    if( RotationState > 360 )
      RotationState -= 360;
    
    double angle = RotationState;
    double rad_angle = angle / 180. * M_PI;

    icon->lat_lon.x = Viewer->InitCenter.x + radius * cos( rad_angle );
    icon->lat_lon.y = Viewer->InitCenter.y + radius * sin( rad_angle );
    icon->lat_lon.z = 0.;
    icon->angle = angle + 90.;

    return GlgTrue;
}


