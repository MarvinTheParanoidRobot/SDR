/***********************************************************************
 * Real-Time StripChart Demo.
 * The stripchart uses integrated data filtering for handling real-time
 *  updates of plots with huge number of data samples.
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#ifdef _WINDOWS
#include "resource.h"
# pragma warning( disable : 4244 )
# pragma warning( disable : 4996 )    /* Allow cross-platform sscanf() */
#endif
#include "GlgApi.h"

#define DEBUG_TIMER     0   /* Set to 1 to debug timer intervals */

#define NUM_PLOTS       3            /* Number of plot lines in the chart. */
#define NUM_Y_AXES      NUM_PLOTS    /* One axis for each plot in this demo, 
                                        may be different. */
/* Set to 1 to allow the user to ZoomTo using Button3-Drag-Release. */
#define ENABLE_ZOOM_TO_ON_BUTTON3    1

#define DAY     ( 3600 * 24 )        /* Number of seconds in a day */

/* Sampling interval for historical data points in seconds. */
#define HISTORICAL_DATA_INTERVAL     60.   /* Once a minute */

/* Demo modes */
typedef enum
{
   REAL_TIME = 0,   /* Real-time mode: updates graph with data using the 
                       current time for time stamps. */
   HISTORICAL,      /* Historical mode: displays and scrolls through 
                       historical data. */
   CALENDAR         /* Calendar mode: displays daily data. */

} DemoMode;

/* Constants for scrolling to the ends of the time range. */
typedef enum
{
   DONT_CHANGE = 0,
   MOST_RECENT,    /* Make the most recent data visible. */
   LEAST_RECENT,   /* Make the least recent data visible. */
} ShowDataEnd;

typedef struct _DataPoint
{
   double value;
   GlgBoolean value_valid;
   double time_stamp;
   GlgBoolean has_time_stamp;
   GlgBoolean has_marker;
} DataPoint;

#include "stripchart.h"   /* Function prototypes. */

GlgObject
  Drawing, 
  ChartVP,
  Chart,
  Plot[ NUM_PLOTS ],
  YAxis[ NUM_Y_AXES ];
GlgLong
  BufferSize = 50000,     /* Number of samples to keep in the buffer for 
                             each line. */
  PrefillData = True,     /* Setting to False suppresses pre-filling the 
                             chart's buffer with data on start-up in the 
                             REAL_TIME mode. */
  UpdateInterval = 30;    /* Update interval in msec */

  /* Variables used to keep current state. */
GlgLong
  TimeSpan = 0,           /* The currently displayed span in seconds. */
  StoredScrollState = 0,  /* Stored AutoScroll state to be restored if ZoomTo
                             is aborted. */
  AutoScroll = True,      /* Current auto-scroll state: enabled or disabled. */
  Mode = REAL_TIME,       /* Current mode: real-time, historical or calendar.*/
  SpanIndex = 1,          /* Index of the currently displayed time span. */
  YAxisLabelType = 0;     /* Used to demonstrate diff. Y axis labels. */ 

/* Stores initial range values, used to restore after zooming. */
double
  Min[ NUM_PLOTS ],
  Max[ NUM_PLOTS ];

static GlgBoolean StopAutoScroll = False;

GlgAppContext AppContext;   /* Global, used to install a timeout. */

#include "GlgMain.h"    /* Cross-platform entry point. */

