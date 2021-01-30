/*-----------------------------------------------------------------------
|  This example demonstrates the following features:
|  - the use of the Trace callback;
|  - drag&drop an icon from a palette to the Drawing Area;
|  - create a polygon object with the mouse;
|  - scale an object as it is placed in the drawing area;
|  - move an object with the mouse;
|  - highlight the selected object by displaying a rectangle surrounding the
|    object's bounding box; 
|  - cut/paste operations;
|  - save/load operations.
|
|  The drawing name is obj_position.g.
|
|  This examples uses methods of the Extended API.
-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "GlgApi.h"

#define SELECTION_RESOLUTION  5    /* Selection sensitivity in pixels */
#define DRAWING_NAME "new_diagram.g"

typedef enum _InteractionMode
{
   SELECT = 0,
   MOVE,
   CREATE,
   CREATE_POLYGON
} InteractionMode;

typedef enum _EventType
{
   MOUSE_PRESSED = 0,
   MOUSE_MOVED_EVENT,
   MOUSE_RELEASED
} EventType;

InteractionMode Mode = SELECT;

GlgObject DrawingArea = (GlgObject)0;
GlgObject Viewport = (GlgObject)0;
GlgObject PaletteObject = (GlgObject)0;
GlgObject SelectedObject = (GlgObject)0;
GlgObject CutBufferObject = (GlgObject)0;
GlgObject HighlightRectangle = (GlgObject)0;
GlgObject TemplatesVp = (GlgObject)0;
GlgObject NewPolygon = (GlgObject)0;

GlgPoint 
   DrawingArea_wp1, 
   DrawingArea_wp2;

/* Variable to store Grid interval for a DrawingArea. 
   It is initially defined in the drawing. */
double GridValue;

/* Function prototypes */
void Input( GlgObject viewport, GlgAnyType client_data, 
           GlgAnyType call_data );
void Trace( GlgObject viewport, GlgAnyType client_data, 
           GlgAnyType call_data );
void CreateObjectHandler( EventType event_type, int button, 
                         GlgPoint * cursor_position );
void CreatePolygonHandler( EventType event_type, int button, 
                         GlgPoint * cursor_position );
void MoveObjectHandler( EventType event_type, int button, 
                       GlgPoint * cursor_position );
GlgObject CreatePolygon( GlgPoint * scr_point );
void SetPointCoord( GlgObject point, GlgPoint * scr_point );
GlgObject SelectTopLevelObject( GlgObject viewport, 
                               GlgObject event_viewport, 
                               GlgPoint *cursor_position );
void SetSelectedObject( GlgObject selected_object );
void HighlightObject( GlgObject selected_object );
void PositionHighlightRectangle( GlgObject selected_object );
GlgObject GetDirectChild( GlgObject viewport, GlgObject child );
void GetBoxCenter( GlgObject object, GlgPoint * box_center );
void ResetMode();
void Init();

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*-----------------------------------------------------------------------
  Main entry point.
  */
int GlgMain( argc, argv, InitAppContext )
     int argc;
     char *argv[];
     GlgAppContext InitAppContext;
{
   GlgAppContext AppContext;

   AppContext = GlgInit( False, InitAppContext, argc, argv );

   /* Load a GLG drawing from a file. */
   Viewport = GlgLoadWidgetFromFile( "obj_position.g" );

   if( !Viewport )
   {
      GlgError( GLG_USER_ERROR, "Can't load <obj_position.g> drawing." );
      exit( GLG_EXIT_ERROR );
   }
       
   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( Viewport, "Point1", -550., -600., 0. );
   GlgSetGResource( Viewport, "Point2", 550., 600., 0. );

   /* Setting the window name (title). */
   GlgSetSResource( Viewport, "ScreenName", "GlgObjectPosition example" );
   
   /* Initializaion. */
   Init();

   /* Add input and trace callbacks to handle user interaction and 
      mouse events. Add Input callback to the top level viewport.
      Add Trace callback to the DrawingArea viewport.
   */
   GlgAddCallback( Viewport, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );
   GlgAddCallback( DrawingArea, GLG_TRACE_CB, (GlgCallbackProc)Trace, NULL );

   /* Paint the drawing */
   GlgInitialDraw( Viewport );

   return GlgMainLoop( AppContext );
}

