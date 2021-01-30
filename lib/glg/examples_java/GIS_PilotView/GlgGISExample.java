import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgGISExample extends GlgJBean implements ActionListener, 
   GlgGISRequestObserver
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////

   static final long serialVersionUID = 0;

   // Animation parameters.

   // Time interval for periodic updates, in millisec.
   final int UPDATE_INTERVAL = 100; 

   // ZOOM/PAN coefficients.
   final double MAP_ZOOM_FACTOR = 2.;
   final double MAP_PAN_FACTOR = 0.2;

   // Minimum delta in pixels. Minimum value is 1.
   final int MIN_PIXEL_DELTA = 2;

   // Use synchronous map image loading on map dragging.
   final boolean USE_SYNC_DRAGGING = true;

   GlgObject GISObject;
   GlgObject MapViewport;
   GlgObject StatusObject;
   
   boolean MapLoaded = false;
   boolean PilotView = false;
   boolean AsyncImageLoading;

   // Create an instance of a class to store icon information 
   IconData Icon = new IconData();

   // Store initial extent and center, used to reset the drawing 
   GlgPoint InitExtent, InitCenter;

   // Current extent and center, updated after zooming or panning.
   GlgPoint CurrExtent, CurrCenter;

   // GIS Projection as defined in the .g file -- RECTANGULAR or ORTHOGRAPHIC.
   int MapProjection = GlgObject.UNDEFINED_PROJECTION;

   boolean IsReady = false;

   Timer timer = null;

   // Temporary variables - allocate once.
   GlgMinMax LonMinMax = new GlgMinMax(); 
   GlgMinMax LatMinMax = new GlgMinMax(); 

   GlgPoint point1 = new GlgPoint();
   GlgPoint point2 = new GlgPoint();

   // A counter used to calculate icon position in GetDemoIconData().
   int RotationState = 0;   

   final boolean DEBUG_POSITION = false;
   final boolean DEBUG_MAP_REQUEST = false;
   final boolean DEBUG_PILOT_VIEW = false;

   //////////////////////////////////////////////////////////////////////////
   public GlgGISExample()
   {
      super();

      // Activate Trace callback.
      AddListener( GlgObject.TRACE_CB, this );
   }

   //////////////////////////////////////////////////////////////////////////
   public void LoadDrawing( String drawing_filename )
   {
      if( drawing_filename == null )
      {
         System.out.println( "No drawing file: drawing loading failed." );
         return;
      }

      SetDrawingName( drawing_filename );
   }

   //////////////////////////////////////////////////////////////////////////
   // HCallback is invoked before the hierarchy setup.
   //////////////////////////////////////////////////////////////////////////
   public void HCallback( GlgObject viewport )
   {
      MapViewport = viewport.GetResourceObject( "MapViewport" );
      GISObject = MapViewport.GetResourceObject( "GISObject" );
      
      StatusObject = MapViewport.GetResourceObject( "StatusObject" );
      MapLoaded = false;    // Reset the flag if the drawing was reset.
      DisplayStatus( "Loading map, please wait." );

      /* Set GIS Zoom mode: generate a new map request for a 
         new area on zoom/pan.
      */
      MapViewport.SetZoomMode( null, GISObject, null, GlgObject.GIS_ZOOM_MODE );
      
      Init();
   }

   //////////////////////////////////////////////////////////////////////////
   // VCallback is invoked after the hierarchy setup but before the map is 
   // drawn.
   //////////////////////////////////////////////////////////////////////////
   public void VCallback( GlgObject viewport )
   {
      GetIconData();
      PositionIcons( true /*update extent*/ );
   }

   ///////////////////////////////////////////////////////////////////////
   // ReadyCallback is invoked after the drawing has been displayed for
   // the first time.
   ///////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback();

      // Enable dynamic updates 
      IsReady = true;    

      // Start periodic dynamic updates.
      StartUpdates();
   }

   //////////////////////////////////////////////////////////////////////
   // Initialize the drawing. 
   //////////////////////////////////////////////////////////////////////
   public void Init()
   {
      // Enable asynchronous image loading for the GIS object.
      AsyncImageLoading = true;
      SetMapLoadingMode( AsyncImageLoading );

      /* Query and store initial map extent from the GIS object.
         It is used to reset the drawing to the initial extent 
         when the user clicks on the ZoomReset button.
      */
      InitExtent = GISObject.GetGResource( "GISExtent" );
      InitCenter = GISObject.GetGResource( "GISCenter" );

      // Obtain map projection.
      MapProjection = GISObject.GetDResource( "GISProjection" ).intValue();

      // Obtain an object ID of the Icon object.
      Icon.icon_obj = GISObject.GetResourceObject( "GISArray/Aircraft" );

      // Enable pilot view mode.
      SetPilotView( true );

      ////////////////////////////////////////////////////////////////////////
      // Map Server Setup Info
      ////////////////////////////////////////////////////////////////////////
      String map_server_info = 
        "This example uses the GLG Map Server to display a map.\n" +
        "The Map Server has to be installed either on the " +
        "local host or on a remote web server.\n\n" +
        "After the Map Server has been installed, enable the map by " +
        "modifying the source code to comment out the statement that sets " +
        "the GISDisable resource (the next statement after this message), " +
        "set SuppliedMapServerURL to point to the Map Server location and " +
        "rebuild the example.";

      System.out.println( map_server_info );

      /* If uncommented, the next line will disable the map.
         Comment it out to enable the map.
      */
      GISObject.SetDResource( "GISDisabled", 1.0 );
 
      ////////////////////////////////////////////////////////////////////////

      // Try to use the GLG map server installed on localhost by default.
      String SuppliedMapServerURL = "http://localhost/cgi-bin/GlmScript";

      if( SuppliedMapServerURL != null )
        // Override the URL defined in the drawing with the supplied URL.
        GISObject.SetSResource( "GISMapServerURL", SuppliedMapServerURL );
   }

   //////////////////////////////////////////////////////////////////////
   // timer's ActionListener method to be invoked peridically 
   //////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateDrawing();
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked as a timer procedure to perform periodic dynamic
   // updates. In this case, it repositions the icon on the map
   // using simulated data. 
   //////////////////////////////////////////////////////////////////////////
   public void UpdateDrawing()
   {      
      if( !IsReady() )
        return;

      /* Check map loading status on initial appearance --
         GISObject/ImageLoaded resource will be set to 1 when 
         the map image is loaded on initial draw. 
         Reset the "Loading map" prompt when the map image is ready.
      */
      if( !MapLoaded && GISObject.GetDResource( "ImageLoaded" ).intValue() != 0 )
      {
         MapLoaded = true;
         DisplayStatus( "" );
      }

      GetIconData();
      PositionIcons( false /*don't update extent*/ );

      Update(); 
   }

   ///////////////////////////////////////////////////////////////////////
   public void PositionIcons( boolean update_extent )
   {
      /* In RECTANGULAR map projection, extract current map extent 
         and center, and convert it to lat/lon range.
      */
      if( update_extent && MapProjection == GlgObject.RECTANGULAR_PROJECTION )
      {
         CurrExtent = GetMapExtent( null );
         CurrCenter = GetMapCenter( null );
         GetLatLonRange( CurrCenter, CurrExtent, LonMinMax, LatMinMax );          
         if( DEBUG_POSITION )
           System.out.println( "Current LON map extent: " +
                               LonMinMax.min + " " + LonMinMax.max );
      }

      /* In this example, there is only one icon. The example
         can be extended to loop through an array of icons and
         call PositionIcon() for each icon.
      */
      PositionIcon( Icon );
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Position an icon on the map.
   ///////////////////////////////////////////////////////////////////////
   public void PositionIcon( IconData icon )
   {   
      /* If the displayed icon position is not within the visible map extent,
         need to check if it has to be adjusted to make the icon visible
         when the map is scrolled beyond +-180 degrees.
      */
      AdjustDisplayedIconPosition( icon );

      /* Update icon parameters in the drawing using tags.
         It will automatically set all tags with a matching
         TagSource. Push new values only if received new data or
         the icon position was adjusted, in which case icon's
         displayed_position_changed was set to true;
      */
      if( icon.displayed_position_changed )
      {
         /* Use lat/lon values from the adjusted position 
            (icon.lat_lon_displayed).
            If the icon pposition didn't need to be adjusted,
            icon.lat_lon_displayed will be the same as the original 
            data (icon.lat_lon).
         */
         SetGTag( "Position", icon.lat_lon_displayed, false ); 
         SetDTag( "Lat", icon.lat_lon.y, false ); 
         SetDTag( "Lon", icon.lat_lon.x, false ); 
         SetDTag( "Altitude", icon.altitude, false );
         SetDTag( "Roll", icon.roll, false );
         SetDTag( "Pitch", icon.pitch, false );
         SetDTag( "Yaw", icon.yaw, false );

         /* PILOT VIEW: Adjust map center so that the icon stays in 
            the center of the map. In case of several icons, the GIS center
            would be adjusted to center some specific icon.
         */
         if( PilotView )
           SetMapCenter( icon.lat_lon_displayed );

         icon.displayed_position_changed = false;
      }
   }

   ///////////////////////////////////////////////////////////////////////
   // Adjust displayed icon position to handle cases when the map is 
   // scrolled outside the +-180 degrees of longitude.
   ///////////////////////////////////////////////////////////////////////
   public void AdjustDisplayedIconPosition( IconData icon )
   {      
      if( MapProjection != GlgObject.RECTANGULAR_PROJECTION )
        return;  /* Adjustment is needed only in the RECTANGULAR projection. */

      /* When the whole world is displayed and the map is scrolled outside of 
         the [-180;+180] longitute extent in RECTANGULAR projection, the map 
         image is replicated outside the [-180;+180] extent to allow centering
         the map on an area such as Alaska. 

         To accomplish that, the map server replicates the whole world imagery
         on the left and right of the whole world's [-180;+180] extent, 
         so that the map imagery may be scrolled up to +-540 degrees of 
         longitude. The resulting whole world extents are called "center",
         "left" and "right" in the comments below.

         If the map is scrolled outside the [-180;+180] center extent in the 
         RECTANGULAR projection, the icon(s) (whose longitude position is in 
         the range +-180), have to be adjusted to be inside the currently
         displayed map extent.
      */

      /* If the displayed icon position is not within the visible map extent,
         need to check if it has to be adjusted to make the icon visible
         when the map is scrolled beyond +-180 degrees.
      */
      if( icon.lat_lon_displayed.x >= LonMinMax.min &&
          icon.lat_lon_displayed.x <= LonMinMax.max )             
      {
         /* The displayed icon position is already inside the visible map 
            extent - do nothing. */
      }
      /* Check if the icon is visible in the center map extent. */
      else if( icon.lat_lon.x >= LonMinMax.min &&
               icon.lat_lon.x <= LonMinMax.max )
      {
         /* The map extent is within +-180 (the "center" map extent):
            use icon position as is.
         */
         icon.lat_lon_displayed.x = icon.lat_lon.x;
         icon.displayed_position_changed = true;
      }
      /* Check if the icon is visible in the left map extent. */
      else if( icon.lat_lon.x - 360. >= LonMinMax.min &&
               icon.lat_lon.x - 360. <= LonMinMax.max )
      {
         /* The map is scrolled beyond -180 degrees to the left and
            the icon is inside "left" extended image of the world: 
            adjust the icon position to be shown inside that extent.
         */               
         icon.lat_lon_displayed.x = icon.lat_lon.x - 360.;
         icon.displayed_position_changed = true;
      }
      /* Check if the icon is visible in the right map extent. */
      else if( icon.lat_lon.x + 360. >= LonMinMax.min &&
               icon.lat_lon.x + 360. <= LonMinMax.max )
      {
         /* The map is scrolled beyond +180 degrees to the right and
            the icon is inside "right" extended image of the world: 
            adjust the icon position to be shown inside that extent.
         */               
         icon.lat_lon_displayed.x = icon.lat_lon.x + 360.;
         icon.displayed_position_changed = true;
      }
      /* else: The icon is not in the center, left or right extent,
         do nothing.
      */
      
      if( DEBUG_POSITION )            
        System.out.println( "Displayed icon position " + 
                            icon.lat_lon_displayed.x );
   }

   //////////////////////////////////////////////////////////////////////////
   // This callback is invoked when user interacts with input objects in GLG
   // drawing. It is also used to handle CustomEvents such as
   // MouseClick or MouseMove.
   //////////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject viewport, GlgObject message_obj )
   {
      super.InputCallback( viewport, message_obj );

      String origin = message_obj.GetSResource( "Origin" );
      String format = message_obj.GetSResource( "Format" );
      String action = message_obj.GetSResource( "Action" );
      String subaction = message_obj.GetSResource( "SubAction" );
      
      // Handle window closing if run stand-alone
      if( format.equals( "Window" ) && action.equals( "DeleteWindow" ) )
        System.exit( 0 );

      if( format.equals( "Button" ) )
      {
         if( !action.equals( "Activate" ) && !action.equals( "ValueChanged" ) )
           return;

         // Push buttons.
         if( action.equals( "Activate" ) )
         {
            if( origin.equals( "ZoomIn" ) )
              Zoom( 'i', MAP_ZOOM_FACTOR );
            else if( origin.equals( "ZoomOut" ) )
              Zoom( 'o', MAP_ZOOM_FACTOR );
            else if( origin.equals( "ZoomReset" ) )
              Zoom( 'n', 0. );
            else if( origin.equals( "ZoomTo" ) )
              Zoom( 't', 0. );  /* Start ZoomTo op */
            else if( origin.equals( "Up" ) )
              Zoom( 'u', MAP_PAN_FACTOR );  
            else if( origin.equals( "Down" ) )
              Zoom( 'd', MAP_PAN_FACTOR );  
            else if( origin.equals( "Left" ) )
              Zoom( 'l', MAP_PAN_FACTOR );  
            else if( origin.equals( "Right" ) )
              Zoom( 'r', MAP_PAN_FACTOR );
         }
         // Pilot View Toggle button.
         else if( action.equals( "ValueChanged" ) &&
                  origin.equals( "PilotViewToggle" ) )
         {
            // Set PilotView mode from the toggle button.
            PilotView = 
              MapViewport.GetDResource( "PilotViewToggle/OnState" ).intValue() != 0;
         }

         MapViewport.Update();
      }
      else if( format.equals( "Zoom" ) )
      {
         if( action.equals( "Zoom" ) && subaction.equals( "End" ) ||
             !USE_SYNC_DRAGGING &&
             action.equals( "Pan" ) && subaction.equals( "End" ) ) 
         {
            /* After ZoomTo or Drag in asynchoronous drag mode are finished,
               disable Pilot View mode. Pilot view is temporarily 
               disabled in SetMapCenter() when these operations are in progress,
               and is automatically resumed if they are aborted.
               Icons will be repositioned when the new asynchronous map request 
               is installed.
            */
            SetPilotView( false );
            DisplayStatus( "Loading new map, please wait." );

            MapViewport.Update();
         } 
         else if( USE_SYNC_DRAGGING && action.equals( "Pan" ) )
         {
            if( subaction.equals( "Drag" ) )
            {
               /* In synchronous drag mode, adjust the icon position 
                  if needed, in case the map is dragged 
                  outside of the +-180 longitude boundary. 
               */
               PositionIcons( true /*update extent*/ );
               MapViewport.Update();
            }
            else if( subaction.equals( "End" ) )
              // Reset ImageLoading mode to the previous value.
              SetMapLoadingMode( AsyncImageLoading );
         }
      }
      else if( format.equals( "CustomEvent" ) &&
               action.equals( "MouseClick" ) )
      {
         /* Map dragging mode is activated on a left mouse click in the Trace 
            callback. Abort the dragging mode if an object with a custom event
            was selected. This gives custom events a higher priority compared 
            to the dragging mode. However, if ZoomTo mode was activated 
            from a ZoomTo button, don't abort ZoomTo mode and ignore 
            object selection.
         */
         int zoom_mode = ZoomToMode();
         if(  zoom_mode == 0 ||
              ( zoom_mode & GlgObject.PAN_DRAG_STATE ) != 0 )
         {
            if( zoom_mode != 0 )
            {
               /* Abort drag mode: zoom_mode is GlgObject.PAN_DRAG_STATE. */
               if( USE_SYNC_DRAGGING )
                 MapViewport.SetZoom( null, 'e', 0. );
               else
                 Zoom( 'e', 0.0 );
            }

            String custom_event = message_obj.GetSResource( "EventLabel" );
            GlgObject selected_obj = message_obj.GetResourceObject( "Object" );
            
            // Do something with the selected object here.
            System.out.println( "Received event: " + custom_event );
            
            // Example:
            if( custom_event.equals( "IconSelected3" ) )  //right click
            {
               // Retrieve icon position.
               GlgPoint lat_lon = new GlgPoint();
               lat_lon = selected_obj.GetGResource( "Position" );
               System.out.println( "MouseButton3, Icon position " + 
                                   lat_lon.x + " " + lat_lon.y );
            }
            else if( custom_event.equals( "IconSelected1" ) ) //left click
            {
               System.out.println( "MouseButton1: " + custom_event );
            }
            
            MapViewport.Update();
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Used to obtain coordinates of the mouse click. 
   //////////////////////////////////////////////////////////////////////////
   public void TraceCallback( GlgObject viewport, GlgTraceData trace_info )
   {      
      if( !IsReady )
        return;
   
      int event_type = trace_info.event.getID();

      /* Handle events in the MapViewport only. Exclude events in 
         light viewports, such as mouse clicks in the PilotView toggle
         if it is a light viewport.
      */
      GlgObject event_vp = ( trace_info.light_viewport != null ? 
                             trace_info.light_viewport : trace_info.viewport );
      if( event_vp != MapViewport )
        return;

      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
         if( ZoomToMode() != 0 )
           return; // ZoomTo or dragging mode in progress.
         
         // Obtain cursor position if needed. 
         double x = (double) ((MouseEvent)trace_info.event).getX();
         double y = (double) ((MouseEvent)trace_info.event).getY();
         
         /* COORD_MAPPING_ADJ is added to the cursor coordinates for 
            precise pixel mapping.
         */
         x += GlgObject.COORD_MAPPING_ADJ;
         y += GlgObject.COORD_MAPPING_ADJ;
         
         // Start dragging the map with the mouse on a mouse click. 

         if( USE_SYNC_DRAGGING )
         {
            /* Use synchronous map loading method on dragging.
               New map image query and display occurs on Swing thread.
            */
            SetMapLoadingMode( false );

            MapViewport.SetZoom( null, 's', 0. );

            SetPilotView( false );
            MapViewport.Update();
         }
         else
         {
            /* Use asynchronous map request on dragging. 
               The new map request will be submitted to the map server 
               when the dragging ends on MouseRelease.
            */
            Zoom( 's', 0. );
         }
         break;
       case ComponentEvent.COMPONENT_RESIZED:
         break;
       default: return;
      } 
   }

   //////////////////////////////////////////////////////////////////////////
   // Enable/Disable asynchronous map image loading mode for the GISObject.
   // If set to false, the application can combine synchronous map image
   // requests with asynchronous requests.
   //////////////////////////////////////////////////////////////////////////
   public void SetMapLoadingMode( boolean async_mode )
   {
      GISObject.SetDResource( "AsyncMode", async_mode ? 1.0 : 0.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   // Perform zoom operation.
   //////////////////////////////////////////////////////////////////////////
   void Zoom( char type, double value )
   {
      switch( type )
      {
       case 'u':
       case 'd':
       case 'l':
       case 'r':
       case 'n':
         SetPilotView( false ); // Disable pilot view mode.
         DisplayStatus( "Loading new map on zoom or scroll, please wait." );
         break;
      }

      switch( type )
      {
       default:
         /* Prepare new map image in a separate thread. The requested image
            will be installed using InstallGISRequest when it is ready.
         */
         MapViewport.RequestGISZoom( null, type, value, this );
         break;
         
       case 'n':
         // Reset map to the initial extent and center.
         ResetMap();
         break;
      }

      /* A request was sent, but GIS object parameters have not changed yet.
         They will be changed to the requested values when the request is 
         installed, and PositionIcons() will be invoked at that time to
         update icon position(s).
      */
   }

   //////////////////////////////////////////////////////////////////////////
   public int ZoomToMode()
   {
      return MapViewport.GetDResource( "ZoomToMode" ).intValue();
   }

   //////////////////////////////////////////////////////////////////////////
   // Enable/disable pilot view mode and set toggle state in the drawing.
   //////////////////////////////////////////////////////////////////////////
   public void SetPilotView( boolean mode )
   {
      PilotView = mode;
      MapViewport.SetDResource( "PilotViewToggle/OnState", PilotView ? 1. : 0. );
   }

   //////////////////////////////////////////////////////////////////////////
   // GIS request observer interface: enables/disables AdjustRequest() 
   // callback. If RequestAdjustment() returns true, AdjustRequest() callback 
   // is invoked, allowing an application to make adjustments to the map request 
   // parameters. Otherwise, AdjustRequest() is not invoked.
   //////////////////////////////////////////////////////////////////////////
   public boolean RequestAdjustment()
   {
      return true;
   }

   //////////////////////////////////////////////////////////////////////////
   // GIS request observer interface: request data adjustment callback.
   // AdjustRequest() is used to make adjustments to the GIS parameters,
   // such as center, extent, layers, projection, etc., before the new 
   // map image request gets processed. 
   // requested_data.flags indicates which GIS parameters were requested
   // to be changed.
   // AdjustRequest() is enabled if RequestAdjustment() returns true.
   //////////////////////////////////////////////////////////////////////////
   public boolean AdjustRequest( GlgGISRequestData request_data )
   {
      int flags = request_data.flags;

      // Diagnostics.
      if( DEBUG_MAP_REQUEST )
      {
         System.out.println( "AjustRequest called." );

         if( ( flags & GlgObject.GIS_REQUEST_EXTENT ) != 0 )
           System.out.println( "Map extent requested: " + 
                               request_data.extent.x +
                               " " + request_data.extent.y );
         
         if( ( flags & GlgObject.GIS_REQUEST_CENTER ) != 0 )
           System.out.println( "Map center requested: " + 
                               request_data.center.x +
                               " " + request_data.center.y );
      }

      /* In rectangular projection, adjust map center if needed,
         to limit scrolling to the allowed map boundaries.
         Adjust only for scrolling, and only if not in a pilot view mode.
         In the pilot view mode, the center lat/lon are in the correct range:
         allow centering on the icon even when looking at the whole world.
         The pilot mode is disabled when scrolling operation is started
         by clicking on a button or by dragging the map.
      */
      if( !PilotView && ( flags & GlgObject.GIS_REQUEST_CENTER ) != 0 &&
          request_data.projection == GlgObject.RECTANGULAR_PROJECTION )
      {
         CheckScrollLimits( request_data );
         return true;
      }

      return false;   // No adjustments
   }

   //////////////////////////////////////////////////////////////////////////
   // GIS request observer interface: receives request status notifications.
   // RequestUpdate() is invoked with the status of the GIS request 
   // when the map request is finished (map image is ready) or aborted.
   //////////////////////////////////////////////////////////////////////////
   public void RequestUpdate( GlgObject gis_object, int status )
   {
      if( DEBUG_MAP_REQUEST )
        System.out.println( "RequestUpdate called." );

      switch( status )
      {
       case GlgObject.GIS_REQUEST_READY:
         if( gis_object.InstallGISRequest( null ) )
         {
            if( DEBUG_MAP_REQUEST )
            {
               GlgPoint extent = GetMapExtent( null );
               GlgPoint center = GetMapCenter( null );
               System.out.println( "New map is installed: Extent " +
                                   extent.x + " " + extent.y + " ; Center " + 
                                   center.x + " " + center.y );
            }

            /* Adjust the icon position if needed, in case the map is dragged 
               outside of the +-180 longitude boundary. 
            */
            PositionIcons( true /*update extent*/ );
            
            DisplayStatus( "" );
            Update(); // display new map image.
         }
         else
          System.out.println( "Failed to install map request." );
         break;

       case GlgObject.GIS_ABORT_ON_NEW_REQUEST:
         System.out.println( "Aborting previous GIS request." );          
         break;

       case GlgObject.GIS_ABORT_BY_API:
         System.out.println( "Aborting GIS request by API." );          
         break;

       case GlgObject.GIS_ABORT_OF_ZOOM_MODE:
         System.out.println( "Aborting GIS dragging/zooming request." );
         break;

       default:
         System.out.println( "Aborting GIS request." ); 
         break;
      }
   }

   //////////////////////////////////////////////////////////////////////
   // Reset map extent and center.
   //////////////////////////////////////////////////////////////////////
   public boolean ResetMap()
   {
      int flags = GlgObject.GIS_REQUEST_EXTENT | GlgObject.GIS_REQUEST_CENTER;

      return GISObject.RequestGISMap( null, InitExtent.x, InitExtent.y, 
                                      InitCenter.x, InitCenter.y, 
                                      0, 0, null, flags, this );
   }

   ////////////////////////////////////////////////////////////////////////
   // In rectangular projection, don't allow scrolling beyond
   // the boundaries of [-540,+540]LON (up to an additional 360 degrees 
   // world extent in the horizontal direction) or [-90,+90]LAT in the 
   // vertical direction.
   // Adjust requested map center if needed, so that the map
   // is scrolled only up to the maximum allowed boundary.
   ////////////////////////////////////////////////////////////////////////
   public void CheckScrollLimits( GlgGISRequestData request_data )
   {
      GlgPoint extent = GetMapExtent( request_data );
      GlgPoint center = GetMapCenter( request_data );

      /* Check longitude boundaries. Adjust map center if the requested map 
         is scrolled beyond allowed longitute boundaries of [-540,+540].
      */
      if( center.x > 360. )
      {
         center.x = 360.;
         if( DEBUG_MAP_REQUEST )
           System.out.println( "LON LIMITS REACHED." );
      }
      else if( center.x < -360 )
      {
         center.x = -360;
         if( DEBUG_MAP_REQUEST )
           System.out.println( "LON LIMITS REACHED." );
      }

      /* Check latitude boundaries. If the map is zoomed out, 
         don't scroll vertically. Othertwise, make sure the map 
         is not scrolled beyond the poles.
      */
      if( extent.y >= 180. )  // map is zoomed out.
      {
         if( center.y != 0. )
         {
            center.y = 0.;
            if( DEBUG_MAP_REQUEST )
              System.out.println( "LAT LIMITS REACHED." );
         }
      }
      else if( center.y + extent.y / 2. > 90. )  // North boundary reached. 
      {
         center.y = 90. - extent.y / 2.;
         if( DEBUG_MAP_REQUEST )
           System.out.println( "LAT LIMITS REACHED." );
      }
      else if( center.y - extent.y / 2. < -90. )   // South boundary reached. 
      {
         center.y = -90. + extent.y / 2.;
         if( DEBUG_MAP_REQUEST )
           System.out.println( "LAT LIMITS REACHED." );
      }
   }

   //////////////////////////////////////////////////////////////////////
   // Set new map center. Send new map request to adjust map center if:
   // - there is no current map request in progress (that includes 
   //   ZoomTo/Drag operation in progress), and
   // - requested map center delta >= MIN_PIXEL_DELTA
   //////////////////////////////////////////////////////////////////////
   public boolean SetMapCenter( GlgPoint center )
   {
      if( GISObject.GetGISRequestInfo( null ) != null )
      {
         if( DEBUG_PILOT_VIEW )
           System.out.println( "Map center request not sent, current request in progress." );

         return false;
      }

      // Check how much new requested center has shifted from the current center.
      CurrCenter = GetMapCenter( null );
      GISObject.GISConvert( null, GlgObject.SCREEN_COORD, false, CurrCenter,
                            point1 );
      GISObject.GISConvert( null, GlgObject.SCREEN_COORD, false, center,
                            point2 );

      /* Send new map request only if the pixel difference between the old and
         new map center exceeds MIN_PIXEL_DELTA in any direction.
      */
      if( Math.abs( point2.x - point1.x ) < MIN_PIXEL_DELTA &&
          Math.abs( point2.y - point1.y ) < MIN_PIXEL_DELTA )
        return false;

      // Request new map with with a new center.
      return GISObject.RequestGISMap( null, 0, 0,
                                      center.x, center.y, 
                                      0, 0, null,   
                                      GlgObject.GIS_REQUEST_CENTER, 
                                      this /*observer*/ );
   }

   //////////////////////////////////////////////////////////////////////
   // Get extent from either request_data or GIS object.
   //////////////////////////////////////////////////////////////////////
   public GlgPoint GetMapExtent( GlgGISRequestData request_data )
   {   
      if( request_data != null )
        return request_data.extent;
      else
        return GISObject.GetGResource( "GISExtent" );
   }

   //////////////////////////////////////////////////////////////////////
   // Get center from either request_data or GIS object.
   //////////////////////////////////////////////////////////////////////
   public GlgPoint GetMapCenter( GlgGISRequestData request_data )
   {   
      if( request_data != null )
        return request_data.center;
      else
        return GISObject.GetGResource( "GISCenter" );
   }
   
   ////////////////////////////////////////////////////////////////////////
   // Get projection from either request_data or GIS object.
   //////////////////////////////////////////////////////////////////////
   public int GetMapProjection( GlgGISRequestData request_data )
   {   
      if( request_data != null )
        return request_data.projection;
      else
        return GISObject.GetDResource( "GISProjection" ).intValue();
   }

   ////////////////////////////////////////////////////////////////////////
   public void DisplayStatus( String message )
   {
      StatusObject.SetSResource( "String", message );
      
      if( DEBUG_MAP_REQUEST && !message.equals( "" ) )
        System.out.println( "####### " + message );
   }
   
   //////////////////////////////////////////////////////////////////////
   // StartUpdates() creates a timer to perform periodic updates.
   // The timer invokes the bean's UpdateGraphProc() method to update
   // drawing's resoures with new data values.
   //////////////////////////////////////////////////////////////////////
   public void StartUpdates()
   {
      if( timer == null )
      {
         Timer timer = new Timer( UPDATE_INTERVAL, this );
         timer.setRepeats( true );
         timer.start();
      }
   }

   //////////////////////////////////////////////////////////////////////
   // StopUpdate() method stops periodic updates
   //////////////////////////////////////////////////////////////////////
   public void StopUpdates()
   {      
      if( timer != null )
      {
         timer.stop();
         timer = null;
      }

      // Disable dynamic updates of the drawing
      IsReady = false;
   }

   //////////////////////////////////////////////////////////////////////////
   // Obtain new icon's position. 
   //////////////////////////////////////////////////////////////////////////
   public void GetIconData()
   {
      /* In this example, there is only one icon. The example can be extended 
         to loop through an array of icons.
      */
      IconData icon = Icon;

      /* Get simulated demo data. Replace this with the real-time 
         data acquisition. 
      */
      GetDemoIconData( icon );

      /* Copy new lat/lon into the displayed icon position. */
      icon.lat_lon_displayed.CopyFrom( icon.lat_lon );
      icon.displayed_position_changed = true;
   }

   //////////////////////////////////////////////////////////////////////////
   // Simulation: uses the center of the map by default. 
   //////////////////////////////////////////////////////////////////////////
   public void GetDemoIconData( IconData icon )
   {
      double radius = InitExtent.x / 20.0;

      // Altitude low/high range.
      double AltLow = 0.;
      double AltHigh = 10000.;

      ++RotationState;
      if( RotationState > 360 )
         RotationState -= 360;
      
      double angle = RotationState;
      double rad_angle = angle / 180. * Math.PI;
      icon.lat_lon.x = InitCenter.x + radius * Math.cos( rad_angle );
      icon.lat_lon.y = InitCenter.y + radius * Math.sin( rad_angle );
      icon.lat_lon.z = 0.;

      icon.altitude = GlgObject.Rand( AltLow, AltHigh );
      icon.yaw = angle + 90.;
      icon.pitch =  GlgObject.Rand( 0., 360. );
      icon.roll = GlgObject.Rand( 0., 360. );
   }

   ////////////////////////////////////////////////////////////////////////
   public void GetLatLonRange( GlgPoint center, GlgPoint extent, 
                               GlgMinMax lon_minmax, GlgMinMax lat_minmax )
   {
      double lon_extent;

      /* If the map is zoomed too far (longitude extent > 360. ),
         only 360 degrees of longitude are displayed.
      */
      lon_extent = extent.x;
      if( lon_extent > 360. )
        lon_extent = 360.;

      lon_minmax.min = center.x - lon_extent / 2.;
      lon_minmax.max = center.x + lon_extent / 2.;
      lat_minmax.min = center.y - extent.y / 2.;
      lat_minmax.max = center.y + extent.y / 2.;
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

   //////////////////////////////////////////////////////////////////////////
   double RELATIVE_TO_NEW_RANGE( double low, double high, double rel_value )
   {
      return ( (low) + ((high) - (low)) * rel_value );
   }

   //////////////////////////////////////////////////////////////////////////
   // For use as a stand-alone application
   //////////////////////////////////////////////////////////////////////////
   public static void main( final String arg[] )
   {
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Main( arg ); } } );
   }

   //////////////////////////////////////////////////////////////////////////
   public static void Main( final String arg[] )
   {
      class DemoQuit extends WindowAdapter
      {
         public void windowClosing( WindowEvent e ) { System.exit( 0 ); }
      } 

      JFrame frame = new JFrame();
      frame.setResizable( true );
      frame.setSize( 900, 600 );
      frame.addWindowListener( new DemoQuit() );

      GlgGISExample gis_example = new GlgGISExample(); 
      frame.getContentPane().add( gis_example );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      //
      gis_example.LoadDrawing( "gis_example.g" );
   }

   class IconData
   {   
      GlgObject icon_obj; // Graphical object representing an icon

      GlgPoint lat_lon;   // Icon position in lat/lon coordinates.

      /* Icon position that might be adjusted, in case the map is scrolled
         outside of the {-180;+180} longitude boundary.
      */
      GlgPoint lat_lon_displayed; 
      boolean displayed_position_changed;

      double altitude;
      double roll;
      double pitch;
      double yaw;

      IconData()
      {
         lat_lon = new GlgPoint();
         lat_lon_displayed = new GlgPoint();
      }
   }
}
