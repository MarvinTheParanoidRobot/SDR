/*----------------------------------------------------------------------
| This example demonstrates how to use GLG tags feature in a generic 
| way. The program loads the drawing specified as the first parameter
| on the command line, queries a list of all tags defined in the drawing 
| and uses tags to subscribe to the event based data updates. The tags 
| define the fields in the process database which the drawing needs the 
| data for. The program then waits for data and updates the drawing when 
| the data are received.
|
| The prototype does not actually connect to a process database, 
| since it is very specific to the application environment. Instead,
| it shows a sample framework and uses simulated random data. 
| An application developer needs to write application-speciific code 
| for the SubscribeTagData method, using a preferred data aquisition
| method (a custom framework or 3-rd party tools) to connect to the 
| application's data.
----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "GlgApi.h"

GlgAppContext AppContext;
GlgObject viewport;

#define UpdateInterval   200  /* Update interval in msec */

/* Set to 0 and provide SubscribeTagData method to connect to life data */
#define SIMULATED_DATA   1 

/* Function prototypes */
void Input ( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void Init( GlgObject viewport );
void SubscribeToData( GlgObject viewport, GlgObject tag_list );
void SubscribeTagData( GlgObject viewport, GlgDataType data_type, 
		      char * tag_source );
void GenerateTagData( GlgAnyType data, GlgLong * timer_id );
void ProcessDataEvent( char * tag_source, GlgDataType data_type, double value1,
		      double value2, double value3, char * svalue );

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*--------------------------------------------------------------------*/
int GlgMain( int argc, char * argv[], GlgAppContext InitAppContext )
{
   char * filename;

   AppContext = GlgInit( False, InitAppContext, argc, argv );

   if( argc < 2 )
   {
      printf( "No drawing file supplied on command line, using tags_example.g\n" );
      filename = "tags_example.g";
   }
   else
     filename = argv[ 1 ];

   /* Load a drawing from the file. */
   viewport = GlgLoadWidgetFromFile( filename );
   if( !viewport )
     exit( 1 );
   
   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( viewport, "Point1", -300., -300., 0. );
   GlgSetGResource( viewport, "Point2", 300., 300., 0. );

   /* Setting the window name (title). */
   GlgSetSResource( viewport, "ScreenName", "Glg Tags Example" );

   /* Add Input callback to handle user interraction. */
   GlgAddCallback( viewport, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );

   GlgSetupHierarchy( viewport );

   /* Query tags after HierarchySetup: more efficient. */
   Init( viewport );

   /* Paint the drawing. */   
   GlgUpdate( viewport );

   return GlgMainLoop( AppContext );
}

/*--------------------------------------------------------------------*/
void Init( GlgObject viewport )
{
   GlgObject TagList;

   /* Query a list of tag sources defined in the drawing */
   TagList = 
     GlgCreateTagList( viewport, /* List each tag source only once */ True );

   if( TagList && GlgGetSize( TagList ) )
     SubscribeToData( viewport, TagList );
     
   GlgDropObject( TagList );   /* Discard the tag list */
}

/*--------------------------------------------------------------------
| Subscribe all tags in the list.
*/
void SubscribeToData( GlgObject viewport, GlgObject tag_list )
{
   GlgObject tag_obj;
   char * tag_source;
   long i, size;
   double dtype;

   size = GlgGetSize( tag_list );
   for( i=0; i<size; ++i )
   {
      tag_obj = GlgGetElement( tag_list, i );

      /* Get the name of the database field to use as a data source */
      GlgGetSResource( tag_obj, "TagSource", &tag_source );
      if( !tag_source || !*tag_source )
	continue;   /* Skip empty tag sources */

      /* Get tag object's data type: GLG_D, GLG_S or GLG_G */
      GlgGetDResource( tag_obj, "DataType", &dtype );

      /* Subscribe this tag */
      SubscribeTagData( viewport, (GlgDataType)dtype, tag_source );
   }
}

/*--------------------------------------------------------------------
| Subscribe to the data field indicated by the tag source, so that 
| data_callback is invoked when the data comes. The data_type parameter
| may be used to ensure that the tag type in the database matches
| the tag type in the drawing.
*/
void SubscribeTagData( GlgObject viewport, GlgDataType data_type, 
		      char * tag_source )
{
#if SIMULATED_DATA
   /* Simulated data example: activate periodic data update. */
   GlgAddTimeOut( AppContext, UpdateInterval, (GlgTimerProc)GenerateTagData, 
		 tag_source );
#else
   /* Fill with application's code to subscribe to data. */
#endif
}

#if SIMULATED_DATA
/*--------------------------------------------------------------------
| Generate data for selected tag.
*/
void GenerateTagData( GlgAnyType data, GlgLong * timer_id )
{
   GlgObject tag_obj;
   char * tag_source = data;
   char buffer[ 100 ];

   /* Get tag object by TagSource, only to query its DataType for generating
      data. */
   tag_obj =
     GlgGetTagObject( viewport, tag_source, /* by TagSource */ False, 
                     False, /* single tag */ True, GLG_DATA_TAG );
   if( tag_obj )
   {
      double dtype;
      GlgDataType data_type;

      /* Get type of data needed to be generated/simulated */
      GlgGetDResource( tag_obj, "DataType", &dtype );
      data_type = dtype;

      switch( data_type )
      {
       case GLG_D:	 
	 /* Invoke data callback with random D data */
	 ProcessDataEvent( tag_source, data_type, 
			  GlgRand( 0., 1. ), 0., 0., NULL );
	 break;

       case GLG_S:
	 /* Invoke data callback with random S data */
	 sprintf( buffer, "%.2lf", GlgRand( 0., 1. ) );
	 ProcessDataEvent( tag_source, data_type, 0., 0., 0., buffer );
	 break;

       case GLG_G:
	 /* Invoke data callback with random G data */
	 ProcessDataEvent( tag_source, data_type, GlgRand( 0., 1. ),
			  GlgRand( 0., 1. ), GlgRand( 0., 1. ), NULL );
	 break;

       default: return;
      }

      /* Restart timer to continue generating data. */
      GlgAddTimeOut( AppContext, UpdateInterval,
		    (GlgTimerProc)GenerateTagData, tag_source );
   }   
}

#endif

/*--------------------------------------------------------------------
| Update tag values in the drawing.
*/
void ProcessDataEvent( char * tag_source, GlgDataType data_type, double value1,
		      double value2, double value3, char * svalue )
{
   switch( data_type )
   {
    case GLG_D:	 
      GlgSetDTag( viewport, tag_source, value1, True );
      break;

    case GLG_S:
      GlgSetSTag( viewport, tag_source, svalue, True );
      break;

    case GLG_G:
      GlgSetGTag( viewport, tag_source, value1, value2, value3, True );
      break;

    default: return;
   }

   GlgUpdate( viewport );
}


/*----------------------------------------------------------------------
| In this example, Input callback is used to handle DeleteWindow message.
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
      
   message_obj = (GlgObject) call_data;

   /* Get the message's format, action and origin. */
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );

   /* Handle window closing. May use viewport's name. */
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );
}
