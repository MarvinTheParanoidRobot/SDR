#include "stdafx.h"
#include "LiveDataFeed.h"
#include "GlgMapViewer.h"

#ifdef _WINDOWS
# pragma warning( disable : 4800 )
#endif

/*--------------------------------------------------------------------
| LiveDataFeed class is used to generate simulated demo data.
*/
LiveDataFeed::LiveDataFeed( GlgMapViewer * viewer ) : 
   DataFeedC( viewer )
{
}

LiveDataFeed::~LiveDataFeed()
{
}

/*--------------------------------------------------------------------
| Generate simulated data for animation.
*/
GlgBoolean LiveDataFeed::GetIconData( IconData * icon )
{
   // Place custom code here.

   /*
    icon->lat_lon.x = 
    icon->lat_lon.y = 
    icon->lat_lon.z =
    icon->angle = 
   */

   return GlgTrue;
}


