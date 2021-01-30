#include <stdio.h>
#include <stdlib.h>
/**********************************************************************
| Supported command line arguments:
| -random-data  
|     Use simulated demo data for animation.
| -live-data
|     Use live application data.
| <filename>
|        specifies GLG drawing filename to be loaded and animated;
|        if not defined, DEFAULT_DRAWING_FILENAME is used.
***********************************************************************/

#include <math.h>
#include "gis_example.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

/* If set to true, simulated demo data will used be used for
   animation. Otherwise, live application data will be used.
   This flag can be overriden by the command line argument
   -random-data or -live-data.
*/
#define RANDOM_DATA          True

/* Default GLG drawing. Can be overriden by supplying a drawing name
   as a command line argument.
*/
#define DEFAULT_DRAWING_FILENAME "gis_example.g"

#define  UPDATE_INTERVAL     50     /* 50ms timer = 20 times/sec refresh */
#define  MAP_ZOOM_FACTOR     1.2
#define  MAP_PAN_FACTOR      .2

/* A choice of the 2 icon symbols defined in the .g file.
   A desired icon is specfied in the Init() function.
*/
#define ICON_NAME1          "Aircraft"
#define ICON_NAME2          "Icon_Triangle"

/* Specify SDF file as needed, if different from the setting in the .g file. 
   Assign the specified file for the GISObject in the InitBeforeH() function.
*/
#define GIS_DATA_FILE       "../../map_data/sample.sdf"

GlgObject 
   Drawing,    /* Main viewport of the drawing */
   MapVp,      /* Map viewport containing a GIS object */
   GISObject;  /* GIS object */

/* Stores icon information */
IconData Icon;

/* Store initial extent and center, used to reset the drawing */
GlgPoint
  InitExtent,
  InitCenter;

GlgLong TimerID;
   
// Temporary points, allocate once.
GlgPoint point;
GlgPoint lat_lon;

GlgBoolean RandomData = RANDOM_DATA;

GlgAppContext AppContext;   /* Global, used to install a work procedure. */

#include "GlgMain.h"    /* Cross-platform entry point. */

