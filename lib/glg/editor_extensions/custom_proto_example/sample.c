/*---------------------------------------------------------------------
| This code shows an example of a custom prototyping module.
| The module may be used by either the Graphics Builder or HMI 
| Configurator to animate the drawing in the Run mode.
|
| The module can connect to custom datasources, such as PLC or 
| process database, and animate the drawing with real data inside
| the Builder. The module can use all functions of the GLG Standard
| and Extended APIs to implement a complete application program,
| including handling of mouse events, custom dialogs, object selection 
| and other user input.
| 
| When the module is used as a complete application program, the
| Builder or HMI Configurator can be started in the Run mode to run the
| application program. Stopping the Run mode will bring the Builder
| for editing the drawing; when editing is finished, the Run mode can
| be restarted with the modified drawing. If required, the Run mode may 
| be started in a separate window to hide the GLG editor's menus and 
| toolbars.
|
| The code of this prototyping module may be shared between a stand-alone 
| run-time application that does not depend on the Builder, and the
| custom proto module used inside teh Builder or HMI Configurator.
*/ 

#include "glg_custom_dll.h"

/* Contains prototypes for the required entry points of the custom
   proto module.
   */
#include "glg_custom_proto.h"

/* Set to 1 to update tags directly using stored object IDs.
   Set to 0 to update tags using their TagSource names, which is more 
   convenient when updating using data change events that contain the
   data ID (TagSource), data type and data value.
   */
#define UPDATE_TAG_USING_RESOURCES   0

/* Prototypes for the utility functions */
void UpdateTags( void );
void UpdateResources( void );
void UpdateDialog( void );
void UpdateTagValue( GlgObject data_obj );
void UpdateDTagValue( GlgObject data_obj, char * tag_source );
void UpdateSTagValue( GlgObject data_obj, char * tag_source );
void UpdateGTagValue( GlgObject data_obj, char * tag_source );
void SaveInitValues( GlgBoolean save_init_values );
void RestoreInitValues( GlgBoolean restore_init_values );
void PopupDialog( void );
void EraseDialog( void );
void SetDialogSize( GlgObject dialog, GlgLong width, GlgLong height, 
                   GlgLong x, GlgLong y );
void CustomDialogEventHandler( GlgObject message );

/* Globals */
static char * LoadPath = NULL;
static GlgBoolean HMIMode = 0;
static GlgObject Drawing = (GlgObject)0;
static GlgObject TagList = (GlgObject)0;
static GlgObject SavedList = (GlgObject)0;
static int Counter = 0;
static GlgBoolean StopRequested = False;
static GlgObject CustomDialog = (GlgObject)0;

/*---------------------------------------------------------------------
| Required entry point:
|
| Returns the custom DLL's version number. The version parameter 
| provides the latest DLL version supported by the editor. The return 
| value should not be changed unless the DLL is aware of the version
| differences.
*/
glg_export GlgLong GlgCustomProtoVersionNumber( GlgLong version )
{
   /* Current version number defined in include files. */
   return GLG_CUSTOM_PROTO_VERSION_NUMBER;
}

/*---------------------------------------------------------------------
| Required entry point:
|
| Performs data connection initialization on startup if any required.
|
| The init_data parameter contains a pointer to a structure with the 
| following fields: 
|
| load_path - supplies the load path of the shared library or DLL
| if provided by the -proto-lib command-line option, or NULL otherwise.
| If not NULL, it always contains a trailing slash or back slash.
|
| hmi_mode - is set to True when used with the HMI Configurator;
| it is set to False in the Graphics Builder.
|
| can_restore_init_values - is used to let the caller know if the editor 
| should save/restore the drawing before and after the Run mode if the 
| Restore Values On Stop editor option is ON. If this module will save 
| and restore initial values and there is no need to save/restore the 
| drawing, set it to False, otherwise set it to True.
|
| Return a non-zero code on failure.
*/
glg_export GlgLong GlgCustomProtoInit( GlgCustomProtoInitData * init_data )
{
   LoadPath = GlgStrClone( init_data->load_path );

   HMIMode = init_data->hmi_mode;    /* Store mode: Builder or HMI */

   /* Set save_drawing = False to indicate that this module will save and 
      restore initial values and there is no need to save/restore the drawing
      before and after the Run mode.
      */
   init_data->save_drawing = False;

   /* Disable datagen-based Run Mode animation to animate only by the 
      custom DLL. Set to True to allow the user to use the datagen command 
      for animating the drawing in addition to the custom DLL animation 
      (if any).
      */
   init_data->enable_datagen = False;
 
   /* No initilization required for this simple example. 
      Return 0 to indicate success.
      */
   return 0;
}

