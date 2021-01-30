#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "GlgApi.h"
#include "gis_demo.h"

GlgObject
  Drawing,     
  NodeTemplate[ 2 ],
  Map[ 2 ],
  GISObject[ 2 ],
  NodeGroup[ 2 ];

GlgPoint
  InitExtent[ 2 ],    /* Store initial extent and center, used to reset */
  InitCenter[ 2 ];

long
  NumNodes,
  MapProjection[ 2 ],
  PanMode = False;

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

GlgAppContext AppContext;   /* Global, used to install a work procedure. */

#include "GlgMain.h"    /* Cross-platform entry point. */

/*----------------------------------------------------------------------
|
*/
int GlgMain( argc, argv, app_context )
     int argc;
     char *argv[];
     GlgAppContext app_context;
{
   char * message = "Loading map, please wait....";

   AppContext = GlgInit( False, app_context, argc, argv );

   NumNodes = sizeof( NodeArray ) / sizeof( NodeArray[ 0 ] );

   GlgSetDResource( (GlgObject)0, "$config/GlgMouseTooltipTimeout", 0.05 );

   Drawing = GlgLoadWidgetFromFile( "gis_demo.g" );
   if( !Drawing )
     error( "Can't load drawing file.", True );

   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( Drawing, "Point1", -500., -500., 0. );
   GlgSetGResource( Drawing, "Point2",  500.,  500., 0. );

   /* Get IDs of the two map viewports. */
   Map[0] = GlgGetResourceObject( Drawing, "TopMap" );   /* Thumbnail map */
   Map[1] = GlgGetResourceObject( Drawing, "Map" );      /* Detailed map */

   /* Display thumbnail map in a separate window. It is kept as a child window
      in the drawing for the convinience of editing. */
   GlgSetDResource( Map[ 0 ], "ShellType", (double) GLG_DIALOG_SHELL );
   GlgSetSResource( Map[ 0 ], "Screen/ScreenName", "Thumbnail View" );

   GlgAddCallback( Drawing, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );
   GlgAddCallback( Drawing, GLG_TRACE_CB, (GlgCallbackProc)Trace, NULL );

   Init();

   GlgSetupHierarchy( Drawing );

   /* Position icons before showing them. */
   UpdateObjectsOnMap( 0, message );
   UpdateObjectsOnMap( 1, message );

   GlgUpdate( Drawing );

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
   long i, j;

   for( i=0; i<2; ++i )   /* For each map window */
   {
      /* Get IDs of the GIS map objects in each of the map viewports. */
      GISObject[ i ] = GlgGetResourceObject( Map[ i ], "GISObject" );

      /* Query and store the GIS projection (ORTHOGRAPHIC or RECTANGULAR)
	 used to render the map. */ 
      GlgGetDResource( GISObject[ i ], "GISProjection", &projection );
      MapProjection[ i ] = projection;

      /* Set GIS Zoom mode: generate a new map request for a new area on 
	 zoom/pan. */
      GlgSetZoomMode( Map[ i ], NULL, GISObject[ i ], NULL, GLG_GIS_ZOOM_MODE );

      /* Store initial map extent for resetting after zooming. */
      GlgGetGResource( GISObject[ i ], "GISExtent",
		      &InitExtent[i].x, &InitExtent[i].y, &InitExtent[i].z );
      GlgGetGResource( GISObject[ i ], "GISCenter",
		      &InitCenter[i].x, &InitCenter[i].y, &InitCenter[i].z );
   }

   /* Get the palette containing templates for plane and node icons.
      Only node templates are used in this simpler version of the gis demo.
      */
   palette = GlgGetResourceObject( Drawing, "Palette" );

   /* Reference the palette to keep it around and delete it from the drawing */
   GlgReferenceObject( palette );
   GlgDeleteThisObject( Drawing, palette );

   /* Get node from the palette. Two sets of templates are used: smaller icons
      for the thumbnail view and more elaborate ones for the detailed map.
      */
   for( i=0; i<2; ++i )
     NodeTemplate[ i ] = 
       GlgGetResourceObject( palette, i == 0 ? "Node1" : "Node2" );      

   /* Add node icons to both thumbnail and detailed map. */
   for( i=0; i<2; ++i )
     CreateNodeIcons( i );    /* Add city icons */

   /* Selected area annotates the currently viewed area of the detailed map 
      in the thumbnail map view. Reorder SelectedArea on top of icons
      (last in the array). */
   selected_area = GlgGetResourceObject( Map[ 0 ], "SelectedArea" );
   GlgReorderElement( Map[ 0 ], GlgGetIndex( Map[ 0 ], selected_area ), 
		     GlgGetSize( Map[ 0 ] ) - 1 );

   InitSelection();  /* Zoom on the US. */
}

