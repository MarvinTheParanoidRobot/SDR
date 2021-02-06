#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef _WINDOWS
#include "resource.h"
#pragma warning( disable : 4244 )
#endif
#include "GlgApi.h"
#include "gis_demo.h"

#define DEBUG_TIMER      0   /* Set to 1 to debug timer intervals */

#define NUM_PLANES      10

double PlaneSpeed = 0.0005;    /* Relative units */
double MaxZoomSpeed = 5.;      /* In XY coordinates */
long UpdateInterval = 50;  /* Update interval in msec */

long USZoomDelay1 = 2500;  /* Delay to zoom to the US area to show details. */
long USZoomDelay2 = 2000;  /* Delay to remove US zooming message. */

#define SMALL_SIZE   0.23
#define MEDIUM_SIZE  0.33
#define BIG_SIZE     0.43

GlgObject
  Drawing,     
  NodeTemplate[ 2 ],
  PlaneTemplate[ 2 ],
  Map[ 2 ],
  GISObject[ 2 ],
  NodeGroup[ 2 ],
  PlaneGroup[ 2 ],
  PositionArea,
  PositionObject;

GlgPoint
  InitExtent[ 2 ],    /* Store initial extent and center, used to reset */
  InitCenter[ 2 ];

double
  PlaneSize = MEDIUM_SIZE,
  /* Dimensions of the map viewport windows */
  window_width[ 2 ],
  window_height[ 2 ];

long
  NumPlanes = NUM_PLANES,
  NumNodes,
  MapProjection[ 2 ],
  LockSelectedPlane,       /* If True, pan the map to make the selected plane 
			      visible in the current zoomed area */
  DoUpdate = True,
  PanMode = False,
  SuspendPromptUpdates = False,
  CityLabels = True,
  StateDisplay = True,
  DraggingFromButton = False;

PlaneData PlaneArray[ NUM_PLANES ];
PlaneData * SelectedPlane = (PlaneData*)0;

/* Array of icons to place on the map as GLG objects in addition to the icons
   defined in GIS server's data. The icons that use GLG objects may be selected
   with the mouse and their attributes can be changed dynamically, based on 
   data. When the mouse moves over an icon, it may be highlighted with a 
   different color or a tooltip may be displayed. */
NodeData NodeArray[] =
{
   { "Boston",        -71.01789,  42.33602 },
   { "New York",      -73.97213,  40.77436 },
   { "San Francisco", -122.55478, 37.79325 },
   { "Miami",         -80.21084,  25.77566 },
   { "Seattle",       -122.35032, 47.62180 },
   { "Houston",       -95.38672,  29.76870 },
   { "Denver",        -104.87265, 39.76803 },
   { "Minneapolis",   -93.26684,  44.96185 },
   { "Chicago",       -87.68496,  41.83705 },
   { "Dallas",        -96.76524,  32.79415 }
};

GlgAppContext AppContext;   /* Global, used to install a timer. */

#define RELATIVE_TO_NEW_RANGE( low, high, rel_value ) \
   ( (low) + ((high) - (low)) * rel_value )

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

#define RadToDeg( angle )   ( ( angle ) / M_PI * 180. )

#include "GlgMain.h"    /* Cross-platform entry point. */

