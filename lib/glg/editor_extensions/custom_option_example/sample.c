/*---------------------------------------------------------------------
| This code shows an example of a custom options module.
| The module may be used to customize the Graphics Builder or HMI 
| Configurator by adding custom icons and menu options, as well
| as providing custom OEM dialogs to execute custom application 
| logic inside the GLG editor. The module may also be used to 
| remove unwanted icons and menu options of a GLG editor.
*/ 

#include "glg_custom_dll.h"
#include "glg_custom_editor_dll.h"

/* Contains prototypes for the required entry points of the custom
   option module.
   */
#include "glg_custom_option.h"

/* Custom command tokens. */
typedef enum _CustomOP
{
   /* Action OPs */
   ADD_ACTION = GLG_CUSTOM_START,
   EDIT_ACTION,
   DELETE_ACTION,
   WRITE_VALUE,
   WRITE_CURRENT,
   POPUP_ACTION,
   GOTO_ACTION,

   /* Sample menu OPs */
   SAMPLE_MENU,
   SET_INVISIBLE,
   SET_VISIBLE,
   TOGGLE_VISIBILITY,
   VISIBILITY_MENU,
   
   /* Run time sample OP */
   RUN_TIME_ACTION
} CustomOP;

typedef enum _OEMActionType
{
   OEM_UNDEFINED_ACTION,
   OEM_WRITE_VALUE_ACTION = 1,
   OEM_WRITE_CURRENT_ACTION = 2,
   OEM_POPUP_ACTION = 3,
   OEM_GOTO_ACTION = 4
} OEMActionType;

static char * LoadPath = NULL;
static GlgBoolean RunWindowMode = False;
static GlgObject TopViewport = (GlgObject)0;
static GlgObject DialogParent = (GlgObject)0;
static GlgObject WriteActionDialog = (GlgObject)0;
static GlgObject GotoActionDialog = (GlgObject)0;
static GlgObject SelectedObject = (GlgObject)0;
static GlgObject EditActionIcon = (GlgObject)0;
static GlgObject CurrentDialog = (GlgObject)0;
static CustomOP CurrentAction = 0;
static GlgLong ModalDialogActive = False;
static GlgBoolean StopRunning = False;

extern GlgCustomMenuItem VisibilityMenuTable[];
extern GlgCustomMenuItem VisibilityMenuTable2[];
GlgCustomMenuItem AddActionMenuTable[];
GlgCustomMenuItem AddActionMenuTable2[];
GlgCustomMenuItem SampleMenuTable[];
GlgCustomMenuItem SampleMenuTable2[];

/* This table defines a custom OEM menu that will be added to the main
   menu of the GLG editor. 
   The GlgCustomMenuItem typedef and the GlgMenuEntryType enum constants 
   (DEF, SEP, UTG, etc.) are defined in the glg_custom_editor.h include file.
   */
GlgCustomMenuItem PulldownMenuTable[] =
{
   /* label | token | sub_items | level | widget_class | mnemonic | TAIL */

   /* Custom Actions with Dialogs example. */
   "Add Custom Action",         ADD_ACTION, AddActionMenuTable, 
                                                  0xff, DEF, 'A', TAIL,
   "Edit Custom Action",       EDIT_ACTION, NULL, 0xff, DEF, 'E', TAIL,
   "Delete Custom Action",   DELETE_ACTION, NULL, 0xff, DEF, 'D', TAIL,

   /* Separator */
   "-----",                              0, NULL, 0xff, SEP,   0, TAIL,

   /* Provides samples of different menu items: toggle and push buttons, 
      cascading radio menus, etc. */
   "Custom Menu Samples",     SAMPLE_MENU, SampleMenuTable, 
                                                  0xff, DEF, 'C', TAIL,
   /* Separator */
   "-----",                              0, NULL, 0xff, SEP,   0, TAIL_PART,
               &RunWindowMode,    /* Disable the entry in the RunWindow mode */

   /* Run-time action sample: active in the run mode, is disabled in the 
      RunWindow mode which uses a separate pulldown menu.
      */
   "Run-Time Action",      RUN_TIME_ACTION, NULL, 0xff, DEF, 'R', TAIL_PART,
               &RunWindowMode,    /* Disable the entry in the RunWindow mode */
   NULL
};

/* This table defines a custom OEM menu that be added to the popup
   menu of the GLG editor. A separate instance of the table is required,
   even if the table entries as the same as in the PulldownMenuTable above.
   */   
GlgCustomMenuItem PopupMenuTable[] =
{
   /* label | token | sub_items | level | widget_class | mnemonic | TAIL */

   "Add Custom Action",         ADD_ACTION, AddActionMenuTable2, 
                                                  0xff, DEF, 'A', TAIL,
   "Edit Custom Action",       EDIT_ACTION, NULL, 0xff, DEF, 'E', TAIL,
   "Delete Custom Action",   DELETE_ACTION, NULL, 0xff, DEF, 'D', TAIL,
   "-----",                              0, NULL, 0xff, SEP,   0, TAIL,
   "Custom Menu Samples",     SAMPLE_MENU, SampleMenuTable2, 
                                                  0xff, DEF, 'C', TAIL,
   NULL
};

/* This table defines entries of the AddAction Menu cascading
   sub-menu in the pull-down menu. */
GlgCustomMenuItem AddActionMenuTable[] =
{
   /* Write a value defined in the action to a specified tag. */
   "Write Value",     WRITE_VALUE, NULL, 0xff, DEF, 'W', TAIL,

   /* Write the current state (ON or OFF) of a toggle button to a 
      specified tag. */
   "Write State",     WRITE_CURRENT, NULL, 0xff, DEF, 'C', TAIL,

   /* Popup a specified dialog. */
   "Popup",          POPUP_ACTION, NULL, 0xff, DEF, 'P', TAIL,

   /* Go To a new (specified) drawing). */
   "GoTo",            GOTO_ACTION, NULL, 0xff, DEF, 'G', TAIL,
   NULL
};

/* This table defines entries of the AddAction Menu cascading
   sub-menu in the popup-down menu. */
GlgCustomMenuItem AddActionMenuTable2[] =
{
   "Write Value",    WRITE_VALUE, NULL, 0xff, DEF, 'W', TAIL,
   "Write State",  WRITE_CURRENT, NULL, 0xff, DEF, 'C', TAIL,
   "Popup",         POPUP_ACTION, NULL, 0xff, DEF, 'P', TAIL,
   "GoTo",           GOTO_ACTION, NULL, 0xff, DEF, 'G', TAIL,
   NULL
};

/* This table defines entries of the Custom Menu Samples cascading
   sub-menu in the pull-down menu. */
GlgCustomMenuItem SampleMenuTable[] =
{
   /* Command button examples: execute custom commands. */
   "Set Invisible",          SET_INVISIBLE, NULL, 0xff, DEF, 'I', TAIL,
   "Set Visible",              SET_VISIBLE, NULL, 0xff, DEF, 'V', TAIL,

   /* Separator */
   "-----",                              0, NULL, 0xff, SEP,   0, TAIL,

   /* Toggle example: toggles visibility of the selected object. */
   "Visibility Toggle",  TOGGLE_VISIBILITY, NULL, 0xff, UTG, 'T', TAIL,

   /* Cascading menu sample: a sub-menu with a radio behavior. */
   "Visibility Menu",      VISIBILITY_MENU, VisibilityMenuTable, 
                                                  0xff, RAD, 'T', TAIL,
   NULL
};

/* This table defines entries of the Custom Menu Samples cascading
   sub-menu in the in the popup menu. */
GlgCustomMenuItem SampleMenuTable2[] =
{
   /* Command button examples: execute custom commands. */
   "Set Invisible",          SET_INVISIBLE, NULL, 0xff, DEF, 'I', TAIL,
   "Set Visible",              SET_VISIBLE, NULL, 0xff, DEF, 'V', TAIL,

   /* Separator */
   "-----",                              0, NULL, 0xff, SEP,   0, TAIL,

   /* Toggle example: toggles visibility of the selected object. */
   "Visibility Toggle",  TOGGLE_VISIBILITY, NULL, 0xff, UTG, 'T', TAIL,

   /* Cascading menu sample: a sub-menu with a radio behavior. */
   "Visibility Menu",      VISIBILITY_MENU, VisibilityMenuTable2, 
                                                  0xff, RAD, 'T', TAIL,
   NULL
};

