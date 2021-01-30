#ifndef _Gvf_h
#define _Gvf_h

#if defined _WINDOWS || defined _WIN32 || defined _WIN64 
#define GLG_EXIT_OK     EXIT_SUCCESS
#define GLG_EXIT_ERROR  EXIT_FAILURE
#else
#define GLG_EXIT_OK     0
#define GLG_EXIT_ERROR  1
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _Glg_Api_h    /* Allow stand-alone compilation. */
#if defined _WIN64 || defined WIN64
typedef long long GlgLong;
#else
typedef long GlgLong;
#endif
#endif

#define GVF_MAGIC        1692
#define GVF_VERSION      2
#define GVF_OBJ_HEADER   7
#define GVF_BBOX_HEADER  -1

typedef enum _GvfSaveFormat
{
   GVF_ASCII = 0,
   GVF_BINARY,
   GVF_EXTENDED,
   GVF_MAX_FORMAT
} GvfSaveFormat;

typedef enum _GvfObjType
{
   GVF_POLYGON = 0,
   GVF_MARKER,
   GVF_MAX_OBJ_TYPE
} GvfObjType;

typedef enum _GvfMarkerType
{
   GVF_M_MARKER = 0,
   GVF_M_LABEL,
   GVF_M_TEXT,
   GVF_MAX_MARKER_TYPE
} GvfMarkerType;

typedef enum _GlmDataType
{
   GLM_UNDEFINED_DATA_TYPE = 0,
   GLM_S,
   GLM_D,
   GLM_G,
   GLM_I      /* Write-only: uses %.0f in ASCII mode */
} GlmDataType;

/* for conversion process */
void GvfWriteHeader( FILE *, GvfSaveFormat );
void GvfWritePolygon( FILE *, GvfSaveFormat, GlgLong, GlgLong, GlgLong );
void GvfWriteMarker( FILE *, GvfSaveFormat, GvfMarkerType, double, double, GlgLong, char *, GlgLong );
void GvfWritePoint( FILE *, GvfSaveFormat, double, double, char * );
void GvfWriteAttribute( FILE *, GvfSaveFormat, GlgLong, GlmDataType, double, char *, double, double, double );
void GvfWriteString( FILE *, GvfSaveFormat, char * );

#ifdef __cplusplus
}
#endif

#endif
