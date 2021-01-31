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

#include "GlgApi.h"

/* Function prototypes */
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void InitializeDrawing( GlgObject viewport );

GlgAppContext AppContext;

/* Initial parameters for a dial and slider controls */
double LowRange = 0.;
double HighRange = 50.;
double InitValue = 30.;

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*----------------------------------------------------------------------*/
int GlgMain( argc, argv, InitAppContext )
     int argc;
     char *argv[];
     GlgAppContext InitAppContext;
{

   GlgObject viewport;

   AppContext = GlgInit( False, InitAppContext, argc, argv );

   /* Load a drawing from the file. */
   viewport = GlgLoadWidgetFromFile( "controls.g" );

   if( !viewport )
     exit( 1 );
   
   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( viewport, "Point1", -600., -700., 0. );
   GlgSetGResource( viewport, "Point2", 600., 700., 0. );

   /* Setting the window name (title). */
   GlgSetSResource( viewport, "ScreenName", "Glg ControlPanel" );

   /* Add Input callback to handle user interraction in the GLG
      control. */
   GlgAddCallback( viewport, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );

   /* Set initial values for GLG widgets. */
   InitializeDrawing( viewport );

   /* Paint the drawing. */   
   GlgInitialDraw( viewport );

   return (int) GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| Set initial parameters for GLG objects/widgets.
*/
void InitializeDrawing( viewport )
   GlgObject viewport;
{
   /* Disable InputHandler for a dial control so that the dial acts
       nly as an output device and doesn't respond to user input. */
   GlgSetDResource( viewport, "Dial/HandlerDisabled", 1. );

   /* Set High and Low range resources for both Slider and Dial */
   GlgSetDResource( viewport, "Dial/Low", LowRange );
   GlgSetDResource( viewport, "Slider/Low", LowRange );
   
   GlgSetDResource( viewport, "Dial/High", HighRange );
   GlgSetDResource( viewport, "Slider/High", HighRange );
   
   /* Set initial values of a dial and a slider */
   GlgSetDResource( viewport, "Dial/Value", InitValue );
   GlgSetDResource( viewport, "Slider/ValueY", InitValue );
}

/*----------------------------------------------------------------------
| This callback is invoked when user interacts with input objects in GLG
| drawing, such as a slider, dial or a button.
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
      GlgGetDResource( message_obj, "ValueY", &slider_value );

      /* Set a data value for a dial control */
      GlgSetDResource( viewport, "Dial/Value", slider_value );

      /* Update the viewport to reflect new resource settings */
      GlgUpdate( viewport );  
   }
}


