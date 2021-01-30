#include "DefaultHMIPage.h"
#include "PageType.h"
#include "GlgSCADAViewer.h"

/* Default HMI Page; can be used for any page that does not need any special
   handling other than tag data updates and handling user interaction via 
   commands attached to objects at design time.

   The data in GlgSCADAViewer are accessible via the global Viewer 
   variable of HMIPageBase class.
*/

// Constructor
DefaultHMIPage::DefaultHMIPage( GlgSCADAViewer * viewer )  
  : HMIPageBase( viewer )
{
}

// Destructor
DefaultHMIPage::~DefaultHMIPage()
{
}

/*----------------------------------------------------------------------
| Returns an update interval in msec for animating drawing with data.
*/
int DefaultHMIPage::GetUpdateInterval()
{
   switch( PageType )
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
GlgBoolean DefaultHMIPage::NeedTagRemapping( void )
{
   /* In demo mode, unset/undefined tags need to be remapped to enable animation. */
   if( Viewer->RandomData )
     return GlgTrue;
   else
     return GlgFalse;   /* Remap tags only if necessary. */
}

/*----------------------------------------------------------------------
| Returns true if tag sources need to be remapped for the page.
*/
void DefaultHMIPage::RemapTagObject( GlgObjectC& tag_obj, 
                                     SCONST char * tag_name, 
                                     SCONST char * tag_source )
{
   if( Viewer->RandomData )
   {
      /* Skip tags with undefined TagName. */
      if( IsUndefined( tag_name ) )
        return;
            
      /* In demo mode, assign unset/undefined tag sources to be 
         the same as tag names to enable animation with demo data,
         i.e. assign TagSource to be the same as TagName.
      */
      if( IsUndefined( tag_source ) )
        Viewer->AssignTagSource( tag_obj, tag_name );
   }
   else
   {
#if 0
      /* Assign new TagSource as needed. */
      Viewer->AssignTagSource( tag_obj, new_tag_source );
#endif
   }
}
