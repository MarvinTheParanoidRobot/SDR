#include <stdio.h>
#include <stdlib.h>

#include "GlgApi.h"

/* Set to 1 to print debugging information */
#define DEBUG 0

#define SELECTION_RESOLUTION  5    /* Selection sensitivity in pixels */

/* Function prototypes */
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void ResetSelectedObject( void );
void ProcessCommand( GlgObject viewport, GlgObject selected_obj, 
                     GlgObject command_obj, char * event_label );
void ProcessSelectionEvent( GlgObject viewport, GlgObject selected_obj, 
                            char * event_label );
GlgBoolean ProcessOnRelease( GlgObject selected_obj, GlgObject action_obj );
void UpdateComment( GlgObject viewport, GlgObject selected_obj, char * str ); 
GlgObject LoadDrawing( char * drawing_file );
GlgObject GetDirectChild( GlgObject viewport, GlgObject object );
void ProcessCommandGoTo( GlgObject command_obj, GlgLong * timer_id );
void PositionViewport( GlgObject viewport, double x, double y, 
                       double width, double height );

/* Currently loaded drawing. */
GlgObject Viewport = (GlgObject)0;

/* Store information for the object selected with the mouse, used to
   process selection events on MouseRelease instead of MouseClick.  
*/
GlgObject SelectedObject = (GlgObject) 0;
char * SelectedEventLabel = NULL;

