#ifndef _glg_custom_option_h
#define _glg_custom_option_h

#ifdef __cplusplus
extern "C" {
#endif

#define GLG_CUSTOM_OPTION_VERSION_NUMBER   1

typedef struct _GlgCustomOptionInitData
{
   char * load_path;             /* Load path of the custom option dll. */
   GlgObject top_viewport;       /* Top viewport of the GLG editor. */
   GlgObject dialog_parent;      /* Parent viewport of custom dialogs. */
   GlgBoolean hmi_mode;          /* Set to True for the HMI configurator. */
   GlgBoolean run_window_mode;   /* Set to True is a separate Run window is
                                    requested (RunWindow mode). */
   GlgObject top_viewport_run;   /* Top viewport of the Run window in the 
                                    RunWindow mode. */
   GlgObject dialog_parent_run;  /* Dialog parent for run mode. */
   GlgBoolean disable_menu;      /* Returns a value. If the custom menu added 
                                    by the dll does not contain run-time 
                                    commends, this field may be set to True to 
                                    request the editor to disable the custom 
                                    menu at run time. */
   GlgBoolean save_drawing;      /* Returns a value. May be set to True to 
                                    request the editor to save the drawing 
                                    before entering the Run mode and reload
                                    it afterwards to preserve resource 
                                    values. */
   GlgBoolean enable_datagen;    /* Returns a value. Setting it to False
                                    disables datagen command used to animate 
                                    the drawing wnen the animation is  
                                    performed by the custom DLL.
                                    Setting it to True enables datagen-based
                                    animation in addition to the animation by  
                                    the custom DLL.
                                    */
} GlgCustomOptionInitData;

/* Prototypes for the required entry points of the custom option module.
   These functions must be provided by any custom option library.
   */
glg_export GlgLong GlgCustomOptionVersionNumber( GlgLong version );
glg_export GlgLong GlgCustomOptionInit( GlgCustomOptionInitData * init_data );
glg_export GlgCustomMenuItem * 
  GlgCustomOptionAddMenu( GlgEditorMenuType menu_type, char ** menu_name );
glg_export void GlgCustomOptionSetupMenu( void );
glg_export GlgBoolean 
  GlgCustomOptionDisableIcon( GlgEditorIconType icon_type, char * label,
                             GlgLong reserved );
glg_export GlgBoolean
  GlgCustomOptionDisableMenuEntry( GlgEditorMenuType menu_type, char * label, 
                                  GlgLong reserved );
glg_export GlgLong 
  GlgCustomOptionManageIcon( GlgEditorIconType icon_type, char * label, 
                            GlgObject icon_viewport, GlgObject icon_drawing );
glg_export GlgBoolean GlgCustomOptionModalDialogCheck( GlgLong op );
glg_export GlgBoolean 
  GlgCustomOptionProcessCommand( GlgLong op, GlgObject selected_obj, 
                                GlgObject drawing );
glg_export void GlgCustomOptionSelectionCallback( GlgObject drawing, 
                                                 GlgObject selected_obj );
glg_export void GlgCustomOptionLoadDrawingCallback( GlgObject drawing, 
                                                   char * filename );
glg_export GlgBoolean GlgCustomOptionSaveDrawingCallback( GlgObject drawing, 
                                                         char * filename );
glg_export void 
  GlgCustomOptionEventHandler( GlgObject drawing, GlgObject top_viewport, 
                              GlgObject message, GlgObject viewport );
glg_export void GlgCustomOptionTraceCallback( GlgTraceCBStruct * trace_data );

/* Run-mode entry points (same as in the custom proto lib) */
glg_export GlgLong GlgCustomOptionSetSpeed( GlgLong min_speed, 
                                          GlgLong max_speed, GlgLong speed );
glg_export void GlgCustomOptionStartRun( GlgObject drawing, 
                                        GlgLong save_init_values );
glg_export GlgBoolean GlgCustomOptionUpdateRun( GlgObject drawing, 
                                               GlgBoolean pause );
glg_export void GlgCustomOptionStopRun( GlgObject drawing, 
                                       GlgLong restore_init_values );


#ifdef __cplusplus
}
#endif

#endif
