/*----------------------------------------------------------------------
| This example demonstrates how to use GLG tags feature in a generic 
| way. The program loads the drawing specified as the first parameter
| on the command line and queries a list of all tags defined in the drawing,
| The tags define the fields in the process database which the drawing needs
| the data for. The program periodically traverses the tag list, 
| queries a new data value for each tag and updates the drawing.
|
| The prototype does not actually connect to a process database, 
| since it is very specific to the application environment. Instead,
| it shows a sample framework and uses simulated random data. 
| An application developer needs to write application-speciific code 
| for the GetTagData method, using a preferred data aquisition
| method (a custom framework or 3-rd party tools) to connect to the 
| application's data.
----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "GlgApi.h"

GlgAppContext AppContext;
GlgObject viewport;

#define UpdateInterval   200  /* Update interval in msec */

#define USE_SIMPLE_WAY     1

/* Set to 0 and provide SubscribeTagData method to connect to life data */
#define SIMULATED_DATA   1 

typedef struct _TagRecord
{
   char * tag_source;
   GlgDataType data_type;

   /* May be used to further optimize performance. */
   GlgObject tag_obj;

} TagRecord;

TagRecord * TagRecordArray = NULL;
long NumTagRecords = 0;

/* Function prototypes */
void Input ( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void Init( GlgObject viewport );
TagRecord * CreateTagRecords( GlgObject tag_list );
void PollTagData( GlgAnyType data, GlgLong * timer_id );
double GetDTagData( char * tag_source );
char * GetSTagData( char * tag_source );

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

   /* Store tag information as an array of records for faster processing */
   TagRecordArray = CreateTagRecords( TagList );
   GlgDropObject( TagList );

   /* Start update timer */
   if( NumTagRecords )
     GlgAddTimeOut( AppContext, UpdateInterval, (GlgTimerProc)PollTagData,
                   NULL );
}

/*--------------------------------------------------------------------
| Traverses a tag list and parses tag information to store it as an array
| of records for faster processing.
*/
TagRecord * CreateTagRecords( GlgObject tag_list )
{
   GlgObject tag_obj;
   char * tag_source;
   long i, size;
   double dtype;

   NumTagRecords = 0;
   if( !tag_list )
     return NULL;   
   
   size = GlgGetSize( tag_list );
   if( !size )
     return NULL;
   
   TagRecordArray = GlgAlloc( sizeof( TagRecord ) * size );

   NumTagRecords = 0;
   for( i=0; i<size; ++i )
   {
      tag_obj = GlgGetElement( tag_list, i );

      /* Get the name of the database field to use as a data source */
      GlgGetSResource( tag_obj, "TagSource", &tag_source );
      if( !tag_source || !*tag_source )
	continue;   /* Skip empty tag sources */

      /* Get tag object's data type: GLG_D, GLG_S or GLG_G */
      GlgGetDResource( tag_obj, "DataType", &dtype );

      TagRecordArray[ NumTagRecords ].tag_source = GlgStrClone( tag_source );
      TagRecordArray[ NumTagRecords ].data_type = dtype;

      /* For optional performance optimization. */
      TagRecordArray[ NumTagRecords ].tag_obj = tag_obj;

      ++NumTagRecords;    /* Number of used tag records */
   }
   return TagRecordArray;
}

/*--------------------------------------------------------------------
| Traverses the array of tag records, gets new data for each tag and 
| updates the drawing with new values.
*/
void PollTagData( GlgAnyType data, GlgLong * timer_id )
{
   TagRecord * tag_record;
   double d_value;
   char * s_value;
   long i;   

   for( i=0; i<NumTagRecords; ++i )
   {
      tag_record = &TagRecordArray[ i ];

      switch( tag_record->data_type )
      {
       case GLG_D:
         d_value = GetDTagData( tag_record->tag_source );

#if USE_SIMPLE_WAY
         GlgSetDTag( viewport, tag_record->tag_source, d_value, True );

#else  /* Optional performance optimization: faster */
         GlgSetDResourceIf( tag_record->tag_object, NULL, d_value, True );
#endif
         break;

       case GLG_S:         
         s_value = GetSTagData( tag_record->tag_source );

#if USE_SIMPLE_WAY
         GlgSetSTag( viewport, tag_record->tag_source, s_value, True );

#else  /* Faster */
         GlgSetSResourceIf( tag_record->tag_object, NULL, a_value, True );
#endif
         break;
      }
   }

   /* Update the drawing with new data. */
   GlgUpdate( viewport );

   /* Restart timer to continue generating data. */
   GlgAddTimeOut( AppContext, UpdateInterval, (GlgTimerProc)PollTagData, 
                 NULL );
}

/*--------------------------------------------------------------------
| Queries new D tag value from the database.
*/
double GetDTagData( char * tag_source )
{
#if SIMULATED_DATA 
   /* Simulate with random data */
   return GlgRand( 0., 1. );
#else

   /* Replace with with application's code to query D tag value. */
   return 0;
#endif
}

/*--------------------------------------------------------------------
| Queries new S tag value from the database.
*/
char * GetSTagData( char * tag_source )
{   
#if SIMULATED_DATA 
   /* Simulate with random data */
   return GlgRand( 0., 1. ) > 0.5 ? "On" : "Off";
#else

   /* Replace with with application's code to query D tag value. */
   return "undefined";
#endif
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
