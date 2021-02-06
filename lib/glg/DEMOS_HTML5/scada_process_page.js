//////////////////////////////////////////////////////////////////////////////
// Extends HMIPageBase with functionality to animate process page with
// simulated data.
//////////////////////////////////////////////////////////////////////////////
function ProcessPage( /* GlgObject */ viewport )
{
   // Demonstrates updating the drawing using either tags or resources.
   this.UseTags = true;   /* boolean  */

   // Constants (int)
   this.UpdateInterval = 50;    // ms
   this.SOLVENT_FLOW = 0;
   this.STEAM_FLOW = 1;
   this.COOLING_FLOW = 2;
   this.WATER_FLOW = 3;

   // Constants (double)
   this.PROCESS_SPEED = 0.05;
   this.HEATER_LEVEL_SPEED = 0.05;
   this.WATER_LEVEL_SPEED = 0.02;
   this.VALVE_CHANGE_SPEED = 0.05;
   this.STEAM_VALVE_CHANGE_SPEED = 0.05;


   // boolean variables
   this.ShowFlow = true;
   this.WaterAlarm = false;
   this.HeaterAlarm = false;

   // int variables
   this.ProcessCounter = 0;
   this.heater_high = 0;
   this.heater_low = 0;
   this.water_high = 0;
   this.water_low = 0;
   this.steam_high = 0;
   this.steam_low = 0;
   this.cooling_high = 0;
   this.cooling_low = 0;

   // double variables
   this.SolventValve = 0.85;
   this.SteamValve = 1.0;
   this.CoolingValve = 0.8;
   this.WaterValve = 0.4;
   this.SolventFlow = 0.0;
   this.SteamFlow = 0.0;
   this.CoolingFlow = 0.0;
   this.WaterFlow = 0.0;
   this.OutFlow = 3495.0;
   this.SteamTemperature = 0.0;
   this.HeaterTemperature = 0.0;
   this.BeforePreHeaterTemperature = 0.0;
   this.PreHeaterTemperature = 0.0;
   this.AfterPreHeaterTemperature = 0.0;
   this.CoolingTemperature = 0.0;
   this.HeaterPressure = 0.0;
   this.HeaterLevel = 0.5;
   this.WaterLevel = 0.1;
   
   this.Viewport = viewport;
}

ProcessPage.prototype = Object.create( HMIPageBase.prototype );
ProcessPage.prototype.constructor = ProcessPage;

////////////////////////////////////////////////////////////////////////////// 
// Returns an update interval in msec for animating drawing with data.
////////////////////////////////////////////////////////////////////////////// 
ProcessPage.prototype.GetUpdateInterval = function()   /* int */
{
   return this.UpdateInterval;
}

//////////////////////////////////////////////////////////////////////////////
// A custom UpdateData method to simulate process data in random demo mode.
//////////////////////////////////////////////////////////////////////////////
ProcessPage.prototype.UpdateData = function()  /* boolean  */
{
   if( !RandomData )
     /* Return false to let the viewer update tags in the drawing from 
        the process database. */
     return false;
   
   this.IterateProcess();
   this.UpdateProcess();

   return true;  /* Return true to prevent the viewer from updating tags. */
}

