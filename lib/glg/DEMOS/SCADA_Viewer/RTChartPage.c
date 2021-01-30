#include "RTChartPage.h"
#include "scada.h"
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

/*----------------------------------------------------------------------*/
HMIPage * CreateRTChartPage()
{
   RTChartPage * rtc = GlgAllocStruct( sizeof( RTChartPage ) );
   rtc->HMIPage.type = HMI_PAGE_TYPE;
   
   rtc->HMIPage.Destroy = rtcDestroy;
   rtc->HMIPage.GetUpdateInterval = rtcGetUpdateInterval;
   rtc->HMIPage.NeedTagRemapping = rtcNeedTagRemapping;
   rtc->HMIPage.RemapTagObject = rtcRemapTagObject;
   rtc->HMIPage.InitBeforeSetup = rtcInitBeforeSetup;
   rtc->HMIPage.InitAfterSetup = rtcInitAfterSetup;
   rtc->HMIPage.InputCB = rtcInputCB;
   rtc->HMIPage.TraceCB = rtcTraceCB;
   rtc->HMIPage.Ready = rtcReady;

   /* The rest of not used function pointers were initialized to NULL. */

   /* Initial values. */

   rtc->SpanIndex = INIT_SPAN;  /* Index of the currently displayed time span.*/

   rtc->PrefillData = GlgTrue;  /* Setting to False suppresses pre-filling the 
                                   chart's buffer with data on start-up. */
   rtc->AutoScroll = 1;         /* Current auto-scroll state: enabled(1) or 
                                   disabled(0). */
   return (HMIPage*) rtc;
}

/*----------------------------------------------------------------------
| Frees resources used by the page.
*/
static void rtcDestroy( HMIPage * hmi_page )
{
   RTChartPage * rtc_page = (RTChartPage*) hmi_page;
   int i;

   /* The hmi_page itself is freed with GlgFree() by the superclass 
      automatically. Free or dereference only the additional data stored
      by the page.     
   */

   GlgDropObject( rtc_page->Viewport );
   GlgDropObject( rtc_page->ChartVP );
   GlgDropObject( rtc_page->Chart );

   for( i=0; i < rtc_page->NumPlots; ++i )
     GlgDropObject( rtc_page->PlotArray[ i ] );
   GlgFree( rtc_page->PlotArray );

   GlgFree( rtc_page->Low );
   GlgFree( rtc_page->High );
}

/*----------------------------------------------------------------------
| Returns an update interval in msec for animating drawing with data.
*/
static int rtcGetUpdateInterval( HMIPage * hmi_page )
{
   return 100;
}

/*----------------------------------------------------------------------
| Perform any desired initialization of the drawing before hierarchy setup.
*/
static void rtcInitBeforeSetup( HMIPage * hmi_page )
{
   RTChartPage * rtc_page = (RTChartPage*) hmi_page;
   double num_plots, num_y_axes;

   rtc_page->Viewport = Viewer.DrawingAreaVP;
   GlgReferenceObject( rtc_page->Viewport );

   rtc_page->ChartVP = 
     GlgGetResourceObject( rtc_page->Viewport, "ChartViewport" );
   GlgReferenceObject( rtc_page->ChartVP );
   if( !rtc_page->ChartVP )
     GlgError( GLG_USER_ERROR, "Can't find ChartViewport" );

   rtc_page->Chart = GlgGetResourceObject( rtc_page->ChartVP, "Chart" );
   GlgReferenceObject( rtc_page->Chart );
   if( !rtc_page->Chart )
     GlgError( GLG_USER_ERROR, "Can't find Chart object" );

   /* Retrieve number of plots and Y axes defined in the drawing. */
   GlgGetDResource( rtc_page->Chart, "NumPlots", &num_plots );
   GlgGetDResource( rtc_page->Chart, "NumYAxes", &num_y_axes );
   rtc_page->NumPlots = num_plots;
   rtc_page->NumYAxes = num_y_axes;

   /* Enable AutoScroll, both for the toggle button and the chart. */
   rtcChangeAutoScroll( rtc_page, 1 );

   /* Set Chart Zoom mode. It was set and saved with the drawing, 
      but do it again programmatically just in case.
   */
   GlgSetZoomMode( rtc_page->ChartVP, NULL, rtc_page->Chart, NULL, 
                   GLG_CHART_ZOOM_MODE );
}

