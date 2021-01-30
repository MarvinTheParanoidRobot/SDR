#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef _WINDOWS
#include "resource1.h"
# pragma warning( disable : 4244 )
#endif
#include "GlgApi.h"
#include "util.h"

#define UPDATE_INTERVAL    30    /* Update interval in msec */
#define NUM_ITERATIONS     500
#define DROP_LINE_INTERVAL 20    /* Display a drop line for every 
                                    DROP_LINE_INTERVAL iterations. */

static GlgObject
   Drawing,    /* The top viewport of the drawing. */
   Map,        /* The map viewport. */
   GISObject,
   GISArray,
   CraftIcon[2],
   TrajectoryEdgeTemplate,
   TrajectoryFillTemplate,
   DropLineTemplate,
   Craft = (GlgObject)0,
   TrajectoryEdge = (GlgObject)0,
   TrajectoryFill = (GlgObject)0,
   DropLineArray = (GlgObject)0,
    /* Provides visual feedback when the user defines the trajectory's start
       and end points with the mouse. */  
   NewTrajectoryPolygon = (GlgObject)0;

static GlgPoint GISCenter;            /* Center of the GIS projection. */

/* The last craft position, is used to calculate heading angles. */
static GlgPoint PrevPosition;

static GlgLong UpdateCount = 0;
static GlgLong ZoomLevel = 2;
static GlgBoolean Pause = False;
static GlgBoolean NewTrajectoryMode = False;

/**** Simulation parameters ****/

static double TrajectoryHeight = 400000.; /* Maximum height of the trajectory,
                                             may be changed with a slider. */
static double Curvature = -1.75;           /* Trajectory curvature value, 
                                              may be changed with a slider. */
static double CurvatureY;

/* Initial values for the start and end of the trajectory; may be changed
      interactively using the mouse. */
static GlgPoint
   StartPoint = { -80.644861, 28.572872, 0. },
   EndPoint = { -80.644861 + 5., 28.572872 - 3., 0. },
   /* Are used to define the trajectory's start and end points with the mouse. */
   Point1,
   Point2;

static GlgPoint NormalVector;   /* Used for simulation: normal to the 
                                   trajectory's general direction. */
double TrajectoryLength;        /* Used for simulation; in abstract units. */

static GlgAppContext AppContext;   /* Global, is used to install a timeout. */

/* Function prototypes. */
static void Input( GlgObject viewport, GlgAnyType client_data, 
                   GlgAnyType call_data );
static void Trace( GlgObject viewport, GlgAnyType client_data, 
                   GlgAnyType call_data );
static void Restart( void );
static void Pan( char direction );
static void SetZoomLevel( void );
static void AbortNewTrajectoryMode( void );
static void UpdatePosition( GlgAnyType data, GlgIntervalID * id );
void CreateCraftIcon( void );
static void GetCraftPosition( double * lon, double * lat, double * elev );
static void SetCraftAngles( double lon, double lat, double elev,
                            GlgPoint * PrevPosition );
static void GetGlobeAngles( GlgPoint * curr_position, GlgPoint * prev_position,
                            double * yaw, double * pitch, double * roll );
static void InitSimulationParameters( void );
static void GetNormalVector( GlgPoint *vector, GlgPoint *normal );
static void GetPointXYZ( GlgPoint * lat_lon, GlgPoint * xyz );

