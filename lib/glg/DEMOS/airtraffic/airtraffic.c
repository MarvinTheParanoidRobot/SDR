#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef _WINDOWS
#include "resource.h"
# pragma warning( disable : 4244 )
# pragma warning( disable : 4996 )    /* Allow cross-platform sscanf() */
#endif
#include "GlgApi.h"
#include "GlmApi.h"
#include "airtraffic.h"

/* Enables code that demonstrates how to change layer attributes 
   programmatically. */
#define CHANGE_LAYER_ATTRIBUTES_SAMPLE    0

#define MAX_NUM_PLANES      10000

double PlaneSpeed = 0.002;    /* Relative units */

#define SMALL_SIZE   1.
#define MEDIUM_SIZE  1.5
#define BIG_SIZE     2.

#define SMALL_MARKER_SIZE    7.
#define MEDIUM_MARKER_SIZE   9.
#define BIG_MARKER_SIZE     11.

#define NUM_NODE_TYPES     2
#define NUM_PLANE_TYPES    3

#define NORMAL     0
#define WARNING    1
#define ALARM      2
#define SELECTED   3

#define DEBUG_TIMER          0   /* Set to 1 to debug timer intervals */

GlgObject
  Drawing,     
  NodeTemplate[ NUM_NODE_TYPES ],
  PlaneTemplate[ NUM_PLANE_TYPES ],
  TrajectoryTemplate,
  NodePool[ NUM_NODE_TYPES ],
  PlanePool[ NUM_PLANE_TYPES ],
  TrajectoryPool,
  Map,
  FlightInfoPopup,
  GISObject,
  GISArray,
  NodeGroup,
  PlaneGroup,
  TrajectoryGroup,
  DistancePolygon = (GlgObject)0,
  PositionArea,
  PositionObject;

GlgPoint
  InitExtent,    /* Store initial extent and center, used to reset */
  InitCenter;

double PlaneSize = SMALL_SIZE;

char * Layers = NULL;

long
  UpdateInterval = 100,      /* Update interval in msec */
  FloridaZoomDelay1 = 2500,  /* Delay to zoom to Florida to show details. */
  FloridaZoomDelay2 = 2000,  /* Delay to remove the Florida zooming message. */
  NumPlanes = 100,
  NumNodes,
  NumTrajectoryPoints,
  InitialMapProjection,
  MapProjection,
  OrthoOnly = False,
  NoFill = False,
  DoUpdate = True,
  PanMode = False,
  DistanceMode = False,
  RedoIcons = True,        /* True to add icons the first time. */
  PlaneType = 0,
  NodeType = 0,
  HasAngle = False,
  HasElevation = False,
  SelectedPlaneIndex = -1,
  NumDistancePoints,
  CityLabels = True,
  StateDisplay = True,
  OpenGLEnabled;

PlaneData PlaneArray[ MAX_NUM_PLANES ];

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

GlgAppContext AppContext;   /* Global, used to install a timeout. */

#define RELATIVE_TO_NEW_RANGE( low, high, rel_value ) \
   ( (low) + ((high) - (low)) * rel_value )

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

#define RadToDeg( angle )   ( ( angle ) / M_PI * 180. )
#define DegToRad( angle )   ( ( angle ) /180. * M_PI )

#include "GlgMain.h"    /* Cross-platform entry point. */

