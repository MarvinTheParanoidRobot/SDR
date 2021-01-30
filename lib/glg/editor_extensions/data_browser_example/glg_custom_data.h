#ifndef _glg_custom_data_h
#define _glg_custom_data_h

#define GLG_CUSTOM_DATA_VERSION_NUMBER   1

#ifdef __cplusplus
extern "C" {
#endif

/* Prototypes for the required entry points of the custom data browser module.
 */
glg_export GlgLong GlgCustomDataVersionNumber( GlgLong version );
glg_export GlgLong GlgCustomDataInit( char * load_path );
glg_export GlgObject GlgGetCustomDataTagArray( GlgObject path_array,
                                              char * filter );
glg_export GlgBoolean GlgGetCustomDataTagString( GlgObject path_array, 
                                                char ** return_tag );
#ifdef __cplusplus
}
#endif

#endif
