/* GTK2 - GLG integration example. */

#include <math.h>
#include <stdio.h>
#include <gtk/gtk.h>
#ifndef _WINDOWS
#include <gdk/gdkx.h>
#else
#include <gdk/gdkwin32.h>
#endif

#include "gtkglg.h"

static GtkWidgetClass *parent_class = NULL;

static void gtk_glg_class_init( GtkGlgClass * class );
static void gtk_glg_init( GtkGlg * glg );
static void gtk_glg_destroy( GtkObject * object );
static void gtk_glg_realize( GtkWidget * widget );
static void gtk_glg_unrealize( GtkWidget * widget);
static void gtk_glg_size_allocate( GtkWidget * widget, 
                                   GtkAllocation * allocation );
static void gtk_glg_resize_viewport( GtkGlg * glg, int width, int height,
                                     int draw );
static void gtk_glg_attach_viewport( GtkGlg * glg, int attach );
#ifndef _WINDOWS
static GdkFilterReturn GlgEventFilter( GdkXEvent *xevent_p, GdkEvent *event,
                                       gpointer data );
#endif

/*----------------------------------------------------------------------
  Initialize GLG using applicable command-line options.
 */
void gtk_glg_toolkit_init( int argc, char *argv[] )
{
   GlgInit( 0, NULL, argc, argv );
}

/*----------------------------------------------------------------------*/
GType gtk_glg_get_type()
{
   static GType glg_type = 0;
   
   if( !glg_type )
   {
      static const GTypeInfo glg_info =
      {
         sizeof( GtkGlgClass ),
         NULL,  /* base_init */
         NULL,  /* base_finalize */
         (GClassInitFunc) gtk_glg_class_init,   /* class_init */
         NULL,  /* class_finalize */
         NULL,  /* class_data */
         sizeof( GtkGlg ),
         0,     /* n_preallocs */
         (GInstanceInitFunc) gtk_glg_init,      /* instance_init */
      };

      glg_type =
        g_type_register_static( GTK_TYPE_WIDGET, "GtkGlg", &glg_info, 0 );
   }

   return glg_type;
}

/*----------------------------------------------------------------------*/
static void gtk_glg_class_init( GtkGlgClass * class )
{
#ifndef _WINDOWS
   static int initialized = False;
#endif
   
   GtkObjectClass *object_class;
   GtkWidgetClass *widget_class;
   
   object_class = (GtkObjectClass*) class;
   widget_class = (GtkWidgetClass*) class;
   
   parent_class = g_type_class_ref (gtk_widget_get_type ());
   
   object_class->destroy = gtk_glg_destroy;
   
   widget_class->realize = gtk_glg_realize;
   widget_class->unrealize = gtk_glg_unrealize;
   widget_class->size_allocate = gtk_glg_size_allocate;
   
   /* Invoke again in case gtk_glg_toolkit_init() call was omitted. */
   GlgInit( 0, NULL, 0, NULL );
      
#ifndef _WINDOWS
   if( !initialized )
   {
      initialized = True;
      
      /* A global event filter used to pass events to the GLG window.
         GTK is X11-based and there is no other way to install a handler 
         on per-widget basis in Xt-like manner.
      */
      gdk_window_add_filter( (GdkWindow*)0, (GdkFilterFunc)GlgEventFilter,
                             (gpointer)0 );
      
      GlgSetAddTimerFunc( gtk_glg_add_timer );
      GlgSetRemoveTimerFunc( gtk_glg_remove_timer );
      GlgSetDrawTooltipFunc( gtk_glg_draw_tooltip );
   }
#endif
}

/*----------------------------------------------------------------------*/
static void gtk_glg_init( GtkGlg * glg )
{
   glg->attached = False;
   glg->drawn = False;
   glg->viewport = (GlgObject)0;
}

/*----------------------------------------------------------------------*/
GtkWidget * gtk_glg_new()
{
   GtkGlg * glg;
   
   glg = g_object_new( gtk_glg_get_type(), NULL );
   
   return GTK_WIDGET( glg );
}

/*----------------------------------------------------------------------*/
static void gtk_glg_destroy( GtkObject *object )
{
   GtkGlg * glg;
   
   g_return_if_fail( object != NULL );
   g_return_if_fail( GTK_IS_GLG( object ) );

   glg = GTK_GLG( object );

   gtk_glg_attach_viewport( glg, False );   /* Detach */

   GlgDropObject( glg->viewport );
   
   if( GTK_OBJECT_CLASS( parent_class )->destroy )
     (*GTK_OBJECT_CLASS( parent_class )->destroy)( object );
}

/*----------------------------------------------------------------------*/
static void gtk_glg_size_allocate( GtkWidget * widget, 
                                   GtkAllocation * allocation )
{
   GtkGlg * glg;

   g_return_if_fail( widget != NULL );
   g_return_if_fail( GTK_IS_GLG( widget ) );
   g_return_if_fail( allocation != NULL );

   widget->allocation = *allocation;
   glg = GTK_GLG( widget );

   if( gtk_widget_get_realized( widget ) )
   {
      gdk_window_move_resize( widget->window, allocation->x, allocation->y,
                              allocation->width, allocation->height );
      
      gtk_glg_resize_viewport( glg, allocation->width, allocation->height, 
                               True );
   }
}

