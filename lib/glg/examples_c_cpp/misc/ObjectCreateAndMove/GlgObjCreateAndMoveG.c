/*------------------------------------------------------------------------
|  This example demonstrates the following features:
|  - the use of the Trace callback;
|  - how to create a polygon object using the mouse;
|  - how to add an object to the drawing;
|  - how to traverse a container object, such as a polygon;
|  - how to convert screen coordinates to world coordinates and vise versa;
|  - how to select and move objects with the mouse.
|
|  Drawing name is obj_create_move.g. 
|
|  This example uses methods of the Extended API.
------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "GlgApi.h"

#define SELECTION_RESOLUTION  5    /* Selection sensitivity in pixels */

typedef enum _InteractionMode
{
   MOVE = 0,
   CREATE
} InteractionMode;

typedef enum _EventType
{
   MOUSE_PRESSED = 0,
   MOUSE_MOVED_EVENT,
   MOUSE_RELEASED
} EventType;

InteractionMode Mode = MOVE;

GlgObject DrawingArea = (GlgObject)0;
GlgObject Viewport = (GlgObject)0;
GlgObject NewPolygon = (GlgObject)0;
GlgObject SelectedObject = (GlgObject)0;

/* Function prototypes */
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void Trace( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void CreateObjectHandler( EventType event_type, int button, 
                          GlgPoint * cursor_position );
void MoveObjectHandler( EventType event_type, int button, 
			GlgPoint * cursor_position );
GlgObject SelectTopLevelObject( GlgObject viewport, 
				GlgObject event_viewport, 
				GlgPoint *cursor_position );
GlgObject GetDirectChild( GlgObject viewport, GlgObject child );
GlgObject CreatePolygon( GlgPoint * scr_point );
void SetPointCoord( GlgObject point, GlgPoint * scr_point );
void ResetMode();
void SetSize( GlgObject viewport, GlgLong x, GlgLong y, 
              GlgLong width, GlgLong height );

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*--------------------------------------------------------------------*/
int GlgMain( argc, argv, InitAppContext )
     int argc;
     char *argv[];
     GlgAppContext InitAppContext;
{
   GlgAppContext AppContext;

   AppContext = GlgInit( False, InitAppContext, argc, argv );

   /* Load a GLG drawing from a file. */
   Viewport = GlgLoadWidgetFromFile( "obj_create_move.g" );

   if( !Viewport )
   {
      fprintf( stderr, "\007Can't load <object_handling.g> drawing.\n" );
      exit( 0 );
   }
       
   DrawingArea = GlgGetResourceObject( Viewport, "DrawingArea" );
   if( !DrawingArea )
   {
      fprintf( stderr, "\007Can't find DrawingArea Viewport.\n" );
      exit( 0 );
   }

   /* Set window size. */
   SetSize( Viewport, 0, 0, 700, 600 );

   /* Setting the window name (title). */
   GlgSetSResource( Viewport, "ScreenName", "Glg Example" );

   /* Add input and trace callbacks to handle user interaction and 
      mouse events.
   */
   GlgAddCallback( Viewport, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );
   GlgAddCallback( DrawingArea, GLG_TRACE_CB, (GlgCallbackProc)Trace, NULL );

   /* Set EditMode for DrawingArea, for preventing viewports with
      handlers to react to mouse clicks. 
   */
   GlgSetEditMode( DrawingArea, NULL, GlgTrue );

   /* Paint the drawing. */
   GlgInitialDraw( Viewport );

   return (int) GlgMainLoop( AppContext );
}

/*-------------------------------------------------------------------------
|  Input callback is invoked when user interacts with input objects in GLG
|  drawing. In this program, it is used to handle selection of the 
|  CreatePolyButton object.
*/
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{
   GlgObject message_obj;
   char
     * format,
     * action,
     * origin;
      
   message_obj = (GlgObject) call_data;

   /* Get the message's format, action and origin. */
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "Origin", &origin );

   /* Handle window closing. May use viewport's name. */
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );

   /* Handle events occurred in a button. */
   if( strcmp( format, "Button" ) == 0 ) 
   {
      if( strcmp( action, "Activate" ) != 0 ) 
	 return;
      
      /* Handle selection in the CreatePolyButton button. */
      if( strcmp( origin , "CreatePolyButton" ) == 0 )
      {
	 /* Turn ON ObjectCreation mode. */
	 Mode = CREATE;
      }
   }
}

