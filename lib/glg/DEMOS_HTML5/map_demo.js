//////////////////////////////////////////////////////////////////////////////
// GLG Supply Chain Visualization Demo
//
//The demo demonstrates GLG features that can be used in supply chain
// monitoring applications, including on-the-fly creation of dynamic
// nodes and links, positioning them on the map and updating with
// real-time data.
//
// The demo is written in pure HTML5 and JavaScript. The source code of the
// demo uses the GLG Toolkit JavaScript Library supplied by the included
// Glg*.js and GlgToolkit*.js files.
//
// The library loads a GLG drawing and renders it on a web page, providing
// an API to animate the drawing with real-time data and handle user
// interaction with graphical objects in the drawing.
//
// The drawings are created using the GLG Graphics Builder, an interactive
// editor that allows to create grahical objects and define their dynamic
// behavior without any programming.
//
// Except for the changes to comply with the JavaScript syntax, this source
// is identical to the source code of the corresponding C/C++, Java and C#
// versions of the demo.
//////////////////////////////////////////////////////////////////////////////

// Get a handle to the GLG Toolkit library.
var GLG = new GlgToolkit();

// Debugging aid: uncomment the next line to throw an exception on a GLG error.
//GLG.ThrowExceptionOnError( true, true, true );

// Set initial size of the drawing.
SetDrawingSize( false );

// Increase canvas resoultion for mobile devices.
var CoordScale = SetCanvasResolution();

/* Loads misc. assets used by the program and invokes the LoadDrawing function
   when done.
*/
LoadAssets( LoadDrawing );

//////////////////////////////////////////////////////////////////////////////
function LoadDrawing()
{
   /* Load a drawing from the map_demo.g file. 
      The LoadCB callback will be invoked when the drawing has been loaded.
   */
   GLG.LoadWidgetFromURL( "map_demo.g", null, LoadCB, null );
}

//////////////////////////////////////////////////////////////////////////////
function LoadCB( drawing, data, path )
{
   if( drawing == null )
   {
      window.alert( "Can't load drawing, check console message for details." );
      return;
   }
   
   // Define the element in the HTML page to display the drawing in.
   drawing.SetParentElement( "glg_area" );
   
   // Disable viewport border to use the border of the glg_area.
   drawing.SetDResource( "LineWidth", 0 );

   StartSupplyChainDemo( drawing );
}

//////////////////////////////////////////////////////////////////////////////
// Control variables and constants
//////////////////////////////////////////////////////////////////////////////

// Graphics update interval.
const UPDATE_INTERVAL = 30;    // msec

var
   Drawing,               /* GlgObject */
   // Viewport used as a parent of a loaded map viewport.
   MapContainer,          /* GlgObject */
   LinkTemplate,          /* GlgObject */
   IconArray,             /* GlgObject */
   FacilitiesGroup,       /* GlgObject */
   LinksGroup,            /* GlgObject */
   SelectedColorIndex,    /* GlgObject */
   SelectedObject,        /* GlgObject */
   flow_display_obj,      /* GlgObject */
   MapViewport,           /* GlgObject */
   Palette;               /* GlgObject */

var
   USMapData,             /* Raw data */
   USFacilities,          /* String */
   USLinks,               /* String */
   WorldMapData,          /* Raw data */
   WorldFacilities,       /* String */
   WorldLinks;            /* String */
   
var
   IconScale = 0.6,                         /* double - Icon size coeff. */
      // GLG extent in world coordinates
   GlgMinX, GlgMaxX, GlgMinY, GlgMaxY,      /* double */
      // Map extent in degrees, mapped to the GLG extent.
   MapMinX, MapMaxX, MapMinY, MapMaxY;      /* double */

var
   // Temporary variables
   point       = GLG.CreateGlgPoint( 0, 0, 0 ),   /* GlgPoint */
   world_point = GLG.CreateGlgPoint( 0, 0, 0 ),   /* GlgPoint */
   lat_lon     = GLG.CreateGlgPoint( 0, 0, 0 ),   /* GlgPoint */
   xy          = GLG.CreateGlgPoint( 0, 0, 0 ),   /* GlgPoint */
   // Calculated values
   MapCenter   = GLG.CreateGlgPoint( 0, 0, 0 ),   /* GlgPoint */
   MapExtent   = GLG.CreateGlgPoint( 0, 0, 0 );   /* GlgPoint */

var
   // Update on every 4-th iteration of the simulated data
   UpdateN = 4,        /* int */
   ColorScheme = 0,    /* int */
   // Controls update frequency.
   counter = 0;        /* int */

var
   // Start with the US map.
   USMap = true,	     /* boolean */   
   PerformUpdates = true,    /* boolean */
   // Animate flow with "moving ants" dynamics.
   ShowFlow = true,          /* boolean */
   Stretch = false,          /* boolean */
   StartDragging = false,    /* boolean */
   FirstLoadError = true,    /* boolean */
   stream_error = false;     /* boolean */

var
   us_map_file = "us_map.g",
   us_facilities_file = "facilities_s",
   us_links_file = "links_s",

   world_map_file = "world_map.g",
   world_facilities_file = "facilities_w",
   world_links_file = "links_w",

   palette_file = "palette.g";

var buffer_length = 256;                        /* Initial buffer length */
var buffer = new Uint16Array( buffer_length );  /* Buffer for reading strings */

var timer = null;

//////////////////////////////////////////////////////////////////////////////
function StartSupplyChainDemo( drawing )
{
   Drawing = drawing;

   InitDrawing();
   Drawing.InitialDraw();

   // Start periodic updates.
   timer = setTimeout( UpdateMap, UPDATE_INTERVAL );
}
   
