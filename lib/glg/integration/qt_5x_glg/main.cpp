/* QT - GLG integration example. 
   See README.txt for notes on using GLG C++ vs. C API.
*/

#include <QApplication>
#include "MainWindow.h"
#include "QGlgApplication.h"

int main( int argc, char ** argv )
{
   /* Initialize GLG using applicable command-line options (optional). */
   QGlgInit( argc, argv );

   QGlgApplication a( argc, argv );

   MainWindow * mw = new MainWindow( argv[0] );
   mw->show();
   return a.exec();
}

