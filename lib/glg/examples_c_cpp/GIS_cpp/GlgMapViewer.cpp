#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#include "GlgMapViewer.h"

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

// Function prototype.
extern "C" void OnTimerEvent( GlgMapViewer *viewer, GlgLong * timer_id );

// Temporary points, allocate once.
GlgPoint point;
GlgPoint lat_lon;

/*----------------------------------------------------------------------
| Constructor. 
*/
GlgMapViewer::GlgMapViewer()
{
   UpdateInterval = 100; 
   TimerID = 0;
   has_parent = false;
   DataFeed = NULL;
   Icon = new IconData();

   // Set to False to use live data by default. 
   // It can be overriden by a command line option, 
   // -random-data or -live-data
   RANDOM_DATA = True; 
}

/*----------------------------------------------------------------------
| Destructor. 
*/
GlgMapViewer::~GlgMapViewer()
{
   // Free memory allocated for the Icon object.
   if( Icon )
     delete Icon;

   // Delete DataFeed object, if defined. 
   if( DataFeed )
     delete DataFeed;
}

/*----------------------------------------------------------------------
| Add DataFeed object for supplying data for animation.
*/
void GlgMapViewer::AddDataFeed( DataFeedC * data_feed )
{
   if( DataFeed )
     delete DataFeed;
   
   DataFeed = data_feed;
}

/*----------------------------------------------------------------------
| Initialize drawing parameters as needed. Callbacks must be enabled
| before hierarchy setup, in InitBeforeH(). Init() method is invoked
| only if the viewer is used outside of the Windows MFC environment 
| (has_parent=false). In the MFC environment, custom GlgControl is
| created which has its own Init() method. 
*/
void GlgMapViewer::Init( void )
{
   /* Initialize drawing parameters before hierarchy is set up. */
   InitBeforeH();
   
   /* Set up object hierarchy. */ 
   SetupHierarchy();

   /* Initialize drawing parameters after hierarchy setup took place. */
   InitAfterH();
}

/*--------------------------------------------------------------------
| Place custom code as needed to initialize  the drawing before
| hierarchy setup took place.
*/
void GlgMapViewer::InitBeforeH()
{
   /* Callbacks must be enabled before  hierarchy setup.
      Enable callbacks only if the viewer object is a top level window and 
      there is no parent. 
      
      When this class is used in an MFC application, GlgSCADAViewer is 
      displayed in a GLG MFC control, has_parent=true and callbacks are 
      enabled at the parent level in GlgMapViewerControl::Init() before 
      the hierarchy setup.
   */
   if( !has_parent )
   {
      EnableCallback( GLG_INPUT_CB, NULL );
      EnableCallback( GLG_TRACE_CB, NULL );
   }

   /* Obtain an object ID of the viewport named "MapVp" and 
      GIS object named "GISObject".
   */
   MapVp = GetResourceObject( "MapVp" );
   GISObject = MapVp.GetResourceObject( "GISObject" );
   
   /**************** GIS DATA FILE ***************/
   /* Uncomment the line below to assign GISDataFile, if different 
      from definition in the .g file.
   */
   //GISObject.SetResource( "GISDataFile", GIS_DATA_FILE );

   // Set GIS Zoom mode: generate a new map request for a new area on zoom/pan.
   MapVp.SetZoomMode( NULL, &GISObject, NULL, GLG_GIS_ZOOM_MODE );
   
   /* Query and store initial map extent and center. 
      It is used to reset the drawing to the initial extent 
      when the user clicks on the ZoomReset button.
   */
   GISObject.GetResource( "GISExtent", &InitExtent.x, 
                          &InitExtent.y, &InitExtent.z  );

   GISObject.GetResource( "GISCenter", &InitCenter.x, 
                          &InitCenter.y, &InitCenter.z  );

   // Obtain and store an object ID of the icon with a specified icon name.
   GlgObjectC icon;
   icon = GetIconObject( ICON_NAME1 );
   Icon->icon_obj = icon;

   // Make the icon visible.
   SetIconVisibility( icon, GlgTrue );

   // Hide other/unused icon(s).
   icon = GetIconObject( ICON_NAME2 );
   SetIconVisibility( icon, GlgFalse );
}

