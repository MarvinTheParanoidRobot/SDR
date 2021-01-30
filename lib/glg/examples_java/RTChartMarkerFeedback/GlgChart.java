/***************************************************************************
  This example demonstrates the following features:
   - how to display and update a GLG realtime stripchart;
   - use the charts integrated interactive behavior, such as scrolling and dragging;
   - show/hide a marker for a data sample selected with a double click. 

  GlgChart class is derived from GlgJBean and encapsulates methods
  for initializing the chart, updating the chart with data and handling
  user interaction. GlgChart displays a drawing "stripchart_example.g" 
  created in the GLG Builder; the drawing includes a viewport named
  ChartViewport as well as interface widgets allowing to scroll and 
  zoom the graph. ChartViewport contains the Chart object which plots
  the chart data.
  
  The chart is initilaized in the H and V callbacks and updated with 
  simulated data using a timer. 

  The X axis labels display current date and time using the time format
  defined in the drawing. Data points in the chart are positioned according
  to their time stamp. An application may provide a time stamp for each 
  data point, otherwise the chart will automatically use current time for 
  the time stamp.

  This example is written using GLG Intermediate API.
 ***************************************************************************/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.lang.reflect.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgChart extends GlgJBean implements ActionListener
{
   static final long serialVersionUID = 0;

   final double TIME_SPAN = 60.;          // Time Span in sec.
   final double SCROLL_INCREMENT = 10.;   // Scroll increment in sec.

   // Scroll factor for the X axis. 
   final double ScrollFactor = SCROLL_INCREMENT / TIME_SPAN;

   DataFeedInterface DataFeed;        // Used for supplying chart data.
   Timer timer = null;
   Timer dbl_click_timer = null;

   // Number of lines in a chart. May be overriden by the parent object.
   public int NumPlots = 3;      
   
   // Low and High range of the incoming data values.
   public double Low = 0.;
   public double High = 10.;
   public double TimeSpan = TIME_SPAN; // Time axis span in sec.

   public int UpdateInterval = 100;   // Update interval in msec.
   public int BufferSize = 5000;      /* Number of samples in the history 
                                         buffer per line. */
   boolean PrefillData;        /* Setting to False suppresses pre-filling the 
                                  chart's buffer with data on start-up. */
   int AutoScroll = 1;         /* Current auto-scroll state: enabled(1) or 
                                  disabled(0). */
   int StoredScrollState;      /* Stored AutoScroll state to be restored 
                                  if ZoomTo is aborted. */

   // Object IDs for ChartViewport and Chart.
   GlgObject ChartVP;
   GlgObject Chart;

   // Store object IDs for each plot. 
   // Used for performance optimization in the chart data feed.
   GlgObject Plots[];

   // Number of plots as defined in the drawing.
   int num_plots_drawing;

   boolean IsReady = false;
   
   // Used by DataFeed to return data values.
   DataPoint data_point = new DataPoint();

   // Selection delta in pixels.
   final double
     dx = 5., 
     dy = 5.; 

   // Flag to differentiate between a single click and a double-click.
   boolean isSingleClick = false;

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
      SetDrawingName( "stripchart_markers.g" );
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

      ChartVP = viewport.GetResourceObject( "ChartViewport" );
      Chart = ChartVP.GetResourceObject( "Chart" );

      // Retreive number of plots defined in .g file.
      num_plots_drawing = (int) Chart.GetDResource( "NumPlots" ).intValue();

      // Set new number of plots.
      Chart.SetDResource( "NumPlots", NumPlots );
 
      // Set Time Span for the X axis.
      Chart.SetDResource( "XAxis/Span", TimeSpan );
      
      // Set tick intervals for the Time axis.
      // Use positive values for absolute time interval, for example
      // set major_interval = 10 for a major tick every 10 sec.
      //
      major_interval = -6;      // 6 major intervals
      minor_interval = -5;      // 5 minor intervals
      Chart.SetDResource( "XAxis/MajorInterval", major_interval );
      Chart.SetDResource( "XAxis/MinorInterval", minor_interval );
      
      // Set data value range. Since the graph has one Y axis and
      // common data range for the plots, Low/High data range is
      // set on the YAxis level.
      //
      Chart.SetDResource( "YAxis/Low", Low );
      Chart.SetDResource( "YAxis/High", High );
      
      // Enable AutoScroll, both for the toggle button and the chart.
      ChangeAutoScroll( 1 );

      // Set Chart Zoom mode. It was set and saved with the drawing, 
      // but do it again programmatically just in case.
      //
      ChartVP.SetZoomMode( null, Chart, null, 
                           GlgObject.CHART_ZOOM_MODE );

      // Set Chart TooltipMode to XY, to match the data sample selection
      // mode on a doble-click in GetDataSample().
      Chart.SetDResource( "TooltipMode", 3. );
   }
   
   //////////////////////////////////////////////////////////////////////
   // VCallback() is invoked after the drawing is loaded and setup, 
   // but before it is drawn for the first time. 
   /////////////////////////////////////////////////////////////////////
   public void VCallback( GlgObject viewport )
   {
      int i;

      // Store objects IDs for each plot.
      Plots = new GlgObject[ NumPlots ];

      GlgObject plot_array = Chart.GetResourceObject( "Plots" );
      for( i=0; i<NumPlots; ++i )
        Plots[i] = (GlgObject) plot_array.GetElement( i );

      // Disable markers for all plots. Markers will be shown or hidden
      // based on user clicks.
      for( i=0; i<NumPlots; ++i )
      {
         Plots[i].SetDResource( "Marker/Visibility", 0. );
      }
      
      // For the existing plots, use color and line annotation setting 
      // from the drawing; initialize new plots using random colors and strings
      // for demo purposes.
      //
      if( num_plots_drawing < NumPlots )
        for( i=num_plots_drawing; i < NumPlots; ++i )
        {
           // Using a random color for a demo.
           Plots[i].SetGResource( "EdgeColor", GlgObject.Rand(0., 1.), 
                                  GlgObject.Rand(0., 1.), 
                                  GlgObject.Rand(0., 1.) );
           Plots[i].SetSResource( "Annotation", "Var" + i );
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
   // timer's ActionListener method to be invoked peridically 
   //////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateGraphProc();
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
            ChartVP.SetZoom( null, 't', 0. );  
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
            // double end_value = Chart.GetDResource( "XAxis/EndValue" ); 
            // end_value -= SCROLL_INCREMENT;
            // Chart.SetDResource( "XAxis/EndValue", end_value );
            
            ChartVP.SetZoom( null, 'l', ScrollFactor );
         }
         else if( origin.equals( "ScrollForward" ) )
         {
            ChangeAutoScroll( 0 );
            
            // Scroll right.
            ChartVP.SetZoom( null, 'r', ScrollFactor );
         }
         else if( origin.equals( "ScrollToRecent" ) )
         {
            // Scroll to show most recent data.
            ScrollToDataEnd();
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
      GlgMinMax min_max = Chart.GetDataExtent( null, true /* x extent */ );
      if( min_max == null )
        return;
      
      Chart.SetDResource( "XAxis/EndValue", min_max.max );
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
      Chart.SetDResource( "AutoScroll", (double) AutoScroll );
      
      // Activate time scrollbar if AutoScroll is Off. The Y value scrollbar 
      // uses GLG_PAN_Y_AUTO and appears automatically as needed.
      //
      pan_x = ( AutoScroll != 0 ? GlgObject.NO_PAN : GlgObject.PAN_X );
      ChartVP.SetDResource( "Pan", (double) ( pan_x | GlgObject.PAN_Y_AUTO ) );
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Changes the time span shown in the graph.
   ///////////////////////////////////////////////////////////////////////
   public void SetChartSpan( double span )
   {
      if( span > 0 )
        Chart.SetDResource( "XAxis/Span", span );
      else  // Reset span to show all data accumulated in the buffer.
        ChartVP.SetZoom( null, 'N', 0. );
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Restore Y axis range to the initial Low/High values.
   ///////////////////////////////////////////////////////////////////////
   public void RestoreInitialYRanges()
   {
      Chart.SetDResource( "YAxis/Low",  Low );
      Chart.SetDResource( "YAxis/High", High );
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Returns True if the chart's viewport is in ZoomToMode.
   // ZoomToMode is activated on Dragging and ZoomTo operations.
   ///////////////////////////////////////////////////////////////////////
   public boolean ZoomToMode()
   {
      int zoom_mode = (int) ChartVP.GetDResource( "ZoomToMode" ).intValue();
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
         ChartVP.SetZoom( null, 'e', 0. ); 
         Update();
      }
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Used to obtain coordinates of the mouse click.
   ///////////////////////////////////////////////////////////////////////
   public void TraceCallback( GlgObject viewport, GlgTraceData trace_info )
   {
      double x, y;
      int button_index;

      if( !IsReady )
        return;
      
      // Process only events that occur in ChartViewport.
      if( trace_info.viewport != ChartVP )
        return;
      
      int event_type = trace_info.event.getID();
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
       case MouseEvent.MOUSE_MOVED:
       case MouseEvent.MOUSE_DRAGGED:
         // Obtain mouse coordinates.
         x = (double) ((MouseEvent)trace_info.event).getX();
         y = (double) ((MouseEvent)trace_info.event).getY();
         break;
         
       default: return;
      }
      
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
         if( ZoomToMode() )
           return; // ZoomTo or dragging mode in progress.

         // Disable AutoScroll not to interfere with dragging or
         // marking data samples.
         //
         ChangeAutoScroll( 0 );

         // Handle left mouse clicks.         
         if( ((MouseEvent)trace_info.event).getButton() == MouseEvent.BUTTON1 ) 
         {
            // Get click count to differentiate between double-click and 
            // single click. 
            if( GetClickCount( x, y ) == 2 )
            {
               // Double-click: show/hide a marker for a selected data sample, 
               // if any.
               MarkDataSample( x, y );
            }
            else // Single click.
            {
               // Start dragging with the mouse on a mouse click. 
               // If the user clicked on an axis, the dragging will 
               // be activated in the direction of that axis. 
               // If the user clicked in the chart area, dragging 
               // in both the time and the Y direction will be activated.
               //
               ChartVP.SetZoom( null, 's', 0. );
            }
            break;
         }
          
       default: return;
      }
   }
   
   /////////////////////////////////////////////////////////////////////// 
   // Show/Hide a marker for the selected data sample at the cursor position.
   /////////////////////////////////////////////////////////////////////// 
   public void MarkDataSample( double x, double y )
   {
      // Obtain the closest data sample at the cursor position.
      GlgDataSample data_sample = GetDataSample( x, y );

      if( data_sample == null )
        return;

       data_sample.marker_vis = 
         ( data_sample.marker_vis == 0. ? /*show*/ 1.f : /*hide*/ 0.f );

      Chart.ChangeObject( null );
      ChartVP.Update();
   }

   /////////////////////////////////////////////////////////////////////// 
   // Obtain a chart data sample at the cursor position. 
   /////////////////////////////////////////////////////////////////////// 
   public GlgDataSample GetDataSample( double x, double y )
   {
      GlgDataSample data_sample;
      int num_samples;

      // Query chart selection at the cursor position.
      GlgObject selection = 
        Chart.CreateChartSelection( null /* query all plots */, 
                                    x, y, dx, dy,
                                    true  /* x/y in screen coords */, 
                                    false /* don't include invalid points */, 
                                    false  /* use smallest xy distance */ );

      if( selection == null )
      {
         error( "No data sample at the cursor.", false );
         return null;    // No valid data sample is selected.
      }

      // Query X/Y values of the selected data sample.
      GlgObject plot = selection.GetResourceObject( "SelectedPlot" );
      String plot_name = plot.GetSResource( "Name" );
      double sample_x_value = selection.GetDResource( "SampleX" ).doubleValue();
      double sample_y_value = selection.GetDResource( "SampleY" ).doubleValue();

      // Debugging info.
      String sample_x_string =  selection.GetSResource( "SampleXString" );
      System.out.println( "Selection: " + plot_name + 
                          " time_string=" + sample_x_string + 
                          " y_value=" + sample_y_value );

      // Obtain a list of data samples from the selected plot.
      GlgObject data_array = plot.GetResourceObject( "Array" );
      
      if( data_array != null && 
          ( num_samples = data_array.GetSize() ) !=0 )
      {
         // Traverse the array to find a data sample with a matching x/y value.
         // data_array is a linked list and should be traversed
         // using SetStart() and Iterate() for efficiency, as opposed to
         // using indexes via GetElement(i);
         //
         data_array.SetStart();
         for( int i=0; i<num_samples; ++i )
         {
            data_sample  = (GlgDataSample) data_array.Iterate();

            // Debugging info.
            // System.out.println( "DataSample info: time = " + data_sample.time + 
              //                  " value =  " + data_sample.value );
            
            if( data_sample.valid && 
                data_sample.time == sample_x_value &&
                data_sample.value == sample_y_value  )
              return data_sample;     // Found matching data sample.
         }
      } 

      return null;    // No matching samples found.
   }

   /////////////////////////////////////////////////////////////////////// 
   // Get click count -- return 2 for a double-click, and 1 for a single
   // click. Use a timer as opposed to an integrated Java method 
   // MouseEvent.getClickCount(), to improve mouse feedback using a
   // custom defined click interval.
   /////////////////////////////////////////////////////////////////////// 
   public int GetClickCount( double x, double y )
   {
      if( isSingleClick )
      { 
         isSingleClick = false; // Reset flag.
         return 2;              // Double click occurred.
      }
      else
      {
         isSingleClick = true;
         
         // Start a timer to identify a double click.
         dbl_click_timer = 
           new Timer( 500 /*interval in msec*/,
                      new ActionListener() 
                      { public void actionPerformed( ActionEvent e ) {	 
                            isSingleClick = false; } 
                      } );
         
         dbl_click_timer.setRepeats( false );
         dbl_click_timer.start();
      }

      return 1; // Single click.
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
         
         half_amplitude = ( High - Low ) / 2.;
         center = Low + half_amplitude;
         
         period = 100. * ( 1. + plot_index * 2. );
         alpha = 2. * Math.PI * counter / period;
         
         value = center + 
           half_amplitude * Math.sin( alpha ) * Math.sin( alpha / 30. );
         
         ++counter;
         return value;
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
