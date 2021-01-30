#ifndef GTKMM_GLG_WIDGET_H
#define GTKMM_GLG_WIDGET_H

#include <glibmm/main.h>
#include <gtkmm/widget.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkwidget.h>
#ifndef _WINDOWS
#include <gdk/gdkx.h>
#else
#include <gdk/gdkwin32.h>
#endif
#include "GlgApi.h"

class GtkmmGlgWidget : public Gtk::Widget
{
public:
   // variables
   int displayed;
   int attached;
   GlgObject viewport;
   
   // methods
   GtkmmGlgWidget();
   virtual ~GtkmmGlgWidget();
   void SetViewport( GlgObject );
   void ResizeViewport( int, int, int, int, int );
   void AttachViewport( void );
   void EnableCallback( GlgCallbackType callback_type );
   
   /* Callbacks to be overridden by subclasses. */
   virtual void InputCB( GlgObject vp, GlgObject message );
   virtual void SelectCB( GlgObject vp, char ** name_array );
   virtual void TraceCB( GlgObject vp, GlgTraceCBStruct * trace_info );
   virtual void Trace2CB( GlgObject vp, GlgTraceCBStruct * trace_info );
   virtual void HierarchyCB( GlgObject vp, 
                             GlgHierarchyCBStruct * hierarchy_info );
   virtual void HCB( GlgObject vp );
   virtual void VCB( GlgObject vp );
   
 protected:
   GlgBoolean enable_input_cb;
   GlgBoolean enable_select_cb;
   GlgBoolean enable_trace_cb;
   GlgBoolean enable_trace2_cb;
   GlgBoolean enable_hierarchy_cb;
   
   // overrides
   virtual void on_size_request(Gtk::Requisition* requisition);
   virtual void on_size_allocate(Gtk::Allocation& allocation);
   virtual void on_realize();
   virtual void on_unrealize();
};

void GtkGlgInit( int argc, char * argv[] );

#ifndef _WINDOWS
GdkFilterReturn GlgEventFilter( GdkXEvent *xevent_p, GdkEvent *event,
				gpointer data );

typedef int (*QX11EventFilter) (XEvent*);
QX11EventFilter qt_set_x11_event_filter( QX11EventFilter );

extern "C" void GlgMainEH( Window, void*, XEvent*, GlgLong );
extern "C" GlgObject GlgFindScreen( GlgLong );
#endif

#endif