/*----------------------------------------------------------------------
|
*/
int TrajectoryMain( int argc, char * argv[], GlgAppContext app_context )
{   
   GlgLong i;
   char * full_path;

   AppContext = GlgInit( False, app_context, argc, argv );

   GlgSetDResource( (GlgObject)0, "$config/GlgMouseTooltipTimeout", 0.05 );

   full_path = GlgGetRelativePath( argv[0], "trajectory.g" );
   Drawing = GlgLoadWidgetFromFile( full_path );
   if( !Drawing )
   {
      GlgError( GLG_USER_ERROR, "Can't load drawing file." );
      exit( GLG_EXIT_ERROR );
   }

   GlgFree( full_path );

   /* Set widget dimensions in screen pixels. If not set, default 
      dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( Drawing, "Point1", 0., 0., 0. );
   GlgSetGResource( Drawing, "Point2", 0., 0., 0. );
   GlgSetDResource( Drawing, "Screen/WidthHint", 600. );
   GlgSetDResource( Drawing, "Screen/HeightHint", 600. );

   /* Set the window title. */
   GlgSetSResource( Drawing, "ScreenName", "GLG Trajecory Demo" );

   /* Get ID of the map viewport. */
   Map = GlgGetResourceObject( Drawing, "Map" );

   /* Get ID of the GIS object. */
   GISObject = GlgGetResourceObject( Map, "GISObject" );

   /* Get ID of the GIS Object's GISArray, which holds objects displayed
      on top of the map in lat/lon coordinates.
      */
   GISArray = GlgGetResourceObject( GISObject, "GISArray" );

   /* Get trajectory templates and remove them from the drawing initially.
      Each template is a polygon with 2 points that stores graphical
      attributes of the trajectory, to allow the user to define 
      the attributes interactively via the Graphics Builder.
      One polygon is used for the edge of the trajectory, and another
      polygon is used for semi-transparent fill.
      Alternatively, template polygons can be created programmatically.
   */
   TrajectoryEdgeTemplate = 
     GlgGetResourceObject( Map, "TrajectoryEdgeTemplate" );
   GlgReferenceObject( TrajectoryEdgeTemplate );
   GlgDeleteThisObject( Map, TrajectoryEdgeTemplate );

   TrajectoryFillTemplate = 
     GlgGetResourceObject( Map, "TrajectoryFillTemplate" );
   GlgReferenceObject( TrajectoryFillTemplate );
   GlgDeleteThisObject( Map, TrajectoryFillTemplate );

   /* Vertical drop lines. */
   DropLineTemplate = 
     GlgGetResourceObject( Map, "DropLineTemplate" );
   GlgReferenceObject( DropLineTemplate );
   GlgDeleteThisObject( Map, DropLineTemplate );

   /* Delete craft icons from the top drawing, where it was placed for 
      the ease of editing. The selected icon will be added inside the 
      GISArray to position it in lat/lon. 
   */
   for( i=0; i<2; ++i )
   {
      CraftIcon[i] = GlgGetResourceObject( Map, i ? "Craft1" : "Craft0" );
      GlgReferenceObject( CraftIcon[ i ] );
      GlgDeleteThisObject( Map, CraftIcon[ i ] );
   }

   /* Query the center of the GIS projection. */
   GlgGetGResource( GISObject, "GISCenter", 
                    &GISCenter.x, &GISCenter.y, &GISCenter.z );

   /* Set initial trajectory height and curvature to predefined values. */
   GlgSetDResource( Drawing, "Toolbar/HeightSlider/ValueX", TrajectoryHeight );
   GlgSetDResource( Drawing, "Toolbar/CurvatureSlider/ValueX", Curvature );

   GlgAddCallback( Drawing, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );
   GlgAddCallback( Drawing, GLG_TRACE_CB, (GlgCallbackProc)Trace, NULL );

   /* Set GIS Zoom mode. It was set and saved with the drawing, but do it 
      again programmatically just in case.
      */
   GlgSetZoomMode( Map, NULL, GISObject, NULL, GLG_GIS_ZOOM_MODE );

   SetZoomLevel();

   GlgInitialDraw( Drawing );

#ifdef _WINDOWS            
   GlgLoadExeIcon( Drawing, IDI_ICON2 );
#endif

   /* Installs a timeout to update moving object. */
   GlgAddTimeOut( AppContext, UPDATE_INTERVAL, (GlgTimerProc)UpdatePosition,
                  NULL );

   return (int) GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| Handle user interaction.
*/
static void Input( GlgObject viewport, GlgAnyType client_data, 
                   GlgAnyType call_data )
{
   GlgObject message_obj;
   double dvalue;
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

   if( strcmp( format, "Button" ) == 0 )         /* Handle button clicks. */
   {
      if( strcmp( action, "Activate" ) != 0 && 
          strcmp( action, "ValueChanged" ) != 0 )
	return;

      AbortNewTrajectoryMode();

      if( strcmp( origin, "Restart" ) == 0 )
      {
         Restart();
      }
      else if( strcmp( origin, "PauseResume" ) == 0 )
      {
         GlgGetDResource( message_obj, "OnState", &dvalue );
         Pause = ( dvalue == 0. );
      }
      else if( strcmp( origin, "IconType" ) == 0 )
      {
         CreateCraftIcon();   /* Change icon. */

         /* Set craft position and angles, same as in UpdatePosition(). */
         if( Pause && UpdateCount ) 
         {
            double lat, lon, elev;

            GetCraftPosition( &lon, &lat, &elev ); 
            GlgSetGResource( Craft, "Position", lon, lat, elev ); 
            SetCraftAngles( lon, lat, elev, &PrevPosition ); 
            GlgSetDResource( Craft, "Visibility", 1. ); 
            GlgUpdate( Drawing );
         }
      }
      else if( strcmp( origin, "NewTrajectory" ) == 0 )
      {
         NewTrajectoryMode = True;
         GlgSetDResource( Map, "Prompt/Visibility", 1. );
         GlgUpdate( Map );
      }
      else if( strcmp( origin, "Up" ) == 0 )
        Pan( 'u' );
      else if( strcmp( origin, "Down" ) == 0 )
        Pan( 'd' );
      else if( strcmp( origin, "Left" ) == 0 )
        Pan( 'l' );
      else if( strcmp( origin, "Right" ) == 0 )
        Pan( 'r' );
      else if( strcmp( origin, "ZoomIn" ) == 0 )
      {
         if( ZoomLevel < 4 )
         {
            ++ZoomLevel;
            SetZoomLevel();
         }
      }
      else if( strcmp( origin, "ZoomOut" ) == 0 )
      {
         if( ZoomLevel > 0 )
         {
            --ZoomLevel;
            SetZoomLevel();
         }
      }
   }
   else if( strcmp( format, "Slider" ) == 0 )
   {
      if( strcmp( action, "ValueChanged" ) == 0 )
        AbortNewTrajectoryMode();
   }
}

/*------------------------------------------------------------------------
| Restart simulation with new parameters.
*/
static void Restart()
{
   UpdateCount = 0;   /* Reset update counter */

   /* Resume if was paused. */
   Pause = False;
   GlgSetDResource( Drawing, "Toolbar/PauseResume/OnState", 1. );
   GlgUpdate( Drawing );
}

/*------------------------------------------------------------------------
| Pan the map in the specified direction.
*/
static void Pan( char direction )
{
   double distance;

   /* Pan in lat/lon instead of screen pixels to allow contineous rotation
      of the globe upside down.
   */
   switch( direction )
   {
    default:
    case 'u': direction = 'Y'; distance =  8.; break;
    case 'd': direction = 'Y'; distance = -8.; break;
    case 'l': direction = 'X'; distance = -8.; break;
    case 'r': direction = 'X'; distance =  8.; break;
   }

   GlgSetZoom( Map, NULL, direction, distance );
   GlgUpdate( Map );

   /* Query the new center of the GIS projection. */
   GlgGetGResource( GISObject, "GISCenter", 
                    &GISCenter.x, &GISCenter.y, &GISCenter.z );   
}

/*------------------------------------------------------------------------
| 
*/
static void SetZoomLevel()
{
   double scale;

   switch( ZoomLevel )
   {
    case 0: scale = 3.; break;
    case 1: scale = 5.; break;
    default:
    case 2: scale = 8.25; break;
    case 3: scale = 10.; break;
    case 4: scale = 12.; break;
   }

   /* Instead of the GIS zoom, GIS object is scaled (via the attached scale 
      transformation) to have the GIS object partially clipped out with 
      the top of it visible as a horizon line.      
      Scaling the GIS object does not change projection center.
   */
   GlgSetDResource( GISObject, "GISScale", scale );

   /* Set scaling limits depending on the resolution of available data. */
   GlgSetDResource( Map, "ZoomIn/DisableInput", ZoomLevel >= 4 ? 1. : 0. );
   GlgSetDResource( Map, "ZoomOut/DisableInput", ZoomLevel <= 0 ? 1. : 0. );
}

/*----------------------------------------------------------------------
| Updates moving object with new position data.
*/
static void UpdatePosition( GlgAnyType data, GlgIntervalID * id )
{   
   GlgObject
     trajectory,
     drop_line;
   GlgULong sec1, microsec1;
   GlgLong timer_interval;
   GlgLong i;
   double
     lon, lat, elev;

   GlgGetTime( &sec1, &microsec1 );  /* Start time */

   if( !Pause )
   {
      if( UpdateCount == NUM_ITERATIONS )
        UpdateCount = 0; /* Reached the end of the trajectory: start over. */
      
      /* Obtain new craft position. */
      GetCraftPosition( &lon, &lat, &elev );

      if( !UpdateCount )     /* First time. */
      {
         InitSimulationParameters();
         CreateCraftIcon();
      }
      else
      {
         /* Update craft position in the drawing with new data.
            Also update angles: previous position is valid.
         */
         GlgSetGResource( Craft, "Position", lon, lat, elev );      
         SetCraftAngles( lon, lat, elev, &PrevPosition );

         GlgSetDResource( Craft, "Visibility", 1. );  /* Make it visible. */
      }

      /* Store previous position to calculate heading angles. */
      PrevPosition.x = lon;
      PrevPosition.y = lat;
      PrevPosition.z = elev;      

      /* Update craft trajectory. */
      if( !UpdateCount )
      {
         /* First time: add trajectory polygons to the GIS Object. */
         if( TrajectoryEdge )
         {
            /* Destroy previous trajectory polygons. */
            GlgDeleteThisObject( GISArray, TrajectoryEdge );
            TrajectoryEdge = (GlgObject)0;
            
            GlgDeleteThisObject( GISArray, TrajectoryFill );
            TrajectoryFill = (GlgObject)0;
            
            if( DropLineArray )
            {
               GlgDeleteThisObject( GISArray, DropLineArray );
               DropLineArray = (GlgObject)0;
            }
         }
         
         for( i=0; i<2; ++i )
         {
            if( !i )
            {
               TrajectoryEdge = GlgCopyObject( TrajectoryEdgeTemplate );
               trajectory = TrajectoryEdge;
            }
            else
            {
               TrajectoryFill = GlgCopyObject( TrajectoryFillTemplate );
               trajectory = TrajectoryFill;
            }

            /* Use two points initially, set both points to the same lat/lon. */
            SetPolygonPoint( trajectory, 0, lon, lat, elev );
            SetPolygonPoint( trajectory, 1, lon, lat, elev );
            
            /* Add to top to draw the trajectory first, with the craft 
               on top of it. */
            GlgAddObjectToTop( GISArray, trajectory );
            GlgDropObject( trajectory );    /* Let GIS object manage it. */
         }
      }
      else
      {
         for( i=0; i<2; ++i )
         {
            trajectory = ( !i ? TrajectoryEdge : TrajectoryFill );
            
            /* Add two points for every postion: one using the trajectory 
               elevation, and one at the ground level to show the ground 
               position.
            */
            AddPolygonPoint( trajectory, lon, lat, elev, True );
            AddPolygonPoint( trajectory, lon, lat, 0., False );
         }
         
         /* Display a drop line for every DROP_LINE_INTERVAL iterations. */
         if( !( UpdateCount % DROP_LINE_INTERVAL ) )
         {
            if( !DropLineArray )
            {
               GlgLong position;
               
               /* First drop line: create a group to hold drop lines. */
               DropLineArray = 
                 GlgCreateObject( GLG_GROUP, NULL, NULL, NULL, NULL, NULL );
               
               /* Add after trajectory polygons but before the craft. */
               position = GlgGetIndex( GISArray, Craft );
               GlgAddObjectAt( GISArray, DropLineArray, position );
            }
            
            drop_line = GlgCopyObject( DropLineTemplate );
            
            /* Draw a line from craft to the ground point (elev=0) with the
               same lat/lon. */
            SetPolygonPoint( drop_line, 0, lon, lat, elev );
            SetPolygonPoint( drop_line, 1, lon, lat, 0. );
            
            GlgAddObjectToTop( DropLineArray, drop_line );
            GlgDropObject( drop_line );
         }
      }
      
      ++UpdateCount;
      GlgUpdate( Drawing );
      GlgSync( Drawing );    /* Improves interactive response */
   }

   timer_interval = GetAdjustedTimeout( sec1, microsec1, UPDATE_INTERVAL );

   GlgAddTimeOut( AppContext, timer_interval, (GlgTimerProc)UpdatePosition, 
		 NULL );
}


/*------------------------------------------------------------------------
| Creates a craft icon based on the icon type requested by the IconType 
| toolbar button.
*/
void CreateCraftIcon()
{
   GlgLong icon_type;
   double dtype;   

   /* Delete previously used craft icon. */
   if( Craft )
   {
      GlgDeleteThisObject( GISArray, Craft );
      GlgDropObject( Craft );
      Craft = (GlgObject)0;
   }
   
   /* Query icon type requested by the IconType toolbar button. */
   GlgGetDResource( Drawing, "Toolbar/IconType/OnState", &dtype );
   icon_type = ( dtype ? 1 : 0 );

   /* Add a craft icon. */
   Craft = GlgReferenceObject( CraftIcon[ icon_type ] );

   /* Make invisible initially: has no position or angle information. */
   GlgSetDResource( Craft, "Visibility", 0. );

   /* Add to the drawing inside the GIS object to position in lat/lon. */
   GlgAddObjectToBottom( GISArray, CraftIcon[ icon_type ] );
}
         
typedef enum
{
   BUTTON_PRESS = 0,
   BUTTON_RELEASE,
   MOUSE_MOVE
} EventType;

/*------------------------------------------------------------------------
| Is used to obtain coordinates of the mouse click when defining 
| the trajectory's start and end points with the mouse.
*/
static void Trace( GlgObject viewport, GlgAnyType client_data, 
                   GlgAnyType call_data )
{      
   GlgTraceCBStruct * trace_data;
   GlgPoint point, lat_lon;
   double x, y;
   GlgLong event_type = 0;

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
	 AbortNewTrajectoryMode();	 
	 return;  /* Use the left button clicks only. */
      }

      event_type = BUTTON_PRESS;
      x = trace_data->event->xbutton.x;
      y = trace_data->event->xbutton.y;
      break;
      
    case ButtonRelease:
      if( trace_data->event->xbutton.button != 1 )
        return;

      event_type = BUTTON_RELEASE;
      break;
      
    case MotionNotify:
      event_type = MOUSE_MOVE;
      x = trace_data->event->xmotion.x;
      y = trace_data->event->xmotion.y;
      break;

    default: return;
   }
