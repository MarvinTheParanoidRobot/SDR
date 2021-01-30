#ifndef _glg_custom_proto_h
#define _glg_custom_proto_h

#ifdef __cplusplus
extern "C" {
#endif

#define GLG_CUSTOM_PROTO_VERSION_NUMBER  1

typedef struct _GlgCustomProtoInitData
{
   char * load_path;             /* Load path of the custom option dll. */
   GlgBoolean hmi_mode;          /* Set to True for the HMI configurator. */
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
} GlgCustomProtoInitData;


/* Prototypes for the required entry points of the custom proto module. */
glg_export GlgLong GlgCustomProtoVersionNumber( GlgLong version );
glg_export GlgLong GlgCustomProtoInit( GlgCustomProtoInitData * init_data );
glg_export GlgLong GlgCustomProtoSetSpeed( GlgLong min_speed, 
                                          GlgLong max_speed, GlgLong speed );
glg_export void GlgCustomProtoStartRun( GlgObject drawing, 
                                       GlgLong save_init_values );
glg_export GlgBoolean GlgCustomProtoUpdateRun( GlgObject drawing, 
                                              GlgBoolean pause );
glg_export void 
  GlgCustomProtoEventHandler( GlgObject drawing, GlgObject top_viewport, 
                             GlgObject message, GlgObject viewport );
glg_export void GlgCustomProtoTraceCallback( GlgTraceCBStruct * trace_data );
glg_export void GlgCustomProtoStopRun( GlgObject drawing, 
                                      GlgLong restore_init_values );

#ifdef __cplusplus
}
#endif

#endif
