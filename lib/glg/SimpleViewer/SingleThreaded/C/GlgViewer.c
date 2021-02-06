/**********************************************************************
| Supported command line options:
|
| -random-data  
|        uses simulated demo data for animation
|
| -live-data
|        uses live application data for animatx1ion
|
| <filename>
|        specifies GLG drawing filename to be loaded and animated;
|        if not defined, DEFAULT_DRAWING_FILENAME is used.
***********************************************************************/

#include "GlgViewer.h"

/* Set to False to provide live application data. May be overriden via command
   line option -random-data or -live-data.
*/
GlgBoolean RANDOM_DATA=True;

/* Top level viewport */
GlgObject Viewport;

char* DEFAULT_DRAWING_FILENAME="tags_example.g";
GlgLong UpdateInterval=100;  /* Update interval in msec */

TagRecord* TagRecordArray = NULL;
GlgLong NumTagRecords = 0;

GlgLong TimerID = 0;
GlgAppContext AppContext;

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*--------------------------------------------------------------------*/
int GlgMain( int argc, char * argv[], GlgAppContext InitAppContext )
{
   char * drawing_filename = NULL;
   int skip;

   AppContext = GlgInit( False, InitAppContext, argc, argv );

   /* Process command line arguments. */
   for( skip = 1; skip < argc; ++skip )
   {
      if( strcmp( argv[ skip ], "-random-data" ) == 0 )
      {
         /* Use simulated demo data for animation. */
         RANDOM_DATA = True;
         GlgError( GLG_INFO, (char *) "Using simulated data for animation." );
      }
      else if( strcmp( argv[ skip ], "-live-data" ) == 0 )
      {
         /* Use live application data for animation. */
         RANDOM_DATA = False;
         GlgError( GLG_INFO, (char *) "Using live application data for animation." );
      }
      else if( strncmp( argv[skip], "-", 1 ) == 0 )
        continue;
      else
      {
         /* Use the drawing file from the command line, if any.*/
         drawing_filename = argv[ skip ];
      }
   }

   /* If drawing file is not supplied on the command line, use 
      default drawing filename defined by DEFAULT_DRAWING_FILENAME.
   */
   if( !drawing_filename )
   {
      GlgError( GLG_INFO, 
          "Drawing file is not supplied on command line, using default drawing tags_example.g" );
      drawing_filename = DEFAULT_DRAWING_FILENAME;
   }

   /* Load a drawing from the file. */
   Viewport = GlgLoadWidgetFromFile( drawing_filename );
   if( !Viewport )
   {
      GlgError( GLG_USER_ERROR, "Can't load drawing." );
      exit( GLG_EXIT_ERROR );
   }
   
   /* Setting top level window dimensions. */
   GlgSetGResource( Viewport, "Point1", 0., 0., 0. );
   GlgSetGResource( Viewport, "Point2", 0., 0., 0. );
   GlgSetDResource( Viewport, "Screen/XHint", 50. );
   GlgSetDResource( Viewport, "Screen/YHint", 50. );
   GlgSetDResource( Viewport, "Screen/WidthHint", 800. );
   GlgSetDResource( Viewport, "Screen/HeightHint", 700. );

   /* Setting the window name (title). */
   GlgSetSResource( Viewport, "ScreenName", "Glg Tags Example" );

   /* Add Input callback to handle user interraction. */
   GlgAddCallback( Viewport, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );

   /* Initialize drawing before hierarchy setup. */
   InitBeforeH();

   /* Setup hierarchy */
   GlgSetupHierarchy( Viewport );

   /* Initialize drawing after hierarchy setup. */
   InitAfterH();

   /* Start periodic dynamic updates. */
   StartUpdates();

   /* Paint the drawing. */   
   GlgUpdate( Viewport );

   return (int) GlgMainLoop( AppContext );
}

/*--------------------------------------------------------------------
| Place custom code as needed to initialize  the drawing before
| hierarchy setup took place.
*/
void InitBeforeH()
{
   /* Place custom code here. */
}

/*--------------------------------------------------------------------
| Initialize the drawing after hierarchy setup. In this example, 
| it builds an array of tag records, which will be used to animate
| the drawing.
*/
void InitAfterH()
{ 
   /* Store tag information as an array of records for faster processing */
   CreateTagRecords( Viewport );
}

