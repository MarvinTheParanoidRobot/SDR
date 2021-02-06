#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#ifdef _WINDOWS
#include "resource.h"
# pragma warning( disable : 4244 )
# pragma warning( disable : 4996 )    /* Allow cross-platform fscanf(), etc. */
#endif
#include "GlgApi.h"
#include "map_proto.h"

typedef enum
{
   UNKNOWN_EVENT = 0,
   BUTTON_PRESS,
} EventType;

GlgObject
   Drawing = (GlgObject)0,
   /* Viewport used as a parent of a loaded map viewport. */
   MapContainer = (GlgObject)0,
   IconArray = (GlgObject)0,
   LinkTemplate = (GlgObject)0,
   FacilitiesGroup = (GlgObject)0,
   LinksGroup = (GlgObject)0,
   SelectedColorIndex = (GlgObject)0,
   SelectedObject = (GlgObject)0,
   flow_display_obj = (GlgObject)0,
   MapViewport = (GlgObject)0;
double
   IconScale = 0.6,   /* Icon size */
   GlgMinX, GlgMaxX, GlgMinY, GlgMaxY,  /* GLG extent in world coordinates */
   MapMinX, MapMaxX, MapMinY, MapMaxY;  /* Map extent in degrees, mapped to 
					   the GLG extent. */
GlgPoint    /* Calculated values */
   MapCenter,
   MapExtent;

long
   UpdateInterval = 100,  /* Update interval in msec */
   ColorScheme = 0,
   DoUpdate = True,  /* Controls dynamic update. */
   ShowFlow = True,  /* Animate flow with "moving ants". */
   UpdateN = 20,     /* Update on every N-th iteration of the simulated data */
   Stretch;
char   
   * drawing_file = "map_demo.g",
   * map_file = "us_map.g",
   * facilities_file = "facilities_s",
   * links_file = "links_s",
   * palette_file = "palette.g",
   * exe_path = NULL;
   
GlgAppContext AppContext;   /* Global, used to install a timeout. */

#include "GlgMain.h"

