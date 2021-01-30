/* GTK2 - GLG integration example. */

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "gtkglg.h"

#define SIMPLE             0  /* If SIMPLE==1, use just one top-level window */
#define UPDATE_INTERVAL    50 /* millisec */ 
#define TRACE_GLG_MESSAGES 0

extern int NumAnimationResources;
static GlgBoolean AnimateDrawing = True;

void Input( GlgObject, GlgAnyType, GlgAnyType );
void Select( GlgObject, GlgAnyType, GlgAnyType );

static gint AnimateControlPanel( gpointer data );
void AnimateOneResource( Object viewport, int array_index );

/*----------------------------------------------------------------------*/
int main( int argc, char *argv[])
{
  GtkWidget * window;
  GtkWidget * glg;
#if !SIMPLE
  GtkWidget * frame;
  GtkWidget * vbox;
  GtkWidget * label;
#endif
  GlgObject viewport;
  char * full_path;

  gtk_init( &argc, &argv );

  gtk_glg_toolkit_init( argc, argv );

  window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
  gtk_window_set_default_size( GTK_WINDOW( window ), 800, 700 );

  gtk_window_set_title( GTK_WINDOW( window ), "GLG-GTK Integration Example" ); 
  g_signal_connect( G_OBJECT( window ), "destroy", G_CALLBACK( exit ), NULL );
  
  gtk_container_set_border_width( GTK_CONTAINER( window ), 10 );

  glg = gtk_glg_new();
  gtk_widget_set_size_request( glg, 400, 350 );

  /* Set search path for locating drawing files to the ../drawing directory
     relative to the location of the executable.
  */
  full_path = GlgCreateRelativePath( argv[0], "../drawings", False, False );
  GlgSetSResource( NULL, "$config/GlgSearchPath", full_path );
  GlgFree( full_path );

  viewport = GlgLoadWidgetFromFile( "controls.g" );
  GlgAddCallback( viewport, GLG_INPUT_CB, (GlgCallbackProc) Input, NULL );
  GlgAddCallback( viewport, GLG_SELECT_CB, (GlgCallbackProc) Select, NULL );

  gtk_glg_set_viewport( glg, viewport );
  GlgDropObject( viewport );
  
#if !SIMPLE
  vbox = gtk_vbox_new( FALSE, 5 );
  gtk_container_add( GTK_CONTAINER( window ), vbox );
  gtk_widget_show( vbox );

  frame = gtk_frame_new( NULL );
  gtk_frame_set_shadow_type( GTK_FRAME( frame ), GTK_SHADOW_IN );
  gtk_container_add( GTK_CONTAINER( vbox ), frame );
  gtk_widget_show( frame ); 
  gtk_container_add( GTK_CONTAINER( frame ), glg );

  label = gtk_label_new( "GLG Integraton Example" );
  gtk_box_pack_end( GTK_BOX( vbox ), label, 0, 0, 0 );
  gtk_widget_show( label );
#else
  gtk_container_add( GTK_CONTAINER( window ), glg );
#endif

  gtk_widget_show( glg );
  gtk_widget_show( window );
  
  /* Add timer to update control panel with data. */
  g_timeout_add( (guint32) UPDATE_INTERVAL, AnimateControlPanel, 
                 (gpointer) viewport );   

  gtk_main();
  
  return 0;
}

/*----------------------------------------------------------------------*/
static gint AnimateControlPanel( gpointer data )
{
   GlgObject viewport = (GlgObject) data;
   int i;

   /* Animate gauges in the control panel with random data. */
   if( AnimateDrawing )
   {
      for( i=0; i<NumAnimationResources; ++i )
        AnimateOneResource( viewport, i );
      
      GlgUpdate( viewport );
      GlgSync( viewport );
   }

   /* Reinstall the timer */
   g_timeout_add( (guint32) UPDATE_INTERVAL, AnimateControlPanel, 
                  (gpointer) viewport );   

   return False;
}

/*----------------------------------------------------------------------
  This callback handles input and custom selection events.
*/
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{
   GlgObject message_obj;
   char
     * format,
     * action,
     * subaction,
     * origin,
     * full_origin;

   message_obj = (GlgObject) call_data;

   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "SubAction", &subaction );
   GlgGetSResource( message_obj, "Origin", &origin );
   GlgGetSResource( message_obj, "FullOrigin", &full_origin );

#if TRACE_GLG_MESSAGES
   printf( "Format: %s\n", format );
   printf( "Action: %s\n", action );
   printf( "SubAction: %s\n", subaction );
   printf( "Origin: %s\n", origin );
   printf( "FullOrigin: %s\n\n", full_origin );
#endif

   if( strcmp( format, "Button" ) == 0 )
   {
      if( strcmp( action, "Activate" ) == 0 )
      {
         if( strcmp( origin, "Start" ) == 0 )
           AnimateDrawing = True;
         else if( strcmp( origin, "Stop" ) == 0 )
           AnimateDrawing = False;
      }
   }

   GlgUpdate( viewport );
}

/*----------------------------------------------------------------------
  This callback handles simple selection by a mouse click.

  For more elaborate selection techniques for both mouse click
  and mouse over, a custom selection events can be attached to
  object in the draiwng in the GLG Buildder, and then handled in
  the Input callback, as shown in the GlgObjectSelectionG.c example.
*/
void Select( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{
   char ** name_array;
   char * name;
   long i;

   name_array = (char **) call_data;

   if( !name_array )
     printf( "Nothing was selected\n" );
   else
     for( i=0; ( name = name_array[i] ); ++i )
       printf( "Selected: %s\n", name );
   
   printf( "\n" );
}
