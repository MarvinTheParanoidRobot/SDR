#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "GlgViewer.h"
#include "DemoDataFeed.h"
#include "LiveDataFeed.h"

/* If GUI is not keeping up with refreshing graphics due to very high rate 
   of incoming data, the size of the accumulated data will grow exponentially.
   This example generates a warning if it happens.
*/
#define HIGH_WATER_MARK_LIMIT   100000

// Function prototypes
extern "C"
{ 
   void UpdateDisplay( GlgViewer * glg_viewer, GlgLong * timer_id );
}

/*----------------------------------------------------------------------
| Constructor. 
*/
GlgViewer::GlgViewer()
{
   // Set to False to use live data by default. 
   // It can be overriden by a command line option, 
   // -random-data or -live-data
   RANDOM_DATA = GlgTrue; 

   // Default update interval. 
   UpdateInterval = 100;   

   TimerID = 0;
   NumTagRecords = 0;
   has_parent = GlgFalse;
   DataFeed = NULL;

   GUIData = new BaseDataVectorType();
}

/*----------------------------------------------------------------------
| Destructor. 
*/
GlgViewer::~GlgViewer()
{
   // Clear TagRecords.
   DeleteTagRecords();
   
   BaseData::ClearVector( GUIData );
   delete GUIData;

   // Delete DataFeed object, if defined. Clear GUIData if not empty. 
   if( DataFeed )
   {
      delete DataFeed;
      DataFeed = NULL;
   }
}

/*----------------------------------------------------------------------
| Create DataFeed object to supply data for animation.
| If RANDOM_DATA=True, use simulated data, otherwise use
| live data. An application should provide a custom implementation
| of LiveDataFeed class which derives from DataFeedBase.
*/
void GlgViewer::AddDataFeed( void )
{
   if( RANDOM_DATA )
     AddDataFeed( new DemoDataFeed( this ) );
   else
     AddDataFeed( new LiveDataFeed( this ) );
}

/*----------------------------------------------------------------------
| Overloaded: Add DataFeed object passed as a data_feed parameter.
*/
void GlgViewer::AddDataFeed( DataFeedBase * data_feed )
{
   if( DataFeed )
     delete DataFeed;
   
   DataFeed = data_feed;
}

/*----------------------------------------------------------------------
| Delete DataFeed object if any.
*/
void GlgViewer::DeleteDataFeed( void )
{
   if( !DataFeed )
     return;

   delete DataFeed;
   DataFeed = NULL;
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
   SCONST char 
     * tag_source,
     * tag_name,
     * tag_comment;
   GlgLong 
     i,
     size;
   double dtype;
   double access_type;

   NumTagRecords = 0;
   
   /* Query a list of tags defined in the drawing. Obtain unique
      tag sources only, listing a tag source only once.
   */
   GlgObjectC tag_list = 
     (GlgObjectC) CreateTagList( /* list tag source once*/ GlgTrue );

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

   // Populate TagRecords vector.
   for( i=0; i<size; ++i )
   {
      GlgObjectC tag_obj = (GlgObjectC) tag_list.GetElement( i );

      /* Get the name of the database field to use as a data source */
      tag_obj.GetResource( (char *) "TagSource", &tag_source );

      /* Obtain TagName and TagComment and use it as needed. */
      tag_obj.GetResource( "TagName", &tag_name );
      tag_obj.GetResource( "TagComment", &tag_comment );

      /* Skip tags with undefined TagSource */
      if( IsUndefined( tag_source ) )
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

      /* Populate a new tag record in TagRecords. */
      /* Create a new tag record. */
      TagRecord * tag_record = new TagRecord();
      tag_record->tag_source = GlgStrClone( tag_source );
      tag_record->data_type = (int) dtype;
      tag_record->tag_obj = tag_obj;

      /* For further performance optimization, set if_changed=true which will
         push the value into the tag only if the value has changed.
         Set if_changed = false for the tags assigned to the charts.
      */
      if( tag_comment && strstr( tag_comment, "Chart" ) )
        tag_record->if_changed = GlgFalse;
      else
        tag_record->if_changed = GlgTrue;        

      // Store new tag_record in TagRecords.
      TagRecords.push_back( tag_record );
   }

   NumTagRecords = TagRecords.size();    /* Number of used tag records */
}

/*--------------------------------------------------------------------
| Clear TagRecords.
*/
void GlgViewer::DeleteTagRecords()
{
   if( !NumTagRecords )
      return;

   TagRecordsType::iterator it;
   /* Free memory for the vector elements  */
   for( it = TagRecords.begin(); it != TagRecords.end(); ++it )
     delete *it;
   
   TagRecords.clear();
   NumTagRecords = 0;
}

