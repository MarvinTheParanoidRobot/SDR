#pragma once

#ifdef _WINDOWS
# pragma warning( disable : 4244 )
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "GlgClass.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

class DataPoint
{
 public:
   double value;
   bool value_valid;
   bool has_time_stamp;
   double time_stamp;
		    
 public:
   DataPoint( void ) {};
   ~DataPoint( void ) {};
};

class GlgChart : public GlgObjectC
{
 public:
   GlgChart();
   ~GlgChart(void);
   
 public:

   void Init( void );
   void InitBeforeH( void );
   void InitAfterH( void );

   void GetPlotPoint( int plot_index, DataPoint& data_point );
   void PushPlotPoint( int plot_index, DataPoint& data_point );
   double GetDemoValue( int plot_index );

   void StartUpdates( void );
   void StopUpdates( void );
   void error( CONST char * string, GlgLong quit );

   // Input callback.
   virtual void Input( GlgObjectC& callback_viewport, GlgObjectC& message);

   /* Store object IDs for each plot. Used for performance 
      optimization in the chart data feed.
   */
   GlgObjectC * Plots;
   
   GlgLong UpdateInterval;      
   GlgLong TimeSpan;
   
   /* Low and High range of the incoming data. */
   double Low;
   double High;
   
   /* Number of plots in the chart. */
   GlgLong NumPlots;
   
   /* Number of plots as defined in .g file. */
   GlgLong num_plots_drawing;  

   GlgLong TimerID;
   GlgAppContext AppContext;

   GlgChart& operator= ( const GlgObjectC& object );
};

