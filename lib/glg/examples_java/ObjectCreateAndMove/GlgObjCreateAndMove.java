/*-----------------------------------------------------------------------
|  This example demonstrates the following features:
|  - the use of the Trace callback;
|  - how to create a polygon object using the mouse;
|  - how to add an object to the drawing;
|  - how to traverse a container object, such as a polygon;
|  - how to convert screen coordinates to world coordinates;
|  - how to select and move objects with the mouse.
|
|  Drawing name is obj_create_move.g. 
|
|  This example uses methods of the Extended API.
-----------------------------------------------------------------------*/

import java.awt.*;
import java.awt.event.*;
import java.lang.reflect.*;
import javax.swing.*;
import com.genlogic.*;

public class GlgObjCreateAndMove extends GlgJBean
{
   static final long serialVersionUID = 0;

   static int SELECTION_RESOLUTION = 3;    // Selection sensitivity in pixels

   static final int MOVE = 0;
   static final int CREATE = 1;

   int Mode = MOVE;

   GlgObject DrawingArea;
   GlgObject NewPolygon;
   GlgObject SelectedObject;

   GlgPoint CursorPosition = new GlgPoint();
   GlgPoint WorldPoint = new GlgPoint();
   GlgPoint LastPosition = new GlgPoint();

   GlgCube SelectRectangle = new GlgCube();

   boolean IsReady = false; 
        
   //////////////////////////////////////////////////////////////////////
   // Constructor
   //////////////////////////////////////////////////////////////////////
   public GlgObjCreateAndMove()
   {
      // Bean's trace callback is disabled by default: activate.
      AddListener( GlgObject.TRACE_CB, this );
   }

