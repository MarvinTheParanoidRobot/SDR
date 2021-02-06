#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "GlgViewer.h"

// Function prototype.
void OnTimerEvent( GlgViewer *viewer, GlgLong * timer_id );

/*----------------------------------------------------------------------
| Constructor. 
*/
GlgViewer::GlgViewer()
{
   UpdateInterval = 100; 
   TimerID = 0;
   TagRecordArray = NULL;
   NumTagRecords = 0;
   has_parent = false;
   DataFeed = NULL;

   // Set to False to use live data by default. 
   // It can be overriden by a command line option, 
   // -random-data or -live-data
   RANDOM_DATA = True; 
}

/*----------------------------------------------------------------------
| Destructor. 
*/
GlgViewer::~GlgViewer()
{
   // Clear TagRecordArray.
   DeleteTagRecords();
   
   // Delete DataFeed object, if defined. 
   if( DataFeed )
     delete DataFeed;
}

/*----------------------------------------------------------------------
| Add DataFeed object for supplying data for animation.
*/
void GlgViewer::AddDataFeed( DataFeedC * data_feed )
{
   if( DataFeed )
     delete DataFeed;
   
   DataFeed = data_feed;
}

/*----------------------------------------------------------------------
| Initialize drawing parameters as needed. Callbacks must be enabled
| before hierarchy setup.
*/
void GlgViewer::Init( void )
{
   /* Initialize drawing parameters before hierarchy is set up. */
   InitBeforeH();
   
   /* Set up object hierarchy. */ 
   SetupHierarchy();

   /* Initialize drawing parameters after hierarchy setup took place. */
   InitAfterH();
}

/*--------------------------------------------------------------------
| Place custom code as needed to initialize  the drawing before
| hierarchy setup took place.
*/
void GlgViewer::InitBeforeH()
{
   /* Callbacks must be enabled before  hierarchy setup.
      Enable callbacks only if the viewer object is a top level window and 
      there is no parent. 
      
      When this class is used in an MFC application, GlgSCADAViewer is 
      displayed in a GLG MFC control, has_parent=true and callbacks are 
      enabled at the parent level in GlgViewerControl::Init() before 
      the hierarchy setup.
   */
   if( !has_parent )
      EnableCallback( GLG_INPUT_CB, NULL );

   /* Place custom code here. */
}

/*--------------------------------------------------------------------
| Initialize the drawing after hierarchy setup. In this example, 
| it builds an array of tag records, which will be used to animate
| the drawing.
*/
void GlgViewer::InitAfterH()
{
   /* Store tag information as an array of records for faster processing */
   CreateTagRecords();
}

/*--------------------------------------------------------------------
| Obtains a list of tags in the loaded drawing and builds
| an array of tag records used to animate the drawing in a timer procedure
| UpdateDrawing().
*/
void GlgViewer::CreateTagRecords()
{
   CONST char 
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
   GlgObjectC tag_list = (GlgObjectC) CreateTagList( /* List each tag source only once */ True );

   /* CreateTagList creates an object (a group containing a list of tags).
      The returned object has to be explicitly dereferenced to prevent a 
      memory leak. The object is still referenced by the tag_list variable 
      instance.
   */
   tag_list.Drop();

   if( tag_list.IsNull() )
     return;   
   
   size = tag_list.GetSize();
   if( !size )
     return;   /* no tags found */
   
   TagRecordArray = (TagRecordC **) GlgAlloc( sizeof( TagRecordC * ) * size );

   for( i=0; i<size; ++i )
   {
      GlgObjectC tag_obj = (GlgObjectC) tag_list.GetElement( i );

      /* Get the name of the database field to use as a data source */
      tag_obj.GetResource( (char *) "TagSource", &tag_source );

      /* Obtain TagName and TagComment and use it as needed. */
      tag_obj.GetResource( "TagName", &tag_name );
      tag_obj.GetResource( "TagComment", &tag_comment );

      /* Skip tags with undefined TagSource */
      if( !tag_source || !*tag_source || ( strcmp( tag_source, "unset" ) == 0 ) )
	continue;

      /* Obtain tag access type. */
      tag_obj.GetResource( "TagAccessType", &access_type );

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
      tag_obj.GetResource( "DataType", &dtype );

      /* Populate a new tag record in TagRecordArray. */
      /* Create a new tag record. */
      TagRecordC * tag_record = new TagRecordC();
      tag_record->tag_source = GlgStrClone( (char*) tag_source );
      tag_record->data_type = (int) dtype;
      tag_record->tag_obj = tag_obj;

      /* For further performance optimization, set if_changed=true which will
         push the value into the tag only if the value has changed.
         The if_changed flag is ignored for tags attached to the plots 
         in a real time chart, and the new value is always pushed to the 
         chart even if it is the same.
      */
      tag_record->if_changed = True;        

      // Store new tag_record in TagRecordArray.
      TagRecordArray[ NumTagRecords ] = tag_record;

      ++NumTagRecords;    /* Number of used tag records */
   }

   if( !NumTagRecords )
   {
      GlgFree( TagRecordArray );
      TagRecordArray = NULL;
   }
}

