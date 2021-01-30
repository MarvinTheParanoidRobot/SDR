#include <QApplication>
#include <QAbstractEventDispatcher>
#include <QAbstractNativeEventFilter>
#ifndef _WINDOWS
#include <QX11Info>    /* Used with "QT += x11extras" in the project file. */
#endif
#include "GlgApi.h"

#ifndef Q_GLG_APPLICATION_H
#define Q_GLG_APPLICATION_H

class QGlgApplication: public QApplication
{
    Q_OBJECT
public:
    // methods
    QGlgApplication( int & argc, char ** argv );
    virtual ~QGlgApplication();

protected:

signals:

public slots:
};

void QGlgInit( int argc, char ** argv );

#if !defined _WINDOWS

class GlgNativeEventFilter : public QAbstractNativeEventFilter
{
public:
   Display * display;

   GlgNativeEventFilter();
   virtual ~GlgNativeEventFilter();

   bool nativeEventFilter( const QByteArray &eventType, void *message, 
                           long *result ) Q_DECL_OVERRIDE;
};

#endif

#endif