/*--------------------------------------------------------------------
| Initialize tag with a given tag source.
*/
void GlgViewer::InitTag( SCONST char * tag_source )
{
   double value = -1.0;
   
   if( RANDOM_DATA )
   {
      /* Initialize "State" tag. */
      if( StrEqual( tag_source, "State" ) )
        value = 1.0;
   }
   else;
   // Place custom code here to set value as needed.
   
   // Set the tag value in the drawing.
   SetTag( tag_source, value, GlgFalse );
}

/*----------------------------------------------------------------------
| Timer procedure to push accumulated data into graphics
| and repaint the display. 
*/
void UpdateDisplay( GlgViewer * viewer, GlgLong * timer_id )
{
   GlgULong sec1, microsec1;

   GlgGetTime( &sec1, &microsec1 );  /* Start time */

   // Obtain data values and push them into graphics.
   viewer->GetGUIData();
   viewer->ProcessGUIData();    
   
   viewer->Update();       // Repaints the drawing.
   viewer->Sync( );        // Improves interactive response.

   // Get adjusted time interval.
   GlgLong timer_interval = 
     GetAdjustedTimeout( sec1, microsec1, viewer->UpdateInterval );

   // Reinstall the timeout to continue updating.
   viewer->TimerID = 
     GlgAddTimeOut( viewer->AppContext, timer_interval, 
                    (GlgTimerProc)UpdateDisplay, viewer );
}

/*----------------------------------------------------------------------
| Access real-time data accumulated so far in a buffer.
*/
void GlgViewer::GetGUIData( void )
{
   if( TagRecords.empty() )
       return;
   
   /* Get data accumulated in the data thread in the AccumulatedData vector
      since the last query, and store them in GUIData vector.
      We simply swap the vectors containing the old and new accumulated data,
      and use the old vector to accumulate the next portion of the data.
   */
   GUIData = DataFeed->GetAccumulatedData( GUIData );
   if( GUIData->size() > HIGH_WATER_MARK_LIMIT )
     GlgError( GLG_USER_ERROR, 
               "GIU is overwhelmed with data, use merging or drop data!" );

#if DEBUG
   printf( "GUIData size: %d\n", (int) GUIData->size() );
#endif
}

/*----------------------------------------------------------------------
| Push all data accumulated in the GUIData vector into graphics. 
*/
void GlgViewer::ProcessGUIData( void )
{
   if( GUIData->empty() )
     return;

   BaseDataVectorType::iterator it;
   for( it = GUIData->begin(); it != GUIData->end(); ++it )
   {
      BaseData * data = *it;
      PushData( data );
   }
}

/*----------------------------------------------------------------------
| Process data in a provided data structure based on the structure type
| (data->type). For each field in a data structure, push the data value into
| a tag with a matching TagSource and data type, if found. 
|
| !!!!! IMPORTANT: TagSource in the drawing MUST MATCH hardcoded 
| tag source strings below. For example, the tag sources in the drawing 
| must be "LAT" and "LON" to push lat and lon values from the
| GPS data structure. 
|
| If the application navigates through multiple drawings, not all
| data fields in the incoming data structures may have corresponding
| tags in the currently loaded drawing. 
|
| PushTagData function performs a check to make sure a data value
| is pushed to the graphics only if the tag with a matching TagSource
| exists in the currently loaded drawing.
|
| The application can modify the tag sources and data structures as needed.
*/
void GlgViewer::PushData( BaseData * data )
{
   GPSData * gps_data;
   TelemetryData * telem_data;

   switch( data->type )
   {
    case GPS:
      gps_data = (GPSData *) data;
      PushTagData( "LAT", gps_data->lat );
      PushTagData( "LON", gps_data->lon );
      PushTagData( "SPEED", gps_data->speed );
      PushTagData( "ALTITUDE", gps_data->altitude );
      PushTagData( "PITCH", gps_data->pitch );
      PushTagData( "ROLL", gps_data->roll );
      PushTagData( "YAW", gps_data->yaw );
      PushTagData( "POSITION", gps_data->lat, gps_data->lon, 
                   gps_data->altitude );
      break;

    case TELEMETRY:
      telem_data = (TelemetryData *) data;
      PushTagData( "POWER", telem_data->power );
      PushTagData( "VOLTAGE", telem_data->voltage );
      PushTagData( "CURRENT", telem_data->current );
      PushTagData( "TEMPERATURE", telem_data->temperature );
      PushTagData( "PRESSURE", telem_data->pressure );
      PushTagData( "STATE_HEALTH", telem_data->state_health );
      break;

    default:
      GlgError( GLG_USER_ERROR, 
                "Unknown Data Structure Type." );
      exit( GLG_EXIT_ERROR );
   }
}
   
