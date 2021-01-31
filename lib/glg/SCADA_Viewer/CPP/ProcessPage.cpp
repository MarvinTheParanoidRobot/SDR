#include "ProcessPage.h"
#include "GlgSCADAViewer.h"

/* Process Control Page.

   The data in GlgSCADAViewer class are accessible via the global Viewer 
   variable in the base class HMIPageBase.
*/

/* Demonstrates updating the drawing using either tags (GlgTrue) or 
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

// Constructor
ProcessPage::ProcessPage( GlgSCADAViewer * viewer ) : HMIPageBase( viewer )
{
   // Store object ID of the viewport of the loaded page.
   Viewport = Viewer->DrawingAreaVP;
}

// Destructor
ProcessPage::~ProcessPage( void )
{
}

/*----------------------------------------------------------------------
| Returns an update interval in msec for animating drawing with data.
*/
int ProcessPage::GetUpdateInterval( void )
{
   return 50;   /* 50ms = 20 times/sec refresh */
}

/*----------------------------------------------------------------------
| A custom UpdateData method to simulate process data in random data mode.
*/
GlgBoolean ProcessPage::UpdateData( void )
{
   if( !Viewer->RandomData )
     /* Return false to let the viewer update tags in the drawing from 
        the process database. */
     return GlgFalse;

   IterateProcess( Viewer->DrawingAreaVP );
   UpdateProcess( Viewer->DrawingAreaVP );

   return GlgTrue; /* Return true to prevent the Viewer from updating tags. */
}