/*----------------------------------------------------------------------
|
*/
int GlgMain( argc, argv, app_context )
     int argc;
     char *argv[];
     GlgAppContext app_context;
{   
   long skip;
   char * full_path;

   AppContext = GlgInit( False, app_context, argc, argv );

   /* Don't expand selection area for exact tooltips. */
   GlgSetDResource( (GlgObject)0, "$config/GlgPickResolution", 0. );

   for( skip = 1; skip < argc; ++skip )
   {
      if( strcmp( argv[ skip ], "-num_planes" ) == 0 )
      {
	 ++skip;
	 if( argc <= skip || sscanf( argv[ skip ], "%ld", &NumPlanes ) != 1 )
	   error( "No plane number.", True );
	 if( NumPlanes > MAX_NUM_PLANES )
	   error( "Increase plane array size and run again.", True ); 
      }
      else if( strcmp( argv[ skip ], "-speed" ) == 0 )
      {
	 ++skip;
	 if( argc <= skip || sscanf( argv[ skip ], "%lf", &PlaneSpeed ) != 1 )
	   error( "No plane speed.", True );
      }
      else if( strcmp( argv[ skip ], "-ortho" ) == 0 )
      {
	 OrthoOnly = True;
      }
      else if( strcmp( argv[ skip ], "-no_fill" ) == 0 )
      {
	 NoFill = True;
      }
      else if( strcmp( argv[ skip ], "-help" ) == 0 )
      {
	 printf( "Options: -num_planes <number> -speed <number> -ortho -no_fill\n" );
	 printf( "Defaults: -num_planes 100 -speed 0.005 \n" );
	 exit( GLG_EXIT_OK );
      }
      else if( strcmp( argv[ skip ], "-verbose" ) == 0 ||
	      strcmp( argv[ skip ], "-non_verbose" ) == 0 ||
	      strcmp( argv[ skip ], "-glg-enable-opengl" ) == 0 ||
	      strcmp( argv[ skip ], "-glg-disable-opengl" ) == 0 )
	;   /* Allow: handled by GLG. */
      else
        error( "Invalid option. Use -help for the list of options.", True );
   }

   GlgSetDResource( (GlgObject)0, "$config/GlgMouseTooltipTimeout", 0.05 );

   NumNodes = sizeof( NodeArray ) / sizeof( NodeArray[ 0 ] );

   full_path = GlgGetRelativePath( argv[0], "airtraffic.g" );
   Drawing = GlgLoadWidgetFromFile( full_path );
   if( !Drawing )
     error( "Can't load drawing file.", True );

   GlgFree( full_path );

   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( Drawing, "Point1", -700., -800., 0. );
   GlgSetGResource( Drawing, "Point2",  700.,  800., 0. );

   /* Setting the window title. */
   GlgSetSResource( Drawing, "ScreenName", "GLG Air Traffic Demo" );

   /* Get IDs of the map viewport and GIS object. */
   Map = GlgGetResourceObject( Drawing, "Map" );
   FlightInfoPopup = GlgGetResourceObject( Map, "FlightInfoPopup" );
   GISObject = GlgGetResourceObject( Map, "GISObject" );

   /* Get ID of the GIS Object's GISArray, which holds all icons displayed
      on top of the map in lat/lon coordinates.
      */
   GISArray = GlgGetResourceObject( GISObject, "GISArray" );

   GlgAddCallback( Drawing, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );
   GlgAddCallback( Drawing, GLG_TRACE_CB, (GlgCallbackProc)Trace, NULL );

   Init(); 

   GlgSetupHierarchy( Drawing );

   /* Adds visible icons to the drawing and handles zoom level. */
   UpdateObjectsOnMap( "Loading map, please wait...." );

   AdjustRendering();

#ifdef _WINDOWS            
   GlgLoadExeIcon( Drawing, IDI_ICON1 );
#endif

   GlgUpdate( Drawing );

   StartUpdate();   /* Install a timeout. */

   /* Zoom to Florida after a few seconds to show details. */
   GlgAddTimeOut( AppContext, FloridaZoomDelay1, 
                 (GlgTimerProc)ZoomToFloridaStart, NULL );

   return (int) GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| Initializes the drawing
*/
void Init()
{
   GlgObject palette;
   double
     projection,
     factor;
   long i;
   char * icon_name;

   /* Set GIS Zoom mode. It was set and saved with the drawing, but do it 
      again programmatically just in case.
      */
   GlgSetZoomMode( Map, NULL, GISObject, NULL, GLG_GIS_ZOOM_MODE );

   /* Store initial map extent and projection for reset action. */
   GlgGetGResource( GISObject, "GISExtent",
		      &InitExtent.x, &InitExtent.y, &InitExtent.z );
   GlgGetGResource( GISObject, "GISCenter" ,
		   &InitCenter.x, &InitCenter.y, &InitCenter.z );
   GlgGetDResource( GISObject, "GISProjection", &projection );
   InitialMapProjection = projection;
   MapProjection = projection;

   /* Make popup dialog and distance popup invisible */
   GlgSetDResource( FlightInfoPopup, "Visibility", 0. );
   GlgSetDResource( Map, "DistancePopup/Visibility", 0. );

   /* Get the palette containing templates for plane and node icons. */
   palette = GlgGetResourceObject( Drawing, "Palette" );

   /* Reference the palette to keep it around and delete it from the drawing */
   GlgReferenceObject( palette );
   GlgDeleteThisObject( Drawing, palette );

   /* Get node and plane templates from the palette. A few sets of templates
      with different level of details are used, depending on the zoom level.
      Palette aproach is used to implement icon types instead of subdrawings,
      since icons are kept in the icon pool and are dynamically added/deleted 
      from the drawing to show only the icons visible in the zoomed area.
      */
   for( i=0; i < NUM_PLANE_TYPES; ++i )
   {
      icon_name = GlgCreateIndexedName( "Plane", i );
      PlaneTemplate[ i ] = GlgGetResourceObject( palette, icon_name );
      GlgFree( icon_name );

      /* Turn labels on initially */
      if( GlgGetResourceObject( PlaneTemplate[ i ], "Label" ) )
	GlgSetDResource( PlaneTemplate[ i ], "Label/Visibility", 1. );
   }

   TrajectoryTemplate = GlgGetResourceObject( palette, "Trajectory" );
   GlgGetDResource( TrajectoryTemplate, "Factor", &factor );
   NumTrajectoryPoints = factor;
   
   for( i=0; i < NUM_NODE_TYPES; ++i )
   {
      icon_name = GlgCreateIndexedName( "Node", i );
      NodeTemplate[ i ] = GlgGetResourceObject( palette, icon_name );
      GlgFree( icon_name );
   }

   /* Initialize plane structures used for simulation. */
   for( i=0; i < NumPlanes; ++i )
   {
      PlaneArray[ i ].name = GlgCreateIndexedName( "", i );
      PlaneArray[ i ].tooltip = NULL;
      PlaneArray[ i ].color_index = NORMAL;
      PlaneArray[ i ].graphics = (GlgObject)0;
      PlaneArray[ i ].trajectory = (GlgObject)0;
      PlaneArray[ i ].iteration = 0;

      StartPlane( &PlaneArray[ i ], True );
   }

   /* Create groups to hold nodes and planes */
   NodeGroup = 
     GlgCreateObject( GLG_GROUP, "NodeGroup", NULL, NULL, NULL, NULL );
   TrajectoryGroup = 
     GlgCreateObject( GLG_GROUP, "TrajectoryGroup", NULL, NULL, NULL, NULL );
   PlaneGroup = 
     GlgCreateObject( GLG_GROUP, "PlaneGroup", NULL, NULL, NULL, NULL );

   /* Add all icons to the GIS object, so that the icon's position may be
      defined in lat/lon. The GIS object handles all details of the 
      GIS coordinate conversion.
      */
   GlgAddObjectToBottom( GISObject, NodeGroup );
   GlgAddObjectToBottom( GISObject, PlaneGroup );
   GlgAddObjectToBottom( GISObject, TrajectoryGroup );

   /* Create groups to keep pooled objects. */
   for( i=0; i<NUM_NODE_TYPES; ++i )
     NodePool[i] = GlgCreateObject( GLG_GROUP, NULL, NULL, NULL, NULL, NULL );

   for( i=0; i<NUM_PLANE_TYPES; ++i )
     PlanePool[i] = GlgCreateObject( GLG_GROUP, NULL, NULL, NULL, NULL, NULL );

   TrajectoryPool = GlgCreateObject( GLG_GROUP, NULL, NULL, NULL, NULL, NULL );

   SetPlaneSize();   /* Initial plane size. */

   /* Demos starts with the whole word view, then zooms to the Florida area
      in a few seconds to show more details. Set initial parameters for the 
      whole world view.
      */
   HandleZoomLevel();

   /* Store objects used to display lat/lon on mouse move. */
   PositionArea = GlgGetResourceObject( Drawing, "PositionArea" );
   PositionObject = GlgGetResourceObject( Drawing, "PositionLabel/String" );
   GlgSetSResource( PositionObject, NULL, "" );

   /* Set Florida zooming message to OFF initially. */
   GlgSetDResource( Drawing, "Map/FloridaZoomingMessage/Visibility", 0. );
}

/*----------------------------------------------------------------------
| Makes sure the icons visible at the current zoom level are added
| to the drawing, keeping the rest of the icons in the object pool.
| Also handles the use of more detailed icons depending on the zoom 
| level.
| The object pools are used to handle huge number (thousands and tens of 
| thousands) of icons efficiently. For a smaller number of icons, icons
| may always be kept in the drawing and this function is not needed.
*/
void UpdateObjectsOnMap( char * message )
{
   long i;

   /* GlgSetupHierarchy causes the new map to be generated if necessary.
      Display the wait message while the map is being generated.
      */
   SetStatus( message );

   if( RedoIcons )      /* Zoom or pan */
   {
      DeleteNodes();
      DeletePlanes();

      HandleZoomLevel();
   }

   /* Update the GIS object with a new extent but don't draw it yet:
      we want to position icons on the map first.
      */
   GlgSetupHierarchy( Map );

   for( i = 0; i < NumNodes; ++i )      /* Position nodes */
     PositionNode( &NodeArray[ i ], i );

   /* Add trajectories before positioning planes to setup trajectories' 
      history. */
   if( RedoIcons )
     GlgAddObjectToBottom( GISObject, TrajectoryGroup );

   for( i = 0; i < NumPlanes; ++i )     /* Position planes */
     PositionPlane( &PlaneArray[ i ], i );

   if( RedoIcons )
   {
      GlgAddObjectToBottom( GISObject, NodeGroup );
      GlgAddObjectToBottom( GISObject, PlaneGroup );
      RedoIcons = False;
   }

   SelectPlane( SelectedPlaneIndex );
   SetStatus( "" );
}

/*----------------------------------------------------------------------
| Set the node type depending on zoom level. Also change to rectangular
| projection for high zoom factors.
*/
void HandleZoomLevel()
{
   GlgPoint extent;

   GetExtentDegrees( &extent );

   if( extent.x < 20. && extent.y < 20. )
   {
      /* High Zoom: use the most detailed icon */
      NodeType = 1;
      PlaneType = 2;         /* Most detailed icon */
      HasAngle = True;       /* Most detailed plane icons show angle */
      HasElevation = True;   /* Most detailed plane icons show elevation */
      CityLabels = True;     /* Use city names instead of airport labels. */
      Layers = "default_air";
   }
   else if( extent.x < 70. && extent.y < 70. )
   {
      /* Zoom: use detailed icon 1 */
      NodeType = 1;
      PlaneType = 1;         /* Detailed icon */
      HasAngle = True;       /* Detailed plane icons show angle */
      HasElevation = False;  
      CityLabels = True;     /* Use city names instead of airport labels. */
      Layers = "default_air";
   }
   else
   {
      /* Whole world view */
      NodeType = 1;          /* City icons are always visible */
      PlaneType = 0;         /* Simple icon */
      HasAngle = False;
      HasElevation = False;
      CityLabels = False;   /* Use airport labels instead of all city names. */
      Layers = "default_gis,grid70,outline";
   }

   SetGISLayers();     /* Set airport labels. */

   if( !OrthoOnly )
     ChangeProjection( &extent );
}

/*----------------------------------------------------------------------
| Change projection to rectangular for zoomed views, and back to 
| orthographics for high-level views.
*/
void ChangeProjection( GlgPoint * extent )
{
   if( extent->x < 30. && extent->y < 30. )
   {
      if( MapProjection == GLG_RECTANGULAR_PROJECTION )
	return;    /* Already rect, no change */      

      /* Change to the rectangular projection */
      MapProjection = GLG_RECTANGULAR_PROJECTION;

      GlgSetDResource( GISObject, "GISProjection", 
		      (double) GLG_RECTANGULAR_PROJECTION );
      /* Set extent in degrees */
      GlgSetGResource( GISObject, "GISExtent", extent->x, extent->y, 0. );

#if CHANGE_LAYER_ATTRIBUTES_SAMPLE
      SetFillType( True );
#endif
   }
   else
   {
      if( MapProjection == GLG_ORTHOGRAPHIC_PROJECTION )
	return;    /* Already ortho, no change */

      /* Change to the orthographic projection */
      MapProjection = GLG_ORTHOGRAPHIC_PROJECTION;

      GlgSetDResource( GISObject, "GISProjection",
		      (double) GLG_ORTHOGRAPHIC_PROJECTION );
      
      /* Set extent in meters */
      GlgSetGResource( GISObject, "GISExtent", 
		      extent->x / 90. * GLG_EQUATOR_RADIUS,
		      extent->y / 90. * GLG_POLAR_RADIUS, 0. );

#if CHANGE_LAYER_ATTRIBUTES_SAMPLE
      SetFillType( False );
#endif
   }
}

#if CHANGE_LAYER_ATTRIBUTES_SAMPLE
/*----------------------------------------------------------------------
| Change fill type depending on the zoom level.
*/
void SetFillType( long fill )
{
   GlgObject dataset;

   if( NoFill )
     return;

   dataset = GlgGISGetDataset( GISObject, NULL );
   if( fill )
   {
      /* Set fill type */
      GlgSetDResource( dataset, "polbnd_polbnd/FillType",
		      (double) GLM_FILL_AND_EDGE );
      GlgSetGResource( dataset, "polbnd_polbnd/FillColor",
		      0.45, 0.45, 0.32 );
      GlgSetGResource( dataset, "polbnd_polbnd/EdgeColor",
		      0., 0., 0. );		      
      GlgSetGResource( dataset, "states_dcw/EdgeColor",
		      0., 0., 0. );
      GlgSetGResource( dataset, "grid/LabelEdgeColor",
		      1., 1., 1. );      
   }
   else
   {
      /* Unset fill type */
      GlgSetDResource( dataset, "polbnd_polbnd/FillType", (double) GLM_EDGE );
      GlgSetGResource( dataset, "polbnd_polbnd/EdgeColor", 
		      0.5, 0.5, 0.5 );
      GlgSetGResource( dataset, "states_dcw/EdgeColor",
		      0.6, 0.6, 0.6 );
      GlgSetGResource( dataset, "grid/LabelEdgeColor",
		      0.4, 0.4, 0.4 );      
   }
}
#endif

/*----------------------------------------------------------------------
| Handle user interaction.
*/
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{
   GlgObject message_obj;
   long i;
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

   /* Handle window closing. Could use viewport's name if there is more than
      one top viewport. */
   if( strcmp( format, "Window" ) == 0 &&
       strcmp( action, "DeleteWindow" ) == 0 )
     exit( GLG_EXIT_OK );

   if( strcmp( format, "Button" ) == 0 )         /* Handle button clicks */
   {
      if( strcmp( action, "Activate" ) != 0 )
	return;

      PanMode = False;    /* Abort Pan mode */
      AbortDistanceMode();

      if( strcmp( origin, "CloseDialog" ) == 0 )
      {
	 GlgSetDResource( Drawing, "SelectionDialog/Visibility", 0. );
	 GlgUpdate( Drawing );	 
      }
      else if( strcmp( origin, "ToggleColor" ) == 0 )
      {
	 ToggleColor();
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
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "ZoomOut" ) == 0 )
      {
	 Zoom( 'o', 2. );
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin,  "ZoomReset" ) == 0 )
      {
	 Zoom( 'n', 0. );
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "ZoomTo" ) == 0 )
      {
	 GlgSetZoom( Map, NULL, 't', 0. );  /* Start Zoom op */
	 SetStatus( "Define a rectangular area to zoom to." );
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "Pan" ) == 0 )
      {	    
	 GlgSetZoom( Map, NULL, 'e', 0. );  /* Abort ZoomTo/Drag mode */

	 PanMode = True;
	 SetStatus( "Click to define a new center." );
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "Drag" ) == 0 )
      {
         /* Activate dragging mode. Dragging will start on the mouse click. 
            If no object of interest is selected by the mouse click, 
            dragging will be started by the code in the Trace callback 
            anyway. The "Drag" button demostrates an alternative way 
            to start dragging from a button.
            */
	 GlgSetZoom( Map, NULL, 's', 0. );
	 SetStatus( "Click and drag the map with the mouse." );
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "AirportLabels" ) == 0 )
      {
         CityLabels = !CityLabels;
         SetGISLayers();
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "Planes" ) == 0 )
      {
	 ToggleResource( Map, "PlaneGroup/Visibility" );
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "ValueDisplay" ) == 0 )
      {
	 if( PlaneType == 0 )
	 {
	    GlgBell( viewport );
	    SetStatus( "Zoom in to see plane labels." );
	 }
	 else
	 {
	    /* Visibility of all labels is constrained, set just one. */
	    for( i=1; i<NUM_PLANE_TYPES; ++i )
	       ToggleResource( PlaneTemplate[ i ], "Label/Visibility" );
	 }
	 GlgUpdate( Drawing );
      }
      else if( strcmp( origin, "ToggleStates" ) == 0 )
      {
         StateDisplay = !StateDisplay;
         SetGISLayers();
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
      else if( strcmp( origin, "CloseFlightInfo" ) == 0 )
      {
	 SelectPlane( -1 ); /* Unselect the plane and erase popup display */
	 GlgUpdate( Drawing );
      }	
      else if( strcmp( origin, "Distance" ) == 0 )
      {
	 AbortDistanceMode();  /* Abort prev. distance mode if any */

	 DistanceMode = True;
	 SetStatus( "Click on the map to define distance to measure, "
                    "right click to finish." );
	 GlgUpdate( Drawing );
      }	
   }
   /* Process mouse clicks on plane icons, implemented as an Action with
      the Plane label attached to an icon and activated on a mouse click. 
   */
   else if( strcmp( format, "CustomEvent" ) == 0 )
   {
      char * event_label;

      if( DistanceMode )
	return;  /* Ignore selection in the Distance mode */

      GlgGetSResource( message_obj, "EventLabel", &event_label );

      if( strcmp( event_label, "Plane" ) == 0 )  /* Plane icon was selected */
      {
         GlgZoomState zoom_mode;
	 double plane_index;
         
         /* Map dragging mode is activated on a mouse click in the trace 
            callback. Abort the dragging mode if an object with custom event
            was selected. This gives custom events a higher priority compared 
            to the dragging mode. If it's a ZoomTo mode activated by a button,
            don't abort and ignore the object selection.
         */
         zoom_mode = ZoomToMode();
         if( !zoom_mode || ( zoom_mode & GLG_PAN_DRAG_STATE ) )
         {
            if( zoom_mode )
              GlgSetZoom( Map, NULL, 'e', 0. );  /* Abort zoom mode */
            
            /* Get plane index */
            GlgGetDResource( message_obj, "Object/DataIndex", &plane_index );
            SelectPlane( (long) plane_index );
            
            /* Show message in the bottom */
            SetStatus( PlaneArray[ SelectedPlaneIndex ].tooltip );
            
            DisplayPlaneInfo();   /* Display popup */
            GlgUpdate( Drawing );
         }
      }
   }
   else if( strcmp( action, "Zoom" ) == 0 )
   {
      if( strcmp( subaction, "End" ) == 0 )
      {
	 /* Update icon positions after zooming. */
	 RedoIcons = True;
	 UpdateObjectsOnMap( "Zooming or scrolling, please wait..." );
	 GlgUpdate( Drawing );
      }
   }
   else if( strcmp( action, "Pan" ) == 0 )
   {
      if( strcmp( subaction, "Start" ) == 0 )   /* Map dragging start */
      {
	 SetStatus( "Drag the map with the mouse." );
      }
      else if( strcmp( subaction, "Drag" ) == 0 )    /* Dragging */
      {
	 /* Update icon positions when scrolling. */
	 RedoIcons = True;
         UpdateObjectsOnMap( "Dragging the map with the mouse...." );

         /* Keep the message when dragging. */
         SetStatus( "Dragging the map with the mouse...." );
	 GlgUpdate( Drawing );         
      }
      else if( strcmp( subaction, "ValueChanged" ) == 0 )   /* Scrollbars */
      {
	 /* Update icon positions after zooming. */
	 RedoIcons = True;
         UpdateObjectsOnMap( "Scrolling, please wait..." );
	 GlgUpdate( Drawing );         
      }
      /* Dragging ended or aborted (right mouse button, etc.). */
      else if( strcmp( subaction, "End" ) == 0 ||
              strcmp( subaction, "Abort" ) == 0 )
      {
         SetStatus( "" );   /* Reset prompt dragging ends. */
      }     
   }   
}