/*----------------------------------------------------------------------
|
*/
int GlgMain( argc, argv, app_context )
     int argc;
     char *argv[];
     GlgAppContext app_context;
{
   char 
     * full_path,
     * message = "Loading map, please wait....";

   AppContext = GlgInit( False, app_context, argc, argv );

   NumNodes = sizeof( NodeArray ) / sizeof( NodeArray[ 0 ] );

   GlgSetDResource( (GlgObject)0, "$config/GlgMouseTooltipTimeout", 0.05 );

   full_path = GlgGetRelativePath( argv[0], "gis_demo.g" );
   Drawing = GlgLoadWidgetFromFile( full_path );
   if( !Drawing )
     error( "Can't load drawing file.", True );

   GlgFree( full_path );

   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( Drawing, "Point1", -700., -700., 0. );
   GlgSetGResource( Drawing, "Point2",  700.,  700., 0. );

   /* Setting the window title. */
   GlgSetSResource( Drawing, "ScreenName", "GLG GIS Demo: Detailed Map" );

   /* Get IDs of the two map viewports. */
   Map[0] = GlgGetResourceObject( Drawing, "TopMap" );   /* Thumbnail map */
   Map[1] = GlgGetResourceObject( Drawing, "Map" );      /* Detailed map */

   /* Display thumbnail map in a separate window. It is kept as a child window
      in the drawing for the convinience of editing. */
   GlgSetDResource( Map[ 0 ], "ShellType", (double) GLG_DIALOG_SHELL );
   GlgSetSResource( Map[ 0 ], "Screen/ScreenName", 
                   "GLG GIS Demo: Globe View" );

   GlgAddCallback( Drawing, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );
   GlgAddCallback( Drawing, GLG_TRACE_CB, (GlgCallbackProc)Trace, NULL );

   Init();

   GlgSetupHierarchy( Drawing );

   /* Adjust selected region on the thumbnail map to match the zoomed area 
      of the detailed map. */
   SetSelectedArea();

#ifdef _WINDOWS            
   /* Set icons for both top-level windows. */
   GlgLoadExeIcon( Drawing, IDI_ICON1 );
   GlgLoadExeIcon( Map[0],  IDI_ICON1 );
#endif

   GlgUpdate( Drawing );

   StartUpdate();   /* Install an update timer. */

   /* Zoom to the US area after a few seconds to show details. */
   GlgAddTimeOut( AppContext, USZoomDelay1, 
                 (GlgTimerProc)ZoomToUSStart, NULL );

   return (int) GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| Initializes the drawing
*/
void Init()
{
   GlgObject
     palette,
     resource,
     selected_area;
   double projection;
   long i;

   for( i=0; i<2; ++i )   /* For each map window */
   {
      /* Get IDs of the GIS map objects in each of the map viewports. */
      GISObject[ i ] = GlgGetResourceObject( Map[ i ], "GISObject" );

      /* Query and store the GIS projection (ORTHOGRAPHIC or RECTANGULAR)
	 used to render the map. */ 
      GlgGetDResource( GISObject[ i ], "GISProjection", &projection );
      MapProjection[ i ] = (long) projection;

      /* Set GIS Zoom mode. It was set and saved with the drawing, but do it 
         again programmatically just in case.
         */
      GlgSetZoomMode( Map[ i ], NULL, GISObject[ i ], NULL, GLG_GIS_ZOOM_MODE );

      /* Store initial map extent for resetting after zooming. */
      GlgGetGResource( GISObject[ i ], "GISExtent",
		      &InitExtent[i].x, &InitExtent[i].y, &InitExtent[i].z );
      GlgGetGResource( GISObject[ i ], "GISCenter",
		      &InitCenter[i].x, &InitCenter[i].y, &InitCenter[i].z );
   }

   /* Get the palette containing templates for plane and node icons. */
   palette = GlgGetResourceObject( Drawing, "Palette" );

   /* Reference the palette to keep it around and delete it from the drawing */
   GlgReferenceObject( palette );
   GlgDeleteThisObject( Drawing, palette );

   /* Get node and plane templates from the palette. Two sets of templates
      are used: smaller icons for the thumbnail view and more elaborate ones
      for the detailed map.
      */
   for( i=0; i<2; ++i )
   {
      NodeTemplate[ i ] = 
	GlgGetResourceObject( palette, i == 0 ? "Node1" : "Node2" );
      
      PlaneTemplate[ i ] = 
	GlgGetResourceObject( palette, i == 0 ? "Plane1" : "Plane2" );

      /* If the icon is not a marker (IconScale resource exists), set the 
         icon's size. */
      resource = GlgGetResourceObject( PlaneTemplate[ i ], "IconScale" );
      if( resource )
	GlgSetDResource( resource, NULL, PlaneSize );
   }

   /* Initialize plane structures used for simulation. */
   for( i =0; i < NumPlanes; ++i )
   {
      PlaneArray[ i ].name = GlgCreateIndexedName( "", i );
      PlaneArray[ i ].tooltip[ 0 ] = PlaneArray[ i ].tooltip[ 1 ] = NULL;
   }
      
   /* Add node and plane icons to both thumbnail and detailed map. */
   for( i=0; i<2; ++i )
   {	 
      CreateAirportIcons( i ); /* Add airport icons */
      CreatePlaneIcons( i );   /* Add plane icons */
   }

   /* Start all planes. */
   for( i=0; i < NumPlanes; ++i )
     StartPlane( &PlaneArray[ i ], True );

   /* Selected area annotates the currently viewed area of the detailed map 
      in the thumbnail map view. Reorder SelectedArea on top of icons
      (last in the array). */
   selected_area = 
     GlgGetResourceObject( GISObject[ 0 ], "GISArray/SelectedArea" );
   GlgReorderElement( GISObject[ 0 ], 
                     GlgGetIndex( GISObject[ 0 ], selected_area ), 
		     GlgGetSize( GISObject[ 0 ] ) - 1 );

   /* Set state display on the thumbnail map. Airport labels on the detailed
      map are handled by HandleZoomLevel(). 
      */
   SetGISLayers( 0 );

   /* Demos starts with the whole word view, then zooms to the US area
      in a few seconds to show more details. Set initial parameters
      for the whole world view.
      */
   HandleZoomLevel();

   InitSelection();  /* Select some plane. */

   /* Store objects used to display lat/lon on mouse move. */
   PositionArea = GlgGetResourceObject( Drawing, "PositionArea" );
   PositionObject = 
     GlgGetResourceObject( Drawing, "PositionLabel/PointerLatLon" );
   GlgSetSResource( PositionObject, NULL, "" );

   /* Set US zooming message to OFF initially. */
   GlgSetDResource( Drawing, "Map/USZoomingMessage/Visibility", 0. );
}

/*----------------------------------------------------------------------
| Creates NodeGroup to hold airport icons and adds the icons to it.
*/
void CreateAirportIcons( long map )
{
   long i;

   /* Create a group to hold city icons as a layer. */
   NodeGroup[ map ] = 
     GlgCreateObject( GLG_GROUP, "NodeGroup", NULL, NULL, NULL, NULL );

   /* Add city/node icons */
   for( i = 0; i < NumNodes; ++i )
     AddNode( &NodeArray[ i ], map, i );
   
   /* Add the group to the GIS Object which will automatically manage 
      the GIS coordinate conversion. This allows to specify node 
      positions in lat/lon instead of the X/Y world coordinates.
      */
   GlgAddObjectToBottom( GISObject[ map ], NodeGroup[ map ] );
}

/*----------------------------------------------------------------------
| Creates PlaneGroup to hold plane icons and adds the icons to it.
*/
void CreatePlaneIcons( long map )
{
   long i;

   /* Create a group to hold plane icons as a layer. */
   PlaneGroup[ map ] = 
     GlgCreateObject( GLG_GROUP, "PlaneGroup", NULL, NULL, NULL, NULL );
   
   /* Add plane icons */
   for( i=0; i < NumPlanes; ++i )
     AddPlane( &PlaneArray[ i ], map, i );

   /* Add the group to the GIS Object which will automatically manage 
      the GIS coordinate conversion. This allows to specify plane 
      positions in lat/lon instead of the X/Y world coordinates.
      */
   GlgAddObjectToBottom( GISObject[ map ], PlaneGroup[ map ] );
}

/*----------------------------------------------------------------------
| Adds a city icon, fills labels, tooltips, etc.
*/
void AddNode( NodeData * node_data, long map, long index )
{      
   GlgObject node;
   char
     * tooltip,
     * lat_lon_string,
     buffer[ 1002 ];

   /* Create a copy of a node. */
   node = GlgCloneObject( NodeTemplate[ map ], GLG_STRONG_CLONE );

   GlgSetSResource( node, "Name", node_data->name );   /* Object name */

   /* Set node position in lat/lon coordinates. The GIS object will handle
      the GIS coordinate conversion depending on the map the node icon is 
      displayed in, as well as the map's zoom and pan state.
      */
   GlgSetGResource( node, "Position", 
                   node_data->lat_lon.x, node_data->lat_lon.y, 0. );

   /* Set index for direct access */     
   GlgSetDResource( node, "DataIndex", (double)index );

   if( map == 1 )   /* On the detailed map, show node name label. */     
     GlgSetSResource( node, "LabelString", node_data->name );

   if( map == 0 )   /* On the thumbnail map, show node name in the tooltip. */
     tooltip = node_data->name;
   else   /* On the detailed map, include lat/lon into the tooltip. */
   {
      lat_lon_string = 
        CreateLocationString( node_data->lat_lon.x, node_data->lat_lon.y, 1. );

      sprintf( buffer, "%.500s, %s", node_data->name, lat_lon_string );
      GlgFree( lat_lon_string );

      tooltip = buffer;
   }
   GlgSetSResource( node, "TooltipString", tooltip );
	
   node_data->graphics[ map ] = node;

   /* Add the node to the requested map (thumbnail or detailed). */
   GlgAddObjectToBottom( NodeGroup[ map ], node );
}

/*----------------------------------------------------------------------
| Adds a plane icon, fills labels, tooltips, etc.
*/
void AddPlane( PlaneData * plane_data, long map, long index )
{
   GlgObject plane;

   /* Create a copy of a node. */
   plane = GlgCloneObject( PlaneTemplate[ map ], GLG_STRONG_CLONE );

   GlgSetSResource( plane, "Name", plane_data->name );   /* Object name */

   /* Index for direct access */
   GlgSetDResource( plane, "DataIndex", (double)index );           

   plane_data->graphics[ map ] = plane;

   /* Check if the icon has an angle to indicate its direction. */
   if( GlgGetResourceObject( plane, "Angle" ) )
     plane_data->has_angle[ map ] = True;

   /* The plane will be positioned with PositionPlane() after the GIS object 
      have been setup and the plane's lat/lon has been calculated by the 
      flight simulation.
      */

   /* Add the plane to the requested map (thumbnail or detailed). */
   GlgAddObjectToBottom( PlaneGroup[ map ], plane );
}

/*----------------------------------------------------------------------
| 
*/
void PositionPlane( PlaneData * plane, long map )
{
   if( !plane->graphics[ map ] || 
      !plane->from_node || !plane->to_node )
     return;

   /* Obtain the plane's current position. */
   GetPlaneLatLon( plane );

   /* Update plane's icon in the drawing by setting its lat/lon coordinates.
      The GIS object will handle the GIS coordinate conversion depending on 
      the map the plane icon is displayed in, as well as the map's zoom 
      and pan state.
      */
   GlgSetGResource( plane->graphics[ map ], "Position", 
		   plane->lat_lon.x, plane->lat_lon.y, 0. );

   /* Update icon's direction angle is necessary */
   if( plane->has_angle[ map ] )
   {
      double angle = GetPlaneAngle( plane, map );
      GlgSetDResource( plane->graphics[ map ], "Angle", angle );
   }
}

/*----------------------------------------------------------------------
| FOR FLIGHT SIMULATION ONLY: Converts plane lat/lon to the GLG world 
|     coordinates for calculating plane speed and directional angle.
*/
void GetPlanePosition( PlaneData * plane, long map, GlgPoint * xyz )
{
   /* Converts lat/lon to X/Y using GIS object's current projection. */
   GlgGISConvert( GISObject[ map ], NULL, GLG_OBJECT_COORD, 
                 /* Lat/Lon to XY */ False, &plane->lat_lon, xyz );
}

/*----------------------------------------------------------------------
| FOR FLIGHT SIMULATION ONLY: Converts node lat/lon to the GLG world 
|     coordinates for calculating plane's initial directional angle.
*/
void GetNodePosition( NodeData * node, long map, GlgPoint * xyz )
{
   /* Converts lat/lon to X/Y using GIS object's current projection. */
   GlgGISConvert( GISObject[ map ], NULL, GLG_OBJECT_COORD, 
                 /* Lat/Lon to XY */ False, &node->lat_lon, xyz );
}

/*----------------------------------------------------------------------
| FOR FLIGHT SIMULATION ONLY: Calculates plane icon's directional angle.
|     In an application, it will query the plane's directional angle.
*/
double GetPlaneAngle( PlaneData * plane, long map  )
{
   GlgPoint last_xyz, curr_xyz;

   /* Rectangular projection preserves straight lines, we can use the 
      angle of the line connecting the start and end nodes. For the
      orthographic projection, use this case if the plane has just started
      and there is no previous position stored.
      */
   if( MapProjection[ map ] == GLG_RECTANGULAR_PROJECTION ||
      plane->path_position == plane->path_position_last )   /* Just started */
   {
      GetNodePosition( plane->from_node, map, &last_xyz );
      GetNodePosition( plane->to_node, map, &curr_xyz );
   }
   else  /* In the orthographic projection straight lines are drawn as curves.
	    Use the angle of the line connecting the current and last 
	    position of the plane. */
   {
      double stored_position;
      
      stored_position = plane->path_position;    /* Store current position. */
      
      /* Get coordinates of the plane's previous position */
      plane->path_position = plane->path_position_last;
      GetPlaneLatLon( plane );
      GetPlanePosition( plane, map, &last_xyz );

      /* Restore the plane's current position and get its coordinates. */
      plane->path_position = stored_position;
      GetPlaneLatLon( plane );
      GetPlanePosition( plane, map, &curr_xyz );
   }   

   /* Calculate the angle of a line connecting the previous and 
      current position. */
   return GetAngle( &last_xyz, &curr_xyz );
}

/*----------------------------------------------------------------------
| Checks if the object is visible in the current zoom region.
*/
long GetVisibility( GlgPoint * position, double adj )
{
   /* Use adj as a gap */
   return
     position->x > -1000. * adj && position->x < 1000. * adj &&
     position->y > -1000. * adj && position->y < 1000. * adj;
}

/*----------------------------------------------------------------------
| Installs a timer to update the simulation.
*/
void StartUpdate()
{
   GlgAddTimeOut( AppContext, UpdateInterval, (GlgTimerProc)UpdatePlanes, 
		 NULL );
}

/*----------------------------------------------------------------------
| Updates plane positions on both maps.
*/
void UpdatePlanes( GlgAnyType data, GlgIntervalID * id )
{
   GlgULong sec1, microsec1;
   GlgLong timer_interval;
   long i;

   if( !DoUpdate )
   {
      GlgAddTimeOut( AppContext, 
		    UpdateInterval, (GlgTimerProc)UpdatePlanes, NULL );      
      return;
   }

   GlgGetTime( &sec1, &microsec1 );  /* Start time */

   for( i = 0; i < NumPlanes; ++i )
     UpdatePlane( &PlaneArray[ i ] );

   UpdateSelectedPlaneStatus();

   /* Pan the map if necessary to keep the selected plane in sight. */
   if( LockSelectedPlane )
     UpdateLocking();

   GlgUpdate( Drawing );
   GlgSync( Drawing );    /* Improves interactive response */

   timer_interval = GetAdjustedTimeout( sec1, microsec1, UpdateInterval );

   GlgAddTimeOut( AppContext, timer_interval, (GlgTimerProc)UpdatePlanes, 
		 NULL );
}

/*----------------------------------------------------------------------
| 
*/
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, 
                           GlgLong interval )
{
   GlgULong sec2, microsec2;
   GlgLong elapsed_time, adj_interval;

   GlgGetTime( &sec2, &microsec2 );  /* End time */
   
   /* Elapsed time in millisec */
   elapsed_time = 
     ( sec2 - sec1 ) * 1000 + (long) ( microsec2 - microsec1 ) / 1000;

   /* Maintain constant update interval regardless of the system speed. */
   if( elapsed_time + 20 >= interval )
      /* Slow system: update as fast as we can, but allow a small interval 
         for handling input events. */
     adj_interval = 20;
   else
     /* Fast system: keep constant update interval. */
     adj_interval = interval - elapsed_time;

#if DEBUG_TIMER
   printf( "sec= %ld, msec= %ld\n",
           (long)( sec2 - sec1 ), (long)( microsec2 - microsec1 ) );
   printf( "*** elapsed= %ld, requested= %ld, adjusted= %ld\n",
           (long) elapsed_time, (long) interval, (long) adj_interval );
#endif

   return adj_interval;
}