GlgAppContext AppContext;

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*----------------------------------------------------------------------
|  Main entry point.
*/
int GlgMain( int argc, char *argv[], GlgAppContext InitAppContext )
{
   AppContext = GlgInit( False, InitAppContext, argc, argv );

   /* Load the drawing, add callbacks. */
   Viewport = LoadDrawing( "obj_selection.g" );
   
   if( !Viewport )
   {
      GlgError( GLG_USER_ERROR, "Can't load drawing.\n" );
      exit( 0 );
   }

   /* Position the loaded viewport on the screen. */
   PositionViewport( Viewport, 50., 50., 700., 600. );

   /* Display the drawing. */
   GlgInitialDraw( Viewport );

   /* Event loop */
   return (int) GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| Input callback is used to handle object selection events in this
| example, and invoked when the user selects a GLG object with the mouse.
*/
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{
   GlgObject 
     message_obj,
     selection_array,
     selected_obj,
     action_obj,
     command_obj;
   
   char
     * format,
     * action,
     * event_label,
     * obj_name;

   /* Flag indicating whether to process object selection on MouseRelease
      or MouseClick. In this example, the flag is set based
      on the resource "OnRelease" defined in the object at design time.
   */
   static GlgBoolean process_on_release = GlgFalse;

   message_obj = (GlgObject) call_data;

   /* Get the format, action and origin resources from the message object. */
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );

   /* Handle window closing. May use viewport's name. */
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );

   /* Process object selection via Actions attached to an object in
      the GLG Builder (at design time). Actions may be of type Command
      (ActionType=SEND COMMAND) or CustomEvent (ActioType=SEND EVENT).
   */

   if( strcmp( format, "Command" ) == 0 || strcmp( format, "CustomEvent" ) == 0 )
   {
      /* Retrieve ActionObject */
      action_obj = GlgGetResourceObject( message_obj, "ActionObject" ); 

      /* Retrieve selected object. */
      selected_obj = GlgGetResourceObject( message_obj, "Object" );

      /* Extract EventLabel */
      GlgGetSResource( message_obj, "EventLabel", &event_label );

      if( DEBUG ) /* print selected_obj and event_label */
        printf( "selected_obj = %lx, event_label = %s\n", 
                 (long) selected_obj, event_label );
         
      /* Process Actions with ActionType = SEND COMMAND */      
      if( strcmp( format, "Command" ) == 0 )
      {
         /* Retrieve Command object. */
         command_obj = GlgGetResourceObject( action_obj, "Command" );
         if( !command_obj )
           return;
         
         /* Process command */
         ProcessCommand( viewport, selected_obj, command_obj, event_label );

         /* Update comment string in the drawing. */
         UpdateComment( viewport, selected_obj, 
                        "Processing Command\nObject Name: " );

         /* Refresh the display. */
         GlgUpdate( viewport );
      }

      /* Process Actions with ActionType = SEND EVENT */ 
      else if( strcmp( format, "CustomEvent" ) == 0 )
      {
         if( strcmp( action, "MouseClick" ) == 0 )
         {
            /* Determine if the event should be processed on MouseRelease
               or MouseClick.
            */
            process_on_release = ProcessOnRelease( selected_obj, action_obj );
            if( !process_on_release )
            {
               /* Process selection on MouseClick */
               ProcessSelectionEvent( viewport, selected_obj, event_label );  

               /* Update comment string in the drawing. */
               UpdateComment( viewport, selected_obj, 
                   "Processing Custom Event on MouseClick\nObject Name: " );
            }

            /* Process selection event on MouseRelease. */
            else 
            {
               /* On mouse click, store selected object and EventLabel. 
                  The event will be processed on MouseRelease, using the
                  stored object and event label.
               */
               SelectedObject = selected_obj;
               SelectedEventLabel = GlgStrClone( event_label );
               
               if( DEBUG )
               {
                  printf( "Process selection on MouseRelease\n" );
                  printf( "Stored SelectedObject = %lx SelectedEventLabel = %s\n", 
                       (long) SelectedObject, SelectedEventLabel );
               }
            }
         }
         else if( strcmp( action, "MouseOver" ) == 0 && process_on_release )
         {
            /* Mouse may be moved outside of the object, or moved to 
               another (intersecting) object, in which case reset SelectedObject
               and do not process the event.
            */
            if( SelectedObject && SelectedObject != selected_obj )
            {
               ResetSelectedObject();
               process_on_release = GlgFalse;

               /* Update comment string in the drawing. */
               UpdateComment( viewport, SelectedObject, 
                 "Custom Event not processed,\nmouse is moved away from object " );
               return;
            }
         }
         else if( strcmp( action, "MouseRelease" ) == 0 && process_on_release )
         {
            if( !SelectedObject )  /* nothing to do */
              return;
            
            if( DEBUG )
              printf( "MouseRelease, SelectedObject = %lx\n", 
                      (long) SelectedObject );
            /* Ready to execute selection event: MouseRelease occurred 
               inside the object.
            */
            ProcessSelectionEvent( viewport, SelectedObject, SelectedEventLabel ); 
            
            /* Update comment string in the drawing. */
            UpdateComment( viewport, SelectedObject, 
                    "Processing Custom Event on MouseRelease\nObject Name: " ); 
            
            /* Reset SelectedObject and SelectedEventLabel. */
            ResetSelectedObject();
            process_on_release = GlgFalse;
         }

         GlgUpdate( viewport );    /* Make changes visible. */
      } /* format=CustomEvent */
   }
   else if( strcmp( format, "Tooltip" ) == 0 && 
            strcmp( action, "ObjectTooltip" ) == 0 )
   {
      /* Extract TooltipString */
      char * tooltip_str;
      GlgObject action_obj;
      char * comment;

      /* Tooltip string may be obtained either as EventLabel from the
         message_obj, or as "Tooltip" resource from the ActionObject
         for tooltips added in GLG v.3.5 and later.
      */

      GlgGetSResource( message_obj, "EventLabel", &tooltip_str );
      if( DEBUG )
        printf( "Tooltip String from EventLabel = %s\n", tooltip_str );
      
      action_obj = GlgGetResourceObject( message_obj, "ActionObject" );
      if( action_obj )
      {
         GlgGetSResource( action_obj, "Tooltip", &tooltip_str );
         if( DEBUG )
           printf( "Tooltip String from ActionObject = %s\n", tooltip_str );
      }

      /* Retrieve selected object. */
      selected_obj = GlgGetResourceObject( message_obj, "Object" );

      if( selected_obj )
        comment = "Tooltip displayed for object ";
      else
        comment = "";

      /* Update the comment string in the drawing. */
      UpdateComment( viewport, selected_obj, comment );
      GlgUpdate( viewport );
   }
   
   /* Handle object selection on MouseClick for top level
      objects without Actions. 
   */
   else if( strcmp( format, "ObjectSelection" ) == 0 && 
            strcmp( action, "MouseClick" ) == 0 )
   {
      GlgObject widget;
      char * obj_name;
      double button_index;
      int i;

      /* Retrieve mouse button index and process selection as needed. */
      GlgGetDResource( message_obj, "ButtonIndex", &button_index );
      
      switch( (int) button_index )
      {
       case 1:
       case 3:
         /* Retrieve SelectionArray resource containing an array of object
            IDs potentially selected with the mouse */
         selection_array = 
           GlgGetResourceObject( message_obj, "SelectionArray" );
         
         if( !selection_array )
           return;

         /* Traverse an array of selected objects and print their 
            names (for the demonstration purposes). */
         for( i=0; i < GlgGetSize( selection_array ); ++i )
         {
            selected_obj = (GlgObject) GlgGetElement( selection_array, i );
            GlgGetSResource( selected_obj, "Name", &obj_name );

            if( DEBUG )
              /* Print selected object name */
              printf( "ObjectSelection event; selected object name: %s\n",
                      obj_name );
         }

         /* Obtain object ID of the top most object (widget) which is
            a direct child of the top level viewport.
         */

         /* Obtain the first selected object (the object drawn on top). */
         selected_obj = (GlgObject) GlgGetElement( selection_array, 0 );
         
         /* Obtain an ID of the object's parent which is a direct child 
            of the given viewport.
         */
         widget = GetDirectChild( viewport, selected_obj );
         if( widget )
         {
            /* In this example, process generic selection events only
               for widgets with no Actions.
            */
            if( GlgHasResourceObject( widget, "Actions" ) )
              return;
            
            /* Place custom code here to process selection. */

            /* Print widget name (ford emo purposes). */
            GlgGetSResource( widget, "Name", &obj_name );
            printf( "Processing generic ObjectSelection for object Name=%s\n", 
                    obj_name ); 

            /* Update the comment string in the drawing. */
            UpdateComment( viewport, widget, "Selected Widget Name: " ); 
         }
         break;
         
       default: return;
      }

      GlgUpdate( viewport );
   } /* format="ObjectSelection" */
} 

