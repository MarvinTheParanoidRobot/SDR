#pragma once

#include "scada.h"

class ActivePopupMenuRecord
{
public:
   ActivePopupMenuRecord( void );
   ~ActivePopupMenuRecord( void );
public:
   PopupMenuType menu_type;
   GlgObjectC menu_obj;           /* menu object ID */
   GlgObjectC subwindow;          /* Subwindow object inside menu_obj, 
				     holds viewport loaded into the subwindow. */
   GlgObjectC menu_vp;            /* Viewport loaded into subwindow. */
   GlgObjectC selected_obj;       /* Symbol that trigerred popup menu. */
   GlgBoolean isVisible;
};
