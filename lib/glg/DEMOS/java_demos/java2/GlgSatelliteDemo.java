//////////////////////////////////////////////////////////////////////////
// Satellite Demo: demonstrates the use of the GIS Object to position 
// satellites by specifying lat/lon coordinates and elevation above the Earth.
// The GLG Map Server is used to display a globe in the orthographic 
// projection.
//
// This demo uses GLG as a bean and may be used in a browser or stand-alone.
//
// This demo uses the GLG Map Server to display a globe.
// The Map Server has to be installed either on the local host or on a
// remote web server. After the Map Server has been installed, enable
// the map by modifying the source code to comment out the statement that
// sets the GISDisable resource, set SuppliedMapServerURL to point 
// to the Map Server location and rebuild the demo.
//////////////////////////////////////////////////////////////////////////
import java.awt.*;
import java.awt.event.*;
import java.lang.reflect.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgSatelliteDemo extends GlgJBean implements ActionListener
{
   final int UPDATE_INTERVAL = 20;   // Update interval in msec
   final double ANGLE_DELTA = 10.0;   // Defines pan distance.
   // Simulation parameters. 
   final int SAT1_PERIOD = 500;
   final int SAT2_PERIOD = 400;

   GlgObject
     Drawing,     
     GISObject,
     GISArray,
     Satellite1,
     Satellite2,
     Orbit1,
     Orbit2;   
   int
     NumOrbitPoints = 0,
     UpdateCount = 0;
   GlgPoint 
     Position1 = new GlgPoint(),
     Position2 = new GlgPoint();

   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   // If supplied, overrides the URL of the GIS object in the drawing
   static String SuppliedMapServerURL = null; 

   static boolean StandAlone = false;

   Timer timer = null;

   //////////////////////////////////////////////////////////////////////////
   public GlgSatelliteDemo()
   {
      super();

      AddListener( GlgObject.TRACE_CB, this );
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
   // -map_server_url       - specify MapServerURL; it may be supplied as an
   //                         applet parameter when used as an applet;
   //                         default: use GIS object's MapServerURL property 
   //                         from the drawing file.
   //////////////////////////////////////////////////////////////////////////
   public static void Main( String [] arg )
   {
      class DemoQuit extends WindowAdapter
      {
         public void windowClosing( WindowEvent e ) { System.exit( 0 ); }
      } 

      JFrame frame = new JFrame( "GLG Satellite Demo" );

      frame.setResizable( true );
      frame.setSize( 600, 600 );
      frame.setLocation( 20, 20 );

      GlgSatelliteDemo.StandAlone = true;
      GlgSatelliteDemo satellite_demo = new GlgSatelliteDemo();      

      // Process command line arguments.
      satellite_demo.ProcessArgs( arg );

      frame.getContentPane().add( satellite_demo );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );

      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers ReadyCallback which starts updates.
      //
      satellite_demo.SetDrawingName( "satellite.g" );
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
            if( arg[ skip ].equals( "-map_server_url" ) )
            {
               ++skip;
               if( num_arg <= skip )
                 error( "Missing map server URL.", true );

               GlgSatelliteDemo.SuppliedMapServerURL = arg[ skip ];
            }
            else if( arg[ skip ].equals( "-help" ) )
            {
               error( "Options: -map_server_url <URL>\n", true );
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
      Drawing = viewport;

      // Get ID of the GIS object.
      GISObject = Drawing.GetResourceObject( "GISObject" );   

      /* Get ID of the GIS Object's GISArray, which holds all icons displayed
         on top of the map in lat/lon coordinates. */
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
   // Initializes satellite icons and orbits.
   //////////////////////////////////////////////////////////////////////////
   void Init()
   {
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
      Drawing.SetDResource( "JavaSetupInfo/Visibility", 1.0 );

      ////////////////////////////////////////////////////////////////////////

      // Try to use the GLG map server on the localhost if no URL was supplied.
      if( SuppliedMapServerURL == null )
        SuppliedMapServerURL = "http://localhost/cgi-bin/GlmScript";
      
      if( SuppliedMapServerURL != null )
        // Override the URL defined in the drawing.
        GISObject.SetSResource( "GISMapServerURL", SuppliedMapServerURL );

      // Get IDs of satellite icons.
      Satellite1 = GISArray.GetResourceObject( "Satellite1" );
      Satellite2 = GISArray.GetResourceObject( "Satellite2" );

      /* Get orbit templates and remove them from the drawing initially.
         The templates are polygons with 2 points that store graphical
         attributes of the orbit polygons, to allow user to define the
         attributes interactively via the Graphics Builder. 
         Alternatively, the orbit polygons can be created programmatically.
      */
      Orbit1 = Drawing.GetResourceObject( "Orbit1" );
      Orbit2 = Drawing.GetResourceObject( "Orbit2" );
      Drawing.DeleteObject( Orbit1 );
      Drawing.DeleteObject( Orbit2 );   

      /* Set the GIS Zoom mode. It was set and saved with the drawing, 
         but do it again programmatically just in case.
      */
      Drawing.SetZoomMode( null, GISObject, null, GlgObject.GIS_ZOOM_MODE );
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked after the hierarchy setup.
   //////////////////////////////////////////////////////////////////////////
   public void VCallback( GlgObject viewport )
   {
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Starts updates
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );
      StartUpdates();
   }   

   //////////////////////////////////////////////////////////////////////////
   // Handle user interaction.
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

      if( format.equals( "Button" ) )         // Handle button clicks
      {
         if( !action.equals( "Activate" ) )
           return;

         GlgPoint gis_center = GISObject.GetGResource( "GISCenter" );
         double lon = gis_center.x;
         double lat = gis_center.y;
         
         /* Rotate the globe when a directional button is pressed. */
         if( origin.equals( "Up" ) ) 
           GISObject.SetGResource( "GISCenter", lon, lat + ANGLE_DELTA, 0.0 );
         else if( origin.equals( "Down" ) ) 
           GISObject.SetGResource( "GISCenter", lon, lat - ANGLE_DELTA, 0.0 );
         else if( origin.equals( "Left" ) ) 
           GISObject.SetGResource( "GISCenter", lon - ANGLE_DELTA, lat, 0.0 );
         else if( origin.equals( "Right" ) ) 
           GISObject.SetGResource( "GISCenter", lon + ANGLE_DELTA, lat, 0.0 );
         else
           return;

         Update(); 
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Is used to start globe rotation on a mouse click.
   //////////////////////////////////////////////////////////////////////////
   public void TraceCallback( GlgObject viewport, GlgTraceData trace_info )
   {      
      int event_type = trace_info.event.getID();

      /* Start dragging only on a mouse click in the Drawing area itself,
         not the sliders.
      */
      if( trace_info.viewport != Drawing )
        return;
      
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
         if( GetButton( trace_info.event ) == 1 )
           Drawing.SetZoom( null, 's', 0.0 );
         break;
      }
   }

   ////////////////////////////////////////////////////////////////////////
   // Updates moving objects with new position data.
   ////////////////////////////////////////////////////////////////////////
   void UpdatePosition()
   {
      if( timer == null )
        return;   // Prevents race conditions

      // Obtain new positions for both satellites.
      GetSatellitePosition( 0, Position1 );
      GetSatellitePosition( 1, Position2 );
      
      // Update satellite positions in the drawing with new data.
      Satellite1.SetGResource( "Position", Position1 );
      Satellite2.SetGResource( "Position", Position2 );
      
      /* Update satellite trajectories. */
      if( NumOrbitPoints <= SAT1_PERIOD || NumOrbitPoints <= SAT2_PERIOD )
      {
         if( NumOrbitPoints == 0 )
         {
            /* First time: add orbit polygons to the GIS Object. 
               Use two points initially, set both points to the same lat/lon.
            */
            SetPolygonPoint( Orbit1, 0, Position1 );
            SetPolygonPoint( Orbit1, 1, Position1 );
            
            // Add to top to draw orbits first, with satellites on top of them.
            GISArray.AddObjectToTop( Orbit1 );
            
            SetPolygonPoint( Orbit2, 0, Position2 ); 
            SetPolygonPoint( Orbit2, 1, Position2 ); 
            GISArray.AddObjectToTop( Orbit2 );
         }
         else if( NumOrbitPoints == 1 )
         {
            // Set the second point.
            SetPolygonPoint( Orbit1, 1, Position1 );
            SetPolygonPoint( Orbit2, 1, Position2 );
         }
         else
         {
            // Add one more point.
            if( NumOrbitPoints <= SAT1_PERIOD )
              AddPolygonPoint( Orbit1, Position1, true );
            
            if( NumOrbitPoints <= SAT2_PERIOD )
              AddPolygonPoint( Orbit2, Position2, true );
         }
         
         ++NumOrbitPoints; 
      } 
      ++UpdateCount;
      Update();

      timer.start();   // Restart the update timer
   }

   ////////////////////////////////////////////////////////////////////////
   // FOR SIMULATION ONLY: Calculates satellite postion (lon, lat and 
   // elevation above the Earth in meters) and returns it in the passed
   // position parameter.
   ////////////////////////////////////////////////////////////////////////
   void GetSatellitePosition( int satellite_index, GlgPoint position )
   {
      double rel_value;

      switch( satellite_index )
      {
       case 0:
         rel_value = ( UpdateCount % SAT1_PERIOD ) / (double) SAT1_PERIOD;
         position.x = -180.0 + 360.0 * rel_value;
         position.y = 20.0 * Math.sin( 2.0 * Math.PI * rel_value );
         position.z = 1000000.0;
         break;
         
       default:
       case 1:
         rel_value =
           ( ( 100 + UpdateCount ) % SAT2_PERIOD ) / (double) SAT2_PERIOD;
         position.x = 0.0 + 360.0 * rel_value;
         position.y = 20.0 * Math.sin( 2.0 * Math.PI * rel_value );
         position.z = 1000000.0;
         break;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void SetPolygonPoint( GlgObject polygon, int point_index, GlgPoint position )
   {
      GlgObject point;

      point = (GlgObject) polygon.GetElement( point_index );
      point.SetGResource( null, position );
   }

   //////////////////////////////////////////////////////////////////////////
   void AddPolygonPoint( GlgObject polygon, GlgPoint position,
                         boolean add_at_the_end )
   {
      GlgObject point;

      // Copy the first point and add it at the end of the polygon.
      point = (GlgObject) polygon.GetElement( 0 );
      point = point.CopyObject();

      point.SetGResource( null, position );
      if( add_at_the_end )
        polygon.AddObjectToBottom( point );
      else                // add at the beginning of the polygon.
        polygon.AddObjectToTop( point );
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
   void StartUpdates()
   {
      if( timer == null )
      {
         // Restart the timer after each update (instead of using repeats)
         // to avoid flooding the event queue with timer events on slow 
         // machines.
         timer = new Timer( UPDATE_INTERVAL, this );
         timer.setRepeats( false );
         timer.start();
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // ActionListener method to use the bean as update timer's ActionListener.
   //////////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdatePosition();
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked by the browser to stop the applet
   //////////////////////////////////////////////////////////////////////////
   public void stop()
   {
      StopUpdates();
      super.stop();
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
   void error( String message_str, boolean quit )
   {
      System.out.println( message_str );
      if( quit )
        System.exit( 1 );
   }
}
