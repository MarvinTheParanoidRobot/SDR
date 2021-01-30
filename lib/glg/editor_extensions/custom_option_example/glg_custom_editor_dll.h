#ifndef _glg_custom_editor_dll_h
#define _glg_custom_editor_dll_h

#ifndef _Glg_Api_h
#include "GlgApi.h"
#endif

#ifdef _WINDOWS
#define Widget HWND
#define TAIL        NULL, NULL, (Widget)0, (GlgObject)0, 0, 0,      (GlgLong*)0
#define TAIL_PART   NULL, NULL, (Widget)0, (GlgObject)0, 0, 0
#else
#define TAIL        NULL, NULL, (Widget)0, (GlgObject)0, (Widget)0, (GlgLong*)0
#define TAIL_PART   NULL, NULL, (Widget)0, (GlgObject)0, (Widget)0
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define GLG_CUSTOM_START   10000

typedef enum _GlgEditorUpdateFlags
{
   GD_UPDATE_ATTR_VALUES = 1,
   GD_UPDATE_POINTS      = 2,  /* Needed only if xfrom changes */
   GD_UPDATE_HIERARCHY   = 4   /* After adding custom propertiers, xforms, 
                                  rendering attributes, etc. */
} GlgEditorUpdateFlags;

typedef enum _GlgEditorIconType
{
   GD_OBJECT_ICON = 1,     /* Object palette icon */
   GD_TOOLBAR_ICON          /* Toolbar icon */

} GlgEditorIconType;

typedef enum _GlgEditorMenuType
{
   GD_MAIN_MENU = 1,     /* Main editor menu */
   GD_POPUP_MENU,        /* Editor's popup menu */
   GD_RUN_MENU           /* Run mode pulldown menu */

} GlgEditorMenuType;

typedef enum _GlgMenuEntryType
{
   GD_DEFAULT_ET = 0,        /* Default: push button or cascade sub-menu if 
                                 sub_items field is defined. */
   GD_PUSH_ET,               /* For internal use. */
   GD_UNSET_TOGGLE_ET,       /* Non-radio toggle, initially unset. */
   GD_SET_TOGGLE_ET,         /* Non-radio toggle, initially set. */
   GD_UNSET_RADIO_TOGGLE_ET, /* Radio toggle, initially unset. */
   GD_SET_RADIO_TOGGLE_ET,   /* Radio toggle, initially set. */
   GD_SEPARATOR_ET,          /* Separator */
   GD_CASCADE_ET,            /* Cascade sub-menu containing sub-items. */
   GD_RADIO_CASCADE_ET       /* Radio-style cascade sub-menu. */
} GlgMenuEntryType;

/* Abbreviations */
#define DEF  GD_DEFAULT_ET
#define UTG  GD_UNSET_TOGGLE_ET
#define STG  GD_SET_TOGGLE_ET
#define URT  GD_UNSET_RADIO_TOGGLE_ET
#define SRT  GD_SET_RADIO_TOGGLE_ET
#define SEP  GD_SEPARATOR_ET
#define RAD  GD_RADIO_CASCADE_ET

#undef GlgCustomMenuItem

typedef struct _GlgCustomMenuItem
{
   char * label;
   GlgLong token;
   struct _GlgCustomMenuItem * sub_items;
   GlgLong level;
   GlgMenuEntryType widget_class;
   char mnemonic;
   char * accelerator;
   char * accelerator_text;
   Widget widget;
   GlgObject viewport;
#ifndef _WINDOWS
   Widget top_widget;
#else
   GlgLong menu;
   GlgLong disabled;
#endif
   GlgLong * enable_disable_ptr;    /* Used to disable entry if *ptr == 1 */

} GlgCustomMenuItem;

typedef enum _GlgHandlerType
{
   GLG_BROWSE_DATA_HANDLER,
   GLG_BROWSE_TAGS_HANDLER,
   GLG_BROWSE_RESOURCES_HANDLER,
   GLG_BROWSE_ALARMS_HANDLER
} GlgHandlerType;

typedef enum _GlgHandlerReturnCode
{
   GLG_HANDLER_ABORTED = 0,
   GLG_HANDLER_RETURN_VALUE
} GlgHandlerReturnCode;

typedef struct _GlgInstallHandlerData GlgInstallHandlerData;

struct _GlgInstallHandlerData
{
   GlgHandlerType type;          /* Type of the handler to install, */
   GlgLong version;              /* Resuest version number for future 
                                    compatibility. */
   GlgTagType tag_type;          /* Tag type for the tag browser */
   char * start_path;            /* Start path for the resource browser:
                                    ".", "~" or "/". If NULL, selected object
                                    (".") is used if selected, or the whole
                                    drawing otherwise. */

   GlgLong (*callback)( GlgInstallHandlerData * request_data );
   GlgAnyType user_data;         /* User-supplied data */

   GlgHandlerReturnCode return_code;
   char * return_string;
   GlgObject return_object;
   
};

#define GMrqType( data )          data->type
#define GMrqVersion( data )       data->version
#define GMrqTagType( data )       data->tag_type
#define GMrqStartPath( data )     data->start_path
#define GMrqCallback( data )      data->callback
#define GMrqUserData( data )      data->user_data
#define GMrqReturnCode( data )    data->return_code
#define GMrqReturnString( data )  data->return_string
#define GMrqReturnObject( data )  data->return_object

void GMSetMenuSensitivity( GlgCustomMenuItem* table, GlgLong token, GlgLong state );
void GMSetMenuState( GlgCustomMenuItem* table, GlgLong token, GlgLong state );
void GMSetMenuLabel( GlgCustomMenuItem* table, GlgLong token, char * label );
void GMSetIconSensitivity( GlgObject icon, GlgLong state );
void GMUpdateEditorState( GlgLong flags );
GlgBoolean GMInstallHandler( GlgInstallHandlerData * request_data );
void GMAbortAllHandlers( void );

#ifdef __cplusplus
}
#endif

#endif
