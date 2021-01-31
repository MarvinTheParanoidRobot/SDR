//////////////////////////////////////////////////////////////////////////
// This demo uses GLG as a bean and may be used in a browser or stand-alone.
//////////////////////////////////////////////////////////////////////////
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import java.lang.reflect.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgMapDemo extends GlgJBean implements ActionListener
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   boolean debug = false;

   static final int UpdateInterval = 30;    // Update interval in milliseconds.

   GlgObject
     Drawing = null,
     LinkTemplate = null,
     IconArray = null,
     FacilitiesGroup = null,
     LinksGroup = null,
     SelectedColorIndex = null,
     SelectedObject = null,
     flow_display_obj = null,
     MapViewport = null;

   double
     IconScale = 0.6,                     // Icon size
     GlgMinX, GlgMaxX, GlgMinY, GlgMaxY,  // GLG extent in world coordinates
     MapMinX, MapMaxX, MapMinY, MapMaxY;  // Map extent in degrees, mapped to
                                            // the GLG extent.
   GlgPoint 
     // Create tmp points just once
     point       = new GlgPoint(),
     world_point = new GlgPoint(),
     lat_lon     = new GlgPoint(),
     xy          = new GlgPoint(),
     // Calculated values
     MapCenter   = new GlgPoint(),
     MapExtent   = new GlgPoint();

   int
     UpdateN = 4,      // Update on every N-th iteration of the simulated data
     ColorScheme = 0,
     counter = 0;      // Used to control update frequency.

   boolean
     IsReady = false,
     PerformUpdates = true,    // Controls dynamic update
     ShowFlow = true,   // Animate flow with "moving ants" dynamics.
     Stretch,			   
     stream_error = false;

   static boolean StandAlone = false;
   
   String
     // Default parameter values to display US map if no arguments.
     drawing_file = "map_demo.g",
     map_file = "us_map.g",
     facilities_file = "facilities_s",
     links_file = "links_s",
     palette_file = "palette.g";

   byte [] buffer = new byte[ 100 ];
   int buffer_length = 100;
   Timer timer = null;

   //////////////////////////////////////////////////////////////////////////
   public GlgMapDemo()
   {
      super();
      SetDResource( "$config/GlgMouseTooltipTimeout", 0.05 );

      // Don't expand selection area for exact state tooltips.
      SetDResource( "$config/GlgPickResolution", 0.0 );

      // Activate Trace callback.
      AddListener( GlgObject.TRACE_CB, this );

      // Disable not used old-style select callback.
      SelectEnabled = false;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   // main() method for using as a stand-alone java demo
   //
   //////////////////////////////////////////////////////////////////////////
   // Optional arguments (supplied as applet parameters when used as an 
   // applet, or as command-line arguments when used as a stand-alone program).
   // 
   // -drawing drawing_file   Drawing template to use (map_demo.g).
   // -map map_file           Map drawing to load (us_map.g or world_map.g).
   // -facilities data_file   Facilities data file, lists the nodes to add 
   //                         to the map drawing.
   // -links data_file        Link data file, lists the links between nodes
   //                         to be added.
   // -palette palette_file   Palette drawing of facility icons
   // -icon_scale scale       Scale factor for the the added icons (set via
   //                         the icon's "IconScale" resource).
   // -show_flow              Animate flow with "moving ants"
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

      JFrame frame = new JFrame( "GLG Supply Chain Demo" );

      frame.setResizable( true );
      frame.setSize( 800, 600 );
      frame.setLocation( 20, 20 );

      GlgMapDemo.StandAlone = true;
      GlgMapDemo map_demo = new GlgMapDemo();

      // Process command line arguments.
      map_demo.ProcessArgs( arg );

      // Use getContentPane() for GlgJBean
      frame.getContentPane().add( map_demo );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );

      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers ReadyCallback which starts updates.
      //
      map_demo.SetDrawingName( map_demo.drawing_file );
   }

   //////////////////////////////////////////////////////////////////////////
   // Processes command line arguments.
   //////////////////////////////////////////////////////////////////////////
   public void ProcessArgs( String [] arg )
   {
      if( arg == null )
        return;

      int argc = arg.length;

      // Scan options
      int skip;
      for( skip = 0; skip < argc; ++skip )
      {
         if( arg[ skip ].equals( "-drawing" ) )
         {
            ++skip;
            if( argc <= skip )
              error( "Missing drawing file.", true );
            drawing_file = arg[ skip ];
         }
         else if( arg[ skip ].equals( "-map" ) )
         {
            ++skip;
            if( argc <= skip )
              error( "Missing map drawing file.", true );
            map_file = arg[ skip ];
         }
         else if( arg[ skip ].equals( "-facilities" ) )
         {
            ++skip;
            if( argc <= skip )
              error( "Missing facilities file.", true );
            facilities_file = arg[ skip ];
         }
         else if( arg[ skip ].equals( "-links" ) )
         {
            ++skip;
            if( argc <= skip )
              error( "Missing links file.", true );
            links_file = arg[ skip ];
         }
         else if( arg[ skip ].equals( "-palette" ) )
         {
            ++skip;
            if( argc <= skip )
              error( "Missing icon palette file.", true );
            palette_file = arg[ skip ];
         }
         else if( arg[ skip ].equals( "-show_flow" ) )
           ShowFlow = true;
         else if( arg[ skip ].equals( "-icon_scale" ) )
         {
            ++skip;
            if( argc <= skip )
               error( "Missing icon scale.", true );

            try
            {
               IconScale = Double.parseDouble( arg[ skip ] );
            }
            catch( NumberFormatException e )
            {
               error( "Missing icon scale.", true );
            }
         }
         else
           break;
      }     
   }

   //////////////////////////////////////////////////////////////////////////
   // Initializes the drawing and starts updates.
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );

      if( !StandAlone )
      {
         // Get parameters
         map_file = GetParameter( "map_file", map_file );
         facilities_file = GetParameter( "facilities", facilities_file );
         links_file = GetParameter( "links", links_file );
         palette_file = GetParameter( "palette", palette_file );
         ShowFlow = GetBoolParameter( "show_flow", ShowFlow );
         IconScale = GetDoubleParameter( "icon_scale", IconScale );
      }

      Initialize();
      IsReady = true;
   }

   //////////////////////////////////////////////////////////////////////////
   void Initialize()
   {
      Drawing = GetViewport();      

      LoadMap();
      Update();
      StartUpdates();
   }

   //////////////////////////////////////////////////////////////////////////
   // Load new map file as specified by the map_file parameter. 
   //////////////////////////////////////////////////////////////////////////
   void LoadMap()
   {
      // Load the map viewport
      MapViewport = 
         GlgObject.LoadWidget( GetFullPath( map_file ), 
                               StandAlone ? GlgObject.FILE : GlgObject.URL );

      if( MapViewport == null )
        error( "Can't load map viewport.", true );
      
      // Set name and position.
      MapViewport.SetSResource( "Name", "MapArea" );
      MapViewport.SetGResource( "Point1", -990.0,  890.0, 0.0 );
      MapViewport.SetGResource( "Point2",  990.0, -990.0, 0.0 );

      Drawing.AddObjectToTop( MapViewport );

      // Query extent info from the map.
      GetExtentInfo( MapViewport );

      // Extract node icons from the palette      
      IconArray = ReadPalette( palette_file );

      // Read facilities info and creates facilities group (used as a layer).
      FacilitiesGroup = ReadFacilities( facilities_file );

      if( FacilitiesGroup != null )
      {
         // Create connection links (creates a group used as a layer)
         LinksGroup = ConnectFacilities( links_file );

         if( LinksGroup != null )   // Add link group to the drawing
           MapViewport.AddObjectToBottom( LinksGroup ); 

         // Add facilities last to be in front of links.
         MapViewport.AddObjectToBottom( FacilitiesGroup ); 

         // Set the icon size of the facility nodes
         SetIconSize();
      }
      
      // Uncomment to save generated drawing.
      // Drawing.SaveObject( "out.g" );

      // Set initial visibility of the value display labels to on.
      Drawing.SetDResource( "MapArea/Icon0/Group/ValueLabel/Visibility", 1.0 );
      Drawing.SetDResource( "MapArea/Link0/ValueLabel/Visibility", 1.0 );

      // Set initial color sheme.
      Drawing.SetDResource( "MapArea/ColorIndex", (double) ColorScheme );
      Drawing.SetDResource( "MapArea/Icon0/Group/ColorIndex",
                           (double) ColorScheme );

      // Add selection dialog on top of the map (no way to reorder in AWT).
      GlgObject dialog = Drawing.GetResourceObject( "SelectionDialog" );
      dialog.SetDResource( "Visibility", 0.0 ); // Erase the dialog if displayed
      Drawing.DeleteObject( dialog );
      Drawing.AddObjectToBottom( dialog );

      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   void UnloadMap()
   {
      FacilitiesGroup = null;
      LinksGroup = null;
      LinkTemplate = null;
      IconArray = null;
      flow_display_obj = null;
      SelectedColorIndex = null;
      SelectedObject = null;

      // Delete the old map drawing.
      Drawing.DeleteObject( MapViewport );
      MapViewport = null;
   }

   //////////////////////////////////////////////////////////////////////////
   void SetIconSize()
   {
      /* Adjust icon size by a specified factor. */
      if( IconScale > 0.0 )
        Drawing.SetDResource( "MapArea/Icon0/Template/IconScale", IconScale );
   }

   //////////////////////////////////////////////////////////////////////////
   // Reads icon palette and returns an array of icons to use for nodes.
   //////////////////////////////////////////////////////////////////////////
   GlgObject ReadPalette( String palette_file )
   {
      GlgObject
        icon_array = null,
        palette_drawing,
        palette;
      int i;
      
      if( palette_file == null )
        return null;    // No palette specified: don't generate icons.
                  
      // Load palette drawing
      palette_drawing = 
        GlgObject.LoadObject( GetFullPath( palette_file ), 
                              StandAlone ? GlgObject.FILE : GlgObject.URL );

      if( palette_drawing == null )
        error( "Can't load icon palette.", true );

      // Get palette object named "Icons"
      palette = palette_drawing.GetResourceObject( "Icons" );
      if( palette == null )
        error( "Can't find palette object.", true );

      // Find link template and store it for creating links.
      LinkTemplate = palette.GetResourceObject( "Link" );

      if( !ShowFlow )   // Set line type to solid line if not supported
        LinkTemplate.SetDResource( "Line/LineType", 0.0 );

      for( i=0; ; ++i )
      {
         // Get icon[i]
         GlgObject icon = palette.GetResourceObject( "Icon" + i );

         if( icon != null )
         {
            if( icon_array == null )    // First icon: create icon array
              icon_array = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
         
            icon_array.AddObjectToBottom( icon );
         }
         else
         {
            if( i == 0 )     // First time: can't find any icons!
              error( "Can't find facilities icons.", true );
         
            // Done reading icons.
            System.out.println( "Scanned " + icon_array.GetSize() + 
                               " icon(s)" );
            break;
         }
      }
   
      return icon_array;
   }

   //////////////////////////////////////////////////////////////////////////
   // Reads facilities data file and generates an array of facility objects
   // using the facility palette. 
   //////////////////////////////////////////////////////////////////////////
   GlgObject ReadFacilities( String facilities_file )
   {
      GlgObject
        facility_group = null,
        icon;

      if( facilities_file == null )
        return null;      // No facilities file: already on the map?

      if( IconArray == null )
      {
         error( "No icon palette.", true );
         return null;
      }

      BufferedInputStream stream = OpenStream( facilities_file );
      if( stream == null )
      {
         error( "Can't open facilities URL: " + facilities_file, true );
         return null;
      }

      // int num_icons = IconArray.GetSize();
      int num_facilities = 0;
      while( true )
      {
         stream_error = false;

         // Read facility record
         String facility_name = ReadName( stream );

         if( facility_name == null )
           break;

         double y = ReadDouble( stream );
         char y_char = ReadChar( stream );
         
         double x = ReadDouble( stream );
         char x_char = ReadChar( stream );

         if( stream_error )
         {
            error( "Syntax error reading facilities file.", true );	   
            break;
         }

         ++num_facilities;
      
         if( debug )
           System.out.println( "facility: " + 
                              num_facilities + " " + facility_name );

         if( x_char == 'W' ||  x_char == 'w' )
           x = 180.0 + ( 180.0 - x );
         if( y_char == 'S' ||  x_char == 's' )
           y = -y;
      
         if( facility_group == null )   // First time: create.
         {
            facility_group = 
              new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
            facility_group.SetSResource( "Name", "Facilities" );
         }

         // Add named icon.
         String icon_name = "Icon" + ( num_facilities - 1 );
         icon = AddNode( facility_group, icon_name, x, y );

         // Set facility display label and value.
         icon.SetSResource( "Template/Label/String", facility_name );
         icon.SetDResource( "Template/Value", GlgObject.Rand( 10.0, 300.0 ) );
      }

      try
      {
         stream.close();
      }
      catch( IOException e ) {}

      System.out.println( "Scanned " + num_facilities + " facilities" );
      return facility_group;
   }

   //////////////////////////////////////////////////////////////////////////
   GlgObject AddNode( GlgObject container, String obj_name,
                     double lon, double lat )
   {      
      // Always use the first icon. Subdrawing dynamics is used to change
        // shapes
      GlgObject icon = (GlgObject) IconArray.GetElement( 0 );

      // Create a copy of it.
      icon = icon.CloneObject( GlgObject.STRONG_CLONE );

      // Set object name
      icon.SetSResource( "Name", obj_name );      
      icon.SetSResource( "TooltipString", obj_name );      
        
      // Set position
      lat_lon.x = lon;
      lat_lon.y = lat;
      lat_lon.z = 0.0;
      GetXY( lat_lon, xy );

      icon.SetGResource( "Position", xy );      
         
      container.AddObjectToBottom( icon );
      return icon;
   }

   //////////////////////////////////////////////////////////////////////////
   // Reads connectivity info from a data file and creates links to connect
   // facilities.
   //////////////////////////////////////////////////////////////////////////
   GlgObject ConnectFacilities( String links_file )
   {
      GlgObject link_group = null;
      
      if( links_file == null || FacilitiesGroup == null )
        return null;

      if( LinkTemplate == null )
      {
         error( "Can't find link template.", true );
         return null;
      }

      BufferedInputStream stream = OpenStream( links_file );

      if( stream == null )
      {
         error( "Can't open links file.", true );
         return null;
      }

      int size = FacilitiesGroup.GetSize();
      int num_links = 0;
      while( true )
      {
         stream_error = false;

         // Read link record

         int from_node = ReadInt( stream );	 
         if( stream_error )
           break;

         int to_node = ReadInt( stream );	 
      
         if( stream_error )
         {
            error( "Syntax error reading links file.", true );	   
            break;
         }

         ++num_links;
      
         if( debug )
           System.out.println( "Link= " + from_node + " : " + to_node );

         if( from_node < 0 || to_node < 0 || 
            from_node >= size || to_node >= size )
         {
            error( "Invalid link index.", true );
            return null;
         }

         if( link_group == null )   // First time: create.
         {
            link_group = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );      
            link_group.SetSResource( "Name", "Connections" );
         }
         
         GlgObject link = 
           AddLink( link_group, from_node, to_node, "Link" + (num_links-1) );

         // Set flow attribute and color.
         double flow = GlgObject.Rand( 1.0, 9.0 );
         double color = (double) ( ( (int) flow ) / 2 );
         link.SetDResource( "Line/LineWidth", flow );
         link.SetDResource( "Line/LineColorIndex", color );
         link.SetDResource( "Value", flow );
      }

      try
      {
         stream.close();
      }
      catch( IOException e ) {}

      System.out.println( "Scanned " + num_links + " links" );
      return link_group;
   }

   //////////////////////////////////////////////////////////////////////////
   GlgObject AddLink( GlgObject container, int from_node, int to_node, 
                     String name )
   {      
      GlgObject
        icon_point,
        link_point,
        xform_point;

      // Create an instance of the link template (polygon and label).
      GlgObject link = LinkTemplate.CloneObject( GlgObject.STRONG_CLONE );
      link.SetSResource( "Name", name );
      link.SetSResource( "TooltipString", name );

      // Constrain the end points to facility nodes. Constrain the link
      // polygon end points, and also the point of the path xform used
      // to keep the label in the middle of the link.
        
      GlgObject link_polygon = link.GetResourceObject( "Line" );
      GlgObject xform_pt_array = 
        link.GetResourceObject( "PathXform/XformAttr1" );

      // First point
      GlgObject icon = (GlgObject) FacilitiesGroup.GetElement( from_node );
      icon_point = icon.GetResourceObject( "Point" );
      link_point = (GlgObject) link_polygon.GetElement( 0 );
      xform_point = (GlgObject) xform_pt_array.GetElement( 0 );
      link_point.ConstrainObject( icon_point );
      xform_point.ConstrainObject( icon_point );
         
      // Second point.
      icon = (GlgObject) FacilitiesGroup.GetElement( to_node );
      icon_point = icon.GetResourceObject( "Point" );
      link_point = (GlgObject) link_polygon.GetElement( 1 );
      xform_point = (GlgObject) xform_pt_array.GetElement( 1 );
      link_point.ConstrainObject( icon_point );
      xform_point.ConstrainObject( icon_point );

      container.AddObjectToBottom( link );

      return link;
   } 

   //////////////////////////////////////////////////////////////////////////
   // Query the extend info from the generated map. The map drawing has named 
   // custom properties attached to it which keep the extent information. The
   // map was generated in such a way that map's extent in lat/lon degrees 
   // (MapMinX, MapMinY, MapMaxX and MapMaxY) was mapped to the full GLG extent
   // of +-1000.0 This information is used later to convert from lat/lon to x/y
   // and vice versa.
   //////////////////////////////////////////////////////////////////////////
   void GetExtentInfo( GlgObject drawing )
   {
      // Query map extent from the loaded map drawing (kept as named custom 
        // properties attached to the drawing).
      MapMinX = drawing.GetDResource( "MinX" ).doubleValue();
      MapMaxX = drawing.GetDResource( "MaxX" ).doubleValue();
      MapMinY = drawing.GetDResource( "MinY" ).doubleValue();
      MapMaxY = drawing.GetDResource( "MaxY" ).doubleValue();

      // Calculate center and extent, used in coordinate conversion.
      MapCenter.x = ( MapMinX + MapMaxX ) / 2.0;
      MapCenter.y = ( MapMinY + MapMaxY ) / 2.0;
      MapCenter.z = 0.0;

      MapExtent.x = MapMaxX - MapMinX;
      MapExtent.y = MapMaxY - MapMinY;
      MapExtent.z = 0.0;

      // Full Glg extent is used for the map, hardcoded. Stretch must be TRUE.
      GlgMinX = -1000.0;
      GlgMaxX =  1000.0;
      GlgMinY = -1000.0;
      GlgMaxY =  1000.0;

      // Query if the drawing preserves X/Y ratio.
      double stretch = drawing.GetDResource( "Stretch" ).doubleValue();
      Stretch = ( stretch != 0.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   // Handle user interaction.
   //////////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject vp, GlgObject message_obj )
   {
      super.InputCallback( vp, message_obj );

      String origin = message_obj.GetSResource( "Origin" );
      String format = message_obj.GetSResource( "Format" );
      String action = message_obj.GetSResource( "Action" );
      // String subaction = message_obj.GetSResource( "SubAction" );

      // Handle window closing.
      if( format.equals( "Window" ) )
      {
         if( action.equals( "DeleteWindow" ) &&
             origin.equals( "SelectionDialog" ) )
         {
            // Close selection dialog
            Drawing.SetDResource( "SelectionDialog/Visibility", 0.0 );
            Drawing.Update();	 
         }
         return;
      }

      if( format.equals( "Button" ) )
      {
         if( !action.equals( "Activate" ) )
           return;

         if( origin.equals( "CloseDialog" ) )
         {
            Drawing.SetDResource( "SelectionDialog/Visibility", 0.0 );
            Drawing.Update();	 
         }
         else if( origin.equals( "ZoomIn" ) )
           MapViewport.SetZoom( null, 'i', 0.0 );
         else if( origin.equals( "ZoomOut" ) )
           MapViewport.SetZoom( null, 'o', 0.0 );
         else if( origin.equals( "ZoomReset" ) )
           MapViewport.SetZoom( null, 'n', 0.0 );
         else if( origin.equals( "ZoomTo" ) )
           MapViewport.SetZoom( null, 't', 0.0 );
         else if( origin.equals( "ColorScheme" ) )
         {
            ColorScheme ^= 1;    // Toggle between 0 and 1
            Drawing.SetDResource( "MapArea/ColorIndex", (double) ColorScheme );
            Drawing.SetDResource( "MapArea/Icon0/Group/ColorIndex",
                                 (double) ColorScheme );
            Drawing.Update();
         }
         else if( origin.equals( "Connections" ) )
         {
            ToggleResource( Drawing, "MapArea/Connections/Visibility" );
         }
         else if( origin.equals( "ValueDisplay" ) )
         {
            // Visibility of all labels is constrained, set just one.
            ToggleResource( Drawing,
                           "MapArea/Icon0/Group/ValueLabel/Visibility" );
            ToggleResource( Drawing, "MapArea/Link0/ValueLabel/Visibility" );
         }
         else if( origin.equals( "Map" ) )
         {
            ToggleResource( Drawing, "MapArea/MapGroup/Visibility" );
         }
         else if( origin.equals( "Update" ) )
         {
            PerformUpdates = !PerformUpdates;
         }	
         else if( origin.equals( "MapType" ) )  // Togle US and world map
         {
            if( map_file.equals( "us_map.g" ) )
            {
               map_file = "world_map.g";
               facilities_file = "facilities_w";
               links_file = "links_w";
            }
            else
            {
               map_file = "us_map.g";
               facilities_file = "facilities_s";
               links_file = "links_s";
            }

            StopUpdates();

            UnloadMap();   // Delete the old map.	    
            LoadMap();     // Load new map.
            Update();

            StartUpdates();
            return;
         }	
      }
      /* Process mouse clicks on objects of interests in the drawing: 
         implemented as an Action with the "Node", "Link" or other label 
         attached to an object and activated on a mouse click. 
      */
      else if( format.equals( "CustomEvent" ) )
      {
         if( MapViewport.GetDResource( "ZoomToMode" ).intValue() != 0 )	   
           return;  // Don't handle selection in ZoomTo mode.

         String 
           label = null,
           visibility_name = null;
         boolean has_data = false;
         GlgObject highlight_obj = null;
         double data = 0.0;
         String icon_name;
         
         String event_label = message_obj.GetSResource( "EventLabel" );

         if( event_label.equals( "BackgroundVP" ) )
         {
            // The background viewport is selection is reported only if there
              // are no other selections: erase the highlight.
            Highlight( Drawing, null );
            Update();
            return;
         }
         // Process state selection on the US map.
         else if( event_label.equals( "MapSelection" ) )
         {
            label = "None";

            // The selection is reported for the MapGroup. The OrigObject is
            // used to get the object ID of the selected lower level state 
              // polygon.
            highlight_obj = message_obj.GetResourceObject( "OrigObject" );
            icon_name = highlight_obj.GetSResource( "Name" );
            has_data = false;
            visibility_name = "MapArea/MapGroup/Visibility";	    
            
            // Location is set to the mouse click by the preceding TraceCB.
         }
         else if( event_label.equals( "Node" ) )
         {
            GlgObject node = message_obj.GetResourceObject( "Object" );
            icon_name = node.GetSResource( "Name" );
            SelectedObject = node;

            // Query the label of the selected node
            label = node.GetSResource( "Group/Label/String" );

            // Query node location.
            GlgPoint position = node.GetGResource( "Position" );

            // Convert world coordinates to lat/lon
            GetLatLon( position, lat_lon );

            // Generate a location info string by converting +- sign info 
              // into the N/S, E/W suffixes.
            String location_str = CreateLocationString( lat_lon );

            // Display position info in the dialog
            Drawing.SetSResource( "SelectionDialog/Location", location_str);

            data = node.GetDResource( "Group/Value" ).doubleValue();
            has_data = true;
            visibility_name = "MapArea/Icon0/Visibility";
         }
         else if( event_label.equals( "Link" ) )
         {
            GlgObject link = message_obj.GetResourceObject( "Object" );
            icon_name = link.GetSResource( "Name" );
            SelectedObject = link;

            label = "None";
            data = link.GetDResource( "Value" ).doubleValue();
            has_data = true;
            visibility_name = "MapArea/Connections/Visibility";

            // Location is set to the mouse click by the preceding TraceCB.
         }
         else
           return;  // No selection

         // Check if this layer is visible.
         double visibility_value = 
           Drawing.GetDResource( visibility_name ).doubleValue();

         if( visibility_value == 1.0 )
         {
            if( icon_name == null )
              icon_name = "";

            // Display the icon name, label and data in the dialog.
            Drawing.SetSResource( "SelectionDialog/ID", icon_name ); 
            Drawing.SetSResource( "SelectionDialog/Facility", label ); 
            if( has_data )
              Drawing.SetDResource( "SelectionDialog/Data", data );
            Drawing.SetDResource( "SelectionDialog/DataLabel/Visibility",
                                  has_data ? 1.0 : 0.0 );

            // Graph's Visibility is constrained to the DataLabel's Visibility
            if( has_data )
              // Reset the graph by setting all datasamples to 0
              Drawing.SetDResource( 
                "SelectionDialog/Graph/DataGroup/Points/DataSample%/Value",
                                   0.0 );

            Drawing.SetDResource( "SelectionDialog/Visibility", 1.0 );	    
            Highlight( Drawing, highlight_obj );
         }
      }
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Used to obtain coordinates of the mouse click. 
   //////////////////////////////////////////////////////////////////////////
   public void TraceCallback( GlgObject viewport, GlgTraceData trace_info )
   {      
      // Use the MapArea's events only.
      if( !IsReady || trace_info.viewport != MapViewport )
        return;

      int event_type = trace_info.event.getID();
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
         if( GetButton( trace_info.event ) != 1 )
            return;  // Use the left button clicks only.
         point.x = (double) ((MouseEvent)trace_info.event).getX();
         point.y = (double) ((MouseEvent)trace_info.event).getY();

         // COORD_MAPPING_ADJ is added to the cursor coordinates for 
           // precise pixel mapping.
         point.x += GlgObject.COORD_MAPPING_ADJ;
         point.y += GlgObject.COORD_MAPPING_ADJ;
         break;

       default: return;
      }      

      if( MapViewport.GetDResource( "ZoomToMode" ).intValue() != 0 )	   
        return;    // Ignore clicks in zoom mode. 

      viewport.ScreenToWorld( true, point, world_point );

      // Generate a location info string by converting +- sign info into the
      // N/S, E/W suffixes.
      String location_str = CreateLocationString( world_point );
      Drawing.SetSResource( "SelectionDialog/Location", location_str );   

      // Set facility to "None" for now: will be set by the Select callback
        // if any selected.
      Drawing.SetSResource( "SelectionDialog/ID", "None" );
      Drawing.SetSResource( "SelectionDialog/Facility", "None" );

      // Not an icon or link: no associated data.
      Drawing.SetDResource( "SelectionDialog/DataLabel/Visibility", 0.0 );
      
      Drawing.SetDResource( "SelectionDialog/Visibility", 1.0 );
      Drawing.Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Set selection dialog's visibility to 0 for the intial appearance 
   // in case it was 1 in the drawing.
   //////////////////////////////////////////////////////////////////////////
   public void HCallback( GlgObject viewport )
   {
      viewport.SetDResource( "SelectionDialog/Visibility", 0.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   // Highlight or unhighlight selected map polygon.
   // Changes the index of the color list transform attached to the object's
   // FillColor.
   //////////////////////////////////////////////////////////////////////////
   void Highlight( GlgObject viewport, GlgObject sel_object )
   {
      // Restore the color of the prev. highlighted object.
      if( SelectedColorIndex != null )
      {
         SelectedColorIndex.SetDResource( null, 0.0 );
         SelectedColorIndex = null;
      }

      // Highlight new object by changing its color
      if( sel_object != null )
      {
         SelectedColorIndex = 
           sel_object.GetResourceObject( "SelectColorIndex" );
         if( SelectedColorIndex != null )
           SelectedColorIndex.SetDResource( null, 1.0 );
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Converts Lat/Lon to X/Y in GLG world coordinates.
   //////////////////////////////////////////////////////////////////////////
   void GetXY( GlgPoint lat_lon, GlgPoint xy )
   {
      GlgObject.GlmConvert( GlgObject.RECTANGULAR_PROJECTION, Stretch, 
                           GlgObject.OBJECT_COORD, /*coord_to_lat_lon*/ false,
                           MapCenter, MapExtent, 0.0,
                           GlgMinX, GlgMaxX, GlgMinY, GlgMaxY,
                           lat_lon, xy );
   }

   //////////////////////////////////////////////////////////////////////////
   // Converts Lat/Lon to X/Y in GLG world coordinates.
   //////////////////////////////////////////////////////////////////////////
   void GetLatLon( GlgPoint xy, GlgPoint lat_lon )
   { 
      GlgObject.GlmConvert( GlgObject.RECTANGULAR_PROJECTION, Stretch, 
                           GlgObject.OBJECT_COORD, /*coord_to_lat_lon*/ true,
                           MapCenter, MapExtent, 0.0,
                           GlgMinX, GlgMaxX, GlgMinY, GlgMaxY,
                           xy, lat_lon );
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
      
      return "Lon=" + x_deg + "\u00B0" + x_min + "\'" + x_sec + "\"" + char_x +
            "  Lat=" + y_deg + "\u00B0" + y_min + "\'" + y_sec + "\"" + char_y;
   }

   //////////////////////////////////////////////////////////////////////////
   // Update display with data.
   //////////////////////////////////////////////////////////////////////////
   void UpdateMap()
   {
      int i, size;
      double value;
      String res_name;

      if( timer == null )
        return;   // Prevents race conditions

      if( PerformUpdates )
      {
         if( ShowFlow )
         {
            if( flow_display_obj == null )   // First time.
              flow_display_obj =
                Drawing.GetResourceObject( "MapArea/Link0/Line/LineType" );
            
            // Links's flow is constrained: animating one animates all. 
            // Flow direction is defined by the order of the links points when
            // constrained.
            if( flow_display_obj != null )
            {
               // Query the current line type and offset
               double flow_data = 
                 flow_display_obj.GetDResource( null ).doubleValue();
               int line_type = ( (int)flow_data ) % 32;
               int offset = ( (int)flow_data ) / 32;
               
               // Increase the offset and set it back.
               --offset;
               if( offset < 0 )
                 offset = 32 * 31;
               flow_data = offset * 32 + line_type;
               flow_display_obj.SetDResource( null, flow_data );      
            }
         }
      
         // Update facility values every time.
         size = FacilitiesGroup.GetSize();
         for( i=0; i<size; ++i )
         {
            value = GlgObject.Rand( 30.0, 500.0 );
            res_name = "MapArea/Icon" + i;
            GlgObject icon = Drawing.GetResourceObject( res_name );
            icon.SetDResource( "Group/Value", value );
            
            // Update selected object data display in the SelectionDialog.
            if( icon == SelectedObject )
            {
               Drawing.SetDResource( "SelectionDialog/Data", value );
               Drawing.SetDResource( "SelectionDialog/Graph/DataGroup/EntryPoint",
                                     value / 500.0 );
               // To scroll ticks.
               Drawing.SetSResource( "SelectionDialog/Graph/XMajorGroup/TicksEntryPoint",
                                     "" );
            }
            
            if( ( counter % UpdateN ) == 0 ) // Update icon type every n-th time.
            {
               if( GlgObject.Rand( 0.0, 10.0 ) > 2.0 )
               {
                  double icon_type = 
                    icon.GetDResource( "Group/Graphics/IconType" ).doubleValue();
                  
                  if( icon_type != 0.0 )
                    icon_type = 0.0;
                  else		 
                    icon_type = GlgObject.Rand( 0.0, 6.0 );
                  
                  icon.SetDResource( "Group/Graphics/IconType", icon_type );
               }
            }
         }
         
         if( ( counter % UpdateN ) == 0 )  // Update link values every n-th time.
         {
            size = LinksGroup.GetSize();
            for( i=0; i<size; ++i )
            {      
               value = GlgObject.Rand( 1.0, 9.0 );
               res_name = "MapArea/Link" + i;
               GlgObject link = Drawing.GetResourceObject( res_name );
               link.SetDResource( "Value", value );
               if( ShowFlow )
                 link.SetDResource( "Line/LineWidth", value );
               link.SetDResource( "Line/LineColorIndex",
                                  (double)(((long)value)/2) );
               
               // Update selected object data display in the SelectionDialog.
               if( link == SelectedObject )
               {
                  Drawing.SetDResource( "SelectionDialog/Data", value );
                  Drawing.SetDResource( "SelectionDialog/Graph/DataGroup/EntryPoint", 
                                       value / 10.0 );
                  // To scroll ticks.
                  Drawing.SetSResource( "SelectionDialog/Graph/XMajorGroup/TicksEntryPoint",
                                       "" );
               }
            }
         }
         
         ++counter;
         if( counter > 1000 )
           counter = 0;
         
         Drawing.Update();
      }

      timer.start();   // Restart the update timer
   }

   //////////////////////////////////////////////////////////////////////////
   // Toggle resource between 0 and 1.
   //////////////////////////////////////////////////////////////////////////
   void ToggleResource( GlgObject glg_object, String res_name )
   {
      double value = glg_object.GetDResource( res_name ).doubleValue();
      glg_object.SetDResource( res_name, value != 0.0 ? 0.0 : 1.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   static void error( String message, boolean quit )
   {
      System.out.println( message );

      // Thread.currentThread().dumpStack();

      if( quit )
        System.exit( 1 );
   }

   //////////////////////////////////////////////////////////////////////////
   BufferedInputStream OpenStream( String filename )
   {
      filename = GetFullPath( filename );

      if( StandAlone )
      {
         try
         {
            File file = new File( filename );
            return new BufferedInputStream( new FileInputStream( file ) );
         }
         catch( NullPointerException e )
         {
            error( "null filename", true );
         }
         catch( FileNotFoundException e )
         {
            error( "Can't open file: " + filename, true );
         }
         catch( SecurityException e )
         {
            error( "Can't open file: " + filename, true );
         }
      }
      else
      {
         URL url;

         try
         {
            url = new URL( filename );
         }
         catch( MalformedURLException e )
         {
            error( "Invalid URL: " + filename, true );
            return null;
         }
         
         try
         {
            URLConnection connection = url.openConnection();
            connection.setUseCaches( false ); 
            connection.setDefaultUseCaches( false ); 
            
            return new BufferedInputStream( url.openStream() );
         }
         catch( IOException e )
         {
            error( "Can't open URL: " + filename, true );
         }
      }
      return null;
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
   String GetParameter( String name, String default_value )
   {
      String param = getParameter( name );
      if( param != null )
        return param;
      else
        return default_value;
   }

   ////////////////////////////////////////////////////////////////////////
   boolean GetBoolParameter( String name, boolean default_value )
   {
      String param = getParameter( name );
      if( param == null )
        return default_value;

      return param.equals( "True" ) || param.equals( "true" ) || 
        param.equals( "TRUE" );
   }

   ////////////////////////////////////////////////////////////////////////
   double GetDoubleParameter( String name, double default_value )
   {
      String param = getParameter( name );
      if( param == null )
        return default_value;

      try
      {
         return new Double( param ).doubleValue();
      }
      catch( NumberFormatException e )
      {
         error( "Can't parse double parameter: " + name, false );
         return default_value;
      }
   }

   /////////////////////////////////////////////////////////////////////
   // Reads a string into a buffer, returns string length.
   // Automatically eats trailing white space
   /////////////////////////////////////////////////////////////////////
   int ReadSimpleString( InputStream stream )
   {
      int
        i,
        offset = 0,
        next_char;
      
      buffer[ 0 ] = '\0';
      
      try
      {
         // Skip white places
         while( buffer[ 0 ] == ' ' ||
               buffer[ 0 ] == '\n' ||
               buffer[ 0 ] == '\r' ||
               buffer[ 0 ] == '\0' )
         {
            next_char = stream.read();
            if( next_char == -1 )
              return 0;

            buffer[ 0 ] = (byte) next_char;
         }

         // offset = 0 here
         while( buffer[ offset ] != ' ' &&
               buffer[ offset ] != '\n' &&
               buffer[ offset ] != '\r' &&
               buffer[ offset ] != 0 )
         {
            ++offset;
            if( offset == buffer_length )   // Increase buffer size
            {	 
               byte [] new_buffer = new byte[ buffer_length * 2 ];
               for( i=0; i < buffer_length; ++i )	   
                 new_buffer[i] = buffer[i];
               buffer = new_buffer;
               buffer_length *= 2;
            }
            
            next_char = stream.read();
            if( next_char == -1 )
              return 0;   // The last char may have no space after it: accept

            buffer[ offset ] = (byte) next_char;
         }
         return offset;
      }
      catch( IOException e )
      {
         stream_error = true;
         return 0;
      }
   }

   String ReadString( InputStream stream )
   {
      int length = ReadSimpleString( stream );
      if( length == 0 )
        return null;
      return new String( buffer, 0, length );
   }

   String ReadName( InputStream stream )
   {
      String 
        name = null,
        name_part;

      while( true )
      {
         name_part = ReadString( stream );
         if( name_part == null || name_part.equals( ":" ) )
           return name;

         if( name == null )
           name = name_part;
         else
           name = name + " " + name_part;
      }
   }

   char ReadChar( InputStream stream )
   {
      int ch;

      while( true )
      {
         try
         {
            ch = stream.read();
         }
         catch( IOException e )
         {
            ch = -1;
         }

         if( ch != ' ' && ch != '\n' && ch != '\r' && ch != 0 )
           break;
      }

      if( ch == -1 )
      {
         stream_error = true;   // EOF
      }
      return (char) ch;
   }

   int ReadInt( InputStream stream )
   {
      int length = ReadSimpleString( stream );
      if( length == 0 )
      {
         stream_error = true;   // EOF
         return 0;
      }

      try
      {
         return Integer.parseInt( new String( buffer, 0, length ) );
      }
      catch( NumberFormatException e )
      {
         stream_error = true;
         error( "Error parsing integer value.", false );
         return 0;
      }
   }

   double ReadDouble( InputStream stream )
   {
      int length = ReadSimpleString( stream );
      if( length == 0 )
      {
         stream_error = true;   // EOF
         return 0.0;
      }

      try
      {
         return Double.parseDouble( new String( buffer, 0, length ) );
      }
      catch( NumberFormatException e )
      {
         error( "Error parsing double value.", false );
         stream_error = true;
         return 0.0;
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
   void StartUpdates()
   {
      if( timer == null )
      {
         // Restart the timer after each update (instead of using repeats)
         // to avoid flooding the event queue with timer events on slow 
         // machines.
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
      UpdateMap();
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked by the browser to stop the applet
   //////////////////////////////////////////////////////////////////////////
   public void stop()
   {
      StopUpdates();
      IsReady = false;
      super.stop();
   }
}