/*--------------------------------------------------------------------
| Clear TagRecordArray.
*/
void GlgViewer::DeleteTagRecords()
{
   GlgLong i;

   if( !NumTagRecords )
      return;

   /* Free memory for the tag_source */
   for( i = 0; i < NumTagRecords; ++i )
      delete TagRecordArray[i];
      
   /* Free memory allocated for the TagRecordArray */
   GlgFree( TagRecordArray );
   
   TagRecordArray = NULL;
   NumTagRecords = 0;
}

/*--------------------------------------------------------------------
| Initialize tag with a given tag source.
*/
void GlgViewer::InitTag( CONST char * tag_source )
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
   SetTag( tag_source, value, False );
}


/*----------------------------------------------------------------------
| Timer procedure, invoked periodically on a timer to update the
| drawing with new data values.
*/
void OnTimerEvent( GlgViewer *viewer, GlgLong * timer_id )
{
   if( !viewer->NumTagRecords )
   {
      viewer->TimerID = 0;
      return;   // Don't restart the timer: no tags found.
   }

   // Update the drawing with new data values. 
   viewer->UpdateDrawing();

   /* Restart the timer. */
   viewer->TimerID = GlgAddTimeOut( viewer->AppContext, viewer->UpdateInterval, 
                                    (GlgTimerProc)OnTimerEvent, viewer );

}

/*--------------------------------------------------------------------
| Animate the drawing: 
| - Traverse TagRecordArray
| - For each tag record, obtain a new data value and push it into the graphics
| - Refresh the display.
*/
void GlgViewer::UpdateDrawing()
{
   TagRecordC * tag_record;
   double d_value;
   char * s_value;
   long i;   
   bool status;
   
   if( !NumTagRecords )
     return;

   if( !DataFeed )
   {
      GlgError( GLG_USER_ERROR, (char *) "Invalid DataFeed object, drawing animation failed." );
      return;
   }
   
   for( i=0; i<NumTagRecords; ++i )
   {
      tag_record = TagRecordArray[ i ];

      switch( tag_record->data_type )
      {
       case GLG_D:
         /* Obtain a new data value. */
         status = DataFeed->ReadDValue( tag_record, &d_value );
         if( !status )
         {
            /* Generate an error. */
            GlgError( GLG_USER_ERROR, (char *) "ReadDValue failed." );
            continue;
         }

         /* Push new value into graphics. */
         SetTag( tag_record->tag_source, d_value, 
                 tag_record->if_changed /*if_changed*/ );

         /* Optional performance optimization: faster */
         /* tag_record->tag_object.SetResource( NULL, d_value, tag_record->if_changed );
         */
         break;

       case GLG_S:         
         /* Obtain a new data value. */
         status = DataFeed->ReadSValue( tag_record, &s_value );
         if( !status )
         {
            /* Generate an error. */
            GlgError( GLG_USER_ERROR, (char *) "ReadSValue failed." );
            continue;
         }

         /* Push the new value into graphics. */
         SetTag( tag_record->tag_source, s_value, tag_record->if_changed /*if_changed*/ );

         /* Optional performance optimization: Faster */
         /* tag_record->tag_object.SetResourceIf( NULL, s_value, tag_record->if_changed ); 
         */
         break;
      }
   }

   /* Update the drawing with new data. */
   Update();
   Sync();    /* Improves interactive response. */
}