/*----------------------------------------------------------------------
| Creates NodeGroup to hold city/node icons and adds the icons to it.
*/
void CreateNodeIcons( long map )
{
   long i;

   NodeGroup[ map ] = 
     GlgCreateObject( GLG_GROUP, "NodeGroup", NULL, NULL, NULL, NULL );

   /* Add city/node icons */
   for( i = 0; i < NumNodes; ++i )
     AddNode( &NodeArray[ i ], map, i );
   
   GlgAddObjectToBottom( Map[ map ], NodeGroup[ map ] );
}

/*----------------------------------------------------------------------
| Adds a city icon, fills labels, tooltips, etc.
*/
void AddNode( NodeData * node_data, long map, long index )
{      
   GlgObject node;
   char
     * tooltip,
     buffer[ 1002 ];

   /* Create a copy of a node. */
   node = GlgCloneObject( NodeTemplate[ map ], GLG_STRONG_CLONE );

   GlgSetSResource( node, "Name", node_data->name );   /* Object name */

   /* Index for direct access */     
   GlgSetDResource( node, "DataIndex", (double)index );

   if( map == 1 )   /* On detailed map, show node name */     
     GlgSetSResource( node, "LabelString", node_data->name );

   if( map == 0 )   /* On thumbnail-view, show node names only. */
     tooltip = node_data->name;
   else   /* On detailed map, include lat/lon display into the tooltip. */
   {
      sprintf( buffer, "%.500s lat= %lf lon= %lf", 
	      node_data->name, node_data->lat_lon.y, node_data->lat_lon.x );
      tooltip = buffer;
   }
   GlgSetSResource( node, "TooltipString", tooltip );
	
   node_data->graphics[ map ] = node;

   /* The node will be positioned after the GIS object is setup.
      PositionNode( node_data, map );  
      */

   GlgAddObjectToBottom( NodeGroup[ map ], node );
}

/*----------------------------------------------------------------------
| 
*/
void PositionNode( NodeData * node, long map )
{
   if( !node->graphics[ map ] )
     return;

   /* Converts node position from lat/lon to GLG coordinates. */
   GetNodePosition( node, map );

   /* Update node's icon in the drawing */
   GlgSetGResource( node->graphics[ map ], "Position",
		   node->adj_xyz[ map ].x, node->adj_xyz[ map ].y, 0. );
}

/*----------------------------------------------------------------------
| Converts node's position from lat/lon to X/Y in GLG world coordinates.
*/
void GetNodePosition( NodeData * node, long map )
{
   /* Converts lat/lon to X/Y using GIS object's current projection. */
   GlgGISConvert( GISObject[ map ], NULL, GLG_OBJECT_COORD,
		 /* Lat/Lon to XY */ False,
		 &node->lat_lon, &node->xyz[ map ] );

   /* Prevent wrap-around errors under big zoom factors. Also handles
      visibility of hidden nodes on another side of the globe
      (z < 0 in ORTHOGRAPHIC projection ).
      */
   if( node->xyz[ map ].z < 0. || !GetVisibility( &node->xyz[ map ], 1.1 ) )
   {
      /* Not visible. Use smaller coordinates just outside the visible area
	 to prevent wrap-around errors. We could remove the node from the 
	 drawing and show only the visible ones. */
      node->adj_xyz[ map ].x = 2000.;
      node->adj_xyz[ map ].y = 2000.;
   }
   else
     node->adj_xyz[ map ] = node->xyz[ map ];
}

/*----------------------------------------------------------------------
| Checks if the object is visible in the current zoom region.
| This prevents wrap-around errors under big zoom factors.
*/
long GetVisibility( GlgPoint * position, double adj )
{
   /* Use adj as a gap */
   return
     position->x > -1000. * adj && position->x < 1000. * adj &&
     position->y > -1000. * adj && position->y < 1000. * adj;
}

