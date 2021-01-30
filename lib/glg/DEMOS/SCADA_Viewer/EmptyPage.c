#include "EmptyPage.h"

/*----------------------------------------------------------------------*/
HMIPage * CreateEmptyPage()
{
   EmptyPage * ep = GlgAllocStruct( sizeof( EmptyPage ) );
   ep->HMIPage.type = HMI_PAGE_TYPE;
   
   ep->HMIPage.GetUpdateInterval = epGetUpdateInterval;
   return (HMIPage*) ep;
}

/*----------------------------------------------------------------------
| Returns an update interval in msec for animating drawing with data.
*/
static int epGetUpdateInterval( HMIPage * hmi_page )
{
   return 0;
}
