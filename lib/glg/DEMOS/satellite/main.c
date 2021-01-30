#include <stdio.h>
#include "GlgApi.h"
#include "util.h"

#include "GlgMain.h"    /* Cross-platform entry point. */

int GlgMain( int argc, char * argv[], GlgAppContext app_context )
{   
   GlgLong skip;
   GlgBoolean
     TrajectoryDemo = False,
     has_demo_option = False;

#ifndef _WINDOWS
   /* Start as a Trajectory Demo if invoked through the trajectory
      symbolic link on Unix. 
      */
   if( strstr( argv[0], "trajectory" ) || 
       strstr( argv[0], "trajectory_no_opengl" ) )
     TrajectoryDemo = True;
#endif

   for( skip = 1; skip < argc; ++skip )
   {
      /* Handle options to start as either Satellite or Trajectory demo. */
      if( strcmp( argv[ skip ], "-satellite" ) == 0 )
      {
         TrajectoryDemo = False;
         has_demo_option = True;
      }
      else if( strcmp( argv[ skip ], "-trajectory" ) == 0 )
      {
         TrajectoryDemo = True;
         has_demo_option = True;
      }
   }

   if( !has_demo_option )
     DisplayUsage();

   if( TrajectoryDemo )
     return TrajectoryMain( argc, argv, app_context );
   else
     return SatelliteMain( argc, argv, app_context );
}

void DisplayUsage()
{
#ifndef _WINDOWS            
   printf( "Command-line options: \n" );
   printf( "   -satellite for Satellite demo\n" );
   printf( "   -trajectory for Trajectory demo\n" );
#endif
}