/*---------------------------------------------------------------------
| Required entry point:
|
| Speed change callback, allows the DLL to accept or reject the speed
| requested by the user.
| The min_speed and max_speed parameters provide the min and max speed limits
| in relative units (currently 0 and 9). The speed parameter is the speed
| requested by the user. To reject the requested speed, return a different
| valid spped value.
*/
glg_export GlgLong GlgCustomProtoSetSpeed( GlgLong min_speed, 
                                          GlgLong max_speed, GlgLong speed )
{
   return speed;    /* Accept the speed requested by the user. */
}

/*---------------------------------------------------------------------
| Required entry point:
|
| Performs any drawing-specific initialization when the Run mode is 
| started, such as querying a list of tags defined in the drawing.
|
| The save_init_values parameter is set to True when the Builder's
| Restore Values on Stop option is active.
*/
glg_export void GlgCustomProtoStartRun( GlgObject drawing, 
                                       GlgLong save_init_values )
{
   Drawing = drawing;    /* Store the drawing in a global variable. */

   /* On start-up, query the list of tags defined in the drawing.
      The tags will be animated with data in GlgCustomProtoUpdateRun() 
      method.
      */
   TagList = GlgCreateTagList( drawing, True );

   /* Saves tags' initial values if requested by the save_init_values flag.
      If it is impossible for the module to save and restore initial values,
      it may request the Builder to save and restore the whole drawing by
      returning save_drawing=True in the GlgCustomProtoInit() method.
    */
   SaveInitValues( save_init_values );

   /* Reset the counter used to generate some data in this example. */
   Counter = 0;

   /* Shows example of loading a drawing at run-time and handling its events.
      A popup may also be activated based on the user interaction.
      */
   PopupDialog();

   StopRequested = False;   /* Reset the stop flag */
}

/*---------------------------------------------------------------------
| Required entry point:
| Updates the drawing, returns True to stop the Run mode or False to
| continue. The pause parameter is set to True when the Run mode is paused.
*/
glg_export GlgBoolean GlgCustomProtoUpdateRun( GlgObject drawing, 
                                              GlgBoolean pause )
{
   if( !pause )
   {
      /* Update counter used to generate some data in this example. */
      ++Counter;

      /* An example of animation using tags: updates all tags defined in the 
         drawing with new data values.
         */
      UpdateTags();

      /* Provides an example of animating the drawing using resources. */
      UpdateResources();
      
      /* Provides an example of animating resources in the custom run-time
         dialogs. */
      UpdateDialog();

      /* Redraw the changes, may be done every n-th iteration if required. */
      GlgUpdate( drawing );
   }

   if( StopRequested )
     return True;     /* Return True to stop Run mode */
   else
     return False;    /* Return False to continue */
}