/*----------------------------------------------------------------------
| Optional arguments
|    
| Options: (before args)
| -drawing drawing_file   Main drawing (map_demo.g).
| -map map_file           Map drawing to load (us_map.g or world_map.g).
| -facilities data_file   Facilities data file, lists the nodes to add 
|                         to the map drawing.
| -links data_file        Link data file, lists the links between nodes
|                         to be added.
| -palette palette_file   Palette drawing of facility icons
| -icon_scale scale       Scale factor for the the added icons (set via
|                         the icon's "IconScale" resource).
| -no_flow                Disable flow animation with "moving ants"
*/
int GlgMain( argc, argv, app_context )
     int argc;
     char *argv[];
     GlgAppContext app_context;
{
   long skip;
   char * full_path;

   AppContext = GlgInit( False, app_context, argc, argv );

   GlgSetDResource( (GlgObject)0, "$config/GlgMouseTooltipTimeout", 0.2 );

   /* Don't expand selection area for exact state tooltips. */
   GlgSetDResource( (GlgObject)0, "$config/GlgPickResolution", 0. );

   /* Scan options */
   for( skip = 1; skip < argc; ++skip )
   {
      if( strcmp( argv[ skip ], "-drawing" ) == 0 )
      {
	 ++skip;
	 if( argc <= skip )
	   error( "No drawing file.", True );
	 drawing_file = argv[ skip ];
      }
      else if( strcmp( argv[ skip ], "-map" ) == 0 )
      {
	 ++skip;
	 if( argc <= skip )
	   error( "No map file.", True );
	 map_file = argv[ skip ];
      }
      else if( strcmp( argv[ skip ], "-facilities" ) == 0 )
      {
	 ++skip;
	 if( argc <= skip )
	   error( "No facilities file.", True );
	 facilities_file = argv[ skip ];
      }
      else if( strcmp( argv[ skip ], "-links" ) == 0 )
      {
	 ++skip;
	 if( argc <= skip )
	   error( "No links file.", True );
	 links_file = argv[ skip ];
      }
      else if( strcmp( argv[ skip ], "-palette" ) == 0 )
      {
	 ++skip;
	 if( argc <= skip )
	   error( "No icon palette file.", True );
	 palette_file = argv[ skip ];
      }
      else if( strcmp( argv[ skip ], "-no_flow" ) == 0 )
	ShowFlow = False;
      else if( strcmp( argv[ skip ], "-icon_scale" ) == 0 )
      {
	 ++skip;
	 if( argc <= skip || sscanf( argv[ skip ], "%lf", &IconScale ) != 1 )
	   error( "No icon scale.", True );
	 printf( "Using icon scale = %lf\n", IconScale );
      }
      else if( strcmp( argv[ skip ], "-verbose" ) == 0 ||
	      strcmp( argv[ skip ], "-non_verbose" ) == 0 ||
	      strcmp( argv[ skip ], "-glg-enable-opengl" ) == 0 ||
	      strcmp( argv[ skip ], "-glg-disable-opengl" ) == 0 )
	;   /* Allow: handled by GLG. */
      else
	break;
   }

   exe_path = argv[0];   /* Store exe path */

   /* Load main drawing. */
   full_path = GlgGetRelativePath( exe_path, drawing_file );

   Drawing = GlgLoadWidgetFromFile( full_path );
   if( !Drawing )
     error( "Can't load drawing file.", True );

   GlgFree( full_path );

   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( Drawing, "Point1", -700., -700., 0. );
   GlgSetGResource( Drawing, "Point2",  700.,  700., 0. );

   /* Set window title. */
   GlgSetSResource( Drawing, "ScreenName", "GLG Supply Chain Demo" );

   /* Make Selection Dialog a floating dialog, adjust its height and vertical 
      placement, and set its title.
   */
   GlgSetDResource( Drawing, "SelectionDialog/ShellType",
                    (double) GLG_DIALOG_SHELL );
   GlgSetDResource( Drawing, "SelectionDialog/DialogHeight", 140. );
   GlgSetDResource( Drawing, "SelectionDialog/DialogY", -700. );
   GlgSetSResource( Drawing, "SelectionDialog/Screen/ScreenName",
                    "Selection Information" );
   
   MapContainer = GlgGetResourceObject( Drawing, "MapContainer" );
   if( !MapContainer )
     error( "Can't find MapContainer viewport.", True );
   
   LoadMap();

   GlgAddCallback( Drawing, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );
   GlgAddCallback( Drawing, GLG_TRACE_CB, (GlgCallbackProc)Trace, NULL );

   GlgInitialDraw( Drawing );

#ifdef _WINDOWS            
   GlgLoadExeIcon( Drawing, IDI_ICON1 );
#endif

   StartUpdate( Drawing );   /* Install a timeout. */

   return (int) GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| 
*/
void LoadMap()
{
   char * full_path;

   /* Erase SelectionDialog if displayed. */
   GlgSetDResource( Drawing, "SelectionDialog/Visibility", 0. );

   /* Load the map viewport */

   full_path = GlgGetRelativePath( exe_path, map_file );
   MapViewport = GlgLoadWidgetFromFile( full_path ); 
   if( !MapViewport )
     error( "Can't load map viewport.", True );
      
   GlgFree( full_path );

   /* Set viewport name. */
   GlgSetSResource( MapViewport, "Name", "MapArea" );

   /* Set control points of the map viewport to fill the whole area of the
      MapContainer viewport.
    */
   GlgSetGResource( MapViewport, "Point1", -1000., -1000., 0. );
   GlgSetGResource( MapViewport, "Point2",  1000.,  1000., 0. );

   /* Query extent info from the map. */
   GetExtentInfo( MapViewport );

   /*  Extract node icons from the palette. */
   IconArray = ReadPalette( palette_file );

   /* Read facilities info and creates facilities group (used as a layer). */
   FacilitiesGroup = ReadFacilities( facilities_file );

   if( FacilitiesGroup )
   {
      /* Create connection links (creates a group used as a layer) */
      LinksGroup = ConnectFacilities( links_file );

      if( LinksGroup )   /* Add link group to the drawing */
        GlgAddObjectToBottom( MapViewport, LinksGroup ); 

      /* Add facilities last to be in front of links. */
      GlgAddObjectToBottom( MapViewport, FacilitiesGroup ); 

      /* Set the icon size of the facility nodes */
      SetIconSize();
   }
      
   /* Add map viewport to the drawing after adding icons. */
   GlgAddObjectToTop( MapContainer, MapViewport );

   /* Uncomment the next line to save generated drawing. */
   /* GlgSaveObject( Drawing, "out.g" ); */

   /* Set initial visibility of the value display labels to on. */
   GlgSetDResource( Drawing, "MapArea/Icon0/Group/ValueLabel/Visibility", 1. );
   GlgSetDResource( Drawing, "MapArea/Link0/ValueLabel/Visibility", 1. );

   /* Set initial color sheme. */
   GlgSetDResource( Drawing, "MapArea/ColorIndex", (double) ColorScheme );
   GlgSetDResource( Drawing, "MapArea/Icon0/Group/ColorIndex",
		   (double) ColorScheme );
}

/*----------------------------------------------------------------------
| Dereferences all stored objects and deletes the map.
*/
void UnloadMap()
{
   GlgDropObject( FacilitiesGroup );
   FacilitiesGroup = (GlgObject)0;

   GlgDropObject( LinksGroup );
   LinksGroup = (GlgObject)0;
   
   GlgDropObject( LinkTemplate );
   LinkTemplate = (GlgObject)0;

   GlgDropObject( IconArray );
   IconArray = (GlgObject)0;
   
   flow_display_obj = (GlgObject)0;
   SelectedColorIndex = (GlgObject)0;
   SelectedObject = (GlgObject)0;

   /* Delete the old map drawing */
   GlgDeleteThisObject( MapContainer, MapViewport );

   GlgDropObject( MapViewport );
   MapViewport = (GlgObject)0;
}   

/*----------------------------------------------------------------------
|
*/
void SetIconSize()
{
   /* Adjust icon size by a specified factor. */
   if( IconScale > 0. )
     GlgSetDResource( MapViewport, "Icon0/Template/IconScale", IconScale );
}

/*----------------------------------------------------------------------
| Reads icon palette and returns an array of icons.
*/
GlgObject ReadPalette( char * palette_file )
{
   GlgObject
     palette_drawing,
     palette,
     icon,
     icon_array = (GlgObject)0;
   int i;
   char
     * full_path,
     * icon_name;

   if( !palette_file )
     return (GlgObject)0;    /* No palette specified: don't generate icons. */

   /* Load palette */
   full_path = GlgGetRelativePath( exe_path, palette_file );

   palette_drawing = GlgLoadObject( full_path );
   if( !palette_drawing )
     error( "Can't load icon palette.", True );

   GlgFree( full_path );
   
   /* Get palette object named "Icons" */
   palette = GlgGetResourceObject( palette_drawing, "Icons" );
   if( !palette )
     error( "Can't find palette object.", True );

   /* Find link template and store it for creating links. */
   LinkTemplate = GlgGetResourceObject( palette, "Link" );
   GlgReferenceObject( LinkTemplate );
   if( !ShowFlow )   /* Set to solid line for Win95 */
     GlgSetDResource( LinkTemplate, "Line/LineType", 0. );

   for( i=0; ; ++i )
   {
      /* Generate a sequential icon name */
      icon_name = GlgCreateIndexedName( "Icon", i );
      icon = GlgGetResourceObject( palette, icon_name );
      GlgFree( icon_name );

      if( icon )
      {
	 if( !icon_array )    /* First time: create icon array */
	   icon_array = 
	     GlgCreateObject( GLG_GROUP, NULL, NULL, NULL, NULL, NULL );
	 
	 GlgAddObjectToBottom( icon_array, icon );
      }
      else
      {
	 if( !i )     /* First time: can't find any icons! */ 
	   error( "Can't find facilities icons.", True );
	 
	 /* Done reading icons. */
	 printf( "Scanned %ld icons\n", (long) GlgGetSize( icon_array ) );
	 break;
      }
   }
   
   GlgDropObject( palette_drawing );
   return icon_array;
}

/*----------------------------------------------------------------------
| Reads facilities data file and generates an array of facility objects
| using the facility palette. 
*/
GlgObject ReadFacilities( char * facilities_filename )
{
   GlgObject
     facility_group = (GlgObject)0,
     icon;
   FILE * facilities_file = NULL;
   char
     x_char,
     y_char,
     facility_name[ 1002 ],
     * full_path,
     * icon_name;
   double x, y;
   long
     num_read,
     num_icons,
     num_facilities;

   if( !facilities_filename )
     return (GlgObject)0;      /* No facilities file: already on the map? */

   if( !IconArray )
   {
      error( "No icon palette.", True );
      return (GlgObject)0;
   }

   full_path = GlgGetRelativePath( exe_path, facilities_filename );

   facilities_file = fopen( full_path, "r" );
   if( !facilities_file )
     error( "Can't open facilities file.", True );

   GlgFree( full_path );

   num_icons = (long) GlgGetSize( IconArray );
   num_facilities = 0;
   while( 1 )
   {
      /* Read the facilities record */
      num_read = 
	fscanf( facilities_file, "%1001[^:]:%lf %c%lf %c\n",
	       facility_name, &y, &y_char, &x, &x_char );
      
      if( num_read != 5 )
	if( num_read <= 0 && feof( facilities_file ) )
	  break;
	else
	  error( "Syntax error reading facilities file.", True );
      
      ++num_facilities;

#if GLG_DEBUG
      printf( "%ld %s\n", num_facilities, facility_name );
#endif

      if( x_char == 'W' ||  x_char == 'w' )
	x = 180. + ( 180. - x );
      if( y_char == 'S' ||  x_char == 's' )
	y = -y;
            
      if( !facility_group )   /* First time: create. */
	facility_group =
	  GlgCreateObject( GLG_GROUP, "Facilities", NULL, NULL, NULL, NULL );

      /* Add named icon */
      icon_name = GlgCreateIndexedName( "Icon", num_facilities - 1 );
      icon = AddNode( facility_group, icon_name, x, y );
      GlgFree( icon_name );

      /* Set facility display label and value. */
      GlgSetSResource( icon, "Template/Label/String", facility_name );
      GlgSetDResource( icon, "Template/Value", GlgRand( 10., 300. ) );
      GlgSetSResource( icon, "TooltipString", facility_name );	
   }

   if( facilities_file )
     fclose( facilities_file );

   printf( "Scanned %ld facilities\n", num_facilities );
   return facility_group;
}     

/*----------------------------------------------------------------------
|
*/
GlgObject AddNode( GlgObject container, char * obj_name, 
                  double lon, double lat )
{
   GlgObject icon;
   GlgPoint lat_lon, xy;

   /* Always use the first icon. Subdrawing dynamics is used to change
      shapes. */
   icon = GlgGetElement( IconArray, 0 );

   /* Create a copy of it. */
   icon = GlgCloneObject( icon, GLG_STRONG_CLONE );

   /* Set object name. */
   GlgSetSResource( icon, "Name", obj_name );	

   /* Convert lat/lon to XY */
   lat_lon.x = lon;
   lat_lon.y = lat;
   lat_lon.z = 0.;
   GetXY( &lat_lon, &xy );

   /* Set node position */
   GlgSetGResource( icon, "Position", xy.x, xy.y, 0. );      
	 
   /* Add the node to the group */
   GlgAddObjectToBottom( container, icon );
   GlgDropObject( icon );
   return icon;
}

/*----------------------------------------------------------------------
| Reads connectivity info from a data file and creates links to connect
| facilities.
*/
GlgObject ConnectFacilities( char * links_filename )
{
   GlgObject
     link_group = (GlgObject)0,
     link;
   FILE * links_file;
   double
     color,
     flow;
   long 
     size,
     num_read,
     from_node,
     to_node,
     num_links;
   char 
     * full_path,
     * link_name;
   
   if( !links_filename || !FacilitiesGroup )
     return (GlgObject)0;   

   if( !LinkTemplate )
     error( "Can't find link template.", True );

   full_path = GlgGetRelativePath( exe_path, links_filename );

   links_file = fopen( full_path, "r" );
   if( !links_file )
     error( "Can't open links file.", True );

   GlgFree( full_path );

   size = (long) GlgGetSize( FacilitiesGroup );
   num_links = 0;
   while( 1 )
   {
      /* Read the link record */
      num_read = fscanf( links_file, "%ld%ld\n", &from_node, &to_node );
      
      if( num_read != 2 )
	if( num_read <= 0 && feof( links_file ) )
	  break;
	else
	  error( "Syntax error reading links file.", True );

      ++num_links;
      
#if GLG_DEBUG
      printf( "Link= %ld:%ld\n", from_node, to_node );
#endif

      if( from_node < 0 || to_node < 0 || 
	 from_node >= size || to_node >= size )
	error( "Invalid link index.", True );

      if( !link_group )   /* First time: create. */
	link_group = 
	  GlgCreateObject( GLG_GROUP, "Connections", NULL, NULL, NULL, NULL );
      
      link_name = GlgCreateIndexedName( "Link", num_links - 1 );
      link = AddLink( link_group, from_node, to_node, link_name );
      GlgFree( link_name );

      /* Set flow attribute and color. */
      flow = GlgRand( 1., 9. );
      color = ( ( (int) flow ) / 2 );
      GlgSetDResource( link, "Line/LineWidth", flow );
      GlgSetDResource( link, "Line/LineColorIndex", color );
      GlgSetDResource( link, "Value", (double)flow );
   }

   printf( "Scanned %ld links\n", num_links );
   return link_group;
}

/*----------------------------------------------------------------------
|
*/
GlgObject AddLink( GlgObject container, int from_node, int to_node, 
		  char * name )
{
   GlgObject
     link,
     link_polygon,
     xform_pt_array,
     icon,
     icon_point,
     link_point,
     xform_point;

   /* Create an instance of the link template (polygon and label). */
   link = GlgCloneObject( LinkTemplate, GLG_STRONG_CLONE );
   GlgSetSResource( link, "Name", name );
   GlgSetSResource( link, "TooltipString", name );	

   /* Constrain the end points to facility nodes. Constrain the link
      polygon end points, and also the point of the path xform used
      to keep the label in the middle of the link.
      */
   link_polygon = GlgGetResourceObject( link, "Line" );
   xform_pt_array = GlgGetResourceObject( link, "PathXform/XformAttr1" );

   /* First point. */
   icon = GlgGetElement( FacilitiesGroup, from_node );
   icon_point = GlgGetResourceObject( icon, "Point" );
   link_point = GlgGetElement( link_polygon, 0 );
   xform_point = GlgGetElement( xform_pt_array, 0 );
   GlgConstrainObject( link_point, icon_point );
   GlgConstrainObject( xform_point, icon_point );

   /* Second point. */
   icon = GlgGetElement( FacilitiesGroup, to_node );
   icon_point = GlgGetResourceObject( icon, "Point" );
   link_point = GlgGetElement( link_polygon, 1 );
   xform_point = GlgGetElement( xform_pt_array, 1 );
   GlgConstrainObject( link_point, icon_point );
   GlgConstrainObject( xform_point, icon_point );
   
   GlgAddObjectToBottom( container, link );
   GlgDropObject( link );
   return link;
}

/*----------------------------------------------------------------------
| Query the extend info from the generated map. The map drawing has named 
| custom properties attached to it which keep the extent information. The
| map was generated in such a way that map's extent in lat/lon degrees 
| (MapMinX, MapMinY, MapMaxX and MapMaxY) was mapped to the full GLG extent
| of +-1000. This information is used later to convert from lat/lon to x/y
| and vice versa.
*/
void GetExtentInfo( GlgObject drawing )
{
   double stretch;

   /* Query map extent from the loaded map drawing (kept as named custom 
      properties attached to the drawing). */
   GlgGetDResource( drawing, "MinX", &MapMinX );
   GlgGetDResource( drawing, "MaxX", &MapMaxX );
   GlgGetDResource( drawing, "MinY", &MapMinY );
   GlgGetDResource( drawing, "MaxY", &MapMaxY );

   /* Calculate center and extent, used in coordinate conversion. */
   MapCenter.x = ( MapMinX + MapMaxX ) / 2.;
   MapCenter.y = ( MapMinY + MapMaxY ) / 2.;
   MapCenter.z = 0.;

   MapExtent.x = MapMaxX - MapMinX;
   MapExtent.y = MapMaxY - MapMinY;
   MapExtent.z = 0.;

   /* Full Glg extent is used for the map, hardcoded. Stretch must be TRUE. */
   GlgMinX = -1000.;
   GlgMaxX =  1000.;
   GlgMinY = -1000.;
   GlgMaxY =  1000.;

   /* Query if the drawing preserves X/Y ratio. */
   GlgGetDResource( drawing, "Stretch", &stretch );
   Stretch = (long) stretch;
}

/*----------------------------------------------------------------------
| Handle user interaction.
*/
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{
   GlgObject
     message_obj;
   char
     * format,
     * origin,
     * full_origin,
     * action,
     * subaction;

   message_obj = (GlgObject) call_data;

   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Origin", &origin );
   GlgGetSResource( message_obj, "FullOrigin", &full_origin );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "SubAction", &subaction );

   /* Handle window closing. */
   if( strcmp( format, "Window" ) == 0 )
   {
      if( strcmp( action, "DeleteWindow" ) == 0 )
        if( strcmp( origin, "SelectionDialog" ) == 0 )
        {
           /* Close selection dialog. */
           GlgSetDResource( Drawing, "SelectionDialog/Visibility", 0. );
           GlgUpdate( Drawing );	 
        }
        else
          exit( GLG_EXIT_OK );    /* Closing the main window: exit. */
   }
   else if( strcmp( format, "Button" ) == 0 )
   {
      if( strcmp( action, "Activate" ) != 0 )
	return;

      if( strcmp( origin, "CloseDialog" ) == 0 )
      {
	 GlgSetDResource( Drawing, "SelectionDialog/Visibility", 0. );
	 GlgUpdate( Drawing );	 
      }
      else if( strcmp( origin, "ZoomIn" ) == 0 )
	GlgSetZoom( MapViewport, NULL, 'i', 0. );	 
      else if( strcmp( origin, "ZoomOut" ) == 0 )
	GlgSetZoom( MapViewport, NULL, 'o', 0. );	 
      else if( strcmp( origin, "ZoomReset" ) == 0 )
	GlgSetZoom( MapViewport, NULL, 'n', 0. );	 
      else if( strcmp( origin, "ZoomTo" ) == 0 )
	GlgSetZoom( MapViewport, NULL, 't', 0. );	 
      else if( strcmp( origin, "ColorScheme" ) == 0 )
      {
	 ColorScheme ^= 1;    /* Toggle between 0 and 1 */
	 GlgSetDResource( Drawing, "MapArea/ColorIndex", (double)ColorScheme );
	 GlgSetDResource( Drawing, "MapArea/Icon0/Group/ColorIndex",
			 (double) ColorScheme );
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "Connections" ) == 0 )
      {
	 ToggleResource( Drawing, "MapArea/Connections/Visibility" );
      }
      else if( strcmp( origin, "ValueDisplay" ) == 0 )
      {
	 /* Visibility of all labels is constrained, set just one. */
	 ToggleResource( Drawing,
			"MapArea/Icon0/Group/ValueLabel/Visibility" );
	 ToggleResource( Drawing,
			"MapArea/Link0/ValueLabel/Visibility" );
      }
      else if( strcmp( origin, "Map" ) == 0 )
      {
	 ToggleResource( Drawing, "MapArea/MapGroup/Visibility" );
      }
      else if( strcmp( origin, "Update" ) == 0 )
      {
	 DoUpdate = !DoUpdate;
      }	
      else if( strcmp( origin, "MapType" ) == 0 )
      {
	 if( strcmp( map_file, "us_map.g" ) == 0 )
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
	 
	 /* Disable updates: on Windows, loading the map generates events
	    which trigger update timer . */
	 DoUpdate = False;

	 /* Do clean-up. */
	 UnloadMap();

	 /* Load a new map. */
	 LoadMap();
	 GlgUpdate( Drawing );

	 DoUpdate = True;   /* Enable updates again */

	 return;
      }	
   }
   /* Process mouse clicks on objects of interests in the drawing: 
      implemented as an Action with the "Node", "Link" or other label 
      attached to an object and activated on a mouse click. 
   */
   else if( strcmp( format, "CustomEvent" ) == 0 )
   {
      GlgObject highlight_obj = (GlgObject)0;
      GlgBoolean has_data = False;
      char
	* label = NULL,
	* visibility_name = NULL,
	* event_label,
	* location_str,
	* icon_name;
      double
	data,
	visibility_value,
	zoom_to_mode;

      GlgGetDResource( MapViewport, "ZoomToMode", &zoom_to_mode );
      if( zoom_to_mode )
	return;   /* Don't handle selection in ZoomTo mode. */

      GlgGetSResource( message_obj, "EventLabel", &event_label );      
      if( strcmp( event_label, "BackgroundVP" ) == 0 )
      {
	 /* The background viewport selection is reported only if there
	    are no other selections: erase the highlight.
	    */
	 Highlight( Drawing, (GlgObject)0 );
	 GlgUpdate( Drawing );
	 return;
      }
      /* Process state selection on the US map. */
      else if( strcmp( event_label, "MapSelection" ) == 0 )
      {
	 label = "None";
	 
	 /* The selection is reported for the MapGroup. The OrigObject is
	    used to get the object ID of the selected lower level state 
	    polygon.
	    */
	 highlight_obj = GlgGetResourceObject( message_obj, "OrigObject" );
	 GlgGetSResource( highlight_obj, "Name", &icon_name );
	 has_data = False;
	 visibility_name = "MapArea/MapGroup/Visibility";	    
	    
	 /* Location is set to the mouse click by the preceding TraceCB. */
      }
      else if( strcmp( event_label, "Node" ) == 0 )
      {
	 GlgObject node;
	 GlgPoint xy, lat_lon;

	 node = GlgGetResourceObject( message_obj, "Object" );
	 GlgGetSResource( node, "Name", &icon_name );
	 SelectedObject = node;

	 /* Query the label of the selected node */
	 GlgGetSResource( node, "Group/Label/String", &label );

	 /* Query node location. */
	 GlgGetGResource( node, "Position", &xy.x, &xy.y, &xy.z );

	 /* Convert world coordinates to lat/lon */
	 GetLatLon( &xy, &lat_lon );

	 /* Generate a location info string by converting +- sign info 
	    into the N/S, E/W suffixes.
	    */
	 location_str = CreateLocationString( lat_lon.x, lat_lon.y );

	 /* Display position info in the dialog. */
	 GlgSetSResource( Drawing, "SelectionDialog/Location", location_str );
	 GlgFree( location_str );

	 GlgGetDResource( node, "Group/Value", &data );
	 has_data = True;
	 visibility_name = "MapArea/Icon0/Visibility";
      }
      else if( strcmp( event_label, "Link" ) == 0 )
      {
	 GlgObject link;

	 link = GlgGetResourceObject( message_obj, "Object" );
	 GlgGetSResource( link, "Name", &icon_name );
	 SelectedObject = link;

	 label = "None";
	 GlgGetDResource( link, "Value", &data );
	 has_data = True;
	 visibility_name = "MapArea/Connections/Visibility";

	 /* Location is set to the mouse click by the preceding TraceCB. */
      }
      else
	return;    /* No selection */

      /* Check if this layer is visible. */
      GlgGetDResource( Drawing, visibility_name, &visibility_value );

      if( visibility_value == 1. )
      {
	 if( !icon_name )
	   icon_name = "";

	 /* Display the icon name, label and data in the dialog. */
	 GlgSetSResource( Drawing, "SelectionDialog/ID", icon_name ); 
	 GlgSetSResource( Drawing, "SelectionDialog/Facility", label ); 
	 if( has_data )
	   GlgSetDResource( Drawing, "SelectionDialog/Data", data );
	 GlgSetDResource( Drawing, "SelectionDialog/DataLabel/Visibility",
			 has_data ? 1. : 0. );
	 
	 /* Graph's Visibility is constrained to the DataLabel's Visibility */
	 if( has_data )
	   /* Reset the graph by setting all datasamples to 0 */
	   GlgSetDResource( Drawing, 
	      "SelectionDialog/Graph/DataGroup/Points/DataSample%/Value", 0. );

	 GlgSetDResource( Drawing, "SelectionDialog/Visibility", 1. );	 
	 Highlight( Drawing, highlight_obj );
      }
   }
   GlgUpdate( Drawing );
}