//////////////////////////////////////////////////////////////////////////////
// SIMULATION ONLY
// All code below is used only to animate the demo with simulated data.
// In a real application, live process data will be queried and used
// to update the drawing.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Recalculates new values for the process using a simulation
// model and updates display with the new values.
//////////////////////////////////////////////////////////////////////////////
ProcessPage.prototype.IterateProcess = function()
{
   ++this.ProcessCounter;
   if( this.ProcessCounter == 0x7fffffff )
     this.ProcessCounter = 0;
   
   this.SteamTemperature += ( this.SteamValve - 0.6 ) * 2 * this.PROCESS_SPEED;
   this.SteamTemperature = this.PutInRange( this.SteamTemperature, 0.0, 1.0 );
      
   this.HeaterTemperature += 
     ( this.SteamTemperature - this.HeaterTemperature * this.HeaterLevel ) *
     this.PROCESS_SPEED;
   this.HeaterTemperature = this.PutInRange( this.HeaterTemperature, 0.0, 1.5 );
      
   this.BeforePreHeaterTemperature +=
     ( 1.5 * this.HeaterTemperature - this.BeforePreHeaterTemperature ) *
     this.PROCESS_SPEED;
   this.BeforePreHeaterTemperature =
     this.PutInRange( this.BeforePreHeaterTemperature, 0.0, 1.0 );
      
   this.PreHeaterTemperature +=
     ( this.BeforePreHeaterTemperature - this.PreHeaterTemperature ) *
     this.PROCESS_SPEED;
   this.PreHeaterTemperature =
     this.PutInRange( this.PreHeaterTemperature, 0.0, 1.0 );
      
   this.AfterPreHeaterTemperature +=
     ( 0.9 * this.HeaterTemperature - this.AfterPreHeaterTemperature ) *
     this.PROCESS_SPEED ;
   this.AfterPreHeaterTemperature =
     this.PutInRange( this.AfterPreHeaterTemperature, 0.0, 1.0 );
      
   this.CoolingTemperature +=
     ( this.AfterPreHeaterTemperature - this.CoolingTemperature -
       this.CoolingValve ) * this.PROCESS_SPEED;
   this.CoolingTemperature =
     this.PutInRange( this.CoolingTemperature, 0.0, 1.0 );
      
   this.OutFlow = this.SolventValve * 3495.0;
      
   this.HeaterLevel += ( this.SolventValve - 0.75 ) * this.HEATER_LEVEL_SPEED;
   this.HeaterLevel = this.PutInRange( this.HeaterLevel, 0.0, 1.0 );
      
   // Inversed
   this.WaterLevel += ( 0.5 - this.WaterValve ) * this.WATER_LEVEL_SPEED;
   this.WaterLevel = this.PutInRange( this.WaterLevel, 0.0, 1.0 );
      
   if( this.HeaterLevel > 0.9 || this.heater_high != 0 )
   {
      this.heater_high = this.LugVar( this.heater_high, 10 );
      this.SolventValve -= this.VALVE_CHANGE_SPEED;
   }
   else if( this.HeaterLevel < 0.45 || this.heater_low != 0 )
   {
      this.heater_low = this.LugVar( this.heater_low, 10 );
      this.SolventValve += this.VALVE_CHANGE_SPEED;
   }
   this.SolventValve = this.PutInRange( this.SolventValve, 0.0, 1.0 );
      
   // Inversed
   if( this.WaterLevel > 0.2 || this.water_high != 0 )
   {
      this.water_high = this.LugVar( this.water_high, 10 );
      this.WaterValve += this.VALVE_CHANGE_SPEED;
   }
   else if( this.WaterLevel < 0.05 || this.water_low != 0 )
   {
      this.water_low = this.LugVar( this.water_low, 10 );
      this.WaterValve -= this.VALVE_CHANGE_SPEED;
   }
   this.WaterValve = this.PutInRange( this.WaterValve, 0.0, 1.0 );
      
   if( this.SteamTemperature > 0.9 || this.steam_high != 0 )
   {
      this.LugVar( this.steam_high, 20 );
      this.SteamValve -= this.STEAM_VALVE_CHANGE_SPEED;
   }
   else if( this.SteamTemperature < 0.2 || this.steam_low != 0 )
   {
      this.LugVar( this.steam_low, 20 );
      this.SteamValve += this.STEAM_VALVE_CHANGE_SPEED;
   }
   this.SteamValve = this.PutInRange( this.SteamValve, 0.0, 1.0 );
   
   if( this.CoolingTemperature > 0.7 || this.cooling_high != 0 )
   {
      this.LugVar( this.cooling_high, 10 );
      this.CoolingValve += this.VALVE_CHANGE_SPEED;
   }
   else if( this.CoolingTemperature < 0.3 || this.cooling_low != 0 )
   {
      this.LugVar( this.cooling_low, 10 );
      this.CoolingValve -= this.VALVE_CHANGE_SPEED;
   }
   this.CoolingValve = this.PutInRange( this.CoolingValve, 0.0, 1.0 );
   
   this.HeaterPressure =
     this.HeaterLevel * ( this.HeaterTemperature + 1.0 ) / 2.0;
   this.HeaterPressure = this.PutInRange( this.HeaterPressure, 0.0, 1.0 );
      
   this.HeaterAlarm = ( this.HeaterLevel < 0.45 || this.HeaterLevel > 0.9 );
   this.WaterAlarm = ( this.WaterLevel > 0.2 || this.WaterLevel < 0.05 );
}

