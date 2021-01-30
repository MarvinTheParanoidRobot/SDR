/*------------------------------------------------------------------------
|  This example demonstrates how to constrain object attributes.
|
|  The controls.g drawing used in this example contains two triangles
|  named Object1 (green triangle) and Object2 (pink triangle). Both 
|  triangles have a move transformation attached, and the movement of 
|  the green triangle is controled by the slider (Factor attribute of 
|  the Object1 xform is constrained to  the Value resource of the slider). 
|
|  When  the user clicks on the Constrain button, the Factor of the Object2
|  xform is constrained to the Factor of the Object1 xform 
|  add custom properties to an object,
|  such as custom MouseClickEvent and TooltipString, as well as custom
|  DataValue property of the D (double) type.
------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "GlgApi.h"


GlgObject Viewport = (GlgObject)0;

/* Function prototypes */
void Input( GlgObject viewport, GlgAnyType client_data, 
	    GlgAnyType call_data );

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*------------------------------------------------------------------------
|  Main entry point.
*/
int GlgMain( argc, argv, InitAppContext )
     int argc;
     char *argv[];
     GlgAppContext InitAppContext;
{
   GlgAppContext AppContext;

   AppContext = GlgInit( False, InitAppContext, argc, argv );

   /* Load a GLG drawing from a file. */
   Viewport = GlgLoadWidgetFromFile( "constrain.g" );

   if( !Viewport )
   {
      fprintf( stderr, "\007Can't load <constrain.g> drawing.\n" );
      exit( 0 );
   }

   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( Viewport, "Point1", -600., -700., 0. );
   GlgSetGResource( Viewport, "Point2", 600., 700., 0. );

   /* Setting the window name (title). */
   GlgSetSResource( Viewport, "ScreenName", "GlgConstrain example" );

   /* Add an input callback to handle user interaction and 
      mouse events. */
   GlgAddCallback( Viewport, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );

   /* Paint the drawing */
   GlgInitialDraw( Viewport );

   /* Event loop */
   return (int) GlgMainLoop( AppContext );
}

/*------------------------------------------------------------------------
|  Input callback is invoked when user interacts with input objects in GLG
|  drawing. 
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
   
   GlgObject
      object2,
      from_attr,
      to_attr,
      attr,
      suspend_info;

   double state_value;

   message_obj = (GlgObject) call_data;

   /* Get the message's format, action and origin. */
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "Origin", &origin );

   /* Handle window closing. May use viewport's name. */
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );

   if( strcmp( format, "Button" ) == 0 )  
   {
      /* User clicked on the Contsrain/Unconstrain button */
      if( strcmp( action, "ValueChanged" ) == 0 )
      {
	 GlgGetDResource( message_obj, "OnState", &state_value );

	 /* Object2 has been drawn, need to suspend and then release it
	    to constrain or unconstrain its attributes. */
	 object2 = GlgGetResourceObject( viewport, "DrawingArea/Object2" );
	 suspend_info = GlgSuspendObject( object2 );
	 
	 if( state_value == 1. )   /* Constrain */
	 {
	    /* Constrain factor of the Object2's xform to the factor
	       of the Object1's xform. */

	    /* Get attribute to be constrained: 
               must use default attribute name for the last element of the 
               resource path. For the Move xform, Factor is XformAttr3. 
               Xform is the default attribute name for the transformation 
               attached  to an object. */
	    from_attr = GlgGetResourceObject( object2, "Xform/XformAttr3" );

	    /* Get the attribute to constrain to. */
	    to_attr = GlgGetResourceObject( viewport, 
				"DrawingArea/Object1/Xform/XformAttr3" );

	    /* Constrain "from" attribute to "to" attribute. */
	    GlgConstrainObject( from_attr, to_attr );	       
	 }
	 else /* state_value == 0 :  Unconstrain */
	 {
	    /* Unconstrain factor of the Object2's xform */

	    /* Get attribute to be unconstrained: must use default
	       attribute name for the last element of the resource path. */
	    attr = GlgGetResourceObject( object2, "Xform/XformAttr3" );
	    
	    GlgUnconstrainObject( attr );	       
	 }

	 GlgReleaseObject( object2, suspend_info ); /* Release */
	 GlgUpdate( viewport );
      }
   }
   else if( strcmp( format, "Slider" ) == 0 ) /* input event from a slider */
      GlgUpdate( viewport );
} 
