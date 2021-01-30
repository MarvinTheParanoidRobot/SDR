#include <stdio.h>
#include <QApplication>
#include "QGlgApplication.h"

/* Pass all constructors to QApplication, override only x11EventFilter 
   to pass events to GLG. */

QGlgApplication::QGlgApplication( int & argc, char ** argv ) : 
     QApplication( argc, argv )
{
}

QGlgApplication::QGlgApplication( int & argc, char ** argv, bool GUIenabled ) :
     QApplication( argc, argv, GUIenabled )
{
}

QGlgApplication::QGlgApplication( int & argc, char ** argv, Type type ) :
     QApplication( argc, argv, type )
{
}

#ifndef _WINDOWS
QGlgApplication::QGlgApplication( Display * display, Qt::HANDLE visual, 
			       Qt::HANDLE colormap ) :
     QApplication( display, visual, colormap )
{
}

QGlgApplication::QGlgApplication( Display * display, int & argc, char ** argv, 
			       Qt::HANDLE visual, Qt::HANDLE colormap )
     : QApplication( display, argc, argv, visual, colormap )
{
}
#endif

QGlgApplication::~QGlgApplication()
{
}

/*----------------------------------------------------------------------*/
void QGlgInit( int argc, char ** argv )
{
   /* Initialize GLG using applicable command-line options. */
   GlgInit( 0, NULL, argc, argv );
}

#ifndef _WINDOWS
/*----------------------------------------------------------------------
  Event filter to pass events to the GLG window. Qt is X11-based
  and there is no other way to install a widget handler in Xt-like manner.
*/
bool QGlgApplication::x11EventFilter( XEvent * event )
{
   GlgEventFilter( event );

   return QApplication::x11EventFilter( event );
}

/*----------------------------------------------------------------------*/
void QGlgApplication::GlgEventFilter( XEvent * xevent )
{
   if( xevent->xany.window )   /* Check window-based events only. */
   {
      GlgObject screen = GlgFindScreen( (GlgLong) xevent->xany.window );

      if( screen )     /* Event from one of the GLG viewports: pass to GLG. */
        GlgMainEH( xevent->xany.window, screen, xevent, True );
   }
}
#endif