/*-----------------------------------------------------------------------
  | Initialization code.
  */
void Init()
{
   DrawingArea = GlgGetResourceObject( Viewport, "DrawingArea" );
   if( !DrawingArea )
   {
      GlgError( GLG_USER_ERROR, "Can't find DrawingArea viewport." );
      exit( GLG_EXIT_ERROR );
   }

   /* Retrieve and store the world coordinates of the DrawingArea. */
   GlgGetGResource( DrawingArea, "Point1", &DrawingArea_wp1.x,
                  &DrawingArea_wp1.y, &DrawingArea_wp1.z  );
   GlgGetGResource( DrawingArea, "Point2", &DrawingArea_wp2.x,
                    &DrawingArea_wp2.y, &DrawingArea_wp2.z );

   /* Set Edit mode for DrawingArea, so that children viewports with
      handlers, such as sliders or dials, will not change values
      on mouse click in edit mode.
   */
   GlgSetEditMode( DrawingArea, NULL, True );

   /* Set grid color for the DrawingArea. Grid interval is defined in the
      drawing as an attribute of the DrawingArea viewport.
   */
   GlgSetGResource( (GlgObject)0, "$config/GlgGridPolygon/EdgeColor",
                    0.8, 0.8, 0.8 );

   /* Retrieve TemplesVp viewport containing icons. */
   TemplatesVp = GlgGetResourceObject( Viewport, "TemplatesVp" );
   if( !TemplatesVp )
   {
      GlgError( GLG_USER_ERROR, "Can't find TemplatesVp viewport." );
      exit( GLG_EXIT_ERROR );
   }

   /* Remove TemplesVp from the drawing, increase its referemce count and
      store it for future use.
   */
   GlgReferenceObject( TemplatesVp );
   GlgDeleteThisObject( Viewport, TemplatesVp );
   
   /* Rename the drawing area viewport to $Widget so that it could be saved
      and used as a widget in an application.
      */
   GlgSetSResource( DrawingArea, "Name", "$Widget" );
 }  

/*-----------------------------------------------------------------------
  |  The trace callback receives all low-level events and is used for
  |  handling mouse interaction, such as creating and dragging 
  |  objects.
  */
void Trace( viewport, client_data, call_data )
     GlgObject viewport;
     GlgAnyType client_data;
     GlgAnyType call_data;
{
   GlgTraceCBStruct * trace_data;
   EventType event_type;
   int button;
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
      button = 0;

      cursor_position.x = trace_data->event->xmotion.x;
      cursor_position.y = trace_data->event->xmotion.y;
      cursor_position.z = 0,;

      root_coord.x = trace_data->event->xmotion.x_root;
      root_coord.y = trace_data->event->xmotion.y_root;

      event_type = MOUSE_MOVED_EVENT;
      break;

    default: return;            /* Nothing to do for other events */
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
     return;        /* Don't handle mouse selection in ZoomTo mode. */

   /* Cursor position is defined relatively to the viewport where the
      event occurred. Translate cursor position relatively to the DrawingArea
      if the event occurred in a child viewport, such as a toggle or button.
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
    case MOVE:     /* Object move mode. */
    case SELECT:   /* Object selection mode. */
      /* Pass all events to the object move handler */
      MoveObjectHandler( event_type, button, &cursor_position );
      break;

    case CREATE:    /* Create an object from the icon template. */
      /* Pass all events to the object creation handler. */
      CreateObjectHandler( event_type, button, &cursor_position );
      break;

    case CREATE_POLYGON:
       /* Polygon Creation mode: create a polygon using a mouse. */
       CreatePolygonHandler( event_type, button, &cursor_position );
       break;
   }
}

