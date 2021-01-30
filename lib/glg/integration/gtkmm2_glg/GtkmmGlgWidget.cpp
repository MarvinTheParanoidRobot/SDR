/* GTKMM2 - GLG integration example.
   See README.txt for notes on using GLG C++ vs. C API.
 */

#include "GtkmmGlgWidget.h"
#include <gtk/gtkwidget.h>
#include <gtk/gtklabel.h>

static int initialized = False;

#ifndef _WINDOWS
extern "C" GlgLong gtk_glg_add_timer( GlgLong interval, GlgTimerProc proc, 
                                      GlgAnyType client_data );
extern "C" void gtk_glg_remove_timer( GlgLong timer_id );
extern "C" void gtk_glg_draw_tooltip( char * tooltip_string, 
                                      GlgObject viewport,
                                      GlgObject top_viewport,
                                      GlgLong root_x, GlgLong root_y,
                                      GlgLong win_x, GlgLong win_y );
#endif

/*----------------------------------------------------------------------*/
void GtkGlgInit( int argc, char ** argv )
{
   /* Initialize GLG using applicable command-line options. */
   GlgInit( 0, NULL, argc, argv );
}

/*----------------------------------------------------------------------*/
GtkmmGlgWidget::GtkmmGlgWidget()
{
   set_has_window( false );
   set_redraw_on_allocate(false);
   
   if( !initialized )
   {
      initialized = True;
      
      /* Invoke again in case GtkGlgInit() call was omitted. */
      GlgInit( 0, NULL, 0, NULL );

#ifndef _WINDOWS
     /* A global event filter used to pass events to the GLG window.
	GTK is X11-based and there is no other way to install a handler 
	on per-widget basis in Xt-like manner.
	*/
     gdk_window_add_filter( (GdkWindow*)0, (GdkFilterFunc)GlgEventFilter,
			   (gpointer)0 );

     GlgSetAddTimerFunc( gtk_glg_add_timer );
     GlgSetRemoveTimerFunc( gtk_glg_remove_timer );
     GlgSetDrawTooltipFunc( gtk_glg_draw_tooltip );
#endif
   }

   displayed = False;
   attached = False;
   viewport = (GlgObject)0;

   enable_input_cb = False;
   enable_select_cb = False;
   enable_trace_cb = False;
   enable_trace2_cb = False;
   enable_hierarchy_cb = False;
}

/*----------------------------------------------------------------------*/
GtkmmGlgWidget::~GtkmmGlgWidget()
{
   if( displayed )
     GlgResetHierarchy( viewport );
   GlgDropObject( viewport );
}

/*----------------------------------------------------------------------*/
void GtkmmGlgWidget::on_size_request( Gtk::Requisition* requisition )
{
  // Initialize the output parameter:
  *requisition = Gtk::Requisition();

  // Discover the total amount of minimum space needed by this widget.
  
  // Don't allow size < 5 pixels
  requisition->height = 5;
  requisition->width = 5;
}

/*----------------------------------------------------------------------*/
void GtkmmGlgWidget::on_size_allocate( Gtk::Allocation& allocation )
{
  // Use the whole space we have actually been given:
  // (We will not be given heights or widths less than we have requested, 
  // though we might get more)

  // Use the offered allocation for this container:
  set_allocation( allocation );

  if( !viewport || !attached )
    return;

  /* Set the viewport size to match widget's size if displayed. */
  ResizeViewport( allocation.get_x(), allocation.get_y(),
		  allocation.get_width(), allocation.get_height(), True );
}

/*----------------------------------------------------------------------
| To reparent, set null viewport, reparent, then set the viewport back
| (dynamic changes will be lost and will have to be recreated). Rererence
| the viewport for the duration of reparenting to keep it around, and 
| drop/deref when finished.
*/
void GtkmmGlgWidget::SetViewport( GlgObject viewport_p ) 
{
   GtkWidget * widget;
   GdkWindow * gdk_window;

   if( displayed )
   {
      GlgResetHierarchy( viewport );
      displayed = False;
      attached = False;
   }

   if( viewport_p != viewport )
   {
      GlgDropObject( viewport );
      viewport = GlgReferenceObject( viewport_p );
   }

   AttachViewport();
}

