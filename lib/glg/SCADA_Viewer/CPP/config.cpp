#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "scada.h"

#define GLG_DEBUG     0

/* Read and parse configuration file, storing it in a MenuRecord array of
   menu structures. 
*/
MenuRecord * ReadMenuConfig( SCONST char * exe_path, 
                             SCONST char * config_filename, 
                             GlgLong * num_read_records )
{
   FILE * menu_config_file;
   char 
     label_string[ 1002 ],
     drawing_name[ 1002 ],
     tooltip_string[ 1002 ],
     drawing_title[ 1002 ],
     row[ 5002 ],
     * full_path;
   GlgLong
     num_read,
     num_items;

   MenuRecord * menu_array = NULL;
   GlgLong menu_array_size = 0;

   if( !config_filename || !*config_filename )
   {
      *num_read_records = 0;
      return NULL;
   }

   full_path = GlgGetRelativePath( exe_path, config_filename );

   menu_config_file = fopen( full_path, "r" );
   GlgFree( full_path );

   if( !menu_config_file )
   {
      GlgError( GLG_USER_ERROR, (char*) "Can't open configuration file" );

      *num_read_records = 0;
      return NULL;    /* Proceed with the menu table defined in the code. */
   }
   
   num_items = 0;
   while( 1 )
   {
      if( !fgets( row, 5001, menu_config_file ) )
      {
         if( !feof( menu_config_file ) )    /* Read error if not EOF. */
	   GlgError( GLG_USER_ERROR, (char*) "Error reading configuration file." );
         break;
      }

#if GLG_DEBUG
      printf( "row = %s", row );
#endif

      /* Skip empty lines and comment lines starting with '#' */
      if( *row == '#' || EmptyLine( row ) )
	continue;

      /* Read a menu record. Comma is used as a field separator to allow
         spaces in strings without extensive parsing.
      */
      num_read = 
	sscanf( row, "%1001[^,],%1001[^,],%1001[^,],%1001[^,]", 
                label_string, drawing_name, tooltip_string, drawing_title );

      if( num_read != 4 )
      {
	 GlgError( GLG_USER_ERROR, (char*)
		   "Syntax error reading configuration file" );
         break;   /* Use just the lines read in so far. */
      }

#if GLG_DEBUG
      printf( "label=%s drawing=%s tooltip=%s title=%s\n",
              label_string, drawing_name, tooltip_string, drawing_title );
#endif

      /* Processes \n and \\ escape sequences to allow multi-line strings 
         in the config file.
      */
      ProcessEscapeSequences( label_string );
	 
      /* Allocate menu array and adjust its size as needed */
      AdjustMenuArraySize( &menu_array, &menu_array_size, num_items + 1 );
  
      menu_array[ num_items ].label_string = CloneTrimmedString( label_string );
      menu_array[ num_items ].drawing_name = CloneTrimmedString( drawing_name );
      menu_array[ num_items ].tooltip_string = CloneTrimmedString( tooltip_string );
      menu_array[ num_items ].drawing_title = CloneTrimmedString( drawing_title );

      ++num_items;
   }

   /* Store total number of menu items in a global variable NumMenuItems. */
   *num_read_records = num_items;
   return menu_array;
}

/*--------------------------------------------------------------------
| Adjusts MenuArray size in fixed increments to accomodate a newly 
| read item. Reallocates the menu array and copies the old contents,
| initializes new entries with 0s.
*/
static void AdjustMenuArraySize( MenuRecord ** menu_array, 
                                 GlgLong * menu_array_size, 
                                 GlgLong requested_size )
{
   MenuRecord 
     * menu_array_old,
     * menu_array_new;
   GlgLong old_size, new_size;

#define SIZE_INCREMENT   5

   old_size = *menu_array_size;

   if( old_size < requested_size )
   {
      menu_array_old = *menu_array;

      /* It's always incremented by 1 here, bu use the while loop just in case
         it's  changed and we need to add more then 1 increment.
      */
      new_size = old_size;
      while( new_size < requested_size )
        new_size = old_size + SIZE_INCREMENT;
      
      /* Allocate new array */
      menu_array_new = (MenuRecord *) GlgAlloc( sizeof( MenuRecord ) * new_size );
        
      if( menu_array_old )     
      {
         /* Copy the old content if there was any. */
         memmove( menu_array_new, menu_array_old, old_size );
         GlgFree( menu_array_old );
      }

      /* Initialize newly allocated structures with 0s to be safe. */
      memset( ( (char*) menu_array_new ) + old_size * sizeof( MenuRecord ), 
              0, ( new_size - old_size ) * sizeof( MenuRecord ) );

      *menu_array = menu_array_new;
      *menu_array_size = new_size;
   }
}

/*--------------------------------------------------------------------
| Removes all leading and trailing spaces from the string and 
| returns a copy of the trimmed string.
*/
static SCONST char * CloneTrimmedString( SCONST char * string )
{
   char * start_ptr;
   char * end_ptr;

   if( !string )
     return NULL;

   start_ptr = (char *) string;
   
   /* Find the first non-space character. */
   while( *start_ptr && isspace( *start_ptr ) )
     ++start_ptr;

   /* Find the last non-space character if the string has any. */
   if( *start_ptr )
   {
      end_ptr = start_ptr + strlen( start_ptr ) - 1;
      while( isspace( *end_ptr ) )
        --end_ptr;

      *( end_ptr + 1 ) = '\0';    /* Null-terminate */
   }

   return GlgStrClone( start_ptr );
}

/*--------------------------------------------------------------------
| Processes \n and \\ escape sequences to allow multi-line strings 
| in the config file. 
| Processing is done in-place, without allocating a new string.
*/
static void ProcessEscapeSequences( char * label_string )
{   
   int i, length;
   GlgBoolean escape_sequence;
   char
     * from_ptr,
     * to_ptr;

   if( !label_string || !strchr( label_string, '\\' ) )
     return;    /* No escape sequences to process */

   length = (int) strlen( label_string );

   from_ptr = label_string;
   to_ptr = label_string;
   escape_sequence = False;

   for( i=0; i<length; ++i )
   {
      if( !escape_sequence )
      {
         if( *from_ptr != '\\' )
         {
            *to_ptr = *from_ptr;     /* Copy a regular char */
            ++from_ptr;
            ++to_ptr;
         }
         else
         {
            escape_sequence = True;   /* Start of a 2-char escape sequence */
            ++from_ptr;
            continue;
         }
      }
      else   /* The second char of an escape sequence */
      {
         /* No need to handle '"' and ''' cases. */
         switch( *from_ptr )
         {
          case 'n': *to_ptr = '\n'; break;
          case '\\': *to_ptr = '\\'; break;    /* Translate "\\" to "\" */ 
          default:
            /* Not-supported escape sequence - copy literally. */
            *to_ptr = '\\';
            ++to_ptr;
            *to_ptr = *from_ptr;
            break;
         }
         ++from_ptr;
         ++to_ptr;
         escape_sequence = False;    /* End of escape sequence */
      }
   }

   *to_ptr = '\0';
}

/*--------------------------------------------------------------------
| Checks if line contains only white space characters.
*/
static GlgBoolean EmptyLine( SCONST char * line )
{
   int i, length;

   if( !line )
     return True;

   length = (int )strlen( line );

   for( i=0; i<length; ++i )
     if( !isspace( line[i] ) )
       return False;

   return True;
}