#else
   switch( trace_data->event->message )
   {
    case WM_LBUTTONDOWN:
      event_type = BUTTON_PRESS;

      x = LOWORD( trace_data->event->lParam );
      y = HIWORD( trace_data->event->lParam );
      break;
      
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
      AbortNewTrajectoryMode();
      return;

    case WM_MOUSEMOVE:
      event_type = MOUSE_MOVE;

      x = LOWORD( trace_data->event->lParam );
      y = HIWORD( trace_data->event->lParam );
      break;

    case WM_LBUTTONUP:
      event_type = BUTTON_RELEASE;
      break;
      
    default: return;
   }
#endif
   
   /* Use the Map area events only. */
   if( trace_data->viewport != Map )
     return;

   /* COORD_MAPPING_ADJ is added to the cursor coordinates for precise 
      pixel mapping.
   */
   x += GLG_COORD_MAPPING_ADJ;
   y += GLG_COORD_MAPPING_ADJ;

   switch( event_type )
   {
    case BUTTON_PRESS:
      if( !NewTrajectoryMode )
        return;

      /* Delete the previous trajectory polygon, if any. */
      if( NewTrajectoryPolygon )
      {
         AbortNewTrajectoryMode();
         NewTrajectoryMode = True;    /* Restore mode */
      }

      /* Define the start point of the trajectory. */
      point.x = x;
      point.y = y;
      point.z = 0.;

      /* Convert screen coordinates of the mouse to the world coordinates 
         inside the GIS Object's GISArray - lat/lon.
      */ 
      GlgScreenToWorld( GISArray, True, &point, &lat_lon );

      GlgSetDResource( Map, "Prompt/Visibility", 0. );  /* Remove prompt */
      
      NewTrajectoryPolygon = GlgCreateObject( GLG_POLYGON, NULL,
                                              NULL, NULL, NULL, NULL );
      GlgSetGResource( NewTrajectoryPolygon, "EdgeColor", 1., 1., 0. );
      
      /* Initially, set both polygon points to the the mouse position. */
      Point1 = lat_lon;
      Point2 = lat_lon;
      SetPolygonPoint( NewTrajectoryPolygon, 0, Point1.x, Point1.y, 0. );
      SetPolygonPoint( NewTrajectoryPolygon, 1, Point1.x, Point1.y, 0. );
      
      GlgAddObjectToBottom( GISObject, NewTrajectoryPolygon );
      GlgUpdate( Map );
      break;

    case MOUSE_MOVE:
      if( !NewTrajectoryMode || !NewTrajectoryPolygon )
        return;
      
      /* Start point has been defined: show the end point when dragging. */
      point.x = x;
      point.y = y;
      point.z = 0.;
      
      /* Convert screen coordinates of the mouse to the world coordinates 
         inside the GIS Object's GISArray - lat/lon. */ 
      GlgScreenToWorld( GISArray, True, &point, &lat_lon );
      
      /* Store the end point of the trajectory. */
      Point2 = lat_lon;

      SetPolygonPoint( NewTrajectoryPolygon, 1, lat_lon.x, lat_lon.y, 0. );
      GlgUpdate( Map );
      break;

    case BUTTON_RELEASE:
      if( !NewTrajectoryMode || !NewTrajectoryPolygon )
        return;

      AbortNewTrajectoryMode();   /* Delete NewTrajectoryPolygon. */

      if( Point1.x == Point2.x && Point1.y == Point2.y )
        return;    /* Mouse didn't move: no trajectory was defined. */

      /* Trajectory was defined: store the new start and end point. */
      StartPoint = Point1;
      EndPoint = Point2;
      
      /* Set a smaller curvature to follow the start/end line. */
      Curvature = -0.2;
      GlgSetDResource( Drawing, "Toolbar/CurvatureSlider/ValueX", Curvature ); 
         
      Restart();             /* Restart with the new trajectory. */
      GlgUpdate( Drawing );
      break;
      
    default: return;
   } 
}

