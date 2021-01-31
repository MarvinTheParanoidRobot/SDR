import com.genlogic.*;

public class ProcessPage extends HMIPageBase
{
   // Demonstrates updating the drawing using either tags or resources.
   boolean UseTags = true;

   // Constants
   static final int
     UpdateInterval = 50,    // ms
     SOLVENT_FLOW = 0,
     STEAM_FLOW = 1,
     COOLING_FLOW = 2,
     WATER_FLOW = 3;

   static final double 
     PROCESS_SPEED = 0.05,
     HEATER_LEVEL_SPEED = 0.05,
     WATER_LEVEL_SPEED = 0.02,
     VALVE_CHANGE_SPEED = 0.05,
     STEAM_VALVE_CHANGE_SPEED = 0.05;

   // Variables
   boolean
     ShowFlow = true,
     WaterAlarm = false,
     HeaterAlarm = false;
   int
     ProcessCounter = 0,
     heater_high = 0,
     heater_low = 0,
     water_high = 0,
     water_low = 0,
     steam_high = 0,
     steam_low = 0,
     cooling_high = 0,
     cooling_low = 0;
   double
     SolventValve = 0.85,
     SteamValve = 1.0,
     CoolingValve = 0.8,
     WaterValve = 0.4,
     SolventFlow = 0.0,
     SteamFlow = 0.0,
     CoolingFlow = 0.0,
     WaterFlow = 0.0,
     OutFlow = 3495.0,
     SteamTemperature = 0.0,
     HeaterTemperature = 0.0,
     BeforePreHeaterTemperature = 0.0,
     PreHeaterTemperature = 0.0,
     AfterPreHeaterTemperature = 0.0,
     CoolingTemperature = 0.0,
     HeaterPressure = 0.0,
     HeaterLevel = 0.5,
     WaterLevel = 0.1;

   GlgObject Viewport;

   /////////////////////////////////////////////////////////////////////
   // Viewer and PageType variables are defined in and assigned by the 
   // base HMIPageBase class.
   /////////////////////////////////////////////////////////////////////
   public ProcessPage( GlgSCADAViewer viewer ) 
   {
      super( viewer );

      Viewport = Viewer.DrawingAreaVP;
   }

   // Returns an update interval in msec for animating drawing with data.
   public int GetUpdateInterval()
   {
      return UpdateInterval;
   }

   //////////////////////////////////////////////////////////////////////////
   // A custom UpdateData method to simulate process data in random demo mode.
   //////////////////////////////////////////////////////////////////////////
   public boolean UpdateData()
   {
      if( !Viewer.RandomData )
        /* Return false to let the viewer update tags in the drawing from 
           the process database. */
        return false;

      IterateProcess();
      UpdateProcess();

      return true;  /* Return true to prevent the viewer from updating tags. */
   }