//////////////////////////////////////////////////////////////////////////////
function InitDrawing()
{
   // Extract node icons from the palette      
   IconArray = ReadPalette( Palette );

   AdjustForMobileDevices();

   MapContainer = Drawing.GetResourceObject( "MapContainer" );
   
   LoadMap();

   /* Make Selection Dialog a floating dialog, adjust its height and vertical 
      placement, and set its title.
   */
   Drawing.SetDResource( "SelectionDialog/ShellType",
                         GLG.GlgShellType.DIALOG_SHELL );
   Drawing.SetDResource( "SelectionDialog/DialogHeight", 150 );
   Drawing.SetDResource( "SelectionDialog/DialogY", -700 );
   Drawing.SetSResource( "SelectionDialog/Screen/ScreenName",
                         "Selection Information" );

   Drawing.AddListener( GLG.GlgCallbackType.INPUT_CB, InputCallback );
   Drawing.AddListener( GLG.GlgCallbackType.TRACE_CB, TraceCallback );
}

//////////////////////////////////////////////////////////////////////////
// Load a new map file. 
//////////////////////////////////////////////////////////////////////////
function LoadMap()
{
   // Load the map viewport from pre-fetched raw data.
   MapViewport = GLG.LoadWidget( USMap ? USMapData : WorldMapData );

   if( MapViewport == null )
     alert( "Can't load map viewport, check console message for details." );
      
   // Set viewport name.
   MapViewport.SetSResource( "Name", "MapArea" );

   /* Set control points of the map viewport to fill the whole area of the
      MapContainer viewport.
   */
   MapViewport.SetGResource( "Point1", -1000.0, -1000.0, 0.0 );
   MapViewport.SetGResource( "Point2",  1000.0,  1000.0, 0.0 );
   
   MapContainer.AddObjectToTop( MapViewport );

   // Query extent info from the map.
   GetExtentInfo( MapViewport );

   // Read facilities info and creates facilities group (used as a layer).
   FacilitiesGroup = ReadFacilities( USMap ? USFacilities : WorldFacilities );

   if( FacilitiesGroup != null )
   {
      // Create connection links (creates a group used as a layer)
      LinksGroup = ConnectFacilities( USMap ? USLinks : WorldLinks );

      if( LinksGroup != null )   // Add link group to the drawing
        MapViewport.AddObjectToBottom( LinksGroup ); 

      // Add facilities last to be in front of links.
      MapViewport.AddObjectToBottom( FacilitiesGroup ); 

      // Set the icon size of the facility nodes
      SetIconSize();
   }
      
   // Set initial visibility of the value display labels to on.
   Drawing.SetDResource( "MapArea/Icon0/Group/ValueLabel/Visibility", 1.0 );
   Drawing.SetDResource( "MapArea/Link0/ValueLabel/Visibility", 1.0 );

   // Set initial color sheme.
   Drawing.SetDResource( "MapArea/ColorIndex", ColorScheme );
   Drawing.SetDResource( "MapArea/Icon0/Group/ColorIndex", ColorScheme );

   // Erase the dialog if displayed
   Drawing.SetDResource( "SelectionDialog/Visibility", 0.0 ); 

   Drawing.Update();
}

//////////////////////////////////////////////////////////////////////////
function UnloadMap()
{
   FacilitiesGroup = null;
   LinksGroup = null;

   flow_display_obj = null;
   SelectedColorIndex = null;
   SelectedObject = null;
   
   // Delete the old map drawing.
   MapContainer.DeleteThisObject( MapViewport );
   MapViewport = null;
}

//////////////////////////////////////////////////////////////////////////
function ChangeMap()
{
   // Show "Loading map" message.
   MapViewport.SetDResource( "LoadMessage/Visibility", 1 );
   Drawing.Update();

   // Yield to let browser shows the message before proceeding.
   setTimeout( ChangeMapCB, 10 );
}

//////////////////////////////////////////////////////////////////////////
function ChangeMapCB()
{        
   UnloadMap();   // Delete the old map.	    
   LoadMap();     // Load new map.
   Drawing.Update();
}

//////////////////////////////////////////////////////////////////////////
function SetIconSize()
{
   /* Adjust icon size by a specified factor. */
   if( IconScale > 0.0 )
     Drawing.SetDResource( "MapArea/Icon0/Template/IconScale", IconScale );
}

//////////////////////////////////////////////////////////////////////////
// Extract icons from the palette and return an array of icons to use
// for nodes.
//////////////////////////////////////////////////////////////////////////
function ReadPalette( /* GlgObject */ palette_obj )   /* GlgObject */
{
   if( palette_obj == null )
     return null;    // No palette: don't generate icons.
   
   // Get palette viewport named "Icons"
   palette_vp = palette_obj.GetResourceObject( "Icons" );   /* GlgObject */
   if( palette_vp == null )
     alert( "Can't find palette object." );
   
   // Find link template and store it for creating links.
   LinkTemplate = palette_vp.GetResourceObject( "Link" );
   
   if( !ShowFlow )   // Set line type to solid line if no flow.
     LinkTemplate.SetDResource( "Line/LineType", 0.0 );

   var icon_array;   /* GlgObject */
   for( var i=0; ; ++i )
   {
      // Get icon[i]
      var icon = palette_vp.GetResourceObject( "Icon" + i );   /* GlgObject */
      
      if( icon != null )
      {
         if( icon_array == null )    // First icon: create icon array
           icon_array =
             GLG.CreateObject( GLG.GlgObjectType.ARRAY,
                               GLG.GlgContainerType.GLG_OBJECT,
                               0, 0, null, null );
         
         icon_array.AddObjectToBottom( icon );
      }
      else
      {
         if( i == 0 )     // First time: can't find any icons!
           alert( "Can't find facilities icons." );
         
         break;
      }
   }
   return icon_array;
}

