#include "RTChartPage.h"
#include "DataFeedBase.h"
#include "PlotDataPoint.h"
#include "GlgSCADAViewer.h"

/* Real-Time Chart Page.

   The data in GlgSCADAViewer structure are accessible via the global Viewer 
   variable.
*/

/* Convenient time span constants. */
#define ONE_MINUTE    60
#define ONE_HOUR      3600
#define ONE_DAY       ( 3600 * 24 )

/* Prefill time interval, specifies amount of data to prefill in the 
   real time chart. */
#define PREFILL_SPAN  ( ONE_HOUR * 8 )

#define INIT_SPAN      0     /* Index of the initial span to display. */

// Constructor
RTChartPage::RTChartPage( GlgSCADAViewer * viewer ) : HMIPageBase( viewer )
{
   Viewport = Viewer->DrawingAreaVP;
  
   PlotArray = NULL;
   High = NULL;
   Low = NULL;
   SpanIndex = INIT_SPAN;  /* Index of the currently displayed time span.*/
   PrefillData = GlgTrue;  /* Setting to False suppresses pre-filling the 
                              chart's buffer with data on start-up. */
   AutoScroll = 1;         /* Current auto-scroll state: enabled(1) 
                              or disabled(0). */
}

// Destructor
RTChartPage::~RTChartPage( void )
{
   delete PlotArray;
   delete Low;
   delete High;
}

/*----------------------------------------------------------------------
| Returns an update interval in msec for animating drawing with data.
*/
int RTChartPage::GetUpdateInterval( void )
{
   return 100;
}

/*----------------------------------------------------------------------
| Perform any desired initialization of the drawing before hierarchy setup.
*/
void RTChartPage::InitBeforeSetup( void )
{
   ChartVP = Viewport.GetResourceObject( "ChartViewport" );
   if( ChartVP.IsNull() )
     GlgError( GLG_USER_ERROR, "Can't find ChartViewport" );
   
   Chart = ChartVP.GetResourceObject( "Chart" );
   if( Chart.IsNull() )
     GlgError( GLG_USER_ERROR, "Can't find Chart object" );

   /* Retrieve number of plots and Y axes defined in the drawing. */
   double num_plots, num_y_axes;
   Chart.GetResource( "NumPlots", &num_plots );
   Chart.GetResource( "NumYAxes", &num_y_axes );
   NumPlots = num_plots;
   NumYAxes = num_y_axes;

   /* Enable AutoScroll, both for the toggle button and the chart. */
   ChangeAutoScroll( 1 );

   /* Set Chart Zoom mode. It was set and saved with the drawing, 
      but do it again programmatically just in case.
   */
   ChartVP.SetZoomMode( NULL, &Chart, NULL, GLG_CHART_ZOOM_MODE );
}

/*----------------------------------------------------------------------
| Perform any desired initialization of the drawing after hierarchy setup.
*/
void RTChartPage::InitAfterSetup( void )
{
   GlgObjectC 
     plot_array,
     axis_array,
     axis;

   /* Store objects IDs for each plot */
   PlotArray = new GlgObjectC[ NumPlots ];

   plot_array = Chart.GetResourceObject( "Plots" );
   for( int i=0; i < NumPlots; ++i )
      PlotArray[i] = plot_array.GetElement( i ); 

   /* Store initial range for each Y axis to restore on zoom reset. 
      Assumes that plots are linked with the corresponding axes in the 
      drawing.
   */         
   Low = new double[ NumYAxes ];
   High = new double[ NumYAxes ];

   axis_array = Chart.GetResourceObject( "YAxisGroup" );
   for( int i=0; i<NumYAxes; ++i )
   {
      axis = axis_array.GetElement( i ); 
      axis.GetResource( "Low", &Low[ i ] );
      axis.GetResource( "High", &High[ i ] );
   }
}

/*----------------------------------------------------------------------
| Invoked when the page has been loaded and the tags have been remapped.
*/
void RTChartPage::Ready( void )
{
   SetChartSpan( SpanIndex );
   
   /* Prefill chart's history bufer with data. */
   if( PrefillData )
     FillChartHistory();
}