/* This table defines entries of the Visibility Menu cascading
   sub-menu in the pull-down menu. */
GlgCustomMenuItem VisibilityMenuTable[] =
{
   /* Radio toggles example: only one is active at a time. */
   "Visible",      SET_VISIBLE, NULL, 0xff, SRT, 'V', TAIL,
   "Invisible",  SET_INVISIBLE, NULL, 0xff, URT, 'I', TAIL,
   NULL
};

/* This table defines entries of the Visibility Menu cascading
   sub-menu in the popup menu. It is the same as VisibilityMenuTable above, 
   but needs a separate table instance. 
   */
GlgCustomMenuItem VisibilityMenuTable2[] =
{
   "Visible",      SET_VISIBLE, NULL, 0xff, SRT, 'V', TAIL,
   "Invisible",  SET_INVISIBLE, NULL, 0xff, URT, 'I', TAIL,
   NULL
};

/* This table defines a custom OEM menu that will be added to the main
   menu of the GLG editor's Run window in the RunWindow mode. This
   menu will be visible in the Run mode only if the Run mode uses a
   separate window that hides GLG editor's menus and toolbars. 
   The separate window mode may be requested by the -run-window
   command-line option or the RunWindow configuration file variable.
*/
GlgCustomMenuItem RunMenuTable[] =
{
   /* Run-time action sample: active in the run mode. */
   "Run-Time Action",      RUN_TIME_ACTION, NULL, 0xff, DEF, 'R', TAIL,
   NULL
};

/* Prototypes for the utility functions */
void CustomDialogEventHandler( GlgObject message );
void EnableDisableCommands( GlgBoolean edit_mode );
GlgBoolean PopupCustomActionDialog( CustomOP action_type, 
                                   GlgObject selected_obj );
GlgBoolean AddAction( CustomOP action_type, GlgObject selected_obj );
GlgBoolean DeleteAction( GlgObject selected_obj );
GlgBoolean DisplayAction( CustomOP action_type, GlgObject selected_obj );
void SetDialogSize( GlgObject dialog, GlgLong width, GlgLong height, 
                   GlgLong x, GlgLong y );
GlgLong GetVisibility( GlgObject object );
void UpdateVisibilityState( GlgObject object );
CustomOP GetActionType( GlgObject selected_obj );
GlgBoolean HasCustomAction( GlgObject selected_obj );
GlgBoolean HasForeignMouseClickEvent( GlgObject selected_obj );
GlgBoolean MouseClickIsForeign( GlgObject mouse_click_prop );
CustomOP GetActionTypeFromOEM( OEMActionType action_type );
OEMActionType GetOEMActionType( CustomOP action_type );
char * GetActionString( CustomOP action_type );
GlgObject GetCustomDialog( CustomOP action_type );
GlgBoolean LoadDialogs( void );
GlgBoolean LoadDialog( CustomOP op );
void CloseCurrentDialog( void );
GlgBoolean ApplyDialogInput( void );
GlgBoolean DialogInputValid( GlgObject source, GlgBoolean check_only, 
                            GlgBoolean generate_error );
GlgBoolean CheckDialogInput( void );
GlgBoolean ApplyDialogInput( void );
GlgBoolean DialogEntryValid( char * origin );
GlgBoolean ValueRangeValid( char * source, char * text_field_name, 
                           char * label_name, char * error_message,
                           GlgBoolean generate_error );
GlgBoolean TagValid( char * source, char * text_field_name, char * label_name,
                    char * error_message, GlgBoolean generate_error );
GlgBoolean PopupValid( char * source, char * text_field_name, 
                      char * label_name, char * error_message,
                      GlgBoolean generate_error );
GlgBoolean TextNotEmpty( char * source, char * text_field_name, 
                        char * label_name, char * error_message,
                        GlgBoolean generate_error );
GlgLong CustomGotTagNameCallback( GlgInstallHandlerData * data );
GlgBoolean DeleteElement( GlgObject group, GlgObject data_obj );

/*---------------------------------------------------------------------
| Required entry point:
|
| Returns the custom DLL's version number. The version parameter 
| provides the latest DLL version supported by the editor. The return 
| value should not be changed unless the DLL is aware of the version
| differences.
*/
glg_export GlgLong GlgCustomOptionVersionNumber( GlgLong version )
{
   /* Current version number defined in include files. */
   return GLG_CUSTOM_OPTION_VERSION_NUMBER;
}

/*---------------------------------------------------------------------
| Required entry point:
|
| Performs initialization on startup if any required.
| Parameters:
|   load_path - load path of the shared library or DLL, if provided by the
|               -option-lib command-line option, or NULL otherwise.
|               If not NULL, it always contains a trailing slash or back slash.
|   top_viewport - top vieport of the Builder, used to call GlgUpdate().
|   dialog_parent - the viewport to add custom dialogs to.
|   hmi_mode - set to True when used with the HMI Configurator; set to 
|              False in the Graphics Builder.
|   run_window_mode - set to True is a separate Run window is used for the 
|                     Run mode.
|   disable_menu - set to True to ask the Builder to disable the pulldown 
|                  menu in the Run mode in case it contains only Edit mode 
|                  actions, otherwise set to False to keep Run mode actions
|                  active. 
|   save_drawing - set to True to ask the Builder to save/restore the drawing 
|                  before and after the Run mode in case if the option DLL
|                  is also used as a proto DLL to implement a custom run-time
|                  application; otherwise set it to False. Refer to the 
|                  proto DLL example for a sample of run-time animation. 
|
| Return a non-zero code on failure.
*/
glg_export GlgLong GlgCustomOptionInit( GlgCustomOptionInitData * init_data )
{
   /* Store as globals */
   LoadPath = GlgStrClone( init_data->load_path );
   TopViewport = GlgReferenceObject( init_data->top_viewport );
   DialogParent = GlgReferenceObject( init_data->dialog_parent );

   /* True if a separate Run window is used. */
   RunWindowMode = init_data->run_window_mode;

   /* The custom pulldown menu of this example contains run-time actions:
      return False not to disable the menu in the Run mode. 
      */
   init_data->disable_menu = False;

   /* Set save_drawing = False to do nothing. Refer to the proto DLL example
      for more details. */
   init_data->save_drawing = False;

   /* Allow the user to use the datagen command for animating the drawing
      in the Run Mode in addition to the custom DLL animation (if any). 
      Set it to False to disable datagen-based animation and animate only 
      by the custom DLL.
      */
   init_data->enable_datagen = True;
   
   if( !LoadDialogs() )
     return 1;    /* Error status */

   return 0;   /* Return 0 to indicate success */
}

/*---------------------------------------------------------------------
| Required entry point:
| Returns a pointer to a table that defines a custom pull-down menu, 
| or NULL if no entries need to be added.
| The menu_name provides the label for the button used to activate
| the custom menu.
*/
glg_export GlgCustomMenuItem * 
  GlgCustomOptionAddMenu( GlgEditorMenuType menu_type, char ** menu_name )
{
   switch( menu_type )
   {
    case GD_MAIN_MENU:    /* Main pulldown menu */
      *menu_name = "OEM Menu";
      return PulldownMenuTable;

    case GD_POPUP_MENU:   /* Right-click popup menu */
      /* Can't use PulldownMenuTable, need a separate instance for the popup 
         menu even if the content is the same.
         */
      *menu_name = "OEM Menu";
      return PopupMenuTable;

    case GD_RUN_MENU:    /* Pulldown menu for the RunWindow mode. */
      *menu_name = "OEM RunTime Sample";
      return RunMenuTable;

    default: return NULL;
   }
}

