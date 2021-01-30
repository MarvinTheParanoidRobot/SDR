/*----------------------------------------------------------------------
| This example demonsrates how to process actions added to objects
| in the GLG Builder or HMI Configurator using custom OEM menu. 
| The example doesn't include period dynamic updates, its purpose
| is to demonstrate how to handle actions such as WriteValue,
| WriteCurrent, Popup and GoToView.
------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>

#include "GlgApi.h"

/* Top level viewport */
GlgObject 
   TopViewport = (GlgObject)0,
   DrawingArea = (GlgObject)0,
   PopupDialog = (GlgObject)0;

GlgAppContext AppContext;

/* Function prototypes */
void Input( GlgObject viewport, GlgAnyType client_data, 
	    GlgAnyType call_data );
void WriteTagValue( GlgObject viewport, char *tag_source, double value );
void DisplayPopup( char *target );
void ClosePopup();
void GoToView( char *drawing_name, GlgLong *timer_id );

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*----------------------------------------------------------------------
|  Main entry point.
*/
int GlgMain( argc, argv, InitAppContext )
     int argc;
     char *argv[];
     GlgAppContext InitAppContext;
{
   AppContext = GlgInit( False, InitAppContext, argc, argv );

   /* Load a GLG drawing from a file. */
   TopViewport = GlgLoadWidgetFromFile( "top_level.g" );

   if( !TopViewport )
   {
      fprintf( stderr, "\007Can't load <top_level.g> drawing.\n" );
      exit( 0 );
   }

   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( TopViewport, "Point1", -600., -700., 0. );
   GlgSetGResource( TopViewport, "Point2", 600., 700., 0. );

   /* Setting the window name (title). */
   GlgSetSResource( TopViewport, "ScreenName", "OEM Actions GLG Example" );

   /* Add an input callback to handle user interaction and 
      mouse events. 
      */
   GlgAddCallback( TopViewport, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );

   /* Obtain an object ID of the DrawingArea object, which is a
      SubWindow object in this example.
      */
   DrawingArea = GlgGetResourceObject( TopViewport, "DrawingArea" );

   /* Obtain object ID of the PopupDilaog viewport. */
   PopupDialog = GlgGetResourceObject( TopViewport, "PopupDialog" );

   /* Make PopupDialog invisible. */
   GlgSetDResource( PopupDialog, "Visibility", 0.0 );

   /* Paint the drawing */
   GlgInitialDraw( TopViewport );

   /* Event loop */
   return (int) GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
|  Input callback is invoked when user interacts with input objects in 
|  a GLG drawing. In this program, it is used to handle object selection 
|  events, when the user selects a GLG object with the mouse.
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
     * tag_source,
     * target;

   int i;
   GlgObject 
      selected_obj, /* selected object */
      action_res_obj; /* object id of OEMAction resource */

   double 
      action_type,
      value;

   message_obj = (GlgObject) call_data;

   /* Get the message's format, action and origin. */
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "Origin", &origin );

   /* Handle window closing. May use viewport's name. */
   if( strcmp( format, "Window" ) == 0 &&
       strcmp( action, "DeleteWindow" ) == 0 )
   {
      /* Close PopupDialog */
      if( strcmp( origin, "PopupDialog" ) == 0 )
         ClosePopup();
      else
         /* Exit application */
         exit( 0 );
   }

   /* Handle object selection events for objects with Custom MouseClick event */
   if( strcmp( format, "CustomEvent" ) == 0 )
   {
      /* Retrieve EventLabel */
      GlgGetSResource( message_obj, "EventLabel", &event_label );

      /* Prevent recursive GlgUpdate while destroying the drawing in 
         GoToView action; */
      if( strcmp( event_label, "" ) == 0 )
        return;    

      if( strcmp( action, "MouseClick" ) == 0 ) 
      {
         if( strcmp( event_label, "OEMActionEvent" ) == 0 )
         {
            /* Handle actions attached to the selected object */
            selected_obj = GlgGetResourceObject( message_obj, "Object" );
            if( !GlgHasResourceObject( selected_obj, "OEMAction" ) )
              return;
            
            action_res_obj = GlgGetResourceObject( selected_obj, "OEMAction" );
            GlgGetDResource( action_res_obj, "ActionType", &action_type );
            switch( (int)action_type )
            {
             default:
             case 0: /* Undefined action */
               break;
             case 1:   /* WriteValue */
               GlgGetSResource( action_res_obj, "Tag", &tag_source );
               GlgGetDResource( action_res_obj, "Value", &value );
               WriteTagValue( viewport, tag_source, value );
               break;
             case 2: /* WriteCurrent value */
               GlgGetSResource( action_res_obj, "Tag", &tag_source );
               GlgGetDTag( viewport, tag_source, &value );
               WriteTagValue( viewport, tag_source, value );
               break;
             case 3: /* Popup */
               GlgGetSResource( action_res_obj, "Target", &target );
               DisplayPopup( target ); 
               break;
             case 4: /* GOTO view */
               GlgGetSResource( action_res_obj, "Target", &target );
               /* Since GoToView action causes the current drawing
                  to be destroyed and it is executed while processing
                  events in that same drawing, GoToView action should 
                  be executed on a timer, after events for the current
                  drawing are finished processing. */
               GlgAddTimeOut( AppContext, 1 /*Time interval */, 
                             (GlgTimerProc)GoToView, (GlgAnyType)target );
               break;
            }
         }
      }
   }
   /* Handle events from a toggle button */
   else if( strcmp( format, "Button" ) == 0 &&
            strcmp( action, "ValueChanged" ) == 0 )
   {
      double button_value;
      GlgGetDResource( message_obj, "OnState", &button_value );

      /* If OnState resource of the button has a tag object, the value of that
         tag will correspond to the new value of OnState resource.
         If there are other tag objects in the drawing that have the same
         TagSource as the button, we need to synchronize these tags 
         with the new value from the button. 
         */
      selected_obj = GlgGetResourceObject( message_obj, "Object" );
      GlgGetSResource( selected_obj, "OnState/TagSource", &tag_source );

      /* Synchronize all tags that have a given TagSource */
      if( tag_source && *tag_source && strcmp( tag_source, "unset" ) !=0 )
         GlgSetDTag( viewport, tag_source, button_value, True );
   }
   /* Handle events from a push button */
   else if( strcmp( format, "Button" ) == 0 &&
            strcmp( action, "Activate" ) == 0 )
   {
      if( !origin ) /* don't process events for unnamed buttons */
	return;

      if( strcmp( origin, "PopupOKButton" ) == 0 )
	/* Close PopupDialog if the dialog's OK button was pressed */ 
	ClosePopup();
      else if( strcmp( origin, "MainButton" ) == 0 )
	/* Display main_drawing.g in the DrawingArea */
	GlgSetSResource( TopViewport, "DrawingArea/SourcePath",
			 "main_drawing.g" );
      else if( strcmp( origin, "QuitButton" ) == 0 )
          /* Exit */
          exit( GLG_EXIT_OK );
   }

   GlgUpdate( viewport );
} 

void WriteTagValue( GlgObject viewport, char *tag_source, double value )
{
   printf( "Write action, tag_source=%s value=%lf\n", 
           tag_source, value );

   if( !tag_source )
      return;
  
   /* Comment out the following line and add code to write tag value 
      to the database */
   GlgSetDTag( viewport, tag_source, value, True );
 }

void DisplayPopup( char *target )
{
   printf( "Popup action, target=%s\n", target );
   GlgSetSResource( PopupDialog, "DrawingArea/SourcePath", target );
   GlgSetSResource( PopupDialog, "ScreenName", target );
   GlgSetDResource( PopupDialog, "Visibility", 1.0 );
}

void ClosePopup()
{
   GlgSetDResource( PopupDialog, "Visibility", 0.0 );

}

void GoToView( char *drawing_name, GlgLong * timer_id )
{
   printf( "GoTo action, target=%s\n", drawing_name );
   GlgSetSResource( DrawingArea, "SourcePath", drawing_name );
   GlgUpdate( TopViewport );
}
 