/*----------------------------------------------------------------------*/
void GtkmmGlgWidget::AttachViewport()
{
   GtkWidget * widget;
   GdkWindow * gdk_window;
#ifndef _WINDOWS
   GdkScreen * gdk_screen;
   Display * display;
   Screen * x_screen;
   Window win_id;
#else
   HWND win_id;
#endif

   if( !viewport )
      return;

   /* Attach Glg viewport to the widget using widget's window as a 
      parent. */
   widget = GTK_WIDGET( gobj() );
   gdk_window = GDK_WINDOW( widget->window );

   if( !gdk_window )
      return;   /* No window yet */

#ifndef _WINDOWS
   win_id = GDK_WINDOW_XID( gdk_window );
   display = GDK_WINDOW_XDISPLAY( gdk_window );
#ifdef GDK_2_2
   gdk_screen = gtk_widget_get_screen( widget );
   x_screen = GDK_SCREEN_XSCREEN( gdk_screen );
#else
   x_screen = DefaultScreenOfDisplay( display );
#endif

   GlgSetParentWidget( viewport, display, x_screen, (void*) win_id );   
#else
   win_id = GDK_WINDOW_HWND( gdk_window );
   GlgSetParentWidget( viewport, NULL, 0, (void*) win_id );   
#endif
   GlgSetLResource( viewport, (char*) "DataSlot", (GlgLong)widget );
   attached = True;

   Gtk::Allocation allocation = get_allocation();
   ResizeViewport( allocation.get_x(), allocation.get_y(),
		   allocation.get_width(), allocation.get_height(), True );
}

/*----------------------------------------------------------------------
  Stubs for implementing C++ callbacks
*/
extern "C" void 
  GtkmmGlgWidgetInputStub( GlgObject callback_viewport,
                           GlgAnyType client_data, GlgAnyType call_data )
{
   GtkmmGlgWidget * initiator = (GtkmmGlgWidget*)client_data;

   initiator->InputCB( callback_viewport, (GlgObject)call_data );
}

extern "C" void 
  GtkmmGlgWidgetSelectStub( GlgObject callback_viewport,
                            GlgAnyType client_data, GlgAnyType call_data )
{
   GtkmmGlgWidget * initiator = (GtkmmGlgWidget*)client_data;

   initiator->SelectCB( callback_viewport, (char **)call_data );
}

extern "C" void 
  GtkmmGlgWidgetTraceStub( GlgObject callback_viewport,
                           GlgAnyType client_data, GlgAnyType call_data )
{
   GtkmmGlgWidget * initiator = (GtkmmGlgWidget*)client_data;

   initiator->TraceCB( callback_viewport, (GlgTraceCBStruct*) call_data );
}

extern "C" void 
  GtkmmGlgWidgetTrace2Stub( GlgObject callback_viewport,
                            GlgAnyType client_data, GlgAnyType call_data )
{
   GtkmmGlgWidget * initiator = (GtkmmGlgWidget*)client_data;

   initiator->Trace2CB( callback_viewport, (GlgTraceCBStruct*) call_data );
}

extern "C" void 
  GtkmmGlgWidgetHierarchyStub( GlgObject callback_viewport,
                               GlgAnyType client_data, GlgAnyType call_data )
{
   GtkmmGlgWidget * initiator = (GtkmmGlgWidget*)client_data;

   initiator->HierarchyCB( callback_viewport, 
                          (GlgHierarchyCBStruct*) call_data );
}

/*----------------------------------------------------------------------
| We use our own window, so need to set x and y as well.
*/
void GtkmmGlgWidget::ResizeViewport( int x, int y, int width, int height, 
				     int draw )
{
   if( !viewport || !attached )
      return;

   GlgSetGeometry( viewport, x, y, width, height );
   
   if( !draw )
     return;

   /* Display of the viewport is requested. */
   if( !displayed )
   {
      /* Setup and draw the first time */
      displayed = True;

      /* Add callbacks if enabled. */
      if( enable_input_cb )
        GlgAddCallback( viewport, GLG_INPUT_CB, 
                       (GlgCallbackProc)GtkmmGlgWidgetInputStub, 
                       (GlgAnyType)this );

      if( enable_select_cb )
        GlgAddCallback( viewport, GLG_SELECT_CB,
                       (GlgCallbackProc)GtkmmGlgWidgetSelectStub, 
                       (GlgAnyType)this );

      if( enable_trace_cb )
        GlgAddCallback( viewport, GLG_TRACE_CB,
                       (GlgCallbackProc)GtkmmGlgWidgetTraceStub,
                       (GlgAnyType)this );
      if( enable_trace2_cb )
        GlgAddCallback( viewport, GLG_TRACE2_CB,
                       (GlgCallbackProc)GtkmmGlgWidgetTrace2Stub, 
                       (GlgAnyType)this );

      if( enable_hierarchy_cb )
        GlgAddCallback( viewport, GLG_HIERARCHY_CB,
                       (GlgCallbackProc)GtkmmGlgWidgetHierarchyStub, 
                       (GlgAnyType)this );

      HCB( viewport );   /* New H callback */     

      GlgSetupHierarchy( viewport );
      
      VCB( viewport );   /* New V callback */     

      GlgUpdate( viewport );
   }
   else
     /* Update to redraw after a size change. */
     GlgUpdate( viewport );
}