/*----------------------------------------------------------------------
| Pre-fill the graph's history buffer with data. 
*/
void RTChartPage::FillChartHistory( void )
{
   double current_time = GetCurrTime();

   /* Fill the amount of data requested by the PREFILL_SPAN, up to the 
      available chart's buffer size defined in the drawing.
      Add an extra second to avoid rounding errors.
   */
   int num_seconds = PREFILL_SPAN + 1;

   double dval;
   Chart.GetResource( "BufferSize", &dval );

   int buffer_size = (int) dval;
   if( buffer_size < 1 )
     buffer_size = 1;

   int max_num_samples;
   if( Viewer->RandomData )
   {
      /* In random demo data mode, simulate data stored once per second. */
      double samples_per_second = 1.0;
      max_num_samples = num_seconds * samples_per_second;
      
      if( max_num_samples > buffer_size )
        max_num_samples = buffer_size;
   }
   else
     max_num_samples = buffer_size;
   
   /* Start and end time for querying data. */
   double start_time = current_time - num_seconds;
   double end_time = current_time;   /* Stop at the current time. */

   for( int i=0; i < NumPlots; ++i )
   {
      SCONST char * tag_source = NULL;

      PlotDataArrayType data_array;
      /* Get tag source of the plot's ValueEntryPoint. */
      PlotArray[ i ].GetResource( "ValueEntryPoint/TagSource", &tag_source );
      Viewer->DataFeed->GetPlotData( tag_source, start_time, end_time, 
                                    max_num_samples, data_array );
      if( data_array.empty() )
        continue;
      
      FillPlotData( PlotArray[ i ], data_array );
      Viewer->DataFeed->FreePlotData( data_array );
   }
}

/*----------------------------------------------------------------------
| Fills plot with data from the provided data array.
*/
void RTChartPage::FillPlotData( GlgObjectC& plot, 
                                PlotDataArrayType &data_array )
{
   GlgObjectC 
     value_entry_point,
     time_entry_point,
     valid_entry_point;

   /* Obtain object IDs of plot's data entry points for faster processing. */
   value_entry_point = plot.GetResourceObject( "ValueEntryPoint" );
   time_entry_point = plot.GetResourceObject( "TimeEntryPoint" );
   valid_entry_point = plot.GetResourceObject( "ValidEntryPoint" );

   PlotDataArrayType::iterator it;
   for( it = data_array.begin(); it != data_array.end(); ++it )
   {
      PlotDataPoint * data_point = *it;

      /* Using NULL resource name to push data directly into each
         entry point object.
      */
      value_entry_point.SetResource( NULL, data_point->value );
      time_entry_point.SetResource( NULL, data_point->time_stamp );
      valid_entry_point.SetResource( NULL, 
                                     data_point->value_valid ? 1.0 : 0.0 );
   }
}

/*----------------------------------------------------------------------
| A custom UpdateData method to supply chart data. If returns false,
| the default Viewer's UpdateData method is used for animation;
| otherwise, the custom UpdateData method of this page is used
| to obtain and push data values into the page.
|
| The default Viewer's UpdateData method uses tags for animation.
| In this demo, tags are assigned to the plots' ValueEntryPoint
| resource, while TimeEntryPoint does not have a tag and the time
| stamp for the X axis is supplied automatically using current time.
| To supply application specific time stamp for the X axis,
| provide a custom implementation of the page's UpdateData method.
*/
GlgBoolean RTChartPage::UpdateData( void )
{
   if( Viewer->RandomData )
     /* Return false to let the viewer update tags in the drawing from 
        the process database. In this demo, tags are assigned to 
        the plots' ValueEntryPoint resource. 
     */
     return GlgFalse;
   else
   {
      /* Place custom code here as needed to override the
         Viewer's UpdateData method.
      */
      return GlgTrue;
   }
}

