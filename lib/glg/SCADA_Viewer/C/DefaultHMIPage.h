#ifndef _DefaultHMIPage_h_
#define _DefaultHMIPage_h_

#include "GlgApi.h"
#include "HMIPage.h"
#include "GlgSCADAViewer.h"

typedef struct _DefaultHMIPage
{
   HMIPage HMIPage;
   
   /* Additional fields may be added as needed. */

} DefaultHMIPage;

HMIPage * CreateDefaultHMIPage( void );
static void dhpDestroy( HMIPage * hmi_page );
static int dhpGetUpdateInterval( HMIPage * hmi_page );
static GlgBoolean dhpNeedTagRemapping( HMIPage * hmi_page );
static void dhpRemapTagObject( HMIPage * hmi_page, GlgObject tag_obj, 
                               char * tag_name, char * tag_source );
static void dhpReady( HMIPage * hmi_page );

#endif
