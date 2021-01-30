/* GTKMM2 - GLG integration example. */

#ifndef GTKMM_GLG_EXAMPLE_H
#define GTKMM_GLG_EXAMPLE_H

#include <gtkmm.h>
#include "GtkmmGlgWidget.h"

class GlgExample : public Gtk::Window
{
public:
  GlgExample( char * argv0 );
  virtual ~GlgExample();

protected:
  // Signal handlers:
  virtual void on_button_quit();

  // Child widgets:
  Gtk::VBox m_VBox;
  GtkmmGlgWidget * m_GtkmmGlgWidget;
  Gtk::HButtonBox m_ButtonBox;
  Gtk::Button m_Button_Quit;
};

#endif