/*----------------------------------------------------------------------*/
void GtkmmGlgWidget::EnableCallback( GlgCallbackType callback_type )
{
   switch( callback_type )
   {
    case GLG_INPUT_CB: enable_input_cb = True; break;
    case GLG_SELECT_CB: enable_select_cb = True; break;
    case GLG_TRACE_CB: enable_trace_cb = True; break;
    case GLG_TRACE2_CB: enable_trace2_cb = True; break;
    case GLG_HIERARCHY_CB: enable_hierarchy_cb = True; break;
    default: GlgError( GLG_WARNING, (char*) "Invalid callback type." ); break;
   }
}

/*----------------------------------------------------------------------
  Callbacks to be overriden by subclasses.
*/
void GtkmmGlgWidget::InputCB( GlgObject vp, GlgObject message )
{
}

void GtkmmGlgWidget::SelectCB( GlgObject vp, char ** name_array )
{
}

void GtkmmGlgWidget::TraceCB( GlgObject vp, GlgTraceCBStruct * trace_info )
{
}

void GtkmmGlgWidget::Trace2CB( GlgObject vp, GlgTraceCBStruct * trace_info )
{
}

void GtkmmGlgWidget::HierarchyCB( GlgObject vp, 
                             GlgHierarchyCBStruct * hierarchy_info )
{
}

void GtkmmGlgWidget::HCB( GlgObject vp )
{
}

void GtkmmGlgWidget::VCB( GlgObject vp )
{
}

/*----------------------------------------------------------------------*/
void GtkmmGlgWidget::on_realize()
{
   // Call base class:
   Gtk::Widget::on_realize();

   AttachViewport();
}

/*----------------------------------------------------------------------*/
void GtkmmGlgWidget::on_unrealize()
{
   if( displayed )
   {
      GlgResetHierarchy( viewport );
      displayed = False;
      attached = False;
   }

   // Call base class:
   Gtk::Widget::on_unrealize();
}

#ifndef _WINDOWS
/*----------------------------------------------------------------------*/
GdkFilterReturn GlgEventFilter( GdkXEvent *xevent_p, GdkEvent *event, 
				gpointer data )
{
   XEvent * xevent = (XEvent*) xevent_p;

   if( xevent->xany.window )   /* Check window-based events only. */
   {
      GlgObject screen = GlgFindScreen( (long) xevent->xany.window );

      if( screen )     /* Event from one of the GLG viewports: pass to GLG. */
      {
	 GlgMainEH( xevent->xany.window, screen, xevent, True );
	 
	 return GDK_FILTER_REMOVE;  /* event handled, don't pass to GTK */
      }
   }

   return GDK_FILTER_CONTINUE;   /* event not handled, pass to GTK */
}

typedef struct _GlgTimerData
{
   GlgTimerProc proc;
   void * client_data;
   GlgLong timer_id;
   
} GlgTimerData;

/*----------------------------------------------------------------------*/
extern "C" gint glg_timer_proc( gpointer data )
{
   GlgTimerData * timer_data = (GlgTimerData*) data;
   
   (*timer_data->proc)( timer_data->client_data, NULL );

   g_source_remove( timer_data->timer_id );    /* Remove timer */
   GlgFree( timer_data );                      /* Free timer data */
   return False;
}

/*----------------------------------------------------------------------*/
extern "C" GlgLong gtk_glg_add_timer( GlgLong interval, GlgTimerProc proc, 
                                     GlgAnyType client_data )
{
   GlgTimerData * timer_data;

   timer_data = (GlgTimerData*) GlgAlloc( sizeof( *timer_data ) );
   timer_data->proc = proc;
   timer_data->client_data = client_data;

   timer_data->timer_id = g_timeout_add( (guint32) interval, glg_timer_proc,
                                         (gpointer) timer_data );   

   return (GlgLong) timer_data;
}

