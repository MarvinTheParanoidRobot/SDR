////////////////////////////////////////////////////////////////////////
// GIS MAP SERVER SETUP INFO
//
// This demo uses the GLG Map Server to display a map. The GLG Map Server 
// has to be installed either on the local host or on a remote web server.
// Refer to the GLG Map Server documentation for setup instructions.
// After the Map Server has been installed, enable the map display by 
// modifying the source code:
//
// 1. Comment out the following two statements:
//
//    GISObject.SetDResource( "GISDisabled", 1.0 );
//    Map.SetDResource( "MapServerSetupInfo/Visibility", 1.0 );
//
// 2. Initialize the SuppliedMapServerURL variable in the below code 
//    to point to the GLG Map Server location, rebuild the demo and 
//    restart the JSP server.
//////////////////////////////////////////////////////////////////////

package glg_demos;

import java.io.*;
import java.net.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.imageio.*;
import java.awt.image.*;
import com.genlogic.*;

public final class GlgGISServlet extends HttpServlet 
   implements GlgErrorHandler
{
   static final long serialVersionUID = 0;

   // Drawing path relative to the servlet app's dir.
   static final String drawing_name = "/drawings/gis2.g";

   // If supplied, overrides the URL of the GIS object in the drawing
   static String SuppliedMapServerURL = null; 

   final double PlaneSpeed = 0.02;      // Relative units
   final boolean UseFixedWidth = true;  // Change to false to take width and 
                                        // height from the http request.
   final int MinUpdatePeriod = 2900;    // In milliseconds
   final double                         // Plane size constants
     SMALL_SIZE  = 0.23,
     MEDIUM_SIZE = 0.33,
     BIG_SIZE =    0.43;
   final double PlaneSize = MEDIUM_SIZE; 

   // Using global class instances (static) because the drawing is loaded 
   // just once and shares between all servlets and threads, 
   // Alternatively, each servlet may load its own drawing.
   static GlgObject
     Map,
     GISObject,
     GISArray,
     NodeGroup,
     PlaneGroup,
     NodeTemplate,
     PlaneTemplate;
   static int     
     NumNodes,
     NumPlanes = 10,   
     CurrWidth = -1,
     CurrHeight = -1;
   static boolean SizeChanged = true;
   static int MapProjection = GlgObject.ORTHOGRAPHIC_PROJECTION;
   // If true, uses the GIS projection defined by the MapProjection above.
   // If set to false, uses the GIS projection specified in the drawing.
   final boolean SetProjection = true;

   static PlaneData [] PlaneArray;

   // Array of icons to place on the map as GLG objects in addition to the 
   // icons defined in GIS server's data. The icons that use GLG objects may
   // be selected with the mouse and their attributes can be changed 
   // dynamically, based on data. When the mouse moves over an icon, it may 
   // be highlighted with a different color or a tooltip may be displayed.
     //
   static NodeData [] NodeArray =
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

   // Temp vars: allocate once
   GlgPoint 
     last_xyz = new GlgPoint(),
     curr_xyz = new GlgPoint();

   long LastUpdateTime = 0;

   /////////////////////////////////////////////////////////////////
   // A wrapper around the main method, doGet2(), to properly handle
   // the access synchronization and unlocking on an error.
   /////////////////////////////////////////////////////////////////
   public void doGet( HttpServletRequest request, 
                      HttpServletResponse response ) 
      throws ServletException
   { 
      try
      {
         doGet2( request, response );
      } 
      catch( Exception e ) 
      {
         // Unlock if was interrupted by the exception while locked.
         GlgObject.UnlockThread();

         throw new ServletException( e );  // Re-thow to log an error
      }

      // Unlock just in case the code did not do it due to a programming error.
      GlgObject.UnlockThread();
   }

   /////////////////////////////////////////////////////////////////
   // Main servlet's method: everything is handled here.
   /////////////////////////////////////////////////////////////////
   // Supported actions (action parameter):
   //    GetImage - generates GIS image with dynamic icons
   //    ProcessEvent - processes object selection on MouseClick or 
   //                   Tooltip event types.
   //    GetDialogData - returns data for requested dialogs.
   /////////////////////////////////////////////////////////////////
   public void doGet2( HttpServletRequest request, 
                      HttpServletResponse response ) 
   {
      InitGLG();   // Init the Toolkit

      String action = GetStringParameter( request, "action", "GetImage" );
      if( action.equals( "GetDialogData" ) )
      {
         // Simply return requested dialog data: no drawing required.
         ProcessDialogData( request, response );
         return;
      }

      // The rest of actions (GetImage, ProcessEvent) require a drawing -
      // load it (if first time) and update with data.

      int width, height;

      if( UseFixedWidth )
      {
         width = 800;
         height = 600;
      }
      else
      {
         // Get requested width/height of the image: need for all other actions.
         width = GetIntegerParameter( request, "width", 800 );
         height = GetIntegerParameter( request, "height", 600 );
         
         // Limit max. size to avoid running out of heap space creating an 
         // image.
         if( width > 1000 ) width = 1000;       
         if( height > 1000 ) height = 1000;       
      }

      // This example reuses the same drawing between all servlets' threads.
      // Therefore lock to synchronize and prevent other servlets from 
      // changing the drawing size, etc., before we are done.
      GlgObject.Lock();

      // Load the drawing just once and share it between all servlets and 
      // threads. Alternatively, each servlet may load its own drawing.
      //
      if( Map == null )    // First time: load the drawing.
      {
         Map = LoadDrawing( drawing_name );

         Init();   // Create airport and plane icons.

         SetImageSize( width, height );
         Map.SetupHierarchy();      // Setup to prepare to receive data

         // Preposition planes at random positions on startup.
         for( int i=0; i<100; ++i )
           UpdatePlanes( false );
      }
      else   // Already loaded, reuse the drawing.
        SetImageSize( width, height );
       
      // Main action: Generate Image.
      if( action.equals( "GetImage" ) )
      {
         // Update plane positions with current data.
         UpdatePlanes( true );
             
         // Setup after data update to prepare to generate image.
         Map.SetupHierarchy();
         SizeChanged = false;

         // Create an image of the viewport's graphics.
         BufferedImage image = (BufferedImage) Map.CreateImage( null );
            
         GlgObject.Unlock();
         
         // Write the image
         try
         {
            response.setContentType("image/png");
            OutputStream out_stream = response.getOutputStream();
            ImageIO.write( image, "png", out_stream );
            out_stream.close();
         }
         catch( IOException e )
         {
            // Log( "Aborted writing of image file." );
         }
      }
      // Secondary action: ProcessEvent.
      else if( action.equals( "ProcessEvent" ) )
      {
         String selection_info = null;

         // Setup after a possible image size change. Perform only on a real
         // size change to avoid unnecessary map server requests.
         if( SizeChanged )
         {
            Map.SetupHierarchy();
            SizeChanged = false;
         }

         String event_type = GetStringParameter( request, "event_type", "" );

         int selection_type;
         if( event_type.equals( "MouseClick" ) )      // Get selected object
         {
            // Find object with the MouseClick custom event.
            selection_type = GlgObject.CLICK_SELECTION;
         }
         else if( event_type.equals( "Tooltip" ) )    // Get object's tooltip
         {
            // Find object with the TooltipString property.
            selection_type = GlgObject.TOOLTIP_SELECTION;
         }
         else
         {
            selection_type = 0;
            selection_info = "Unsupported event_type";
         }

         if( selection_type != 0 )
         {
            // Get x and y coordinates of the mouse click.
            int x = GetIntegerParameter( request, "x", -1 );
            int y = GetIntegerParameter( request, "y", -1 );
            
            // Selection rectangle around the mouse click.
            GlgCube click_box = new GlgCube();
            int selection_sensitivity = 3;   // in pixels
            click_box.p1.x = x - selection_sensitivity;
            click_box.p1.y = y - selection_sensitivity;
            click_box.p2.x = x + selection_sensitivity;
            click_box.p2.y = y + selection_sensitivity;

            // Find selected object with MouseClick custom event attached.
            if( x > 0 && y > 0 )
            {
               // Select using MouseClick custom events.
               GlgObject selection_message = null;
               selection_message =
                 GlgObject.CreateSelectionMessage( Map, click_box, Map, 
                                                   selection_type, 1 );
               
               if( selection_message != null )
                 switch( selection_type )
                 {
                  default:
                  case GlgObject.CLICK_SELECTION:   // Return object name
                    selection_info = 
                      selection_message.GetSResource( "Object/Name" );
                    break;
                     
                  case GlgObject.TOOLTIP_SELECTION: // Return tooltip string
                    selection_info = 
                      selection_message.GetSResource( "Object/Tooltip" );
                    break;
                 }
            }
            else
              selection_info = "Invalid x/y coordinates.";
         }
         
         GlgObject.Unlock();

         WriteAsPlainText( response, 
                          selection_info == null ? "None" : selection_info );
      }
      else
      {
         Log( "Unsupported action!" );
         GlgObject.Unlock();
      }
   }

   /////////////////////////////////////////////////////////////////
   void SetImageSize( int width, int height )
   {
      if( width != CurrWidth || height != CurrHeight )
      {
         Map.SetImageSize( width, height );
         SizeChanged = true;
      }
   }

   /////////////////////////////////////////////////////////////////
   // Helper methods
   /////////////////////////////////////////////////////////////////

   int GetIntegerParameter( HttpServletRequest request, String name, 
                           int default_value )
   {
      String parameter_string = request.getParameter( name );
      if( parameter_string == null || parameter_string.equals( "" ) )
        return default_value;

      try
      {
         return Integer.parseInt( parameter_string );
      }
      catch( NumberFormatException e )
      {
         Log( "Invalid parameter value for: " + name + 
             " = " + parameter_string );
         return default_value;
      }
   }

   String GetStringParameter( HttpServletRequest request, String name, 
                             String default_value )
   {
      String parameter_string = request.getParameter( name );
      if( parameter_string == null )
        return default_value;
      else
        return parameter_string;
   }

   /////////////////////////////////////////////////////////////////
   void Log( String msg )
   {
      getServletContext().log( "GlgGISServlet: " + msg );
   }

   // GlgErrorHandler interface method for error handling.
   public void Error( String message, int error_type, Exception e )
   {
      Log( message );   // Log errors

      Log( GlgObject.GetStackTraceAsString() );   // Print stack
   }

   /////////////////////////////////////////////////////////////////
   void WriteAsPlainText( HttpServletResponse response, String string )
   {
      try
      {
         response.setContentType("text/plain");
         PrintWriter out_stream = 
            new PrintWriter( response.getOutputStream() );
         out_stream.write( string );
         out_stream.close();
      }
      catch( IOException e )
      {
         // Log( "Aborted writing of text response." );
      }
   }

   /////////////////////////////////////////////////////////////////
   void InitGLG()
   {
      // Set an error handler to log errors.
      GlgObject.SetErrorHandler( this );

      GlgObject.Init();    // Init GlgToolkit
   }

   /////////////////////////////////////////////////////////////////
   GlgObject LoadDrawing( String drawing_name )
   {
      GlgObject drawing;

      // Get drawing URL relative to the servlet's directory.
      URL drawing_url = null; 
      try
      {
         drawing_url = 
            getServletConfig().getServletContext().getResource( drawing_name );
      }
      catch( MalformedURLException e )
      {
         Log( "Malformed URL: " + drawing_name );
         return null;
      }

      if( drawing_url == null )
      {
         Log( "Can't find drawing: " + drawing_name );
         return null;
      }

      // Load drawing from the URL
      drawing = GlgObject.LoadWidget( drawing_url.toString(), GlgObject.URL );
      if( drawing == null )
      {
         Log( "Can't load drawing: " + drawing_name );
         return null;
      }

      // Disable viewport border in the image: let html define it if needed.
      drawing.SetDResource( "LineWidth", 0. );

      return drawing;
   }

   /////////////////////////////////////////////////////////////////
   // Returns data for a requested dialog.
   /////////////////////////////////////////////////////////////////
   void ProcessDialogData( HttpServletRequest request, 
                          HttpServletResponse response )
   {
      int index;

      String obj_name = 
        GetStringParameter( request, "dialog_type", "None" );

      if( obj_name.startsWith( "Plane" ) )
      {
         // Get DataIndex for direct access to the plane's data.
         index = GISArray.GetDResource( obj_name + "/DataIndex" ).intValue();

         PlaneData plane = PlaneArray[ index ];

         WriteAsPlainText( response, 
                           "<b>Flight " + plane.flight_number + "</b><br>" +
                           "From: " + plane.from_node.city_name + "<br>" +
                           "To: &nbsp;&nbsp;&nbsp;&nbsp;" + plane.to_node.city_name + "<br>" +
                           "Lat: &nbsp;&nbsp;&nbsp;" + CreateLocationString( plane.lat_lon, true ) + "<br>" +
                           "Lon: &nbsp;&nbsp;" + CreateLocationString( plane.lat_lon, false ) );
      }
      else if( obj_name.startsWith( "Node" ) )
      {
         // Get DataIndex for direct access to the node's data.
         index = GISArray.GetDResource( obj_name + "/DataIndex" ).intValue();
         NodeData node = NodeArray[ index ];
         
         WriteAsPlainText( response, 
                           "<b>" + node.city_name + "</b><br>" +
                           "Lat: &nbsp;" + CreateLocationString( node.lat_lon, true ) + "<br>" +
                           "Lon: " + CreateLocationString( node.lat_lon, false ) );
      }
      else
        WriteAsPlainText( response, "None" );
   }

   //////////////////////////////////////////////////////////////////////////
   String Format( double value )
   {
      // Use 5 digits after the decimal point.
      return GlgObject.Printf( "%.5f", value );
   }

   //////////////////////////////////////////////////////////////////////////
   // Initializes icons in the drawing
   //////////////////////////////////////////////////////////////////////////
   void Init()
   {
      int i;

      // Get IDs of the GIS object in the map viewport. 
      GISObject = Map.GetResourceObject( "GISObject" );

      // GIS container that holds plane and node icons inside the GIS object.
      GISArray = GISObject.GetResourceObject( "GISArray" );

      // Comment out the next two statements to enable the map.
      // Refer to the map server setup note at the beginning of this file.
      //
      GISObject.SetDResource( "GISDisabled", 1.0 );
      Map.SetDResource( "MapServerSetupInfo/Visibility", 1.0 );

      // Try to use the GLG map server on the localhost if no URL was supplied.
      if( SuppliedMapServerURL == null )
        SuppliedMapServerURL = "http://localhost/cgi-bin/GlmScript";

      if( SuppliedMapServerURL != null )
        // Override the URL defined in the drawing.
        GISObject.SetSResource( "GISMapServerURL", SuppliedMapServerURL );

      // Set the GIS Zoom mode. It was set and saved with the drawing, 
      // but do it again programmatically just in case.
      Map.SetZoomMode( null, GISObject, null, GlgObject.GIS_ZOOM_MODE );

      if( SetProjection )
        // Set GIS projection.
        GISObject.SetDResource( "GISProjection", (double) MapProjection );
      else
        // Query the GIS projection defined in the drawing.
        MapProjection = GISObject.GetDResource( "GISProjection" ).intValue();

      // Zoom to the US area. Without this call, the map will be shown 
      // in the zoom state defined in the drawing.
      ZoomToUS();

      // Get the palette containing templates for plane and node icons.
      GlgObject palette = Map.GetResourceObject( "Palette" );

      // Delete it from the drawing
      Map.DeleteObject( palette );

      // Get node and plane templates from the palette. 
      NodeTemplate = palette.GetResourceObject( "Node" );
      PlaneTemplate = palette.GetResourceObject( "Plane" );

      // If the icon is not a marker (IconScale resource exists), set the
      // icon's size.
      GlgObject resource = PlaneTemplate.GetResourceObject( "IconScale" );
      if( resource != null )
        resource.SetDResource( null, PlaneSize );

      NumNodes = NodeArray.length;

      // Create and initialize plane structures used for simulation.
      PlaneArray = new PlaneData[ NumPlanes ];
      for( i =0; i < NumPlanes; ++i )
        PlaneArray[ i ] = new PlaneData();
      
      // Add airport nodes and plane icons to the map.
      CreateAirportIcons();
      CreatePlaneIcons();

      // Turn on airport node labels display.
      NodeTemplate.SetDResource( "Label/Visibility", 1.0 );
      
      // Select GIS layers to be displayed by the map server.
      GISObject.SetSResource( "GISLayers", "default,states,-us_cities" );
      SetPlaneSize();

      SetPlaneLabels( true );   // Turn on plane labels
   }

   //////////////////////////////////////////////////////////////////////////
   void SetPlaneSize()
   {
      GlgObject resource = PlaneTemplate.GetResourceObject( "IconScale" );
      if( resource != null )
        resource.SetDResource( null, PlaneSize );
   }

   //////////////////////////////////////////////////////////////////////////
   // Turns plane icons' labels ON or OFF.
   //////////////////////////////////////////////////////////////////////////
   void SetPlaneLabels( boolean on )
   {
      GlgObject label = PlaneArray[0].graphics.GetResourceObject( "Label" );
      if( label != null )
        label.SetDResource( "Visibility", on ? 1.0 : 0.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   // Creates NodeGroup to hold airport icons and adds the icons to it.
   //////////////////////////////////////////////////////////////////////////
   void CreateAirportIcons()
   {
      NodeGroup = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
      NodeGroup.SetSResource( "Name", "NodeGroup" );

      // Add city/node icons
      for( int i = 0; i < NumNodes; ++i )
        AddNode( NodeArray[ i ], i );

      // Add the group to the GIS Object which will automatically manage 
      // the GIS coordinate conversion. This allows to specify node 
        // positions in lat/lon instead of the X/Y world coordinates.
      GISObject.AddObjectToBottom( NodeGroup );
   }

   //////////////////////////////////////////////////////////////////////////
   // Creates PlaneGroup to hold plane icons and adds the icons to it.
   //////////////////////////////////////////////////////////////////////////
   void CreatePlaneIcons()
   {
      PlaneGroup = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
      PlaneGroup.SetSResource( "Name", "PlaneGroup" );

      // Add plane icons
      for( int i=0; i < NumPlanes; ++i )
      {
         AddPlane( PlaneArray[ i ], i );
         StartPlane( PlaneArray[ i ], true );
      }
    
      // Add the group to the GIS Object which will automatically manage 
      // the GIS coordinate conversion. This allows to specify plane 
        // positions in lat/lon instead of the X/Y world coordinates.
      GISObject.AddObjectToBottom( PlaneGroup );
   }

   //////////////////////////////////////////////////////////////////////////
   void AddNode( NodeData node_data, int index )
   {      
      // Create a copy of a node.
      GlgObject node = NodeTemplate.CloneObject( GlgObject.STRONG_CLONE );
      
      // Set object name to access the object as a named resource.
      String name = "Node" + Integer.toString( index );
      node.SetSResource( "Name", name );

      // Set node position in lat/lon coordinates. The GIS object will handle
      // the GIS coordinate conversion.
      node.SetGResource( "Position", 
                         node_data.lat_lon.x, node_data.lat_lon.y, 0.0 );

      // Index for direct access
      node.SetDResource( "DataIndex", (double)index );

      // Set node name display label.
      node.SetSResource( "LabelString", node_data.city_name );
      
      // Set node tooltip that includes name and lat/lon into.
      String tooltip = 
        node_data.city_name + ", " + CreateLocationString( node_data.lat_lon );
      node.SetSResource( "Tooltip", tooltip );
        
      node_data.graphics = node;      
      NodeGroup.AddObjectToBottom( node );  // Add the node to the map.
   }

   //////////////////////////////////////////////////////////////////////////
   // Adds a plane icon, fills labels, tooltips, etc.
   //////////////////////////////////////////////////////////////////////////
   void AddPlane( PlaneData plane_data, int index )
   {
      // Create a copy of a node.
      GlgObject plane = PlaneTemplate.CloneObject( GlgObject.STRONG_CLONE );

      // Set object name to access the object as a named resource.
      String name = "Plane" + Integer.toString( index );
      plane.SetSResource( "Name", name );

      // Index for direct access
      plane.SetDResource( "DataIndex", (double)index );

      // Check if the icon has an angle to indicate its direction.
      if( plane.GetResourceObject( "Angle" ) != null )
        plane_data.has_angle = true;

      // The plane will be positioned with PositionPlane() after the GIS object
      // have been setup and the plane's lat/lon has been calculated by the 
      // flight simulation.

      plane_data.graphics = plane;
      PlaneGroup.AddObjectToBottom( plane );  // Add the plane to the map.
   }

   //////////////////////////////////////////////////////////////////////////
   void PositionPlane( PlaneData plane )
   {
      if( plane.graphics == null || 
         plane.from_node == null || plane.to_node == null )
        return;

      // Obtain the plane's current position.
      GetPlaneLatLon( plane );

      // Update plane's icon in the drawing by setting its lat/lon coordinates.
      // The GIS object will handle the GIS coordinate conversion.
      plane.graphics. SetGResource( "Position", 
                                    plane.lat_lon.x, plane.lat_lon.y, 0.0 );

      // Update icon's direction angle is necessary
      if( plane.has_angle )
      {
         double angle = GetPlaneAngle( plane );
         plane.graphics.SetDResource( "Angle", angle );
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
         Error( "Less then two nodes: can't start planes.", 0, null );
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
         plane.path_position = 0.;
         plane.path_position_last = 0.;
      }

      plane.tooltip = "Flight " + plane.flight_number;

      // Set tooltip to display the flight number, as well as the from/to info.
      plane.tooltip = "Flight " + plane.flight_number +
        " from " + plane.from_node.city_name + " to " + plane.to_node.city_name;
      plane.graphics.SetSResource( "Tooltip", plane.tooltip );

      // Show the flight number as the icon label.
      plane.graphics.SetSResource( "LabelString", 
                                   "Flight " + plane.flight_number );
   }

   //////////////////////////////////////////////////////////////////////////
   // Zoom to the US or some other area. If this method is not invoked,
   // the map will be shown in the zoom state defined in the drawing.
   //////////////////////////////////////////////////////////////////////////
   void ZoomToUS()
   {
      // Zoom to the US boundaries.
      GISObject.SetGResource( "GISCenter", -95., 37., 0.0 );

      // GISExtent is in lon/lat for the rectangular GIS projection,
      // and in meters for the orthographic projection.
      // To find proper values, zoom the map in the GlgBuilder and 
      // copy the GISExtent values.
      //
      if( MapProjection == GlgObject.RECTANGULAR_PROJECTION )
        GISObject.SetGResource( "GISExtent", 69.71, 34.85, 0.0 );
      else
        GISObject.SetGResource( "GISExtent", 5111620., 5111620., 0. );
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates plane positions on the map.
   //////////////////////////////////////////////////////////////////////////
   void UpdatePlanes( boolean check_time_interval )
   {
      if( check_time_interval )
      {
         // Prevent plane positions from adjusting on each image request.
         long curr_time = System.currentTimeMillis();
         if( curr_time - LastUpdateTime < MinUpdatePeriod )
           return;

         LastUpdateTime = curr_time;
      }

      for( int i = 0; i < NumPlanes; ++i )
        UpdatePlane( PlaneArray[ i ] );
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
         // Store last position for calculating the angle in the ORTHO 
         // projection.
         plane.path_position_last = plane.path_position;

         // Move the plane.
         plane.path_position += plane.speed * PlaneSpeed;
         if( plane.path_position > 1.0 )
           plane.path_position = 1.0; // Clamp to 1: can't go past the airport!
      }

      PositionPlane( plane );    // Position the plane on the map.
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
   // FOR FLIGHT SIMULATION ONLY: Converts plane lat/lon to the GLG world 
   //     coordinates for calculating plane speed and directional angle.
   //////////////////////////////////////////////////////////////////////////
   void GetPlanePosition( PlaneData plane, GlgPoint xyz )
   {
      // Converts lat/lon to X/Y using GIS object's current projection.
      GISObject. GISConvert( null, GlgObject.OBJECT_COORD, 
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
      // Rectangular projection preserves straight lines, we can use the 
      // angle of the line connecting the start and end nodes. For the
      // orthographic projection, use this case if the plane has just started
        // and there is no previous position stored.
      if( MapProjection == GlgObject.RECTANGULAR_PROJECTION ||
          plane.path_position == plane.path_position_last )   // Just started
      {
         GetNodePosition( plane.from_node, last_xyz );
         GetNodePosition( plane.to_node, curr_xyz );
      }
      else  // In the orthographic projection straight lines are drawn as 
            // curves. Use the angle of the line connecting the current and 
              // last position of the plane.
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

      // Calculate the angle of a line connecting the previous and 
        // current position.
      return GetAngle( last_xyz, curr_xyz );
   }

   //////////////////////////////////////////////////////////////////////////
   // Generate a location info string by converting +- sign info into the
   // N/S, E/W suffixes, and decimal fraction to deg, min, sec.
   //////////////////////////////////////////////////////////////////////////
   String CreateLocationString( GlgPoint point )
   {
      return
        "Lon="   + CreateLocationString( point, false ) + 
        "  Lat=" + CreateLocationString( point, true );
   }

   //////////////////////////////////////////////////////////////////////////
   // Generate a location info string by converting +- sign info into the
   // N/S, E/W suffixes, and decimal fraction to deg, min, sec.
   //////////////////////////////////////////////////////////////////////////
   String CreateLocationString( GlgPoint point, boolean is_lat )
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

      if( is_lat )
      {
         lat = point.y;

         if( lat < 0.0 )
         {
            lat = -lat;
            char_y = 'S';
         }
         else
           char_y = 'N';
         
         y_deg = (int) lat;
         y_min = (int) ( ( lat - y_deg ) * 60.0 );
         y_sec = (int) ( ( lat - y_deg - y_min / 60.0 ) * 3600.0 );
      
         return "" + y_deg + "\u00B0" + padded( y_min ) + "\'" + 
           padded( y_sec ) + "\"" + char_y;
      }
      else
      {
         lon = point.x;
         
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
         
         x_deg = (int) lon;
         x_min = (int) ( ( lon - x_deg ) * 60.0 );
         x_sec = (int) ( ( lon - x_deg - x_min / 60.0 ) * 3600.0 );
         
         return "" + x_deg + "\u00B0" + padded( x_min ) + "\'" + 
           padded( x_sec ) + "\"" + char_x;
      }
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
   // UTILITY FUNCTION: Calculates a distance between two points in 2D.
   //////////////////////////////////////////////////////////////////////////
   double GetLength( GlgPoint pt1, GlgPoint pt2 )
   {
      return Math.sqrt( ( pt2.x - pt1.x ) * ( pt2.x - pt1.x ) +
                       ( pt2.y - pt1.y ) * ( pt2.y - pt1.y ) );
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
}

class NodeData
{
   String city_name;
   GlgPoint lat_lon;
   GlgObject graphics;
   
   NodeData( String city_name_p, double lon, double lat )
   {
      city_name = city_name_p;
      lat_lon = new GlgPoint( lon, lat, 0.0 );
   }
}
   
class PlaneData
{
   GlgPoint lat_lon;
   int flight_number;
   String tooltip;
   GlgObject graphics;
   NodeData from_node;
   NodeData to_node;
   double path_position;
   double path_position_last;
   double speed;
   boolean has_angle;
   double angle;
   
   PlaneData()
   {
      lat_lon = new GlgPoint();
   }
}