/*----------------------------------------------------------------------
| Reposition icons when a new map is displayed. 
*/
void UpdateObjectsOnMap( long map, char * message )
{
   long i;

   /* GlgSetupHierarchy causes the new map to be generated if necessary, 
      so display the wait message while the map is being generated.
      */
   SetStatus( message );

   /* Update the GIS object with new extent but don't draw it yet:
      we want to update objects on the map first.
      */
   GlgSetupHierarchy( Map[ map ] );

   for( i = 0; i < NumNodes; ++i )
     PositionNode( &NodeArray[ i ], map );

   SetStatus( "" );

   /* Perform this on zooming/paning of either map: may be a resize of just 
      one map */
   SetSelectedArea();
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
     * subaction,
     * layers;

   message_obj = (GlgObject) call_data;

   GlgGetSResource( message_obj, "Origin", &origin );
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "SubAction", &subaction );

   /* Handle window closing. */
   if( strcmp( format, "Window" ) == 0 )
   {
      if( strcmp( action, "DeleteWindow" ) == 0 )
	if( strcmp( origin, "TopMap" ) == 0 )
	{
	   /* Close top map window */
	   GlgSetDResource( Drawing, "TopMap/Visibility", 0. );
	   GlgUpdate( Drawing );	 
	   return;
	}
	else
	{
	   /* Closing main window: exit. */
	   exit( 0 );
	}
      return;
   }

   if( strcmp( format, "Button" ) == 0 )
   {
      if( strcmp( action, "Activate" ) != 0 )
	return;

      PanMode = False;
      if( strcmp( origin, "ZoomIn" ) == 0 )
	Zoom( 'i', 2. );
      else if( strcmp( origin, "ZoomOut" ) == 0 )
	Zoom( 'o', 2. );
      else if( strcmp( origin,  "ZoomReset" ) == 0 )
      {	
	 SetLabels( False );
	 Zoom( 'n', 0. );
      }
      else if( strcmp( origin, "ZoomTo" ) == 0 )
      {
	 PanMode = False;    /* Abort Pan mode */

	 GlgSetZoom( Map[ 1 ], NULL, 't', 0. );  /* Start Zoom op */
	 SetStatus( "Define a rectangular area to zoom to." );
      }
      else if( strcmp( origin, "Pan" ) == 0 )
      {	    
	 GlgSetZoom( Map[ 1 ], NULL, 'e', 0. );  /* Abort ZoomTo mode */

	 PanMode = True;
	 SetStatus( "Click to define a new center." );
      }
      else if( strcmp( origin, "Nodes" ) == 0 )
      {
	 ToggleResource( Map[ 0 ], "NodeGroup/Visibility" );
	 ToggleResource( Map[ 1 ], "NodeGroup/Visibility" );
      }
      else if( strcmp( origin, "ValueDisplay" ) == 0 )
      {
	 /* Visibility of all labels is constrained, set just one. */
	 ToggleResource( NodeArray[ 0 ].graphics[ 1 ], "Label/Visibility" );
      }
      else if( strcmp( origin, "ToggleStates" ) == 0 )
      {
	 GlgGetSResource( Map[ 1 ], "GISObject/GISLayers", &layers );
	 if( strcmp( layers, "default" ) == 0 )
	   layers = "default,states";   /* Enable state outline display */
	 else
	   layers = "default";          /* Disable states display */
	 GlgSetSResource( Map[ 1 ], "GISObject/GISLayers", layers );
      }
   }
   else if( strcmp( action, "Zoom" ) == 0 && strcmp( subaction, "End" ) == 0 )
   {
      GlgPoint center;

      /* Update icon positions after zooming. */
      UpdateObjectsOnMap( 1, "Zooming, please wait..." );
      GlgUpdate( Map[ 1 ] );

      /* Get the center of the detailed map. */
      GlgGetGResource( GISObject[ 1 ], "GISCenter",
		      &center.x, &center.y, &center.z );

      /* Rotate the thumbnail globe to show the same area */
      GlgSetGResource( GISObject[ 0 ], "GISCenter",
		      center.x, center.y, center.z );      
      
      /* Updates icons and selected area display. */
      UpdateObjectsOnMap( 0, "Zooming, please wait..." );
   }
   else if( strcmp( format, "CustomEvent" ) == 0 &&
	   strcmp( action, "MouseClick" ) == 0 )   /* Mouse click on an icon */
   {
      double zoom_mode, button_index;
      char * custom_event;

      GlgGetDResource( Map[ 1 ], "ZoomToMode", &zoom_mode );
      if( zoom_mode )
	return;  /* Don't handle selection in ZoomTo mode. */

      GlgGetDResource( message_obj, "ButtonIndex", &button_index );
      if( button_index != 1 )
	return;    /* Ignore middle and right mouse button clicks */

      GlgGetSResource( message_obj, "EventLabel", &custom_event );

      /* Do something with the selected object here. */
   }
   GlgUpdate( Drawing );
}

