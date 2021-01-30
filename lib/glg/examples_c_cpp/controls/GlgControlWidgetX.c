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

#include "GlgWrapper.h"

/* Function prototypes */
void Input( Widget widget, XtPointer client_data, XtPointer call_data );
void VInit( Widget widget, XtPointer client_data, XtPointer call_data );

#define WIDTH 300
#define HEIGHT 300

/* Initial parameters for a dial and slider controls */
double LowRange = 0.;
double HighRange = 50.;
double InitValue = 30.;

XtAppContext AppContext;

/*----------------------------------------------------------------------
|
*/
main( argc, argv )
     int argc;
     char *argv[];
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
   display = XtOpenDisplay( AppContext, NULL, "GlgDialWidget", "Glg",
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
     XtAppCreateShell( "GlgDialWidget", "Glg", applicationShellWidgetClass,
		      display, al, ac );
   
   /* Create a GLG widget. */
   ac = 0;
   
   /* Specify the GLG drawing name. */
   XtSetArg( al[ac], XtNglgDrawingFile, "meter5.g" ); ac++;
   
   /* Create an instance of a GLG Wrapper widget. */
   glgWrapper =
     XtCreateWidget( "GlgWrapper", glgWrapperWidgetClass, shell, al, ac );

   /* Add Input callback to handle user interaction in the GLG 
      input objects, such as a slider, a button or a dial.
      */
   XtAddCallback( glgWrapper, XtNglgInputCB, Input, NULL );
   
   /* Add a callback to initialize or query parameters of a GLG 
      drawing/widget 
      */
   XtAddCallback( glgWrapper, XtNglgVInitCB, VInit, NULL );

   XtManageChild( glgWrapper );    
   XtRealizeWidget( shell );
   
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
void VInit( widget, client_data, call_data )
   Widget widget;
   GlgAnyType client_data;
   GlgAnyType call_data;
{
   GlgObject viewport;

   /* Retrieve GLG viewport object from  the wrapper widget. */
   viewport = XglgGetWidgetViewport( widget );

   /* Set initial values for a GLG dial widget */
   XglgSetDResource( viewport, "Low", LowRange );
   XglgSetDResource( viewport, "High", HighRange );
   XglgSetDResource( viewport, "Value", InitValue );
}

/*----------------------------------------------------------------------
| This callback is invoked when user interacts with input objects in GLG
| drawing. In this program, it is used to increase or decrease the animation
| speed.
*/
void Input( widget, client_data, call_data )
     Widget widget;
     XtPointer client_data;
     XtPointer call_data;
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

   /* Handle window closing. May use viewport's name. */
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );

   /* Input occurred in a dial widget. */
   if( strcmp( format, "Knob" ) == 0 )
   {
      double dial_value;
	      
      /* Retreive a current data value from a dial control */
      XglgGetDResource( message_obj, "Value", &dial_value );

      /* Print the value */
      printf( "Dial value = %lf\n", dial_value );
   }
}
