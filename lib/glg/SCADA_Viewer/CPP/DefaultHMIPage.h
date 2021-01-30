#pragma once

#include "HMIPageBase.h"

class DefaultHMIPage : public HMIPageBase
{
 public:
   DefaultHMIPage( GlgSCADAViewer * viewer );
   ~DefaultHMIPage( void );

   /* Additional fields may be added as needed. */

   int GetUpdateInterval( void );
   GlgBoolean NeedTagRemapping( void );
   void RemapTagObject( GlgObjectC& tag_obj, SCONST char * tag_name, 
                        SCONST char * tag_source );
};
