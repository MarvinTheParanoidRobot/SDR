#ifndef _Glm_Api_h
#define _Glm_Api_h

#include <stdio.h>
#if GLG_BUILD
#include "../GlgApi.h"
#else
#include "GlgApi.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _GlmObjectType
{
   GLM_MAP = 0,
   GLM_LAYER,
   GLM_FONT,
   GLM_MAX_OBJECT_TYPE
} GlmObjectType;

typedef enum _GlmMapRequestType
{
   GLM_UNDEFINED_MAP_REQUEST = 0,
   GLM_GET_MAP_REQUEST,
   GLM_GET_FEATURE_INFO_REQUEST,
   GLM_GET_LOCATION_INFO_REQUEST,
   GLM_GET_CAPABILITIES_REQUEST,
   GLM_GET_SELECTION_REQUEST
} GlmMapRequestType;

typedef enum _GlmMapOP
{
   GLM_UNDEFINED_OP = 0,
   GLM_GET_CAPABILITIES_OP,
   GLM_GET_MAP_OP,
   GLM_GET_LL_OP,
   GLM_GET_ELEVATION_OP,
} GlmMapOP;

typedef enum _GlmFontType
{
   GLM_HERSH_FONT = 0,
   GLM_FT_FONT
} GlmFontType;

typedef enum _GlmEncoding
{
   GLM_LATIN_ENCODING = 0,
   GLM_UNICODE_ENCODING
} GlmEncoding;

typedef enum _GlmProjection
{
   GLM_RECTANGULAR = 0,
   GLM_SINUSOIDAL,
   GLM_ORTHOGRAPHIC,
} GlmProjectionType;

typedef enum _GlmZoomFactorType
{
   /* Zoom factor is calculated based on the ratio of the whole and visible 
      map extents. */
   GLM_EXTENT_ONLY = 0,

   /* Zoom factor is calculated based on the ratio of the map extents
      as well as the ratio of the map width and the base width.
   */
   GLM_EXTENT_AND_WIDTH,
} GlmZoomFactorType;

typedef enum _GlmLayerType
{
   GLM_IMAGE = 0,
   GLM_GRID,
   GLM_VECTOR,
   GLM_OUTLINE,
   GLM_BACKGROUND,
   GLM_MAX_LAYER
} GlmLayerType;

typedef enum _GlmTransType
{
   GLM_OPAQUE = 0,
   GLM_ALPHA_WHITE_OPAQUE,
   GLM_ALPHA_BLACK_OPAQUE,
   GLM_XOR,
   GLM_BUMP_MAP,
   GLM_TRANSPARENT_COLOR_OPAQUE,
   GLM_MAX_TRANS
} GlmTransType;

typedef enum _GlmImageFormat 
{
   GLM_UNDEFINED_IMAGE_FORMAT = 0,
   GLM_JPEG,
   GLM_TIF,
   GLM_PNG,
   GLM_GIF,
   GLM_BMP,
   GLM_RGB,
   GLM_MAX_IMAGE_FORMAT
} GlmImageFormat;

typedef enum _GlmVectorFormat 
{
   GLM_UNDEFINED_VECTOR_FORMAT = 0,
   GLM_GVF,
   GLM_SHP,
   GLM_MAX_VECTOR_FORMAT
} GlmVectorFormat;

typedef enum _GlmElevationType
{
   GLM_ELEVATION_NONE = 0,
   GLM_ELEVATION_DATA,     /* Data used to query elevation */
   GLM_ELEVATION_COLOR,    /* Convert elevation data to RGB using a threshold */
   GLM_ELEVATION_DATA_AS_COLOR   /* Display elevation data as color for 
                                    debugging. */
} GlmElevationType;

typedef enum _GlmFillType
{
   GLM_NO_EDGE_NO_FILL = 0,
   GLM_EDGE,
   GLM_FILL,
   GLM_FILL_AND_EDGE
} GlmFillType;