/*----------------------------------------------------------------------
| A custom input handler for the page. If it returns false, the default
| input handler of the SCADA Viewer will be used to process common
| events and commands.
*/
GlgBoolean RTChartPage::Input( GlgObjectC& viewport, GlgObjectC& message )
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
      if( strcmp( action, "Activate" ) != 0 &&      /* Not a push button */
          strcmp( action, "ValueChanged" ) != 0 )   /* Not a toggle button */
        return GlgFalse;
      
      AbortZoomTo();
      
      processed = GlgTrue;
      if( strcmp( origin, "ToggleAutoScroll" ) == 0 )
      {         
         /* Set Chart AutoScroll based on the ToggleAutoScroll toggle button
            setting.
         */
         ChangeAutoScroll( -1 ); 
      }
      else if( strcmp( origin, "ZoomTo" ) == 0 )
      {
         /* Start ZoomTo operation. */
         ChartVP.SetZoom( NULL, 't', 0. );  
      }
      else if( strcmp( origin, "ZoomReset" ) == 0 )
      {         
         /* Set initial time span and reset initial Y ranges. */
         SetChartSpan( SpanIndex );  
         RestoreInitialYRanges();   
      }
      /* Handle both the buttons on the RTChart page, as well as the zoom
         controls on the left of the top level window.
      */
      else if( strcmp( origin, "ScrollBack" ) == 0 || 
               strcmp( origin, "Left" ) == 0 )
      {
         ChangeAutoScroll( 0 );
         
         /* Scroll left by 1/3 of the span. */
         ChartVP.SetZoom( NULL, 'l', 0.33 );
      }
      else if( strcmp( origin, "ScrollForward" ) == 0 || 
               strcmp( origin, "Right" ) == 0 )
      {
         ChangeAutoScroll( 0 );
            
         /* Scroll right by 1/3 of the span. */
         ChartVP.SetZoom( NULL, 'r', 0.33 );
      }
      else if( strcmp( origin, "ScrollBack2" ) == 0 )
      {
         ChangeAutoScroll( 0 );
         
         /* Scroll left by a full span. */
         ChartVP.SetZoom( NULL, 'l', 1. );
      }
      else if( strcmp( origin, "ScrollForward2" ) == 0 )
      {
         ChangeAutoScroll( 0 );
         
         /* Scroll right by a full span. */
         ChartVP.SetZoom( NULL, 'r', 1. );
      }
      else if( strcmp( origin, "Up" ) == 0 )
      {
         /* Scroll up. */
         ChartVP.SetZoom( NULL, 'u', 0.5 );
      }
      else if( strcmp( origin, "Down" ) == 0 )
      {
         /* Scroll down. */
         ChartVP.SetZoom( NULL, 'd', 0.5 );
      }
      else if( strcmp( origin, "ZoomIn" ) == 0 )
      {
         /* Zoom in in Y direction. */
         ChartVP.SetZoom( NULL, 'I', 1.5 );
      }
      else if( strcmp( origin, "ZoomOut" ) == 0 )
      {
         /* Zoom out in Y direction. */
         ChartVP.SetZoom( NULL, 'O', 1.5 );
      }
      else if( strcmp( origin, "ScrollToRecent" ) == 0 )
      {
         /* Scroll to show most recent data. */
         ScrollToDataEnd( MOST_RECENT, GlgTrue );
      }
      else
        processed = GlgFalse;
      
      if( processed )
        Viewport.Update();
   }
   else if( strcmp( format, "Menu" ) == 0 )
   {
      if( strcmp( action, "Activate" ) != 0 )
        return GlgFalse;
      
      AbortZoomTo();
      
      if( strcmp( origin, "SpanSelector" ) == 0 )    /* Span change */
      { 
         GlgMinMax min_max;
         double selected_index;

         processed = GlgTrue;
         message.GetResource( "SelectedIndex", &selected_index );
         SpanIndex = selected_index;

         SetChartSpan( SpanIndex );

         /* Restore in case the chart was zoomed.*/
         RestoreInitialYRanges();

         /* Scroll to show the recent data to avoid showing an empty chart
            if user scrolls too much into the future or into the past.
            
            Invoke ScrollToDataEnd() even if AutoScroll is GlgTrue to 
            scroll ahead by a few extra seconds to show a few next updates
            without scrolling the chart.
         */
         if( Chart.GetDataExtent( NULL, &min_max, /* x extent */ GlgTrue ) )
         {
            double
              first_time_stamp,
              last_time_stamp,
              displayed_time_end;

            first_time_stamp = min_max.min;
            last_time_stamp = min_max.max;            
            Chart.GetResource( "XAxis/EndValue", &displayed_time_end );

            if( AutoScroll != 0 )
              ScrollToDataEnd( MOST_RECENT, GlgTrue );

            else if( displayed_time_end >
                     last_time_stamp + GetExtraSeconds( TimeSpan ) )
              ScrollToDataEnd( MOST_RECENT, GlgTrue );

            else if( displayed_time_end - TimeSpan <= 
                     first_time_stamp )
              ScrollToDataEnd( LEAST_RECENT, GlgTrue );

            Viewport.Update();
         }
      }
   }
   else if( strcmp( format, "Chart" ) == 0 && 
            strcmp( action, "CrossHairUpdate" ) == 0 )
   {
      /* To avoid slowing down real-time chart updates, invoke Update() 
         to redraw cross-hair only if the chart is not updated fast 
         enough by the timer.
      */
      if( GetUpdateInterval() > 100 )
        Viewport.Update();

      processed = GlgTrue;
   }            
   else if( strcmp( action, "Zoom" ) == 0 )    /* Zoom events */
   {
      processed = GlgTrue;
      if( strcmp( subaction, "ZoomRectangle" ) == 0 )
      {
         /* Store AutoSCroll state to restore it if ZoomTo is aborted. */
         StoredScrollState = AutoScroll;
         
         /* Stop scrolling when ZoomTo action is started. */
         ChangeAutoScroll( 0 );
      }
      else if( strcmp( subaction, "End" ) == 0 )
      {
         /* No additional actions on finishing ZoomTo. The Y scrollbar 
            appears automatically if needed: it is set to GLG_PAN_Y_AUTO. 
            Don't resume scrolling: it'll scroll too fast since we zoomed 
            in. Keep it still to allow inspecting zoomed data.
         */
      }
      else if( strcmp( subaction, "Abort" ) == 0 )
      {
         /* Resume scrolling if it was on. */
         ChangeAutoScroll( StoredScrollState ); 
      }
      
      Viewport.Update();
   }
   else if( strcmp( action, "Pan" ) == 0 )    /* Pan events */
   {
      processed = GlgTrue;
      
      /* This code may be used to perform custom action when dragging the 
         chart's data with the mouse. 
      */
      if( strcmp( subaction, "Start" ) )   /* Chart dragging start */
      {
      }
      else if( strcmp( subaction, "Drag" ) == 0 )    /* Dragging */
      {
      }
      else if( strcmp( subaction, "ValueChanged" ) == 0 )   /* Scrollbars */
      {
      }
      /* Dragging ended or aborted. */
      else if( strcmp( subaction, "End" ) == 0 || 
               strcmp( subaction, "Abort" ) == 0 )
      {
      }     

      Viewport.Update();
   }

   /* If event was processed here, return GlgTrue not to process it outside. */
   return processed;
}

