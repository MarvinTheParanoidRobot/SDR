#include "GtkmmGlgWidget.h"
#include "GlgApi.h"

#ifndef CUSTOM_WIDGET_H
#define CUSTOM_WIDGET_H

class CustomWidget: public GtkmmGlgWidget
{
public:
   // variables
   GlgBoolean AnimateDrawing;
   
   // methods
   CustomWidget();
   ~CustomWidget();

   void AnimateControlPanel( void );
   void StartUpdateTimeout( void );
   
   /* Callbacks to be overridden by subclasses. */
   virtual void InputCB( GlgObject vp, GlgObject message );
   virtual void SelectCB( GlgObject vp, char ** name_array );
   virtual void TraceCB( GlgObject vp, GlgTraceCBStruct * trace_info );
   virtual void Trace2CB( GlgObject vp, GlgTraceCBStruct * trace_info );
   virtual void HierarchyCB( GlgObject vp, 
                             GlgHierarchyCBStruct * hirerachy_info );
   virtual void HCB( GlgObject vp );
   virtual void VCB( GlgObject vp );
   
 protected:
   // overrides
   virtual void on_size_request(Gtk::Requisition* requisition);
   virtual void on_size_allocate(Gtk::Allocation& allocation);
   virtual void on_realize();
   virtual void on_unrealize();
};

#endif

