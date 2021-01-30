#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "Gvf.h"

/*--------------------------------------------------------------------------
| This utility is used to convert city data. It may be used to save
| a file in the Glg Map Server format, or may be used as a filter to convert
| the data on the fly (see cities.lif for an example).
*/
int main( int argc, char ** argv )
{
  FILE * cities_file;
  char * filename;
  char state[5];
  char city[1024];
  double lat, lon, elev;
  long pop;
  long num_read;
  int num_cities = 0;

  if( argc < 2 )
    {
      fprintf( stderr, "not enough parameters\n" );
      exit( 0 );
    }
  
  filename = argv[1];
  
  cities_file = fopen( filename, "rb" );
  if( !cities_file )
  {
     fprintf( stderr, "cannot open file %s\n", filename );
     exit( 0 );
  }
  
  GvfWriteHeader( stdout, GVF_ASCII ); /* write the header */
  
  while(1)
  {
     num_read = fscanf( cities_file, 
		       "%2s %1022[^ ] %lf %lf %ld %lf", 
		       state, city, &lat, &lon, &pop, &elev );

     if( num_read != 6 )
     {
	if( num_read <= 0 && feof( cities_file ) )
	{
	   fclose( cities_file );
	   exit( 1 );
	}
	fprintf( stderr, "error reading cities data\n" );
	exit( 0 );
     }

     GvfWriteMarker( stdout, GVF_ASCII, GVF_M_MARKER, 0., 0., 0, NULL, 3 );
     GvfWriteAttribute( stdout, GVF_ASCII, 0, GLM_S, 0., city, 0., 0., 0. );
     GvfWriteAttribute( stdout, GVF_ASCII, 1, GLM_D, (double)pop, NULL, 
		       0., 0., 0. );
     GvfWriteAttribute( stdout, GVF_ASCII, 2, GLM_S, 0., state, 0., 0., 0. );
     GvfWritePoint( stdout, GVF_ASCII, lon, lat, "%lf %lf " );
     ++num_cities;
  }
}

