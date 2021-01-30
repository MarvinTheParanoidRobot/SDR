/* QT - GLG integration example. 
   See README.txt for notes on using GLG C++ vs. C API.
*/

#include <stdio.h>
#include <QApplication>
#include <QResizeEvent>
#include "MainWindow.h"

#define WIDTH  800
#define HEIGHT 700
#define GAP     10
#define MIN     10

/*----------------------------------------------------------------------*/
MainWindow::MainWindow( char * argv0 ) : QMainWindow( 0, 0 )
{
   setAttribute( Qt::WA_DeleteOnClose, true );

   /* CustomWidget is a subclass of QGlgWidget that implements custom 
      initialization and event callbacks (HCB, VCB, SelectCB, InpitCB).
   */
   glg_widget = new CustomWidget( this );
   glg_widget->move( GAP, GAP );

   /* Set search path for locating drawing files to the ../drawing directory
      relative to the location of the executable.
   */
   char * full_path = 
     GlgCreateRelativePath( argv0, (char*) "../drawings", False, False );
   GlgSetSResource( NULL, (char*) "$config/GlgSearchPath", full_path );
   GlgFree( full_path );

   /* Load drawing */
   GlgObject viewport = GlgLoadWidgetFromFile( (char*) "controls.g" );

   /* Enable input and select callbacks */
   glg_widget->EnableCallback( GLG_INPUT_CB );
   glg_widget->EnableCallback( GLG_SELECT_CB );

   glg_widget->SetViewport( viewport );
   GlgDropObject( viewport );
   
   resize( WIDTH, HEIGHT );
}

/*----------------------------------------------------------------------*/
void MainWindow::resizeEvent( QResizeEvent * event )
{
   QMainWindow::resizeEvent( event );

   // Manually resize the widget inside the main window to occupy all space 
   // less the gap. Alternatively, a layout manager is used to manage the 
     // size.
       
   QSize size = event->size();

   int w = size.width();
   int h = size.height();

   w -= 2 * GAP;
   if( w < 0 )
     w = MIN;

   h -= 2 * GAP;
   if( h < 0 )
     h = MIN;

   glg_widget->resize( w, h );
}

/*----------------------------------------------------------------------*/
MainWindow::~MainWindow()
{
}