/*-----------------------------------------------------------------------
  |  Input callback is invoked when the user interacts with input objects 
  |  in a GLG drawing. In this program, it is used to handle the selection 
  |  of the CreatePolyButton object.
  */
void Input( viewport, client_data, call_data )
     GlgObject viewport;
     GlgAnyType client_data;
     GlgAnyType call_data;
{
   GlgObject 
     message_obj,
     icon;
   char
     * format,
     * action,
     * origin,
     * full_origin;
      
   double icon_index;
   char * icon_name;

   message_obj = (GlgObject) call_data;

   /* Get the message's format, action and origin. */
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "Origin", &origin );
   GlgGetSResource( message_obj, "FullOrigin", &full_origin );

   /* Handle window closing. May use viewport's name. */
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( GLG_EXIT_OK  );

   /* Handle events occurred in a button. */
   if( strcmp( format, "Button" ) == 0 ) 
   {
      if( strcmp( action, "Activate" ) != 0 ) 
        return;
      
      /* Exit if the user selected the QuitButton */
      if( strcmp( origin, "QuitButton" ) == 0 )
         exit( GLG_EXIT_OK );
         
      /* Handle selection of the CreatePolygon button */
      if( strcmp( origin, "CreatePolygon" ) == 0 )
      {
         /* Turn ON Polygon Creation mode. */
	 Mode = CREATE_POLYGON;
      }
      /* Handle selection in other Create buttons. */
      else if( strstr( origin, "Create" ) != NULL )
      {
         /* Turn On ObjectCreation mode. */
         Mode = CREATE;
         
         /* Retrieve button index using Index resource attached to
            each Create button as a custom property.
         */
         GlgGetDResource( message_obj, "Object/Index", &icon_index );
         
         /* Retrieve the icon object with the given index from 
            TemplatesVp.
         */
         icon_name = GlgCreateIndexedName( "Icon", icon_index );
         icon = GlgGetResourceObject( TemplatesVp, icon_name );
         GlgFree( icon_name );
         
         if( !icon )
            return;

         /* Store the icon object for later use*/
         PaletteObject = icon;
      }
      else if( strcmp( origin, "SaveButton" ) == 0 ) /* Save the diagram. */
      {
         GlgObject tmp_object;

         /* Store SelectedObject, so that selection can be restored later. */
         tmp_object = SelectedObject;
         
         /* Retrieve and store the current Grid interval. 
            Set GridInterval=0 for the saved drawing. */
         GlgGetDResource( DrawingArea, "GridInterval", &GridValue );
         GlgSetDResource( DrawingArea, "GridInterval", 0. );

         /* Unselect objects, if any. */
         SetSelectedObject( (GlgObject)0 );

         /* Save diagram. */
         GlgSaveObject( DrawingArea, DRAWING_NAME );

         /* Restore current selection. */
         SetSelectedObject( tmp_object );
         
         /* Restore GridInterval */
         GlgSetDResource( DrawingArea, "GridInterval", GridValue );
      }
      else if( strcmp( origin, "LoadButton" ) == 0 ) 
      {
         GlgObject new_drawing;

         /* Load previously saved diagram. */
         new_drawing = GlgLoadWidgetFromFile( DRAWING_NAME );
         if( !new_drawing )
           return;

         /* Replace the current DrawingArea viewport with a new viewport. */
         GlgSetGResource( new_drawing, "Point1", DrawingArea_wp1.x, 
                         DrawingArea_wp1.y, 0.0 );

         GlgSetGResource( new_drawing, "Point2", DrawingArea_wp2.x, 
                         DrawingArea_wp2.y, 0.0 );

         SetSelectedObject( (GlgObject)0 );
         GlgDeleteThisObject( Viewport, DrawingArea );

         /* Reattach a Trace callback before hierarchy setup, which happens
            in AddObjectToBottom.
         */
         GlgAddCallback( new_drawing, GLG_TRACE_CB, (GlgCallbackProc)Trace, 
                         NULL );

         /* Add a new drawing viewport to the top $Widget viewport. */
         GlgAddObjectToBottom( Viewport, new_drawing );

         DrawingArea = new_drawing;
         GlgDropObject( new_drawing );
         
         /* Set EditMode for DrawingArea, for preventing viewports with
            handlers to react to mouse clicks.
         */
         GlgSetEditMode( DrawingArea, NULL, True );

         /* Set GridInterval */
         GlgSetDResource( DrawingArea, "GridInterval", GridValue );
      }
      else if( strcmp( origin, "CutButton" ) == 0 ) 
      {
         /* If no object is selected, do nothing. */
         if( !SelectedObject )
         {
            GlgBell( DrawingArea );
            return;
         }

         /* Destroy the object in the cut buffer if exists, works with NULL. */
         GlgDropObject( CutBufferObject );

         /* Increase reference count of the SelectedObject and store
            in the cut buffer.
         */
         CutBufferObject = GlgReferenceObject( SelectedObject );

         /* Delete the SelectedObject from the drawing. */
         GlgDeleteThisObject( DrawingArea, SelectedObject );

         /* The object was cut, unselect it. */
         SetSelectedObject( (GlgObject)0 );
      }
      else if( strcmp( origin, "PasteButton" ) == 0 ) 
      {
         if( !CutBufferObject )
         {
            GlgBell( DrawingArea );
            return;
         }

         /* Add object in the cut buffer to the drawing area. */
         GlgAddObjectToBottom( DrawingArea, CutBufferObject );
        
         /* Select pasted object. */
         SetSelectedObject( CutBufferObject );

         /* Prepare the cut buffer for the next paste. */ 

         /* Dereference previous object, it is still referenced by the 
            drawing.
         */
         GlgDropObject( CutBufferObject );
         
         /* Clone the object in the cut buffer. */
         CutBufferObject = GlgCloneObject( CutBufferObject, GLG_STRONG_CLONE );
      }
   }
   GlgUpdate( viewport );
}