/*----------------------------------------------------------------------
| Handle mouse events.
*/
void Trace( viewport, client_data, call_data )
     GlgObject viewport;
     GlgAnyType client_data;
     GlgAnyType call_data;
{
   GlgTraceCBStruct * trace_data;
   GlgPoint screen_point, world_point, lat_lon;
   EventType event_type = 0;
   double zoom_to_mode;
   char * location_str;
   
   trace_data = (GlgTraceCBStruct*) call_data;

   if( trace_data->viewport != MapViewport )
     return;

   /* Platform-specific code to extract event information. 
      GLG_COORD_MAPPING_ADJ is added to the cursor coordinates for precise
      pixel mapping. 
   */
#ifndef _WINDOWS
   switch( trace_data->event->type )
   {
    case ButtonPress:
      if( trace_data->event->xbutton.button != 1 )
	return;  /* Use the left button clicks only. */
      screen_point.x = trace_data->event->xbutton.x + GLG_COORD_MAPPING_ADJ;
      screen_point.y = trace_data->event->xbutton.y + GLG_COORD_MAPPING_ADJ;
      screen_point.z = 0;
      event_type = BUTTON_PRESS;
      break;

    default: return;
   }      
#else
   switch( trace_data->event->message )
   {
    case WM_LBUTTONDOWN:
      screen_point.x = LOWORD( trace_data->event->lParam ) + 
        GLG_COORD_MAPPING_ADJ;
      screen_point.y = HIWORD( trace_data->event->lParam ) + 
        GLG_COORD_MAPPING_ADJ;
      screen_point.z = 0;
      event_type = BUTTON_PRESS;
      break;

    default: return;
   }
#endif

   switch( event_type )
   {
    case BUTTON_PRESS:
      GlgGetDResource( MapViewport, "ZoomToMode", &zoom_to_mode );
      if( zoom_to_mode )
	return;   /* Ignore clicks in ZoomTo mode. */
      
      GlgScreenToWorld( MapViewport, True, &screen_point, &world_point );

      GetLatLon( &world_point, &lat_lon ); 
      
      /* Generate a location info string by converting +- sign info into the
	 N/S, E/W suffixes. */
      location_str = CreateLocationString( lat_lon.x, lat_lon.y );
      GlgSetSResource( viewport, "SelectionDialog/Location", location_str );   
      GlgFree( location_str );
      
      /* Set facility to "None" for now: will be set by the Select callback
	 if any selected. */
      GlgSetSResource( viewport, "SelectionDialog/ID", "None" );
      GlgSetSResource( viewport, "SelectionDialog/Facility", "None" );
      
      /* Not an icon or link: no associated data. */
      GlgSetDResource( viewport, "SelectionDialog/DataLabel/Visibility", 0. );
      
      GlgSetDResource( viewport, "SelectionDialog/Visibility", 1. );
      GlgUpdate( viewport );
      break;
   }
}