typedef enum _GlmTextType
{
   GLM_FLAT = 0,
   GLM_MAPPED_FLAT,
   GLM_TRANSFORMED
} GlmTextType;

typedef enum _GlmTilingType
{
   GLM_UNTILED = 0,
   GLM_SQUARE,
   GLM_TREE
} GlmTilingType;

typedef enum _GlmLabelSelectionMode
{
   GLM_LBL_SEL_NONE = 0,
   GLM_LBL_SEL_IN_TILE_PRECISION,
   GLM_LBL_SEL_MAX_PRECISION,
   GLM_LBL_SEL_UNSET
} GlmLabelSelectionMode;

typedef enum _GlmLabelType
{
   GLM_LINE_LABEL = 0,
   GLM_MID_LINE_LABEL,
   GLM_AREA_LABEL,
   GLM_BBOX_LABEL
} GlmLabelType;

typedef enum _GlmGradientType
{
   GLM_NO_GRADIENT = 0,
   GLM_AROUND_GLOBE
} GlmGradientType;

typedef enum _GlmGlyphCacheType
{
   GLM_NO_GLYPH_CACHE = 0,
   GLM_GLOBAL_GLYPH_CACHE
} GlmGlyphCacheType;

typedef enum _GlmOverrideType
{
   GLM_DISABLE_OVERRIDE = 0,
   GLM_OVERRIDE_UNTILED,
   GLM_OVERRIDE_ALL
} GlmOverrideType;




typedef enum _GlmGridLabelCharPosition
{
   GLM_NO_CHAR = 0,
   GLM_APPEND_CHAR,
   GLM_PREPEND_CHAR
} GlmGridLabelCharPosition;

typedef enum _GlmMGRSAccuracy
{
   GLM_NO_ACCURACY = 0,
   GLM_10000_METER,
   GLM_1000_METER,
   GLM_100_METER,
   GLM_10_METER,
   GLM_1_METER,
   GLM_MAX_ACCURACY
} GlgMGRSAccuracy;

typedef struct _GlmUtmPoint
{
   GlgLong northing;
   GlgLong easting;
   GlgLong zone_number;
   char zone_letter;
} GlmUtmPoint;

typedef enum _GlmLayoutType
{
   GLM_LAYOUT_NONE = 0,
   GLM_LAYOUT_LAYER,
   GLM_LAYOUT_GLOBAL
} GlmLayoutType;

typedef enum _GlmClipType
{
   GLM_CLIP_NONE = 0,
   GLM_CLIP_BOX,
   GLM_CLIP_POLYGON_PIXELS,
   GLM_CLIP_ALL_PIXELS
} GlmClipType;

typedef GlgPoint GlmPoint;
typedef GlgPoint2 GlmPoint2;

typedef GlgObject GlmObject; 

GlgImport( GlgObject ) GlmCreateDataset( char * dataset_name, char * filename );
GlgImport( void ) GlmInitDatasets( char * dataset_filename );
GlgImport( GlgBoolean ) GlmDestroyDataset( char * dataset_name );
GlgImport( void ) GlmDestroyAllDatasets( void );
GlgImport( GlmObject ) GlmAddLayer( char * dataset_name, char * layer_name, 
                                    char * lif_filename );
GlgImport( void ) GlmRemoveLayer( char * dataset_name, char * layer_name );
GlgImport( void ) GlmSetLayers( GlmObject map, char * layers );
GlgImport( void ) GlmResetLayers( GlmObject map );
GlgImport( void ) GlmEnableLayer( GlmObject map, char * layer, 
                                  GlgBoolean override );