/*-----------------------------------------------------------------------
| Object creation handler: handles all aspects of object creation.
| An object is created from the icon template.
*/
void CreateObjectHandler( event_type, button, cursor_position )
     EventType event_type;
     int button;
     GlgPoint * cursor_position;
     
{
   GlgPoint world_point;
   static int icon_num = 0;
   double obj_type;
   char *name;

   switch( event_type )
   {
    case MOUSE_PRESSED:         /* Add points on each mouse press. */
      switch( button )
      {
       case 1:
         if( !PaletteObject )
         {
            GlgError( GLG_USER_ERROR, "No PalleteObject.\n" );
            return;
         }
                
         /* Create a copy of the object instance. */
         PaletteObject = GlgCloneObject( PaletteObject, GLG_STRONG_CLONE );
           
         /* Set EdgeColor to black. */
         if( GlgHasResourceObject( PaletteObject, "EdgeColor" ) )
           GlgSetGResource( PaletteObject, "EdgeColor", 0., 0., 0. );

         /* Name the object. */
         icon_num++;
         name = GlgCreateIndexedName( "Icon%", icon_num );
         GlgSetSResource( PaletteObject, "Name", name );
         GlgFree( name );

         /* Add a new object to the drawing. Object gets setup by
            GlgAddObjectToBottom.
         */
         GlgAddObjectToBottom( DrawingArea, PaletteObject );
          
         /* Retreive object type. */
         GlgGetDResource( PaletteObject, "Type", &obj_type );

         /* If the object is of type REFERENCE, position it using its
            control point, named by default "Point".
         */
         if( obj_type == GLG_REFERENCE )
         {
            /* Convert cursor position in screen coordinates to world 
               coordinates.
            */
            GlgScreenToWorld( DrawingArea, True, cursor_position, &world_point );
            GlgSetGResource( PaletteObject, "Point", 
                            world_point.x, world_point.y, world_point.z );
         }
         else 
         {
            /* Position arbitrary object at the cursor position. */
            GlgPositionObject( PaletteObject, GLG_SCREEN_COORD,
                              GLG_HCENTER | GLG_VCENTER,
                              cursor_position->x, cursor_position->y, 
                              cursor_position->z );
         }

         if( GlgHasResourceObject( PaletteObject, "Scale" ) )
           GlgSetDResource( PaletteObject, "Scale", 1. );
         else
         {
            /* Scale an object around its center, by passing NULL as the
               center point. Alternatively, obtain the center of the bounding
               box and pass it as a center point. Object must be setup before
               doing so; GlgSetupHierarchy may need to be called after 
               GlgPositionObject, to update current object's coordinates.
            */
            GlgScaleObject( PaletteObject, GLG_SCREEN_COORD,
                           NULL, /*scale around object center*/
                           0.5, 0.5, 1. );
         }

         GlgSetupHierarchy( PaletteObject );

         /* Store currently selected object. */
         SetSelectedObject( PaletteObject );

         /* Dereference PaletteObject, as it is referenced by the drawing. */
         GlgDropObject( PaletteObject ); 
         PaletteObject = (GlgObject)0;

         ResetMode();           /* Unset object creation mode. */
                                 
         GlgUpdate( Viewport );
         break;

         /* Abort on button 2 and 3. */
       case 2:
       case 3:
         /* Don't dereference: it was assigned in the Input callback 
            without referencing it.
         */
         PaletteObject = (GlgObject)0;

         ResetMode();           /* Unset object creation mode. */
         GlgUpdate( Viewport );
         break;
      }
      break;                    /* End of case: MOUSE_PRESSED */ 

    default: return;
   }  
}


