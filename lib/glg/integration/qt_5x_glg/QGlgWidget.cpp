/* QT - GLG integration example. 
   See README.txt for notes on using GLG C++ vs. C API.
*/

#include <stdio.h>
#include <QApplication>
#include <QResizeEvent>
#include <QBasicTimer>
#include <QPoint>
#include <QToolTip>
#include <QSizePolicy>
#include "QGlgWidget.h"

#if !defined _WINDOWS
#include <QX11Info>

extern "C" GlgLong QGlgAddTimer( GlgLong interval, GlgTimerProc proc, 
                                 GlgAnyType client_data );
extern "C" void QGlgRemoveTimer( GlgLong timer_id );
extern "C" void QGlgDrawTooltip( char * tooltip_string, 
                                 GlgObject viewport, GlgObject top_viewport,
                                 GlgLong root_x, GlgLong root_y,
                                 GlgLong win_x, GlgLong win_y );

#define DEBUG_X_ERRORS   0

#if DEBUG_X_ERRORS
void TrapXError( Display * display );
#endif

#endif

/*----------------------------------------------------------------------*/
QGlgWidget::QGlgWidget( QWidget * parent ) : QWidget( parent, 0 )
{
   static int initialized = False;

   if( !initialized )
   {
      initialized = True;

      /* Invoke again in case QGlgInit() call was omitted. */
      GlgInit( 0, NULL, 0, NULL );

#if !defined _WINDOWS
      GlgSetAddTimerFunc( QGlgAddTimer );
      GlgSetRemoveTimerFunc( QGlgRemoveTimer );
      GlgSetDrawTooltipFunc( QGlgDrawTooltip );
#endif

#if DEBUG_X_ERRORS
      TrapXError( QX11Info::display() );
#endif
   }

   displayed = False;
   viewport = (GlgObject)0;

   enable_input_cb = False;
   enable_select_cb = False;
   enable_trace_cb = False;
   enable_trace2_cb = False;
   enable_hierarchy_cb = False;

   h_callback = (QGlgCallback)0;
   v_callback = (QGlgCallback)0;

   /* Set minimum width and height to make sure the widget is visible when
      a layout manager is used and no widget minimum size is defined, 
      otherwise the width/height of zero might be supplied by the layout 
      manager, rendering the widget invisible.
      */
   setMinimumHeight( 10 );
   setMinimumWidth( 10 );
   setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

/*----------------------------------------------------------------------*/
QGlgWidget::~QGlgWidget()
{
   if( displayed )
     GlgResetHierarchy( viewport );
   GlgDropObject( viewport );
}

/*----------------------------------------------------------------------*/
void QGlgWidget::resizeEvent( QResizeEvent * event ) 
{
   QWidget::resizeEvent( event );

   if( !viewport )
     return;

   /* Set the viewport size to match widget's size. */
   QSize size = event->size();
   ResizeViewport( size.width(), size.height(), True );
}

/*----------------------------------------------------------------------
| To reparent, set null viewport, reparent, then set the viewport back
| (dynamic changes will be lost and will have to be recreated). Rererence
| the viewport for the duration of reparenting to keep it around, and 
| drop/deref when finished.
*/
void QGlgWidget::SetViewport( GlgObject viewport_p ) 
{
#if !defined _WINDOWS
   Window win_id;
   Display * display;
   Screen * x_screen;
#else
   HWND win_id;
#endif
   int was_displayed = displayed;

   if( displayed )
   {
      GlgResetHierarchy( viewport );
      displayed = False;
   }

   if( viewport != viewport_p )
   {
      if( viewport )
      {
         GlgSetLResource( viewport, (char*) "DataSlot", 0 );
         GlgDropObject( viewport );      
      }

      viewport = GlgReferenceObject( viewport_p );
   }

   if( !viewport )
     return;

   /* Attach Glg viewport to the widget using widget's window as a 
      parent. */
#if !defined _WINDOWS
   win_id = (Window) winId();
   display = QX11Info::display();
   x_screen = ScreenOfDisplay( display, QX11Info::appScreen() );
      
   GlgSetParentWidget( viewport, display, x_screen, (void*) win_id );
#else
   win_id = (HWND) winId();
   GlgSetParentWidget( viewport, NULL, 0, (void*) win_id );
#endif
   
   GlgSetLResource( viewport, (char*) "DataSlot", (GlgLong)this );

   QSize size = this->size();
   ResizeViewport( size.width(), size.height(), was_displayed );
}

/*----------------------------------------------------------------------
  Stubs for implementing C++ callbacks.
*/
extern "C" void 
  QGlgWidgetInputStub( GlgObject callback_viewport,
                      GlgAnyType client_data, GlgAnyType call_data )
{
   QGlgWidget * initiator = (QGlgWidget*)client_data;

   initiator->InputCB( callback_viewport, (GlgObject)call_data );
}

extern "C" void 
  QGlgWidgetSelectStub( GlgObject callback_viewport,
                       GlgAnyType client_data, GlgAnyType call_data )
{
   QGlgWidget * initiator = (QGlgWidget*)client_data;

   initiator->SelectCB( callback_viewport, (char **)call_data );
}

extern "C" void 
  QGlgWidgetTraceStub( GlgObject callback_viewport,
                      GlgAnyType client_data, GlgAnyType call_data )
{
   QGlgWidget * initiator = (QGlgWidget*)client_data;

   initiator->TraceCB( callback_viewport, (GlgTraceCBStruct*) call_data );
}

extern "C" void 
  QGlgWidgetTrace2Stub( GlgObject callback_viewport,
                       GlgAnyType client_data, GlgAnyType call_data )
{
   QGlgWidget * initiator = (QGlgWidget*)client_data;

   initiator->Trace2CB( callback_viewport, (GlgTraceCBStruct*) call_data );
}

extern "C" void 
  QGlgWidgetHierarchyStub( GlgObject callback_viewport,
                          GlgAnyType client_data, GlgAnyType call_data )
{
   QGlgWidget * initiator = (QGlgWidget*)client_data;

   initiator->HierarchyCB( callback_viewport, 
                          (GlgHierarchyCBStruct*) call_data );
}

/*----------------------------------------------------------------------*/
void QGlgWidget::ResizeViewport( int width, int height, int draw )
{
   GlgSetGeometry( viewport, 0, 0, width, height );
   
   if( !draw )
     return;

   if( !displayed )
   {
      /* Setup and draw the first time */
      displayed = True;

      /* Add callbacks if enabled. */
      if( enable_input_cb )
        GlgAddCallback( viewport, GLG_INPUT_CB, 
                       (GlgCallbackProc)QGlgWidgetInputStub, 
                       (GlgAnyType)this );

      if( enable_select_cb )
        GlgAddCallback( viewport, GLG_SELECT_CB,
                       (GlgCallbackProc)QGlgWidgetSelectStub, 
                       (GlgAnyType)this );

      if( enable_trace_cb )
        GlgAddCallback( viewport, GLG_TRACE_CB,
                       (GlgCallbackProc)QGlgWidgetTraceStub,
                       (GlgAnyType)this );
      if( enable_trace2_cb )
        GlgAddCallback( viewport, GLG_TRACE2_CB,
                       (GlgCallbackProc)QGlgWidgetTrace2Stub, 
                       (GlgAnyType)this );

      if( enable_hierarchy_cb )
        GlgAddCallback( viewport, GLG_HIERARCHY_CB,
                       (GlgCallbackProc)QGlgWidgetHierarchyStub, 
                       (GlgAnyType)this );

      HCB( viewport );   /* New H callback */     

      if( h_callback )
        (*h_callback )( this );     /* Deprecated callback */


      GlgSetupHierarchy( viewport );
      
      VCB( viewport );   /* New V callback */     

      if( v_callback )
        (*v_callback )( this );     /* Deprecated callback */

      GlgUpdate( viewport );
   }
   /* else: Update to redraw after a size change. */

   GlgUpdate( viewport );
}

/*----------------------------------------------------------------------*/
void QGlgWidget::EnableCallback( GlgCallbackType callback_type )
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

/* Callbacks to be overriden by subclasses. */
void QGlgWidget::InputCB( GlgObject vp, GlgObject message )
{
   Q_UNUSED( vp );
   Q_UNUSED( message );
}

void QGlgWidget::SelectCB( GlgObject vp, char ** name_array )
{
   Q_UNUSED( vp );
   Q_UNUSED( name_array );
}

void QGlgWidget::TraceCB( GlgObject vp, GlgTraceCBStruct * trace_info )
{
   Q_UNUSED( vp );
   Q_UNUSED( trace_info );
}

void QGlgWidget::Trace2CB( GlgObject vp, GlgTraceCBStruct * trace_info )
{
   Q_UNUSED( vp );
   Q_UNUSED( trace_info );
}

void QGlgWidget::HierarchyCB( GlgObject vp, 
                             GlgHierarchyCBStruct * hierarchy_info )
{
   Q_UNUSED( vp );
   Q_UNUSED( hierarchy_info );
}

void QGlgWidget::HCB( GlgObject vp )
{
   Q_UNUSED( vp );
}

void QGlgWidget::VCB( GlgObject vp )
{
   Q_UNUSED( vp );
}

/**** Deprecated, superseded by EnableCallback(). ****/
/*----------------------------------------------------------------------*/
void QGlgWidget::AddCallback( QGlgCallbackType callback_type, 
                             QGlgCallback callback )
{
   switch( callback_type )
   {
    case Q_GLG_H_CB: h_callback = callback; break;
    case Q_GLG_V_CB: v_callback = callback; break;
    default: GlgError( GLG_WARNING, (char*) "Invalid callback type." ); break;
   }
}

#if !defined _WINDOWS

/* Callbacks to enable timer xforms and tooltips. */

/*----------------------------------------------------------------------*/
class QGlgTimer : public QObject
{
public:

   QGlgTimer( GlgTimerProc proc_p, void * client_data_p );
   ~QGlgTimer( void );

   void timerEvent( QTimerEvent * event );
   GlgTimerProc proc;
   void * client_data;
   int id;
};

QGlgTimer::QGlgTimer( GlgTimerProc proc_p, void * client_data_p )
{
   proc = proc_p;
   client_data = client_data_p;
}

QGlgTimer::~QGlgTimer( void ){}

void QGlgTimer::timerEvent( QTimerEvent * event )
{
   Q_UNUSED( event );

   killTimer( id );
   (*proc)( client_data, NULL );

   proc = NULL;
   client_data = NULL;

   delete this;
}

/*----------------------------------------------------------------------*/
extern "C" GlgLong QGlgAddTimer( GlgLong interval, GlgTimerProc proc, 
                                GlgAnyType client_data )
{
   QGlgTimer * timer = new QGlgTimer( proc, client_data );

   timer->id = timer->startTimer( (int) interval );
   return (GlgLong)timer;
}

/*----------------------------------------------------------------------*/
extern "C" void QGlgRemoveTimer( GlgLong timer_id )
{
   QGlgTimer * timer = (QGlgTimer*) timer_id;
   timer->killTimer( timer->id );
   delete timer;
}

/*----------------------------------------------------------------------*/
extern "C" void QGlgDrawTooltip( char * tooltip_string, 
                                GlgObject viewport, GlgObject top_viewport,
                                GlgLong root_x, GlgLong root_y,
                                GlgLong win_x, GlgLong win_y )
{
   static QGlgWidget * q_widget = NULL;  /* Used to determine screen info */
   GlgLong lvalue;

   Q_UNUSED( viewport );
   Q_UNUSED( win_x );
   Q_UNUSED( win_y );

   if( top_viewport )
   {
      GlgGetLResource( top_viewport, (char*) "DataSlot", &lvalue );
      q_widget = (QGlgWidget*)lvalue;

      QString q_string( tooltip_string );
      QPoint q_point( (int)root_x, (int)root_y );
   
      QToolTip::showText( q_point, q_string, q_widget );
   }
   else
      QToolTip::hideText();
}

#endif

#if DEBUG_X_ERRORS
static int (*old_handler)( Display *, XErrorEvent * );

int MyXErrorHandler( Display * display, XErrorEvent * myerr )
{
   char msg[1000];
   
   XGetErrorText( display, myerr->error_code, msg, 1000 );
   fprintf( stderr, "XError\nFailed X request code: %d\n%s\n",
           myerr->request_code, msg );
   
   printf("X error, handing off to the previous handler\n");
   return (*old_handler)( display, myerr );
}

void TrapXError( Display * display )
{
   XSynchronize( display, True );
   old_handler = XSetErrorHandler( MyXErrorHandler );
}
  
#endif
