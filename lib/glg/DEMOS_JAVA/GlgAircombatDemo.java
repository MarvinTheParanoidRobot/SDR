//////////////////////////////////////////////////////////////////////////
// This program simulates a missile fight. It creates a number of planes
// and missiles, and animates them. Every missile is following one plane 
// until it destroys the plane. Initailly, the missiles are going slower
// than the planes, but eventually they are catching up.
//
// This demo uses the GLG Map Server to display a map.
// The Map Server has to be installed either on the local host or on a
// remote web server. After the Map Server has been installed, enable
// the map by modifying the source code to comment out the statement that 
// sets the GISDisable resource, set SuppliedMapServerURL to point to 
// the Map Server location and rebuild the demo.
//////////////////////////////////////////////////////////////////////////
import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

public class GlgAircombatDemo extends GlgJBean implements ActionListener
{
   static final long serialVersionUID = 0;

   static boolean StandAlone = false;

   // If supplied, overrides the URL of the GIS object in the drawing
   static String SuppliedMapServerURL = null; 

   // Some constants defining simulation parameters.
   static final double
     MAX_DISTANCE      = 0.8,   // Max allowed distance from the center,
                                // in relative units.
     DESTROY_DISTANCE  = 0.03,  // How close a missile needs to get to a plane 
                                // to destroy it (relative units).
     CHANGE_THRESHOLD  = 0.99,  // The number in the range [0;1]; defines 
                                // how often planes change direction. 
     PLANE_SPEED       = 0.01,  // Plane's speed 
     MISSILE_SPEED     = 0.6;   // Initial speed relatively to the plane
   static final int
     UPDATE_INTERVAL   = 30,    // Update interval in msec
     TRANSITION_NUM    = 20,    // The number of steps required for a turn 
     DELAY             =  2,    // Delay after a turn 
     MISSILE_DELAY     = 10;    // Missile adjustment delay 

   // Global variables.
   int     
     NumPlanePairs = 8,
     NumPlanes,
     SelectedPlane = -1;
   double PlaneScale = 1.0;
   double UpdateSpeed = 1.0;
   double MaxDistance;
   GlgObject 
     MapViewport,
     GISObject,
     PlaneTemplate,
     MissileTemplate,
     InfoDialog;
   GlgPoint
     Center = new GlgPoint( 0.0, 0.0, 0.0 ),
     buf_position1 = new GlgPoint( 0.0, 0.0, 0.0 ),
     buf_position2 = new GlgPoint( 0.0, 0.0, 0.0 );
   Plane buf_plane = new Plane();

   // Array to keep the state of the simulation (both planes and missiles)
   Plane [] Planes;

   Timer timer = null;

   // Functions to convert between degrees and radians. 
   double RAD( double angle ) { return angle / 180.0 * Math.PI; }
   double DEG( double angle ) { return angle * 180.0 / Math.PI; }

   boolean IS_MISSILE( int plane ) { return ( plane % 2 ) != 0; }
   boolean IS_PLANE( int plane ) { return ! IS_MISSILE( plane ); }

   //////////////////////////////////////////////////////////////////////////
   // Support class, keeps data for one airplane or missile.
   //////////////////////////////////////////////////////////////////////////
   class Plane
   {
      GlgObject glg_object;
      double speed;
      int change_state;
      int correcting_distance;
      boolean destroyed;
      int missile_delay;
      GlgPoint position;
      GlgPoint direction;
      GlgPoint angle;
      GlgPoint angle_change;
      
      ////////////////////////////////////////////////////////////////////////
      Plane()
      {
         glg_object = null;
         speed = 0;
         change_state = 0;
         correcting_distance = 0;
         destroyed = true;  // No graphics created for the plane yet
         missile_delay = 0; // Prevents from changing for
           // some time after a turn to let the plane correct the distance
             
             position = new GlgPoint( 0.0, 0.0, 0.0 );
         direction = new GlgPoint( 0.0, 0.0, 0.0 );
         angle = new GlgPoint( 0.0, 0.0, 0.0 );
         angle_change = new GlgPoint( 0.0, 0.0, 0.0 );
      }
      
