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

#include "GlgWrapper.h"

#define WIDTH     600
#define HEIGHT    600

void Input( Widget widget, XtPointer client_data, XtPointer call_data );
void Select( Widget widget, XtPointer client_data, XtPointer call_data );
void VInit( Widget widget, XtPointer client_data, XtPointer call_data );

/* Initial parameters for a dial and slider controls */
double LowRange = 0.;
double HighRange = 50.;
double InitValue = 30.;

/* Application context */
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
   XtSetArg( al[ac], XtNglgDrawingFile, "controls.g" ); ac++;
   
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

   /* Disable InputHandler for a dial control so that the dial acts
       nly as an output device and doesn't respond to user input. */
   XglgSetDResource( viewport, "Dial/HandlerDisabled", 1. ); 

   /* Set High and Low range resources for both Slider and Dial */
   XglgSetDResource( viewport, "Dial/Low", LowRange );   
   XglgSetDResource( viewport, "Slider/Low", LowRange );

   
   XglgSetDResource( viewport, "Dial/High", HighRange );
   XglgSetDResource( viewport, "Slider/High", HighRange );

   /* Set initial values of a dial and a slider */
   XglgSetDResource( viewport, "Dial/Value", InitValue );
   XglgSetDResource( viewport, "Slider/ValueY", InitValue );
}

/*----------------------------------------------------------------------
| This callback is invoked when user interacts with input objects in GLG
| drawing, such as a slider, dial or a button.
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

   /* Retrieve a viewport object from the wrapper widget */
   viewport = XglgGetWidgetViewport( widget );

   /* Handle window closing. May use viewport's name. */
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );

   /* Input event occurred in a button. */
   if( strcmp( format, "Button" ) == 0 )
   {
      if( strcmp( action, "Activate" ) != 0 )
	 return;

      /* The user selected a Quit button: exit the program. */
      if( strcmp( origin , "QuitButton" ) == 0 )
        exit( 0 );
   }
   /* Input occurred in a slider. */
   else if( strcmp( format, "Slider" ) == 0 )
   {
      double slider_value;
	      
      /* Retreive current slider value from a message object */
      XglgGetDResource( message_obj, "ValueY", &slider_value );

      /* Set a data value for a dial control */
      XglgSetDResource( viewport, "Dial/Value", slider_value );

      /* Update the viewport to reflect new resource settings */
      XglgUpdate( viewport );  
   }
}

