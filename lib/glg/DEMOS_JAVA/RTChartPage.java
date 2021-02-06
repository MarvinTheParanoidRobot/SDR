import java.awt.event.*;
import java.util.ArrayList;
import com.genlogic.*;

public class RTChartPage extends HMIPageBase
{
   GlgObject Viewport;
   GlgObject ChartVP;
   GlgObject Chart;

   /* Convenient time span constants. */
   static final int ONE_MINUTE = 60;
   static final int ONE_HOUR   = 3600;
   static final int ONE_DAY    = 3600 * 24;

   /* Prefill time interval, specifies amount of data to prefill in the 
      real time chart. */
   static final int PREFILL_SPAN = ONE_HOUR * 8;

   /* Constants for scrolling to the beginning or the end of the time range. */
   static final int DONT_CHANGE  = 0;
   static final int MOST_RECENT  = 1;  /* Make the most recent data visible. */
   static final int LEAST_RECENT = 2;  /* Make the least recent data visible.*/

   static final int INIT_SPAN = 0;     /* Index of the initial span to display. */

   int SpanIndex = INIT_SPAN; /* Index of the currently displayed time span.*/

   // Number of lines and Y axes in a chart as defined in the drawing.
   int NumPlots;
   int NumYAxes;

   /* Store object IDs for each plot,  used for performance optimization 
      in the chart data feed.
   */
   GlgObject [] PlotArray;

   /* Lists that hold initial Low and High ranges of the Y axes in the drawing.
      Plots' Low and High are assumed to be linked to the corresponding Y axes.
   */
   double [] Low;
   double [] High;

   int TimeSpan; // Time axis span in sec.

   boolean PrefillData = true; /* Setting to False suppresses pre-filling the 
                                  chart's buffer with data on start-up. */
   int AutoScroll = 1;         /* Current auto-scroll state: enabled(1) or 
                                  disabled(0). */
   int StoredScrollState;      /* Stored AutoScroll state to be restored 
                                  if ZoomTo is aborted. */

   /////////////////////////////////////////////////////////////////////
   // Viewer and PageType variables are defined in and assigned by the 
   // base HMIPageBase class.
   /////////////////////////////////////////////////////////////////////
   public RTChartPage( GlgSCADAViewer viewer ) 
   {
      super( viewer );

      Viewport = Viewer.DrawingAreaVP;
   }

   /////////////////////////////////////////////////////////////////////// 
   // Returns an update interval in msec for animating drawing with data.
   /////////////////////////////////////////////////////////////////////// 
   public int GetUpdateInterval()
   {
      return 100;
   }

   /////////////////////////////////////////////////////////////////////// 
   // Perform any desired initialization of the drawing before hierarchy setup.
   /////////////////////////////////////////////////////////////////////// 
   public void InitBeforeSetup()
   {
      ChartVP = Viewport.GetResourceObject( "ChartViewport" );
      if( ChartVP == null )
        GlgObject.Error( GlgObject.USER_ERROR,
                         "Can't find ChartViewport", null );

      Chart = ChartVP.GetResourceObject( "Chart" );
      if( Chart == null )
        GlgObject.Error( GlgObject.USER_ERROR,
                         "Can't find Chart object", null );

      // Retrieve number of plots and Y axes defined in the drawing.
      NumPlots = Chart.GetDResource( "NumPlots" ).intValue();
      NumYAxes = Chart.GetDResource( "NumYAxes" ).intValue();

      // Enable AutoScroll, both for the toggle button and the chart.
      ChangeAutoScroll( 1 );

      /* Set Chart Zoom mode. It was set and saved with the drawing, 
         but do it again programmatically just in case.
      */
      ChartVP.SetZoomMode( null, Chart, null, GlgObject.CHART_ZOOM_MODE );
   }