/*--------------------------------------------------------------------
| Initialize the drawing after hierarchy setup.
*/
void GlgMapViewer::InitAfterH()
{
   /* Set initial position of the icon. Uses the center of the map
      by default. 
   */
   GetIconData( Icon );
   PositionIcon( Icon );
   
   /* Set a tooltip string for an icon */
   Icon->icon_obj.SetResource( "TooltipString", "Flight 1237" ); 
}

/*----------------------------------------------------------------------
| Timer procedure, invoked periodically on a timer to update the
| drawing with new data values.
*/
void OnTimerEvent( GlgMapViewer *viewer, GlgLong * timer_id )
{
   viewer->UpdateDrawing();

   /* Restart the timer. */
   viewer->TimerID = GlgAddTimeOut( viewer->AppContext, 
                                    viewer->UpdateInterval, 
                                    (GlgTimerProc)OnTimerEvent, 
                                    viewer );
}

/*--------------------------------------------------------------------
| Animate the drawing.
*/
void GlgMapViewer::UpdateDrawing()
{
   GetIconData( Icon );       // Get new position
   PositionIcon( Icon );      // Set new icon position

   Update();                  // Refresh display
   Sync();    /* Improves interactive response. */
}

/*--------------------------------------------------------------------
|
*/
void GlgMapViewer::GetIconData( IconData * icon )
{
   if( !DataFeed )
   {
      GlgError( GLG_USER_ERROR, 
              (char *)"Invalid DataFeed object, drawing animation failed." );
      return;
   }

   DataFeed->GetIconData( icon );
}

/*--------------------------------------------------------------------
| Position an icon on the map.
*/
void GlgMapViewer::PositionIcon( IconData * icon )
{
   if( !icon || icon->icon_obj.IsNull() )
   {
      GlgError( GLG_USER_ERROR, (char *) "Invalid icon object." );
      return;
   }

   // Update icon position in the drawing.
   icon->icon_obj.SetResource( "Position", icon->lat_lon.x, 
                               icon->lat_lon.y, 0. );
   icon->icon_obj.SetResource( "Yaw", icon->angle );
}