/*----------------------------------------------------------------------
| FOR FLIGHT SIMULATION ONLY: Calculates new plane position using a
|     flight simulation. In an application, the real plane position 
|     will be queried here.
*/
void UpdatePlane( PlaneData * plane )
{      
   long i;

   if( !plane->graphics ||
      !plane->from_node || !plane->to_node )
     return;
            
   if( plane->path_position == 1. )
     StartPlane( plane, False );     /* Finished old path, start a new one. */
   else
   {
      double 
	speed,
	stored_path_position,	
	dist;
      GlgPoint old_position, new_position;

      speed = PlaneSpeed;

      /* Slow the selected plane down when zoomed on it for a nice
	 demo effect */
      if( plane == SelectedPlane && LockSelectedPlane )
      {
	 GetPlanePosition( plane, 1, &old_position );

         /* Store the current path position. */
	 stored_path_position = plane->path_position;

	 plane->path_position += plane->speed * speed;   /* Increment it */
	    
	 GetPlanePosition( plane, 1, &new_position );

	 /* Distance between the old and the new position of the plane. */
	 dist = GetLength( &old_position, &new_position );

         /* Adjust the plane's speed to slow it down if the distance
            is too big. */
	 if( dist > MaxZoomSpeed )
	 {
	    double slow_down = dist / MaxZoomSpeed;
	    speed /= slow_down;
	 }
	 
         /* Restore the current path position. */
	 plane->path_position = stored_path_position;
      }
      
      /* Store the last position for calculating the angle in the ORTHO 
         projection. */
      plane->path_position_last = plane->path_position;

      /* Move the plane. */
      plane->path_position += plane->speed * speed;
      if( plane->path_position > 1. )
        plane->path_position = 1.; /* Clamp to 1: can't go past the airport! */
   }

   for( i =0; i<2; ++i )
     PositionPlane( plane, i );   /* Position the plane on both maps */
}

/*----------------------------------------------------------------------
| In the lock mode, pans the map to keep selected plane visible when the 
| plane moves out of the detailed map area.
*/
void UpdateLocking()
{
   long map = 1;    /* Checking is done on the detailed map */
   GlgPoint position;
   
   GetPlanePosition( SelectedPlane, map, &position );

   /* If selected plane goes on another side of the globe or off the 
      visible portion of the map, pan the map to re-center on the 
      selected plane. The Z coordinate goes to zero when the plane gets 
      close to the globe's edge, and becomes negative on the invisible 
      side of the globe.
      */
   if( MapProjection[ map ] == GLG_ORTHOGRAPHIC_PROJECTION &&
      position.z < 0.1 || !GetVisibility( &position, 0.9 ) )
   {
      char * message = 
	"Loading new map to keep the selected plane in sight, please wait...";
      
      CenterOnPlane( SelectedPlane, 1 );
      UpdateMapWithMessage( 1, message );
      
      /* If the thumbnail map used orthographic projection, rotate the 
	 thumbnail globe to keep the selected plane visible. In rectangular
	 projection, the whole world is visible and no action is required.
	 */
      if( MapProjection[ 0 ] == GLG_ORTHOGRAPHIC_PROJECTION )
      {
	 CenterOnPlane( SelectedPlane, 0 );
	 UpdateMapWithMessage( 0, message );
      }
   }
}