   /////////////////////////////////////////////////////////////////////// 
   // Perform any desired initialization of the drawing after hierarchy setup.
   /////////////////////////////////////////////////////////////////////// 
   public void InitAfterSetup()
   {
      int i;

      // Store objects IDs for each plot.
      PlotArray = new GlgObject[ NumPlots ];

      GlgObject plot_array = Chart.GetResourceObject( "Plots" );
      for( i=0; i<NumPlots; ++i )
        PlotArray[i] = (GlgObject) plot_array.GetElement( i ); 

      /* Store initial range for each Y axis to restore on zoom reset. 
         Assumes that plots are linked with the corresponding axes in the 
         drawing.
      */         
      Low = new double[ NumYAxes ];
      High = new double[ NumYAxes ];

      GlgObject axis_array = Chart.GetResourceObject( "YAxisGroup" );
      for( i=0; i<NumYAxes; ++i )
      {
         GlgObject axis = (GlgObject) axis_array.GetElement( i ); 
         Low[ i ] = axis.GetDResource( "Low" ).doubleValue();
         High[ i ] = axis.GetDResource( "High" ).doubleValue();
      }
   }

   /////////////////////////////////////////////////////////////////////// 
   // Invoked when the page has been loaded and the tags have been remapped.
   /////////////////////////////////////////////////////////////////////// 
   public void Ready()
   {
      SetChartSpan( SpanIndex );

      // Prefill chart's history bufer with data. 
      if( PrefillData )
        FillChartHistory();
   }
   
   /////////////////////////////////////////////////////////////////////// 
   // Pre-fill the graph's history buffer with data. 
   /////////////////////////////////////////////////////////////////////// 
   public void FillChartHistory()
   {
      double current_time = Viewer.GetCurrTime();
      
      /* Fill the amount of data requested by the PREFILL_SPAN, up to the 
         available chart's buffer size defined in the drawing.
         Add an extra second to avoid rounding errors.
      */
      int num_seconds = PREFILL_SPAN + 1;

      int buffer_size = Chart.GetDResource( "BufferSize" ).intValue();
      if( buffer_size < 1 )
        buffer_size = 1;

      int max_num_samples;
      if( Viewer.RandomData )
      {
         /* In random demo data mode, simulate data stored once per second. */
         double samples_per_second = 1.0;
         max_num_samples = (int) ( num_seconds * samples_per_second );

         if( max_num_samples > buffer_size )
           max_num_samples = buffer_size;
      }
      else
        max_num_samples = buffer_size;

      /* Start and end time for querying data. */
      double start_time = current_time - num_seconds;
      double end_time = current_time;   /* Stop at the current time. */

      for( int i=0; i<NumPlots; ++i )
      {
         // Get tag source of the plot's ValueEntryPoint.
         String tag_source = 
           PlotArray[ i ].GetSResource( "ValueEntryPoint/TagSource" );

         ArrayList<PlotDataPoint> data_array = 
           Viewer.DataFeed.GetPlotData( tag_source, start_time, end_time,
                                        max_num_samples );
         if( data_array == null )
           continue;

         FillPlotData( PlotArray[ i ], data_array );
      }
   }

