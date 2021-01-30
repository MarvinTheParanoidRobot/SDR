/**********************************************************************
| This C example demonstrates the use of a separate data thread for
| supplying data. The data are then send to the GUI thread using
| messaging implemented with the ZeroMQ library. 
| 
| The C++ version of this example shows an alternative way of 
| exchanging data between the data and GUI threads using a queue
| and thread locks for thread synchronization.
|
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

#define DEBUG_NUM_MESSAGES   1
#define DEBUG_CONNECT        1
#define DEBUG_MESSAGES       0
#define DEBUG_TAGS           0

/* If GUI is not keeping up with refreshing graphics due to very high rate 
   of incoming data, the size of the accumulated data in the socket message 
   queue will grow exponentially. 
   This example generates a warning if it happens.
*/
#define HIGH_WATER_MARK_LIMIT   100000

/* Top level viewport */
GlgObject Viewport;

char * DEFAULT_DRAWING_FILENAME = "tags_example.g";
GlgLong UpdateInterval = 100;  /* Update interval in msec */

TagRecord * TagRecordArray = NULL;
GlgLong NumTagRecords = 0;

GlgLong TimerID = 0;
GlgAppContext AppContext;

#ifndef _WINDOWS
   pthread_t DataThread;
#else
   HANDLE DataThread;
#endif

GlgBoolean DataThreadActive = False;
GlgBoolean StopDataThread = False;