/*----------------------------------------------------------------------
| Updates map image, displaying the "wait..." message while the new 
| map image is generated. 
| Also updates selected region display on the thumbnail map.
*/
void UpdateMapWithMessage( long map, char * message )
{
   /* Display the wait message while the new map image is being generated. */
   if( message )
     SetStatus( message );

   /* Just setup, don't draw: will be done at the end by the caller
      after selected area, etc., was updated. */
   GlgSetupHierarchy( Map[ map ] );

   /* Adjust selected region on the thumbnail map to match the new 
      zoom area of the detailed map. */
   SetSelectedArea();

   if( message )
     SetStatus( "" );
}

/*----------------------------------------------------------------------
| Handle user interaction.
*/
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{
   GlgObject message_obj;
   char
     * origin,
     * format,
     * action,
     * subaction;

   message_obj = (GlgObject) call_data;

   GlgGetSResource( message_obj, "Origin", &origin );
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "SubAction", &subaction );

   /* Handle window closing. */
   if( strcmp( format, "Window" ) == 0 )
   {
      if( strcmp( action, "DeleteWindow" ) == 0 )
	if( strcmp( origin, "SelectionDialog" ) == 0 )
	{
	   /* Closing the selection dialog */
	   GlgSetDResource( Drawing, "SelectionDialog/Visibility", 0. );
	   GlgUpdate( Drawing );	 
	   return;
	}
	else if( strcmp( origin, "TopMap" ) == 0 )
	{
	   /* Closing the top map window */
	   GlgSetDResource( Drawing, "TopMap/Visibility", 0. );
	   GlgUpdate( Drawing );	 
	   return;
	}
	else
          exit( GLG_EXIT_OK );     /* Closing the main window: exit. */

      return;
   }

   if( strcmp( format, "Button" ) == 0 )   /* Handle button clicks */
   {
      if( strcmp( action, "Activate" ) != 0 )
	return;

      PanMode = False;    /* Abort Pan mode */

      /* Abort ZoomTo/Drag modes (if any) on both maps. */
      GlgSetZoom( Map[ 0 ], NULL, 'e', 0. );
      GlgSetZoom( Map[ 1 ], NULL, 'e', 0. );

      if( strcmp( origin, "CloseDialog" ) == 0 )
      {
	 GlgSetDResource( Drawing, "SelectionDialog/Visibility", 0. );
	 GlgUpdate( Drawing );	 
      }
      else if( strcmp( origin, "ToggleLock" ) == 0 )
      {
	 SetLocking( !LockSelectedPlane );
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "Up" ) == 0 )
      {
	 Zoom( 'u', 0. );
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "Down" ) == 0 )
      {
	 Zoom( 'd', 0. );
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "Left" ) == 0 )
      {
	 Zoom( 'l', 0. );
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "Right" ) == 0 )
      {
	 Zoom( 'r', 0. );
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "ZoomIn" ) == 0 )
      {
	 Zoom( 'i', 2. );
         HandleZoomLevel();
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "ZoomOut" ) == 0 )
      {
	 Zoom( 'o', 2. );
         HandleZoomLevel();
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin,  "ZoomReset" ) == 0 )
      {	
	 Zoom( 'n', 0. );
         HandleZoomLevel();
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "ZoomTo" ) == 0 )
      {
         /* Abort a possible Drag mode on thumbnail map. */
	 GlgSetZoom( Map[ 0 ], NULL, 'e', 0. );

         /* Start ZoomTo mode on the detailed map. */
	 GlgSetZoom( Map[ 1 ], NULL, 't', 0. );
	 SetStatus( "Define a rectangular area on the detailed map to zoom to." );
         SuspendPromptUpdates = True;
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "Pan" ) == 0 )
      {	    
	 PanMode = True;
	 SetStatus( "Click to define a new center." );
         SuspendPromptUpdates = True;
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "Drag" ) == 0 )
      {
         /* Activate dragging mode. Dragging will start on the mouse click. 
            If no object of interest is selected by the mouse click, 
            dragging will be started by the code in the Trace callback 
            anyway, but only if no object of interest was selected. 
            The "Drag" button demostrates an alternative way to start dragging 
            from a button which starts dragging even if an object of interest
            is selected by the mouse click.
            */
         DraggingFromButton = True;
	 GlgSetZoom( Map[ 0 ], NULL, 's', 0. );
	 GlgSetZoom( Map[ 1 ], NULL, 's', 0. );
	 SetStatus( "Click and drag the map with the mouse." );
         SuspendPromptUpdates = True;
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "AirportLabels" ) == 0 )
      {
         CityLabels = !CityLabels;
         SetGISLayers( 1 );
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "Planes" ) == 0 )
      {
	 ToggleResource( Map[ 1 ], "PlaneGroup/Visibility" );
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "ValueDisplay" ) == 0 )
      {
	 /* Visibility of all labels is constrained, set just one. */
	 ToggleResource( PlaneArray[ 0 ].graphics[ 1 ], "Label/Visibility" );
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "ToggleStates" ) == 0 )
      {
         StateDisplay = !StateDisplay;
         SetGISLayers( 0 );    /* Thumbnail map */
         SetGISLayers( 1 );    /* Detailed map */

	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "Update" ) == 0 )
      {
	 DoUpdate = !DoUpdate;
	 GlgUpdate( Drawing );
      }	
      else if( strcmp( origin, "PlaneSize" ) == 0 )
      {
	 /* Change plane icon's size. */
	 if( PlaneSize == SMALL_SIZE )
	   PlaneSize = MEDIUM_SIZE;
	 else if( PlaneSize == MEDIUM_SIZE )
	   PlaneSize = BIG_SIZE;
	 else /* BIG_SIZE */
	   PlaneSize = SMALL_SIZE;	 

	 SetPlaneSize();
	 GlgUpdate( Drawing );
      }	
   }
   /* Process mouse clicks on plane icons, implemented as an Action with
      the Plane label attached to an icon and activated on a mouse click. 
   */
   else if( strcmp( format, "CustomEvent" ) == 0 )
   {
      char * event_label;

      GlgGetSResource( message_obj, "EventLabel", &event_label );

      if( strcmp( event_label, "Plane" ) == 0 )  /* Plane icon was selected */
      {
         GlgZoomState zoom_mode;
	 double data_index;

         /* Map dragging mode is activated either on a mouse click in the trace 
            callback, or with the Drag toolbar button. Abort the dragging mode
            if an object with custom event was selected and the dragging
            was activated on a mouse click. This gives custom events a higher
            priority compared to the dragging mode. If it's a ZoomTo mode 
            activated by a button or dragging activated from the Drag button,
            don't abort and ignore object selection.
         */
         zoom_mode = ZoomToMode();
         if( !zoom_mode || 
             ( zoom_mode & GLG_PAN_DRAG_STATE ) && !DraggingFromButton )
         {
            if( zoom_mode )
              GlgSetZoom( Map[1], NULL, 'e', 0. );  /* Abort zoom mode */

            GlgGetDResource( message_obj, "Object/DataIndex", &data_index );
            
            if( SelectedPlane != &PlaneArray[ (int)data_index ] )
            {
               SelectPlane( SelectedPlane, 0 );  /* Unhighlight old */
               SelectedPlane = &PlaneArray[ (int)data_index ];
               SelectPlane( SelectedPlane, 1 );  /* Highlight new */
               
               GlgUpdate( Drawing );
            }

            /* Lock on the selected plane to automatically scroll map
               when needed. */
            SetLocking( True );
         }
      }
   }
   else if( strcmp( action, "Zoom" ) == 0 )
   {
      /* Disable locking: we may be zooming on a different area. */
      SetLocking( False );

      if( strcmp( subaction, "Start" ) == 0 )    /* Starting ZoomTo */
      {
         SuspendPromptUpdates = True;
      }
      else if( strcmp( subaction, "End" ) == 0 )   /* Finishing ZoomTo */
      {
         /* Rotate the thumbnail globe to show the same area as the 
            detailed map. */
         SyncGlobeWithDetailedMap( origin, "Zooming the map, please wait..." );

         HandleZoomLevel();
         SuspendPromptUpdates = False;
         GlgUpdate( Drawing );
      }
      /* Aborting ZoomTo (right mouse button, etc.). */
      else if( strcmp( subaction, "Abort" ) == 0 )
      {
         SuspendPromptUpdates = False;
      }
   }
   else if( strcmp( action, "Pan" ) == 0 )
   {
      /* Disable locking when scrolling the map with the mouse. */
      SetLocking( False );

      if( strcmp( subaction, "Start" ) == 0 )   /* Map dragging start */
      {
	 SetStatus( "Drag the map with the mouse." );
         SuspendPromptUpdates = True;
         GlgUpdate( Drawing );
      }
      else if( strcmp( subaction, "Drag" ) == 0 ||         /* Dragging */
              strcmp( subaction, "ValueChanged" ) == 0 )   /* Scrollbars */
      {
         char * message_str;

         if( strcmp( subaction, "Drag" ) == 0 )
         {
            message_str = "Dragging the map with the mouse....";
            DraggingFromButton = False;
         }
         else
           message_str = "Scrolling the map, please wait...";

         /* When dragging the detailed map with the mouse, rotate the 
            thumbnail globe to show the same area as the detailed map. */
         SyncGlobeWithDetailedMap( origin, message_str );
         
         if( strcmp( subaction, "Drag" ) == 0 )
           SetStatus( message_str );   /* Keep the message when dragging. */

         GlgUpdate( Drawing );
      }
      else if( strcmp( subaction, "End" ) == 0 )   /* Map dragging ended */
      {
         DraggingFromButton = False;
         SuspendPromptUpdates = False;
         SetStatus( "" );   /* Reset prompt when done dragging. */

         /* Reset dragging mode on both maps, in case it was started with the
            Drag button. */
	 GlgSetZoom( Map[ 0 ], NULL, 'e', 0. );
	 GlgSetZoom( Map[ 1 ], NULL, 'e', 0. );
         GlgUpdate( Drawing );         
      }
      /* Dragging aborted (right mouse button, etc.). */
      else if( strcmp( subaction, "Abort" ) == 0 )
      {
         DraggingFromButton = False;
         SuspendPromptUpdates = False;
         SetStatus( "" );   /* Reset prompt when aborting dragging. */
         GlgUpdate( Drawing );         
      }
   }
}

