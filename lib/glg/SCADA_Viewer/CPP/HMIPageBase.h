#pragma once

#include "scada.h"

class GlgSCADAViewer;

// Base class for a GLG HMI page dispalyed in the Viewer.
class HMIPageBase
{ 
 public:
   HMIPageBase( GlgSCADAViewer * viewer );
   ~HMIPageBase( void );

 public:
   GlgSCADAViewer * Viewer;
   int PageType;

   // Virtual methods that can be overriden by the derived classses.

   virtual int GetUpdateInterval( void );
   virtual GlgBoolean UpdateData( void );

   // Callbacks.
   virtual GlgBoolean Input( GlgObjectC& viewport, GlgObjectC& message );
   virtual GlgBoolean Trace( GlgObjectC& callback_viewport, 
                             GlgTraceCBStruct * trace_data );

   // Invoked when the page has been loaded and the tags have been remapped.
   virtual void Ready( void );

   // Initialization
   virtual void InitBeforeSetup( void );
   virtual void InitAfterSetup( void );

   // Tag reassignment.
   virtual GlgBoolean NeedTagRemapping( void );
   virtual void RemapTagObject( GlgObjectC& tag_obj, SCONST char * tag_name, 
                                SCONST char * tag_source );
};

