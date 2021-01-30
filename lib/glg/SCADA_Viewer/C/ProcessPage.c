#include "ProcessPage.h"
#include "scada.h"
#include "GlgSCADAViewer.h"

/* Process Control Page.

   The data in GlgSCADAViewer structure are accessible via the global Viewer 
   variable.
*/

/* Demonstrates updating the drawing using either tags (True) or 
   resources (False).
*/
#define USE_TAGS  GlgTrue

#define PROCESS_SPEED            0.05 /* The rate of simulation's changes. */
#define HEATER_LEVEL_SPEED       0.05
#define WATER_LEVEL_SPEED        0.02
#define VALVE_CHANGE_SPEED       0.05
#define STEAM_VALVE_CHANGE_SPEED 0.05

/* Control variables for simulating process data in random data mode. */
static long
   ProcessCounter = 0,
   WaterAlarm = 0,
   HeaterAlarm = 0,
   heater_high = 0,
   heater_low = 0,
   water_high = 0,
   water_low = 0,
   steam_high = 0,
   steam_low = 0,
   cooling_high = 0,
   cooling_low = 0;
static double
   SolventValve = 0.85,
   SteamValve = 1.,
   CoolingValve = 0.8,
   WaterValve = 0.4,
   SolventFlow = 0.,
   SteamFlow = 0.,
   CoolingFlow = 0.,
   WaterFlow = 0.,
   OutFlow = 3495.,
   SteamTemperature = 0.,
   HeaterTemperature = 0.,
   BeforePreHeaterTemperature = 0.,
   PreHeaterTemperature = 0.,
   AfterPreHeaterTemperature = 0.,
   CoolingTemperature = 0.,
   HeaterPressure = 0.,
   HeaterLevel = 0.5,
   WaterLevel = 0.1;

/*----------------------------------------------------------------------*/
HMIPage * CreateProcessPage()
{
   HMIPage * proc = GlgAllocStruct( sizeof( HMIPage ) );
   proc->type = HMI_PAGE_TYPE;
   
   proc->GetUpdateInterval = procGetUpdateInterval;
   proc->NeedTagRemapping = procNeedTagRemapping;
   proc->RemapTagObject = procRemapTagObject;
   proc->InputCB = procInputCB;
   proc->UpdateData = procUpdateData;

   /* The rest of not used function pointers were initialized to NULL. */

   return proc;
}

/*----------------------------------------------------------------------
| Returns an update interval in msec for animating drawing with data.
*/
static int procGetUpdateInterval( HMIPage * hmi_page )
{
   return 50;   /* 50ms = 20 times/sec refresh */
}

/*----------------------------------------------------------------------
| A custom UpdateData method to simulate process data in random data mode.
*/
static GlgBoolean procUpdateData( HMIPage * hmi_page )
{
   if( !Viewer.RandomData )
     /* Return false to let the viewer update tags in the drawing from 
        the process database. */
     return GlgFalse;

   IterateProcess( Viewer.DrawingAreaVP );
   UpdateProcess( Viewer.DrawingAreaVP );

   return GlgTrue; /* Return true to prevent the viewer from updating tags. */
}

