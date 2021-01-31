#pragma once

#include "HMIPageBase.h"

/* Constants for scrolling to the ends of the time range. */
#define DONT_CHANGE    0
#define MOST_RECENT    1  /* Make the most recent data visible. */
#define LEAST_RECENT   2  /* Make the least recent data visible.*/

class RTChartPage : public HMIPageBase
{
 public:
   RTChartPage( GlgSCADAViewer * viewer );
   ~RTChartPage( void );

 public:
   /* Additional fields may be added as needed. */
   GlgObjectC Viewport;   // top level viewport of the loaded page
   GlgObjectC ChartVP;    // viewport containing the Chart object
   GlgObjectC Chart;      // Chart object

   int SpanIndex; /* Index of the currently displayed time span.*/

   /* Number of lines and Y axes in ga chart as defined in the drawing. */
   int NumPlots;
   int NumYAxes;

   /* Store object IDs for each plot,  used for performance optimization 
      in the chart data feed.
   */
   GlgObjectC * PlotArray;

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

 public:
   // Callbacks.
   GlgBoolean Input( GlgObjectC& viewport, GlgObjectC& message );
   GlgBoolean Trace( GlgObjectC& callback_viewport, 
                             GlgTraceCBStruct * trace_data );
   void Ready( void );

   // Initialization
   void InitBeforeSetup( void );
   void InitAfterSetup( void );
   
   // Tag reassignment.
   GlgBoolean NeedTagRemapping( void );
   void RemapTagObject( GlgObjectC& tag_obj, SCONST char * tag_name, 
                                SCONST char * tag_source );
   

   // Chart data supply.
   int GetUpdateInterval( void );
   GlgBoolean UpdateData( void );
   void FillChartHistory( void );
   void FillPlotData( GlgObjectC& plot, PlotDataArrayType& data_array );

   // Misc methods
   void ChangeAutoScroll( int new_value );
   void SetChartSpan( int span_index );
   void SetTimeFormats( void );
   void ScrollToDataEnd( int data_end, GlgBoolean show_extra );
   double GetExtraSeconds( int time_span );
   void RestoreInitialYRanges( void );
   GlgBoolean ZoomToMode( void );
   void AbortZoomTo( void );
};
