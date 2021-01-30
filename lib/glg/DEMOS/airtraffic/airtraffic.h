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
   
   GlgObject graphics;
};

struct PlaneDataStruct
{
   char * name;
   GlgPoint lat_lon;
   int flight_number;
   char * tooltip;
   GlgObject graphics;
   GlgObject trajectory;
   NodeData * from_node;
   NodeData * to_node;
   long has_angle;

   /* Simulation parameters */
   double path_position;
   double path_position_last;
   double speed;
   long color_index;
   long iteration;
};

#if defined _WINDOWS
#define strcasecmp   _stricmp
#endif

/* Function prototypes */
void Init( void );
void AddNodeGraphics( NodeData * node_data, long node_type, long index );
void AddPlaneGraphics( PlaneData * plane_data, long plane_type, long index );
GlgObject CreateNodeIcon( long index );
GlgObject CreatePlaneIcon( long index );
GlgObject CreateTrajectoryIcon( void );
GlgObject CreateTrajectoryIcon( void );
void PositionNode( NodeData * node, long index );
void PositionPlane( PlaneData * plane, long index );
void GetPlanePosition( PlaneData * plane, GlgPoint * xyz );
void GetNodePosition( NodeData * node, GlgPoint * xyz );
double GetPlaneAngle( PlaneData * plane );
double GetPlaneElevation( PlaneData * plane );
void StartPlane( PlaneData * plane, long init );
void GetPlaneLatLon( PlaneData * plane );
double GetLength( GlgPoint * pt1, GlgPoint * pt2 );
double GetAngle( GlgPoint * pt1, GlgPoint * pt2 );
void StartUpdate( void );
void UpdatePlanes( GlgAnyType data, GlgIntervalID * id );
void UpdatePlane( PlaneData * plane, long index );
long IconVisible( GlgPoint * lat_lon );
void DeleteNodes( void );
void DeletePlanes( void );
void DeleteTrajectories( void );
void GetExtentDegrees( GlgPoint * extent );
void HandleZoomLevel( void );
void ChangeProjection( GlgPoint * extent );
void SetPlaneColor( PlaneData * plane );
void ToggleColor( void );
void SetFillType( long fill );
void UpdateObjectsOnMap( char * message );
void SetLabels( long on );
void SetStatus( char * message );
void UpdateStatus( void );
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void Trace( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
long ZoomToMode( void );
void Zoom( long type, double value );
void CheckScrollLimits( long type );
void SetPlaneSize( void );
void ToggleResource( GlgObject object, char * res_name );
void SetGISLayers( void );
void PrintSize( char * label );
void error( char * string, GlgBoolean quit );
void SelectPlane( long selected_plane_index );
void DisplayPlaneInfo( void );
void DisplayDistance( GlgObject polygon );
void AbortDistanceMode( void );
double GetGlobeDistance( GlgObject point1_obj, GlgObject point2_obj );
void GetPointXYZ( GlgPoint * lat_lon, GlgPoint * xyz );
GlgLong GetAdjustedTimeout ( GlgULong sec1, GlgULong microsec1, 
			    GlgLong interval );
char * CreateLocationString( double lon, double lat, double z );
void ZoomToFloridaStart( GlgAnyType data, GlgIntervalID * id );
void ZoomToFloridaEnd( GlgAnyType data, GlgIntervalID * id );
void AdjustRendering( void );