/*----------------------------------------------------------------------
| Deletes NewTrajectoryPolygon if created.
*/
static void AbortNewTrajectoryMode()
{
   /* Remove prompt if it's visible. */
   GlgSetDResourceIf( Map, "Prompt/Visibility", 0., True );

   if( NewTrajectoryPolygon )
   {
      if( GlgContainsObject( GISObject, NewTrajectoryPolygon ) )
        GlgDeleteThisObject( GISObject, NewTrajectoryPolygon );

      GlgDropObject( NewTrajectoryPolygon );	 
      NewTrajectoryPolygon = (GlgObject)0;
      GlgUpdate( Map );
   }
   NewTrajectoryMode = False;
}

/*----------------------------------------------------------------------
| Simulation: returns craft postion (lon, lat and elevation above the
| Earth in meters). 
*/
static void GetCraftPosition( double * lon, double * lat, double * elev )
{
   double rel_value, rel_sin;

   rel_value = ( UpdateCount % NUM_ITERATIONS ) / (double) NUM_ITERATIONS;
   rel_sin = sin( M_PI * rel_value );

   *lon = StartPoint.x + ( EndPoint.x - StartPoint.x ) * rel_value +
     NormalVector.x * TrajectoryLength * Curvature * rel_sin;

   *lat = StartPoint.y + ( EndPoint.y - StartPoint.y ) * rel_value +
     NormalVector.y * TrajectoryLength * CurvatureY * rel_sin;

   *elev = TrajectoryHeight * rel_sin;
}

