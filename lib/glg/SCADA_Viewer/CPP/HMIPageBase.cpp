#include "HMIPageBase.h"
#include "GlgSCADAViewer.h"

/*----------------------------------------------------------------------
| Construction/Destruction
*/
HMIPageBase::HMIPageBase( GlgSCADAViewer * viewer )
{
   Viewer = viewer;
   PageType = Viewer->PageType;
}

HMIPageBase::~HMIPageBase( void )
{
}

/*-------------------------------------------------------------------------
| Returns an update interval in msec for animating drawing with data.
*/
int HMIPageBase::GetUpdateInterval( void )
{
   return 1000;    // Update once a second by default.
}

/*-------------------------------------------------------------------------
| A custom update method for animating drawing with data; may be used
| to implement any additional data update logic.
*/
GlgBoolean HMIPageBase::UpdateData( void )
{
   /* Return false to automatically update all tags defined in the drawing 
      (via the UpdateData method of the GlgSCADAViewer class).
   */
   return GlgFalse;
}

/*-------------------------------------------------------------------------
| A custom input handler for the page. If it returns false, the default
| input handler of the SCADA Viewer will be used to process common
| events and commands.
*/
GlgBoolean HMIPageBase::Input( GlgObjectC& viewport, GlgObjectC& message )
{
   return GlgFalse;
}

/*-------------------------------------------------------------------------
| A custom trace callback for the page. If it returns false, the default 
| trace callback of the SCADA Viewer will be used to process events.
*/
GlgBoolean HMIPageBase::Trace( GlgObjectC& callback_viewport, 
                               GlgTraceCBStruct * trace_data )
{
   return GlgFalse;
}

/*-------------------------------------------------------------------------
| Returns true if tag sources need to be remapped for the page,
| in which case RemapTagObject() must provide code to perform desired
| remapping logic.
*/
GlgBoolean HMIPageBase::NeedTagRemapping()
{
   return GlgFalse;
}

/*-------------------------------------------------------------------------
| Used if NeedTagRemapping() returns true.
| Reassigns TagSource parameter for a given tag object to a new
| TagSource value. tag_source and tag_name parameters are the current 
| TagSource and TagName of the tag_obj.
*/
void HMIPageBase::RemapTagObject( GlgObjectC& tag_obj, SCONST char * tag_name, 
                                  SCONST char * tag_source )
{
}

/*-------------------------------------------------------------------------
| Perform any desired initialization of the drawing before hierarchy setup.
*/
void HMIPageBase::InitBeforeSetup(){}

/*-------------------------------------------------------------------------
| Perform any desired initialization of the drawing after hierarchy setup.
*/
void HMIPageBase::InitAfterSetup(){}

/*-------------------------------------------------------------------------
| Invoked when the page has been loaded and the tags have been remapped.
*/
void HMIPageBase::Ready(){}

