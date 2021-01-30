#pragma once

#include "scada.h"

class ActiveDialogRecord
{
public:
   ActiveDialogRecord( void );
   ~ActiveDialogRecord( void );
public:
   DialogType dialog_type;
   GlgObjectC dialog;           /* dialog object ID */
   GlgObjectC subwindow;        /* Subwindow object inside a dialog. */
   GlgObjectC popup_vp;         /* Viewport loaded into subwindow. */    
   GlgBoolean isVisible;
};
