#include "EmptyPage.h"
#include "GlgSCADAViewer.h"

EmptyPage::EmptyPage( GlgSCADAViewer * viewer ) : HMIPageBase( viewer ) 
{
}

EmptyPage::~EmptyPage( void )
{
}

int EmptyPage::GetUpdateInterval( void )
{ 
   return 0;   // Empty page: no data. 
}
