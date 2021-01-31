/*------------------------------------------------------------------------
|  This example demonstrates how to interact with a Glg drawing
|  containing a panel of GLG controls. The drawing has been composed in
|  the GlgBuilder and contains several Glg control widgets.  The top
|  level viewport of the drawing, the $Widget viewport, contains
|  children viewports named "Dial", "Slider" and "QuitButton" that
|  represent a dial, a slider and a button Glg controls. These controls
|  were inserted into the drawing in the GlgBuilder and their viewports
|  were renamed from $Widget to "Dial", "Slider" and "QuitButton"
|  respectively.
|
|  The drawing name is "controls.g". User interaction in the GLG
|  controls  is handled in the Input callback.
-------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include "GlgClass.h"

#define WIDTH     600
#define HEIGHT    600

XtAppContext AppContext;         // Appplication context

class ControlPanelExample : public GlgWrapperC
{
 private:
   // Disallow assigments and copying a widget
   ControlPanelExample& operator= ( const ControlPanelExample& object );
   ControlPanelExample( ControlPanelExample& object );

 public:

   ControlPanelExample( void );
   virtual ~ControlPanelExample( void );

   // Override to supply custom Input method
   void Input( GlgObjectC& callback_viewport, GlgObjectC& message );

   // Initialize drawing's parameters.
   void InitializeDrawing();

 public:
   // Initial parameters for a dial and slider controls.
   double LowRange;
   double HighRange;
   double InitValue;
};

//////////////////////////////////////////////////////////////////////////
// Main Entry point.
//////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] )
{
   Display * display;
   Widget shell, form;
   Cardinal ac;
   Arg al[20];
   ControlPanelExample control_panel;
 
   // Initialize X Toolkit and create an application context.
   XtToolkitInitialize();
   AppContext = XtCreateApplicationContext();
   
   // Open a display connection.
   display =
     XtOpenDisplay( AppContext, 0, "GlgControlPanel", "Glg", 0, 0, &argc, argv );
   
   // Create a shell.
   ac = 0;
   XtSetArg( al[ac], XtNbaseWidth, WIDTH ); ac++;
   XtSetArg( al[ac], XtNbaseHeight, HEIGHT ); ac++;
   XtSetArg( al[ac], XtNwidth, WIDTH ); ac++;
   XtSetArg( al[ac], XtNheight, HEIGHT ); ac++;
   XtSetArg( al[ac], XtNminWidth, 10 ); ac++;
   XtSetArg( al[ac], XtNminHeight, 10 ); ac++;
   XtSetArg( al[ac], XtNallowShellResize, True ); ac++;
   XtSetArg( al[ac], XtNinput, TRUE ); ac++;
   shell =
     XtAppCreateShell( "GlgExample", "Glg", applicationShellWidgetClass,
		      display, al, ac );
   
   // Create a GLG widget and specify a drawing filename to be displayed
   control_panel.Create( "controls.g", shell );
   
   // Enable Input callback
   control_panel.EnableCallback( GLG_INPUT_CB );
         
   XtRealizeWidget( shell );

   control_panel.GetViewport();   // Get the viewport object after realizing.
   control_panel.InitializeDrawing();

   XtAppMainLoop( AppContext );
}

/////////////////////////////////////////////////////////////////////////////
// Set initial parameters for GLG objects/widgets.
/////////////////////////////////////////////////////////////////////////////
void ControlPanelExample::InitializeDrawing()
{
   // Disable InputHandler for a dial control so that the dial acts
     // only as an output device and doesn't respond to user input.
   SetResource( "Dial/HandlerDisabled", 1. );

   // Set High and Low range resources for both Slider and Dial
   SetResource( "Dial/Low", LowRange );
   SetResource( "Slider/Low", LowRange );
   
   SetResource( "Dial/High", HighRange );
   SetResource( "Slider/High", HighRange );
   
   // Set initial values of a dial and a slider
   SetResource( "Dial/Value", InitValue );
   SetResource( "Slider/ValueY", InitValue );
}

/////////////////////////////////////////////////////////////////////////////
// This callback is invoked when user interacts with input objects in GLG
// drawing, such as a slider, dial or a button.
/////////////////////////////////////////////////////////////////////////////
void ControlPanelExample::Input( GlgObjectC& viewport, GlgObjectC& message )
{
   CONST char
     * format,
     * action,
     * origin;
      
   // Get the message's format, action and origin.
   message.GetResource( "Format", &format );
   message.GetResource( "Action", &action );
   message.GetResource( "Origin", &origin );

   // Handle window closing. May use viewport's name.
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );

   // Input event occurred in a button.
   if( strcmp( format, "Button" ) == 0 )
   {
      if( strcmp( action, "Activate" ) != 0 )
	 return;

      // User selected a Quit button: exit the program.
      if( strcmp( origin , "QuitButton" ) == 0 )
        exit( 0 );
   }
   // Input occurred in a slider. 
   else if( strcmp( format, "Slider" ) == 0 )
   {
      double slider_value;
	      
      // Retreive current slider value from a message object.
      message.GetResource( "ValueY", &slider_value );

      // Set a data value for a dial control
      SetResource( "Dial/Value", slider_value );

      // Update the viewport to reflect new resource settings
      Update();  
   }
}

ControlPanelExample::ControlPanelExample( void )
{
   LowRange = 0.;
   HighRange = 50.;
   InitValue = 30.;
}

ControlPanelExample::~ControlPanelExample( void )
{
}





