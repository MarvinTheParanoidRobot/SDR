#include <stdio.h>
#include <stdlib.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <math.h>
#include "GlgWrapper.h"

#define WIDTH     550
#define HEIGHT    500

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

/* Function prototypes */
void Input( Widget widget, XtPointer client_data, XtPointer call_data );
void VInit( Widget widget, XtPointer client_data, XtPointer call_data );
void UpdateDrawing( Widget widget, XtIntervalId * timer_id );
void StopUpdates( void );
void StartUpdates( Widget widget );
double GetData( double low, double high );

#define  UPDATE_INTERVAL     100     /* Update interval, msec. */

GlgLong TimerID;

/* Application context */
XtAppContext AppContext;

/*----------------------------------------------------------------------
|
*/
main( int argc, char * argv[] )
{
   Display * display;
   Widget shell, glgWrapper;
   GlgObject viewport;
   Cardinal ac;
   Arg al[20];
   
   /* Initialize X Toolkit and create an application context. */
   XtToolkitInitialize();
   AppContext = XtCreateApplicationContext();
   
   /* Open a display connection. */
   display = XtOpenDisplay( AppContext, NULL, "GlgAnimation", "Glg",
			   0, 0, &argc, argv );
   
   /* Create a shell */
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
     XtAppCreateShell( "GlgAnimation", "Glg", applicationShellWidgetClass,
		      display, al, ac );
   
   /* Create a GLG widget. */
   ac = 0;
   
   /* Specify a drawing to use */
   XtSetArg( al[ac], XtNglgDrawingFile, "dashboard.g" ); ac++;
   
   /* Create an instance of a GLG Wrapper widget. */
   glgWrapper =
     XtCreateWidget( "GlgWrapper", glgWrapperWidgetClass, shell, al, ac );

   /* Add Input callback to handle user interaction in the GLG objects. */
   XtAddCallback( glgWrapper, XtNglgInputCB, Input, NULL );
   
   /* Add a callback to initialize or query parameters of a GLG
      drawing. */
   XtAddCallback( glgWrapper, XtNglgVInitCB, VInit, NULL );

   XtManageChild( glgWrapper );    
   XtRealizeWidget( shell );

   /* Start periodic dynamic updates. */
   StartUpdates( glgWrapper );

   XtAppMainLoop( AppContext );
}

/*----------------------------------------------------------------------
|  Value Initialization callback, VInit. It is invoked after the 
|  drawing is loaded and object hierarchy is set up, but before
|  it is painted for the first time. It may be used to initialize
|  or query drawing parameters. 
|
|  Unlike VInit callback, another type of initialization callback,
|  HInit callback, is invoked BEFORE object hierarchy is set up.
*/ 
void VInit( Widget widget, GlgAnyType client_data, GlgAnyType call_data )
{
   /* Retrieve GLG viewport object from  the wrapper widget. */
   GlgObject viewport = XglgGetWidgetViewport( widget );
 
   /* Set initial patameters as needed. */
   XglgSetDResource( viewport, "DialPressure/Low", 0. );   
   XglgSetDResource( viewport, "DialVoltage/Low", 0. );   
   XglgSetDResource( viewport, "DialAmps/Low", 0. );   
   XglgSetDResource( viewport, "SliderPressure/Low", 0. );

   XglgSetDResource( viewport, "DialPressure/High", 50. );   
   XglgSetDResource( viewport, "DialVoltage/High", 120. );   
   XglgSetDResource( viewport, "DialAmps/High", 10. );   
   XglgSetDResource( viewport, "SliderPressure/High", 50. );
}

/*----------------------------------------------------------------------
| Start periodic dynamic updates.
*/
void StartUpdates( Widget widget )
{
   /* Retrieve GLG viewport object from  the wrapper widget. */
   GlgObject viewport = XglgGetWidgetViewport( widget );

   /* Add a work procedure to update the animation. */
   TimerID = XtAppAddTimeOut( AppContext, UPDATE_INTERVAL,
                              (XtTimerCallbackProc) UpdateDrawing, 
                              (XtPointer)widget );
}

/*----------------------------------------------------------------------
| Stop dynamic updates.
*/
void StopUpdates()
{
   /* Stop update timers */
   if( TimerID )
   {
      XtRemoveTimeOut( TimerID );
      TimerID = 0;
   }
}

/*----------------------------------------------------------------------
| Periodic dynamic updates.
*/
void UpdateDrawing( Widget widget, XtIntervalId * timer_id )
{
   GlgObject viewport = XglgGetWidgetViewport( widget );

   /* Obtain simulated demo data values in a specified range.
      The application should provide a custom implementation
      of the data acquisition interface to obtain real-time
      data values.
   */
   double voltage = GetData( 0.0, 120.0 );
   double current = GetData( 0.0, 10.0 );

   /* Push values to the objects using resource paths. */
   XglgSetDResource( viewport, "DialVoltage/Value", voltage );
   XglgSetDResource( viewport, "DialAmps/Value", current );
   
   XglgUpdate( viewport );    /* Make changes visible. */

   /* Improves interactive response. */
   XSync( XtDisplay( widget ), False );

   /* Reinstall the timeout to continue updating */
   TimerID = XtAppAddTimeOut( AppContext, UPDATE_INTERVAL,
                              (XtTimerCallbackProc)UpdateDrawing, 
                              (XtPointer)widget );
}

/*----------------------------------------------------------------------
| This callback is invoked when user interacts with input objects in GLG
| drawing, such as a slider, dial or a button.
*/
void Input( Widget widget, XtPointer client_data, XtPointer call_data )
{
   GlgObject
     message_obj,
     viewport;
   char
     * format,
     * action,
     * origin;
      
   message_obj = (GlgObject) call_data;

   /* Get the message's format, action and origin. */
   XglgGetSResource( message_obj, "Format", &format );
   XglgGetSResource( message_obj, "Action", &action );
   XglgGetSResource( message_obj, "Origin", &origin );

   /* Retrieve a viewport object from the wrapper widget */
   viewport = XglgGetWidgetViewport( widget );

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
      else if( strcmp( action, "ValueChanged" ) == 0 ) /* Toggle button events */
      {
         if( strcmp( origin, "StartButton" ) == 0 )
         {
            double value;
            XglgGetDResource( message_obj, "OnState", &value );

            switch( (int) value)
            {
             case 0:
               StopUpdates();   /* Stop updates. */
               break;
             case 1:
               StartUpdates( widget );  /* Start updates. */
               break;
             default: break;
            }
         }
      }
      
      XglgUpdate( viewport );
   }
   /* Input occurred in a slider. */
   else if( strcmp( format, "Slider" ) == 0 && 
            strcmp( origin, "SliderPressure" ) == 0 )
   {
      double slider_value;
	      
      /* Retrieve current slider value from the message object. */
      XglgGetDResource( message_obj, "ValueY", &slider_value );

      /* Set a data value for a dial control */
      XglgSetDResource( viewport, "DialPressure/Value", slider_value );

      /* Update the viewport to reflect new resource settings. */
      XglgUpdate( viewport );  
   }
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

