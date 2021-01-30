/* Satellite Demo: demonstrates the use of the GIS Object to position 
 * satellites by specifying lat/lon coordinates and elevation above the Earth.
 * The GLG Map Server is used to display a globe in the orthographic 
 * projection.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef _WINDOWS
#include "resource1.h"
# pragma warning( disable : 4244 )
#endif
#include "GlgApi.h"
#include "util.h"

static GlgObject
   Drawing,     
   GISObject,
   GISArray,
   Satellite1,
   Satellite2,
   Satellite1_GroundTrack,
   Satellite2_GroundTrack,
   Orbit1,
   Orbit2;
static GlgLong
   NumOrbitPoints = 0,
   UpdateCount = 0;

#define UPDATE_INTERVAL  30  /* Update interval in msec */

/* Simulation parameters. */
#define SAT1_PERIOD      250
#define SAT2_PERIOD      200

static GlgAppContext AppContext;   /* Global, is used to install a timeout. */

/* Function prototypes. */
static void Input( GlgObject viewport, GlgAnyType client_data, 
                   GlgAnyType call_data );
static void Trace( GlgObject viewport, GlgAnyType client_data, 
                   GlgAnyType call_data );
static void UpdatePosition( void );
static void UpdatePositionCB( GlgAnyType data, GlgIntervalID * id );
static void GetSatellitePosition( GlgLong satellite_index, 
                                  double * lon, double * lat, double * elev );

/*----------------------------------------------------------------------
|
*/
int SatelliteMain( int argc, char * argv[], GlgAppContext app_context )
{   
   char * full_path;

   AppContext = GlgInit( False, app_context, argc, argv );

   GlgSetDResource( (GlgObject)0, "$config/GlgMouseTooltipTimeout", 0.05 );

   full_path = GlgGetRelativePath( argv[0], "satellite.g" );
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
   GlgSetSResource( Drawing, "ScreenName", "GLG Satellite Demo" );

   /* Get ID of the GIS object. */
   GISObject = GlgGetResourceObject( Drawing, "GISObject" );

   /* Get ID of the GIS Object's GISArray, which holds objects displayed
      on top of the map in lat/lon coordinates.
      */
   GISArray = GlgGetResourceObject( GISObject, "GISArray" );

   /* Get IDs of satellite icons. */
   Satellite1 = GlgGetResourceObject( GISArray, "Satellite1" );
   Satellite2 = GlgGetResourceObject( GISArray, "Satellite2" );
   Satellite1_GroundTrack =
     GlgGetResourceObject( GISArray, "Satellite1_GroundTrack" );
   Satellite2_GroundTrack =
     GlgGetResourceObject( GISArray, "Satellite2_GroundTrack" );

   /* Get orbit templates and remove them from the drawing initially.
      The templates are polygons with 2 points that store graphical
      attributes of the orbit polygons, to allow user to define the
      attributes interactively via the Graphics Builder. 
      Alternatively, the orbit polygons can be created programmatically.
   */
   Orbit1 = GlgGetResourceObject( Drawing, "Orbit1" );
   Orbit2 = GlgGetResourceObject( Drawing, "Orbit2" );
   GlgReferenceObject( Orbit1 );
   GlgReferenceObject( Orbit2 );
   GlgDeleteThisObject( Drawing, Orbit1 );
   GlgDeleteThisObject( Drawing, Orbit2 );   

   GlgAddCallback( Drawing, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );
   GlgAddCallback( Drawing, GLG_TRACE_CB, (GlgCallbackProc)Trace, NULL );

   /* Set the GIS Zoom mode. It was set and saved with the drawing, but do it 
      again programmatically just in case.
      */
   GlgSetZoomMode( Drawing, NULL, GISObject, NULL, GLG_GIS_ZOOM_MODE );
   
   UpdatePosition();   /* Set initial satellite position, */

   GlgInitialDraw( Drawing );

#ifdef _WINDOWS            
   GlgLoadExeIcon( Drawing, IDI_ICON1 );
#endif

   GlgAddTimeOut( AppContext, UPDATE_INTERVAL, (GlgTimerProc)UpdatePositionCB, 
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
   double lon, lat, z;
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
      if( strcmp( action, "Activate" ) != 0 )
	return;

      GlgGetGResource( GISObject, "GISCenter", &lon, &lat, &z );

#define ANGLE_DELTA    10.   /* Defines pan distance. */

      /* Rotate the globe when a directional button is pressed. */
      if( strcmp( origin, "Up" ) == 0 )      
        GlgSetGResource( GISObject, "GISCenter", lon, lat + ANGLE_DELTA, 0. );
      else if( strcmp( origin, "Down" ) == 0 )      
        GlgSetGResource( GISObject, "GISCenter", lon, lat - ANGLE_DELTA, 0. );
      else if( strcmp( origin, "Left" ) == 0 )      
        GlgSetGResource( GISObject, "GISCenter", lon - ANGLE_DELTA, lat, 0. );
      else if( strcmp( origin, "Right" ) == 0 )      
        GlgSetGResource( GISObject, "GISCenter", lon + ANGLE_DELTA, lat, 0. );
      else
        return;

      GlgUpdate( Drawing ); 
   }
}

