/***********************************************************************
 * Real-Time StripChart Demo.
 * The stripchart uses integrated data filtering for handling real-time
 *  updates of plots with huge number of data samples.
 ***********************************************************************/

import java.awt.event.*;
import javax.swing.*;
import java.lang.reflect.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgRealTimeChart extends GlgJBean implements ActionListener
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   /* Set to true to allow the user to ZoomTo using Button3-Drag-Release. */
   static final boolean ENABLE_ZOOM_TO_ON_BUTTON3 = true;

   static final int DAY = 3600 * 24;    /* Number of seconds in a day */

   static final int NUM_PLOTS  = 3;         /* Number of plot lines in the chart. */
   static final int NUM_Y_AXES = NUM_PLOTS; /* One axis for each plot in this 
                                           demo, may be different. */
   /* Sampling interval for historical data points in seconds. */
   static final double HISTORICAL_DATA_INTERVAL = 60.0;   /* Once a minute */

   static final int REAL_TIME  = 0;     /* Real-time mode: updates graph with 
                                          data using the current time for time 
                                          stamps. */
   static final int HISTORICAL = 1;    /* Historical mode: displays and scrolls
                                          through historical data. */
   static final int CALENDAR   = 2;    /* Calendar mode: displays daily data.*/

   /* Constants for scrolling to the ends of the time range. */
   static final int DONT_CHANGE  = 0;
   static final int MOST_RECENT  = 1;  /* Make the most recent data visible. */
   static final int LEAST_RECENT = 2;  /* Make the least recent data visible.*/

   int UpdateInterval = 30;            /* Update interval in msec */

   boolean IsReady = false;
   Timer timer = null; 

   GlgObject Drawing; 
   GlgObject ChartVP;
   GlgObject Chart;
   GlgObject [] Plot = new GlgObject[ NUM_PLOTS ];
   GlgObject [] YAxis = new GlgObject[ NUM_Y_AXES ];
   int BufferSize = 50000;     /* Number of samples to keep in the buffer for 
                                  each line. */
   boolean PrefillData = true; /* Setting to false suppresses pre-filling the 
                                  chart's buffer with data on start-up in the 
                                  REAL_TIME mode. */

   /* Variables used to keep current state. */
   int TimeSpan = 0;           /* The currently displayed span in seconds. */
   int StoredScrollState;      /* Stored AutoScroll state to be restored if 
                                  ZoomTo is aborted. */
   int AutoScroll = 1;         /* Current auto-scroll state: enabled (1) or 
                                  disabled (0). */
   int Mode = REAL_TIME;       /* Current mode: real-time, historical or 
                                  calendar. */
   int SpanIndex = 1;          /* Index of the currently displayed time span.*/
   int YAxisLabelType = 0;     /* Used to demonstrate diff. Y axis labels. */ 

   /* Variables that keep state information used to generate simulated 
      data for the demo. */
   int [] PlotCounter = null;
   boolean Plot0Valid = true;

   /* Stores initial range values, used to restore after zooming. */
   double [] Min = new double[ NUM_PLOTS ];
   double [] Max = new double[ NUM_PLOTS ];

   static GlgObject SelectedPlot = null;
   static boolean StopAutoScroll = false;

   /* Is used to hold and pass around all information about one data point. */
   class DataPoint
   {
      double value;
      boolean value_valid;
      double time_stamp;
      boolean has_time_stamp;
      boolean has_marker;
   }

   DataPoint data_point = new DataPoint();

   //////////////////////////////////////////////////////////////////////////
   // For use as a stand-alone java demo
   //
   // Command-line options:
   //   -real-time (default)  - start in real-time mode
   //   -historical           - start in historical data mode
   //   -calendar             - start in calendar data mode
   //   -buffer-size <N>      - buffer size to use (default 100000)
   //   -dont-prefill-data    - suppress pre-filling buffer with data on 
   //                           start-up for REAL_TIME mode 
   //                           (default is to pre-fill).
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

      JFrame frame = new JFrame( "GLG Real-Time Strip-Chart Demo" );

      frame.setResizable( true );
      frame.setSize( 800, 600 );
      frame.setLocation( 20, 20 );

      GlgRealTimeChart chart = new GlgRealTimeChart();

      frame.getContentPane().add( chart );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );

      chart.SetDResource( "$config/GlgMouseTooltipTimeout", 0.25 );

      // Handle command-line options.
      int num_arg = Array.getLength( arg );
      if( num_arg != 0 )
      {
         for( int skip = 0; skip < num_arg; ++skip )
         {
            if( arg[ skip ].equals( "-real-time" ) )
              chart.Mode = REAL_TIME;
            else if( arg[ skip ].equals( "-historical" ) )
              chart.Mode = HISTORICAL;
            else if( arg[ skip ].equals( "-calendar" ) )
              chart.Mode = CALENDAR;
            else if( arg[ skip ].equals( "-buffer-size" ) )
            {
               ++skip;
               if( num_arg <= skip )
                 chart.error( "Missing buffer size.", true );

               try
               {
                  chart.BufferSize = Integer.parseInt( arg[ skip ] );
               }
               catch( NumberFormatException e )
               {
                  chart.error( "Invalid buffer size.", true );
               }

               System.out.println( "Using buffer_size= " + chart.BufferSize );
            }
            else if( arg[ skip ].equals( "-dont-prefill-data" ) )
              chart.PrefillData = false; 
            else if( arg[ skip ].equals( "-help" ) )
            {
               System.out.println( "Options: [-help] [-real-time|-historical|-calendar] [-buffer-size <number>] [-dont-prefill-data]" );
               System.out.println( "Defaults: -buffer-size " + 
                                   chart.BufferSize );
               System.exit( 0 );
            }
            else
            {
               System.out.println ( "Invalid option. Use -help for the list of options." );
               System.exit( 1 );
            }              
         }
      }

      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers ReadyCallback which starts updates.
      //
      chart.SetDrawingName( "stripchart.g" );
   }

   //////////////////////////////////////////////////////////////////////////
   public GlgRealTimeChart()
   {
      super();

      /* Change ZoomTo button from 1 to 3 if needed. */
      if( ENABLE_ZOOM_TO_ON_BUTTON3 )
        SetDResource( "$config/GlgZoomToButton", 3.0 );

      /* Activate trace callback: used to start chart scrolling by dragging
         it with the mouse.
      */
      AddListener( GlgObject.TRACE_CB, this );
      AddListener( GlgObject.TRACE2_CB, this );

      // Disable not used old-style select callback.
      SelectEnabled = false;

      /* Disable automatic update for input events to avoid slowing down 
         real-time chart updates. 
      */
      SetAutoUpdateOnInput( false );
   }

   //////////////////////////////////////////////////////////////////////////
   // Initializes the chart and starts updates.
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );

      /* Create and the update timer to update the chart with data.
         It will be started for REAL_TIME mode by the SetMode() method.
      */
      if( timer == null )
      {
         // Restart the timer after each update (instead of using repeats)
         // to avoid flooding the event queue with timer events on slow 
         // machines.
         timer = new Timer( UpdateInterval / 2, this );
         timer.setRepeats( false );
      }

      Drawing = GetViewport();
      ChartVP = Drawing.GetResourceObject( "ChartViewport" );
      Chart = ChartVP.GetResourceObject( "Chart" );

      InitChart(); 

      /* Sets initial mode: real-time, historical or calendar. */
      SetMode( Mode );

      IsReady = true;
   }

   //////////////////////////////////////////////////////////////////////////
   // Initializes the drawing and the chart.
   //////////////////////////////////////////////////////////////////////////
   void InitChart()
   {
      int i;
            
      /* Set the requested buffer size. */
      Chart.SetDResource( "BufferSize", (double) BufferSize );

      /* Increase the number of plots and Y axes if the matching number of 
         them are not already defined in the chart's drawing. 
      */
      Chart.SetDResource( "NumPlots", (double) NUM_PLOTS );
      Chart.SetDResource( "NumYAxes", (double) NUM_Y_AXES );
      Chart.SetupHierarchy();

      /* Using an Intermediate API to store plot IDs in an array for convenient
         access. To query plot IDs with the Standard API, use GlgGetNamedPlot()
         in conjunction with GlgCreateIndexedName().
      */
      GlgObject plot_array = Chart.GetResourceObject( "Plots" );
      for( i=0; i<NUM_PLOTS; ++i )
        Plot[i] = (GlgObject) plot_array.GetElement( i );

      /* Store Y axes in an array for convenient access using an Intermediate 
         API. Alternatively, Y axes' resources can be accessed by their 
         resource names via the Standard API, for example: 
         "ChartVP/Chart/YAxisGroup/YAxis#0/Low"
      */
      GlgObject y_axis_array = Chart.GetResourceObject( "YAxisGroup" );
      for( i=0; i<NUM_Y_AXES; ++i )
        YAxis[i] = (GlgObject) y_axis_array.GetElement( i );

      /* Set the Chart Zoom mode. It was set and saved with the drawing, 
         but do it again programmatically just in case.
      */
      ChartVP.SetZoomMode( null, Chart, null, GlgObject.CHART_ZOOM_MODE );

      /* Set the initial Y axis label type. */
      ChangeYAxisLabelType( YAxisLabelType );

      /* Query the initial Y ranges defined in the drawing and store them
         for the Restore Ranges action.
      */
      StoreInitialYRanges();

      DisplaySelection( null, true );
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates the chart with simulated data
   //////////////////////////////////////////////////////////////////////////
   void UpdateChart()
   {
      if( timer == null )
        return;   // Prevents race conditions

      /* Supply demo data to update plot lines. */
      for( int i=0; i<NUM_PLOTS; ++i )
      {
         GetDemoData( i, data_point );
         PushPlotPoint( i, data_point );
      }
      
      Update();    // Draw new data.

      timer.start();   // Restart the update timer
   }

   //////////////////////////////////////////////////////////////////////////
   // Pushes the data_point's data into the plot.
   //////////////////////////////////////////////////////////////////////////  
   void PushPlotPoint( int plot_index, DataPoint data_point )
   {
      GlgObject plot = Plot[ plot_index ];

      /* Supply plot value for the chart via ValueEntryPoint. */
      plot.SetDResource( "ValueEntryPoint", data_point.value );
                 
      if( data_point.has_time_stamp )
      {
         /* Supply an optional time stamp. If not supplied, the chart will 
            automatically generate a time stamp using current time. 
         */
         plot.SetDResource( "TimeEntryPoint", data_point.time_stamp );
      }
      
      // Using markers to annotate spikes on the first plot. The plot type
      // was set to LINE & MARKERS in the drawing; marker's Visibility
      // can be used as an entry point for marker visibility values.
      //
      if( plot_index == 0 )
        plot.SetDResource( "Marker/Visibility", 
                           data_point.has_marker ? 1.0 : 0.0 );

      if( !data_point.value_valid )
      {	   
         /* If the data point is not valid, set ValidEntryPoint resource to 
            display holes for invalid data points. If the point is valid,
            it is automatically set to 1.0 by the chart.
         */
         plot.SetDResource( "ValidEntryPoint", 0.0 );
      }
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
      UpdateChart();
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
   // Handle user interaction.
   //////////////////////////////////////////////////////////////////////////
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

      if( format.equals( "Button" ) )         /* Handle button clicks */
      {
         if( !action.equals( "Activate" ) &&      /* Not a push button */
             !action.equals( "ValueChanged" ) )   /* Not a toggle button */
           return;

         AbortZoomTo();

         if( origin.equals( "ScrollToRecent" ) )
         {         
            /* Set time axis's end to current time. */
            ScrollToDataEnd( MOST_RECENT, true ); 
         }
         else if( origin.equals( "ToggleAutoScroll" ) )
         {         
            ChangeAutoScroll( -1 );   /* Toggle curr. value between 0 and 1.0 */
         }
         else if( origin.equals( "ZoomTo" ) )
         {
            ChartVP.SetZoom( null, 't', 0.0 );  /* Start ZoomTo op */
         }
         else if( origin.equals( "ResetZoom" ) )
         {         
            SetChartSpan( SpanIndex );
            RestoreInitialYRanges();
         }
         else if( origin.equals( "ScrollBack" ) )
         {
            ChangeAutoScroll( 0 );

            /* Scroll left by 1/3 of the span. */
            ChartVP.SetZoom( null, 'l', 0.33 );
         }
         else if( origin.equals( "ScrollBack2" ) )
         {
            ChangeAutoScroll( 0 );

            /* Scroll left by a full span. */
            ChartVP.SetZoom( null, 'l', 1.0 );
         }
         else if( origin.equals( "ScrollForward" ) )
         {
            ChangeAutoScroll( 0 );

            /* Scroll right by 1/3 of the span. */
            ChartVP.SetZoom( null, 'r', 0.33 );
         }
         else if( origin.equals( "ScrollForward2" ) )
         {
            ChangeAutoScroll( 0 );

            /* Scroll right by a full span. */
            ChartVP.SetZoom( null, 'r', 1.0 );
         }
         else if( origin.equals( "ToggleLabels" ) )
         {
            ChangeYAxisLabelType( -1 );   /* Change to the next type. */
         }
         else if( origin.equals( "DemoMode" ) )
         {
            /* Toggle the current mode between REAL_TIME, HISTORICAL and
               CALENDAR. */
            SetMode( -1 );
         }

         Update();
      }
      else if( format.equals( "Menu" ) )
      {
         if( !action.equals( "Activate" ) )
           return;

         AbortZoomTo();

         if( origin.equals( "SpanSelector" ) )    /* Span change */
         {         
            SpanIndex = message_obj.GetDResource( "SelectedIndex" ).intValue();

            SetChartSpan( SpanIndex );
            RestoreInitialYRanges(); /* Restore in case the chart was zoomed.*/

            /* Scroll to show the recent data to avoid showing an empty chart
               if user scrolls too much into the future or into the past.
               
               In the real-time mode, invoke ScrollToDataEnd() even if 
               AutoScroll is True to scroll ahead by a few extra seconds to 
               show a few next updates without scrolling the chart.
            */
            GlgMinMax min_max = 
              Chart.GetDataExtent( null, /* x extent */ true );

            if( min_max != null )
            {
               double first_time_stamp = min_max.min;
               double last_time_stamp = min_max.max;
               double displayed_time_end = 
                 Chart.GetDResource( "XAxis/EndValue" ).doubleValue();

               if( Mode == REAL_TIME && AutoScroll != 0 )
                 ScrollToDataEnd( MOST_RECENT, true );
               else if( displayed_time_end >
                        last_time_stamp + GetExtraSeconds() )
                 ScrollToDataEnd( MOST_RECENT, true );
               else if( displayed_time_end - TimeSpan <= first_time_stamp )
                 ScrollToDataEnd( LEAST_RECENT, true );
            }
            Update();
         }
      }
      else if( format.equals( "Zoom" ) )
      {
         if( action.equals( "Zoom" ) )
         {
            if( subaction.equals( "ZoomRectangle" ) )
            {
               /* Store the current AutoScroll state to restore it if ZoomTo 
                  is aborted. */
               StoredScrollState = AutoScroll;
               
               /* Stop scrolling: ZoomTo action is being started. */
               ChangeAutoScroll( 0 );
            }
            else if( subaction.equals( "End" ) )
            {
               /* No addtional actions on finishing ZoomTo. The Y scrollbar 
                  appears automatically if needed: it is set to PAN_Y_AUTO. 
                  Don't resume scrolling: it'll scroll too fast since we zoomed 
                  in. Keep it still to allow inspecting zoomed data.
               */
            }
            else if( subaction.equals( "Abort" ) )
            {
               /* Resume scrolling if it was on. */
               ChangeAutoScroll( StoredScrollState );         
            }
            
            Update();
         }
      }
      else if( format.equals( "Pan" ) )
      {
         if( action.equals( "Pan" ) )
         {
            /* This code may be used to perform custom actions when dragging 
               the chart's data with the mouse. 
            */
            if( subaction.equals( "Start" ) )   /* Chart dragging start */
            {
            }
            else if( subaction.equals( "Drag" ) )    /* Dragging */
            {
            }
            else if( subaction.equals( "ValueChanged" ) )   /* Scrollbars */
            {
            }
            /* Dragging ended or aborted. */
            else if( subaction.equals( "End" ) ||
                     subaction.equals( "Abort" ) )
            {
            }
         }
      }
      else if( format.equals( "Tooltip" ) )
      {
         if( action.equals( "SpecialTooltip" ) )
         {
            /* When the chart tooltip appears, erase selection text, but
               keep selection marker from the tooltip. 
            */
            DisplaySelection( null, false );
         }
      }
      else if( format.equals( "Chart" ) )
      {
         if( action.equals( "CrossHairUpdate" ) )
         {
            /* No need to invoke Update() to redraw the new position of the 
               chart's cross hair cursor: the drawing will be redrawn in one
               batch by either the update timer or DisplaySelection().
            */
         }
      }
      else if( format.equals( "CustomEvent" ) )
      {
         String event_label = message_obj.GetSResource( "EventLabel" );
         if( event_label != null )
           if( event_label.equals( "LegendSelect" ) )
           {
              SelectPlot( GlgObject.GetSelectedPlot() );   /* Select plot. */
              /* Don't stop auto-scroll if legend was clicked on. */
              StopAutoScroll = false;
           }
           else if( event_label.equals( "LegendUnselect" ) )
           {
              SelectPlot( null );                   /* Unselect plot. */
              StopAutoScroll = false;
           }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Changes line width of the selected plot.
   //////////////////////////////////////////////////////////////////////////
   void SelectPlot( GlgObject plot )
   {
      if( plot == SelectedPlot )
        return;

      if( SelectedPlot != null )
      {
         /* Unselect the previously selected plot. */
         SelectedPlot.SetDResource( "Selected", 0.0 );
         SelectedPlot = null;
      }
      
      if( plot != null )
      {
         /* Select a new plot. "Selected" resource controls transformation 
            attached to the plot's line width. When the Selected resource 
            is set to 1, the plot's LineWidth changes to 2.
         */
         plot.SetDResource( "Selected", 1.0 );
         SelectedPlot = plot;
      }
      
      Drawing.Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Used to start scrolling the chart by dragging it with the mouse.
   //////////////////////////////////////////////////////////////////////////
   public void TraceCallback( GlgObject viewport, GlgTraceData trace_info )
   {
      double x, y;
      boolean display_selection = false;

      /* Use ChartViewport's events only. */
      if( !IsReady || trace_info.viewport != ChartVP )
        return;

      int event_type = trace_info.event.getID();
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
       case MouseEvent.MOUSE_MOVED:
       case MouseEvent.MOUSE_DRAGGED:
         x = (double) ((MouseEvent)trace_info.event).getX();
         y = (double) ((MouseEvent)trace_info.event).getY();
         display_selection = true;
         
         /* COORD_MAPPING_ADJ is added to the cursor coordinates for precise
            pixel mapping.
         */
         x += GlgObject.COORD_MAPPING_ADJ;
         y += GlgObject.COORD_MAPPING_ADJ;
         break;
        
       case MouseEvent.MOUSE_EXITED: 
         /* Erase last selection when cursor leaves the window. */
         DisplaySelection( null, true );
         return;

       default: return;
      }
   
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
         if( ZoomToMode() )
           return; /* ZoomTo or dragging mode in progress. */
      
         /* Start dragging with the mouse on a mouse click on Button1,
            or start ZoomTo on Button3.

            For Button1, if user clicked on an axis, the dragging will
            be activated in the direction of that axis. If the user
            clicked on the chart area, dragging in both the time and
            the Y direction will be activated.

            To allow dragging just in one direction, use '>' instead of 's' 
            for horizontal scrolling and '^' for vertical.
         */
         int button = ((MouseEvent)trace_info.event).getButton();
         switch( button )
         {
          case 1:
            ChartVP.SetZoom( null, 's', 0.0 );   /* Drag */
            break;
          case 3: 
            if( ENABLE_ZOOM_TO_ON_BUTTON3 )
              ChartVP.SetZoom( null, 't', 0.0 ); /* ZoomTo */
            break;
         }
         
         /* Disable AutoScroll not to interfere with dragging - but do it later
            in the Trace2 callback, only if legend was not clicked on.
         */
         StopAutoScroll = true;
         break;
      }

      /* In addition to a tooltip appearing after a timeout when the mouse 
         stops, display selection information when the mouse moves over a 
         chart or axis. The selection is displayed using the same format as 
         the tooltip, which is configured via the TooltipFormat attribute 
         of the chart. Alternatively, an application can invoke 
         GlgCreateChartSelection() and display the returned data in a 
         custom format.
      */
      if( display_selection && !ZoomToMode() )
      {
         String selection_string = 
           Chart.CreateTooltipString( x, y, 10.0, 10.0, "<single_line>" );

         /* Display new selection or erase last selection if no selection
            (when string is null). 
         */
         DisplaySelection( selection_string, true );
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Trace2 callback is invoked after the Input callback.
   //////////////////////////////////////////////////////////////////////////
   public void Trace2Callback( GlgObject viewport, GlgTraceData trace_info )
   {
      /* Use ChartViewport's events only. */
      if( !IsReady || trace_info.viewport != ChartVP )
        return;

      /* Stop auto-scroll on a click in the chart, but not if the legend was 
         clicked.
      */
      int event_type = trace_info.event.getID();
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
         if( StopAutoScroll )
         {
            StopAutoScroll = false;
            ChangeAutoScroll( 0 );
         }
         break;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Display information about the selected point. It is used on a mouse move 
   // in addition to a tooltip.
   //////////////////////////////////////////////////////////////////////////
   void DisplaySelection( String selection_string, 
                          boolean erase_selection_marker )
   {   
      if( selection_string == null )
      {
         selection_string = "";

         // No selection: erase selection highlight marker if requested.
         if( erase_selection_marker )
           Chart.SetDResource( "DrawSelected", 0.0, /* if changed */ true );
      }

      ChartVP.SetSResource( "SelectionLabel/String", selection_string, true );

      /* In the real-time mode the drawing is updated on a timer, 
         otherwise update it here.
      */
      if( Mode != REAL_TIME )
        ChartVP.Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Scrolls the graph to the minimum or maximum time stamp to show the 
   // most recent or the least recent data. If show_extra is True, adds a 
   // few extra seconds in the real-time mode to show a few next updates
   // without scrolling the chart.
   //
   // Enabling AutoScroll automatically scrolls to show current data points 
   // when the new time stamp is more recent then the EndValue of the axis, 
   // but it is not the case when the chart is scrolled into the future 
   // (to the right) - still need to invoke this method.
   //////////////////////////////////////////////////////////////////////////
   void ScrollToDataEnd( int data_end, boolean show_extra )
   {
      double end_value, extra_sec;
      
      if( data_end == DONT_CHANGE )
        return;

      /* Get the min and max time stamp. */
      GlgMinMax min_max = Chart.GetDataExtent( null, /* x extent */ true );
      if( min_max == null )
        return;

      if( show_extra )   
        extra_sec = GetExtraSeconds();
      else
        extra_sec = 0.0;

      if( data_end == MOST_RECENT )
        end_value = min_max.max + extra_sec;
      else   /* LEAST_RECENT */
        end_value = min_max.min - extra_sec + TimeSpan ;

      Chart.SetDResource( "XAxis/EndValue", end_value );
   }

   //////////////////////////////////////////////////////////////////////////
   // Determines a good number of extra seconds to be added at the end in
   // the real-time mode to show a few next updates without scrolling the
   // chart.
   //////////////////////////////////////////////////////////////////////////
   double GetExtraSeconds()
   {
      double extra_sec, max_extra_sec;
      
      if( Mode != REAL_TIME )
        return 0.0;
      
      extra_sec = TimeSpan * 0.1;
      switch( SpanIndex )
      {
       default:
       case 0:
       case 1: 
       case 2: max_extra_sec = 3.0; break;
       case 3: max_extra_sec = 5.0; break;
      }

      if( extra_sec > max_extra_sec )
        extra_sec = max_extra_sec;

      return extra_sec;
   }

   //////////////////////////////////////////////////////////////////////////
   void ChangeAutoScroll( int new_value )
   {
      int pan_x;

      if( new_value == -1 ) /* Use the state of the ToggleAutoScroll button. */
      {
         AutoScroll = (int) GetDResource( "ToggleAutoScroll/OnState" );
      }
      else    /* Set to the supplied value. */
      {
         AutoScroll = new_value;

         /* Update the AutoScroll toggle with the new value. */
         SetDResource( "ToggleAutoScroll/OnState", (double) AutoScroll );
      }

      /* Set chart's auto-scroll. */
      Chart.SetDResource( "AutoScroll", (double) AutoScroll );

      /* Activate time scrollbar if AutoScroll is Off. The Y value scrollbar 
         uses PAN_Y_AUTO and appears automatically as needed.
      */
      pan_x = ( AutoScroll != 0 ? GlgObject.NO_PAN : GlgObject.PAN_X );
      ChartVP.SetDResource( "Pan", (double) ( pan_x | GlgObject.PAN_Y_AUTO ) );
   }

   //////////////////////////////////////////////////////////////////////////
   // Changes the time span shown in the graph, adjusts major and minor tick 
   // intervals to match the time span.
   //////////////////////////////////////////////////////////////////////////
   void SetChartSpan( int span_index )
   {
      int span, major_interval, minor_interval, time_offset;
      int num_vis_points;
      double sampling_interval;

      boolean in_the_middle = false;
      boolean fix_leap_years = false;

      /* Change chart's time span, as well as major and minor tick intervals.*/
      switch( Mode )
      {
       case REAL_TIME:
         switch( span_index )
         {
          case 0:         
            span = 10;            /* 10 sec. */
            major_interval = 3;   /* major tick every 3 sec. */
            minor_interval = 1;   /* minor tick every sec. */
            break;
            
          case 1:
            span = 60;            /* 1 min. */
            major_interval = 10;  /* major tick every 10 sec. */
            minor_interval = 1;   /* minor tick every sec. */
            break;
            
          case 2:
            span = 600;           /* 10 min. */
            major_interval = 180; /* major tick every 3 min. */
            minor_interval = 60;  /* minor tick every min. */
            break;
            
          case 3:
            span = -1;             /* Show all data */
            major_interval = -4;   /* 4 major ticks */
            minor_interval = -5;   /* 5 minor ticks */
            break;
            
          default: error( "Invalid span index", false ); return;
         }
         time_offset = 0;
         sampling_interval = UpdateInterval / 1000.0;
         break;

       case HISTORICAL:   
         switch( span_index )
         {
          case 0:
            span = 3600;                /* 1 hour */
            major_interval = 60 * 10;   /* major ticks every 10 min. */
            minor_interval = 60;        /* minor ticks every min. */
            break;

          case 1:
            span = 3600 * 8;            /* 8 hours */
            major_interval = 3600 * 2;  /* major tick every 2 hours */
            minor_interval = 60 * 15;   /* minor tick every 15 minutes */
            break;
         
          case 2:
            span = DAY;                 /* 24 hours */
            major_interval = 3600 * 6;  /* major tick every 6 hours */
            minor_interval = 3600;      /* minor tick every hour */
            break;
         
          case 3:
            span = DAY * 10;            /* 10 days */
            major_interval = DAY * 2;   /* major tick every 2 days */
            minor_interval = 3600 * 12; /* minor tick every 12 hours */
            break;

          default: error( "Invalid span index", false ); return;
         }

         /* Positions major ticks and labels at 8 AM instead of midnight
            when the major tick interval is set to one day or a whole number 
            of days.
         */
         time_offset = 3600 * 8;
         sampling_interval = HISTORICAL_DATA_INTERVAL;
         break;

       case CALENDAR: 
         switch( span_index )
         {
          case 0:
            span = DAY * 31;             /* 1 month */
            major_interval = DAY * 5 ;   /* major ticks every 5 days */
            minor_interval = DAY;        /* minor ticks every day. */
            
            /* Positions ticks and labels at noon instead of midnight. */
            time_offset = DAY / 2;
            break;
            
          case 1:
            span = DAY * 31 * 3;         /* 1 quarter. */
            major_interval = DAY * 14;   /* major tick every 2 weeks */
            minor_interval = DAY;        /* minor tick every day */
            
            /* Positions ticks and labels at noon instead of midnight. */
            time_offset = DAY / 2;
            break;
            
          case 2:
            /* Display labels in the middle of each month's interval. */
            in_the_middle = true; 
            
            /* Offsets month labels by 1/2 month to position them in the 
               midddle of the month's interval.
            */
            time_offset = DAY * 15;
            
            span = DAY * 365;          /* 1 year */
            major_interval = -12;      /* major tick every month (12 ticks) */
            minor_interval = -2;       /* minor tick in the middle (2 ticks)
                                          to show the extent of the month. */
            break;
         
          case 3:
            span = DAY * 365 * 10;      /* 10 years */
            major_interval = DAY * 365; /* major tick every year */
            minor_interval = -12;       /* minor tick every month (12 ticks) */

            /* Time labels display only the year for this time scale.
               Position major ticks and labels a bit past the 1st the month 
               to avoid any rounding errors. The tooltips display the exact
               date regardless.
            */
            time_offset = 3600;
            break;
            
          default: error( "Invalid span index", false ); return;
         }

         if( span_index >= 2 )     /* 1 year or 10 years */
           /* The major tick is positioned at the start of the month or the 
              start of the year. Tell the chart to properly calculate the label
              position by adjusting by the number of accumulated leap days.
              It matters only in the calendar mode when the major tick interval
              is greater then a day.
           */
           fix_leap_years = true;

         sampling_interval = DAY / 2;
         break;

       default: error( "Invalid mode", false ); return;
      }

      /* Update the menu in the drawing with the initial value if different. */
      SetDResource( "SpanSelector/SelectedIndex", (double)span_index, true );

      /* Set intervals before SetZoom() below to avoid redrawing huge number 
         of labels. */
      Chart.SetDResource( "XAxis/MajorInterval", (double) major_interval );
      Chart.SetDResource( "XAxis/MinorInterval", (double) minor_interval );

      Chart.SetDResource( "XAxis/MajorOffset", (double) time_offset );
      Chart.SetDResource( "XAxis/FixLeapYears", ( fix_leap_years ? 1.0 : 0.0 ) );

      /* Set the X axis span which controls how much data is displayed in the 
         chart. */
      if( span > 0 )
      {
         TimeSpan = span;
         Chart.SetDResource( "XAxis/Span", (double) TimeSpan );
      }
      else   /* span == -1 : show all accumulated data. */
      {
         /* 'N' resets span to show all data accumulated in the buffer. */
         ChartVP.SetZoom( null, 'N', 0.0 );

         /* Query the actual time span: set it to the extent of the data 
            accumulated in the chart's buffer, plus a few extra seconds
            at the end to show a few updates without scrolling the chart.
         */
         GlgMinMax min_max = Chart.GetDataExtent( null, /* x extent */ true );
         TimeSpan = (int) ( min_max.max - min_max.min + GetExtraSeconds() );
      }

      /* Turn on data filtering for large spans. FilterType and FilterPrecision
         attributes of all plots are constrained, so that they may be set in one
         place, on one plot.
      */
      if( span_index > 1 )
      {
         /* Agregate multiple data samples to minimize a number of data points 
            drawn per each horizontal FilterPrecision interval.
            Show only one set of MIN/MAX values per each pixel interval. 
            An averaging data filter is also available.
         */
         Plot[0].SetDResource( "FilterType", 
                               (double) GlgObject.MIN_MAX_FILTER );
         Plot[0].SetDResource( "FilterPrecision", 1.0 );
      }
      else
        Plot[0].SetDResource( "FilterType", 
                              (double) GlgObject.NULL_FILTER );

      /* Display the filter state in the drawing. */
      ChartVP.SetSResource( "DataFilterState", span_index > 1 ? "ON" : "OFF" );

      /* Erase major ticks if showing month labels in the middle of the month 
         interval in the CALENDAR mode. */
      Chart.SetDResource( "XAxis/MajorTickSize", 
                          ( in_the_middle ? 0.0 : 10.0 ) );
      Chart.SetDResource( "XAxis/LabelOffset",
                          ( in_the_middle ? 10.0 : 0.0 ) );

      /* Display the number of data points visible in all three lines in the 
         current time span.
      */
      num_vis_points = (int) ( TimeSpan / sampling_interval * NUM_PLOTS );

      /* Must be divisible by NUM_PLOTS */
      num_vis_points = num_vis_points / NUM_PLOTS * NUM_PLOTS;
      num_vis_points = Math.min( num_vis_points, BufferSize * NUM_PLOTS );
      ChartVP.SetDResource( "NumDataPointsVisible", num_vis_points );

      /* Change time and tooltip formatting to match the demo mode and the 
         selected span. */
      SetTimeFormats();

      SetMarkerSize();   /* Decrease marker size for large spans. */
   }

   //////////////////////////////////////////////////////////////////////////
   // Changes labels in the span selection buttons when switching between 
   // the REAL_TIME, HISTORICAL and CALENDAR modes.
   //////////////////////////////////////////////////////////////////////////
   void SetSelectorLabels()
   {
      final int NUM_SPAN_OPTIONS = 4;

      GlgObject button;
      String res_name, label;
      int i;

      for( i=0; i<NUM_SPAN_OPTIONS; ++i )
      {
         res_name = "SpanSelector/Button" +  i;
         button = Drawing.GetResourceObject( res_name );

         switch( Mode )
         {
          case REAL_TIME:
            switch( i )
            {
             case 0: label = "10 sec"; break;
             case 1: label = "1 min";  break;
             case 2: label = "10 min"; break;
             case 3: label = "All";    break;           
             default: error( "Invalid span index", false ); return;
            }
            break;

          case HISTORICAL:
            switch( i )
            {
             case 0: label = "1 hour"; break;
             case 1: label = "8 hours";  break;
             case 2: label = "24 hours"; break;
             case 3: label = "1 week";    break;           
             default: error( "Invalid span index", false ); return;
            }
            break;

          case CALENDAR:
            switch( i )
            {
             case 0: label = "1 month"; break;
             case 1: label = "1 quarter";  break;
             case 2: label = "1 year"; break;
             case 3: label = "10 years";    break;           
             default: error( "Invalid span index", false ); return;
            }
            break;

          default: error( "Invalid mode", false ); return;
         }

         button.SetSResource( "LabelString", label );
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void StoreInitialYRanges()
   {
      int i;

      /* In this demo, each plot is associated with the corresponding axis by
         setting the plot's LinkedAxis property in the drawing file. When a 
         plot is linked to an axis, the plot and the axis use the same Y range, 
         
         We are using an Intermediate API to access Y axes here for convenience.
         Alternatively, Y axes' resources can be accessed by their resource 
         names via the Standard API, for example:
            "ChartVP/Chart/YAxisGroup/YAxis#0/Low"
      */
      for( i=0; i<NUM_Y_AXES; ++i )
      {
         Min[i] = YAxis[i].GetDResource( "Low" ).doubleValue();
         Max[i] = YAxis[i].GetDResource( "High" ).doubleValue();
      }
   }


   //////////////////////////////////////////////////////////////////////////
   void RestoreInitialYRanges()
   {
      int i;

      /* In this demo, each plot is associated with the corresponding axis by
         setting the plot's LinkedAxis property in the drawing file. When a plot
         is linked to an axis, changing the plot's and axis' ranges may be done
         by changing ranges of just one object: either a plot or its linked 
         axis. If a plot is not linked to an axis, its range may be different.
      */
      for( i=0; i<NUM_Y_AXES; ++i )
      {
         YAxis[i].SetDResource( "Low",  Min[i] );
         YAxis[i].SetDResource( "High", Max[i] );
      }
   }

   //////////////////////////////////////////////////////////////////////////
   boolean ZoomToMode()
   {
      int zoom_mode = ChartVP.GetDResource( "ZoomToMode" ).intValue();
      return ( zoom_mode != 0 );
   }

   //////////////////////////////////////////////////////////////////////////
   void AbortZoomTo()
   {
      if( ZoomToMode() )
      {
         /* Abort zoom mode in progress. */
         ChartVP.SetZoom( null, 'e', 0.0 ); 
         ChartVP.Update();
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Demonstrates different styles of Y axis label positioning.
   //
   // This is usually done by configuring the chart in the GlgBuilder.
   // This code just toggles through a few options via an API.
   //////////////////////////////////////////////////////////////////////////
   void ChangeYAxisLabelType( int new_type )
   {
      final int NUM_LABEL_TYPES = 4;

      String label0, label1, label2;
      boolean offset_labels;
      int text_direction;      
      int label_anchoring; /* Label anchoring relatively to its control point. */
      int label_position;  /* Label position relatively to its axis. */
      int i;        

      if( new_type < 0 )              /* Toggle through the values. */   
      {
         ++YAxisLabelType;
         if( YAxisLabelType >= NUM_LABEL_TYPES )
           YAxisLabelType = 0;
      }
      else
        YAxisLabelType = new_type;    /* Use the supplied value. */

      switch( YAxisLabelType )
      {
       case 0:
         label0 = "Var1"; 
         label1 = "Var2"; 
         label2 = "Var3"; 
         text_direction = GlgObject.HORIZONTAL;

         /* Position and anchor axis labels at the center of each axis in the
            horizontal direction. */
         label_position = ( GlgObject.HCENTER | GlgObject.VTOP );
         label_anchoring = ( GlgObject.HCENTER | GlgObject.VBOTTOM );
         offset_labels = false;
         break;

       case 1:
         label0 = "Var1"; 
         label1 = "Var2"; 
         label2 = "Var3"; 
         text_direction = GlgObject.VERTICAL_ROTATED_LEFT;

         /* Position and anchor axis labels at the center of each axis in the
            horizontal direction. */
         label_position = ( GlgObject.HCENTER | GlgObject.VTOP );
         label_anchoring = ( GlgObject.HCENTER | GlgObject.VBOTTOM );
         offset_labels = false;
         break;

       case 2:
         label0 = "Variable 1"; 
         label1 = "Variable 2"; 
         label2 = "Variable 3"; 
         text_direction = GlgObject.HORIZONTAL;

         /* Position and anchor axis labels on the left edge of each axis in 
            the horizontal direction. */
         label_position = ( GlgObject.HLEFT | GlgObject.VTOP );
         label_anchoring = ( GlgObject.HLEFT | GlgObject.VBOTTOM );
         offset_labels = true;
         break;

       case 3:
         label0 = "Var1"; 
         label1 = "Var2"; 
         label2 = "Var3"; 
         text_direction = GlgObject.VERTICAL_ROTATED_LEFT;
         /* Position and anchor axis labels at the center of each axis in the
            vertical direction. */      
         label_position = ( GlgObject.HLEFT | GlgObject.VCENTER );
         label_anchoring = ( GlgObject.HRIGHT | GlgObject.VCENTER );
         offset_labels = false;
         break;

       default:
         error( "Wrong type", false ); 
         YAxisLabelType = 0; 
         return;
      }

      YAxis[0].SetSResource( "AxisLabel/String", label0 );
      YAxis[1].SetSResource( "AxisLabel/String", label1 );
      YAxis[2].SetSResource( "AxisLabel/String", label2 );

      /* Set text direction for all labels using the % wildcard. */
      Chart.SetDResource( "YAxisGroup/YAxis#%/AxisLabel/TextDirection", 
                          (double) text_direction );

      if( offset_labels )
      {
         for( i=0; i<3; ++i )
           /* Set increasing Y offsets. */
           YAxis[i].SetGResource( "AxisLabelOffset", 0.0, 32.0 - i * 13, 0.0 );
      }
      else    /* Set all Y offsets = 10 using the % wildcard. */
        Chart.SetGResource( "YAxisGroup/YAxis#%/AxisLabelOffset", 
                            0.0, 10.0, 0.0 );

      /* Set position and anchoring of all labels using the % wildcard. */
      Chart.SetDResource( "YAxisGroup/YAxis#%/AxisLabelPosition", 
                          (double) label_position );
      Chart.SetDResource( "YAxisGroup/YAxis#%/AxisLabelAnchoring", 
                          (double) label_anchoring );

      /* Adjusts the space taken by the Y axes to accomodate different axis 
         label layouts.
      */
      AdjustYAxisSpace();
   }

   //////////////////////////////////////////////////////////////////////////
   // Returns data sample querying interval (in sec.) depending on the demo 
   // mode.
   //////////////////////////////////////////////////////////////////////////
   double GetPointInterval()
   {
      switch( Mode )
      {
       case REAL_TIME:
         return UpdateInterval / 1000.0;  /* Update interval is in millisec. */
         
       case HISTORICAL:
         /* Historical data are sampled once per minute. */
         return HISTORICAL_DATA_INTERVAL;
         
       case CALENDAR:
         /* Sample calendar data twice per day. 
            libc can not go back beyond 1900 - only ~40K days.
            If sampling once per day, limit BufferSize to 40K.
         */
         return DAY / 2;
         
       default: error( "Invalid mode", false ); return 100.0;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void PreFillChartData()
   {
      double
        current_time, start_time, end_time,
        num_seconds, dt;

      /* Display "Pre-filling the chart" message. */
      ChartVP.SetDResource( "PreFillMessage/Visibility", 1.0 );
      ChartVP.UpdateImmediately();   /* Redraw not waiting for paint. */

      current_time = GetCurrTime();
   
      /* Roll back by the amount corresponding to the buffer size. */
      dt = GetPointInterval();
      num_seconds = BufferSize * dt;
      
      if( Mode == REAL_TIME )
        num_seconds += 1.0;  /* Add an extra second to avoid rounding errors. */

      start_time = current_time - num_seconds;
      end_time = 0.0;        /* Stop at the current time. */

      for( int i=0; i<NUM_PLOTS; ++i )
        FillHistData( i, start_time, end_time );

      /* Remove the message. */
      ChartVP.SetDResource( "PreFillMessage/Visibility", 0.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   // Prefills the chart with data using simulated data. In a real application,
   // data will be coming from an application-specific data source.
   //////////////////////////////////////////////////////////////////////////
   void FillHistData( int plot_index, double start_time, double end_time )
   {
      boolean check_curr_time;

      /* Demo: generate demo pre-fill data with the same frequency as the 
         UpdateInterval (in millisec). In an application, data will be queried
         from a real data source, returning an array of data points.
      */
      double dt = GetPointInterval();

      if( end_time == 0.0 )
      {
         check_curr_time = true;
         end_time = GetCurrTime();
      }
      else
        check_curr_time = false;
      
      /* When prefilling up to the current time, use the result of 
         GetCurrTime() as the loop's end condition and check it after
         each iteration to account for the time it takes to prefill 
         the chart.
      */
      for( double time_stamp = start_time; 
           time_stamp < end_time && ( !check_curr_time || 
                                      time_stamp < GetCurrTime() );
           time_stamp += dt )
      {
         GetDemoData( plot_index, data_point );

         /* Set the time stamp. */
         data_point.time_stamp = time_stamp;
         data_point.has_time_stamp = true;
      
         PushPlotPoint( plot_index, data_point );
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Supplies demo data, including the plot's value, an optional time stamp
   // and an optional sample_valid flag, as well as visibility of a mraker used
   // to annotate some data pooints. 
   //
   // In a real application, data will be coming from an application-specific 
   // data source.
   //////////////////////////////////////////////////////////////////////////
   void GetDemoData( int plot_index, DataPoint data_point )
   {
      GetDemoPlotValue( plot_index, data_point );  /* Fills a value to plot. */

      /* Let the chart to use current time as the time stamp. Optionally,
         an application can provide a time stamp in data_point.time_stamp
         and set data_point.has_time_stamp = true.
      */
      data_point.has_time_stamp = false;
      
      /* Set an optional ValidEntryPoint to make some samples invalid for
         Plot0.0 It is optional: default=True is used for the rest of the plots 
         when ValidEntryPoint is not supplied.
      */   
      if( plot_index == 0 )
      {
         if( Plot0Valid ) 
           /* Make samples invalid occasionally. */
           Plot0Valid = ( GlgObject.Rand( 0.0, 100.0 ) > 2.0 );
         else
           /* Make it valid again after a while. */
           Plot0Valid= ( GlgObject.Rand( 0.0, 100.0 ) > 30.0 );
         
         data_point.value_valid = Plot0Valid;
      }
      else
        data_point.value_valid = true;
   }

   /* DATA SIMULATION: These variables used for simulating data displayed 
      in the chart. In a real application, data will be coming from a 
      real data source.
   */
   static final int MAX_COUNTER = 50000;
   static final int PERIOD = 1000;
   static final int SPIKE_DURATION_RT = 25;
   static final int SPIKE_DURATION_HS = 8;
   static final int APPROX_PERIOD = 100;

   static boolean first_error = true;
   static long state = 0;
   static long change_counter = 10;
   static long spike_counter = 1000;
   static long approx_counter = 0;
   static long last_direction = 1;

   static double max_spike_height = 0.0;
   static double last_value = 5.0;
   static double increment_sign = 1.0;
   static double last_value2 = 0.0; 
   static double increment_sign2 = 1.0;         
   static double last_value3 = 70.0;

   //////////////////////////////////////////////////////////////////////////
   // Supplies plot values for the demo; also sets data_point.has_marker field
   // to annotate some data points with a marker.
   //
   // In a real application, data will be coming from an application-specific 
   // data source.
   //////////////////////////////////////////////////////////////////////////
   void GetDemoPlotValue( int plot_index, DataPoint data_point )
   {
      double
        value, alpha, period, 
        spike_sign, spike_height;   
      int spike_duration;
   
      /* First time: init plot's state counters used to simulate data. */
      if( PlotCounter == null )
      {
         PlotCounter = new int[ NUM_PLOTS ];
         for( int i=0; i<NUM_PLOTS; ++i )
           PlotCounter[ i ] = 0;
      }

      alpha = 2.0 * Math.PI * PlotCounter[ plot_index ] / PERIOD;
      switch( plot_index )
      {
       case 0:      
         if( Mode == REAL_TIME )           
           value = 5.0 + 1.5 * Math.sin( alpha / 5.0 ) + Math.sin( 2.0 * alpha );
         else               
         {
            last_value += GlgObject.Rand( 0.0, 0.01 ) * increment_sign;
            last_value2 += GlgObject.Rand( 0.0, 0.03 ) * increment_sign2;

            value = last_value + last_value2;

            if( GlgObject.Rand( 0.0, 1000.0 ) > 995.0 )
              increment_sign *= -1;

            if( GlgObject.Rand( 0.0, 1000.0 ) > 750.0 )
              increment_sign2 *= -1;

            if( value > 6.2 )
              increment_sign2 = -1.0;
            else if( value < 3.8 )
              increment_sign2 = 1.0;
         }
 
         /* Add a spike */
         spike_height = 0;
         spike_duration = 
           ( Mode == REAL_TIME ? SPIKE_DURATION_RT : SPIKE_DURATION_HS );

         if( spike_counter >= spike_duration * 3 )
         {
            if( GlgObject.Rand( 0.0, 1000.0 ) > 990.0 ) 
            {
               /* Start a spike */
               spike_counter = 0;
               spike_sign = ( GlgObject.Rand( 0.0, 10.0 ) > 4.0 ? 1.0 : -1.0 );
               max_spike_height = spike_sign * 
                 GlgObject.Rand( 0.0, Mode == REAL_TIME ? 1.0 : 0.5 );
            }
         }

         /* Annotate spikes with a marker. */
         data_point.has_marker = ( spike_counter == 0 );

         if( spike_counter <= spike_duration )
         {
            double spike_coeff;
         
            spike_coeff = 1.0 - spike_counter / (double) spike_duration;
            spike_height = 
              0.3 * max_spike_height * spike_coeff  * spike_coeff * 
              ( 1.0 + Math.cos( 2.0 * Math.PI * spike_counter / 12.0 ) );
         }

         ++spike_counter;
         value += spike_height; 
         break;
         
       case 1:
         if( change_counter != 0 )
         {
            --change_counter;
         
            if( change_counter == 0 )
            {            
               state = ( state == 0 ? 1 : 0 );   /* Change the state */
               
               /* Time of the next change */
               change_counter = (int) GlgObject.Rand( 10.0, 100.0 );
            }
         }
      
         value = state;
         break;
      
       case 2:
         if( Mode == REAL_TIME )
         {
            period = ( 0.95 + 0.05 * Math.abs( Math.sin( alpha / 10.0 ) ) ); 
            value = 8.3 + Math.sin( 30.0 * period * alpha ) * 
              Math.sin( Math.PI / 8.0 + alpha );
            value *= 10.0;
         }
         else
         {
            value = last_value3 + last_direction * 0.1 * 
              ( 1.0 - Math.cos( 2.0 * Math.PI * approx_counter / 
                               APPROX_PERIOD ) );
            
            last_value3 = value;
            if( Mode == HISTORICAL )
              approx_counter += 3;
            else
              approx_counter += 1;
            
            if( last_direction < 0.0 && value < 0.6  * Max[ plot_index ] ||
                last_direction > 0.0 && value > 0.95 * Max[ plot_index ] ||
                GlgObject.Rand( 0.0, 1000.0 ) > 900.0 )
            {
               last_direction *= -1;
               approx_counter = 0;
            }
         }
         break;
      
       default:
         if( first_error )
         {
            first_error = false;
            error( "Add a case to provide demo data for added plots.", false );
         }
         value = 62.0;
         break;
      }

      /* Increase the plot's state counter used to simulate demo data. */
      ++PlotCounter[ plot_index ];
      PlotCounter[ plot_index ] = ( PlotCounter[ plot_index ] % MAX_COUNTER );

      data_point.value = value;   /* Supplied value. */
   }

   //////////////////////////////////////////////////////////////////////////
   // Sets the display mode: REAL_TIME, HISTORICAL or CALENDAR.
   // If invoked with mode=-1, switch the mode between all three demo mode.
   //////////////////////////////////////////////////////////////////////////
   void SetMode( int mode )
   {
      SelectPlot( null );   /* Unselect a previously selected plot, if any. */

      if( mode >= 0 )
        Mode = mode;   /* Set to the specified mode. */
      else    /* Negative value: switch between all modes. */
      {
         ++Mode;
         if( Mode > CALENDAR )
           Mode = REAL_TIME;
      }

      switch( Mode )
      {
       case REAL_TIME:      
         SpanIndex = 1;
         AutoScroll = 1; 
         timer.start();     /* Start timer to update data in REAL_TIME mode. */
         break;

       case HISTORICAL:
       case CALENDAR:      
         SpanIndex = ( Mode == HISTORICAL ? 2 : 3 );
         AutoScroll = 0;
         timer.stop();  /* No data updates in HISTORICAL and CALENDAR modes. */
         break;

       default: error( "Invalid mode", false ); return;
      }

      /* Display the number of data points per line and the total number of 
         points. */
      ChartVP.SetDResource( "NumDataPoints", (double) BufferSize );
      ChartVP.SetDResource( "NumDataPointsTotal", 
                            (double) ( BufferSize * NUM_PLOTS ) );      

      /* Disable "Toggle AutoScroll" button in HISTORICAL and CALENDAR modes.*/
      SetDResource( "ToggleAutoScroll/HandlerDisabled", 
                    ( Mode != REAL_TIME ? 1.0 : 0.0 ) );

      /* Disable AutoScroll in non-real-time modes. */
      ChangeAutoScroll( AutoScroll );
   
      SetSelectorLabels();
      SetChartSpan( SpanIndex );
      RestoreInitialYRanges();

      /* Erase the step plot and its axis in the CALENDAR mode. */
      double enabled = ( Mode == CALENDAR ? 0.0 : 1.0 );
      YAxis[1].SetDResource( "Visibility", enabled );
      Plot[1].SetDResource( "Enabled", enabled );
      Chart.SetDResource( "Levels/Level#0/Enabled", enabled );
      Chart.SetDResource( "Levels/Level#1/Enabled", enabled );
      AdjustYAxisSpace();

      /* Clear all accumulated data samples: the data will be refilled 
         according to the new display mode.
      */
      Chart.ClearDataBuffer( null );

      /* Switch label in the DemoMode button. */
      Drawing.SetDResource( "DemoMode/Mode", (double) Mode );

      /* Update using UpdateImmediately() to show the "Prefilling chart..."
         message right away instead of waiting for the paint event, which
         would come only after prefilling has finished.
      */         
      Drawing.UpdateImmediately();

      /* In the real-time mode pre-fill chart data only if PrefillData=true.
         Always prefill in the historical and calendar mode.
      */
      if( Mode != REAL_TIME || PrefillData )
        PreFillChartData();

      ScrollToDataEnd( MOST_RECENT, false );
   }

   //////////////////////////////////////////////////////////////////////////
   // Sets the formats of time labels and tooltips depending on the demo mode
   // and the selected time span.
   //////////////////////////////////////////////////////////////////////////
   void SetTimeFormats()
   {
      String time_label_format, time_tooltip_format, chart_tooltip_format;

      /* No additional code is required to use the defaults defined in the 
         drawing. This elaborate example illustrates advanced options for 
         customizing label and tooltip formatting when switching between 
         time spans and data display modes modes.
         
         For an even greater control over labels and tooltips, an application 
         can define custom Label and Tooltip formatters that will supply 
         custom strings for axis labels and tooltips.
      */

      switch( Mode )
      {
       case REAL_TIME:
         /* See strftime() for all time format options. */
         time_label_format = "%X%n%x"; 

         time_tooltip_format = "Time: <axis_time:%X> +0.<axis_time_ms:%03.0lf> sec.\nDate: <axis_time:%x>";

         /* <sample_time:%s> inherits time format from the X axis. */
         chart_tooltip_format = "Plot <plot_string:%s> value= <sample_y:%.2lf>\n<sample_time:%s>";
         break;

       case HISTORICAL:
         /* See strftime() for all time format options. */
         time_label_format = "%R%n%e %b %Y";

         time_tooltip_format = "Time: <axis_time:%c>";
         chart_tooltip_format = "Plot <plot_string:%s> value= <sample_y:%.2lf>\nTime: <sample_x_time:%c>";
         break;

       case CALENDAR:
         /* See strftime() for all time format options. */
         if( SpanIndex == 0 || SpanIndex == 1 )   /* 1 month or 1 quarter */
         {
            /* Include day of month. */
            time_label_format = "%e %b\n%Y";
            time_tooltip_format = "Date: <axis_time:%a> <axis_time:%d> <axis_time:%b> <axis_time:%Y>";
            chart_tooltip_format = "<plot_string:%s> value= <sample_y:%.2lf>\nDate: <sample_x_time:%a> <sample_x_time:%d> <sample_x_time:%b> <sample_x_time:%Y>";
         }
         else    /* SpanIndex == 2 or 3 :   1 year or 10 years */
         {
            /* Exclude day of month. */
            time_tooltip_format = "Date: <axis_time:%d> <axis_time:%b> <axis_time:%Y>";
            chart_tooltip_format = "<plot_string:%s> value= <sample_y:%.2lf>\nDate: <sample_x_time:%d> <sample_x_time:%b> <sample_x_time:%Y>";
            
            if( SpanIndex == 2 )      /* 1 year */
            {
               /* Display only month + short year in time labels. */
               time_label_format = "%b\n%Y";
            }
            else    /* SpanIndex == 3 : 10 years */
            {
               /* Display only year in labels. */
               time_label_format = "%Y";
            }
         } 
         break;

       default: error( "Invalid mode", false ); return;
      }

      /* Set time label and tooltip formats. */
      Chart.SetSResource( "XAxis/TimeFormat", time_label_format );
      Chart.SetSResource( "XAxis/TooltipFormat", time_tooltip_format );
      Chart.SetSResource( "TooltipFormat", chart_tooltip_format );
   }

   //////////////////////////////////////////////////////////////////////////
   // The chart layout and Y axis space may be configured interactively in
   // the Graphics Builder.
   //
   // This function adjusts the space taken by the Y axes at run time when a 
   // number of displayed Y axes and/or their label layout changes. 
   //////////////////////////////////////////////////////////////////////////
   void AdjustYAxisSpace()
   {
      double axis_offset, label_offset;
      
      if( Mode == CALENDAR )
      {
         /* Only two axes are displayed in non-CALENDAR modes. */     
         axis_offset = -25.0;
         
         /* YAxisLabelType == 3 needs extra space to position labels for 
            two axes.
         */
         label_offset = ( ( YAxisLabelType == 3 ) ? 30.0 : 0.0 );
      }
      else
      {
         /* All three axes are displayed in non-CALENDAR modes. */
         axis_offset = 0.0;
         
         /* YAxisLabelType == 3 needs extra space to position labels for three
            axes. */
         label_offset = ( ( YAxisLabelType == 3 ) ? 45.0 : 0.0 );
      }
      
      ChartVP.SetDResource( "OffsetLeft", axis_offset + label_offset );
   }

   //////////////////////////////////////////////////////////////////////////
   // Decreases marker size for large spans.
   //////////////////////////////////////////////////////////////////////////
   void SetMarkerSize()
   {
      Plot[0].SetDResource( "Marker/MarkerSize", SpanIndex < 2 ? 7.0 : 5.0 );

   }

   //////////////////////////////////////////////////////////////////////////
   void error( String message, boolean quit )
   {
      System.out.println( message );
      if( quit )
        System.exit( 0 );
   }

   //////////////////////////////////////////////////////////////////////////
   double GetCurrTime()
   {
      return System.currentTimeMillis() / 1000.0;
   }
}