//////////////////////////////////////////////////////////////////////////
// Reads facilities data and generates an array of facility objects
// using the facility palette. 
//////////////////////////////////////////////////////////////////////////
function ReadFacilities( /* String */ facilities_data )   /* GlgObject */
{
   var
     facility_group,   /* GlgObject */
     icon;             /* GlgObject */

   if( facilities_data == null )     
     return null;   // No facilities
   
   if( IconArray == null )
   {
      alert( "No icon palette." );
      return null;
   }

   var stream = new InputStream( facilities_data );    /* InputStream */
   
   var num_facilities = 0;    /* int */
   while( true )
   {
      // Read facility record
      var facility_name = ReadName( stream );   /* String  */

      if( facility_name == null )
        break;

      var y = ReadDouble( stream );
      var y_char = ReadChar( stream );
      
      var x = ReadDouble( stream );
      var x_char = ReadChar( stream );

      if( y == null || y_char == null || x == null || x_char == null )
      {
         alert( "Syntax error reading facilities file." );
         break;
      }

      ++num_facilities;
      
      if( x_char == 'W' ||  x_char == 'w' )
        x = 180.0 + ( 180.0 - x );
      if( y_char == 'S' ||  x_char == 's' )
        y = -y;
      
      if( facility_group == null )   // First time: create.
      {
         facility_group =
           GLG.CreateObject( GLG.GlgObjectType.ARRAY,
                             GLG.GlgContainerType.GLG_OBJECT, 0, 0, null, null );
         facility_group.SetSResource( "Name", "Facilities" );
      }
      
      // Add named icon.
      var icon_name = "Icon" + ( num_facilities - 1 );
      icon = AddNode( facility_group, icon_name, x, y );

      // Set facility display label and value.
      icon.SetSResource( "Template/Label/String", facility_name );
      icon.SetDResource( "Template/Value", GLG.Rand( 10.0, 300.0 ) );
   }

   console.log( "Scanned " + num_facilities + " facilities" );
   return facility_group;
}

//////////////////////////////////////////////////////////////////////////
function AddNode( /* GlgObject */ container, /* String */ obj_name,
                  /* double */ lon, /* double */ lat )         /* GlgObject */
{      
   /* Always use the first icon. Subdrawing dynamics is used to change
      shapes.
   */
   var icon = IconArray.GetElement( 0 );   /* GlgObject  */

   // Create a copy of it.
   icon = icon.CloneObject( GLG.GlgCloneType.STRONG_CLONE );

   // Set object name
   icon.SetSResource( "Name", obj_name );      
   icon.SetSResource( "TooltipString", obj_name );      
   
   // Set position
   lat_lon.x = lon;
   lat_lon.y = lat;
   lat_lon.z = 0.0;
   GetXY( lat_lon, xy );
   
   icon.SetGResourceFromPoint( "Position", xy );      
   
   container.AddObjectToBottom( icon );
   return icon;
}

//////////////////////////////////////////////////////////////////////////
// Reads connectivity data and creates links to connect facilities.
//////////////////////////////////////////////////////////////////////////
function ConnectFacilities( /* String */ links_data )   /* GlgObject */
{
   var link_group = null;   /* GlgObject */
      
   if( links_data == null || FacilitiesGroup == null )
     return null;

   if( LinkTemplate == null )
   {
      alert( "Can't find link template." );
      return null;
   }

   var stream = new InputStream( links_data );    /* InputStream */

   var size = FacilitiesGroup.GetSize();
   var num_links = 0;
   while( true )
   {
      // Read link record
      
      var from_node = ReadInt( stream );
      if( from_node == null )
        break;

      var to_node = ReadInt( stream );	 
      if( to_node == null )
      {
         alert( "Syntax error reading links file." );
         break;
      }

      ++num_links;
      
      if( from_node < 0 || to_node < 0 || from_node >= size || to_node >= size )
      {
         alert( "Invalid link index." );
         break;
      }

      if( link_group == null )   // First time: create.
      {
         link_group =
           GLG.CreateObject( GLG.GlgObjectType.ARRAY,
                             GLG.GlgContainerType.GLG_OBJECT, 0, 0, null, null );
         link_group.SetSResource( "Name", "Connections" );
      }
         
      var link =   /* GlgObject */
        AddLink( link_group, from_node, to_node, "Link" + ( num_links - 1 ) );

      // Set flow attribute and color.
      var flow = GLG.Rand( 1.0, 9.0 );    /* double */
      var color = Math.trunc( flow ) / 2;       /* double */
      link.SetDResource( "Line/LineWidth", flow );
      link.SetDResource( "Line/LineColorIndex", color );
      link.SetDResource( "Value", flow );
   }

   console.log( "Scanned " + num_links + " links" );
   return link_group;
}