/*----------------------------------------------------------------------*/
extern "C" void gtk_glg_remove_timer( GlgLong timer_id )
{
   GlgTimerData * timer_data = (GlgTimerData*) timer_id;
   
   g_source_remove( timer_data->timer_id );
   GlgFree( timer_data );     
}

/*----------------------------------------------------------------------*/
extern "C" void gtk_glg_draw_tooltip( char * tooltip_string, 
                                      GlgObject viewport, 
                                      GlgObject top_viewport,
                                      GlgLong root_x, GlgLong root_y,
                                      GlgLong win_x, GlgLong win_y )
{
   static GtkWidget 
     * tooltip_window = NULL,
     * label = NULL;
   GtkWidget * g_widget;
   GlgLong 
     lvalue,
     flipped_x,
     tooltip_x, tooltip_y;
   GtkRequisition requisition;
   GdkScreen * screen;
   GdkRectangle monitor;
   gint
     monitor_num, 
     label_width, label_height;

   if( top_viewport )
   {
      GlgGetLResource( top_viewport, (char*) "DataSlot", &lvalue );
      g_widget = (GtkWidget*) lvalue;

      if( !tooltip_window )
      {
         GtkWidget
           * vbox = NULL,
           * event_box = NULL;

         tooltip_window = gtk_window_new( GTK_WINDOW_POPUP );
         
         gtk_window_set_type_hint( GTK_WINDOW( tooltip_window ), 
                                   GDK_WINDOW_TYPE_HINT_TOOLTIP );
         gtk_window_set_resizable( GTK_WINDOW( tooltip_window ), FALSE );

         event_box = gtk_event_box_new();
         gtk_container_add( GTK_CONTAINER( tooltip_window ), event_box );
         gtk_widget_show( event_box );

         vbox = gtk_vbox_new( FALSE, 0 );
         gtk_container_add( GTK_CONTAINER( event_box ), vbox );
         gtk_widget_show( vbox );

         label = gtk_label_new( "" );
         gtk_box_pack_end( GTK_BOX( vbox ), label, 0, 0, 0 );
         gtk_widget_show( label );

         /* Tooltip border width. */
         gtk_container_set_border_width( GTK_CONTAINER( tooltip_window ), 1 );

         /* Extra spacing between the border and the label. */
         gtk_container_set_border_width( GTK_CONTAINER( vbox ), 1 );

         /* Tooltip border color. */
         GdkColor border_color = { 0, 0x0, 0x0, 0x0 };
         gtk_widget_modify_bg( tooltip_window, GTK_STATE_NORMAL, &border_color );

         /* Tooltip background color */
         GdkColor bg_color = { 0, 0xFFFF, 0xFFFF, 0xFFFF };
         gtk_widget_modify_bg( event_box, GTK_STATE_NORMAL, &bg_color );
         
         gtk_widget_realize( tooltip_window );
      }

      gtk_label_set_text( GTK_LABEL( label ), tooltip_string );
      gtk_widget_size_request( tooltip_window, &requisition );
      label_width = requisition.width;
      label_height = requisition.height;

      /* Update screen info. */
      screen = gtk_widget_get_screen( g_widget );
      if( screen != gtk_widget_get_screen( tooltip_window ) )
        gtk_window_set_screen( GTK_WINDOW( tooltip_window ), screen );      

      /* Get screen geometry */
      monitor_num = gdk_screen_get_monitor_at_point( screen, root_x, root_y );
      gdk_screen_get_monitor_geometry( screen, monitor_num, &monitor );
   
      /* If extends outside the screen, force inside */
      if( root_x + 15 + label_width + 5 < monitor.x + monitor.width )
        tooltip_x = root_x + 15; 
      else
      {
         flipped_x = root_x - label_width;
         if( flipped_x < monitor.x ||
            flipped_x + label_width + 5 > monitor.x + monitor.width )
           /* Tooltip is too long: position at left edge to use whole screen */
           tooltip_x = monitor.x;
         else
           tooltip_x = flipped_x;
      }
      
      if( root_y + 15 + label_height + 5 < 
         monitor.y + monitor.height - label_height )
        tooltip_y = root_y + 15;
      else
        tooltip_y = root_y - 10 - label_height;

      gtk_window_move( GTK_WINDOW( tooltip_window ), tooltip_x, tooltip_y );
      gtk_widget_show( tooltip_window );
   }
   else
   {
      gtk_widget_hide( tooltip_window );
   }
}

#endif

