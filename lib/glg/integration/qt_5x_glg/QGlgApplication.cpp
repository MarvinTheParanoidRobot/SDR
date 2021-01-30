#include <stdio.h>
#include "QGlgApplication.h"

/* Pass all constructors to QApplication, add native event filter
   to pass events to GLG. */

QGlgApplication::QGlgApplication( int & argc, char ** argv ) : 
     QApplication( argc, argv )
{
#if !defined _WINDOWS
   GlgNativeEventFilter * glg_event_filter = new GlgNativeEventFilter();

   QApplication::eventDispatcher()->installNativeEventFilter( glg_event_filter );
#endif
}

QGlgApplication::~QGlgApplication()
{
}

/*----------------------------------------------------------------------*/
void QGlgInit( int argc, char ** argv )
{
   /* Initialize GLG using applicable command-line options. */
   GlgInit( 0, NULL, argc, argv );
}

#if !defined _WINDOWS

/*----------------------------------------------------------------------
  Event filter to pass events to the GLG window. Qt is X11-based
  and there is no other way to install a widget handler in Xt-like manner.
*/
GlgNativeEventFilter::GlgNativeEventFilter()
{
   display = (Display*)0;
}

/*----------------------------------------------------------------------*/
bool GlgNativeEventFilter::nativeEventFilter( const QByteArray &eventType, 
                                              void *message, 
                                              long *result )
{
   Q_UNUSED(eventType);
   Q_UNUSED(result);

   if( !display )    /* First time: get display. */
     display = QX11Info::display();
 
   /* If the event is processed by GLG, return True to filter it out. */
   return GlgProcessXCBEvent( display, (xEvent*) message );
}

/*----------------------------------------------------------------------*/
GlgNativeEventFilter::~GlgNativeEventFilter()
{
}

#endif