   //////////////////////////////////////////////////////////////////////////
   // Recalculates new values for the process using a simulation model and
   // updates display with the new values.
   //////////////////////////////////////////////////////////////////////////
   void IterateProcess()
   {
      ++ProcessCounter;
      if( ProcessCounter == 0x7fffffff )
        ProcessCounter = 0;
      
      SteamTemperature += ( SteamValve - 0.6 ) * 2 * PROCESS_SPEED;
      SteamTemperature = PutInRange( SteamTemperature, 0.0, 1.0 );
      
      HeaterTemperature += 
        ( SteamTemperature - HeaterTemperature * HeaterLevel ) * PROCESS_SPEED;
      HeaterTemperature = PutInRange( HeaterTemperature, 0.0, 1.5 );
      
      BeforePreHeaterTemperature +=
        ( 1.5 * HeaterTemperature - BeforePreHeaterTemperature ) *
        PROCESS_SPEED;
      BeforePreHeaterTemperature =
        PutInRange( BeforePreHeaterTemperature, 0.0, 1.0 );
      
      PreHeaterTemperature +=
        ( BeforePreHeaterTemperature - PreHeaterTemperature ) * PROCESS_SPEED;
      PreHeaterTemperature = PutInRange( PreHeaterTemperature, 0.0, 1.0 );
      
      AfterPreHeaterTemperature +=
        ( 0.9 * HeaterTemperature - AfterPreHeaterTemperature ) *
        PROCESS_SPEED ;
      AfterPreHeaterTemperature =
        PutInRange( AfterPreHeaterTemperature, 0.0, 1.0 );
      
      CoolingTemperature +=
        ( AfterPreHeaterTemperature - CoolingTemperature - CoolingValve )
        * PROCESS_SPEED;
      CoolingTemperature = PutInRange( CoolingTemperature, 0.0, 1.0 );
      
      OutFlow = SolventValve * 3495.0;
      
      HeaterLevel += ( SolventValve - 0.75 ) * HEATER_LEVEL_SPEED;
      HeaterLevel = PutInRange( HeaterLevel, 0.0, 1.0 );
      
      // Inversed
      WaterLevel += ( 0.5 - WaterValve ) * WATER_LEVEL_SPEED;
      WaterLevel = PutInRange( WaterLevel, 0.0, 1.0 );
      
      if( HeaterLevel > 0.9 || heater_high != 0 )
      {
         heater_high = LugVar( heater_high, 10 );
         SolventValve -= VALVE_CHANGE_SPEED;
      }
      else if( HeaterLevel < 0.45 || heater_low != 0 )
      {
         heater_low = LugVar( heater_low, 10 );
         SolventValve += VALVE_CHANGE_SPEED;
      }
      SolventValve = PutInRange( SolventValve, 0.0, 1.0 );
      
      // Inversed
      if( WaterLevel > 0.2 || water_high != 0 )
      {
         water_high = LugVar( water_high, 10 );
         WaterValve += VALVE_CHANGE_SPEED;
      }
      else if( WaterLevel < 0.05 || water_low != 0 )
      {
         water_low = LugVar( water_low, 10 );
         WaterValve -= VALVE_CHANGE_SPEED;
      }
      WaterValve = PutInRange( WaterValve, 0.0, 1.0 );
      
      if( SteamTemperature > 0.9 || steam_high != 0 )
      {
         LugVar( steam_high, 20 );
         SteamValve -= STEAM_VALVE_CHANGE_SPEED;
      }
      else if( SteamTemperature < 0.2 || steam_low != 0 )
      {
         LugVar( steam_low, 20 );
         SteamValve += STEAM_VALVE_CHANGE_SPEED;
      }
      SteamValve = PutInRange( SteamValve, 0.0, 1.0 );
      
      if( CoolingTemperature > 0.7 || cooling_high != 0 )
      {
         LugVar( cooling_high, 10 );
         CoolingValve += VALVE_CHANGE_SPEED;
      }
      else if( CoolingTemperature < 0.3 || cooling_low != 0 )
      {
         LugVar( cooling_low, 10 );
         CoolingValve -= VALVE_CHANGE_SPEED;
      }
      CoolingValve = PutInRange( CoolingValve, 0.0, 1.0 );
      
      HeaterPressure = HeaterLevel * ( HeaterTemperature + 1.0 ) / 2.0;
      HeaterPressure = PutInRange( HeaterPressure, 0.0, 1.0 );
      
      HeaterAlarm = ( HeaterLevel < 0.45 || HeaterLevel > 0.9 );
      WaterAlarm = ( WaterLevel > 0.2 || WaterLevel < 0.05 );
   }

