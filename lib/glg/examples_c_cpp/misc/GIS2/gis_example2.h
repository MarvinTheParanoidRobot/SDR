#define EQUATOR_RADIUS     6378136.
#define POLAR_RADIUS       6356752.

typedef enum
{
   BUTTON_PRESS = 0,
   RESIZE
} EventType;

typedef struct _IconDataStruct IconDataStruct;

struct _IconDataStruct
{
   GlgObject icon;    /* Graphical object representing an icon */
   GlgPoint lat_lon;  /* Icon position in lat/lon coordinates */
   double angle;      /* Icon's rotation angle */
};

typedef struct TargetDataStruct TargetData;

struct TargetDataStruct
{
   char * name;
   GlgPoint lat_lon;
   GlgObject graphics;
};

#if defined _WINDOWS
#define strcasecmp   _stricmp
#endif

/* Function prototypes */
void Init( void );
void PositionIcon( IconDataStruct * icon_data );
void CreateTargetsArray();
void AddTarget( TargetData * target_data, long index );
void Zoom( long type, double value );
void GetIconPosition( IconDataStruct * icon_data  );
void StartUpdate( void );
void UpdateIconPosition( GlgAnyType client_data, GlgLong * timer_id );
void SetLayersMenuButtons( GlgObject menu_object );
void SetLayersFromMenu( GlgObject menu_object, GlgBoolean init );
void ToggleResource( GlgObject object, char * res_name );
void DisplayDistance();
double GetGlobeDistance( GlgPoint lat_lon1, GlgPoint lat_lon2 );
void GetPointXYZ( GlgPoint * lat_lon, GlgPoint * xyz );

void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void Trace( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );

void error( char * string, long quit );