/*----------------------------------------------------------------------
| Handle user interaction. 
*/
void GlgMapViewer::Input( GlgObjectC& viewport, GlgObjectC& message )
{
   CONST char
     * format,
     * action,
     * origin;
      
   GlgBoolean status = False;
   double d_value = -1.;;

   /* Get the message's format, action and origin. */
   message.GetResource( "Format", &format );
   message.GetResource( "Action", &action );
   message.GetResource( "Origin", &origin );

   /* Handle window closing. May use viewport's name. */
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );

   else if( strcmp( format, "Button" ) == 0 )
   {
      if(  strcmp( action, "Activate" ) !=0 )
        return;
      
      if( strcmp( origin, "ZoomIn")  == 0 )
         Zoom( 'i', MAP_ZOOM_FACTOR );
      else if( strcmp( origin, "ZoomOut")  == 0 )
         Zoom( 'o', MAP_ZOOM_FACTOR );
      else if( strcmp( origin, "ZoomReset")  == 0 )
         Zoom( 'n', 0. );
      else if( strcmp( origin, "ZoomTo")  == 0 )
        MapVp.SetZoom( NULL, 't', 0. );  /* Start Zoom op */
      else if( strcmp( origin, "Up")  == 0 )
         Zoom( 'u', MAP_PAN_FACTOR );
      else if( strcmp( origin, "Down")  == 0 )
         Zoom( 'd', MAP_PAN_FACTOR );
      else if( strcmp( origin, "Left")  == 0 )
         Zoom( 'l', MAP_PAN_FACTOR );
      else if( strcmp( origin, "Right")  == 0 )
         Zoom( 'r', MAP_PAN_FACTOR );

      /* Refresh the display. */
      MapVp.Update();
   } 

   // Handle custom mouse click events.
   else if( strcmp( format, "CustomEvent" ) == 0 &&
            strcmp( action, "MouseClick" )  == 0 )
   {
      CONST char * custom_event;
      GlgObjectC selected_obj;
 
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
           MapVp.SetZoom( NULL, 'e', 0. );  /* Abort zoom/drag mode */
         
         message.GetResource( "EventLabel", &custom_event );
         selected_obj = message.GetResourceObject( "Object" );

         /* Do something with the selected object, as needed.
            For example, if the icon was selected, extract its
            position and display it at the bottom of the drawing.
         */
         if( strcmp( custom_event, "IconSelected" ) == 0 )
         {
            selected_obj.GetResource( "Position", 
                                      &lat_lon.x, &lat_lon.y, &lat_lon.z );
            ShowInfoDisplay( GlgTrue, &lat_lon );

            printf( "Selected Icon Position = %lf %lf %lf\n", 
                    lat_lon.x, lat_lon.y, lat_lon.z );
         }

         viewport.Update();
      }
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
void GlgMapViewer::Trace( GlgObjectC& viewport, 
                          GlgTraceCBStruct * trace_data )
{
   GlgObjectC event_vp;
   int
     event_type = 0,
     width, height,
     button = 0;

   event_vp = trace_data->viewport;

   /* Platform-specific code to extract event information. */
#ifndef _WINDOWS  /* X/Linux */

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
         Shift. */
      length = XLookupString( &event->xkey, buf, TEXT_BUFFER_LENGTH,
                              &keysym, &status );
      buf[ length ] = '\0';
      switch( keysym )
      {
       default: break;
       case XK_Escape:
         // Erase text information displayed at the bottom of the drawing.
         ShowInfoDisplay( GlgFalse, NULL );
         viewport.Update();
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
       case WM_RBUTTONDOWN:
         button = 3;
         event_type = BUTTON_PRESS_EVENT;
         break;
       case WM_LBUTTONDOWN:
         button = 1;
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
       case VK_ESCAPE: // ESC key  0x1B
         // Erase text information displayed at the bottom of the drawing.
         ShowInfoDisplay( GlgFalse, NULL );
         viewport.Update();
         break;

	 /* Add custom code to handle other keys as needed. */
      }
      break;
      
    case WM_MOUSEWHEEL:
      int zDelta;
      zDelta = GET_WHEEL_DELTA_WPARAM( trace_data->event->wParam );

      if( zDelta > 0 ) // Mousewheel is moved up, ZoomIn the map
        Zoom( 'i', MAP_ZOOM_FACTOR );
      else // Mousewheel is moved down, ZoomOut the map
        Zoom( 'o', MAP_ZOOM_FACTOR );
      
      MapVp.Update();
      break;

    default: return;
   }