/*---------------------------------------------------------------------
| Required entry point:
|
| Sets initial state of the custom command entries in the pulldown menu 
| for the Edit mode. There is no need to process the popup table's 
| entries which are enabled or disabled automatically.
| 
| This entry may not be invoked if the Builder is started in the Run mode.
*/
glg_export void GlgCustomOptionSetupMenu()
{
   /* Initialize the state of custom entries to the edit mode. */
   EnableDisableCommands( /*Edit mode*/ True );

   /* Example of changing menu item label. */
   if( RunWindowMode )
     /* In the RunWindow mode, a separate run menu is used for the run-mode 
        commands. */
     GMSetMenuLabel( RunMenuTable, RUN_TIME_ACTION, "Stop" );
   else     
     GMSetMenuLabel( PulldownMenuTable, RUN_TIME_ACTION, "Stop" );
}

/*---------------------------------------------------------------------
| Required entry point:
|
| May be used to disable some of the Builder's menu entries.
| Returns True for the entries that need to be disabled.
| The menu_type parameter is set to GD_MAIN_MENU for the main menu,
| GD_POPUP_MENU for the popup menu and GD_RUN_MENU for the menu of 
| the Run window in the RunWindow mode.
*/
glg_export GlgBoolean 
  GlgCustomOptionDisableMenuEntry( GlgEditorMenuType menu_type, char * label, 
                                  GlgLong reserved )
{
   /* Example: enable the code below to disable "Select Multiple Objects"
      menu options. */
#if 0
   if( label && strcmp( label, "Select Multiple Objects" ) == 0 )
     return True;
#endif

   /* Example: enable the code below to disable "Change Update Speed"
      menu option of the Run menu used in the RunWindow mode. */
#if 0
   if( label && strcmp( label, "Change Update Speed" ) == 0 )
     return True;
#endif

   return False;
}

/*---------------------------------------------------------------------
| Required entry point:
|
| May be used to disable some of the Builder's toolbar and object palette 
| icons.
| Returns True for the icons that need to be disabled.
| The icon_type parameter is set to the icon type: GD_OBJECT_ICON or 
| GD_TOOLBAR_ICON.
*/
glg_export GlgBoolean 
  GlgCustomOptionDisableIcon( GlgEditorIconType icon_type, char * label,
                             GlgLong reserved )
{
   /* Example: enable the code below to disable the icon for creating
      GIS objects. */
#if 0
   if( icon_type == GD_OBJECT_ICON && label && strcmp( label, "GIS" ) == 0 )
     return True;
#endif

   return False;
}

/*---------------------------------------------------------------------
| Required entry point:
|
| Is invoked by the Builder to notify the custom DLL of a custom icon.
| Refer to the README file in this directory for information on providing
| a drawing that contains custom icons.
|
| To accept the icon, the method should return a non-null command token
| to be associated with the icon. The method can return 0 to decline 
| the icon.
| The icon_type parameter is set to the icon type: GD_OBJECT_ICON or 
| GD_TOOLBAR_ICON.
| The label parameter is the icon's label. The icon_drawing parameter is 
| the graphical object that will be used as the icon's graphics; it's 
| often a group that contains graphical objects used to draw the icon.
| The icon_viewport parameter is the object ID of the viewport that will
| contain icon_drawing.
*/
glg_export GlgLong 
  GlgCustomOptionManageIcon( GlgEditorIconType icon_type, char * label, 
                            GlgObject icon_viewport, GlgObject icon_drawing )
{
   if( icon_type == GD_TOOLBAR_ICON && 
      label && strcmp( label, "EditOEMAction" ) == 0 )
   {
      /* Store icon's object ID to set icon's sensitivity when an object
         is selected. */
      GlgDropObject( EditActionIcon );
      EditActionIcon = GlgReferenceObject( icon_viewport );

      return EDIT_ACTION;    /* Associate EDIT_ACTION command with the icon. */
   }

   return 0;    /* Decline to accept any other icons. */
}

/*---------------------------------------------------------------------
| Required entry point:
|
| Disables editor actions if a modal dialog is displayed.
| Returns True to disable actions if a modal dialog is active,
| otherwise returns False to allow action processing.
*/
glg_export GlgBoolean GlgCustomOptionModalDialogCheck( GlgLong op )
{
   /* Make dialogs modal if required. */
   if( ModalDialogActive )
   {
      GlgError( GLG_WARNING, "Custom dialog is active!" );
      return True;   /* Suppress the action */
   }
   return False;   /* Allow an action to proceed */
}

/*---------------------------------------------------------------------
| Required entry point:
| Processes custom commands defined in the custom pulldown and popup menus.
| Returns True is the command was processed.
*/
glg_export GlgBoolean GlgCustomOptionProcessCommand( GlgLong op, 
                                                    GlgObject selected_obj, 
                                                    GlgObject drawing )
{
   CustomOP action_type;
   GlgLong visibility;

   /* Set to true when opening a modal custom dialog */
   switch( op )
   {
    case SET_INVISIBLE:
      if( !selected_obj )
        return True;

      GlgSetDResource( selected_obj, "Visibility", 0. );

      /* Update toggle state in both menus. */
      UpdateVisibilityState( selected_obj );

      GMUpdateEditorState( GD_UPDATE_ATTR_VALUES );
      GlgUpdate( TopViewport );
      return True;

    case SET_VISIBLE:
      if( !selected_obj )
        return True;

      GlgSetDResource( selected_obj, "Visibility", 1. );

      /* Update toggle state in both menus. */
      UpdateVisibilityState( selected_obj );

      GMUpdateEditorState( GD_UPDATE_ATTR_VALUES );
      GlgUpdate( TopViewport );
      return True;

    case TOGGLE_VISIBILITY:
      if( !selected_obj )
        return True;

      /* Toggle visibility */
      visibility = GetVisibility( selected_obj );
      visibility = !visibility;
      GlgSetDResource( selected_obj, "Visibility", (double) visibility );

      /* Update toggle state in both menus. */
      UpdateVisibilityState( selected_obj );

      GMUpdateEditorState( GD_UPDATE_ATTR_VALUES );
      GlgUpdate( TopViewport );
      return True;

    case WRITE_VALUE:
    case WRITE_CURRENT:
    case POPUP_ACTION:
    case GOTO_ACTION:
      if( !selected_obj )
        return True;

      if( HasForeignMouseClickEvent( selected_obj ) )
      {
         GlgError( GLG_USER_ERROR, 
                  "Can't add action: delete foreign MouseClick event first." );
         return True;
      }

      if( !AddAction( op, selected_obj ) )
        return True;

      /* Invoke itself with EDIT_ACTION to open up a dialog. */
      GlgCustomOptionProcessCommand( EDIT_ACTION, selected_obj, drawing );
      return True;

    case EDIT_ACTION:
      if( !selected_obj )
        return True;

      action_type = GetActionType( selected_obj );
      if( !action_type )
        return True;

      if( !PopupCustomActionDialog( action_type, selected_obj ) )
        return True;

      GlgUpdate( TopViewport );
      ModalDialogActive = True;     /* Make dialog model */
      return True;

    case DELETE_ACTION:
      if( !selected_obj )
        return True;

      DeleteAction( selected_obj );
      return True;

    case RUN_TIME_ACTION:
      /* In this example, just stop the Run mode. */
      StopRunning = True;
      return True;

    default: return False;    /* Not a custom command - pass to the Builder. */
   }
}

/*---------------------------------------------------------------------
| Required entry point:
|
| Invoked on object selection change, should set sensitivity of custom 
| command entries depending on the object selection and perform any 
| other object selection related activity. The selected_object parameter
| is NULL when there is no object selected.
|
| To implement custom logic, the program can check for presence some
| application-specific custom properties attached to the object (such 
| as NodeType, etc.) and activate only custom commands that are 
| applicable to the object.
*/
glg_export void GlgCustomOptionSelectionCallback( GlgObject drawing,
                                                 GlgObject selected_obj )
{
   GlgBoolean selected, has_action;

   selected = ( selected_obj != (GlgObject)0 );
   has_action = ( selected && HasCustomAction( selected_obj ) );

   /* Enable or disable actions depending on object selection, etc. */
   GMSetMenuSensitivity( PulldownMenuTable, ADD_ACTION, 
                        selected && !has_action );
   GMSetMenuSensitivity( PulldownMenuTable, EDIT_ACTION, has_action );
   GMSetMenuSensitivity( PulldownMenuTable, DELETE_ACTION, has_action );
   GMSetMenuSensitivity( PulldownMenuTable, SAMPLE_MENU, selected );
   
   /* Ditto for the popup menu. */
   GMSetMenuSensitivity( PopupMenuTable, ADD_ACTION, selected && !has_action );
   GMSetMenuSensitivity( PopupMenuTable, EDIT_ACTION, has_action );
   GMSetMenuSensitivity( PopupMenuTable, DELETE_ACTION, has_action );
   GMSetMenuSensitivity( PopupMenuTable, SAMPLE_MENU, selected );

   /* Enable custom icon for editing object's action only when the object is
      selected and has an action attached. */
   GMSetIconSensitivity( EditActionIcon, selected && has_action );

   /* Update the state of the visibility toggle to reflect the visibility
      of the selected object. */
   UpdateVisibilityState( selected_obj );

   /* Store selected object */
   GlgDropObject( SelectedObject );
   SelectedObject = GlgReferenceObject( selected_obj );
}