/*----------------------------------------------------------------------
|
*/
long ZoomToMode()
{
   double zoom_mode;

   GlgGetDResource( Map, "ZoomToMode", &zoom_mode );
   return (long) zoom_mode;
}

/*----------------------------------------------------------------------
| Used to obtain coordinates of the mouse click.
*/
void Trace( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{      
   GlgTraceCBStruct * trace_data;
   GlgObject point_obj;
   GlgPoint
     point,
     lat_lon;
   char * lat_lon_string;
   double x, y;
   long
     event_type = 0,
     width, height;

   trace_data = (GlgTraceCBStruct*) call_data;

   /* Platform-specific code to extract event information.
      GLG_COORD_MAPPING_ADJ is added to the cursor coordinates for precise
      pixel mapping.
   */
#ifndef _WINDOWS
   switch( trace_data->event->type )
   {
    case ButtonPress:
      if( trace_data->event->xbutton.button != 1 )
      {
	 AbortDistanceMode();	 
	 return;  /* Use the left button clicks only. */
      }

      x = trace_data->event->xbutton.x + GLG_COORD_MAPPING_ADJ;
      y = trace_data->event->xbutton.y + GLG_COORD_MAPPING_ADJ;
      event_type = BUTTON_PRESS;
      break;
      
    case MotionNotify:
      x = trace_data->event->xmotion.x + GLG_COORD_MAPPING_ADJ;
      y = trace_data->event->xmotion.y + GLG_COORD_MAPPING_ADJ;
      event_type = MOUSE_MOVE;
      break;

    case ConfigureNotify:
      width = trace_data->event->xconfigure.width;
      height = trace_data->event->xconfigure.height;
      event_type = RESIZE;
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
      
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
      AbortDistanceMode();
      break;

    case WM_MOUSEMOVE:
      x = LOWORD( trace_data->event->lParam ) + GLG_COORD_MAPPING_ADJ;
      y = HIWORD( trace_data->event->lParam ) + GLG_COORD_MAPPING_ADJ;
      event_type = MOUSE_MOVE;
      break;

    case WM_SIZE:
      width = LOWORD( trace_data->event->lParam );
      height = HIWORD( trace_data->event->lParam );
      event_type = RESIZE;
      break;
      
    default: return;
   }
#endif
   
   /* Use the Map area events only. */
   if( trace_data->viewport != Map )
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
    case RESIZE:
      /* No need to adjust icon positions: GIS Object handles it. */
      break;

    case BUTTON_PRESS:
      if( ZoomToMode() )
        return; /* ZoomTo or dragging mode in progress: pass it through. */

      point.x = x;
      point.y = y;
      point.z = 0.;

      /* Handle paning: set the new map center to the location of the click. */
      if( PanMode )
      {      
	 PanMode = False;
	 
	 /* Converts X/Y to lat/lon using GIS object's current projection */
	 GlgGISConvert( GISObject, NULL, GLG_SCREEN_COORD,
		       /* X/Y to Lat/Lon */ True,
		       &point, &lat_lon );
	 
	 /* Pan the map */
	 GlgSetGResource( GISObject, "GISCenter", 
			 lat_lon.x, lat_lon.y, lat_lon.z );
	 RedoIcons = True;
	 UpdateObjectsOnMap( "Panning, please wait..." );
	 GlgUpdate( Map );
      }
      else if( DistanceMode )
      {	 
         /* Convert screen coordinates of the mouse to the world coordinates 
            inside the GIS Object's GISArray - lat/lon. */ 
         GlgScreenToWorld( GISArray, True, &point, &lat_lon );

	 if( !DistancePolygon )
	 {
	    DistancePolygon = GlgCreateObject( GLG_POLYGON, NULL,
					      NULL, NULL, NULL, NULL );
	    GlgSetGResource( DistancePolygon, "EdgeColor", 1., 1., 0. );
	    NumDistancePoints = 1;

	    point_obj = GlgGetElement( DistancePolygon, 0 );
	    GlgSetGResource( point_obj, NULL, 
                            lat_lon.x, lat_lon.y, lat_lon.z );

	    point_obj = GlgGetElement( DistancePolygon, 1 );
	    GlgSetGResource( point_obj, NULL, 
                            lat_lon.x, lat_lon.y, lat_lon.z );

	    GlgAddObjectToBottom( GISObject, DistancePolygon );
	    GlgUpdate( Map );	    
	 }
	 else /* Not the first point */
	 {
	    /* Set current point to the coord. of the click. */
	    point_obj = GlgGetElement( DistancePolygon, NumDistancePoints );

	    GlgSetGResource( point_obj, NULL, 
                            lat_lon.x, lat_lon.y, lat_lon.z );
	    ++NumDistancePoints;

	    DisplayDistance( DistancePolygon );

	    /* Add next point, same coords. */
	    point_obj = GlgCopyObject( point_obj );
	    GlgAddObjectToBottom( DistancePolygon, point_obj );
	    GlgDropObject( point_obj );

	    GlgUpdate( Map );
	 }
      }
      /* Not a Pan or Distance mode: start dragging the map with the mouse. */
      else
        GlgSetZoom( Map, NULL, 's', 0. );
      break;

    case MOUSE_MOVE:
      point.x = x;
      point.y = y;
      point.z = 0.;
	 
      /* Convert screen coordinates of the mouse to the world coordinates 
         inside the GIS Object's GISArray - lat/lon. */ 
      GlgScreenToWorld( GISArray, True, &point, &lat_lon );

      /* Handle distance-measuring mode. */
      if( DistanceMode && DistancePolygon )
      {
	 point_obj = GlgGetElement( DistancePolygon, NumDistancePoints );
	 GlgSetGResource( point_obj, NULL, lat_lon.x, lat_lon.y, lat_lon.z );
	 DisplayDistance( DistancePolygon );
	 GlgUpdate( Map );
      }
      
      /* Display lat/lon of a point under the mouse. */
      lat_lon_string = CreateLocationString( lat_lon.x, lat_lon.y, lat_lon.z );
      GlgSetSResource( PositionObject, NULL, lat_lon_string );
      GlgFree( lat_lon_string );

      GlgUpdate( PositionArea );
      break;

    default: return;
   } 
}