//////////////////////////////////////////////////////////////////////////////
// Injects the new recalculated values into the drawing.
//////////////////////////////////////////////////////////////////////////////
ProcessPage.prototype.UpdateProcess = function()
{
   // The drawing can be updated using either tags or resources.
   if( this.UseTags )
     this.UpdateProcessTags();
   else
     this.UpdateProcessResources();      
   
   this.Viewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
// Updates drawing using resources.
//////////////////////////////////////////////////////////////////////////////
ProcessPage.prototype.UpdateProcessTags = function()
{
   this.Viewport.SetDTag( "SolventValveValue", this.SolventValve, true );
   this.Viewport.SetDTag( "SteamValveValue", this.SteamValve, true );
   this.Viewport.SetDTag( "CoolingValveValue", this.CoolingValve, true );
   this.Viewport.SetDTag( "WaterValveValue", this.WaterValve, true );
   
   if( this.ShowFlow )
   {
      this.Viewport.SetDTag( "SolventFlow", this.GetFlow( this.SOLVENT_FLOW ),
                               true );
      this.Viewport.SetDTag( "SteamFlow", this.GetFlow( this.STEAM_FLOW ),
                             true );
      this.Viewport.SetDTag( "CoolingFlow", this.GetFlow( this.COOLING_FLOW ),
                               true );
      this.Viewport.SetDTag( "WaterFlow", this.GetFlow( this.WATER_FLOW ),
                             true );
      
      this.Viewport.SetDTag( "OutFlow", this.OutFlow, true );
   }
   
   this.Viewport.SetDTag( "SteamTemperature", this.SteamTemperature, true );
   this.Viewport.SetDTag( "HeaterTemperature", this.HeaterTemperature, true );
   this.Viewport.SetDTag( "BeforePreHeaterTemperature",
                            this.BeforePreHeaterTemperature, true );
   this.Viewport.SetDTag( "PreHeaterTemperature", this.PreHeaterTemperature,
                            true );
   this.Viewport.SetDTag( "AfterPreHeaterTemperature",
                            this.AfterPreHeaterTemperature, true );
   this.Viewport.SetDTag( "CoolingTemperature", this.CoolingTemperature,
                            true );
      
   this.Viewport.SetDTag( "HeaterLevel", this.HeaterLevel, true );
   this.Viewport.SetDTag( "WaterLevel", this.WaterLevel, true );
   
   this.Viewport.SetDTag( "HeaterAlarm", this.HeaterAlarm ? 1.0 : 0.0, true );
   this.Viewport.SetDTag( "WaterAlarm", this.WaterAlarm ? 1.0 : 0.0, true );
   
   /* Pass if_changed=false to move the chart even if the value did not 
      change. The rest of resources use true to update them only if their 
      values changed.
   */
   this.Viewport.SetDTag( "PlotValueEntryPoint", this.HeaterTemperature,
                            false );
      
   this.Viewport.SetDTag( "PressureValue", 5.0 * this.HeaterPressure, true );
}   
   
//////////////////////////////////////////////////////////////////////////////
// Updates drawing using resources.
//////////////////////////////////////////////////////////////////////////////
ProcessPage.prototype.UpdateProcessResources = function()
{
   this.Viewport.SetDResourceIf( "SolventValve/Value", this.SolventValve, true );
   this.Viewport.SetDResourceIf( "SteamValve/Value", this.SteamValve, true );
   this.Viewport.SetDResourceIf( "CoolingValve/Value", this.CoolingValve, true );
   this.Viewport.SetDResourceIf( "WaterValve/Value", this.WaterValve, true );
   
   if( this.ShowFlow )
   {
      this.Viewport.SetDResourceIf( "SolventFlow",
                                    this.GetFlow( this.SOLVENT_FLOW ), true );
      this.Viewport.SetDResourceIf( "SteamFlow",
                                    this.GetFlow( this.STEAM_FLOW ), true );
      this.Viewport.SetDResourceIf( "CoolingFlow",
                                    this.GetFlow( this.COOLING_FLOW ), true );
      this.Viewport.SetDResourceIf( "WaterFlow",
                                    this.GetFlow( this.WATER_FLOW ), true );
      
      this.Viewport.SetDResourceIf( "OutFlow", this.OutFlow, true );
   }
   
   this.Viewport.SetDResourceIf( "Heater/SteamTemperature",
                                 this.SteamTemperature, true );
   this.Viewport.SetDResourceIf( "Heater/HeaterTemperature",
                                 this.HeaterTemperature, true );
   this.Viewport.SetDResourceIf( "BeforePreHeaterTemperature",
                                 this.BeforePreHeaterTemperature, true );
   this.Viewport.SetDResourceIf( "PreHeaterTemperature",
                                 this.PreHeaterTemperature, true );
   this.Viewport.SetDResourceIf( "AfterPreHeaterTemperature", 
                                 this.AfterPreHeaterTemperature, true );
   this.Viewport.SetDResourceIf( "CoolingTemperature", this.CoolingTemperature,
                                 true );
   
   this.Viewport.SetDResourceIf( "Heater/HeaterLevel", this.HeaterLevel, true );
   this.Viewport.SetDResourceIf( "WaterSeparator/WaterLevel", this.WaterLevel,
                                 true );
   
   this.Viewport.SetDResourceIf( "HeaterAlarm", this.HeaterAlarm ? 1.0 : 0,
                                 true );
   this.Viewport.SetDResourceIf( "WaterAlarm", this.WaterAlarm ? 1.0 : 0, true );
   
   /* Pass if_change=false to move the chart even if the value did not change.
      The rest of resources use true to update them only if their values 
      changed.
   */
   this.Viewport.SetDResourceIf( "ChartVP/Chart/Plots/Plot#0/ValueEntryPoint", 
                                 this.HeaterTemperature, false );
   
   this.Viewport.SetDResourceIf( "PressureGauge/Value",
                                 5.0 * this.HeaterPressure, true );
}
   
//////////////////////////////////////////////////////////////////////////////
// A custom input handler for the page. If it returns false, the default
// input handler of the SCADA Viewer will be used to process common
// events and commands.
//////////////////////////////////////////////////////////////////////////////
ProcessPage.prototype.InputCallback =    /* boolean */
  function( /* GlgObject */ viewport, /* GlgObject */ message_obj )
{
   var origin = message_obj.GetSResource( "Origin" );
   var format = message_obj.GetSResource( "Format" );
   var action = message_obj.GetSResource( "Action" );

   if( format == "Button" )
   {	 
      if( action != "ValueChanged" )
        return false;
      
      var pipe_vis =   /* int */
        Math.trunc( this.Viewport.GetDResource( "3DPipesToggle/OnState" ) );
      var flow_vis =   /* int */
        Math.trunc( this.Viewport.GetDResource( "FlowToggle/OnState" ) );

      if( origin == "3DPipesToggle" )
      {
         // Make sure either pipes or flow is visible.
         if( pipe_vis == 0 && flow_vis == 0 )
           this.Viewport.SetDResource( "FlowToggle/OnState", 1.0 );
      }
      else if( origin == "FlowToggle" )
      {
         // Make sure either pipes or flow is visible.
         if( pipe_vis == 0 && flow_vis == 0 )
           this.Viewport.SetDResource( "3DPipesToggle/OnState", 1.0 );
      }
      else if( origin == "ToggleAutoScroll" )
      {
         /* Activate chart's X pan slider when AutoScroll=OFF.
            The toggle is connected to the chart's AutoScroll and controls 
            it. The X pan slider is activated here.
         */
         var auto_scroll =    /* boolean */
           ( this.Viewport.GetDResource( "ChartVP/Chart/AutoScroll" ) != 0.0 );
         this.Viewport.SetDResource( "ChartVP/Pan",
                                     ( auto_scroll ? GLG.GlgPanType.PAN_Y_AUTO : 
                                       GLG.GlgPanType.PAN_X | GLG.GlgPanType.PAN_Y ) );

         if( auto_scroll )
         {
            /* Reset the chart's ranges when returning to auto-scroll. */
            this.Viewport.SetDResource( "ChartVP/Chart/Plots/Plot#0/YLow",
                                        50.0 );
            this.Viewport.SetDResource( "ChartVP/Chart/Plots/Plot#0/YHigh",
                                        150.0 );
         }

         this.Viewport.Update();
         return true;
      }
   }

   // Handle mouse clicks on the valves to open or close them.
   else if( format == "CustomEvent" )
   {
      var event_label = message_obj.GetSResource( "EventLabel" );   /* String */
      var button =    /* int */
        Math.trunc( message_obj.GetDResource( "ButtonIndex" ) );
         
      var increment;   /* double */
      if( button == 1 )
        increment = 1.0;
      else
        increment = -1.0;

      if( event_label == "SolventValveClick" )
      { 
         this.SolventValve += 0.2 * increment;
         this.SolventValve = this.PutInRange( this.SolventValve, 0.0, 1.0 );
         this.Viewport.SetDResource( "SolventValve/Value", this.SolventValve );
      }
      else if( event_label == "SteamValveClick" )
      {
         this.SteamValve += 0.2 * increment;
         this.SteamValve = this.PutInRange( this.SteamValve, 0.0, 1.0 );
         this.Viewport.SetDResource( "SteamValve/Value", this.SteamValve );
         }
      else if( event_label == "CoolingValveClick" )
      {
         this.CoolingValve += 0.2 * increment;
         this.CoolingValve = this.PutInRange( this.CoolingValve, 0.0, 1.0 );
         this.Viewport.SetDResource( "CoolingValve/Value", this.CoolingValve );
      }
      else if( event_label == "WaterValveClick" )
      {
         this.WaterValve += 0.2 * increment;
         this.WaterValve = this.PutInRange( this.WaterValve, 0.0, 1.0 );
         this.Viewport.SetDResource( "WaterValve/Value", this.WaterValve );
      }
      /* Erase or display the pressure gauge when the gauge or the heater
         are clicked on.
      */
      else if( event_label == "HeaterClick" ||
               event_label == "PressureGaugeClick" )
      {
         var visibility =    /* int */
           Math.trunc( this.Viewport.GetDResource( "PressureGauge/Visibility" ) );
         this.Viewport.SetDResource( "PressureGauge/Visibility",
                                     visibility == 0 ? 1.0 : 0.0 );
      }
      else
        return false;
      
      return true;
   }
   
   return false;
}

//////////////////////////////////////////////////////////////////////////////
ProcessPage.prototype.AdjustForMobileDevices = function()
{
   if( screen.width < 600 )
     // Increase button lengths for small screens to fit labels.
     this.Viewport.SetDResource( "FlowToggle/XScale", 1.2 );

   /* Adjust chart offsets to fit chart labels on mobile devices with 
      canvas scaling.
   */
   if( CoordScale != 1.0 )
   {
      var chart =     /* GlgObject */
        this.Viewport.GetResourceObject( "ChartVP/Chart" );
      AdjustOffset( chart, "OffsetTop", 10. );
      AdjustOffset( chart, "OffsetLeft", 10. );
      AdjustOffset( chart, "OffsetBottom", -5. );
   }
}

//////////////////////////////////////////////////////////////////////////////
// Returns the flow value, which is later used as a line type value used 
// to simulate liquid flow.
//////////////////////////////////////////////////////////////////////////////
ProcessPage.prototype.GetFlow = function( /* int */ type )   /* double */
{
   if( type == this.SOLVENT_FLOW )
     return this.SolventFlow =
       this.GetFlowValue( this.SolventFlow, this.SolventValve );
   else if( type == this.STEAM_FLOW )
     return this.SteamFlow =
       this.GetFlowValue( this.SteamFlow, this.SteamValve );
   else if( type == this.COOLING_FLOW )
     return this.CoolingFlow =
       this.GetFlowValue( this.CoolingFlow, this.CoolingValve );
   else if( type == this.WATER_FLOW )
     return this.WaterFlow =
       this.GetFlowValue( this.WaterFlow, this.WaterValve );
   else
     return 0.0;
}
   
//////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////
ProcessPage.prototype.GetFlowValue =
  function( /* double */ state, /* double */ valve )    /* double  */
{
   var
     value,             /* int */
     update_interval;   /* int */
   
   const FLOW_LINE_TYPE = 24;
   const NO_FLOW_LINE_TYPE = 0;
   const MAX_FLOW = 5;
      
   if( valve == 0 )
     value = NO_FLOW_LINE_TYPE;     // Valve is closed - no flow.
   else
   {
      if( state == 0.0 )
        value = FLOW_LINE_TYPE;    // First time: init to FLOW_LINE_TYPE.
      else
      {
         // Skip a few intervals to represent variable flow speed.
         update_interval = MAX_FLOW - Math.trunc( ( valve + 0.1 ) * MAX_FLOW );
         update_interval = Math.min( 0, update_interval );
         update_interval = Math.max( MAX_FLOW, update_interval );
         if( update_interval == 0 ||
             ( this.ProcessCounter % update_interval ) == 0 )
         {
            /* Add 32 to the line type value to increase the line type 
               pattern's offset by 1.
            */
            value = Math.trunc( state ) + 32;

            /* Reset periodically at the end of the pattern to prevent 
               overflow. Since the length of the GDI pattern is 24 and 
               the length of the OpenGL pattern is 16, reset after 24 * 16 
               iterations to handle both.
            */
            if( value == FLOW_LINE_TYPE + 32 * 24 * 16 )
              value = FLOW_LINE_TYPE;
         }
         else
           // No change: skipping a few intervals to show a slow speed.
           value = Math.trunc( state );
      }
   }
   return value;
}
   
//////////////////////////////////////////////////////////////////////////////
// Helps to implement lug behavior
//////////////////////////////////////////////////////////////////////////////
ProcessPage.prototype.LugVar =
  function( /* int */ variable, /* int */ lug )   /* int */
{
   if( variable != 0 )
     return --variable;
   else
     return lug;
}

//////////////////////////////////////////////////////////////////////////////
ProcessPage.prototype.PutInRange =    /* double */
  function( /* double */ variable, /* double */ low,
            /* double */ high )
{
   if( variable < low )
     return low;
   else if( variable > high )
     return high;
   else
     return variable;
}
   
//////////////////////////////////////////////////////////////////////////////
ProcessPage.prototype.NeedTagRemapping = function()   /* boolean  */
{      
   /* In the random data mode, remap tags to use TagNames as TagSources
      for updating with simulated data.
      In the live data mode, use TagSources defined in the drawing by the
      user.
   */
   return RandomData ? true : false;
}

//////////////////////////////////////////////////////////////////////////////
ProcessPage.prototype.RemapTagObject =
  function( /* GlgObject */ tag_obj, /* String */ tag_name,
            /* String */ tag_source )
{
   if( !RandomData )
     return;
   
   // Use tag object's TagName as TagSource for updating with simulated data.
   AssignTagSource( tag_obj, tag_name );
}
