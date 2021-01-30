/*----------------------------------------------------------------------
|  This example demonstrates how to use GLG control widgets as they are,
|  without emdedding them into another GLG drawing, and handle user 
|  interaction. The example displays a dial control and prints its
|  current value, based on the user moving a needle of the dial.
|
|  The drawing name is meter5.g. 
----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include "GlgClass.h"

#define WIDTH     300
#define HEIGHT    300

XtAppContext AppContext;         // Appplication context

class ControlWidgetExample : public GlgWrapperC
{
 private:
   // Disallow assigments and copying a widget
   ControlWidgetExample& operator= ( const ControlWidgetExample& object );
   ControlWidgetExample( ControlWidgetExample& object );

 public:

   ControlWidgetExample( void );
   virtual ~ControlWidgetExample( void );

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
   ControlWidgetExample control_example;
 
   // Initialize X Toolkit and create an application context.
   XtToolkitInitialize();
   AppContext = XtCreateApplicationContext();
   
   // Open a display connection.
   display =
     XtOpenDisplay( AppContext, 0, "GlgControlWidget", "Glg", 0, 0, &argc, argv );
   
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
   control_example.Create( "meter5.g", shell );
   
   // Enable Input callback
   control_example.EnableCallback( GLG_INPUT_CB );
         
   XtRealizeWidget( shell );

   control_example.GetViewport();   // Get the viewport object after realizing.
   control_example.InitializeDrawing();

   XtAppMainLoop( AppContext );
}

/////////////////////////////////////////////////////////////////////////////
// Set initial parameters for GLG objects/widgets.
/////////////////////////////////////////////////////////////////////////////
void ControlWidgetExample::InitializeDrawing()
{
   // Set High and Low range resources for a dial.
   SetResource( "Low", LowRange );
   SetResource( "High", HighRange );

   // Set initial value.
   SetResource( "Value", InitValue );
}

/////////////////////////////////////////////////////////////////////////////
// This callback is invoked when user interacts with input objects in GLG
// drawing, such as a slider, dial or a button.
/////////////////////////////////////////////////////////////////////////////
void ControlWidgetExample::Input( GlgObjectC& viewport, GlgObjectC& message )
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

   // Input event occurred in a dial.
   if( strcmp( format, "Knob" ) == 0 )
   { 
      double dial_value;
	      
      /* Retreive a current data value from a dial control */
      message.GetResource( "Value", &dial_value );

      /* Print the value */
      printf( "Dial value = %lf\n", dial_value );
   }

}

ControlWidgetExample::ControlWidgetExample( void )
{
   LowRange = 0.;
   HighRange = 50.;
   InitValue = 30.;
}

ControlWidgetExample::~ControlWidgetExample( void )
{
}