/*--------------------------------------------------------------------
| Obtains a list of tags in the loaded drawing and builds
| an array of tag records used to animate the drawing in a timer procedure
| UpdateDrawing().
*/
void CreateTagRecords( GlgObject viewport )
{
   GlgObject 
     tag_list,
     tag_obj;
   char 
     * tag_source,
     * tag_name,
     * tag_comment;
   GlgLong
     i, 
     size;
   double dtype;
   double access_type;

   NumTagRecords = 0;
   
   /* Query a list of tags defined in the drawing */
   tag_list = 
     GlgCreateTagList( viewport, /* List each tag source only once */ True );

   if( !tag_list )
     return;   
   
   size = GlgGetSize( tag_list );
   if( !size )
   {
      GlgDropObject( tag_list );
      return;
   }
   
   TagRecordArray = GlgAlloc( sizeof( TagRecord) * size );

   for( i=0; i<size; ++i )
   {
      tag_obj = GlgGetElement( tag_list, i );

      /* Get the name of the database field to use as a data source */
      GlgGetSResource( tag_obj, "TagSource", &tag_source );

      /* Obtain TagName and TagComment and use it as needed. */
      GlgGetSResource( tag_obj, "TagName", &tag_name );
      GlgGetSResource( tag_obj, "TagComment", &tag_comment );

      /* Skip tags with undefined TagSource */
      if( !tag_source || !*tag_source || ( strcmp( tag_source, "unset" ) == 0 ) )
	continue;

      /* Obtain tag access type. */
      GlgGetDResource( tag_obj, "TagAccessType", &access_type );

      /* Skip OUTPUT tags and INIT_ONLY tags. 
         Initialiaze INPUT_ONLY tags as needed. 
      */
      if( access_type == GLG_OUTPUT_TAG )
	continue;
      else if( access_type == GLG_INIT_ONLY_TAG )
      {
         InitTag( tag_source );
         continue;
      }

      /* Get tag object's data type: GLG_D, GLG_S or GLG_G */
      GlgGetDResource( tag_obj, "DataType", &dtype );

      /* Populate a new tag record in TagRecordArray. */
      TagRecordArray[ NumTagRecords ].tag_source = GlgStrClone( tag_source );
      TagRecordArray[ NumTagRecords ].data_type = dtype;

      /* For optional performance optimization, store the tag object ID,
         it may be used for direct access.
      */
      TagRecordArray[ NumTagRecords ].tag_obj = tag_obj;

      /* For further performance optimization, set if_changed=true which will
         push the value into the tag only if the value has changed.
         The if_changed flag is ignored for tags attached to the plots 
         in a real time chart, and the new value is always pushed to the 
         chart even if it is the same.
      */
      TagRecordArray[ NumTagRecords ].if_changed = True;        

      ++NumTagRecords;    /* Number of used tag records */
   }

   if( !NumTagRecords )
   {
      GlgFree( TagRecordArray );
      TagRecordArray = NULL;
   }

   /* Dereference tag_list object */
   GlgDropObject( tag_list );
}

/*--------------------------------------------------------------------
| Clear TagRecordArray.
*/
void DeleteTagRecords()
{
   GlgLong i;

   if( !NumTagRecords )
      return;

   /* Free memory for the tag_source */
   for( i = 0; i < NumTagRecords; ++i )
      GlgFree( TagRecordArray[i].tag_source );
   
   /* Free memory allocated for the TagRecordArray */
   GlgFree( TagRecordArray );
   
   TagRecordArray = NULL;
   NumTagRecords = 0;
}


/*--------------------------------------------------------------------
| Initialize tag with a given tag source.
*/
void InitTag( char * tag_source )
{
   double value = -1.0;
   
   if( RANDOM_DATA )
   {
      /* Initialize "State" tag. */
      if( strcmp( tag_source, "State" ) == 0 )
        value = 1.0;
   }
   else;
   // Place custom code here to set value as needed.
   
   // Set the tag value in the drawing.
   GlgSetDTag( Viewport, tag_source, value, False );
}