/*----------------------------------------------------------------------
| Highlight or unhighlight selected map polygon.
| Changes the index of the color list transform attached to the object's
| FillColor.
*/
void Highlight( GlgObject drawing, GlgObject sel_object )
{
   /* Restore the color of the prev. highlighted object. */
   if( SelectedColorIndex )
   {
      GlgSetDResource( SelectedColorIndex, NULL, 0. );
      SelectedColorIndex = (GlgObject)0;
   }

   /* Highlight new object by changing its color */
   if( sel_object )
   {
      SelectedColorIndex = 
	GlgGetResourceObject( sel_object, "SelectColorIndex" );
      if( SelectedColorIndex )
	GlgSetDResource( SelectedColorIndex, NULL, 1. );
   }
}

/*----------------------------------------------------------------------
| Converts Lat/Lon to X/Y in GLG world coordinates.
*/
void GetXY( GlgPoint * lat_lon, GlgPoint * xy )
{ 
   GlmConvert( GLG_RECTANGULAR_PROJECTION, Stretch, GLG_OBJECT_COORD,
	      /* coord_to_lat_lon */ False, &MapCenter, &MapExtent, 0.,
	      GlgMinX, GlgMaxX, GlgMinY, GlgMaxY,
	      lat_lon, xy );
}

/*----------------------------------------------------------------------
| Converts Lat/Lon to X/Y in GLG world coordinates.
*/
void GetLatLon( GlgPoint * xy, GlgPoint * lat_lon )
{ 
   GlmConvert( GLG_RECTANGULAR_PROJECTION, Stretch, GLG_OBJECT_COORD,
	      /* coord_to_lat_lon */ True, &MapCenter, &MapExtent, 0.,
	      GlgMinX, GlgMaxX, GlgMinY, GlgMaxY,
	      xy, lat_lon );
}