/*----------------------------------------------------------------------
| Perform any desired initialization of the drawing after hierarchy setup.
*/
static void rtcInitAfterSetup( HMIPage * hmi_page )
{
   RTChartPage * rtc_page = (RTChartPage*) hmi_page;
   GlgObject 
     plot_array, axis_array,
     plot, axis;
   int i;

   /* Store objects IDs for each plot */
   rtc_page->PlotArray = GlgAlloc(  sizeof( GlgObject ) * rtc_page->NumPlots );

   plot_array = GlgGetResourceObject( rtc_page->Chart, "Plots" );
   for( i=0; i < rtc_page->NumPlots; ++i )
   {
      plot = GlgGetElement( plot_array, i ); 
      rtc_page->PlotArray[i] = GlgReferenceObject( plot );
   }

   /* Store initial range for each Y axis to restore on zoom reset. 
      Assumes that plots are linked with the corresponding axes in the 
      drawing.
   */         
   rtc_page->Low = GlgAlloc( sizeof( double ) * rtc_page->NumYAxes );
   rtc_page->High = GlgAlloc( sizeof( double ) * rtc_page->NumYAxes );

   axis_array = GlgGetResourceObject( rtc_page->Chart, "YAxisGroup" );
   for( i=0; i < rtc_page->NumYAxes; ++i )
   {
      axis = GlgGetElement( axis_array, i ); 
      
      GlgGetDResource( axis, "Low", &rtc_page->Low[ i ] );
      GlgGetDResource( axis, "High", &rtc_page->High[ i ] );
   }
}

/*----------------------------------------------------------------------
| Invoked when the page has been loaded and the tags have been remapped.
*/
static void rtcReady( HMIPage * hmi_page )
{
   RTChartPage * rtc_page = (RTChartPage*) hmi_page;
   
   rtcSetChartSpan( rtc_page, rtc_page->SpanIndex );
   
   /* Prefill chart's history bufer with data. */
   if( rtc_page->PrefillData )
     rtcFillChartHistory( rtc_page );
}

/*----------------------------------------------------------------------
| Pre-fill the graph's history buffer with data. 
*/
static void rtcFillChartHistory( RTChartPage * rtc_page )
{
   double
     current_time,
     start_time, end_time,
     dval;
   int 
     i,
     num_seconds,
     buffer_size,
     max_num_samples;

   current_time = GetCurrTime();

   /* Fill the amount of data requested by the PREFILL_SPAN, up to the 
      available chart's buffer size defined in the drawing.
      Add an extra second to avoid rounding errors.
   */
   num_seconds = PREFILL_SPAN + 1;
   
   GlgGetDResource( rtc_page->Chart, "BufferSize", &dval );
   buffer_size =dval;
   if( buffer_size < 1 )
     buffer_size = 1;

   if( Viewer.RandomData )
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
   start_time = current_time - num_seconds;
   end_time = current_time;   /* Stop at the current time. */

   for( i=0; i < rtc_page->NumPlots; ++i )
   {
      char * tag_source = NULL;
      GlgObject data_array;

      /* Get tag source of the plot's ValueEntryPoint. */
      GlgGetSResource( rtc_page->PlotArray[ i ], "ValueEntryPoint/TagSource",
                       &tag_source );

      data_array = DF_GetPlotData( Viewer.DataFeedPtr, tag_source, 
                                   start_time, end_time, max_num_samples );
      if( !data_array )
        continue;
      
      rtcFillPlotData( rtc_page, rtc_page->PlotArray[ i ], data_array );

      DF_FreePlotData( Viewer.DataFeedPtr, data_array );
   }
}