/*----------------------------------------------------------------------
| Recalculates new values for the process using a simulation model and
| updates display with the new values.
*/
void ProcessPage::IterateProcess( GlgObjectC& drawing )
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
      heater_high = LugVar( heater_high, 10 );
      SolventValve -= VALVE_CHANGE_SPEED;
   }
   else if( HeaterLevel < 0.45 || heater_low )
   {
      heater_low = LugVar( heater_low, 10 );
      SolventValve += VALVE_CHANGE_SPEED;
   }
   SolventValve = PutInRange( SolventValve, 0., 1. );

   /* Inversed */
   if( WaterLevel > 0.2 || water_high )
   {
      water_high = LugVar( water_high, 10 );
      WaterValve += VALVE_CHANGE_SPEED;
   }
   else if( WaterLevel < 0.05 || water_low )
   {
      water_low = LugVar( water_low, 10 );
      WaterValve -= VALVE_CHANGE_SPEED;
   }
   WaterValve = PutInRange( WaterValve, 0., 1. );

   if( SteamTemperature > 0.9 || steam_high )
   {
      steam_high = LugVar( steam_high, 20 );
      SteamValve -= STEAM_VALVE_CHANGE_SPEED;
   }
   else if( SteamTemperature < 0.2 || steam_low )
   {
      steam_low = LugVar( steam_low, 20 );
      SteamValve += STEAM_VALVE_CHANGE_SPEED;
   }
   SteamValve = PutInRange( SteamValve, 0., 1. );

   if( CoolingTemperature > 0.7 || cooling_high )
   {
      cooling_high = LugVar( cooling_high, 10 );
      CoolingValve += VALVE_CHANGE_SPEED;
   }
   else if( CoolingTemperature < 0.3 || cooling_low )
   {
      cooling_low = LugVar( cooling_low, 10 );
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
void ProcessPage::UpdateProcess( GlgObjectC& drawing )
{
   /* The drawing can be updated using either tags or resources. */
   if( USE_TAGS )
     UpdateProcessTags( drawing );
   else
     UpdateProcessResources( drawing );    
   
   drawing.Update();
}

/*---------------------------------------------------------------------
|
*/
void ProcessPage::UpdateProcessTags( GlgObjectC& drawing )
{
   drawing.SetTag( "SolventValveValue", SolventValve, GlgTrue );
   drawing.SetTag( "SteamValveValue", SteamValve, GlgTrue );
   drawing.SetTag( "CoolingValveValue", CoolingValve, GlgTrue );
   drawing.SetTag( "WaterValveValue", WaterValve, GlgTrue );
   
   drawing.SetTag( "SolventFlow", GetFlow( SOLVENT_FLOW ), GlgTrue );
   drawing.SetTag( "SteamFlow", GetFlow( STEAM_FLOW ), GlgTrue );
   drawing.SetTag( "CoolingFlow", GetFlow( COOLING_FLOW ), GlgTrue );
   drawing.SetTag( "WaterFlow", GetFlow( WATER_FLOW ), GlgTrue );
   
   drawing.SetTag( "OutFlow", OutFlow, GlgTrue );
   
   drawing.SetTag( "SteamTemperature", SteamTemperature, GlgTrue );
   drawing.SetTag( "HeaterTemperature", HeaterTemperature, GlgTrue );
   drawing.SetTag( "BeforePreHeaterTemperature", BeforePreHeaterTemperature,
               GlgTrue );
   drawing.SetTag( "PreHeaterTemperature", PreHeaterTemperature, GlgTrue );
   drawing.SetTag( "AfterPreHeaterTemperature", AfterPreHeaterTemperature,
               GlgTrue );
   drawing.SetTag( "CoolingTemperature", CoolingTemperature, GlgTrue );
   
   drawing.SetTag( "HeaterLevel", HeaterLevel, GlgTrue );
   drawing.SetTag( "WaterLevel", WaterLevel, GlgTrue );
   
   drawing.SetTag( "HeaterAlarm", HeaterAlarm ? 1.0 : 0, GlgTrue );
   drawing.SetTag( "WaterAlarm", WaterAlarm ? 1.0 : 0, GlgTrue );
   
   /* Pass if_changed=False to move the chart even if the value did not 
      change. The rest of resources use GlgTrue to update them only if their 
      values changed.
   */
   drawing.SetTag( "PlotValueEntryPoint", HeaterTemperature, GlgFalse );
   
   drawing.SetTag( "PressureValue", 5.0 * HeaterPressure, GlgTrue );
}

/*---------------------------------------------------------------------
|
*/
void ProcessPage::UpdateProcessResources( GlgObjectC& drawing )
{
   drawing.SetResource( "SolventValve/Value", SolventValve, GlgTrue );
   drawing.SetResource( "SteamValve/Value", SteamValve, GlgTrue );
   drawing.SetResource( "CoolingValve/Value", CoolingValve, GlgTrue );
   drawing.SetResource( "WaterValve/Value", WaterValve, GlgTrue );

   drawing.SetResource( "SolventFlow", GetFlow( SOLVENT_FLOW ), GlgTrue );
   drawing.SetResource( "SteamFlow", GetFlow( STEAM_FLOW ), GlgTrue );
   drawing.SetResource( "CoolingFlow", GetFlow( COOLING_FLOW ), GlgTrue );
   drawing.SetResource( "WaterFlow", GetFlow( WATER_FLOW ), GlgTrue );
   
   drawing.SetResource( "OutFlow", OutFlow, GlgTrue );

   drawing.SetResource( "Heater/SteamTemperature", 
                        SteamTemperature, GlgTrue );
   drawing.SetResource( "Heater/HeaterTemperature", 
                        HeaterTemperature, GlgTrue );
   drawing.SetResource( "BeforePreHeaterTemperature", 
                        BeforePreHeaterTemperature, GlgTrue );
   drawing.SetResource( "PreHeaterTemperature",
                        PreHeaterTemperature, GlgTrue );
   drawing.SetResource( "AfterPreHeaterTemperature", 
                        AfterPreHeaterTemperature, GlgTrue );
   drawing.SetResource( "CoolingTemperature", 
                        CoolingTemperature, GlgTrue );

   drawing.SetResource( "Heater/HeaterLevel", HeaterLevel, GlgTrue );
   drawing.SetResource( "WaterSeparator/WaterLevel", WaterLevel, GlgTrue );
   
   drawing.SetResource( "HeaterAlarm", HeaterAlarm, GlgTrue );
   drawing.SetResource( "WaterAlarm", WaterAlarm, GlgTrue );

   /* Pass if_changed=False to move the chart even if the value did not 
      change. The rest of resources use GlgTrue to update them only if their 
      values changed.
   */
   drawing.SetResource( "ChartVP/Chart/Plots/Plot#0/ValueEntryPoint",
                        HeaterTemperature, GlgFalse );

   drawing.SetResource( "PressureGauge/Value", 5. * HeaterPressure, 
                        GlgTrue );
}

/*----------------------------------------------------------------------
| A custom input handler for the page. If it returns false, the default
| input handler of the SCADA Viewer will be used to ess common
| events and commands.
*/
GlgBoolean ProcessPage::Input( GlgObjectC& viewport, GlgObjectC& message )
{
   SCONST char 
     * origin,
     * format,
     * action,
     * subaction;

   GlgBoolean processed = GlgFalse;
  
   /* Get the message's format, action and origin. */
   message.GetResource( "Format", &format );
   message.GetResource( "Action", &action );
   message.GetResource( "Origin", &origin );
   message.GetResource( "SubAction", &subaction );

   /* Process button events. */
   if( strcmp( format, "Button" ) == 0 )   
   {
      if( strcmp( action, "ValueChanged" ) != 0 )
	return GlgFalse;

      processed = GlgTrue; 

      double pipe_vis, flow_vis;
      Viewport.GetResource( "3DPipesToggle/OnState", &pipe_vis );
      Viewport.GetResource( "FlowToggle/OnState", &flow_vis );

      if( strcmp( origin, "3DPipesToggle" ) == 0 )
      {
	 /* Make sure either pipes or flow is visible. */
	 if( !pipe_vis && !flow_vis )
           Viewport.SetResource( "FlowToggle/OnState", 1. );
      }
      else if( strcmp( origin, "FlowToggle" ) == 0 )
      {
	 /* Make sure either pipes or flow is visible. */
	 if( !pipe_vis && !flow_vis )
           Viewport.SetResource( "3DPipesToggle/OnState", 1. );
      }
      else if( strcmp( origin, "ToggleAutoScroll" ) == 0 )
      {
         /* Activate chart's X pan slider when AutoScroll=OFF.
            The toggle is connected to the chart's AutoScroll and controls it.
            The X pan slider is activated here.
         */
         double auto_scroll;
         Viewport.GetResource( "ChartVP/Chart/AutoScroll", &auto_scroll );
         Viewport.SetResource( "ChartVP/Pan", (double)
                               ( auto_scroll ? GLG_PAN_Y_AUTO : 
                                 GLG_PAN_X | GLG_PAN_Y_AUTO ) );
         
         if( auto_scroll )
         {
            /* Reset the chart's ranges when returning to auto-scroll. */
            Viewport.SetResource( "ChartVP/Chart/Plots/Plot#0/YLow", 50. );
            Viewport.SetResource( "ChartVP/Chart/Plots/Plot#0/YHigh", 150. );
         }
      }
      else
        processed = GlgFalse;
   }

   else if( strcmp( format, "CustomEvent" ) == 0 )
   {
      SCONST char * event_label;
      double button, increment;
         
      /* Handle custom event actions attached to valves to open or close 
         them when the user clicks on them with the left or right mouse 
         button.
      */
      message.GetResource( "EventLabel", &event_label );
      message.GetResource( "ButtonIndex", &button );
      
      if( button == 1. )
        increment = 1.;
      else
        increment = -1.;

      processed = GlgTrue;
      if( strcmp( event_label, "SolventValveClick" ) == 0 )
      { 
         SolventValve += 0.2 * increment;
         SolventValve = PutInRange( SolventValve, 0., 1. );
         Viewport.SetResource( "SolventValve/Value", SolventValve );
      }
      else if( strcmp( event_label, "SteamValveClick" ) == 0 )
      {
         SteamValve += 0.2 * increment;
         SteamValve = PutInRange( SteamValve, 0., 1. );
         Viewport.SetResource( "SteamValve/Value", SteamValve );
      }
      else if( strcmp( event_label, "CoolingValveClick" ) == 0 )
      {
         CoolingValve += 0.2 * increment;
         CoolingValve = PutInRange( CoolingValve, 0., 1. );
         Viewport.SetResource( "CoolingValve/Value", CoolingValve );
      }
      else if( strcmp( event_label, "WaterValveClick" ) == 0 )
      {
         WaterValve += 0.2 * increment;
         WaterValve = PutInRange( WaterValve, 0., 1. );
         Viewport.SetResource( "WaterValve/Value", WaterValve );
      }
      /* Erase or display the pressure gauge when the gauge or the heater
         are clicked on.
      */
      else if( strcmp( event_label, "HeaterClick" ) == 0 ||
               strcmp( event_label, "PressureGaugeClick" ) == 0 )
      {
         double visibility;

         Viewport.GetResource( "PressureGauge/Visibility", &visibility );
         Viewport.SetResource( "PressureGauge/Visibility",
                               !visibility ? 1. : 0. );
      }
      else
        processed = GlgFalse;
   }
      
   if( processed )
     Viewport.Update();

   /* If event was processed here, return GlgTrue not to process it outside. */
   return processed;
}

/*----------------------------------------------------------------------
| Returns GlgTrue if tag sources need to be remapped for the page.
*/
GlgBoolean ProcessPage::NeedTagRemapping( void )
{
   /* In the random data mode, remap tags to use TagNames as TagSources
      for updating with simulated data.
      In the live data mode, use TagSources defined in the drawing by the
      user.
   */
   return Viewer->RandomData ? GlgTrue : GlgFalse;
}

/*----------------------------------------------------------------------
| Reassign TagSource parameter for a given tag object to a new
| TagSource value. tag_source and tag_name parameters are the current 
| TagSource and TagName of the tag_obj.
*/
void ProcessPage::RemapTagObject( GlgObjectC& tag_obj, 
                                  SCONST char * tag_name, 
                                  SCONST char * tag_source )
{
   if( !Viewer->RandomData )
     return;

   /* Use tag object's TagName as TagSource for updating with simulated data. */
   Viewer->AssignTagSource( tag_obj, tag_name );
}

/*---------------------------------------------------------------------
| Returns the flow value, which is later used as a line type value used 
| to simulate liquid flow.
*/
double ProcessPage::GetFlow( FlowType type )
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
double ProcessPage::GetFlowValue( double state, double valve )
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
| Helps to implement lug behavior
*/
int ProcessPage::LugVar( int variable, int lug )
{
   if( variable )
     return --variable;
   else
     return lug;
}

/*---------------------------------------------------------------------
|
*/
double ProcessPage::PutInRange( double variable, double low, double high )
{
   if( variable < low )
     return low;
   else if( variable > high )
     return high;
   else
     return variable;
}
