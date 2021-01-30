#ifndef _EmptyPage_h_
#define _EmptyPage_h_

#include "GlgApi.h"
#include "HMIPage.h"

typedef struct _EmptyPage
{
   HMIPage HMIPage;
   
} EmptyPage;

HMIPage * CreateEmptyPage( void );
static int epGetUpdateInterval( HMIPage * hmi_page );

#endif