/*----------------------------------------------------------------------
| Fills plot with data from the provided data array.
*/
static void rtcFillPlotData( RTChartPage * rtc_page, GlgObject plot,
                             GlgObject data_array )
{
   GlgObject
     value_entry_point,
     time_entry_point,
     valid_entry_point;
   PlotDataPoint * data_point;
   int i, array_size;

   /* Obtain object IDs of plot's data entry points for faster processing. */
   value_entry_point = GlgGetResourceObject( plot, "ValueEntryPoint" );
   time_entry_point = GlgGetResourceObject( plot, "TimeEntryPoint" );
   valid_entry_point = GlgGetResourceObject( plot, "ValidEntryPoint" );
   
   array_size = GlgArrayGetSize( data_array );
   for( i=0; i<array_size; ++i )
   {
      data_point = (PlotDataPoint*) GlgArrayGetElement( data_array, i );

      /* Using NULL resource name to push data directly into each
         entry point object.
      */
      GlgSetDResource( value_entry_point, NULL, data_point->value );
      GlgSetDResource( time_entry_point, NULL, data_point->time_stamp );
      GlgSetDResource( valid_entry_point, NULL, 
                       data_point->value_valid ? 1.0 : 0.0 );
   }
}
   
/*----------------------------------------------------------------------
| A custom input handler for the page. If it returns false, the default
| input handler of the SCADA Viewer will be used to process common
| events and commands.
*/
static GlgBoolean rtcInputCB( HMIPage * hmi_page, GlgObject viewport, 
                              GlgAnyType client_data, GlgAnyType call_data )

