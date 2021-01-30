import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;
import com.genlogic.GraphLayout.*;

//////////////////////////////////////////////////////////////////////////
public class GlgGraphLayoutDemo extends GlgJBean implements ActionListener
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   // Selection sensitivity in pixels
   static final int SELECTION_RESOLUTION = 2; 

   GlgGraphLayout graph;
   GlgObject drawing;
   GlgObject graph_viewport;

   int NumNodes;
   int NumNodeTypes;

   boolean IsReady;
   boolean Untangle;
   boolean Star;

   GlgObject SelectedNode;
   GlgGraphNode SelectedGraphNode;
   static GlgObject last_color = null;
   static GlgObject   stored_color = null;
   GlgPoint
     screen_point = new GlgPoint(),
     world_point = new GlgPoint();

   Timer timer = null;

   int INTERVAL = 30;

   //////////////////////////////////////////////////////////////////////////
   public GlgGraphLayoutDemo()
   {
      NumNodes = 20;
      NumNodeTypes = 2;
      Untangle = true;
      Star = false;
      IsReady = false;

      // Activate trace callback.
      AddListener( GlgObject.TRACE_CB, this );

      // Disable not used old-style select callback.
      SelectEnabled = false;
   }

   //////////////////////////////////////////////////////////////////////////
   // For use as a stand-alone java demo
   //////////////////////////////////////////////////////////////////////////
   public static void main( final String [] arg )
   {
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Main( arg ); } } );
   }

   //////////////////////////////////////////////////////////////////////////
   public static void Main( final String [] arg )
   {
      class DemoQuit extends WindowAdapter
      {
         public void windowClosing( WindowEvent e ) { System.exit( 0 ); }
      } 

      GlgObject.Init();

      JFrame frame = new JFrame( "GLG Graph Layout Demo" );

      GlgGraphLayoutDemo layout = new GlgGraphLayoutDemo();
      
      frame.setResizable( true );
      frame.setSize( 800, 800 );
      frame.setLocation( 20, 20 );

      frame.getContentPane().add( layout );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );

      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers ReadyCallback which starts updates.
      //
      layout.SetDrawingName( "graph.g" );
   }

   //////////////////////////////////////////////////////////////////////////
   // Starts updates
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );

      drawing = GetViewport();

      graph_viewport = drawing.GetResourceObject( "Graph" );
      if( graph_viewport == null )
      {
         GlgGraphLayout.Error( "Can't find <Graph> viewport." );
         return;
      }

      // Check for a icon palette in the drawing file: "Graph/Viewport"
        // Instead, the palette can be loaded from a separate palette file.
      GlgObject palette = graph_viewport.GetResourceObject( "Palette" );

      // Delete palette from the drawing.
      graph_viewport.DeleteObject( palette );

      // Palettes may be set on per graph basis using GlgGraphSetPalette 
      // method. Here, setting the same palette for all graphs with 
      // GlgGraphSetDefPalette.
        //
      GlgGraphLayout.SetDefPalette( palette );

      // Set initial values of toggle buttons.
      drawing.SetDResource( "Untangle/OnState", Untangle ? 1.0 : 0.0 );
      drawing.SetDResource( "Star/OnState", Star ? 1.0 : 0 );
      drawing.SetDResource( "NumNodes/Value", (double) NumNodes );

      // A graph may also be created using GlgGraphCreate(), GlgGraphAddNode() 
      // and GlgGraphAddEdge() functions. See CreateGraph method below for an
      // example:
      //    graph = CreateGraph();
      //
      graph = 
        GlgGraphLayout.CreateRandom( NumNodes, NumNodeTypes, 
                                     Star ? GlgGraphLayout.STAR_GRAPH :
                                     GlgGraphLayout.CIRCULAR_GRAPH );

      graph.SetUntangle( Untangle );
      graph.update_rate = GetUpdateRate();

      graph.CreateGraphics( graph_viewport, null );

      SetIconSize();

      graph_viewport.Update();

      if( timer == null )
      {
         // Restart the timer after each update (instead of using repeats)
         // to avoid flooding the event queue with timer events on slow 
         // machines.
         timer = new Timer( INTERVAL, this );
         timer.setRepeats( false );
         timer.start();
      }

      IsReady = true;
   }

   //////////////////////////////////////////////////////////////////////////
   void UpdateGraphLayout()
   {
      if( timer == null )
        return;   // Prevents race conditions

      if( INTERVAL != 200 )   // Active mode, not idling
      {
         boolean finished = graph.SpringIterate();
         
         if( finished )
           INTERVAL = 200;   // Slow down updates in the idle mode
      }

      timer.setDelay( INTERVAL );
      timer.start();   // Restart the update timer
   }

   //////////////////////////////////////////////////////////////////////////
   public void HCallback( GlgObject viewport )
   {
      // Applet: disable the Quit button
      if( getAppletContext() != null )
        SetDResource( "Quit/Visibility", 0.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   public void StopUpdates()
   {
      if( timer != null )
      {
         timer.stop();
         timer = null;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // This callback is invoked when user interacts with input objects in GLG
   // drawing. 
   //////////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject viewport, GlgObject message_obj )
   {
      String
        origin,
        format,
        action;

      super.InputCallback( viewport, message_obj );

      origin = message_obj.GetSResource( "Origin" );
      format = message_obj.GetSResource( "Format" );
      action = message_obj.GetSResource( "Action" );

      if( format.equals( "Button" ) )
      {
         if( action.equals( "Activate" ) )
         {	 
            // Act based on the selected button.
            if( origin.equals( "New" ) )
            {
               INTERVAL = 200;  // Set idle mode to stop a layout in progress
               graph.Destroy();
               graph = 
                 GlgGraphLayout.CreateRandom( NumNodes, NumNodeTypes, 
                                              Star ? GlgGraphLayout.STAR_GRAPH :
                                              GlgGraphLayout.CIRCULAR_GRAPH );
               graph.CreateGraphics( graph_viewport, null );
               graph.SetUntangle( Untangle );
               graph.update_rate = GetUpdateRate();
               graph.Update();

               INTERVAL = 30;  // Set active mode
            }
            else if( origin.equals( "Scramble" ) )
            {
               graph.Scramble();
               graph.Update();
               
               INTERVAL = 30;  // Set active mode
            }
            else if( origin.equals( "Circular" ) )
            {
               graph.CircularLayout();
               graph.Update();
            }
            else if( origin.equals( "Quit" ) )
            {
               if( getAppletContext() == null )  // Stand-alone, not an applet
                 System.exit( 0 );
            }
         }
         else if( action.equals( "ValueChanged" ) )
         {
            if( origin.equals( "Untangle" ) )
            {
               Untangle = 
                 (drawing.GetDResource( "Untangle/OnState" ).intValue() == 1);
               graph.SetUntangle( Untangle );
            }
            else if( origin.equals( "Star" ) )
              // Will be used on the next New graph.
              Star = (drawing.GetDResource( "Star/OnState" ).intValue() == 1);
         }
      }
      else if( format.equals( "Slider" ) && action.equals( "ValueChanged" ) )
      {
         if( origin.equals( "UpdateRate" ) )
           graph.update_rate = GetUpdateRate();
         else if( origin.equals( "NumNodes" ) )
           NumNodes = GetNumNodes();
         else if( origin.equals( "IconSize" ) )
           SetIconSize();

      }
   }
   
// Allow fallthrough in TraceCallback.
@SuppressWarnings("fallthrough")

   ////////////////////////////////////////////////////////////////////////
   //  Implements moving the node with the mouse.
   ////////////////////////////////////////////////////////////////////////
   public void TraceCallback( GlgObject viewport, GlgTraceData trace_info )
   {
      GlgGraphNode node;   
      int
        i,
        x, y,
        num_selected;
      
      // Don't call super.TraceCallback(): it performs html link hot spotting,
      // We don't need it here, override it with custom functionality.

      // Use the graph_viewport's events only.
      if( !IsReady || trace_info.viewport != graph_viewport )
        return;

      int event_type = trace_info.event.getID();
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
         if( GetButton( trace_info.event ) != 1 )
           return;  // Use the left button clicks only.
         /* Intended fallthough */         
       case MouseEvent.MOUSE_DRAGGED:
         x = ((MouseEvent)trace_info.event).getX();
         y = ((MouseEvent)trace_info.event).getY();
         break;
            
       case MouseEvent.MOUSE_RELEASED:
         if( GetButton( trace_info.event ) == 1 )	   
           SelectNode( null );
         return;

       default: return;
      }      
      
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED: 
         // Select all object in the vicinity of the +-SELECTION_RESOLUTION 
         // pixels from the actual mouse click position.
         //
         GlgCube select_rect = new GlgCube();
         select_rect.p1.x = x - SELECTION_RESOLUTION;
         select_rect.p1.y = y - SELECTION_RESOLUTION;
         select_rect.p2.x = x + SELECTION_RESOLUTION;
         select_rect.p2.y = y + SELECTION_RESOLUTION;
         
         GlgObject selection = 
           GlgObject.CreateSelection( /** top vp **/ viewport, 
                                     select_rect,
                                     /** event vp **/ trace_info.viewport );

         if( selection != null && ( num_selected = selection.GetSize() ) != 0 )
         {
            // Some object were selected, process the selection.

            selection.SetStart();
            for( i=0; i < num_selected; ++i )
            {
               GlgObject sel_object = (GlgObject) selection.Iterate();
            
               // Find the node the selected object belongs to.
               sel_object = GetSelectedNode( sel_object );	 
               if( sel_object != null )
               {
                  SelectNode( sel_object );  // Sets SelectedNode 
                  return;
               }
            }
         }

         // No nodes were selected.
         SelectNode( null );    // Unselect.
         break;

       case MouseEvent.MOUSE_DRAGGED:
         if( SelectedNode == null )
           break;

         node = graph.FindNode( SelectedNode );
         if( node == null )
         {
            GlgGraphLayout.Error( "Can't find the node in the graph." );
            break;
         }

         if( graph.finished )
         {
            graph.IncreaseTemperature( false );
            INTERVAL = 30;   // Wake it up (set Active mode)
         }

         screen_point.x = x;
         screen_point.y = y;
         screen_point.z = 0;
         graph_viewport.ScreenToWorld( true, screen_point, world_point );
         graph.SetNodePosition( node, world_point.x, world_point.y, 0.0 );

         // If invoked in the middle of a graph layout, updates the nodes
         // with the latest layout results before updating the graphics. 
           //
         graph.Update();
         break;
      }
   }

   ////////////////////////////////////////////////////////////////////////
   // Some part of the node may be selected, find the node it belongs to.
   ////////////////////////////////////////////////////////////////////////
   GlgObject GetSelectedNode( GlgObject glg_object )
   {
      String name;

      while( glg_object != null )
      {
         name = glg_object.GetSResource( "Name" );
         if( name != null && name.startsWith("Node") )
           return glg_object;

         glg_object = glg_object.GetParent();
      }
      return null;
   }

   ////////////////////////////////////////////////////////////////////////
   void SelectNode( GlgObject node )
   {
      GlgGraphNode graph_node;

      if( node == SelectedNode )
        return;   // No change

      // Restore the color of previously selected node.
      if( last_color != null )  
      {
         last_color.SetResourceFromObject( null, stored_color );
         last_color = null;
      }

      SelectedNode = node;

      // Change color to highlight selected node.
      if( node != null )
      {
         last_color = node.GetResourceObject( "Group/HighlightColor" );
         if( last_color == null )
           GlgGraphLayout.Error( "Can't find Icon's highlight color " + 
                                "(Group/HighlightColor)." );

         // Store original color
         if( stored_color == null )   
           stored_color = last_color.CopyObject();     // First time
         else
           stored_color.SetResourceFromObject( null, last_color );

         // Set color to red to highlight selection.
         last_color.SetGResource( null, 1.0, 0.0, 0.0 );

         graph_node = graph.FindNode( node );
         if( graph_node == null )
         {
            GlgGraphLayout.Error( "Can't find the graph node." );
            return;
         }
         graph_node.anchor = true;
         SelectedGraphNode = graph_node;

         // Set idle mode while selected: speed up dragging with the mouse
         INTERVAL = 200;
      }
      else   // Unselecting
        if( SelectedGraphNode != null )
        {
           SelectedGraphNode.anchor = false;
           SelectedGraphNode = null;   
           INTERVAL = 30;  // Resume layout when unselected
        }

      graph_viewport.Update();
   }

   ////////////////////////////////////////////////////////////////////////
   int GetUpdateRate()
   {
      double update_rate, high;
      
      update_rate = drawing.GetDResource( "UpdateRate/Value" ).doubleValue();
      high = drawing.GetDResource( "UpdateRate/High" ).doubleValue();
      
      if( update_rate == high )
        return 1000000;    // Update just once when finished.
      else
        return (int) update_rate;
   }

   ////////////////////////////////////////////////////////////////////////
   int GetNumNodes()
   {
      return drawing.GetDResource( "NumNodes/Value" ).intValue();
   }

   ////////////////////////////////////////////////////////////////////////
   void SetIconSize()
   {
      double icon_size = 
        drawing.GetDResource( "IconSize/Value" ).doubleValue();

      graph_viewport.SetDResource( "Node%/Group/IconScale", icon_size );

      GlgObject text_obj = 
        graph_viewport.GetResourceObject( "Node0/Group/Label" );
   
      text_obj.ResetSizeConstraint();
      graph_viewport.Update();
   }

   ////////////////////////////////////////////////////////////////////////
   int GetButton( AWTEvent event )
   {
      if( ! ( event instanceof InputEvent ) )
        return 0;
      
      InputEvent input_event = (InputEvent) event;
      int modifiers = input_event.getModifiers();
      
      if( ( modifiers & InputEvent.BUTTON1_MASK ) != 0 )
        return 1;
      else if( ( modifiers & InputEvent.BUTTON2_MASK ) != 0 )
        return 2;
      else if( ( modifiers & InputEvent.BUTTON3_MASK ) != 0 )
        return 3;
      else
        return 0;
   }

   ////////////////////////////////////////////////////////////////////////
   // Example: creating a graph with 3 nodes (one of type 0 and two of type 1)
   // connected by edges in a circle.
   ////////////////////////////////////////////////////////////////////////
   GlgGraphLayout CreateGraph()
   {
     GlgGraphLayout graph;
     GlgGraphNode
       node1,
       node2,
       node3;
     GlgGraphEdge
       edge1,
       edge2,
       edge3;
     
     graph = new GlgGraphLayout();
     
     node1 = graph.AddNode( null, 0, null );
     node2 = graph.AddNode( null, 1, null );
     node3 = graph.AddNode( null, 1, null );

     edge1 = graph.AddEdge( node1, node2, null, 0, null );
     edge2 = graph.AddEdge( node2, node3, null, 0, null );
     edge3 = graph.AddEdge( node3, node1, null, 0, null );

     return graph;
  }


   //////////////////////////////////////////////////////////////////////////
   // ActionListener method to use the bean as update timer's ActionListener.
   //////////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateGraphLayout();
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked by the browser to stop the applet
   //////////////////////////////////////////////////////////////////////////
   public void stop()
   {
      StopUpdates();
      IsReady = false;
      super.stop();
   }
}
