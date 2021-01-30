/***************************************************************************
  This example demonstrates how to display and update a GLG realtime
  stripchart widget.

  The example instantiates GlgChart class and loads a stripchart widget 
  "chart2.g".

  GlgChart class is derived from GlgObjectC and encapsulates methods
  for initializing the chart, updating the chart with data.

  For demo pourposes, the chart displays simulated data. In a real 
  application, the data will be coming from an application-specific 
  data source.
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "GlgClass.h"
#include "GlgChart.h"

#define NUM_PLOTS 3

GlgAppContext AppContext;

// Defines a platform-specific program entry point.
#include "GlgMain.h"

/*----------------------------------------------------------------------
| Main Entry point.
*/
int GlgMain( int argc, char *argv[], GlgAppContext InitAppContext )
{
   GlgSessionC glg_session( False, InitAppContext, argc, argv );

   // Instantiate GlgChart.
   GlgChart glg_chart;

   AppContext = glg_session.GetAppContext();

   // Store AppContext for the glg_viewer instance.
   glg_chart.AppContext = AppContext;

   // Load GLG drawing from a file.
   glg_chart.LoadWidget( "chart2.g" );

   // Set widget dimensions using world coordinates [-1000;1000].
   // If not set, default dimensions will be used as set in the GLG editor.
   //
   glg_chart.SetResource( "Point1", 0., 0., 0. );
   glg_chart.SetResource( "Point2", 0., 0., 0. );
   glg_chart.SetResource( "Screen/WidthHint",  800. );
   glg_chart.SetResource( "Screen/HeightHint", 600. );
   
   // Setting the window title. */
   glg_chart.SetResource( "ScreenName", 
			  "GLG RealTime Stripchart Simple Example" );

   // Set chart parameters, such as number of plots and data range.
   glg_chart.NumPlots = NUM_PLOTS;
   glg_chart.Low = 0.;
   glg_chart.High = 10.;

   // Initialize the chart.
   glg_chart.Init();
      
   // Display the chart.
   glg_chart.InitialDraw();

   // Start dynamic updates.
   glg_chart.StartUpdates();

   return (int) GlgMainLoop( AppContext );
}