/*----------------------------------------------------------------------
| Simulation: Set craft angles depending on its position on the globe
| and asimuthal angle. Angles are not needed if a simpler marker icon
| is used to mark position of the craft.
*/
static void SetCraftAngles( double lon, double lat, double elev, 
                            GlgPoint * prev_position )
{
   double roll, pitch, yaw;
   GlgPoint curr_position;
  
   curr_position.x = lon;
   curr_position.y = lat;
   curr_position.z = elev;

   GetGlobeAngles( &curr_position, prev_position, &yaw, &pitch, &roll );

#if 0
   printf( "Center x=%.0f y=%f  lon=%.0f lat=%f  roll=%.0f pitch=%f yaw=%.0f\n",
           GISCenter.x, GISCenter.y, lon, lat, roll, pitch, yaw );
#endif

   GlgSetDResource( Craft, "Roll", roll );
   GlgSetDResource( Craft, "Pitch", pitch );
   GlgSetDResource( Craft, "Yaw", yaw );
}

/*----------------------------------------------------------------------
| Simulation: calculates craft's heading angles based on the current
| and previous position.
*/
static void GetGlobeAngles( GlgPoint * curr_position, GlgPoint * prev_position,
                            double * yaw, double * pitch, double * roll )
{
   GlgPoint curr_xyz, prev_xyz;
   double 
     dx, dy, dz,
     rel_value;

   /* Calculate angles using XYZ position on the globe. If Z coordinate
      is not required, GlgGISConvert() may be used to get X and Y screen 
      coordinates in pixels instead of GetPointXYZ().
   */

   /* XYZ of the first point, in meters */
   GetPointXYZ( curr_position, &curr_xyz );

   /* XYZ of the second point, in meters */
   GetPointXYZ( prev_position, &prev_xyz );

   dx = curr_xyz.x - prev_xyz.x;
   dy = curr_xyz.y - prev_xyz.y;
   dz = curr_xyz.z - prev_xyz.z;

   /* Visible heading angles on the globe in the current projection. */
   if( !dx )
     *yaw = ( dy ? ( dy > 0. ? 90. : -90. ) : 0. );
   else
   {
      *yaw = atan( dy / dx );
      *yaw = RadToDeg( *yaw );
      if( dx < 0. )
        *yaw += 180.;
   }

   if( !dx && !dy )
     *pitch = - ( dz ? ( dz > 0. ? 90. : -90. ): 0. );
   else
   {
      *pitch = - atan( dz / sqrt( dx * dx + dy * dy ) / 3. );
      *pitch = RadToDeg( *pitch );
   }

   rel_value = ( UpdateCount % NUM_ITERATIONS ) / (double) NUM_ITERATIONS;

   /* Roll it a bit. In an application, supply a real roll angle. */
   *roll = -60. * sin( DegToRad( *yaw ) ) * sin( M_PI * rel_value );
}

