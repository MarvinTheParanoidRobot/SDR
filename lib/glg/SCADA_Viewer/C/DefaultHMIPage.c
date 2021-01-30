#include "DefaultHMIPage.h"
#include "GlgSCADAViewer.h"

/* Default HMI Page; can be used for any page that does not need any special
   handling other then tag data updates and handling user interaction via 
   commands.

   The data in GlgSCADAViewer structure are accessible via the global Viewer 
   variable.
*/

/*----------------------------------------------------------------------*/
HMIPage * CreateDefaultHMIPage()
{
   DefaultHMIPage * dhp = GlgAllocStruct( sizeof( DefaultHMIPage ) );
   dhp->HMIPage.type = HMI_PAGE_TYPE;
   
   dhp->HMIPage.Destroy = dhpDestroy;
   dhp->HMIPage.GetUpdateInterval = dhpGetUpdateInterval;
   dhp->HMIPage.NeedTagRemapping = dhpNeedTagRemapping;
   dhp->HMIPage.RemapTagObject = dhpRemapTagObject;

   /* The rest of not used function pointers were initialized to NULL. */

   return (HMIPage*) dhp;
}

/*----------------------------------------------------------------------*/
static void dhpDestroy( HMIPage * hmi_page )
{
   /* The hmi_page itself is freed with GlgFree() by the superclass 
      automatically, and this method is not needed for this page since 
      it does not allocate or reference any additional data. 

      Place code here to free or derereference any added data stored 
      by the page as needed.
   */
} 

/*----------------------------------------------------------------------
| Returns an update interval in msec for animating drawing with data.
*/
static int dhpGetUpdateInterval( HMIPage * hmi_page )
{
   switch( Viewer.PageType )
   {
    case AERATION_PAGE:      return 2000;
    case CIRCUIT_PAGE:       return 1000;
    case RT_CHART_PAGE:      return 30;
    case TEST_COMMANDS_PAGE: return 100;
    default:                 return 500;   /* Use default update interval. */
   }
}

/*----------------------------------------------------------------------
| Returns true if tag sources need to be remapped for the page.
*/
static GlgBoolean dhpNeedTagRemapping( HMIPage * hmi_page )
{
   /* In demo mode, unset tags need to be remapped to enable animation. */
   if( Viewer.RandomData )
     return GlgTrue;
   else
     return GlgFalse;   /* Remap tags only if necessary. */
}

/*----------------------------------------------------------------------
| Returns true if tag sources need to be remapped for the page.
*/
static void dhpRemapTagObject( HMIPage * hmi_page, GlgObject tag_obj, 
                               char * tag_name, char * tag_source )
{
   if( Viewer.RandomData )
   {
      /* Skip tags with undefined TagName. */
      if( IsUndefined( tag_name ) )
        return;
            
      /* In demo mode, assign unset tag sources to be the same as tag names
         to enable animation with demo data.
      */
      if( IsUndefined( tag_source ) )
        AssignTagSource( tag_obj, tag_name );
   }
   else
   {
#if 0
      /* Assign new TagSource as needed. */
      AssignTagSource( tag_obj, new_tag_source );
#endif
   }
}
