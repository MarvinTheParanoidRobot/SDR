#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Gvf.h"

void GvfWriteHeader( FILE * file, GvfSaveFormat mode )
{
   fprintf( file, "%lx %lx %lx ",
	   (long)GVF_MAGIC, (long)GVF_VERSION, (long)mode );
}

void GvfWritePolygon( FILE * file, GvfSaveFormat mode, GlgLong ring, 
		     GlgLong num_points, GlgLong num_attr )
{
   GlgLong lvalue;

   if( mode == GVF_ASCII )
   {
      /* changed \r\n for checking pipe buffer size */
      fprintf( file, "\r\n%lx %lx %lx %lx %lx ", 
	      (long)GVF_OBJ_HEADER, (long)GVF_POLYGON, 
	      (long)ring, (long)num_points, (long)num_attr );

      /* the ring is for polygons with holes. if ring is 0, it will render, 
	 otherwise it is an extension */
   }
   else
   {
      lvalue = GVF_OBJ_HEADER;
      fwrite( &lvalue, sizeof( lvalue ), 1, file );

      lvalue = GVF_POLYGON;
      fwrite( &lvalue, sizeof( lvalue ), 1, file );

      fwrite( &ring, sizeof( ring ), 1, file );
      fwrite( &num_points, sizeof( num_points ), 1, file );
      fwrite( &num_attr, sizeof( num_attr ), 1, file );
   }
}

void GvfWriteMarker( FILE * file, GvfSaveFormat mode,
		    GvfMarkerType type, 
		    double scale, double angle, GlgLong anchor, char * string, 
		    GlgLong num_attr )
{
   GlgLong lvalue;
   
   if( mode == GVF_ASCII )
   {
      fprintf( file, "\r\n%lx %lx %lx ", 
	      (long)GVF_OBJ_HEADER, (long)GVF_MARKER, (long)type );
   }
   else
   {
      lvalue = GVF_OBJ_HEADER;
      fwrite( &lvalue, sizeof( lvalue ), 1, file );

      lvalue = GVF_MARKER;
      fwrite( &lvalue, sizeof( lvalue ), 1, file );

      lvalue = type;
      fwrite( &lvalue, sizeof( lvalue ), 1, file );
   }

   switch( type )
   {
    case GVF_M_MARKER:
      break;
    case GVF_M_TEXT:
      if( mode == GVF_ASCII )
	fprintf( file, "%lf %lf %lx ", scale, angle, (long)anchor );
      else
      {
	 fwrite( &scale, sizeof( scale ), 1, file );
	 fwrite( &angle, sizeof( angle ), 1, file );
	 fwrite( &anchor, sizeof( anchor ), 1, file );
      }
    case GVF_M_LABEL:
      GvfWriteString( file, mode, string ); /* for both text and label */
      break;
    default:
      fprintf( stderr, "Unsupported marker type\n" );
      break;
   }

   if( mode == GVF_ASCII )
     fprintf( file, "%lx ", (long)num_attr );
   else
     fwrite( &num_attr, sizeof( num_attr ), 1, file );     
}

void GvfWritePoint( FILE * file, GvfSaveFormat mode, double x, double y, 
		   char * format )
{
   if( mode == GVF_ASCII )
     fprintf( file, format, x, y );
   else
   {
      fwrite( &x, sizeof( x ), 1, file );     
      fwrite( &y, sizeof( x ), 1, file );     
   }
}

void GvfWriteAttribute( FILE * file, GvfSaveFormat mode, GlgLong name, 
		       GlmDataType type, 
		       double d_val, 
		       char * s_val, 
		       double g_val_x, double g_val_y, double g_val_z )
{
   GlmDataType real_type;
   GlgLong lvalue;

   if( type == GLM_I )
     real_type = GLM_D;
   else
     real_type = type;

   if( mode == GVF_ASCII )
     fprintf( file, "%lx %lx ", (long)name, (long)real_type );
   else
   {
      fwrite( &name, sizeof( name ), 1, file ); 
      lvalue = real_type;
      fwrite( &lvalue, sizeof( lvalue ), 1, file ); 
   }

   switch( type )
   {
    case GLM_I:
      if( mode == GVF_ASCII )
	fprintf( file, "%.0lf ", d_val );
      else
	fwrite( &d_val, sizeof( d_val ), 1, file ); 
      break;
      
    case GLM_D:
      if( mode == GVF_ASCII )
	fprintf( file, "%lf ", d_val );
      else
	fwrite( &d_val, sizeof( d_val ), 1, file ); 
      break;
      
    case GLM_G:
      if( mode == GVF_ASCII )
	fprintf( file, "%lf %lf %lf ", g_val_x, g_val_y, g_val_z );
      else
      {
	 fwrite( &g_val_x, sizeof( g_val_x ), 1, file ); 
	 fwrite( &g_val_y, sizeof( g_val_y ), 1, file ); 
	 fwrite( &g_val_z, sizeof( g_val_z ), 1, file ); 
      }
      break;

    case GLM_S:
      GvfWriteString( file, mode, s_val );
      break;

    default:
      fprintf( stderr, "Unsupported data type\n" );
      break;
   }
}

void GvfWriteString( FILE * file, GvfSaveFormat mode, char * string )
{
   GlgLong length;

   if( !string )
   {
      if( mode == GVF_ASCII )
	fprintf( file, "-1 " );
      else
      {
	 length = -1;
	 fwrite( &length, sizeof( length ), 1, file ); 	
      }
   }
   else
   {  
      length = strlen( string );

      if( mode == GVF_ASCII )
      {
	 fprintf( file, "%lx ", (long)length );
	 fwrite( string, length + 1, 1, file );
	 fprintf( file, " " );
      }
      else
      {
	 fwrite( &length, sizeof( length ), 1, file ); 	
	 fwrite( string, length + 1, 1, file );
      }
   }
}
