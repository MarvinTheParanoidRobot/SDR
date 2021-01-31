import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgGISExample extends GlgJBean implements ActionListener
{
   static final long serialVersionUID = 0;

   // Animation parameters.

   /* If set to true, simulated demo data will used be used for
      animation. Otherwise, live application data will be used.
   */
   final static boolean RANDOM_DATA = true;

   // Time interval for periodic updates, in millisec.
   double UPDATE_INTERVAL = 50.; 

   // GLG Drawing name to be displayed in the GLG control
   static String DRAWING_NAME = "gis_example.g";

   /* Specify MapServerURL as needed if different from the
      setting in the .g file. Assign the specified GIS_MAP_SERVER_URL
      for the GISObject in the Init() function.
   */
   // Windows:
   // String GIS_MAP_SERVER_URL = "http://localhost/Scripts/GlmScript.pl";
   // Linux:
   // String GIS_MAP_SERVER_URL= "http://localhost/cgi-bin/GlmScript";

   // A choice of one of the 2 icon symbols as defined in the .g file.
   // A desired icon is specfied in the Init() function.
   String ICON_NAME1 = "Icon_Triangle";
   String ICON_NAME2= "Aircraft";
  
   GlgObject 
      MapVp,      // Map viewport containing a GIS object
      GISObject;  // GIS object

   // Create an instance of a class to store icon information 
   IconData Icon = new IconData();

   // Store initial extent and center, used to reset the drawing 
   GlgPoint
      InitExtent = new GlgPoint(),
      InitCenter = new GlgPoint();

   boolean IsReady = false;

   // A counter used to calculate icon position in GetIconPosition().
   int RotationState = 0;   

   // Temp vars: allocate once
   GlgPoint lat_lon = new GlgPoint();
   GlgPoint point = new GlgPoint();

   Timer timer = null;

   double MAP_ZOOM_FACTOR = 1.2;
   double MAP_PAN_FACTOR = 0.2;
   double GLG_COORD_MAPPING_ADJ = 0.5;

   //////////////////////////////////////////////////////////////////////////
   public GlgGISExample()
   {
      super();

      // Turn on Java diagnostics
      // SetJavaLog( true );
      
      // Activate Trace callback.
      AddListener( GlgObject.TRACE_CB, this );
   }

   /////////////////////////////////////////////////////////////////////
   // Invoked by the browser to start the applet
   /////////////////////////////////////////////////////////////////////
   public void start()
   {
      super.start();

      // Set a Glg drawing URL to be displayed in the Glg bean.
      // The drawing URL is relative to applet's document base directory.
      SetDrawingURL( DRAWING_NAME );

      // Start periodic updates 
      StartUpdates();
   }

   //////////////////////////////////////////////////////////////////////
   // Invoked by the browser asynchronously to stop the applet.
   //////////////////////////////////////////////////////////////////////
   public void stop()
   {
      // Stop periodic updates
      StopUpdates();

      // GlgJBean handles asynchronous invocation when used as an applet.
      super.stop();
   }

   //////////////////////////////////////////////////////////////////////////
   // HCallback is invoked before the hierarchy setup.
   //////////////////////////////////////////////////////////////////////////
   public void HCallback( GlgObject viewport )
   {
      /* Obtain an object ID of the viewport named "MapVp" and 
         GIS object named "GISObject".
      */
      MapVp = viewport.GetResourceObject( "MapVp" );
      GISObject = MapVp.GetResourceObject( "GISObject" );

      /* Set GIS Zoom mode: generate a new map request for a 
         new area on zoom/pan.
      */
      MapVp.SetZoomMode( null, GISObject, null, GlgObject.GIS_ZOOM_MODE );
      
      // Make icon objects initially invisible.
      String res_name1 = "GISArray/" + ICON_NAME1 + "/Visibility";
      String res_name2 = "GISArray/" + ICON_NAME2 + "/Visibility";
      GISObject.SetDResource( res_name1, 0.0 );
      GISObject.SetDResource( res_name2, 0.0 );

      Init();
   }

   ///////////////////////////////////////////////////////////////////////
   // ReadyCallback is invoked after the drawing has been displayed for
   // the first time.
   ///////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      if( GetJavaLog() )
        PrintToJavaConsole( "Debug: Ready\n" );

      super.ReadyCallback();

      // Enable dynamic updates 
      IsReady = true;      
   }

   //////////////////////////////////////////////////////////////////////
   // Initialize the drawing. 
   //////////////////////////////////////////////////////////////////////
   public void Init()
   {
      /* Query and store initial map extent from the GIS object. named 
         It is used to reset the drawing to the initial extent 
         when the user clicks on the ZoomReset button.
      */
      InitExtent = GISObject.GetGResource( "GISExtent" );
      InitCenter = GISObject.GetGResource( "GISCenter" );

      // Obtain an object ID of the icon object with a specified ICON_NAME.
      String icon_res_name = "GISArray/" + ICON_NAME2;
      Icon.icon_obj = GISObject.GetResourceObject( icon_res_name );
 
      /* Set initial position of the icon. Uses the center of the map
         by default. 
      */
      GetIconPosition( Icon );
      PositionIcon( Icon );

      /* Set a tooltip string for an icon */
      Icon.icon_obj.SetSResource( "TooltipString", "Flight 1237" ); 
      
      // Make the icon visible.
      Icon.icon_obj.SetDResource( "Visibility", 1.0 );

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
        "set GIS_MAP_SERVER_URL to point to the Map Server location and " +
        "rebuild the example.";

      System.out.println( map_server_info );

      // Comment out the next two lines to enable the map.
      GISObject.SetDResource( "GISDisabled", 1.0 );
      MapVp.SetDResource( "MapServerSetupInfo/Visibility", 1.0 );

      ////////////////////////////////////////////////////////////////////////
      //////  Uncomment the line below to set GISMapServerURL, if different
      /////   from this setting in the .g file.
      // GISObject.SetSResource( "GISMapServerURL", GIS_MAP_SERVER_URL );

      ////////////////////////////////////////////////////////////////////////
   }

   ///////////////////////////////////////////////////////////////////////
   // Position an icon on the map.
   ///////////////////////////////////////////////////////////////////////
   public void PositionIcon( IconData icon )
   {
      // Update icon position in the drawing.
      icon.icon_obj.SetGResource( "Position", icon.lat_lon );
      icon.icon_obj.SetDResource( "Yaw", icon.angle );
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked as a timer procedure to perform periodic dynamic
   // updates. In this case, it repositions the icon on the map
   // using simulated data. 
   //////////////////////////////////////////////////////////////////////////
   public void UpdateIconPosition()
   {      
      if( !IsReady() )
        return;

      GetIconPosition( Icon );   // Get new position
      PositionIcon( Icon );      // Set new icon position

      MapVp.Update();    // Make changes visible.
   }

   //////////////////////////////////////////////////////////////////////////
   // This callback is invoked when user interacts with input objects in GLG
   // drawing. It is also used to handle CustomEvents such as
   // MouseClick or MouseMove.
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

      // Handle window closing if run stand-alone
      if( format.equals( "Window" ) && action.equals( "DeleteWindow" ) )
        System.exit( 0 );

      if( format.equals( "Button" ) )
      {
         if( !action.equals( "Activate" ) )
           return;

         if( origin.equals( "ZoomIn" ) )
            Zoom( 'i', MAP_ZOOM_FACTOR );
         else if( origin.equals( "ZoomOut" ) )
            Zoom( 'o', MAP_ZOOM_FACTOR );
         else if( origin.equals( "ZoomReset" ) )
            Zoom( 'n', 0. );
         else if( origin.equals( "ZoomTo" ) )
           MapVp.SetZoom( null, 't', 0. );  /* Start Zoom op */
         else if( origin.equals( "Up" ) )
           Zoom( 'u', MAP_PAN_FACTOR );
         else if( origin.equals( "Down" ) )
           Zoom( 'd', MAP_PAN_FACTOR );
         else if( origin.equals( "Left" ) )
           Zoom( 'l', MAP_PAN_FACTOR );
         else if( origin.equals( "Right" ) )
           Zoom( 'r', MAP_PAN_FACTOR );

         Update();
      }

      // Handle custom mouse click events.
      else if( format.equals( "CustomEvent" ) &&
               action.equals( "MouseClick" ) )
      {
         /* Map dragging mode is activated on a left mouse click in the Trace 
            callback. Abort the dragging mode if an object with custom event
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
               MapVp.SetZoom( null, 'e', 0.0 );  /* Abort zoom/drag mode */
         
             String custom_event = message_obj.GetSResource( "EventLabel" );
             GlgObject selected_obj = message_obj.GetResourceObject( "Object" );
             
             // Do something with the selected object here.
             System.out.println( "Received custom event: " + custom_event );
             
             Update();
         }
      }
   }

   //////////////////////////////////////////////////////////////////////
   // Returns true if the mao is in the ZoomTo mode.
   //////////////////////////////////////////////////////////////////////
   public int ZoomToMode()
   {
      return MapVp.GetDResource( "ZoomToMode" ).intValue();
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Perform zoom operation.
   //////////////////////////////////////////////////////////////////////////
   void Zoom( char type, double value )
   {
      switch( type )
      {
         default:
            MapVp.SetZoom( null, type, value );
            break;
            
         case 'n':
            // Reset map to the initial extent.
            GISObject.SetGResource( "GISCenter", InitCenter );
            GISObject.SetGResource( "GISExtent", InitExtent );
            break;
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
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
         if( ZoomToMode() != 0 )
           return; // ZoomTo or dragging mode in progress: pass it through.
         
         // Retrieve cursor coordinates relatively to the MapVp viewport.
         point.x = (double) ((MouseEvent)trace_info.event).getX();
         point.y = (double) ((MouseEvent)trace_info.event).getY();
         
         /* COORD_MAPPING_ADJ is added to the cursor coordinates for precise
            pixel mapping.
         */
         point.x += GlgObject.COORD_MAPPING_ADJ;
         point.y += GlgObject.COORD_MAPPING_ADJ;
           
         switch( GetButton( trace_info.event ) )
         {
          case 1:  
            // Start dragging the map with the mouse on a left mouse click.
            MapVp.SetZoom( null, 's', 0.0 );
            MapVp.Update();
            break;
          case 2: break;
          case 3:
            /* On Right mouse click, obtain lat/lon values at cursor 
               position and display this information in the drawing.
            */
            if( GetLatLonInfo( point, lat_lon ) )
              ShowInfoDisplay( true, lat_lon );
            else
              ShowInfoDisplay( true, null );
            Update();
            break;
         }
         break;
         
       case MouseEvent.MOUSE_WHEEL:
         /* In this example, the map is zoomed in/out by the specified factor
            for each mouse wheel scroll event. The application may extend the
            logic to fine-tune the map scrolling amount, based on num_rotations.
         */
         int num_rotations = 
           ((MouseWheelEvent)trace_info.event).getWheelRotation();
         
         if( num_rotations > 0 ) // Mousewheel is scrolled up, zoom in the map.
           Zoom( 'i', MAP_ZOOM_FACTOR ); 
         else // Mousewheel is scrolled down, zoom out the map.
           Zoom( 'o', MAP_ZOOM_FACTOR );
         
         MapVp.Update();

         break;

       case KeyEvent.KEY_PRESSED:
         // Obtain key code.
         int key = ((KeyEvent)trace_info.event).getKeyCode();

         // Handle key codes as needed.
         switch( key )
         {
          case KeyEvent.VK_ESCAPE:
            // Erase text information displayed at the bottom of the drawing.
            ShowInfoDisplay( false, null );
            Update();
            break;
          default: break;
         }
         break;

       default: break;
      }
   }

   /////////////////////////////////////////////////////////
   // Retrieve lat/lon information at the specified cursor 
   // position.
   /////////////////////////////////////////////////////////
   public boolean GetLatLonInfo( GlgPoint in_point, 
                                 GlgPoint out_point )
   {
      // Convert cursor position to lat/lon.
      return GISObject.GISConvert( null, GlgObject.SCREEN_COORD,
                                   /* X/Y to Lat/Lon */ true, 
                                   in_point, out_point );
   }

   /////////////////////////////////////////////////////////
   public void ShowInfoDisplay( boolean show, GlgPoint lat_lon )
   {
      /* InfoObject/Status resource toggles text display:
         Status=0 displays general prompt;
         Status=1 displays lat/lon values.
      */
      SetDResource( "InfoObject/Status", show ? 1. : 0. );
      SetDResource( "InfoObject/Visibility", 1.0 );

      if( lat_lon == null )
      {
         SetDResource( "InfoObject/LAT", -1000. );
         SetDResource( "InfoObject/LON", -1000. );
      }
      else
      {
         SetDResource( "InfoObject/LAT", lat_lon.y );
         SetDResource( "InfoObject/LON", lat_lon.x );
      }
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
         Timer timer = new Timer( 200, this );
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
   // Animation: obtain a new icon's position.
   // In an application, update the position from real data source.
   //////////////////////////////////////////////////////////////////////////
   public void GetIconPosition( IconData icon )
   {   
      if( RANDOM_DATA )
        GetDemoData( icon );
      else
        GetLiveData( icon );
   }

   //////////////////////////////////////////////////////////////////////////
   // Generate simulated demo data.
   //////////////////////////////////////////////////////////////////////////
   public boolean GetDemoData( IconData icon )
   {
      double radius = InitExtent.x / 5.0;

      ++RotationState;
      if( RotationState > 360 )
         RotationState -= 360;
      
      double angle = RotationState;
      double rad_angle = angle / 180. * Math.PI;
      icon.lat_lon.x = InitCenter.x + radius * Math.cos( rad_angle );
      icon.lat_lon.y = InitCenter.y + radius * Math.sin( rad_angle );
      icon.lat_lon.z = 0.;
      icon.angle = angle + 90.;

      return true;
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Generate simulated demo data.
   //////////////////////////////////////////////////////////////////////////
   public boolean GetLiveData( IconData icon )
   {
      // Place custom code here to obtain live application data.
      return true;
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

   //////////////////////////////////////////////////////////////////////
   // timer's ActionListener method to be invoked peridically 
   //////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateIconPosition();
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
      frame.setSize( 600, 500 );
      frame.addWindowListener( new DemoQuit() );

      GlgGISExample gis_example = new GlgGISExample(); 
      frame.getContentPane().add( gis_example );
      frame.setVisible( true );
 
      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      //
      gis_example.SetDrawingName( DRAWING_NAME );
      
      // Start periodic updates.
      gis_example.StartUpdates();
   }

   class IconData
   {   
      GlgObject icon_obj; // Graphical object representing an icon
      GlgPoint lat_lon;   // Icon position in lat/lon coordinates
      double angle;       // Icon's rotation angle

      IconData()
      {
         lat_lon = new GlgPoint();
      }
   }
}