      ///////////////////////////////////////////////////////////////////////
      // A partial copy only
      ///////////////////////////////////////////////////////////////////////
      void CopyPlaneInfoFrom( Plane from )
      {
         this.position.x = from.position.x;
         this.position.y = from.position.y;
         this.position.z = from.position.z;
         
         this.angle.x = from.angle.x;
         this.angle.y = from.angle.y;
         this.angle.z = from.angle.z;
         
         this.angle_change.x = from.angle_change.x;
         this.angle_change.y = from.angle_change.y;
         this.angle_change.z = from.angle_change.z;
         
         this.change_state = from.change_state;
         this.correcting_distance = from.correcting_distance;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   public GlgAircombatDemo()
   {
      super();

      NumPlanes = NumPlanePairs * 2;
      Planes = new Plane[ NumPlanes ];

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

      JFrame frame = new JFrame( "GLG Air Combat Simulation Demo" );

      GlgAircombatDemo.StandAlone = true;
      GlgAircombatDemo aircombat = new GlgAircombatDemo();
      
      frame.setResizable( true );
      frame.setSize( 800, 700 );
      frame.setLocation( 20, 20 );

      frame.getContentPane().add( aircombat );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );

      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers ReadyCallback which starts updates.
      //
      aircombat.SetDrawingName( "aircombat.g" );
   }

   //////////////////////////////////////////////////////////////////////////
   public void StartUpdates()
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
   public void StopUpdates()
   {
      if( timer != null )
      {
         timer.stop();
         timer = null;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked after loading the drawing, but before it is set up and drawn.
   //////////////////////////////////////////////////////////////////////////
   public void HCallback( GlgObject viewport )
   {
      // Load plane icons and delete the icon palette from the drawing
      // before it becomes visible.
      LoadPlaneIcons();

      MapViewport = viewport.GetResourceObject( "Map" );
      InfoDialog = MapViewport.GetResourceObject( "Info" );
      GISObject = MapViewport.GetResourceObject( "GISObject" );
      GetMapExtent();

      InfoDialog.SetDResource( "Visibility", 0.0 );
      SetDResource( "UpdateSpeed/ValueX", UpdateSpeed );

      if( !StandAlone )
      {
         String param = getParameter( "MapServerURL" );
         if( param != null )
           SuppliedMapServerURL = param;
      }

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
      MapViewport.SetDResource( "JavaSetupInfo/Visibility", 1.0 );

      ////////////////////////////////////////////////////////////////////////

      // Try to use the GLG map server on the localhost if no URL was supplied.
      if( SuppliedMapServerURL == null )
        SuppliedMapServerURL = "http://localhost/cgi-bin/GlmScript";
      
      if( SuppliedMapServerURL != null )
        // Override the URL defined in the drawing.
        GISObject.SetSResource( "GISMapServerURL", SuppliedMapServerURL );
   }

   //////////////////////////////////////////////////////////////////////////
   // Initializes the simulation and starts updates
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );

      // Populate the drawing with planes: copy the plane template
      CreatePlanes( NumPlanes );

      Update();

      StartUpdates();
   }

   //////////////////////////////////////////////////////////////////////////
   void GetMapExtent()
   {
      Center = GISObject.GetGResource( "GISCenter" );
      GlgPoint extent = GISObject.GetGResource( "GISExtent" );
      MaxDistance = Math.min( extent.x / 2.0, extent.y / 2.0 ) * MAX_DISTANCE;
   }