/*----------------------------------------------------------------------
|
*/
long ZoomToMode()
{
   double zoom_mode;

   GlgGetDResource( Map[ 1 ], "ZoomToMode", &zoom_mode );
   return (long)zoom_mode;
}

/*----------------------------------------------------------------------
|
*/
void Zoom( long type, double value )
{
   switch( type )
   {
    default:
      GlgSetZoom( Map[ 1 ], NULL, type, value );
      CheckScrollLimits( type );
      UpdateMapWithMessage( 1, "Zooming or panning, please wait..." );

      /* Sync thumbnail map when panning. After "1:1" zoom reset, the maps' 
         centers may differ, sync the thumbnail map when zooming in the 
         first time. */      
      switch( type )
      {
       case 'i':
         SyncGlobeWithDetailedMap( NULL, "Zooming the map, please wait..." );
         break;
       case 'u':
       case 'd':
       case 'l':
       case 'r':
         SyncGlobeWithDetailedMap( NULL, "Panning the map, please wait..." );
	 break;
      }
      break;

    case 'n':
      if( MapProjection[ 0 ] == GLG_ORTHOGRAPHIC_PROJECTION )
      {
	 /* Reset thumbnail globe to initial position. */
	 GlgSetGResource( GISObject[ 0 ], "GISCenter", 
			 InitCenter[0].x, InitCenter[0].y, InitCenter[0].z );
	 GlgSetGResource( GISObject[ 0 ], "GISExtent",
			 InitExtent[0].x, InitExtent[0].y, InitExtent[0].z );
	 UpdateMapWithMessage( 0, "Reloading map, please wait..." );
      }

      /* Reset detailed map to initial extent. */
      GlgSetGResource( GISObject[ 1 ], "GISCenter",
		      InitCenter[1].x, InitCenter[1].y, InitCenter[1].z );
      GlgSetGResource( GISObject[ 1 ], "GISExtent", 
		      InitExtent[1].x, InitExtent[1].y, InitExtent[1].z );
      UpdateMapWithMessage( 1, "Reloading map, please wait..." );

      /* Make selected area rectangle invisible when no zoom  */
      GlgSetDResource( GISObject[ 0 ],
                      "GISArray/SelectedArea/Visibility", 0. );
      break;
   }
}

/*----------------------------------------------------------------------
| For rectangular projection on the detailed map, make sure the map 
| does not scroll beyond the poles in the vertical direction.
*/
void CheckScrollLimits( long type )
{
   GlgPoint extent, center;
   double min_y, max_y;
   long adjust_x, adjust_y;

   if( MapProjection[ 1 ] == GLG_ORTHOGRAPHIC_PROJECTION )
     return;   /* Allow unlimited scrolling on ortho. */

   switch( type )
   {
    case 'u':  /* Scroll up */
    case 'd':  /* Scroll down */
    case 'l':  /* Scroll left */
    case 'r':  /* Scroll right */
      break; /* Adjust only for scroll types. */
    default: return;   /* Don't adjust for other zoom types. */
   }

   GlgGetGResource( GISObject[ 1 ], "GISExtent", 
                   &extent.x, &extent.y, &extent.z );
   GlgGetGResource( GISObject[ 1 ], "GISCenter",
                   &center.x, &center.y, &center.z );
   
   min_y = center.y - extent.y / 2.;
   max_y = center.y + extent.y / 2.;

   /* Check and adjust center lat to make sure the map does not scroll 
      beyond the poles in the vertical direction. 
      */
   adjust_y = True;
   if( extent.y >= 180. )
     center.y = 0.;
   else if( min_y < -90. )
     center.y = -90. + extent.y / 2.;
   else if( max_y > 90. )
     center.y = 90. - extent.y / 2.;
   else
     adjust_y = False;

   /* Allow scrolling tp +-180 in horizontal direction, to match the
      range of the horizontal scrollbar. */
   adjust_x = True;
   if( center.x < -180. )
     center.x = -180.;
   else if( center.x > 180. )
     center.x = 180.;
   else
     adjust_x = False;

   /* Set adjusted center */
   if( adjust_x || adjust_y )
     GlgSetGResource( GISObject[ 1 ], "GISCenter",
                     center.x, center.y, center.z );
}

