/*----------------------------------------------------------------------
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
| toolbar button. The menu's InitStateList resource defines a list of
| states of the individual menu buttons when the menu is displayed
| for the first time. In this example, the values for the InitStateList are
| set in the drawing, but they may be changed at run-time as well, 
| before hierarchy setup.
|
| The program also supports zooming and panning of the map using toolbar 
| buttons. 
|
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "GlgApi.h"
#include "gis_example2.h"

#define UPDATE_INTERVAL   200     /* 200ms timer = 5 times/sec refresh */

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

#define RadToDeg( angle )   ( ( angle ) / M_PI * 180. )
#define DegToRad( angle )   ( ( angle ) /180. * M_PI )

GlgObject 
     Drawing,        /* Main viewport of the drawing */
     MapVp,          /* Map viewport containing a GIS object */
     LayersMenu,     /* Layers menu object has toggles for turning map layers 
		      on/off. */
     GISObject,      /* GIS object */
     TargetTemplate, /* Template for a target object */
     TargetGroup,    /* Group of target objects */
     SelectedTarget = NULL; /* Currently selected target object */


/* Store initial extent and center, used to reset the drawing */
GlgPoint
  InitExtent,
  InitCenter;

long
   PanMode = False,
   NumTargets;

IconDataStruct IconData;

/* Array of icons/targets to place on the map as GLG objects. 
   In this example, these objects are populated at the specified 
   lat/lon positions, but the code can be added to position them
   at the lat/lon location defined by the user at run-time.
   Since these targets are GLG objects, they may be selected with 
   the mouse and their attributes may be changed dynamically. 
   For example, when a prticular target is selected, its color may 
   be changed to indicate the current selection. These objects may be 
   also moved/dragged using the mouse, to change their location. 
   */
TargetData TargetArray[] =
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

char * buffer[ 1002 ];  /* buffer to hold a tooltip string */

GlgAppContext AppContext;       /* Global, used to install a work procedure. */

#include "GlgMain.h"    /* Cross-platform entry point. */