   /////////////////////////////////////////////////////////////////////// 
   // Fills plot with data from the provided data array.
   /////////////////////////////////////////////////////////////////////// 
   public void FillPlotData( GlgObject plot,
                             ArrayList<PlotDataPoint> data_array )
   {
      // Obtain object IDs of plot's data entry points for faster processing.
      GlgObject value_entry_point = plot.GetResourceObject( "ValueEntryPoint" );
      GlgObject time_entry_point = plot.GetResourceObject( "TimeEntryPoint" );
      GlgObject valid_entry_point = plot.GetResourceObject( "ValidEntryPoint" );

      int size = data_array.size();
      for( int i=0; i<size; ++i )
      {
         PlotDataPoint data_point = data_array.get( i );

         /* Using null resource name to push data directly into each
            entry point object.
         */
         value_entry_point.SetDResource( null, data_point.value );
         time_entry_point.SetDResource( null, data_point.time_stamp );
         valid_entry_point.SetDResource( null, 
                                         data_point.value_valid ? 1.0 : 0.0 );
      }      
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
        action,
        subaction;
      boolean processed = false;

      origin = message_obj.GetSResource( "Origin" );
      format = message_obj.GetSResource( "Format" );
      action = message_obj.GetSResource( "Action" );
      subaction = message_obj.GetSResource( "SubAction" );

      // Process button events.
      if( format.equals( "Button" ) )   
      {
         if( !action.equals( "Activate" ) &&      /* Not a push button */
             !action.equals( "ValueChanged" ) )   /* Not a toggle button */
           return false;
         
         AbortZoomTo();
         
         processed = true;
         if( origin.equals( "ToggleAutoScroll" ) )
         {         
            /* Set Chart AutoScroll based on the ToggleAutoScroll toggle button 
               setting.
            */
            ChangeAutoScroll( -1 ); 
         }
         else if( origin.equals( "ZoomTo" ) )
         {
            // Start ZoomTo operation.
            ChartVP.SetZoom( null, 't', 0.0 );  
         }
         else if( origin.equals( "ZoomReset" ) )
         {         
            // Set initial time span and reset initial Y ranges.
            SetChartSpan( SpanIndex );  
            RestoreInitialYRanges();   
         }
         /* Handle both the buttons on the RTChart page, as well as the zoom
            controls on the left of the top level window.
         */
         else if( origin.equals( "ScrollBack" ) || origin.equals( "Left" ) )
         {
            ChangeAutoScroll( 0 );
            
            // Scroll left by 1/3 of the span.
            ChartVP.SetZoom( null, 'l', 0.33 );
         }
         else if( origin.equals( "ScrollForward" ) || origin.equals( "Right" ) )
         {
            ChangeAutoScroll( 0 );
            
            // Scroll right by 1/3 of the span.
            ChartVP.SetZoom( null, 'r', 0.33 );
         }
         else if( origin.equals( "ScrollBack2" ) )
         {
            ChangeAutoScroll( 0 );

            // Scroll left by a full span.
            ChartVP.SetZoom( null, 'l', 1.0 );
         }
         else if( origin.equals( "ScrollForward2" ) )
         {
            ChangeAutoScroll( 0 );

            // Scroll right by a full span.
            ChartVP.SetZoom( null, 'r', 1.0 );
         }
         else if( origin.equals( "Up" ) )
         {
            // Scroll up.
            ChartVP.SetZoom( null, 'u', 0.5 );
         }
         else if( origin.equals( "Down" ) )
         {
            // Scroll down.
            ChartVP.SetZoom( null, 'd', 0.5 );
         }
         else if( origin.equals( "ZoomIn" ) )
         {
            // Zoom in in Y direction.
            ChartVP.SetZoom( null, 'I', 1.5 );
         }
         else if( origin.equals( "ZoomOut" ) )
         {
            // Zoom out in Y direction.
            ChartVP.SetZoom( null, 'O', 1.5 );
         }
         else if( origin.equals( "ScrollToRecent" ) )
         {
            // Scroll to show most recent data.
            ScrollToDataEnd( MOST_RECENT, true );
         }
         else
           processed = false;

         if( processed )
           Viewport.Update();
      }
      else if( format.equals( "Menu" ) )
      {
         if( !action.equals( "Activate" ) )
           return false;

         AbortZoomTo();

         if( origin.equals( "SpanSelector" ) )    /* Span change */
         { 
            processed = true;
            SpanIndex = message_obj.GetDResource( "SelectedIndex" ).intValue();

            SetChartSpan( SpanIndex );
            RestoreInitialYRanges(); /* Restore in case the chart was zoomed.*/

            /* Scroll to show the recent data to avoid showing an empty chart
               if user scrolls too much into the future or into the past.
               
               Invoke ScrollToDataEnd() even if AutoScroll is True to 
               scroll ahead by a few extra seconds to show a few next updates
               without scrolling the chart.
            */
            GlgMinMax min_max = Chart.GetDataExtent( null, /* x extent */ true );

            if( min_max != null )
            {
               double first_time_stamp = min_max.min;
               double last_time_stamp = min_max.max;
               double displayed_time_end = 
                 Chart.GetDResource( "XAxis/EndValue" ).doubleValue();

               if( AutoScroll != 0 )
                 ScrollToDataEnd( MOST_RECENT, true );

               else if( displayed_time_end >
                        last_time_stamp + GetExtraSeconds() )
                 ScrollToDataEnd( MOST_RECENT, true );

               else if( displayed_time_end - TimeSpan <= first_time_stamp )
                 ScrollToDataEnd( LEAST_RECENT, true );

               Viewport.Update();
            }
         }
      }
      else if( format.equals( "Chart" ) && 
               action.equals( "CrossHairUpdate" ) )
      {
         /* To avoid slowing down real-time chart updates, invoke Update() 
            to redraw cross-hair only if the chart is not updated fast 
            enough by the timer.
         */
         if( GetUpdateInterval() > 100 )
           Viewport.Update();         

         processed = true;
      }            
      else if( action.equals( "Zoom" ) )    // Zoom events
      {
         processed = true;
         if( subaction.equals( "ZoomRectangle" ) )
         {
            // Store AutoSCroll state to restore it if ZoomTo is aborted.
            StoredScrollState = AutoScroll;
            
            // Stop scrolling when ZoomTo action is started.
            ChangeAutoScroll( 0 );
         }
         else if( subaction.equals( "End" ) )
         {
            /* No additional actions on finishing ZoomTo. The Y scrollbar 
               appears automatically if needed: it is set to GLG_PAN_Y_AUTO. 
               Don't resume scrolling: it'll scroll too fast since we zoomed 
               in. Keep it still to allow inspecting zoomed data.
            */
         }
         else if( subaction.equals( "Abort" ) )
         {
            // Resume scrolling if it was on.
            ChangeAutoScroll( StoredScrollState ); 
         }
         
         Viewport.Update();
      }
      else if( action.equals( "Pan" ) )    // Pan events
      {
         processed = true;

         /* This code may be used to perform custom action when dragging the 
            chart's data with the mouse. 
         */
         if( subaction.equals("Start" ) )   // Chart dragging start
         {
         }
         else if( subaction.equals( "Drag" ) )    // Dragging
         {
         }
         else if( subaction.equals( "ValueChanged" ) )   // Scrollbars
         {
         }
         // Dragging ended or aborted.
         else if( subaction.equals( "End" ) || subaction.equals( "Abort" ) )
         {
         }     

         Viewport.Update();
      }

      // If event was processed here, return true not to process it outside.
      return processed;
   }