/*----------------------------------------------------------------------
| Options:
|   -real-time (default)  - start in real-time mode
|   -historical           - start in historical data mode
|   -calendar             - start in calendar data mode
|   -buffer-size <N>      - buffer size to use (default 100000)
|   -dont-prefill-data    - suppress pre-filling buffer with data on start-up
|                           for REAL_TIME mode (default is to pre-fill).
*/
int GlgMain( argc, argv, app_context )
     int argc;
     char *argv[];
     GlgAppContext app_context;
{   
   GlgLong skip;
   long buffer_size;
   char * full_path;

   /* Picks up the current locale settings that define the preferred time 
      label format. */
   GlgInitLocale( NULL );

   AppContext = GlgInit( False, app_context, argc, argv );

   for( skip = 1; skip < argc; ++skip )
   {
      if( strcmp( argv[ skip ], "-real-time" ) == 0 )
        Mode = REAL_TIME;
      else if( strcmp( argv[ skip ], "-historical" ) == 0 )
        Mode = HISTORICAL;
      else if( strcmp( argv[ skip ], "-calendar" ) == 0 )
        Mode = CALENDAR;
      else if( strcmp( argv[ skip ], "-buffer-size" ) == 0 )
      {
         ++skip;
         if( skip >= argc || 
             sscanf( argv[ skip ], "%ld", &buffer_size ) != 1 || 
             buffer_size < 2 )
           error( "Invalid -buffer-size parameter value.", False );         
         else
           BufferSize = buffer_size;

         printf( "Using buffer_size= %ld\n", (long) BufferSize );
      }
      else if( strcmp( argv[ skip ], "-dont-prefill-data" ) == 0 )
        PrefillData = False;      
   }

   GlgSetDResource( (GlgObject)0, "$config/GlgMouseTooltipTimeout", 0.25 );

   full_path = GlgGetRelativePath( argv[0], "stripchart.g" );
   Drawing = GlgLoadWidgetFromFile( full_path );
   if( !Drawing )
     error( "Can't load drawing file.", True );

   GlgFree( full_path );

   ChartVP = GlgGetResourceObject( Drawing, "ChartViewport" );
   Chart = GlgGetResourceObject( ChartVP, "Chart" );

   /* Setting widget dimensions in screen pixels. If not set, default 
      dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( Drawing, "Point1", 0., 0., 0. );
   GlgSetGResource( Drawing, "Point2", 0., 0., 0. );
   GlgSetDResource( Drawing, "Screen/WidthHint",  800. );
   GlgSetDResource( Drawing, "Screen/HeightHint", 600. );

   /* Setting the window title. */
   GlgSetSResource( Drawing, "ScreenName", "GLG Stripchart Demo" );

   GlgAddCallback( Drawing, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );
   GlgAddCallback( Drawing, GLG_TRACE_CB, (GlgCallbackProc)Trace, NULL );
   GlgAddCallback( Drawing, GLG_TRACE2_CB, (GlgCallbackProc)Trace2, NULL );

   InitChart(); 
   GlgSetupHierarchy( Drawing );

#ifdef _WINDOWS            
   GlgLoadExeIcon( Drawing, IDI_ICON1 );
#endif

   AdjustButtonColor();

   SetMode( Mode ); /* Sets initial mode: real-time, historical or calendar. */

   GlgUpdate( Drawing );

   StartUpdate();   /* Installs a timeout to update chart with data. */

   return (int) GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| Initializes the drawing and the chart.
*/
void InitChart()
{
   GlgObject plot_array, y_axis_array;
   GlgLong i;

   /* Set the requested buffer size. */
   GlgSetDResource( Chart, "BufferSize", (double) BufferSize );

   /* Increase the number of plots and Y axes if the matching number of them
      are not already defined in the chart's drawing. 
   */
   GlgSetDResource( Chart, "NumPlots", (double) NUM_PLOTS );
   GlgSetDResource( Chart, "NumYAxes", (double) NUM_Y_AXES );
   GlgSetupHierarchy( Drawing );

   /* Using an Intermediate API to store plot IDs in an array for convenient 
      access. To query plot IDs with the Standard API, use GlgGetNamedPlot() 
      in conjunction with GlgCreateIndexedName().
   */
   plot_array = GlgGetResourceObject( Chart, "Plots" );
   for( i=0; i<NUM_PLOTS; ++i )
     Plot[i] = GlgGetElement( plot_array, i );

   /* Store Y axes in an array for convenient access using an Intermediate API.
      Alternatively, Y axes' resources can be accessed by their resource names 
      via the Standard API, for example: "ChartVP/Chart/YAxisGroup/YAxis#0/Low"
   */
   y_axis_array = GlgGetResourceObject( Chart, "YAxisGroup" );
   for( i=0; i<NUM_Y_AXES; ++i )
     YAxis[i] = GlgGetElement( y_axis_array, i );

   /* Set the Chart Zoom mode. It was set and saved with the drawing, 
      but do it again programmatically just in case.
      */
   GlgSetZoomMode( ChartVP, NULL, Chart, NULL, GLG_CHART_ZOOM_MODE );

   /* Set the initial Y axis label type. */
   ChangeYAxisLabelType( YAxisLabelType );

   /* Query the initial Y ranges defined in the drawing and store them
      for the Restore Ranges action.
   */
   StoreInitialYRanges();

   DisplaySelection( NULL, True );
}

/*----------------------------------------------------------------------
| Handle user interaction.
*/
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{
   GlgObject message_obj;
   double span_index;   
   char
     * origin,
     * format,
     * action,
     * subaction;

   message_obj = (GlgObject) call_data;

   GlgGetSResource( message_obj, "Origin", &origin );
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "SubAction", &subaction );

#if 0
   printf( "format= %s, action= %s, subaction= %s\n", 
           format, action, subaction );
#endif

   /* Handle window closing. Could use viewport's name if there is more than
      one top viewport. */
   if( strcmp( format, "Window" ) == 0 && 
       strcmp( action, "DeleteWindow" ) == 0 )
     exit( GLG_EXIT_OK );

   if( strcmp( format, "Button" ) == 0 )            /* Handle button clicks */
   {
      if( strcmp( action, "Activate" ) != 0 &&      /* Not a push button */
          strcmp( action, "ValueChanged" ) != 0 )   /* Not a toggle button */
	return;

      AbortZoomTo();

      if( strcmp( origin, "ScrollToRecent" ) == 0 )
      {         
         /* Set time axis's end to current time. */
         ScrollToDataEnd( MOST_RECENT, True ); 
      }
      else if( strcmp( origin, "ToggleAutoScroll" ) == 0 )
      {         
         ChangeAutoScroll( -1 );    /* Toggle curr. value between 0 and 1. */
      }
      else if( strcmp( origin, "ZoomTo" ) == 0 )
      {
#if ENABLE_ZOOM_TO_ON_BUTTON3
         /* Allow the ZoomTo toolbar button to use the left mouse button. */
         GlgSetDResource( (GlgObject)0, "$config/GlgZoomToButton", 1. );
#endif
         GlgSetZoom( ChartVP, NULL, 't', 0. );  /* Start ZoomTo op */
      }
      else if( strcmp( origin, "ResetZoom" ) == 0 )
      {         
         SetChartSpan( SpanIndex );
         RestoreInitialYRanges();
      }
      else if( strcmp( origin, "ScrollBack" ) == 0 )
      {
         ChangeAutoScroll( 0 );

         /* Scroll left by 1/3 of the span. */
         GlgSetZoom( ChartVP, NULL, 'l', 0.33 );
      }
      else if( strcmp( origin, "ScrollBack2" ) == 0 )
      {
         ChangeAutoScroll( 0 );

         /* Scroll left by a full span. */
         GlgSetZoom( ChartVP, NULL, 'l', 1. );
      }
      else if( strcmp( origin, "ScrollForward" ) == 0 )
      {
         ChangeAutoScroll( 0 );

         /* Scroll right by 1/3 of the span. */
         GlgSetZoom( ChartVP, NULL, 'r', 0.33 );
      }
      else if( strcmp( origin, "ScrollForward2" ) == 0 )
      {
         ChangeAutoScroll( 0 );

         /* Scroll right by a full span. */
         GlgSetZoom( ChartVP, NULL, 'r', 1. );
      }
      else if( strcmp( origin, "ToggleLabels" ) == 0 )
      {
         ChangeYAxisLabelType( -1 );   /* Change to the next type. */
      }
      else if( strcmp( origin, "DemoMode" ) == 0 )
      {
         /* Toggle the current mode between REAL_TIME, HISTORICAL and 
            CALENDAR. */
         SetMode( -1 );
      }

      GlgUpdate( Drawing );
   }
   else if( strcmp( format, "Menu" ) == 0 )
   {
      if( strcmp( action, "Activate" ) != 0 )
	return;

      AbortZoomTo();

      if( strcmp( origin, "SpanSelector" ) == 0 )    /* Span change */
      {         
         GlgMinMax min_max;
         double
           displayed_time_end, 
           first_time_stamp, last_time_stamp;

         GlgGetDResource( message_obj, "SelectedIndex", &span_index );
         SpanIndex = span_index;

         SetChartSpan( SpanIndex );
         RestoreInitialYRanges();   /* Restore in case the chart was zoomed. */

         /* Scroll to show the recent data to avoid showing an empty chart
            if user scrolls too much into the future or into the past.
            
            In the real-time mode, invoke ScrollToDataEnd() even if 
            AutoScroll is True to scroll ahead by a few extra seconds to 
            show a few next updates without scrolling the chart.
         */
         if( GlgGetDataExtent( Chart, NULL, &min_max, /* x extent */ True ))
         {
            first_time_stamp = min_max.min;
            last_time_stamp = min_max.max;
            
            GlgGetDResource( Chart, "XAxis/EndValue", &displayed_time_end );
            
            if( Mode == REAL_TIME && AutoScroll )
              ScrollToDataEnd( MOST_RECENT, True );
            else if( displayed_time_end > 
                     last_time_stamp + GetExtraSeconds() )
              ScrollToDataEnd( MOST_RECENT, True );
            else if( displayed_time_end - TimeSpan < first_time_stamp )
              ScrollToDataEnd( LEAST_RECENT, True );
         }
	 GlgUpdate( Drawing );
      }
   }
   else if( strcmp( format, "Zoom" ) == 0 )
   {
      if( strcmp( action, "Zoom" ) == 0 )
      {
         if( strcmp( subaction, "ZoomRectangle" ) == 0 )
         {
            /* Store the current AutoScroll state to restore it if ZoomTo 
               is aborted. */
            StoredScrollState = AutoScroll;
            
            /* Stop scrolling: ZoomTo action is being started. */
            ChangeAutoScroll( 0 );
         }
         else if( strcmp( subaction, "End" ) == 0 )
         {
            /* No addtional actions on finishing ZoomTo. The Y scrollbar 
               appears automatically if needed: it is set to GLG_PAN_Y_AUTO. 
               Don't resume scrolling: it'll scroll too fast since we zoomed in.
               Keep it still to allow inspecting zoomed data.
            */
         }
         else if( strcmp( subaction, "Abort" ) == 0 )
         {
            /* Resume scrolling if it was on. */
            ChangeAutoScroll( StoredScrollState );         
         }
         
         GlgUpdate( Drawing );
      }
      else if( strcmp( action, "Pan" ) == 0 )
      {
         /* This code may be used to perform custom actions when dragging the 
            chart's data with the mouse. 
         */
         if( strcmp( subaction, "Start" ) == 0 )   /* Chart dragging start */
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
      }
   }
   else if( strcmp( format, "Tooltip" ) == 0 )
   {
      if( strcmp( action, "SpecialTooltip" ) == 0 )
      {
         /* When the chart tooltip appears, erase selection text, but
            keep selection marker from the tooltip. 
         */
         DisplaySelection( NULL, False );
      }
   }
   else if( strcmp( format, "Chart" ) == 0 )
   {
      if( strcmp( action, "CrossHairUpdate" ) == 0 )
      {
         /* No need to invoke GlgUpdate() to redraw the new position of the 
            chart's cross hair cursor: the drawing will be redrawn in one
            batch by either the update timer or DisplaySelection().
         */
      }
   }
   else if( strcmp( format, "CustomEvent" ) == 0 )
   {
      char * event_label;

      GlgGetSResource( message_obj, "EventLabel", &event_label );
      if( event_label )
      {         
         if( strcmp( event_label, "LegendSelect" ) == 0 )
         {
            SelectPlot( GlgGetSelectedPlot() );   /* Select plot. */
            /* Don't stop auto-scroll if legend was clicked on. */
            StopAutoScroll = False;
         }
         else if( strcmp( event_label, "LegendUnselect" ) == 0 )
         {
            SelectPlot( NULL );                   /* Unselect plot. */
            StopAutoScroll = False;
         }
      }
   }
}