/*----------------------------------------------------------------------
|
*/
void AbortDistanceMode()
{
   if( DistanceMode )
   {
      if( DistancePolygon )   /* Delete distance polygon */
      {
	 if( GlgContainsObject( GISObject, DistancePolygon ) )
	   GlgDeleteThisObject( GISObject, DistancePolygon );

	 GlgDropObject( DistancePolygon );	 
	 DistancePolygon = (GlgObject)0;
      }
      GlgSetDResource( Map, "DistancePopup/Visibility", 0. );
      SetStatus( "" );
      DistanceMode = False;
   }
}

/*----------------------------------------------------------------------
|
*/
void DisplayDistance( GlgObject polygon )
{
   GlgObject point, last_point;
   long i, size;
   double distance;

   /* Popup distance display */
   GlgSetDResource( Map, "DistancePopup/Visibility", 1. );

    /* Last point is for dragging, not set yet - don't include. */
   size = (long) GlgGetSize( polygon );
   if( size < 2 )
     return;

   distance = 0.;
   point = (GlgObject)0;
   for( i=0; i<size; ++i )
   {
      last_point = point;
      point = GlgGetElement( polygon, i );

      if( last_point )
	/* Nautical mile = 1842m */
	distance += GetGlobeDistance( point, last_point ) / 1842.;
   }
   
   GlgSetDResource( Map, "DistancePopup/Distance", distance );
}

