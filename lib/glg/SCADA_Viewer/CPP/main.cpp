#include <stdio.h>
#include <stdlib.h>
#include "GlgSCADAViewer.h"

/* Default configurattion filename. */
SCONST char * DEFAULT_CONFIG_FILENAME = "scada_config_menu.txt";

// Defines a platform-specific program entry point.
#include "GlgMain.h"

/////////////////////////////////////////////////////////////////////////////
// Main Entry point.
/////////////////////////////////////////////////////////////////////////////
int GlgMain( int argc, char *argv[], GlgAppContext InitAppContext )
{
   GlgSessionC glg_session( False, InitAppContext, argc, argv );
   GlgSCADAViewer viewer;
   SCONST char * config_filename = DEFAULT_CONFIG_FILENAME;
   SCONST char * exe_path = argv[0];     /* Full path of the executable */
   SCONST char * full_path;
   long skip;
   GlgAppContext AppContext;

   AppContext = glg_session.GetAppContext();

   // Store AppContext for the viewer instance.
   viewer.AppContext = AppContext;
   viewer.AlarmUpdateInterval = 1000;        /* Query alarms once a second. */

   /* By default, use simulated data to demo or to test without live data.
      Set to false or use -live-data command-line argument to use 
      custom live data feed.
   */
   viewer.RANDOM_DATA = GlgTrue;

   // Process command line arguments.   
   for( skip = 1; skip < argc; ++skip )
   {
      /* Use configuration file supplied as a command-line argument. */ 
      if( strcmp( argv[ skip ], "-config" ) == 0 )
      {
	 ++skip;
	 if( argc <= skip )
	 {
	    GlgError( GLG_USER_ERROR, "Missing configuration file name." );
	    break;
	 }
	 config_filename = argv[ skip ];
      }
      else if( strcmp( argv[ skip ], "-random-data" ) == 0 )
        viewer.RANDOM_DATA = GlgTrue;
      else if( strcmp( argv[ skip ], "-live-data" ) == 0 )
        viewer.RANDOM_DATA = GlgFalse;
   }

   // Fill MenuArray from a supplied configuration file.
   viewer.FillMenuArray( exe_path, config_filename );

   /* Load the main top level drawing using the full file path. */    
   full_path = GlgGetRelativePath( exe_path, "scada_main.g" );
   viewer.LoadWidget( full_path );
   GlgFree( (void*) full_path );

   if( viewer.IsNull() )
   {
      GlgError( GLG_USER_ERROR, "Can't load drawing." );
      exit( GLG_EXIT_ERROR );
   }

   /* Set widget dimensions. If not set, default dimensions will 
      be used as set in the GLG editor.
   */
   viewer.SetSize( 0, 0, 900., 700. );

   // Setting the window title. */
   viewer.SetResource( "ScreenName", "GLG SCADA Viewer Example" );

   // Initialize glg control.
   viewer.Init();
      
   // Display the control.
   viewer.InitialDraw();
   
   // Start dynamic updates.
   viewer.StartUpdates();

   return (int) GlgMainLoop( AppContext );
}

