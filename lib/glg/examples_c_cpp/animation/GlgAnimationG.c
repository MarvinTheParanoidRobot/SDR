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

#include "GlgApi.h"

/* Set this defined constant to be 1 to use code generated by the
 * GLG Code Generation Utility.
 */
#define USE_GENERATED_CODE     0

/* Function prototypes */
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void Select( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void Update( GlgObject viewport, GlgLong *timer_id );
void GetNewPosition( double *x_pos, double *y_pos );

#if USE_GENERATED_CODE
/* The following symbols should be defined in the file generated by the
 * GLG Code Generation Utility. Save the drawing uncompressed to generate code.
 */
extern long AnimationData[];
extern long AnimationDataSize;
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

/* Global animation parameters. */

#define DELTA     ( 2. * M_PI / 500. )

double XIncrement = DELTA * 8.;
double YIncrement = DELTA * 7.;

/* Time interval for periodic dynamic updates, in millisec. */
GlgLong TimeInterval = 20; 

double Radius;   /* Moving ball's radius */
double
  XMin, XMax,    /* Ball movement's area */
  YMin, YMax;

GlgAppContext AppContext;

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*----------------------------------------------------------------------
|
*/
int GlgMain( argc, argv, InitAppContext )
     int argc;
     char *argv[];
     GlgAppContext InitAppContext;
{
   GlgObject viewport;
   double z;

   AppContext = GlgInit( False, InitAppContext, argc, argv );

#if USE_GENERATED_CODE   
   /* Use a generated drawing image. */
   viewport =
     GlgLoadWidgetFromImage( (char*)AnimationData, AnimationDataSize );
#else
   /* Take a drawing from the file. */
   viewport = GlgLoadWidgetFromFile( "animation.g" );
#endif
   if( !viewport )
     exit( 1 );
   
   /* Query the ball's radius. */
   GlgGetDResource( viewport, "CatchMe/Radius", &Radius );

   /* Query the extent of the ball's movement area. */
   GlgGetGResource( viewport, "Area/LLPoint", &XMin, &YMin, &z );
   GlgGetGResource( viewport, "Area/URPoint", &XMax, &YMax, &z );

   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( viewport, "Point1", -600., -700., 0. );
   GlgSetGResource( viewport, "Point2", 600., 700., 0. );

   /* Setting the window name (title). */
   GlgSetSResource( viewport, "ScreenName", "GlgAnimation" );

   /* Add callbacks to handle user interaction in the GLG objects. 
      Select callback is invoked when the user selects a graphical 
      object with the mouse, such as a ball. Input callback is invoked
      when the user selects a GLG input object, such as a button.
   */
   GlgAddCallback( viewport, GLG_SELECT_CB, (GlgCallbackProc)Select, NULL );
   GlgAddCallback( viewport, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );
   
   GlgInitialDraw( viewport );

   /* Add a timer to update the animation. */
   GlgAddTimeOut( AppContext, TimeInterval, 
		  (GlgTimerProc)Update, (GlgAnyType)viewport );
   
   return (int) GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| Animates the ball by moving it inside the area defined in the drawing.
*/
void Update( viewport, timer_id )
     GlgObject viewport;
     GlgLong * timer_id;
{
   double x_pos, y_pos;     

   /* Calculate new coordinates of the ball. */
   GetNewPosition( &x_pos, &y_pos );

   /* Set coordinates of the ball in the drawing */
   GlgSetDResource( viewport, "CatchMe/XValue", x_pos );
   GlgSetDResource( viewport, "CatchMe/YValue", y_pos );

   GlgUpdate( viewport );    /* Makes changes visible. */

   /* Reinstall the timer to continue updating */
   GlgAddTimeOut( AppContext, TimeInterval, (GlgTimerProc)Update,
		 (GlgAnyType)viewport );
}

/*----------------------------------------------------------------------
| This callback is invoked when user selects some object in the drawing
| with the mouse. In this program, it's used to exit animation if
| user selects the moving object (named "CatchMe").
*/
void Select( viewport, client_data, call_data )
     GlgObject viewport;
     GlgAnyType client_data;
     GlgAnyType call_data;
{
   char ** name_array;
   char * name;
   int i, j;

   name_array = (char **) call_data;

   if( name_array )    /* Something was selected. */
   {
      for( i=0; name = name_array[ i ]; ++i )
	if( strcmp( name, "Faster" ) == 0 || strcmp( name, "Slower" ) == 0 )
	{
	   /* Ignore buttons selection */
	   return;   
	}

      for( i=0; name = name_array[ i ]; ++i )
	if( strcmp( name, "CatchMe" ) == 0 )
	{
	   /* Got the moving object: blink three times and exit. */
	   for( j=0; j<6; ++j )
	   {
	      /* Change the index of the color dynamics attached to the
	       * object's FillColor attribute from 0 to 1 and back to blink.
	       * You can check it in the editor by selecting the FillColor 
	       * attribute of the object and clicking on the Edit Dynamics 
	       * button in the Attribute Dialog.
	       */
	      GlgSetDResource( viewport, "CatchMe/ColorIndex",
			      ( j % 2 ) ? 1. : 0. );

	      GlgUpdate( viewport );
	      GlgSleep( 200 );   /* Delay for 0.2 sec */
	   }
	   exit( 0 );
	}
   }   
}

/*----------------------------------------------------------------------
| This callback is invoked when user interacts with input objects in GLG
| drawing. In this program, it is used to increase or decrease the 
| animation speed.
*/
void Input( viewport, client_data, call_data )
     GlgObject viewport;
     GlgAnyType client_data;
     GlgAnyType call_data;
{
   GlgObject message_obj;
   char
     * format,
     * action,
     * origin;
      
   message_obj = (GlgObject) call_data;

   /* Get the message's format, action and origin. */
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "Origin", &origin );

   /* Handle window closing. May use viewport's name. */
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );

   /* Do not do anything if the message did not come from a button or
    * action is different from Activate.
    */
   if( strcmp( format, "Button" ) != 0  ||
      strcmp( action, "Activate" ) != 0 )
     return;

   /* Act based on the selected button. */
   if( strcmp( origin , "Faster" ) == 0 )
   {
      /* Increase animation speed */
      if( TimeInterval > 1 )
        TimeInterval /= 2;
   }
   else if( strcmp( origin , "Slower" ) == 0 )
   {
      /* Decrease animation speed */
      if( TimeInterval < 5000 )
        TimeInterval *= 2;
   }
}

/*----------------------------------------------------------------------
| Simulation: calculates the new position of the ball.
*/
void GetNewPosition( x_pos, y_pos )
     double
       * x_pos,
       * y_pos;
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