/*----------------------------------------------------------------------
| Generate a location info string by converting +- signs info into the
| N/S, E/W suffixes.
*/
char * CreateLocationString( double world_x, double world_y )
{
   long
     x_deg, y_deg,
     x_min, y_min,
     x_sec, y_sec;
   char
     char_x,
     char_y,
     buffer[ 100 ];

   if( world_x < 0. )
   {
      world_x = -world_x;
      char_x = 'W';
   }
   else if( world_x >= 360. )
   {
      world_x -= 360.;
      char_x = 'E';
   }
   else if( world_x >= 180. )
   {
      world_x = 180. - ( world_x - 180. );
      char_x = 'W';
   }
   else
     char_x = 'E';

   if( world_y < 0. )
   {
      world_y = -world_y;
      char_y = 'S';
   }
   else
     char_y = 'N';

   x_deg = world_x;
   x_min = ( world_x - x_deg ) * 60.;
   x_sec = ( world_x - x_deg - x_min / 60. ) * 3600.;

   y_deg = world_y;
   y_min = ( world_y - y_deg ) * 60.;
   y_sec = ( world_y - y_deg - y_min / 60. ) * 3600.;

   sprintf( buffer, "Lon=%.3ld.%.2ld'%.2ld\"%c  Lat=%.2ld.%.2ld'%.2ld\"%c",
	   x_deg, x_min, x_sec, char_x, y_deg, y_min, y_sec, char_y ); 
   
   return GlgStrClone( buffer );
}