/*----------------------------------------------------------------------
| Simulation: Initializes simulation parameters.
*/
static void InitSimulationParameters()
{
   GlgPoint trajectory_vector;
   double max_lat;

   /* Query requested trajectory height from the slider. */
   GlgGetDResource( Drawing, "Toolbar/HeightSlider/ValueX",
                    &TrajectoryHeight );      

   /* Query requested trajectory curvature from the slider. */
   GlgGetDResource( Drawing, "Toolbar/CurvatureSlider/ValueX",
                    &Curvature );      

   trajectory_vector.x = EndPoint.x - StartPoint.x;
   trajectory_vector.y = EndPoint.y - StartPoint.y;

   /* Obtain a normal vector used to curve the simulated trajectory. */
   GetNormalVector( &trajectory_vector, &NormalVector );

   /* Calculate trajectory length in abstract units for simulation. */
   TrajectoryLength = sqrt( trajectory_vector.x * trajectory_vector.x +
                            trajectory_vector.y * trajectory_vector.y );

   /* Point with the maximum lat. */
   max_lat = StartPoint.y + ( EndPoint.y - StartPoint.y ) * 
     NormalVector.y * TrajectoryLength * Curvature;
   if( max_lat > 89. )
     max_lat = 89.;
   else if( max_lat < -89. )
     max_lat = -89.;
   else 
     max_lat = 0.;

   /* Limit Y curvature to avoid flying "off the globe". */
   if( max_lat )
     CurvatureY = ( max_lat - StartPoint.y ) / 
       ( NormalVector.y * TrajectoryLength * ( EndPoint.y - StartPoint.y ) );
   else
     CurvatureY = Curvature;
}

