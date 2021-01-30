/*---------------------------------------------------------------------
| This code shows an example of a custom data browsing library. It 
| concentrates on showing how to use the data browsing API, and does 
| not show how to connect to a particular process database: this is
| the responsibility of an application which is supposed to know 
| how to do that.
|
| In this example, the data tags are organized in three levels of hierarchy:
| there are several controllers, each controller has several tag groups, and
| each group has several tags. The application's implementation of the data
| browsing library can implement more or less hierarchy levels, including
| a simplest implementation with a flat list of tags and no hierarchy levels.
*/ 

#include "glg_custom_dll.h"

/* Contains prototypes for the required entry points of the custom
   data browser module.
   */
#include "glg_custom_data.h"

typedef struct ARRAY_ENTRY_DEF ARRAY_ENTRY;

struct ARRAY_ENTRY_DEF
{
   char * name;
   ARRAY_ENTRY * array;
};

ARRAY_ENTRY controller_array[];
ARRAY_ENTRY c1_tag_group_array[];
ARRAY_ENTRY c2_tag_group_array[];
ARRAY_ENTRY c3_tag_group_array[];
ARRAY_ENTRY g11_tag_array[];
ARRAY_ENTRY g12_tag_array[];
ARRAY_ENTRY g13_tag_array[];
ARRAY_ENTRY g21_tag_array[];
ARRAY_ENTRY g22_tag_array[];
ARRAY_ENTRY g23_tag_array[];
ARRAY_ENTRY g31_tag_array[];
ARRAY_ENTRY g32_tag_array[];
ARRAY_ENTRY g33_tag_array[];

/* Utility function prototypes, for this example only */
GlgLong GetControllerIndex( char * string );
GlgLong GetTagGroupIndex( GlgLong controller, char * string );
GlgLong GetTagIndex( GlgLong controller, GlgLong group, char * string );

/*---------------------------------------------------------------------
| Required entry point:
|
| Returns the custom DLL's version number. The version parameter 
| provides the latest DLL version supported by the editor. The return 
| value should not be changed unless the DLL is aware of the version
| differences.
*/
glg_export GlgLong GlgCustomDataVersionNumber( GlgLong version )
{
   /* Current version number defined in include files. */
   return GLG_CUSTOM_DATA_VERSION_NUMBER;
}

/*---------------------------------------------------------------------
| Required entry point:
|
| Performs data connection initialization on startup if any required.
| The load_path parameter supplies the load path of the shared library or DLL
| if provided by the -data-lib command-line option, or NULL otherwise.
| If not NULL, it always contains a trailing slash or back slash.
|
| Return a non-zero code on failure.
*/
glg_export GlgLong GlgCustomDataInit( char * load_path )
{
   return 0;    /* Return 0 to indicate success. */
}

