#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "GlgApi.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

/* Function prototypes */
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void InitializeDrawing( GlgObject viewport );
void UpdateDrawing( GlgObject viewport, GlgLong *timer_id );
void SetSize( GlgObject viewport, GlgLong x, GlgLong y, 
              GlgLong width, GlgLong height );
void StopUpdates( void );
void StartUpdates( void );
double GetData( double low, double high );

#define  UPDATE_INTERVAL     100     /* Update interval, msec. */

/* If set to true, tags defined in the drawing are used for animation.
   Otherwise, object resources are used to push real-time values into the
   drawing.
*/
#define USE_TAGS True

GlgObject Drawing;
GlgAppContext AppContext;

GlgLong TimerID;

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*----------------------------------------------------------------------*/
int GlgMain( argc, argv, InitAppContext )
     int argc;
     char *argv[];
     GlgAppContext InitAppContext;
{

   AppContext = GlgInit( False, InitAppContext, argc, argv );

   /* Load a drawing from the file. */
   Drawing = GlgLoadWidgetFromFile( "dashboard.g" );

   if( !Drawing )
     exit( 1 );
   
   /* Set window size. */
   SetSize( Drawing, 0, 0, 550, 500 );

   /* Setting the window name (title). */
   GlgSetSResource( Drawing, "ScreenName", "Glg Dashboard Example" );

   /* Add Input callback to handle user interaction in a GLG input widget
      such as a button or a slider.
   */
   GlgAddCallback( Drawing, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );

   /* Set initial values for GLG widgets. */
   InitializeDrawing( Drawing );

   /* Paint the drawing. */   
   GlgInitialDraw( Drawing );

   /* Start periodic dynamic updates. */
   StartUpdates();
   
   return (int) GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| Set initial parameters for GLG objects/widgets.
*/
void InitializeDrawing( GlgObject viewport )
{
   /* Set initial patameters as needed. */
   GlgSetDResource( viewport, "DialPressure/Low", 0. );
   GlgSetDResource( viewport, "DialVoltage/Low", 0. );
   GlgSetDResource( viewport, "DialAmps/Low", 0. );
   GlgSetDResource( viewport, "SliderPressure/Low", 0. );
   
   GlgSetDResource( viewport, "DialPressure/High", 50. );
   GlgSetDResource( viewport, "DialVoltage/High", 120. );
   GlgSetDResource( viewport, "DialAmps/High", 10. );
   GlgSetDResource( viewport, "SliderPressure/High", 50. );
}

/*----------------------------------------------------------------------
| Periodic dynamic updates.
*/
void UpdateDrawing( GlgAnyType client_data, GlgLong * timer_id )
{
   /* Obtain simulated demo data values in a specified range.
      The application should provide a custom implementation
      of the data acquisition interface to obtain real-time
      data values.
   */
   double voltage = GetData( 0.0, 120.0 );
   double current = GetData( 0.0, 10.0 );
   
   if( USE_TAGS ) /* Use tags for animation. */
   {
      /* Push values to the objects using tags defined in the drawing. */
      GlgSetDTag( Drawing, "Voltage", voltage, GlgTrue /*if_changed*/ );
      GlgSetDTag( Drawing, "Current", current, GlgTrue /*if_changed*/ );
   }
   else /* Use resources for animation. */
   {
      /* Push values to the objects using resource paths. */
      GlgSetDResource( Drawing, "DialVoltage/Value", voltage );
      GlgSetDResource( Drawing, "DialAmps/Value", current );
   }
   
   GlgUpdate( Drawing );    /* Make changes visible. */
   
   /* Reinstall the timer to continue updating */
   TimerID = GlgAddTimeOut( AppContext, UPDATE_INTERVAL, 
                          (GlgTimerProc) UpdateDrawing, (GlgAnyType) NULL );
}

/*----------------------------------------------------------------------
| Start periodic dynamic updates.
*/
void StartUpdates()
{
   TimerID = GlgAddTimeOut( AppContext, UPDATE_INTERVAL,
                            (GlgTimerProc) UpdateDrawing, 
                            (GlgAnyType) NULL );
}

/*----------------------------------------------------------------------
| Stop dynamic updates.
*/
void StopUpdates()
{
   /* Stop update timers */
   if( TimerID )
   {
      GlgRemoveTimeOut( TimerID );
      TimerID = 0;
   }
}

/*----------------------------------------------------------------------
| This callback is invoked when user interacts with input objects in GLG
| drawing, such as a slider, dial or a button.
*/
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
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

   /* Input event occurred in a button. */
   if( strcmp( format, "Button" ) == 0 )
   {
      if( strcmp( action, "Activate" ) == 0 ) /* Push button events. */
      {
         /* The user selected a Quit button: exit the program. */
         if( strcmp( origin , "QuitButton" ) == 0 )
           exit( 0 );
      }
      else if( strcmp( action, "ValueChanged" ) == 0 ) /* Toggle button events. */
      {
         if( strcmp( origin, "StartButton" ) == 0 )
         {
            double value;
            GlgGetDResource( message_obj, "OnState", &value );

            switch( (int) value)
            {
             case 0:
               StopUpdates();   /* Stop updates. */
               break;
             case 1:
               StartUpdates();  /* Start updates. */
               break;
             default: break;
            }
         }
      }
      
      GlgUpdate( viewport );
   }
   /* Input occurred in a slider. */
   else if( strcmp( format, "Slider" ) == 0 && 
            strcmp( origin, "SliderPressure" ) == 0 )
   {
      double slider_value;
	      
      /* Retrieve current slider value from the message object. */
      GlgGetDResource( message_obj, "ValueY", &slider_value );

      /* Set a data value for a dial control */
      GlgSetDResource( viewport, "DialPressure/Value", slider_value );

      /* Update the viewport to reflect new resource settings. */
      GlgUpdate( viewport );  
   }
}

/*----------------------------------------------------------------------
| Set viewport size in screen cooridnates. 
*/
void SetSize( GlgObject viewport, GlgLong x, GlgLong y, 
              GlgLong width, GlgLong height )
{
   GlgSetGResource( viewport, "Point1", 0., 0., 0. );
   GlgSetGResource( viewport, "Point2", 0., 0., 0. );

   GlgSetDResource( viewport, "Screen/XHint", (double) x );
   GlgSetDResource( viewport, "Screen/YHint", (double) y );
   GlgSetDResource( viewport, "Screen/WidthHint", (double) width );
   GlgSetDResource( viewport, "Screen/HeightHint", (double) height );
}

/*----------------------------------------------------------------------
| Generates demo data value within a specified range. 
| An application can replace code in this function to supply 
| real-time data from a custom data source.
*/
double GetData( double low, double high )
{
   double
     half_amplitude, center,
     period,
     value,
     alpha;
   
   static int counter = 0;

   half_amplitude = ( high - low ) / 2.0;
   center = low + half_amplitude;
   
   period = 100.0;
   alpha = 2.0 * M_PI * counter / period;
   
   value = center +
     half_amplitude * sin( alpha ) * sin( alpha / 30.0 );
   
   ++counter;
   return value;
}