/*----------------------------------------------------------------------
| Handle user interaction. 
*/
void GlgViewer::Input( GlgObjectC& viewport, GlgObjectC& message )
{
   CONST char
     * format,
     * action,
     * origin;
      
   bool status = False;
   double d_value = -1.;;

   /* Get the message's format, action and origin. */
   message.GetResource( "Format", &format );
   message.GetResource( "Action", &action );
   message.GetResource( "Origin", &origin );

   /* Handle window closing. May use viewport's name. */
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );
   else if( strcmp( format, "Button" ) == 0 )
   {
      if(  strcmp( action, "Activate" ) !=0 && 
           strcmp( action, "ValueChanged" ) !=0 )
        return;

      if( strcmp( action, "Activate" ) == 0 ) /* Push button event */
      {
         if( strcmp( origin, "StartButton")  == 0 )
         {
            d_value = 1.;

            // Open the valve and start the pipe flow.
            if( RANDOM_DATA )
            {
               status = DataFeed->WriteDValue( "State", d_value );
               StartUpdates();
            }
            else
              // Place custom code here to write value to the appropriate tag.
              // status = DataFeed->WriteDValue( "State", d_value );
              ;
         }
         else if( strcmp( origin, "StopButton")  == 0 )
         {
            d_value = 0.;
            
            // Close the valve and stop the pipe flow.
            if( RANDOM_DATA )
            {
               status = DataFeed->WriteDValue( "State", d_value );
               StopUpdates();
            }
            else
              // Place custom code here to write value to the appropriate tag.
              // status = DataFeed->WriteDValue( "State", d_value );
              ;
         }
         else if( strcmp( origin, "QuitButton" ) == 0 )
         {
            exit( 0 );
         }
      }
      else if(  strcmp( action, "ValueChanged" ) == 0 ) /* Toggle button */
      {
         double state;
         message.GetResource( "OnState", &state );

         /* Place custom code here to handle toggle buttons as needed. */
      } 

      /* Refresh the display. */
      Update();
   } 
   else if( strcmp( format, "Timer" ) == 0 )
   {
      /* Refresh the display for the objects with integrated Timer dynamics. */
      Update();
   }
}


/*----------------------------------------------------------------------
| Start dynamic updates.
*/
void GlgViewer::StartUpdates()
{
   /* Start update timer. */
   TimerID = GlgAddTimeOut( AppContext, UpdateInterval, 
                           (GlgTimerProc)OnTimerEvent, this );
}


/*----------------------------------------------------------------------
| Stop dynamic updates.
*/
void GlgViewer::StopUpdates()
{
   /* Stop update timers */
   if( TimerID )
   {
      GlgRemoveTimeOut( TimerID );
      TimerID = 0;
   }
}

/*----------------------------------------------------------------------
| Set viewer size in screen cooridnates. 
*/
void GlgViewer::SetSize( GlgLong x, GlgLong y, 
                         GlgLong width, GlgLong height )
{
   SetResource( "Point1", 0., 0., 0. );
   SetResource( "Point2", 0., 0., 0. );

   SetResource( "Screen/XHint", (double) x );
   SetResource( "Screen/YHint", (double) y );
   SetResource( "Screen/WidthHint", (double) width );
   SetResource( "Screen/HeightHint", (double) height );
}

// Assignment operator
GlgViewer& GlgViewer::operator= ( const GlgObjectC& object )
{
   GlgObjectC::operator=( object );
   return *this;
}