/*----------------------------------------------------------------------
| Recalculates new values for the process using a simulation model and
| updates display with the new values.
*/
void IterateProcess( GlgObject drawing )
{
   ++ProcessCounter;
   if( ProcessCounter == 0x7fffffff )
     ProcessCounter = 0;

   SteamTemperature += ( SteamValve - 0.6 ) * 2 * PROCESS_SPEED;
   SteamTemperature = PutInRange( SteamTemperature, 0., 1. );

   HeaterTemperature +=
     ( SteamTemperature - HeaterTemperature * HeaterLevel )
       * PROCESS_SPEED;
   HeaterTemperature = PutInRange( HeaterTemperature, 0., 1.5 );

   BeforePreHeaterTemperature +=
     ( 1.5 * HeaterTemperature - BeforePreHeaterTemperature ) *
       PROCESS_SPEED;
   BeforePreHeaterTemperature = 
     PutInRange( BeforePreHeaterTemperature, 0., 1. );

   PreHeaterTemperature +=
     ( BeforePreHeaterTemperature - PreHeaterTemperature ) *
       PROCESS_SPEED;
   PreHeaterTemperature = PutInRange( PreHeaterTemperature, 0., 1. );

   AfterPreHeaterTemperature +=
     ( 0.9 * HeaterTemperature - AfterPreHeaterTemperature ) *
       PROCESS_SPEED ;
   AfterPreHeaterTemperature = PutInRange( AfterPreHeaterTemperature, 0., 1. );

   CoolingTemperature +=
     ( AfterPreHeaterTemperature - CoolingTemperature -
       CoolingValve ) * PROCESS_SPEED;
   CoolingTemperature = PutInRange( CoolingTemperature, 0., 1. );

   OutFlow = SolventValve * 3495.;

   HeaterLevel += ( SolventValve - 0.75 ) * HEATER_LEVEL_SPEED;
   HeaterLevel = PutInRange( HeaterLevel, 0., 1. );

   /* Inversed */
   WaterLevel += ( 0.5 - WaterValve ) * WATER_LEVEL_SPEED;
   WaterLevel = PutInRange( WaterLevel, 0., 1. );

   if( HeaterLevel > 0.9 || heater_high )
   {
      heater_high = LagVar( heater_high, 10 );
      SolventValve -= VALVE_CHANGE_SPEED;
   }
   else if( HeaterLevel < 0.45 || heater_low )
   {
      heater_low = LagVar( heater_low, 10 );
      SolventValve += VALVE_CHANGE_SPEED;
   }
   SolventValve = PutInRange( SolventValve, 0., 1. );

   /* Inversed */
   if( WaterLevel > 0.2 || water_high )
   {
      water_high = LagVar( water_high, 10 );
      WaterValve += VALVE_CHANGE_SPEED;
   }
   else if( WaterLevel < 0.05 || water_low )
   {
      water_low = LagVar( water_low, 10 );
      WaterValve -= VALVE_CHANGE_SPEED;
   }
   WaterValve = PutInRange( WaterValve, 0., 1. );

   if( SteamTemperature > 0.9 || steam_high )
   {
      steam_high = LagVar( steam_high, 20 );
      SteamValve -= STEAM_VALVE_CHANGE_SPEED;
   }
   else if( SteamTemperature < 0.2 || steam_low )
   {
      steam_low = LagVar( steam_low, 20 );
      SteamValve += STEAM_VALVE_CHANGE_SPEED;
   }
   SteamValve = PutInRange( SteamValve, 0., 1. );

   if( CoolingTemperature > 0.7 || cooling_high )
   {
      cooling_high = LagVar( cooling_high, 10 );
      CoolingValve += VALVE_CHANGE_SPEED;
   }
   else if( CoolingTemperature < 0.3 || cooling_low )
   {
      cooling_low = LagVar( cooling_low, 10 );
      CoolingValve -= VALVE_CHANGE_SPEED;
   }
   CoolingValve = PutInRange( CoolingValve, 0., 1. );

   HeaterPressure = HeaterLevel * ( HeaterTemperature + 1. ) / 2.;
   HeaterPressure = PutInRange( HeaterPressure, 0., 1. );

   HeaterAlarm = ( HeaterLevel < 0.45 || HeaterLevel > 0.9 );
   WaterAlarm = ( WaterLevel > 0.2 || WaterLevel < 0.05 );
}

/*---------------------------------------------------------------------
|
*/
void UpdateProcess( GlgObject drawing )
{
   /* The drawing can be updated using either tags or resources. */
   if( USE_TAGS )
     UpdateProcessTags( drawing );
   else
     UpdateProcessResources( drawing );    
   
   GlgUpdate( drawing );
}