/*----------------------------------------------------------------------
| Is used to start globe rotation on a mouse click.
*/
static void Trace( GlgObject viewport, GlgAnyType client_data, 
                   GlgAnyType call_data )
{
   GlgTraceCBStruct * trace_data;

   trace_data = (GlgTraceCBStruct*) call_data;

   /* Start dragging only on a mouse click in the Drawing area itself,
      not the sliders.
   */
   if( trace_data->viewport != Drawing )
     return;

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
      break;
    default: return;
   }
#else
   switch( trace_data->event->message )
   {
    case WM_LBUTTONDOWN:
      break; 
    default: return;
   }
#endif

   GlgSetZoom( Drawing, NULL, 's', 0. );
}

/*----------------------------------------------------------------------
| Timer callback, updates the drawing.
*/
static void UpdatePositionCB( GlgAnyType data, GlgIntervalID * id )
{   
   GlgULong sec1, microsec1;
   GlgLong timer_interval;

   GlgGetTime( &sec1, &microsec1 );  /* Start time */

   UpdatePosition();

   timer_interval = GetAdjustedTimeout( sec1, microsec1, UPDATE_INTERVAL );

   GlgAddTimeOut( AppContext, timer_interval, (GlgTimerProc)UpdatePositionCB, 
		 NULL );
}

/*----------------------------------------------------------------------
| Updates moving objects with new position data.
*/
static void UpdatePosition()
{
   double 
     lon1, lat1, elev1,
     lon2, lat2, elev2;

   /* Obtain new positions for both satellites. */
   GetSatellitePosition( 0, &lon1, &lat1, &elev1 );
   GetSatellitePosition( 1, &lon2, &lat2, &elev2 );

   /* Update satellite positions in the drawing with new data. */
   GlgSetGResource( Satellite1, "Position", lon1, lat1, elev1 );
   GlgSetGResource( Satellite2, "Position", lon2, lat2, elev2 );

   GlgSetGResource( Satellite1_GroundTrack, "Position", lon1, lat1, 0. );
   GlgSetGResource( Satellite2_GroundTrack, "Position", lon2, lat2, 0. );

   /* Update satellite trajectories. */
   if( NumOrbitPoints <= SAT1_PERIOD || NumOrbitPoints <= SAT2_PERIOD )
   {
      if( !NumOrbitPoints )
      {
         /* First time: add orbit polygons to the GIS Object. 
            Use two points initially, set both points to the same lat/lon.
         */
         SetPolygonPoint( Orbit1, 0, lon1, lat1, elev1 );
         SetPolygonPoint( Orbit1, 1, lon1, lat1, elev1 );
         /* Add to top to draw orbits first, with satellites on top of them. */
         GlgAddObjectToTop( GISArray, Orbit1 );
         GlgDropObject( Orbit1 );    /* Let GIS object manage it. */

         SetPolygonPoint( Orbit2, 0, lon2, lat2, elev2 ); 
         SetPolygonPoint( Orbit2, 1, lon2, lat2, elev2 ); 
         GlgAddObjectToTop( GISArray, Orbit2 );
         GlgDropObject( Orbit2 );

         /* Reorder ground tracks to be under the orbit polygons. */
         GlgReferenceObject( Satellite1_GroundTrack );
         GlgReferenceObject( Satellite2_GroundTrack );

         GlgDeleteThisObject( GISArray, Satellite1_GroundTrack );
         GlgDeleteThisObject( GISArray, Satellite2_GroundTrack );

         GlgAddObjectToTop( GISArray, Satellite1_GroundTrack );
         GlgAddObjectToTop( GISArray, Satellite2_GroundTrack );
 
         GlgDropObject( Satellite1_GroundTrack );
         GlgDropObject( Satellite2_GroundTrack );
      }
      else if( NumOrbitPoints == 1 )
      {
         /* Set the second point. */
         SetPolygonPoint( Orbit1, 1, lon1, lat1, elev1 );
         SetPolygonPoint( Orbit2, 1, lon2, lat2, elev2 );
      }
      else
      {
         /* Add one more point. */
         if( NumOrbitPoints <= SAT1_PERIOD )
           AddPolygonPoint( Orbit1, lon1, lat1, elev1, True );

         if( NumOrbitPoints <= SAT2_PERIOD )
           AddPolygonPoint( Orbit2, lon2, lat2, elev2, True );
      }

      ++NumOrbitPoints; 
   } 
   ++UpdateCount;

   GlgUpdate( Drawing );
   GlgSync( Drawing );    /* Improves interactive response */
}

/*----------------------------------------------------------------------
| Simulation: returns satellite postion (lon, lat and elevation above the
| Earth in meters).
*/
static void GetSatellitePosition( GlgLong satellite_index, 
                                  double * lon, double * lat, double * elev )
{
   double rel_value;

   switch( satellite_index )
   {
    case 0:
      rel_value = ( UpdateCount % SAT1_PERIOD ) / (double) SAT1_PERIOD;
      *lon = -180. + 360. * rel_value;
      *lat = 20. * sin( 2. * M_PI * rel_value );
      *elev = 1000000.;
      break;

    default:
    case 1:
      rel_value =
        ( ( 100 + UpdateCount ) % SAT2_PERIOD ) / (double) SAT2_PERIOD;
      *lon = 0. + 360. * rel_value;
      *lat = 20. * sin( 2. * M_PI * rel_value );
      *elev = 1000000.;
      break;
   }
}

