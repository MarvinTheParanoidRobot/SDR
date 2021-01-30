/* GTKMM2 - GLG integration example.
   See README.txt for notes on using GLG C++ vs. C API.
 */

#include <gtkmm/main.h>
#include "GlgExample.h"
#include "GtkmmGlgWidget.h"

int main( int argc, char * argv[] )
{
  Gtk::Main kit( argc, argv );

  /* Initialize GLG using applicable command-line options (optional). */
  GtkGlgInit( argc, argv );

  GlgExample window( argv[0] );

  kit.run( window );

  return 0;
}