/*---------------------------------------------------------------------
| Required entry point:
|
| Invoked after loading a new drawing. The filename parameter contains 
| the path to the file from which the drawing was loaded or NULL for
| a File/New command.
*/
glg_export void GlgCustomOptionLoadDrawingCallback( GlgObject drawing, 
                                                   char * filename )
{
   /* Add code to perform any application-specific actions for a new drawing.
    */
}

/*---------------------------------------------------------------------
| Required entry point:
|
| Invoked before saving a drawing. The filename parameter contains 
| the path to the file into which the drawing will be saved.
| If the method returns False, the save operation will be aborted.
*/
glg_export GlgBoolean GlgCustomOptionSaveDrawingCallback( GlgObject drawing, 
                                                         char * filename )
{
   /* Add code to perform any application-specific checks before saving the 
      drawing. */

   return True; /* Return True to allow the Builder to proceed with saving. */
}

/*---------------------------------------------------------------------
| Required entry point, Run mode:
|
| Speed change callback, allows the DLL to accept or reject the speed
| requested by the user.
| The min_speed and max_speed parameters provide the min and max speed limits
| in relative units (currently 0 and 9). The speed parameter is the speed
| requested by the user. To reject the requested speed, return a different
| valid spped value.
*/
glg_export GlgLong GlgCustomOptionSetSpeed( GlgLong min_speed, 
                                           GlgLong max_speed, GlgLong speed )
{
   return speed;    /* Accept the speed requested by the user. */
}

/*---------------------------------------------------------------------
| Required entry point, Run mode:
|
| Invoked at the start of the Run mode, may be used to disable the 
| Edit mode custom command entries in the pulldown menu and enable the 
| Run mode entries. There is no need to process the popup table's 
| entries since it is automatically disabled in the Run mode.
|
| If the option DLL is also used as a proto DLL to implement a custom
| application at run-time, the drawing and save_init_values parameters
| provide additional information. Refer to the proto DLL example
| for a sample of run-time animation. 
*/
glg_export void GlgCustomOptionStartRun( GlgObject drawing, 
                                        GlgLong save_init_values )
{   
   /* Change state of the custom entries in the PulldownMenu to the Run mode.*/
   EnableDisableCommands( /*Run mode*/ False );

   StopRunning = False;
}

/*---------------------------------------------------------------------
| Required entry point, Run mode:
|
| Invoked at the end of the Run mode, may be used to enable the 
| Edit mode custom command entries in the pulldown menu and disable the 
| Run mode entries. There is no need to process the popup table's 
| entries which are enabled or disabled automatically.
|
| If the option DLL is also used as a proto DLL to implement a custom
| application at run-time, the drawing and restore_init_values parameters
| provide additional information. Refer to the proto DLL example
| for a sample of run-time animation. 
*/
glg_export void GlgCustomOptionStopRun( GlgObject drawing, 
                                       GlgLong restore_init_values )
{   
   /* Change state of the custom entries in the PulldownMenu back to the 
      Edit mode. 
      */
   EnableDisableCommands( /*Edit mode*/ True );
}

/*---------------------------------------------------------------------
| Required entry point, Run mode:
|
| Updates the drawing in case the option DLL is also used as a proto DLL
| to implement a custom application at run-time. Refer to the proto DLL 
| example for a sample of run-time animation. 
|
| The pause parameter is set to True when the Run mode is paused.
| The method should return True to stop the Run mode or False to
| continue.
*/
glg_export GlgBoolean GlgCustomOptionUpdateRun( GlgObject drawing, 
                                               GlgBoolean pause )
{
   /* Do nothing here and return False to continue, unless the StopRunning
      flag was set. 
      Refer to the proto DLL example for a sample of run-time animation. 
      */
   return StopRunning;
}

/*---------------------------------------------------------------------
| Required entry point:
|
| Handles user iteraction with the custom dialogs and run mode events.
| Parameters:
|   drawing - the drawing displayed in the editor.
|   top_viewport - top viewport of a custom dialog for custom dialog events,
|                  or drawing for events in the Run mode.
|   message - message object
|   viewport - the viewport that generated the event.
*/
glg_export void 
  GlgCustomOptionEventHandler( GlgObject drawing, GlgObject top_viewport, 
                              GlgObject message, GlgObject viewport )
{
   /* Process events only for objects from custom dialogs. 
      Ignore other events by checking if object ID of the viewport 
      parameter refers to custom dialogs.

      Alternatively, unique names could be used. For example,
      names of all custom dialogs and all input objects in the dialogs
      could use a unique prefix, which could be checked instead of the
      viewport parameter:
         strncmp( origin, "XXX", strlen( "XXX" ) ) != 0      
      */      
   if( top_viewport && 
      ( top_viewport == WriteActionDialog || 
       top_viewport == GotoActionDialog ) )
   {
      CustomDialogEventHandler( message ); 
      return;
   }
   else if( top_viewport == drawing )
   {
      /* Handle run mode events here, same as in proto DLL example. */
      /* RunModeEventHandler( message ); */
   }
}