   ///////////////////////////////////////////////////////////////////////
   // A custom trace callback for the page; is used to obtain coordinates 
   // of the mouse click. If it returns false, the default trace callback 
   // of the SCADA Viewer will be used to process events.
   // Used to obtain coordinates of the mouse click.
   ///////////////////////////////////////////////////////////////////////
   public boolean TraceCallback( GlgObject viewport, GlgTraceData trace_info )
   {
      // Process only events that occur in ChartViewport.
      if( trace_info.viewport != ChartVP )
        return false;
      
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
         
       default: return false;
      }
      
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
         if( ZoomToMode() )
           return false; // ZoomTo or dragging mode in progress.
         
         /* Start dragging with the mouse on a mouse click. 
            If user clicked of an axis, the dragging will be activated in the
            direction of that axis. If the user clicked in the chart area,
            dragging in both the time and the Y direction will be activated. 
         */
         ChartVP.SetZoom( null, 's', 0.0 );
         
         // Disable AutoScroll not to interfere with dragging.
         ChangeAutoScroll( 0 ); 
         return true;
         
       default: break;
      }
      return false;
   }
   
   //////////////////////////////////////////////////////////////////////
   // Returns true if tag sources need to be remapped for the page.
   //////////////////////////////////////////////////////////////////////
   public boolean NeedTagRemapping()
   {
      // In demo mode, unset tags need to be remapped to enable animation.
      if( Viewer.RandomData )
        return true;
      else
        return false;   //  remap tags only if necessary.
   }

   //////////////////////////////////////////////////////////////////////
   // Reassign TagSource parameter for a given tag object to a new
   // TagSource value. tag_source and tag_name parameters are the current 
   // TagSource and TagName of the tag_obj.
   //////////////////////////////////////////////////////////////////////
   public void RemapTagObject( GlgObject tag_obj, 
                                         String tag_name, String tag_source )
   {
      String new_tag_source;

      if( Viewer.RandomData )
      {
         // Skip tags with undefined TagName.
         if( Viewer.IsUndefined( tag_name ) )
           return;
            
         /* In demo mode, assign unset tag sources to be the same as tag names
            to enable animation with demo data.
         */
         new_tag_source = tag_name;
         if( Viewer.IsUndefined( tag_source ) )
           Viewer.AssignTagSource( tag_obj, new_tag_source );
      }
      else
      {
         // Assign new TagSource as needed.
         // Viewer.AssignTagSource( tag_obj, new_tag_source );
      }
   }

   ///////////////////////////////////////////////////////////////////////
   // Change chart's AutoScroll mode.
   ///////////////////////////////////////////////////////////////////////
   public void ChangeAutoScroll( int new_value )
   {
      int auto_scroll;
      int pan_x;
      
      if( new_value == -1 )  // Use the state of the ToggleAutoScroll button.
      {
         auto_scroll = 
           Viewport.GetDResource( "Toolbar/ToggleAutoScroll/OnState" ).intValue();
         AutoScroll = auto_scroll;
      }
      else    // Set to the supplied value. 
      {
         AutoScroll = new_value;
         Viewport.SetDResource( "Toolbar/ToggleAutoScroll/OnState", 
                                (double) AutoScroll );
      }
      
      // Set chart's auto-scroll.
      Chart.SetDResource( "AutoScroll", (double) AutoScroll );
      
      /* Activate time scrollbar if AutoScroll is Off. The Y value scrollbar 
         uses GLG_PAN_Y_AUTO and appears automatically as needed.
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
      int span, major_interval, minor_interval;

      /* Change chart's time span, as well as major and minor tick intervals.*/
      switch( span_index )
      {
       default:
       case 0:
         span = ONE_MINUTE;
         major_interval = 10;  /* major tick every 10 sec. */
         minor_interval = 1;   /* minor tick every sec. */
         break;
         
       case 1:
         span = 10 * ONE_MINUTE;
         major_interval = ONE_MINUTE * 2; /* major tick every tow minutes. */
         minor_interval = 30;             /* minor tick every 30 sec. */
         break;
         
       case 2:
         span = ONE_HOUR;
         major_interval = ONE_MINUTE * 10; /* major tick every 10 min. */
         minor_interval = ONE_MINUTE;      /* minor tick every min. */
         break;

       case 3:
         span = ONE_HOUR * 8;
         major_interval = ONE_HOUR;        /* major tick every hour. */
         minor_interval = ONE_MINUTE * 15; /* minor tick every 15 minutes. */
         break;
      }

      /* Update the menu in the drawing with the initial value if different. */
      Viewport.SetDResource( "Toolbar/SpanSelector/SelectedIndex", 
                             (double)span_index, true );

      /* Set intervals before SetZoom() below to avoid redrawing huge number 
         of labels. */
      Chart.SetDResource( "XAxis/MajorInterval", (double) major_interval );
      Chart.SetDResource( "XAxis/MinorInterval", (double) minor_interval );

      /* Set the X axis span which controls how much data is displayed in the 
         chart. */
      TimeSpan = span;
      Chart.SetDResource( "XAxis/Span", (double) TimeSpan );

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
         PlotArray[0].SetDResource( "FilterType", 
                                    (double) GlgObject.MIN_MAX_FILTER );
         PlotArray[0].SetDResource( "FilterPrecision", 1.0 );
      }
      else
        PlotArray[0].SetDResource( "FilterType", 
                                   (double) GlgObject.NULL_FILTER );

      /* Change time and tooltip formatting to match the selected span. */
      SetTimeFormats();
   }

   //////////////////////////////////////////////////////////////////////////
   // Sets the formats of time labels and tooltips depending on the selected 
   // time span.
   //////////////////////////////////////////////////////////////////////////
   void SetTimeFormats()
   {
      String time_label_format, time_tooltip_format, chart_tooltip_format;

      /* No additional code is required to use the defaults defined in the 
         drawing. The code below illustrates advanced options for 
         customizing label and tooltip formatting when switching between 
         time spans and data display modes.

         For an even greater control over labels and tooltips, an application 
         can define custom Label and Tooltip formatters that will supply 
         custom strings for axis labels and tooltips.
      */

      /* Different time formats are used depending on the selected
         time span. See strftime() for all time format options.
      */
      switch( SpanIndex )
      {
       default:  /* 1 minute and 10 minutes spans */
         /* Use the preferred time and date display format for the current 
            locale. */
         time_label_format = "%X%n%x";
         break;

       case 2: /* 1 hour span */
         /* Use the 12 hour time display with no seconds, and the default 
            date display format for the current locale.
         */
         time_label_format = "%I:%M %p%n%x";
         break;

       case 3: /* 1 hour and 8 hour spans */
         /* Use 24 hour notation and don't display seconds. */
         time_label_format = "%H:%M%n%x";
         break;
      }
      Chart.SetSResource( "XAxis/TimeFormat", time_label_format );

      /* Specify axis and chart tooltip format, if different from default 
         formats defined in the drawing.
      */
      time_tooltip_format = "Time: <axis_time:%X> +0.<axis_time_ms:%03.0lf> sec.\nDate: <axis_time:%x>";

      /* <sample_time:%s> inherits time format from the X axis. */
      chart_tooltip_format = "Plot <plot_string:%s> value= <sample_y:%.2lf>\n<sample_time:%s>";

      /* Set time label and tooltip formats. */
      Chart.SetSResource( "XAxis/TooltipFormat", time_tooltip_format );
      Chart.SetSResource( "TooltipFormat", chart_tooltip_format );
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

      extra_sec = TimeSpan * 0.1;
      max_extra_sec = ( TimeSpan > ONE_HOUR ? 5.0 : 3.0 );

      if( extra_sec > max_extra_sec )
        extra_sec = max_extra_sec;

      return extra_sec;
   }

   ///////////////////////////////////////////////////////////////////////
   // Restore Y axis range to the initial Low/High values.
   ///////////////////////////////////////////////////////////////////////
   public void RestoreInitialYRanges()
   {
      GlgObject axis_array = Chart.GetResourceObject( "YAxisGroup" );
      for( int i=0; i<NumYAxes; ++i )
      {
         GlgObject axis = (GlgObject) axis_array.GetElement( i ); 
         axis.SetDResource( "Low", Low[ i ] );
         axis.SetDResource( "High", High[ i ] );
      }
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Returns True if the chart's viewport is in ZoomToMode.
   // ZoomToMode is activated on Dragging and ZoomTo operations.
   ///////////////////////////////////////////////////////////////////////
   public boolean ZoomToMode()
   {
      int zoom_mode = ChartVP.GetDResource( "ZoomToMode" ).intValue();
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
         ChartVP.SetZoom( null, 'e', 0.0 ); 
         Viewport.Update();
      }
   }
}
