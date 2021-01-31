#include "GlgApi.h"

#define EQUATOR_RADIUS     6378136.
#define POLAR_RADIUS       6356752.

typedef enum
{
   BUTTON_PRESS_EVENT = 0,
   MOUSE_WHEEL_EVENT,
   MOUSE_MOVE_EVENT,
   RESIZE_EVENT
} EventType;

typedef struct _IconDataStruct IconData;

struct _IconDataStruct
{
   GlgObject icon_obj;    /* Graphical object representing an icon */
   GlgPoint lat_lon;  /* Icon position in lat/lon coordinates */
   double angle;      /* Icon's rotation angle */
};

#if defined _WINDOWS
#define strcasecmp   _stricmp
#endif

/* Function prototypes */
void Init( void );
void InitBeforeH();
void InitAfterH();
void StopUpdates();
void StartUpdates();
void UpdateDrawing( GlgAnyType client_data, GlgLong * timer_id );
GlgBoolean GetLatLonInfo( GlgPoint * in_point, GlgPoint * out_point );
void ShowInfoDisplay( GlgBoolean show, GlgPoint * lat_lon );
void GetIconData( IconData * icon );
void PositionIcon( IconData * icon );
GlgObject GetIconObject( char * icon_name );
void SetIconVisibility( GlgObject icon, GlgBoolean show );
int ZoomToMode();
void Zoom( char type, double value );
void SetSize( GlgObject viewport, GlgLong x, GlgLong y, 
              GlgLong width, GlgLong height );

/* Callbacks. */
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void Trace( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );

/* Data acquisition. */
GlgBoolean GetDemoData( IconData * icon );
GlgBoolean GetLiveData( IconData * icon );