/*---------------------------------------------------------------------
| Required entry point:
|
| May be used to process native windowing events.
*/
glg_export void GlgCustomOptionTraceCallback( GlgTraceCBStruct * trace_data )
{
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
*/
void CustomDialogEventHandler( GlgObject message )
{
   char
     * format,
     * origin,
     * full_origin,
     * action,
     * subaction,
     * dialog_name;

   if( !CurrentDialog )
     return;

   GlgGetSResource( message, "Origin", &origin );
   GlgGetSResource( message, "Format", &format );
   GlgGetSResource( message, "FullOrigin", &full_origin );
   GlgGetSResource( message, "Action", &action );
   GlgGetSResource( message, "SubAction", &subaction );

#if 0
   /* Enable to debug messages (Linux/Unix only, does nothing on Windows) */ 
   printf( "Custom dialog event: format= %s, action= %s, origin= %s, full_origin= %s\n",
          format, action, origin, full_origin );
#endif

   /* Handle window closing. */
   if( strcmp( format, "Window" ) == 0 )
   {
      if( strcmp( action, "DeleteWindow" ) == 0 )
      {
         GlgGetSResource( CurrentDialog, "Name", &dialog_name );

         if( strcmp( origin, dialog_name ) == 0 && /* Came from curr. dialog */
            ApplyDialogInput() )
         {
            GMAbortAllHandlers();   /* Abort tag browser if active */
            CloseCurrentDialog();   /* Resets CurrentDialog to NULL. */
         }
      }
      return;
   }

   if( strcmp( format, "Button" ) == 0 )
   {
      /* Check if it was a button press */
      if( strcmp( action, "Activate" ) != 0 )
      {
         /* Not a button press: just update. */
         GlgUpdate( CurrentDialog );
         return;
      }

      GMAbortAllHandlers();   /* Abort tag browser if active */
      if( strcmp( origin, "CustomOK" ) == 0 )
      {
         /* Check input validity, apply new action parameters and close 
            the dialog if input is valid.
            */
         if( ApplyDialogInput() )
         {
            /* Resets CurrentDialog to NULL - return right away. */
            CloseCurrentDialog();
            return;
         }
      }
      else if( strcmp( origin, "CustomDelete" ) == 0 )
      {
         /* Delete button in the dialog: delete the action. */
         DeleteAction( SelectedObject );

         /* Resets CurrentDialog to NULL - return right away. */
         CloseCurrentDialog();
         return;         
      }
      else if( strcmp( origin, "CustomBrowse" ) == 0 )
      {
         GlgInstallHandlerData * request_data;

         request_data = (GlgInstallHandlerData*) 
           GlgAllocStruct( sizeof( GlgInstallHandlerData ) );
         GMrqType( request_data ) = GLG_BROWSE_DATA_HANDLER;
         GMrqCallback( request_data ) = CustomGotTagNameCallback;

         GMInstallHandler( request_data );
         return;         
      }
   }
   else if( strcmp( format, "Text" ) == 0 )
   {
      if( strcmp( action, "Activate" ) != 0 && 
         strcmp( action, "LosingFocus" ) != 0 )
      {
         GlgUpdate( CurrentDialog );
         return;
      }

      /* Abort data tag browser if user enters a new text input. */
      if( strcmp( action, "Activate" ) == 0 )
        GMAbortAllHandlers();

      /* Check input validity and highlight errors. */
      DialogEntryValid( origin );
   }

   GlgUpdate( CurrentDialog );
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Handles tag source returned from the data browser started by the Browse 
| button.
*/
GlgLong CustomGotTagNameCallback( GlgInstallHandlerData * data )
{
   switch( GMrqReturnCode( data ) )
   {
    case GLG_HANDLER_ABORTED: break;   /* Nothing to do/free here. */

    case GLG_HANDLER_RETURN_VALUE:
      if( ( CurrentAction == WRITE_VALUE || CurrentAction == WRITE_CURRENT ) &&
         GMrqReturnString( data ) )
      {         
         GlgSetSResource( WriteActionDialog, "CustomTagField/TextString",
                         GMrqReturnString( data ) );
         CheckDialogInput();          
         GlgUpdate( WriteActionDialog );
      }
      break;
   }
   return True;
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Enables or disables custom command entries in the PulldownMenu 
| depending on the mode - Edit or Run. There is no need to handle
| the state of entries that depend on object selection: if an object 
| was selected in the Edit mode, GlgCustomOptionSelectionCallback()
| will be invoked to handle selected state when the Run mode is exited.
|
| There is no need to handle entries of PopupMenu or RunMenu tables
| depending on the Edit/Run mode, since the the popup menu is always 
| disabled in the Run mode and the run menu is always disabled in the 
| Edit mode.
*/
void EnableDisableCommands( GlgBoolean edit_mode )
{
   /* In the RunWindow mode a separate RunMenu is used and we don't need 
      to handle Edit mode entries.
      */
   if( !RunWindowMode )
   {
      /* Make these actions enabled only in the edit mode. */
      GMSetMenuSensitivity( PulldownMenuTable, ADD_ACTION, edit_mode );
      GMSetMenuSensitivity( PulldownMenuTable, EDIT_ACTION, edit_mode );
      GMSetMenuSensitivity( PulldownMenuTable, DELETE_ACTION, edit_mode );
      GMSetMenuSensitivity( PulldownMenuTable, SAMPLE_MENU, edit_mode );
      
      /* Enable custom icon for setting object's value only in the Edit mode.*/
      GMSetIconSensitivity( EditActionIcon, edit_mode );
   }

   /* Make RUN_TIME_ACTION enabled only in the run mode. */
   GMSetMenuSensitivity( PulldownMenuTable, RUN_TIME_ACTION, !edit_mode );
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Activates a custom dialog. Returns True on success.
*/
GlgBoolean PopupCustomActionDialog( CustomOP action_type, 
                                   GlgObject selected_obj )
{
   CurrentDialog = GetCustomDialog( action_type );
   if( !CurrentDialog )
     return False;
   CurrentAction = action_type;
     
   if( !DisplayAction( action_type, selected_obj ) )
     return False;

   /* Make the current dialog visible. */
   GlgSetDResource( CurrentDialog, "Visibility", 1. );
   return True;
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Queries selected object to get parameters of a custom action attached to
| it and displays the action's parameters in the dialog.
| Returns True on success.
*/
GlgBoolean DisplayAction( CustomOP action_type, GlgObject selected_obj )
{
   GlgObject action_obj;
   char * string;
   double value;

   /* Get an action attached to the selected object. */
   action_obj = GlgGetResourceObject( selected_obj, "OEMAction" );
   if( !action_obj )
   {
      /* It must exist here... */
      GlgError( GLG_USER_ERROR, "No action attached: can't edit." );
      return False;      
   }

   /* Get action's parameters based on the action type and display them in
      the dialog. */
   switch( action_type )
   {
    case WRITE_VALUE:
      /* Set dialog title. */
      GlgSetSResource( CurrentDialog, "ScreenName", "OEM Write Value Action" );

      /* Set action type display in the dialog to Write Value. */
      GlgSetDResource( CurrentDialog, "ActionNameLabel/WriteValueAction", 1. );

      /* Enable display of the second row in the dialog for entering value. */
      GlgSetDResource( CurrentDialog, "SecondRowVisibility", 1. );      

      /* Fill properties from the action */
      GlgGetSResource( selected_obj, "OEMAction/Tag", &string );
      GlgSetSResource( CurrentDialog, "CustomTagField/TextString", string );

      GlgGetDResource( selected_obj, "OEMAction/Value", &value );
      GlgSetDResource( CurrentDialog, "CustomValueField/Value", value );
      break;

    case WRITE_CURRENT:
      /* Set dialog title. */
      GlgSetSResource( CurrentDialog, "ScreenName", "OEM Write State Action" );

      /* Set action type display in the dialog to Write State. */
      GlgSetDResource( CurrentDialog, "ActionNameLabel/WriteValueAction", 0. );

      /* Disable display of the second row in the dialog for entering value. */
      GlgSetDResource( CurrentDialog, "SecondRowVisibility", 0. ); 

      /* Fill properties from the action */
      GlgGetSResource( selected_obj, "OEMAction/Tag", &string );
      GlgSetSResource( CurrentDialog, "CustomTagField/TextString", string );
      break;

    case POPUP_ACTION:
      /* Set dialog title. */
      GlgSetSResource( CurrentDialog, "ScreenName", "OEM Popup Action" );

      /* Set action type display in the dialog to Popup. */
      GlgSetDResource( CurrentDialog, "ActionNameLabel/GotoAction", 0. );

      /* Fill properties from the action */
      GlgGetSResource( selected_obj, "OEMAction/Target", &string );
      GlgSetSResource( CurrentDialog, "CustomTargetField/TextString", string );
      break;

    case GOTO_ACTION:
      /* Set dialog title. */
      GlgSetSResource( CurrentDialog, "ScreenName", "OEM GoTo Action" );

      /* Set action type display in the dialog to GoTo. */
      GlgSetDResource( CurrentDialog, "ActionNameLabel/GotoAction", 1. );

      /* Fill properties from the action */
      GlgGetSResource( selected_obj, "OEMAction/Target", &string );
      GlgSetSResource( CurrentDialog, "CustomTargetField/TextString", string );
      break;

    default: GlgError( GLG_USER_ERROR, "Invalid action type." ); return False;
   }

   /* Check validity of action parameters to highlight invalid empty 
      entries that need to be filled when the new action is added.
      Don't generate error messages, just hightligh the invalid empty
      entries of the new message that needs to be filled.
      It also resets any possible error highlights left after deleting the
      previous message.
      */
   CheckDialogInput();
   return True;
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Validates all fields of the action editing dialogs' input, highlights
| errors by changing the color of labels of invalid fields to red.
| If input is valid, resets all error color highlights.
| Returns True on success.
*/
GlgBoolean CheckDialogInput()
{
   /* - Pass origin=NULL to validate all dialog fields.
      - Pass check_only=True to check only without applying new values.
      - Pass generate_error=False to highligh errors with color but not to 
      display error messages for the just added actions which have invalid
      empty fields that need to be filled.
      */
   return DialogInputValid( NULL, True, False );
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Applies action editing dialog input to action parameters checking the
| input's validity. Highlights errors by changing the color of labels of
| invalid fields to red and generates error messages that prevent dialog 
| from closing. If input is valid, resets all error color highlights.
| Returns True on success.
*/
GlgBoolean ApplyDialogInput()
{
   /* - Pass origin=NULL to validate all dialog fields.
      - Pass check_only=False to apply new values.
      - Pass generate_error=True to display error messages that prevents
      the dialog from being closed.
      */
   return DialogInputValid( NULL, False, True );
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Validates the input field specified by origin and highlights its
| label in red color if error. Resets the label color if input is valid.
*/
GlgBoolean DialogEntryValid( char * origin )
{
   /* - Pass origin to check input for just that one text field.
      - Pass check_only=True to check only without applying new input to 
      the action. 
      - Pass generate_error=False to highlight only, without generating an
      error message.
      */

   return DialogInputValid( origin, True, False );
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Applies action editing dialog input to action parameters
| checking input's validity. Returns True on success.
| If source is suppled, only the source field is checked.
| If check_only=True, verifies input validity without storing it in 
| the action object.
*/
GlgBoolean DialogInputValid( GlgObject source, GlgBoolean check_only,
                            GlgBoolean generate_error )
{
   GlgBoolean rval = True;
   char * string;
   double value;

   switch( CurrentAction )
   {
    case WRITE_VALUE:
    case WRITE_CURRENT:
      rval &= TagValid( source, "CustomTagField", "TagLabel", "Empty tag.", 
                       generate_error );
      
      if( CurrentAction == WRITE_VALUE )
        rval &= ValueRangeValid( source, "CustomValueField", "ValueLabel", 
                            "Invalid action value, valid range is [0,1000].",
                                generate_error );

      /* Store dialog input in the action if not check only. */
      if( !check_only && rval )
      {
         GlgGetSResource( CurrentDialog, "CustomTagField/TextString", 
                         &string );
         GlgSetSResource( SelectedObject, "OEMAction/Tag", string );

         if( CurrentAction == WRITE_VALUE )
         {
            GlgGetDResource( CurrentDialog, "CustomValueField/Value", 
                            &value );
            GlgSetDResource( SelectedObject, "OEMAction/Value", value );
         }
      }
      break;

    case POPUP_ACTION:
    case GOTO_ACTION:
      rval &= PopupValid( source, "CustomTargetField", "TargetLabel", 
                         "Empty Target.", generate_error );
      
      /* Store dialog input in the action if not check only. */
      if( !check_only && rval )
      {
         /* Store dialog input in the action. */
         GlgGetSResource( CurrentDialog, "CustomTargetField/TextString", 
                         &string );
         GlgSetSResource( SelectedObject, "OEMAction/Target", string );
      }
      break;

    default: 
      GlgError( GLG_USER_ERROR, "Invalid action type." ); 
      /* Don't reset rval to allow closing dialog if error. */
      break;
   }

   return rval;    /* Don't verify other fields/action_types. */
}     

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Checks the value text input for a valid range.
| Returns True on success.
*/
GlgBoolean ValueRangeValid( char * source, char * text_field_name, 
                           char * label_name, char * error_message,
                           GlgBoolean generate_error )
{
   GlgObject text_field, label;
   double input_invalid;

   if( source && strcmp( source, text_field_name ) != 0 )
     return True;  /* Requested to check a different text field - return. */

   label = GlgGetResourceObject( CurrentDialog, label_name );
   text_field = GlgGetResourceObject( CurrentDialog, text_field_name );

   GlgGetDResource( text_field, "InputInvalid", &input_invalid );
   if( input_invalid )
   {
      if( generate_error )
        GlgError( GLG_USER_ERROR, error_message );
            
      /* Highlight invalid input in the red color. */
      GlgSetGResource( label, "TextColor", 1., 0., 0. );
      return False;
   }
   else  /* Input valid */
   {
      /* Set color to black */         
      GlgSetGResource( label, "TextColor", 0., 0., 0. );
      return True;
   }
}           

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Check if tag name is valid: just check if empty as an example.
*/
GlgBoolean TagValid( char * source, char * text_field_name, char * label_name, 
                    char * error_message, GlgBoolean generate_error )
{
   return TextNotEmpty( source, text_field_name, label_name, error_message,
                       generate_error );
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Check if popup name is valid: just check if empty as an example.
*/
GlgBoolean PopupValid( char * source, char * text_field_name, 
                      char * label_name, char * error_message, 
                      GlgBoolean generate_error )
{
   return TextNotEmpty( source, text_field_name, label_name, error_message,
                       generate_error );
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Checks the string text input to be non-empty.
| Returns True on success.
*/
GlgBoolean TextNotEmpty( char * source, char * text_field_name, 
                        char * label_name, char * error_message,
                        GlgBoolean generate_error )
{
   GlgObject text_field, label;
   char * string = NULL;

   if( source && strcmp( source, text_field_name ) != 0 )
     return True;  /* Requested to check a different text field - return. */

   label = GlgGetResourceObject( CurrentDialog, label_name );
   text_field = GlgGetResourceObject( CurrentDialog, text_field_name );

   GlgGetSResource( text_field, "TextString", &string );
   if( !string || !*string )
   {
      if( generate_error )
        GlgError( GLG_USER_ERROR, error_message );
            
      /* Highlight invalid input in the red color. */
      GlgSetGResource( label, "TextColor", 1., 0., 0. );
      return False;
   }
   else  /* Input valid */
   {
      /* Set color to black */         
      GlgSetGResource( label, "TextColor", 0., 0., 0. );
      return True;
   }
}           

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Adds requested action to the selected object.
| Returns True on success.
*/
GlgBoolean AddAction( CustomOP action_type, GlgObject selected_obj )
{
   CustomOP current_action_type;
   GlgObject suspend_info, group, action_group, property;
   double has_resources;
   GlgBoolean rval = True;

   /* Make sure the selected object has the HasResources flag set, 
      so that an action attached to the object is visible as the object's 
      resource. HasResource is always set to YES in the HMI editor, 
      but we have to check for the Graphics Builder.      
      */                           
   GlgGetDResource( selected_obj, "HasResources", &has_resources );
   if( !has_resources )
   {
      GlgError( GLG_WARNING,
               "Setting HasResources flag of the selected object!" );
      GlgSetDResource( selected_obj, "HasResources", 1. );
   }
   
   /* If the selected object already has the action, check if it's an 
      action of a different type. It should not happen (handled by the 
      menu item disabling logis), but check just in case... 
      */
   current_action_type = GetActionType( selected_obj );
   if( current_action_type )
   {
      /* Action already attached: check type. */
      if( current_action_type == action_type )
      {
         GlgError( GLG_WARNING, "Action already attached." );
         return True;
      }
      else   /* A different action is attached: display an error, only one
                action is allowed. */
      {
         GlgError( GLG_USER_ERROR,
                  "To add an action, delete an existing action of a different type first." );
         return False;
      }      
   }

   /* No action is attached: attach a new action. */

   suspend_info = GlgSuspendObject( selected_obj );
      
   /* Check if the selected object has custom data group and add it if 
      it does not exists.
      */     
   group = GlgGetResourceObject( selected_obj, "CustomData" );
   if( group )
   {
      /* Selected object has custom data - make sure the custom data 
         group does not have its HasResources flag set, which is a 
         convention used in this example.
         */                        
      GlgGetDResource( selected_obj, "CustomData/HasResources", 
                      &has_resources );
      if( has_resources )
      {
         GlgSetDResource( selected_obj, "CustomData/HasResources", 0. );
         GlgError( GLG_WARNING, "Resetting HasResources for CustomData!" );
      }
   }
   else   /* No custom data - add! */
   {
      /* Create a group that holds all custom properties of an object*/
      group = GlgCreateObject( GLG_ARRAY, NULL, 
                              (GlgAnyType)GLG_NON_DRAWABLE_OBJECT,
                              NULL, NULL, NULL );
      
      /* Attach the group that will contain custom properties to the 
         selected object. */
      GlgSetResourceObject( selected_obj, "CustomData", group );      
      GlgDropObject( group );
   }

   /* Add MouseClickEvent that activates the action to the CustomData group. 
      We've already checked for absence of foreign mouse click events before 
      invoking AddAction().
      */
   property = GlgCreateObject( GLG_ATTRIBUTE, NULL,
                              (GlgAnyType)GLG_S, (GlgAnyType)GLG_SDATA_XR, 
                              NULL, NULL );
   GlgSetSResource( property, "Name", "MouseClickEvent" ); /* Assign a name */
   GlgSetSResource( property, NULL, "OEMActionEvent" );    /* Assign a label */
   GlgAddObjectToBottom( group, property );    
   GlgDropObject( property );

   /* Add a custom action, which is a group that contains action's 
      parameters. */
   action_group = GlgCreateObject( GLG_ARRAY, "OEMAction", 
                                  (GlgAnyType)GLG_NON_DRAWABLE_OBJECT,
                                  NULL, NULL, NULL );
   /* Set HasResources to make action properies to be visible as resources
      of the action. */
   GlgSetDResource( action_group, "HasResources", 1. );
   GlgAddObjectToBottom( group, action_group );
   GlgDropObject( action_group );
   
   /* Add action type property that holds an OEM_defined action type constants
      to the action. */
   property = GlgCreateObject( GLG_ATTRIBUTE, NULL,
                              (GlgAnyType)GLG_D, (GlgAnyType)GLG_DDATA_XR, 
                              NULL, NULL );
   GlgSetSResource( property, "Name", "ActionType" ); /* Name the property. */
   /* Store action type (OEM-provided value). */
   GlgSetDResource( property, NULL, (double) GetOEMActionType( action_type ) );
   GlgAddObjectToBottom( action_group, property );    
   GlgDropObject( property );

   /* Add action type string for a user-readable action type description. */
   property = GlgCreateObject( GLG_ATTRIBUTE, NULL,
                              (GlgAnyType)GLG_S, (GlgAnyType)GLG_SDATA_XR, 
                              NULL, NULL );
   GlgSetSResource( property, "Name", "ActionString" );
   GlgSetSResource( property, NULL, GetActionString( action_type ) );
   GlgAddObjectToBottom( action_group, property );    
   GlgDropObject( property );

   /* Create and add custom parameters to the action based on its type. */
   switch( action_type )
   {
    case WRITE_VALUE:
      /* Tag to write to. */
      property = GlgCreateObject( GLG_ATTRIBUTE, NULL,
                                 (GlgAnyType)GLG_S, (GlgAnyType)GLG_SDATA_XR, 
                                 NULL, NULL );
      GlgSetSResource( property, "Name", "Tag" ); /* Name the property. */
      GlgAddObjectToBottom( action_group, property );    
      GlgDropObject( property );

      /* Value to write. */
      property = GlgCreateObject( GLG_ATTRIBUTE, NULL,
                                 (GlgAnyType)GLG_D, (GlgAnyType)GLG_DDATA_XR, 
                                 NULL, NULL );
      GlgSetSResource( property, "Name", "Value" );
      GlgAddObjectToBottom( action_group, property );
      GlgDropObject( property );
      break;

    case WRITE_CURRENT:
      /* Tag to write to. */
      property = GlgCreateObject( GLG_ATTRIBUTE, NULL,
                                 (GlgAnyType)GLG_S, (GlgAnyType)GLG_SDATA_XR, 
                                 NULL, NULL );
      GlgSetSResource( property, "Name", "Tag" );
      GlgAddObjectToBottom( action_group, property );    
      GlgDropObject( property );
      break;

    case POPUP_ACTION:
      property = GlgCreateObject( GLG_ATTRIBUTE, NULL,
                                 (GlgAnyType)GLG_S, (GlgAnyType)GLG_SDATA_XR, 
                                 NULL, NULL );
      GlgSetSResource( property, "Name", "Target" );
      GlgAddObjectToBottom( action_group, property );    
      GlgDropObject( property );
      break;

    case GOTO_ACTION:
      property = GlgCreateObject( GLG_ATTRIBUTE, NULL,
                                 (GlgAnyType)GLG_S, (GlgAnyType)GLG_SDATA_XR, 
                                 NULL, NULL );
      GlgSetSResource( property, "Name", "Target" );
      GlgAddObjectToBottom( action_group, property );    
      GlgDropObject( property );
      break;      

    default: GlgError( GLG_USER_ERROR, "Invalid action type." );
      rval = False;
      break;
   }

   GlgReleaseObject( selected_obj, suspend_info );

   /* Activate EditAction OEM icon. */
   GMSetIconSensitivity( EditActionIcon, 1 );

   /* We've added objects - update editor hierarchy display to indicate 
      the presence of custom data attached to the selected object.
      */
   GMUpdateEditorState( GD_UPDATE_HIERARCHY );
   GlgUpdate( TopViewport );
   return rval;
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Deletes a custom action attached to the selected object.
| Returns True on success.
*/
GlgBoolean DeleteAction( GlgObject selected_obj )
{
   GlgObject suspend_info, group, action_obj, mouse_click_prop;
   GlgBoolean rval = True;

   action_obj = GlgGetResourceObject( selected_obj, "OEMAction" );
   if( !action_obj )
   {
      GlgError( GLG_USER_ERROR, "Can't find an action to delete." );
      return False;
   }

   /* Get CustomData group */
   group = GlgGetResourceObject( selected_obj, "CustomData" );
   if( !group )
   {
      GlgError( GLG_USER_ERROR, "Can't find CustomData!" );
      return False;
   }

   suspend_info = GlgSuspendObject( selected_obj );
      
   if( !GlgDeleteThisObject( group, action_obj ) )
   {
      GlgError( GLG_USER_ERROR, "Deleting action failed!" );
      rval = False;
   }

   /* Delete MouseClick event. */
   mouse_click_prop = GlgGetResourceObject( selected_obj, "MouseClickEvent" );
   if( !mouse_click_prop )
     GlgError( GLG_WARNING, 
              "Missing MouseClick while deleting an action." );     
   else if( MouseClickIsForeign( mouse_click_prop ) )
   {
      GlgError( GLG_USER_ERROR,
               "Detected foreign MouseClick: it will not be deleted." );
      rval = False;
   }
   else   /* Has an action's MouseClick - delete it. */
     if( !DeleteElement( group, mouse_click_prop ) )
     {
        GlgError( GLG_USER_ERROR, "Deleting MouseClick failed!" );
        rval = False;
     }
     
   /* If action was was the only item in the CustomData group, delete 
      CustomData group.
      */
   if( GlgGetSize( group ) == 0 )
     GlgSetResourceObject( selected_obj, "CustomData", (GlgObject)0 );

   GlgReleaseObject( selected_obj, suspend_info );

   /* De-activate EditAction OEM icon. */
   GMSetIconSensitivity( EditActionIcon, 0 );

   /* We've deleted objects - update editor hierarchy display to indicate 
      a potential absence of custom data attached to the selected object.
      */
   GMUpdateEditorState( GD_UPDATE_HIERARCHY );
   GlgUpdate( TopViewport );
   return rval;
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Loads all custom action dialogs, returns False on error.
*/
GlgBoolean LoadDialogs()
{
   return LoadDialog( WRITE_VALUE ) && LoadDialog( GOTO_ACTION );
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Loads one specified custom action dialog, returns False on error. 
*/
GlgBoolean LoadDialog( CustomOP op )
{
   GlgObject 
     dialog, 
     * dialog_ptr;
   char 
     * dialog_file,
     * dialog_name,
     * dialog_title,
     * filename;
   GlgLong width, height;

   switch( op )
   {
    case WRITE_VALUE:
      dialog_file = "write_action_dialog.g";      
      dialog_ptr = &WriteActionDialog;
      dialog_name = "WriteActionDialog";
      dialog_title = "OEM Write Action";
      /* Use Layout Toolbox to determine good screen width and height:
         size the dialog, set Coord: Screen in the layout box, then
         click on the Set Width and Set Height buttons to display
         the dialog's current width and height in pixels.
         */
      width = 310;
      height = 140;
      break;

    case GOTO_ACTION:
      dialog_file = "goto_action_dialog.g";
      dialog_ptr = &GotoActionDialog;
      dialog_name = "GotoActionDialog";
      dialog_title = "OEM Popup/Goto Action";
      width = 310;
      height = 105;
      break;
      
    default: 
      GlgError( GLG_USER_ERROR, "Invalid action in LoadDialog()" );
      break;
   }

   /* Load using a path relative to the DLL's load path. */
   filename = GlgConcatStrings( LoadPath, dialog_file );
   dialog = GlgLoadWidgetFromFile( filename );
   GlgFree( filename );
   
   if( !dialog )
   {
      GlgError( GLG_USER_ERROR, "Can't load dialog." );
      return False;
   }
   *dialog_ptr = dialog;
      
   GlgSetSResource( dialog, "Name", dialog_name );
   GlgSetDResource( dialog, "ShellType", (double) GLG_DIALOG_SHELL );
   GlgSetSResource( dialog, "ScreenName", dialog_title );
   GlgSetDResource( dialog, "Visibility", 0. );  /* Make invisible initially */
      
   SetDialogSize( dialog, width, height, /*x*/ 300, /*y*/ 300 );
   
   /* Makes sure the dialog events are reported with the dialog as the 
      top_viewport parameter, making it easier to handle events from 
      custom dialogs.
      */
   GlgActivateCustomDialogEvents( dialog );
      
   GlgAddObjectToBottom( DialogParent, dialog );

   return True;
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Sets an absolute dialog size in pixels instead of a size relative to
| the parent drawing.
*/
void SetDialogSize( GlgObject dialog, GlgLong width, GlgLong height, 
                   GlgLong x, GlgLong y )
{
   /* Disable positioning by the control points be setting both to (0,0,0). */
   GlgSetGResource( dialog, "Point1", 0., 0., 0. );
   GlgSetGResource( dialog, "Point2", 0., 0., 0. );

   /* Set size. */
   GlgSetDResource( dialog, "Screen/WidthHint", (double) width );
   GlgSetDResource( dialog, "Screen/HeightHint", (double) height );

   /* Set position. */
   GlgSetDResource( dialog, "Screen/XHint", (double) x );
   GlgSetDResource( dialog, "Screen/YHint", (double) y );
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Queries visibility of an object.
*/
GlgLong GetVisibility( GlgObject object )
{
   double visibility;

   GlgGetDResource( object, "Visibility", &visibility );
   return visibility > 0.5;     /* Round it up. */
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Updates the state of the visibility toggle to reflect the visibility
| of the object.
*/
void UpdateVisibilityState( GlgObject object )
{
   GlgLong visibility;

   if( !object )
     visibility = False;
   else
     visibility = GetVisibility( object );

   GMSetMenuState( SampleMenuTable,  TOGGLE_VISIBILITY, visibility );   
   GMSetMenuState( SampleMenuTable2, TOGGLE_VISIBILITY, visibility );   

   /* Set the state of both radio toggles in the cascading radio sub-menu.
      Both toggles have to be set for Windows compatibility.
      */
   GMSetMenuState( VisibilityMenuTable,  SET_VISIBLE,  visibility );   
   GMSetMenuState( VisibilityMenuTable2, SET_VISIBLE,  visibility );   

   GMSetMenuState( VisibilityMenuTable,  SET_INVISIBLE, !visibility );   
   GMSetMenuState( VisibilityMenuTable2, SET_INVISIBLE, !visibility );   
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Returns a type of an action attached to the selected object.
*/
CustomOP GetActionType( GlgObject selected_obj )
{
   GlgObject action_type_obj;
   double action_type;

   if( !GlgHasResourceObject( selected_obj, "OEMAction" ) )
     return 0;

   action_type_obj = 
     GlgGetResourceObject( selected_obj, "OEMAction/ActionType" );
   if( !action_type_obj || 
      !GlgGetDResource( action_type_obj, NULL, &action_type ) )
   {
      GlgError( GLG_USER_ERROR, "Action has no ActionType." );
      return 0;
   }

   return GetActionTypeFromOEM( (OEMActionType) action_type );
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Returns a True if the selected object has a custom action attached.
*/
GlgBoolean HasCustomAction( GlgObject selected_obj ) 
{
   return ( GetActionType( selected_obj ) != 0 );
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Returns a True if the selected object has a mouse click event that was
| not attached as a part of a custom message.
*/
GlgBoolean HasForeignMouseClickEvent( GlgObject selected_obj ) 
{
   GlgObject mouse_click_prop;

   mouse_click_prop = GlgGetResourceObject( selected_obj, "MouseClickEvent" );
   return MouseClickIsForeign( mouse_click_prop );
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Returns a True if the mouse click event is foreign (not attached as a 
| part of a custom message).
*/
GlgBoolean MouseClickIsForeign( GlgObject mouse_click_prop ) 
{
   char * label = NULL;

   if( !mouse_click_prop )
     return False;

   GlgGetSResource( mouse_click_prop, NULL, &label );
   return ( !label || strcmp( label, "OEMActionEvent" ) != 0 );
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Checks if action type is valid and returns a valid action or 0.
*/
CustomOP GetActionTypeFromOEM( OEMActionType action_type )
{
   switch( action_type )
   {
      /* Valid action types */
    case OEM_WRITE_VALUE_ACTION:   return WRITE_VALUE;
    case OEM_WRITE_CURRENT_ACTION: return WRITE_CURRENT;
    case OEM_POPUP_ACTION:         return POPUP_ACTION;
    case OEM_GOTO_ACTION:          return GOTO_ACTION;

    default:
      GlgError( GLG_USER_ERROR, "Invalid action type." );
      return 0;
   }
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Converts GLG value (GLG_CUSTOM_START + N) to OEM value.
*/
OEMActionType GetOEMActionType( CustomOP action_type )
{
   switch( action_type )
   {
      /* Valid action types */
    case WRITE_VALUE:   return OEM_WRITE_VALUE_ACTION;
    case WRITE_CURRENT: return OEM_WRITE_CURRENT_ACTION;
    case POPUP_ACTION:  return OEM_POPUP_ACTION;
    case GOTO_ACTION:   return OEM_GOTO_ACTION;
    default:
      GlgError( GLG_USER_ERROR, "Invalid action type." );
      return OEM_UNDEFINED_ACTION;
   }   
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Converts GLG value (GLG_CUSTOM_START + N) to OEM value.
*/
char * GetActionString( CustomOP action_type )
{
   switch( action_type )
   {
      /* Valid action types */
    case WRITE_VALUE:   return "WriteValue";
    case WRITE_CURRENT: return "WriteCurrent";
    case POPUP_ACTION:  return "Popup";
    case GOTO_ACTION:   return "GoTo";
    default:
      GlgError( GLG_USER_ERROR, "Invalid action type." );
      return "Undefined";
   }   
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Returns a dialog to use for the specified action type.
*/
GlgObject GetCustomDialog( CustomOP action_type )
{
   switch( action_type )
   {
    case WRITE_VALUE:
    case WRITE_CURRENT:
      return WriteActionDialog;

    case POPUP_ACTION:
    case GOTO_ACTION:
      return GotoActionDialog;

    default:
      GlgError( GLG_USER_ERROR, "Invalid action type in GetCustomDialog()" );
      return (GlgObject)0;      
   }
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Closes the current dialog.
*/
void CloseCurrentDialog()
{
   GlgSetDResource( CurrentDialog, "Visibility", 0. );
   GlgUpdate( CurrentDialog );

   ModalDialogActive = False;
   CurrentDialog = (GlgObject)0;
   CurrentAction = 0;
}

/*----------------------------------------------------------------------
| Utility function for the sample implementation only.
|
| Deletes element of CustomData that holds data_obj.
*/
GlgBoolean DeleteElement( GlgObject group, GlgObject data_obj )
{
   GlgObject elem, elem_data_obj;
   GlgLong i, size;
   double d_type;

   size = GlgGetSize( group );
   for( i=0; i<size; ++i )
   {
      elem = GlgGetElement( group, i );
      if( !elem )
        continue;
      GlgGetDResource( elem, "Type", &d_type );
      if( ( (int)d_type ) != GLG_ATTRIBUTE )
        continue;

      elem_data_obj = GlgGetResourceObject( elem, "Data" );
      if( elem_data_obj == data_obj )
        return GlgDeleteThisObject( group, elem );
   }
   return False;
}