/*---------------------------------------------------------------------
| Required entry point:
|
| Handles selection and user iteraction in the Run mode to provide
| custom application logic inside the Builder.
*/
glg_export void 
  GlgCustomProtoEventHandler( GlgObject drawing, GlgObject top_viewport, 
                             GlgObject message, GlgObject viewport )
{
   char
     * format,
     * origin,
     * full_origin,
     * action,
     * event_label;

   if( CustomDialog && top_viewport == CustomDialog )
   {
      /* Custom dialog events may be handles separately if required. */
      CustomDialogEventHandler( message ); 
      return;
   }

   GlgGetSResource( message, "Format", &format );
   GlgGetSResource( message, "Action", &action );
   GlgGetSResource( message, "Origin", &origin );
   GlgGetSResource( message, "FullOrigin", &full_origin );

#if 1
   /* Enable to debug messages (Linux/Unix only, does nothing on Windows) */ 
   printf( "Proto event: format = %s, action= %s, origin= %s, full_origin= %s \n", 
          format, action, origin, full_origin );
#endif
   
   if( strcmp( format, "Button" ) == 0 )
   {
      if( strcmp( action, "Activate" ) == 0 )
      {
         if( origin && strcmp( origin, "StopProto" ) == 0 )
           /* Stop if a button named StopProto is activated. */
           StopRequested = True;
      }
   }
   /* Set viewport's ProcessMouse attribute to activate custom events. */
   else if( strcmp( format, "CustomEvent" ) == 0 )
   {
      if( strcmp( action, "MouseClick" ) == 0 )
      {
         GlgGetSResource( message, "EventLabel", &event_label );

         /* printf does not do anything on Windows - just a sample. */
         printf( "Custom event, label= %s\n", 
                event_label ? event_label : "NULL" ); 
      }
   }
}

/*---------------------------------------------------------------------
| Required entry point:
|
| May be used to process native windowing events.
*/
glg_export void GlgCustomProtoTraceCallback( GlgTraceCBStruct * trace_data )
{
}

/*---------------------------------------------------------------------
| Required entry point:
|
| Performs cleanup when the Run mode is finised, such as deleting a 
| list of tags created by the GlgCustomProtoStartRun() method.
|
| The restore_init_values parameter is set to True when the Builder's
| Restore Values on Stop option is active and always matches the value 
| of the save_init_values paremeter of the GlgCustomProtoStartRun() 
| method.
*/
glg_export void GlgCustomProtoStopRun( GlgObject drawing, 
                                      GlgLong restore_init_values )
{
   /* Restore initial values if requested.  */
   RestoreInitValues( restore_init_values );

   /* Destroy stored data. */

   GlgDropObject( TagList );
   SavedList = (GlgObject)0;

   GlgDropObject( SavedList );
   SavedList = (GlgObject)0;
   
   EraseDialog();
}

/*---------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| An example of animation using tags: updates all tags defined in the
| drawing with new data values.
|
| This method does not need to know anything about the drawing; all it 
| needs to know is the list of tags defined in the drawing, which was
| obtained and stored by the GlgCustomProtoStartRun() method.
|
| This method updates tags with random data. In a real application,
| new values will be received from real data source, such as a PLC or 
| a process data base. The tag's TagSource attribute is used as a tag
| id when requesting the new tag value.
|
| Instead of traversing the list of tags and querying a new value for 
| each tag on each update, the program's GlgCustomProtoStartRun() method 
| can subscribe to data updates for all tags defined in the drawing,
| and process data change events in the GlgCustomProtoUpdateRun() method.
| A sample of using this technique is shown in the GlgTagsExampleEventG.c 
| example located in the GLG's examples_c_cpp/misc/TagsExample directory.
*/ 
void UpdateTags()
{
   GlgLong i, size;
   GlgObject data_obj;

   if( !TagList )
     return;    /* No tags were defined in the drawing. */

   /* This is an example of a poll-based data update. Refer to the
      GlgTagsExampleEventG.c example for an alternative event-based
      update method (in the GLG's examples_c_cpp/misc/TagsExample directory).
      */

   size = GlgGetSize( TagList );
   for( i=0; i<size; ++i )
   {
      data_obj = GlgGetElement( TagList, i );
      UpdateTagValue( data_obj );
   }
}

/*---------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| An example of tag value animation with random data. A real data 
| source such as a PLC or process database can be used in a real 
| application.
*/
void UpdateTagValue( GlgObject data_obj )
{
   double d_type;
   char * tag_source;

   GlgGetDResource( data_obj, "DataType", &d_type );
   GlgGetSResource( data_obj, "TagSource", &tag_source );

   switch( (int) d_type )
   {
    case GLG_D: UpdateDTagValue( data_obj, tag_source ); break;
    case GLG_S: UpdateSTagValue( data_obj, tag_source ); break;
    case GLG_G: UpdateGTagValue( data_obj, tag_source ); break;
   }
}