/*-----------------------------------------------------------------------
|  Object Move handler. Selects an object on the mouse click,
|  moves the object on the mouse drag.
*/
void MoveObjectHandler( event_type, button, cursor_position )
     EventType event_type;
     int button;
     GlgPoint * cursor_position;
{
   static GlgPoint last_position;
   GlgObject selected_object;
   
   switch( event_type )
   {
    case MOUSE_PRESSED:         /* Select an object at the cursor. */
      if( button != 1 )
        return;

      /* Obtain and store an objectID of the selected object at the cursor
         position. The selected object should be an object at the top
         level of the object hierarchy. Since cursor position is translated 
         in Trace callback relatively to the DrawingArea, we can pass
         DrawingArea to SelectTopLevelObject both as top viewport and
         event viewport.
      */
      selected_object = 
        SelectTopLevelObject( DrawingArea, DrawingArea, cursor_position );
      if( !selected_object || selected_object == HighlightRectangle )
        return;

      /* If an object was selected, prepare to move it. */

      /* Store selected object */
      SetSelectedObject( selected_object );
      GlgUpdate( DrawingArea );   /* Show selection. */

      /* Store cursor position. */
      last_position = * cursor_position;

      /* Switch to MOVE mode. */
      Mode = MOVE;
      break;

    case MOUSE_MOVED_EVENT:     /* Drag the selected object. */
      if( Mode != MOVE || !SelectedObject )
        return;    

      GlgMoveObject( SelectedObject, GLG_SCREEN_COORD,
                    &last_position, cursor_position );
      GlgSetupHierarchy( SelectedObject ); 
      PositionHighlightRectangle( SelectedObject );

      /* Store cursor position for the next move. */
      last_position = * cursor_position;

      GlgUpdate( DrawingArea );
      break;
            
    case MOUSE_RELEASED:        /* Stop moving. */
      ResetMode();
      break;

    default: return;
   }  
}

