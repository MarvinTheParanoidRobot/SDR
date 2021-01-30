/* QT - GLG integration example. 
   See README.txt for notes on using GLG C++ vs. C API.
*/

#include <stdio.h>
#include "CustomWidget.h"

#define TRACE_GLG_MESSAGES   0

extern int NumAnimationResources;
extern "C" void AnimateOneResource( GlgObject viewport, int array_index );

/*----------------------------------------------------------------------*/
CustomWidget::CustomWidget( QWidget *parent ) : QGlgWidget( parent )
{
   AnimateDrawing = True;
   startTimer( 100 );
}

/*----------------------------------------------------------------------*/
CustomWidget::~CustomWidget()
{
}

/*----------------------------------------------------------------------*/
void CustomWidget::resizeEvent( QResizeEvent * event ) 
{
   QGlgWidget::resizeEvent( event );
}

/*----------------------------------------------------------------------*/
void CustomWidget::timerEvent( QTimerEvent * event )
{
   Q_UNUSED( event );

   AnimateControlPanel();
}

/*----------------------------------------------------------------------*/
void CustomWidget::AnimateControlPanel()
{
   int i;

   /* Animate gauges in the control panel with random data. */
   if( AnimateDrawing )
   {
      for( i=0; i<NumAnimationResources; ++i )
        AnimateOneResource( viewport, i );
      
      GlgUpdate( viewport );
   }
}

/*----------------------------------------------------------------------
  This callback is invoked before the hierarchy of the widget's viewport 
  is setup.
*/
void CustomWidget::HCB( GlgObject vp )
{
   Q_UNUSED( vp );
}

/*----------------------------------------------------------------------
  This callback is invoked after the widget's viewport hierarchy has been 
  setup.
*/
void CustomWidget::VCB( GlgObject vp )
{
   Q_UNUSED( vp );
}

/*----------------------------------------------------------------------
  This callback handles input and custom selection events.
*/
void CustomWidget::InputCB( GlgObject vp, GlgObject message_obj )
{
   char
     * format,
     * action,
     * subaction,
     * origin,
     * full_origin;

   Q_UNUSED( vp );

   GlgGetSResource( message_obj, (char*) "Format", &format );
   GlgGetSResource( message_obj, (char*) "Action", &action );
   GlgGetSResource( message_obj, (char*) "SubAction", &subaction );
   GlgGetSResource( message_obj, (char*) "Origin", &origin );
   GlgGetSResource( message_obj, (char*) "FullOrigin", &full_origin );

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
void CustomWidget::SelectCB( GlgObject vp, char ** name_array )
{
   char * name;
   int i;

   Q_UNUSED( vp );

   if( !name_array )
     printf( "Nothing was selected\n" );
   else
     for( i=0; ( name = name_array[i] ); ++i )
       printf( "Selected: %s\n", name );
   
   printf( "\n" );
}

/*----------------------------------------------------------------------
  This callback handles all other native events and invoked before InputCB.
*/
void CustomWidget::TraceCB( GlgObject vp, GlgTraceCBStruct * trace_info )
{
   Q_UNUSED( vp );
   Q_UNUSED( trace_info );
}

/*----------------------------------------------------------------------
  This callback handles all other native events and invoked after InputCB.
*/
void CustomWidget::Trace2CB( GlgObject vp, GlgTraceCBStruct * trace_info )
{
   Q_UNUSED( vp );
   Q_UNUSED( trace_info );
}

/*----------------------------------------------------------------------
  This callback invoked when a SubWindow or SubDrawing object loads its 
  template drawing.
*/
void CustomWidget::HierarchyCB( GlgObject vp, 
                               GlgHierarchyCBStruct * hierarchy_info )
{
   Q_UNUSED( vp );
   Q_UNUSED( hierarchy_info );
}