/*----------------------------------------------------------------------
|
*/
void Zoom( long type, double value )
{
   GlgPoint center, globe_center;

   switch( type )
   {
    default:
      GlgSetZoom( Map[ 1 ], NULL, type, value );
      UpdateObjectsOnMap( 1, "Zooming, please wait..." );

      /* After "1:1" zoom reset, the maps' centers differ, sync the centers
	 when zooming in the first time. */      
      if( type == 'i' )
      {
	 /* Get the center of the detailed map. */
	 GlgGetGResource( GISObject[ 1 ], "GISCenter",
			 &center.x, &center.y, &center.z );
      
	 /* Get the center of the thumbnail globe. */
	 GlgGetGResource( GISObject[ 0 ], "GISCenter",
			 &globe_center.x, &globe_center.y, &globe_center.z );
	 
	 /* First time: centers differ, sync up. */
	 if( globe_center.x != center.x || globe_center.y != center.y ||
	    globe_center.z != center.z )
	 {
	    /* Rotate the thumbnail globe to show the same area */
	    GlgSetGResource( GISObject[ 0 ], "GISCenter",
			    center.x, center.y, center.z );      
	    UpdateObjectsOnMap( 0, "Zooming, please wait..." );
	 }
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
	 UpdateObjectsOnMap( 0, "Reloading map, please wait..." );
      }

      /* Reset detailed map to initial extent. */
      GlgSetGResource( GISObject[ 1 ], "GISCenter",
		      InitCenter[1].x, InitCenter[1].y, InitCenter[1].z );
      GlgSetGResource( GISObject[ 1 ], "GISExtent", 
		      InitExtent[1].x, InitExtent[1].y, InitExtent[1].z );
      UpdateObjectsOnMap( 1, "Reloading map, please wait..." );

      /* Make selected area rectangle invisible when no zoom  */
      GlgSetDResource( Map[ 0 ], "SelectedArea/Visibility", 0. );
      break;
   }
}

/*----------------------------------------------------------------------
| Used to obtain coordinates of the mouse click.
*/
void Trace( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{      
   GlgTraceCBStruct * trace_data;
   GlgPoint
     point,
     lat_lon;
   long
     i,
     event_type = 0,
     x, y,
     map,
     width, height;

   trace_data = (GlgTraceCBStruct*) call_data;

   /* Use the Map area events only. */
   if( trace_data->viewport == Map[ 0 ] )
     map = 0;
   else if( trace_data->viewport == Map[ 1 ] )
     map = 1;
   else
     return;

   /* Platform-specific code to extract event information. */
#ifndef _WINDOWS
   switch( trace_data->event->type )
   {
    case ButtonPress:
      if( trace_data->event->xbutton.button != 1 )
	return;  /* Use the left button clicks only. */
      x = trace_data->event->xbutton.x;
      y = trace_data->event->xbutton.y;
      event_type = BUTTON_PRESS;
      break;
      
    default: return;
   }
#else
   switch( trace_data->event->message )
   {
    case WM_LBUTTONDOWN:
      x = LOWORD( trace_data->event->lParam );
      y = HIWORD( trace_data->event->lParam );
      event_type = BUTTON_PRESS;
      break;
      
    default: return;
   }
#endif
   
   switch( event_type )
   {
    case BUTTON_PRESS:
      /* Handle paning: set the new map center to the location of the click. */
      if( !PanMode )
	return;
      
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
	 /* Pan/Rotate globe on the thumbnail map as well. 
	    Don't do anything for the rectangular projection: the whole world
	    is displayed anyway. */
	 GlgSetGResource( GISObject[ 0 ], "GISCenter", 
			  lat_lon.x, lat_lon.y, lat_lon.z );
	 UpdateObjectsOnMap( 0, "Paning, please wait..." );
	 GlgUpdate( Map[ 0 ] );
      }

      /* Pan detailed map */
      GlgSetGResource( GISObject[ 1 ], "GISCenter", 
		       lat_lon.x, lat_lon.y, lat_lon.z );
      UpdateObjectsOnMap( 1, "Paning the map..." );
      GlgUpdate( Map[ 1 ] );
      break;

    default: return;
   } 
}

