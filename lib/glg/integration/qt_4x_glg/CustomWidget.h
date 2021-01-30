#include "QGlgWidget.h"
#include "GlgApi.h"

#ifndef CUSTOM_WIDGET_H
#define CUSTOM_WIDGET_H

class CustomWidget: public QGlgWidget
{
public:
    // variables
    GlgBoolean AnimateDrawing;

    // methods
    CustomWidget( QWidget * );
    ~CustomWidget();

    void timerEvent( QTimerEvent * event );
    void AnimateControlPanel( void );

    /* Callbacks to be overridden by subclasses. */
    virtual void InputCB( GlgObject vp, GlgObject message );
    virtual void SelectCB( GlgObject vp, char ** name_array );
    virtual void TraceCB( GlgObject vp, GlgTraceCBStruct * trace_info );
    virtual void Trace2CB( GlgObject vp, GlgTraceCBStruct * trace_info );
    virtual void HierarchyCB( GlgObject vp, 
                             GlgHierarchyCBStruct * hirerachy_info );
    virtual void HCB( GlgObject vp );
    virtual void VCB( GlgObject vp );

protected:
    void resizeEvent ( QResizeEvent * );

signals:

public slots:
};

#endif