/*-----------------------------------------------------------------------
| Create a polygon object using a mouse. 
*/
void CreatePolygonHandler( event_type, button, cursor_position )
   EventType event_type;
   int button;
   GlgPoint * cursor_position;
  
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
         
       case 2:
       case 3:
         if( NewPolygon != NULL )
           if( GlgGetSize( NewPolygon ) == 2 )
             /* Only the first point has been created, the second point
                is used for dragging: delete the polygon.
             */
             GlgDeleteThisObject( DrawingArea, NewPolygon );
           else
             /* Delete the last point of the polygon, which was
                used for dragging.
             */
             GlgDeleteBottomObject( NewPolygon );
         
         NewPolygon = NULL;
         ResetMode();    /* Unset polygon creation mode. */
         GlgUpdate( DrawingArea );
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
|  Create a new polygon object with 2 control points. Initialize 
|  both points to the given point position in screen coordinates. 
*/
GlgObject CreatePolygon( scr_point )
   GlgPoint * scr_point;
{
   GlgObject 
      poly_obj,
      point;

   poly_obj = GlgCreateObject( GLG_POLYGON, NULL, (GlgAnyType) 2, 
			       NULL, NULL, NULL );
   
   /* Set EdgeColor to black. */
   GlgSetGResource( poly_obj, "EdgeColor", 0.0, 0.0, 0.0 );

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
void SetPointCoord( point, scr_point )
   GlgObject point;
   GlgPoint * scr_point;
{
   GlgPoint world_point;

   /* Convert screen coordinates  to world coordinates. */
   GlgScreenToWorld( DrawingArea, True, scr_point, &world_point );
   
   /* Set point coordinates, use NULL as the resource name. */
   GlgSetGResource( point, NULL, world_point.x, world_point.y, 0. );
}

/*-----------------------------------------------------------------------
|  Returns the center of the object's bounding box.
*/
void GetBoxCenter( object, box_center )
   GlgObject object;
   GlgPoint * box_center;
{ 
   /* Get object's center in screen coords. */
   GlgCube * box;
   box = GlgGetBoxPtr( object );
   
   box_center->x = ( box->p1.x + box->p2.x ) / 2.;
   box_center->y = ( box->p1.y + box->p2.y ) / 2.;
   box_center->z = 0.;   /* Ignore z */
}

/*-----------------------------------------------------------------------
|  Returns an objectID of the selected object on the top level of the
|  object hierarchy. There could be multiple objects potentially selected
|  with the mouse, so the SelectTopLevelObject returns a top level
|  object.
*/
GlgObject SelectTopLevelObject( viewport, event_viewport, 
                               cursor_position )
     GlgObject viewport;
     GlgObject event_viewport;
     GlgPoint * cursor_position;
{
   GlgObject 
     selected_array,
     selected_object;
      
   GlgRectangle select_rectangle;

   /* Select all objects in the vicinity of the +-SELECTION_RESOLUTION 
      pixels from the actual mouse click position. */
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
     return (GlgObject)0;

   /* We'll take the first selected object (the object drawn on top). */
   selected_object = GlgGetElement( selected_array, 0 );

   GlgDropObject( selected_array );

   /* Obtain an ID of the object's parent which is a direct child of the
      DrawingArea viewport.
   */
   return GetDirectChild( DrawingArea, selected_object );
}

/*-----------------------------------------------------------------------
|  An object drawn in a viewport may be inside of an arbitrary number
|  of groups. The selection method returns IDs of objects on the 
|  lowest level of hierarchy, while an application needs to move 
|  a top level group containing the object (consider a case when
|  a node needs to be moved, but the node's label or icon are 
|  selected). This function returns the parent of the object which
|  is a direct child of the viewport (it may be the object itself).
*/
GlgObject GetDirectChild( viewport, child )
   GlgObject viewport;
   GlgObject child;
{
   GlgObject container;

   /* Get the viewport's container (Array resource). */
   container = GlgGetResourceObject( viewport, "Array" );
   
   while( child )
   {
      GlgObject parent;
      parent = GlgGetParent( child, (GlgLong*)0 ); 
      if( parent == container )
        return child;
      
      child = parent;
   }
   return (GlgObject)0;    /* Not found: should not happen in this exercise. */
}


/*-------------------------------------------------g----------------------
|  Reset the value of the Mode variable.
*/
void ResetMode()
{
   /* Reset mode back to SELECT */
   Mode = SELECT;
}

/*-----------------------------------------------------------------------
|  Assign selected object.
*/
void SetSelectedObject( object )
     GlgObject object;
{
   if( SelectedObject == object )
     return;

   /* Remove highlight from the currently selected object (if any) and
      highlight the new one.
   */
   HighlightObject( object );

   /* Drop the old selected object and store the new one, works for NULL. */
   GlgDropObject( SelectedObject );
   SelectedObject = GlgReferenceObject( object );
}

/*-----------------------------------------------------------------------
| Highlight/Unhighlight object.
*/
void HighlightObject( object )
     GlgObject object;
{
   /* Create HighlightRectangle the first time if it doesn't exist. */
   if( !HighlightRectangle )
   {
      /* Create a polygon object with 4 points. */
      HighlightRectangle = GlgCreateObject( GLG_POLYGON, "HighlightRectangle", 
                                            (GlgAnyType) 4, 
                                            NULL, NULL, NULL );
   
      /* Set various attributes. */
      GlgSetDResource( HighlightRectangle, "FillType", 1. ); /*Edge*/
      GlgSetDResource( HighlightRectangle, "OpenType", 0. ); /*Closed*/
      GlgSetGResource( HighlightRectangle, "EdgeColor", 1., 0., 0. );
      GlgSetDResource( HighlightRectangle, "LineWidth", 1. );
   }

   /* Unhighlight object. */
   if( !object )
   {
      /* If there was a selected object, remove highlight from the drawing. */
      if( SelectedObject )
        GlgDeleteThisObject( DrawingArea, HighlightRectangle );
      return;
   }
   
   /* Position HighlightRectangle */
   PositionHighlightRectangle( object );

   /* If there was no previous selection, HighlightRectangle was not added to
      the drawing, add it now.
   */
   if( !SelectedObject )
     GlgAddObjectToBottom( DrawingArea, HighlightRectangle );
}

/*-----------------------------------------------------------------------
| Position HighlightRectangle to highlight the bounding box of the
| selected object.
*/
void PositionHighlightRectangle( selected_object )
     GlgObject selected_object;
{
#define PADDING 8

   GlgCube * box;
   GlgPoint wp1, wp2;

   /* Retrieve bounding box of the object in screen coordinates. */
   box = GlgGetBoxPtr( selected_object );
   
   /* Convert screen coordinates of the bounding box to world coordinates
      of the DrawingArea.
   */
   GlgScreenToWorld( DrawingArea, True, &box->p1, &wp1 );
   GlgScreenToWorld( DrawingArea, True, &box->p2, &wp2 );

   /* Set point coordinates of the HighlightRectangle */
   GlgSetGResource( GlgGetElement( HighlightRectangle, 0 ), NULL,
                    wp1.x - PADDING, wp1.y + PADDING, 0. );
   GlgSetGResource( GlgGetElement( HighlightRectangle, 1 ), NULL,
                    wp1.x - PADDING, wp2.y - PADDING, 0. );
   GlgSetGResource( GlgGetElement( HighlightRectangle, 2 ), NULL,
                    wp2.x + PADDING , wp2.y - PADDING, 0. );
   GlgSetGResource( GlgGetElement( HighlightRectangle, 3 ), NULL,
                    wp2.x + PADDING, wp1.y + PADDING, 0. );
}