/*----------------------------------------------------------------------
| A custom trace callback for the page; is used to obtain coordinates 
| of the mouse click. If it returns false, the default trace callback 
| of the SCADA Viewer will be used to process events.
| Used to obtain coordinates of the mouse click.
*/
GlgBoolean RTChartPage::Trace( GlgObjectC& viewport, 
                               GlgTraceCBStruct * trace_data )
{
   EventType event_type;
   int button = 0;
   double x, y;

   GlgObjectC event_vp;
   event_vp = trace_data->viewport;

   /* Process only events that occur in ChartViewport. */
   if( !event_vp.Same( ChartVP ) )
     return GlgFalse;
      
   /* Platform-specific code to extract event information.
      GLG_COORD_MAPPING_ADJ is added to the cursor coordinates for precise
      pixel mapping.
   */
#ifndef _WINDOWS
   switch( trace_data->event->type )
   {
    case ButtonPress:
      x = trace_data->event->xbutton.x + GLG_COORD_MAPPING_ADJ;
      y = trace_data->event->xbutton.y + GLG_COORD_MAPPING_ADJ;
      button = trace_data->event->xbutton.button;
      event_type = BUTTON_PRESS_EVENT;
      break;
      
    case MotionNotify:
      x = trace_data->event->xmotion.x + GLG_COORD_MAPPING_ADJ;
      y = trace_data->event->xmotion.y + GLG_COORD_MAPPING_ADJ;
      event_type = MOUSE_MOVE_EVENT;
      break;

    default: return GlgFalse;
   }
#else
   switch( trace_data->event->message )
   {
    case WM_LBUTTONDOWN: button = 1; goto button_event;
    case WM_MBUTTONDOWN: button = 2; goto button_event;
    case WM_RBUTTONDOWN: button = 3; goto button_event;

    button_event:
    case WM_MOUSEMOVE:
      x = LOWORD( trace_data->event->lParam ) + GLG_COORD_MAPPING_ADJ;
      y = HIWORD( trace_data->event->lParam ) + GLG_COORD_MAPPING_ADJ;
      switch( trace_data->event->message )
      {
       case WM_LBUTTONDOWN: event_type = BUTTON_PRESS_EVENT; break;
       case WM_MOUSEMOVE: event_type = MOUSE_MOVE_EVENT; break;
      } 
      break;
      
    default: return GlgFalse;
   }
#endif
   
   switch( event_type )
   {
    case BUTTON_PRESS_EVENT:
      if( ZoomToMode() )
        return GlgFalse; // ZoomTo or dragging mode in progress.
         
      /* Start dragging with the mouse on a mouse click. 
         If user clicked of an axis, the dragging will be activated in the
         direction of that axis. If the user clicked in the chart area,
         dragging in both the time and the Y direction will be activated.
      */

      ChartVP.SetZoom( NULL, 's', 0. );
         
      /* Disable AutoScroll not to interfere with dragging. */
      ChangeAutoScroll( 0 ); 
      return GlgTrue;
         
    default: break;
   }
   return GlgFalse;
}
   