void * ZMQContext = NULL;
void * ZMQReceiver = NULL;

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*--------------------------------------------------------------------*/
int GlgMain( int argc, char * argv[], GlgAppContext InitAppContext )
{
   char * drawing_filename = NULL;
   int skip;

   AppContext = GlgInit( False, InitAppContext, argc, argv );

   ZMQContext = zmq_ctx_new();
   if( !ZMQContext )
   {
      GlgError( GLG_USER_ERROR, (char *) "Can't create ZMQ context, exiting." );
      exit( GLG_EXIT_ERROR );
   } 

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
         GlgError( GLG_INFO, 
                   (char *) "Using live application data for animation." );
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
      GlgError( GLG_INFO, "Drawing file is not supplied on command line, "
                "using default drawing tags_example.g" );
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
   /* Place custom code here to set value as needed. */
   
   /* Set the tag value in the drawing. */
   GlgSetDTag( Viewport, tag_source, value, False );
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
            /* Start data acquisition thread and GUI data updates. */
            StartUpdates();
         }
         else if( strcmp( origin, "StopButton")  == 0 )
         {
            /* Stop data acquisition thread and GUI data updates. */
            StopUpdates();
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

/*----------------------------------------------------------------------
| Start dynamic updates.
*/
void StartUpdates()
{
   GlgBoolean error;
   static void * zcontext = NULL;

   if( DataThreadActive )
     return;

   /* Start data thread that receives data. */
#ifndef _WINDOWS
   error =
     ( pthread_create( &DataThread, NULL, DataThreadFunc, NULL ) != 0 );
#else
   DataThread = CreateThread( NULL, 0, DataThreadFunc, NULL, 0, NULL );
   error = ( DataThread == 0 );
#endif   

   if( error )
   {
      GlgError( GLG_USER_ERROR, "Can't create DataThread.\n" );
      exit( GLG_EXIT_ERROR );
   }
   DataThreadActive = GlgTrue;

   /* Create a ZMQ socket used by the GUI thread to to receive data from
      the data thread.
   */
   ZMQReceiver = CreateZMQReceiver( ZMQContext );

   /* Start update timer that periodically checks pending data and updates 
      the drawing.
   */
   TimerID = GlgAddTimeOut( AppContext, UpdateInterval, 
                           (GlgTimerProc) UpdateDrawing, NULL );
}

/*----------------------------------------------------------------------
| Stop dynamic updates.
*/
void StopUpdates()
{
   if( !DataThreadActive )
     return;

   /* Set StopDataThread flag to true and join data thread with the main thread,
      to make sure the thread will finish execution before it gets destroyed.
   */
   StopDataThread = GlgTrue;

#ifndef _WINDOWS
   pthread_join( DataThread, NULL );
#else
   WaitForSingleObject( DataThread, INFINITE );
   CloseHandle( DataThread );
#endif

   /* Close the receiving socket. */
   zmq_close( ZMQReceiver );
   ZMQReceiver = NULL;
   
   StopDataThread = GlgFalse;
   DataThreadActive = GlgFalse;

   /* Stop update timer. */
   if( TimerID )
   {
      GlgRemoveTimeOut( TimerID );
      TimerID = 0;
   }
}

/*--------------------------------------------------------------------
| Animate the drawing with data.
| If there are new data accumulated in the receiver socket queue,
| get the data and push new values into the GUI, then refresh the 
| display.
*/
void UpdateDrawing( GlgAnyType data, GlgLong * timer_id )
{
   GlgULong sec1, microsec1;
   GlgLong 
     adjusted_interval,
     num_data_messages;
   GlgBoolean got_new_data;
   
   GlgGetTime( &sec1, &microsec1 );  /* Start time */

   got_new_data = False;
   num_data_messages = 0;

   /* Receive new data accumulated in the socket's queue (if any) and 
      push them into the GUI.
      ProcessNewData() returns False if it didn't process any data 
      messages because there were no more queued messages.
   */
   while( ProcessNewData( ZMQReceiver ) )
   {
      got_new_data = True;
      ++num_data_messages;
   }

   if( num_data_messages > HIGH_WATER_MARK_LIMIT )
     GlgError( GLG_USER_ERROR, 
               "GUI is overwhelmed with data, use merging or drop data!" );

#if DEBUG_NUM_MESSAGES   
   printf( "Received %ld messages.\n", (long) num_data_messages );
#endif

   /* Update the drawing with new data, if any. */
   if( got_new_data )
   {
      GlgUpdate( Viewport );
      GlgSync( Viewport );    /* Improves interactive response. */
   }

   adjusted_interval = 
     GetAdjustedTimeout( sec1, microsec1, UpdateInterval );

   /* Restart the timer. */
   TimerID = GlgAddTimeOut( AppContext, adjusted_interval,
                            (GlgTimerProc) UpdateDrawing, NULL );
}

/*----------------------------------------------------------------------
| Retrieves a queued data message from the ZMQ socket and pushes data
| into the GUI. Returns False when there are no more queued data messages.
*/
GlgBoolean ProcessNewData( void * receiver_socket )
{
   int data_type;
   union
   {
      BaseData base;
      GPSData gps_data;
      TelemetryData telemetry_data;
   } data;

   data_type = ReceiveDataType( receiver_socket );

   switch( data_type )
   {
    case GPS:
    case TELEMETRY: 
      if( ReceiveDataStruct( receiver_socket, data_type, (BaseData*) &data ) )
        PushDataToGUI( (BaseData*) &data );

      /* Return True to keep processing received messages even if an erroneous
         message was skipped. In case of repeated errors, it will be
         interrupted on error in ReceiveDataType().
      */
      return True;

    case -1: return False;      /* No more data. */

    default: 
      GlgError( GLG_USER_ERROR, "Received invalid data type." );
      exit( GLG_EXIT_ERROR );
   }
}

/*----------------------------------------------------------------------
| Push data from the received data structures into the GUI.
| For each field in a data structure, push the data value into
| a tag with a matching TagSource and data type, if found. 
|
| !!!!! IMPORTANT: TagSource in the drawing MUST MATCH hardcoded 
| tag source strings below. For example, the tag sources in the drawing 
| must be "LAT" and "LON" to push lat and lon values from the
| GPS data structure. 
|
| If the application navigates through multiple drawings, not all
| data fields in the incoming data structure may have corresponding
| tags in the currently loaded drawing. 
|
| PushDTagData, PushGTagData and PushSTagData functions perform a 
| check to make sure a data value is pushed to the graphics only if 
| the tag with a matching TagSource exists in the currently loaded drawing.
*/
void PushDataToGUI( BaseData * data )
{
   GPSData * gps_data;
   TelemetryData * telem_data;

   switch( data->type )
   {
    case GPS:
      gps_data = (GPSData*) data; 

#if DEBUG_MESSAGES
      printf( "Got GPS data\n" );
#endif

      PushDTagData( "LAT", gps_data->lat );
      PushDTagData( "LON", gps_data->lon );
      PushDTagData( "SPEED", gps_data->speed );
      PushDTagData( "ALTITUDE", gps_data->altitude );
      PushDTagData( "PITCH", gps_data->pitch );
      PushDTagData( "ROLL", gps_data->roll );
      PushDTagData( "YAW", gps_data->yaw );
      PushGTagData( "POSITION", gps_data->lat, gps_data->lon, 
                    gps_data->altitude );
      break;

    case TELEMETRY: 
      telem_data = (TelemetryData *) data;

#if DEBUG_MESSAGES
      printf( "Got Telemetry data\n" );
#endif

      PushDTagData( "POWER", telem_data->power );
      PushDTagData( "VOLTAGE", telem_data->voltage );
      PushDTagData( "CURRENT", telem_data->current );
      PushDTagData( "TEMPERATURE", telem_data->temperature );
      PushDTagData( "PRESSURE", telem_data->pressure );
      PushDTagData( "STATE_HEALTH", telem_data->state_health );
      break;

    default:
      GlgError( GLG_USER_ERROR, "Unknown data structure type." );
      exit( GLG_EXIT_ERROR );
   }
}

/*--------------------------------------------------------------------
| Push a double value into a D-type tag, if found.
| Verify that the tag with a specified tag source is present in the drawing
| before pushing the value to the graphics.
*/
GlgBoolean PushDTagData( char * data_source, double d_value )
{
   TagRecord * tag_record;

   /* Find a tag record with a matching tag source and data type.
      If not found, return from function and don't push the value
      to the graphics.
   */
   tag_record = LookupTagRecords( data_source, GLG_D );
   
   if( !tag_record )
   { 
      if( DEBUG_TAGS )
        printf( "Tag %s not found.\n", data_source );
      return GlgFalse;
   }

   /* Push value into graphics. */
   GlgSetDTag( Viewport, tag_record->tag_source, d_value, 
               tag_record->if_changed );
      
   return GlgTrue;
}

/*----------------------------------------------------------------------
| Push x,y,z values into a G-type tag, if found. 
*/
GlgBoolean PushGTagData( char * data_source, 
                         double x, double y, double z )
{
   TagRecord * tag_record;

   /* Find a tag record with a matching tag source and data type. */
   tag_record = LookupTagRecords( data_source, GLG_G );
   
   if( !tag_record ) 
   {
      if( DEBUG_TAGS )
        printf( "Tag %s not found.\n", data_source );
      return GlgFalse;
   }

   /* Push value into graphics. */
   GlgSetGTag( Viewport, tag_record->tag_source, x, y, z, 
               tag_record->if_changed );
      
   return GlgTrue;
}

/*--------------------------------------------------------------------
| Push a string value into an S-type tag, if found.
*/
GlgBoolean PushSTagData( char * data_source, char * s_value )
{
   TagRecord * tag_record;

   /* Find a tag record with a matching tag source and data type. */
   tag_record = LookupTagRecords( data_source, GLG_S );
   
   if( !tag_record ) 
   {
      if( DEBUG_TAGS )
        printf( "Tag %s not found.\n", data_source );
      return GlgFalse;
   }

   /* Push value into graphics. */
   GlgSetSTag( Viewport, tag_record->tag_source, s_value, 
               tag_record->if_changed );
      
   return GlgTrue;
}

/*--------------------------------------------------------------------
| Returns a tag record with a matching tag_source and data_type.
*/
TagRecord * LookupTagRecords( char * tag_source, GlgLong data_type )
{  
   int i;
   TagRecord * tag_record;
   
   for( i = 0; i < NumTagRecords; ++i )
   { 
      tag_record = &TagRecordArray[ i ];

      if( strcmp( tag_record->tag_source, tag_source ) == 0 &&
        tag_record->data_type == data_type )
        return tag_record;
   }

   return NULL; /* not found */
}

/*----------------------------------------------------------------------
| A data thread that receives data and sends then to GUI via a ZeroMQ
| socket.
*/
#ifndef _WINDOWS
void * DataThreadFunc( void * arg ) 
#else
DWORD WINAPI DataThreadFunc( LPVOID arg )
#endif
{
   void * zmq_sender;

   /* Creates a ZMQ socket used by the data thread to send data to the 
      GUI thread.
   */
   zmq_sender = CreateZMQSender( ZMQContext );

   while( !StopDataThread )
   {
      /* Get data from an application data source and send them to GUI via
         the provided sender socket.
      */
      if( RANDOM_DATA )
        ProcessDemoData( zmq_sender );
      else
        ProcessRealData( zmq_sender );
   }

   /* Close the sending socket. */
   zmq_close( zmq_sender );

#ifndef _WINDOWS
   return NULL;
#else
   return 0;
#endif
}

/*----------------------------------------------------------------------
| Read the first frame of a message containing the data type.
| Returns a received data type, or -1 if there are no data in the 
| socket's queue.
*/
int ReceiveDataType( void * receiver_socket )
{
   int rval;
   DataStructType data_type;
   
   /* Receive the first frame of the massage containing data type.
      Using non-blocking mode to return right away if there are no more 
      messages.
      There is no need for the network byte-order conversion, since
      we are sending data between two threads on the same machine.
   */
   rval = 
     zmq_recv( receiver_socket, &data_type, sizeof( data_type ), ZMQ_DONTWAIT );
   switch( rval )
   {
    case -1:   /* Error or no more messages. */
      if( zmq_errno() == EAGAIN )
        return -1;   /* No more messages. */ 

      GlgError( GLG_USER_ERROR, "Error receiving data type." );
      exit( GLG_EXIT_ERROR );

    case sizeof( data_type ):  /* A valid data type message: return content. */
      return data_type;

    default:
      GlgError( GLG_USER_ERROR,
                "Error receiving data type: invalid message size" );
      exit( GLG_EXIT_ERROR );
   }
}

/*----------------------------------------------------------------------
| Read the second frame of a message containing the data structure.
| Returns True if the structure was successfully read, or False otherwise.
*/
GlgBoolean ReceiveDataStruct( void * receiver_socket, DataStructType type, 
                              BaseData * data )
{
   int 
     more,
     rval,
     data_length;
   size_t more_size = sizeof( more );
   
   /* Determine if more message frames are to follow. */
   rval = zmq_getsockopt( receiver_socket, ZMQ_RCVMORE, &more, &more_size );
   if( rval != 0 )
   {
      GlgError( GLG_USER_ERROR, "Error querying message for more frames." ); 
      exit( GLG_EXIT_ERROR );
   }
   else if( !more )
   {
      GlgError( GLG_USER_ERROR, "Invalid message: missing data frame." ); 
      return False;
   }
   
   switch( type )
   {
    case GPS:       data_length = sizeof( GPSData ); break;
    case TELEMETRY: data_length = sizeof( TelemetryData ); break;
    default: 
      GlgError( GLG_USER_ERROR, "Invalid data type." ); 
      exit( GLG_EXIT_ERROR );
   }

   /* Receive the second frame of the massage containing data type.
      Using non-blocking mode to return right away if there are no more 
      messages.
   */
   rval = zmq_recv( receiver_socket, data, data_length, ZMQ_DONTWAIT );
   if( rval == -1 )
   {
      GlgError( GLG_USER_ERROR, "Error receiving message data frame." );
      return False;
   }
   else if( rval != data_length )
   {
      GlgError( GLG_USER_ERROR, "Invalid data length of received data." );
      return False;
   }
   else if( data->type != type )
   {
      GlgError( GLG_USER_ERROR, "Message data type mismatch." );
      return False;
   }

   return True;
}

/*--------------------------------------------------------------------
| Send data from the data structure to the GUI thread via the provided
| ZeroMQ socket. The socket handles asynchronous queueing of messages.
*/
void SendDataToGUIThread( void * sender_socket, BaseData * data )
{
   int
     type_size,
     struct_size;

   switch( data->type )
   {
    default:
      fprintf( stderr, "Invalid Data Type, skipping data record.\n" );
      exit( GLG_EXIT_ERROR );

    case GPS:
      struct_size = sizeof( GPSData );
      break;
      
    case TELEMETRY: 
      struct_size = sizeof( TelemetryData );
      break;
   }
   type_size = sizeof( data->type );

   /* Send the data structure type as the first frame of the message.
      There is no need for the network byte-order conversion, since
      we are sending data between two threads on the same machine.
   */
   if( zmq_send( sender_socket, &data->type, type_size, ZMQ_SNDMORE ) 
       != type_size )
   {
      fprintf( stderr, "Sending data type failed.\n" );
      exit( GLG_EXIT_ERROR );
   }

   /* Send the data structure itself as the second frame of the message. */
   if( zmq_send( sender_socket, data, struct_size, 0 ) != struct_size )
   {
      fprintf( stderr, "Sending data structure failed.\n" );
      exit( GLG_EXIT_ERROR );
   }
}

/* Use either inproc or TCP over a local host. */
#define USE_INPROC_TRANSPORT    1

#if USE_INPROC_TRANSPORT
   /* inproc */
#  define SOCKET_END_POINT   "inproc://data_receiver"
#  define SOCKET_TYPE        ZMQ_PAIR
#else
   /* TCP over a local host */
#  define SOCKET_END_POINT   "tcp://127.0.0.1:55555"
#  define SOCKET_TYPE        ZMQ_DEALER
#endif

/*----------------------------------------------------------------------
| Creates a ZMQ socket used by the GUI thread to to receive data from
| the data thread.
*/
void * CreateZMQReceiver( void * zmq_context )
{  
   void * zmq_receiver;

   /* Create a data receiving socket. */
   zmq_receiver = zmq_socket( zmq_context, SOCKET_TYPE );
   if( !zmq_receiver )
   {
      GlgError( GLG_USER_ERROR, "Can't create receiving socket.\n" );
      exit( GLG_EXIT_ERROR );
   }

   /* Bind the data receiving socket to the "inproc://data_receiver" end point
      that will use inter-thread transport.
   */
   if( zmq_bind( zmq_receiver, SOCKET_END_POINT ) != 0 )
   {
      GlgError( GLG_USER_ERROR, "Can't bind the data receiver.\n" );
      exit( GLG_EXIT_ERROR );
   }

   return zmq_receiver;
}

/*----------------------------------------------------------------------
| Creates a ZMQ socket used by the data thread to send data to the 
| GUI thread.
*/
void * CreateZMQSender( void * zmq_context )
{
   int num_tries = 0;
   void * zmq_sender;

   /* Create a data sending socket. */
   zmq_sender = zmq_socket( zmq_context, SOCKET_TYPE );
   if( !zmq_sender )
   {
      GlgError( GLG_USER_ERROR, "Can't create sending socket.\n" );
      exit( GLG_EXIT_ERROR );
   }

   /* Connect the data sending socket to the "inproc://data_receiver" end point
      in the main thread using inter-thread transport.
      Try a few times to avoid timing issues for inproc sockets.
   */
   while( 1 )
   {
      ++num_tries;
      if( zmq_connect( zmq_sender, SOCKET_END_POINT ) == 0 )
        break;

      if( num_tries == 3 )
      {
         GlgError( GLG_USER_ERROR, "Can't connect to the data receiver.\n" );
         GlgError( GLG_USER_ERROR, (char*) zmq_strerror( zmq_errno() ) );
         exit( GLG_EXIT_ERROR );
      }
      SleepMS( 30 );
   }

#if DEBUG_CONNECT   
   printf( "Connected sender socket after %d tries.\n", num_tries );
#endif   

   return zmq_sender;
}

/*----------------------------------------------------------------------
| Return exact time including fractions of seconds.
*/
double GetCurrTime()
{
   GlgULong sec, microsec;
   
   if( !GlgGetTime( &sec, &microsec ) )
     return 0.;
     
   return sec + microsec / 1000000.;
}

/*----------------------------------------------------------------------
| 
*/
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, GlgLong interval )
{
   GlgULong sec2, microsec2;
   GlgLong elapsed_time, adj_interval;

   GlgGetTime( &sec2, &microsec2 );  /* End time */
   
   /* Elapsed time in millisec */
   elapsed_time = 
     ( sec2 - sec1 ) * 1000 + (long) ( microsec2 - microsec1 ) / 1000;

   /* Maintain constant update interval regardless of the system speed. */
   if( elapsed_time + 20 >= interval )
      /* Slow system: update as fast as we can, but allow a small interval 
         for handling input events. */
     adj_interval = 20;
   else
     /* Fast system: keep constant update interval. */
     adj_interval = interval - elapsed_time;

#if DEBUG_TIMER
   printf( "sec= %ld, msec= %ld\n", 
           (long)( sec2 - sec1 ), (long)( microsec2 - microsec1 ) );
   printf( "*** elapsed= %ld, requested= %ld, adjusted= %ld\n",
           (long) elapsed_time, (long) interval, (long) adj_interval );
#endif

   return adj_interval;
}

/*----------------------------------------------------------------------
| 
*/
void SleepMS( GlgLong millisec )
{
#ifndef _WINDOWS
      usleep( millisec * 1000 );
#else
      Sleep( (DWORD) millisec );
#endif
}
