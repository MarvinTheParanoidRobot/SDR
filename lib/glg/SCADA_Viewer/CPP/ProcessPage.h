#pragma once

#include "HMIPageBase.h"

#define MAX( x, y )   ( (x) > (y) ? (x) : (y) )
#define MIN( x, y )   ( (x) < (y) ? (x) : (y) )

typedef enum
{
   SOLVENT_FLOW = 0,
   STEAM_FLOW,
   COOLING_FLOW,
   WATER_FLOW
} FlowType;


class ProcessPage : public HMIPageBase
{
 public:
   ProcessPage( GlgSCADAViewer * viewer );
   ~ProcessPage( void );

 public:
   GlgObjectC Viewport;   // top level viewport of the loaded page

 public:
   int GetUpdateInterval( void );
   GlgBoolean UpdateData( void );

   // Callbacks.
   GlgBoolean Input( GlgObjectC& viewport, GlgObjectC& message );
   
   // Tag reassignment.
   GlgBoolean NeedTagRemapping( void );
   void RemapTagObject( GlgObjectC& tag_obj, SCONST char * tag_name, 
                        SCONST char * tag_source );

   void IterateProcess( GlgObjectC& drawing );
   void UpdateProcess( GlgObjectC& drawing );
   void UpdateProcessTags( GlgObjectC& drawing );
   void UpdateProcessResources( GlgObjectC& drawing );
   double GetFlow( FlowType type );
   double GetFlowValue( double state, double valve );
   int LugVar( int variable, int lug );
   double PutInRange( double variable, double low, double high );
};
