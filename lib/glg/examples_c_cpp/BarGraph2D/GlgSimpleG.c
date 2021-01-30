#include <stdio.h>
#include <stdlib.h>
#include "GlgApi.h"

/* Function prototypes */
GlgBoolean UpdateGraph( GlgObject viewport );
double GetData( void );
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*------------------------------------------------------------------------
|
| This is a simple program which creates a GLG widget with a bar graph
| in it and fills it with random data. The program uses generic GLG API
| and may be used in both X Windows and MS Windows environments.
| In order to use this program template with a diffrent drawing file,
| replace "bar1.g" with the name of your drawing file and adust resource
| names.
*/
int GlgMain( argc, argv, InitAppContext )
     int argc;
     char *argv[];
     GlgAppContext InitAppContext;
{
   GlgAppContext AppContext;
   GlgObject viewport;

   AppContext = GlgInit( False, InitAppContext, argc, argv );

   viewport = GlgLoadWidgetFromFile( "bar1.g" );
   if( !viewport )
     exit( 1 );

   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( viewport, "Point1", -700., -700., 0. );
   GlgSetGResource( viewport, "Point2", 700., 700., 0. );
 
   /* Setting the window name (title). */
   GlgSetSResource( viewport, "ScreenName", "GlgExample" );

   /* Add input callback for handling window closing. */
   GlgAddCallback( viewport, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );

   GlgInitialDraw( viewport );

   /* Add a work procedure to update the graph. */
   GlgAddWorkProc( AppContext, (GlgWorkProc)UpdateGraph,
		  (GlgAnyType)viewport );
   
   return (int) GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| Pushes the next data and label values and updates the graph.
*/
GlgBoolean UpdateGraph( viewport )
     GlgObject viewport;
{
   static long iteration_counter = 2; /* A counter used to generate labels. */
   char * label;

   /* Push the next data value, let the graph handle scrolling. */
   GlgSetDResource( viewport, "DataGroup/EntryPoint", GetData() );

   /* Generate the next label to use. */
   label = GlgCreateIndexedName( "Value ", iteration_counter );
   ++iteration_counter;

   /* Push the next label. The graph handles labels scrolling.
    * To set labels directly, use "XLabelGroup/Xlabel<n>/String" as a
    * name of a resource, where <n> is the sequential zero-based label
    * index. In this case you will be responsible for handling label
    * scrolling.
    */
   GlgSetSResource( viewport, "XLabelGroup/EntryPoint", label );

   GlgFree( label );        /* Free the label. */

   GlgUpdate( viewport );   /* Update the graph to make changes visible. */
   GlgSync( viewport );     /* Improves interactive response */

   return False;            /* Return False to continue updating. */
}

/*----------------------------------------------------------------------
| Returns a random number in the [0;1] range. Use a real source of data
| instead of this function.
*/
double GetData()
{
   return GlgRand( 0., 1. );
}

/*----------------------------------------------------------------------
| Handles windows closing.
*/
void Input( viewport, client_data, call_data )
     GlgObject viewport;
     GlgAnyType client_data;
     GlgAnyType call_data;
{
   GlgObject message_obj;
   char
     * format,
     * action;
 
   /* Get the message's format and action. */
   message_obj = (GlgObject) call_data;
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );

   /* Handle window closing. May use viewport's name. */
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );
}