/*---------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Update double value. 
*/
void UpdateDTagValue( GlgObject data_obj, char * tag_source )
{
   double value;

   /* Get random value. In a real application, query the value from the
      real data source using TagSource as an ID.
      */
   value = GlgRand( 0., 1. );

#if UPDATE_TAG_USING_RESOURCES
   GlgSetDResource( data_obj, NULL, value );
#else
   GlgSetDTag( Drawing, tag_source, value, True );
#endif
}

/*---------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Update string value. 
*/
void UpdateSTagValue( GlgObject data_obj, char * tag_source )
{
   char * string;

   /* Generate a string showing the value of the counter. 
      In a real application, query the value from a real data source 
      using TagSource as an ID.
      */
   string = GlgCreateIndexedName( "Iteration = ", Counter );

#if UPDATE_TAG_USING_RESOURCES
   GlgSetSResource( data_obj, NULL, string );
#else
   GlgSetSTag( Drawing, tag_source, string, True );
#endif

   GlgFree( string );
}

/*---------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Update a triplet of double values (RGB color or XYZ point).  
*/
void UpdateGTagValue( GlgObject data_obj, char * tag_source )
{
   double r_value, g_value, b_value;

   /* Assumint it's a color tag, get random values in the range [0;1].
      For XYZ points, the range would be [-1000;1000]
      In a real application, query the value from a real data source 
      using TagSource as an ID. 
      */
   r_value = GlgRand( 0., 1. );
   g_value = GlgRand( 0., 1. );
   b_value = GlgRand( 0., 1. );

#if UPDATE_TAG_USING_RESOURCES
   GlgSetGResource( data_obj, NULL, r_value, g_value, b_value );
#else
   GlgSetGTag( Drawing, tag_source, r_value, g_value, b_value, True );
#endif
}

/*---------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| An example of resource-based animation.
| This method needs to know what resources are defined in the drawing.
| It may check for presence of resources using GlgHasResource() method.
| A custom property (such as DrawingType, etc.) may be defined in the 
| drawing to provide additional information for this method about the 
| content of the drawing.
*/
void UpdateResources()
{
   GlgObject widget;

   if( HMIMode )
     widget = Drawing;    /* $Widget viewport is hidden in HMI */
   else
   {
      widget = GlgGetResourceObject( Drawing, "$Widget" );
      if( !widget )
        widget = Drawing;  /* Use drawing as widget if $Widget is not found. */
   }

   /* Update the IterationCounter resource if it is defined in the drawing. */
   if( GlgHasResourceObject( widget, "IterationCounter" ) )
     GlgSetDResource( widget, "IterationCounter", (double) Counter );
}

/*---------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| An example of animating objects in the custom run-time dialogs via
| resources.
| Since the content and resource hierarchy of the custom dialog are known, 
| there is no need to check if resources exist.
*/
void UpdateDialog()
{
   /* Update the IterationCounter resource of the custom dialog. */
   GlgSetDResource( CustomDialog, "IterationCounter", (double) Counter );
}

/*---------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Saves tags' initial values if requested by the save_init_values flag.
*/
void SaveInitValues( GlgBoolean save_init_values )
{
   /* Copy the tag list to save the initial values if requested by the
      save_init_values flag. The tag list contains data objects the
      tags are attached to; saving the list saves their values on
      start-up.  GlgCopyObject performs a full clone to avoid
      intefering via constraints.
      */   
   if( save_init_values )
     SavedList = GlgCopyObject( TagList );
   else
     SavedList = (GlgObject)0;
}

/*---------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Restores tags' initial values if requested by the restore_init_values 
| flag.
*/
void RestoreInitValues( GlgBoolean restore_init_values )
{
   GlgLong i, size;
   GlgObject data_obj, saved_data_obj;

   if( restore_init_values && TagList )
   {
      size = GlgGetSize( TagList );
      for( i=0; i<size; ++i )
      {
         data_obj = GlgGetElement( TagList, i );
         saved_data_obj = GlgGetElement( SavedList, i );

         /* The elements of the tag list contain data object the tags
            are attached to. Restore their values from the saved copy
            of the list that stores their values at the start-up time.
            */
         GlgSetResourceFromObject( data_obj, NULL, saved_data_obj );
      }
   }
}