/*----------------------------------------------------------------------
| Show the area of the detailed map on the thumbnail map.
*/
void SetSelectedArea()
{
   GlgObject
     rect,
     point_obj[ 4 ];
   GlgPoint
     extent,
     lat_lon[ 4 ],
     point;
   long i;

   /* Set the coordinates of the SelectedArea polygon. */
   rect = GlgGetResourceObject( Map[ 0 ], "SelectedArea" );
      
   GlgGetGResource( GISObject[ 1 ], "GISExtent",
		   &extent.x, &extent.y, &extent.z );

   if( MapProjection[ 1 ] == GLG_ORTHOGRAPHIC_PROJECTION &&
      extent.x > 1.5 * GLG_POLAR_RADIUS &&
      extent.y > 1.5 * GLG_POLAR_RADIUS ||
      MapProjection[ 1 ] == GLG_RECTANGULAR_PROJECTION &&
      extent.x > 300. && extent.y > 130. )
   {
      /* Big area: don't need to show. */
      GlgSetDResource( rect, "Visibility", 0. );
   }
   else
   {
      GlgSetDResource( rect, "Visibility", 1. );

      /* Get polygon points */
      for( i=0; i<4; ++i )
	point_obj[ i ] = GlgGetElement( rect, i );

      /* Get lat/lon on detailed map */
      GetLatLon( -1000., -1000., 1, &lat_lon[ 0 ] );
      GetLatLon( -1000.,  1000., 1, &lat_lon[ 1 ]  );
      GetLatLon(  1000.,  1000., 1, &lat_lon[ 2 ]  );
      GetLatLon(  1000., -1000., 1, &lat_lon[ 3 ]  );

      for( i=0; i<4; ++i )
      {
	 /* Converts lat/lon on the detailed map to X/Y on the thumbnail map
	    using GIS object's current projection. */
	 GlgGISConvert( GISObject[ 0 ], NULL, GLG_OBJECT_COORD,
		       /* Lat/Lon to X/Y */ False,
		       &lat_lon[ i ], &point );
	 
	 GlgSetGResource( point_obj[ i ], NULL, point.x, point.y, point.z );
      }
   }

   GlgSetGResource( rect, "EdgeColor", 0.9, 0.9, 0.9 ); /* Light color */      

   GlgUpdate( Map[ 0 ] );
}

/*----------------------------------------------------------------------
| Zoom on the US on the initial appearance.
*/
void InitSelection()
{
   /* Zoom to the US boundaries on detailed map. */
   GlgSetGResource( GISObject[ 1 ], "GISCenter", -95.35, 37.37, 0. );
   GlgSetGResource( GISObject[ 1 ], "GISExtent", 69.71, 34.85, 0. );

   if( MapProjection[ 0 ] == GLG_ORTHOGRAPHIC_PROJECTION )
     /* Rotate thumbnail globe too to show the same location. */
     GlgSetGResource( GISObject[ 0 ], "GISCenter", -95.35, 37.37, 0. );

   /* Turn labels on on the detailed view: zoomed on US. */
   SetLabels( True );
}

/*----------------------------------------------------------------------
| Turns node icons' labels ON or OFF on the detailed map.
*/
void SetLabels( long on )
{
   GlgObject label;

   label = GlgGetResourceObject( NodeArray[ 0 ].graphics[ 1 ], "Label" );
   if( label )
     GlgSetDResource( label, "Visibility", on ? 1. : 0. );
}

/*----------------------------------------------------------------------
| Displays a message in the status area.
*/
void SetStatus( String message )
{
   GlgSetSResource( Drawing, "StatusLabel/String", message );
   GlgUpdate( GlgGetResourceObject( Drawing, "StatusArea" ) );
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
|
*/
void error( char * string, long quit )
{
   printf( "%s\n", string );
   if( quit )
     exit( 0 );
}