   //////////////////////////////////////////////////////////////////////////
   // Loads plane icons from the plane palette defined in the drawing and
   // select the icons to use depending on the number of planes.
   //////////////////////////////////////////////////////////////////////////
   void LoadPlaneIcons()
   {
      GlgObject 
        Plane3DFull, Plane3DLight, Plane2D,
        Missile3D, Missile2D;

      /* Find a plane palette. */
      GlgObject plane_palette = GetResourceObject( null, "PlanePalette" );      

      // Find a plane's template in the drawing
      Plane3DFull  = plane_palette.GetResourceObject( "plane_3D_full" );
      Plane3DLight = plane_palette.GetResourceObject( "plane_3D_light" );
      Plane2D      = plane_palette.GetResourceObject( "plane_3D_light" );
      Missile3D    = plane_palette.GetResourceObject( "missile_3D_full" );
      Missile2D    = plane_palette.GetResourceObject( "missile_2D" );

      // Delete the palette from the drawing 
      DeleteObject( null, plane_palette );

      if( NumPlanes <= 30 )
      {
         PlaneTemplate = Plane3DFull;
         MissileTemplate = Missile3D; 
      }
      else if( NumPlanes <= 80 )
      {
         PlaneTemplate = Plane3DLight;
         MissileTemplate = Missile3D; 
      }
      else
      {
         PlaneTemplate = Plane2D;
         MissileTemplate = Missile2D; 
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Creates planes by copying the template and adds the planes to the
   // drawing. Annotates missiles with a different size and color.
   //////////////////////////////////////////////////////////////////////////
   void CreatePlanes( int num_planes )
   {
      for( int i=0; i<num_planes; ++i )
      {
         // Create a copy of a plane template and add it to the drawing 
         Planes[ i ] = new Plane();

         String name;
         if( IS_MISSILE( i ) )
         {         
            Planes[ i ].glg_object = CopyObject( MissileTemplate );
            name = "Missile " + Integer.toString( i );
         }
         else
         {
            Planes[ i ].glg_object = CopyObject( PlaneTemplate );
            name = "Plane " + Integer.toString( i );
         }

         // Store name.
         SetSResource( Planes[ i ].glg_object, "Name", name );
                       
         // Store a plane index used to identify the plane when it is selected 
         // with the mouse. It is stored in a custom property attached to the 
         // plane icon.
         SetDResource( Planes[ i ].glg_object, "PlaneIndex", (double) i );

         // Add planes and missiles to GISArray of GISObject to be able to 
         // position them using lat/lon coordinates.
         //
         AddObjectToBottom( GISObject, Planes[ i ].glg_object );

         // Set initial simulation parameters 
         Planes[ i ].destroyed = false;
         Planes[ i ].correcting_distance = 0;
         Planes[ i ].missile_delay = 0;

         if( IS_PLANE( i ) )
         {
            // It's a plane: set a random initial position
            Planes[ i ].position.x = 
              Center.x + MaxDistance * GetData( -1.0, 1.0 );
            Planes[ i ].position.y = 
              Center.y + MaxDistance * GetData( -1.0, 1.0 );
            Planes[ i ].position.z = 
              Center.z + MaxDistance * GetData( -1.0, 1.0 );
         }
         else
         {
            // It's a missile: set in some proximity of a plane.
            Planes[ i ].position.x = 
              Planes[ i - 1 ].position.x + MaxDistance * GetData( -0.3, 0.3 );
            Planes[ i ].position.y =
              Planes[ i - 1 ].position.y + MaxDistance * GetData( -0.3, 0.3 );
            // Set at the same depth
            Planes[ i ].position.z = Planes[ i - 1 ].position.z;
         }

         if( IS_MISSILE( i ) )   // Missile
           // Set missiles initial speed to be lower 
           Planes[ i ].speed = MISSILE_SPEED * PLANE_SPEED;         
         else   // Plane 
           Planes[ i ].speed = PLANE_SPEED;

         Planes[ i ].change_state = 1;
         SetPlane( i );     // Set initial plane parameters. 
         Planes[ i ].change_state = 0;
      }

      AdjustPlanesScale( PlaneScale ); // Set initial plane scale 
   }

   //////////////////////////////////////////////////////////////////////////
   // Simulates a plane or a missile.
   //////////////////////////////////////////////////////////////////////////
   void AutoPilot( int plane )
   {
      if( Planes[ plane ].destroyed )
        return;

      if( IS_PLANE( plane ) )
      {
         if( Distance( Planes[ plane ].position, Center ) > MaxDistance )
         {
            // Change direction to correct the distance from a center 
            if( Planes[ plane ].correcting_distance == 0 )
              ChangeDirection( plane, true );
         }
         else if( GetData( 0.0, 1.0 ) > CHANGE_THRESHOLD )
           ChangeDirection( plane, false );  // Random direction change 
      }
      else   // Missile 
      {
         ++Planes[ plane ].missile_delay;
         if( ( Planes[ plane ].missile_delay % MISSILE_DELAY ) == 0 )
           // Adjust missile to the new plane position 
           ChangeDirection( plane, false );
      }

      // Move the plane by one simulation step. 
      MakeStep( plane, 1, null );
   
      if( !Planes[ plane ].destroyed )
        SetPlane( plane );  // Set the plane in the drawing 
   }

   //////////////////////////////////////////////////////////////////////////
   // For a missile: adjust the missile's direction to point to the plane it
   //   follows.
   // For a plane:
   //   a) if required==True, change the plane direction to correct the
   //      distance from the center to avoid going off the screen.
   //   b) if required==False, randomly change the plane's direction to make
   //      it more complicated for a missile to catch it.
   //////////////////////////////////////////////////////////////////////////
   void ChangeDirection( int plane, boolean required )
   {
      double dx, dy, dz, angle_y, angle_z;

      if( IS_PLANE( plane ) )
      {
         // Start a new change and set flags to indicate a change in progress 
         Planes[ plane ].change_state = TRANSITION_NUM - 1; 
         if( required )
           Planes[ plane ].correcting_distance = DELAY * TRANSITION_NUM + 1;
      
         while( true )
         {
            // Choose a new random direction 
            Planes[ plane ].angle_change.x = 
              60.0 * GetData( -1.0, 1.0 ) / TRANSITION_NUM;
            Planes[ plane ].angle_change.y =
              60.0 * GetData( -1.0, 1.0 ) / TRANSITION_NUM;
            Planes[ plane ].angle_change.z = 
              360.0 * GetData( 0.0, 1.0 ) / TRANSITION_NUM;

            // Test if it lowers the distance from the center, if required. 
            if( !required ||
               DecreaseDistance( plane, 0, DELAY * TRANSITION_NUM + 1 ) )
              break;
         }
      }
      else  // Missile 
      {
         // Calculate the angles to direct the missile at the plane. 
         dx = Planes[ plane - 1 ].position.x - Planes[ plane ].position.x;
         dy = Planes[ plane - 1 ].position.y - Planes[ plane ].position.y;
         dz = Planes[ plane - 1 ].position.z - Planes[ plane ].position.z;

         if( dx == 0.0 )
           angle_z = ( dy > 0.0 ? 90.0 : 270.0 );
         else
         {
            angle_z = Math.atan( dy / dx );
            angle_z = DEG( angle_z );
            if( dx < 0.0 )
              angle_z += 180.0;
         }
         
         if( dx == 0.0 )
           angle_y = ( dz > 0.0 ? 270.0 : 90.0 );
         else
         {
            angle_y = Math.atan( dz / dx );
            angle_y = DEG( angle_y );
         }

         // Change missile direction instantly (it has a much smaller inertia). 
           
         // Take x angle from a plane. 
         Planes[ plane ].angle.x = Planes[ plane - 1 ].angle.x; 
         Planes[ plane ].angle.y = angle_y;
         Planes[ plane ].angle.z = angle_z;
         Planes[ plane ].change_state = 1;

         // Randomly speed the missile up until it is faster then the plane. 
         if( Planes[ plane ].speed < 1.2 * PLANE_SPEED && 
             GetData( 0.0, 1.0 ) > 0.5 )
           Planes[ plane ].speed *= 1.04;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Calculate and return a distance between two positions.
   //////////////////////////////////////////////////////////////////////////
   double Distance( GlgPoint position1, GlgPoint position2 )
   {
      double dx, dy, dz;

      dx = position1.x - position2.x;
      dy = position1.y - position2.y;
      dz = position1.z - position2.z;
      
      return Math.sqrt( dx * dx + dy * dy + dz * dz );
   }

   //////////////////////////////////////////////////////////////////////////
   /// Tests whether or not the distance will decrease after the required
   /// number of simulation steps. Compares the distance after steps1 number
   /// of steps with the distance after steps2 number of steps.
   ///////////////////////////////////////////////////////////////////////////
   boolean DecreaseDistance( int plane, int steps1, int steps2 )
   {
      MakeStep( plane, steps1, buf_position1 );
      MakeStep( plane, steps2, buf_position2 );
      return ( Distance( buf_position2, Center ) <
              Distance( buf_position1, Center ) );
   }

   //////////////////////////////////////////////////////////////////////////
   // Makes the requested number of simulation steps and returns the
   // calculated postion of a plane after that.
   // If the position_value parameter is not NULL, it is a trial step; the
   // calculated values are returned in this structure and the plane
   // parametrs are restored afterwards.
   // If the parameter is NULL, it is a real step affecting the plane.
   //////////////////////////////////////////////////////////////////////////
   void MakeStep( int plane, int num_steps, GlgPoint position )
   {
      int i;

      if( position != null )
        buf_plane.CopyPlaneInfoFrom( Planes[ plane ] );     // Save

      for( i=0; i<num_steps; ++i )
      {
         // Make a step in the current direction 
         Planes[ plane ].position.x += Planes[ plane ].speed * UpdateSpeed *
           Math.cos( RAD( Planes[ plane ].angle.z ) ) *
             Math.cos( RAD( Planes[ plane ].angle.y ) );
         Planes[ plane ].position.y += Planes[ plane ].speed * UpdateSpeed *
           Math.sin( RAD( Planes[ plane ].angle.z ) ) *
             Math.cos( RAD( Planes[ plane ].angle.y ) );
         Planes[ plane ].position.z += Planes[ plane ].speed * UpdateSpeed *
           Math.sin( RAD( Planes[ plane ].angle.y ) ) *
             Math.cos( RAD( Planes[ plane ].angle.z ) );

         // If a direction change is in progress, adjust the plane angles. 
         if( Planes[ plane ].change_state != 0 && IS_PLANE( plane ) )
         {
            Planes[ plane ].angle.x += Planes[ plane ].angle_change.x;
            Planes[ plane ].angle.y += Planes[ plane ].angle_change.y;
            Planes[ plane ].angle.z += Planes[ plane ].angle_change.z;
            
            if( Planes[ plane ].angle.x >= 360.0 )
              Planes[ plane ].angle.x -= 360.0;
            if( Planes[ plane ].angle.y >= 360.0 )
              Planes[ plane ].angle.y -= 360.0;
            if( Planes[ plane ].angle.z >= 360.0 )
              Planes[ plane ].angle.z -= 360.0;
            
            --Planes[ plane ].change_state;
         }
         
         if( Planes[ plane ].correcting_distance != 0 )
           --Planes[ plane ].correcting_distance;
      }

      if( position != null ) // Just checking the distance: restore and return.
      {
         // Return the result 
           position.x = Planes[ plane ].position.x;
         position.y = Planes[ plane ].position.y;
         position.z = Planes[ plane ].position.z;
         
         Planes[ plane ].CopyPlaneInfoFrom( buf_plane );     // Restore
      }
      else  // Real step: if it is a missile, check if it got its plane. 
        if( IS_MISSILE( plane ) &&
            Distance( Planes[ plane - 1 ].position, Planes[ plane ].position ) <
            DESTROY_DISTANCE * MaxDistance )
        {
           // Got it: Delete both the plane and the missile. 
           DeletePlane( plane - 1, false );
           DeletePlane( plane, false );
           
           // Restart if no more planes left. 
           if( NoPlanesLeft() )
           {
              CreatePlanes( NumPlanes );
              Update();
           }
        }
   }

   //////////////////////////////////////////////////////////////////////////
   // Deletes the plane or missile from the drawing. Flashes the size of a
   // missile for an explosive effect.
   //////////////////////////////////////////////////////////////////////////
   void DeletePlane( int plane, boolean deleting_all )
   {
      if( plane == SelectedPlane )
        UnselectPlane();

      if( IS_MISSILE( plane ) )         /* Create an explosive effect. */
      {
         // Speed it up for large number of planes when deleting all - 
         // do it only a few times.
         //
         if( !deleting_all ||
             NumPlanes > 2 && ( ( plane - 1 ) % ( NumPlanes / 3 ) ) == 0 )
         {
            for( int i=0; i<6; ++i )
            {
               GlgObject plane_graphics = Planes[ plane ].glg_object;
               
               double scale = GetDResource( plane_graphics, "Scale" );
               if( ( (i+1) % 2 ) != 0 )
                 SetDResource( plane_graphics, "Scale", scale );
               else
                 SetDResource( plane_graphics, "Scale", scale * 1.3 );
               MapViewport.UpdateImmediately();
            }
         }
      }

      // Find and delete the plane from the drawing 
      if( !DeleteObject( GISObject, Planes[ plane ].glg_object ) )
      {
         PrintToJavaConsole( "Cannot find the plane.\n" );
      }
      Planes[ plane ].destroyed = true;
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates the plane in the drawing with the new values. Ignores 3D
   // rotation for a missle.
   //////////////////////////////////////////////////////////////////////////
   void SetPlane( int plane )
   {
      // Plane is changing its direction. 
      if( Planes[ plane ].change_state != 0 )
      {
         SetDResource( Planes[ plane ].glg_object, "AngleX",
                       Planes[ plane ].angle.x );
         SetDResource( Planes[ plane ].glg_object, "AngleY",
                       Planes[ plane ].angle.y );
         SetDResource( Planes[ plane ].glg_object, "AngleZ",
                       Planes[ plane ].angle.z );

         if( IS_MISSILE( plane ) )     // Missile: an instant change 
           Planes[ plane ].change_state = 0;
      }
      
      SetGResource( Planes[ plane ].glg_object, "Position",
                    Planes[ plane ].position.x,
                    Planes[ plane ].position.y,
                    Planes[ plane ].position.z );
   }

   //////////////////////////////////////////////////////////////////////////
   // Makes one simulation step for every plane or missile.
   //////////////////////////////////////////////////////////////////////////
   void UpdateBattleField()
   {
      if( timer == null )
        return;   // Prevents race conditions

      for( int i=0; i<NumPlanes; ++i )
        AutoPilot( i );
      
      DisplaySelectedPlaneInfo();
      
      // Update the drawing after changes. 
      Update();

      timer.start();   // Restart the update timer
   }

   //////////////////////////////////////////////////////////////////////////
   // Returns a random number in the requested range.
   //////////////////////////////////////////////////////////////////////////
   double GetData( double min, double max )
   {
      return GlgObject.Rand( min, max );
   }

   //////////////////////////////////////////////////////////////////////////
   void DisplaySelectedPlaneInfo()
   {
      if( SelectedPlane == -1 )
        return;

      double roll = Planes[ SelectedPlane ].angle.x;
      double pitch = Planes[ SelectedPlane ].angle.y;
      double yaw = Planes[ SelectedPlane ].angle.z - 90.0;

      while( roll > 180.0 )
        roll -= 360.0;
      while( pitch > 180.0 )
        pitch -= 360.0;
      while( yaw > 180.0 )
        yaw -= 360.0;
      
      InfoDialog.SetDResource( "Roll",  roll );
      InfoDialog.SetDResource( "Pitch", pitch );
      InfoDialog.SetDResource( "Yaw",   yaw );

      String lat_string = 
        CreateLocationString( Planes[ SelectedPlane ].position.y, true );
      String lon_string = 
        CreateLocationString( Planes[ SelectedPlane ].position.x, false );

      InfoDialog.SetSResource( "Lat", lat_string);
      InfoDialog.SetSResource( "Lon", lon_string );
   }

   //////////////////////////////////////////////////////////////////////////
   void SelectPlane( int plane_index )
   {
      if( SelectedPlane == -1 )
        // No previously selected plane: activate an information popup dialog.
        InfoDialog.SetDResource( "Visibility", 1.0 );
      else 
        // The dialog is already active, just unselect the previously selected 
        // plane.
        if( !Planes[ SelectedPlane ].destroyed )
          SetDResource( Planes[ SelectedPlane ].glg_object,
                        "SelectedState", 0.0 );

      SelectedPlane = plane_index;

      // Display selected plane or missile info stored as its name.
      String plane_name = 
        Planes[ SelectedPlane ].glg_object.GetSResource( "Name" );
      InfoDialog.SetSResource( "PlaneName", plane_name );

      // Set the SelectedState resource which controls an icon's color. 
      SetDResource( Planes[ plane_index].glg_object, "SelectedState", 1.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   void UnselectPlane()
   {
      if( SelectedPlane == -1 )
        return;

      if( !Planes[ SelectedPlane ].destroyed )
        SetDResource( Planes[ SelectedPlane ].glg_object, 
                      "SelectedState", 0.0 );

      InfoDialog.SetDResource( "Visibility", 0.0 );
      SelectedPlane = -1;
   }

   //////////////////////////////////////////////////////////////////////////
   // Processes buttons' and sliders' events.
   //////////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject vp, GlgObject message_obj )
   {
      String
        format,
        action,
        origin;

      super.InputCallback( vp, message_obj );

      format = message_obj.GetSResource( "Format" );
      origin = message_obj.GetSResource( "Origin" );
      action = message_obj.GetSResource( "Action" );

      if( format.equals( "Slider" ) )
      {
         if( !action.equals( "ValueChanged" ) )
           return;
         
         if( origin.equals( "UpdateSpeed" ) )
           // Query the new value of the UpdateSpeed Slider (relative value).
           UpdateSpeed = message_obj.GetDResource( "ValueX" ).doubleValue(); 
      }
      else if( format.equals( "Button" ) )
      {
         if( !action.equals( "Activate" ) )
           return;

         if( origin.equals( "Restart" ) )
         {
            for( int i=0; i<NumPlanes; ++i )
              if( !Planes[ i ].destroyed )
                DeletePlane( i, true );
            
            CreatePlanes( NumPlanes );
            Update();
         }
         else if( origin.equals( "BigPlanes" ) )  // Increase plane size
         {
            PlaneScale *= 1.5;
            AdjustPlanesScale( 1.5 );
         }
         else if( origin.equals( "SmallPlanes" ) )  // Decrease plane size
         {
            PlaneScale /= 1.5;
            AdjustPlanesScale( 1.0 / 1.5 );
         }
         else if( origin.equals( "ClosePlaneInfo" ) )
         {
            // Unselect the plane and close its information popup dialog.
            UnselectPlane();
            Update();
         }
         else if( origin.equals( "Quit" ) )
           System.exit( 0 );
      }
      /* Process mouse clicks on plane icons, implemented as an Action with
         the PlaneSelection label attached to an icon and activated on a 
         mouse click. 
      */
      else if( format.equals( "CustomEvent" ) )
      {
         String event_label = message_obj.GetSResource( "EventLabel" );

         /* Plane icon was selected */
         if( event_label.equals( "PlaneSelection" ) )
         {
            /* Get plane index */
            int plane_index = 
              message_obj.GetDResource( "Object/PlaneIndex" ).intValue();
            SelectPlane( plane_index );
            Update(); 
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   boolean NoPlanesLeft()
   {
      int i;

      for( i=0; i<NumPlanes; ++i )
        if( !Planes[ i ].destroyed )
          return false;

      return true;
   }

   //////////////////////////////////////////////////////////////////////////
   void AdjustPlanesScale( double factor )
   {
      int i;
      double scale;
      final double MAX_PLANE_SCALE = 3.375;

      for( i=0; i<NumPlanes; ++i )
        if( !Planes[ i ].destroyed )
        {
           scale = GetDResource( Planes[ i ].glg_object, "Scale" );

           scale *= factor;        
           if( scale > MAX_PLANE_SCALE )
             scale = MAX_PLANE_SCALE;

           SetDResource( Planes[ i ].glg_object, "Scale", scale );
        }
      
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Generate a location info string by converting +- sign info into the
   // N/S, E/W suffixes, and decimal fraction to deg, min, sec.
   //////////////////////////////////////////////////////////////////////////
   String CreateLocationString( double value, boolean return_lat )
   {
      int deg, min, sec;
      char suffix;

      if( return_lat )
      {
         if( value < 0.0 )
         {
            value = -value;
            suffix = 'S';
         }
         else
           suffix = 'N';
         
         deg = (int) value;
         min = (int) ( ( value - deg ) * 60.0 );
         sec = (int) ( ( value - deg - min / 60.0 ) * 3600.0 );
         
         return "" + deg + "\u00B0" + 
           GlgObject.Printf( "%02d", min ) + "\'" + 
           GlgObject.Printf( "%02d", sec ) + "\"" + suffix;
      }
      else   // return lon
      {
         if( value < 0.0 )
         {
            value = -value;
            suffix = 'W';
         }
         else if( value >= 360.0 )
         {
            value -= 360.0;
            suffix = 'E';
         }
         else if( value >= 180.0 )
         {
            value = 180.0 - ( value - 180.0 );
            suffix = 'W';
         }
         else
           suffix = 'E';
         
         deg = (int) value;
         min = (int) ( ( value - deg ) * 60.0 );
         sec = (int) ( ( value - deg - min / 60.0 ) * 3600.0 );
         return "" + deg + "\u00B0" + 
           GlgObject.Printf( "%02d", min ) + "\'" + 
           GlgObject.Printf( "%02d", sec ) + "\"" + suffix;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // ActionListener method to use the bean as update timer's ActionListener.
   //////////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateBattleField();
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked by the browser to stop the applet
   //////////////////////////////////////////////////////////////////////////
   public void stop()
   {
      StopUpdates();
      super.stop();
   }
}