/*----------------------------------------------------------------------
| Changes line width of the selected plot.
*/
void SelectPlot( GlgObject plot )
{
   static GlgObject selected_plot = NULL;

   if( plot == selected_plot )
     return;

   if( selected_plot )
   {
      /* Unselect the previously selected plot. */
      GlgSetDResource( selected_plot, "Selected", 0. );
      GlgDropObject( selected_plot );
      selected_plot = NULL;
   }
   
   if( plot )
   {
      /* Select a new plot. "Selected" resource controls transformation 
         attached to the plot's line width. When the Selected resource 
         is set to 1, the plot's LineWidth changes to 2.
      */
      GlgSetDResource( plot, "Selected", 1. );
      selected_plot = GlgReferenceObject( plot );
   }
   
   GlgUpdate( Drawing );
}

/*----------------------------------------------------------------------
| Used to start scrolling the chart by dragging it with the mouse.
*/
void Trace( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{      
   GlgTraceCBStruct * trace_data;
   GlgBoolean display_selection = False;
   double x, y;
   int event_type = 0;
   int button = 0;

   trace_data = (GlgTraceCBStruct*) call_data;

   /* Use ChartViewport's events only. */
   if( trace_data->viewport != ChartVP )
     return;

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

    case LeaveNotify: event_type = LEAVE_EVENT; break;

    default: return;
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
       case WM_MOUSEMOVE: event_type = MOUSE_MOVE_EVENT; break;
       default:           event_type = BUTTON_PRESS_EVENT; break;
      } 
      break;
      
    case WM_KILLFOCUS: event_type = LEAVE_EVENT; break;

    default: return;
   }
#endif
   
   switch( event_type )
   {
    case BUTTON_PRESS_EVENT:
      if( ZoomToMode() )
        return; /* ZoomTo or dragging mode in progress. */
      
      /* Start dragging with the mouse on a mouse click on Button1,
         or start ZoomTo on Button3.

         For Button1, if user clicked of an axis, the dragging will be
         activated in the direction of that axis. If the user clicked
         on the chart area, dragging in both the time and the Y
         direction will be activated.

         To allow dragging just in one direction, use '>' instead of 's' 
         for horizontal scrolling and '^' for vertical.
      */
      switch( button )
      {
       case 1: GlgSetZoom( ChartVP, NULL, 's', 0. ); break;   /* Drag */
#if ENABLE_ZOOM_TO_ON_BUTTON3
       case 3:
         /* Change ZoomTo button from 1 to 3. */
         GlgSetDResource( (GlgObject)0, "$config/GlgZoomToButton", 3. );

         GlgSetZoom( ChartVP, NULL, 't', 0. );      /* ZoomTo */
         break;
#endif
      }

      /* Disable AutoScroll not to interfere with dragging - but do it later
         in the Trace2 callback, only if legend was not clicked on.
      */
      StopAutoScroll = True;

      display_selection = True;
      break;
      
    case MOUSE_MOVE_EVENT:
      display_selection = True;
      break;

    case LEAVE_EVENT:
      /* Erase last selection when cursor leaves the window. */
      DisplaySelection( NULL, True );
      break;

    default: return;
   } 

   /* In addition to a tooltip appearing after a timeout when the mouse 
      stops, display selection information when the mouse moves over a chart
      or axis. The selection is displayed using the same format as the tooltip,
      which is configured via the TooltipFormat attribute of the chart.
      Alternatively, an application can invoke GlgCreateChartSelection()
      and display the returned data in a custom format.
   */
   if( display_selection && !ZoomToMode() )
   {
      char * string;
      
      string = 
        GlgCreateTooltipString( Chart, x, y, 10., 10., "<single_line>" );

      /* Display new selection or erase last selection if no selection
         (when string is null). 
      */
      DisplaySelection( string, True );
      GlgFree( string );
   }
}

