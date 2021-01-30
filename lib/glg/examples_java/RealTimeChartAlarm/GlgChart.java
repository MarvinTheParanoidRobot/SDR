/***************************************************************************
  This example demonstrates how to display and update a GLG realtime
  stripchart widget and use its integrated interactive behavior.
  The example also shows how to handle alarms using a global Alarm handler.

  The example displays a drawing "chart_with_alarm.g" created in the 
  GLG Builder. The drawing includes:

    - the Toolbar viewport, containing some interface widgets, as well as
      two alarm indicators;

    - the ChartViewport,  containing the Chart object which plots
      the chart data;

    - ValueEntryPoint attribute of each plot (Plot#0/ValueEntryPoint and
      Plot#1/ValueEntryPoint) has an Alarm object attached to it in the
      Builder. High, Low, HighHigh and LowLow resources of the alarm
      object define "soft" and "hard" limits, and can be changed in 
      the application code.  
  
  GlgChart class is derived from GlgJBean and encapsulates methods
  for initializing the chart, updating the chart with data and handling
  user interaction. 

  Alarm messages are handled using a global Alarm handler. 
  
  The chart is initilaized in the H and V callbacks and updated with 
  simulated data using a timer. 

  The X axis labels display current date and time using the time format
  defined in the drawing. Data points in the chart are positioned according
  to their time stamp. An application may provide a time stamp for each 
  data point, otherwise the chart will automatically use current time for 
  the time stamp.

  This example is written using GLG Standard API. 

 ***************************************************************************/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.lang.reflect.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgChart extends GlgJBean implements ActionListener, GlgAlarmHandler
{
   static final long serialVersionUID = 0;
   static final String PLOT_RES_NAME = "ChartViewport/Chart/Plots/Plot#";

   final double TIME_SPAN = 60.;          // Time Span in sec.
   final double SCROLL_INCREMENT = 10.;   // Scroll increment in sec.

   // If AUTO_CLEAR_YELLOW is true, the yellow indicator state will be 
   // automatically reset to green when the data value returns back 
   // within the limits. If AUTO_CLEAR_YELLOW is false, the indicator 
   // state will be locked until the user clicks on the Clear button.
   boolean AUTO_CLEAR_YELLOW = false;

   // Alarm labels must start with "Alarm", followed by the alarm index. */
   String ALARM_LABEL_PREFIX = "Alarm";

    // Scroll factor for the X axis. 
   final double ScrollFactor = SCROLL_INCREMENT / TIME_SPAN;
   
   DataFeedInterface DataFeed;        // Used for supplying chart data.
   Timer timer = null;

   // Number of lines in a chart as specified in the drawing file.
   public int NumPlots;      
   
   // Low and High range of the incoming data values. 
   // May be defined by the parent object.
   public double YLow = 0.;
   public double YHigh = 10.;

   // Level lines. May be defined by the parent object.		    
   public double LowHard= 1.;
   public double LowSoft = 2.;
   public double HighHard = 9.;
   public double HighSoft = 8.;

   public double TimeSpan = TIME_SPAN; // Time axis span in sec.

   public int UpdateInterval = 2000;   // Update interval in msec.
   public int BufferSize = 5000;      /* Number of samples in the history 
                                         buffer per line. */
   boolean PrefillData;        /* Setting to False suppresses pre-filling the 
                                  chart's buffer with data on start-up. */
   int AutoScroll = 1;         /* Current auto-scroll state: enabled(1) or 
                                  disabled(0). */
   int StoredScrollState;      /* Stored AutoScroll state to be restored 
                                  if ZoomTo is aborted. */

   // Store object IDs for each plot. 
   // Used for performance optimization in the chart data feed.
   GlgObject Plots[];

   // Number of plots as defined in the drawing.
   int num_plots_drawing;

   boolean IsReady = false;
   
   // Used by DataFeed to return data values.
   DataPoint data_point = new DataPoint();
    
   /////////////////////////////////////////////////////////////////////
   public GlgChart( boolean prefill_data )
   {
      // Activate Trace callback. In this example, it is used
      // to activate chart scrolling by dragging it with the mouse.
      AddListener( GlgObject.TRACE_CB, this );

      // Defines whether to prefill chart's history buffer with
      // data before the chart starts receiving new data.
      PrefillData = prefill_data;

      // Add DataFeed object used to supply chart data.
      // The example uses demo data. To supply application specific
      // data, replace DemoDataFeed with a custom DataFeed object.
      AddDataFeed( new DemoDataFeed( this ) );

      // Disable automatic update for input events to avoid slowing down 
      // real-time chart updates.
      SetAutoUpdateOnInput( false );
   }
   
   /////////////////////////////////////////////////////////////////////
   // Add DataFeed object for supplying chart data.
   /////////////////////////////////////////////////////////////////////
   public void AddDataFeed( DataFeedInterface data_feed )
   {
      DataFeed = data_feed;
   }
   
   //////////////////////////////////////////////////////////////////////
   public void LoadDrawing()
   {
      SetDrawingName( "chart_with_alarm.g" );
   }

   //////////////////////////////////////////////////////////////////////
   // StartUpdates() creates a timer to perform periodic updates.
   // The timer invokes the bean's UpdateGraphProc() method to update
   // drawing's resoures with new data values.
   /////////////////////////////////////////////////////////////////////
   public void StartUpdates( int interval )
   {
      if( interval !=0 )
        UpdateInterval = interval;

      if( timer == null )
      {
         timer = new Timer( UpdateInterval, this );
         timer.setRepeats( true );
         timer.start();
      }
   }

   ///////////////////////////////////////////////////////////////////////
   // StopUpdate() method stops periodic updates
   ///////////////////////////////////////////////////////////////////////
   public void StopUpdates()
   {
      if( timer != null )
      {
         timer.stop();
         timer = null;
      }

      // Disable dynamic updates of the drawing
      IsReady = false;
   }

   ////////////////////////////////////////////////////////////////////////
   // HCallback is invoked after the Glg drawing is loaded, but before the 
   // hierarchy setup takes place and before the drawing is drawn for the 
   // first time. 
   ////////////////////////////////////////////////////////////////////////
   public void HCallback( GlgObject viewport )
   {
      double major_interval, minor_interval;

      // Retreive number of plots defined in .g file.
      NumPlots = (int) GetDResource( "ChartViewport/Chart/NumPlots" );
 
      // Set Time Span for the X axis.
      SetDResource( "ChartViewport/Chart/XAxis/Span", TimeSpan );
      
      // Set tick intervals for the Time axis.
      // Use positive values for absolute time interval, for example
      // set major_interval = 10 for a major tick every 10 sec.
      //
      major_interval = -6;      // 6 major intervals
      minor_interval = -5;      // 5 minor intervals
      SetDResource( "ChartViewport/Chart/XAxis/MajorInterval", 
                    major_interval );
      SetDResource( "ChartViewport/Chart/XAxis/MinorInterval", 
                    minor_interval );
      
      // Set data value range. Since the graph has one Y axis and
      // common data range for the plots, Low/High data range is
      // set on the YAxis level.
      //
      SetDResource( "ChartViewport/Chart/YAxis/Low", YLow );
      SetDResource( "ChartViewport/Chart/YAxis/High", YHigh );

      // Set value limits for the Level lines.
      SetDResource( "ChartViewport/Chart/Levels/LowHard", LowHard );      
      SetDResource( "ChartViewport/Chart/Levels/LowSoft", LowSoft );      
      SetDResource( "ChartViewport/Chart/Levels/HighHard", HighHard );      
      SetDResource( "ChartViewport/Chart/Levels/HighSoft", HighSoft );      

      // Set the initial value of all plots to be within LowSoft/HighSoft limits,
      // to make sure the Alarm is not raised from the initial value stored
      // in the .g file. 
      //
      double initial_value =  (HighSoft + LowSoft ) / 2.;
      SetDResource( "ChartViewport/Chart/Plots/Plot#%/ValueEntryPoint", 
                    initial_value );  

      // Enable AutoScroll, both for the toggle button and the chart.
      ChangeAutoScroll( 1 );

      // Set Chart Zoom mode. It was set and saved with the drawing, 
      // but do it again programmatically just in case.
      //
      viewport.SetZoomMode( "ChartViewport", null, "ChartViewport/Chart", 
                            GlgObject.CHART_ZOOM_MODE );

      // Install global Alarm Handler.
      viewport.SetAlarmHandler( this );
   }
   
   //////////////////////////////////////////////////////////////////////
   // VCallback() is invoked after the drawing is loaded and setup, 
   // but before it is drawn for the first time. 
   /////////////////////////////////////////////////////////////////////
   public void VCallback( GlgObject viewport )
   {
      // Store objects IDs for each plot.
      Plots = new GlgObject[ NumPlots ];
      for( int i=0; i<NumPlots; ++i )
      {
         Plots[i] = viewport.GetNamedPlot( "ChartViewport/Chart", 
                                           "Plot#" + i ); 
      }
            
      // Prefill chart's history bufer with data. 
      if( PrefillData )
        PreFillChartData();
   }
   
   ////////////////////////////////////////////////////////////////////////
   // ReadyCallback is invoked after the drawing is loaded, setup and 
   // initially drawn.
   ////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      IsReady = true;
   }

   ///////////////////////////////////////////////////////////////////////
   // timer's ActionListener method to be invoked periodically 
   //////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateGraphProc();
   }

   ///////////////////////////////////////////////////////////////////////
   // Global Alarm handler. 
   //////////////////////////////////////////////////////////////////////
   public void Alarm( GlgObject data_object, GlgObject alarm_object, 
                      String alarm_label, String action,
                      String subaction, Object reserved )
   {
      boolean DEBUG_ALARM = false; //set to true print alarm details.
      
      if ( DEBUG_ALARM )
      {
         String data_obj_name = data_object.GetSResource( "Name" );
         double data_value = data_object.GetDResource( null ).doubleValue();
            
         System.out.println( "Alarm label=" + alarm_label + 
                             " action=" + action + 
                             " subaction=" + subaction );

         System.out.println( "data_obj=" + data_object );
         System.out.println( "data_obj_name=" + data_obj_name );
         System.out.println( "data_value=" + data_value );
      }

      // If alarm_label doesn't start with "Alarm", generate an error
      // and return from function.
      if( !alarm_label.startsWith( ALARM_LABEL_PREFIX ) )
      {
         error( "Invalid Alarm label", false );
         return;
      }

      if( action.equals( "Set" ) || action.equals( "On" ) ) 
      {
         if( subaction.equals( "LowLow" ) || subaction.equals( "HighHigh" ) )
         {
            // Data value is outside of the "hard" limits,  
            // turn the inidcator red (indicator value = 2.0).
            UpdateAlarmIndicator( alarm_label, 2. );
         }
         else if( subaction.equals( "Low" ) || subaction.equals( "High" ) )
         {
            // Data value is outside of the "soft" limits, but within the
            // "hard" limits; turn the inidcator yellow (indicator value = 1.0).
            UpdateAlarmIndicator( alarm_label, 1. );
         }

         Update();
      }
      else if( action.equals( "Reset" ) ) 
      {
         if(  AUTO_CLEAR_YELLOW )
         {
            // Data value is back within "soft" limits; reset the inidcator 
            // to green (indicator value = 0.0). If the indicator was red, 
            // it is locked by the code in UpdateAlarmIndicator() until 
            // the user clears it.
            //
            UpdateAlarmIndicator( alarm_label, 0. );
            
            Update();
         }
      }
   }

   ///////////////////////////////////////////////////////////////////////
   // Updates the alarm indicator named AlarmIndicator#, where # 
   // corresponds to the index in the provided alarm_label. The indicator is 
   // updated with the provided indicator_value. If the indicator is 
   // already red (current indiator value is 2.0), the indicator is locked 
   // until the user clears it using the Clear button.
   ///////////////////////////////////////////////////////////////////////
   public void UpdateAlarmIndicator( String alarm_label, 
                                     double indicator_value )
   {
      /* Extract the alarm index from the alarm_label. */
      int alarm_index = get_alarm_index( alarm_label );
      if( alarm_index < 0 )
      {
         error( "Invalid alarm label", false );
         return;
      }
      
      String res_name = 
        GlgObject.CreateIndexedName( "Toolbar/AlarmIndicator%/Value", 
                                     alarm_index );
      
      // Update the indicator unless it is red (its current value is 2.0).
      // If it is red, the indicator is locked until it gets cleared 
      // by the user via the Clear button.
      //
      double current_value = GetDResource( res_name );
      
      if( current_value != 2.0 )
        SetDResource( res_name, indicator_value ); 
   }

   ///////////////////////////////////////////////////////////////////////
   // Update procedure to update a graph with random data. This procedure 
   // is invoked periodically by a timer.
   ///////////////////////////////////////////////////////////////////////
   public void UpdateGraphProc()
   {
      // Perform dynamic updates only if the drawing is ready and 
      // the timer is active.
      if( !IsReady )
         return;

      // This example uses demo data. An application will provide a
      // custom DataFeed object for supplying real-time chart data.
      
      // Update plot lines with new data supplied by the DataFeed object.
      for( int i=0; i<NumPlots; ++i )
      {
         // Use DataFeed to get new data value. The DataFeed object
         // fills the data_point object with value, time_stamp, etc.
         DataFeed.GetPlotPoint( i, data_point );

         //String plot_name = PLOT_RES_NAME + i;
         PushPlotPoint( i, data_point );
      }

      Update();
   }

   ///////////////////////////////////////////////////////////////////////
   // Pushes the data_point's data into the plot.
   ///////////////////////////////////////////////////////////////////////
   public void PushPlotPoint( int plot_index, DataPoint data_point )
   {
      // Supply plot value for the chart via ValueEntryPoint.
      Plots[plot_index].SetDResource( "ValueEntryPoint", data_point.value );
                 
      if( data_point.has_time_stamp )
      {
         // Supply an optional time stamp. If not supplied, the chart will 
         // automatically generate a time stamp using current time. 
         //
         Plots[plot_index].SetDResource( "TimeEntryPoint", 
                                         data_point.time_stamp );
      }
      
      if( !data_point.value_valid )
      {	   
         // If the data point is not valid, set ValidEntryPoint resource to 
         // display holes for invalid data points. If the point is valid,
         // it is automatically set to 1. by the chart.
         //
         Plots[plot_index].SetDResource( "ValidEntryPoint", 0. );
      }
   }
      
   /////////////////////////////////////////////////////////////////////// 
   // Pre-fill the graph's history buffer with data. 
   /////////////////////////////////////////////////////////////////////// 
   public void PreFillChartData()
   {
      double 
        current_time, start_time, end_time,
        num_seconds, dt;

      current_time = GetCurrTime();
      
      // Roll back by the amount corresponding to the buffer size.
      dt = UpdateInterval / 1000.;     // Update interval is in millisec.
      
      // Add an extra second to avoid rounding errors.
      num_seconds = BufferSize * dt + 1;
      
      start_time = current_time - num_seconds;
      end_time = 0.;   /* Stop at the current time. */

      for( int i=0; i<NumPlots; ++i )
        DataFeed.FillHistData( i, start_time, end_time, data_point );
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Handle user interaction.
   ///////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject viewport, GlgObject message_obj )
   {
      String
        origin,
        format,
        action,
        subaction;
      
      origin = message_obj.GetSResource( "Origin" );
      format = message_obj.GetSResource( "Format" );
      action = message_obj.GetSResource( "Action" );
      subaction = message_obj.GetSResource( "SubAction" );
      
      // Handle window closing if run stand-alone. 
      if( format.equals( "Window" ) && action.equals( "DeleteWindow" ) )
        System.exit( 0 );
      
      // Process button events.
      if( format.equals( "Button" ) )   
      {
         if( !action.equals( "Activate" ) &&      /* Not a push button */
             !action.equals( "ValueChanged" ) )   /* Not a toggle button */
           return;
         
         AbortZoomTo();
         
         if( origin.equals( "ToggleAutoScroll" ) )
         {         
            // Set Chart AutoScroll based on the 
            // ToggleAutoScroll toggle button setting.
            ChangeAutoScroll( -1 ); 
         }
         else if( origin.equals( "ZoomTo" ) )
         {
            // Start ZoomTo operation.
            SetZoom( "ChartViewport", 't', 0. );  
         }
         else if( origin.equals( "ResetZoom" ) )
         {         
            // Set initial time span and reset initial Y ranges.
            SetChartSpan( TimeSpan );  
            RestoreInitialYRanges();   
         }
         else if( origin.equals( "ScrollBack" ) )
         {
            ChangeAutoScroll( 0 );
            
            // Scroll left. Scrolling can be done by either setting chart's 
            // XAxis/EndValue resource or by using GlgSetZoom().
            
            // double end_value;
            // double end_value = GetDResource( 
            //     "ChartViewport/Chart/XAxis/EndValue" ); 
            // end_value -= SCROLL_INCREMENT;
            // SetDResource( "ChartViewport/Chart/XAxis/EndValue", 
            //               end_value );
            
            SetZoom( "ChartViewport", 'l', ScrollFactor );
         }
         else if( origin.equals( "ScrollForward" ) )
         {
            ChangeAutoScroll( 0 );
            
            // Scroll right.
            SetZoom( "ChartViewport", 'r', ScrollFactor );
         }
         else if( origin.equals( "ScrollToRecent" ) )
         {
            // Scroll to show most recent data.
            ScrollToDataEnd();
         }
         else if( origin.equals( "ClearButton" ) )
         {
            // Clear button from the AlarmIndicator was pressed.
            // Reset the indicator value to 0. 
            message_obj.SetDResource( "Object/Value", 0. );
         }
         
         Update();
      }
      else if( format.equals( "Chart" ) && 
               action.equals( "CrossHairUpdate" ) )
      {
         // To avoid slowing down real-time chart updates, invoke Update() 
         // to redraw cross-hair only if the chart is not updated fast 
         // enough by the timer.
         //
         if( UpdateInterval > 100 )
           Update();
      }            
      else if( action.equals( "Zoom" ) )    // Zoom events
      {
         if( subaction.equals( "ZoomRectangle" ) )
         {
            // Store AutoSCroll state to restore it if ZoomTo is aborted.
            StoredScrollState = AutoScroll;
            
            // Stop scrolling when ZoomTo action is started.
            ChangeAutoScroll( 0 );
         }
         else if( subaction.equals( "End" ) )
         {
            // No additional actions on finishing ZoomTo. The Y scrollbar 
            // appears automatically if needed: it is set to GLG_PAN_Y_AUTO. 
            // Don't resume scrolling: it'll scroll too fast since we zoomed 
            // in. Keep it still to allow inspecting zoomed data.
         }
         else if( subaction.equals( "Abort" ) )
         {
            // Resume scrolling if it was on.
            ChangeAutoScroll( StoredScrollState ); 
         }
         
         Update();
      }
      else if( action.equals( "Pan" ) )    // Pan events
      {
         // This code may be used to perform custom action when dragging the 
         // chart's data with the mouse. 
         if( subaction.equals("Start" ) )   // Chart dragging start
         {
         }
         else if( subaction.equals( "Drag" ) )    // Dragging
         {
         }
         else if( subaction.equals( "ValueChanged" ) )   // Scrollbars
         {
         }
         /* Dragging ended or aborted. */
         else if( subaction.equals( "End" ) || 
                  subaction.equals( "Abort" ) )
         {
         }     
      }   
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Scroll to the end of the data history buffer.
   ///////////////////////////////////////////////////////////////////////
   public void ScrollToDataEnd()
   {
      GlgMinMax min_max = GetViewport().GetDataExtent( "ChartViewport/Chart", 
                                         true /* x extent */ );
      if( min_max == null )
        return;
      
      SetDResource( "ChartViewport/Chart/XAxis/EndValue", min_max.max );
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Change chart's AutoScroll mode.
   ///////////////////////////////////////////////////////////////////////
   public void ChangeAutoScroll( int new_value )
   {
      double auto_scroll;
      int pan_x;
      
      if( new_value == -1 )  // Use the state of the ToggleAutoScroll button.
      {
         auto_scroll = GetDResource( "Toolbar/ToggleAutoScroll/OnState" );
         AutoScroll = (int) auto_scroll;
      }
      else    // Set to the supplied value. 
      {
         AutoScroll = new_value;
         SetDResource( "Toolbar/ToggleAutoScroll/OnState", 
                       (double) AutoScroll );
      }
      
      // Set chart's auto-scroll.
      SetDResource( "ChartViewport/Chart/AutoScroll", (double) AutoScroll );
      
      // Activate time scrollbar if AutoScroll is Off. The Y value scrollbar 
      // uses GLG_PAN_Y_AUTO and appears automatically as needed.
      //
      pan_x = ( AutoScroll != 0 ? GlgObject.NO_PAN : GlgObject.PAN_X );
      SetDResource( "ChartViewport/Pan", 
                    (double) ( pan_x | GlgObject.PAN_Y_AUTO ) );
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Changes the time span shown in the graph.
   ///////////////////////////////////////////////////////////////////////
   public void SetChartSpan( double span )
   {
      if( span > 0 )
        SetDResource( "ChartViewport/Chart/XAxis/Span", span );
      else  // Reset span to show all data accumulated in the buffer.
        SetZoom( "ChartViewport", 'N', 0. );
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Restore Y axis range to the initial Low/High values.
   ///////////////////////////////////////////////////////////////////////
   public void RestoreInitialYRanges()
   {
      SetDResource( "ChartViewport/Chart/YAxis/Low",  YLow );
      SetDResource( "ChartViewport/Chart/YAxis/High", YHigh );
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Returns True if the chart's viewport is in ZoomToMode.
   // ZoomToMode is activated on Dragging and ZoomTo operations.
   ///////////////////////////////////////////////////////////////////////
   public boolean ZoomToMode()
   {
      int zoom_mode = (int) GetDResource( "ChartViewport/ZoomToMode" );
      return ( zoom_mode != 0 );
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Abort ZoomTo mode.
   ///////////////////////////////////////////////////////////////////////
   public void AbortZoomTo()
   {
      if( ZoomToMode() )
      {
         // Abort zoom mode in progress.
         SetZoom( "ChartViewport", 'e', 0. ); 
         Update();
      }
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Used to obtain coordinates of the mouse click.
   ///////////////////////////////////////////////////////////////////////
   public void TraceCallback( GlgObject viewport, GlgTraceData trace_info )
   {
      if( !IsReady )
        return;
      
      // Process only events that occur in ChartViewport.
      String event_vp_name = trace_info.viewport.GetSResource( "Name" );

      if( !event_vp_name.equals( "ChartViewport" ) )
        return;
      
      int event_type = trace_info.event.getID();
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
       case MouseEvent.MOUSE_MOVED:
       case MouseEvent.MOUSE_DRAGGED:
         /* The code to get the mouse coordinates if needed:
            x = ((MouseEvent)trace_info.event).getX();
            y = ((MouseEvent)trace_info.event).getY();
         */
         break;
         
       default: return;
      }
      
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
         if( ZoomToMode() )
           return; // ZoomTo or dragging mode in progress.
         
         // Start dragging with the mouse on a mouse click. 
         // If user clicked of an axis, the dragging will be activated in the
         // direction of that axis. If the user clicked in the chart area,
         // dragging in both the time and the Y direction will be activated.

         SetZoom( "ChartViewport", 's', 0. );
         
         // Disable AutoScroll not to interfere with dragging.
         ChangeAutoScroll( 0 ); 
         break;
         
       default: return;
      }
   }
   
   /////////////////////////////////////////////////////////////////////// 
   // Return exact time including fractions of seconds.
   /////////////////////////////////////////////////////////////////////// 
   public double GetCurrTime()
   {
      return System.currentTimeMillis() / 1000.;
   }

   /////////////////////////////////////////////////////////////////////// 
   void error( String string, boolean quit )
   {
      System.out.println( string );
      if( quit )
         System.exit( 0 );
   }

   /////////////////////////////////////////////////////////////////////// 
   // Extract the index number from the alarm label.
   /////////////////////////////////////////////////////////////////////// 
   public int get_alarm_index( String label )
   {
      if( label == null )
        return -1;
      
      int label_size = label.length();
      int prefix_size = ALARM_LABEL_PREFIX.length();
      if( label_size <=0 || label_size <= prefix_size )
        return -1;
      
      String str = label.substring( prefix_size );
      int number = Integer.parseInt( str );
      
      if( number >= 0 )
        return number;
      else
        return -1;
   }

   /////////////////////////////////////////////////////////////////////// 
   // Sample implementation of DataFeed interface.
   // In a real application, data will be coming from an application
   // data source defined as the DataFeed object.
   /////////////////////////////////////////////////////////////////////// 
   public class DemoDataFeed implements DataFeedInterface
   {
      // Used in GetDemoValue() to generate demo data.
      GlgChart glg_chart;
      long counter = 0;

      DemoDataFeed( GlgChart chart ) 
      {
         glg_chart = chart;
      }
      
      ///////////////////////////////////////////////////////////////////////
      // Implements DataFeed interface to supply dynamic data.
      // The example uses simulated data by calling GetDemoValue.
      // An application will provide a custom implemnetation of GetPlotPoint()
      // to supply real-time data.
      ///////////////////////////////////////////////////////////////////////
      public void GetPlotPoint( int plot_index, DataPoint data_point )
      {
         data_point.value = GetDemoValue( plot_index );
         data_point.value_valid = true;
         
         data_point.has_time_stamp = false; // Use current time stamp for demo
         data_point.time_stamp = 0.;
      }
      
      ///////////////////////////////////////////////////////////////////////
      // Implements DataFeed interface to pre-fill the chart's history buffer 
      // with simulated demo data. 
      /////////////////////////////////////////////////////////////////////// 
      public void FillHistData( int plot_index, double start_time, 
                                double end_time, DataPoint data_point )
      {
         // Update interval is in millisec.
         double dt = glg_chart.UpdateInterval / 1000.;   
         
         // When prefilling up to the current time, use the result of 
         // GetCurrTime() as the loop's end condition and check it after
         // each iteration to account for the time it takes to prefill 
         // the chart.
         
         for( double time_stamp = start_time; 
              time_stamp < 
                ( end_time != 0. ? end_time : glg_chart.GetCurrTime() );
              time_stamp += dt )
         {
            data_point.value = GetDemoValue( plot_index );
            data_point.value_valid = true;
            data_point.time_stamp = time_stamp;
            data_point.has_time_stamp = true;
            
            glg_chart.PushPlotPoint( plot_index, data_point );
         }
      }
      
      /////////////////////////////////////////////////////////////////////// 
      // Generates demo data value.
      ///////////////////////////////////////////////////////////////////////
      double GetDemoValue( int plot_index )
      {
         double 
           half_amplitude, center,
           period,
           value,
           alpha;
         
         half_amplitude = ( HighHard - LowHard ) / 2.;
         center = LowHard + half_amplitude;
         
         switch( plot_index )
         {
          default:
          case 0: period = 300.; break;
          case 1: period = 600.; break;
         }

         alpha = 2. * Math.PI * counter / period;
         
         value = center + 
           half_amplitude * Math.sin( alpha ) * Math.sin( alpha / 4.5 );
         
         if( value < LowSoft )
           value += AddSpike( value - YLow );
         else if( value > HighSoft )
           value += AddSpike( YHigh - value );

         ++counter;
         return value;
      }
      
      //////////////////////////////////////////////////////////////////////   
      // Adds a random spike
      //////////////////////////////////////////////////////////////////////
      double AddSpike( double max_amplitude )
      {
         double amplitude;
         
         if( GlgObject.Rand( 0., 1000. ) < 950. )    /* Sporadic spikes */
           return 0.;
         
         amplitude = GlgObject.Rand( 0., max_amplitude ); /* Random amplitude */
         
         if( GlgObject.Rand( 0., 10. ) < 5. )
           amplitude *= -1.;   /* Random sign */
         
         return amplitude;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked by the browser asynchronously to stop the applet.
   // This method is provided in case the class is used as an applet;
   // it is not used when it is used as a bean in a parent applet in this 
   // example.
   //////////////////////////////////////////////////////////////////////////
   public void stop()
   {
      StopUpdates();

      // GlgJBean handles asynchronous invocation when used as an applet.
      super.stop();
   }
}
