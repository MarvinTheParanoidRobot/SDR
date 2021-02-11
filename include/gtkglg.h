/* GTK3 - GLG integration example. */

#ifndef __GTK_GLG_H__
#define __GTK_GLG_H__

#include <gdk/gdk.h>
#include <gtk/gtkadjustment.h>
#include <gtk/gtkwidget.h>
#include <GlgApi.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GTK_GLG( obj ) \
   G_TYPE_CHECK_INSTANCE_CAST( obj, gtk_glg_get_type(), GtkGlg )
#define GTK_GLG_CLASS( class ) \
   GTK_CHECK_CLASS_CAST( class, gtk_glg_get_type(), GtkGlgClass )
#define GTK_IS_GLG( obj ) \
   G_TYPE_CHECK_INSTANCE_TYPE( obj, gtk_glg_get_type() )

typedef struct _GtkGlg        GtkGlg;
typedef struct _GtkGlgClass   GtkGlgClass;

struct _GtkGlg
{
  GtkWidget widget;
  
  int attached;
  int drawn;
  GlgObject viewport;
};

struct _GtkGlgClass
{
  GtkWidgetClass parent_class;
};

void gtk_glg_toolkit_init( int argc, char *argv[] );
GtkWidget * gtk_glg_new();

GType gtk_glg_get_type( void );
void gtk_glg_set_viewport( GtkWidget * glg_widget, GlgObject viewport_p );

void GlgMainEH( Window window, XtPointer scr_param, XEvent *event, 
                GlgLong compress );
#ifndef _WINDOWS
GlgLong gtk_glg_add_timer( GlgLong interval, GlgTimerProc proc, 
                           GlgAnyType client_data );
void gtk_glg_remove_timer( GlgLong timer_id );
void gtk_glg_draw_tooltip( char * tooltip_string, 
                           GlgObject viewport, GlgObject top_viewport,
                           GlgLong root_x, GlgLong root_y,
                           GlgLong win_x, GlgLong win_y );
void glg_gtk_widget_set_bg_color( GtkWidget * widget, GdkRGBA * color );
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_GLG_H__ */