{
   RTChartPage * rtc_page = (RTChartPage*) hmi_page;
   GlgObject message_obj = (GlgObject) call_data;
   char 
     * origin,
     * format,
     * action,
     * subaction;
   GlgBoolean processed = GlgFalse;

   GlgGetSResource( message_obj, "Origin", &origin );
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "SubAction", &subaction );

   /* Process button events. */
   if( strcmp( format, "Button" ) == 0 )   
   {
      if( strcmp( action, "Activate" ) != 0 &&      /* Not a push button */
          strcmp( action, "ValueChanged" ) != 0 )   /* Not a toggle button */
        return GlgFalse;
      
      rtcAbortZoomTo( rtc_page );
      
      processed = GlgTrue;
      if( strcmp( origin, "ToggleAutoScroll" ) == 0 )
      {         
         /* Set Chart AutoScroll based on the ToggleAutoScroll toggle button
            setting.
         */
         rtcChangeAutoScroll( rtc_page, -1 ); 
      }
      else if( strcmp( origin, "ZoomTo" ) == 0 )
      {
         /* Start ZoomTo operation. */
         GlgSetZoom( rtc_page->ChartVP, NULL, 't', 0. );  
      }
      else if( strcmp( origin, "ZoomReset" ) == 0 )
      {         
         /* Set initial time span and reset initial Y ranges. */
         rtcSetChartSpan( rtc_page, rtc_page->SpanIndex );  
         rtcRestoreInitialYRanges( rtc_page );   
      }
      /* Handle both the buttons on the RTChart page, as well as the zoom
         controls on the left of the top level window.
      */
      else if( strcmp( origin, "ScrollBack" ) == 0 || 
               strcmp( origin, "Left" ) == 0 )
      {
         rtcChangeAutoScroll( rtc_page, 0 );
         
         /* Scroll left by 1/3 of the span. */
         GlgSetZoom( rtc_page->ChartVP, NULL, 'l', 0.33 );
      }
      else if( strcmp( origin, "ScrollForward" ) == 0 || 
               strcmp( origin, "Right" ) == 0 )
      {
         rtcChangeAutoScroll( rtc_page, 0 );
            
         /* Scroll right by 1/3 of the span. */
         GlgSetZoom( rtc_page->ChartVP, NULL, 'r', 0.33 );
      }
      else if( strcmp( origin, "ScrollBack2" ) == 0 )
      {
         rtcChangeAutoScroll( rtc_page, 0 );
         
         /* Scroll left by a full span. */
         GlgSetZoom( rtc_page->ChartVP, NULL, 'l', 1. );
      }
      else if( strcmp( origin, "ScrollForward2" ) == 0 )
      {
         rtcChangeAutoScroll( rtc_page, 0 );
         
         /* Scroll right by a full span. */
         GlgSetZoom( rtc_page->ChartVP, NULL, 'r', 1. );
      }
      else if( strcmp( origin, "Up" ) == 0 )
      {
         /* Scroll up. */
         GlgSetZoom( rtc_page->ChartVP, NULL, 'u', 0.5 );
      }
      else if( strcmp( origin, "Down" ) == 0 )
      {
         /* Scroll down. */
         GlgSetZoom( rtc_page->ChartVP, NULL, 'd', 0.5 );
      }
      else if( strcmp( origin, "ZoomIn" ) == 0 )
      {
         /* Zoom in in Y direction. */
         GlgSetZoom( rtc_page->ChartVP, NULL, 'I', 1.5 );
      }
      else if( strcmp( origin, "ZoomOut" ) == 0 )
      {
         /* Zoom out in Y direction. */
         GlgSetZoom( rtc_page->ChartVP, NULL, 'O', 1.5 );
      }
      else if( strcmp( origin, "ScrollToRecent" ) == 0 )
      {
         /* Scroll to show most recent data. */
         rtcScrollToDataEnd( rtc_page, MOST_RECENT, GlgTrue );
      }
      else
        processed = GlgFalse;
      
      if( processed )
        GlgUpdate( rtc_page->Viewport );
   }
   else if( strcmp( format, "Menu" ) == 0 )
   {
      if( strcmp( action, "Activate" ) != 0 )
        return GlgFalse;
      
      rtcAbortZoomTo( rtc_page );
      
      if( strcmp( origin, "SpanSelector" ) == 0 )    /* Span change */
      { 
         GlgMinMax min_max;
         double selected_index;

         processed = GlgTrue;
         GlgGetDResource( message_obj, "SelectedIndex", &selected_index );
         rtc_page->SpanIndex = selected_index;

         rtcSetChartSpan( rtc_page, rtc_page->SpanIndex );

         /* Restore in case the chart was zoomed.*/
         rtcRestoreInitialYRanges( rtc_page );

         /* Scroll to show the recent data to avoid showing an empty chart
            if user scrolls too much into the future or into the past.
            
            Invoke ScrollToDataEnd() even if AutoScroll is GlgTrue to 
            scroll ahead by a few extra seconds to show a few next updates
            without scrolling the chart.
         */
         if( GlgGetDataExtent( rtc_page->Chart, NULL, &min_max, 
                               /* x extent */ GlgTrue ) )
         {
            double
              first_time_stamp,
              last_time_stamp,
              displayed_time_end;

            first_time_stamp = min_max.min;
            last_time_stamp = min_max.max;            
            GlgGetDResource( rtc_page->Chart, "XAxis/EndValue", 
                             &displayed_time_end );

            if( rtc_page->AutoScroll != 0 )
              rtcScrollToDataEnd( rtc_page, MOST_RECENT, GlgTrue );

            else if( displayed_time_end >
                     last_time_stamp + GetExtraSeconds( rtc_page->TimeSpan ) )
              rtcScrollToDataEnd( rtc_page, MOST_RECENT, GlgTrue );

            else if( displayed_time_end - rtc_page->TimeSpan <= 
                     first_time_stamp )
              rtcScrollToDataEnd( rtc_page, LEAST_RECENT, GlgTrue );

            GlgUpdate( rtc_page->Viewport );
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
      if( rtcGetUpdateInterval( hmi_page ) > 100 )
        GlgUpdate( rtc_page->Viewport );

      processed = GlgTrue;
   }            
   else if( strcmp( action, "Zoom" ) == 0 )    /* Zoom events */
   {
      processed = GlgTrue;
      if( strcmp( subaction, "ZoomRectangle" ) == 0 )
      {
         /* Store AutoSCroll state to restore it if ZoomTo is aborted. */
         rtc_page->StoredScrollState = rtc_page->AutoScroll;
         
         /* Stop scrolling when ZoomTo action is started. */
         rtcChangeAutoScroll( rtc_page, 0 );
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
         rtcChangeAutoScroll( rtc_page, rtc_page->StoredScrollState ); 
      }
      
      GlgUpdate( rtc_page->Viewport );
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

      GlgUpdate( rtc_page->Viewport );
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
static GlgBoolean rtcTraceCB( HMIPage * hmi_page, GlgObject viewport, 
                              GlgAnyType client_data, GlgAnyType call_data )
{
   RTChartPage * rtc_page = (RTChartPage*) hmi_page;
   GlgTraceCBStruct * trace_data = (GlgTraceCBStruct*) call_data;
   EventType event_type = 0;
   int button = 0;
   double x, y;

   /* Process only events that occur in ChartViewport. */
   if( trace_data->viewport != rtc_page->ChartVP )
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
      if( rtcZoomToMode( rtc_page ) )
        return GlgFalse; /* ZoomTo or dragging mode in progress. */
         
      /* Start dragging with the mouse on a mouse click. 
         If user clicked of an axis, the dragging will be activated in the
         direction of that axis. If the user clicked in the chart area,
         dragging in both the time and the Y direction will be activated.
      */

      GlgSetZoom( rtc_page->ChartVP, NULL, 's', 0. );
         
      /* Disable AutoScroll not to interfere with dragging. */
      rtcChangeAutoScroll( rtc_page, 0 ); 
      return GlgTrue;
         
    default: break;
   }
   return GlgFalse;
}
   
/*----------------------------------------------------------------------
| Returns GlgTrue if tag sources need to be remapped for the page.
*/
static GlgBoolean rtcNeedTagRemapping( HMIPage * hmi_page )
{
   /* In demo mode, unset tags need to be remapped to enable animation. */
   if( Viewer.RandomData )
     return GlgTrue;
   else
     return GlgFalse;   /* Remap tags only if necessary. */
}

/*----------------------------------------------------------------------
| Reassign TagSource parameter for a given tag object to a new
| TagSource value. tag_source and tag_name parameters are the current 
| TagSource and TagName of the tag_obj.
*/
static void rtcRemapTagObject( HMIPage * hmi_page, GlgObject tag_obj, 
                               char * tag_name, char * tag_source )
{
   char * new_tag_source;

   if( Viewer.RandomData )
   {
      /* Skip tags with undefined TagName. */
      if( IsUndefined( tag_name ) )
        return;
            
      /* In demo mode, assign unset tag sources to be the same as tag names
         to enable animation with demo data.
      */
      new_tag_source = tag_name;
      if( IsUndefined( tag_source ) )
        AssignTagSource( tag_obj, new_tag_source );
   }
   else
   {
#if 0
      /* Assign new TagSource as needed. */
      AssignTagSource( tag_obj, new_tag_source );
#endif
   }
}

/*----------------------------------------------------------------------
| Change chart's AutoScroll mode.
*/
static void rtcChangeAutoScroll( RTChartPage * rtc_page, int new_value )
{
   double auto_scroll;
   GlgPanType pan_x;
      
   if( new_value == -1 )  /* Use the state of the ToggleAutoScroll button. */
   {
      GlgGetDResource( rtc_page->Viewport, 
                       "Toolbar/ToggleAutoScroll/OnState", &auto_scroll );
      rtc_page->AutoScroll = auto_scroll;
   }
   else    /* Set to the supplied value. */
   {
      rtc_page->AutoScroll = new_value;
      GlgSetDResource( rtc_page->Viewport, "Toolbar/ToggleAutoScroll/OnState", 
                       (double) rtc_page->AutoScroll );
   }
      
   /* Set chart's auto-scroll. */
   GlgSetDResource( rtc_page->Chart, "AutoScroll", 
                    (double) rtc_page->AutoScroll );
      
   /* Activate time scrollbar if AutoScroll is Off. The Y value scrollbar 
      uses GLG_PAN_Y_AUTO and appears automatically as needed.
   */
   pan_x = ( rtc_page->AutoScroll != 0 ? GLG_NO_PAN : GLG_PAN_X );
   GlgSetDResource( rtc_page->ChartVP, "Pan", 
                    (double) ( pan_x | GLG_PAN_Y_AUTO ) );
}
   
/*----------------------------------------------------------------------
| Changes the time span shown in the graph, adjusts major and minor tick 
| intervals to match the time span.
*/
static void rtcSetChartSpan( RTChartPage * rtc_page, int span_index )
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
   GlgSetDResourceIf( rtc_page->Viewport, "Toolbar/SpanSelector/SelectedIndex", 
                      (double) span_index, GlgTrue );
   
   /* Set intervals before SetZoom() below to avoid redrawing huge number 
      of labels. */
   GlgSetDResource( rtc_page->Chart, "XAxis/MajorInterval",
                    (double) major_interval );
   GlgSetDResource( rtc_page->Chart, "XAxis/MinorInterval", 
                    (double) minor_interval );
   
   /* Set the X axis span which controls how much data is displayed in the 
      chart. */
   rtc_page->TimeSpan = span;
   GlgSetDResource( rtc_page->Chart, "XAxis/Span", (double) rtc_page->TimeSpan );

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
      GlgSetDResource( rtc_page->PlotArray[0], "FilterType", 
                       (double) GLG_MIN_MAX_FILTER );
      GlgSetDResource( rtc_page->PlotArray[0], "FilterPrecision", 1. );
   }
   else
     GlgSetDResource( rtc_page->PlotArray[0], "FilterType", 
                      (double) GLG_NULL_FILTER );

   /* Change time and tooltip formatting to match the selected span. */
   rtcSetTimeFormats( rtc_page );
}

