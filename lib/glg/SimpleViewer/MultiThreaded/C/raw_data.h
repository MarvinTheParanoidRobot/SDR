typedef enum _DataStructType
{
   UNDEFINED_DATA_STRUCT_TYPE = -1,
   GPS = 0,
   TELEMETRY
} DataStructType;

typedef struct _BaseData
{
   DataStructType type;
   unsigned char is_valid;        // true if received data are valid
   double time_stamp;
} BaseData;

typedef struct _GPSData
{
   BaseData base;
   double lat;
   double lon;
   double altitude;
   double speed;
   double pitch;
   double roll;
   double yaw;
} GPSData;

typedef struct _TelemetryData
{
   BaseData base;
   double power;
   double voltage;
   double current;
   double temperature;
   double pressure;
   double state_health;
} TelemetryData;
