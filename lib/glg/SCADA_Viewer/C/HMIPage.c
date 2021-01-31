#include <stdio.h>
#include <stdlib.h>
#include "GlgApi.h"
#include "HMIPage.h"

#define HMI_PAGE_TYPE_CHECK( hmip_ptr ) \
   ( hmip_ptr->type != HMI_PAGE_TYPE ? hmip_type_error() : 1 )

/*-------------------------------------------------------------------------*/
static int hmip_type_error( void )
{
   GlgError( GLG_USER_ERROR, "Invalid HMIPage pointer" );
   exit( GLG_EXIT_ERROR );
}

/*-------------------------------------------------------------------------*/
void HMIP_Destroy( HMIPage * hmip )
{
   HMIPage * hmi_page = (HMIPage*) hmip;
   HMI_PAGE_TYPE_CHECK( hmi_page );

   if( hmi_page->Destroy )
     hmi_page->Destroy( hmi_page );

   GlgFree( hmi_page );
}

/*-------------------------------------------------------------------------
| Returns an update interval in msec for animating drawing with data.
*/
int HMIP_GetUpdateInterval( HMIPage * hmip )
{
   HMIPage * hmi_page = (HMIPage*) hmip;
   HMI_PAGE_TYPE_CHECK( hmi_page );
   
   if( !hmi_page->GetUpdateInterval )
     return 1000;     /* Update once a second by default. */

   return hmi_page->GetUpdateInterval( hmi_page );
}

/*-------------------------------------------------------------------------
| A custom update method for animating drawing with data; may be used
| to implement any additional data update logic.
*/
GlgBoolean HMIP_UpdateData( HMIPage * hmip )
{
   HMIPage * hmi_page = (HMIPage*) hmip;
   HMI_PAGE_TYPE_CHECK( hmi_page );

   if( !hmi_page->UpdateData )
     /* Return false to automatically update all tags defined in the drawing 
        (via the UpdateData method of GlgSCADAViewer).
     */
     return False;

   return hmi_page->UpdateData( hmi_page );
}

/*-------------------------------------------------------------------------
| A custom input handler for the page. If it returns false, the default
| input handler of the SCADA Viewer will be used to process common
| events and commands.
*/
GlgBoolean HMIP_InputCB( HMIPage * hmip, GlgObject viewport, 
                         GlgAnyType client_data, GlgAnyType call_data )
{
   HMIPage * hmi_page = (HMIPage*) hmip;
   HMI_PAGE_TYPE_CHECK( hmi_page );
   
   if( !hmi_page->InputCB )
     return GlgFalse;

   return hmi_page->InputCB( hmi_page, viewport, client_data, call_data );
}

/*-------------------------------------------------------------------------
| A custom trace callback for the page. If it returns false, the default 
| trace callback of the SCADA Viewer will be used to process events.
*/
GlgBoolean HMIP_TraceCB( HMIPage * hmip, GlgObject viewport, 
                         GlgAnyType client_data, GlgAnyType call_data )
{
   HMIPage * hmi_page = (HMIPage*) hmip;
   HMI_PAGE_TYPE_CHECK( hmi_page );
   
   if( !hmi_page->TraceCB )
     return GlgFalse;

   return hmi_page->TraceCB( hmi_page, viewport, client_data, call_data );
}

/*-------------------------------------------------------------------------
| Returns true if tag sources need to be remapped for the page,
| in which case RemapTagObject() must provide code to perform desired
| remapping logic.
*/
GlgBoolean HMIP_NeedTagRemapping( HMIPage * hmip )
{
   HMIPage * hmi_page = (HMIPage*) hmip;
   HMI_PAGE_TYPE_CHECK( hmi_page );
   
   if( !hmi_page->NeedTagRemapping )
     return GlgFalse;
     
   return hmi_page->NeedTagRemapping( hmi_page );
}

/*-------------------------------------------------------------------------
| Used if NeedTagRemapping() returns true.
| Reassigns TagSource parameter for a given tag object to a new
| TagSource value. tag_source and tag_name parameters are the current 
| TagSource and TagName of the tag_obj.
*/
void HMIP_RemapTagObject( HMIPage * hmip, 
                          GlgObject tag_obj, char * tag_name, char * tag_source )
{
   HMIPage * hmi_page = (HMIPage*) hmip;
   HMI_PAGE_TYPE_CHECK( hmi_page );
   
   if( hmi_page->RemapTagObject )
     hmi_page->RemapTagObject( hmi_page, tag_obj, tag_name, tag_source );
}

/*-------------------------------------------------------------------------
| Perform any desired initialization of the drawing before hierarchy setup.
*/
void HMIP_InitBeforeSetup( HMIPage * hmip )
{
   HMIPage * hmi_page = (HMIPage*) hmip;
   HMI_PAGE_TYPE_CHECK( hmi_page );

   if( hmi_page->InitBeforeSetup )
     hmi_page->InitBeforeSetup( hmi_page );
}

/*-------------------------------------------------------------------------
| Perform any desired initialization of the drawing after hierarchy setup.
*/
void HMIP_InitAfterSetup( HMIPage * hmip )
{
   HMIPage * hmi_page = (HMIPage*) hmip;
   HMI_PAGE_TYPE_CHECK( hmi_page );
   
   if( hmi_page->InitAfterSetup )
     hmi_page->InitAfterSetup( hmi_page );
}

/*-------------------------------------------------------------------------
| Invoked when the page has been loaded and the tags have been remapped.
*/
void HMIP_Ready( HMIPage * hmip )
{
   HMIPage * hmi_page = (HMIPage*) hmip;
   HMI_PAGE_TYPE_CHECK( hmi_page );
   
   if( hmi_page->Ready )
     hmi_page->Ready( hmi_page );
}
