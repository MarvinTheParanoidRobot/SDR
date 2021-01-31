import javax.swing.*;
import java.awt.event.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgProcessDemo extends GlgJBean implements ActionListener
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   /* Demonstrates updating the drawing using either tags (true) or 
      resources (false).
   */
   boolean UseTags = true;

   /* Demonstrates two ways of processing user clicks on objects: processing 
      actions attached to objects in the input callback (true), or using simple 
      selection via object names in the selection callback (false).
   */
   boolean UseActions = false;

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
     IsReady = false,
     ShowDynamics = true,
     ShowFlow = true,
     Show3D = true,             // Initial setting
     AntiAliasing = true,
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

   Timer timer = null;

   //////////////////////////////////////////////////////////////////////////
   // For use as a stand-alone java demo
   //////////////////////////////////////////////////////////////////////////
   public static void main( final String [] arg )
   {
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Main( arg ); } } );
   }

   //////////////////////////////////////////////////////////////////////////
   public static void Main( final String [] arg )
   {
      class DemoQuit extends WindowAdapter
      {
         public void windowClosing( WindowEvent e ) { System.exit( 0 ); }
      } 

      JFrame frame = new JFrame( "GLG Process Control Monitoring Demo" );

      frame.setResizable( true );
      frame.setSize( 700, 600 );
      frame.setLocation( 20, 20 );

      GlgProcessDemo process = new GlgProcessDemo();      

      frame.getContentPane().add( process );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );

      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers ReadyCallback which starts updates.
      //
      process.SetDrawingName( "process.g" );
   }

   //////////////////////////////////////////////////////////////////////////
   public GlgProcessDemo()
   {
      super();
      SetDResource( "$config/GlgAntiAliasing", AntiAliasing ? 1.0 : 0.0 );

      // Disable automatic update for input events to avoid slowing down 
      // real-time chart updates.
      SetAutoUpdateOnInput( false );
   }

   //////////////////////////////////////////////////////////////////////////
   // Starts updates
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );

      if( timer == null )
      {
         // Restart the timer after each update (instead of using repeats)
         // to avoid flooding the event queue with timer events on slow 
         // machines.
         timer = new Timer( UpdateInterval, (ActionListener) this );
         timer.setRepeats( false );
         timer.start();
      }

      IsReady = true;
   }

   //////////////////////////////////////////////////////////////////////////
   // Recalculates new values for the process using a simulation model and
   // updates display with the new values.
   //////////////////////////////////////////////////////////////////////////
   public void IterateProcess()
   {
      if( timer == null )
        return;   // Prevents race conditions

      if( IsReady && ShowDynamics )
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
         
         UpdateProcess();    // Updates display with the new values.
      }

      timer.start();   // Restart the update timer
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

      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates drawing using resources.
   //////////////////////////////////////////////////////////////////////////
   public void UpdateProcessTags()
   {
      SetDTag( "SolventValveValue", SolventValve, true );
      SetDTag( "SteamValveValue", SteamValve, true );
      SetDTag( "CoolingValveValue", CoolingValve, true );
      SetDTag( "WaterValveValue", WaterValve, true );
      
      if( ShowFlow )
      {
         SetDTag( "SolventFlow", GetFlow( SOLVENT_FLOW ), true );
         SetDTag( "SteamFlow", GetFlow( STEAM_FLOW ), true );
         SetDTag( "CoolingFlow", GetFlow( COOLING_FLOW ), true );
         SetDTag( "WaterFlow", GetFlow( WATER_FLOW ), true );
 
         SetDTag( "OutFlow", OutFlow, true );
      }

      SetDTag( "SteamTemperature", SteamTemperature, true );
      SetDTag( "HeaterTemperature", HeaterTemperature, true );
      SetDTag( "BeforePreHeaterTemperature", BeforePreHeaterTemperature, true );
      SetDTag( "PreHeaterTemperature", PreHeaterTemperature, true );
      SetDTag( "AfterPreHeaterTemperature", AfterPreHeaterTemperature, true );
      SetDTag( "CoolingTemperature", CoolingTemperature, true );
      
      SetDTag( "HeaterLevel", HeaterLevel, true );
      SetDTag( "WaterLevel", WaterLevel, true );
      
      SetDTag( "HeaterAlarm", HeaterAlarm ? 1.0 : 0.0, true );
      SetDTag( "WaterAlarm", WaterAlarm ? 1.0 : 0.0, true );
      
      /* Pass if_changed=false to move the chart even if the value did not 
         change. The rest of resources use true to update them only if their 
         values changed.
      */
      SetDTag( "PlotValueEntryPoint", HeaterTemperature, false );
      
      SetDTag( "PressureValue", 5.0 * HeaterPressure, true );
   }   
   
   //////////////////////////////////////////////////////////////////////////
   // Updates drawing using resources.
   //////////////////////////////////////////////////////////////////////////
   public void UpdateProcessResources()
   {
      SetDResource( "SolventValve/Value", SolventValve, true );
      SetDResource( "SteamValve/Value", SteamValve, true );
      SetDResource( "CoolingValve/Value", CoolingValve, true );
      SetDResource( "WaterValve/Value", WaterValve, true );
      
      if( ShowFlow )
      {
         SetDResource( "SolventFlow", GetFlow( SOLVENT_FLOW ), true );
         SetDResource( "SteamFlow", GetFlow( STEAM_FLOW ), true );
         SetDResource( "CoolingFlow", GetFlow( COOLING_FLOW ), true );
         SetDResource( "WaterFlow", GetFlow( WATER_FLOW ), true );
 
         SetDResource( "OutFlow", OutFlow, true );
      }

      SetDResource( "Heater/SteamTemperature", SteamTemperature, true );
      SetDResource( "Heater/HeaterTemperature", HeaterTemperature, true );
      SetDResource( "BeforePreHeaterTemperature", BeforePreHeaterTemperature, 
                    true );
      SetDResource( "PreHeaterTemperature", PreHeaterTemperature, true );
      SetDResource( "AfterPreHeaterTemperature", AfterPreHeaterTemperature, 
                    true );
      SetDResource( "CoolingTemperature", CoolingTemperature, true );
      
      SetDResource( "Heater/HeaterLevel", HeaterLevel, true );
      SetDResource( "WaterSeparator/WaterLevel", WaterLevel, true );
      
      SetDResource( "HeaterAlarm", HeaterAlarm ? 1.0 : 0.0, true );
      SetDResource( "WaterAlarm", WaterAlarm ? 1.0 : 0.0, true );
      
      /* Pass if_changed=false to move the chart even if the value did not 
         change. The rest of resources use true to update them only if their 
         values changed.
      */
      SetDResource( "ChartVP/Chart/Plots/Plot#0/ValueEntryPoint", 
                    HeaterTemperature, false );
      
      SetDResource( "PressureGauge/Value", 5.0 * HeaterPressure, true );
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Handle user interaction with the buttons.
   //////////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject vp, GlgObject message_obj )
   {
      String
        origin,
        format,
        action;

      super.InputCallback( vp, message_obj );

      origin = message_obj.GetSResource( "Origin" );
      format = message_obj.GetSResource( "Format" );
      action = message_obj.GetSResource( "Action" );
      // String subaction = message_obj.GetSResource( "SubAction" );

      if( format.equals( "Button" ) )
      {	 
         if( !action.equals( "ValueChanged" ) )
           return;
         
         boolean pipe_vis = ( GetDResource( "3DPipesToggle/OnState" ) != 0.0 );
         boolean flow_vis = ( GetDResource( "FlowToggle/OnState" ) != 0.0 );

         if( origin.equals( "3DPipesToggle" ) )
         {
            // Making sure either pipes or flow is visible.
            if( !pipe_vis && !flow_vis )
            {
               SetDResource( "FlowToggle/OnState", 1.0 );
               Update();
            }
         }
         else if( origin.equals( "FlowToggle" ) )
         {
            // Making sure either pipes or flow is visible.
            if( !pipe_vis && !flow_vis )
            {
               SetDResource( "3DPipesToggle/OnState", 1.0 );
               Update();
            }
         }
         else if( origin.equals( "ToggleAutoScroll" ) )
         {
            /* Activate chart's X pan slider when AutoScroll=OFF.
               The toggle is connected to the chart's AutoScroll and controls 
               it. The X pan slider is activated here.
            */
            boolean auto_scroll = 
              ( GetDResource( "ChartVP/Chart/AutoScroll" ) != 0.0 );
            SetDResource( "ChartVP/Pan", (double) 
                          ( auto_scroll ? GlgObject.PAN_Y_AUTO : 
                            GlgObject.PAN_X | GlgObject.PAN_Y ) );

            if( auto_scroll )
            {
               /* Reset the chart's ranges when returning to auto-scroll. */
               SetDResource( "ChartVP/Chart/Plots/Plot#0/YLow", 50.0 );
               SetDResource( "ChartVP/Chart/Plots/Plot#0/YHigh", 150.0 );
            }
            Update();
         }
      }

      else if( format.equals( "CustomEvent" ) )
      {
         if( !UseActions )
           return;     /* User clicks on objects are processed using simple 
                          selection via object names in the Select callback. */
         
         /* Handle custom event actions attached to valves to open or close 
            them when the user clicks on them with the left or right mouse 
            button.
         */

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
            SetDResource( "SolventValve/Value", SolventValve );
         }
         else if( event_label.equals( "SteamValveClick" ) )
         {
            SteamValve += 0.2 * increment;
            SteamValve = PutInRange( SteamValve, 0.0, 1.0 );
            SetDResource( "SteamValve/Value", SteamValve );
         }
         else if( event_label.equals( "CoolingValveClick" ) )
         {
            CoolingValve += 0.2 * increment;
            CoolingValve = PutInRange( CoolingValve, 0.0, 1.0 );
            SetDResource( "CoolingValve/Value", CoolingValve );
         }
         else if( event_label.equals( "WaterValveClick" ) )
         {
            WaterValve += 0.2 * increment;
            WaterValve = PutInRange( WaterValve, 0.0, 1.0 );
            SetDResource( "WaterValve/Value", WaterValve );
         }
         /* Erase or display the pressure gauge when the gauge or the heater
            are clicked on.
         */
         else if( event_label.equals( "HeaterClick" ) ||
                  event_label.equals( "PressureGaugeClick" ) )
         {
            int visibility = (int) GetDResource( "PressureGauge/Visibility" );
            SetDResource( "PressureGauge/Visibility",
                                   visibility == 0 ? 1.0 : 0.0 );
         }
         Update();
      }

      else if( format.equals( "Timer" ) )   // Handles timer transformations.
        Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Selection callback to handle mouse selection: open or close valves on 
   // a mouse click depending on the used mouse button.
   // Actions can be added to objects for more elaborate selection handling.
   //////////////////////////////////////////////////////////////////////////
   public void SelectCallback( GlgObject vp, Object[] name_array, int button )
   {
      double
        visibility,
        increment;
      String name;

      super.SelectCallback( vp, name_array, button );

      if( UseActions )
        return;   /* Actions attached to objects are used to process 
                     user clicks in the Input callback. */

      // Process user clicks on objects using simple selection via object names.
      if( name_array == null )
        return;

      if( button == 1 )
        increment = 1.0;
      else
        increment = -1.0;

      for( int i=0; ( name = (String) name_array[i] ) != null; ++i )
      {
         if( name.equals( "SolventValve" ) )
         { 
            SolventValve += 0.2 * increment;
            SolventValve = PutInRange( SolventValve, 0.0, 1.0 );
            SetDResource( "SolventValve/Value", SolventValve );
            break;
         }
         else if( name.equals( "SteamValve" ) )
         {
            SteamValve += 0.2 * increment;
            SteamValve = PutInRange( SteamValve, 0.0, 1.0 );
            SetDResource( "SteamValve/Value", SteamValve );
            break;
         }
         else if( name.equals( "CoolingValve" ) )
         {
            CoolingValve += 0.2 * increment;
            CoolingValve = PutInRange( CoolingValve, 0.0, 1.0 );
            SetDResource( "CoolingValve/Value", CoolingValve );
            break;
         }
         else if( name.equals( "WaterValve" ) )
         {
            WaterValve += 0.2 * increment;
            WaterValve = PutInRange( WaterValve, 0.0, 1.0 );
            SetDResource( "WaterValve/Value", WaterValve );
            break;
         }
         else if( name.indexOf( "Heater" ) == 0 ||
                 name.indexOf( "PressureGauge" ) == 0 )
         {
            visibility = GetDResource( "PressureGauge/Visibility" );
            SetDResource( "PressureGauge/Visibility",
                         visibility == 0.0 ? 1.0 : 0.0 );
            break;
         }
      }
      Update();
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
   public void StopUpdates()
   {
      if( timer != null )
      {
         timer.stop();
         timer = null;
      }
   }


   //////////////////////////////////////////////////////////////////////////
   // ActionListener method to use the bean as update timer's ActionListener.
   //////////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      IterateProcess();
   }

   //////////////////////////////////////////////////////////////////////////
   public void ToggleDynamics()
   {
      ShowDynamics = !ShowDynamics;
   }

   //////////////////////////////////////////////////////////////////////////
   public void ToggleDrawing()
   {
      if( !IsReady )
        return;

      Show3D = !Show3D;

      ReloadDrawing();
   }

   //////////////////////////////////////////////////////////////////////////
   public void ToggleAntiAliasing()
   {
      if( !IsReady )
        return;

      AntiAliasing = !AntiAliasing;
      SetDResource( "$config/GlgAntiAliasing", AntiAliasing ? 1.0 : 0.0 );

      // Reload with new AntiAliasing setting
      ReloadDrawing();
   }

   //////////////////////////////////////////////////////////////////////////
   void ReloadDrawing()
   { 
      StopUpdates();
      IsReady = false;

      ShowDynamics = true;

      // Unset previous drawing to force re-loading even if the drawing is the
        // same.
      SetDrawingName( null );

      SetDrawingName( Show3D ? "process.g" : "process2D.g" );
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked by the browser to stop the applet
   //////////////////////////////////////////////////////////////////////////
   public void stop()
   {
      StopUpdates();
      IsReady = false;
      super.stop();
   }

   //////////////////////////////////////////////////////////////////////////
   // Inner class for a Runnable interface.
   // Provides an interface between JavaScript and Java: invokes applet's 
   // methods in a synchronous way as required by Swing.
   //////////////////////////////////////////////////////////////////////////
   class GlgBeanRunnable implements Runnable
   {
      GlgProcessDemo bean;
      String request_name;
      int value;

      public GlgBeanRunnable( GlgProcessDemo bean_p, 
                             String request_name_p, int value_p )
      {
         bean = bean_p;
         request_name = request_name_p;
         value = value_p;
      }

      public void run()
      {
         if( request_name.equals( "ToggleDynamics" ) )
           bean.ToggleDynamics();
         else if( request_name.equals( "ToggleDrawing" ) )
           bean.ToggleDrawing();
         else if( request_name.equals( "ToggleAntiAliasing" ) )
           bean.ToggleAntiAliasing();
         else
           PrintToJavaConsole( "Invalid request name: " + 
                              request_name + "\n" );
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Provides an interface between JavaScript and Java: invokes applet's 
   // methods in a synchronous way as required by Swing.
   //////////////////////////////////////////////////////////////////////////
   public void SendRequest( String request_name, int value )
   {
      GlgBeanRunnable runnable = 
        new GlgBeanRunnable( this, request_name, value );

      SwingUtilities.invokeLater( runnable );
   }
}
