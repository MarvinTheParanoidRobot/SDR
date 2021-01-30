public class TelemetryData extends BaseData
{
   double power;
   double voltage;
   double current;
   double temperature;
   double pressure;
   double state_health;

   // Default constructor
   TelemetryData() { type = BaseData.TELEMETRY; }
}
