/****************************************************************************
| This example demonstrates how to integrate mapping functionality with
| the dynamic features of the GLG Toolkit.
|
| The program displays a map of the US using a GLG GIS object.  
| A dynamic icon (aircraft) is displayed on top of the map, whose position 
| is updated periodically using a defined trajectory. The aircraft's
| position is calculated in lat/lon coordinates in the GetIconPosition() 
| function, which can be replaced with the user-defined data acquisitions 
| mechanism in a real application. 
|
| A list of targets (airports, for example) with predefined lat,lon
| coordinates is polulated on the map at application start-up. 
| The target icons are displayed as a group of GLG objects that may 
| be turned on/off using the ToggleTargetsLayer toolbar button. 
|
| A target may be selected with the mouse, making it currently selected
| target. Once a target is selected, a DistancePopup overlay is displayed,
| printing the name of the selected target and distance in km between the
| target and moving aircraft. GetGlobeDistance() function in this program
| is used to calculate distance in meters between two points on the globe,
| defined in lat,lon coordinates.
| 
| The program also demonstrates how to turn ON/OFF map layers dynamically
| at run-time, from a menu containing a list of available layers. 
| LayersDialog may be opened/closed  using the ToggleMapLayers 
| toolbar button. 
|
| The program also supports zooming and panning of the map using toolbar 
| buttons. 
|
****************************************************************************/