/*----------------------------------------------------------------------
| Trace2 callback is invoked after the Input callback.
*/
void Trace2( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{
   GlgTraceCBStruct * trace_data;
   int event_type = 0;

   trace_data = (GlgTraceCBStruct*) call_data;

   /* Use ChartViewport's events only. */
   if( trace_data->viewport != ChartVP )
     return;

#ifndef _WINDOWS
   switch( trace_data->event->type )
   {
    case ButtonPress:
      event_type = BUTTON_PRESS_EVENT;
      break;
   }
#else
   switch( trace_data->event->message )
   {
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
     event_type = BUTTON_PRESS_EVENT;
     break;
   }
#endif

   /* Stop auto-scroll on a click in the chart, but not if the legend was 
      clicked.
   */
   switch( event_type )
   {
    case BUTTON_PRESS_EVENT:
      if( StopAutoScroll )
      {
         /* ChangeAutoScroll() may trigger events on Windows,
            Unset StopAutoScroll before invoking it to avoid issues with
            reentrancy.
         */
         StopAutoScroll = False;
         ChangeAutoScroll( 0 );
      }
      break;
   }
}

/*----------------------------------------------------------------------
| Display information about the selected point. It is used on a mouse move 
| in addition to a tooltip.
*/
void DisplaySelection( char * string, GlgBoolean erase_selection_marker )
{   
   if( !string )
   {
      string = "";

      /* No selection: erase selection highlight marker if requested. */
      if( erase_selection_marker )
        GlgSetDResourceIf( Chart, "DrawSelected", 0., True );
   }

   GlgSetSResourceIf( ChartVP, "SelectionLabel/String", string, True );

   /* In the real-time mode the drawing is updated on a timer, 
      otherwise update it here. 
   */
   if( Mode != REAL_TIME )
     GlgUpdate( ChartVP );
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
void ScrollToDataEnd( ShowDataEnd data_end, GlgBoolean show_extra )
{
   double end_value, extra_sec;
   GlgMinMax min_max;

   if( data_end == DONT_CHANGE )
     return;

   /* Get the min and max time stamp. */
   if( !GlgGetDataExtent( Chart, NULL, &min_max, /* x extent */ True ) )
     return;

   if( show_extra )   
     extra_sec = GetExtraSeconds();
   else
     extra_sec = 0.;

   if( data_end == MOST_RECENT )
     end_value = min_max.max + extra_sec;
   else   /* LEAST_RECENT */
     end_value = min_max.min - extra_sec + TimeSpan;

   GlgSetDResource( Chart, "XAxis/EndValue", end_value );
}

/*----------------------------------------------------------------------
| Determines a good number of extra seconds to be added at the end in
| the real-time mode to show a few next updates without scrolling the
| chart.
*/
double GetExtraSeconds()
{
   double extra_sec, max_extra_sec;

   if( Mode != REAL_TIME )
     return 0.;

   extra_sec = TimeSpan * 0.1;
   switch( SpanIndex )
   {
    default:
    case 0:
    case 1: 
    case 2: max_extra_sec = 3.; break;
    case 3: max_extra_sec = 5.; break;
   }

   if( extra_sec > max_extra_sec )
     extra_sec = max_extra_sec;

   return extra_sec;
}

/*----------------------------------------------------------------------
|
*/
void ChangeAutoScroll( GlgLong new_value )
{
   double auto_scroll;
   GlgLong pan_x;

   if( new_value == -1 )  /* Use the state of the ToggleAutoScroll button. */
   {
      GlgGetDResource( Drawing, "ToggleAutoScroll/OnState", &auto_scroll );
      AutoScroll = auto_scroll;
   }
   else    /* Set to the supplied value. */
   {
      AutoScroll = new_value;

      /* Update the AutoScroll toggle with the new value. */
      GlgSetDResource( Drawing, "ToggleAutoScroll/OnState", 
                       (double) AutoScroll );
   }

   /* Set chart's auto-scroll. */
   GlgSetDResource( Chart, "AutoScroll", (double) AutoScroll );

   /* Activate time scrollbar if AutoScroll is Off. The Y value scrollbar 
      uses GLG_PAN_Y_AUTO and appears automatically as needed.
   */
   pan_x = ( AutoScroll ? GLG_NO_PAN : GLG_PAN_X );
   GlgSetDResource( ChartVP, "Pan", (double) ( pan_x | GLG_PAN_Y_AUTO ) );
}

/*----------------------------------------------------------------------
| Changes the time span shown in the graph, adjusts major and minor tick 
| intervals to match the time span.
*/
void SetChartSpan( GlgLong span_index )
{
   GlgMinMax min_max;
   GlgLong 
     span, major_interval, minor_interval, time_offset, 
     in_the_middle, fix_leap_years,
     num_vis_points;
   double sampling_interval;   /* In seconds */

   in_the_middle = False;
   fix_leap_years = False;

   /* Change chart's time span, as well as major and minor tick intervals. */
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

       default: error( "Invalid span index", False ); return;
      }
      time_offset = 0;
      sampling_interval = UpdateInterval / 1000.;
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

       default: error( "Invalid span index", False ); return;
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
         in_the_middle = True; 

         /* Offsets month labels by 1/2 month to position them in the 
            midddle of the month's interval.
         */
         time_offset = DAY * 15;

         span = DAY * 365;            /* 1 year */
         major_interval = -12;        /* major tick every month (12 ticks) */
         minor_interval = -2;         /* minor tick in the middle (2 ticks)
                                         to show the extent of the month. */
         break;
         
       case 3:
         span = DAY * 365 * 10;       /* 10 years */
         major_interval = DAY * 365;  /* major tick every year */
         minor_interval = -12;        /* minor tick every month (12 ticks) */

         /* Time labels display only the year for this time scale.
            Position major ticks and labels a bit past the 1st the month 
            to avoid any rounding errors. The tooltips display the exact
            date regardless.
         */
         time_offset = 3600;
         break;

       default: error( "Invalid span index", False ); return;
      }

      if( span_index >= 2 )     /* 1 year or 10 years */
        /* The major tick is positioned at the start of the month or the 
           start of the year. Tell the chart to properly calculate the label
           position by adjusting by the number of accumulated leap days.
           It matters only in the calendar mode when the major tick interval
           is greater then a day.
        */
        fix_leap_years = True;

      sampling_interval = DAY / 2;
      break;

    default: error( "Invalid mode", False ); return;
   }

   /* Update the menu in the drawing with the initial value if different. */
   GlgSetDResourceIf( Drawing, "SpanSelector/SelectedIndex", 
                      (double) span_index, True );

   /* Set intervals before GlgSetZoom() below to avoid redrawing huge number 
      of labels. */
   GlgSetDResource( Chart, "XAxis/MajorInterval", (double) major_interval );
   GlgSetDResource( Chart, "XAxis/MinorInterval", (double) minor_interval );

   GlgSetDResource( Chart, "XAxis/MajorOffset", (double) time_offset );
   GlgSetDResource( Chart, "XAxis/FixLeapYears", (double) fix_leap_years );

   /* Set the X axis span which controls how much data is displayed in the 
      chart. */
   if( span > 0 )
   {
      TimeSpan = span;
      GlgSetDResource( Chart, "XAxis/Span", (double) TimeSpan );      
   }
   else   /* span == -1 : show all accumulated data in the REAL_TIME mode. */
   {
      /* 'N' resets span to show all data accumulated in the buffer. */
      GlgSetZoom( ChartVP, NULL, 'N', 0. );

      /* Query the actual time span: set it to the extent of the data 
         accumulated in the chart's buffer, plus a few extra seconds
         at the end to show a few updates without scrolling the chart.
      */
      GlgGetDataExtent( Chart, NULL, &min_max, /* x extent */ True );
      TimeSpan = min_max.max - min_max.min + GetExtraSeconds();
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
      GlgSetDResource( Plot[0], "FilterType", (double) GLG_MIN_MAX_FILTER );
      GlgSetDResource( Plot[0], "FilterPrecision", 1. );
   }
   else
     GlgSetDResource( Plot[0], "FilterType", (double) GLG_NULL_FILTER );

   /* Display the filter state in the drawing. */
   GlgSetSResource( ChartVP, "DataFilterState", span_index > 1 ? "ON" : "OFF" );

   /* Erase major ticks if showing month labels in the middle of the month 
      interval in the CALENDAR mode. */
   GlgSetDResource( Chart, "XAxis/MajorTickSize", 
                    ( in_the_middle ? 0. : 10. ) );
   GlgSetDResource( Chart, "XAxis/LabelOffset", ( in_the_middle ? 10. : 0. ) );

   /* Display the number of data points visible in all three lines in the 
      current time span.
   */
   num_vis_points = TimeSpan / sampling_interval * NUM_PLOTS;

   /* Must be divisible by NUM_PLOTS */
   num_vis_points = num_vis_points / NUM_PLOTS * NUM_PLOTS;
   if( num_vis_points > BufferSize * NUM_PLOTS )
     num_vis_points = BufferSize * NUM_PLOTS;

   GlgSetDResource( ChartVP, "NumDataPointsVisible", num_vis_points );

   /* Change time and tooltip formatting to match the demo mode and the 
      selected span. */
   SetTimeFormats();

   SetMarkerSize();   /* Decrease marker size for large spans. */
}