//////////////////////////////////////////////////////////////////////////
function AddLink( /* GlgObject */ container, /* int */ from_node,
                  /* int */ to_node, /* String */ name )    /* GlgObject */
{      
   var   /* GlgObject */
     icon_point,
     link_point,
     xform_point;
   
   // Create an instance of the link template (polygon and label).
   var link =      /* GlgObject */
     LinkTemplate.CloneObject( GLG.GlgCloneType.STRONG_CLONE );
   link.SetSResource( "Name", name );
   link.SetSResource( "TooltipString", name );

   /* Constrain the end points to facility nodes. Constrain the link
      polygon end points, and also the point of the path xform used
      to keep the label in the middle of the link.
   */
        
   var link_polygon = link.GetResourceObject( "Line" );   /* GlgObject */
   var xform_pt_array =
     link.GetResourceObject( "PathXform/XformAttr1" );    /* GlgObject */
   
   // First point
   var icon = FacilitiesGroup.GetElement( from_node );   /* GlgObject */
   icon_point = icon.GetResourceObject( "Point" );
   link_point = link_polygon.GetElement( 0 );
   xform_point = xform_pt_array.GetElement( 0 );
   link_point.ConstrainObject( icon_point );
   xform_point.ConstrainObject( icon_point );
         
   // Second point.
   icon = FacilitiesGroup.GetElement( to_node );
   icon_point = icon.GetResourceObject( "Point" );
   link_point = link_polygon.GetElement( 1 );
   xform_point = xform_pt_array.GetElement( 1 );
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
function GetExtentInfo( /* GlgObject */ drawing )
{
   /* Query map extent from the loaded map drawing (kept as named custom 
      properties attached to the drawing).
   */
   MapMinX = drawing.GetDResource( "MinX" );
   MapMaxX = drawing.GetDResource( "MaxX" );
   MapMinY = drawing.GetDResource( "MinY" );
   MapMaxY = drawing.GetDResource( "MaxY" );

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
   var stretch = drawing.GetDResource( "Stretch" );   /* double */
   Stretch = ( stretch != 0.0 );
}

//////////////////////////////////////////////////////////////////////////
// Handle user interaction.
//////////////////////////////////////////////////////////////////////////
function InputCallback( /* GlgObject */ vp, /* GlgObject */ message_obj )
{
   var origin = message_obj.GetSResource( "Origin" );
   var format = message_obj.GetSResource( "Format" );
   var action = message_obj.GetSResource( "Action" );

   if( action == "DeleteWindow" )
   {
      if( origin == "SelectionDialog" )
      {
         // Close selection dialog
         Drawing.SetDResource( "SelectionDialog/Visibility", 0.0 );
         Drawing.Update();	 
      }
   }
   else if( format == "Button" )
   {
      if( action != "Activate" )
        return;

      if( origin == "CloseDialog" )
      {
         Drawing.SetDResource( "SelectionDialog/Visibility", 0.0 );
         Drawing.Update();	 
      }
      else if( origin == "ZoomIn" )
        MapViewport.SetZoom( null, 'i', 0.0 );
      else if( origin == "ZoomOut" )
        MapViewport.SetZoom( null, 'o', 0.0 );
      else if( origin == "ZoomReset" )
        MapViewport.SetZoom( null, 'n', 0.0 );
      else if( origin == "ZoomTo" )
      {
         StartDragging =true;
         MapViewport.SetZoom( null, 't', 0.0 );
      }
      else if( origin == "ColorScheme" )
      {
         ColorScheme ^= 1;    // Toggle between 0 and 1
         Drawing.SetDResource( "MapArea/ColorIndex", ColorScheme );
         Drawing.SetDResource( "MapArea/Icon0/Group/ColorIndex", ColorScheme );
         Drawing.Update();
      }
      else if( origin == "Connections" )
      {
         ToggleResource( Drawing, "MapArea/Connections/Visibility" );
      }
      else if( origin == "ValueDisplay" )
      {
         // Visibility of all labels is constrained, set just one.
         ToggleResource( Drawing, "MapArea/Icon0/Group/ValueLabel/Visibility" );
         ToggleResource( Drawing, "MapArea/Link0/ValueLabel/Visibility" );
      }
      else if( origin == "Map" )
      {
         ToggleResource( Drawing, "MapArea/MapGroup/Visibility" );
      }
      else if( origin == "Update" )
      {
         PerformUpdates = !PerformUpdates;
      }	
      else if( origin == "MapType" )  // Toggle US and world map
      {
         if( USMap )
         {
            USMap = false;
            facilities_file = "facilities_w";
            links_file = "links_w";
         }
         else
         {
            USMap = true;
            facilities_file = "facilities_s";
            links_file = "links_s";
         }

         ChangeMap();
         return;
      }	
   }
   /* Process mouse clicks on objects of interests in the drawing: 
      implemented as an Action with the "Node", "Link" or other label 
      attached to an object and activated on a mouse click. 
   */
   else if( format == "CustomEvent" )
   {
      if( MapViewport.GetDResource( "ZoomToMode" ) != 0 )	   
        return;  // Don't handle selection in ZoomTo mode.
      
      var
        label = null,             /* String */
        visibility_name = null,   /* String */
        icon_name;                /* String */
      var has_data = false;       /* boolean */
      var highlight_obj = null;   /* GlgObject */
      var data = 0.0;             /* double  */
         
      var event_label = message_obj.GetSResource( "EventLabel" );  /* String */
      if( event_label == "BackgroundVP" )
      {
         /* The background viewport selection is reported only if there
            are no other selections: erase the highlight.
         */
         Highlight( Drawing, null );
         Drawing.Update();
         return;
      }
      // Process state selection on the US map.
      else if( event_label == "MapSelection" )
      {
         label = "None";
         
         /* The selection is reported for the MapGroup. The OrigObject is
            used to get the object ID of the selected lower level state 
            polygon.
         */
         highlight_obj = message_obj.GetResourceObject( "OrigObject" );
         icon_name = highlight_obj.GetSResource( "Name" );
         has_data = false;
         visibility_name = "MapArea/MapGroup/Visibility";	    
         
         // Location is set to the mouse click by the preceding TraceCallback.
      }
      else if( event_label == "Node" )
      {
         var node = message_obj.GetResourceObject( "Object" );   /* GlgObject */
         icon_name = node.GetSResource( "Name" );
         SelectedObject = node;

         // Query the label of the selected node
         label = node.GetSResource( "Group/Label/String" );
         
         // Query node location.
         var position = node.GetGResource( "Position" );   /* GlgPoint */
         
         // Convert world coordinates to lat/lon
         GetLatLon( position, lat_lon );

         /* Generate a location info string by converting +- sign info 
            into the N/S, E/W suffixes.
         */
         var location_str = CreateLocationString( lat_lon );   /* String */

         // Display position info in the dialog
         Drawing.SetSResource( "SelectionDialog/Location", location_str );
         
         data = node.GetDResource( "Group/Value" );
         has_data = true;
         visibility_name = "MapArea/Icon0/Visibility";
      }
      else if( event_label == "Link" )
      {
         var link = message_obj.GetResourceObject( "Object" );   /* GlgObject */
         icon_name = link.GetSResource( "Name" );
         SelectedObject = link;
         
         label = "None";
         data = link.GetDResource( "Value" );
         has_data = true;
         visibility_name = "MapArea/Connections/Visibility";

         // Location is set to the mouse click by the preceding TraceCallback.
      }
      else
        return;  // No selection
      
      // Check if this layer is visible.
      var visibility_value =            /* double */
        Drawing.GetDResource( visibility_name );

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
           Drawing.SetDResource( "SelectionDialog/Graph/DataGroup/Points/DataSample%/Value", 0.0 );

         Drawing.SetDResource( "SelectionDialog/Visibility", 1.0 );
         Highlight( Drawing, highlight_obj );
      }
   }
   Drawing.Update();
}

