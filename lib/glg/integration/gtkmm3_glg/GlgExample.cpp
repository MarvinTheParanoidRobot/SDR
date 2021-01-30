/* GTKMM2 - GLG integration example.
   See README.txt for notes on using GLG C++ vs. C API.
 */

#include "GlgExample.h"
#include "CustomWidget.h"
#include "GlgApi.h"

/*----------------------------------------------------------------------*/
GlgExample::GlgExample( char * argv0 ) : m_Button_Quit( "Quit" )
{
  set_title( "GLG Widget example" );
  set_border_width( 6 );
  set_default_size( 800, 700 );

  add( m_VBox );
  
  /* CustomWidget is subclassed from the stock GtkmmGlgWidget and provides
     custom functionality to animate the drawing and handle user intercation.
  */
  m_GtkmmGlgWidget = new CustomWidget();

  /* Set search path for locating drawing files to the ../drawing directory
     relative to the location of the executable.
  */
  char * full_path = 
    GlgCreateRelativePath( argv0, (char*) "../drawings", False, False );
  GlgSetSResource( NULL, (char*) "$config/GlgSearchPath", full_path );
  GlgFree( full_path );

  GlgObject viewport = GlgLoadWidgetFromFile( (char*) "controls.g" );

  /* Enable input and select callbacks */
  m_GtkmmGlgWidget->EnableCallback( GLG_INPUT_CB );
  m_GtkmmGlgWidget->EnableCallback( GLG_SELECT_CB );

  m_GtkmmGlgWidget->SetViewport( viewport );
  GlgDropObject( viewport );

  m_VBox.pack_start( m_ButtonBox, Gtk::PACK_SHRINK );
  m_VBox.pack_start( *m_GtkmmGlgWidget, Gtk::PACK_EXPAND_WIDGET );

  m_ButtonBox.pack_start( m_Button_Quit, Gtk::PACK_SHRINK );
  m_ButtonBox.set_border_width( 6 );
  m_ButtonBox.set_layout( Gtk::BUTTONBOX_END );
  m_Button_Quit.signal_clicked().
    connect( sigc::mem_fun( *this, &GlgExample::on_button_quit ) );

  show_all_children();
}

/*----------------------------------------------------------------------*/
GlgExample::~GlgExample()
{
}

/*----------------------------------------------------------------------*/
void GlgExample::on_button_quit()
{
   hide();
}
