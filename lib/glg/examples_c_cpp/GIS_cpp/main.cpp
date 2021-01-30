/**********************************************************************
| Supported command line arguments:
| -random-data  
|     Use simulated demo data for animation.
| -live-data
|     Use live application data.
| <filename>
|        specifies GLG drawing filename to be loaded and animated;
|        if not defined, DEFAULT_DRAWING_FILENAME is used.
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "GlgMapViewer.h"
#include "DemoDataFeed.h"
#include "LiveDataFeed.h"

// Default GLG drawing filename. Custom drawing may be supplied as
// the first command line argument.
CONST char* DEFAULT_DRAWING_FILENAME="gis_example.g";

GlgAppContext AppContext;

// Defines a platform-specific program entry point.
#include "GlgMain.h"

/////////////////////////////////////////////////////////////////////////////
// Main Entry point.
/////////////////////////////////////////////////////////////////////////////
int GlgMain( int argc, char *argv[], GlgAppContext InitAppContext )
{
   GlgSessionC glg_session( False, InitAppContext, argc, argv );
   GlgMapViewer glg_viewer;
   CONST char * drawing_filename = NULL;

   AppContext = glg_session.GetAppContext();

   // Store AppContext for the glg_viewer instance.
   glg_viewer.AppContext = AppContext;
   
   // Process command line arguments.   
   for( int skip = 1; skip < argc; ++skip )
   {
      if( strcmp( argv[ skip ], "-random-data" ) == 0 )
      {
         // Use simulated demo data for animation.
         glg_viewer.RANDOM_DATA = True;
         GlgError( GLG_INFO, (char *) "Using simulated data for animation." );
      }
      else if( strcmp( argv[ skip ], "-live-data" ) == 0 )
      {
         // Use live application data for animation.
         glg_viewer.RANDOM_DATA = False;
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
                (char *) "Using default drawing gis_example.g." );
      drawing_filename = DEFAULT_DRAWING_FILENAME;
   }

   // Load GLG drawing from a file.
   glg_viewer.LoadWidget( drawing_filename );
   if( glg_viewer.IsNull() )
   {
      GlgError( GLG_USER_ERROR, (char *) "Can't load drawing." );
      exit( GLG_EXIT_ERROR );
   }

   /* Set widget dimensions. If not set, default dimensions will 
      be used as set in the GLG editor.
   */
   glg_viewer.SetSize( 0, 0, 800, 700 );

   // Setting the window title. */
   glg_viewer.SetResource( "ScreenName", "GLG GIS Example" );
   
   // Create DataFeed object.
   if( glg_viewer.RANDOM_DATA )
     glg_viewer.AddDataFeed( new DemoDataFeed( &glg_viewer ) );
   else
     glg_viewer.AddDataFeed( new LiveDataFeed( &glg_viewer ) );

   // Initialize glg control.
   glg_viewer.Init();
   
   // Display the control.
   glg_viewer.InitialDraw();

   // Start dynamic updates.
   glg_viewer.StartUpdates();

   return (int) GlgMainLoop( AppContext );
}

