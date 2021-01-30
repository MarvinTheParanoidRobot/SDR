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

import java.awt.*;
import java.awt.event.*;
import java.lang.reflect.*;
import javax.swing.*;
import com.genlogic.*;

public class GlgObjectPosition extends GlgJBean
{
   static final long serialVersionUID = 0;

   static int SELECTION_RESOLUTION = 3;    // Selection sensitivity in pixels
   static String DRAWING_NAME = "new_diagram.g"; //Saved drawing filename 

   // Interraction Modes.
   static final int SELECT = 0;
   static final int MOVE = 1;
   static final int CREATE = 2;
   static final int CREATE_POLYGON = 3;
   
   // Event types.
   static final int MOUSE_PRESSED = 0;
   static final int MOUSE_MOVED_EVENT = 1;
   static final int MOUSE_RELEASED = 2;

   // Global variable Mode stores the current editing mode. 
   int Mode = SELECT;

   GlgObject DrawingArea;
   GlgObject SelectedObject;
   GlgObject PaletteObject;
   GlgObject CutBufferObject;
   GlgObject HighlightRectangle;
   GlgObject TemplatesVp;
   GlgObject NewPolygon;

   GlgPoint CursorPosition = new GlgPoint();
   GlgPoint LastPosition = new GlgPoint();
   GlgPoint BoxCenter = new GlgPoint();
   GlgPoint DrawingArea_wp1 = new GlgPoint();
   GlgPoint DrawingArea_wp2 = new GlgPoint();
   
   // Allocate variables to perfrom coordinate coneversion when neeeded.
   GlgPoint wp1 = new GlgPoint();
   GlgPoint wp2 = new GlgPoint();

   // Variable to store Grid interval for a DrawingArea. 
   // It is initially defined in the drawing.
   double GridValue;

   GlgCube SelectRectangle = new GlgCube();

   // Object counter.
   int IconNum;
   
   boolean IsReady = false; 
        
   //////////////////////////////////////////////////////////////////////
   // Constructor
   //////////////////////////////////////////////////////////////////////
   public GlgObjectPosition()
   {
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
         filename = "obj_position.g";
      else
         filename = arg[ 0 ];

      JFrame frame = new JFrame();
      frame.setResizable( true );
      frame.setSize( 850, 700 );
      frame.addWindowListener( new DemoQuit() );

      GlgObjectPosition glg_component = new GlgObjectPosition(); 
      frame.getContentPane().add( glg_component );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      //
      glg_component.SetDrawingName( filename );
   }