GlgImport( void ) GlmDisableLayer( GlmObject map, char * layer );
GlgImport( GlmObject ) GlmCreateMap( void );
GlgImport( void ) GlmDestroyMap( GlmObject map );
GlgImport( GlgBoolean ) GlmSetupMap( GlmObject map );
GlgImport( GlgBoolean ) GlmUpdateMap( GlmObject map );
/* Deprecated */
GlgImport( GlgBoolean ) GlmUpdate( GlmObject map );
GlgImport( GlgLong ) GlmGetProjection( GlmObject map );
GlgImport( GlgObject ) GlmGetSelection( GlmObject map, GlgLong x, GlgLong y, 
                                        GlgLong select_labels );
GlgImport( GlgBoolean ) GlmGetElevation( GlmObject, char *, double, double, double * );
GlgImport( GlgBoolean ) GlmGetLatLon( GlmObject map, double x, double y, 
                                      GlmPoint * lat_lon );
GlgImport( void ) GlmGetXYZ( GlmObject map, double lat, double lon, 
                             GlmPoint * xyz );
GlgImport( void ) GlmResetImageErrors( void );
GlgImport( void ) GlmInit( char * default_dataset_filename );
GlgImport( void ) GlmTerminate( void );
GlgImport( GlgBoolean ) GlmSetDResource( GlmObject map, char * res_name, 
                                         double value );
GlgImport( GlgBoolean ) GlmSetSResource( GlmObject map, char * res_name, 
                                         char * value );
GlgImport( GlgBoolean ) GlmSetGResource( GlmObject map, char * res_name, 
                                         double x_value, double y_value, 
                                         double z_value );
GlgImport( GlgBoolean ) GlmGetDResource( GlmObject map, char * res_name, 
                                         double * value );
GlgImport( GlgBoolean ) GlmGetSResource( GlmObject map, char * res_name, 
                                         char ** value );
GlgImport( GlgBoolean ) GlmGetGResource( GlmObject map, char * res_name, 
                                         double * x_value, double * y_value,
                                         double * z_value );
GlgImport( GlgObject ) GlmGetDatasetArray( void );
GlgImport( unsigned char * ) GlmGetMapData( GlmObject map, GlgLong * size, 
                                            GlgBoolean reset_data );
GlgImport( GlgLong ) GlmWriteImage( FILE * file, char * filename, 
                                    GlmObject map );
GlgImport( int ) GlmMain( int argc, char ** argv );

GlgImport( void ) GlmLatLonToUtm( double lat, double lon, 
                                  GlmUtmPoint * utm_point );
GlgImport( GlgLong ) GlmUtmToLatLon( GlmUtmPoint * utm_point, 
                                     double * lat, double * lon );
GlgImport( char ) * GlmUtmToMgrs( GlmUtmPoint * utm_point, 
                                  GlgMGRSAccuracy accuracy, char * mgrs_buf );
GlgImport( GlgLong ) GlmMgrsToUtm( char * mgrs_buf, GlmUtmPoint * utm_point );

#define GLM_HORIZONTAL_MASK   0x0f
#define GLM_VERTICAL_MASK     0xf0

#define GLM_HCENTER    GLG_HCENTER
#define GLM_HRIGHT     GLG_HRIGHT
#define GLM_HLEFT      GLG_HLEFT
#define GLM_VCENTER    GLG_VCENTER
#define GLM_VTOP       GLG_VTOP
#define GLM_VBOTTOM    GLG_VBOTTOM

#define GLM_CROSS          GLG_CROSS
#define GLM_BOX            GLG_BOX 
#define GLM_FILLED_BOX     GLG_FILLED_BOX
#define GLM_CIRCLE         GLG_CIRCLE 
#define GLM_FILLED_CIRCLE  GLG_FILLED_CIRCLE
#define GLM_DOT            GLG_DOT
#define GLM_CUSTOM_ICON    0x4000  /* doesn't exist in GLG */

#define GLM_TEXT_BG_EDGE   0x1
#define GLM_TEXT_BG_FILL   0x2
#define GLM_TEXT_OUTLINE   0x4

#ifdef __cplusplus
}
#endif
#endif