#define NUM_SPAN_OPTIONS     4

/*----------------------------------------------------------------------
| Changes labels in the span selection buttons when switching between 
| the REAL_TIME, HISTORICAL and CALENDAR modes.
*/
void SetSelectorLabels()
{
   GlgObject button;
   GlgLong i;   
   char * res_name, * label;

   for( i=0; i<NUM_SPAN_OPTIONS; ++i )
   {
      res_name = GlgCreateIndexedName( "SpanSelector/Button%", i );
      button = GlgGetResourceObject( Drawing, res_name );
      GlgFree( res_name );

      switch( Mode )
      {
       case REAL_TIME:
         switch( i )
         {
          case 0: label = "10 sec"; break;
          case 1: label = "1 min";  break;
          case 2: label = "10 min"; break;
          case 3: label = "All";    break;           
          default: error( "Invalid span index", False ); return;
         }
         break;

       case HISTORICAL:
         switch( i )
         {
          case 0: label = "1 hour"; break;
          case 1: label = "8 hours";  break;
          case 2: label = "24 hours"; break;
          case 3: label = "1 week";    break;           
          default: error( "Invalid span index", False ); return;
         }
         break;

       case CALENDAR:
         switch( i )
         {
          case 0: label = "1 month"; break;
          case 1: label = "1 quarter";  break;
          case 2: label = "1 year"; break;
          case 3: label = "10 years";    break;           
          default: error( "Invalid span index", False ); return;
         }
         break;

       default: error( "Invalid mode", False ); return;
      }

      GlgSetSResource( button, "LabelString", label );
   }
}

/*----------------------------------------------------------------------
|
*/
void StoreInitialYRanges()
{
   GlgLong i;
      
   /* In this demo, each plot is associated with the corresponding axis by
      setting the plot's LinkedAxis property in the drawing file. When a 
      plot is linked to an axis, the plot and the axis use the same Y range, 
      
      We are using an Intermediate API to access Y axes here for convenience.
      Alternatively, Y axes' resources can be accessed by their resource names 
      via the Standard API, for example: 
         "ChartVP/Chart/YAxisGroup/YAxis#0/Low"
   */
   for( i=0; i<NUM_Y_AXES; ++i )
   {
      GlgGetDResource( YAxis[i], "Low",  &Min[i] );
      GlgGetDResource( YAxis[i], "High", &Max[i] );
   }
}

/*----------------------------------------------------------------------
|
*/
void RestoreInitialYRanges()
{
   GlgLong i;

   /* In this demo, each plot is associated with the corresponding axis by
      setting the plot's LinkedAxis property in the drawing file. When a plot
      is linked to an axis, changing the plot's and axis' ranges may be done
      by changing ranges of just one object: either a plot or its linked axis.
      If a plot is not linked to an axis, its range may be different.
   */
   for( i=0; i<NUM_Y_AXES; ++i )
   {
      GlgSetDResource( YAxis[i], "Low",  Min[i] );
      GlgSetDResource( YAxis[i], "High", Max[i] );
   }
}

/*----------------------------------------------------------------------
|
*/
GlgBoolean ZoomToMode()
{
   double zoom_mode;

   GlgGetDResource( ChartVP, "ZoomToMode", &zoom_mode );
   return ( ((int) zoom_mode ) != 0 );
}

/*----------------------------------------------------------------------
|
*/
void AbortZoomTo()
{
   if( ZoomToMode() )
   {
      /* Abort zoom mode in progress. */
      GlgSetZoom( ChartVP, NULL, 'e', 0. ); 
      GlgUpdate( ChartVP );
   }
}

/*----------------------------------------------------------------------
| Installs a timeout to update the chart with new date.
*/
void StartUpdate()
{
   GlgAddTimeOut( AppContext, UpdateInterval, (GlgTimerProc)UpdateChart, 
                  NULL );
}

/*----------------------------------------------------------------------
| 
*/
void UpdateChart( GlgAnyType data, GlgIntervalID * id )
{
   DataPoint data_point;
   GlgULong sec1, microsec1;
   GlgLong i, timer_interval;

   if( Mode != REAL_TIME )
   {
      /* No updates in HISTORICAL and CALENDAR modes - just keep the timer 
         ticking with a larger interval until it switches to the REAL_TIME 
         mode again.
      */
      GlgAddTimeOut( AppContext, 300, (GlgTimerProc)UpdateChart, NULL );      
      return;
   }

   /* Start time for adjusting timer intervals. */
   GlgGetTime( &sec1, &microsec1 );

   /* Supply demo data to update plot lines. */
   for( i=0; i<NUM_PLOTS; ++i )
   {
      GetDemoData( i, &data_point );
      PushPlotPoint( i, &data_point );
   }

   GlgUpdate( Drawing );
   GlgSync( Drawing );    /* Improves interactive response */

   /* Adjust timer intervals to have a constant update rate regardless
      of the time it takes the chart to redraw one iteration of data.
   */      
   timer_interval = GetAdjustedTimeout( sec1, microsec1, UpdateInterval );

   GlgAddTimeOut( AppContext, timer_interval, (GlgTimerProc)UpdateChart, 
                  NULL );
}

/*----------------------------------------------------------------------
| Pushes the data_point's data into the plot.
*/
void PushPlotPoint( GlgLong plot_index, DataPoint * data_point )
{
   GlgObject plot;

   plot = Plot[ plot_index ];

   /* Supply plot value for the chart via ValueEntryPoint. */
   GlgSetDResource( plot, "ValueEntryPoint", data_point->value );
	 	 
   if( data_point->has_time_stamp )
   {
      /* Supply an optional time stamp. If not supplied, the chart will 
         automatically generate a time stamp using current time. 
      */
      GlgSetDResource( plot, "TimeEntryPoint", data_point->time_stamp );
   }

   /* Using markers to annotate spikes on the first plot. The plot type
      was set to LINE & MARKERS in the drawing; marker's Visibility
      can be used as an entry point for marker visibility values.
   */
   if( plot_index == 0 )
     GlgSetDResource( plot, "Marker/Visibility", 
                      (double) data_point->has_marker );
      
   if( !data_point->value_valid )
   {	   
      /* If the data point is not valid, set ValidEntryPoint resource to 
         display holes for invalid data points. If the point is valid,
         it is automatically set to 1. by the chart.
      */
      GlgSetDResource( plot, "ValidEntryPoint", 0. );
   }
}

