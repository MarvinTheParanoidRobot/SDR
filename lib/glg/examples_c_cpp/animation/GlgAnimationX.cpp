/*----------------------------------------------------------------------
| This example demonstrates how to load a GLG drawing and update
| its dynamic attributes periodically, as well as handle object
| selection with the mouse. The program exits when user clicks 
| on a moving object.
|
| Periodic dynamic updates are handled in the Update procedure, which
| is invoked when a specified time interval elapses. It is registered
| as a Timer procedure using GlgAddTimeOut function with a specified
| time interval.
|
| Object selection event is handled in the Select callback.
|
| The "animation.g" drawing contains a ball named "CatchMe" and a polygon
| named "Area". The lower left and upper right control points of the area
| polygon are named "LLPoint" and "URPoint". The program queries the 
| coordinates of these control points and uses them to animate the ball
| by moving it inside the area. The ball has X and Y move transformations
| attached to it to control its position. The program sets the XValue 
| and YValue parameters of the transformations to move the ball.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include "GlgClass.h"

// Set this defined constant to be 1 to use code generated by the
// GLG Code Generation Utility.
//
#define USE_GENERATED_CODE     0

#define WIDTH     600
#define HEIGHT    600

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

#define DELTA     ( 2. * M_PI / 500. )

// Global animation parameters.
double XIncrement = DELTA * 8.;
double YIncrement = DELTA * 7.;
double Radius;                   // Moving ball's radius
double XMin, XMax, YMin, YMax;   // Ball movement area   

// Time interval for periodic dynamic updates.
unsigned int TimeInterval = 20; 

XtAppContext AppContext;         // Appplication context

class AnimationExample : public GlgWrapperC
{
 private:
   // Disallow assigments and copying a widget
   AnimationExample& operator= ( const AnimationExample& object );
   AnimationExample( AnimationExample& object );

 public:

   AnimationExample( void );
   virtual ~AnimationExample( void );

   // Override to supply custom Input and Selection methods
   void Input( GlgObjectC& callback_viewport, GlgObjectC& message );
   void Select( GlgObjectC& callback_viewport, CONST char ** name_array );
};

// Function prototypes
void Update( AnimationExample*, GlgLong* );
void GetNewPosition( double *x_pos, double *y_pos );

#if USE_GENERATED_CODE
// The following symbols should be defined in the file generated by the
// GLG Code Generation Utility.
//
extern long AnimationData[];
extern long AnimationDataSize;
#endif

//////////////////////////////////////////////////////////////////////////
// Main Entry point.
//////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] )
{
   Display * display;
   Widget shell, form;
   Cardinal ac;
   Arg al[20];
   AnimationExample animation;
   double z;

   // Initialize X Toolkit and create an application context.
   XtToolkitInitialize();
   AppContext = XtCreateApplicationContext();
   
   // Open a display connection.
   display =
     XtOpenDisplay( AppContext, 0, "GlgAnimation", "Glg", 0, 0, &argc, argv );
   
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
   
   // Create a GLG widget either from a file or a generated image.
#if USE_GENERATED_CODE
   // Use a generated drawing image.
   animation.Create( AnimationData, AnimationDataSize, shell );
#else
   // Take the drawing from a file.
   animation.Create( "animation.g", shell );
#endif
   
   // Enable input and selection callbacks
   animation.EnableCallback( GLG_INPUT_CB );
   animation.EnableCallback( GLG_SELECT_CB );
      
   XtRealizeWidget( shell );

   animation.GetViewport();   // Get the viewport object after realizing.

   // Query the ball's radius.
   animation.GetResource( "CatchMe/Radius", &Radius );

   // Query the extent of the ball's movement area.
   animation.GetResource( "Area/LLPoint", &XMin, &YMin, &z );
   animation.GetResource( "Area/URPoint", &XMax, &YMax, &z );

   // Add a timer to update the animation. 
   XtAppAddTimeOut( AppContext, TimeInterval,
		    (XtTimerCallbackProc)Update, (XtPointer)&animation );

   XtAppMainLoop( AppContext );
}

/////////////////////////////////////////////////////////////////////////////
// Animates the ball by moving it inside the area defined in the drawing.
/////////////////////////////////////////////////////////////////////////////
void Update( AnimationExample * animation, GlgLong *timer_id )
{
   double x_pos, y_pos;     

   // Calculate new coordinates of the ball.
   GetNewPosition( &x_pos, &y_pos );

   // Set coordinates of the ball in the drawing.
   animation->SetResource( "CatchMe/XValue", x_pos );
   animation->SetResource( "CatchMe/YValue", y_pos );

   animation->Update();    // Makes changes visible.
   animation->Sync();      // Improves interactive response.
   
   // Reinstall the timer to continue updating.
   XtAppAddTimeOut( AppContext, TimeInterval,
		    (XtTimerCallbackProc)Update, (XtPointer)animation );
}

/////////////////////////////////////////////////////////////////////////////
// This callback is invoked when user selects some object in the drawing
// with the mouse. In this program, it's used to exit animation if
// user selects the moving object (named "CatchMe").
/////////////////////////////////////////////////////////////////////////////
void AnimationExample::Select( GlgObjectC& viewport, CONST char ** name_array )
{
   CONST char * name;
   int i, j;

   if( name_array )    // Something was selected.
   {
      for( i=0; name = name_array[ i ]; ++i )
	if( strcmp( name, "Faster" ) == 0 || strcmp( name, "Slower" ) == 0 )
	  // Ignore buttons selection
	  return;   

      for( i=0; name = name_array[ i ]; ++i )
	if( strcmp( name, "CatchMe" ) == 0 )
	{
	   // Got the moving object: blink three times and exit.
	   for( j=0; j<6; ++j )
	   {
	      // Change the index of the color dynamics attached to the
	      // object's FillColor attribute from 0 to 1 and back to blink.
	      // You can check it in the editor by selecting the FillColor 
	      // attribute of the object and clicking on the Edit Dynamics 
	      // button in the Attribute Dialog.
		
	      SetResource( "CatchMe/ColorIndex", ( j % 2 ) ? 1. : 0. );
	      Update();
	      GlgSleep( 100 );   // Delay for 0.1 sec
	   }
	   exit( 0 );
	}
   }
	  
   GlgBell( viewport );   // Missed: beep'em up!
}

/////////////////////////////////////////////////////////////////////////////
// This callback is invoked when user interacts with input objects in GLG
// drawing. In this program, it is used to increase or decrease the animation
// speed.
/////////////////////////////////////////////////////////////////////////////
void AnimationExample::Input( GlgObjectC& viewport, GlgObjectC& message )
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

   // Do not do anything if the message did not come from a button or
   // action is different from Activate.
   //
   if( strcmp( format, "Button" ) != 0  ||
      strcmp( action, "Activate" ) != 0 )
     return;

   // Act based on the selected button.
   if( strcmp( origin , "Faster" ) == 0 )
   {
       // Increase animation speed
      TimeInterval /= 2;
   }
   else if( strcmp( origin , "Slower" ) == 0 )
   {
      // Decrease animation speed
      if( !TimeInterval )
	TimeInterval = 1;
      else
	TimeInterval *= 2;
   }
}

AnimationExample::AnimationExample( void )
{
}

AnimationExample::~AnimationExample( void )
{
}

/////////////////////////////////////////////////////////////////////////////
// Simulation: calculates the new position of the ball.
/////////////////////////////////////////////////////////////////////////////
void GetNewPosition( double * x_pos, double * y_pos )
{
   static double     /* Change in range 0-2*PI */
     x_value = 0.,
     y_value = 0.;    
   double
     x_center,
     y_center,
     x_amplitude,
     y_amplitude;
   
   /* Increase x value counter */
   x_value += XIncrement;
   if( x_value > 2. * M_PI )
     x_value -= 2. * M_PI;

   /* Increase y value counter */
   y_value += YIncrement;
   if( y_value > 2. * M_PI )
     y_value -= 2. * M_PI;

   /* Find the center of the ball's movement area */
   x_center = ( XMax + XMin ) / 2.;
   y_center = ( YMax + YMin ) / 2.;

   /* The extent of the ball's movements */
   x_amplitude = ( XMax - XMin ) / 2. - Radius;
   y_amplitude = ( YMax - YMin ) / 2. - Radius;
   
   /* Calculate the ball's current position */
   *x_pos = x_center + x_amplitude * sin( x_value );
   *y_pos = y_center + y_amplitude * cos( y_value );
}





