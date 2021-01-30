/*----------------------------------------------------------------------
| This example demonstrates how to use integrated blinking feature
| of the GLG Toolkit. 
| 
| When water level in the tank reaches 90%, two alarm objects will
| start blinking. 
|
| In order to update blinking objects, GlgUpdate() must be called
| in the Input callback. GlgUpdate() can be invoked either
| at the end of the Input callback for all the messages, or it can be
| called for Timer messages only.
|
| The "blinking.g" drawing contains two alarm objects, where one of them
| blinks using the color attribute (a circle), and another blinks using 
| the visibility attribute (a text object). In both cases, blinking
| functionality is implemented by attaching a Timer transformation to the
| attribute being blinked (toggled). 
|
| One alarm object is a group named "Alarm" containing a circle whose 
| FillColor has a List transformation. ValueIndex parameter of that 
| transformation has a Timer transformation, causing ValueIndex
| being toggled between 0 and 1 when Timer is enabled (Alarm/AlarmEnabled 
| resource is set to 1.). Toggling the ValueIndex attribute consequently
| causes color to alternate between green and red. 
|
| The second alarm object is a text object named "AlarmText" with a 
| string OVERFLOW, and its Visibility attribute has a Timer transformation 
| attached. When AlarmText/AlarmEnabled resource is set to 1., 
| the text starts blinking by toggling its Visibility resource.
|
| Periodic dynamic updates are handled in the Update procedure, which
| is invoked when a specified time interval elapses. It is registered
| using GlgAddTimeOut function with a specified time interval.
|
*/

#include <stdio.h>
#include <stdlib.h>

#include "GlgApi.h"

/* Function prototypes */
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void Update( GlgObject viewport, GlgLong *timer_id );

/* Time interval for periodic dynamic updates, in millisec. */
GlgLong TimeInterval = 50; 

/* Low and High range of the water lavel in the tank */
double Low, High;

/* Increment value for dynamic data simulation. */
double Increment;

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

   AppContext = GlgInit( False, InitAppContext, argc, argv );

   /* Take a drawing from the file. */
   viewport = GlgLoadWidgetFromFile( "blinking.g" );
   if( !viewport )
     exit( 1 );
   
   /* Extract the Low and High ranges of the water level value in 
      the tank. */
   GlgGetDResource( viewport, "WaterTank/Low", &Low );
   GlgGetDResource( viewport, "WaterTank/High", &High );

   /* Make 50 iterations before starting an alarm. */
   Increment = (High - Low) / 50.; 

   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( viewport, "Point1", -400., -450., 0. );
   GlgSetGResource( viewport, "Point2", 400., 450., 0. );

   /* Setting the window name (title). */
   GlgSetSResource( viewport, "ScreenName", "GlgBlinkingExample" );

   /* Add Inout callback to handle Blinking messages, if necessary. */
   GlgAddCallback( viewport, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );
   
   GlgInitialDraw( viewport );

   /* Add a timer procedure to update the animation. */
   GlgAddTimeOut( AppContext, TimeInterval, 
		  (GlgTimerProc)Update, (GlgAnyType)viewport );
   
   return GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| Animates the drawing
*/
void Update( viewport, timer_id )
     GlgObject viewport;
     GlgLong * timer_id;
{
   static double water_level = 0.;

   /* Calculate new water level. When water level reaches 90% of the
      tank, enable alarms.  */
   water_level += Increment;
   if( water_level >= (High - Low) * .9 )
   {
      GlgSetDResource( viewport, "Alarm/AlarmEnabled", 1. );
      GlgSetDResource( viewport, "AlarmText/AlarmEnabled", 1. );
   }
   else
   {
      /* Update dynamic resources of the tank and the meter. */
      GlgSetDResource( viewport, "WaterTank/WaterLevel", water_level );
      GlgSetDResource( viewport, "WaterMeter/Value", water_level );
   }

   GlgUpdate( viewport );    /* Makes changes visible. */

   /* Reinstall the timeout to continue updating */
   GlgAddTimeOut( AppContext, TimeInterval, (GlgTimerProc)Update,
		 (GlgAnyType)viewport );
}

/*----------------------------------------------------------------------
| This callback is used to handle input events, object selection
| events and blinking events.
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

   /* Blinking messages. */
   if( strcmp( format , "Timer" ) == 0  &&
       strcmp( action , "Update" ) == 0 )
   {
      /* GlgUpdate() may be called here to update blinking objects
	 only, instead of calling it at the end of the Input callback.
	 */
   }
   else if( strcmp( format , "Button" ) == 0 && 
           strcmp( action, "Activate" ) == 0 )
   {
      if( strcmp( origin , "QuitButton" ) == 0 )
	 exit( 0 );
   }

   /* Update the viewport. GlgUpdate() is necessary for updating 
      blinking objects.
   */
   GlgUpdate( viewport );
   
}
