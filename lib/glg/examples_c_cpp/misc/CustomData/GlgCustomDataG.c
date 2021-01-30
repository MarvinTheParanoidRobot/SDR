/*-----------------------------------------------------------------------
| This example demonstrates how to programmatically add custom properties 
| to an object, such as custom MouseClickEvent and TooltipString, as well 
| as a custom DataValue property of the D (double) type.
-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "GlgApi.h"


GlgObject Viewport = (GlgObject)0;

/* Function prototypes */
void Input( GlgObject viewport, GlgAnyType client_data, 
	    GlgAnyType call_data );
void AddCustomData( GlgObject object, char *obj_name, double value );

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*-----------------------------------------------------------------------
|  Main entry point.
*/
int GlgMain( argc, argv, InitAppContext )
     int argc;
     char *argv[];
     GlgAppContext InitAppContext;
{
   GlgAppContext AppContext;
   GlgObject
      circle,
      triangle;

   AppContext = GlgInit( False, InitAppContext, argc, argv );

   /* Load a GLG drawing from a file. */
   Viewport = GlgLoadWidgetFromFile( "custom_data.g" );

   if( !Viewport )
   {
      fprintf( stderr, "\007Can't load <obj_selection.g> drawing.\n" );
      exit( 0 );
   }

   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( Viewport, "Point1", -600., -700., 0. );
   GlgSetGResource( Viewport, "Point2", 600., 700., 0. );

   /* Setting the window name (title). */
   GlgSetSResource( Viewport, "ScreenName", "GlgCustomData example" );

   /* Add an input callback to handle user interaction and 
      mouse events. */
   GlgAddCallback( Viewport, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );

   /* Retrieve an object named "Circle" and add CustomData 
      property to it */
   circle = GlgGetResourceObject( Viewport, "Circle" );
   AddCustomData( circle, "Circle", 1. );

   /* Retrieve an object named "Triangle" and add CustomData 
      property to it */
   triangle = GlgGetResourceObject( Viewport, "Triangle" );
   AddCustomData( triangle, "Triangle", 2. );

   /* Paint the drawing */
   GlgInitialDraw( Viewport );

   /* Event loop */
   return (int) GlgMainLoop( AppContext );
}

/*-----------------------------------------------------------------------
|  Add CustomData properties to objects. This function demonstrates
|  how to add custom porperties of type MouseClickEvent, TooltipString
|  and Double value.
*/
void AddCustomData( object, obj_name, value )
   GlgObject object;
   char *obj_name;
   double value;
{
   GlgObject
      suspend_info,
      group,
      tooltip_prop,
      event_prop,
      double_prop;

   char *value_string;

   /* Suspend the object since it has been already drawn */
   suspend_info = GlgSuspendObject( object );

   /* Create a group that holds all custom properties of an object*/
   group = GlgCreateObject( GLG_ARRAY, NULL, (GlgAnyType)GLG_OBJECT, 
			    NULL, NULL, NULL );

   /* Add tooltip property. Create a GLG attribute object of type S,
      name the attribute "TooltipString" and set its value to obj_name  */
   tooltip_prop =  GlgCreateObject( GLG_ATTRIBUTE, "TooltipString",
				    (GlgAnyType)GLG_S, 
				    (GlgAnyType)GLG_SDATA_XR, 
				    NULL, NULL );
   GlgSetSResource( tooltip_prop, NULL, obj_name );

   /* Add tooltip attribute to the group containing all custom properties */
   GlgAddObjectToBottom( group, tooltip_prop );    

   /* Add custom mouse click event property. Create a GLG attribute object 
      of type S, name the attribute "MouseClickEvent" and set its value 
      to (obj_name + "Event")  */
   event_prop = GlgCreateObject( GLG_ATTRIBUTE, "MouseClickEvent",
				 (GlgAnyType)GLG_S, 
				 (GlgAnyType)GLG_SDATA_XR, 
				 NULL, NULL );
   value_string = GlgConcatStrings( obj_name, "Event" );
   GlgSetSResource( event_prop, NULL, value_string );   /* Set value */

   GlgFree( value_string );

   /* Add mouse click event to the group */
   GlgAddObjectToBottom( group, event_prop );        
							 
   /* Add custom property of D type. Set the property name to "DoubleValue",
      and the property value to "value"  */
   double_prop = GlgCreateObject( GLG_ATTRIBUTE, "DoubleValue",
				  (GlgAnyType)GLG_D, 
				  (GlgAnyType)GLG_DDATA_XR, 
				  NULL, NULL );
		  
   GlgSetDResource( double_prop, NULL, value );   /* Set value */

   /* Add to group. */
   GlgAddObjectToBottom( group, double_prop );

   /* Set object's HasResources=1 so that the custom properties are
      visible as object's resources. */
   GlgSetDResource( object, "HasResources", 1. );

   /* Attach the group containing custom properties to the object. */
   GlgSetResourceObject( object, "CustomData", group );      

   GlgReleaseObject( object, suspend_info );
}

/*-----------------------------------------------------------------------
|  Input callback is invoked when user interacts with input objects in 
|  GLG drawing. In this program, it is used to handle object selection 
|  events, when teh user selects a GLG object with the mouse.
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
     * origin,
     * event_label,
     * object_name;
   double value;

   message_obj = (GlgObject) call_data;

   /* Get the message's format, action and origin. */
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "Origin", &origin );

   /* Handle window closing. May use viewport's name. */
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );

   /* Print custom properties of the selected object if any. */
   if( strcmp( format, "CustomEvent" ) == 0 )
   {
      GlgGetSResource( message_obj, "EventLabel", &event_label );

      /* Process mouse release and tooltip erase: invoked with
         an empty lable and null object.
      */
      if( !*event_label )
      { 
         if( strcmp( action, "ObjectTooltip" ) == 0 )
           printf( "Custom event: Tooltip Erased\n\n" );
         else if( strcmp( action, "MouseRelease" ) == 0 )
           printf( "Custom event: Mouse Released\n\n" );
         return;
      }

      /* An example of getting an object id of the selected object.
         GlgObject selected_object;
         selected_object = GlgGetResourceObject( message_obj, "Object" );
      */

      GlgGetSResource( message_obj, "Object/Name", &object_name );
      GlgGetDResource( message_obj, "Object/DoubleValue", &value );

      printf( "Custom event:\n" );
      printf( "   action: %s\n", action );
      printf( "   event label: %s\n", event_label );
      printf( "   selected object name: %s\n", object_name );
      printf( "   data value: %lf\n\n", value ); 
   }
} 