import java.awt.*;
import java.awt.event.*;
import java.lang.reflect.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgGISExample2 extends GlgJBean implements ActionListener
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////

   static final long serialVersionUID = 0;

   double EQUATOR_RADIUS = 6378136.;

   // Time interval for periodic updates, in millisec.
   double UPDATE_INTERVAL = 200.;

   GlgObject
      MapVp, // Map viewport containing a GIS object
      GISObject, // GIS object
      LayersMenu, // Layers menu object has toggles for turning map layers 
                      // on/off. 
      TargetTemplate, // Template for a target object
      TargetGroup, // Group of target objects
      SelectedTarget = null; // Currently selected target object

   // Create a n instance of a class to store icon information 
   IconData Icon = new IconData();

   // Array of icons/targets to place on the map as GLG objects. 
   // In this example, these objects are populated at the specified 
   // lat/lon positions, but the code can be added to position them
   // at the lat/lon location defined by the user at run-time.
   // Since these targets are GLG objects, they may be selected with 
   // the mouse and their attributes may be changed dynamically. 
   // For example, when a prticular target is selected, its color may 
   // be changed to indicate the current selection. These objects may be 
   // also moved/dragged using the mouse, to change their location. 
   TargetData TargetArray[] =
   {
      new TargetData( "Boston", -71.01789, 42.33602 ),
      new TargetData( "New York", -73.97213, 40.77436 ),
      new TargetData( "San Francisco", -122.55478, 37.79325 ),
      new TargetData( "Miami", -80.21084, 25.77566 ),
      new TargetData( "Seattle", -122.35032, 47.62180 ),
      new TargetData( "Houston", -95.38672, 29.76870 ),
      new TargetData( "Denver", -104.87265, 39.76803 ),
      new TargetData( "Minneapolis", -93.26684, 44.96185 ),
      new TargetData( "Chicago", -87.68496, 41.83705 ),
      new TargetData( "Dallas", -96.76524, 32.79415 )
   };

   int NumTargets;

   // Store initial extent and center, used to reset the drawing 
   GlgPoint
      InitExtent = new GlgPoint(),
      InitCenter = new GlgPoint();

   boolean PanMode = false;
   boolean IsReady = false;

   // Variable used to calculate the icon's angle in GetIconPosition().
   double PathAngle = 0.;

   // Angle increment, used in GetIconPosition().
   double AngleIncrement = 0.2;

   // Temp vars: allocate once
   GlgPoint
      lat_lon = new GlgPoint(),
      point = new GlgPoint();

   Timer timer = null;

   //////////////////////////////////////////////////////////////////////////
   public GlgGISExample2()
   {
      super();
      // Turn on Java diagnostics
      // SetJavaLog( true );

      // Activate Trace callback.
      AddListener( GlgObject.TRACE_CB, this );

   }

   /////////////////////////////////////////////////////////////////////
   // Invoked by the browser to start the applet.
   /////////////////////////////////////////////////////////////////////
   public void start()
   {
      super.start();

      // Set a Glg drawing URL to be displayed in the Glg bean.
      // The drawing URL is relative to applet's document base directory.
      SetDrawingURL( "gis_example2.g" );

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
      // Obtain an object ID of the viewport named "MapVp" and 
      // GIS object named "GISObject".
      MapVp = viewport.GetResourceObject( "MapVp" );
      GISObject = MapVp.GetResourceObject( "GISObject" );

      // Obtain object ID of the viewport LayersMenu, from the LayersDialog
      // object.
      LayersMenu = viewport.GetResourceObject( "LayersDialog/LayersMenu" );

      // Set GIS Zoom mode: generate a new map request for a 
      // new area on zoom/pan.
      MapVp.SetZoomMode( null, GISObject, null, GlgObject.GIS_ZOOM_MODE );

      // Initialize the drawing 
      Init();

      // Create a group of target objects and add it to the drawing.
      // The targets may be turned ON/OFF using the ShowTargets tollbar button.
      CreateTargetsArray();

      // Set initial layer string for the GISObject based on 
      // layer menu's InitLayerStates array. It has to be done before 
      // the hierarchy setup - before the map is generated using it. 
      SetLayersFromMenu( LayersMenu, true );

   }

   //////////////////////////////////////////////////////////////////////
   // VCallback() is invoked after the drawing is loaded and setup,
   // but before it is drawn for the first time.
   // Set intial icon's position here. 
   /////////////////////////////////////////////////////////////////////
   public void VCallback( GlgObject viewport )
   {
      // Initialize layer menu's initial button states from menu's 
      // InitLayerStates array.
      SetLayersMenuButtons( LayersMenu );
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
      // Query and store initial map extent from the GIS object. named 
      // It is used to reset the drawing to the initial extent 
      // when the user clicks on the ZoomReset button.
      InitExtent = GISObject.GetGResource( "GISExtent" );
      InitCenter = GISObject.GetGResource( "GISCenter" );

      // Obtain an object ID of the Icon object.
      Icon.icon_obj = GISObject.GetResourceObject( "GISArray/Icon" );

      /* Get initial position of the icon. Uses the center of the map
         by default. */
      GetIconPosition( Icon );

      /* Position an icon */
      PositionIcon( Icon );

      // Make the DistancePopup object invisible. 
      MapVp.SetDResource( "DistancePopup/Visibility", 0.0 );

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

      // Comment out the next two lines line to enable the map.
      GISObject.SetDResource( "GISDisabled", 1.0 );
      MapVp.SetDResource( "JavaSetupInfo/Visibility", 1.0 );

      ////////////////////////////////////////////////////////////////////////

      // Try to use the GLG map server installed on the local host by default.
      String SuppliedMapServerURL = "http://localhost/cgi-bin/GlmScript";

      if( SuppliedMapServerURL != null )
        // Override the URL defined in the drawing with the supplied URL.
        GISObject.SetSResource( "GISMapServerURL", SuppliedMapServerURL );
   }

   ///////////////////////////////////////////////////////////////////////
   // Creates TargetGroup to hold target icons and add it to the map viewport.
   // The group's Visibility may be toggled using ShowTargets tollbar button.
   ///////////////////////////////////////////////////////////////////////
   public void CreateTargetsArray()
   {
      NumTargets = Array.getLength( TargetArray );
      TargetTemplate = GISObject.GetResourceObject( "GISArray/Target" );
      GISObject.DeleteObject( TargetTemplate );

      TargetGroup = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
      TargetGroup.SetSResource( "Name", "TargetGroup" );

      /* Add target icons */
      for( int i = 0; i < NumTargets; ++i )
         AddTarget( TargetArray[ i ], i );

      GISObject.AddObjectToBottom( TargetGroup );
   }

   ///////////////////////////////////////////////////////////////////////
   // Adds a target icon, set tooltips, etc.
   ///////////////////////////////////////////////////////////////////////
   public void AddTarget( TargetData target_data, int index )
   {
      GlgObject target;

      // Create a copy of a target template. 
      target = TargetTemplate.CloneObject( GlgObject.STRONG_CLONE );

      // Store the target's Object ID.
      target_data.graphics = target;

      // Set target's name.
      target.SetSResource( "Name", target_data.name );

      // Store index for each target object for direct access.
      target.SetDResource( "DataIndex", (double)index );

      // Define the tooltip, including the lat/lon information and a target
      // name.
      String tooltip;
      tooltip = target_data.name +
          " lat= " + target_data.lat_lon.y + " lon= " + target_data.lat_lon.x;
      target.SetSResource( "TooltipString", tooltip );

      // Set target's position in the drawing.
      target.SetGResource( "Position", target_data.lat_lon );

      // Add new target object to the TargetGroup
      TargetGroup.AddObjectToBottom( target );
   }

   ///////////////////////////////////////////////////////////////////////
   // Position an icon on the map.
   ///////////////////////////////////////////////////////////////////////
   public void PositionIcon( IconData icon )
   {
 
      // Update icon position in the drawing. Set the angle for the icon.
      icon.icon_obj.SetGResource( "Position", icon.lat_lon );
      icon.icon_obj.SetDResource( "Angle", icon.angle );

      // Define icon tooltip, including the lat/lon information.
      String tooltip;
      tooltip = "Flight 1237" +
          " lat= " + icon.lat_lon.y + " lon= " + icon.lat_lon.x;
      icon.icon_obj.SetSResource( "TooltipString", tooltip );
   }

   ///////////////////////////////////////////////////////////////////////
   // Creates a comma separated list of layer strings, based on the
   // menu selection and assigns a layer string to the GIS object.
   // If init parameter is true, uses menu's InitLayerStates array instead
   // of button states to build the layer string.
   ///////////////////////////////////////////////////////////////////////
   public void SetLayersFromMenu( GlgObject menu_obj, boolean init )
   {
      GlgObject
         layer_state_list,
         layer_state_obj ;

      int layer_on;

      String layers = "";

      // Query InitStateList object from the menu.  The number of elements
      // in this list should correspond to the number of buttons in the
      // menu. Each element has Name attribute corresponding to the
      // layer name defined in the .sdf file, and its Value attribute
      // defines the layer's state, i.e. whether this layer should be ON/OFF
      // on initial appearance.
      layer_state_list = menu_obj.GetResourceObject( "InitStateList" );

      // Query the number of layers. Number of layers is defined by the 
      // NumRows attribute of the menu.
      int num_layers = menu_obj.GetDResource( "NumRows" ).intValue();

      // Traverse the menu, determine which layer should be activated
      // and build a layer string for the GISObject.
      for( int i = 0; i < num_layers; ++i )
      {
         // Retrieve the element with index i from InitStateList.
         layer_state_obj = (GlgObject) layer_state_list.GetElement( i );

         if( init )
         {
            // Get layer_on state from the menu's InitStateList array.
            layer_on = layer_state_obj.GetDResource( null ).intValue();
         }
         else
         {
            // Get layer_on state from the corresponding menu button.
            String res_name = 
              GlgObject.CreateIndexedName( "Button%/OnState", i );
            layer_on = menu_obj.GetDResource( res_name ).intValue();
         }

         // Layer is not included, continue traversal.
         if( layer_on == 0 )
            continue;

         // Layer with index i is active, add the layer name with index i
         // to the layer list. Layer name is stored in the Name attribute
         // of the corresponding element of the InitStateList. 
         String layer_name = layer_state_obj.GetSResource( "Name" );

         // If layers string is not empty (not the first layer), 
         // add a comma to separate individual layer names.
         if( !layers.equals( "" ) )
            layers = layers + ",";

         // Add layer_name to the layers string.
         layers = layers + layer_name;
      }

      // Assign the assembled list of active layers to the GISObject. 
      GISObject.SetSResource( "GISLayers", (layers != null) ? layers : "" );

   }

   ///////////////////////////////////////////////////////////////////////
   // Initialize layer menu's initial button states from menu's 
   // InitLayerStates array.
   ///////////////////////////////////////////////////////////////////////
   public void SetLayersMenuButtons( GlgObject menu_obj )
   {
      GlgObject
         layer_state_list,
         layer_state_obj;

      double layer_on;

      layer_state_list = menu_obj.GetResourceObject( "InitStateList" );

      int num_layers = menu_obj.GetDResource( "NumRows" ).intValue();
      for( int i = 0; i < (int)num_layers; ++i )
      {
         // Get layer_on state from the menu's InitStateList array.
         layer_state_obj = (GlgObject) layer_state_list.GetElement( i );
         layer_on = layer_state_obj.GetDResource( null ).doubleValue();
         
         // Set OnState resource of the corresponding menu button. 
         String res_name = GlgObject.CreateIndexedName( "Button%/OnState", i );
         menu_obj.SetDResource( res_name, layer_on );
      }
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
        action,
        full_origin;

      super.InputCallback( viewport, message_obj );

      origin = message_obj.GetSResource( "Origin" );
      format = message_obj.GetSResource( "Format" );
      action = message_obj.GetSResource( "Action" );
      full_origin = message_obj.GetSResource( "FullOrigin" );
    
      // Handle window closing if run stand-alone
      if( format.equals( "Window" ) && action.equals( "DeleteWindow" ) )
      {
         if( origin.equals( "LayersDialog" ) )
            // Close dialog
            SetDResource( "LayersDialog/Visibility", 0.0 );
         else
            // Exit application
            System.exit( 0 );
      }
      else if( format.equals( "Button" ) && action.equals( "Activate" ) )
      {
         PanMode = false;
         if( origin.equals( "ZoomIn" ) )
            Zoom( 'i', 2. );
         else if( origin.equals( "ZoomOut" ) )
            Zoom( 'o', 2. );
         else if( origin.equals( "ZoomReset" ) )
            Zoom( 'n', 0. );
         else if( origin.equals( "ZoomTo" ) )
         {
            PanMode = false; /* Abort Pan mode */
            MapVp.SetZoom( null, 't', 0. ); /* Start Zoom op */
         }
         else if( origin.equals( "Pan" ) )
         {
            MapVp.SetZoom( null, 'e', 0. ); /* Abort ZoomTo mode */
            PanMode = true;
         }
         else if( origin.equals( "ToggleLayers" ) ||
                  full_origin.equals( "LayersDialog/OKButton" ) )
         {
            // Popup/popdown a dialog to toggle map layers.
            ToggleResource( viewport, "LayersDialog/Visibility" );
         }
         else if( origin.equals( "ShowTargets" ) )
            ToggleResource( GISObject, "GISArray/TargetGroup/Visibility" );
      }
      else if( format.equals( "CustomEvent" ) &&
               action.equals( "MouseClick" ) ) // Mouse click on the icon
      {
         int zoom_mode, button_index;
         String event_label;

         zoom_mode = MapVp.GetDResource( "ZoomToMode" ).intValue();
         if( (zoom_mode != 0) || PanMode )
            return; // Don't handle selection in ZoomTo or Pan modes. 

         button_index = message_obj.GetDResource( "ButtonIndex" ).intValue();
         if( button_index != 1 )
            return; // Ignore middle and right mouse button clicks.

         event_label = message_obj.GetSResource( "EventLabel" );

         if( event_label.equals( "TargetSelected" ) )
         {
            // Process target selection event.
            if( SelectedTarget != null )
               ToggleResource( SelectedTarget, "ColorIndex" );

            // Retrieve an object ID of the selected object and highlight it
            // by chenging its color.
            GlgObject temp_obj = message_obj.GetResourceObject( "Object" );

            if( SelectedTarget == temp_obj )
               SelectedTarget = null;
            else
            {
               ToggleResource( temp_obj, "ColorIndex" );
               SelectedTarget = temp_obj;
            }
         }
         else if( event_label.equals( "PlaneSelected" ) )
         {
            // Process icon(plane) selection event here. 
         }
      }
      else if( format.equals( "Menu" ) )
      {
         // Process events from the LayersMenu object in the LayersDialog.
         if( origin.equals( "LayersMenu" ) )
         {
            // Activate map layers based on the menu settings.
            SetLayersFromMenu( LayersMenu, false );
         }

         // Do something with the selected object here.
      }

      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Toggle resource between 0 and 1.
   //////////////////////////////////////////////////////////////////////////
   public void ToggleResource( GlgObject object, String res_name )
   {
      GlgObject resource;
      double value;

      resource = object.GetResourceObject( res_name );
      if( resource == null )
         return;

      value = resource.GetDResource( null ).doubleValue();
      resource.SetDResource( null, value != 0. ? 0. : 1. );
   }

   //////////////////////////////////////////////////////////////////////////
   // Perform zoom operation.
   //////////////////////////////////////////////////////////////////////////
   public void Zoom( char type, double value )
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

      // Use the Map area events only.
      if( trace_info.viewport != MapVp )
         return;

      int event_type = trace_info.event.getID();
      switch( event_type )
      {
         case MouseEvent.MOUSE_PRESSED:
            // Handle paning: set the new map center to the location 
            // of the click.
            if( !PanMode )
               return;

            // Use the left button clicks only.
            if( GetButton( trace_info.event ) != 1 )
               return;

            PanMode = false;

            point.x = (double) ((MouseEvent)trace_info.event).getX();
            point.y = (double) ((MouseEvent)trace_info.event).getY();

            // COORD_MAPPING_ADJ is added to the cursor coordinates for precise
            // pixel mapping.
            point.x += GlgObject.COORD_MAPPING_ADJ;
            point.y += GlgObject.COORD_MAPPING_ADJ;

            // Converts X/Y to lat/lon using GIS object's current projection.
            GISObject.GISConvert( null, GlgObject.SCREEN_COORD,
                    /* X/Y to Lat/Lon */ true,
                    point, lat_lon );

            // Pan the map.
            GISObject.SetGResource( "GISCenter", lat_lon );
            MapVp.Update();
            break;

         default: return;
      }
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

      GetIconPosition( Icon ); // Get new position
      PositionIcon( Icon ); // Set new position of the icon

      // Display distance between the icon and a selected target, if any.
      DisplayDistance();

      Update(); // Make changes visible.
   }

   //////////////////////////////////////////////////////////////////////////
   // Calculates distance in meters between the moving icon and currently
   // selected target, and displays the distance in the text object
   // named DistancePopup.
   //////////////////////////////////////////////////////////////////////////
   public void DisplayDistance()
   {
      // If no target is selected with the mouse, don't do anything.
      if( SelectedTarget == null )
      {
         MapVp.SetDResource( "DistancePopup/Distance", 0.0 );
         MapVp.SetSResource( "DistancePopup/DestinationString", "" );
         MapVp.SetDResource( "DistancePopup/Visibility", 0.0 );
         return;
      }

      // Make DistancePopup object visible.
      MapVp.SetDResource( "DistancePopup/Visibility", 1.0 );

      // Use DataIndex resource of the SelectedTarget object
      // for direct access in the TargetArray.
      int data_index = SelectedTarget.GetDResource( "DataIndex" ).intValue();
      double distance = GetGlobeDistance( Icon.lat_lon,
                                   TargetArray[ data_index ].lat_lon );

      // Display distance valu in km.
      MapVp.SetDResource( "DistancePopup/Distance", distance / 1000. );

      // Set the DestinationString, indicating the currently selected
      // target name 
      MapVp.SetSResource( "DistancePopup/DestinationString",
                          TargetArray[ data_index ].name );
   }

   //////////////////////////////////////////////////////////////////////////
   // Returns length (in meters) in 3D between two points defined in
   // lat/lon coordinates. Use more complex math for curves around the Earth.
   //////////////////////////////////////////////////////////////////////////
   public double GetGlobeDistance( GlgPoint lat_lon1, GlgPoint lat_lon2 )
   {
      GlgPoint globe_point1 = new GlgPoint(),
               globe_point2 = new GlgPoint();

      // XYZ of the first point, in meters.
      GetPointXYZ( lat_lon1, globe_point1 );

      // XYZ of the second point, in meters.
      GetPointXYZ( lat_lon2, globe_point2 );

      return Math.sqrt(
                  ( globe_point1.x - globe_point2.x ) *
                  ( globe_point1.x - globe_point2.x ) +
                  ( globe_point1.y - globe_point2.y ) *
                  ( globe_point1.y - globe_point2.y ) +
                  ( globe_point1.z - globe_point2.z ) *
                  ( globe_point1.z - globe_point2.z )
                  );
   }

   //////////////////////////////////////////////////////////////////////////
   public void GetPointXYZ( GlgPoint lat_lon, GlgPoint xyz )
   {
      double
         angle_x,
         angle_y;

      // Place [0,0] at the math's x axis for simplicity.

      angle_x = DegToRad( lat_lon.x );
      angle_y = DegToRad( lat_lon.y );

      xyz.x = EQUATOR_RADIUS * Math.cos( angle_x ) * Math.cos( angle_y );
      xyz.y = EQUATOR_RADIUS * Math.sin( angle_y );
      xyz.z = EQUATOR_RADIUS * Math.sin( angle_x ) * Math.cos( angle_y );
   }

   // Simulation: obtain a new icon's position.
   // In an application, update the position from real data source.
   //////////////////////////////////////////////////////////////////////////
   public void GetIconPosition( IconData icon )
   {
      double rad_angle, radius = 10.;

      PathAngle += AngleIncrement;
      if( PathAngle > 360. )
         PathAngle = 0.;

      rad_angle = DegToRad( PathAngle );
      icon.lat_lon.x = InitCenter.x + radius * Math.cos( rad_angle );
      icon.lat_lon.y = InitCenter.y + radius * Math.sin( rad_angle );
      icon.lat_lon.z = 0.;
      icon.angle = PathAngle + 90.;
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
         Timer timer = new Timer( 30, this );
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
   double DegToRad( double angle )
   {
      return angle / 180. * Math.PI;
   }

   //////////////////////////////////////////////////////////////////////////
   double RadToDeg( double angle )
   {
      return angle / Math.PI * 180.;
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
      frame.setSize( 900, 700 );
      frame.addWindowListener( new DemoQuit() );

      GlgGISExample2 gis_example = new GlgGISExample2();
      frame.getContentPane().add( gis_example );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      //
      gis_example.SetDrawingName( "gis_example2.g" );

      // Start periodic updates.
      gis_example.StartUpdates();
   }

   class IconData
   {
      GlgObject icon_obj; // Graphical object representing an icon
      GlgPoint lat_lon; // Icon position in lat/lon coordinates
      double angle; // Icon's rotation angle

      IconData()
      {
         lat_lon = new GlgPoint();
      }
   }

   class TargetData
   {
      String name; // Target's name
      GlgObject graphics; // Graphical object representing a target

      GlgPoint lat_lon; // Target position in lat/lon coordinates

      TargetData( String name_p, double lon, double lat )
      {
         name = name_p;
         lat_lon = new GlgPoint( lon, lat, 0. );
      }
   }
}
