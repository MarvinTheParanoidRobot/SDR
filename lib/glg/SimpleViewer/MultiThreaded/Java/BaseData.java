public class BaseData
{
   int type;
   double time_stamp;
   boolean is_valid;  // true if received data are valid
   
   public static final int GPS = 0;
   public static final int TELEMETRY = 1;
}