/*----------------------------------------------------------------------
| Pushes the data_point's data into the plot using low level API methods
| for increased performance. It is used to prefill a chart with large
| quantities of data.
*/
void PushPlotPointDirect( GlgLong plot_index, DataPoint * data_point )
{
   /* Supply an optional time stamp. Use the current time if the time stamp
      is not supplied.
   */
   double time_stamp =
     ( data_point->has_time_stamp ? data_point->time_stamp : GetCurrTime() );

   float marker_visibility =
     ( plot_index == 0 && data_point->has_marker ? 1.0f : 0.0f );

   GlgDataSample * datasample = (GlgDataSample*) GlgCreateDataSample( False );
   datasample->value = data_point->value;
   datasample->time = time_stamp;
   datasample->valid = data_point->value_valid;
   datasample->marker_vis = marker_visibility;
   datasample->filter_mark = 0;
   datasample->extended_data = 0;
   GlgAddDataSample( Plot[ plot_index ], datasample );   
}

/*----------------------------------------------------------------------
| Demonstrates different styles of Y axis label positioning.
|
| This is usually done by configuring the chart in the GlgBuilder.
| This code just toggles through a few options via an API.
*/
void ChangeYAxisLabelType( int new_type )
{
   char * label0, * label1, * label2;
   GlgBoolean offset_labels;
   int
     i, text_direction,
     label_position,    /* Label position relatively to its axis. */
     label_anchoring;   /* Label anchoring relatively to its control point. */

#define NUM_LABEL_TYPES    4

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
      text_direction = GLG_HORIZONTAL_TEXT;

      /* Position and anchor axis labels at the center of each axis in the
         horizontal direction. */
      label_position = ( GLG_HCENTER | GLG_VTOP );
      label_anchoring = ( GLG_HCENTER | GLG_VBOTTOM );
      offset_labels = False;
      break;

    case 1:
      label0 = "Var1"; 
      label1 = "Var2"; 
      label2 = "Var3"; 
      text_direction = GLG_VERTICAL_ROTATED_LEFT;

      /* Position and anchor axis labels at the center of each axis in the
         horizontal direction. */
      label_position = ( GLG_HCENTER | GLG_VTOP );
      label_anchoring = ( GLG_HCENTER | GLG_VBOTTOM );
      offset_labels = False;
      break;

    case 2:
      label0 = "Variable 1"; 
      label1 = "Variable 2"; 
      label2 = "Variable 3"; 
      text_direction = GLG_HORIZONTAL_TEXT;

      /* Position and anchor axis labels on the left edge of each axis in the
         horizontal direction. */
      label_position = ( GLG_HLEFT | GLG_VTOP );
      label_anchoring = ( GLG_HLEFT | GLG_VBOTTOM );
      offset_labels = True;
      break;

    case 3:
      label0 = "Var1"; 
      label1 = "Var2"; 
      label2 = "Var3"; 
      text_direction = GLG_VERTICAL_ROTATED_LEFT;
      /* Position and anchor axis labels at the center of each axis in the
         vertical direction. */      
      label_position = ( GLG_HLEFT | GLG_VCENTER );
      label_anchoring = ( GLG_HRIGHT | GLG_VCENTER );
      offset_labels = False;
      break;

    default:
      error( "Wrong type", False ); 
      YAxisLabelType = 0; 
      return;
   }

   GlgSetSResource( YAxis[0], "AxisLabel/String", label0 );
   GlgSetSResource( YAxis[1], "AxisLabel/String", label1 );
   GlgSetSResource( YAxis[2], "AxisLabel/String", label2 );

   /* Set text direction for all labels using the % wildcard. */
   GlgSetDResource( Chart, "YAxisGroup/YAxis#%/AxisLabel/TextDirection", 
                    text_direction );

   if( offset_labels )
   {
      for( i=0; i<3; ++i )
        /* Set increasing Y offsets. */
        GlgSetGResource( YAxis[i], "AxisLabelOffset", 0., 30. - i * 13, 0. );
   }
   else    /* Set all Y offsets = 10, using the % wildcard. */
     GlgSetGResource( Chart, 
                      "YAxisGroup/YAxis#%/AxisLabelOffset", 0., 10., 0. );

   /* Set position and anchoring of all labels using the % wildcard. */
   GlgSetDResource( Chart, "YAxisGroup/YAxis#%/AxisLabelPosition", 
                    (double) label_position );
   GlgSetDResource( Chart, "YAxisGroup/YAxis#%/AxisLabelAnchoring", 
                    (double) label_anchoring );

   /* Adjusts the space taken by the Y axes to accomodate different axis label 
      layouts.
   */
   AdjustYAxisSpace();
}

/*----------------------------------------------------------------------
| Returns data sample querying interval (in sec.) depending on the demo mode.
*/
double GetPointInterval()
{
   switch( Mode )
   {
    case REAL_TIME:
      return UpdateInterval / 1000.;    /* Update interval is in millisec. */

    case HISTORICAL:
      /* Historical data are sampled once per minute. */
      return HISTORICAL_DATA_INTERVAL;
 
    case CALENDAR:
      /* On Linux/Unix, localtime can not go back beyond 1900 (~40K days).
         Sample calendar data twice per day to avoid going past 1900.
         If sampling once per day, limit BufferSize to 40K.
         
         On Window, localtime can not go back beyond 1970 - only ~15K days.
         Sample calendar data four times per day to avoid going past 1900.
         If sampling once per day, limit BufferSize to 15K.
      */
#ifdef _WINDOWS
      return DAY / 4;
#else
      return DAY / 2;
#endif

    default: error( "Invalid mode", False ); return 100.;
   }
}

/*----------------------------------------------------------------------
| 
*/
void PreFillChartData()
{
   GlgLong i;   
   double
     current_time, start_time, end_time,
     num_seconds, dt;

   /* Display "Pre-filling the chart" message. */
   GlgSetDResource( ChartVP, "PreFillMessage/Visibility", 1. );
   GlgUpdate( ChartVP );

   current_time = GetCurrTime();
   
   /* Roll back by the amount corresponding to the buffer size. */
   dt = GetPointInterval();
   num_seconds = BufferSize * dt;
  
   if( Mode == REAL_TIME )
     num_seconds += 1.;  /* Add an extra second to avoid rounding errors. */

   start_time = current_time - num_seconds;
   end_time = 0.;        /* Stop at the current time. */

   for( i=0; i<NUM_PLOTS; ++i )
     FillHistData( i, start_time, end_time );

   /* Remove the "Pre-filling..." message. */
   GlgSetDResource( ChartVP, "PreFillMessage/Visibility", 0. );
}