/*---------------------------------------------------------------------
| Required entry point:
|
| Returns an array of data tag strings to be displayed for the path 
| defined by the path array. The path array contains parsed-out elements 
| of the selected path. For example, for the "/controller2/group1/tag3"
| path, the path array will contain "controller2", "group1" and "tag3"
| elements.
|
| This example does not use the filter parameter which defines a 
| regular expression filter with "*" and "?" wildcards. If the application
| may return very long list of tags, it may use the filter parameter to
| shorten the list. Otherwise, GLG will apply the filter to the returned
| list.
*/
glg_export GlgObject GlgGetCustomDataTagArray( GlgObject path_array, 
                                              char * filter )
{
   GlgObject return_array;
   ARRAY_ENTRY * array;
   char 
     * controller_str,
     * tag_group_str,
     * tag_str;
   GlgLong
     i,
     size,
     controller,  /* Controller index in the controller_array */
     tag_group,   /* Tag group index in the selected controller's tag_group 
                     array. */
     tag;         /* Tag index in the selected tag group's tag_array. */

   /* This example shows a 3-level hierarchy: controller/tag_group/tag.
      The size of the path array is the number of hierarchical selections:
      - When size=0 (no selection), the list of available controllers is 
        returned.
      - When size=1 (a controller is selected), the list of tag groups for
        the selected controller is returned.
      - When size=2 (a tag group in the selected controller is selected),
        the list of available tags in the selected tag group is returned.
      - When size=3, a tag is selected - NULL is returned to show no 
        selectable items inside the tag.

      To implement a simple flat list of tags with no hierarchy, 
      return a list of all tags for size=0 (no tag selection), 
      and return NULL for size > 0 (a tag is selected).
      */

   if( !path_array || !( size = GlgGetSize( path_array ) ) ) /* size==0 */
   {
      /* Top level ("/"): use controller_array to list all controllers.
         In a real application, replace this with a process database query 
         to get a list of all controllers.
         */
      array = controller_array;
      size = 0;   /* Initialize in case path_array was NULL. */
   }
   else   /* Controller has been selected: size >= 1 */
   {
      controller_str = (char*) GlgGetElement( path_array, 0 );
      controller = GetControllerIndex( controller_str );

      if( size == 1 || !controller )
      {
         /* If size=1, only the controller is selected - display its groups.
            If controller index is 0, it's an unknown controller - it will 
            display no selectable elements inside it.
            */
         if( !controller )
           GlgError( GLG_USER_ERROR, "Invalid controller." );

         /* Use selected controller's tag_group_array to list all its tag 
            groups. In a real application, replace this with a process 
            database query to get a list of all tag groups of the selected 
            controller.
            */
         array = controller_array[ controller ].array;
      }
      else /* Controller and tag group have been selected: size >= 2 */
      {
         tag_group_str = (char*) GlgGetElement( path_array, 1 );
         tag_group = GetTagGroupIndex( controller, tag_group_str );
         
         if( size == 2 || !tag_group )
         {
            /* If size=2, a controller and a tag group inside it is selected -
               display the tags list of the selected tag group.
               If tag_group index is 0, it's an unknown tag group - it will 
               display no selectable elements inside it.
               */
            if( !tag_group )
              GlgError( GLG_USER_ERROR, "Invalid tag group." );

            /* Use selected tag_group's tag_array to list all its tags.
               In a real application, replace this with a process 
               database query to get a list of all tags of the selected 
               tag group.
            */
            array = controller_array[ controller ].array[ tag_group ].array;
         }
         else /* Controller, tag group and tag have been selected: size ==3 */
         {
            /* Since the user might just type the tag name instead of 
               selecting from a list of choices, check if the tag name is 
               valid.
               */
            tag_str = (char*) GlgGetElement( path_array, 2 );
            tag = GetTagIndex( controller, tag_group, tag_str );

            /* If tag index is 0, it's an unknown tag - give a warning. */
            if( !tag )
              GlgError( GLG_USER_ERROR, "Invalid tag." );         

            /* Tag is selected: no tags inside it, show an empty list. */
            array = NULL;
         }
      }
   }
   
   if( !array )
     /* No selectable items inside the currently selected elements. */
     return (GlgObject)0;

   /* Create an array to hold the elements of the list. */
   return_array = GlgCreateObject( GLG_LIST, NULL, (GlgAnyType)GLG_STRING, 
                                  NULL, NULL, NULL );
   
   /* Add elements of the array to the returned list.
      Start with 1: element 0 is used for an unknown controller, group or tag.
    */
   for( i=1; array[ i ].name; ++i )
   {
      /* If size < 2, the entries are controllers or tag groups that may be 
         opened to see the list of elements inside it, add the " >>" suffix 
         to indicate that the user can open it up via the double-click or 
         by using the Select button.
         If the size=2, the tag group is selected and the entries are tags -
         don't add " >>".
         */
      if( size < 2 )
      {
         char * string;

         /* Add the " >>" suffix to the entry's name to indicate the entry 
            may be opened (a controller or tag group). The data browser 
            will recognize the " >>" suffix and will remove it when reporting 
            the path hierarchy using the path_array parameter.
            */
         string = GlgConcatStrings( array[ i ].name, " >>" );
         GlgAddObjectToBottom( return_array, (GlgObject) string );
         GlgFree( string );
      }
      else   /* The entries are tags - don't add " >>" to their names. */
        GlgAddObjectToBottom( return_array, (GlgObject) array[ i ].name );
   }

   return return_array;
}

/*---------------------------------------------------------------------
| Required entry point:
|
| If the path array (see comments for the previous function) defines
| a valid tag, returns True and sets the return_tag parameter to
| the string defining the selected data tag using preferred syntax.
*/
glg_export GlgBoolean GlgGetCustomDataTagString( GlgObject path_array,
                                                char ** return_tag )
{
   char buffer[ 800 ];
   char
     * controller_str,
     * tag_group_str,
     * tag_str;
   GlgLong
     size,
     controller,
     tag_group,
     tag;

   if( return_tag )
     *return_tag = NULL;

   if( !path_array || ( size = GlgGetSize( path_array ) ) != 3 )
   {
      /* Not a tag, size != 3. For a valid tag, the controller, tag 
         group and tag must be defined, resulting in size ==3 */
      return False;
   }
   else
   {
      controller_str = (char*) GlgGetElement( path_array, 0 );
      controller = GetControllerIndex( controller_str );

      if( !controller )
      {
         GlgError( GLG_USER_ERROR, "Invalid controller." );
         return False;   /* Unknown controller */
      }

      tag_group_str = (char*) GlgGetElement( path_array, 1 );
      tag_group = GetTagGroupIndex( controller, tag_group_str );

      if( !tag_group )
      {
         GlgError( GLG_USER_ERROR, "Invalid tag group." );
         return False;   /* Unknown tag group */
      }

      tag_str = (char*) GlgGetElement( path_array, 2 );
      tag = GetTagIndex( controller, tag_group, tag_str );
      if( !tag )
      {
         GlgError( GLG_USER_ERROR, "Invalid tag." );         
         return False;   /* Unknown tag */
      }

      if( return_tag )
      {
         /* Form the tag string according to a desired syntax */
         sprintf( buffer, "(%.256s)%.256s#%.256s",
                 controller_str, tag_group_str, tag_str );

         /* Must be cloned: will be freed by the invoked function. */
         *return_tag = GlgStrClone( buffer );
      }
      return True;
   }
}