/*---------------------------------------------------------------------
|
*/
void UpdateProcessTags( GlgObject drawing )
{
   GlgSetDTag( drawing, "SolventValveValue", SolventValve, True );
   GlgSetDTag( drawing, "SteamValveValue", SteamValve, True );
   GlgSetDTag( drawing, "CoolingValveValue", CoolingValve, True );
   GlgSetDTag( drawing, "WaterValveValue", WaterValve, True );
   
   GlgSetDTag( drawing, "SolventFlow", GetFlow( SOLVENT_FLOW ), True );
   GlgSetDTag( drawing, "SteamFlow", GetFlow( STEAM_FLOW ), True );
   GlgSetDTag( drawing, "CoolingFlow", GetFlow( COOLING_FLOW ), True );
   GlgSetDTag( drawing, "WaterFlow", GetFlow( WATER_FLOW ), True );
   
   GlgSetDTag( drawing, "OutFlow", OutFlow, True );
   
   GlgSetDTag( drawing, "SteamTemperature", SteamTemperature, True );
   GlgSetDTag( drawing, "HeaterTemperature", HeaterTemperature, True );
   GlgSetDTag( drawing, "BeforePreHeaterTemperature", BeforePreHeaterTemperature,
               True );
   GlgSetDTag( drawing, "PreHeaterTemperature", PreHeaterTemperature, True );
   GlgSetDTag( drawing, "AfterPreHeaterTemperature", AfterPreHeaterTemperature,
               True );
   GlgSetDTag( drawing, "CoolingTemperature", CoolingTemperature, True );
   
   GlgSetDTag( drawing, "HeaterLevel", HeaterLevel, True );
   GlgSetDTag( drawing, "WaterLevel", WaterLevel, True );
   
   GlgSetDTag( drawing, "HeaterAlarm", HeaterAlarm ? 1.0 : 0, True );
   GlgSetDTag( drawing, "WaterAlarm", WaterAlarm ? 1.0 : 0, True );
   
   /* Pass if_changed=False to move the chart even if the value did not 
      change. The rest of resources use True to update them only if their 
      values changed.
   */
   GlgSetDTag( drawing, "PlotValueEntryPoint", HeaterTemperature, False );
   
   GlgSetDTag( drawing, "PressureValue", 5.0 * HeaterPressure, True );
}

/*---------------------------------------------------------------------
|
*/
void UpdateProcessResources( GlgObject drawing )
{
   GlgSetDResourceIf( drawing, "SolventValve/Value", SolventValve, True );
   GlgSetDResourceIf( drawing, "SteamValve/Value", SteamValve, True );
   GlgSetDResourceIf( drawing, "CoolingValve/Value", CoolingValve, True );
   GlgSetDResourceIf( drawing, "WaterValve/Value", WaterValve, True );

   GlgSetDResourceIf( drawing, "SolventFlow", GetFlow( SOLVENT_FLOW ), True );
   GlgSetDResourceIf( drawing, "SteamFlow", GetFlow( STEAM_FLOW ), True );
   GlgSetDResourceIf( drawing, "CoolingFlow", GetFlow( COOLING_FLOW ), True );
   GlgSetDResourceIf( drawing, "WaterFlow", GetFlow( WATER_FLOW ), True );
   
   GlgSetDResourceIf( drawing, "OutFlow", OutFlow, True );

   GlgSetDResourceIf( drawing, "Heater/SteamTemperature", 
                      SteamTemperature, True );
   GlgSetDResourceIf( drawing, "Heater/HeaterTemperature", 
                      HeaterTemperature, True );
   GlgSetDResourceIf( drawing, "BeforePreHeaterTemperature", 
                      BeforePreHeaterTemperature, True );
   GlgSetDResourceIf( drawing, "PreHeaterTemperature",
                      PreHeaterTemperature, True );
   GlgSetDResourceIf( drawing, "AfterPreHeaterTemperature", 
                      AfterPreHeaterTemperature, True );
   GlgSetDResourceIf( drawing, "CoolingTemperature", 
                      CoolingTemperature, True );

   GlgSetDResourceIf( drawing, "Heater/HeaterLevel", HeaterLevel, True );
   GlgSetDResourceIf( drawing, "WaterSeparator/WaterLevel", WaterLevel, True );
   
   GlgSetDResourceIf( drawing, "HeaterAlarm", HeaterAlarm, True );
   GlgSetDResourceIf( drawing, "WaterAlarm", WaterAlarm, True );

   /* Pass if_changed=False to move the chart even if the value did not 
      change. The rest of resources use True to update them only if their 
      values changed.
   */
   GlgSetDResourceIf( drawing, "ChartVP/Chart/Plots/Plot#0/ValueEntryPoint",
                      HeaterTemperature, False );

   GlgSetDResourceIf( drawing, "PressureGauge/Value", 5. * HeaterPressure, 
                      True );
}

/*----------------------------------------------------------------------
| A custom input handler for the page. If it returns false, the default
| input handler of the SCADA Viewer will be used to process common
| events and commands.
*/
static GlgBoolean procInputCB( HMIPage * hmi_page, GlgObject viewport, 
                               GlgAnyType client_data, GlgAnyType call_data )

