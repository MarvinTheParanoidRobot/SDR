#pragma once

#include "HMIPageBase.h"

//////////////////////////////////////////////////////////////////////////
class EmptyPage : public HMIPageBase
{
 public:
   EmptyPage( GlgSCADAViewer * viewer );
   ~EmptyPage( void );
   int GetUpdateInterval( void );
};