/*---------------------------------------------------------------------
| Utility function for the sample implementation only. 
*/
void PopupDialog()
{
   char * filename;
   
   /* Load using a path relative to the DLL's load path. */
   filename = GlgConcatStrings( LoadPath, "dialog.g" );
   CustomDialog = GlgLoadWidgetFromFile( filename );
   GlgFree( filename );

   if( !CustomDialog )
   {
      GlgError( GLG_USER_ERROR, "Can't load custom dialog." );
      return;
   }

   GlgSetSResource( CustomDialog, "Name", "CustomDialog" );
   GlgSetDResource( CustomDialog, "ShellType", (double) GLG_DIALOG_SHELL );
   GlgSetSResource( CustomDialog, "ScreenName", "OEM Run Dialog" );

   SetDialogSize( CustomDialog, 250, 100, 500, 400 );

   /* Makes sure the dialog events are reported with the dialog as the 
      top_viewport parameter, instead of the drawing as the top_viewport 
      by default. It can make it easier to handle events from custom dialogs, 
      but it's not mandatory.
      */
   GlgActivateCustomDialogEvents( CustomDialog );

   GlgAddObjectToBottom( Drawing, CustomDialog );
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Sets an absolute dialog size in pixels instead of a size relative to
| the parent drawing.
*/
void SetDialogSize( GlgObject dialog, GlgLong width, GlgLong height, 
                   GlgLong x, GlgLong y )
{
   /* Disable positioning by the control points be setting both to (0,0,0). */
   GlgSetGResource( dialog, "Point1", 0., 0., 0. );
   GlgSetGResource( dialog, "Point2", 0., 0., 0. );

   /* Set size. */
   GlgSetDResource( dialog, "Screen/WidthHint", (double) width );
   GlgSetDResource( dialog, "Screen/HeightHint", (double) height );

   /* Set position. */
   GlgSetDResource( dialog, "Screen/XHint", (double) x );
   GlgSetDResource( dialog, "Screen/YHint", (double) y );
}

/*---------------------------------------------------------------------
| Utility function for the sample implementation only. 
*/
void EraseDialog()
{
   if( !CustomDialog )
     return;   

   GlgDeleteThisObject( Drawing, CustomDialog );
   GlgDropObject( CustomDialog );
   CustomDialog = (GlgObject)0;
}

/*---------------------------------------------------------------------
| Utility function for the sample implementation only. 
*/
void CustomDialogEventHandler( GlgObject message )
{
   char
     * format,
     * origin,
     * full_origin,
     * action;

   GlgGetSResource( message, "Format", &format );
   GlgGetSResource( message, "Action", &action );
   GlgGetSResource( message, "Origin", &origin );
   GlgGetSResource( message, "FullOrigin", &full_origin );

#if 1
   /* Enable to debug messages (Linux/Unix only, does nothing on Windows) */ 
   printf( "CustomDialog event : format = %s, action= %s, origin= %s, full_origin= %s \n", 
          format, action, origin, full_origin );
#endif
   
   /* Handle window closing. */
   if( strcmp( format, "Window" ) == 0 )
   {
      if( strcmp( action, "DeleteWindow" ) == 0 )
        if( strcmp( origin, "CustomDialog" ) == 0 )
        {
           /* Close the dialog. */
           GlgSetDResource( CustomDialog, "Visibility", 0. );
           GlgUpdate( CustomDialog );
        }
      return;
   }

   if( strcmp( format, "Button" ) == 0 )
   {
      if( strcmp( action, "Activate" ) == 0 )
      {
         if( origin && strcmp( origin, "StopButton" ) == 0 )
           /* Stop Run mode and close dialog. */
           StopRequested = True;
         else if( origin && strcmp( origin, "CloseButton" ) == 0 )
         {
            /* Close dialog without stopping the Run mode */
            GlgSetDResource( CustomDialog, "Visibility", 0. );
            GlgUpdate( CustomDialog );
         }
      }
   }
}