#endif
   
   switch( event_type )
   {
    case BUTTON_PRESS_EVENT:
      if( ZoomToMode() != 0 )
        return; // ZoomTo or dragging mode in progress: pass it through.

      /* GLG_COORD_MAPPING_ADJ is added to the cursor coordinates for precise
         pixel mapping.
      */
      point.x += GLG_COORD_MAPPING_ADJ;
      point.y += GLG_COORD_MAPPING_ADJ;

      switch( button )
      {
       case 1:  
         // Start dragging the map with the mouse on a left mouse click.
         MapVp.SetZoom( NULL, 's', 0.0 );
         MapVp.Update();
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

         viewport.Update();
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
| Returns object ID (GlgObject) of the icon with a specified name.
*/
GlgObject GlgMapViewer::GetIconObject( CONST char * icon_name )
{
   char * icon_res_name = GlgConcatResNames( (char *) "GISArray", 
                                             (char *) icon_name );
   GlgObject icon_obj = GISObject.GetResourceObject( icon_res_name );
   GlgFree( icon_res_name );

   return icon_obj;
}

/*--------------------------------------------------------------------
| Set icon visibility.
*/
void GlgMapViewer::SetIconVisibility( GlgObjectC& icon, GlgBoolean show )
{
   if( icon.IsNull() )
   {
      GlgError( GLG_USER_ERROR, (char *) "Icon not found." );
      return;
   }

   // Show/hide the icon.
   icon.SetResource( "Visibility", show ? 1. : 0. );
}

/*----------------------------------------------------------------------
| Retrieve lat/lon information at the specified cursor position.
*/
GlgBoolean GlgMapViewer::GetLatLonInfo( GlgPoint * in_point, 
                                        GlgPoint * out_point )
{
   // Convert cursor position to lat/lon.
   return GlgGISConvert( GISObject, NULL, GLG_SCREEN_COORD,
                         /* X/Y to Lat/Lon */ GlgTrue, 
                         in_point, out_point );
}

/*----------------------------------------------------------------------
| 
*/
void GlgMapViewer::ShowInfoDisplay( GlgBoolean show, GlgPoint * lat_lon )
{
   /* InfoObject/Status resource toggles text display:
      Status=0 displays general prompt;
      Status=1 displays lat/lon values.
   */
   SetResource( "InfoObject/Status", show ? 1. : 0. );
   SetResource( "InfoObject/Visibility", 1.0 );
   
   if( !lat_lon )
   {
      // Initialize LAT/LON to an undefined value (such as -1000). 
      SetResource( "InfoObject/LAT", -1000. );
      SetResource( "InfoObject/LON", -1000. );
   }
   else
   {
      SetResource( "InfoObject/LAT", lat_lon->y );
      SetResource( "InfoObject/LON", lat_lon->x );
   }
}

/*----------------------------------------------------------------------
| Returns map viewport's ZoomTo mode.
*/
int GlgMapViewer::ZoomToMode()
{
   double zoom_mode;

   MapVp.GetResource( "ZoomToMode", &zoom_mode );

   return (int)zoom_mode;
}

/*----------------------------------------------------------------------
| Perform zoom operation.
*/
void GlgMapViewer::Zoom( char type, double value )
{
   switch( type )
   {
    default:
      MapVp.SetZoom( NULL, type, value );
      break;
      
    case 'n':
      // Reset map to the initial extent.
      GISObject.SetResource( "GISCenter", InitCenter.x, InitCenter.y, 0. );
      GISObject.SetResource( "GISExtent", InitExtent.x, InitExtent.y, 0. );
      break;
   }
}

/*----------------------------------------------------------------------
| Start dynamic updates.
*/
void GlgMapViewer::StartUpdates()
{
   /* Start update timer. */
   TimerID = GlgAddTimeOut( AppContext, UpdateInterval, 
                           (GlgTimerProc)OnTimerEvent, this );
}

/*----------------------------------------------------------------------
| Stop dynamic updates.
*/
void GlgMapViewer::StopUpdates()
{
   /* Stop update timers */
   if( TimerID )
   {
      GlgRemoveTimeOut( TimerID );
      TimerID = 0;
   }
}

/*----------------------------------------------------------------------
| Set viewer size in screen cooridnates. 
*/
void GlgMapViewer::SetSize( GlgLong x, GlgLong y, 
                            GlgLong width, GlgLong height )
{
   SetResource( "Point1", 0., 0., 0. );
   SetResource( "Point2", 0., 0., 0. );

   SetResource( "Screen/XHint", (double) x );
   SetResource( "Screen/YHint", (double) y );
   SetResource( "Screen/WidthHint", (double) width );
   SetResource( "Screen/HeightHint", (double) height );
}

// Assignment operator
GlgMapViewer& GlgMapViewer::operator= ( const GlgObjectC& object )
{
   GlgObjectC::operator=( object );
   return *this;
}