/*--------------------------------------------------------------------
| Animate the drawing: 
| - Traverse TagRecordArray
| - For each tag record, obtain a new data value and push it into the graphics
| - Refresh the display.
*/
void UpdateDrawing( GlgAnyType data, GlgLong * timer_id )
{
   TagRecord* tag_record;
   double d_value;
   char * s_value;
   long i;   
   GlgBoolean status;

   for( i=0; i<NumTagRecords; ++i )
   {
      tag_record = &TagRecordArray[ i ];

      switch( tag_record->data_type )
      {
       case GLG_D:
         /* Obtain a new data value. */
	 if( RANDOM_DATA )
	   status = DemoReadDValue( tag_record, &d_value );
         else
	   status = ReadDValue( tag_record, &d_value );

         if( !status )
         {
            /* Generate an error. */
            GlgError( GLG_USER_ERROR, "ReadDValue failed." );
            continue;
         }

         /* Push new value into graphics. */
         GlgSetDTag( Viewport, tag_record->tag_source, d_value, 
                     tag_record->if_changed /*if_changed*/ );

         /* Optional performance optimization: faster */
         /* GlgSetDResourceIf( tag_record->tag_object, NULL, d_value, 
                               tag_record->if_changed );
         */
         continue;

       case GLG_S:         
         /* Obtain a new data value. */
	 if( RANDOM_DATA )
	   status = DemoReadSValue( tag_record, &s_value );
         else
	   status = ReadSValue( tag_record, &s_value );
       
         if( !status )
         {
            /* Generate an error. */
            GlgError( GLG_USER_ERROR, "ReadSValue failed." );
            break;
         }

         /* Push the new value into graphics. */
         GlgSetSTag( Viewport, tag_record->tag_source, s_value, 
                     tag_record->if_changed /*if_changed*/ );

         /* Optional performance optimization: Faster */
         /* GlgSetSResourceIf( tag_record->tag_object, NULL, a_value, 
                               tag_record->if_changed ); 
         */
         break;
      }
   }

   /* Update the drawing with new data. */
   GlgUpdate( Viewport );
   GlgSync( Viewport );    /* Improves interactive response. */

   /* Restart the timer. */
   TimerID = GlgAddTimeOut( AppContext, UpdateInterval, (GlgTimerProc)UpdateDrawing, NULL );
}

/*----------------------------------------------------------------------
| Start dynamic updates.
*/
void StartUpdates()
{
   /* Start update timer. */
   TimerID = GlgAddTimeOut( AppContext, UpdateInterval, 
                           (GlgTimerProc)UpdateDrawing, NULL );
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
| Handle user interaction. 
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
      
   GlgBoolean status = False;
   double d_value = -1.;;

   message_obj = (GlgObject) call_data;

   /* Get the message's format, action and origin. */
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "Origin", &origin );

   /* Handle window closing. May use viewport's name. */
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );
   else if( strcmp( format, "Button" ) == 0 )
   {
      if( strcmp( action, "Activate" ) == 0 ) /* Push button event */
      {
         if( strcmp( origin, "StartButton")  == 0 )
         {
            d_value = 1.;

            // Open the valve and start the pipe flow.
            if( RANDOM_DATA )
            {
               status = DemoWriteDValue( "State", d_value );
               StartUpdates();
            }
            else
              status = WriteDValue( "State", d_value );
         }
         else if( strcmp( origin, "StopButton")  == 0 )
         {
            d_value = 0.;
            // Close the valve and stop the pipe flow.
            if( RANDOM_DATA )
            {
               status = DemoWriteDValue( "State", d_value );
               StopUpdates();
            }
            else
              status = WriteDValue( "State", d_value );
         }
         else if( strcmp( origin, "QuitButton" ) == 0 )
         {
            exit( 0 );
         }
      }
      else if(  strcmp( action, "ValueChanged" ) == 0 ) /* Toggle button */
      {
         double state;
         GlgGetDResource( message_obj, "OnState", &state );

         /* Place custom code here to handle toggle buttons as needed. */
      } 

      /* Refresh the display. */
      GlgUpdate( viewport );
   } 
   else if( strcmp( format, "Timer" ) == 0 )
   {
      /* Refresh the display for the objects with integrated Timer dynamics. */
      GlgUpdate( viewport );
   }
}
