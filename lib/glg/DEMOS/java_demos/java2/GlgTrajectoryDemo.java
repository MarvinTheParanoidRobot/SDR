//////////////////////////////////////////////////////////////////////////
// Trajectory Demo: demonstrates the use of the GIS Object to display
// a 3D trajectory by specifying lat/lon coordinates and elevation above 
// the Earth. The GLG Map Server is used to display a globe in the 
// orthographic projection.
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
public class GlgTrajectoryDemo extends GlgJBean implements ActionListener
{
   final int UPDATE_INTERVAL = 30;    // Update interval in msec
   final int NUM_ITERATIONS = 500;
   // Display a drop line for every DROP_LINE_INTERVAL iterations.
   final int DROP_LINE_INTERVAL = 20;    

   GlgObject [] CraftIcon = new GlgObject[2];
   GlgObject
     Drawing,     
     Map,        // Map viewport
     GISObject,
     GISArray,
     TrajectoryEdgeTemplate,
     TrajectoryFillTemplate,
     DropLineTemplate,
     Craft = null,
     TrajectoryEdge = null,
     TrajectoryFill = null,
     DropLineArray = null,
     /* Provides visual feedback when the user defines the trajectory's start
        and end points with the mouse. */
     StartEndPolygon = null;

   GlgPoint GISCenter;                     /* Center of the GIS projection. */
   GlgPoint PrevPosition = new GlgPoint(); /* The last craft position, is used 
                                              to calculate heading angles. */
   GlgPoint Position = new GlgPoint();
   GlgPoint GroundPosition = new GlgPoint();
   GlgPoint CursorPosition = new GlgPoint();

   int UpdateCount = 0;
   int ZoomLevel = 2;
   boolean Pause = false;

   /**** Simulation parameters ****/
   double TrajectoryHeight = 400000.0; /* Maximum height of the trajectory,
                                          may be changed with a slider. */
   double Curvature = -1.75;           /* Trajectory curvature value, 
                                          may be changed with a slider. */
   double CurvatureY;
   double TrajectoryLength;  /* Used for simulation; in abstract units. */

   /* Initial values for the start and end of the trajectory; may be changed
      interactively using the mouse. */
   GlgPoint
     StartPoint = new GlgPoint( -80.644861,       28.572872,       0.0 ),
     EndPoint   = new GlgPoint( -80.644861 + 5.0, 28.572872 - 3.0, 0.0 );

   // Is used to define the trajectory's start and end points with the mouse.
   GlgPoint Point1 = new GlgPoint();

   // Temporary objects used for simulation math.
   GlgPoint CraftAngles = new GlgPoint();
   GlgPoint CurrXYZ = new GlgPoint();
   GlgPoint PrevXYZ = new GlgPoint();
   GlgPoint TrajectoryVector = new GlgPoint();
   // Normal to the trajectory's general direction.
   GlgPoint NormalVector = new GlgPoint();

   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   // If supplied, overrides the URL of the GIS object in the drawing
   static String SuppliedMapServerURL = null; 

   static boolean StandAlone = false;

   Timer timer = null;

   //////////////////////////////////////////////////////////////////////////
   public GlgTrajectoryDemo()
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

      JFrame frame = new JFrame( "GLG Trajectory Demo" );

      frame.setResizable( true );
      frame.setSize( 600, 600 );
      frame.setLocation( 20, 20 );

      GlgTrajectoryDemo.StandAlone = true;
      GlgTrajectoryDemo trajectory_demo = new GlgTrajectoryDemo();      

      // Process command line arguments.
      trajectory_demo.ProcessArgs( arg );