/*----------------------------------------------------------------------
| Returns GlgTrue if tag sources need to be remapped for the page.
*/
GlgBoolean RTChartPage::NeedTagRemapping( void )
{
   /* In demo mode, unset tags need to be remapped to enable animation. */
   if( Viewer->RandomData )
     return GlgTrue;
   else
     return GlgFalse;   /* Remap tags only if necessary. */
}

/*----------------------------------------------------------------------
| Reassign TagSource parameter for a given tag object to a new
| TagSource value. tag_source and tag_name parameters are the current 
| TagSource and TagName of the tag_obj.
*/
void RTChartPage::RemapTagObject( GlgObjectC& tag_obj, 
                                  SCONST char * tag_name, 
                                  SCONST char * tag_source )
{
   SCONST char * new_tag_source;

   if( Viewer->RandomData )
   {
      /* Skip tags with undefined TagName. */
      if( IsUndefined( tag_name ) )
        return;
            
      /* In demo mode, assign unset tag sources to be the same as tag names
         to enable animation with demo data.
      */
      new_tag_source = tag_name;
      if( IsUndefined( tag_source ) )
        Viewer->AssignTagSource( tag_obj, new_tag_source );
   }
   else
   {
#if 0
      /* Assign new TagSource as needed. */
      Viewer->AssignTagSource( tag_obj, new_tag_source );
#endif
   }
}

/*----------------------------------------------------------------------
| Change chart's AutoScroll mode.
*/
void RTChartPage::ChangeAutoScroll( int new_value )
{
   double auto_scroll;
       
   if( new_value == -1 )  /* Use the state of the ToggleAutoScroll button. */
   {
      Viewport.GetResource( "Toolbar/ToggleAutoScroll/OnState", &auto_scroll );
      AutoScroll = auto_scroll;
   }
   else    /* Set to the supplied value. */
   {
      AutoScroll = new_value;
      Viewport.SetResource( "Toolbar/ToggleAutoScroll/OnState", 
                            (double) AutoScroll );
   }
      
   /* Set chart's auto-scroll. */
   Chart.SetResource( "AutoScroll", (double) AutoScroll );
      
   /* Activate time scrollbar if AutoScroll is Off. The Y value scrollbar 
      uses GLG_PAN_Y_AUTO and appears automatically as needed.
   */
   GlgPanType pan_x = ( AutoScroll != 0 ? GLG_NO_PAN : GLG_PAN_X );
   ChartVP.SetResource( "Pan", (double) ( pan_x | GLG_PAN_Y_AUTO ) );
}
   
/*----------------------------------------------------------------------
| Changes the time span shown in the graph, adjusts major and minor tick 
| intervals to match the time span.
*/
void RTChartPage::SetChartSpan( int span_index )
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
   Viewport.SetResource( "Toolbar/SpanSelector/SelectedIndex", 
                         (double) span_index, GlgTrue );
   
   /* Set intervals before SetZoom() below to avoid redrawing huge number 
      of labels. */
   Chart.SetResource( "XAxis/MajorInterval", (double) major_interval );
   Chart.SetResource( "XAxis/MinorInterval", (double) minor_interval );
   
   /* Set the X axis span which controls how much data is displayed in the 
      chart. */
   TimeSpan = span;
   Chart.SetResource( "XAxis/Span", (double) TimeSpan );

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
      PlotArray[0].SetResource( "FilterType", (double) GLG_MIN_MAX_FILTER );
      PlotArray[0].SetResource( "FilterPrecision", 1. );
   }
   else
     PlotArray[0].SetResource( "FilterType", (double) GLG_NULL_FILTER );

   /* Change time and tooltip formatting to match the selected span. */
   SetTimeFormats();
}