/*----------------------------------------------------------------------
| Returns the length (in meters) of the shortest arc along the earth surface 
| connecting the two points.
*/
double GetGlobeDistance( GlgObject point1_obj, GlgObject point2_obj )
{
   GlgPoint lat_lon1, lat_lon2, globe_point1, globe_point2;
   double 
     globe_radius,
     dx, dy, dz,
     straight_dist,
     arc_dist;

   GlgGetGResource( point1_obj, NULL, &lat_lon1.x, &lat_lon1.y, &lat_lon1.z );
   GlgGetGResource( point2_obj, NULL, &lat_lon2.x, &lat_lon2.y, &lat_lon2.z );

   /* XYZ of the first point, in meters */
   GetPointXYZ( &lat_lon1, &globe_point1 );

   /* XYZ of the second point, in meters */
   GetPointXYZ( &lat_lon2, &globe_point2 );

   dx = globe_point1.x - globe_point2.x;
   dy = globe_point1.y - globe_point2.y;
   dz = globe_point1.z - globe_point2.z;

   /* Length of a straight line between the points. */
   straight_dist = sqrt( dx * dx + dy * dy + dz * dz );

   /* Use the average value. */
   globe_radius = ( GLG_EQUATOR_RADIUS + GLG_POLAR_RADIUS ) / 2.;

   /* The length of the shortest connecting arc along the earth surface. */
   arc_dist = 2. * globe_radius * 
     asin( straight_dist / ( 2. * globe_radius ) );
   
   return arc_dist;
}

/*----------------------------------------------------------------------
| Returns XYZ coordinate of a lat/lon point in meters for projection with
| the GISCenter at [0;0].
*/
void GetPointXYZ( GlgPoint * lat_lon, GlgPoint * xyz )
{
   double
     globe_radius,
     lon, lat;

   lon = DegToRad( lat_lon->x );
   lat = DegToRad( lat_lon->y );

   /* Use the average value. */
   globe_radius = ( GLG_EQUATOR_RADIUS + GLG_POLAR_RADIUS ) / 2.;

   xyz->x = globe_radius * cos( lat ) * sin( lon );
   xyz->y = globe_radius * sin( lat );
   xyz->z = globe_radius * cos( lat ) * cos( lon );
}

/*----------------------------------------------------------------------
|
*/
void Zoom( long type, double value )
{
   switch( type )
   {
    default:
      GlgSetZoom( Map, NULL, type, value );
      CheckScrollLimits( type );

      RedoIcons = True;
      UpdateObjectsOnMap( "Zooming or scrolling, please wait..." );
      break;

    case 'n':
      /* Reset map to the initial extent. */
      MapProjection = GLG_ORTHOGRAPHIC_PROJECTION;
      GlgSetDResource( GISObject, "GISProjection",
		      (double) GLG_ORTHOGRAPHIC_PROJECTION );
      GlgSetGResource( GISObject, "GISCenter",
		      InitCenter.x, InitCenter.y, InitCenter.z );
      GlgSetGResource( GISObject, "GISExtent", 
		      InitExtent.x, InitExtent.y, InitExtent.z );

#if CHANGE_LAYER_ATTRIBUTES_SAMPLE 
      SetFillType( False );
#endif

      RedoIcons = True;
      UpdateObjectsOnMap( "Reloading map, please wait..." );
      break;
   }
}

/*----------------------------------------------------------------------
| For rectangular projection, make sure the map does not scroll 
| beyond the poles in the vertical direction.
*/
void CheckScrollLimits( long type )
{
   GlgPoint extent, center;
   double min_y, max_y;
   long adjust_x, adjust_y;

   if( MapProjection == GLG_ORTHOGRAPHIC_PROJECTION )
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

   GlgGetGResource( GISObject, "GISExtent", &extent.x, &extent.y, &extent.z );
   GlgGetGResource( GISObject, "GISCenter", &center.x, &center.y, &center.z );
   
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
     GlgSetGResource( GISObject, "GISCenter", center.x, center.y, center.z );
}

/*----------------------------------------------------------------------
| Changes plane color to indicate selection and displayes or erases the 
| flight info popup dialog.
*/
void SelectPlane( long selected_plane_index )
{
   PlaneData * plane;
   long popup_visibility;

   if( SelectedPlaneIndex != -1 )   /* Unselect previously selected plane */
   {
      plane = &PlaneArray[ SelectedPlaneIndex ];
      if( plane->graphics )       /* Restore color if plane is visible */
	GlgSetDResource( plane->graphics, "ColorIndex",
			(double) plane->color_index );
   }
   
   if( selected_plane_index != -1 )   /* Select new plane */
   {
      plane = &PlaneArray[ selected_plane_index ];
      if( plane->graphics )  /* Set selected color color if plane is visible */
	GlgSetDResource( plane->graphics, "ColorIndex",	(double) SELECTED );
   }

   /* Display or erase the flight info popup. */
   if( selected_plane_index == -1 )    /* Unselected */
     popup_visibility = 0.;
   else   /* Selected */
     popup_visibility = 1.;
   GlgSetDResource( FlightInfoPopup, "Visibility", (double) popup_visibility );

   SelectedPlaneIndex = selected_plane_index;
}

/*----------------------------------------------------------------------
| 
*/
void DisplayPlaneInfo()
{
   PlaneData * plane;
   char * lat_lon_string;

   plane = &PlaneArray[ SelectedPlaneIndex ];
   GlgSetSResource( FlightInfoPopup, "FlightInfo", plane->tooltip );
   GlgSetDResource( FlightInfoPopup, "Elevation", GetPlaneElevation( plane ) );
   GlgSetDResource( FlightInfoPopup, "StatusIndex", 
                    (double) plane->color_index );

   lat_lon_string = 
     CreateLocationString( plane->lat_lon.x, plane->lat_lon.y, 1. );
   GlgSetSResource( FlightInfoPopup, "Location", lat_lon_string );
   GlgFree( lat_lon_string );
}

/*----------------------------------------------------------------------
| Delete node icons and place them into the object pool. 
*/
void DeleteNodes()
{
   GlgObject icon;
   long i, size;

   /* Move node icons into the object pool. */
   size = (long) GlgGetSize( NodeGroup );
   for( i=0; i<size; ++i )
   {
      icon = GlgGetElement( NodeGroup, i );
      GlgAddObjectToBottom( NodePool[ NodeType ], icon );
   }

   /* Delete node icons from the drawing and node group */
   GlgDeleteThisObject( GISObject, NodeGroup );
   for( i=0; i<size; ++i )
     GlgDeleteBottomObject( NodeGroup );

   /* Set nodes' graphics to NULL */
   if( size )
     for( i=0; i<NumNodes; ++i )
       NodeArray[i].graphics = (GlgObject)0;
}

/*----------------------------------------------------------------------
| Delete plane icons and place them into the object pool. 
*/
void DeletePlanes()
{
   GlgObject icon;
   long i, size;

   /* Move plane icons into the object pool. */
   size = (long) GlgGetSize( PlaneGroup );
   for( i=0; i<size; ++i )
   {
      icon = GlgGetElement( PlaneGroup, i );
      GlgAddObjectToBottom( PlanePool[ PlaneType ], icon );
   }

   /* Delete plane icons from the drawing and plane group */
   GlgDeleteThisObject( GISObject, PlaneGroup );
   for( i=0; i<size; ++i )
     GlgDeleteBottomObject( PlaneGroup );

   /* Set planes' graphics to NULL */
   if( size )
     for( i=0; i<NumPlanes; ++i )
       PlaneArray[i].graphics = (GlgObject)0;

   DeleteTrajectories();
}