/*----------------------------------------------------------------------
|
*/
int GlgMain( int argc, char *argv[], GlgAppContext app_context )
{
   char * drawing_filename = NULL;
   int skip;

   AppContext = GlgInit( False, app_context, argc, argv );

   GlgSetDResource( (GlgObject)0, "$config/GlgMouseTooltipTimeout", 0.05 );

   /* Process command line arguments. */
   for( skip = 1; skip < argc; ++skip )
   {
      if( strcmp( argv[ skip ], "-random-data" ) == 0 )
      {
         /* Use simulated demo data for animation. */
         RandomData = True;
         GlgError( GLG_INFO, (char *) "Using simulated data for animation." );
      }
      else if( strcmp( argv[ skip ], "-live-data" ) == 0 )
      {
         /* Use live application data for animation. */
         RandomData = False;
         GlgError( GLG_INFO, 
              (char *) "Using live application data for animation." );
      }
      else if( strncmp( argv[skip], "-", 1 ) == 0 )
        continue;
      else
      {
         /* Use the drawing file from the command line, if any. */
         drawing_filename = argv[ skip ];
      }
   }

   /* If drawing file is not supplied on the command line, use 
      default drawing filename defined by DEFAULT_DRAWING_FILENAME.
   */
   if( !drawing_filename )
   {
      GlgError( GLG_INFO, 
           (char *) "Using default drawing gis_example.g." );
      drawing_filename = DEFAULT_DRAWING_FILENAME;
   }

   /* Load the GLG drawing. */
   Drawing = GlgLoadWidgetFromFile( drawing_filename );
   if( !Drawing )
   {
      GlgError( GLG_USER_ERROR, "Can't load drawing file." );
      exit( GLG_EXIT_ERROR );
   }

   /* Set window size. */
   SetSize( Drawing, 0, 0, 800, 700 );

   /* Set window title. */
   GlgSetSResource( Drawing, "ScreenName", "GLG GIS Example" );

   /* Add callbacks. */
   GlgAddCallback( Drawing, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );
   GlgAddCallback( Drawing, GLG_TRACE_CB, (GlgCallbackProc)Trace, NULL );

   /* Initialize the drawing before hierarchy setup. */
   InitBeforeH();

   /* Setup object hierarchy, to be able to perform coordinate
      conversion using GISConvert. The drawing is not displayed yet.
   */
   GlgSetupHierarchy( Drawing );

   /* Initialize the drawing after hierarchy setup. */
   InitAfterH();

   /* Display the drawing. */
   GlgUpdate( Drawing );

   /* Start periodic dynamic updates. */
   StartUpdates();

   return GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| Initialize the drawing before hierarchy setup.
*/
void InitBeforeH()
{
   /* Obtain an object ID of the viewport named "MapVp", a GIS object
      named "GISObject" and a group containing other objects inside the
      GIS object. 
   */
   MapVp = GlgGetResourceObject( Drawing, "MapVp" );
   GISObject = GlgGetResourceObject( MapVp, "GISObject" );

   /**************** GIS DATA FILE ***************/
   /* Uncomment the line below to assign GISDataFile, if different 
      from definition in the .g file.
   */
   /* GlgSetSResource( GISObject, "GISDataFile", GIS_DATA_FILE ); */

   /* Set GIS Zoom mode: generate a new map request for a new area on 
      zoom/pan. This information is stored in the drawing, but just in case
      we are setting here as well. 
   */
   GlgSetZoomMode( MapVp, NULL, GISObject, NULL, GLG_GIS_ZOOM_MODE );

   /* Query and store initial map extent and center.
      It is used to reset the drawing to the initial extent 
      when the user clicks on the ZoomReset button. 
   */
   GlgGetGResource( GISObject, "GISExtent",
		    &InitExtent.x, &InitExtent.y, &InitExtent.z );
   GlgGetGResource( GISObject, "GISCenter",
		    &InitCenter.x, &InitCenter.y, &InitCenter.z );
   
   /* Obtain and store object ID of the icon with a specified icon name. */
   Icon.icon_obj = GetIconObject( ICON_NAME1 );

   /* Make icon object visible. */
   SetIconVisibility( Icon.icon_obj, GlgTrue );

   /* Hide other/unused icon(s). */
   GlgObject icon = GetIconObject( ICON_NAME2 );
   SetIconVisibility( icon, GlgFalse );
}

/*----------------------------------------------------------------------
| Initialize the drawing after hierarchy setup.
*/
void InitAfterH()
{
   /* Set initial position of the icon. Uses the center of the map
      by default. 
   */
   GetIconData( &Icon );
   PositionIcon( &Icon );
   
   /* Set a tooltip string for the icon */
   GlgSetSResource( Icon.icon_obj, "TooltipString", "Flight 1237" ); 
}

/*----------------------------------------------------------------------
| Start periodic dynamic updates.
*/
void StartUpdates()
{
   TimerID = GlgAddTimeOut( AppContext, UPDATE_INTERVAL,
                            (GlgTimerProc)UpdateDrawing, 
                            (GlgAnyType)NULL );
}

/*----------------------------------------------------------------------
| Stop dynamic updates.
*/
void StopUpdates()
{
   /* Stop update timers */
   if( TimerID )
   {
      GlgRemoveTimeOut( TimerID );
      TimerID = 0;
   }
}

/*----------------------------------------------------------------------
| Animate the drawing.
*/
void UpdateDrawing( GlgAnyType client_data, GlgLong * timer_id )
{
   GetIconData( &Icon );       /* Get new position */
   PositionIcon( &Icon );      /* Set new icon position */
   
   GlgUpdate( Drawing );       /* Refresh display */
   GlgSync( Drawing );         /* Improves interactive response */

   /* Reinstall the timer. */
   GlgAddTimeOut( AppContext, UPDATE_INTERVAL,
		 (GlgTimerProc)UpdateDrawing, client_data );
}

/*--------------------------------------------------------------------
| Obtain data values for the icon position and other icon parameters.
*/
void GetIconData( IconData * icon )
{
   if( RandomData )
     GetDemoData( icon );
   else
     GetLiveData( icon );
}

/*----------------------------------------------------------------------
| Set icon position in lat/lon coordinates.
*/
void PositionIcon( IconData * icon )
{
   /* Update icon position in the drawing */
   GlgSetGResource( icon->icon_obj, "Position", 
		    icon->lat_lon.x, icon->lat_lon.y, 0. );
   GlgSetDResource( icon->icon_obj, "Yaw", icon->angle );
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

      if( strcmp( origin, "ZoomIn" ) == 0 )
	Zoom( 'i', MAP_ZOOM_FACTOR );
      else if( strcmp( origin, "ZoomOut" ) == 0 )
	Zoom( 'o', MAP_ZOOM_FACTOR );
      else if( strcmp( origin,  "ZoomReset" ) == 0 )
	Zoom( 'n', 0. );
      else if( strcmp( origin, "ZoomTo" ) == 0 )
	 GlgSetZoom( MapVp, NULL, 't', 0. );  /* Start Zoom op */
      else if( strcmp( origin, "Up" ) == 0 )
        Zoom( 'u', MAP_PAN_FACTOR );
      else if( strcmp( origin, "Down" ) == 0 )
        Zoom( 'd', MAP_PAN_FACTOR );
      else if( strcmp( origin, "Left" ) == 0 )
        Zoom( 'l', MAP_PAN_FACTOR );
      else if( strcmp( origin, "Right" ) == 0 )
        Zoom( 'r', MAP_PAN_FACTOR );

      GlgUpdate( MapVp );
   }

   /* Handle custom mouse click events. */
   else if( strcmp( format, "CustomEvent" ) == 0 &&
            strcmp( action, "MouseClick" ) == 0 )
   {
      char * custom_event;
      GlgObject selected_obj;
      
      /* Map dragging mode is activated on a left mouse click in the Trace 
         callback. Abort the dragging mode if an object with custom event
         was selected. This gives custom events a higher priority compared 
         to the dragging mode. However, if ZoomTo mode was activated 
         from a ZoomTo button, don't abort ZoomTo mode and ignore 
         object selection.
      */
      int zoom_mode = ZoomToMode();
      if(  zoom_mode == 0 ||
           ( zoom_mode & GLG_PAN_DRAG_STATE ) != 0 )
      {
         if( zoom_mode != 0 )
           GlgSetZoom( MapVp, NULL, 'e', 0. );  /* Abort zoom/drag mode */
         
         GlgGetSResource( message_obj, "EventLabel", &custom_event );
         selected_obj = GlgGetResourceObject( message_obj, "Object" );

         /* Do something with the selected object, as needed.
            For example, if the icon was selected, extract its
            position and display it at the bottom of the drawing.
         */
         if( strcmp( custom_event, "IconSelected" ) == 0 )
         {
            GlgGetGResource( selected_obj, "Position", 
                             &lat_lon.x, &lat_lon.y, &lat_lon.z );
            ShowInfoDisplay( GlgTrue, &lat_lon );

            printf( "Selected Icon Position = %lf %lf %lf\n", 
                    lat_lon.x, lat_lon.y, lat_lon.z );
         }
      }
      
      GlgUpdate( Drawing );
   }
}