//////////////////////////////////////////////////////////////////////////
// Is used to obtain coordinates of the mouse click. 
//////////////////////////////////////////////////////////////////////////
function TraceCallback( /* GlgObject */ viewport, /* GlgTraceData */ trace_info )
{      
   // Use the MapArea events only.
   if( !trace_info.viewport.Equals( MapViewport ) )
     return;

   var event_type = trace_info.event_type;
   switch( event_type )
   {
    case GLG.GlgEventType.TOUCH_START:
      // On mobile devices, enable touch dragging for defining ZoomTo region.
      if( !StartDragging )
         return;
      
      GLG.SetTouchMode();        /* Start dragging via touch events. */
      StartDragging = false;     /* Reset for the next time. */
      /* Fall through */

     case GLG.GlgEventType.MOUSE_PRESSED:
      point.x = trace_info.mouse_x * CoordScale;
      point.y = trace_info.mouse_y * CoordScale;
      point.z = 0;
      
      /* COORD_MAPPING_ADJ is added to the cursor coordinates for precise
         pixel mapping.
      */
      point.x += GLG.COORD_MAPPING_ADJ;
      point.y += GLG.COORD_MAPPING_ADJ;
      break;

    default: return;
   }      

   if( MapViewport.GetDResource( "ZoomToMode" ) != 0 )	   
     return;    // Ignore clicks in zoom mode. 

   viewport.ScreenToWorld( true, point, world_point );

   /* Generate a location info string by converting +- sign info into the
      N/S, E/W suffixes.
   */
   var location_str = CreateLocationString( world_point );   /* String */
   Drawing.SetSResource( "SelectionDialog/Location", location_str );   

   /* Set facility to "None" for now: will be set by the Select callback
      if any selected.
   */
   Drawing.SetSResource( "SelectionDialog/ID", "None" );
   Drawing.SetSResource( "SelectionDialog/Facility", "None" );
   
   // Not an icon or link: no associated data.
   Drawing.SetDResource( "SelectionDialog/DataLabel/Visibility", 0.0 );
      
   Drawing.SetDResource( "SelectionDialog/Visibility", 1.0 );
   Drawing.Update();
}

//////////////////////////////////////////////////////////////////////////
// Highlight or unhighlight selected map polygon.
// Changes the index of the color list transform attached to the object's
// FillColor.
//////////////////////////////////////////////////////////////////////////
function Highlight( /* GlgObject */ viewport, /* GlgObject */ sel_object )
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
      SelectedColorIndex = sel_object.GetResourceObject( "SelectColorIndex" );
      if( SelectedColorIndex != null )
        SelectedColorIndex.SetDResource( null, 1.0 );
   }
}

//////////////////////////////////////////////////////////////////////////
// Converts Lat/Lon to X/Y in GLG world coordinates.
//////////////////////////////////////////////////////////////////////////
function GetXY( /* GlgPoint */ lat_lon, /* GlgPoint */ xy )
{
   GLG.GlmConvert( GLG.GlgProjectionType.RECTANGULAR_PROJECTION, Stretch, 
                   GLG.GlgCoordType.OBJECT_COORD, /* coord_to_lat_lon */ false,
                   MapCenter, MapExtent, 0.0,
                   GlgMinX, GlgMaxX, GlgMinY, GlgMaxY,
                   lat_lon, xy );
}

//////////////////////////////////////////////////////////////////////////
// Converts Lat/Lon to X/Y in GLG world coordinates.
//////////////////////////////////////////////////////////////////////////
function GetLatLon( /* GlgPoint */ xy, /* GlgPoint */ lat_lon )
{ 
   GLG.GlmConvert( GLG.GlgProjectionType.RECTANGULAR_PROJECTION, Stretch, 
                   GLG.GlgCoordType.OBJECT_COORD, /* coord_to_lat_lon */ true,
                   MapCenter, MapExtent, 0.0,
                   GlgMinX, GlgMaxX, GlgMinY, GlgMaxY,
                   xy, lat_lon );
}

//////////////////////////////////////////////////////////////////////////
// Generate a location info string by converting +- sign info into the
// N/S, E/W suffixes, and decimal fraction to deg, min, sec.
//////////////////////////////////////////////////////////////////////////
function CreateLocationString( /* GlgPoint */ point )   /* String */
{
   var x_deg, y_deg, x_min, y_min, x_sec, y_sec;  /* int */
   var char_x, char_y;  /* String */
   var lat, lon;        /* double */

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
   
   x_deg = Math.trunc( lon );
   x_min = Math.trunc( ( lon - x_deg ) * 60.0 );
   x_sec = Math.trunc( ( lon - x_deg - x_min / 60.0 ) * 3600.0 );
   
   y_deg = Math.trunc( lat );
   y_min = Math.trunc( ( lat - y_deg ) * 60.0 );
   y_sec = Math.trunc( ( lat - y_deg - y_min / 60.0 ) * 3600.0 );
   
   var location_string =
     "Lon=" + x_deg + "\u00B0" + 
          GLG.PrintfI( "%02d", x_min ) + "'" + 
          GLG.PrintfI( "%02d", x_sec ) + '"' + char_x +
     "  Lat=" + y_deg + "\u00B0" + 
          GLG.PrintfI( "%02d", y_min ) + "'" + 
          GLG.PrintfI( "%02d", y_sec ) + '"' + char_y;
   return location_string;
}