   //////////////////////////////////////////////////////////////////////////
   // For use as a stand-alone application
   //////////////////////////////////////////////////////////////////////////
   public static void main( final String arg[] )
   {
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Main( arg ); } } );
   }

   //////////////////////////////////////////////////////////////////////
   public static void Main( final String arg[] )
   {
      String filename;

      class DemoQuit extends WindowAdapter
      {
         public void windowClosing( WindowEvent e ) { System.exit( 0 ); }
      } 

      if( Array.getLength( arg ) == 0 || arg[ 0 ] == null )
         filename = "obj_create_move.g";
      else
         filename = arg[ 0 ];

      JFrame frame = new JFrame();
      frame.setResizable( true );
      frame.setSize( 800, 650 );
      frame.addWindowListener( new DemoQuit() );

      GlgObjCreateAndMove glg_component = new GlgObjCreateAndMove(); 
      frame.getContentPane().add( glg_component );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      //
      glg_component.SetDrawingName( filename );
   }

   /////////////////////////////////////////////////////////////////
   // ReadyCallback() is invoked after the drawing is loaded, setup
   //  and initially drawn.
   /////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      DrawingArea = viewport.GetResourceObject( "DrawingArea" );
      if( DrawingArea == null )
        PrintToJavaConsole( "Can't find DrawingArea viewport.\n" );
      else
        // The drawing is ready to process dynamic updates.
        IsReady = true;    

      /* Set EditMode for DrawingArea, for preventing viewports with
         handlers to react to mouse clicks. 
      */
      GlgObject.SetEditMode( DrawingArea, null, true );
   }

   ///////////////////////////////////////////////////////////////////
   // The trace callback receives all low-level events and is used for
   // handling mouse interaction, such as creating and dragging 
   // objects.
   ///////////////////////////////////////////////////////////////////
   public void TraceCallback( GlgObject viewport, GlgTraceData trace_info )
   {
      if( !IsReady )
        return;

      // Retrieve information from the native event, trace_info.event.
      int event_type = trace_info.event.getID();
      int button = 0;
      switch( event_type )
      {
       // Handle mouse events.
       case MouseEvent.MOUSE_PRESSED:
       case MouseEvent.MOUSE_RELEASED:
         button = GetButton( trace_info.event );
       case MouseEvent.MOUSE_MOVED:
       case MouseEvent.MOUSE_DRAGGED:
         // Retrieve cursor position in screen coordinates.   
         CursorPosition.x = (double) ((MouseEvent)trace_info.event).getX();
         CursorPosition.y = (double) ((MouseEvent)trace_info.event).getY();
         CursorPosition.z = 0.;
         break;
            
       default: return;   // Nothing to do for other events
      }      

      if( DrawingArea.GetDResource( "ZoomToMode" ).intValue() != 0 )
        return;   // Don't handle mouse selection in ZoomTo mode.

      /* Cursor position is defined relatively to the viewport where the
         event occurred. Translate cursor position relatively to the 
         DrawingArea if the event occurred in a child viewport, such as a 
         toggle or a button.
      */
      if( trace_info.viewport != DrawingArea )
        GlgObject.TranslatePointOrigin( trace_info.viewport, DrawingArea, 
                                        CursorPosition );

      /* Add COORD_MAPPING_ADJ to the cursor coordinates for precise
         pixel mapping.
      */
      CursorPosition.x += GlgObject.COORD_MAPPING_ADJ;
      CursorPosition.y += GlgObject.COORD_MAPPING_ADJ;

      switch( Mode )
      {
       case MOVE:   // Object selection and move mode
         // Pass all events to the object move handler.
         MoveObjectHandler( event_type, button, CursorPosition );
         break;

       case CREATE:  // Object creation mode: create polygon
         // Pass all events to the object creation handler
         CreateObjectHandler( event_type, button, CursorPosition );
         break;
      }
   }

   ///////////////////////////////////////////////////////////////////
   // Object creation handler: handles all aspects of object creation.
   ///////////////////////////////////////////////////////////////////
   void CreateObjectHandler( int event_type, int button,
                             GlgPoint cursor_position )
   {
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:    // Add points on each mouse press
         switch( button )
         {
          case 1:
            if( NewPolygon == null )
            {
               // First click: polygon object is not created yet. 
               NewPolygon = CreatePolygon( CursorPosition );
            }
            else
            {
               // Get object id of the last point of the polygon
               GlgObject last_point = (GlgObject)
                 NewPolygon.GetElement( NewPolygon.GetSize() - 1 );
               
               // Set the last point coordinates to the cursor position
               SetPointCoord( last_point, CursorPosition );

               // Clone the last point to create a new point instance
                 // and add the new point to the polygon. It's coordinates
                 // are already set to the cursor position.
               NewPolygon.AddObjectToBottom( last_point.CopyObject() );
            }
            Update();
            break;

          case 2:
          case 3:
            if( NewPolygon != null )
              if( NewPolygon.GetSize() == 2 )
                // Only the first point has been created, the second point
                  // is used for dragging: delete the polygon.
                DrawingArea.DeleteObject( NewPolygon );
              else
                // Delete the last polygon's point used for dragging.
                NewPolygon.DeleteBottomObject();

            NewPolygon = null;
            ResetMode();    // Unset object creation mode
            Update();
            break;
         }
         break;          /* End of case: MOUSE_PRESSED */ 

       case MouseEvent.MOUSE_MOVED:    // Drag the last point on a mouse move
       case MouseEvent.MOUSE_DRAGGED:
         if( NewPolygon == null )
           return;    // No object created: nothing to do

         // Get object id of the last point of the polygon
         GlgObject last_point = (GlgObject)
           NewPolygon.GetElement( NewPolygon.GetSize() - 1 );
               
         // Set the last point coordinates to the cursor position
         SetPointCoord( last_point, CursorPosition );
         Update();
         break;
            
       default: return;
      }  
   }

   ///////////////////////////////////////////////////////////////////
   // Move creation handler. Selects an object on the mouse click,
   // moves the object on the mouse drag.
   ///////////////////////////////////////////////////////////////////
   void MoveObjectHandler( int event_type, int button, 
                           GlgPoint cursor_position )
   {
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:    // Select an object at the cursor
         if( button != 1 )
           return;

         // Store selected object.
         SelectedObject = 
           SelectTopLevelObject( DrawingArea, DrawingArea, cursor_position );
         
         // Store cursor position
         LastPosition.CopyFrom( cursor_position );
         break;

       case MouseEvent.MOUSE_MOVED:    // Drag the object
       case MouseEvent.MOUSE_DRAGGED:
         if( SelectedObject == null )
           return;    // No selected object: nothing to do

         SelectedObject.MoveObject( GlgObject.SCREEN_COORD, 
                                    LastPosition, cursor_position );

         // Store cursor position for the next move
         LastPosition.CopyFrom( cursor_position );

         Update();
         break;
            
       case MouseEvent.MOUSE_RELEASED:    // Stop moving
         SelectedObject = null;
         break;

       default: return;
      }  
   }

   ///////////////////////////////////////////////////////////////////
   // Input callback. Process Glg button selection here.
   ///////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject viewport, GlgObject message_obj )
   {
      String origin = message_obj.GetSResource( "Origin" );
      String format = message_obj.GetSResource( "Format" );
      String action = message_obj.GetSResource( "Action" );
      // String subaction = message_obj.GetSResource( "SubAction" );
      
      if( format.equals( "Button" ) ) // Input event occurred in a button
      {
         if( !action.equals( "Activate" ) )
            return;
         
         if( origin.equals( "CreatePolyButton" ) )
         {
            // Turn On ObjectCreation mode.
            Mode = CREATE;
         }
      }                            
   }

   ///////////////////////////////////////////////////////////////////
   GlgObject SelectTopLevelObject( GlgObject viewport, 
                                   GlgObject event_viewport,
                                   GlgPoint cursor_position )
   {
      /* Select all objects in the vicinity of the +-SELECTION_RESOLUTION 
         pixels from the actual mouse click position.
      */
      SelectRectangle.p1.x = cursor_position.x - SELECTION_RESOLUTION;
      SelectRectangle.p1.y = cursor_position.y - SELECTION_RESOLUTION;
      SelectRectangle.p2.x = cursor_position.x + SELECTION_RESOLUTION;
      SelectRectangle.p2.y = cursor_position.y + SELECTION_RESOLUTION;

      /* Returns an array of selected objects' IDs on the bottom of the object
         hierarchy.
      */
      GlgObject selected_array =
        GlgObject.CreateSelection( viewport, SelectRectangle, event_viewport );

      if( selected_array == null )
        return null;

      // We'll take the first selected object (the object drawn on top).
      GlgObject selected_object = (GlgObject) selected_array.GetElement( 0 );

      /* Obtain an ID of the object's parent which is a direct child of the
         DrawingArea viewport.
      */
      return GetDirectChild( DrawingArea, selected_object );
   }

   ///////////////////////////////////////////////////////////////////
   // An object drawn in a viewport may be inside of an arbitrary number
   // of groups. The selection method returns IDs of objects on the 
   // lowest level of hierarchy, while an application needs to move 
   // a top level group containing the object (consider a case when
   // a node needs to be moved, but the node's label or icon are 
   // selected). This function returns the parent of the object which
   // is a direct child of the viewport (it may be the object itself).
   ///////////////////////////////////////////////////////////////////
   GlgObject GetDirectChild( GlgObject viewport, GlgObject child )
   {
      // Get the viewport's container (Array resource).
      GlgObject container = viewport.GetResourceObject( "Array" );

      while( child != null )
      {
         GlgObject parent = child.GetParent();	
         if( parent == container )
           return child;

         child = parent;
      }
      return null;    // Not found: should not happen in this exercise.
   }
        
   ///////////////////////////////////////////////////////////////////
   // Create a new polygon object with 2 control points. Initialize 
   // both points to the given point position in screen coordinates. 
   ///////////////////////////////////////////////////////////////////
   GlgObject CreatePolygon( GlgPoint scr_point )
   {
        GlgObject poly_obj = new GlgPolygon( 2, null );

        // Get the first point of the polygon and set its coordinates.
        // GetElement returns Object, cast it to GlgObject. 
        GlgObject point = (GlgObject) poly_obj.GetElement( 0 );
        SetPointCoord( point, scr_point );

        // Get the second point of the polygon and set its coordinates.
        point = (GlgObject) poly_obj.GetElement( 1 );
        SetPointCoord( point, scr_point );

        // Add a new polygon object to the drawing.
        DrawingArea.AddObjectToBottom( poly_obj );
        
        return poly_obj;
   }

   ////////////////////////////////////////////////////////////////////////
   // Set coordinates of the control point to the given position in screen
   // coordinates.
   ////////////////////////////////////////////////////////////////////////
   void SetPointCoord( GlgObject point, GlgPoint scr_point )
   {
      // Convert screen coordinates  to world coordinates.
      DrawingArea.ScreenToWorld( true, scr_point, WorldPoint );

      // Set point coordinates, use null as the resource name.
      point.SetGResource( null, WorldPoint.x, WorldPoint.y, 0. );

   }

   ////////////////////////////////////////////////////////////////////////
   void ResetMode()
   {
      // Reset mode back to MOVE
      Mode = MOVE;
   }

   ////////////////////////////////////////////////////////////////////////
   int GetButton( AWTEvent event )
   {
      if( ! ( event instanceof InputEvent ) )
        return 0;
      
      InputEvent input_event = (InputEvent) event;
      int modifiers = input_event.getModifiers();
      
      if( ( modifiers & InputEvent.BUTTON3_MASK ) != 0 )
        return 3;
      else if( ( modifiers & InputEvent.BUTTON2_MASK ) != 0 )
        return 2;
      else
        return 1;
   }
}
