//////////////////////////////////////////////////////////////////////////
// A GIS demo with a map displayed using the GLG Map Server.
// This demo uses GLG as a bean and may be used in a browser or stand-alone.
//
// This demo uses the GLG Map Server to display a map.
// The Map Server has to be installed either on the local host or on a
// remote web server. After the Map Server has been installed, enable
// the map by modifying the source code to comment out the statement that 
// sets the GISDisable resource, set SuppliedMapServerURL to point to 
// the Map Server location and rebuild the demo.
//////////////////////////////////////////////////////////////////////////
import java.awt.*;
import java.awt.event.*;
import java.lang.reflect.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgGISDemo extends GlgJBean implements ActionListener
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   static final double PlaneSpeed = 0.0005;    // Relative units
   static final double MaxZoomSpeed = 5.0;     // In XY Coordinates

   static final int
     UpdateInterval = 30,  // Update interval in milliseconds.
     USZoomDelay1 = 3000,  // Delay to zoom to the US area to show details.
     USZoomDelay2 = 1000;  // Delay to remove US zooming message.

   // If supplied, overrides the URL of the GIS object in the drawing
   static String SuppliedMapServerURL = null; 

   GlgObject
     Drawing,
     PositionObject,
     PositionArea;
   GlgObject [] NodeTemplate = new GlgObject[ 2 ];
   GlgObject [] PlaneTemplate = new GlgObject[ 2 ];
     GlgObject [] Map = new GlgObject[ 2 ];
     GlgObject [] GISObject = new GlgObject[ 2 ];
     GlgObject [] NodeGroup = new GlgObject[ 2 ];
     GlgObject [] PlaneGroup = new GlgObject[ 2 ];

   static final double          // Plane size constants
     SMALL_SIZE  = 0.23,
     MEDIUM_SIZE = 0.33,
     BIG_SIZE =    0.43;

   double PlaneSize = MEDIUM_SIZE;
   
   int
     NumNodes,
     NumPlanes = 10;

   int [] MapProjection = new int[ 2 ];

   /* If true, pan the map to make the selected plane visible in the current
      zoomed area.
   */
   boolean LockSelectedPlane;

   boolean 
     CityLabels = true,
     StateDisplay = true,
     SuspendPromptUpdates = false;

   // Store initial extent and center, used to reset
   GlgPoint [] InitExtent = new GlgPoint[ 2 ];
   GlgPoint [] InitCenter = new GlgPoint[ 2 ];

   GlgPoint 
     // Temp vars: allocate once
     lat_lon = new GlgPoint(),
     position = new GlgPoint(),
     old_position = new GlgPoint(),
     new_position = new GlgPoint(),
     last_xyz = new GlgPoint(),
     curr_xyz = new GlgPoint(),
     point = new GlgPoint(),
     util_point = new GlgPoint();
   GlgCube selection_rect = new GlgCube();

   static boolean StandAlone = false;
   boolean MapIsReady = false;
   boolean TopMapIsReady = false;
   boolean PanMode = false;

   Timer timer = null;
   Timer zoom_timer = null;
   boolean PerformUpdates = true;

   PlaneData [] PlaneArray;
   PlaneData SelectedPlane;

   // Array of icons to place on the map as GLG objects in addition to the 
   // icons defined in GIS server's data. The icons that use GLG objects may
   // be selected with the mouse and their attributes can be changed 
   // dynamically, based on data. When the mouse moves over an icon, it may 
   // be highlighted with a different color or a tooltip may be displayed.
   //
   NodeData [] NodeArray =
   {
      new NodeData( "Boston",        -71.01789,  42.33602 ),
      new NodeData( "New York",      -73.97213,  40.77436 ),
      new NodeData( "San Francisco", -122.55478, 37.79325 ),
      new NodeData( "Miami",         -80.21084,  25.77566 ),
      new NodeData( "Seattle",       -122.35032, 47.62180 ),
      new NodeData( "Houston",       -95.38672,  29.76870 ),
      new NodeData( "Denver",        -104.87265, 39.76803 ),
      new NodeData( "Minneapolis",   -93.26684,  44.96185 ),
      new NodeData( "Chicago",       -87.68496,  41.83705 ),
      new NodeData( "Dallas",        -96.76524,  32.79415 )
   };

   //////////////////////////////////////////////////////////////////////////
   public GlgGISDemo()
   {
      super();
      SetDResource( "$config/GlgMouseTooltipTimeout", 0.05 );

      // Don't expand selection area for exact tooltips.
      SetDResource( "$config/GlgPickResolution", 0.0 );

      // Activate Trace callback.
      AddListener( GlgObject.TRACE_CB, this );

      // Disable not used old-style select callback.
      SelectEnabled = false;
   }

   //////////////////////////////////////////////////////////////////////////
   // main() method for using as a stand-alone java demo
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

      // Map server URL from the command line, if supplied.
      if( Array.getLength( arg ) != 0 )
         GlgGISDemo.SuppliedMapServerURL = arg[ 0 ]; 

      JFrame frame = new JFrame( "GLG GIS Demo" );

      frame.setResizable( true );
      frame.setSize( 800, 650 );
      frame.setLocation( 20, 20 );

      GlgGISDemo.StandAlone = true;
      GlgGISDemo map_demo = new GlgGISDemo();

      // Use getContentPane() for GlgJBean
      frame.getContentPane().add( map_demo );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );
      
      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers ReadyCallback which starts updates.
      //
      map_demo.SetDrawingName( "gis_demo.g" );
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked before the hierarchy setup.
   //////////////////////////////////////////////////////////////////////////
   public void HCallback( GlgObject viewport )
   {
      Drawing = viewport;
      Map[ 0 ] = Drawing.GetResourceObject( "TopMap" );   // Thumbnail map
      Map[ 1 ] = Drawing.GetResourceObject( "Map" );      // Detailed map
      
      // Display thumbnail map in a separate window. It is kept as a child
        // window in the drawing for the convinience of editing.
      Map[ 0 ].SetDResource( "ShellType", (double) GlgObject.DIALOG_SHELL );

      if( !StandAlone )
      {
         String param = getParameter( "MapServerURL" );
         if( param != null )
           SuppliedMapServerURL = param;
      }

      Init();
   }

   //////////////////////////////////////////////////////////////////////////
   // Initializes icons in the drawing
   //////////////////////////////////////////////////////////////////////////
   void Init()
   {
      GlgObject resource;
      int i;

      for( i=0; i<2; ++i )   // For each map window
        // Get IDs of the GIS objects in each of the map viewports. 
        GISObject[ i ] = Map[ i ].GetResourceObject( "GISObject" );

      ////////////////////////////////////////////////////////////////////////
      // Map Server Setup Info
      ////////////////////////////////////////////////////////////////////////
      String map_server_info = 
        "This demo uses the GLG Map Server to display a map.\n" +
        "The Map Server has to be installed either on the " +
        "local host or on a remote web server.\n\n" +
        "After the Map Server has been installed, enable the map by " +
        "modifying the source code to comment out the statement that sets " +
        "the GISDisable resource (the next statement after this message), " +
        "set SuppliedMapServerURL to point to the Map Server location and " +
        "rebuild the demo.";

      System.out.println( map_server_info );

      // Comment out the next three lines to enable the map.
      for( i=0; i<2; ++i )   // For each map window
        GISObject[i].SetDResource( "GISDisabled", 1.0 );

      Map[1].SetDResource( "JavaSetupInfo/Visibility", 1.0 );

      ////////////////////////////////////////////////////////////////////////

      // Try to use the GLG map server on the localhost if no URL was supplied.
      if( SuppliedMapServerURL == null )
        SuppliedMapServerURL = "http://localhost/cgi-bin/GlmScript";

      for( i=0; i<2; ++i )   // For each map window
      {
         if( SuppliedMapServerURL != null )
           // Override the URL defined in the drawing.
           GISObject[ i ].SetSResource( "GISMapServerURL", 
                                        SuppliedMapServerURL );

         /* Query and store the GIS projection (ORTHOGRAPHIC or RECTANGULAR)
            used to render the map.
         */
         MapProjection[ i ] = 
           GISObject[ i ].GetDResource( "GISProjection" ).intValue();

         /* Set the GIS Zoom mode. It was set and saved with the drawing, 
            but do it again programmatically just in case.
         */
         Map[ i ].SetZoomMode( null, GISObject[ i ], null, 
                               GlgObject.GIS_ZOOM_MODE );

         // Store initial map extent for resetting after zooming.
         InitExtent[i] = GISObject[ i ].GetGResource( "GISExtent" );
         InitCenter[i] = GISObject[ i ].GetGResource( "GISCenter" );
      }

      // Get the palette containing templates for plane and node icons.
      GlgObject palette = Drawing.GetResourceObject( "Palette" );

      // Delete it from the drawing
      Drawing.DeleteObject( palette );

      /* Get node and plane templates from the palette. Two sets of templates
         are used: smaller icons for the thumbnail view and more elaborate 
         ones for the detailed map.
      */
      for( i=0; i<2; ++i )
      {
         NodeTemplate[ i ] = 
           palette.GetResourceObject( i == 0 ? "Node1" : "Node2" );

         PlaneTemplate[ i ] = 
           palette.GetResourceObject( i == 0 ? "Plane1" : "Plane2" );

         /* If the icon is not a marker (IconScale resource exists), set the
            icon's size.
         */
         resource = PlaneTemplate[ i ].GetResourceObject( "IconScale" );
         if( resource != null )
           resource.SetDResource( null, PlaneSize );
      }

      NumNodes = NodeArray.length;

      // Create and initialize plane structures used for simulation.
      PlaneArray = new PlaneData[ NumPlanes ];
      for( i =0; i < NumPlanes; ++i )
      {
         PlaneArray[ i ] = new PlaneData();
         PlaneArray[ i ].name = Integer.toString( i );
      }
      
      // Add node and plane icons to both thumbnail and detailed map.
      for( i=0; i<2; ++i )
      {	 
         CreateAirportIcons( i ); // Add airport icons
         CreatePlaneIcons( i );   // Add plane icons
      }

      // Start all planes.
      for( i=0; i < NumPlanes; ++i )
        StartPlane( PlaneArray[ i ], true );

      /* Selected area annotates the currently viewed area of the detailed map 
         in the thumbnail map view. Reorder SelectedArea on top of icons
         (last in the array).
      */
      GlgObject selected_area = 
        GISObject[ 0 ].GetResourceObject( "GISArray/SelectedArea" );
      GISObject[ 0 ].ReorderElement( GISObject[ 0 ].GetIndex( selected_area ),
                                    GISObject[ 0 ].GetSize() - 1 );

      /* Set state display on the thumbnail map. Airport labels on the detailed
         map are handled by HandleZoomLevel(). 
      */
      SetGISLayers( 0 );

      /* Demos starts with the whole word view, then zooms to the US area
         in a few seconds to show more details. Set initial parameters
         for the whole world view.
      */
      HandleZoomLevel();

      InitSelection();  // Select some plane.

      // Store objects used to display lat/lon on mouse move.
      PositionArea = Drawing.GetResourceObject( "PositionArea" );
      PositionObject = 
        Drawing.GetResourceObject( "PositionLabel/PointerLatLon" );
      PositionObject.SetSResource( null, "" );

      // Set US zooming message to OFF initially.
      Drawing.SetDResource( "Map/USZoomingMessage/Visibility", 0.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked after the hierarchy setup.
   //////////////////////////////////////////////////////////////////////////
   public void VCallback( GlgObject viewport )
   {
      /* Adjust selected region on the thumbnail map to match the zoomed area 
         of the detailed map.
      */
      SetSelectedArea();
   }

   //////////////////////////////////////////////////////////////////////////
   // Initializes the drawing and starts updates.
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );

      StartUpdates();

      // Zoom to the US area after a few seconds to show details.        
      zoom_timer = new Timer( USZoomDelay1, new ZoomPerformer( this ) );
      zoom_timer.setRepeats( false );
      zoom_timer.start();
   }

   //////////////////////////////////////////////////////////////////////////
   // Creates NodeGroup to hold airport icons and adds the icons to it.
   //////////////////////////////////////////////////////////////////////////
   void CreateAirportIcons( int map )
   {
      NodeGroup[ map ] = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
      NodeGroup[ map ].SetSResource( "Name", "NodeGroup" );

      // Add city/node icons
      for( int i = 0; i < NumNodes; ++i )
        AddNode( NodeArray[ i ], map, i );

      /* Add the group to the GIS Object which will automatically manage 
         the GIS coordinate conversion. This allows to specify node 
         positions in lat/lon instead of the X/Y world coordinates.
      */
      GISObject[ map ].AddObjectToBottom( NodeGroup[ map ] );
   }

   //////////////////////////////////////////////////////////////////////////
   // Creates PlaneGroup to hold plane icons and adds the icons to it.
   //////////////////////////////////////////////////////////////////////////
   void CreatePlaneIcons( int map )
   {
      PlaneGroup[ map ] = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
      PlaneGroup[ map ].SetSResource( "Name", "PlaneGroup" );

      // Add plane icons
      for( int i=0; i < NumPlanes; ++i )
        AddPlane( PlaneArray[ i ], map, i );
    
      /* Add the group to the GIS Object which will automatically manage 
         the GIS coordinate conversion. This allows to specify plane 
         positions in lat/lon instead of the X/Y world coordinates.
      */
      GISObject[ map ].AddObjectToBottom( PlaneGroup[ map ] );
   }

   //////////////////////////////////////////////////////////////////////////
   void AddNode( NodeData node_data, int map, int index )
   {      
      // Create a copy of a node.
      GlgObject node = 
        NodeTemplate[ map ].CloneObject( GlgObject.STRONG_CLONE );
      
      node.SetSResource( "Name", node_data.name );  // Set object name

      /* Set node position in lat/lon coordinates. The GIS object will handle
         the GIS coordinate conversion depending on the map the node icon is 
         displayed in, as well as the map's zoom and pan state.
      */
      node.SetGResource( "Position", 
                        node_data.lat_lon.x, node_data.lat_lon.y, 0.0 );

      // Index for direct access
      node.SetDResource( "DataIndex", (double)index );

      if( map == 1 )        // On the detailed map, show node name label.
        node.SetSResource( "LabelString", node_data.name );
      
      String tooltip;
      if( map == 0 )   // On the thumbnail map, show node name in the tooltip.
        tooltip = node_data.name;
      else   // On the detailed map, include lat/lon into the tooltip.
        tooltip = 
          node_data.name + ", " + CreateLocationString( node_data.lat_lon );
      node.SetSResource( "TooltipString", tooltip );
        
      node_data.graphics[ map ] = node;

      // Add the node to the requested map (thumbnail or detailed).
      NodeGroup[ map ].AddObjectToBottom( node );
   }

   //////////////////////////////////////////////////////////////////////////
   // Adds a plane icon, fills labels, tooltips, etc.
   //////////////////////////////////////////////////////////////////////////
   void AddPlane( PlaneData plane_data, int map, int index )
   {
      // Create a copy of a node.
      GlgObject plane =
        PlaneTemplate[ map ].CloneObject( GlgObject.STRONG_CLONE );

      plane.SetSResource( "Name", plane_data.name );      // Object name

      // Index for direct access
      plane.SetDResource( "DataIndex", (double)index );

      plane_data.graphics[ map ] = plane;

      // Check if the icon has an angle to indicate its direction.
      if( plane.GetResourceObject( "Angle" ) != null )
        plane_data.has_angle[ map ] = true;

      /* The plane will be positioned with PositionPlane() after the GIS object
         have been setup and the plane's lat/lon has been calculated by the 
         flight simulation. */

      // Add the plane to the requested map (thumbnail or detailed).
      PlaneGroup[ map ].AddObjectToBottom( plane );
   }

   //////////////////////////////////////////////////////////////////////////
   void PositionPlane( PlaneData plane, int map )
   {
      if( plane.graphics[ map ] == null || 
         plane.from_node == null || plane.to_node == null )
        return;

      // Obtain the plane's current position.
      GetPlaneLatLon( plane );

      /* Update plane's icon in the drawing by setting its lat/lon coordinates.
         The GIS object will handle the GIS coordinate conversion depending on 
         the map the plane icon is displayed in, as well as the map's zoom 
         and pan state.
      */
      plane.graphics[ map ].
        SetGResource( "Position", plane.lat_lon.x, plane.lat_lon.y, 0.0 );

      // Update icon's direction angle is necessary
      if( plane.has_angle[ map ] )
      {
         double angle = GetPlaneAngle( plane, map );
         plane.graphics[ map ].SetDResource( "Angle", angle );
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // FOR FLIGHT SIMULATION ONLY: Converts plane lat/lon to the GLG world 
   //     coordinates for calculating plane speed and directional angle.
   //////////////////////////////////////////////////////////////////////////
   void GetPlanePosition( PlaneData plane, int map, GlgPoint xyz )
   {
      // Converts lat/lon to X/Y using GIS object's current projection.
      GISObject[ map ].
        GISConvert( null, GlgObject.OBJECT_COORD, 
                   /* Lat/Lon to XY */ false, plane.lat_lon, xyz );
   }

   //////////////////////////////////////////////////////////////////////////
   // FOR FLIGHT SIMULATION ONLY: Converts node lat/lon to the GLG world 
   //     coordinates for calculating plane's initial directional angle.
   //////////////////////////////////////////////////////////////////////////
   void GetNodePosition( NodeData node, int map, GlgPoint xyz )
   {
      // Converts lat/lon to X/Y using GIS object's current projection.
      GISObject[ map ].
        GISConvert( null, GlgObject.OBJECT_COORD, 
                   /* Lat/Lon to XY */ false, node.lat_lon, xyz );
   }

   //////////////////////////////////////////////////////////////////////////
   // FOR FLIGHT SIMULATION ONLY: Calculates plane icon's directional angle.
   //     In an application, it will query the plane's directional angle.
   //////////////////////////////////////////////////////////////////////////
   double GetPlaneAngle( PlaneData plane, int map )
   {
      /* Rectangular projection preserves straight lines, we can use the 
         angle of the line connecting the start and end nodes. For the
         orthographic projection, use this case if the plane has just started
         and there is no previous position stored.
      */
      if( MapProjection[ map ] == GlgObject.RECTANGULAR_PROJECTION ||
         plane.path_position == plane.path_position_last )   // Just started
      {
         GetNodePosition( plane.from_node, map, last_xyz );
         GetNodePosition( plane.to_node, map, curr_xyz );
      }
      else  /* In the orthographic projection straight lines are drawn as 
               curves. Use the angle of the line connecting the current and 
               last position of the plane. */
      {
         double stored_position;
      
         stored_position = plane.path_position;    // Store current position.
      
         // Get coordinates of the plane's previous position
         plane.path_position = plane.path_position_last;
         GetPlaneLatLon( plane );
         GetPlanePosition( plane, map, last_xyz );
      
         // Restore the plane's current position and get its coordinates.
         plane.path_position = stored_position;
         GetPlaneLatLon( plane );
         GetPlanePosition( plane, map, curr_xyz );
      }

      /* Calculate the angle of a line connecting the previous and 
         current position.
      */
      return GetAngle( last_xyz, curr_xyz );
   }

   //////////////////////////////////////////////////////////////////////////
   // Checks if the object is visible in the current zoom region.
   // This prevents wrap-around errors under big zoom factors.
   //////////////////////////////////////////////////////////////////////////
   boolean GetVisibility( GlgPoint position, double adj )
   {
      // Use adj as a gap
      return
        position.x > -1000.0 * adj && position.x < 1000.0 * adj &&
        position.y > -1000.0 * adj && position.y < 1000.0 * adj;
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates plane positions on both maps.
   //////////////////////////////////////////////////////////////////////////
   void UpdatePlanes()
   {
      if( timer == null )
        return;   // Prevents race conditions

      if( PerformUpdates && MapIsReady && TopMapIsReady )
      {
         for( int i = 0; i < NumPlanes; ++i )
           UpdatePlane( PlaneArray[ i ] );
         
         UpdateSelectedPlaneStatus();

         if( LockSelectedPlane )
           UpdateLocking();
         
         Update();
      }

      timer.start();   // Restart the update timer
   }

   //////////////////////////////////////////////////////////////////////////
   // FOR FLIGHT SIMULATION ONLY: Calculates new plane position using a
   //     flight simulation. In an application, the real plane position 
   //     will be queried here.
   //////////////////////////////////////////////////////////////////////////
   void UpdatePlane( PlaneData plane )
   {      
      if( plane.graphics == null ||
         plane.from_node == null || plane.to_node == null )
        return;
            
      if( plane.path_position == 1.0 )
        StartPlane( plane, false );     // Finished old path, start a new one. 
      else
      {
         double speed = PlaneSpeed;

         /* Slow the selected plane down when zoomed on it for a nice
            demo effect.
         */
         if( plane == SelectedPlane && LockSelectedPlane )
         {
            GetPlanePosition( plane, 1, old_position );

            // Store the current path position.
            double stored_position = plane.path_position;

            plane.path_position += plane.speed * speed;    // Increment it
            
            GetPlanePosition( plane, 1, new_position );

            // Distance between the old and current position of the plane
            double dist = GetLength( old_position, new_position );

            /* Adjust the plane's speed to slow it down if the distance 
               is too big.
            */
            if( dist > MaxZoomSpeed )
            {
               double slow_down = dist / MaxZoomSpeed;
               speed /= slow_down;
            }

            // Restore the current path position.
            plane.path_position = stored_position;
         }

         /* Store last position for calculating the angle in the ORTHO 
            projection.
         */
         plane.path_position_last = plane.path_position;

         // Move the plane.
         plane.path_position += plane.speed * speed;
         if( plane.path_position > 1.0 )
           plane.path_position = 1.0; // Clamp to 1: can't go past the airport!
      }

      for( int i =0; i<2; ++i )
        PositionPlane( plane, i );    // Position the plane on both maps
   }

   //////////////////////////////////////////////////////////////////////////
   // In the lock mode, pans the map to keep selected plane visible when the 
   // plane moves out of the detailed map area.
   //////////////////////////////////////////////////////////////////////////
   void UpdateLocking()
   {	 
      int map = 1;    // Checking is done on the detailed map

      GetPlanePosition( SelectedPlane, map, position );

      /* If selected plane goes on another side of the globe or off the 
         visible portion of the map, pan the map to re-center on the 
         selected plane. The Z coordinate goes to zero when the plane gets 
         close to the globe's edge, and becomes negative on the invisible 
         side of the globe.
      */
      if( MapProjection[ map ] == GlgObject.ORTHOGRAPHIC_PROJECTION &&
         position.z < 0.1 || !GetVisibility( position, 0.9 ) )
      {
         String message =
           "Loading new map to keep the selected plane in sight, please wait...";

         CenterOnPlane( SelectedPlane, 1 );
         UpdateMapWithMessage( 1, message );

         /* If the thumbnail map used orthographic projection, rotate the 
            thumbnail globe to keep the selected plane visible. In rectangular
            projection, the whole world is visible and no action is required.
         */
         if( MapProjection[ map ] == GlgObject.ORTHOGRAPHIC_PROJECTION )
         {
            CenterOnPlane( SelectedPlane, 0 );
            UpdateMapWithMessage( 0, message );
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates map image, displaying the "wait..." message while the new 
   // map image is generated. 
   // Also updates selected region display on the thumbnail map.
   //////////////////////////////////////////////////////////////////////////
   void UpdateMapWithMessage( int map, String message )
   {
      // Display the wait message while the new map image is being generated.
      if( message != null )
        SetStatus( message );

      /* Just setup, don't draw: will be done at the end by the caller
         after selected area, etc., was updated.
      */
      Map[ map ].SetupHierarchy();

      /* Adjust selected region on the thumbnail map to match the new 
         zoom area of the detailed map.
      */
      SetSelectedArea();

      if( message != null )
        SetStatus( "" );
   }

   //////////////////////////////////////////////////////////////////////////
   // Handle user interaction.
   //////////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject vp, GlgObject message_obj )
   {
      String
        origin,
        format,
        action,
        subaction;

      super.InputCallback( vp, message_obj );

      origin = message_obj.GetSResource( "Origin" );
      format = message_obj.GetSResource( "Format" );
      action = message_obj.GetSResource( "Action" );
      subaction = message_obj.GetSResource( "SubAction" );

      // Handle window closing.
      if( format.equals( "Window" ) )
      {
         if( action.equals( "DeleteWindow" ) )
         {
            if( origin.equals( "SelectionDialog" ) )
            {
               // Close selection dialog
               Drawing.SetDResource( "SelectionDialog/Visibility", 0.0 );
               Update();	 
            }
            else if( origin.equals( "TopMap" ) )
            {
               // Close top map window
               Drawing.SetDResource( "TopMap/Visibility", 0.0 );
               Update();	 
            }
         }
         else if( action.equals( "FirstExposure" ) )
         {
            origin = message_obj.GetSResource( "Origin" );
            if( origin.equals( "Map" ) )
              MapIsReady = true;
            else if( origin.equals( "TopMap" ) )
              TopMapIsReady = true;
         }
         return;
      }

      if( format.equals( "Button" ) )  // Handle button clicks
      {	 
         if( !action.equals( "Activate" ) )
           return;
         
         PanMode = false;    // Abort Pan mode

         // Abort ZoomTo/Drag modes (if any) on both maps.
         Map[ 0 ].SetZoom( null, 'e', 0.0 );
         Map[ 1 ].SetZoom( null, 'e', 0.0 );

         if( origin.equals( "CloseDialog" ) )
         {
            Drawing.SetDResource( "SelectionDialog/Visibility", 0.0 );
            Update();	 
         }
         else if( origin.equals( "ToggleLock" ) )
         {
            SetLocking( !LockSelectedPlane );
            Update();	 
         }
         else if( origin.equals( "Up" ) )
         {	    
            Zoom( 'u', 0.0 );
            Update();
         }
         else if( origin.equals( "Down" ) )
         {	    
            Zoom( 'd', 0.0 );
            Update();
         }
         else if( origin.equals( "Left" ) )
         {	    
            Zoom( 'l', 0.0 );
            Update();
         }
         else if( origin.equals( "Right" ) )
         {	    
            Zoom( 'r', 0.0 );
            Update();
         }
         else if( origin.equals( "ZoomIn" ) )
         {
            Zoom( 'i', 2.0 );
            HandleZoomLevel();
            Update();	 
         }
         else if( origin.equals( "ZoomOut" ) )
         {
            Zoom( 'o', 2.0 );
            HandleZoomLevel();
            Update();	 
         }
         else if( origin.equals( "ZoomReset" ) )
         {	
            Zoom( 'n', 0.0 );
            HandleZoomLevel();
            Update();	 
         }
         else if( origin.equals( "ZoomTo" ) )
         {
            // Abort a possible Drag mode on thumbnail map.
            Map[ 0 ].SetZoom( null, 'e', 0.0 );

            // Start ZoomTo mode on the detailed map.
            Map[ 1 ].SetZoom( null, 't', 0.0 );
            SetStatus( "Define a rectangular area on the detailed map to zoom to." );
            SuspendPromptUpdates = true;
            Update();	 
         }
         else if( origin.equals( "Pan" ) )
         {	    
            PanMode = true;
            SetStatus( "Click to define a new center." );
            SuspendPromptUpdates = true;
            Update();	 
         }
         else if( origin.equals( "Drag" ) )
         {
            /* Activate dragging mode on both maps. Dragging will start on the
               mouse click. If no object of interest is selected by the mouse 
               click, the dragging will be started by the code in the Trace 
               callback anyway. The "Drag" button demonstrates an alternative 
               way to start dragging from a button.
            */
            Map[ 0 ].SetZoom( null, 's', 0.0 );
            Map[ 1 ].SetZoom( null, 's', 0.0 );
            SetStatus( "Click and drag the map with the mouse." );
            SuspendPromptUpdates = true;
            Update();
         }
         else if( origin.equals( "AirportLabels" ) )
         {
            CityLabels = !CityLabels;
            SetGISLayers( 1 );
            Update();	 
         }
         else if( origin.equals( "Planes" ) )
         {
            ToggleResource( Map[ 1 ], "PlaneGroup/Visibility" );
            Update();	 
         }
         else if( origin.equals( "ValueDisplay" ) )
         {
            // Visibility of all labels is constrained, set just one.
            ToggleResource( PlaneArray[ 0 ].graphics[ 1 ], 
                           "Label/Visibility" );
            Update();	 
         }
         else if( origin.equals( "ToggleStates" ) )
         {
            StateDisplay = !StateDisplay;
            SetGISLayers( 0 );    // Thumbnail map
            SetGISLayers( 1 );    // Detailed map 

            Update();	 
         }
         else if( origin.equals( "Update" ) )
         {
            PerformUpdates = !PerformUpdates;
            Update();
         }	
         else if( origin.equals( "PlaneSize" ) )
         {
            // Change plane icon's size.
            if( PlaneSize == SMALL_SIZE )
              PlaneSize = MEDIUM_SIZE;
            else if( PlaneSize == MEDIUM_SIZE )
              PlaneSize = BIG_SIZE;
            else // BIG_SIZE
              PlaneSize = SMALL_SIZE;	 

            SetPlaneSize();
            Update();	 
         }	
      }
      /* Process mouse clicks on plane icons, implemented as an Action with
         the Plane label attached to an icon and activated on a mouse click. 
      */
      else if( format.equals( "CustomEvent" ) )
      {
         String event_label = message_obj.GetSResource( "EventLabel" );

         if( event_label.equals( "Plane" ) )
         {
            /* Map dragging mode is activated on a mouse click in the trace 
               callback. Abort the dragging mode if an object with custom event
               was selected. This gives custom events a higher priority compared 
               to the dragging mode. If it's a ZoomTo mode activated by a button,
               don't abort and ignore the object selection.
            */
            int zoom_mode = ZoomToMode();
            if( zoom_mode == 0 ||
                ( zoom_mode & GlgObject.PAN_DRAG_STATE ) != 0 )
            {
               if( zoom_mode != 0 )
                 Map[1].SetZoom( null, 'e', 0.0 );  /* Abort zoom mode */

               int data_index = 
                 message_obj.GetDResource( "Object/DataIndex" ).intValue();
               
               if( SelectedPlane != PlaneArray[ data_index ] )
               {
                  SelectPlane( SelectedPlane, 0 );  // Unhighlight old
                  SelectedPlane = PlaneArray[ data_index ];
                  SelectPlane( SelectedPlane, 1 );  // Highlight new 
                  Update();
               }
            }
         }
      }
      else if( action.equals( "Zoom" ) )
      {
         // Disable locking: we may be zooming on a different area.
         SetLocking( false );

         if( subaction.equals( "Start" ) )   // Starting ZoomTo
         {
            SuspendPromptUpdates = true;
         }
         else if( subaction.equals( "ZoomRectangle" ) )
         {
            /* ZoomTo rectangle created - set a custom distinct color
               good for colors at all zoom levels.
            */
            GlgObject zoom_rect = Map[1].GetResourceObject( "GlgZoomRect" );
            zoom_rect.SetGResource( "EdgeColor", 1.0, 0.0, 0.0 );
         }
         else if( subaction.equals( "End" ) )   // Finishing ZoomTo
         {
            /* Rotate the thumbnail globe to show the same area as the 
               detailed map.
            */
            SyncGlobeWithDetailedMap( origin,
                                      "Zooming the map, please wait..." );
            
            HandleZoomLevel();
            SuspendPromptUpdates = false;
            Update();
         }
         // Aborting ZoomTo (right mouse button, etc.).
         else if( subaction.equals( "Abort" ) )
         {
            SuspendPromptUpdates = false;
         }
      }
      else if( action.equals( "Pan" ) )
      {
         // Disable locking when scrolling the map with the mouse.
         SetLocking( false );

         // Starting dragging with the mouse
         if( subaction.equals( "Start" ) )    // Map dragging start
         {
            SetStatus( "Drag the map with the mouse." );
            SuspendPromptUpdates = true;
            Update();
         }
         else if( subaction.equals( "Drag" ) ||         // Dragging
                 subaction.equals( "ValueChanged" ) )   // Scrollbars
         {
            String message_str;

            if( subaction.equals( "Drag" ) )
              message_str = "Dragging the map with the mouse....";
            else
              message_str = "Scrolling the map, please wait...";

            /* When dragging the detailed map with the mouse, rotate the 
               thumbnail globe to show the same area as the detailed map.
            */
            SyncGlobeWithDetailedMap( origin, message_str );
         
            if( subaction.equals( "Drag" ) )
              SetStatus( message_str );   // Keep the message when dragging.

            Update();
         }
         else if( subaction.equals( "End" ) )      // Map dragging end
         {
            SuspendPromptUpdates = false;
            SetStatus( "" );   // Reset prompt when done dragging.

            /* Reset dragging mode on both maps, in case it was started 
               with the Drag button.
            */
            Map[ 0 ].SetZoom( null, 'e', 0.0 );
            Map[ 1 ].SetZoom( null, 'e', 0.0 );
            Update();         
         }
         // Dragging aborted (right mouse button, etc.).
         else if( subaction.equals( "Abort" ) )
         {
            SuspendPromptUpdates = false;
            SetStatus( "" );   // Reset prompt when aborting dragging.
            Update();         
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   int ZoomToMode()
   {
      // Check if the detailed map is in the ZoomTo mode.
      return Map[ 1 ].GetDResource( "ZoomToMode" ).intValue();
   }

   //////////////////////////////////////////////////////////////////////////
   void Zoom( char type, double value )
   {
      switch( type )
      {
       default:
         Map[ 1 ].SetZoom( null, type, value );
         CheckScrollLimits( type );
         UpdateMapWithMessage( 1, "Zooming or panning, please wait..." );

         /* Sync thumbnail map when panning. After "1:1" zoom reset, the maps' 
            centers may differ, sync the thumbnail map when zooming in the 
            first time.
         */
         switch( type )
         {
          case 'i':
            SyncGlobeWithDetailedMap( null, "Zooming the map, please wait..." );
            break;
          case 'u':
          case 'd':
          case 'l':
          case 'r':
            SyncGlobeWithDetailedMap( null, "Panning the map, please wait..." );
            break;
         }
         break;

       case 'n':
         if( MapProjection[ 0 ] == GlgObject.ORTHOGRAPHIC_PROJECTION )
         {
            // Reset thumbnail globe to initial position.
            GISObject[ 0 ].SetGResource( "GISCenter", InitCenter[0] );
            GISObject[ 0 ].SetGResource( "GISExtent", InitExtent[0] );

            UpdateMapWithMessage( 0, "Reloading map, please wait..." );
         }

         // Reset detailed map to initial extent.
         GISObject[ 1 ].SetGResource( "GISCenter", InitCenter[1] );
         GISObject[ 1 ].SetGResource( "GISExtent", InitExtent[1] );
         UpdateMapWithMessage( 1, "Reloading map, please wait..." );

         // Make selected area rectangle invisible when no zoom
         GISObject[ 0 ].SetDResource( "GISArray/SelectedArea/Visibility", 0.0 );
         break;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // For rectangular projection on the detailed map, make sure the map 
   // does not scroll beyond the poles in the vertical direction.
   //////////////////////////////////////////////////////////////////////////
   void CheckScrollLimits( char type )
   {
      GlgPoint extent, center;
      double min_y, max_y;
      boolean adjust_x, adjust_y;
      
      if( MapProjection[ 1 ] == GlgObject.ORTHOGRAPHIC_PROJECTION )
        return;   // Allow unlimited scrolling on ortho.
      
      switch( type )
      {
       case 'u':  // Scroll up
       case 'd':  // Scroll down
       case 'l':  // Scroll left
       case 'r':  // Scroll right
         break; // Adjust only for scroll types.
       default: return;   // Don't adjust for other zoom types.
      }
      
      extent = GISObject[ 1 ].GetGResource( "GISExtent" );
      center = GISObject[ 1 ].GetGResource( "GISCenter" );
   
      min_y = center.y - extent.y / 2.0;
      max_y = center.y + extent.y / 2.0;
      
      /* Check and adjust center lat to make sure the map does not scroll 
         beyond the poles in the vertical direction. 
      */
      adjust_y = true;
      if( extent.y >= 180.0 )
        center.y = 0.0;
      else if( min_y < -90.0 )
        center.y = -90.0 + extent.y / 2.0;
      else if( max_y > 90.0 )
        center.y = 90.0 - extent.y / 2.0;
      else
        adjust_y = false;

      /* Allow scrolling tp +-180 in horizontal direction, to match the
         range of the horizontal scrollbar.
      */
      adjust_x = true;
      if( center.x < -180.0 )
        center.x = -180.0;
      else if( center.x > 180.0 )
        center.x = 180.0;
      else
        adjust_x = false;

      // Set adjusted center
      if( adjust_x || adjust_y )
        GISObject[ 1 ].SetGResource( "GISCenter", center );
   }

   //////////////////////////////////////////////////////////////////////////
   // Rotates the globe on the thumbnail map to show the same place as the
   // detailed map.
   //////////////////////////////////////////////////////////////////////////
   void SyncGlobeWithDetailedMap( String origin, String message )
   {
      GlgPoint center, globe_center;
      
      UpdateMapWithMessage( 1, message );

      /* Sync up only of the detailed map (origin == "Map") is rotated, 
         not the thumbnail map (origin == "TopMap").
      */
      if( origin != null && origin.equals( "Map" ) )
      {
         // Get the center of the detailed map.
         center = GISObject[ 1 ].GetGResource( "GISCenter" );
      
         // Get the center of the thumbnail globe.
         globe_center = GISObject[ 0 ].GetGResource( "GISCenter" );
      
         // Sync up if centers differ.
         if( globe_center.x != center.x || globe_center.y != center.y ||
            globe_center.z != center.z )
         {
            // Rotate the thumbnail globe to show the same area.
            GISObject[ 0 ].SetGResource( "GISCenter", center );
         
            UpdateMapWithMessage( 0, message );
         }
      }
      else
        /* Don't sync when the globe on the thumbnail map is rotated
           (origin == "TopMap"), to allow moving it separately. Just
           update the selected area display.
        */
        UpdateMapWithMessage( 0, message );
   }

   //////////////////////////////////////////////////////////////////////////
   // Used to obtain coordinates of the mouse click. 
   //////////////////////////////////////////////////////////////////////////
   public void TraceCallback( GlgObject viewport, GlgTraceData trace_info )
   {      
      int map;

      if( !MapIsReady || !TopMapIsReady )
        return;

      int event_type = trace_info.event.getID();

      // Use the Map area events only.
      if( trace_info.viewport == Map[ 0 ] )
        map = 0;
      else if( trace_info.viewport == Map[ 1 ] )
        map = 1;
      else
      {
         /* Erase the current postion display when the mouse moves outside 
            of the map.
         */
         switch( event_type )
         {
          case MouseEvent.MOUSE_MOVED:
          case MouseEvent.MOUSE_DRAGGED:
            PositionObject.SetSResource( null, "" );
            PositionArea.Update();
            break;
         }
         return;
      }

      switch( event_type )
      {
       case MouseEvent.MOUSE_MOVED: // Report lat/lon position under the mouse.
       case MouseEvent.MOUSE_DRAGGED:
         point.x = (double) ((MouseEvent)trace_info.event).getX();
         point.y = (double) ((MouseEvent)trace_info.event).getY();
         point.z = 0.0;

         /* COORD_MAPPING_ADJ is added to the cursor coordinates for 
            precise pixel mapping.
         */
         point.x += GlgObject.COORD_MAPPING_ADJ;
         point.y += GlgObject.COORD_MAPPING_ADJ;

         /* Converts X/Y to lat/lon using GIS object's current projection,
            handles both maps.
         */
         GISObject[ map ].
           GISConvert( null, GlgObject.SCREEN_COORD,
                      /* X/Y to Lat/Lon */ true, point, lat_lon );

         PositionObject.SetSResource( null, CreateLocationString( lat_lon ) );
         PositionArea.Update();
         break;

       case MouseEvent.MOUSE_PRESSED:
         if( ZoomToMode() != 0 )
           return; // ZoomTo or dragging mode in progress: pass it through.

         double x = (double) ((MouseEvent)trace_info.event).getX();
         double y = (double) ((MouseEvent)trace_info.event).getY();

         /* COORD_MAPPING_ADJ is added to the cursor coordinates for 
            precise pixel mapping.
         */
         x += GlgObject.COORD_MAPPING_ADJ;
         y += GlgObject.COORD_MAPPING_ADJ;

         // Handle paning: set the new map center to the location of the click.
         if( PanMode )
         {
            if( GetButton( trace_info.event ) != 1 )
              return;  // Use the left button clicks only.

            PanMode = false;
            
            point.x = x;
            point.y = y;
            point.z = 0.0;
            
            /* Converts X/Y to lat/lon using GIS object's current projection,
               handles clicks on either map.
            */
            GISObject[ map ].GISConvert( null, GlgObject.SCREEN_COORD,
                                        /* X/Y to Lat/Lon */ true,
                                        point, lat_lon );

            if( MapProjection[ 0 ] == GlgObject.ORTHOGRAPHIC_PROJECTION )
            {
               /* For the orthographic projection, Pan/Rotate the globe on the 
                  thumbnail map as well. Don't do anything for the rectangular 
                  projection: the whole world is already displayed.
               */
               GISObject[ 0 ].SetGResource( "GISCenter", lat_lon );
               UpdateMapWithMessage( 0, "Panning, please wait..." );
               Map[ 0 ].Update();
            }

            // Pan the detailed map.
            GISObject[ 1 ].SetGResource( "GISCenter", lat_lon );
            UpdateMapWithMessage( 1, "Panning the map..." );

            // Disable locking: we scrolled the map to see a different area.
            SetLocking( false );
            Update();
         }
         else
           /* Not a pan mode: start dragging the map with the mouse. */
           Map[ map ].SetZoom( null, 's', 0.0 );
         break;

       default: return;
      }      
   }

   //////////////////////////////////////////////////////////////////////////
   // Adjust selected region on the thumbnail map to match detailed map.
   //////////////////////////////////////////////////////////////////////////
   void SetSelectedArea()
   {
      int i;

      // Set the coordinates of the SelectedArea polygon.
      GlgObject rect = 
        GISObject[ 0 ].GetResourceObject( "GISArray/SelectedArea" );
      
      GlgPoint extent = GetExtentDegrees( 1 );

      if( extent.x >= 120.0 )
      {
         // Big area: don't need to show.
         rect.SetDResource( "Visibility", 0.0 );
      }
      else
      {
         rect.SetDResource( "Visibility", 1.0 );

         /*  Get lat/lon of the visible area of the detailed map. 
             Use 16 points for better precision, since the area is not 
             rectangular in the orthographic projection used for the thumbnail 
             globe.
         */
         GlgPoint [] lat_lon = new GlgPoint[ 16 ];
         lat_lon[ 0  ] = GetLatLon( -1000.0, -1000.0, 1 );
         lat_lon[ 1  ] = GetLatLon( -1000.0,  -500.0, 1 );
         lat_lon[ 2  ] = GetLatLon( -1000.0,     0.0, 1 );
         lat_lon[ 3  ] = GetLatLon( -1000.0,   500.0, 1 );
         lat_lon[ 4  ] = GetLatLon( -1000.0,  1000.0, 1 );
         lat_lon[ 5  ] = GetLatLon(  -500.0,  1000.0, 1 );
         lat_lon[ 6  ] = GetLatLon(     0.0,  1000.0, 1 );
         lat_lon[ 7  ] = GetLatLon(   500.0,  1000.0, 1 );
         lat_lon[ 8  ] = GetLatLon(  1000.0,  1000.0, 1 );
         lat_lon[ 9  ] = GetLatLon(  1000.0,   500.0, 1 );
         lat_lon[ 10 ] = GetLatLon(  1000.0,     0.0, 1 );
         lat_lon[ 11 ] = GetLatLon(  1000.0,  -500.0, 1 );
         lat_lon[ 12 ] = GetLatLon(  1000.0, -1000.0, 1 );
         lat_lon[ 13 ] = GetLatLon(   500.0, -1000.0, 1 );
         lat_lon[ 14 ] = GetLatLon(     0.0, -1000.0, 1 );
         lat_lon[ 15 ] = GetLatLon(  -500.0, -1000.0, 1 );

         for( i=0; i<16; ++i )
         {
            // Get polygon's points
            GlgObject point_obj = (GlgObject) rect.GetElement( i );

            // Set point's lat/lon
            point_obj.SetGResource( null, lat_lon[ i ] );
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // FOR FLIGHT SIMULATION ONLY: Starts simulation for a plane, 
   //     assigns its start and end nodes.
   //////////////////////////////////////////////////////////////////////////
   void StartPlane( PlaneData plane, boolean init )
   {
      if( NumNodes < 2 )
      {
         System.out.println( "Less then two nodes: can't start planes." );
         return;
      }

      int to_index;
      int from_index = (int) GlgObject.Rand( 0, NumNodes - 0.001 );
      do
      {
         to_index = (int) GlgObject.Rand( 0, NumNodes - 0.001 );
      } while( to_index == from_index );

      plane.from_node = NodeArray[ from_index ];
      plane.to_node = NodeArray[ to_index ];
      plane.flight_number = (int) GlgObject.Rand( 101.0, 1999.0 );
      plane.speed = GlgObject.Rand( 0.4, 1.0 );   // Vary plane speed

      if( init )
      {
         plane.path_position = GlgObject.Rand( 0.1, 0.2 );
         plane.path_position_last = plane.path_position - 0.05;  // For angle
      }
      else
      {
         plane.path_position = 0.0;
         plane.path_position_last = 0.0;
      }

      plane.tooltip[ 0 ] = "Flight " + plane.flight_number;

      // On the detailed map, add from/to node info to the tooltip.
      plane.tooltip[ 1 ] = plane.tooltip[ 0 ] +
        " from " + plane.from_node.name + " to " + plane.to_node.name;

      // Set the tooltip
      for( int i = 0; i < 2; ++i )      
        plane.graphics[ i ].SetSResource( "TooltipString", 
                                           plane.tooltip[ i ] );

      // On detailed map[1], show the flight number as icon label
      plane.graphics[1].SetSResource( "LabelString",
                                      "Flight " + plane.flight_number );

      // Stop tracking the selected flight when it reaches destination.
      if( plane == SelectedPlane )
        SetLocking( false );
   }

   //////////////////////////////////////////////////////////////////////////
   // FOR FLIGHT SIMULATION ONLY: Calculates plane's lat/lon using simulated 
   //     data. In an application, it will query the current plane position.
   //
   // The simulation moves the plane from the start to the end node/city
   // as controlled by the path_position parameter. The path_position changes
   // in the range from from 0 (start node) to 1 (end node).
   //////////////////////////////////////////////////////////////////////////
   void GetPlaneLatLon( PlaneData plane )
   {
      plane.lat_lon.x = 
        RELATIVE_TO_NEW_RANGE( plane.from_node.lat_lon.x, 
                              plane.to_node.lat_lon.x,
                              plane.path_position );

      plane.lat_lon.y = 
        RELATIVE_TO_NEW_RANGE( plane.from_node.lat_lon.y, 
                              plane.to_node.lat_lon.y,
                              plane.path_position );
   }

   //////////////////////////////////////////////////////////////////////////
   // FOR FLIGHT SIMULATION ONLY: Select some plane on the initial appearance.
   //////////////////////////////////////////////////////////////////////////
   void InitSelection()
   {
      // Select the first plane
      SelectedPlane = PlaneArray[ 0 ];
      SelectPlane( SelectedPlane, 1 );

      // Lock on the selected plane, pan the map to keep it visible.
      SetLocking( true );

      // Rotate thumbnail globe too to show the same location.
      if( MapProjection[ 0 ] == GlgObject.ORTHOGRAPHIC_PROJECTION )
        GISObject[ 0 ].SetGResource( "GISCenter", -95.35, 37.37, 0.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   // UTILITY FUNCTION: Calculates an angle between the line defined by two 
   // points and the X axis.
   //////////////////////////////////////////////////////////////////////////
   double GetAngle( GlgPoint pt1, GlgPoint pt2 )
   {
      double length, angle;
      
      length = GetLength( pt1, pt2 );
      
      if( length == 0.0 )
        angle = 0.0;
      else
      {
         angle = Math.acos( ( pt2.x - pt1.x ) / length );

         if( pt2.y - pt1.y < 0.0 )  // ScreenSpace Z axis points to the user.
           angle = - angle;
      }

      return RadToDeg( angle );
   }

   //////////////////////////////////////////////////////////////////////////
   // UTILITY FUNCTION: Calculates a distance between two points in 2D.
   //////////////////////////////////////////////////////////////////////////
   double GetLength( GlgPoint pt1, GlgPoint pt2 )
   {
      return Math.sqrt( ( pt2.x - pt1.x ) * ( pt2.x - pt1.x ) +
                       ( pt2.y - pt1.y ) * ( pt2.y - pt1.y ) );
   }

   //////////////////////////////////////////////////////////////////////////
   // Turns plane icons' labels ON or OFF on the detailed map.
   //////////////////////////////////////////////////////////////////////////
   void SetPlaneLabels( boolean on )
   {
      GlgObject label = 
        PlaneArray[ 0 ].graphics[ 1 ].GetResourceObject( "Label" );
      if( label != null )
        label.SetDResource( "Visibility", on ? 1.0 : 0.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   // Sets locking mode ON or OFF. If locking is ON, the map is automatically
   // scrolled to keep the selected plane icon in view.
   //////////////////////////////////////////////////////////////////////////
   void SetLocking( boolean lock_state )
   {
      LockSelectedPlane = lock_state;
   }

   //////////////////////////////////////////////////////////////////////////
   // Displays the selected plane's location and locking status.
   //////////////////////////////////////////////////////////////////////////
   void UpdateSelectedPlaneStatus()
   {
      String message;

      if( SuspendPromptUpdates )
        return;

      if( SelectedPlane != null )
      {
         GetPlaneLatLon( SelectedPlane );

         if( LockSelectedPlane )
           message = "Map is locked on selected Flight " + 
             SelectedPlane.flight_number + ", " + 
               CreateLocationString( SelectedPlane.lat_lon );
         else
           message = "Selected Flight " +  SelectedPlane.flight_number +
             ", " + CreateLocationString( SelectedPlane.lat_lon );
      }
      else
        message = "Click on the plane icon to select.";
      Drawing.SetSResource( "StatusLabel/String", message );
   }

   //////////////////////////////////////////////////////////////////////////
   // Displays a message in the status area.
   //////////////////////////////////////////////////////////////////////////
   void SetStatus( String message )
   {
      Drawing.SetSResource( "StatusLabel/String", message );
      Drawing.GetResourceObject( "StatusArea" ).Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Centers the map on the selected plane, used when locking mode is ON.
   //////////////////////////////////////////////////////////////////////////
   void CenterOnPlane( PlaneData plane, int map )
   {      
      // Center the map on the plane
      GISObject[ map ].SetGResource( "GISCenter", plane.lat_lon );
   }

   //////////////////////////////////////////////////////////////////////////
   // Highlights the selected plane on both maps by changing its 
   // SelectedIndex value.
   //////////////////////////////////////////////////////////////////////////
   void SelectPlane( PlaneData plane, int selected )
   {
      for( int i=0; i < 2; ++i )
        if( plane.graphics[ i ] != null )
          plane.graphics[ i ].SetDResource( "SelectedIndex",
                                           (double)selected );
   }

   //////////////////////////////////////////////////////////////////////////
   void SetPlaneSize()
   {
      for( int i=0; i<2; ++i )
      {
         GlgObject resource = 
           PlaneTemplate[ i ].GetResourceObject( "IconScale" );
         if( resource != null )
           resource.SetDResource( null, PlaneSize );
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Toggle resource between 0 and 1.
   //////////////////////////////////////////////////////////////////////////
   void ToggleResource( GlgObject glg_object, String res_name )
   {
      double value = glg_object.GetDResource( res_name ).doubleValue();
      glg_object.SetDResource( res_name, value != 0.0 ? 0.0 : 1.0 );
   }
   ////////////////////////////////////////////////////////////////////////
   // Toggle map layers: airport/city labels and states.
   ////////////////////////////////////////////////////////////////////////
   void SetGISLayers( int map )
   {
      String layers;

      // Airport labels should be visible only when city labels are off.
      NodeTemplate[1].SetDResource( "Label/Visibility", CityLabels ? 0.0 : 1.0 );

      layers = "default_gis";

      // Add city layers if they are on on the detailed map.
      if( map == 1 )
        if( CityLabels )
          layers = layers + ",us_cities";
        else
          layers = layers + ",-us_cities";

      if( StateDisplay )   // Add states layer if it is on.
        // Enable states regardless of the default.
        layers = layers + ",states";
      else
        // Disable state outline display.
        layers = layers + ",-states";

      GISObject[ map ].SetSResource( "GISLayers", layers );
   }

   ////////////////////////////////////////////////////////////////////////
   // Convenience wrapper
   ////////////////////////////////////////////////////////////////////////
   GlgPoint GetLatLon( double x, double y, int map )
   {
      GlgPoint lat_lon = new GlgPoint();

      util_point.x = x;
      util_point.y = y;
      util_point.z = 0.0;
      GISObject[ map ].GISConvert( null, GlgObject.OBJECT_COORD,
                                  /* X/Y to Lat/Lon */ true,
                                  util_point, lat_lon );
      return lat_lon;
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

   //////////////////////////////////////////////////////////////////////////
   double RELATIVE_TO_NEW_RANGE( double low, double high, double rel_value )
   {
      return ( (low) + ((high) - (low)) * rel_value );
   }

   //////////////////////////////////////////////////////////////////////////
   double VALUE_TO_RELATIVE( double low, double high, double value )
   {
      return ( high - low != 0.0 ? ((value) - (low)) / ((high) - (low)) : 0.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   double DegToRad( double angle )
   {
      return angle / 180.0 * Math.PI;
   }

   //////////////////////////////////////////////////////////////////////////
   double RadToDeg( double angle )
   {
      return angle / Math.PI * 180.0;
   }

   //////////////////////////////////////////////////////////////////////////
   public void StopUpdates()
   {
      if( timer != null )
      {
         timer.stop();
         timer = null;
      }

      if( zoom_timer != null )
      {
         zoom_timer.stop();
         zoom_timer = null;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void StartUpdates()
   {
      if( timer == null )
      {
         /* Restart the timer after each update (instead of using repeats)
            to avoid flooding the event queue with timer events on slow 
            machines.
         */
         timer = new Timer( UpdateInterval, this );
         timer.setRepeats( false );
         timer.start();
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // ActionListener method to use the bean as update timer's ActionListener.
   //////////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdatePlanes();
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked by the browser to stop the applet
   //////////////////////////////////////////////////////////////////////////
   public void stop()
   {
      StopUpdates();
      MapIsReady = false;
      TopMapIsReady = false;
      super.stop();
   }

   //////////////////////////////////////////////////////////////////////////
   // Turn airport and plane labels on or off depending on the zoom level and
   // adjust plane icon size. 
   //////////////////////////////////////////////////////////////////////////
   void HandleZoomLevel()
   {
      GlgPoint extent;
      boolean high_zoom;
      
      extent = GetExtentDegrees( 1 );
      
      high_zoom = ( extent.x < 100.0 && extent.y < 50.0 );
      
      SetPlaneLabels( high_zoom );     // Plane labels.
      
      CityLabels = true;
      SetGISLayers( 1 );     // Airport labels.
      
      // Plane icons size.
      PlaneSize = ( high_zoom ? MEDIUM_SIZE : SMALL_SIZE );
      SetPlaneSize();
   }

   //////////////////////////////////////////////////////////////////////////
   // Gets extent in lat/lon.
   // For the ortho projection, roughly converts from meters to lat/lon.
   //////////////////////////////////////////////////////////////////////////
   GlgPoint GetExtentDegrees( int map )
   {   
      GlgPoint extent = GetGResource( GISObject[ map ], "GISExtent" );
      if( MapProjection[ map ] == GlgObject.ORTHOGRAPHIC_PROJECTION )
      {
         extent.x = extent.x / GlgObject.EQUATOR_RADIUS * 90.0;
         extent.y = extent.y / GlgObject.POLAR_RADIUS * 90.0;
      }
      return extent;
   }

   //////////////////////////////////////////////////////////////////////////
   // Generate a location info string by converting +- sign info into the
   // N/S, E/W suffixes, and decimal fraction to deg, min, sec.
   //////////////////////////////////////////////////////////////////////////
   String CreateLocationString( GlgPoint point )
   {
      int
        x_deg, y_deg,
        x_min, y_min,
        x_sec, y_sec;
      char
        char_x,
        char_y;
      double lat, lon;

      if( point.z < 0.0 )
        return "";

      lon = point.x;
      lat = point.y;

      if( lon < 0.0 )
      {
         lon = -lon;
         char_x = 'W';
      }
      else if( lon >= 360.0 )
      {
         lon -= 360.0;
         char_x = 'E';
      }
      else if( lon >= 180.0 )
      {
         lon = 180.0 - ( lon - 180.0 );
         char_x = 'W';
      }
      else
        char_x = 'E';
      
      if( lat < 0.0 )
      {
         lat = -lat;
         char_y = 'S';
      }
      else
        char_y = 'N';
      
      x_deg = (int) lon;
      x_min = (int) ( ( lon - x_deg ) * 60.0 );
      x_sec = (int) ( ( lon - x_deg - x_min / 60.0 ) * 3600.0 );
      
      y_deg = (int) lat;
      y_min = (int) ( ( lat - y_deg ) * 60.0 );
      y_sec = (int) ( ( lat - y_deg - y_min / 60.0 ) * 3600.0 );
      
      return "Lon=" + x_deg + "\u00B0" + 
                 padded( x_min ) + "\'" + padded( x_sec ) + "\"" + char_x +
            "  Lat=" + y_deg + "\u00B0" + 
                 padded( y_min ) + "\'" + padded( y_sec ) + "\"" + char_y;
   }

   //////////////////////////////////////////////////////////////////////////
   // Pads the value with 0 if needed to have a constant field width. 
   //////////////////////////////////////////////////////////////////////////
   String padded( int value )
   {
      if( value < 10 )
        return "0" + value;
      else
        return "" + value;
   }

   //////////////////////////////////////////////////////////////////////////
   // Show zoom message.
   //////////////////////////////////////////////////////////////////////////
   void ZoomToUSStart()
   {
      Drawing.SetDResource( "Map/USZoomingMessage/Visibility", 1.0 );
      Drawing.Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Zoom to the US area after a few seconds to show details.
   //////////////////////////////////////////////////////////////////////////
   void ZoomToUS()
   {
      /* GISExtent is in lon/lat for the rectangular GIS projection,
         and in meters for the orthographic projection.
         To find proper values, zoom the map in the GlgBuilder and 
         copy the GISExtent values. */

      // Zoom to the US boundaries on detailed map.
      GISObject[ 1 ].SetGResource( "GISCenter", -95.35, 37.37, 0.0 );
      GISObject[ 1 ].SetGResource( "GISExtent", 69.71, 34.85, 0.0 );
            
      if( MapProjection[ 0 ] == GlgObject.ORTHOGRAPHIC_PROJECTION )
        // Rotate thumbnail globe too to show the same location.
        GISObject[ 0 ].SetGResource( "GISCenter", -95.35, 37.37, 0.0 );
            
      HandleZoomLevel();
            
      UpdateMapWithMessage( 1, "Zooming, please wait..." );
      UpdateMapWithMessage( 0, "Zooming, please wait..." );

      /* Reorder US zoom message to top, otherwise airplane icons 
         would be flying on top of it.
      */
      GlgObject florida_message = 
        Drawing.GetResourceObject( "Map/USZoomingMessage" );
      Map[1].ReorderElement( Map[1].GetIndex( florida_message ), 
                            Map[1].GetSize() - 1 );

      Drawing.Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Remove the US zooming message after a few seconds.
   //////////////////////////////////////////////////////////////////////////
   void ZoomToUSEnd()
   {
      Drawing.SetDResource( "Map/USZoomingMessage/Visibility", 0.0 );
      Drawing.Update();
   }

   class NodeData
   {
      String name;
      GlgPoint lat_lon;
      GlgObject [] graphics = new GlgObject[ 2 ];

      NodeData( String name_p, double lon, double lat )
      {
         name = name_p;
         lat_lon = new GlgPoint( lon, lat, 0.0 );
      }
   }

   class PlaneData
   {
      String name;
      GlgPoint lat_lon;
      int flight_number;
      String []tooltip = new String[ 2 ];
      GlgObject [] graphics = new GlgObject[ 2 ];
      NodeData from_node;
      NodeData to_node;
      double path_position;
      double path_position_last;
      double speed;
      boolean [] has_angle = new boolean[ 2 ];
      double [] angle = new double[ 2 ];

      PlaneData()
      {
         lat_lon = new GlgPoint();
      }
   }

   class ZoomPerformer implements ActionListener
   {
      GlgGISDemo bean;
      int stage;

      ZoomPerformer( GlgGISDemo bean_p )
      {
         bean = bean_p;
         stage = 0;
      }

      public void actionPerformed( ActionEvent e )
      {
         PerformZoomAction();
      }

      void PerformZoomAction()
      {
         switch( stage )
         {
          case 0:  // Display zoom message, yield to let event thread draw it.
            bean.ZoomToUSStart();

            stage = 1;
            bean.zoom_timer.setInitialDelay( 1 );
            bean.zoom_timer.start();
            break;

          case 1:  // Zoom to the US area
            bean.ZoomToUS();

            stage = 2;
            bean.zoom_timer.setInitialDelay( USZoomDelay2 );
            bean.zoom_timer.start();            
            break;

          case 2:  // Erase zoom message after a delay
            bean.ZoomToUSEnd();
            bean.zoom_timer = null;
            break;
         }
      }
   }
}