   /////////////////////////////////////////////////////////////////
   // HCallback() is invoked after the drawing is loaded, but before
   // hierarchy setup.
   /////////////////////////////////////////////////////////////////
   public void HCallback( GlgObject viewport )
   {
      // Retrieve DrawingArea viewport.
      DrawingArea = viewport.GetResourceObject( "DrawingArea" );
      if( DrawingArea == null )
      {
         PrintToJavaConsole( "Can't find DrawingArea viewport.\n" );
         return;
      }

      // Retrieve TemplesVp viewport containing icons.
      TemplatesVp = viewport.GetResourceObject( "TemplatesVp" );
      if( TemplatesVp == null )
      {
         PrintToJavaConsole( "Can't find TemplatesVp viewport.\n" );
         return;
      }
      
      // Remove TemplesVp from the drawing, increase its referemce count and
      // store it for future use.
      DeleteObject( viewport, TemplatesVp );
      
      // Retrieve and store the world coordinates of the DrawingArea.
      DrawingArea_wp1 = DrawingArea.GetGResource( "Point1" );
      DrawingArea_wp2 = DrawingArea.GetGResource( "Point2" );
      
      // Set Edit mode for DrawingArea, so that children viewports with
      // handlers, such as sliders or dials, will not change values
      // on mouse click in edit mode.
      GlgObject.SetEditMode( DrawingArea, null, true );
   
      // Rename the drawing area viewport to $Widget so that it could be saved
      //  and used as a widget in an application.
      DrawingArea.SetSResource( "Name", "$Widget" );

      // Set grid color for the DrawingArea. Grid interval is defined in the
      // drawing as an attribute of the DrawingArea viewport.
      SetGResource( null, "$config/GlgGridPolygon/EdgeColor",
                    0.8, 0.8, 0.8 );

      // Enable Trace listener for the DrawingArea viewport. It needs
      // to be done before hierarchy setup, therefore it is invoked in the
      // H callback. The rest of the initialization is done in Ready callback.
      DrawingArea.AddListener( GlgObject.TRACE_CB, this );
   }
   
      
   /////////////////////////////////////////////////////////////////
   // ReadyCallback() is invoked after the drawing is loaded, setup
   //  and initially drawn.
   /////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      IsReady = true;    
   }

   ///////////////////////////////////////////////////////////////////
   // The trace callback receives all low-level events and is used for
   // handling mouse interaction, such as creating and dragging 
   // objects.
   ///////////////////////////////////////////////////////////////////
   public void TraceCallback( GlgObject viewport, GlgTraceData trace_info )
   {
      if( !IsReady() )
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

      // Cursor position is defined relatively to the viewport where the
      // event occurred. Translate cursor position relatively to the 
      // DrawingArea if the event occurred in a child viewport, such as a 
      // toggle or a button.
      if( trace_info.viewport != DrawingArea )
         GlgObject.TranslatePointOrigin( trace_info.viewport, DrawingArea, 
                                         CursorPosition );
      
      // COORD_MAPPING_ADJ is added to the cursor coordinates for precise
      // pixel mapping.
      CursorPosition.x += GlgObject.COORD_MAPPING_ADJ;
      CursorPosition.y += GlgObject.COORD_MAPPING_ADJ;

      switch( Mode )
      {
       case SELECT: // Object selection mode
       case MOVE:   // Object move mode
         // Pass all events to the object move handler.
         MoveObjectHandler( event_type, button, CursorPosition );
         break;

       case CREATE:  // Object creation mode: create polygon
         // Pass all events to the object creation handler
         CreateObjectHandler( event_type, button, CursorPosition );
         break;

       case CREATE_POLYGON:
         // Polygon Creation mode: create a polygon using a mouse.
         CreatePolygonHandler( event_type, button, CursorPosition );
         break;
      }
   }

   ///////////////////////////////////////////////////////////////////
   // Input callback. Process Glg button selection here.
   ///////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject viewport, GlgObject message_obj )
   {
      String full_origin = message_obj.GetSResource( "FullOrigin" );
      String origin = message_obj.GetSResource( "Origin" );
      String format = message_obj.GetSResource( "Format" );
      String action = message_obj.GetSResource( "Action" );
      // String subaction = message_obj.GetSResource( "SubAction" );
      
      if( format.equals( "Button" ) ) // Input event occurred in a button
      {
         if( !action.equals( "Activate" ) )
            return;
         
         // Exit if the user selected the QuitButton.
         if( origin.equals("QuitButton") )
            System.exit( 0 );
         
         // Handle selection of the CreatePolygon button.
         if( origin.equals( "CreatePolygon" ) )
         {
            // Turn ON Polygon Creation mode.
            Mode = CREATE_POLYGON;
         }
         // Handle selection in other Create buttons.
         else if( origin.startsWith( "Create" ) )
         {
            // Turn On ObjectCreation mode.
            Mode = CREATE;

            // Retrieve button index using Index resource attached to
            // each Create button as a custom property.
            int icon_index = 
              message_obj.GetDResource( "Object/Index" ).intValue();

            // Retrieve the icon object with the given index from 
            // TemplatesVp. 
            GlgObject icon = 
              TemplatesVp.GetResourceObject( "Icon" + icon_index );
            if( icon == null )
               return;
            
            // Store the icon object for later use.
            PaletteObject = icon;
         }
         else if( origin.equals("SaveButton") ) // Save the diagram.
         {
            GlgObject tmp_object;
            
            // Store SelectedObject, so that selection can be restored later.
            tmp_object = SelectedObject;

            // Retrieve and store the current Grid interval. 
            // Set GridInterval=0 for the saved drawing.
            GridValue = 
              DrawingArea.GetDResource( "GridInterval" ).doubleValue();
            DrawingArea.SetDResource( "GridInterval", 0. );
                        
            // Unselect objects, if any.
            SetSelectedObject( null );

            // Save diagram.
            DrawingArea.SaveObject( DRAWING_NAME );

            // Restore current selection.
            SetSelectedObject( tmp_object );
            
            // Restore GridInterval.
            DrawingArea.SetDResource( "GridInterval", GridValue );
         }
         else if( origin.equals("LoadButton") ) 
         {
            GlgObject new_drawing;

            // Load previously saved diagram.
            new_drawing = GlgObject.LoadWidget( DRAWING_NAME, GlgObject.FILE );
            if( new_drawing == null )
               return;

            // Replace the current DrawingArea viewport with a new viewport.
            new_drawing.SetGResource( "Point1", DrawingArea_wp1 );
            new_drawing.SetGResource( "Point2", DrawingArea_wp2 );
            
            SetSelectedObject( null );
            viewport.DeleteObject( DrawingArea );

            // Reattach a Trace callback before hierarchy setup, which happens
            // in AddObjectToBottom.
            new_drawing.AddListener( GlgObject.TRACE_CB, this );

            // Add a new drawing viewport to the top $Widget viewport.
            viewport.AddObjectToBottom( new_drawing );

            DrawingArea = new_drawing;

            // Set EditMode for DrawingArea, for preventing viewports with
            // handlers to react to mouse clicks. 
            GlgObject.SetEditMode( DrawingArea, null, true );

            // Set GridInterval.
            DrawingArea.SetDResource( "GridInterval", GridValue );
         }
         else if( origin.equals("CutButton") ) 
         {
            /* If no object is selected, do nothing */
            if( SelectedObject == null )
               return;

            // Store SelectedObject in the CutBuffer
            CutBufferObject = SelectedObject;

            // Delete SelectedObject from the drawing.
            DrawingArea.DeleteObject( SelectedObject );

            // The object was cut, unselect it.
            SetSelectedObject( null );
         }
         else if( origin.equals("PasteButton") ) 
         {
            if( CutBufferObject == null )
               return;

            // Add object in the cut buffer to the drawing area.
            DrawingArea.AddObjectToBottom( CutBufferObject );
        
            // Select pasted object.
            SetSelectedObject( CutBufferObject );

            // Prepare the cut buffer for the next paste. 
            // Clone the object in the cut buffer.
            CutBufferObject = 
              CutBufferObject.CloneObject( GlgObject.STRONG_CLONE );
         }
      }        
      Update();                    
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
            if( PaletteObject == null )
            {
               PrintToJavaConsole( "No PalletteObject. \n");
               return;
            }

            // Create a copy of the object instance
            PaletteObject = 
              PaletteObject.CloneObject( GlgObject.STRONG_CLONE );

            // Set EdgeColor to black. 
            if( PaletteObject.HasResourceObject( "EdgeColor" ) )
               PaletteObject.SetGResource( "EdgeColor", 0., 0., 0. );
            
            // Name the object.
            IconNum++;
            PaletteObject.SetSResource( "Name", "Icon" + IconNum );
            
            // Add a new object to the drawing. Object gets setup by
            // AddObjectToBottom.
            DrawingArea.AddObjectToBottom( PaletteObject );

            // Retreive object type.
            double obj_type = 
              PaletteObject.GetDResource( "Type" ).doubleValue();
            
            // If the object is of type REFERENCE, position it using its
            // control point, named by default "Point".
            if( obj_type == GlgObject.REFERENCE )
            {
               // Convert cursor position in screen coordinates 
               // to world coordinates.
               DrawingArea.ScreenToWorld( true, cursor_position, wp1 );
               PaletteObject.SetGResource( "Point", wp1 );
            }
            else 
            {
               // Position arbitrary object at the cursor position */
               PaletteObject.
                 PositionObject( GlgObject.SCREEN_COORD,
                                 GlgObject.HCENTER | GlgObject.VCENTER,
                                 cursor_position.x, cursor_position.y, 
                                 cursor_position.z );
            }

            if( PaletteObject.HasResourceObject( "Scale" ) )
               PaletteObject.SetDResource( "Scale", 1. );
            else
            {
               // Scale object around its center, by passing null as the
               // center point. Alternatively, obtain the center of the 
               // bounding box and pass it as a center point. Object must be 
               // setup before doing so; SetupHierarchy may need
               // to be called after PositionObject, to update current
               // object's coordinates.
               PaletteObject.ScaleObject( GlgObject.SCREEN_COORD,
                                          null /*scale around object center*/ , 
                                          0.5, 0.5, 1.0 );
            }

            PaletteObject.SetupHierarchy();

            // Store currently selected object.
            SetSelectedObject( PaletteObject );

            PaletteObject = null;
            ResetMode();           // Unset object creation mode.
                                 
            Update();
            break;

            // Abort on button 2 and 3
         case 2:
         case 3:
            PaletteObject = null;
            ResetMode();    // Unset object creation mode
            Update();
            break;
         }
         break;          // End of case: MOUSE_PRESSED

       default: return;
      }  
   }

   ///////////////////////////////////////////////////////////////////
   // Polygon object creation handler: creates a polygon with the mouse.
   ///////////////////////////////////////////////////////////////////
   void CreatePolygonHandler( int event_type, int button,
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
               // First click: polygon object hasn't been created yet. 
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
   // Create a new polygon object with 2 control points. Initialize 
   // both points to the given point position in screen coordinates. 
   ///////////////////////////////////////////////////////////////////
   GlgObject CreatePolygon( GlgPoint scr_point )
   {
      GlgObject poly_obj = new GlgPolygon( 2, null );
      
      // Set EdgeColor to black.
      poly_obj.SetGResource( "EdgeColor", 0.0, 0.0, 0.0 );

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
      DrawingArea.ScreenToWorld( true, scr_point, wp1 );

      // Set point coordinates, use null as the resource name.
      point.SetGResource( null, wp1.x, wp1.y, 0. );
   }

   ///////////////////////////////////////////////////////////////////
   // Returns the center of the object's bounding box.
   ///////////////////////////////////////////////////////////////////
   void GetBoxCenter( GlgObject object, GlgPoint box_center )
   { 
      // Get object center in screen coords.
      GlgCube box = object.GetBox();

      box_center.x = ( box.p1.x + box.p2.x ) / 2.;
      box_center.y = ( box.p1.y + box.p2.y ) / 2.;
      box_center.z = 0.;   // Ignore z
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

         // Obtain and store an objectID of the selected object at the cursor
         // position. The selected object should be an object at the top
         // level of the object hierarchy. Since cursor position is translated 
         // in Trace callback relatively to the DrawingArea, we can pass
         // DrawingArea to SelectTopLevelObject both as top viewport and
         // event viewport.
         GlgObject selected_object = 
            SelectTopLevelObject( DrawingArea, DrawingArea, cursor_position );

         if( selected_object == null || selected_object == HighlightRectangle )
            return;

         // If an object was selected, prepare to move it.

         // Store selected object.
         SetSelectedObject( selected_object );
         DrawingArea.Update();   // show selection

         // Store cursor position
         LastPosition.CopyFrom( cursor_position );
         
         // Switch to MOVE mode
         Mode = MOVE;
         break;

       case MouseEvent.MOUSE_MOVED:    // Drag the object
       case MouseEvent.MOUSE_DRAGGED:
         if( Mode != MOVE || SelectedObject == null )
            return; 

         // Move an object.
         SelectedObject.MoveObject( GlgObject.SCREEN_COORD,
                                    LastPosition, cursor_position );

         SelectedObject.SetupHierarchy(); 
         PositionHighlightRectangle( SelectedObject );

         // Store cursor position for the next move
         LastPosition.CopyFrom( cursor_position );

         Update();
         break;
            
       case MouseEvent.MOUSE_RELEASED:    // Stop moving
         ResetMode();
         break;

       default: return;
      }  
   }

   ///////////////////////////////////////////////////////////////////
   GlgObject SelectTopLevelObject( GlgObject viewport, 
                                  GlgObject event_viewport,
                                  GlgPoint cursor_position )
   {
      // Select all objects in the vicinity of the +-SELECTION_RESOLUTION 
        // pixels from the actual mouse click position.
      SelectRectangle.p1.x = cursor_position.x - SELECTION_RESOLUTION;
      SelectRectangle.p1.y = cursor_position.y - SELECTION_RESOLUTION;
      SelectRectangle.p2.x = cursor_position.x + SELECTION_RESOLUTION;
      SelectRectangle.p2.y = cursor_position.y + SELECTION_RESOLUTION;

      // Returns an array of selected objects' IDs on the bottom of the object
        // hierarchy.
      GlgObject selected_array =
        GlgObject.CreateSelection( viewport, SelectRectangle, event_viewport );

      if( selected_array == null )
        return null;

      // We'll take the first selected object (the object drawn on top).
      GlgObject selected_object = (GlgObject) selected_array.GetElement( 0 );

      // Obtain an ID of the object's parent which is a direct child of the
        // DrawingArea viewport.
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
        
   ////////////////////////////////////////////////////////////////////////
   void ResetMode()
   {
      // Reset mode back to SELECT
      Mode = SELECT;
   }

   ////////////////////////////////////////////////////////////////////////
   // Assign selected object.
   void SetSelectedObject( GlgObject object )
   {
      if( SelectedObject == object )
         return;

      // Remove highlight from the currently selected object (if any) and
      // highlight the new one. 
      HighlightObject( object );
      
      SelectedObject = object;
   }

   ////////////////////////////////////////////////////////////////////////
   // Highlight/Unhighlight object.
   void HighlightObject( GlgObject object )
   {
      // Create HighlightRectangle the first time if it doesn't exist.
      if( HighlightRectangle == null )
      {
         // Create a polygon object with 4 points.
         HighlightRectangle = new GlgPolygon( 4, null );
                                 
         // Set various attributes.
         HighlightRectangle.SetDResource( "FillType", 1. ); //Edge
         HighlightRectangle.SetDResource( "OpenType", 0. ); //Closed
         HighlightRectangle.SetGResource( "EdgeColor", 1., 0., 0. );
         HighlightRectangle.SetDResource( "LineWidth", 1. );
         HighlightRectangle.SetSResource( "Name", "HighlightRectangle" );
      }

      // Unhighlight object. 
      if( object == null )
      {
         // If there was a selected object, remove highlight rectangle 
         // from the drawing.
         if( SelectedObject != null )
           DrawingArea.DeleteObject( HighlightRectangle );
         return;
      }
   
      // Position HighlightRectangle.
      PositionHighlightRectangle( object );

      // If there was no previous selection, HighlightRectangle was not added to
      // the drawing, add it now.
      if( SelectedObject == null )
         DrawingArea.AddObjectToBottom( HighlightRectangle );
   }

   ////////////////////////////////////////////////////////////////////////
   // Position HighlightRectangle to highlight the bounding box of the
   // selected object.
   void PositionHighlightRectangle( GlgObject selected_object )
   {
      int PADDING = 8;
      
      // Retrieve bounding box of the object in screen coordinates.
      GlgCube box = selected_object.GetBox();
   
      // Convert screen coordinates of the bounding box to world coordinates
      // of the DrawingArea.
      DrawingArea.ScreenToWorld( true, box.p1, wp1 );
      DrawingArea.ScreenToWorld( true, box.p2, wp2 );

      // Set point coordinates of the HighlightRectangle.
      GlgObject point = (GlgObject) HighlightRectangle.GetElement(0);
      point.SetGResource( null, wp1.x - PADDING, wp1.y + PADDING, 0. );

      point = (GlgObject) HighlightRectangle.GetElement(1);
      point.SetGResource( null, wp1.x - PADDING, wp2.y - PADDING, 0.0 );

      point = (GlgObject) HighlightRectangle.GetElement(2);
      point.SetGResource( null, wp2.x + PADDING , wp2.y - PADDING, 0.0 );

      point = (GlgObject) HighlightRectangle.GetElement(3);
      point.SetGResource( null, wp2.x + PADDING, wp1.y + PADDING, 0.0 );
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