/*----------------------------------------------------------------------
|
*/
int GlgMain( argc, argv, app_context )
     int argc;
     char *argv[];
     GlgAppContext app_context;
{
   AppContext = GlgInit( False, app_context, argc, argv );

   GlgSetDResource( (GlgObject)0, "$config/GlgMouseTooltipTimeout", 0.05 );

   Drawing = GlgLoadWidgetFromFile( "gis_example2.g" );
   if( !Drawing )
     error( "Can't load drawing file.", True );

   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( Drawing, "Point1", -500., -600., 0. );
   GlgSetGResource( Drawing, "Point2",  500.,  500., 0. );

   /* Obtain an object ID of the viewport named "MapVp" and GIS object
      named "GISObject" */
   MapVp = GlgGetResourceObject( Drawing, "MapVp" );
   GISObject = GlgGetResourceObject( MapVp, "GISObject" );

   /* Set GIS Zoom mode: generate a new map request for a new area on 
      zoom/pan. */
   GlgSetZoomMode( MapVp, NULL, GISObject, NULL, GLG_GIS_ZOOM_MODE );

   GlgAddCallback( Drawing, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );
   GlgAddCallback( Drawing, GLG_TRACE_CB, (GlgCallbackProc)Trace, NULL );

   /* Initialize the drawing */
   Init();

   /* Create a group of target objects and add it to the drawing.
      The targets may be turned ON/OFF using the ShowTargets tollbar button.
      */
   CreateTargetsArray();

   /* Set initial layer string for the GISObject based on 
      layer menu's InitLayerStates array. It has to be done before 
      the hierarchy setup - before the map is generated using it. 
      */
   SetLayersFromMenu( LayersMenu, True );

   /* Display the drawing */
   GlgInitialDraw( Drawing );

   /* Start periodic updates */
   StartUpdate();   /* Install an update timer. */

   return GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| Initializes the drawing
*/
void Init()
{ 
   /* Obtain object ID of the viewport LayersMenu, from the LayersDialog
      object.
      */
   LayersMenu = GlgGetResourceObject( Drawing, "LayersDialog/LayersMenu" );

   /* Query and store initial map extent from the GIS object.
      It is used to reset the drawing to the initial extent 
      when the user clicks on the ZoomReset button. */
   GlgGetGResource( GISObject, "GISExtent",
		    &InitExtent.x, &InitExtent.y, &InitExtent.z );
   GlgGetGResource( GISObject, "GISCenter",
		    &InitCenter.x, &InitCenter.y, &InitCenter.z );

   /* Obtain an object ID of the Icon object and store in the IconData
      structure */
   IconData.icon = GlgGetResourceObject( GISObject, "GISArray/Icon" );

   /* Get initial position of the icon. Uses the center of the map
      by default. */
   GetIconPosition( &IconData );
   
   /* Position an icon */
   PositionIcon( &IconData );

   /* Make the DistancePopup object invisible. */
   GlgSetDResource( MapVp, "DistancePopup/Visibility", 0.0 );
}

/*----------------------------------------------------------------------
| Creates TargetGroup to hold target icons and add it to the map viewport.
| The group's Visibility may be toggled using ShowTargets tollbar button.
*/
void CreateTargetsArray()
{
   long i;

   NumTargets = sizeof( TargetArray ) / sizeof( TargetArray[ 0 ] );
   TargetTemplate = GlgGetResourceObject( GISObject, "GISArray/Target" );
   
   GlgReferenceObject( TargetTemplate );
   GlgDeleteThisObject( GISObject, TargetTemplate );

   TargetGroup = 
     GlgCreateObject( GLG_GROUP, "TargetGroup", NULL, NULL, NULL, NULL );

   /* Add target icons */
   for( i = 0; i < NumTargets; ++i )
     AddTarget( &TargetArray[ i ], i );
   
   GlgAddObjectToTop( GISObject, TargetGroup );
}

/*----------------------------------------------------------------------
| Adds a target icon, set tooltips, etc.
*/
void AddTarget( TargetData * target_data, long index )
{      
   GlgObject target;
   char * tooltip;

   /* Create a copy of a target template. */
   target = GlgCloneObject( TargetTemplate, GLG_STRONG_CLONE );

   GlgSetSResource( target, "Name", target_data->name );   /* Object name */

   /* Store index for each target object for direct access */     
   GlgSetDResource( target, "DataIndex", (double)index );

   /* Define the tooltip, including the lat/lon information and a target
      name.
      */
   sprintf( buffer, "%.500s lat=%lf lon=%lf", 
	   target_data->name, target_data->lat_lon.y, target_data->lat_lon.x );
   tooltip = buffer;

   GlgSetSResource( target, "TooltipString", tooltip );
	
   target_data->graphics = target;

   /* Set target's position in the drawing */
   GlgSetGResource( target_data->graphics, "Position",
		   target_data->lat_lon.x, target_data->lat_lon.y, 0. );

   GlgAddObjectToBottom( TargetGroup, target);
}


/*----------------------------------------------------------------------
| 
*/
void PositionIcon( IconDataStruct * icon_data )
{
   char * tooltip;

   /* Update icon position in the drawing */
   GlgSetGResource( icon_data->icon, "Position", 
		    icon_data->lat_lon.x, icon_data->lat_lon.y, 0. );
   GlgSetDResource( icon_data->icon, "Angle", icon_data->angle );

   /* Define icon tooltip, including the lat/lon information. */
   sprintf( buffer, "%.500s lat=%lf lon=%lf", 
	   "Flight 1237", icon_data->lat_lon.y, icon_data->lat_lon.x );
   tooltip = buffer;

   GlgSetSResource( icon_data->icon, "TooltipString", tooltip );
}

/*----------------------------------------------------------------------
| Creates a comma separated list of layer strings, based on the
| menu selection and assigns a layer string to the GIS object.
| If init parameter is true, uses menu's InitLayerStates array instead
| of button states to build the layer string.
*/
void SetLayersFromMenu( GlgObject menu_obj, GlgBoolean init )
{
   GlgObject 
     layer_state_list,
     layer_state_obj ;
   double 
     num_layers,
     layer_on;
   long i;
   char 
     * layer_name,
     * layers,
     * layers_buf,
     * res_name;

   layers = NULL;

   /* Query InitStateList object from the menu.  The number of elements
      in this list should correspond to the number of buttons in the
      menu. Each element has Name attribute corresponding to the
      layer name defined in the .sdf file, and its Value attribute
      defines the layer's state, i.e. whether this layer should be ON/OFF
      on initial appearance.
      */
   layer_state_list = GlgGetResourceObject( menu_obj, "InitStateList" );

   /* Query the number of layers. Number of layers is defined by the 
      NumRows attribute of the menu. */
   GlgGetDResource( menu_obj, "NumRows", &num_layers );

   /* Traverse the menu, determine which layer should be activated
      and build a layer string for the GISObject.
      */
   for( i = 0; i < (int)num_layers; ++i )
   {
      /* Retreive the element with index i from InitStateList.*/
      layer_state_obj = GlgGetElement( layer_state_list, i );   
   
      if( init )
      {
	 /* Get layer_on state from the menu's InitStateList array. */
	 GlgGetDResource( layer_state_obj, NULL, &layer_on );
      }
      else
      {
	 /* Get layer_on state from the corresponding menu button. */
	 res_name = GlgCreateIndexedName( "Button%/OnState", i );
	 GlgGetDResource( menu_obj, res_name, &layer_on );
	 GlgFree( res_name );
      }

      if( !layer_on )
	continue;
	 
      /* Layer with index i is active, add the layer name with index i
	 to the layer list. Layer name is stored in the Name attribute
	 of the corresponding element of the InitStateList. 
	 */
      GlgGetSResource( layer_state_obj, "Name", &layer_name );
      
      /* If layers string is not empty (not the first layer), 
	 add a comma to separate individual layer names. */
      if( layers )
      {
	 layers_buf = GlgConcatStrings( layers, "," );
	 GlgFree( layers );
	 layers = layers_buf;
      }
      
      layers_buf = GlgConcatStrings( layers, layer_name );
      GlgFree( layers );
      layers = layers_buf;
   }
   
   /* Assign the assembled list of active layers to the GISObject. */
   GlgSetSResource( GISObject, "GISLayers", layers ? layers : "" );

   GlgFree( layers );
}

/*----------------------------------------------------------------------
| Handle user interaction.
*/
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{
   GlgObject message_obj;

   char
     * full_origin,
     * origin,
     * format,
     * action,
     * subaction;

   message_obj = (GlgObject) call_data;

   GlgGetSResource( message_obj, "FullOrigin", &full_origin );
   GlgGetSResource( message_obj, "Origin", &origin );
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "SubAction", &subaction );

   /* Handle window closing. */
   if( strcmp( format, "Window" ) == 0 )
   {
      if( strcmp( action, "DeleteWindow" ) == 0 )
      {
	   /* Closing main window: exit. */
	   exit( 0 );
      }
      return;
   }

   if( strcmp( format, "Button" ) == 0 && 
      strcmp( action, "Activate" ) == 0 )
   {
      /* Process events from the toolbar buttons */
      PanMode = False;
      if( strcmp( origin, "ZoomIn" ) == 0 )
	Zoom( 'i', 2. );
      else if( strcmp( origin, "ZoomOut" ) == 0 )
	Zoom( 'o', 2. );
      else if( strcmp( origin,  "ZoomReset" ) == 0 )
	Zoom( 'n', 0. );
      else if( strcmp( origin, "ZoomTo" ) == 0 )
      {
	 PanMode = False;    /* Abort Pan mode */
	 GlgSetZoom( MapVp, NULL, 't', 0. );  /* Start Zoom op */
      }
      else if( strcmp( origin, "Pan" ) == 0 )
      {	    
	 GlgSetZoom( MapVp, NULL, 'e', 0. );  /* Abort ZoomTo mode */
	 PanMode = True;
      }
      else if( strcmp( origin, "ToggleLayers" ) == 0 ||
	      strcmp( full_origin, "LayersDialog/OKButton" ) == 0 )
      {
	 /* Popup/popdown a dialog to toggle map layers */
	 ToggleResource( Drawing, "LayersDialog/Visibility" );
      }      
      else if( strcmp( origin, "ShowTargets" ) == 0 )
      {
	 /* Turn ON/OFF targets. */
	 ToggleResource( GISObject, "GISArray/TargetGroup/Visibility" );
      }      
   }
   else if( strcmp( format, "CustomEvent" ) == 0 &&
	   strcmp( action, "MouseClick" ) == 0 )  
     /* Mouse click on the icon or target */
   {
      double zoom_mode, button_index;
      char * event_label;
      GlgObject temp_obj;

      GlgGetDResource( MapVp, "ZoomToMode", &zoom_mode );
      if( zoom_mode || PanMode )
	return;  /* Don't handle selection in ZoomTo and Pan modes. */

      GlgGetDResource( message_obj, "ButtonIndex", &button_index );
      if( button_index != 1 )
	return;    /* Ignore middle and right mouse button clicks */

      GlgGetSResource( message_obj, "EventLabel", &event_label );
      
      if( strcmp( event_label, "TargetSelected" ) == 0 )
       /* Process target selection event*/
      {
	 if( SelectedTarget )
	    ToggleResource( SelectedTarget, "ColorIndex" );

	 /* Retrieve an object ID of the selected object and highlight it
	    by chenging its color. */
	 temp_obj = GlgGetResourceObject( message_obj, "Object" );
	 
	 if( SelectedTarget == temp_obj )
	    SelectedTarget = NULL;
	 else 
	 {
	    ToggleResource( temp_obj, "ColorIndex" );   
	    SelectedTarget = temp_obj;
	 }
      }
      else if( strcmp( event_label, "PlaneSelected" ) == 0 )
      {
	 /* Process icon(plane) selection event here. */
      }
   }
   else if( strcmp( format, "Menu" ) == 0 &&
            strcmp( action, "Activate" ) == 0 )
   {
      /* Process events from the LayersMenu object in the LayersDialog */
      if( strcmp( origin, "LayersMenu" ) == 0 ) 
      {
	 /* Activate map layers based on the menu settings. */
	 SetLayersFromMenu( LayersMenu, False );
      }
   }

   GlgUpdate( Drawing );
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
|
*/
void Zoom( long type, double value )
{
   switch( type )
   {
    default:
      GlgSetZoom( MapVp, NULL, type, value );
      break;

    case 'n':
      /* Reset map to the initial extent. */
      GlgSetGResource( GISObject, "GISCenter",
		      InitCenter.x, InitCenter.y, InitCenter.z );
      GlgSetGResource( GISObject, "GISExtent", 
		      InitExtent.x, InitExtent.y, InitExtent.z );
      break;
   }
}

/*----------------------------------------------------------------------
| Used to obtain coordinates of the mouse click for Pan operation.
*/
void Trace( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{      
   GlgTraceCBStruct * trace_data;
   GlgPoint
     point,
     lat_lon;
   int
     event_type = 0,
     x, y;

   trace_data = (GlgTraceCBStruct*) call_data;

   /* Use the Map area events only. */
   if( trace_data->viewport != MapVp )
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
      x = GET_X_LPARAM( trace_data->event->lParam );
      y = GET_Y_LPARAM( trace_data->event->lParam );
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

      /* GLG_COORD_MAPPING_ADJ is added to the cursor coordinates for precise
         pixel mapping.
      */
      point.x = x + GLG_COORD_MAPPING_ADJ;
      point.y = y + GLG_COORD_MAPPING_ADJ;
      point.z = 0.;

      /* Converts X/Y to lat/lon using GIS object's current projection */
      GlgGISConvert( GISObject, NULL, GLG_SCREEN_COORD,
		    /* X/Y to Lat/Lon */ True,
		    &point, &lat_lon );

      /* Pan the map */
      GlgSetGResource( GISObject, "GISCenter", 
		       lat_lon.x, lat_lon.y, lat_lon.z );
      GlgUpdate( MapVp );
      break;

    default: return;
   } 
}


/*----------------------------------------------------------------------
| In application, update the position from real data source.
*/
void GetIconPosition( IconDataStruct * icon_data )
{   
   static double path_angle = 0.;
   double rad_angle, radius = 10.;

#define ANGLE_INCREMENT    0.2

   path_angle += ANGLE_INCREMENT;
   if( path_angle >= 360. )
     path_angle = 0.;

   rad_angle = DegToRad( path_angle );
   icon_data->lat_lon.x = InitCenter.x + radius * cos( rad_angle );
   icon_data->lat_lon.y = InitCenter.y + radius * sin( rad_angle  );
   icon_data->lat_lon.z = 0.;
   icon_data->angle = path_angle + 90.;
}

/*----------------------------------------------------------------------
| Installs a timer procedure to update the simulation.
*/
void UpdateIconPosition( GlgAnyType client_data, GlgLong * timer_id )
{
   GetIconPosition( &IconData );   /* Get new position */
   PositionIcon( &IconData );      /* Set new position of the icon */

   /* Display distance between the icon and a selected target, if any */
   DisplayDistance();

   GlgUpdate( MapVp );
   GlgSync( Drawing );    /* Improves interactive response */

   /* Reinstall timer */
   GlgAddTimeOut( AppContext, UPDATE_INTERVAL,
		 (GlgTimerProc)UpdateIconPosition, client_data );
}

/*----------------------------------------------------------------------
| Calculates distance in meters between the moving icon and currently
| selected target, and displays the distance in the text object
| named DistancePopup.
*/
void DisplayDistance()
{
   double 
      d_index,
      distance;

   /* If no target is selected with the mouse, don't do anything. */
   if( !SelectedTarget )
   {
      GlgSetDResource( MapVp, "DistancePopup/Distance", 0.0 );
      GlgSetSResource( MapVp, "DistancePopup/DestinationString", "" );
      GlgSetDResource( MapVp, "DistancePopup/Visibility", 0.0 );
      return;
   }

   /* Make DistancePopup object visible. */
   GlgSetDResource( MapVp, "DistancePopup/Visibility", 1.0 );

   /* Use DataIndex resource of the SelectedTarget object
      for direct access in the TargetArray.
      */
   GlgGetDResource( SelectedTarget, "DataIndex", &d_index );
   distance = GetGlobeDistance( IconData.lat_lon, 
				TargetArray[(int)d_index ].lat_lon );

   /* Display distance valu in km */
   GlgSetDResource( MapVp, "DistancePopup/Distance", distance / 1000. );
   
   /* Set the DestinationString, indicating the currently selected
      target name 
      */
   GlgSetSResource( MapVp, "DistancePopup/DestinationString",
		   TargetArray[(int)d_index ].name );

}

/*----------------------------------------------------------------------
| Returns length (in meters) in 3D between two points defined in
| lat/lon coordinates. Use more complex math for curves around the Earth.
*/
double GetGlobeDistance( GlgPoint lat_lon1, GlgPoint lat_lon2 )
{
   GlgPoint globe_point1, globe_point2;

   /* XYZ of the first point, in meters */
   GetPointXYZ( &lat_lon1, &globe_point1 );

   /* XYZ of the second point, in meters */
   GetPointXYZ( &lat_lon2, &globe_point2 );

   return sqrt( 
     ( globe_point1.x - globe_point2.x ) * ( globe_point1.x - globe_point2.x )+
     ( globe_point1.y - globe_point2.y ) * ( globe_point1.y - globe_point2.y )+
     ( globe_point1.z - globe_point2.z ) * ( globe_point1.z - globe_point2.z ) 
	       );
}

/*----------------------------------------------------------------------
|
*/
void GetPointXYZ( GlgPoint * lat_lon, GlgPoint * xyz )
{
   double
     angle_x,
     angle_y;

   /* Place [0,0] at the math's x axis for simplicity. */

   angle_x = DegToRad( lat_lon->x );
   angle_y = DegToRad( lat_lon->y );

   xyz->x = GLG_EQUATOR_RADIUS * cos( angle_x ) * cos( angle_y );
   xyz->y = GLG_EQUATOR_RADIUS * sin( angle_y );
   xyz->z = GLG_EQUATOR_RADIUS * sin( angle_x ) * cos( angle_y );
}


/*----------------------------------------------------------------------
| Installs a timer to update the simulation.
*/
void StartUpdate()
{
   GlgAddTimeOut( AppContext, UPDATE_INTERVAL,
		 (GlgTimerProc)UpdateIconPosition, (GlgAnyType)NULL );
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