{
   GlgObject message_obj = (GlgObject) call_data;
   GlgObject drawing;
   char 
     * origin,
     * format,
     * action,
     * subaction;
   double
     pipe_vis,
     flow_vis,
     auto_scroll;
   GlgBoolean processed = GlgFalse;

   GlgGetSResource( message_obj, "Origin", &origin );
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "SubAction", &subaction );

   drawing = Viewer.DrawingAreaVP;

   /* Process button events. */
   if( strcmp( format, "Button" ) == 0 )   
   {
      if( strcmp( action, "ValueChanged" ) != 0 )
	return GlgFalse;

      processed = GlgTrue;

      GlgGetDResource( drawing, "3DPipesToggle/OnState", &pipe_vis );
      GlgGetDResource( drawing, "FlowToggle/OnState", &flow_vis );

      if( strcmp( origin, "3DPipesToggle" ) == 0 )
      {
	 /* Make sure either pipes or flow is visible. */
	 if( !pipe_vis && !flow_vis )
           GlgSetDResource( drawing, "FlowToggle/OnState", 1. );
      }
      else if( strcmp( origin, "FlowToggle" ) == 0 )
      {
	 /* Make sure either pipes or flow is visible. */
	 if( !pipe_vis && !flow_vis )
           GlgSetDResource( drawing, "3DPipesToggle/OnState", 1. );
      }
      else if( strcmp( origin, "ToggleAutoScroll" ) == 0 )
      {
         /* Activate chart's X pan slider when AutoScroll=OFF.
            The toggle is connected to the chart's AutoScroll and controls it.
            The X pan slider is activated here.
         */
         GlgGetDResource( drawing, "ChartVP/Chart/AutoScroll", &auto_scroll );
         GlgSetDResource( drawing, "ChartVP/Pan", (double)
                          ( auto_scroll ? GLG_PAN_Y_AUTO : 
                            GLG_PAN_X | GLG_PAN_Y_AUTO ) );

         if( auto_scroll )
         {
            /* Reset the chart's ranges when returning to auto-scroll. */
            GlgSetDResource( drawing, "ChartVP/Chart/Plots/Plot#0/YLow", 50. );
            GlgSetDResource( drawing, "ChartVP/Chart/Plots/Plot#0/YHigh", 150. );
         }
      }
      else
        processed = GlgFalse;
   }

   else if( strcmp( format, "CustomEvent" ) == 0 )
   {
      char * event_label;
      double button, increment;

      /* Handle custom event actions attached to valves to open or close 
         them when the user clicks on them with the left or right mouse 
         button.
      */
      
      GlgGetSResource( message_obj, "EventLabel", &event_label );
      GlgGetDResource( message_obj, "ButtonIndex", &button );
      
      if( button == 1. )
        increment = 1.;
      else
        increment = -1.;

      processed = GlgTrue;
      if( strcmp( event_label, "SolventValveClick" ) == 0 )
      { 
         SolventValve += 0.2 * increment;
         SolventValve = PutInRange( SolventValve, 0., 1. );
         GlgSetDResource( drawing, "SolventValve/Value", SolventValve );
      }
      else if( strcmp( event_label, "SteamValveClick" ) == 0 )
      {
         SteamValve += 0.2 * increment;
         SteamValve = PutInRange( SteamValve, 0., 1. );
         GlgSetDResource( drawing, "SteamValve/Value", SteamValve );
      }
      else if( strcmp( event_label, "CoolingValveClick" ) == 0 )
      {
         CoolingValve += 0.2 * increment;
         CoolingValve = PutInRange( CoolingValve, 0., 1. );
         GlgSetDResource( drawing, "CoolingValve/Value", CoolingValve );
      }
      else if( strcmp( event_label, "WaterValveClick" ) == 0 )
      {
            WaterValve += 0.2 * increment;
            WaterValve = PutInRange( WaterValve, 0., 1. );
            GlgSetDResource( drawing, "WaterValve/Value", WaterValve );
      }
      /* Erase or display the pressure gauge when the gauge or the heater
         are clicked on.
      */
      else if( strcmp( event_label, "HeaterClick" ) == 0 ||
               strcmp( event_label, "PressureGaugeClick" ) == 0 )
      {
         double visibility;

         GlgGetDResource( drawing, "PressureGauge/Visibility", &visibility );
         GlgSetDResource( drawing, "PressureGauge/Visibility",
                          !visibility ? 1. : 0. );
      }
      else
        processed = GlgFalse;
   }
      
   if( processed )
     GlgUpdate( drawing );

   /* If event was processed here, return GlgTrue not to process it outside. */
   return processed;
}

