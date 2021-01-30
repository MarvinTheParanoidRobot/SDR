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

#include "GlgApi.h"

/* Function prototypes */
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );

GlgAppContext AppContext;

/* Initial parameters for a dial */
double LowRange = 0.;
double HighRange = 50.;
double InitValue = 30.;

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*--------------------------------------------------------------------*/
int GlgMain( argc, argv, InitAppContext )
     int argc;
     char *argv[];
     GlgAppContext InitAppContext;
{

   GlgObject viewport;

   AppContext = GlgInit( False, InitAppContext, argc, argv );

   /* Load a drawing from the file. */
   viewport = GlgLoadWidgetFromFile( "meter5.g" );

   if( !viewport )
     exit( 1 );
   
   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( viewport, "Point1", -300., -300., 0. );
   GlgSetGResource( viewport, "Point2", 300., 300., 0. );

   /* Setting the window name (title). */
   GlgSetSResource( viewport, "ScreenName", "Glg Control widget" );

   /* Add Input callback to handle user interraction in the GLG
      control. */
   GlgAddCallback( viewport, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );

   /* Set initial values for a GLG widget */
   GlgSetDResource( viewport, "Low", LowRange );
   GlgSetDResource( viewport, "High", HighRange );
   GlgSetDResource( viewport, "Value", InitValue );
   
   /* Paint the drawing. */   
   GlgInitialDraw( viewport );

   return (int) GlgMainLoop( AppContext );
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

   /* Input occurred in a dial. */
   if( strcmp( format, "Knob" ) == 0 )
   {
      double dial_value;
	      
      /* Retreive a current data value from a dial control */
      GlgGetDResource( message_obj, "Value", &dial_value );

      /* Print the value */
      printf( "Dial value = %lf\n", dial_value );
   }
}