/*----------------------------------------------------------------------
| Prefills the chart with data using simulated data. In a real application, 
| the data will be coming from an application-specific data source.
*/
void FillHistData( GlgLong plot_index, double start_time, double end_time )
{
   DataPoint data_point;
   double time_stamp, dt;   
   GlgBoolean check_curr_time;

   /* Demo: generate demo pre-fill data with the same frequency as the 
      UpdateInterval (in millisec). In an application, data will be queried
      from a real data source, returning an array of data points.
   */
   dt = GetPointInterval();

   if( !end_time )
   {
      check_curr_time = True;
      end_time = GetCurrTime();
   }
   else
     check_curr_time = False;

   /* When prefilling up to the current time, use the result of 
      GetCurrTime() as the loop's end condition and check it after
      each iteration to account for the time it takes to prefill 
      the chart.
   */
   for( time_stamp = start_time; 
        time_stamp < end_time && ( !check_curr_time || 
                                   time_stamp < GetCurrTime() );
        time_stamp += dt )
   {
      GetDemoData( plot_index, &data_point );

      /* Set the time stamp. */
      data_point.time_stamp = time_stamp;
      data_point.has_time_stamp = True;
      
      PushPlotPointDirect( plot_index, &data_point );
   }
}

/*----------------------------------------------------------------------
| Supplies demo data, including the plot's value, an optional time stamp
| and an optional sample_valid flag, as well as visibility of a mraker used
| to annotate some data pooints. 
|
| In a real application, data will be coming from an application-specific 
| data source.
*/
void GetDemoData( GlgLong plot_index, DataPoint * data_point )
{   
   GetDemoPlotValue( plot_index, data_point );  /* Fills a value to plot. */

   /* Let the chart use current time as a time stamp. Optionally,
      an application can provide a time stamp in data_point->time_stamp
      and set data_point->has_time_stamp = True.
   */
   data_point->has_time_stamp = False;

   /* Set an optional ValidEntryPoint to make some samples invalid for
      Plot0. It is optional: default=True is used for the rest of the plots 
      when ValidEntryPoint is not supplied.
   */   
   if( plot_index == 0 )
   {
      static GlgBoolean Plot0Valid = True;

      if( Plot0Valid ) 
        /* Make samples invalid occasionally. */
        Plot0Valid = ( GlgRand( 0., 100. ) > 2. );
      else
        /* Make it valid again after a while. */
        Plot0Valid= ( GlgRand( 0., 100. ) > 30. );

      data_point->value_valid = Plot0Valid;
   }
   else
     data_point->value_valid = True;
}

#define MAX_COUNTER        50000
#define PERIOD              1000
#define SPIKE_DURATION_RT     25
#define SPIKE_DURATION_HS      8
#define APPROX_PERIOD        100

/*----------------------------------------------------------------------
| Supplies plot values for the demo; also sets data_point->has_marker field
| to annotate some data points with a marker.
|
| In a real application, data will be coming from an application-specific 
| data source.
*/
void GetDemoPlotValue( GlgLong plot_index, DataPoint * data_point )
{
   GlgLong i;
   static GlgLong      
     first_error = True,
     plot_counter[ NUM_PLOTS ] = { -1 },
     state = 0, 
     spike = 0,
     change_counter = 10,
     spike_counter = 1000;
   static double max_spike_height = 0.;
   double
     value, alpha, period, 
     spike_sign, spike_height;   
   GlgLong spike_duration;
   
   /* First time: init plot's state counters used to simulate data. */
   if( plot_counter[ 0 ] == -1 )
     for( i=0; i<NUM_PLOTS; ++i )
       plot_counter[ i ] = 0;

   alpha = 2. * M_PI * plot_counter[ plot_index ] / PERIOD;
   switch( plot_index )
   {
    case 0:      
      if( Mode == REAL_TIME )           
        value = 5. + 1.5 * sin( alpha / 5. ) + sin( 2. * alpha );
      else               
      {
         static double 
           last_value = 5., 
           increment_sign = 1.,
           last_value2 = 0., 
           increment_sign2 = 1.;
         
         last_value += GlgRand( 0., 0.01 ) * increment_sign;
         last_value2 += GlgRand( 0., 0.03 ) * increment_sign2;

         value = last_value + last_value2;

         if( GlgRand( 0., 1000. ) > 995. )
           increment_sign *= -1;

         if( GlgRand( 0., 1000. ) > 750. )
           increment_sign2 *= -1;

         if( value > 6.2 )
           increment_sign2 = -1.;
         else if( value < 3.8 )
           increment_sign2 = 1.;
      }
 
      /* Add a spike */
      spike_height = 0;
      spike_duration = 
        ( Mode == REAL_TIME ? SPIKE_DURATION_RT : SPIKE_DURATION_HS );

      if( spike_counter >= spike_duration * 3 )
      {
         if( GlgRand( 0., 1000. ) > 990. ) 
         {
            /* Start a spike */
            spike_counter = 0;
            spike_sign = ( GlgRand( 0., 10. ) > 4. ? 1. : -1. );
            max_spike_height = 
              spike_sign * GlgRand( 0., Mode == REAL_TIME ? 1. : 0.5 );
         }
      }
      
      /* Annotate spikes with a marker. */
      data_point->has_marker = ( spike_counter == 0 );
      
      if( spike_counter <= spike_duration )
      {
         double spike_coeff;
         
         spike_coeff = 1. - spike_counter / (double) spike_duration;
         spike_height = 
           0.3 * max_spike_height * spike_coeff  * spike_coeff * 
           ( 1. + cos( 2. * M_PI * spike_counter / 12. ) );
      }

      ++spike_counter;
      value += spike_height; 
      break;
         
    case 1:
      if( change_counter )
      {
         --change_counter;
         
         if( !change_counter )
         {            
            state = !state;   /* Change the state */
               
            /* Time of the next change */
            change_counter = GlgRand( 10., 100. );
         }
      }
      
      value = state;
      break;
      
    case 2:
      if( Mode == REAL_TIME )
      {
         period = ( 0.95 + 0.05 * fabs( sin( alpha / 10. ) ) ); 
         value = 8.3 + sin( 30. * period * alpha ) * sin( M_PI / 8. + alpha );
         value *= 10.;
      }
      else
      {
         static double last_value3 = 70.;
         static GlgLong 
           last_direction = 1.,
           approx_counter = 0;

         value = last_value3 + last_direction * 
           0.1 * ( 1. - cos( 2. * M_PI * approx_counter / APPROX_PERIOD ) );
         
         last_value3 = value;
         if( Mode == HISTORICAL )
           approx_counter += 3;
         else
           approx_counter += 1;

         if( last_direction < 0. && value < 0.6  * Max[ plot_index ] ||
             last_direction > 0. && value > 0.95 * Max[ plot_index ] ||
             GlgRand( 0., 1000. ) > 900. )
         {
            last_direction *= -1;
            approx_counter = 0;
         }
      }
      break;
      
    default: 
      if( first_error )
      {
         first_error = False;
         error( "Add a case to provide demo data for added plots.", False ); 
      }
      value = 62.;
      break;
   }

   /* Increase the plot's state counter used to simulate demo data. */
   ++plot_counter[ plot_index ];
   plot_counter[ plot_index ] = ( plot_counter[ plot_index ] % MAX_COUNTER );

   data_point->value = value;   /* Returned simulated value. */
}