/*----------------------------------------------------------------------
| Delete trajectory objects and place them into the object pool. 
*/
void DeleteTrajectories()
{
   GlgObject icon;
   long i, size;

   /* Move plane icons into the object pool. */
   size = (long) GlgGetSize( TrajectoryGroup );
   for( i=0; i<size; ++i )
   {
      icon = GlgGetElement( TrajectoryGroup, i );
      GlgAddObjectToBottom( TrajectoryPool, icon );
   }

   /* Delete trajectory icons from the drawing and trajectory group */
   GlgDeleteThisObject( GISObject, TrajectoryGroup );
   for( i=0; i<size; ++i )
     GlgDeleteBottomObject( TrajectoryGroup );

   /* Set trajectorys' graphics to NULL */
   if( size )
     for( i=0; i<NumPlanes; ++i )
       PlaneArray[i].trajectory = (GlgObject)0;
}

/*----------------------------------------------------------------------
| 
*/
void PositionNode( NodeData * node, long index )
{
   if( !IconVisible( &node->lat_lon ) )
     return;

   /* Add node's graphics to the drawing if first time. */
   if( !node->graphics )
     AddNodeGraphics( node, NodeType, index );

   /* Position node's icon. Since the icons are added as children of the 
      GIS Object, their coordinates are specified in lat/lon. The GIS Object
      handles all details of coordinate convesion.
    */
   GlgSetGResource( node->graphics, "Position", 
                   node->lat_lon.x, node->lat_lon.y, 0. );
}

/*----------------------------------------------------------------------
| 
*/
GlgObject CreateNodeIcon( long node_type )
{
   GlgObject icon;
   long size;   

   size = (long) GlgGetSize( NodePool[ node_type ] );
   if( size )   /* Return an icon from the pool */
   {
      icon = GlgGetElement( NodePool[ node_type ], size - 1 );
      GlgReferenceObject( icon );
      GlgDeleteBottomObject( NodePool[ node_type ] );
   }
   else   /* Create a new icon */
   {
      icon = GlgCloneObject( NodeTemplate[ node_type ], GLG_STRONG_CLONE );
   }
   return icon;
}

/*----------------------------------------------------------------------
| 
*/
GlgObject CreatePlaneIcon( long plane_type )
{
   GlgObject icon;
   long size;   

   size = (long) GlgGetSize( PlanePool[ plane_type ] );
   if( size )   /* Return an icon from the pool */
   {
      icon = GlgGetElement( PlanePool[ plane_type ], size - 1 );
      GlgReferenceObject( icon );
      GlgDeleteBottomObject( PlanePool[ plane_type ] );
   }
   else   /* Create a new icon */
   {
      icon = GlgCloneObject( PlaneTemplate[ plane_type ], GLG_STRONG_CLONE );
   }
   return icon;
}

/*----------------------------------------------------------------------
| 
*/
GlgObject CreateTrajectoryIcon()
{
   GlgObject icon;
   long size;   

   size = (long) GlgGetSize( TrajectoryPool );
   if( size )   /* Return an icon from the pool */
   {
      icon = GlgGetElement( TrajectoryPool, size - 1 );
      GlgReferenceObject( icon );
      GlgDeleteBottomObject( TrajectoryPool );
   }
   else   /* Create a new icon */
   {
      icon = GlgCloneObject( TrajectoryTemplate, GLG_STRONG_CLONE );
   }
   return icon;
}

/*----------------------------------------------------------------------
| 
*/
void PositionPlane( PlaneData * plane, long index )
{
   /* Gets the new plane's position, simulated or from real data. */
   GetPlaneLatLon( plane );    

   if( !IconVisible( &plane->lat_lon ) )
   {
      if( plane->graphics )   /* Delete graphics and place into the pool */
      {
	 GlgAddObjectToBottom( PlanePool[ PlaneType ], plane->graphics );
	 GlgDeleteThisObject( PlaneGroup, plane->graphics );
	 plane->graphics = (GlgObject)0;
      }
      if( plane->trajectory )   /* Delete trajectory and place into the pool */
      {
	 GlgAddObjectToBottom( TrajectoryPool, plane->trajectory );
	 GlgDeleteThisObject( TrajectoryGroup, plane->trajectory );
	 plane->trajectory = (GlgObject)0;
      }
      return;
   }

   /* Add the plane icon to the drawing if the first time. */
   if( !plane->graphics )
     AddPlaneGraphics( plane, PlaneType, index );

   /* Position plane's icon. Since the icons are added as children of the 
      GIS Object, their coordinates are specified in lat/lon. The GIS Object
      handles all details of coordinate convesion.
    */
   GlgSetGResource( plane->graphics, "Position", 
		   plane->lat_lon.x, plane->lat_lon.y, 0. );

   /* Update icon's direction angle if necessary. */
   if( HasAngle )
     GlgSetDResource( plane->graphics, "Angle", GetPlaneAngle( plane ) );

   if( HasElevation )
     GlgSetDResource( plane->graphics, "Height", GetPlaneElevation( plane ) );

   if( plane->trajectory )
   {
      /* For small speeds, skip a few iterations to increase the trajectory's
	 length. */
      if( PlaneSpeed < 0.01 )  
      {
         long n;

	 n = 0.01 / PlaneSpeed;
	 if( n )
	 {
	    ++plane->iteration;
	    if( plane->iteration % n )
	      return;   /* Skip n iterations, update every n'th */
	 }
      }      
      
      GlgSetDResource( plane->trajectory, "VisEntryPoint", 1. );
      GlgSetGResource( plane->trajectory, "XYEntryPoint", 
		      plane->lat_lon.x, plane->lat_lon.y, 0. );      
   }
}

/*----------------------------------------------------------------------
| Adds an airport icon, fills labels, tooltips, etc.
*/
void AddNodeGraphics( NodeData * node, long node_type, long index )
{      
   GlgObject icon;
   char
     * tooltip,
     * lat_lon_string,
     buffer[ 1002 ];

   icon = CreateNodeIcon( node_type );

   /* Index for direct access */     
   GlgSetDResource( icon, "DataIndex", (double)index );

   if( node_type > 0 )   /* More detailed icon */
     GlgSetSResource( icon, "LabelString", node->name );

   lat_lon_string = 
     CreateLocationString( node->lat_lon.x, node->lat_lon.y, 1. );
   
   sprintf( buffer, "%.500s, %s", node->name, lat_lon_string );
   GlgFree( lat_lon_string );

   tooltip = buffer;

   GlgSetSResource( icon, "TooltipString", tooltip );
	
   node->graphics = icon;

   GlgAddObjectToBottom( NodeGroup, icon );
   GlgDropObject( icon );
}

/*----------------------------------------------------------------------
| Adds a plane icon, fills labels, tooltips, etc.
*/
void AddPlaneGraphics( PlaneData * plane, long plane_type, long index )
{
   GlgObject icon;
   char * label;
   long i;

   icon = CreatePlaneIcon( plane_type );

   /* Index for direct access */
   GlgSetDResource( icon, "DataIndex", (double)index );           

   /* Icon color */
   GlgSetDResource( icon, "ColorIndex", (double) plane->color_index );

   if( plane_type > 0 )   /* More detailed icon */
   {
      /* Show the flight number as icon label */
      label = GlgCreateIndexedName( "Flight ", plane->flight_number );
      GlgSetSResource( icon, "LabelString", label );
      GlgFree( label );
   }

   /* Set the tooltip */
   GlgSetSResource( icon, "TooltipString", plane->tooltip );

   plane->graphics = icon;

   GlgAddObjectToBottom( PlaneGroup, icon );
   GlgDropObject( icon );
   
   if( plane_type == 2 )   /* For detailed icon, create trajectory */
   {
      icon = CreateTrajectoryIcon();
      plane->trajectory = icon;

      /* Set entries invisible initially */
      GlgSetDResource( icon, "Marker/Visibility", 0. );

      GlgAddObjectToBottom( TrajectoryGroup, icon );

      for( i=0; i<NumTrajectoryPoints; ++i )  /* Set fading */
	GlgSetDResource( icon, "BrightEntryPoint",
			0.2 + 0.8 * i / (double) NumTrajectoryPoints );

      GlgDropObject( icon );
   }
}

