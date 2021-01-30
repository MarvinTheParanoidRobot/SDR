#ifndef _HMIPage_h_
#define _HMIPage_h_

#include "GlgApi.h"

/* Used for run-time pointer check. */
#define HMI_PAGE_TYPE   ( 'h' + 'p' )

typedef struct _HMIPage HMIPage;

struct _HMIPage
{
   int type;
   void (*Destroy)( HMIPage * hmi_page );
   int (*GetUpdateInterval)( HMIPage * hmi_page );
   GlgBoolean (*UpdateData)( HMIPage * hmi_page );
   GlgBoolean (*InputCB)( HMIPage * hmi_page, GlgObject viewport, 
                          GlgAnyType client_data, GlgAnyType call_data );
   GlgBoolean (*TraceCB)( HMIPage * hmi_page, GlgObject viewport, 
                          GlgAnyType client_data, GlgAnyType call_data );
   GlgBoolean (*NeedTagRemapping)( HMIPage * hmi_page );
   void (*RemapTagObject)( HMIPage * hmi_page, GlgObject tag_obj, 
                           char * tag_name, char * tag_source );
   void (*InitBeforeSetup)( HMIPage * hmi_page );
   void (*InitAfterSetup)( HMIPage * hmi_page );

   /* Invoked when the page has been loaded and the tags have been remapped. */
   void (*Ready)( HMIPage * hmi_page );

};

static int hmip_type_error( void );
void HMIP_Destroy( HMIPage * hmi_page );
int HMIP_GetUpdateInterval( HMIPage * hmi_page );
GlgBoolean HMIP_UpdateData( HMIPage * hmi_page );
GlgBoolean HMIP_InputCB( HMIPage * hmi_page, GlgObject viewport, 
                         GlgAnyType client_data, GlgAnyType call_data );
GlgBoolean HMIP_TraceCB( HMIPage * hmi_page, GlgObject viewport, 
                         GlgAnyType client_data, GlgAnyType call_data );
GlgBoolean HMIP_NeedTagRemapping( HMIPage * hmi_page );
void HMIP_RemapTagObject( HMIPage * hmi_page, GlgObject tag_obj, 
                          char * tag_name, char * tag_source );
void HMIP_InitBeforeSetup( HMIPage * hmi_page );
void HMIP_InitAfterSetup( HMIPage * hmi_page );
void HMIP_Ready( HMIPage * hmi_page );

#endif