/*----------------------------------------------------------------------
| Sets the formats of time labels and tooltips depending on the selected 
| time span.
*/
void RTChartPage::SetTimeFormats()
{
   SCONST char 
     * time_label_format, 
     * time_tooltip_format,
     * chart_tooltip_format;

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
   Chart.SetResource( "XAxis/TimeFormat", time_label_format );
   
   /* Specify axis and chart tooltip format, if different from default 
      formats defined in the drawing.
   */
   time_tooltip_format = 
   "Time: <axis_time:%X> +0.<axis_time_ms:%03.0lf> sec.\nDate: <axis_time:%x>";
   
   /* <sample_time:%s> inherits time format from the X axis. */
   chart_tooltip_format = "Plot <plot_string:%s> value= <sample_y:%.2lf>\n<sample_time:%s>";

   /* Set time label and tooltip formats. */
   Chart.SetResource( "XAxis/TooltipFormat", time_tooltip_format );
   Chart.SetResource( "TooltipFormat", chart_tooltip_format );
}
   
/*----------------------------------------------------------------------
| Scrolls the graph to the minimum or maximum time stamp to show the 
| most recent or the least recent data. If show_extra is True, adds a 
| few extra seconds in the real-time mode to show a few next updates
| without scrolling the chart.
|
| Enabling AutoScroll automatically scrolls to show current data points 
| when the new time stamp is more recent then the EndValue of the axis, 
| but it is not the case when the chart is scrolled into the future 
| (to the right) - still need to invoke this method.
*/
void RTChartPage::ScrollToDataEnd( int data_end, GlgBoolean show_extra )
{
   GlgMinMax min_max;
   double end_value, extra_sec;
      
   if( data_end == DONT_CHANGE )
     return;

   /* Get the min and max time stamp. */
   if( !Chart.GetDataExtent( NULL, &min_max, /* x extent */ GlgTrue ) )
     return;

   if( show_extra ) 
     extra_sec = GetExtraSeconds( TimeSpan );
   else
     extra_sec = 0.0;

   if( data_end == MOST_RECENT )
     end_value = min_max.max + extra_sec;
   else   /* LEAST_RECENT */
     end_value = min_max.min - extra_sec + TimeSpan ;

   Chart.SetResource( "XAxis/EndValue", end_value );
}

/*----------------------------------------------------------------------
| Determines a good number of extra seconds to be added at the end in
| the real-time mode to show a few next updates without scrolling the
| chart.
*/
double RTChartPage::GetExtraSeconds( int time_span )
{
   double extra_sec, max_extra_sec;

   extra_sec = time_span * 0.1;
   max_extra_sec = ( time_span > ONE_HOUR ? 5. : 3. );

   if( extra_sec > max_extra_sec )
     extra_sec = max_extra_sec;
   
   return extra_sec;
}

/*----------------------------------------------------------------------
| Restore Y axis range to the initial Low/High values.
*/
void RTChartPage::RestoreInitialYRanges( void )
{
   GlgObjectC 
     axis_array,
     axis;

   axis_array = Chart.GetResourceObject( "YAxisGroup" );
   for( int i=0; i<NumYAxes; ++i )
   {
      axis = axis_array.GetElement( i ); 
      axis.SetResource( "Low", Low[ i ] );
      axis.SetResource( "High", High[ i ] );
   }
}
   
/*----------------------------------------------------------------------
| Returns True if the chart's viewport is in ZoomToMode.
| ZoomToMode is activated on Dragging and ZoomTo operations.
*/
GlgBoolean RTChartPage::ZoomToMode( void )
{
   double zoom_mode;

   ChartVP.GetResource( "ZoomToMode", &zoom_mode );
   return zoom_mode != 0.;
}
   
/*----------------------------------------------------------------------
| Abort ZoomTo mode.
*/
void RTChartPage::AbortZoomTo( void )
{
   if( ZoomToMode() )
   {
      // Abort zoom mode in progress.
      ChartVP.SetZoom( NULL, 'e', 0. ); 
      Viewport.Update();
   }
}

