//////////////////////////////////////////////////////////////////////////
// This demo uses GLG as a bean and may be used in a browser or
// stand-alone.
//////////////////////////////////////////////////////////////////////////
import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgNetworkDemo extends GlgJBean implements ActionListener
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   // Constants
   static final int
     MAX_SIZE = 10000,
     SELECT_NODE = 0,
     MOVE_NODE = 1,
     ADD_NODE = 2,
     ADD_LINK_FIRST_NODE = 3,
     ADD_LINK_SECOND_NODE = 4;
   
   static final int pdateSpeed = 100;    // ms

   static final double 
     BREAK_THRESHOLD = 0.995,
     FIX_THRESHOLD = 0.985,
     TRANSMISSION_START_THRESHOLD = 0.3,
     TRANSMISSION_STOP_THRESHOLD = 0.8;

   // Number of existing nodes, links and 3D towers.
   int
     NUM_LINKS = 16,
     NUM_NODES = 9,
     NUM_TOWERS = 9;

   GlgObject
     CopyNode = null,
     CopyLink = null,
     DraggedObject = null,
     FirstNode = null,
     SecondNode = null;
   int
     EditMode = SELECT_NODE,
     iteration_counter = 2;  // A counter used to generate labels.

   boolean
     TowersMode = false,
     GraphVisible = false;

   Timer timer = null;

   //////////////////////////////////////////////////////////////////////////
   public GlgNetworkDemo()
   {
      super();
   }

   //////////////////////////////////////////////////////////////////////////
   // Starts updates.
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );

      Initialize();

      if( timer == null )
      {
         // Restart the timer after each update (instead of using repeats)
         // to avoid flooding the event queue with timer events on slow 
         // machines.
         timer = new Timer( 100, this );
         timer.setRepeats( false );
         timer.start();
      }
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

      JFrame frame = new JFrame( "GLG Network Monitoring Demo" );

      frame.setResizable( true );
      frame.setSize( 800, 600 );
      frame.setLocation( 20, 20 );

      GlgNetworkDemo network = new GlgNetworkDemo();
      frame.getContentPane().add( network );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );

      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers ReadyCallback which starts updates.
      //
      network.SetDrawingName( "network.g" );
   }

   //////////////////////////////////////////////////////////////////////////
   // Sets initial values for network simulation
   //////////////////////////////////////////////////////////////////////////
   void Initialize()
   {
      GlgObject
        link,
        node;
      int
        i,
        link_index,
        node_index;
      double num_labels;

      SetPrompt( "" );

      // Use slider with a gradient fill in Java2.
      SetDResource( "Zoom/Visibility", 1.0 );
      SetDResource( "ZoomSlider/Visibility", 0.0 );

      // Initialize zoom slider position.
      SetDResource( "Zoom/ValueX", 0.0 );

      // Set initial visibilities of different objects.
      SetDResource( "LoadGraph/Visibility", 0.0 );
      SetDResource( "MapArea/LargeCities/Visibility", 1.0 );
      SetDResource( "MapArea/MediumCities/Visibility", 1.0 );
      SetDResource( "MapArea/TowerVisibility", 0.0 );

      // Get node and link templates from the drawing.
      CopyNode = GetResourceObject( null, "MapArea/Node0" );
      CopyLink = GetResourceObject( null, "MapArea/Link0" );

      for( i=0; i<NUM_LINKS; ++i )
      {
         link = GetIndexedObject( "MapArea/Link%", i );
         link.SetDResource( "OKStatus", 1.0 );
         link.SetDResource( "Blink", 0.0 );
      }

      for( i=0; i<NUM_NODES; ++i )
      {
         node = GetIndexedObject( "MapArea/Node%", i );
         node.SetDResource( "OKStatus", 1.0 );
         node.SetDResource( "Blink", 0.0 );
      }

      for( i=0; i<NUM_TOWERS; ++i )
      {
         node = GetIndexedObject( "MapArea/Transmission%", i );
         node.SetDResource( "ActiveState", 1.0 );
         node.SetDResource( "Blink", 0.0 );
      }

      // Fill initial scrolling labels history.
      num_labels = GetDResource( "ScrollArea/LabelSeries/Factor" );

      for( i=0; i < (int) ( num_labels / 3.0 ); ++i )
      {
         link_index = (int) ( GetData() * NUM_LINKS );
         node_index = (int) ( GetData() * NUM_NODES );
         ScrollLabels( link_index, "Link % is broken.", 0 );
         ScrollLabels( node_index, "Node % is broken.", 0 );
         ScrollLabels( link_index, "Link % is fixed.", 1 );
         ScrollLabels( node_index, "Node % is fixed.", 1 );
      }

      SelectNode();   // Sets initial editing mode.
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Iterates network simulation
   //////////////////////////////////////////////////////////////////////////
   void IterateNetwork()
   {
      GlgObject
        link,
        node;
      int i;
      double
        value,
        ok_status,
        active_state;

      if( timer == null )
        return;   // Prevents race conditions

      if( GraphVisible )
        UpdateGraph();

      for( i=0; i<NUM_LINKS; ++i )
      {
         value = GetData();

         link = GetIndexedObject( "MapArea/Link%", i );
         ok_status = link.GetDResource( "OKStatus" ).doubleValue();

         if( ok_status != 0.0 && value > BREAK_THRESHOLD ) // Is OK: break it.
         {
            ok_status = 0.0;
            link.SetDResource( "OKStatus", ok_status );  // Changes color.
            ScrollLabels( i, "Link % is broken.", 0 );
         }
         else if( ok_status == 0.0 && value > FIX_THRESHOLD ) // Is broken: fix.
         {
            ok_status = 1.0;
            link.SetDResource( "OKStatus", ok_status );  // Changes color.

            ToggleState( link, 1 );   // Reset blinking.
            ScrollLabels( i, "Link % is fixed.", 1 );
         }

         if( ok_status == 0.0 )   // Blinking for broken links.
           ToggleState( link, 0 );   // Blink
      }

      for( i=0; i<NUM_NODES; ++i )
      {
         value = GetData();
         
         node = GetIndexedObject( "MapArea/Node%", i );
         ok_status = node.GetDResource( "OKStatus" ).doubleValue();

         if( ok_status != 0.0 && value > BREAK_THRESHOLD )  // Is OK: break it.
         {
            ok_status = 0.0;
            node.SetDResource( "OKStatus", ok_status );  // Changes color.
            ScrollLabels( i, "Node % is broken.", 0 );
         }
         else if( ok_status == 0.0 && value > FIX_THRESHOLD ) // Is broken: fix.
         {
            ok_status = 1.0;
            node.SetDResource( "OKStatus", ok_status );  // Changes color.
            ToggleState( node, 1 );   // Reset blinking.
            ScrollLabels( i, "Node % is fixed.", 1 );
         }

         if( ok_status == 0.0 )   // Blinking for broken nodes.
         ToggleState( node, 0 );   // Blink
      }

      if( TowersMode ) // Show and blink transmission lines in Towers Mode.
      {
         for( i=0; i<NUM_TOWERS; ++i )
         {
            value = GetData();
         
            node = GetIndexedObject( "MapArea/Transmission%", i );
            active_state = node.GetDResource( "ActiveState" ).doubleValue();
         
            // Show a transmission line
            if( active_state == 0.0 && value > TRANSMISSION_START_THRESHOLD )
            {
               active_state = 1.0;
               // ActiveState is a custom object attached to the Transmission
               // line to keep its On/Off status. Setting it to non-zero value
               // starts blinking in the code below.
               node.SetDResource( "ActiveState", active_state ); 
            }
            else if( active_state !=0.0 && value > TRANSMISSION_STOP_THRESHOLD )
            {
               // Delete transmission line.
               active_state = 0.0;
               // Stops blinking.
               node.SetDResource( "ActiveState", active_state );
               ToggleState( node, 1 );   // Reset blinking.
            }

            if( active_state != 0.0 )    // Shown: blink it.
              ToggleState( node, 0 );
         }
      }

      Update();    // Display changes

      timer.start();   // Restart the update timer
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates graph with simulated data
   //////////////////////////////////////////////////////////////////////////
   void UpdateGraph()
   {
      // Push the next data value, let the graph handle scrolling.
      SetDResource( "LoadGraph/DataGroup/EntryPoint", 70.0 + GetData() * 30.0 );
       
      // Push the next label.
      SetSResource( "LoadGraph/XLabelGroup/EntryPoint",
                    Integer.toString( iteration_counter ) );
      ++iteration_counter;
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
   double ADJUST( double a )
   {
      return ( - 1000.0 + a * 2000.0 );
   }

   public void ToggleVisibility( String object_visibility_attr )
   {
      double visibility;

      if( GetResourceObject( null, object_visibility_attr ) != null )
      {
         visibility = GetDResource( object_visibility_attr );
 
         if( visibility != 0.0 )
           visibility = 0.0;
         else
           visibility = 1.0;
         SetDResource( object_visibility_attr, visibility );
         Update();
      }
   }

   //////////////////////////////////////////////////////////////////////////
   public void ToggleTowers()
   {
      TowersMode = !TowersMode;

      SetDResource( "MapArea/LargeCities/Visibility", TowersMode ? 0.0 : 1.0 );
      SetDResource( "MapArea/MediumCities/Visibility", TowersMode ? 0.0 : 1.0 );
      SetDResource( "MapArea/NodeGroup/Visibility", TowersMode ? 0.0 : 1.0 );
      SetDResource( "MapArea/TowerVisibility", TowersMode ? 1.0 : 0.0 );

      SetDResource( "MapArea/XAngle", TowersMode? -30.0 : 0.0 );
      SetDResource( "MapArea/ZAngle", TowersMode?  20.0 : 0.0 );

      // Reset zoom and pan.
      SetDResource( "MapArea/Scale", 1.0 );
      SetDResource( "MapArea/PanX", 0.0 );
      SetDResource( "MapArea/PanY", 0.0 );

      GraphVisible = false;
      SetDResource( "LoadGraph/Visibility", 0.0 );

      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   void ResetModes()
   {
      EditMode = SELECT_NODE;
      SetPrompt( "" );
   }

   //////////////////////////////////////////////////////////////////////////
   void SelectNode()
   {
      EditMode = SELECT_NODE;
      SetPrompt( "Click on the map to select a node." );
   }

   //////////////////////////////////////////////////////////////////////////
   void MoveNode()
   {
      EditMode = MOVE_NODE;
      SetPrompt( "Move nodes with the mouse." );
   }

   //////////////////////////////////////////////////////////////////////////
   void AddNode()
   {
      EditMode = ADD_NODE;
      SetPrompt( "Click on the map to position a new node." );
   }

   //////////////////////////////////////////////////////////////////////////
   void AddLink()
   {
      EditMode = ADD_LINK_FIRST_NODE;
      SetPrompt( "Select the first node." );
   }
   
   //////////////////////////////////////////////////////////////////////////
   void Print()
   {
      ResetModes();

      // Print just the network part of the drawing.
      GlgObject main_area = GetResourceObject( null, "MainArea" );      
      main_area.Print( "network.ps", -900.0, -900.0, 1800.0, 1800.0, true, false );

      SetPrompt( "PostScript file is saved in network.ps file. " );
   }

   //////////////////////////////////////////////////////////////////////////
   void SetPrompt( String message )
   {
      SetSResource( "Prompt/String", message );
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   void SetName( GlgObject glg_object, String name, int counter )
   {
      glg_object.SetSResource( "Name",
                               GlgObject.CreateIndexedName( name, counter ) );
   }

   //////////////////////////////////////////////////////////////////////////
   void ScrollLabels( int obj_index, String message, int color_index )
   {
      SetSResource( "ScrollArea/LabelSeries/LabelEntryPoint",
                   GlgObject.CreateIndexedName( message, obj_index ) );

      SetDResource( "ScrollArea/LabelSeries/ColorEntryPoint",
                   (double) color_index );
   }
   
   //////////////////////////////////////////////////////////////////////////
   void ConnectLink( GlgObject link )
   {
      GlgObject
        link_point,
        node_point;

      // Using default attr name for the point.

      link_point = (GlgObject) link.GetElement( 0 );
      node_point = FirstNode.GetResourceObject( "Point" );
      link_point.ConstrainObject( node_point );

      link_point = (GlgObject) link.GetElement( 1 );
      node_point = SecondNode.GetResourceObject( "Point" );
      link_point.ConstrainObject( node_point );
   }

   //////////////////////////////////////////////////////////////////////////
   // Toggles the blinking state of the object. The object should have a 
   // resource named "Blink", which will be flipped between 0 and 1.
   //
   // Nodes use a List transformation attached to the small arc's radius
   // attribute to change it. Links use a List transformation attached to 
   // their LineWidth attribute. Tower links use the Visibility attribute.
   //////////////////////////////////////////////////////////////////////////
   void ToggleState( GlgObject glg_object, int reset )
   {
      double blinking_state;
      
      blinking_state = glg_object.GetDResource( "Blink" ).doubleValue();

      if( reset != 0 )
        blinking_state = 0.0;     // Reset
      else       // Toggle it to blink
        blinking_state = ( blinking_state != 0.0 ? 0.0 : 1.0 );

      glg_object.SetDResource( "Blink", blinking_state );
   }

   //////////////////////////////////////////////////////////////////////////
   double GetData()
   {
      return GlgObject.Rand( 0.0, 1.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   // Handles selection on mouse clicks
   //////////////////////////////////////////////////////////////////////////
   public void SelectCallback( GlgObject vp, Object[] name_array, int button )
   {
      GlgObject
        glg_object,
        group;
      String name;
      int i;

      if( button != 1 || name_array == null )
        return;   // Only Button1 is used here

      for( i=0; ( name = (String) name_array[ i ] ) != null; ++i )
      {
         // A click on a graph: close it.
         if( GraphVisible && name.indexOf( "LoadGraph" ) != -1 )
         {
            GraphVisible = false;
            
            SetDResource( "LoadGraph/Visibility", 0.0 );
            Update();
            return;
         }

         if( name.indexOf( "MapArea/Node" ) != -1 )
         {
            int
              match_start, match_start1, match_end;
            String
              node_number_string,
              node_name;
            
            // Get the selected node number as a string
            match_start = name.indexOf( "MapArea/Node" );
            
            match_start1 = match_start + "MapArea/Node".length();
            
            match_end = name.indexOf( "/", match_start1 );
            if( match_end != -1 )
              node_number_string = name.substring(match_start1,match_end);
            else
              node_number_string = name.substring( match_start1 );
            node_name = name.substring(match_start,match_end);
            
            if( EditMode == SELECT_NODE )
            {
               // Node selected: popup the graph to show its load
               GraphVisible = true;
               SetDResource( "LoadGraph/Visibility", 1.0 );
               
               // Annotate the graph with the selected node number.
               SetSResource( "LoadGraph/Title/String",
                            "Node " + node_number_string + " Load" );
               Update();
               return;
            }
            else if( EditMode == ADD_LINK_FIRST_NODE )
            {
               FirstNode = GetResourceObject( null, node_name );
               
               EditMode = ADD_LINK_SECOND_NODE;
               SetPrompt( "Select the second node." );
               return;
            }
            else if( EditMode == ADD_LINK_SECOND_NODE )
            {
               glg_object = GetResourceObject( null, node_name );
               
               if( glg_object == FirstNode )
                 SetPrompt( "Two nodes are the same, select another node." );
               else // Add link
               {
                  SecondNode = glg_object;
                  
                  glg_object = CopyLink.CopyObject();
                  glg_object.SetDResource( "Visibility", 1.0 );
                  SetName( glg_object, "Link", NUM_LINKS );
                  
                  ConnectLink( glg_object );
                  // Add link to top to be behind the nodes.
                  group = GetResourceObject( null, "MapArea/NodeGroup" );
                  group.AddObjectToTop( glg_object );
                  Update();
                  ++NUM_LINKS;

                  EditMode = ADD_LINK_FIRST_NODE;
                  SetPrompt( "Select the first node." );
               }
               return;
            }
            else if( EditMode == MOVE_NODE )
            {
               DraggedObject = GetResourceObject( null, node_name );
               return;
            }
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Handles user input
   //////////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject vp, GlgObject message_obj )
   {
      String
        origin,
        format,
        action,
        subaction;
      double x, y;
      GlgObject group;

      super.InputCallback( vp, message_obj );

      origin = message_obj.GetSResource( "Origin" );
      format = message_obj.GetSResource( "Format" );
      action = message_obj.GetSResource( "Action" );
      subaction = message_obj.GetSResource( "SubAction" );

      if( format.equals( "Button" ) )
      {
         if( !action.equals( "Activate" ) )
           return;
         
         ResetModes();
         
         if( origin.equals( "SelectNode" ) )
           SelectNode();
         else if( origin.equals( "MoveNode" ) )
           MoveNode();
         else if( origin.equals( "AddNode" ) )
           AddNode();
         else if( origin.equals( "AddLink" ) )
           AddLink();
         else if( origin.equals( "LargeCities" ) )
           ToggleVisibility( "MapArea/LargeCities/Visibility" );
         
         else if( origin.equals( "MediumCities" ) )
           ToggleVisibility( "MapArea/MediumCities/Visibility" );
         
         else if( origin.equals( "MapBackground" ) )
           ToggleVisibility( "MapArea/MapBackground/Visibility" );
         
         else if( origin.equals( "Towers" ) )
           ToggleTowers();
         
         else if( origin.equals( "Print" ) )
           Print();
         
         else if( origin.equals( "Exit" ) )
           System.exit( 0 );
      }
      else if( format.equals( "Slider" ) )
      {
         if( !action.equals( "ValueChanged" ) )
           return;
         
         if( origin.equals( "Zoom" ) )   // Zoom slider
         {
            x = message_obj.GetDResource( "ValueX" ).doubleValue();
            SetDResource( "MapArea/Scale", 1.0 + 3.0 * x );
            Update();
         }
         else if( origin.equals( "MapArea" ) )
         {
            // Click in the network map area. It has a slider attached to
            // extract X and Y coordinates of the mouse click to move the
            // selected nodes with the mouse and to position new nodes.
            x = message_obj.GetDResource( "ValueX" ).doubleValue();
            y = message_obj.GetDResource( "ValueY" ).doubleValue();

            if( subaction.equals( "Click" ) )
            {
               if( EditMode == ADD_NODE )
               {
                  GlgObject glg_object;
                  
                  glg_object = CopyNode.CopyObject();
                  glg_object.SetDResource( "Visibility", 1.0 );
                  glg_object.SetGResource( "Point", 
                                           ADJUST( x ), ADJUST( y ), 0.0 );
                  SetName( glg_object, "Node", NUM_NODES );

                  // Add to the bottom to be in front of links.
                  group = GetResourceObject( null, "MapArea/NodeGroup" );
                  group.AddObjectToBottom( glg_object );
                  Update();
                  ++NUM_NODES;
               }
            }
            else if( subaction.equals( "Motion" ) )
            {
               if( EditMode != MOVE_NODE ||
                  DraggedObject == null )
                 return;
               
               // Move node with the mouse
               DraggedObject.SetGResource( "Point", 
                                          ADJUST( x ), ADJUST( y ), 0.0 );
               Update();
            }
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   GlgObject GetIndexedObject( String name, int obj_index )
   {
      return 
        GetResourceObject( null,
                          GlgObject.CreateIndexedName( name, obj_index ) );
   }

   //////////////////////////////////////////////////////////////////////////
   // ActionListener method to use the bean as update timer's ActionListener.
   //////////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      IterateNetwork();
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked by the browser to stop the applet
   //////////////////////////////////////////////////////////////////////////
   public void stop()
   {
      StopUpdates();
      super.stop();
   }

   //////////////////////////////////////////////////////////////////////////
   // Inner class for a Runnable interface.
   // Provides an interface between JavaScript and Java: invokes applet's 
   // methods in a synchronous way as required by Swing.
   //////////////////////////////////////////////////////////////////////////
   class GlgBeanRunnable implements Runnable
   {
      GlgNetworkDemo bean;
      String request_name;
      String value;

      public GlgBeanRunnable( GlgNetworkDemo bean_p, 
                             String request_name_p, String value_p )
      {
         bean = bean_p;
         request_name = request_name_p;
         value = value_p;
      }

      public void run()
      {
         if( request_name.equals( "ToggleVisibility" ) )
           bean.ToggleVisibility( value );
         else if( request_name.equals( "ToggleTowers" ) )
           bean.ToggleTowers();
         else
           PrintToJavaConsole( "Invalid request name: " + 
                              request_name + "\n" );
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Provides an interface between JavaScript and Java: invokes applet's 
   // methods in a synchronous way as required by Swing.
   //////////////////////////////////////////////////////////////////////////
   public void SendRequestString( String request_name, String value )
   {
      GlgBeanRunnable runnable = 
        new GlgBeanRunnable( this, request_name, value );

      SwingUtilities.invokeLater( runnable );
   }
}