/*----------------------------------------------------------------------
| Load a GLG drawing from a specified file. Add callbacks.
*/
GlgObject LoadDrawing( char * drawing_file )
{
   GlgObject viewport;

   /* Load a GLG drawing from a file. */
   viewport = GlgLoadWidgetFromFile( drawing_file );

   if( !viewport )
   {
      GlgError( GLG_USER_ERROR, "Can't load drawing.\n" );
      return NULL;
   }

   /* Set window name (title). */
   GlgSetSResource( viewport, "ScreenName", "GlgObjectSelection example" );

   /* Add Input callback to handle user interaction and mouse events. */
   GlgAddCallback( viewport, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );

   /* Set ProcessMouse property to receive MouseClick, MouseOver and Tooltip
      events in the Input callback. This property may be set in the drawing
      as well.
   */
   GlgSetDResource( viewport, "ProcessMouse", 
          GLG_MOUSE_OVER_SELECTION | GLG_MOUSE_OVER_TOOLTIP | GLG_MOUSE_CLICK );

   /* Initialize the comment string, if found. */
   if( GlgHasResourceObject( viewport, "Comment" ) )
     GlgSetSResource( viewport, "Comment/String", "" );

   return viewport;
} 

/*----------------------------------------------------------------------
| Reset previously stored SelectedObject and SelectedEventLabel.
*/
void ResetSelectedObject()
{
   if( DEBUG )
     printf( "ResetSelectedObject\n" );

   SelectedObject = 0;
   GlgFree( SelectedEventLabel ); /* handles NULL */
   SelectedEventLabel = NULL;
}