      frame.getContentPane().add( trajectory_demo );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );

      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers ReadyCallback which starts updates.
      //
      trajectory_demo.SetDrawingName( "trajectory.g" );
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

               GlgTrajectoryDemo.SuppliedMapServerURL = arg[ skip ];
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

      /* Get ID of the map viewport. */
      Map = Drawing.GetResourceObject( "Map" );

      // Get ID of the GIS object.
      GISObject = Map.GetResourceObject( "GISObject" );   

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
   // Initializes trajectory icons and orbits.
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
      Map.SetDResource( "JavaSetupInfo/Visibility", 1.0 );

      ////////////////////////////////////////////////////////////////////////

      // Try to use the GLG map server on the localhost if no URL was supplied.
      if( SuppliedMapServerURL == null )
        SuppliedMapServerURL = "http://localhost/cgi-bin/GlmScript";
      
      if( SuppliedMapServerURL != null )
        // Override the URL defined in the drawing.
        GISObject.SetSResource( "GISMapServerURL", SuppliedMapServerURL );

      /* Get trajectory templates and remove them from the drawing initially.
         Each template is a polygon with 2 points that stores graphical
         attributes of the trajectory, to allow the user to define 
         the attributes interactively via the Graphics Builder.
         One polygon is used for the edge of the trajectory, and another
         polygon is used for semi-transparent fill.
         Alternatively, template polygons can be created programmatically.
      */
      TrajectoryEdgeTemplate = 
        Map.GetResourceObject( "TrajectoryEdgeTemplate" );
      Map.DeleteObject( TrajectoryEdgeTemplate );
      
      TrajectoryFillTemplate = 
        Map.GetResourceObject( "TrajectoryFillTemplate" );
      Map.DeleteObject( TrajectoryFillTemplate );

      /* Vertical drop lines. */
      DropLineTemplate = Map.GetResourceObject( "DropLineTemplate" );
      Map.DeleteObject( DropLineTemplate );

      /* Delete craft icons from the top drawing, where it was placed for 
         the ease of editing. The selected icon will be added inside the 
         GISArray to position it in lat/lon. 
      */
      for( int i=0; i<2; ++i )
      {
         CraftIcon[i] = Map.GetResourceObject( i != 0 ? "Craft1" : "Craft0" );
         Map.DeleteObject( CraftIcon[ i ] );
      }

      /* Query the center of the GIS projection. */
      GISCenter = GISObject.GetGResource( "GISCenter" );

      /* Set initial trajectory height and curvature to predefined values. */
      Drawing.SetDResource( "Toolbar/HeightSlider/ValueX", TrajectoryHeight );
      Drawing.SetDResource( "Toolbar/CurvatureSlider/ValueX", Curvature );

      /* Set the GIS Zoom mode. It was set and saved with the drawing, 
         but do it again programmatically just in case.
      */
      Drawing.SetZoomMode( null, GISObject, null, GlgObject.GIS_ZOOM_MODE );

      SetZoomLevel();
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
         if( !action.equals( "Activate" ) && !action.equals( "ValueChanged" ) )
           return;

         if( origin.equals( "Restart" ) )
         {
            Restart();
         }
         else if( origin.equals( "PauseResume" ) )
         {
            Pause =
              ( message_obj.GetDResource( "OnState" ).doubleValue() == 0.0 );
         }
         else if( origin.equals( "IconType" ) )
         {
            CreateCraftIcon();   /* Change icon. */
            
            // Set craft position and angles, same as in UpdatePosition().
            if( Pause && UpdateCount != 0 ) 
            {
               GetCraftPosition( Position ); 
               Craft.SetGResource( "Position", Position ); 
               SetCraftAngles( Position, PrevPosition ); 
               Craft.SetDResource( "Visibility", 1.0 ); 
               Update();
            }
         }
         else if( origin.equals( "Up" ) )
           Pan( 'u' );
         else if( origin.equals( "Down" ) )
           Pan( 'd' );
         else if( origin.equals( "Left" ) )
           Pan( 'l' );
         else if( origin.equals( "Right" ) )
           Pan( 'r' );
         else if( origin.equals( "ZoomIn" ) )
         {
            if( ZoomLevel < 4 )
            {
               ++ZoomLevel;
               SetZoomLevel();
            }
         }
         else if( origin.equals( "ZoomOut" ) )
         {
            if( ZoomLevel > 0 )
            {
               --ZoomLevel;
               SetZoomLevel();
            }
         }
      }     
   }

   //////////////////////////////////////////////////////////////////////////
   // Restart simulation with new parameters.
   //////////////////////////////////////////////////////////////////////////
   void Restart()
   {
      UpdateCount = 0;   // Reset update counter
      
      // Resume if was paused.
      Pause = false;
      Drawing.SetDResource( "Toolbar/PauseResume/OnState", 1.0 );
      Update();
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Pan the map in the specified direction.
   //////////////////////////////////////////////////////////////////////////
   void Pan( char direction )
   {
      double distance;

      /* Pan in lat/lon instead of screen pixels to allow contineous rotation
         of the globe upside down.
      */
      switch( direction )
      {
       default:
       case 'u': direction = 'Y'; distance =  8.0; break;
       case 'd': direction = 'Y'; distance = -8.0; break;
       case 'l': direction = 'X'; distance = -8.0; break;
       case 'r': direction = 'X'; distance =  8.0; break;
      }

      Map.SetZoom( null, direction, distance );
      Map.Update();
      
      /* Query the new center of the GIS projection. */
      GISCenter = GISObject.GetGResource( "GISCenter" );
   }

   //////////////////////////////////////////////////////////////////////////
   void SetZoomLevel()
   {
      double scale;
      
      switch( ZoomLevel )
      {
       case 0: scale = 3.0; break;
       case 1: scale = 5.0; break;
       default:
       case 2: scale = 8.25; break;
       case 3: scale = 10.0; break;
       case 4: scale = 12.0; break;
      }

      /* Instead of the GIS zoom, GIS object is scaled (via the attached scale 
         transformation) to have the GIS object partially clipped out with 
         the top of it visible as a horizon line.      
         Scaling the GIS object does not change projection center.
      */
      GISObject.SetDResource( "GISScale", scale );
      
      // Set scaling limits depending on the resolution of available data.
      Map.SetDResource( "ZoomIn/DisableInput", ZoomLevel >= 4 ? 10.0 : 0.0 );
      Map.SetDResource( "ZoomOut/DisableInput", ZoomLevel <= 0 ? 1.0 : 0.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates moving object with new position data.
   //////////////////////////////////////////////////////////////////////////
   void UpdatePosition()
   {   
      GlgObject
        trajectory,
        drop_line;
      int i;

      if( timer == null )
        return;   // Prevents race conditions

      if( !Pause )
      {
         if( UpdateCount == NUM_ITERATIONS )
           UpdateCount = 0; // Reached the end of the trajectory: start over.
         
         // Obtain new craft position. 
         GetCraftPosition( Position );

         if( UpdateCount == 0 )     // First time.
         {
            InitSimulationParameters();
            CreateCraftIcon();
         }
         else
         {
            /* Update craft position in the drawing with new data.
               Also update angles: previous position is valid.
            */
            Craft.SetGResource( "Position", Position );      
            SetCraftAngles( Position, PrevPosition );

            Craft.SetDResource( "Visibility", 1.0 );  // Make it visible.
         }

         // Store previous position to calculate heading angles.
         PrevPosition.CopyFrom( Position );

         // Update craft trajectory.
         if( UpdateCount == 0 )
         {
            // First time: add trajectory polygons to the GIS Object. 
            if( TrajectoryEdge != null )
            {
               // Destroy previous trajectory polygons.
               GISArray.DeleteObject( TrajectoryEdge );
               TrajectoryEdge = null;
            
               GISArray.DeleteObject( TrajectoryFill );
               TrajectoryFill = null;
            
               if( DropLineArray != null )
               {
                  GISArray.DeleteObject( DropLineArray );
                  DropLineArray = null;
               }
            }
            
            for( i=0; i<2; ++i )
            {
               if( i == 0 )
               {
                  TrajectoryEdge = TrajectoryEdgeTemplate.CopyObject();
                  trajectory = TrajectoryEdge;
               }
               else
               {
                  TrajectoryFill = TrajectoryFillTemplate.CopyObject();
                  trajectory = TrajectoryFill;
               }

               // Use two points initially, set both points to the same lat/lon.
               SetPolygonPoint( trajectory, 0, Position );
               SetPolygonPoint( trajectory, 1, Position );
            
               /* Add to top to draw the trajectory first, with the craft 
                  on top of it. */
               GISArray.AddObjectToTop( trajectory );
            }
         }
         else
         {
            GroundPosition.CopyFrom( Position );
            GroundPosition.z = 0.0;   // Zero elevation

            for( i=0; i<2; ++i )
            {
               trajectory = ( i==0 ? TrajectoryEdge : TrajectoryFill );
               
               /* Add two points for every postion: one using the trajectory 
                  elevation, and one at the ground level to show the ground 
                  position.
               */
               AddPolygonPoint( trajectory, Position, true );
               AddPolygonPoint( trajectory, GroundPosition, false );
            }
         
            // Display a drop line for every DROP_LINE_INTERVAL iterations.
            if( ( UpdateCount % DROP_LINE_INTERVAL ) == 0 )
            {
               if( DropLineArray == null )
               {
                  int position;
               
                  // First drop line: create a group to hold droplines.
                  DropLineArray = 
                    new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
               
                  // Add after trajectory polygons but before the craft.
                  position = GISArray.GetIndex( Craft );
                  GISArray.AddObjectAt( DropLineArray, position );
               }
            
               drop_line = DropLineTemplate.CopyObject();
               
               /* Draw a line from craft to the ground point (elev=0) with the
                  same lat/lon. */
               SetPolygonPoint( drop_line, 0, Position );
               SetPolygonPoint( drop_line, 1, GroundPosition );
            
               DropLineArray.AddObjectToTop( drop_line );
            }
         }
         
         ++UpdateCount;
         Update();
      }

      timer.start();   // Restart the update timer
   }

   //////////////////////////////////////////////////////////////////////////
   // Creates a craft icon based on the icon type requested by the IconType 
   // toolbar button.
   //////////////////////////////////////////////////////////////////////////
   void CreateCraftIcon()
   {
      int icon_type;
      
      // Delete previously used craft icon.
      if( Craft != null )
      {
         GISArray.DeleteObject( Craft );
         Craft = null;
      }
   
      // Query icon type requested by the IconType toolbar button.
      icon_type = 
        ( Drawing.GetDResource( "Toolbar/IconType/OnState" ).doubleValue() == 0 ? 0 : 1 );

      // Add a craft icon.
      Craft = CraftIcon[ icon_type ];

      // Make invisible initially: has no position or angle information.
      Craft.SetDResource( "Visibility", 0.0 );

      // Add to the drawing inside the GIS object to position in lat/lon.
      GISArray.AddObjectToBottom( CraftIcon[ icon_type ] );
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Is used to obtain coordinates of the mouse click when defining 
   // the trajectory's start and end points with the mouse.
   //////////////////////////////////////////////////////////////////////////
   public void TraceCallback( GlgObject viewport, GlgTraceData trace_info )
   {      
      int event_type = trace_info.event.getID();

      if( event_type == MouseEvent.MOUSE_PRESSED &&
          GetButton( trace_info.event ) != 1 )
      {
         AbortStartEndMode();	 
         return;  // Use the left button clicks only.
      }

      /* Use the Map area events only. */
      if( trace_info.viewport != Map )
        return;
      
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
         if( GetButton( trace_info.event ) != 1 )
         {
            AbortStartEndMode();	 
            return;  // Use the left button clicks only.
         }
         break;
      }

      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
         CursorPosition.x = (double) ((MouseEvent)trace_info.event).getX();
         CursorPosition.y = (double) ((MouseEvent)trace_info.event).getY();
         CursorPosition.z = 0.0;

         /* COORD_MAPPING_ADJ is added to the cursor coordinates for 
            precise pixel mapping. */
         CursorPosition.x += GlgObject.COORD_MAPPING_ADJ;
         CursorPosition.y += GlgObject.COORD_MAPPING_ADJ;

         /* Convert screen coordinates of the mouse to the world coordinates 
            inside the GIS Object's GISArray - lat/lon. */ 
         GISArray.ScreenToWorld( true, CursorPosition, Position );
         Position.z = 0.0;   // Use elevation = 0
                     
         if( StartEndPolygon == null )   // First time: start point.
         {
            StartEndPolygon = new GlgPolygon( 2, null );
            StartEndPolygon.SetGResource( "EdgeColor", 1.0, 1.0, 0.0 );
         
            // Initially, set both points to the the mouse position.
            Point1.CopyFrom( Position );
            SetPolygonPoint( StartEndPolygon, 0, Point1 );
            SetPolygonPoint( StartEndPolygon, 1, Point1 );

            GISObject.AddObjectToBottom( StartEndPolygon );
            Map.Update();
         }
         else // Second time: end point.
         {
            // Done with defining trajectory: store the new start and end point.
            StartPoint.CopyFrom( Point1 );
            EndPoint.CopyFrom( Position );

            AbortStartEndMode();   // Delete StartEndPolygon.

            // Set new points if trajectory length != 0.
            if( StartPoint.x != EndPoint.x || StartPoint.y != EndPoint.y )
            {
               // Set a smaller curvature to follow the start/end line.
               Curvature = -0.2;
               Drawing.SetDResource( "Toolbar/CurvatureSlider/ValueX", 
                                     Curvature ); 
            
               Restart();             // Restart with the new trajectory.
               Update();
            }
         }
         break;
         
       case MouseEvent.MOUSE_MOVED:
       case MouseEvent.MOUSE_DRAGGED:
         if( StartEndPolygon != null )
         {
            // Start point has been defined: show the end point.
            CursorPosition.x = (double) ((MouseEvent)trace_info.event).getX();
            CursorPosition.y = (double) ((MouseEvent)trace_info.event).getY();
            CursorPosition.z = 0.0;
            
            /* COORD_MAPPING_ADJ is added to the cursor coordinates for 
               precise pixel mapping. */
            CursorPosition.x += GlgObject.COORD_MAPPING_ADJ;
            CursorPosition.y += GlgObject.COORD_MAPPING_ADJ;

            /* Convert screen coordinates of the mouse to the world coordinates 
               inside the GIS Object's GISArray - lat/lon. */ 
            GISArray.ScreenToWorld( true, CursorPosition, Position );
            Position.z = 0.0;   // Use elevation = 0

            SetPolygonPoint( StartEndPolygon, 1, Position );
            Map.Update();
         }
         break;
         
       default: return;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Deletes StartEndPolygon if created.
   //////////////////////////////////////////////////////////////////////////
   void AbortStartEndMode()
   {
      if( StartEndPolygon != null )
      {
         if( GISObject.ContainsObject( StartEndPolygon ) )
           GISObject.DeleteObject( StartEndPolygon );
         
         StartEndPolygon = null;
         Map.Update();
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Simulation: returns craft postion (lon, lat and elevation above the
   // Earth in meters) via the passed position parameter.
   //////////////////////////////////////////////////////////////////////////
   void GetCraftPosition( GlgPoint position )
   {
      double rel_value, rel_sin;
      
      rel_value = ( UpdateCount % NUM_ITERATIONS ) / (double) NUM_ITERATIONS;
      rel_sin = Math.sin( Math.PI * rel_value );

      // Lon
      position.x = StartPoint.x + ( EndPoint.x - StartPoint.x ) * rel_value +
        NormalVector.x * TrajectoryLength * Curvature * rel_sin;

      // Lat
      position.y = StartPoint.y + ( EndPoint.y - StartPoint.y ) * rel_value +
        NormalVector.y * TrajectoryLength * CurvatureY * rel_sin;

      // Elevation
      position.z = TrajectoryHeight * rel_sin;
   }

   //////////////////////////////////////////////////////////////////////////
   // Simulation: Set craft angles depending on its position on the globe
   // and asimuthal angle. Angles are not needed if a simpler marker icon
   // is used to mark position of the craft.
   //////////////////////////////////////////////////////////////////////////
   void SetCraftAngles( GlgPoint curr_position, GlgPoint prev_position )
   {
      GetGlobeAngles( curr_position, prev_position, CraftAngles );

      double yaw = CraftAngles.x;
      double pitch = CraftAngles.y;
      double roll = CraftAngles.z;

      Craft.SetDResource( "Roll", roll );
      Craft.SetDResource( "Pitch", pitch );
      Craft.SetDResource( "Yaw", yaw );
   }

   //////////////////////////////////////////////////////////////////////////
   // Simulation: calculates craft's heading angles based on the current
   // and previous position. The angles are returned via the passed angles
   // parameter.
   //////////////////////////////////////////////////////////////////////////
   void GetGlobeAngles( GlgPoint curr_position, GlgPoint prev_position,
                        GlgPoint angles )
   {
      double 
        dx, dy, dz,
        rel_value,
        yaw, pitch, roll;
      
      /* Calculate angles using XYZ position on the globe. If Z coordinate
         is not required, GlgGISConvert() may be used to get X and Y screen 
         coordinates in pixels instead of GetPointXYZ().
      */
      
      /* XYZ of the first point, in meters */
      GetPointXYZ( curr_position, CurrXYZ );
      
      /* XYZ of the second point, in meters */
      GetPointXYZ( prev_position, PrevXYZ );

      dx = CurrXYZ.x - PrevXYZ.x;
      dy = CurrXYZ.y - PrevXYZ.y;
      dz = CurrXYZ.z - PrevXYZ.z;

      /* Visible heading angles on the globe in the current projection. */
      if( dx == 0.0 )
        yaw = ( dy != 0.0 ? ( dy > 0.0 ? 90.0 : -90.0 ) : 0.0 );
      else
      {
         yaw = Math.atan( dy / dx );
         yaw = RadToDeg( yaw );
         if( dx < 0.0 )
           yaw += 180.0;
      }

      if( dx == 0.0 && dy == 0.0 )
        pitch = - ( dz != 0.0 ? ( dz > 0.0 ? 90.0 : -90.0 ): 0.0 );
      else
      {
         pitch = - Math.atan( dz / Math.sqrt( dx * dx + dy * dy ) / 3.0 );
         pitch = RadToDeg( pitch );
      }

      rel_value = ( UpdateCount % NUM_ITERATIONS ) / (double) NUM_ITERATIONS;

      // Roll it a bit. In an application, supply a real roll angle.
      roll = 
        -60.0 * Math.sin( DegToRad( yaw ) ) * Math.sin( Math.PI * rel_value );

      angles.x = yaw;
      angles.y = pitch;
      angles.z = roll;
   }

   //////////////////////////////////////////////////////////////////////////
   // Simulation: Initializes simulation parameters.
   //////////////////////////////////////////////////////////////////////////
   void InitSimulationParameters()
   {
      double max_lat;
      
      /* Query requested trajectory height from the slider. */
      TrajectoryHeight = 
        Drawing.GetDResource( "Toolbar/HeightSlider/ValueX" ).doubleValue();

      /* Query requested trajectory curvature from the slider. */
      Curvature = 
        Drawing.GetDResource( "Toolbar/CurvatureSlider/ValueX" ).doubleValue();

      TrajectoryVector.x = EndPoint.x - StartPoint.x;
      TrajectoryVector.y = EndPoint.y - StartPoint.y;
      
      // Obtain a normal vector used to curve the simulated trajectory.
      GetNormalVector( TrajectoryVector, NormalVector );

      /* Calculate trajectory length in abstract units for simulation. */
      TrajectoryLength = Math.sqrt( TrajectoryVector.x * TrajectoryVector.x +
                                    TrajectoryVector.y * TrajectoryVector.y );

      /* Point with the maximum lat. */
      max_lat = StartPoint.y + ( EndPoint.y - StartPoint.y ) * 
        NormalVector.y * TrajectoryLength * Curvature;
      if( max_lat > 89.0 )
        max_lat = 89.0;
      else if( max_lat < -89.0 )
        max_lat = -89.0;
      else 
        max_lat = 0.0;
      
      // Limit Y curvature to avoid flying "off the globe".
      if( max_lat != 0.0 )
        CurvatureY = ( max_lat - StartPoint.y ) / 
          ( NormalVector.y * TrajectoryLength * ( EndPoint.y - StartPoint.y ) );
      else
        CurvatureY = Curvature;
   }

   //////////////////////////////////////////////////////////////////////////
   // Simulation: Returns 2D normal vector of length=1.
   //////////////////////////////////////////////////////////////////////////
   void GetNormalVector( GlgPoint vector, GlgPoint normal )
   {
      double length;
      
      if( vector.x == 0.0 )
      {
         if( vector.y == 0.0 )        
           normal.x = 0.0;    // NULL vector: return NULL as a normal.
         else
           normal.x = 1.0;
         
         normal.y = 0.0;
      }
      else if( vector.y == 0.0 )
      {
         normal.x = 0.0;
         normal.y = 1.0;
      }
      else
      {
         normal.x = vector.y;
         normal.y = - vector.x;
         length = Math.sqrt( normal.x * normal.x + normal.y * normal.y );
         if( length != 1.0 )
         {
            normal.x /= length;
            normal.y /= length;
         }
      }
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Returns XYZ coordinate of the lat/lon point, in meters.
   //////////////////////////////////////////////////////////////////////////
   void GetPointXYZ( GlgPoint lat_lon, GlgPoint xyz )
   {
      double
        globe_radius,
        phi, phi1, lon_diff,
        cos_phi, sin_phi,
        cos_phi1, sin_phi1,
        cos_lon_diff, sin_lon_diff;
      
      phi = DegToRad( lat_lon.y );
      phi1 = DegToRad( GISCenter.y );
      lon_diff = DegToRad( lat_lon.x - GISCenter.x );
      
      // Use the average value plus elevation.
      globe_radius = 
        ( GlgObject.EQUATOR_RADIUS + GlgObject.POLAR_RADIUS ) / 2.0;
      globe_radius += lat_lon.z;
      
      cos_phi = Math.cos( phi );
      sin_phi = Math.sin( phi );
      cos_phi1 = Math.cos( phi1 );
      sin_phi1 = Math.sin( phi1 );
      cos_lon_diff = Math.cos( lon_diff );
      sin_lon_diff = Math.sin( lon_diff );
      
      xyz.x = globe_radius * cos_phi * sin_lon_diff;
      xyz.y = globe_radius * 
        ( cos_phi1 * sin_phi - sin_phi1 * cos_phi * cos_lon_diff );
      xyz.z = globe_radius *
        ( cos_lon_diff * cos_phi1 * cos_phi + sin_phi1 * sin_phi );
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