/*----------------------------------------------------------------------
| Sets the display mode: REAL_TIME, HISTORICAL or CALENDAR.
| If invoked with mode=-1, switch the mode between all three demo modes.
*/
void SetMode( GlgLong mode )
{
   double enabled;

   SelectPlot( NULL );   /* Unselect a previously selected plot, if any. */

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
      AutoScroll = True;      
      break;

    case HISTORICAL: 
    case CALENDAR:
      SpanIndex = ( Mode == HISTORICAL ? 2 : 3 );
      AutoScroll = False;
      break;

    default: error( "Invalid mode", False ); return;
   }

   /* Display the number of data points per line and the total number of 
      points. */
   GlgSetDResource( ChartVP, "NumDataPoints", (double) BufferSize );
   GlgSetDResource( ChartVP, "NumDataPointsTotal", 
                    (double) ( BufferSize * NUM_PLOTS ) );

   /* Disable "Toggle AutoScroll" button in HISTORICAL and CALENDAR modes. */
   GlgSetDResource( Drawing, "ToggleAutoScroll/HandlerDisabled", 
                    (double) ( Mode != REAL_TIME ) );

   /* Disable AutoScroll in non-real-time modes. */
   ChangeAutoScroll( AutoScroll );
   
   SetSelectorLabels();   
   SetChartSpan( SpanIndex );
   RestoreInitialYRanges();

   /* Erase the step plot, its axis and level lines in the CALENDAR mode. */
   enabled = ( Mode == CALENDAR ? 0. : 1. );
   GlgSetDResource( YAxis[1], "Visibility", enabled );
   GlgSetDResource( Plot[1], "Enabled", enabled );
   GlgSetDResource( Chart, "Levels/Level#0/Enabled", enabled );
   GlgSetDResource( Chart, "Levels/Level#1/Enabled", enabled );
   AdjustYAxisSpace();

   /* Switch label in the DemoMode button. */
   GlgSetDResource( Drawing, "DemoMode/Mode", (double) Mode );

   /* Clear all accumulated data samples: the data will be refilled according 
      to the new display mode.
   */
   GlgClearDataBuffer( Chart, NULL );

   GlgUpdate( Drawing );

   /* In the real-time mode pre-fill chart data only if PrefillData=True.
      Always prefill in the historical and calendar modes.
   */
   if( Mode != REAL_TIME || PrefillData )
     PreFillChartData();
   else
     /* Erase prefill message. */
     GlgSetDResource( ChartVP, "PreFillMessage/Visibility", 0. );
   
   ScrollToDataEnd( MOST_RECENT, False );
}

/*----------------------------------------------------------------------
| Sets the formats of time labels and tooltips depending on the demo mode 
| and the selected time span.
*/
void SetTimeFormats()
{
   char * time_label_format, * time_tooltip_format, * chart_tooltip_format;

   /* No additional code is required to use the defaults defined in the 
      drawing. This elaborate example illustrates advanced options for 
      customizing label and tooltip formatting when switching between 
      time spans and data display modes.

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

    default: error( "Invalid mode", False ); return;
   }

   /* Set time label and tooltip formats. */
   GlgSetSResource( Chart, "XAxis/TimeFormat", time_label_format );
   GlgSetSResource( Chart, "XAxis/TooltipFormat", time_tooltip_format );
   GlgSetSResource( Chart, "TooltipFormat", chart_tooltip_format );
}

/*----------------------------------------------------------------------
| Returns the exact time including fractions of seconds.
*/
double GetCurrTime()
{
   GlgULong sec, microsec;

   GlgGetTime( &sec, &microsec );
   return sec + microsec / 1000000.;
}

/*----------------------------------------------------------------------
|
*/
void error( char * string, GlgBoolean quit )
{
   GlgError( GLG_USER_ERROR, string );
   if( quit )
     exit( GLG_EXIT_ERROR );
}

/*----------------------------------------------------------------------
| 
*/
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, 
			   GlgLong interval )
{
   GlgULong sec2, microsec2;
   GlgLong elapsed_time, adj_interval;

   GlgGetTime( &sec2, &microsec2 );  /* End time */
   
   /* Elapsed time in millisec */
   elapsed_time = 
     ( sec2 - sec1 ) * 1000 + (GlgLong) ( microsec2 - microsec1 ) / 1000;

   /* Maintain constant update interval regardless of the system speed. */
   if( elapsed_time + 20 >= interval )
      /* Slow system: update as fast as we can, but allow a small interval 
         for handling input events. */
     adj_interval = 20;
   else
     /* Fast system: keep constant update interval. */
     adj_interval = interval - elapsed_time;

#if DEBUG_TIMER
   printf( "sec= %ld, msec= %ld\n", 
           (long)( sec2 - sec1 ), (long)( microsec2 - microsec1 ) );
   printf( "*** elapsed= %ld, requested= %ld, adjusted= %ld\n",
           (long) elapsed_time, (long) interval, (long) adj_interval );
#endif

   return adj_interval;
}

/*----------------------------------------------------------------------
| Adjusts for a softer edge color when run with no anti-aliasing.
*/
void AdjustButtonColor()
{
   double open_gl_used;

   GlgGetDResource( ChartVP, "OpenGL", &open_gl_used );

   if( !open_gl_used )
     /* GDI driver with no anti-aliasing: use a softer color for button's 
        edges. */
     GlgSetGResource( Drawing, "ScrollBack/BodyEdgeColor", 
                      0.774, 0.774, 0.827 );
}

/*----------------------------------------------------------------------
| The chart layout and Y axis space may be configured interactively in
| the Graphics Builder.
|
| This function adjusts the space taken by the Y axes at run time when a 
| number of displayed Y axes and/or their label layout changes. 
*/
void AdjustYAxisSpace()
{
   double axis_offset, label_offset;

   if( Mode == CALENDAR )
   {
      /* Only two axes are displayed in non-CALENDAR modes. */     
      axis_offset = -25.;

      /* YAxisLabelType == 3 needs extra space to position labels for two axes.
       */
      label_offset = ( ( YAxisLabelType == 3 ) ? 30. : 0. );
   }
   else
   {
      /* All three axes are displayed in non-CALENDAR modes. */
      axis_offset = 0.;

      /* YAxisLabelType == 3 needs extra space to position labels for three
         axes. */
      label_offset = ( ( YAxisLabelType == 3 ) ? 45. : 0. );
   }

   GlgSetDResource( ChartVP, "OffsetLeft", axis_offset + label_offset );
}

/*----------------------------------------------------------------------
| Decreases marker size for large spans.
*/
void SetMarkerSize()
{
   GlgSetDResource( Plot[0], "Marker/MarkerSize", SpanIndex < 2 ? 7. : 5. );

   /* Enable smoother sub-pixel scrolling of markers. */
   GlgSetDResource( Plot[0], "Marker/AntiAliasing", 
                    (double) GLG_ANTI_ALIASING_DBL );
}
