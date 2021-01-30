#include <QMainWindow>
#include "CustomWidget.h"

class MainWindow: public QMainWindow
{
    Q_OBJECT
public:
    MainWindow( char * argv0 );
    ~MainWindow();
    void resizeEvent( QResizeEvent * event );

    CustomWidget * glg_widget;

signals:

public slots:
};
