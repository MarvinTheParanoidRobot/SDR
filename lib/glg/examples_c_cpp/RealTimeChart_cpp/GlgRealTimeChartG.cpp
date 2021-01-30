/***************************************************************************
  This example demonstrates how to display and update a GLG realtime
  stripchart widget, as well use the widget's integrated interactive 
  behavior.
  
  Supported command line options:
    -random-data         (use simulated demo data)
    -live-data           (use live application date from LiveDataFeed)
    drawing_name         (specifies GLG drawing to be loaded an animated

  By default, the example uses "stripchart.g", containing a horizontal 
  stripchart and a toolbar with controls allowing to zoom and scroll the chart.

  "stripchart_vertical.g" is similar to stripchart.g, but uses a
  vertical stripchart. 

  GlgChart derives from GlgObjectC and encapsulates methods
  for initializing the chart, updating the chart with data and handling
  user interaction. 

  For demo pourposes, the chart displays simulated data, using
  DemoDataFeed class, which derives from DataFeedC defined 
  in DataFeed.h. The application may provide a custom implementation 
  of the DataFeedC, supplying real-time application specific data. 
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "GlgClass.h"
#include "GlgChart.h"

GlgAppContext AppContext;

// Defines a platform-specific program entry point.
#include "GlgMain.h"

/*----------------------------------------------------------------------
| Main Entry point.
*/
int GlgMain( int argc, char *argv[], GlgAppContext InitAppContext )
{
   /* To use live data, set  RANDOM_DATA=false, or use
      -live-data command line option.
   */
   GlgBoolean RANDOM_DATA = true;

   GlgSessionC glg_session( False, InitAppContext, argc, argv );
   GlgChart glg_chart;

   glg_chart.RandomData = RANDOM_DATA;
   AppContext = glg_session.GetAppContext();
   glg_chart.AppContext = AppContext;

   // GLG drawing filename, may be supplied on a command line.
   CONST char * drawing_filename = NULL;
   
   // Use default drawing name unless specified as a command line argument.
   CONST char * DEFAULT_DRAWING_FILENAME = "stripchart.g";

   // Process command line arguments.   
   for( int skip = 1; skip < argc; ++skip )
   {
      if( strcmp( argv[ skip ], "-random-data" ) == 0 )
      {
         // Use simulated demo data for animation.
         glg_chart.RandomData = true;
         GlgError( GLG_INFO, (char *) "Using simulated data for animation." );
      }
      else if( strcmp( argv[ skip ], "-live-data" ) == 0 )
      {
         // Use live application data for animation.
         glg_chart.RandomData = false;
         GlgError( GLG_INFO, 
                   (char *) "Using live application data for animation." );
      }
      else if( strncmp( argv[skip], "-", 1 ) == 0 )
        continue;
      else
      {
         // Use the drawing file from the command line, if any.
         drawing_filename = argv[ skip ];
      }
   }

   /* If drawing file is not supplied on the command line, use 
      default drawing filename defined by DEFAULT_DRAWING_FILENAME.
   */
   if( !drawing_filename )
   {
      GlgError( GLG_INFO, 
                (char *) "Drawing file is not supplied on command line, using stripchart.g by default." );
      drawing_filename = DEFAULT_DRAWING_FILENAME;
   }

   // Load GLG drawing from a specified file.
   glg_chart.LoadWidget( drawing_filename );

   if( glg_chart.IsNull() )
   {
      GlgError( GLG_USER_ERROR, (char *) "Can't load drawing." );
      exit( GLG_EXIT_ERROR );
   }

   // Set widget dimensions in screen cooridnates.
   glg_chart.SetSize( 0, 0, 800, 600 );
   
   // Set window title. */
   glg_chart.SetResource( "ScreenName", "GLG RealTime Stripchart Example" );

   /* Initialize the chart. Init() also adds DataFeed object used to 
      supply data for animation.
   */
   glg_chart.Init();

   // Display the chart.
   glg_chart.InitialDraw();

   // Start dynamic updates.
   glg_chart.StartUpdates();

   return (int) GlgMainLoop( AppContext );
}

