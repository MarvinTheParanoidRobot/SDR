#ifndef _RTChartPage_h_
#define _RTChartPage_h_

#include "GlgApi.h"
#include "HMIPage.h"
#include "GlgSCADAViewer.h"

/* Constants for scrolling to the ends of the time range. */
#define DONT_CHANGE    0
#define MOST_RECENT    1  /* Make the most recent data visible. */
#define LEAST_RECENT   2  /* Make the least recent data visible.*/

typedef struct _RTChartPage
{
   HMIPage HMIPage;
   
   /* Additional fields may be added as needed. */

   GlgObject Viewport;
   GlgObject ChartVP;
   GlgObject Chart;

   int SpanIndex; /* Index of the currently displayed time span.*/

   /* Number of lines and Y axes in a chart as defined in the drawing. */
   int NumPlots;
   int NumYAxes;

   /* Store object IDs for each plot,  used for performance optimization 
      in the chart data feed.
   */
   GlgObject * PlotArray;

   /* Lists that hold initial Low and High ranges of the Y axes in the drawing.
      Plots' Low and High are assumed to be linked to the corresponding Y axes.
   */
   double * Low;
   double * High;

   int TimeSpan; /* Time axis span in sec. */

   GlgBoolean PrefillData;     /* Setting to False suppresses pre-filling the 
                                  chart's buffer with data on start-up. */
   int AutoScroll;             /* Current auto-scroll state: enabled(1) or 
                                  disabled(0). */
   int StoredScrollState;      /* Stored AutoScroll state to be restored 
                                  if ZoomTo is aborted. */

} RTChartPage;

HMIPage * CreateRTChartPage( void );

static void rtcDestroy( HMIPage * hmi_page );
static int rtcGetUpdateInterval( HMIPage * hmi_page );
static GlgBoolean rtcUpdateData( HMIPage * hmi_page );
static GlgBoolean rtcInputCB( HMIPage * hmi_page, GlgObject viewport, 
                              GlgAnyType client_data, GlgAnyType call_data );
static GlgBoolean rtcTraceCB( HMIPage * hmi_page, GlgObject viewport, 
                              GlgAnyType client_data, GlgAnyType call_data );
static GlgBoolean rtcNeedTagRemapping( HMIPage * hmi_page );
static void rtcRemapTagObject( HMIPage * hmi_page, GlgObject tag_obj, 
                               char * tag_name, char * tag_source );
static void rtcInitBeforeSetup( HMIPage * hmi_page );
static void rtcInitAfterSetup( HMIPage * hmi_page );
static void rtcReady( HMIPage * hmi_page );

static void rtcFillChartHistory( RTChartPage * rtc_page );
static void rtcFillPlotData( RTChartPage * rtc_page, GlgObject plot,
                             GlgObject data_array );
static void rtcChangeAutoScroll( RTChartPage * rtc_page, int new_value );
static void rtcSetChartSpan( RTChartPage * rtc_page, int span_index );
static void rtcSetTimeFormats( RTChartPage * rtc_page );
static void rtcScrollToDataEnd( RTChartPage * rtc_page, 
                                int data_end, GlgBoolean show_extra );
static double GetExtraSeconds( int time_span );
static void rtcRestoreInitialYRanges( RTChartPage * rtc_page );
static GlgBoolean rtcZoomToMode( RTChartPage * rtc_page );
static void rtcAbortZoomTo( RTChartPage * rtc_page );

#endif
