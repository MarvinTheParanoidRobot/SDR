public class GPSData extends BaseData
{
   double lat;
   double lon;
   double altitude;
   double speed;
   double pitch;
   double roll;
   double yaw;

   // Default constructor
   GPSData() { type = BaseData.GPS; }
}