/*----------------------------------------------------------------------
| Rotates the globe on the thumbnail map to show the same place as the
| detailed map.
*/
void SyncGlobeWithDetailedMap( char * origin, char * message )
{
   GlgPoint center, globe_center;
   
   UpdateMapWithMessage( 1, message );

   /* Sync up only if the detailed map (origin == "Map") is rotated,  
      not the thumbnail map (origin == "TopMap").
      */
   if( origin && strcmp( origin , "Map" ) == 0 )
   {
      /* Get the center of the detailed map. */
      GlgGetGResource( GISObject[ 1 ], "GISCenter",
                      &center.x, &center.y, &center.z );
      
      /* Get the center of the thumbnail globe. */
      GlgGetGResource( GISObject[ 0 ], "GISCenter",
                      &globe_center.x, &globe_center.y, &globe_center.z );
      
      /* Sync up if centers differ. */
      if( globe_center.x != center.x || globe_center.y != center.y ||
         globe_center.z != center.z )
      {
         /* Rotate the thumbnail globe to show the same area */
         GlgSetGResource( GISObject[ 0 ], "GISCenter",
                         center.x, center.y, center.z );      
         
         UpdateMapWithMessage( 0, message );
      }
   }
   else
     /* Don't sync when the globe on the thumbnail map is rotated
        (origin == "TopMap"), to allow moving it separately. Just
        update the selected area display. */
     UpdateMapWithMessage( 0, message );
}