/*----------------------------------------------------------------------
| Check if the icon is visible in the current zoom region.
*/
long IconVisible( GlgPoint * lat_lon )
{
   GlgPoint position;

   /* Converts lat/lon to X/Y using GIS object's current projection. */
   GlgGISConvert( GISObject, NULL, GLG_OBJECT_COORD,
		 /* Lat/Lon to XY */ False, lat_lon, &position );

   return position.z >= 0. &&
     position.x > -1100. && position.x < 1100. &&
     position.y > -1100. && position.y < 1100.;
}

#if 0
/*----------------------------------------------------------------------
| Turns plane icons' labels ON or OFF if labels exist.
*/
void SetLabels( long on )
{
   GlgObject label;

   label = GlgGetResourceObject( PlaneArray[ 0 ].graphics, "Label" );
   if( label )
     GlgSetDResource( label, "Visibility", on ? 1. : 0. );
}
#endif

/*----------------------------------------------------------------------
| Displays a message in the status area.
*/
void SetStatus( char * message )
{
   GlgSetSResource( Drawing, "StatusLabel/String", message );
   GlgUpdate( GlgGetResourceObject( Drawing, "StatusArea" ) );
}

/*----------------------------------------------------------------------
|
*/
void SetPlaneSize()
{
   GlgObject resource;
   int i;

   for( i=0; i<NUM_PLANE_TYPES; ++i )
     if( resource = GlgGetResourceObject( PlaneTemplate[ i ], "IconScale" ) )
     {
        /* Polygon icon: set scale. */
        GlgSetDResource( resource, NULL, PlaneSize );
     }
     else if( resource = GlgGetResourceObject( PlaneTemplate[ i ], 
                                              "Marker/MarkerSize" ) )
     {
        /* Marker: set MarkerSize. */
        if( PlaneSize == SMALL_SIZE )
          GlgSetDResource( resource, NULL, SMALL_MARKER_SIZE );
        else if( PlaneSize == MEDIUM_SIZE )
          GlgSetDResource( resource, NULL, MEDIUM_MARKER_SIZE );
        else
          GlgSetDResource( resource, NULL, BIG_MARKER_SIZE );
     }
}

/*----------------------------------------------------------------------
| Toggle resource between 0 and 1.
*/
void ToggleResource( GlgObject object, char * res_name )
{
   GlgObject resource;
   double value;

   resource = GlgGetResourceObject( object, res_name );
   if( !resource )
     return;

   GlgGetDResource( resource, NULL, &value );
   GlgSetDResource( resource, NULL, value != 0. ? 0. : 1. );
}

/*----------------------------------------------------------------------
| Toggle map layers: airport/city labels and states.
*/
void SetGISLayers()
{
   char * layers, * new_layers;

   /* Airport labels should be visible only when city labels are off. */
   GlgSetDResource( NodeTemplate[1], "Label/Visibility", !CityLabels );

   layers = GlgStrClone( Layers );

   if( CityLabels )   /* Add city layers if on. */
     new_layers = GlgConcatStrings( layers, ",us_cities" );
   else
     new_layers = GlgConcatStrings( layers, ",-us_cities" );
   GlgFree( layers );
   layers = new_layers;

   if( StateDisplay )   /* Add states layer if on. */
     /* Enable states regardless of the default. */
     new_layers = GlgConcatStrings( layers, ",states_dcw" );
   else
     /* Disable state outline display. */
     new_layers = GlgConcatStrings( layers, ",-states_dcw" );
   GlgFree( layers );
   layers = new_layers;

   GlgSetSResource( GISObject, "GISLayers", layers );
   GlgFree( layers );
}

/*----------------------------------------------------------------------
| Changes color of the states boundaries.
*/
void ToggleColor()
{
   static GlgPoint color;
   static long color_stored = False;
   GlgObject dataset;

   dataset = GlgGISGetDataset( GISObject, NULL );
   if( !color_stored )
   {
      GlgGetGResource( dataset, "states_dcw/EdgeColor",
		      &color.x, &color.y, &color.z );
      GlgSetGResource( dataset, "states_dcw/EdgeColor", 0., 0.89, 0.89 );
      color_stored = True;
   }
   else
   {
      GlgSetGResource( dataset, "states_dcw/EdgeColor",
		      color.x, color.y, color.z );
      color_stored = False;
   }

   GlgSetZoom( Map, NULL, 'i', 1.000001 );   /* Force to redo the map image */
   GlgUpdate( Drawing );
}