/*----------------------------------------------------------------------
| Process commands attached to an object via Actions with
| ActionType = "SEND COMMAND".
*/
void ProcessCommand( GlgObject command_vp, GlgObject selected_obj, 
                     GlgObject command_obj, char *event_label )
{
   char
     * command_type,
     * tag_source,
     * value_res;;

   double value;
   
   if( !command_obj || !selected_obj )
     return;

   GlgGetSResource( command_obj, "CommandType", &command_type );

   /* Process command with CommandType = "WriteValue" */
   if( strcmp( command_type, "WriteValue" ) == 0 )
   {
      /* Retrieve output tag source (output data variable) from the command.  */
      GlgGetSResource( command_obj, "OutputTagHolder/TagSource", &tag_source );
      
      /* Validate. */
      if( !tag_source || !*tag_source || strcmp( tag_source, "unset" ) == 0 )
      {
         GlgError( GLG_USER_ERROR, 
                   "Invalid TagSource. WriteValue Command failed." );
         return;
      }
      
      /* Retrieve the value to be written to the specified tag source. */
      GlgGetDResource( command_obj, "Value", &value );
            
      /* Place custom code here to write new value to the specified tag source. 
         For demo purposes, set the value of the specified tag in the drawing.
      */
      GlgSetDTag( Viewport, tag_source, value, GlgTrue );
   }
   
   /* Process command with CommandType = "WriteValueFromWidget" */
   else if( strcmp( command_type, "WriteValueFromWidget" ) == 0 )
   {
      /* Obtain ValueResource from the command, an S-type resource
         indicating the resource name of the input widget holds the
         new widget value. For example, for a toggle, it will be resource
         "OnState"; for a swith widget, a spinner or a slider, 
         it will be resource named "Value". 
      */
      GlgGetSResource( command_obj, "ValueResource", &value_res );
      
      /* Obtain new value from the input widget. */
      GlgGetDResource( selected_obj, value_res, &value );

      /* Retrieve output tag source (output data variable) from the command.  */
      GlgGetSResource( command_obj, "OutputTagHolder/TagSource", &tag_source );

       /* Validate. */
      if( !tag_source || !*tag_source || strcmp( tag_source, "unset" ) == 0 )
      {
         GlgError( GLG_USER_ERROR, 
                   "Invalid TagSource. WriteValueFromWidget Command failed." );
         return;
      }

       /* Place custom code here to write new value to the specified tag source. 
         For demo purposes, set the value of the specified tag in the drawing.
      */
      GlgSetDTag( Viewport, tag_source, value, GlgTrue );
   }

   /* Process command with CommandType = "GoTo" */
   else if( strcmp( command_type, "GoTo" ) == 0 )
   {
      /* GoTo command will destroy the current drawing and replace it
         with a new drawing. Since the command is processed within the
         callback invoked on the currently loaded viewport, the
         viewport cannot be destroyed within its own callback, and
         the GoTo command should be processed on a timer. 
         The command_obj passed as a parameter to the timer procedure
         should be referenced.
      */
      GlgAddTimeOut( AppContext, 0, (GlgTimerProc) ProcessCommandGoTo, 
                     GlgReferenceObject( command_obj ) );

   }

   /* Process custom command with CommandType = "TankSelected" */
   else if( strcmp( command_type, "TankSelected" ) == 0 )
   {
      char * obj_name;

      /* Place custom code here to handle the command as needed. 
         selected_obj argument passed to this function holds the
         object ID of the selected tank.
      */
      GlgGetSResource( selected_obj, "Name", &obj_name );
      printf( "Processing custom command TankSelected for object Name=%s\n", 
              obj_name );
   }
}

/*----------------------------------------------------------------------
| Timer procedure for processing the GoTo command. It will
| destroy the currently loaded drawing and replace it with 
| a new drawing. The new drawing filename is specified 
| as the DrawingFile resource of the command object.
*/
void ProcessCommandGoTo( GlgObject command_obj, GlgLong * timer_id )
{
      char * drawing_file;
      GlgObject new_viewport;
      double x, y, width, height;

      /* Retrieve the drawing file from the command. */
      GlgGetSResource( command_obj, "DrawingFile", &drawing_file );

      /* Dereference command_obj, as it is no longer needed. */
      GlgDropObject( command_obj );

      /* If DrawingFile is not valid, abort the command. */
      if( !drawing_file || !*drawing_file )
      {
         GlgError( GLG_USER_ERROR, "Invalid DrawingFile, GoTo Command failed." );
         return;
      }

      /* Load a new drawing. Abort the GoTo command if drawing loading failed. */
      new_viewport = LoadDrawing( drawing_file );
      if( !new_viewport )
        return;

      /* Retrieve width and height of the old viewport, in case
         it was resized, and use it to set the size of the new viewport.
      */
      GlgGetDResource( Viewport, "Screen/Width", &width );
      GlgGetDResource( Viewport, "Screen/Height", &height);

      /* Position newly loaded viewport. */
      PositionViewport( new_viewport, 50., 50., width, height );

      /* Destroy previously loaded drawing, if any. */ 
      if( Viewport )
      {
         GlgResetHierarchy( Viewport );
         GlgDropObject( Viewport ); /* handles NULL */
      }
      
      /* Store the newly loaded viewport. */
      Viewport = new_viewport;

      /* Display the new drawing. */
      GlgInitialDraw( Viewport );
}