/*----------------------------------------------------------------------
| Sets the formats of time labels and tooltips depending on the selected 
| time span.
*/
static void rtcSetTimeFormats( RTChartPage * rtc_page )
{
   char 
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
   switch( rtc_page->SpanIndex )
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
   GlgSetSResource( rtc_page->Chart, "XAxis/TimeFormat", time_label_format );
   
   /* Specify axis and chart tooltip format, if different from default 
      formats defined in the drawing.
   */
   time_tooltip_format = "Time: <axis_time:%X> +0.<axis_time_ms:%03.0lf> sec.\nDate: <axis_time:%x>";
   
   /* <sample_time:%s> inherits time format from the X axis. */
   chart_tooltip_format = "Plot <plot_string:%s> value= <sample_y:%.2lf>\n<sample_time:%s>";

   /* Set time label and tooltip formats. */
   GlgSetSResource( rtc_page->Chart, "XAxis/TooltipFormat", 
                    time_tooltip_format );
   GlgSetSResource( rtc_page->Chart, "TooltipFormat", chart_tooltip_format );
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
static void rtcScrollToDataEnd( RTChartPage * rtc_page, 
                                int data_end, GlgBoolean show_extra )
{
   GlgMinMax min_max;
   double end_value, extra_sec;
      
   if( data_end == DONT_CHANGE )
     return;

   /* Get the min and max time stamp. */
   if( !GlgGetDataExtent( rtc_page->Chart, NULL, &min_max, 
                          /* x extent */ GlgTrue ) )
     return;

   if( show_extra ) 
     extra_sec = GetExtraSeconds( rtc_page->TimeSpan );
   else
     extra_sec = 0.0;

   if( data_end == MOST_RECENT )
     end_value = min_max.max + extra_sec;
   else   /* LEAST_RECENT */
     end_value = min_max.min - extra_sec + rtc_page->TimeSpan ;

   GlgSetDResource( rtc_page->Chart, "XAxis/EndValue", end_value );
}

/*----------------------------------------------------------------------
| Determines a good number of extra seconds to be added at the end in
| the real-time mode to show a few next updates without scrolling the
| chart.
*/
static double GetExtraSeconds( int time_span )
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
static void rtcRestoreInitialYRanges( RTChartPage * rtc_page )
{
   GlgObject axis_array, axis;
   int i;

   axis_array = GlgGetResourceObject( rtc_page->Chart, "YAxisGroup" );
   for( i=0; i < rtc_page->NumYAxes; ++i )
   {
      axis = GlgGetElement( axis_array, i ); 
      GlgSetDResource( axis, "Low", rtc_page->Low[ i ] );
      GlgSetDResource( axis, "High", rtc_page->High[ i ] );
   }
}
   
/*----------------------------------------------------------------------
| Returns True if the chart's viewport is in ZoomToMode.
| ZoomToMode is activated on Dragging and ZoomTo operations.
*/
static GlgBoolean rtcZoomToMode( RTChartPage * rtc_page )
{
   double zoom_mode;

   GlgGetDResource( rtc_page->ChartVP, "ZoomToMode", &zoom_mode );
   return zoom_mode != 0.;
}
   
/*----------------------------------------------------------------------
| Abort ZoomTo mode.
*/
static void rtcAbortZoomTo( RTChartPage * rtc_page )
{
   if( rtcZoomToMode( rtc_page ) )
   {
      /* Abort zoom mode in progress. */
      GlgSetZoom( rtc_page->ChartVP, NULL, 'e', 0. ); 
      GlgUpdate( rtc_page->Viewport );
   }
}