/*----------------------------------------------------------------------
| Simulation: Returns 2D normal vector of length=1.
*/
static void GetNormalVector( GlgPoint *vector, GlgPoint *normal )
{
   double length;

   if( !vector->x )
   {
      if( !vector->y )        
        normal->x = 0.;    /* NULL vector: return NULL as a normal. */
      else
        normal->x = 1.;

      normal->y = 0.;
   }
   else if( !vector->y )
   {
      normal->x = 0.;
      normal->y = 1.;
   }
   else
   {
      normal->x = vector->y;
      normal->y = - vector->x;
      length = sqrt( normal->x * normal->x + normal->y * normal->y );
      if( length != 1. )
      {
         normal->x /= length;
         normal->y /= length;
      }
   }
}

/*----------------------------------------------------------------------
| Returns XYZ coordinate of the lat/lon point, in meters.
*/
static void GetPointXYZ( GlgPoint * lat_lon, GlgPoint * xyz )
{
   double
     globe_radius,
     phi, phi1, lon_diff,
     cos_phi, sin_phi,
     cos_phi1, sin_phi1,
     cos_lon_diff, sin_lon_diff;

   phi = DegToRad( lat_lon->y );
   phi1 = DegToRad( GISCenter.y );
   lon_diff = DegToRad( lat_lon->x - GISCenter.x );

   /* Use the average value plus elevation. */
   globe_radius = ( GLG_EQUATOR_RADIUS + GLG_POLAR_RADIUS ) / 2.;
   globe_radius += lat_lon->z;

   cos_phi = cos( phi );
   sin_phi = sin( phi );
   cos_phi1 = cos( phi1 );
   sin_phi1 = sin( phi1 );
   cos_lon_diff = cos( lon_diff );
   sin_lon_diff = sin( lon_diff );

   xyz->x = globe_radius * cos_phi * sin_lon_diff;
   xyz->y = globe_radius * 
     ( cos_phi1 * sin_phi - sin_phi1 * cos_phi * cos_lon_diff );
   xyz->z = globe_radius *
     ( cos_lon_diff * cos_phi1 * cos_phi + sin_phi1 * sin_phi );
}