/*--------------------------------------------------------------------
| Push a double value into a D-type tag, if found.
| Verify that the tag with a specified tag source is present in the drawing
| before pushing the value to the graphics.
*/
GlgBoolean GlgViewer::PushTagData( SCONST char * data_source, double d_value )
{
   // Find a tag record with a matching tag source and data type.
   TagRecord * tag_record = LookupTagRecords( data_source, GLG_D );
   
   if( !tag_record ) 
     return GlgFalse;

   // Push value into graphics.
   SetTag( tag_record->tag_source, d_value, tag_record->if_changed );
      
   return GlgTrue;
}

/*----------------------------------------------------------------------
| Push x,y,z values into a G-type tag, if found. 
| Verify that the tag with a specified tag source is present in the drawing
| before pushing the value to the graphics.
*/
GlgBoolean GlgViewer::PushTagData( SCONST char * data_source, 
                                    double x, double y, double z )
{
   // Find a tag record with a matching tag source and data type.
   TagRecord * tag_record = LookupTagRecords( data_source, GLG_G );
   
   if( !tag_record ) 
     return GlgFalse;

   // Push value into graphics.
   SetTag( tag_record->tag_source, x, y, z, tag_record->if_changed );
      
   return GlgTrue;
}

/*----------------------------------------------------------------------
| Push a string value into an S-type tag, if found.
| Verify that the tag with a specified tag source is present in the drawing
| before pushing the value to the graphics.
*/
GlgBoolean GlgViewer::PushTagData( SCONST char * data_source, 
                                   SCONST char * s_value )
{
   // Find a tag record with a matching tag source and data type.
   TagRecord * tag_record = LookupTagRecords( data_source, GLG_S );
   
   if( !tag_record ) 
     return GlgFalse;

   // Push value into graphics.
   SetTag( tag_record->tag_source, s_value, tag_record->if_changed );
      
   return GlgTrue;
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
      
   bool status = GlgFalse;
   double d_value = -1.;;

   /* Get the message's format, action and origin. */
   message.GetResource( "Format", &format );
   message.GetResource( "Action", &action );
   message.GetResource( "Origin", &origin );

   /* Handle window closing. May use viewport's name. */
   if( StrEqual( format, "Window" ) && StrEqual( action, "DeleteWindow" ) )
   {
     exit( 0 );
   }
   else if( StrEqual( format, "Button" ) )
   {
      if( !StrEqual( action, "Activate" ) && 
          !StrEqual( action, "ValueChanged" ) )
        return;

      /* In this demo, Start and Stop buttons are used
         for demo purposes demonstrting  how to start/stop data updates.
      */
      if( StrEqual( action, "Activate" ) ) /* Push button event */
      {
         if( StrEqual( origin, "StartButton") )
         {
            // Start data updates.
            StartUpdates();
         }

         else if( StrEqual( origin, "StopButton") )
         {
            // Stop data updates.
            StopUpdates();
         }
         else if( StrEqual( origin, "QuitButton" ) )
         {
            exit( 0 );
         }
      }
      else if(  StrEqual( action, "ValueChanged" ) ) /* Toggle button */
      {
         double state;
         message.GetResource( "OnState", &state );

         /* Place custom code here to handle toggle buttons as needed. */
      } 

      /* Refresh the display. */
      Update();
   } 
   else if( StrEqual( format, "Timer" ) )
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
   if( !DataFeed )
   {
      GlgError( GLG_USER_ERROR, 
                "No DataFeed object, can't start data updates." );
      return;
   }

   DataFeed->StartUpdates();    // Start data thread.

   // Start a timer to update the graphics with data.
   if( !TimerID )
     TimerID = GlgAddTimeOut( AppContext, UpdateInterval, 
                              (GlgTimerProc) UpdateDisplay, this );
}

/*----------------------------------------------------------------------
| Stop dynamic updates.
*/
void GlgViewer::StopUpdates()
{
   if( !DataFeed )
   {
      GlgError( GLG_USER_ERROR, 
                "No DataFeed object, can't stop data updates." );
      return;
   }

   DataFeed->StopUpdates();    // Stop data thread.
   
   if( !TimerID )
     return;
   
   GlgRemoveTimeOut( TimerID );
   TimerID = 0;
}

// Assignment operator
GlgViewer& GlgViewer::operator= ( const GlgObjectC& object )
{
   GlgObjectC::operator=( object );
   return *this;
}

/*--------------------------------------------------------------------
| Returns a tag record with a matching tag_source and data_type.
*/
TagRecord * GlgViewer::LookupTagRecords( SCONST char * tag_source, 
                                         GlgLong data_type )
{  
   TagRecordsType::iterator it;
   for( it = TagRecords.begin(); it != TagRecords.end(); ++it )
   {
      TagRecord * tag_record = *it;
      if( StrEqual( tag_record->tag_source, tag_source ) &&
          tag_record->data_type == data_type )
        return tag_record;
   }

   return NULL; // not found
}
