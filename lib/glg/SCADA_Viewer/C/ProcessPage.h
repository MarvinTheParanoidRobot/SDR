#ifndef _ProcessPage_h_
#define _ProcessPage_h_

#include "GlgSCADAViewer.h"

#define MAX( x, y )   ( (x) > (y) ? (x) : (y) )
#define MIN( x, y )   ( (x) < (y) ? (x) : (y) )

typedef enum
{
   SOLVENT_FLOW = 0,
   STEAM_FLOW,
   COOLING_FLOW,
   WATER_FLOW
} FlowType;

HMIPage * CreateProcessPage( void );

static int procGetUpdateInterval( HMIPage * hmi_page );
static GlgBoolean procUpdateData( HMIPage * hmi_page );
static GlgBoolean procInputCB( HMIPage * hmi_page, GlgObject viewport, 
                               GlgAnyType client_data, GlgAnyType call_data );
static GlgBoolean procNeedTagRemapping( HMIPage * hmi_page );
static void procRemapTagObject( HMIPage * hmi_page, GlgObject tag_obj, 
                                char * tag_name, char * tag_source );

void IterateProcess( GlgObject drawing );
void UpdateProcess( GlgObject drawing );
void UpdateProcessTags( GlgObject drawing );
void UpdateProcessResources( GlgObject drawing );
double GetFlow( FlowType type );
double GetFlowValue( double state, double valve );
int LugVar( int variable, int lug );
double PutInRange( double variable, double low, double high );

#endif