/*----------------------------------------------------------------------
| Gets extent in lat/lon.
| For the ortho projection, roughly converts from meters to lat/lon.
*/
void GetExtentDegrees( GlgPoint * extent )
{   
   GlgGetGResource( GISObject, "GISExtent",
		   &extent->x, &extent->y, &extent->z );
   if( MapProjection == GLG_ORTHOGRAPHIC_PROJECTION )
   {
      extent->x = extent->x / GLG_EQUATOR_RADIUS * 90.;
      extent->y = extent->y / GLG_POLAR_RADIUS * 90.;
   }
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
| FOR FLIGHT SIMULATION ONLY: Converts plane lat/lon to the GLG world 
|     coordinates for calculating plane speed and directional angle.
*/
void GetPlanePosition( PlaneData * plane, GlgPoint * xyz )
{
   /* Converts lat/lon to X/Y using GIS object's current projection. */
   GlgGISConvert( GISObject, NULL, GLG_OBJECT_COORD, 
                 /* Lat/Lon to XY */ False, &plane->lat_lon, xyz );
}

/*----------------------------------------------------------------------
| FOR FLIGHT SIMULATION ONLY: Converts node lat/lon to the GLG world 
|     coordinates for calculating plane's initial directional angle.
*/
void GetNodePosition( NodeData * node, GlgPoint * xyz )
{
   /* Converts lat/lon to X/Y using GIS object's current projection. */
   GlgGISConvert( GISObject, NULL, GLG_OBJECT_COORD, 
                 /* Lat/Lon to XY */ False, &node->lat_lon, xyz );
}

/*----------------------------------------------------------------------
| FOR FLIGHT SIMULATION ONLY: Calculates plane icon's directional angle.
|     In an application, it will query the plane's directional angle.
*/
double GetPlaneAngle( PlaneData * plane )
{
   GlgPoint last_xyz, curr_xyz;

   /* Rectangular projection preserves straight lines, we can use the 
      angle of the line connecting the start and end nodes. For the
      orthographic projection, use this case if the plane has just started
      and there is no previous position stored.
      */
   if( MapProjection == GLG_RECTANGULAR_PROJECTION ||
      plane->path_position == plane->path_position_last )   /* Just started */
   {
      GetNodePosition( plane->from_node, &last_xyz );
      GetNodePosition( plane->to_node, &curr_xyz );
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
      GetPlanePosition( plane, &last_xyz );

      /* Restore the plane's current position and get its coordinates. */
      plane->path_position = stored_position;
      GetPlaneLatLon( plane );
      GetPlanePosition( plane, &curr_xyz );
   }   

   /* Calculate the angle of a line connecting the previous and 
      current position. */
   return GetAngle( &last_xyz, &curr_xyz );
}

/*----------------------------------------------------------------------
| FOR FLIGHT SIMULATION ONLY: Calculates plane icon's elevation.
|     In an application, it will query the plane elevation.
|
| For the simulation, it calculated the elevation using zero at the start
| and end of the path and the maximum elevation in the middle.
*/
double GetPlaneElevation( PlaneData * plane )
{
   return ( 0.5 - fabs( plane->path_position - 0.5 ) ) * 2. * 10000.;   
}

/*----------------------------------------------------------------------
| Installs a timer to update moving icons.
*/
void StartUpdate()
{
   GlgAddTimeOut( AppContext, UpdateInterval, (GlgTimerProc)UpdatePlanes, 
		 NULL );
}

/*----------------------------------------------------------------------
| Updates moving icons with the new position data.
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
     UpdatePlane( &PlaneArray[ i ], i );

   if( SelectedPlaneIndex != -1 )   /* Update selected plane info if any */
     DisplayPlaneInfo();

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
| FOR FLIGHT SIMULATION ONLY: Calculates new plane position using simulated 
|     data. In an application, it will query the plane's lat/lon.
*/
void UpdatePlane( PlaneData * plane, long index )
{      
   if( !plane->from_node || !plane->to_node )
     return;   /* Plane is not in the air - no start/destination node. */
            
   /* Finished the old path, start a new one. */
   if( plane->path_position == 1. )
   {
      if( index == SelectedPlaneIndex )
	SelectPlane( -1 );   /* Unselect the plane: it reached destination */

      StartPlane( plane, False );
   }
   else  /* Continue on the current path. */
   {
      double speed;

      speed = PlaneSpeed;

      /* Store last position for calculating angle in ORTHO projection. */
      plane->path_position_last = plane->path_position;

      plane->path_position += plane->speed * speed;
      if( plane->path_position > 1. )
	plane->path_position = 1.;
   }

   SetPlaneColor( plane );

   PositionPlane( plane, index );   /* Position plane on the map */
}

/*----------------------------------------------------------------------
| FOR FLIGHT SIMULATION ONLY: Simulate data to change plane color to show 
|     warnings and alarms. In an application, it will query the plane's 
|     status.
*/
void SetPlaneColor( PlaneData * plane )
{
   double random_value;
   long new_color_index;

   /* Set random color */
   random_value = GlgRand( 0., 1. );
   if( plane->color_index == NORMAL )
   {
      if( random_value <= 0.999 )
	new_color_index = NORMAL;
      else if( random_value > 0.9999 )
	new_color_index = ALARM;
      else if( random_value > 0.999 )
	new_color_index = WARNING;     
   }
   else if( plane->color_index == WARNING )
   {
      if( random_value > 0.99 )
	new_color_index = NORMAL;
      else
	new_color_index = plane->color_index;   /* Keep alarm for a while */
   }
   else if( plane->color_index == ALARM )
   {
      if( random_value > 0.999 )
	new_color_index = NORMAL;
      else
	new_color_index = plane->color_index;   /* Keep alarm for a while */
   }

   if( plane->graphics && new_color_index != plane->color_index )
     GlgSetDResource( plane->graphics, "ColorIndex", (double)new_color_index );
   
   plane->color_index = new_color_index;
}

/*----------------------------------------------------------------------
| FOR FLIGHT SIMULATION ONLY: Starts simulation for a plane, selects its start 
| and end nodes. 
*/
void StartPlane( PlaneData * plane, long init )
{
   long
     to_index,
     from_index;
   char buffer[ 2000 ];
   char * flight_name;

   if( NumNodes < 2 )
     error( "Less then two nodes: can't start planes.", True );

   from_index = GlgRand( 0., NumNodes - 0.001 );
   do
   {
      to_index = GlgRand( 0., NumNodes - 0.001 );
   } while( to_index == from_index );

   plane->from_node = &NodeArray[ from_index ];
   plane->to_node = &NodeArray[ to_index ];
   plane->flight_number = (int) GlgRand( 101., 1999. );
   plane->speed = GlgRand( 0.4, 1. );   /* Vary the plane's speed */

   if( init )   /* Init the demo: position planes randomly along the paths. */
   {
      plane->path_position = GlgRand( 0.1, 0.9 );
      plane->path_position_last = plane->path_position - 0.05;  /* For angle */
   }
   else         /* Position the plane at the beginning of the path. */
   {
      plane->path_position = 0.;
      plane->path_position_last = 0.;
   }

   GlgFree( plane->tooltip );

   flight_name = GlgCreateIndexedName( "Flight ", plane->flight_number );
      
   /* Add from/to node info to the tooltip. */
   sprintf( buffer, "%.500s from %.500s to %.500s", flight_name,
	   plane->from_node->name, plane->to_node->name ); 
   GlgFree( flight_name );

   plane->tooltip = GlgStrClone( buffer );

   /* If the trajectory exists, set all its points invisible initially. */
   if( plane->trajectory )
     GlgSetDResource( plane->trajectory, "Marker%/Visibility", 0. );
}

/*----------------------------------------------------------------------
| FOR FLIGHT SIMULATION ONLY: Calculates plane's lat/lon using simulated data. 
|     In an application, it will query the plane's position.
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
| Generate a location info string by converting +- signs info into the
| N/S, E/W suffixes, and decimal fraction to deg, min, sec.
*/
char * CreateLocationString( double lon, double lat, double z )
{
   int
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
            "Lon=%d.%.2d'%.2d\"%c  Lat=%d.%.2d'%.2d\"%c",
            x_deg, x_min, x_sec, char_x, y_deg, y_min, y_sec, char_y ); 
   
   return GlgStrClone( buffer );
}

/*----------------------------------------------------------------------
| Zoom to Florida after a few seconds to show details.
*/
void ZoomToFloridaStart( GlgAnyType data, GlgIntervalID * id )
{
   GlgObject florida_message;

   GlgSetDResource( Drawing, "Map/FloridaZoomingMessage/Visibility", 1. );
   GlgUpdate( Drawing );

   /* GISExtent is in lon/lat for the rectangular GIS projection, and in 
      meters for the orthographic projection. To find proper values, zoom 
      the map in the GlgBuilder and copy the GISExtent values.
   */

   GlgSetGResource( GISObject, "GISExtent", 1169530., 1169530., 0. );
   GlgSetGResource( GISObject, "GISCenter" , -82.8239, 28.9382, 0. );

   /* Update icon positions after zooming. */
   RedoIcons = True;
   UpdateObjectsOnMap( "Zooming, please wait..." );

   /* Reorder Florida zoom message to the top, in case any object 
      were added on top of it. */
   florida_message =
     GlgGetResourceObject( Drawing, "Map/FloridaZoomingMessage" );
   GlgReorderElement( Map, GlgGetIndex( Map, florida_message ), 
                     GlgGetSize( Map ) - 1 );

   GlgUpdate( Drawing );

   GlgAddTimeOut( AppContext, FloridaZoomDelay2, 
                 (GlgTimerProc)ZoomToFloridaEnd, NULL );
}

/*----------------------------------------------------------------------
| Remove the Florida zooming message after a few seconds.
*/
void ZoomToFloridaEnd( GlgAnyType data, GlgIntervalID * id )
{
   GlgSetDResource( Drawing, "Map/FloridaZoomingMessage/Visibility", 0. );
   GlgUpdate( Drawing );
}

/*----------------------------------------------------------------------
| Adjusts minor rendering attributes for nicer-looking picture depending
| on the driver used to render the drawing: OpenGL or GDI.
*/
void AdjustRendering( void )
{
   GlgObject shadow_offset;
   double 
     dvalue,
     x, y, z;
   long i;

   GlgGetDResource( Map, "Screen/OpenGL", &dvalue );
   OpenGLEnabled = dvalue;

   if( OpenGLEnabled )
   {
      /* Use markers with FILL and no EDGE in OpenGL. */
      GlgSetDResource( TrajectoryTemplate, "Marker/MarkerType", 
                      (double) GLG_FILLED_CIRCLE );
   }
   else
   {
      /* Small filled circles do not look good in the native GDI driver:
         draw EDGE around them to make them look good.
         */
      GlgSetDResource( TrajectoryTemplate, "Marker/MarkerType", 
                      (double) ( GLG_FILLED_CIRCLE | GLG_CIRCLE ) );      

      /* For non-OpenGL driver, if a polygon with shadows is used for the 
         plane's icons, decrease the shadow size and disable the shadow 
         transparency by setting the z coordinate of the shadow offset to 1.
         */
      for( i=0; i<NUM_PLANE_TYPES; ++i )
      {
         shadow_offset = 
           GlgGetResourceObject( PlaneTemplate[i], "Polygon/ShadowOffset" );

         if( shadow_offset )
         {
            GlgGetGResource( shadow_offset, NULL, &x, &y, &z );
            if( x != 0. && y != 0. )
              GlgSetGResource( shadow_offset, NULL, 1., -1., 1. );
         }
      }
   }
}
