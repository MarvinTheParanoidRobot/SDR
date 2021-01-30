#ifndef _scada_h_
#define _scada_h_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "GlgApi.h"
#include "DataTypes.h"
#include "PageType.h"

#ifdef _WINDOWS
#  include "resource.h"
#  pragma warning( disable : 4244 )
#  pragma warning( disable : 4996 )    /* Allow cross-platform localtime() */
#else
#  include <X11/keysym.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

#define StartsWith( string1, string2 ) \
   ( strncmp( string1, string2, strlen( string2 ) ) == 0 )

#define NO_SCREEN  -1

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
   QUIT,
   ZOOM_IN,
   ZOOM_OUT,
   ZOOM_TO,
   PAN_LEFT,
   PAN_RIGHT,
   PAN_DOWN,
   PAN_UP,
   ZOOM_RESET
} CommandType;

typedef enum _DialogType
{
   UNDEFINED_DIALOG_TYPE = -1,
   GLOBAL_POPUP_DIALOG,
   CUSTOM_DIALOG,
   MAX_DIALOG_TYPE
} DialogType;

#define CLOSE_ALL   UNDEFINED_DIALOG_TYPE

typedef enum _PopupMenuType
{
   UNDEFINED_POPUP_MENU_TYPE = -1,
   GLOBAL_POPUP_MENU
} PopupMenuType;

/* Records of MenuArray, describe menu items. */
typedef struct _MenuRecord
{
   char * label_string;
   char * drawing_name;
   char * tooltip_string;
   char * drawing_title;
} MenuRecord;

/* Used to store information for each active popup dialog. */
typedef struct _ActiveDialogRecord
{
   DialogType dialog_type;
   GlgObject dialog;       /* dialog object ID */
   GlgObject subwindow; /* Subwindow object inside a dialog. */
   GlgObject popup_vp;   /* Viewport loaded into drawing_area. */
   GlgBoolean isVisible;
} ActiveDialogRecord;

/* Used to store information for the active popup. */
typedef struct _ActivePopupMenuRecord
{
   PopupMenuType menu_type;
   GlgObject menu_obj;       /* menu object ID */
   GlgObject subwindow;      /* Subwindow object inside menu_obj. */
   GlgObject menu_vp;        /* Viewport loaded into subwindow. */
   GlgObject selected_obj;   /* Symbol that trigerred popup menu. */
   GlgBoolean isVisible;
} ActivePopupMenuRecord;

/* Records for a table of command types, dialog types, and
   popup menu types.
*/
typedef struct _TypeRecord
{
   char * type_str;
   GlgLong type_enum;
} TypeRecord;

typedef enum
{
   BUTTON_PRESS_EVENT = 1,
   MOUSE_MOVE_EVENT,
   RESIZE_EVENT
} EventType;

/* Tables. */
extern TypeRecord DialogTypeTable[];
extern TypeRecord PopupMenuTypeTable[];
extern TypeRecord CommandTypeTable[];

void FillMenuArray( char * config_filename, char * exe_path );
MenuRecord * ReadMenuConfig( char * exe_path, char * config_filename, 
                             GlgLong * num_read_records );
static void AdjustMenuArraySize( MenuRecord ** menu_array, 
                                 GlgLong * menu_array_size, 
                                 GlgLong requested_size );
static char * CloneTrimmedString( char * string );
static void ProcessEscapeSequences( char * label_string );
static GlgBoolean EmptyLine( char * line );
GlgLong ConvertStringToType( TypeRecord * table, char * type_str, 
			     GlgLong empty_type, GlgLong undefined_type );
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, 
                            GlgLong interval );

#endif