/*----------------------------------------------------------------------*/
static void gtk_glg_realize( GtkWidget *widget )
{
   GtkGlg *glg;
   GdkWindowAttr attributes;
   gint attributes_mask;
   
   g_return_if_fail( widget != NULL );
   g_return_if_fail( GTK_IS_GLG( widget ) );

   GTK_WIDGET_SET_FLAGS( widget, GTK_REALIZED );
   glg = GTK_GLG( widget );
   
   attributes.x = widget->allocation.x;
   attributes.y = widget->allocation.y;
   attributes.width = widget->allocation.width;
   attributes.height = widget->allocation.height;
   attributes.wclass = GDK_INPUT_OUTPUT;
   attributes.window_type = GDK_WINDOW_CHILD;
   attributes.event_mask = gtk_widget_get_events( widget );
   attributes.visual = gtk_widget_get_visual( widget );
   attributes.colormap = gtk_widget_get_colormap( widget );

   attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
   widget->window = 
     gdk_window_new(widget->parent->window, &attributes, attributes_mask );
   
   widget->style = gtk_style_attach( widget->style, widget->window );
   
   gdk_window_set_user_data( widget->window, widget );

   gtk_style_set_background( widget->style, widget->window, GTK_STATE_ACTIVE );

   gtk_glg_attach_viewport( glg, True );
   gtk_glg_resize_viewport( glg, widget->allocation.width, 
                            widget->allocation.height, True );
}

/*----------------------------------------------------------------------*/
static void gtk_glg_unrealize( GtkWidget * widget )
{
   GtkGlg * glg = GTK_GLG( widget );

   gtk_glg_attach_viewport( glg, False );    /* Detach */
   
   glg->attached = False;
   glg->drawn = False;
   
   GTK_WIDGET_CLASS( parent_class )->unrealize( widget );
}

/*----------------------------------------------------------------------*/
void gtk_glg_set_viewport( GtkWidget * widget, GlgObject viewport_p )
{
   GtkGlg * glg = GTK_GLG( widget );

   gtk_glg_attach_viewport( glg, False );   /* Detach old */

   GlgDropObject( glg->viewport );
   glg->viewport = GlgReferenceObject( viewport_p );

   if( glg->viewport )
   {
      GtkWidget * widget = GTK_WIDGET( glg );

      if( gtk_widget_get_realized( widget ) )
      {
	 gtk_glg_attach_viewport( glg, True );    /* Attach new */
	 gtk_glg_resize_viewport( glg, widget->allocation.width, 
                                  widget->allocation.height, True );
      }
   }
}

/*----------------------------------------------------------------------*/
static void gtk_glg_attach_viewport( GtkGlg * glg, int attach ) 
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

   if( !glg->viewport )
     return;

   if( attach )   /* Attach */
   {
      if( !glg->attached )
      {
	 /* Attach Glg viewport to the widget using widget's window as a 
	    parent. */
	 widget = GTK_WIDGET( glg );
	 gdk_window = GDK_WINDOW( widget->window );

#ifndef _WINDOWS
	 win_id = GDK_WINDOW_XID( gdk_window );
	 display = GDK_WINDOW_XDISPLAY( gdk_window );
	 gdk_screen = gtk_widget_get_screen( widget );
	 x_screen = GDK_SCREEN_XSCREEN( gdk_screen );
      	 GlgSetParentWidget( glg->viewport, display, x_screen, (void*) win_id );
#else
	 win_id = GDK_WINDOW_HWND( gdk_window );
	 GlgSetParentWidget( glg->viewport, NULL, 0,
			    (void*) win_id );   
#endif
	 GlgReferenceObject( glg->viewport );
         GlgSetLResource( glg->viewport, "DataSlot", (GlgLong)glg );
	 glg->attached = True;
      }
   }
   else   /* Detach */
   {
      if( glg->attached )
      {
	 GlgResetHierarchy( glg->viewport );
	 GlgDropObject( glg->viewport );
         GlgSetLResource( glg->viewport, "DataSlot", 0 );
	 glg->attached = False;
      }
   }
}

/*----------------------------------------------------------------------*/
void gtk_glg_resize_viewport( GtkGlg * glg, int width, int height, int draw )
{
   if( !glg->viewport || !glg->attached )
     return;
   
   GlgSetGeometry( glg->viewport, 0, 0, width, height );
   
   if( !draw )
     return;

   /* Display the viewport is requested. */
   if( !glg->drawn )
   {
      /* Setup and draw the first time */
      GlgInitialDraw( glg->viewport );
      glg->drawn = True;
   }
   else
     /* Update to redraw after a size change. */
     GlgUpdate( glg->viewport );
}

#ifndef _WINDOWS
/*----------------------------------------------------------------------*/
static GdkFilterReturn GlgEventFilter( GdkXEvent *xevent_p, GdkEvent *event,
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
static gint timer_proc( gpointer data )
{
   GlgTimerData * timer_data = (GlgTimerData*) data;
   
   (*timer_data->proc)( timer_data->client_data, NULL );

   g_source_remove( timer_data->timer_id );    /* Remove timer */
   GlgFree( timer_data );                      /* Free timer data */
   return False;
}

/*----------------------------------------------------------------------*/
GlgLong gtk_glg_add_timer( GlgLong interval, GlgTimerProc proc, 
                           GlgAnyType client_data )
{
   GlgTimerData * timer_data;

   timer_data = GlgAlloc( sizeof( *timer_data ) );
   timer_data->proc = proc;
   timer_data->client_data = client_data;

   timer_data->timer_id = 
     g_timeout_add( (guint32) interval, timer_proc, (gpointer) timer_data );   

   return (GlgLong) timer_data;
}

/*----------------------------------------------------------------------*/
void gtk_glg_remove_timer( GlgLong timer_id )
{
   GlgTimerData * timer_data = (GlgTimerData*) timer_id;
   
   g_source_remove( timer_data->timer_id );
   GlgFree( timer_data );     
}

/*----------------------------------------------------------------------*/
void gtk_glg_draw_tooltip( char * tooltip_string, 
                           GlgObject viewport, GlgObject top_viewport,
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
      GlgGetLResource( top_viewport, "DataSlot", &lvalue );
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