/*----------------------------------------------------------------------
| Trace callback. Used to handle low level events.
| In this example, it handles:
| - map dragging with the left mouse button;
| - query and display lat/lon coordinates at the cursor position, using
|   the right mouse button;
| - erase lat/lon display using ESC key. 
*/
void Trace( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{      
   GlgTraceCBStruct * trace_data;
   GlgObject event_vp;
   long
     i,
     event_type = 0,
     width, height,
     button = 0;

   trace_data = (GlgTraceCBStruct*) call_data;
   event_vp = trace_data->viewport;

   /* Platform-specific code to extract event information. */
#ifndef _WINDOWS /* X/Linux */

#define TEXT_BUFFER_LENGTH 128

   XEvent * event;
   KeySym keysym;
   XComposeStatus status;
   char buf[ TEXT_BUFFER_LENGTH ];
   int length;

   event = trace_data->event;
   switch( event->type )
   {
    case ButtonPress:
      point.x = event->xbutton.x;
      point.y = event->xbutton.y;
      button = event->xbutton.button;
      switch( button )
      {
       case 1:
       case 2:
       case 3:
         event_type = BUTTON_PRESS_EVENT;
         break;
       case 4:
       case 5:
         event_type = MOUSE_WHEEL_EVENT;
         break;
      }
      break;
      
    case MotionNotify:
      point.x = event->xmotion.x;
      point.y = event->xmotion.y;
      event_type = MOUSE_MOVE_EVENT;
      break;

    case ConfigureNotify:
      width = event->xconfigure.width;
      height = event->xconfigure.height;
      event_type = RESIZE_EVENT;
      break;

    case KeyPress:
    case KeyRelease:
      /* Using XLookupString instead of XLookupKeysym to properly handle 
         Shift. 
      */
      length = XLookupString( &event->xkey, buf, TEXT_BUFFER_LENGTH,
                              &keysym, &status );
      buf[ length ] = '\0';
      switch( keysym )
      {
       default: break;
       case XK_Escape:
         /* Erase text information displayed at the bottom of the drawing. */
         ShowInfoDisplay( GlgFalse, NULL );
         GlgUpdate( Drawing );
         break;
      }
      break;
      
    default: return;
   }

#else /* Windows */

   switch( trace_data->event->message )
   {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MOUSEMOVE:
      point.x = GET_X_LPARAM( trace_data->event->lParam );
      point.y = GET_Y_LPARAM( trace_data->event->lParam );
      point.z = 0.;

      switch( trace_data->event->message )
      {
       case WM_LBUTTONDOWN:
         button = 1;
         event_type = BUTTON_PRESS_EVENT;
         break;
       case WM_RBUTTONDOWN:
         button = 3;
         event_type = BUTTON_PRESS_EVENT;
         break;
       case WM_MOUSEMOVE:
         event_type = MOUSE_MOVE_EVENT;
         break;
      }
      break;

    case WM_SIZE:
      width = LOWORD( trace_data->event->lParam );
      height = HIWORD( trace_data->event->lParam );
      event_type = RESIZE_EVENT;
      break;

    case WM_CHAR:
    case WM_KEYDOWN: 
      switch( trace_data->event->wParam )
      {
       default: break;
       case VK_ESCAPE:    /* ESC key - 0x1B */
         /* Erase text information displayed at the bottom of the drawing. */
         ShowInfoDisplay( GlgFalse, NULL );
         GlgUpdate( Drawing );
         break;

	 /* Add custom code to handle other keys as needed. */
      }
      break;
      
    case WM_MOUSEWHEEL:
      int zDelta;
      zDelta = GET_WHEEL_DELTA_WPARAM( trace_data->event->wParam );

      if( zDelta > 0 )   /* Mousewheel is moved up. */
        button = 4;
      else               /* Mousewheel is moved down. */
        button = 5;

      event_type = MOUSE_WHEEL_EVENT;      
      break;

    default: return;
   }

#endif

   switch( event_type )
   {
    case BUTTON_PRESS_EVENT:
      if( ZoomToMode() != 0 )
        return;   /* ZoomTo or dragging mode in progress: pass it through. */

      /* GLG_COORD_MAPPING_ADJ is added to the cursor coordinates for precise
         pixel mapping.
      */
      point.x += GLG_COORD_MAPPING_ADJ;
      point.y += GLG_COORD_MAPPING_ADJ;

      switch( button )
      {
       case 1:  
         /* Start dragging the map with the mouse on a left mouse click. */
         GlgSetZoom( MapVp, NULL, 's', 0.0 );
         GlgUpdate( MapVp );
         break;
       case 2: break;
       case 3:
         /* On Right mouse click, obtain lat/lon values at cursor 
            position and display this information in the drawing.
         */
         if( GetLatLonInfo( &point, &lat_lon ) )
           ShowInfoDisplay( GlgTrue, &lat_lon );
         else
           ShowInfoDisplay( GlgTrue, NULL );

         GlgUpdate( Drawing );
         break;
      }
      break;

    case MOUSE_WHEEL_EVENT:
      /* In this example, the map is zoomed in/out by the specified factor
         for each mouse wheel scroll event. The application may extend the
         logic to fine-tune the map scrolling amount. On Linux/Unix, XInput2
         extension can be used to obtain the mouse wheel scroll amount.
         On Windows, zDelta parameter is provided by the Windows WM_MOUSEWHEEL
         message.
      */
      switch( button )
      {
       case 4:  
         /* Mousewheel is scrolled up, ZoomIn the map. */
         Zoom( 'i', MAP_ZOOM_FACTOR ); 
         break;

       case 5: 
         /* Mousewheel is scrolled down, ZoomOut the map. */
         Zoom( 'o', MAP_ZOOM_FACTOR );
         break;

       default: break;
      }

      GlgUpdate( MapVp );
      break;

    case MOUSE_MOVE_EVENT:
    case RESIZE_EVENT:
    default: 
      break;
   }
}

/*--------------------------------------------------------------------
| Returns object ID of the icon with a specified name.
*/
GlgObject GetIconObject( char * icon_name )
{
   char * res_name;
   GlgObject icon;

   res_name = GlgConcatResNames( "GISArray", icon_name );
   icon = GlgGetResourceObject( GISObject, res_name );
   GlgFree( res_name );

   if( !icon )
     GlgError( GLG_USER_ERROR, (char *) "Icon not found." );
   
   return icon;
}

/*--------------------------------------------------------------------
| Set icon visibility.
*/
void SetIconVisibility( GlgObject icon, GlgBoolean show )
{
   if( !icon )
   {
      GlgError( GLG_USER_ERROR, (char *) "Icon not found." );
      return;
   }

   /* Show/hide the icon. */
   GlgSetDResource( icon, "Visibility", show ? 1. : 0. );
}

/*----------------------------------------------------------------------
| Retrieve lat/lon information at the specified cursor position.
*/
GlgBoolean GetLatLonInfo( GlgPoint * in_point, GlgPoint * out_point )
{
   /* Convert cursor position to lat/lon. */
   return GlgGISConvert( GISObject, NULL, GLG_SCREEN_COORD,
                         /* X/Y to Lat/Lon */ GlgTrue, 
                         in_point, out_point );
}

/*----------------------------------------------------------------------
| 
*/
void ShowInfoDisplay( GlgBoolean show, GlgPoint * lat_lon )
{
   /* InfoObject/Status resource toggles text display:
      Status=0 displays general prompt;
      Status=1 displays lat/lon values.
   */
   GlgSetDResource( Drawing, "InfoObject/Status", show ? 1. : 0. );
   GlgSetDResource( Drawing, "InfoObject/Visibility", 1.0 );
   
   if( !lat_lon )
   {
      /* Initialize LAT/LON to an undefined value (such as -1000). */
      GlgSetDResource( Drawing, "InfoObject/LAT", -1000. );
      GlgSetDResource( Drawing, "InfoObject/LON", -1000. );
   }
   else
   {
      GlgSetDResource( Drawing, "InfoObject/LAT", lat_lon->y );
      GlgSetDResource( Drawing, "InfoObject/LON", lat_lon->x );
   }
}

/*--------------------------------------------------------------------
| Returns map viewport's ZoomTo mode.
*/
int ZoomToMode()
{
   double zoom_mode;

   GlgGetDResource( MapVp, "ZoomToMode", &zoom_mode );

   return (int)zoom_mode;
}

/*----------------------------------------------------------------------
| Perform zoom operation.
*/
void Zoom( char type, double value )
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
| Set viewport size in screen cooridnates. 
*/
void SetSize( GlgObject viewport, GlgLong x, GlgLong y, 
              GlgLong width, GlgLong height )
{
   GlgSetGResource( viewport, "Point1", 0., 0., 0. );
   GlgSetGResource( viewport, "Point2", 0., 0., 0. );

   GlgSetDResource( viewport, "Screen/XHint", (double) x );
   GlgSetDResource( viewport, "Screen/YHint", (double) y );
   GlgSetDResource( viewport, "Screen/WidthHint", (double) width );
   GlgSetDResource( viewport, "Screen/HeightHint", (double) height );
}

/*----------------------------------------------------------------------
| Generate simulated demo data.
*/
GlgBoolean GetDemoData( IconData * icon )
{   
   static int RotationState = 0;
   double radius = InitExtent.x / 5.0;
    
    ++RotationState;
    if( RotationState > 360 )
      RotationState -= 360;
    
    double angle = RotationState;
    double rad_angle = angle / 180. * M_PI;

    icon->lat_lon.x = InitCenter.x + radius * cos( rad_angle );
    icon->lat_lon.y = InitCenter.y + radius * sin( rad_angle );
    icon->lat_lon.z = 0.;
    icon->angle = angle + 90.;

    return GlgTrue;
}

/*----------------------------------------------------------------------
| Provide a custom implementation for animation.
*/
GlgBoolean GetLiveData( IconData * icon )
{   
   /* Place custom code here to obtain live application data. */
   return GlgTrue;
}