/*---------------------------------------------------------------------
| Utility function for the sample implementation only.
*/
GlgLong GetControllerIndex( char * string )
{
   GlgLong i;

   /* Element 0 is used for unknown controller */
   for( i=1; controller_array[ i ].name; ++i )
     if( strcmp( controller_array[ i ].name, string ) == 0 )
       return i;

   return 0;
}

/*---------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Searches controller's list of groups for the tag group and returns
| its index. 0 is returned if not found (element 0 is reserved for 
| "unknown" entry to minimize special cases).
*/
GlgLong GetTagGroupIndex( GlgLong controller, char * string )
{
   GlgLong i;
   ARRAY_ENTRY * array = controller_array[ controller ].array;

   /* Element 0 is used for unknown controller */
   for( i=1; array[ i ].name; ++i )
     if( strcmp( array[ i ].name, string ) == 0 )
       return i;

   return 0;
}

/*---------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Searches tag group's list of tags for the tag and returns
| its index. 0 is returned if not found (element 0 is reserved for 
| "unknown" entry to minimize special cases).
*/
GlgLong GetTagIndex( GlgLong controller, GlgLong group, char * string )
{
   GlgLong i;
   ARRAY_ENTRY * array = controller_array[ controller ].array[ group ].array;

   /* Element 0 is used for unknown controller */
   for( i=1; array[ i ].name; ++i )
     if( strcmp( array[ i ].name, string ) == 0 )
       return i;

   return 0;
}

/*---------------------------------------------------------------------*/
/* Tag arrays, for the sample implementation only. 
   In an application they will be replaced with tag lists returned from 
   the process database query.
*/

/**** Controllers ****/
ARRAY_ENTRY controller_array[] =
{
   { "UNKNOWN CONTROLLER", NULL },
   { "controller1", c1_tag_group_array },
   { "controller2", c2_tag_group_array },
   { "controller3", c3_tag_group_array },
   { NULL, NULL }
};

/**** Tag groups for each controller ****/
ARRAY_ENTRY c1_tag_group_array[] =
{
   { "UNKNOWN TAG GROUP", NULL },
   { "group11", g11_tag_array },
   { "group12", g12_tag_array },
   { "group13", g13_tag_array },
   { NULL, NULL }
};

ARRAY_ENTRY c2_tag_group_array[] =
{
   { "UNKNOWN TAG_GROUP", NULL },
   { "group21", g21_tag_array },
   { "group22", g22_tag_array },
   { "group23", g23_tag_array },
   { NULL, NULL }
};

ARRAY_ENTRY c3_tag_group_array[] =
{
   { "UNKNOWN TAG GROUP", NULL },
   { "group31", g31_tag_array },
   { "group32", g32_tag_array },
   { "group33", g33_tag_array },
   { NULL, NULL }
};

/**** Tags for each group ****/
ARRAY_ENTRY g11_tag_array[] =
{
   { "UNKNOWN TAG", NULL },
   { "tag111", NULL },
   { "tag112", NULL },
   { "tag113", NULL },
   { NULL, NULL }
};

ARRAY_ENTRY g12_tag_array[] =
{
   { "UNKNOWN TAG", NULL },
   { "tag121", NULL },
   { "tag122", NULL },
   { "tag123", NULL },
   { NULL, NULL }
};

ARRAY_ENTRY g13_tag_array[] =
{
   { "UNKNOWN TAG", NULL },
   { "tag131", NULL },
   { "tag132", NULL },
   { "tag133", NULL },
   { NULL, NULL }
};

ARRAY_ENTRY g21_tag_array[] =
{
   { "UNKNOWN TAG", NULL },
   { "tag211", NULL },
   { "tag212", NULL },
   { "tag213", NULL },
   { NULL, NULL }
};

ARRAY_ENTRY g22_tag_array[] =
{
   { "UNKNOWN TAG", NULL },
   { "tag221", NULL },
   { "tag222", NULL },
   { "tag223", NULL },
   { NULL, NULL }
};

ARRAY_ENTRY g23_tag_array[] =
{
   { "UNKNOWN TAG", NULL },
   { "tag231", NULL },
   { "tag232", NULL },
   { "tag233", NULL },
   { NULL, NULL }
};

ARRAY_ENTRY g31_tag_array[] =
{
   { "UNKNOWN TAG", NULL },
   { "tag311", NULL },
   { "tag312", NULL },
   { "tag313", NULL },
   { NULL, NULL }
};

ARRAY_ENTRY g32_tag_array[] =
{
   { "UNKNOWN TAG", NULL },
   { "tag321", NULL },
   { "tag322", NULL },
   { "tag323", NULL },
   { NULL, NULL }
};

ARRAY_ENTRY g33_tag_array[] =
{
   { "UNKNOWN TAG", NULL },
   { "tag331", NULL },
   { "tag332", NULL },
   { "tag333", NULL },
   { NULL, NULL }
};