//////////////////////////////////////////////////////////////////////////
// Update display with data.
//////////////////////////////////////////////////////////////////////////
function UpdateMap()
{
   var i, size;   /* int */
   var value;     /* double */
   var res_name;  /* String */

   if( timer == null )
     return;   // Prevents race conditions
   
   if( PerformUpdates )
   {
      if( ShowFlow )
      {
         if( flow_display_obj == null )   // First time.
           flow_display_obj =
             Drawing.GetResourceObject( "MapArea/Link0/Line/LineType" );
         
         /* Links's flow is constrained: animating one animates all. 
            Flow direction is defined by the order of the links points when
            constrained.
         */
         if( flow_display_obj != null )
         {
            // Query the current line type and offset
            var flow_data = flow_display_obj.GetDResource( null );  /* double */
            var line_type = Math.trunc( flow_data ) % 32;   /* int */
            var offset = Math.trunc( flow_data ) / 32;      /* int */
               
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
         value = GLG.Rand( 30.0, 500.0 );
         res_name = "MapArea/Icon" + i;
         var icon = Drawing.GetResourceObject( res_name );   /* GlgObject */
         icon.SetDResource( "Group/Value", value );
            
         // Update selected object data display in the SelectionDialog.
         if( icon.Equals( SelectedObject ) )
         {
            Drawing.SetDResource( "SelectionDialog/Data", value );
            Drawing.SetDResource( "SelectionDialog/Graph/DataGroup/EntryPoint",
                                  value / 500.0 );
            // To scroll ticks.
            Drawing.SetSResource( "SelectionDialog/Graph/XMajorGroup/TicksEntryPoint", "" );
         }
         
         if( ( counter % UpdateN ) == 0 ) // Update icon type every n-th time.
         {
            if( GLG.Rand( 0.0, 10.0 ) > 2.0 )
            {
               var icon_type =  /* double */
                 icon.GetDResource( "Group/Graphics/IconType" );
               
               if( icon_type != 0.0 )
                 icon_type = 0.0;
               else		 
                 icon_type = GLG.Rand( 0.0, 6.0 );
               
               icon.SetDResource( "Group/Graphics/IconType", icon_type );
            }
         }
      }
         
      if( ( counter % UpdateN ) == 0 )  // Update link values every n-th time.
      {
         size = LinksGroup.GetSize();
         for( i=0; i<size; ++i )
         {      
            value = GLG.Rand( 1.0, 9.0 );
            res_name = "MapArea/Link" + i;
            var link = Drawing.GetResourceObject( res_name );  /* GlgObject */
            link.SetDResource( "Value", value );
            if( ShowFlow )
              link.SetDResource( "Line/LineWidth", value );
            link.SetDResource( "Line/LineColorIndex", Math.trunc( value ) / 2 );
               
            // Update selected object data display in the SelectionDialog.
            if( link.Equals( SelectedObject ) )
            {
               Drawing.SetDResource( "SelectionDialog/Data", value );
               Drawing.SetDResource( "SelectionDialog/Graph/DataGroup/EntryPoint", 
                                     value / 10.0 );
               // To scroll ticks.
               Drawing.SetSResource( "SelectionDialog/Graph/XMajorGroup/TicksEntryPoint", "" );
            }
         }
      }
      
      ++counter;
      if( counter > 1000 )
        counter = 0;
      
      Drawing.Update();
   }
   
   timer = setTimeout( UpdateMap, UPDATE_INTERVAL );   // Restart update timer
}

//////////////////////////////////////////////////////////////////////////
// Toggle resource between 0 and 1.
//////////////////////////////////////////////////////////////////////////
function ToggleResource( /* GlgObject */ glg_object, /* String */ res_name )
{
   var value = glg_object.GetDResource( res_name );   /* double */
   glg_object.SetDResource( res_name, value != 0.0 ? 0.0 : 1.0 );
}

//////////////////////////////////////////////////////////////////////////////
// Loads assets required by the application and invokes the specified
// callback when done.
//////////////////////////////////////////////////////////////////////////////
function LoadAssets( callback )
{
    /* HTML5 doesn't provide a scrollbar input element (only a range input 
       html element is available). This application needs to load GLG scrollbars
       used for integrated chart scrolling. For each loaded scrollbar, the 
       AssetLoaded callback is invoked with the supplied data.
    */
    GLG.LoadWidgetFromURL( "scrollbar_h.g", null, AssetLoaded,
                           { name: "scrollbar_h", callback: callback } );
    GLG.LoadWidgetFromURL( "scrollbar_v.g", null, AssetLoaded,
                           { name: "scrollbar_v", callback: callback } );

    GLG.LoadObjectFromURL( "palette.g", null, AssetLoaded,
                           { name: "palette", callback: callback } );

    GLG.LoadAsset( "us_map.g", GLG.GlgHTTPRequestResponseType.GLG_DRAWING,
                   AssetLoaded, { name: "us_map", callback: callback } );
    GLG.LoadAsset( "facilities_s", GLG.GlgHTTPRequestResponseType.TEXT,
                   AssetLoaded, { name: "facilities_s", callback: callback } );
    GLG.LoadAsset( "links_s", GLG.GlgHTTPRequestResponseType.TEXT,
                   AssetLoaded, { name: "links_s", callback: callback } );

    GLG.LoadAsset( "world_map.g", GLG.GlgHTTPRequestResponseType.GLG_DRAWING,
                   AssetLoaded, { name: "world_map", callback: callback } );
    GLG.LoadAsset( "facilities_w", GLG.GlgHTTPRequestResponseType.TEXT,
                   AssetLoaded, { name: "facilities_w", callback: callback } );
    GLG.LoadAsset( "links_w", GLG.GlgHTTPRequestResponseType.TEXT,
                   AssetLoaded, { name: "links_w", callback: callback } );
}

//////////////////////////////////////////////////////////////////////////////
function AssetLoaded( loaded_data, user_data, path )
{
   switch( user_data.name )
   {
    case "scrollbar_h":
      if( loaded_data != null )    /* GlgObject */
        loaded_data.SetResourceObject( "$config/GlgHScrollbar", loaded_data );
      break;

    case "scrollbar_v":
      if( loaded_data != null )    /* GlgObject */
        loaded_data.SetResourceObject( "$config/GlgVScrollbar", loaded_data );
      break;

    case "palette":
      LoadCheck( loaded_data, user_data.name );
      Palette = loaded_data;       /* GlgObject */
      break;

    case "us_map":
      LoadCheck( loaded_data, user_data.name );
      USMapData = loaded_data;        /* Raw data */
      break;

    case "world_map":
      LoadCheck( loaded_data, user_data.name );
      WorldMapData = loaded_data;     /* Raw data */
      break;

    case "facilities_s":
      LoadCheck( loaded_data, user_data.name );
      USFacilities = loaded_data;     /* Uint8Array */
      break;

    case "facilities_w":
      LoadCheck( loaded_data, user_data.name );
      WorldFacilities = loaded_data;  /* Uint8Array */
      break;

    case "links_s":
      LoadCheck( loaded_data, user_data.name );
      USLinks = loaded_data;          /* Uint8Array */
      break;

    case "links_w":
      LoadCheck( loaded_data, user_data.name );
      WorldLinks = loaded_data;       /* Uint8Array */
      break;
      
    default:
      console.error( "Unexpected asset name" );
      break;
   }
   
   /* Define an internal variable to keep the number of loaded assets. */
   if( AssetLoaded.num_loaded == undefined )
     AssetLoaded.num_loaded = 1;
   else
     ++AssetLoaded.num_loaded;

   // Invoke the callback after all assets have been loaded.
   if( AssetLoaded.num_loaded == 9 )
     user_data.callback();
}

//////////////////////////////////////////////////////////////////////////////
function LoadCheck( loaded_data, asset_name )
{
   if( FirstLoadError && loaded_data == null )
   {
      FirstLoadError = false;
      alert( "Can't load " + asset_name +
             " asset, check console message for details." );
   }
}

//////////////////////////////////////////////////////////////////////////////
function AdjustForMobileDevices()
{
   if( CoordScale == 1.0 )
     return;   /* Desktop version. */

   // Increase toolbar size for mobile devices.
   Drawing.SetDResource( "ToolbarOffsetX", 2000 );
   Drawing.SetDResource( "ToolbarOffsetY", -215 );
}

//////////////////////////////////////////////////////////////////////////////
// Changes drawing size while maintaining width/height aspect ratio.
//////////////////////////////////////////////////////////////////////////////
function SetDrawingSize( next_size )
{
   const ASPECT_RATIO = 800 / 625;

   // Settings for desktop displays.
   const MIN_WIDTH = 600;
   const MAX_WIDTH = 1000;
   const SCROLLBAR_WIDTH = 15;
   
   if( SetDrawingSize.size_index == undefined )   // first time
   {
      SetDrawingSize.size_index = 0;

      SetDrawingSize.small_sizes       = [ 1, 1.5,  2.,   2.5 ];
      SetDrawingSize.medium_sizes      = [ 1, 0.75, 1.25, 1.5 ];
      SetDrawingSize.large_sizes       = [ 1, 0.6,  1.25, 1.5 ];
      SetDrawingSize.num_sizes = SetDrawingSize.small_sizes.length;
      SetDrawingSize.is_mobile = ( screen.width <= 760 );

      window.addEventListener( "resize", ()=>{ SetDrawingSize( false ) } );
   }
   else if( next_size )
   {
      ++SetDrawingSize.size_index;
      SetDrawingSize.size_index %= SetDrawingSize.num_sizes;
   }

   var drawing_area = document.getElementById( "glg_area" );
   if( SetDrawingSize.is_mobile )
   {
      /* Mobile devices use constant device-width, adjust only the height 
         of the drawing to keep the aspect ratio.
      */
      drawing_area.style.height =
        "" + Math.trunc( drawing_area.clientWidth / ASPECT_RATIO ) + "px";
   }
   else   /* Desktop */
   {
      var span = document.body.clientWidth; 
      if( !SetDrawingSize.is_mobile )
        span -= SCROLLBAR_WIDTH;

      var start_width;
      if( span < MIN_WIDTH )
        start_width = MIN_WIDTH;
      else if( span > MAX_WIDTH )
        start_width = MAX_WIDTH;
      else
        start_width = span;

      var size_array;
      if( span < 600 )
        size_array = SetDrawingSize.small_sizes;
      else if( span < 800 )
        size_array = SetDrawingSize.medium_sizes;
      else
        size_array = SetDrawingSize.large_sizes;

      var size_coeff = size_array[ SetDrawingSize.size_index ];
      var width = Math.trunc( Math.max( start_width * size_coeff, MIN_WIDTH ) );
   
      drawing_area.style.width = "" + width + "px";
      drawing_area.style.height = "" + Math.trunc( width / ASPECT_RATIO ) + "px";
   }
}

//////////////////////////////////////////////////////////////////////////////
// Increases canvas resolution for mobile devices with HiDPI displays.
// Returns chosen coordinate scale factor.
//////////////////////////////////////////////////////////////////////////////
function SetCanvasResolution()
{
   // Set canvas resolution only for mobile devices with devicePixelRatio != 1.
   if( window.devicePixelRatio == 1. || !SetDrawingSize.is_mobile )
     return 1.0;   // Use coord scale = 1.0 for desktop.
   
   /* The first parameter defines canvas coordinate scaling with values 
      between 1 and devicePixelRatio. Values greater than 1 increase 
      canvas resolution and result in sharper rendering. The value of 
      devicePixelRatio may be used for very crisp rendering with very thin lines.

      Canvas scale > 1 makes text smaller, and the second parameter defines
      the text scaling factor used to increase text size.

      The third parameter defines the scaling factor that is used to
      scale down text in native widgets (such as native buttons, toggles, etc.)
      to match the scale of the drawing.
   */
   var coord_scale = 2.0;
   GLG.SetCanvasScale( coord_scale, 1.5, 0.6 );
   
   // Mobile devices use fixed device-width: disable Change Drawing Size button.
   var change_size_button = document.getElementById( "change_size" );
   if( change_size_button != null )
     change_size_button.parentNode.removeChild( change_size_button );

   return coord_scale;      // Chosen coord scale for mobile devices.
}

var SPACE_CHAR = ' '.charCodeAt( 0 );
var TAB_CHAR   = '\t'.charCodeAt( 0 );
var TAB_CHAR   = '\t'.charCodeAt( 0 );
var NL_CHAR    = '\n'.charCodeAt( 0 );
var CR_CHAR    = '\r'.charCodeAt( 0 );

/////////////////////////////////////////////////////////////////////
// Reads a string into a buffer, returns string length
// or 0 if no string was read.
// Automatically consumes trailing white space.
/////////////////////////////////////////////////////////////////////
function ReadSimpleString( /* InputStream */ stream )  /* int */
{
   var next_char;    /* String */
      
   buffer_length = 0;
   buffer[ 0 ] = SPACE_CHAR;
      
   // Skip white places
   while( buffer[ 0 ] == SPACE_CHAR ||
          buffer[ 0 ] == TAB_CHAR ||
          buffer[ 0 ] == NL_CHAR ||
          buffer[ 0 ] == CR_CHAR )
   {
      next_char = stream.ReadChar();
      if( next_char == null )
        return 0;
      
      buffer[ 0 ] = next_char;
   }

   var offset = 0;       /* int */
   while( buffer[ offset ] != SPACE_CHAR &&
          buffer[ offset ] != TAB_CHAR &&
          buffer[ offset ] != NL_CHAR &&
          buffer[ offset ] != CR_CHAR )
   {
      ++offset;
      if( offset == buffer_length )   // Increase buffer size
      {	 
         var new_buffer = new Uint16Array( buffer_length * 2 );
         for( var i=0; i<buffer_length; ++i )	   
           new_buffer[i] = buffer[i];
         buffer = new_buffer;
         buffer_length *= 2;
      }
      
      next_char = stream.ReadChar();
      if( next_char == null )
        return offset;   // The last char may have no space after it: accept
      
      buffer[ offset ] = next_char;
   }
   return offset;
}

/////////////////////////////////////////////////////////////////////
// Returns null if no string was read.
/////////////////////////////////////////////////////////////////////
function ReadString( /* InputStream */ stream )   /* String */
{
   var length = ReadSimpleString( stream );   /* Int */
   if( length == 0 )
     return null;

   var xxx = buffer.slice( 0, length );

   return String.fromCharCode.apply( null, buffer.slice( 0, length ) );
}

/////////////////////////////////////////////////////////////////////
// Reads name (that may include spaces) until the ":" terminator.
// Returns null if no name was read.
/////////////////////////////////////////////////////////////////////
function ReadName( /* InputStream */ stream )   /* String */
{
   var name = null;    /* String */

   while( true )
   {
      var name_part = ReadString( stream );    /* String */
      if( name_part == null || name_part == ":" )
        return name;
      
      if( name == null )
        name = name_part;
      else
        name = name + " " + name_part;
   }
}

/////////////////////////////////////////////////////////////////////
// Returns null if no character was read.
/////////////////////////////////////////////////////////////////////
function ReadChar( /* InputStream */ stream )   /* String */
{
   var ch;   /* String */

   while( true )
   {
      ch = stream.ReadChar();
      if( ch == null )
        return null;
      
      if( ch != SPACE_CHAR &&
          ch != TAB_CHAR &&
          ch != NL_CHAR &&
          ch != CR_CHAR )
        break;
   }
   return String.fromCharCode( ch );
}

/////////////////////////////////////////////////////////////////////
// Returns null if no integer was read.
/////////////////////////////////////////////////////////////////////
function ReadInt( /* InputStream */ stream )   /* int */
{
   var string = ReadString( stream );
   if( string == null )
     return null;

   var value = parseInt( string );
   if( isNaN( value ) )
     return null;

   return value;
}

/////////////////////////////////////////////////////////////////////
// Returns null if no double value was read.
/////////////////////////////////////////////////////////////////////
function ReadDouble( /* InputStream */ stream )   /* double */
{
   var string = ReadString( stream );
   if( string == null )
     return null;

   var value = parseFloat( string );
   if( isNaN( value ) )
     return null;

   return value;
}

////////////////////////////////////////////////////////////////////////
function InputStream( /* String */ data )
{
   this.length = data.length;                    /* int */
   this.curr_index = 0;                          /* int */
   this.has_more = ( this.length > 0 );          /* boolean */

   this.array = new Uint16Array( this.length );  /* Uint16Array */
   for( var i=0; i<this.length; ++i )
     this.array[ i ] = data.charCodeAt( i );
}

////////////////////////////////////////////////////////////////////////
// Returns null on end of stream.
////////////////////////////////////////////////////////////////////////
InputStream.prototype.ReadChar = function()    /* String */
{
   if( this.curr_index >= this.length )
     return null;

   var data = this.array[ this.curr_index ];
   ++this.curr_index;
   this.has_more = ( this.curr_index < this.length );
   return data;
}