/*----------------------------------------------------------------------
| Used to obtain coordinates of the mouse click.
*/
void Trace( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{      
   GlgTraceCBStruct * trace_data;
   GlgObject selection;
   GlgPoint
     point,
     lat_lon;
   double x, y;
   long
     event_type = 0,
     map;
   char * lat_lon_string;

   trace_data = (GlgTraceCBStruct*) call_data;

   /* Platform-specific code to extract event information. 
      GLG_COORD_MAPPING_ADJ is added to the cursor coordinates for 
      precise pixel mapping.
   */
#ifndef _WINDOWS
   switch( trace_data->event->type )
   {
    case ButtonPress:
      if( trace_data->event->xbutton.button != 1 )
	return;  /* Use the left button clicks only. */
      x = trace_data->event->xbutton.x + GLG_COORD_MAPPING_ADJ;
      y = trace_data->event->xbutton.y + GLG_COORD_MAPPING_ADJ;
      event_type = BUTTON_PRESS;
      break;
      
    case MotionNotify:
      x = trace_data->event->xmotion.x + GLG_COORD_MAPPING_ADJ;
      y = trace_data->event->xmotion.y + GLG_COORD_MAPPING_ADJ;
      event_type = MOUSE_MOVE;
      break;
      
    default: return;
   }
#else
   switch( trace_data->event->message )
   {
    case WM_LBUTTONDOWN:
      x = LOWORD( trace_data->event->lParam ) + GLG_COORD_MAPPING_ADJ;
      y = HIWORD( trace_data->event->lParam ) + GLG_COORD_MAPPING_ADJ;
      event_type = BUTTON_PRESS;
      break;
      
    case WM_MOUSEMOVE:
      x = LOWORD( trace_data->event->lParam ) + GLG_COORD_MAPPING_ADJ;
      y = HIWORD( trace_data->event->lParam ) + GLG_COORD_MAPPING_ADJ;
      event_type = MOUSE_MOVE;
      break;

    default: return;
   }
#endif
   
   /* Use the Map area events only. */
   if( trace_data->viewport == Map[ 0 ] )
     map = 0;
   else if( trace_data->viewport == Map[ 1 ] )
     map = 1;
   else
   {
      if( event_type == MOUSE_MOVE )
      {
         /* Erase the current postion display when the mouse moves outside 
            of the map. */
         GlgSetSResource( PositionObject, NULL, "" );
         GlgUpdate( PositionArea );
      }
      return;
   }

   switch( event_type )
   {
    case MOUSE_MOVE:   /* Report lat/lon position under the mouse. */
      point.x = x;
      point.y = y;
      point.z = 0.;

      /* Converts X/Y to lat/lon using GIS object's current projection,
	 handles both maps. */
      GlgGISConvert( GISObject[ map ], NULL, GLG_SCREEN_COORD,
		    /* X/Y to Lat/Lon */ True, &point, &lat_lon );

      lat_lon_string = CreateLocationString( lat_lon.x, lat_lon.y, lat_lon.z );
      GlgSetSResource( PositionObject, NULL, lat_lon_string );
      GlgFree( lat_lon_string );

      GlgUpdate( PositionArea );
      break;

    case BUTTON_PRESS:
      if( ZoomToMode() )
        return; /* ZoomTo or dragging mode in progress: pass it through. */

      /* Handle paning: set the new map center to the location of the click. */
      if( PanMode )
      {
         PanMode = False;
         
         point.x = x;
         point.y = y;
         point.z = 0.;
         
         /* Converts X/Y to lat/lon using GIS object's current projection,
            handles clicks on either map. */
         GlgGISConvert( GISObject[ map ], NULL, GLG_SCREEN_COORD,
                       /* X/Y to Lat/Lon */ True,
                       &point, &lat_lon );
         
         if( MapProjection[ 0 ] == GLG_ORTHOGRAPHIC_PROJECTION )
         {
            /* For the orthographic projection, Pan/Rotate the globe on the 
               thumbnail map as well. Don't do anything for the rectangular 
               projection: the whole world is already displayed.
               */
            GlgSetGResource( GISObject[ 0 ], "GISCenter", 
                            lat_lon.x, lat_lon.y, lat_lon.z );
            UpdateMapWithMessage( 0, "Panning, please wait..." );
            GlgUpdate( Map[ 0 ] );
         }

         /* Pan the detailed map. */
         GlgSetGResource( GISObject[ 1 ], "GISCenter", 
                         lat_lon.x, lat_lon.y, lat_lon.z );
         UpdateMapWithMessage( 1, "Panning the map..." );

         /* Disable locking: we scrolled the map to see a different area. */
         SetLocking( False );
         GlgUpdate( Drawing );
      }
      else
        /* Not a pan mode: start dragging the map with the mouse. */
        GlgSetZoom( Map[ map ], NULL, 's', 0. );
      break;

    default: return;
   } 
}

/*----------------------------------------------------------------------
| Adjust selected region on the thumbnail map to match detailed map.
*/
void SetSelectedArea()
{
   GlgObject
     rect,
     point_obj;
   GlgPoint
     extent,
     lat_lon[ 16 ];
   long i;

   /* Set the coordinates of the SelectedArea polygon. */
   rect = GlgGetResourceObject( GISObject[ 0 ], "GISArray/SelectedArea" );
      
   GetExtentDegrees( &extent, 1 );

   if( extent.x >= 120. )
   {
      /* Big area: don't need to show. */
      GlgSetDResource( rect, "Visibility", 0. );
   }
   else
   {
      GlgSetDResource( rect, "Visibility", 1. );

      /* Get lat/lon of the visible area of the detailed map. 
         Use 16 points for better precision, since the area is not 
         rectangular in the orthographic projection used for the thumbnail 
         globe. */ 
      GetLatLon( -1000., -1000., 1, &lat_lon[ 0 ] );
      GetLatLon( -1000.,  -500., 1, &lat_lon[ 1 ]  );
      GetLatLon( -1000.,     0., 1, &lat_lon[ 2 ]  );
      GetLatLon( -1000.,   500., 1, &lat_lon[ 3 ]  );
      GetLatLon( -1000.,  1000., 1, &lat_lon[ 4 ]  );
      GetLatLon(  -500.,  1000., 1, &lat_lon[ 5 ]  );
      GetLatLon(     0.,  1000., 1, &lat_lon[ 6 ]  );
      GetLatLon(   500.,  1000., 1, &lat_lon[ 7 ]  );
      GetLatLon(  1000.,  1000., 1, &lat_lon[ 8 ]  );
      GetLatLon(  1000.,   500., 1, &lat_lon[ 9 ]  );
      GetLatLon(  1000.,     0., 1, &lat_lon[ 10 ]  );
      GetLatLon(  1000.,  -500., 1, &lat_lon[ 11 ]  );
      GetLatLon(  1000., -1000., 1, &lat_lon[ 12 ]  );
      GetLatLon(   500., -1000., 1, &lat_lon[ 13 ]  );
      GetLatLon(     0., -1000., 1, &lat_lon[ 14 ]  );
      GetLatLon(  -500., -1000., 1, &lat_lon[ 15 ]  );

      for( i=0; i<16; ++i )
      {
         /* Get polygon's point */
         point_obj = GlgGetElement( rect, i );

         /* Set point's lat/lon */
         GlgSetGResource( point_obj, NULL, 
                         lat_lon[ i ].x, lat_lon[ i ].y, lat_lon[ i ].z );
      }
   }
}

/*----------------------------------------------------------------------
| FOR FLIGHT SIMULATION ONLY: Starts simulation for a plane, 
|     assigns its start and end nodes.
*/
void StartPlane( PlaneData * plane, long init )
{
   long
     i,
     to_index,
     from_index;
   char 
     buffer[ 2000 ],
     * label;

   if( NumNodes < 2 )
     error( "Less then two nodes: can't start planes.", True );

   from_index = (long) GlgRand( 0., NumNodes - 0.001 );
   do
   {
      to_index = (long) GlgRand( 0., NumNodes - 0.001 );
   } while( to_index == from_index );

   plane->from_node = &NodeArray[ from_index ];
   plane->to_node = &NodeArray[ to_index ];
   plane->flight_number = (int) GlgRand( 101., 1999. );
   plane->speed = GlgRand( 0.4, 1. );   /* Vary plane speed */

   if( init )
   {
      plane->path_position = GlgRand( 0.1, 0.2 );
      plane->path_position_last = plane->path_position - 0.05;  /* For angle */
   }
   else
   {
      plane->path_position = 0.;
      plane->path_position_last = 0.;
   }

   GlgFree( plane->tooltip[ 0 ] );
   GlgFree( plane->tooltip[ 1 ] );

   plane->tooltip[ 0 ] = 
     GlgCreateIndexedName( "Flight ", plane->flight_number );
   
   /* On the detailed map, add from/to node info to the tooltip. */
   sprintf( buffer, "%.500s from %.500s to %.500s", plane->tooltip[ 0 ],
	   plane->from_node->name, plane->to_node->name ); 

   plane->tooltip[ 1 ] = GlgStrClone( buffer );

   /* Set the tooltip */
   for( i = 0; i < 2; ++i )
     GlgSetSResource( plane->graphics[ i ], "TooltipString", 
                      plane->tooltip[ i ] );

   /* On detailed map[1], show the flight number as icon label */
   label = GlgCreateIndexedName( "Flight ", plane->flight_number );
   GlgSetSResource( plane->graphics[ 1 ], "LabelString", label );
   GlgFree( label );

   GetPlaneLatLon( plane );   /* Set the plane's initial position. */

   /* Stop tracking the selected flight when it reaches destination. */
   if( plane == SelectedPlane )
     SetLocking( False );
}

/*----------------------------------------------------------------------
| FOR FLIGHT SIMULATION ONLY: Calculates plane's lat/lon using simulated data.
|     In an application, it will query the current plane position.
|
| The simulation moves the plane from the start to the end node/city
| as controlled by the path_position parameter. The path_position changes
| in the range from from 0 (start node) to 1 (end node).
*/
void GetPlaneLatLon( PlaneData * plane )
{
   plane->lat_lon.x = 
     RELATIVE_TO_NEW_RANGE( plane->from_node->lat_lon.x, 
			   plane->to_node->lat_lon.x, plane->path_position );
   plane->lat_lon.y = 
     RELATIVE_TO_NEW_RANGE( plane->from_node->lat_lon.y, 
			   plane->to_node->lat_lon.y, plane->path_position );
}

/*----------------------------------------------------------------------
| FOR FLIGHT SIMULATION ONLY: Select some plane on the initial appearance.
*/
void InitSelection()
{
   /* Select the first plane */   
   SelectedPlane = &PlaneArray[ 0 ];
   SelectPlane( SelectedPlane, 1 );

   /* Lock on the selected plane, pan the map to keep it visible. */
   SetLocking( True );

   /* Center lon of the thumbnail map on the US initially. */
   if( MapProjection[ 0 ] == GLG_ORTHOGRAPHIC_PROJECTION )
     GlgSetGResource( GISObject[ 0 ], "GISCenter", -95.35, 20., 0. );
}

/*----------------------------------------------------------------------
| UTILITY FUNCTION: Calculates an angle between the line defined by two 
| points and the X axis.
*/
double GetAngle( GlgPoint * pt1, GlgPoint * pt2 )
{
   double length, angle;

   length = GetLength( pt1, pt2 );

   if( !length )
     angle = 0.;
   else
   {
      angle = acos( ( pt2->x - pt1->x ) / length );

      if( pt2->y - pt1->y < 0. )  /* ScreenSpace Z axis points to the user. */
	angle = - angle;
   }

   return RadToDeg( angle );
}

/*----------------------------------------------------------------------
| UTILITY FUNCTION: Calculates a distance between two points in 2D.
*/
double GetLength( GlgPoint * pt1, GlgPoint * pt2 )
{
   return sqrt( ( pt2->x - pt1->x ) * ( pt2->x - pt1->x ) +
	       ( pt2->y - pt1->y ) * ( pt2->y - pt1->y ) );
}

/*----------------------------------------------------------------------
| Turns plane icons' labels ON or OFF on the detailed map.
*/
void SetPlaneLabels( long on )
{
   GlgObject label;

   label = GlgGetResourceObject( PlaneArray[ 0 ].graphics[ 1 ], "Label" );
   if( label )
     GlgSetDResource( label, "Visibility", on ? 1. : 0. );
}

/*----------------------------------------------------------------------
| Sets locking mode ON or OFF. If locking is ON, the map is automatically
| scrolled to keep the selected plane icon in view.
*/
void SetLocking( long lock )
{
   LockSelectedPlane = lock;
}

/*----------------------------------------------------------------------
| Displays the selected plane's location and locking status.
*/
void UpdateSelectedPlaneStatus()
{
   char
     buffer[ 1002 ],
     * message,
     * lat_lon_string;

   if( SuspendPromptUpdates )
     return;

   if( SelectedPlane )
   {
      lat_lon_string = 
        CreateLocationString( SelectedPlane->lat_lon.x,
                             SelectedPlane->lat_lon.y, 1. );
                             
      if( LockSelectedPlane )
        sprintf( buffer, "Map is locked on selected Flight %d, %s", 
                SelectedPlane->flight_number, lat_lon_string );
      else
        sprintf( buffer, "Selected Flight %d, %s", 
                SelectedPlane->flight_number, lat_lon_string );

      GlgFree( lat_lon_string );
      message = buffer;
   }
   else
     message = "Click on the plane icon to select.";
   GlgSetSResource( Drawing, "StatusLabel/String", message );
}

/*----------------------------------------------------------------------
| Displays a message in the status area.
*/
void SetStatus( char * message )
{
   GlgSetSResource( Drawing, "StatusLabel/String", message );
   GlgUpdate( GlgGetResourceObject( Drawing, "StatusArea" ) );
}

/*----------------------------------------------------------------------
| Centers the map on the selected plane, used when locking mode is ON.
*/
void CenterOnPlane( PlaneData * plane, long map )
{      
   /* Center the map on the plane */
   GlgSetGResource( GISObject[ map ], "GISCenter", 
                   plane->lat_lon.x, plane->lat_lon.y, 0. );
}

/*----------------------------------------------------------------------
| Highlights the selected plane on both maps by changing its 
| SelectedIndex value.
*/
void SelectPlane( PlaneData * plane, long selected )
{
   long i;

   for( i=0; i < 2; ++i )
     if( plane->graphics[ i ] )
       GlgSetDResource( plane->graphics[ i ], "SelectedIndex",
		       (double)selected );
}

/*----------------------------------------------------------------------
|
*/
void SetPlaneSize()
{
   GlgObject resource;
   int i;

   for( i=0; i<2; ++i )
     if( resource = GlgGetResourceObject( PlaneTemplate[ i ], "IconScale" ) )
       GlgSetDResource( resource, NULL, PlaneSize );
}

/*----------------------------------------------------------------------
| Toggle resource between 0 and 1.
*/
void ToggleResource( GlgObject object, char * res_name )
{
   double value;

   GlgGetDResource( object, res_name, &value );
   GlgSetDResource( object, res_name, value != 0. ? 0. : 1. );
}

/*----------------------------------------------------------------------
| Toggle map layers: airport/city labels and states.
*/
void SetGISLayers( long map )
{
   char * layers, * new_layers;

   /* Airport labels should be visible only when city labels are off. */
   GlgSetDResource( NodeTemplate[1], "Label/Visibility", !CityLabels );

   layers = GlgStrClone( "default_gis" );

   /* Add city layers if they are on on the detailed map. */
   if( map == 1 )
   {
      if( CityLabels )
        new_layers = GlgConcatStrings( layers, ",us_cities" );
      else
        new_layers = GlgConcatStrings( layers, ",-us_cities" );
      GlgFree( layers );
      layers = new_layers;
   }

   if( StateDisplay )   /* Add states layer if it is on. */
     /* Enable states regardless of the default. */
     new_layers = GlgConcatStrings( layers, ",states" );
   else
     /* Disable state outline display. */
     new_layers = GlgConcatStrings( layers, ",-states" );
   GlgFree( layers );
   layers = new_layers;

   GlgSetSResource( GISObject[ map ], "GISLayers", layers );
   GlgFree( layers );
}

/*----------------------------------------------------------------------
| Convenience wrapper
*/
void GetLatLon( double x, double y, long map, GlgPoint * lat_lon )
{
   GlgPoint xy;

   xy.x = x;
   xy.y = y;
   xy.z = 0.;
   GlgGISConvert( GISObject[ map ], NULL, GLG_OBJECT_COORD,
		 /* X/Y to Lat/Lon */ True,
		 &xy, lat_lon );
}

/*----------------------------------------------------------------------
| Generate a location info string by converting +- signs info into the
| N/S, E/W suffixes.
*/
char * CreateLocationString( double lon, double lat, double z )
{
   long
     x_deg, y_deg,
     x_min, y_min,
     x_sec, y_sec;
   char
     char_x,
     char_y,
     buffer[ 100 ];

   if( z < 0. )
     return GlgStrClone( "" );

   if( lon < 0. )
   {
      lon = -lon;
      char_x = 'W';
   }
   else if( lon >= 360. )
   {
      lon -= 360.;
      char_x = 'E';
   }
   else if( lon >= 180. )
   {
      lon = 180. - ( lon - 180. );
      char_x = 'W';
   }
   else
     char_x = 'E';

   if( lat < 0. )
   {
      lat = -lat;
      char_y = 'S';
   }
   else
     char_y = 'N';

   x_deg = lon;
   x_min = ( lon - x_deg ) * 60.;
   x_sec = ( lon - x_deg - x_min / 60. ) * 3600.;

   y_deg = lat;
   y_min = ( lat - y_deg ) * 60.;
   y_sec = ( lat - y_deg - y_min / 60. ) * 3600.;

   sprintf( buffer, 
	   "Lon=%ld.%.2ld'%.2ld\"%c  Lat=%ld.%.2ld'%.2ld\"%c",
	   x_deg, x_min, x_sec, char_x, y_deg, y_min, y_sec, char_y ); 
   
   return GlgStrClone( buffer );
}

/*----------------------------------------------------------------------
| Turn plane labels on or off depending on the zoom level and adjust 
| plane icon size.
*/
void HandleZoomLevel()
{
   GlgPoint extent;
   long high_zoom;

   GetExtentDegrees( &extent, 1 );

   high_zoom = ( extent.x < 100. && extent.y < 50. );
   
   SetPlaneLabels( high_zoom );     /* Plane labels */ 

   CityLabels = True;
   SetGISLayers( 1 );     /* Set airport labels. */

   /* Plane icons size */
   PlaneSize = ( high_zoom ? MEDIUM_SIZE : SMALL_SIZE );
   SetPlaneSize();
}

/*----------------------------------------------------------------------
| Gets extent in lat/lon.
| For the ortho projection, roughly converts from meters to lat/lon.
*/
void GetExtentDegrees( GlgPoint * extent, long map )
{   
   GlgGetGResource( GISObject[ map ], "GISExtent",
		   &extent->x, &extent->y, &extent->z );
   if( MapProjection[ map ] == GLG_ORTHOGRAPHIC_PROJECTION )
   {
      extent->x = extent->x / GLG_EQUATOR_RADIUS * 90.;
      extent->y = extent->y / GLG_POLAR_RADIUS * 90.;
   }
}

/*----------------------------------------------------------------------
| Zoom to the US area after a few seconds to show details.
*/
void ZoomToUSStart( GlgAnyType data, GlgIntervalID * id )
{
   GlgObject florida_message;

   GlgSetDResource( Drawing, "Map/USZoomingMessage/Visibility", 1. );
   GlgUpdate( Drawing );

   /* GISExtent is in lon/lat for the rectangular GIS projection, and in 
      meters for the orthographic projection. To find proper values, zoom 
      the map in the GlgBuilder and copy the GISExtent values.
   */

   /* Zoom to the US boundaries on detailed map. */
   GlgSetGResource( GISObject[ 1 ], "GISCenter", -95.35, 37.37, 0. );
   GlgSetGResource( GISObject[ 1 ], "GISExtent", 69.71, 34.85, 0. );

   if( MapProjection[ 0 ] == GLG_ORTHOGRAPHIC_PROJECTION )
     /* Rotate thumbnail globe too to show the same location. */
     GlgSetGResource( GISObject[ 0 ], "GISCenter", -95.35, 37.37, 0. );

   HandleZoomLevel();

   UpdateMapWithMessage( 1, "Zooming, please wait..." );
   UpdateMapWithMessage( 0, "Zooming, please wait..." );

   /* Reorder US zoom message to top, otherwise airplane icons 
      would be flying on top of it. */
   florida_message =
     GlgGetResourceObject( Drawing, "Map/USZoomingMessage" );
   GlgReorderElement( Map[1], GlgGetIndex( Map[1], florida_message ), 
                     GlgGetSize( Map[1] ) - 1 );

   GlgUpdate( Drawing );

   GlgAddTimeOut( AppContext, USZoomDelay2, 
                 (GlgTimerProc)ZoomToUSEnd, NULL );
}

/*----------------------------------------------------------------------
| Remove the US zooming message after a few seconds.
*/
void ZoomToUSEnd( GlgAnyType data, GlgIntervalID * id )
{
   GlgSetDResource( Drawing, "Map/USZoomingMessage/Visibility", 0. );
   GlgUpdate( Drawing );
}

/*----------------------------------------------------------------------
|
*/
void error( char * string, GlgBoolean quit )
{
   GlgError( GLG_USER_ERROR, string );
   if( quit )
     exit( GLG_EXIT_ERROR );
}