   //////////////////////////////////////////////////////////////////////////
   // Injects the new recalculated values into the drawing.
   //////////////////////////////////////////////////////////////////////////
   public void UpdateProcess()
   {
      // The drawing can be updated using either tags or resources.
      if( UseTags )
        UpdateProcessTags();
      else
        UpdateProcessResources();      

      Viewport.Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates drawing using resources.
   //////////////////////////////////////////////////////////////////////////
   public void UpdateProcessTags()
   {
      Viewport.SetDTag( "SolventValveValue", SolventValve, true );
      Viewport.SetDTag( "SteamValveValue", SteamValve, true );
      Viewport.SetDTag( "CoolingValveValue", CoolingValve, true );
      Viewport.SetDTag( "WaterValveValue", WaterValve, true );
      
      if( ShowFlow )
      {
         Viewport.SetDTag( "SolventFlow", GetFlow( SOLVENT_FLOW ), true );
         Viewport.SetDTag( "SteamFlow", GetFlow( STEAM_FLOW ), true );
         Viewport.SetDTag( "CoolingFlow", GetFlow( COOLING_FLOW ), true );
         Viewport.SetDTag( "WaterFlow", GetFlow( WATER_FLOW ), true );
 
         Viewport.SetDTag( "OutFlow", OutFlow, true );
      }

      Viewport.SetDTag( "SteamTemperature", SteamTemperature, true );
      Viewport.SetDTag( "HeaterTemperature", HeaterTemperature, true );
      Viewport.SetDTag( "BeforePreHeaterTemperature", 
                        BeforePreHeaterTemperature, true );
      Viewport.SetDTag( "PreHeaterTemperature", PreHeaterTemperature, true );
      Viewport.SetDTag( "AfterPreHeaterTemperature", 
                        AfterPreHeaterTemperature, true );
      Viewport.SetDTag( "CoolingTemperature", CoolingTemperature, true );
      
      Viewport.SetDTag( "HeaterLevel", HeaterLevel, true );
      Viewport.SetDTag( "WaterLevel", WaterLevel, true );
      
      Viewport.SetDTag( "HeaterAlarm", HeaterAlarm ? 1.0 : 0.0, true );
      Viewport.SetDTag( "WaterAlarm", WaterAlarm ? 1.0 : 0.0, true );
      
      /* Pass if_changed=false to move the chart even if the value did not 
         change. The rest of resources use true to update them only if their 
         values changed.
      */
      Viewport.SetDTag( "PlotValueEntryPoint", HeaterTemperature, false );
      
      Viewport.SetDTag( "PressureValue", 5.0 * HeaterPressure, true );
   }   
   
   //////////////////////////////////////////////////////////////////////////
   // Updates drawing using resources.
   //////////////////////////////////////////////////////////////////////////
   public void UpdateProcessResources()
   {
      Viewport.SetDResource( "SolventValve/Value", SolventValve, true );
      Viewport.SetDResource( "SteamValve/Value", SteamValve, true );
      Viewport.SetDResource( "CoolingValve/Value", CoolingValve, true );
      Viewport.SetDResource( "WaterValve/Value", WaterValve, true );
      
      if( ShowFlow )
      {
         Viewport.SetDResource( "SolventFlow", GetFlow( SOLVENT_FLOW ), true );
         Viewport.SetDResource( "SteamFlow", GetFlow( STEAM_FLOW ), true );
         Viewport.SetDResource( "CoolingFlow", GetFlow( COOLING_FLOW ), true );
         Viewport.SetDResource( "WaterFlow", GetFlow( WATER_FLOW ), true );
 
         Viewport.SetDResource( "OutFlow", OutFlow, true );
      }

      Viewport.SetDResource( "Heater/SteamTemperature", SteamTemperature, true );
      Viewport.SetDResource( "Heater/HeaterTemperature", 
                             HeaterTemperature, true );
      Viewport.SetDResource( "BeforePreHeaterTemperature", 
                             BeforePreHeaterTemperature, true );
      Viewport.SetDResource( "PreHeaterTemperature",
                             PreHeaterTemperature, true );
      Viewport.SetDResource( "AfterPreHeaterTemperature", 
                             AfterPreHeaterTemperature, 
                    true );
      Viewport.SetDResource( "CoolingTemperature", CoolingTemperature, true );
      
      Viewport.SetDResource( "Heater/HeaterLevel", HeaterLevel, true );
      Viewport.SetDResource( "WaterSeparator/WaterLevel", WaterLevel, true );
      
      Viewport.SetDResource( "HeaterAlarm", HeaterAlarm ? 1.0 : 0, true );
      Viewport.SetDResource( "WaterAlarm", WaterAlarm ? 1.0 : 0, true );
      
      // Pass if_change=false to move the chart even if the value did not change.
      // The rest of resources use true to update them only if their values 
      // changed.
      Viewport.SetDResource( "ChartVP/Chart/Plots/Plot#0/ValueEntryPoint", 
                             HeaterTemperature, false );
      
      Viewport.SetDResource( "PressureGauge/Value", 5.0 * HeaterPressure, true );
   }
   
   //////////////////////////////////////////////////////////////////////
   // A custom input handler for the page. If it returns false, the default
   // input handler of the SCADA Viewer will be used to process common
   // events and commands.
   //////////////////////////////////////////////////////////////////////
   public boolean InputCallback( GlgObject viewport, GlgObject message_obj )
   {
      String
        origin,
        format,
        action;

      origin = message_obj.GetSResource( "Origin" );
      format = message_obj.GetSResource( "Format" );
      action = message_obj.GetSResource( "Action" );

      if( format.equals( "Button" ) )
      {	 
         if( !action.equals( "ValueChanged" ) )
           return false;
         
         int pipe_vis = 
           Viewport.GetDResource( "3DPipesToggle/OnState" ).intValue();
         int flow_vis = 
           Viewport.GetDResource( "FlowToggle/OnState" ).intValue();

         if( origin.equals( "3DPipesToggle" ) )
         {
            // Make sure either pipes or flow is visible.
            if( pipe_vis == 0 && flow_vis == 0 )
              Viewport.SetDResource( "FlowToggle/OnState", 1.0 );
         }
         else if( origin.equals( "FlowToggle" ) )
         {
            // Make sure either pipes or flow is visible.
            if( pipe_vis == 0 && flow_vis == 0 )
              Viewport.SetDResource( "3DPipesToggle/OnState", 1.0 );
         }
         else if( origin.equals( "ToggleAutoScroll" ) )
         {
            /* Activate chart's X pan slider when AutoScroll=OFF.
               The toggle is connected to the chart's AutoScroll and controls 
               it. The X pan slider is activated here.
            */
            boolean auto_scroll = 
              ( Viewport.GetDResource( "ChartVP/Chart/AutoScroll" ).doubleValue() != 0.0 );
            Viewport.SetDResource( "ChartVP/Pan", (double) 
                                   ( auto_scroll ? GlgObject.PAN_Y_AUTO : 
                                     GlgObject.PAN_X | GlgObject.PAN_Y ) );

            if( auto_scroll )
            {
               /* Reset the chart's ranges when returning to auto-scroll. */
               Viewport.SetDResource( "ChartVP/Chart/Plots/Plot#0/YLow", 50.0 );
               Viewport.SetDResource( "ChartVP/Chart/Plots/Plot#0/YHigh", 150.0 );
            }

            Viewport.Update();
            return true;
         }
      }

      // Handle mouse clicks on the valves to open or close them.
      else if( format.equals( "CustomEvent" ) )
      {
         String event_label = message_obj.GetSResource( "EventLabel" );
         int button = message_obj.GetDResource( "ButtonIndex" ).intValue();
         
         double increment;
         if( button == 1 )
           increment = 1.0;
         else
           increment = -1.0;

         if( event_label.equals( "SolventValveClick" ) )
         { 
            SolventValve += 0.2 * increment;
            SolventValve = PutInRange( SolventValve, 0.0, 1.0 );
            Viewport.SetDResource( "SolventValve/Value", SolventValve );
         }
         else if( event_label.equals( "SteamValveClick" ) )
         {
            SteamValve += 0.2 * increment;
            SteamValve = PutInRange( SteamValve, 0.0, 1.0 );
            Viewport.SetDResource( "SteamValve/Value", SteamValve );
         }
         else if( event_label.equals( "CoolingValveClick" ) )
         {
            CoolingValve += 0.2 * increment;
            CoolingValve = PutInRange( CoolingValve, 0.0, 1.0 );
            Viewport.SetDResource( "CoolingValve/Value", CoolingValve );
         }
         else if( event_label.equals( "WaterValveClick" ) )
         {
            WaterValve += 0.2 * increment;
            WaterValve = PutInRange( WaterValve, 0.0, 1.0 );
            Viewport.SetDResource( "WaterValve/Value", WaterValve );
         }
         /* Erase or display the pressure gauge when the gauge or the heater
            are clicked on.
         */
         else if( event_label.equals( "HeaterClick" ) ||
                  event_label.equals( "PressureGaugeClick" ) )
         {
            int visibility = 
              Viewport.GetDResource( "PressureGauge/Visibility" ).intValue();
            Viewport.SetDResource( "PressureGauge/Visibility",
                                   visibility == 0 ? 1.0 : 0.0 );
         }
         else
           return false;

         return true;
      }

      return false;
   }

   //////////////////////////////////////////////////////////////////////////
   // Returns the flow value, which is later used as a line type value used 
   // to simulate liquid flow.
   //////////////////////////////////////////////////////////////////////////
   double GetFlow( int type )
   {
      if( type == SOLVENT_FLOW )
        return SolventFlow = GetFlowValue( SolventFlow, SolventValve );
      else if( type == STEAM_FLOW )
        return SteamFlow = GetFlowValue( SteamFlow, SteamValve );
      else if( type == COOLING_FLOW )
        return CoolingFlow = GetFlowValue( CoolingFlow, CoolingValve );
      else if( type == WATER_FLOW )
        return WaterFlow = GetFlowValue( WaterFlow, WaterValve );
      else
        return 0.0;
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Recalculates the line type values used to simulate liquid flow based
   //     on the previous line type value and a flow speed defined by the
   //      valve opening.
   // Parameters:
   //     state - last value of the line type
   //     valve - current valve opening
   //
   // Shifting the line type pattern's offset is achieved by increasing the
   // line type value by 32.0 Refer to the documentation of the polygon's 
   // LineType resource for more details.   
   // Alternatively, the flow line widget from the Custom Object palette
   // may be used for integrated flow line functionality, in which case
   // this code is not needed. 
   //////////////////////////////////////////////////////////////////////////
   double GetFlowValue( double state, double valve )
   {
      int
        value,
        update_interval;
      final int 
        FLOW_LINE_TYPE = 24,
        NO_FLOW_LINE_TYPE = 0,
        MAX_FLOW = 5;
      
      if( valve == 0 )
        value = NO_FLOW_LINE_TYPE;     // Valve is closed - no flow.
      else
      {
         if( state == 0.0 )
           value = FLOW_LINE_TYPE;    // First time: init to FLOW_LINE_TYPE.
         else
         {
            // Skip a few intervals to represent variable flow speed.
            update_interval = MAX_FLOW - (int) ( ( valve + 0.1 ) * MAX_FLOW );
            update_interval = Math.min( 0, update_interval );
            update_interval = Math.max( MAX_FLOW, update_interval );
            if( update_interval == 0 ||
               ( ProcessCounter % update_interval ) == 0 )
            {
               // Add 32 to the line type value to increase the line type 
                 // pattern's offset by 1.
               value = ((int)state) + 32;

               // Reset periodically at the end of the pattern to prevent 
               // overflow. Since the length of the GDI pattern is 24 and 
               // the length of the OpenGL pattern is 16, reset after 24 * 16 
                 // iterations to handle both.
               if( value == FLOW_LINE_TYPE + 32 * 24 * 16 )
                 value = FLOW_LINE_TYPE;
            }
            else
              // No change: skipping a few intervals to show a slow speed.
              value = (int)state;
         }
      }
      return value;
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Helps to implement lug behavior
   //////////////////////////////////////////////////////////////////////////
   int LugVar( int variable, int lug )
   {
      if( variable != 0 )
        return --variable;
      else
        return lug;
   }

   //////////////////////////////////////////////////////////////////////////
   double PutInRange( double variable, double low, double high )
   {
      if( variable < low )
        return low;
      else if( variable > high )
        return high;
      else
        return variable;
   }

   //////////////////////////////////////////////////////////////////////////
   public boolean NeedTagRemapping()
   {      
      /* In the random data mode, remap tags to use TagNames as TagSources
         for updating with simulated data.
         In the live data mode, use TagSources defined in the drawing by the
         user.
      */
      return Viewer.RandomData ? true : false;
   }

   //////////////////////////////////////////////////////////////////////////
   public void RemapTagObject( GlgObject tag_obj, 
                                         String tag_name, String tag_source )
   {
      if( !Viewer.RandomData )
        return;

      // Use tag object's TagName as TagSource for updating with simulated data.
      Viewer.AssignTagSource( tag_obj, tag_name );
   }
}