/*----------------------------------------------------------------------
| Process custom events attached to an object via Actions with
| ActionType = "SEND EVENT". 
*/
void ProcessSelectionEvent( GlgObject event_vp, GlgObject selected_obj,
                            char *event_label )
{
   char * obj_name;

   /* Add custom code here to process custom event as needed. */

   if( !selected_obj )
     return;

   /* Obtain selected object name. */
   GlgGetSResource( selected_obj, "Name", &obj_name );
   printf( "Processing selection event for object %s\n", obj_name ); 
}

/*----------------------------------------------------------------------
| Returns True if object selection event should be processed
| on MouseRelease as opposed to MouseClick. In this example, this
| condition is defined in the drawing using resource "OnRelease",
| which is added as a custom resource to an Action object, via
| "Add Data". It is expected that the ActionObject is named 
| and its HasResource=ON, so that we can access resource "OnRelease"
| from the action_obj.
| 
*/
GlgBoolean ProcessOnRelease( GlgObject selected_obj, GlgObject action_obj )
{
   GlgObject res_obj; 

   if( !selected_obj )
     return GlgFalse;

   /* Pre-3.5 Custom MouseClickEvent, process on MouseClick by default. 
      It may changed as needed.
   */
   if( !action_obj ) 
     return GlgFalse;

   /* Events added as Actions (GLG v.3.5 and later): 
      Retrieve OnRelease resource, if any. If set to 0, process selection on
      MouseClick. Otherwise, process selection on MouseRelease.
   */
   res_obj = GlgGetResourceObject( action_obj, "OnRelease" );
   if( res_obj )
   {
      double on_release;
      GlgGetDResource( res_obj, NULL, &on_release );
      if( on_release )      /* OnRelease != 0 */
        return GlgTrue;     /* Process on MouseRelease. */
   }

   /* Process on MouseClick. */
   return GlgFalse;
}

/*----------------------------------------------------------------------
| Returns the object/widget which is a parent of specified child and
| is a direct child of the specified viewport (it may be the object itself).
*/
GlgObject GetDirectChild( GlgObject viewport, GlgObject child )
{
   GlgObject container;
   GlgObject parent;

   /* Get the viewport's container (Array resource). */
   container = GlgGetResourceObject( viewport, "Array" );
   
   while( child )
   {
      parent = GlgGetParent( child, NULL );	
      if( parent == container )
        return child;
      
      child = parent;
   }

   return NULL;    /* Not found: should not happen in this example. */
}

/*----------------------------------------------------------------------
| Position a GLG viewport as a top level window.
*/ 
void PositionViewport( GlgObject viewport, double x, double y, 
                       double width, double height )
{
   if( !viewport )
     return;

   GlgSetGResource( viewport, "Point1", 0., 0., 0. );
   GlgSetGResource( viewport, "Point2", 0., 0., 0. );

   GlgSetDResource( viewport, "Screen/XHint", x );
   GlgSetDResource( viewport, "Screen/YHint", y );
   GlgSetDResource( viewport, "Screen/WidthHint", width );
   GlgSetDResource( viewport, "Screen/HeightHint", height );   
}


/*----------------------------------------------------------------------
| Update the comment string in the viewport using the resource
| Comment/String, if found.
*/
void UpdateComment( GlgObject viewport, GlgObject selected_obj, char *str )
{
   char *obj_name = NULL;
   char *comment = NULL;

   if( !GlgHasResourceObject( viewport, "Comment" ) )
       return;

   /* Retrieve selected object name and update the comment 
      string in the drawing, appending the object name, if any.
   */
   if( selected_obj )
   {
     GlgGetSResource( selected_obj, "Name", &obj_name );
     comment = GlgConcatStrings( str, obj_name );
     GlgSetSResource( viewport, "Comment/String", comment );  
     GlgFree( comment );
   }
   else 
     GlgSetSResource( viewport, "Comment/String", str );  
}
