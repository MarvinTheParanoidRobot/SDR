#include <stdio.h>
#include <stdlib.h>
#include "Gvf.h"

#define FORMAT0          0      /* shore lines */
#define FORMAT1          1      /* states */
#define FORMAT2          2      /* political boundaries */
#define FORMAT3          3      /* CIA World Database, ascii */

/*--------------------------------------------------------------------------
| This utility is used to convert some publicly available data
| (shore lines, states and political boundaries). It may be used to save
| a file in the Glg Map Server format, or may be used as a filter to convert
| the data on the fly (see shores.lif or states.lif for examples).
*/
int main( int argc, char ** argv )
{
   FILE * map_file;
   double
     py_min_x, py_max_x,
     py_min_y, py_max_y,
     min_x = 0., max_x = 0.,
     min_y = 0., max_y = 0.,
     area,
     x, y,
     last_x, last_y,
     error;
   long
     i,
     format,
     first_point = 1,
     num_read,
     id,
     rank,
     num_points,
     level,
     count,
     point_count;
   char 
     * filename,
     unknown_char,
     buffer[ 1001 ];

   if( argc < 3 )
   {
      fprintf( stderr, "Usage: convert format_index filename\n" );
      exit( GLG_EXIT_ERROR );
   }

   sscanf( argv[1], "%d", &format );
   filename = argv[2];

   map_file = fopen( filename, "rb" );
   if( map_file == NULL )
   {
      fprintf( stderr, "could not read %s\n", filename );
      exit( GLG_EXIT_ERROR );
   }
  
   GvfWriteHeader( stdout, GVF_ASCII ); /* writes the header */

   count = 0;

   while( 1 )
   {
      /* read the header */
      switch( format )
      {
       case FORMAT0:
	 num_read = 
	   fscanf( map_file, " P%ld%ld%ld %c%lf%lf%lf%lf%lf\n",
		  &id, &num_points, &level, &unknown_char,
		  &area, &py_min_x, &py_max_x, &py_min_y, &py_max_y );
	 if( num_read != 9 )
	 {
	    if( num_read <= 0 && feof( map_file ) )
	      goto finish;
	    else
	    {
	       fprintf( stderr, "error reading data file\n" );
	       exit( GLG_EXIT_ERROR );
	    }
	 }

	 /* no attributes in this polygon */
	 GvfWritePolygon( stdout, GVF_ASCII, 0, num_points, 0 ); 
	 break;
	  
       case FORMAT1:
	 num_read = 
	   fscanf( map_file, "%ld,%ld,%1000s", &id, &num_points, buffer );
	 if( num_read != 3 )
	 {
	    if( num_read <= 0 && feof( map_file ) )
	      goto finish;
	    else
	    {
	       fprintf( stderr, "error reading data file\n" );
	       exit( GLG_EXIT_ERROR );
	    }
	 }

	 /* no attributes in this polygon */
	 GvfWritePolygon( stdout, GVF_ASCII, 0, num_points, 0 );
	 break;
	  
       case FORMAT2:
	 /* every poly has 2 points */
	 num_points = 2;
	 break;
	  
       case FORMAT3:
	 num_read = 
	   fscanf( map_file, " segment%ld rank%ld points%ld",
		  &id, &rank, &num_points );
	 if( num_read != 3 )
	 {
	    if( num_read <= 0 && feof( map_file ) )
	      goto finish;
	    else
	    {
	       fprintf( stderr, "error reading data file\n" );
	       exit( GLG_EXIT_ERROR );
	    }
	 }

	    /* 1 attribute: rank */
	 GvfWritePolygon( stdout, GVF_ASCII, 0, num_points, 1 );
	 GvfWriteAttribute( stdout, GVF_ASCII, 0, GLM_D, (double)rank, NULL,
			   0., 0., 0. );
	 break;

       default: 
	 error = (double)format;
	 fprintf( stderr, "unknown data file format %lf\n", error );
	 exit( GLG_EXIT_ERROR );
      }

      point_count = 0;
      for( i=0; i<num_points; ++i )
      {
	 switch( format )
	 {
	  case FORMAT0:
	    if( fscanf( map_file, "%lf%lf", &x, &y ) != 2 )
	    {
	       fprintf( stderr, "error reading data file\n" );
	       exit( GLG_EXIT_ERROR );
	    }
	    goto common;

	  case FORMAT1:	    
	    if( fscanf( map_file, "%lf,%lf", &x, &y ) != 2 )
	    {
	       fprintf( stderr, "error reading data file\n" );
	       exit( GLG_EXIT_ERROR );
	    }
	    goto common;

	  case FORMAT3:	    
	    if( fscanf( map_file, "%lf%lf", &y, &x ) != 2 )
	    {
	       fprintf( stderr, "error reading data file\n" );
	       exit( GLG_EXIT_ERROR );
	    }
	    if( x <= -360. )
	      x += 360.;
	    goto common;

	  common:
	    if( i == 0 )
	    {		
	       last_x = x;
	       last_y = y;
	    }

	    /* we are enclosing the bottom, and not the top: [bottom;top) */

	    if( x - last_x == 360. )
	      x -= 360.;
	      
	    /* writes a point */
	    GvfWritePoint( stdout, GVF_ASCII, x, y, "%lf %lf " );

	    last_x = x;
	    last_y = y;
	    break;
	      	      
	  case FORMAT2:
	    if( ( num_read = fscanf( map_file,"%lf%lf", &y, &x ) ) != 2 )
	    {
	       if( num_read <= 0 && feof( map_file ) )
	       {
		  goto finish;
	       }

	       /* else */
	       fprintf( stderr, "error reading points in data file\n" );
	       exit( GLG_EXIT_ERROR );
	    }

	    if( i == 0 )
	    {		
	       /* no attributes in this polygon */
	       GvfWritePolygon( stdout, GVF_ASCII, 0, num_points, 0 );

	       last_x = x;
	       last_y = y;
	    }

	    if( x - last_x == 360. )
	      x -= 360.;

	    /* writes a point */
	    GvfWritePoint( stdout, GVF_ASCII, x, y, "%lf %lf " );
	    break;

	  default: 
	    error = (double)format;
	    fprintf( stderr, "unknown data file format %lf\n", error );
	    exit( GLG_EXIT_ERROR );
	 }

	 ++point_count;
	       
	 if( first_point )
	 {
	    min_x = max_x = x;
	    min_y = max_y = y;
	    first_point = 0;
	 }
	 else
	 {
	    if( x < min_x )
	      min_x = x;
	    else if( x > max_x )
	      max_x = x;
	     
	    if( y < min_y )
	      min_y = y;
	    else if( y > max_y )
	      max_y = y;
	 }
      }
      ++count;
   }
  
 finish:
   if( map_file )
     fclose( map_file );

   if( first_point )
   {
      fprintf( stderr, "error: no points were read\n" );
      exit( GLG_EXIT_ERROR );
   }

   exit( GLG_EXIT_OK );
}

