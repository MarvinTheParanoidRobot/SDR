#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>

/* Set GLG_C_CONST_CHAR_PTR to 1 to use constant strings (const char *)
   and pass them to GLG C functions. If not set, a (char*) cast is required
   when passing strings to GLG C functions, such as GlgStrClone, GlgFree, etc.
   Needs to be defined before GlgClass.h.
 */
#define GLG_C_CONST_CHAR_PTR  1
#include "GlgClass.h"

class GlgTagRecord;
class AlarmRecord;
class PlotDataPoint;

#ifdef _WINDOWS
#include "resource.h"
# pragma warning( disable : 4244 )
# pragma warning( disable : 4996 )    /* Allow cross-platform localtime() */
#endif

#define StartsWith( string1, string2 ) \
   ( strncmp( string1, string2, strlen( string2 ) ) == 0 )

#define NO_SCREEN   -1    // Invalid menu screen index constant.

// An array of pointers to alarm records.
typedef std::vector<AlarmRecord*>  AlarmRecordArrayType;

// An array of pointers to plot data points.
typedef std::vector<PlotDataPoint*> PlotDataArrayType;

// An array of pointers to tag records.
typedef std::vector<GlgTagRecord*> TagRecordArrayType;

typedef enum _CommandType
{
   UNDEFINED_COMMAND_TYPE = -1,
   SHOW_ALARMS,
   GOTO,
   POPUP_DIALOG,
   POPUP_MENU,
   CLOSE_POPUP_DIALOG,
   CLOSE_POPUP_MENU,
   WRITE_VALUE,
   WRITE_VALUE_FROM_WIDGET,
   QUIT
} CommandType;

typedef enum _DialogType
{
   UNDEFINED_DIALOG_TYPE = -1,
   GLOBAL_POPUP_DIALOG,
   ALARM_DIALOG,
   CUSTOM_DIALOG,
   MAX_DIALOG_TYPE
} DialogType;

#define CLOSE_ALL   UNDEFINED_DIALOG_TYPE

typedef enum _PopupMenuType
{
   UNDEFINED_POPUP_MENU_TYPE = -1,
   GLOBAL_POPUP_MENU
} PopupMenuType;


// Records of MenuArray, describe menu items.
typedef struct _MenuRecord
{
   SCONST char * label_string;
   SCONST char * drawing_name;
   SCONST char * tooltip_string;
   SCONST char * drawing_title;
} MenuRecord;

/* Records for a table of command types, dialog types, and
   popup menu types.
*/
typedef struct _TypeRecord
{
   SCONST char * type_str;
   GlgLong type_enum;
} TypeRecord;

typedef enum
{
   BUTTON_PRESS_EVENT = 0,
   RESIZE_EVENT,
   MOUSE_MOVE_EVENT
} EventType;

/* Tables. */
extern TypeRecord DialogTypeTable[];
extern TypeRecord PopupMenuTypeTable[];
extern TypeRecord CommandTypeTable[];

MenuRecord * ReadMenuConfig( SCONST char * exe_path, 
                             SCONST char * config_filename, 
                             GlgLong * num_read_records );
static void AdjustMenuArraySize( MenuRecord ** menu_array, 
                                 GlgLong * menu_array_size, 
                                 GlgLong requested_size );
static SCONST char * CloneTrimmedString( SCONST char * string );
static void ProcessEscapeSequences( char * label_string );
static GlgBoolean EmptyLine( SCONST char * line );
GlgLong ConvertStringToType( TypeRecord * table, SCONST char * type_str, 
			     GlgLong empty_type, GlgLong undefined_type );
double GetCurrTime( void );
GlgBoolean IsUndefined( SCONST char * str );
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, GlgLong interval );





