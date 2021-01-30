#define EQUATOR_RADIUS     6378136.
#define POLAR_RADIUS       6356752.

typedef enum
{
   BUTTON_PRESS = 0,
   RESIZE,
   MOUSE_MOVE
} EventType;

typedef struct NodeDataStruct NodeData;
typedef struct PlaneDataStruct PlaneData;

struct NodeDataStruct
{
   char * name;
   GlgPoint lat_lon;
   GlgObject graphics[ 2 ];   /* Graphics objects used on the thumbnail and
                                 detailed map. */
};

struct PlaneDataStruct
{
   char * name;
   GlgPoint lat_lon;
   int flight_number;
   char * tooltip[ 2 ];
   GlgObject graphics[ 2 ];   /* Graphics objects used on the thumbnail and
                                 detailed map. */
   NodeData * from_node;
   NodeData * to_node;
   double path_position;
   double path_position_last;
   double speed;
   long has_angle[ 2 ];
   double angle[ 2 ];     /* Angles on both maps - might be different! */
};

#if defined _WINDOWS
#define strcasecmp   _stricmp
#endif

/* Function prototypes */
void Init( void );
void CreateAirportIcons( long map );
void CreatePlaneIcons( long map );
void AddNode( NodeData * node_data, long map, long index );
void AddPlane( PlaneData * plane_data, long map, long index );
void PositionPlane( PlaneData * plane, long map );
void GetPlanePosition( PlaneData * plane, long map, GlgPoint * xyz );
void GetNodePosition( NodeData * node, long map, GlgPoint * xyz );
double GetPlaneAngle( PlaneData * plane, long map );
void StartPlane( PlaneData * plane, long init );
void GetPlaneLatLon( PlaneData * plane );
double GetLength( GlgPoint * pt1, GlgPoint * pt2 );
double GetAngle( GlgPoint * pt1, GlgPoint * pt2 );
void StartUpdate( void );
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, 
                           GlgLong interval );
void UpdatePlanes( GlgAnyType data, GlgIntervalID * id );
void UpdatePlane( PlaneData * plane );
void UpdateLocking( void );
long GetVisibility( GlgPoint * position, double adj );
void GetLatLon( double x, double y, long map, GlgPoint * lat_lon );
void UpdateMapWithMessage( long map, char * message );
void SetSelectedArea( void );
void InitSelection( void );
void SetPlaneLabels( long on );
void SetLocking( long lock );
void SetStatus( char * message );
void UpdateSelectedPlaneStatus( void );
void CenterOnPlane( PlaneData * plane, long map );
void SelectPlane( PlaneData * plane, long selected );
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void Trace( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
long ZoomToMode( void );
void Zoom( long type, double value );
void CheckScrollLimits( long type );
void SyncGlobeWithDetailedMap( char * origin, char * message );
void SetPlaneSize( void );
void ToggleResource( GlgObject object, char * res_name );
void SetGISLayers( long map );
char * CreateLocationString( double lon, double lat, double z );
void HandleZoomLevel( void );
void GetExtentDegrees( GlgPoint * extent, long map );
void ZoomToUSStart( GlgAnyType data, GlgIntervalID * id );
void ZoomToUSEnd( GlgAnyType data, GlgIntervalID * id );
void error( char * string, GlgBoolean quit );
