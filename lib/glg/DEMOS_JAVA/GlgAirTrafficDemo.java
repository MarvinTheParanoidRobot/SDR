//////////////////////////////////////////////////////////////////////////
// An Air Traffic Control demo with a map displayed using the GLG Map Server.
//
// This demo uses the GLG Map Server to display a map.
// The Map Server has to be installed either on the local host or on a
// remote web server. After the Map Server has been installed, enable
// the map by modifying the source code to comment out the statement that
// sets the GISDisable resource, set SuppliedMapServerURL to point 
// to the Map Server location and rebuild the demo.
//
// The extended version of the demo (GlgAirTrafficExt) demonstrates the use 
// of the asynchronous map loading request API to load a map in a separate 
// thread, so that the program will continue to operate with the old zoom or
// pan state and will switch to the new zoom or pan when the new map is ready. 
//////////////////////////////////////////////////////////////////////////
import java.awt.*;
import java.awt.event.*;
import java.lang.reflect.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgAirTrafficDemo extends GlgJBean implements ActionListener
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   /* Controls map loading behavior.
      If set to true, the program will operate in a new zoom or pan state with
      no map, and the map will appear when it is ready. 
      If set to false, the program will pause until the new map is loaded.
   */
   boolean ASYNC_MAP = true;

   // If supplied, overrides the URL of the GIS object in the drawing
   static String SuppliedMapServerURL = null; 

   static boolean StandAlone = false;
   double PlaneSpeed = 0.002;    // Relative units   

   int FloridaZoomDelay1 = 3000; // Delay to zoom to Florida to show details.
   int FloridaZoomDelay2 = 1000; // Delay to remove Florida zooming message.

   // Constants
   static final int 
      MAX_NUM_PLANES = 10000,
      NUM_NODE_TYPES = 2,
      NUM_PLANE_TYPES = 3;

   static final double
      SMALL_SIZE  = 1.0,
      MEDIUM_SIZE = 1.5,
      BIG_SIZE    = 2.0;

   static final double
      SMALL_MARKER_SIZE  = 7.0,
      MEDIUM_MARKER_SIZE = 9.0,
      BIG_MARKER_SIZE    = 11;

   static final int
      NORMAL = 0,
      WARNING = 1,
      ALARM = 2,
      SELECTED = 3;

   GlgObject [] NodePool = new GlgObject[ NUM_NODE_TYPES ];
   GlgObject [] PlanePool = new GlgObject[ NUM_PLANE_TYPES ];
   GlgObject [] NodeTemplate = new GlgObject[ NUM_NODE_TYPES ];
   GlgObject [] PlaneTemplate = new GlgObject[ NUM_PLANE_TYPES ];

   GlgObject
     Drawing,     
     PositionObject,
     PositionArea,
     TrajectoryTemplate,
     TrajectoryPool,
     Map,
     FlightInfoPopup,
     GISObject,
     GISArray,
     NodeGroup,
     PlaneGroup,
     TrajectoryGroup,
     DistancePolygon;

   // Store initial extent and center, used to reset.
   GlgPoint 
     InitExtent = new GlgPoint(),
     InitCenter = new GlgPoint();

   String Layers = null;

   double PlaneSize = SMALL_SIZE;

   int MapProjection;

   int
     UpdateInterval = 50,  // Update interval in msec.
     NumPlanes = 100,
     NumNodes,
     NumTrajectoryPoints,
     PlaneType = 0,
     NodeType = 0,
     SelectedPlaneIndex = -1,
     NumDistancePoints;
   
   boolean
     MapLoaded = false,
     OrthoOnly = false,
     PerformUpdates = true,
     PanMode = false,
     DistanceMode = false,
     RedoIcons = true,
     HasAngle = false,
     HasElevation = false,
     CityLabels = true,
     StateDisplay = true,
     DraggingFromButton = false;
   
   PlaneData [] PlaneArray;

   /* Array of icons to place on the map as GLG objects in addition to the icons
      defined in GIS server's data. The icons that use GLG objects may be 
      selected with the mouse and their attributes can be changed dynamically, 
      based on data. When the mouse moves over an icon, it may be highlighted 
      with a different color or a tooltip may be displayed. 
   */
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

   // Temporary variables: allocate once
   GlgPoint
     lat_lon = new GlgPoint(),
     point = new GlgPoint(),
     last_xyz = new GlgPoint(),
     curr_xyz = new GlgPoint();

   Timer timer = null;
   Timer zoom_timer = null;

   //////////////////////////////////////////////////////////////////////////
   public GlgAirTrafficDemo()
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
   // For use as a stand-alone java demo
   //////////////////////////////////////////////////////////////////////////
   public static void main( final String [] arg )
   {
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Main( arg ); } } );
   }

   //////////////////////////////////////////////////////////////////////////
   // Optional arguments.
   // -num_planes <number>  - specify the number of planes to be displayed;
   //                          default: 100
   // -speed <number>       - plane speed in relative units, default: 0.005
   // -ortho                - use only ORTHOGRAPHIC projection, don't change
   //                         to RECTANGULAR projection under high zoom levels.
   // -map_server_url       - specify MapServerURL; it may be supplied as an
   //                         applet parameter when used as an applet;
   //                         default: use GIS object's MapServerURL property 
   //                         from the drawing file.
   // -sync                 - Load map synchronously (see ASYNC_MAP comments)
   // -async                - Load map asynchronously (see ASYNC_MAP comments)
   //////////////////////////////////////////////////////////////////////////
   public static void Main( String [] arg )
   {
      class DemoQuit extends WindowAdapter
      {
         public void windowClosing( WindowEvent e ) { System.exit( 0 ); }
      } 

      JFrame frame = new JFrame( "GLG AirTraffic Monitoring Demo" );

      frame.setResizable( true );
      frame.setSize( 800, 700 );
      frame.setLocation( 20, 20 );

      GlgAirTrafficDemo.StandAlone = true;
      GlgAirTrafficDemo air_traffic_demo = new GlgAirTrafficDemo();      

      // Process command line arguments.
      air_traffic_demo.ProcessArgs( arg );

      frame.getContentPane().add( air_traffic_demo );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );

      /* Assign a drawing filename after the frame became visible and 
         determined its client size to avoid unnecessary resizing of 
         the drawing.
         Loading the drawing triggers ReadyCallback which starts updates.
      */
      air_traffic_demo.SetDrawingName( "airtraffic.g" );
   }

   //////////////////////////////////////////////////////////////////////////
   // Processes command line arguments.
   //////////////////////////////////////////////////////////////////////////
   public void ProcessArgs( String [] arg )
   {
      if( arg == null )
        return;

      int num_arg = arg.length;
      if( num_arg != 0 )
      {
         for( int skip = 0; skip < num_arg; ++skip )
         {
            if( arg[ skip ].equals( "-num_planes" ) )
            {
               ++skip;
               if( num_arg <= skip )
                 error( "Missing number of planes.", true );

               try
               {
                  NumPlanes = Integer.parseInt( arg[ skip ] );
               }
               catch( NumberFormatException e )
               {
                  error( "Invalid number of planes.", true );
               }

               if( NumPlanes > MAX_NUM_PLANES )
                 error( "Increase plane array size and run again.", true );	   
            }
            else if( arg[ skip ].equals( "-speed" ) )
            {
               ++skip;
               if( num_arg <= skip )
                  error( "Missing plane speed.", true );

               try
               {
                  PlaneSpeed = Double.parseDouble( arg[ skip ] );
               }
               catch( NumberFormatException e )
               {
                  error( "Invalid plane speed.", true );
               }
            }
            else if( arg[ skip ].equals( "-ortho" ) )
            {
              OrthoOnly = true;
            }
            else if( arg[ skip ].equals( "-sync" ) )
            {
               ASYNC_MAP = false;
            }
            else if( arg[ skip ].equals( "-async" ) )
            {
               ASYNC_MAP = true;
            }
            else if( arg[ skip ].equals( "-map_server_url" ) )
            {
               ++skip;
               if( num_arg <= skip )
                 error( "Missing map server URL.", true );

               GlgAirTrafficDemo.SuppliedMapServerURL = arg[ skip ];
            }
            else if( arg[ skip ].equals( "-help" ) )
            {
               error( "Options: -num_planes <number> -speed <number> -ortho -map_server_url <URL> -sync -async\n" + 
                      "Defaults: -num_planes 100 -speed 0.005 -sync", true );
            }
            else
            {
               error( "Invalid option. Use -help for the list of options.", 
                      true );
            }
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked before the hierarchy setup.
   //////////////////////////////////////////////////////////////////////////
   public void HCallback( GlgObject viewport )
   {
      SetDResource( "$config/GlgAsyncImageLoading", ASYNC_MAP ? 1.0 : 0.0 );

      Drawing = viewport;
      Map = Drawing.GetResourceObject( "Map" );   
      FlightInfoPopup = Map.GetResourceObject( "FlightInfoPopup" );
      GISObject = Map.GetResourceObject( "GISObject" );

      /* Get ID of the GIS Object's GISArray, which holds all icons displayed
         on top of the map in lat/lon coordinates.
      */
      GISArray = GISObject.GetResourceObject( "GISArray" );

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
      String icon_name;
      int i;

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

      // Comment out the next two lines to enable the map.
      GISObject.SetDResource( "GISDisabled", 1.0 );
      Map.SetDResource( "JavaSetupInfo/Visibility", 1.0 );
      ////////////////////////////////////////////////////////////////////////

      // Try to use the GLG map server on the localhost if no URL was supplied.
      if( SuppliedMapServerURL == null )
        SuppliedMapServerURL = "http://localhost/cgi-bin/GlmScript";
      
      if( SuppliedMapServerURL != null )
        // Override the URL defined in the drawing.
        GISObject.SetSResource( "GISMapServerURL", SuppliedMapServerURL );

      /* Set the GIS Zoom mode. It was set and saved with the drawing, 
         but do it again programmatically just in case.
      */
      Map.SetZoomMode( null, GISObject, null, GlgObject.GIS_ZOOM_MODE );

      // Query and store the GIS projection (ORTHOGRAPHIC or RECTANGULAR)
      int projection = 
        GISObject.GetDResource( "GISProjection" ).intValue();
      MapProjection = projection;

      // Query and store GIS object's Extent and Center.
      InitExtent = GISObject.GetGResource( "GISExtent" );
      InitCenter = GISObject.GetGResource( "GISCenter" );

      // Make popup dialog and distance popup invisible.
      FlightInfoPopup.SetDResource( "Visibility", 0.0 );
      Map.SetDResource( "DistancePopup/Visibility", 0.0 );

      /* Disable color button: this functionality is not supported with
         the remote map sever setup.
      */
      Drawing.SetDResource( "ToggleColor/HandlerDisabled", 1.0 );	  

      // Get the palette containing templates for plane and node icons.
      GlgObject palette = Drawing.GetResourceObject( "Palette" );

      // Delete it from the drawing
      Drawing.DeleteObject( palette );

      /* Get node and plane templates from the palette. A few sets of templates
         with different level of details are used, depending on the zoom level.
         Palette aproach is used to implement icon types instead of 
         subdrawings, since icons are dynamically added/deleted from the 
         drawing any way to show only the icons visible in the zoomed area.
      */
      for( i=0; i < NUM_PLANE_TYPES; ++i )
      {
         icon_name = "Plane" + i;
         PlaneTemplate[ i ] = palette.GetResourceObject( icon_name );

         // Turn labels on initially. 
         if( PlaneTemplate[ i ].GetResourceObject( "Label" ) != null )
           PlaneTemplate[ i ].SetDResource( "Label/Visibility", 1.0 );
      }

      TrajectoryTemplate = palette.GetResourceObject( "Trajectory" );
      NumTrajectoryPoints = 
        TrajectoryTemplate.GetDResource( "Factor" ).intValue();

      /* Small filled circles don't look nice in a native Java renderer.
         Add EDGE to make them look nicer.
      */
      TrajectoryTemplate.SetDResource( "Marker/MarkerType", (double)
                       ( GlgObject.FILLED_CIRCLE | GlgObject.CIRCLE ) );

      for( i=0; i < NUM_NODE_TYPES; ++i )
      {
         icon_name = "Node" + i;
         NodeTemplate[ i ] = palette.GetResourceObject( icon_name );
      }

      NumNodes = NodeArray.length;

      // Create and initialize plane structures used for simulation.
      PlaneArray = new PlaneData[ NumPlanes ];
      for( i =0; i < NumPlanes; ++i )
      {
         PlaneArray[ i ] = new PlaneData();
         PlaneArray[ i ].name = Integer.toString( i );
         PlaneArray[ i ].tooltip = null;
         PlaneArray[ i ].color_index = NORMAL;
         PlaneArray[ i ].graphics = null;
         PlaneArray[ i ].trajectory = null;
         PlaneArray[ i ].iteration = 0;

         StartPlane( PlaneArray[ i ], true );
      }

      // Create groups to hold nodes and planes.
      NodeGroup = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
      NodeGroup.SetSResource( "Name", "NodeGroup" );

      TrajectoryGroup = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
      TrajectoryGroup.SetSResource( "Name", "TrajectoryGroup" );

      PlaneGroup = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
      PlaneGroup.SetSResource( "Name", "PlaneGroup" );

      /* Add all icons to the GIS object, so that the icon's position may be
         defined in lat/lon. The GIS object handles all details of the 
         GIS coordinate conversion.
      */
      GISObject.AddObjectToBottom( NodeGroup );
      GISObject.AddObjectToBottom( PlaneGroup );
      GISObject.AddObjectToBottom( TrajectoryGroup );

      // Create groups to keep pooled objects.
      for( i=0; i<NUM_NODE_TYPES; ++i )
        NodePool[i] = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );

      for( i=0; i<NUM_PLANE_TYPES; ++i )
         PlanePool[i] = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );

      TrajectoryPool = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );

      SetPlaneSize();

      /* Demos starts with the whole word view, then zooms to the Florida area
         in a few seconds to show more details. Set initial parameters
         for the whole world view.
      */
      HandleZoomLevel();

      // Store objects used to display lat/lon on mouse move.
      PositionArea = Drawing.GetResourceObject( "PositionArea" );
      PositionObject = Drawing.GetResourceObject( "PositionLabel/String" );
      PositionObject.SetSResource( null, "" );

      // Set Florida zooming message to OFF initially.
      Drawing.SetDResource( "Map/FloridaZoomingMessage/Visibility", 0.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked after the hierarchy setup.
   //////////////////////////////////////////////////////////////////////////
   public void VCallback( GlgObject viewport )
   {
      // Adds visible icons to the drawing and handles zoom level.
      UpdateObjectsOnMap();

      if( ASYNC_MAP )
        SetMapLoading( true, "Loading map..." );
      else
        SetMapLoading( false, "" );
   }
   
   //////////////////////////////////////////////////////////////////////////
   public void SetMapLoading( boolean show, String message )
   {
      if( show )
      {
         SetStatus( message );
         Map.SetSResource( "LoadingMessage/String", message );
         Map.SetDResource( "LoadingMessage/Visibility", 1.0 );
         MapLoaded = false;
      }
      else
      {
         SetStatus( "" );
         Map.SetDResource( "LoadingMessage/Visibility", 0.0 );
         MapLoaded = true;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Starts updates
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );

      StartUpdates();

      // Zoom to the Florida area after a few seconds to show details.        
      zoom_timer = new Timer( FloridaZoomDelay1, new ZoomPerformer( this ) );
      zoom_timer.setRepeats( false );
      zoom_timer.start();
   }   

   //////////////////////////////////////////////////////////////////////////
   // Makes sure the icons visible at the current zoom level are added
   // to the drawing, keeping the rest of the icons in the object pool.
   // Also handles the use of more detailed icons depending on the zoom 
   // level.
   // The object pools are used to handle huge number (thousands and tens of 
   // thousands) of icons efficiently. For a smaller number of icons, icons
   // may always be kept in the drawing and this function is not needed.
   //////////////////////////////////////////////////////////////////////////
   public void UpdateObjectsOnMap()
   {
      int i;

      if( RedoIcons )      // Zoom or pan.
      {
         DeleteNodes();
         DeletePlanes();

         HandleZoomLevel();
      }

      /* Update the GIS object with new extent but don't draw it yet:
         we want to position icons on the map first.
      */
      Map.SetupHierarchy();

      for( i = 0; i < NumNodes; ++i )      // Position nodes
         PositionNode( NodeArray[ i ], i );

      /* Add trajectories before positioning planes to setup trajectories' 
         history.
      */
      if( RedoIcons )
        GISObject.AddObjectToBottom( TrajectoryGroup );

      for( i = 0; i < NumPlanes; ++i )     // Position planes
         PositionPlane( PlaneArray[ i ], i );

      if( RedoIcons )
      {
         GISObject.AddObjectToBottom( NodeGroup );
         GISObject.AddObjectToBottom( PlaneGroup );
         RedoIcons = false;
      }

      SelectPlane( SelectedPlaneIndex );
   }

   //////////////////////////////////////////////////////////////////////////
   // Set the node type depending on zoom level. Also change to rectangular
   // projection for high zoom factors.
   //////////////////////////////////////////////////////////////////////////
   void HandleZoomLevel()
   {
      GlgPoint extent = GetExtentDegrees();

      if( extent.x < 20.0 && extent.y < 20.0 )
      {
         // High Zoom: use the most detailed icon.
         NodeType = 1;
         PlaneType = 2;         // Most detailed icon
         HasAngle = true;       // Most detailed plane icons show angle
         HasElevation = true;   // Most detailed plane icons show elevation
         CityLabels = true;     // Use city names instead of airport labels.
         Layers = "default_air";
      }
      else if( extent.x < 70.0 && extent.y < 70.0 )
      {
         // Zoom: use detailed icon 1 
         NodeType = 1;
         PlaneType = 1;         // Detailed icon 
         HasAngle = true;       // Detailed plane icons show angle 
         HasElevation = false;  
         CityLabels = true;     // Use city names instead of airport labels.
         Layers = "default_air";
      }
      else
      {
         // Whole world view 
         NodeType = 1;          // City icons are always visible 
         PlaneType = 0;         // Simple icon 
         HasAngle = false;
         HasElevation = false;
         CityLabels = false;   // Use airport labels instead of all city names.
         Layers = "default_gis,grid70,outline";
      }

      SetGISLayers();     // Set airport labels.

      if( !OrthoOnly )
         ChangeProjection( extent );
   }

   //////////////////////////////////////////////////////////////////////////
   // Change projection to rectangular for zoomed views, and back to 
   // orthographics for high-level views.
   //////////////////////////////////////////////////////////////////////////
   void ChangeProjection( GlgPoint extent )
   {
      if( extent.x < 30.0 && extent.y < 30.0 )
      {
         if( MapProjection == GlgObject.RECTANGULAR_PROJECTION )
            return;    // Already rect, no change.

         // Change to the rectangular projection
         MapProjection = GlgObject.RECTANGULAR_PROJECTION;

         GISObject.SetDResource( "GISProjection", (double) MapProjection );
         // Set extent in degrees
         GISObject.SetGResource( "GISExtent", extent );
      }
      else
      {
         if( MapProjection == GlgObject.ORTHOGRAPHIC_PROJECTION )
            return;    // Already ortho, no change

         // Change to the orthographic projection
         MapProjection = GlgObject.ORTHOGRAPHIC_PROJECTION;

         GISObject.SetDResource( "GISProjection", (double) MapProjection );
      
         // Set extent in meters
         GISObject.SetGResource( "GISExtent", 
                      extent.x / 90.0 * GlgObject.EQUATOR_RADIUS,
                      extent.y / 90.0 * GlgObject.POLAR_RADIUS, 0.0 );
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Handle user interaction.
   //////////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject viewport, GlgObject message_obj )
   {
      String
        origin,
        format,
        action,
        subaction;

      super.InputCallback( viewport, message_obj );

      origin = message_obj.GetSResource( "Origin" );
      format = message_obj.GetSResource( "Format" );
      action = message_obj.GetSResource( "Action" );
      subaction = message_obj.GetSResource( "SubAction" );

      if( format.equals( "Button" ) )         // Handle button clicks
      {
         if( !action.equals( "Activate" ) )
            return;

         PanMode = false;       // Abort Pan mode

         if( DistanceMode )
         {
            AbortDistanceMode();
            
            /* Second click on the Distance button: cancel DistanceMode in 
               progress.
            */
            if( origin.equals( "Distance" ) )
              return;
         }
 
         if( origin.equals( "CloseDialog" ) )
         {
            Drawing.SetDResource( "SelectionDialog/Visibility", 0.0 );
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
            Update();	 
         }
         else if( origin.equals( "ZoomOut" ) )
         {
            Zoom( 'o', 2.0 );
            Update();	 
         }
         else if( origin.equals( "ZoomReset" ) )
         {	
            Zoom( 'n', 0.0 );
            Update();	 
         }
         else if( origin.equals( "ZoomTo" ) )
         {
            Map.SetZoom( null, 't', 0.0 );  // Start Zoom op
            SetStatus( "Define a rectangular area to zoom to." );
            Update();	 
         }
         else if( origin.equals( "Pan" ) )
         {	    
            Map.SetZoom( null, 'e', 0.0 );  // Abort ZoomTo/Drag mode

            PanMode = true;
            SetStatus( "Click to define a new center." );
            Update();	 
         }
         else if( origin.equals( "Drag" ) )
         {
            /* Activate dragging mode. Dragging will start on the mouse click. 
               If no object of interest is selected by the mouse click, 
               dragging will be started by the code in the Trace callback 
               anyway, but only if no object of interest was selected. 
               The "Drag" button demostrates an alternative way to start 
               dragging from a button which starts dragging even if an object 
               of interest is selected by the mouse click.
            */
            DraggingFromButton = true;
            Map.SetZoom( null, 's', 0.0 );
            SetStatus( "Click and drag the map with the mouse." );
            Update();
         }
         else if( origin.equals( "AirportLabels" ) )
         {
            CityLabels = !CityLabels;
            SetGISLayers();
            Update();	 
         }
         else if( origin.equals( "Planes" ) )
         {
            ToggleResource( Map, "PlaneGroup/Visibility" );
            Update();	 
         }
         else if( origin.equals( "ValueDisplay" ) )
         {
            if( PlaneType == 0 )
            {
               GlgObject.Bell();
               SetStatus( "Zoom in to see plane labels." );
            }
            else
            {
               // Visibility of all labels is constrained, set just one.
               for( int i=1; i<NUM_PLANE_TYPES; ++i )
                  ToggleResource( PlaneTemplate[ i ], "Label/Visibility" );
            }
            Update();	 
         }
         else if( origin.equals( "ToggleStates" ) )
         {
            StateDisplay = !StateDisplay;
            SetGISLayers();
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
         else if( origin.equals( "CloseFlightInfo" ) )
         {
            SelectPlane( -1 ); // Unselect the plane and erase popup display.
            Update();	 
         }	
         else if( origin.equals( "Distance" ) )
         {
            DistanceMode = true;
            SetStatus( "Click on the map to define distance to measure, right click to finish." );
            Update();	 
         }	
      }
      /* Process mouse clicks on plane icons, implemented as an Action with
         the Plane label attached to an icon and activated on a mouse click. 
      */
      else if( format.equals( "CustomEvent" ) )
      {	
         String event_label;

         if( DistanceMode )
           return;  // Ignore selection in the Distance mode

         event_label = message_obj.GetSResource( "EventLabel" );
         
         if( event_label.equals( "Plane" ) )  // Plane icon was selected
         {
            int plane_index;

            /* Map dragging mode is activated either on a mouse click in the 
               trace callback, or with the Drag toolbar button. 
               Abort the dragging mode if an object with custom event was 
               selected and the dragging was activated on a mouse click. 
               This gives custom events a higher priority compared to the 
               dragging mode. If it's a ZoomTo mode activated by a button or 
               dragging activated from the Drag button, don't abort and ignore
               object selection.
            */
            int zoom_mode = ZoomToMode();
            if( zoom_mode == 0 ||
                ( zoom_mode & GlgObject.PAN_DRAG_STATE ) != 0 && !DraggingFromButton )
            {
               if( zoom_mode != 0 )
                 Map.SetZoom( null, 'e', 0.0 );  /* Abort zoom mode */
            
               // Get plane index 
               plane_index = 
                 message_obj.GetDResource( "Object/DataIndex" ).intValue();
               SelectPlane( plane_index );
               
               // Show message in the bottom
               SetStatus( PlaneArray[ SelectedPlaneIndex ].tooltip );
               
               DisplayPlaneInfo();   // Display popup
               Update();
            }
         }
      }
      else if( action.equals( "Zoom" ) )
      {
         if( subaction.equals( "End" ) )
         {
            // Update icon positions after zooming. 
            RedoIcons = true;
            UpdateObjectsOnMap();
            Update();
            
            if( ASYNC_MAP )
              SetMapLoading( true, "Loading map..." );
         }
         else if( subaction.equals( "Abort" ) )
           SetStatus( "" );
      }
      else if( action.equals( "Pan" ) )
      {
         if( subaction.equals( "Start" ) )   // Map dragging start
         {
            SetStatus( "Drag the map with the mouse." );
         }
         else if( subaction.equals( "Drag" ) )    // Dragging
         {
            DraggingFromButton = false;

            if( ASYNC_MAP )
              SetMapLoading( true, "Loading map..." );
            SetStatus( "Dragging the map with the mouse...." );

            // Update icon positions when scrolling.
            RedoIcons = true;
            UpdateObjectsOnMap();
            Update();         
         }
         else if( subaction.equals( "ValueChanged" ) )   // Scrollbars
         {
            // Update icon positions after zooming.
            RedoIcons = true;
            UpdateObjectsOnMap();
            Update();         
         }
         // Dragging ended or aborted (right mouse button, etc.). 
         else if( subaction.equals( "End" ) || subaction.equals( "Abort" ) )
         {
            DraggingFromButton = false;
            SetStatus( "" );   // Reset prompt when dragging ends.
         }     
      }   
   }

   //////////////////////////////////////////////////////////////////////////
   int ZoomToMode()
   {
      return Map.GetDResource( "ZoomToMode" ).intValue();
   }

   //////////////////////////////////////////////////////////////////////////
   // Used to obtain coordinates of the mouse click. 
   //////////////////////////////////////////////////////////////////////////
   public void TraceCallback( GlgObject viewport, GlgTraceData trace_info )
   {      
      GlgObject point_obj;
      
      int event_type = trace_info.event.getID();

      // Use the Map area events only.
      if( trace_info.viewport != Map )
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
       case MouseEvent.MOUSE_PRESSED:
         if( ZoomToMode() != 0 )
           return; // ZoomTo or dragging mode in progress: pass it through.

         if( GetButton( trace_info.event ) != 1 )
         {
            AbortDistanceMode();	 
            return;  // Use the left button clicks only.
         }
         
         double x = (double) ((MouseEvent)trace_info.event).getX();
         double y = (double) ((MouseEvent)trace_info.event).getY();

         /* COORD_MAPPING_ADJ is added to the cursor coordinates for 
            precise pixel mapping.
         */
         x += GlgObject.COORD_MAPPING_ADJ;
         y += GlgObject.COORD_MAPPING_ADJ;
         
         /* Handle paning: set the new map center to the location of 
            the click.
         */
         if( PanMode )
         {      
            PanMode = false;

            // Converts X/Y to lat/lon using GIS object's current projection.
            point.x = x;
            point.y = y;
            point.z = 0.0;
            GISObject.GISConvert( null, GlgObject.SCREEN_COORD,
                                 /* X/Y to Lat/Lon */ true, point, lat_lon );

            // Pan the map
            GISObject.SetGResource( "GISCenter", lat_lon );
            RedoIcons = true;
            UpdateObjectsOnMap();
            Map.Update();

            if( ASYNC_MAP )
              SetMapLoading( true, "Loading map..." );
         }
         else if( DistanceMode )
         { 
            /* The world coordinates inside the GIS Object's GISArray are 
               in lat/lon.
            */
            point.x = x;
            point.y = y;
            point.z = 0.0;
            GISArray.ScreenToWorld( true, point, lat_lon );
               
            if( DistancePolygon == null )
            {
               DistancePolygon = new GlgPolygon( 2, null );
               DistancePolygon.SetGResource( "EdgeColor", 1.0, 1.0, 0.0 );
               NumDistancePoints = 1;
               
               point_obj = (GlgObject) DistancePolygon.GetElement( 0 );
               point_obj.SetGResource( null, lat_lon );
               
               point_obj = (GlgObject) DistancePolygon.GetElement( 1 );
               point_obj.SetGResource( null, lat_lon );
               
               GISObject.AddObjectToBottom( DistancePolygon );
               Map.Update();	    
            }
            else // Not the first point
            {
               // Set current point to the coord. of the click.
               point_obj = (GlgObject) 
                 DistancePolygon.GetElement( NumDistancePoints );
               point_obj.SetGResource( null, lat_lon );
               ++NumDistancePoints;
               
               DisplayDistance( DistancePolygon );
               
               // Add next point, same coords.
               point_obj = point_obj.CopyObject();
               DistancePolygon.AddObjectToBottom( point_obj );
               
               Map.Update();
            }
         }
         else 
           // Not a Pan or Distance mode: start dragging the map with the mouse.
           Map.SetZoom( null, 's', 0.0 );
         break;
         
       case MouseEvent.MOUSE_MOVED:
       case MouseEvent.MOUSE_DRAGGED:
         point.x = (double) ((MouseEvent)trace_info.event).getX();
         point.y = (double) ((MouseEvent)trace_info.event).getY();
         point.z = 0;
         
         /* COORD_MAPPING_ADJ is added to the cursor coordinates for 
            precise pixel mapping.
         */
         point.x += GlgObject.COORD_MAPPING_ADJ;
         point.y += GlgObject.COORD_MAPPING_ADJ;

         /* The world coordinates inside the GIS Object's GISArray are 
            in lat/lon.
         */
         GISArray.ScreenToWorld( true, point, lat_lon );

         if( DistanceMode && DistancePolygon != null )
         {
            point_obj = (GlgObject)
              DistancePolygon.GetElement( NumDistancePoints );
            point_obj.SetGResource( null, lat_lon );
            DisplayDistance( DistancePolygon );
            Map.Update();
         }

         // Display lat/lon of a point under the mouse.
         PositionObject.SetSResource( null, CreateLocationString( lat_lon ) );
         PositionArea.Update();
         break;
         
       case ComponentEvent.COMPONENT_RESIZED:
         // No need to adjust icon positions: GIS Object handles it.
         // Component widget = (Component) Drawing.GetResource( "Widget" );
         // int width = widget.getSize().width;
         // int height = widget.getSize().height;
         break;
         
       default:
         return;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void AbortDistanceMode()
   {
      if( DistanceMode )
      {
         if( DistancePolygon != null )   // Delete distance polygon
         {
            if( GISObject.ContainsObject( DistancePolygon ) )
               GISObject.DeleteObject( DistancePolygon );
            
            DistancePolygon = null;
         }
         Map.SetDResource( "DistancePopup/Visibility", 0.0 );
         SetStatus( "" );
         DistanceMode = false;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void DisplayDistance( GlgObject polygon )
   {
      // Popup distance display
      Map.SetDResource( "DistancePopup/Visibility", 1.0 );
      
      // Last point is for dragging, not set yet - don't include.
      int size = polygon.GetSize();
      if( size < 2 )
         return;
      
      double distance = 0.0;
      GlgObject point = null;
      for( int i=0; i<size; ++i )
      {
         GlgObject last_point = point;
         point = (GlgObject) polygon.GetElement( i );
            
         if( last_point != null )
            // Nautical mile = 1842m 
            distance += GetGlobeDistance( point, last_point ) / 1842.0;
      }
      
      Map.SetDResource( "DistancePopup/Distance", distance );
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Returns the length (in meters) of the shortest arc along the earth 
   // surface connecting the two points.
   //////////////////////////////////////////////////////////////////////////
   double GetGlobeDistance( GlgObject point1_obj, GlgObject point2_obj )
   {
      GlgPoint lat_lon1, lat_lon2, globe_point1, globe_point2;
      
      lat_lon1 = point1_obj.GetGResource( null );
      lat_lon2 = point2_obj.GetGResource( null );
      
      // XYZ of the first point, in meters.
      globe_point1 = GetPointXYZ( lat_lon1 );
      
      // XYZ of the second point, in meters.
      globe_point2 = GetPointXYZ( lat_lon2 );
      
      double dx = globe_point1.x - globe_point2.x;
      double dy = globe_point1.y - globe_point2.y;
      double dz = globe_point1.z - globe_point2.z;

      // Length of a straight line between the points.
      double straight_dist = Math.sqrt( dx * dx + dy * dy + dz * dz );

      // Use the average value.
      double globe_radius = 
        ( GlgObject.EQUATOR_RADIUS + GlgObject.POLAR_RADIUS ) / 2.0;

      // The length of the shortest connecting arc along the earth surface. 
      double arc_dist = 2.0 * globe_radius * 
        Math.asin( straight_dist / ( 2.0 * globe_radius ) );
   
      return arc_dist;
   }

   //////////////////////////////////////////////////////////////////////////
   GlgPoint GetPointXYZ( GlgPoint lat_lon )
   {
      double
         angle_x,
         angle_y;
   
      GlgPoint xyz = new GlgPoint();

      /* For simplicity, place the origin at the intersection of the x axis 
         with the globe surface. */
      
      angle_x = DegToRad( lat_lon.x );
      angle_y = DegToRad( lat_lon.y );
      
      xyz.x = 
        GlgObject.EQUATOR_RADIUS * Math.cos( angle_x ) * Math.cos( angle_y );
      xyz.y = GlgObject.EQUATOR_RADIUS * Math.sin( angle_y );
      xyz.z =
        GlgObject.EQUATOR_RADIUS * Math.sin( angle_x ) * Math.cos( angle_y );

      return xyz;
   }

   //////////////////////////////////////////////////////////////////////////
   void Zoom( char type, double value )
   {
      switch( type )
      {
       default:
         Map.SetZoom( null, type, value );
         CheckScrollLimits( type );

         RedoIcons = true;
         UpdateObjectsOnMap();
         break;
         
       case 'n':
         // Reset map to the initial extent.
         MapProjection = GlgObject.ORTHOGRAPHIC_PROJECTION;
         GISObject.SetDResource( "GISProjection",
                                (double) MapProjection );
         GISObject.SetGResource( "GISCenter", InitCenter );
         GISObject.SetGResource( "GISExtent", InitExtent );
         
         RedoIcons = true;
         UpdateObjectsOnMap();
         break;
      }

      if( ASYNC_MAP )
        SetMapLoading( true, "Loading map..." );
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
      
      if( MapProjection == GlgObject.ORTHOGRAPHIC_PROJECTION )
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
      
      extent = GISObject.GetGResource( "GISExtent" );
      center = GISObject.GetGResource( "GISCenter" );
   
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
        GISObject.SetGResource( "GISCenter", center );
   }

   ////////////////////////////////////////////////////////////////////////
   // Changes plane color to indicate selection and displayes or erases the 
   // flight info popup dialog.
   ////////////////////////////////////////////////////////////////////////
   void SelectPlane( int selected_plane_index )
   {
      PlaneData plane;
      double popup_visibility;
      
      if( SelectedPlaneIndex != -1 )   // Unselect previously selected plane
      {
         plane = PlaneArray[ SelectedPlaneIndex ];
         if( plane.graphics != null )    // Restore color if plane is visible 
            plane.graphics.SetDResource( "ColorIndex",
                             (double) plane.color_index );
      }
      
      if( selected_plane_index != -1 )   // Select new plane
      {
         plane = PlaneArray[ selected_plane_index ];
         if( plane.graphics != null ) // Set selected color if plane is visible
            plane.graphics.SetDResource( "ColorIndex", (double) SELECTED );
      }
   
      // Display or erase the flight info popup.
      if( selected_plane_index == -1 )    // Unselected
         popup_visibility = 0.0;
      else   // Selected
         popup_visibility = 1.0;
      FlightInfoPopup.SetDResource( "Visibility", popup_visibility );
      
      SelectedPlaneIndex = selected_plane_index;
   }

   ////////////////////////////////////////////////////////////////////////
   void DisplayPlaneInfo()
   {
      PlaneData plane = PlaneArray[ SelectedPlaneIndex ];
      FlightInfoPopup.SetSResource( "FlightInfo", plane.tooltip );
      FlightInfoPopup.SetDResource( "Elevation", GetPlaneElevation( plane ) );
      FlightInfoPopup.SetDResource( "StatusIndex", (double) plane.color_index );
      
      FlightInfoPopup.SetSResource( "Location", 
                                    CreateLocationString( plane.lat_lon ) );
   }
   
   ////////////////////////////////////////////////////////////////////////
   // Delete node icons and place them into the object pool. 
   ////////////////////////////////////////////////////////////////////////
   void DeleteNodes()
   {
      GlgObject icon;
      int i;

      // Move node icons into the object pool. 
      int size = NodeGroup.GetSize();
      for( i=0; i<size; ++i )
      {
         icon = (GlgObject) NodeGroup.GetElement( i );
         NodePool[ NodeType ].AddObjectToBottom( icon );
      }

      // Delete node icons from the drawing and node group 
      GISObject.DeleteObject( NodeGroup );
      for( i=0; i<size; ++i )
         NodeGroup.DeleteBottomObject();
      
      // Set nodes' graphics to null 
      if( size > 0 )
        for( i=0; i<NumNodes; ++i )
          NodeArray[i].graphics = null;
   }

   ////////////////////////////////////////////////////////////////////////
   // Delete plane icons and place them into the object pool. 
   ////////////////////////////////////////////////////////////////////////
   void DeletePlanes()
   {
      GlgObject icon;
      int i;
          
      // Move plane icons into the object pool.
      int size = PlaneGroup.GetSize();
      for( i=0; i<size; ++i )
      {
         icon = (GlgObject) PlaneGroup.GetElement( i );
         PlanePool[ PlaneType ].AddObjectToBottom( icon );
      }
      
      // Delete plane icons from the drawing and plane group 
      GISObject.DeleteObject( PlaneGroup );
      for( i=0; i<size; ++i )
        PlaneGroup.DeleteBottomObject();
      
      // Set planes' graphics to null 
      if( size > 0 )
        for( i=0; i<NumPlanes; ++i )
          PlaneArray[i].graphics = null;
      
      DeleteTrajectories();
   }

   ////////////////////////////////////////////////////////////////////////
   // Delete trajectory objects and place them into the object pool. 
   ////////////////////////////////////////////////////////////////////////
   void DeleteTrajectories()
   {
      GlgObject icon;
      int i;

      // Move plane icons into the object pool.
      int size = TrajectoryGroup.GetSize();
      for( i=0; i<size; ++i )
      {	
         icon = (GlgObject) TrajectoryGroup.GetElement( i );
         TrajectoryPool.AddObjectToBottom( icon );
      }
      
      // Delete trajectory icons from the drawing and trajectory group
      GISObject.DeleteObject( TrajectoryGroup );
      for( i=0; i<size; ++i )
        TrajectoryGroup.DeleteBottomObject();
      
      // Set trajectorys' graphics to null
      if( size > 0 )
        for( i=0; i<NumPlanes; ++i )
          PlaneArray[i].trajectory = null;
   }

   ////////////////////////////////////////////////////////////////////////
   void PositionNode( NodeData node, int index )
   {
      if( !IconVisible( node.lat_lon ) )
         return;
      
      // Add node's graphics to the drawing if first time.
      if( node.graphics == null )
         AddNodeGraphics( node, NodeType, index );
      
      /* Position node's icon. Since the icons are added as children of the 
         GIS Object, their coordinates are specified in lat/lon. The GIS Object
         handles all details of coordinate convesion.
      */
      node.graphics.SetGResource( "Position", node.lat_lon );
   }

   ////////////////////////////////////////////////////////////////////////
   GlgObject CreateNodeIcon( int node_type )
   {
      GlgObject icon;
      
      int size = NodePool[ node_type ].GetSize();
      if( size > 0 )   // Return an icon from the pool
      {
         icon = (GlgObject) NodePool[ node_type ].GetElement( size - 1 );
         NodePool[ node_type ].DeleteBottomObject();
      }
      else   // Create a new icon
      {
         icon = 
           NodeTemplate[ node_type ].CloneObject( GlgObject.STRONG_CLONE );
      }
      return icon;
   }
   
   ////////////////////////////////////////////////////////////////////////
   GlgObject CreatePlaneIcon( int plane_type )
   {
      GlgObject icon;

      int size = PlanePool[ plane_type ].GetSize();
      if( size > 0 )   // Return an icon from the pool
      {
         icon = (GlgObject) PlanePool[ plane_type ].GetElement( size - 1 );
         PlanePool[ plane_type ].DeleteBottomObject();
      }
      else   // Create a new icon 
      {
         icon = 
           PlaneTemplate[ plane_type ].CloneObject( GlgObject.STRONG_CLONE );
      }
      return icon;
   }

   ////////////////////////////////////////////////////////////////////////
   GlgObject CreateTrajectoryIcon()
   {
      GlgObject icon;
      
      int size = TrajectoryPool.GetSize();
      if( size > 0 )   // Return an icon from the pool
      {
         icon = (GlgObject) TrajectoryPool.GetElement( size - 1 );
         TrajectoryPool.DeleteBottomObject();
      }
      else   // Create a new icon 
      {
         icon = TrajectoryTemplate.CloneObject( GlgObject.STRONG_CLONE );
      }
      return icon;
   }

  ////////////////////////////////////////////////////////////////////////
   void PositionPlane( PlaneData plane, int index )
   {
      // Gets the new plane's position, simulated or from real data.
      GetPlaneLatLon( plane );
      
      if( !IconVisible( plane.lat_lon ) )
      {
         // Delete graphics and place into the pool
         if( plane.graphics != null )
         {
            PlanePool[ PlaneType ].AddObjectToBottom( plane.graphics );
            PlaneGroup.DeleteObject( plane.graphics );
            plane.graphics = null;
         }

         // Delete trajectory and place into the pool
         if( plane.trajectory != null )
         {
            TrajectoryPool.AddObjectToBottom( plane.trajectory );
            TrajectoryGroup.DeleteObject( plane.trajectory );
            plane.trajectory = null;
         }
         return;
      }

      // Add the plane icon to the drawing if the first time.
      if( plane.graphics == null  )
        AddPlaneGraphics( plane, PlaneType, index );

      /* Position plane's icon. Since the icons are added as children of the 
         GIS Object, their coordinates are specified in lat/lon. The GIS Object
         handles all details of coordinate convesion.
      */
      plane.graphics.SetGResource( "Position", plane.lat_lon );
      
      // Update icon's direction angle if necessary.
      if( HasAngle )
        plane.graphics.SetDResource( "Angle", GetPlaneAngle( plane ) );

      if( HasElevation )
        plane.graphics.SetDResource( "Height", GetPlaneElevation( plane ) );

      if( plane.trajectory != null )
      {
         /* For small speeds, skip a few iterations to increase the 
            trajectory's length.
         */
         if( PlaneSpeed < 0.01 )  
         {
            int n = (int) ( 0.01 / PlaneSpeed );
            if( n != 0 )
            {
               ++plane.iteration;
               if( ( plane.iteration % n ) != 0 )
                  return;   // Skip n iterations, update every n'th
            }
         }      
      
         plane.trajectory.SetDResource( "VisEntryPoint", 1.0 );
         plane.trajectory.SetGResource( "XYEntryPoint", plane.lat_lon );
      }
   }
   
   ////////////////////////////////////////////////////////////////////////
   // Adds an airport icon, fills labels, tooltips, etc.
   ////////////////////////////////////////////////////////////////////////
   void AddNodeGraphics( NodeData node, int node_type, int index )
   {      
      GlgObject icon = CreateNodeIcon( node_type );
      
      // Index for direct access 
      icon.SetDResource( "DataIndex", (double)index );
      
      if( node_type > 0 )   // More detailed icon
         icon.SetSResource( "LabelString", node.name );
      
      String tooltip = node.name + ", " + CreateLocationString( node.lat_lon );
      icon.SetSResource( "TooltipString", tooltip );
      
      node.graphics = icon;
      
      // The node will be positioned after the GIS object is setup.
      
      NodeGroup.AddObjectToBottom( icon );
   }

   ////////////////////////////////////////////////////////////////////////
   // Adds a plane icon, fills labels, tooltips, etc.
   ////////////////////////////////////////////////////////////////////////
   void AddPlaneGraphics( PlaneData plane, int plane_type, int index )
   {
      GlgObject icon = CreatePlaneIcon( plane_type );
      
      // Index for direct access
      icon.SetDResource( "DataIndex", (double)index );           
      
      // Icon color
      icon.SetDResource( "ColorIndex", (double) plane.color_index );
      
      if( plane_type > 0 )   // More detailed icon
      {
         // Show the flight number as icon label
         String label = "Flight " + plane.flight_number;
         icon.SetSResource( "LabelString", label );
      }
      
      // Set the tooltip
      icon.SetSResource( "TooltipString", plane.tooltip );
      
      plane.graphics = icon;
      
      PlaneGroup.AddObjectToBottom( icon );
      
      if( plane_type == 2 )   // For detailed icon, create trajectory
      {
         icon = CreateTrajectoryIcon();
         plane.trajectory = icon;
         
         // Set entries invisible initially
         icon.SetDResource( "Marker/Visibility", 0.0 );
         
         TrajectoryGroup.AddObjectToBottom( icon );
         
         for( int i=0; i<NumTrajectoryPoints; ++i )  // Set fading
           icon.SetDResource( "BrightEntryPoint",
                             0.2 + 0.8 * i / (double) NumTrajectoryPoints );
      }
   }

   ////////////////////////////////////////////////////////////////////////
   // Check if the icon is visible in the current zoom region.
   ////////////////////////////////////////////////////////////////////////
   boolean IconVisible( GlgPoint lat_lon_pos )
   {
      // Converts lat/lon to X/Y using GIS object's current projection.
      GISObject.GISConvert( null, GlgObject.OBJECT_COORD,
                           /* Lat/Lon to XY */ false, lat_lon_pos, point );

      return point.z >= 0.0 &&
        point.x > -1100.0 && point.x < 1100.0 &&
        point.y > -1100.0 && point.y < 1100.0;
   }

   ////////////////////////////////////////////////////////////////////////
   // Displays a message in the status area.
   ////////////////////////////////////////////////////////////////////////
   void SetStatus( String message )
   {
      Drawing.SetSResource( "StatusLabel/String", message );
      Drawing.GetResourceObject("StatusArea").Update();
   }

   ////////////////////////////////////////////////////////////////////////
   void SetPlaneSize()
   {
      for( int i=0; i<NUM_PLANE_TYPES; ++i )
      {
         GlgObject resource = 
           PlaneTemplate[ i ].GetResourceObject( "IconScale" );
         if( resource != null )
         {
            // Polygon icon: set scale.
            resource.SetDResource( null, PlaneSize );
         }
         else
         {
            resource = GetResourceObject( PlaneTemplate[ i ], 
                                         "Marker/MarkerSize" );
            if( resource != null )
            {
               // Marker: set MarkerSize.
               if( PlaneSize == SMALL_SIZE )
                 resource.SetDResource( null, SMALL_MARKER_SIZE );
               else if( PlaneSize == MEDIUM_SIZE )
                 resource.SetDResource( null, MEDIUM_MARKER_SIZE );
               else
                 resource.SetDResource( null, BIG_MARKER_SIZE );
            }
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////
   // Toggle resource between 0 and 1.
   ////////////////////////////////////////////////////////////////////////
   void ToggleResource( GlgObject obj, String res_name )
   {
      GlgObject resource = obj.GetResourceObject( res_name );
      if( resource == null )
         return;
      
      double value = resource.GetDResource( null ).doubleValue();
      resource.SetDResource( null, value != 0.0 ? 0.0 : 1.0 );
   }

   ////////////////////////////////////////////////////////////////////////
   // Toggle map layers: airport/city labels and states.
   ////////////////////////////////////////////////////////////////////////
   void SetGISLayers()
   {
      String layers;

      // Airport labels should be visible only when city labels are off.
      NodeTemplate[1].SetDResource( "Label/Visibility", CityLabels ? 0.0 : 1.0 );

      layers = Layers;

      // Add city layers if they are on on the detailed map.
      if( CityLabels )
        layers = layers + ",us_cities";
      else
        layers = layers + ",-us_cities";

      if( StateDisplay )   // Add states layer if it is on.
        // Enable states regardless of the default.
        layers = layers + ",states_dcw";
      else
        // Disable state outline display.
        layers = layers + ",-states_dcw";

      GISObject.SetSResource( "GISLayers", layers );
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
   // Gets extent in lat/lon (converts from meters for ortho projection).
   ////////////////////////////////////////////////////////////////////////
   GlgPoint GetExtentDegrees()
   {   
      GlgPoint extent = GISObject.GetGResource( "GISExtent" );

      if( MapProjection == GlgObject.ORTHOGRAPHIC_PROJECTION )
      {
         extent.x = extent.x / GlgObject.EQUATOR_RADIUS * 90.0;
         extent.y = extent.y / GlgObject.POLAR_RADIUS * 90.0;
      }

      return extent;
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
   void error( String message_str, boolean quit )
   {
      System.out.println( message_str );
      if( quit )
        System.exit( 1 );
   }

   //////////////////////////////////////////////////////////////////////////
   // UTILITY FUNCTION: Calculates an angle between the line defined by two 
   // points and the X axis.
   //////////////////////////////////////////////////////////////////////////
   double GetAngle( GlgPoint pt1, GlgPoint pt2 )
   {
      double length, angle;
      
      length = GetLength( pt1, pt2 );
      
      if( length == 0.0  )
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
   // FOR FLIGHT SIMULATION ONLY: Converts plane lat/lon to the GLG world 
   //     coordinates for calculating plane speed and directional angle.
   //////////////////////////////////////////////////////////////////////////
   void GetPlanePosition( PlaneData plane, GlgPoint xyz )
   {
      // Converts lat/lon to X/Y using GIS object's current projection.
      GISObject.GISConvert( null, GlgObject.OBJECT_COORD, 
                           /* Lat/Lon to XY */ false, plane.lat_lon, xyz );
   }

   //////////////////////////////////////////////////////////////////////////
   // FOR FLIGHT SIMULATION ONLY: Converts node lat/lon to the GLG world 
   //     coordinates for calculating plane's initial directional angle.
   //////////////////////////////////////////////////////////////////////////
   void GetNodePosition( NodeData node, GlgPoint xyz )
   {
      // Converts lat/lon to X/Y using GIS object's current projection.
      GISObject.GISConvert( null, GlgObject.OBJECT_COORD, 
                           /* Lat/Lon to XY */ false, node.lat_lon, xyz );
   }

   //////////////////////////////////////////////////////////////////////////
   // FOR FLIGHT SIMULATION ONLY: Calculates plane icon's directional angle.
   //     In an application, it will query the plane's directional angle.
   //////////////////////////////////////////////////////////////////////////
   double GetPlaneAngle( PlaneData plane )
   {
      /* Rectangular projection preserves straight lines, we can use the 
         angle of the line connecting the start and end nodes. For the
         orthographic projection, use this case if the plane has just started
         and there is no previous position stored.
      */
      if( MapProjection == GlgObject.RECTANGULAR_PROJECTION ||
         plane.path_position == plane.path_position_last )   // Just started
      {
         GetNodePosition( plane.from_node, last_xyz );
         GetNodePosition( plane.to_node, curr_xyz );
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
         GetPlanePosition( plane, last_xyz );
      
         // Restore the plane's current position and get its coordinates.
         plane.path_position = stored_position;
         GetPlaneLatLon( plane );
         GetPlanePosition( plane, curr_xyz );
      }

      /* Calculate the angle of a line connecting the previous and 
         current position.
      */
      return GetAngle( last_xyz, curr_xyz );
   }

   ////////////////////////////////////////////////////////////////////////
   // FOR FLIGHT SIMULATION ONLY: Calculates plane icon's elevation.
   //     In an application, it will query the plane elevation.
   //
   // For the simulation, it calculated the elevation using zero at the start
   // and end of the path and the maximum elevation in the middle.
   ////////////////////////////////////////////////////////////////////////
   double GetPlaneElevation( PlaneData plane )
   {
      return ( 0.5 - Math.abs( plane.path_position - 0.5 ) ) * 2.0 * 10000.0;   
   }
   
   ////////////////////////////////////////////////////////////////////////
   // Updates moving icons with the new position data.
   ////////////////////////////////////////////////////////////////////////
   void UpdatePlanes()
   {
      if( timer == null )
        return;   // Prevents race conditions

      if( !MapLoaded && GISObject.GetDResource( "ImageLoaded" ).intValue() != 0 )
        SetMapLoading( false, "" );

      if( PerformUpdates )
      {     
         for( int i = 0; i < NumPlanes; ++i )
           UpdatePlane( PlaneArray[ i ], i );
         
         if( SelectedPlaneIndex != -1 )   // Update selected plane info if any 
           DisplayPlaneInfo();
         
         Update();
      }

      timer.start();   // Restart the update timer
   }

   ////////////////////////////////////////////////////////////////////////
   // FOR FLIGHT SIMULATION ONLY: Calculates new plane position using 
   // simulated data. In an application, it will query the plane's lat/lon.
   ////////////////////////////////////////////////////////////////////////
   void UpdatePlane( PlaneData plane, int index )
   {      
      if( plane.from_node == null || plane.to_node == null )
        return;    // Plane is not in the air - no start/destination node.
            
      // Finished the old path, start a new one.
      if( plane.path_position == 1.0 )
      {
         if( index == SelectedPlaneIndex )
            SelectPlane( -1 );   // Unselect the plane: it reached destination

         StartPlane( plane, false );
      }
      else  // Continue on the current path.
      {
         double speed = PlaneSpeed;

         // Store last position for calculating angle in ORTHO projection. 
         plane.path_position_last = plane.path_position;

         plane.path_position += plane.speed * speed;
         if( plane.path_position > 1.0 )
            plane.path_position = 1.0;
      }

      SetPlaneColor( plane );

      PositionPlane( plane, index );   // Position plane on the map
   }

   ////////////////////////////////////////////////////////////////////////
   // FOR FLIGHT SIMULATION ONLY: Simulate data to change plane color to show 
   //     warnings and alarms. In an application, it will query the plane's 
   //     status.
   ////////////////////////////////////////////////////////////////////////
   void SetPlaneColor( PlaneData plane )
   {
      int new_color_index = NORMAL;
      
      // Set random color
      double random_value = GlgObject.Rand( 0.0, 1.0 );
      if( plane.color_index == NORMAL )
      {
         if( random_value <= 0.999 )
            new_color_index = NORMAL;
         else if( random_value > 0.9999 )
            new_color_index = ALARM;
         else if( random_value > 0.999 )
            new_color_index = WARNING;     
      }
      else if( plane.color_index == WARNING )
      {
         if( random_value > 0.99 )
            new_color_index = NORMAL;
         else
            new_color_index = plane.color_index;   // Keep alarm for a while
      }
      else if( plane.color_index == ALARM )
      {
         if( random_value > 0.999 )
            new_color_index = NORMAL;
         else
            new_color_index = plane.color_index;   // Keep alarm for a while
      }

      if( plane.graphics != null && 
          new_color_index != plane.color_index )
         plane.graphics.SetDResource( "ColorIndex", (double)new_color_index );
      
      plane.color_index = new_color_index;
   }

   ////////////////////////////////////////////////////////////////////////
   // FOR FLIGHT SIMULATION ONLY: Starts simulation for a plane, selects its 
   // start and end nodes. 
   ////////////////////////////////////////////////////////////////////////
   void StartPlane( PlaneData plane, boolean init )
   {
      int
         to_index,
         from_index;
      
      if( NumNodes < 2 )
         error( "Less then two nodes: can't start planes.", true );

      from_index = (int) GlgObject.Rand( 0.0, NumNodes - 0.001 );
      do
         {
            to_index = (int) GlgObject.Rand( 0.0, NumNodes - 0.001 );
         } while( to_index == from_index );

      plane.from_node = NodeArray[ from_index ];
      plane.to_node = NodeArray[ to_index ];
      plane.flight_number = (int) GlgObject.Rand( 101.0, 1999.0 );
      plane.speed = GlgObject.Rand( 0.4, 1.0 );   // Vary plane speed 
      
      if( init )   // Init the demo: position randomly along the path 
      {
         plane.path_position = GlgObject.Rand( 0.1, 0.9 );
         plane.path_position_last = plane.path_position - 0.05;  // For angle
      }
      else         // Position at the beginning of the path 
      {
         plane.path_position = 0.0;
         plane.path_position_last = 0.0;
      }

      String flight_name = "Flight " + plane.flight_number;
      
      // Add from/to node info to the tooltip.
      plane.tooltip = flight_name + " from "  + 
         plane.from_node.name + " to " +  plane.to_node.name;

      // Set all trajectory points invisible, if any 
      if( plane.trajectory != null )
         plane.trajectory.SetDResource( "Marker%/Visibility", 0.0 );
   }

   ////////////////////////////////////////////////////////////////////////
   // FOR FLIGHT SIMULATION ONLY: Calculates plane's lat/lon using simulated 
   //     data. In an application, it will query the plane's position.
   //
   // The simulation moves the plane from the start to the end node/city
   // as controlled by the path_position parameter. The path_position changes
   // in the range from from 0 (start node) to 1 (end node).
   ////////////////////////////////////////////////////////////////////////
   void GetPlaneLatLon( PlaneData plane )
   {
      plane.lat_lon.x = 
         RELATIVE_TO_NEW_RANGE( plane.from_node.lat_lon.x, 
                           plane.to_node.lat_lon.x, plane.path_position );
      plane.lat_lon.y = 
         RELATIVE_TO_NEW_RANGE( plane.from_node.lat_lon.y, 
                                plane.to_node.lat_lon.y, plane.path_position );
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
      
      return 
        "Lon=" + x_deg + "\u00B0" + 
              GlgObject.Printf( "%02d", x_min ) + "\'" + 
              GlgObject.Printf( "%02d", x_sec ) + "\"" + char_x +
        "  Lat=" + y_deg + "\u00B0" + 
              GlgObject.Printf( "%02d", y_min ) + "\'" + 
              GlgObject.Printf( "%02d", y_sec ) + "\"" + char_y;
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
   // Invoked by the browser to stop the applet.
   //////////////////////////////////////////////////////////////////////////
   public void stop()
   {
      StopUpdates();
      super.stop();
   }

   //////////////////////////////////////////////////////////////////////////
   // Show zoom message.
   //////////////////////////////////////////////////////////////////////////
   void ZoomToFloridaStart()
   {
      Drawing.SetDResource( "Map/FloridaZoomingMessage/Visibility", 1.0 );
      Drawing.Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Zoom to the Florida area after a few seconds to show details.
   //////////////////////////////////////////////////////////////////////////
   void ZoomToFlorida()
   {
      AbortDistanceMode();

      /* GISExtent is in lon/lat for the rectangular GIS projection,
         and in meters for the orthographic projection.
         To find proper values, zoom the map in the GlgBuilder and 
         copy the GISExtent values. */

      // Zoom to the Florida boundaries on detailed map.
      GISObject.SetGResource( "GISExtent", 1169530.0, 1169530.0, 0.0 );
      GISObject.SetGResource( "GISCenter" , -82.8239, 28.9382, 0.0 );

      // Update icon positions after zooming.
      RedoIcons = true;
      UpdateObjectsOnMap();

      /* Reorder Florida zoom message to the top, in case any objects 
         were added on top of it.
      */
      GlgObject florida_message = 
        Drawing.GetResourceObject( "Map/FloridaZoomingMessage" );
      Map.ReorderElement( Map.GetIndex( florida_message ), 
                         Map.GetSize() - 1 );

      if( ASYNC_MAP )
        SetMapLoading( true, "Loading map..." );
      
      Drawing.Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Remove the Florida zooming message after a few seconds.
   //////////////////////////////////////////////////////////////////////////
   void ZoomToFloridaEnd()
   {
      Drawing.SetDResource( "Map/FloridaZoomingMessage/Visibility", 0.0 );
      Drawing.Update();
   }

   class NodeData
   {
      String name;
      GlgPoint lat_lon;
      
      GlgObject graphics;

      NodeData( String name_p, double lon, double lat )
      {
         name = name_p;
         lat_lon = new GlgPoint( lon, lat, 0.0 );
      }
   }
   
   class PlaneData
   {
      String name;
      GlgPoint lat_lon = new GlgPoint();
      int flight_number;
      String tooltip;
      GlgObject graphics;
      GlgObject trajectory;
      NodeData from_node;
      NodeData to_node;
      double path_position;
      double path_position_last;
      double speed;
      int color_index;
      int iteration;

      PlaneData()
      {
      }
   }

   class ZoomPerformer implements ActionListener
   {
      GlgAirTrafficDemo bean;
      int stage;

      public ZoomPerformer( GlgAirTrafficDemo bean_p )
      {
         bean = bean_p;
         stage = 0;
      }

      public void actionPerformed( ActionEvent e )
      {
         PerformZoomAction();
      }

      public void PerformZoomAction()
      {
         switch( stage )
         {
          case 0:  // Display zoom message, yield to let event thread draw it.
            bean.ZoomToFloridaStart();

            stage = 1;
            bean.zoom_timer.setInitialDelay( 10 );
            bean.zoom_timer.start();
            break;

          case 1:  // Zoom to the Florida area
            bean.ZoomToFlorida();

            stage = 2;
            bean.zoom_timer.setInitialDelay( bean.FloridaZoomDelay2 );
            bean.zoom_timer.start();            
            break;

          case 2:  // Erase zoom message after a delay
            bean.ZoomToFloridaEnd();
            bean.zoom_timer = null;
            break;
         }
      }
   }
}
