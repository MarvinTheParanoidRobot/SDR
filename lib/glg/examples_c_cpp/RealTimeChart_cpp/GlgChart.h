#pragma once

#ifdef _WINDOWS
# pragma warning( disable : 4244 )
#endif

#include "DataFeedBase.h"

typedef enum
{
   BUTTON_PRESS = 0,
   RESIZE,
   MOUSE_MOVE
} EventType;

class GlgChart : public GlgObjectC
{
 public:
   GlgChart( void );
   ~GlgChart( void );
   
 public:
   GlgAppContext AppContext;   
   GlgBoolean RandomData;
   GlgLong TimerID;
 
   /* Used for supplying chart data. */
   DataFeedBase * DataFeed;     
   
   /* Store object IDs for each plot. Used for performance 
      optimization in the chart data feed.
   */
   GlgObjectC * Plots;

   GlgObjectC ChartVP;
   GlgObjectC Chart;

   /* If set to true, the application supplies time stamp for each data 
      sample explicitly. Otherwise, time stamp is automatically 
      generated by the chart using current time.
   */
   GlgBoolean SupplyTimeStamp;

   // Chart orientation - HORIZONTAL or VERTICAL.                       
   GlgLong ChartOrientation;
   
   GlgBoolean PrefillData;
   GlgLong AutoScroll;
   GlgLong StoredScrollState;   
   GlgLong BufferSize;
   GlgLong UpdateInterval;      
   GlgLong TimeSpan;
   
   /* An array of low/high data range for each Y axis. */
   double * Low;
   double * High;
   
   /* Number of plots in the chart. */
   GlgLong NumPlots;
   GlgLong NumYAxes;

   /* Scroll increment for the X axis. */   
   double ScrollFactor;     

   // Input callback.
   virtual void Input( GlgObjectC& callback_viewport, GlgObjectC& message);

   // Trace callback.
   virtual void Trace( GlgObjectC& callback_viewport,
		       GlgTraceCBStruct * trace_info );

   void Init( void );
   void InitBeforeH( void );
   void InitAfterH( void );
   void AddDataFeed();
   void ScrollToDataEnd( void );
   void ChangeAutoScroll( GlgLong new_state );
   void SetChartSpan( GlgLong span );
   void RestoreInitialYRanges( void );
   GlgLong ZoomToMode( void );
   void AbortZoomTo( void );
   void FillChartHistory( void );
   void FillPlotData( GlgObjectC& plot, DataArrayType& data_array );
   void PushPlotPoint( GlgObjectC& plot, DataPoint& data_point );
   double GetCurrTime( void );
   void StartUpdates( void );
   void StopUpdates( void );
   void SetSize( GlgLong x, GlgLong y, GlgLong width, GlgLong height );
   void error( CONST char * string, GlgBoolean quit );

   GlgChart& operator= ( const GlgObjectC& object );
};