/*----------------------------------------------------------------------
| Installs an update timer.
*/
void StartUpdate( GlgObject map_drawing )
{
   GlgAddTimeOut( AppContext, UpdateInterval, (GlgTimerProc)UpdateMap, NULL );
}

/*----------------------------------------------------------------------
| Updates flow display, data, etc.
*/
void UpdateMap( data, timer_id )
     GlgAnyType data;
     GlgLong * timer_id;
{
   static long counter = 0;    /* Used to control update frequency. */
   GlgObject
     link,
     icon;
   double
     flow_data,
     value;
   long 
     i,
     size,
     line_type,
     offset;
   char * res_name;

   if( !DoUpdate )
   {
      GlgAddTimeOut( AppContext, 
		    UpdateInterval, (GlgTimerProc)UpdateMap, NULL );      
      return;
   }

   if( ShowFlow )
   {
      if( !flow_display_obj )   /* First time. */
	flow_display_obj =
	  GlgGetResourceObject( Drawing, "MapArea/Link0/Line/LineType" );
      
      /* Links's flow is constrained: animating one animates all. 
	 Flow direction is defined by the order of the links points when
	 constrained. */
      if( flow_display_obj )
      {
	 /* Query the current line type and offset. */
	 GlgGetDResource( flow_display_obj, NULL, &flow_data );
	 line_type = ( (long)flow_data ) % 32;
	 offset = ( (long)flow_data ) / 32;
	 
	 /* Increase the offset and set it back. */
	 --offset;
	 if( offset < 0 )
	   offset = 32 * 31;
	 flow_data = offset * 32 + line_type;
	 GlgSetDResource( flow_display_obj, NULL, flow_data );      
      }
   }

   /* Update facility values every time. */
   size = (long)GlgGetSize( FacilitiesGroup );
   for( i=0; i<size; ++i )
   {
      value = GlgRand( 30., 500. );
      res_name = GlgCreateIndexedName( "MapArea/Icon%", i );
      icon = GlgGetResourceObject( Drawing, res_name );
      GlgFree( res_name );
      GlgSetDResource( icon, "Group/Value", value );

      /* Update selected object data display in the SelectionDialog. */
      if( icon == SelectedObject )
      {
	 GlgSetDResource( Drawing, "SelectionDialog/Data", value );
	 GlgSetDResource( Drawing,
			 "SelectionDialog/Graph/DataGroup/EntryPoint",
			 value / 500. );
         /* To scroll ticks */
	 GlgSetSResource( Drawing,
			 "SelectionDialog/Graph/XMajorGroup/TicksEntryPoint",
			 "" );
      }

      if( ( counter % UpdateN ) == 0 )  /* Update icon type every n-th time. */
      {
	 if( GlgRand( 0., 10. ) > 2. )
	 {
	    double icon_type;

	    GlgGetDResource( icon, "Group/Graphics/IconType", &icon_type );

	    if( icon_type != 0. )
	      icon_type = 0.;
	    else		 
	      icon_type = GlgRand( 0., 6. );

	    GlgSetDResource( icon, "Group/Graphics/IconType", icon_type );
	 }
      }
   }

   if( !( counter % UpdateN ) )   /* Update link values every n-th fime. */
   {
      size = (long) GlgGetSize( LinksGroup );
      for( i=0; i<size; ++i )
      {      
	 value = GlgRand( 1., 9. );
	 res_name = GlgCreateIndexedName( "MapArea/Link%", i );
	 link = GlgGetResourceObject( Drawing, res_name );
	 GlgFree( res_name );
	 GlgSetDResource( link, "Value", value );
	 GlgSetDResource( link, "Line/LineWidth", value );
	 GlgSetDResource( link, "Line/LineColorIndex",
			 (double)(((long)value)/2) );

	 /* Update selected object data display in the SelectionDialog. */
	 if( link == SelectedObject )
	 {
	    GlgSetDResource( Drawing, "SelectionDialog/Data", value );
	    GlgSetDResource( Drawing,
			    "SelectionDialog/Graph/DataGroup/EntryPoint", 
			    value / 10. );
            /* To scroll ticks */
            GlgSetSResource( Drawing,
                           "SelectionDialog/Graph/XMajorGroup/TicksEntryPoint",
                            "" );
	 }	
      }
   }

   ++counter;
   if( counter > 1000 )
     counter = 0;

   GlgUpdate( Drawing );
   GlgAddTimeOut( AppContext, UpdateInterval, (GlgTimerProc)UpdateMap, NULL );
}

/*----------------------------------------------------------------------
| Toggle resource between 0 and 1.
*/
void ToggleResource( GlgObject object, char * res_name )
{
   double value;

   GlgGetDResource( object, res_name, &value );
   GlgSetDResource( object, res_name, value ? 0. : 1. );
}

/*----------------------------------------------------------------------
|
*/
void error( char * string, GlgBoolean quit )
{
   printf( "%s\n", string );
   if( quit )
     exit( GLG_EXIT_ERROR );
}

