#pragma once

typedef enum _DataStructType
{
   UNDEFINED_DATA_STRUCT_TYPE = -1,
   GPS = 0,
   TELEMETRY
} DataStructType;

class BaseData;

// A vector of pointers to data points. 
typedef std::vector<BaseData*> BaseDataVectorType;

class BaseData
{
 public:
   DataStructType type;
   double time_stamp;
   GlgBoolean is_valid;  // true if received data are valid

 public:
   static void ClearVector( BaseDataVectorType * data_vector );
};

class GPSData : public BaseData
{
 public:
   double lat;
   double lon;
   double altitude;
   double speed;
   double pitch;
   double roll;
   double yaw;

   // Default constructor
   inline GPSData( void ){ type = GPS; }
};

class TelemetryData : public BaseData
{
 public:
   double power;
   double voltage;
   double current;
   double temperature;
   double pressure;
   double state_health;

   // Default constructor
   inline TelemetryData( void ){ type = TELEMETRY; }
};

