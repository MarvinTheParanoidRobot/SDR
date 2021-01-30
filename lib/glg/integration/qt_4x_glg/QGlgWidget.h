#include <QWidget>
#include "GlgApi.h"

#ifndef Q_GLG_WIDGET_H
#define Q_GLG_WIDGET_H

#ifndef False
#define False    0
#endif

#ifndef True
#define True     1
#endif

typedef void (*QGlgCallback)( class QGlgWidget * );
typedef enum _QGlgCallbackType
{
   Q_GLG_UNDEFINED_CB = 0,
   Q_GLG_H_CB,
   Q_GLG_V_CB
} QGlgCallbackType;

class QGlgWidget: public QWidget
{
    Q_OBJECT
public:
    // variables
    int displayed;
    GlgObject viewport;

    // methods
    QGlgWidget( QWidget * );
    virtual ~QGlgWidget();
    void SetViewport( GlgObject );
    void ResizeViewport( int, int, int );
    void EnableCallback( GlgCallbackType callback_type );

    /* Callbacks to be overridden by subclasses. */
    virtual void InputCB( GlgObject vp, GlgObject message );
    virtual void SelectCB( GlgObject vp, char ** name_array );
    virtual void TraceCB( GlgObject vp, GlgTraceCBStruct * trace_info );
    virtual void Trace2CB( GlgObject vp, GlgTraceCBStruct * trace_info );
    virtual void HierarchyCB( GlgObject vp, 
                             GlgHierarchyCBStruct * hierarchy_info );
    virtual void HCB( GlgObject vp );
    virtual void VCB( GlgObject vp );

    /**** Deprecated methods and variables used by AddCallback(), 
      supreseded by EnableCallback(). */
    QGlgCallback h_callback;
    QGlgCallback v_callback;
    void AddCallback( QGlgCallbackType callback_type, QGlgCallback callback );

protected:
    GlgBoolean enable_input_cb;
    GlgBoolean enable_select_cb;
    GlgBoolean enable_trace_cb;
    GlgBoolean enable_trace2_cb;
    GlgBoolean enable_hierarchy_cb;
    void resizeEvent ( QResizeEvent * );

signals:

public slots:
};

#endif

