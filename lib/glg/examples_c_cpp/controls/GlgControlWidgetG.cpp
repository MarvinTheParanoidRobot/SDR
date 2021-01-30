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
#include "GlgClass.h"

GlgAppContext AppContext;

class ControlWidgetExample : public GlgObjectC
{
 public:
   
   ControlWidgetExample( void );
   virtual ~ControlWidgetExample( void );

   // Override to supply custom Input and Selection methods
   void Input( GlgObjectC& callback_viewport, GlgObjectC& message );

   // Initialize drawing's parameters.
   void InitializeDrawing();

 public:
   // Initial parameters for a dial.
   double LowRange;
   double HighRange;
   double InitValue;
};


// Defines a platform-specific program entry point.
#include "GlgMain.h"

/////////////////////////////////////////////////////////////////////////////
// Main Entry point.
/////////////////////////////////////////////////////////////////////////////
int GlgMain( int argc, char *argv[], GlgAppContext InitAppContext )
{
   GlgSessionC glg_session( False, InitAppContext, argc, argv );
   ControlWidgetExample control_example;

   AppContext = glg_session.GetAppContext();

   // Load a GLG  drawing from a file.
   control_example.LoadWidget( "meter5.g" );
    
   // Set widget dimensions using world coordinates [-1000;1000].
   // If not set, default dimensions will be used as set in the GLG editor.
   control_example.SetResource( "Point1", -300., -300., 0. );
   control_example.SetResource( "Point2", 300., 300., 0. );

   // Enable input and selection callbacks
   control_example.EnableCallback( GLG_INPUT_CB );

   // Set initial drawing parameters.
   control_example.InitializeDrawing();

   // Display GLG window.
   control_example.InitialDraw();

   return (int) GlgMainLoop( AppContext );
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