/*------------------------------------------------------------------------
|  The trace callback receives all low-level events and is used for
|  handling mouse interaction, such as creating and dragging 
|  objects.
*/
void Trace( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{
   GlgTraceCBStruct * trace_data;
   EventType event_type;
   int button = 0;
   double zoom_to_mode;
   GlgPoint cursor_position;
   GlgPoint2 root_coord;

   trace_data = (GlgTraceCBStruct*) call_data;

   /* Platform-specific code to extract event information. */
#ifndef _WINDOWS
   switch( trace_data->event->type )
   {
    case ButtonPress:
    case ButtonRelease:
      button = trace_data->event->xbutton.button;

      cursor_position.x = trace_data->event->xbutton.x;
      cursor_position.y = trace_data->event->xbutton.y;
      cursor_position.z = 0.;

      root_coord.x = trace_data->event->xbutton.x_root;
      root_coord.y = trace_data->event->xbutton.y_root;

      event_type = ( trace_data->event->type == ButtonPress ? 
                     MOUSE_PRESSED : MOUSE_RELEASED );
      break;

    case MotionNotify:
      cursor_position.x = trace_data->event->xmotion.x;
      cursor_position.y = trace_data->event->xmotion.y;
      cursor_position.z = 0.;

      root_coord.x = trace_data->event->xmotion.x_root;
      root_coord.y = trace_data->event->xmotion.y_root;

      event_type = MOUSE_MOVED_EVENT;
      break;

    default: return;  /* Nothing to do for other events */
   }      
#else
   switch( trace_data->event->message )
   {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MOUSEMOVE:
      cursor_position.x = GET_X_LPARAM( trace_data->event->lParam );
      cursor_position.y = GET_Y_LPARAM( trace_data->event->lParam );
      cursor_position.z = 0;

      switch( trace_data->event->message )
      {
       case WM_LBUTTONDOWN:
       case WM_LBUTTONUP:    button = 1; break;
       case WM_MBUTTONDOWN:
       case WM_MBUTTONUP:    button = 2; break;
       case WM_RBUTTONDOWN:
       case WM_RBUTTONUP:    button = 3; break;
      }
      switch( trace_data->event->message )
      {
       case WM_LBUTTONDOWN:
       case WM_MBUTTONDOWN:
       case WM_RBUTTONDOWN:  event_type = MOUSE_PRESSED; break;
       case WM_LBUTTONUP:     
       case WM_MBUTTONUP:     
       case WM_RBUTTONUP:    event_type = MOUSE_RELEASED; break;
       case WM_MOUSEMOVE:    event_type = MOUSE_MOVED_EVENT; break;
      }
      break;

    default: return;
   }
#endif

   GlgGetDResource( DrawingArea, "ZoomToMode", &zoom_to_mode );
   if( zoom_to_mode )
     return;   /* Don't handle mouse selection in ZoomTo mode. */

   /* Cursor position is defined relatively to the viewport where the
      event occurred. Translate cursor position relatively to the DrawingArea
      if the event occurred in a child viewport, such as a toggle or a button.
   */
   if( trace_data->viewport != DrawingArea )
   {
#ifdef _WINDOWS
      POINT root_point;

      /* On Windows, root coordinates are not supplied in the event: query
         them. */
      ClientToScreen( trace_data->window, &root_point );
      root_coord.x = root_point.x;
      root_coord.x = root_point.y
#endif

      GlgRootToScreenCoord( DrawingArea, (GlgPoint2*) &root_coord );
      cursor_position.x = root_coord.x;
      cursor_position.y = root_coord.y;
   }

   /* GLG_COORD_MAPPING_ADJ is added to the cursor coordinates for precise
      pixel mapping.
   */
   cursor_position.x += GLG_COORD_MAPPING_ADJ;
   cursor_position.y += GLG_COORD_MAPPING_ADJ;

   switch( Mode )
   {
    case MOVE:   /* Object selection and move mode. */
      /* Pass all events to the object move handler. */
      MoveObjectHandler( event_type, button, &cursor_position );
      break;

    case CREATE:  /* Object creation mode: create polygon. */
      /* Pass all events to the object creation handler. */
      CreateObjectHandler( event_type, button, &cursor_position );
      break;
   }
}

/*-----------------------------------------------------------------------
| Object creation handler: handles all aspects of object creation.
*/
void CreateObjectHandler( EventType event_type, int button, 
                          GlgPoint * cursor_position )
{
   GlgObject 
     last_point,
     new_point;

   switch( event_type )
   {
    case MOUSE_PRESSED:    /* Add points on each mouse press. */
      switch( button )
      {
       case 1:
         if( NewPolygon == NULL )
         {
            /* First click: polygon object is not created yet. */
            NewPolygon = CreatePolygon( cursor_position );
         }
         else
         {
            /* Get object id of the last point of the polygon. */
            last_point = (GlgObject)
              GlgGetElement( NewPolygon, GlgGetSize(NewPolygon) - 1 );
	       
            /* Set the last point coordinates to the cursor position. */
            SetPointCoord( last_point, cursor_position );

            /* Clone the last point to create a new point instance
               and add the new point to the polygon. Its coordinates
               are already set to the cursor position.
            */
            new_point = GlgCopyObject( last_point );
            GlgAddObjectToBottom( NewPolygon, new_point );
            GlgDropObject( new_point );
         }
         GlgUpdate( Viewport );
         break;

       case 2:    /* Finish adding points. */
       case 3:
         if( NewPolygon != NULL )
           if( GlgGetSize( NewPolygon ) == 2 )
             /* Only the first point has been created, the second point
                is used for dragging: not enough points to create a polygon,
                delete it.
             */
             GlgDeleteThisObject( DrawingArea, NewPolygon );
           else
             /* Delete the last point of the polygon, which was used for 
                dragging: create polygon with the rest of the points.
             */
             GlgDeleteBottomObject( NewPolygon );

         NewPolygon = NULL;
         ResetMode();    /* Unset object creation mode. */
         GlgUpdate( Viewport );
         break;
      }
      break;          /* End of case: MOUSE_PRESSED */ 

    case MOUSE_MOVED_EVENT:    /* Drag the last point on a mouse move. */
      if( NewPolygon == NULL )
        return;    /* No object created: nothing to do. */

      /* Get object id of the last point of the polygon. */
      last_point = (GlgObject)
        GlgGetElement( NewPolygon, GlgGetSize( NewPolygon ) - 1 );
	       
      /* Set the last point coordinates to the cursor position. */
      SetPointCoord( last_point, cursor_position );
      GlgUpdate( Viewport );
      break;
	    
    default: return;
   }  
}

/*----------------------------------------------------------------------
|  Object Move handler. Selects an object on the mouse click,
|  moves the object on the mouse move.
*/
void MoveObjectHandler( EventType event_type, int button, 
                        GlgPoint * cursor_position )
{
   static GlgPoint last_position;
   
   switch( event_type )
   {
    case MOUSE_PRESSED:    /* Select an object at the cursor. */
      if( button != 1 )
        return;

      /* Store selected object. */
      SelectedObject = 
        SelectTopLevelObject( DrawingArea, DrawingArea, cursor_position );

      /* Store cursor position. */
      last_position = * cursor_position;
      break;

    case MOUSE_MOVED_EVENT:    /* Drag the object. */
      if( !SelectedObject )
        return;    /* No selected object: nothing to do. */

      GlgMoveObject( SelectedObject, GLG_SCREEN_COORD, 
                     &last_position, cursor_position );
       
      /* Store cursor position for the next move. */
      last_position = * cursor_position;

      GlgUpdate( Viewport );
      GlgSync( Viewport );
      break;
	    
    case MOUSE_RELEASED:    /* Stop moving. */
      SelectedObject = NULL;
      break;

    default: return;
   }  
}

/*----------------------------------------------------------------------
|  Create a new polygon object with 2 control points. Initialize 
|  both points to the given point position in screen coordinates. 
*/
GlgObject CreatePolygon( GlgPoint * scr_point )
{
   GlgObject 
      poly_obj,
      point;

   poly_obj = GlgCreateObject( GLG_POLYGON, NULL, (GlgAnyType) 2, 
			       NULL, NULL, NULL );
   
   /* Get the first point of the polygon and set its coordinates.
      GetElement returns Object, cast it to GlgObject.
   */
   point = (GlgObject) GlgGetElement( poly_obj, 0 );
   SetPointCoord( point, scr_point );
   
   /* Get the second point of the polygon and set its coordinates. */
   point = (GlgObject) GlgGetElement( poly_obj, 1 );
   SetPointCoord( point, scr_point );
   
   /* Add a new polygon object to the drawing. */
   GlgAddObjectToBottom( DrawingArea, poly_obj );
   GlgDropObject( poly_obj );
   
   return poly_obj;
}

/*---------------------------------------------------------------------
|  Set coordinates of the control point to the given position in screen
|  coordinates.
*/
void SetPointCoord( GlgObject point, GlgPoint * scr_point )
{
   GlgPoint world_point;

   /* Convert screen coordinates  to world coordinates. */
   GlgScreenToWorld( DrawingArea, True, scr_point, &world_point );
   
   /* Set point coordinates, use NULL as the resource name. */
   GlgSetGResource( point, NULL, world_point.x, world_point.y, 0. );
}

/*----------------------------------------------------------------------
|  Returns an objectID of the selected object on the top level of the
|  object hierarchy. There could be multiple objects potentially selected
|  with the mouse, so the SelectTopLevelObject returns a top level
|  object.
*/
GlgObject SelectTopLevelObject( GlgObject viewport, GlgObject event_viewport, 
				GlgPoint * cursor_position )
{
   GlgObject 
      selected_array,
      selected_object;
      
   GlgRectangle select_rectangle;

   /* Select all objects in the vicinity of the +-SELECTION_RESOLUTION 
      pixels from the actual mouse click position. 
   */
   select_rectangle.p1.x = cursor_position->x - SELECTION_RESOLUTION;
   select_rectangle.p1.y = cursor_position->y - SELECTION_RESOLUTION;
   select_rectangle.p2.x = cursor_position->x + SELECTION_RESOLUTION;
   select_rectangle.p2.y = cursor_position->y + SELECTION_RESOLUTION;

   /* Returns an array of selected objects' IDs on the bottom of the 
      object hierarchy.
   */
   selected_array = 
      GlgCreateSelection( viewport, &select_rectangle, event_viewport );

   if( !selected_array )
      return NULL;

   /* We'll take the first selected object (the object drawn on top). */
   selected_object = (GlgObject) GlgGetElement( selected_array, 0 );

   /* Obtain an ID of the object's parent which is a direct child of the
      DrawingArea viewport. 
   */
   return GetDirectChild( DrawingArea, selected_object );
}

/*----------------------------------------------------------------------
|  An object drawn in a viewport may be inside of an arbitrary number
|  of groups. The selection method returns IDs of objects on the 
|  lowest level of hierarchy, while an application needs to move 
|  a top level group containing the object (consider a case when
|  a node needs to be moved, but the node's label or icon are 
|  selected). This function returns the parent of the object which
|  is a direct child of the viewport (it may be the object itself).
*/
GlgObject GetDirectChild( GlgObject viewport, GlgObject child )
{
   GlgObject container;

   /* Get the viewport's container (Array resource). */
   container = GlgGetResourceObject( viewport, "Array" );
   
   while( child != NULL )
   {
      GlgObject parent;
      parent = GlgGetParent( child, (long*)0 );	
      if( parent == container )
	 return child;
      
      child = parent;
   }
   return NULL;    /* Not found: should not happen in this exercise. */
}

/*---------------------------------------------------------------------
|  Reset the value of the Mode variable.
*/
void ResetMode()
{
   /* Reset mode back to MOVE */
   Mode = MOVE;
}

/*----------------------------------------------------------------------
| Set viewport size in screen cooridnates. 
*/
void SetSize( GlgObject viewport, GlgLong x, GlgLong y, 
              GlgLong width, GlgLong height )
{
   GlgSetGResource( viewport, "Point1", 0., 0., 0. );
   GlgSetGResource( viewport, "Point2", 0., 0., 0. );

   GlgSetDResource( viewport, "Screen/XHint", (double) x );
   GlgSetDResource( viewport, "Screen/YHint", (double) y );
   GlgSetDResource( viewport, "Screen/WidthHint", (double) width );
   GlgSetDResource( viewport, "Screen/HeightHint", (double) height );
}
