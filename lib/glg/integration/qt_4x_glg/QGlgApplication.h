#include <QApplication>
#include "GlgApi.h"

#ifndef Q_GLG_APPLICATION_H
#define Q_GLG_APPLICATION_H

class QGlgApplication: public QApplication
{
    Q_OBJECT
public:
    // methods
    QGlgApplication( int & argc, char ** argv );
    QGlgApplication( int & argc, char ** argv, bool GUIenabled );
    QGlgApplication( int & argc, char ** argv, Type type );
#ifndef _WINDOWS
    QGlgApplication( Display * display, Qt::HANDLE visual = 0, 
		  Qt::HANDLE colormap = 0 );
    QGlgApplication( Display * display, int & argc, char ** argv, 
		  Qt::HANDLE visual = 0, Qt::HANDLE colormap = 0 );
#endif
    virtual ~QGlgApplication();

#ifndef _WINDOWS
    bool x11EventFilter( XEvent * event );
    void GlgEventFilter( XEvent * );
#endif

protected:

signals:

public slots:
};

void QGlgInit( int argc, char ** argv );

#ifndef _WINDOWS
extern "C"
{
   void GlgMainEH( Window, void*, XEvent*, GlgLong );
}
#endif

#endif