/*----------------------------------------------------------------------
| Returns GlgTrue if tag sources need to be remapped for the page.
*/
static GlgBoolean procNeedTagRemapping( HMIPage * hmi_page )
{
   /* In the random data mode, remap tags to use TagNames as TagSources
      for updating with simulated data.
      In the live data mode, use TagSources defined in the drawing by the
      user.
   */
   return Viewer.RandomData ? GlgTrue : GlgFalse;
}

/*----------------------------------------------------------------------
| Reassign TagSource parameter for a given tag object to a new
| TagSource value. tag_source and tag_name parameters are the current 
| TagSource and TagName of the tag_obj.
*/
static void procRemapTagObject( HMIPage * hmi_page, GlgObject tag_obj, 
                               char * tag_name, char * tag_source )
{
   if( !Viewer.RandomData )
     return;

   /* Use tag object's TagName as TagSource for updating with simulated data. */
   AssignTagSource( tag_obj, tag_name );
}

/*---------------------------------------------------------------------
| Returns the flow value, which is later used as a line type value used 
| to simulate liquid flow.
*/
double GetFlow( FlowType type )
{
   switch( type )
   {
    case SOLVENT_FLOW:
      return SolventFlow = GetFlowValue( SolventFlow, SolventValve );

    case STEAM_FLOW:
      return SteamFlow = GetFlowValue( SteamFlow, SteamValve );

    case COOLING_FLOW:
      return CoolingFlow = GetFlowValue( CoolingFlow, CoolingValve );

    case WATER_FLOW:
      return WaterFlow = GetFlowValue( WaterFlow, WaterValve );

    default: return 0.;
   }
}

/*---------------------------------------------------------------------
| Recalculates the line type values used to simulate liquid flow based
|     on the previous line type value and a flow speed defined by the
|     valve opening.
| Parameters:
|     state - last value of the line type
|     valve - current valve opening
|
| Shifting the line type pattern's offset is achieved by increasing the
| line type value by 32. Refer to the documentation of the polygon's 
| LineType resource for more details.
| Alternatively, the flow line widget from the Custom Object palette
| may be used for integrated flow line functionality, in which case
| this code is not needed. 
*/
double GetFlowValue( double state, double valve )
{
   int
     value,
     update_interval;

#define FLOW_LINE_TYPE      24
#define NO_FLOW_LINE_TYPE    0
#define MAX_FLOW             5

   if( valve == 0 )
     value = NO_FLOW_LINE_TYPE;     /* Valve is closed - no flow. */
   else
   {
      if( state == 0. )
	value = FLOW_LINE_TYPE;    /* First time: init to FLOW_LINE_TYPE. */
      else
      {
	 /* Skip a few intervals to represent variable flow speed. */
	 update_interval = MAX_FLOW - (int) ( ( valve + 0.1 ) * MAX_FLOW );
	 update_interval = MAX( 0, update_interval );
	 update_interval = MIN( MAX_FLOW, update_interval );
	 if( update_interval == 0 ||
	    ( ProcessCounter % update_interval ) == 0 )
	 {
            /* Add 32 to the line type value to increase the line type 
               pattern's offset by 1. */
	    value = ((int)state) + 32;

            /* Reset periodically at the end of the pattern to prevent 
               overflow. Since the length of the GDI pattern is 24 and 
               the length of the OpenGL pattern is 16, reset after 24 * 16 
               iterations to handle both. */
	    if( ((int)value) == FLOW_LINE_TYPE + 32 * 24 * 16 )
              value = FLOW_LINE_TYPE;
	 }
	 else
           /* No change: skipping a few intervals to show a slow speed. */
	   value = state;
      }
   }
   return value;
}
   
/*---------------------------------------------------------------------
| Helps to implement lag behavior
*/
int LagVar( int variable, int lag )
{
   if( variable )
     return --variable;
   else
     return lag;
}

/*---------------------------------------------------------------------
|
*/
double PutInRange( double variable, double low, double high )
{
   if( variable < low )
     return low;
   else if( variable > high )
     return high;
   else
     return variable;
}
