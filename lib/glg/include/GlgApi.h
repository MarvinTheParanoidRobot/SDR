#ifndef _Glg_Api_h
#define _Glg_Api_h

/* Set GLG_C_CONST_CHAR_PTR to 1 to use constant strings (const char *).
 */
#ifndef GLG_C_CONST_CHAR_PTR
#  define GLG_C_CONST_CHAR_PTR  0
#endif

#define GLG_MAJOR_VERSION     3
#define GLG_MINOR_VERSION     8

#include <stdio.h>
#include <time.h>

#if ( defined _WINDOWS || defined _WIN32 || defined WIN32 || defined _WIN64 )
#  define MS_WINDOWS
#  ifdef __BCPLUSPLUS__
#    include <windows.hpp>
#  else
#    include <windows.h>
#    include <windowsx.h>
#  endif
#  define Pixmap HBITMAP
#else
#  define X_WINDOWS
#  include <X11/Xlib.h>
#  include <X11/StringDefs.h>
#  include <X11/Intrinsic.h>
#  include <X11/Xproto.h>
#endif

#ifdef MS_WINDOWS
  /* DLL export. Need to use GlgExport( ret_type ) form (not GlgExport() 
     to handle pascal case.
  */
#  ifdef GLG_STATIC
#    define GlgExport( ret_type )   ret_type
#    define GlgImport( ret_type )   ret_type
#  else
#    ifndef GLG_INTERNAL
       /* Define __BCPLUSPLUS_OLD__ for older versions of Borland */
#      ifndef __BCPLUSPLUS_OLD__
#        define GlgExport( ret_type )    __declspec( dllexport ) ret_type
#        define GlgImport( ret_type )    __declspec( dllimport ) ret_type
#      else
#        define GlgExport( ret_type )    ret_type pascal
#        define GlgImport( ret_type )    ret_type pascal
#      endif   /* __BCPLUSPLUS_OLD__ */
#    else
#      define GlgExport( ret_type )    __declspec( dllexport ) ret_type
#      define GlgImport( ret_type )    GlgExport( ret_type )
#    endif   /* GLG_INTERNAL */
#  endif   /* GLG_STATIC */
#else   /* X/Unix */
#  define GlgExport( ret_type ) ret_type
#  define GlgImport( ret_type ) ret_type
#endif   /* MS_WINDOWS */

#if GLG_C_CONST_CHAR_PTR
#  define CCONST            const
#else
#  define CCONST
#endif

#ifdef MS_WINDOWS
#  define GLG_EXIT_OK     EXIT_SUCCESS
#  define GLG_EXIT_ERROR  EXIT_FAILURE
#  define GLG_EVENT_BASE  ( WM_APP + 2000 )
#else
#  define GLG_EXIT_OK     0
#  define GLG_EXIT_ERROR  1
#  define GLG_EVENT_BASE  ( LASTEvent + 2000 )
#endif

/* Also in Gvf.h */
#if defined _WIN64 || defined WIN64
  typedef long long GlgLong;
  typedef unsigned long long GlgULong;
#else
  typedef long GlgLong;
  typedef unsigned long GlgULong;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/********** GLG Toolkit's defined constants ***********/

#ifndef __BCPLUSPLUS__
#  define GlgFalse  ((GlgBoolean) 0)
#  define GlgTrue   ((GlgBoolean) 1)
#endif

/* Coordinate adjustement used to map the pixel index to the center of the 
   pixel (i.e pixel #5 is mapped to coord=5.5).
*/
#define GLG_COORD_MAPPING_ADJ    0.5

typedef enum _GlgObjectType
{
  GLG_UNDEFINED_OBJECT_TYPE = 0,
  GLG_REFERENCE,
  GLG_DATA,
  GLG_ATTRIBUTE,
  GLG_MATRIX,
  GLG_FUNCTION,
  GLG_XFORM,
  GLG_VECTOR,
  GLG_POLYGON,
  GLG_ARRAY,
  GLG_LIST,
  GLG_SCREEN,
  GLG_DISPLAY,
  GLG_VIEWPORT,
  GLG_MARKER,
  GLG_TEXT,
  GLG_FONTTABLE,
  GLG_ALIAS,
  GLG_ARC,
  GLG_PARALLELOGRAM,
  GLG_SQUARE_SERIES,
  GLG_SERIES,
  GLG_COLORTABLE,
  GLG_HISTORY,
  GLG_POLYLINE,
  GLG_POLYSURFACE,
  GLG_FRAME,
  GLG_FONT,
  GLG_LINK,
  GLG_IMAGE,
  GLG_CONNECTOR,
  GLG_SPLINE,
  GLG_RENDERING,
  GLG_BOX_ATTR,
  GLG_LIGHT,
  GLG_GIS,
  GLG_TAG,
  GLG_ROUNDED,
  GLG_CHART,
  GLG_PLOT,
  GLG_LINE_ATTR,
  GLG_AXIS,
  GLG_LEVEL_LINE,
  GLG_LEGEND,
  GLG_CHART_AREA,
  GLG_ACTION,
  GLG_LIGHT_VIEWPORT,
  GLG_MAX_OBJECT_TYPE
} GlgObjectType;

/* Container is an array by default. */
#define GLG_GROUP     GLG_ARRAY

typedef enum _GlgDataType
{
   GLG_UNDEFINED_DATA_TYPE = 0,
   GLG_S,
   GLG_D,
   GLG_G,
   GLG_L,        /* Map server use only */
   GLG_E         /* Map server use only */
} GlgDataType;

/* Polygon fill type flags, may be or'ed together. */
#define GLG_EDGE           0x1
#define GLG_FILL           0x2
#define GLG_FILL_EDGE      ( GLG_FILL | GLG_EDGE )
#define GLG_LINE_FILL      0x4
#define GLG_EDGE_LINE_FILL ( GLG_EDGE | GLG_LINE_FILL )

/* Polygon open type flags. */
#define GLG_CLOSED         0
#define GLG_OPEN           1
#define GLG_OPEN_INVERSED  2

/* Polygon convex type for OpenGL */
typedef enum _GlgConvexType
{
   GLG_AUTO_CONVEX = 0,   /* Auto-determine. */
   GLG_CONVEX,
   GLG_NON_CONVEX
} GlgConvexType;

/* OpenGL anti-aliasing type */
typedef enum _GlgAntiAliasingType
{
   GLG_ANTI_ALIASING_OFF = 0,
   GLG_ANTI_ALIASING_INT,
   GLG_ANTI_ALIASING_DBL,

   GLG_ANTI_ALIASING_INHERIT = 15

} GlgAntiAliasingType;

/* Polygon shading type */
typedef enum _GlgPolygonShading
{
   GLG_NO_SHADING = 0,
   GLG_EDGE_SHADING = 1,
   GLG_FILL_SHADING = 2,
   GLG_FILL_EDGE_SHADING = 3

} GlgPolygonShading;

/* Selection type */
typedef enum _GlgSelectionType
{
   GLG_DEFAULT_SELECTION_TYPE = 0,
   GLG_SELECT_AS_FILLED = 1,

} GlgSelectionType;

/* Arc fill type flags */
typedef enum _GlgArcFillType
{
   GLG_CHORD = 0,
   GLG_SECTOR,
   GLG_BAND
} GlgArcFillType;

/* Arc angle usage. */
typedef enum _GlgArcAngleDefType
{
   GLG_START_AND_ANGLE = 0,
   GLG_START_AND_END
} GlgArcAngleDefType;

/* Marker type flags, may be or'ed together. */
#define GLG_CROSS          0x1  
#define GLG_BOX            0x2
#define GLG_FILLED_BOX     0x4
#define GLG_CIRCLE         0x8
#define GLG_FILLED_CIRCLE  0x10
#define GLG_DOT            0x20
#define GLG_DIAMOND        0x40
#define GLG_FILLED_DIAMOND 0x80
/* Composites */
#define GLG_CROSS_BOX      ( GLG_CROSS | GLG_BOX )
#define GLG_CROSS_CIRCLE   ( GLG_CROSS | GLG_CIRCLE )
#define GLG_CROSS_DIAMOND  ( GLG_CROSS | GLG_DIAMOND )

/* Text subtypes */
typedef enum _GlgTextType
{
   GLG_UNDEFINED_TEXT_TYPE = 0,
   GLG_FIXED_TEXT,
   GLG_FIT_TO_BOX_TEXT,
   GLG_SCROLLED_TEXT,
   GLG_SPACED_TEXT,
   GLG_TRUNCATED_TEXT,
   GLG_WRAPPED_TEXT,
   GLG_WRAPPED_TRUNCATED_TEXT
} GlgTextType;

/* Backward compatibility, deprecated. */
#define GLG_AUTOSCALED_TEXT    GLG_FIT_TO_BOX_TEXT

/* Scaling type for fixed size parameters: text size, polygon line width,
   marker size, etc.
*/
typedef enum _GlgScalingType
{
   GLG_NO_SCALING     = 0,
   GLG_ZOOM_SCALING   = 1,
   GLG_RESIZE_SCALING = 2,
   GLG_ZOOM_AND_RESIZE_SCALING = 3
} GlgScalingType;

/* Generic orientation flags. */
typedef enum _GlgOrientationType
{
   GLG_HORIZONTAL = 0,
   GLG_VERTICAL
} GlgOrientationType;

/* Text direction flags. */
typedef enum _GlgTextDirection
{
   GLG_HORIZONTAL_TEXT = GLG_HORIZONTAL,
   GLG_VERTICAL_TEXT = GLG_VERTICAL,
   GLG_VERTICAL_ROTATED_RIGHT,
   GLG_VERTICAL_ROTATED_LEFT
} GlgTextDirection;

/* The chosen value of the horizontal anchoring flag should be or'ed with
   the chosen vertical achoring flag value to form the value of the
   Anchor attribute. */

/* Text and image horizontal anchoring flags. */
#define GLG_HCENTER        0x0
#define GLG_HLEFT          0x1
#define GLG_HRIGHT         0x2

/* Text and image vertical anchoring flags. */
#define GLG_VCENTER        0x00
#define GLG_VTOP           0x10
#define GLG_VBOTTOM        0x20

/* Line anchoring type for multi-line text. */
typedef enum _GlgLineAnchoringType
{
   GLG_LCENTER  = GLG_HCENTER,
   GLG_LLEFT    = GLG_HLEFT,
   GLG_LRIGHT   = GLG_HRIGHT,
   GLG_LINHERIT = 0x400
} GlgLineAnchoringType;

/* Time conversion type for the time format xform. */
typedef enum _GlgTimeConversionType
{
   GLG_LOCAL_TIME = 0,
   GLG_UTC_TIME,
   GLG_RELATIVE_TIME
} GlgTimeConversionType;

/* Axis direction masks. */
#define GLG_POSITIVE_AXIS_MASK  0x1
#define GLG_VERTICAL_AXIS_MASK  0x8

/* Axis direction options */
typedef enum _GlgAxisDirection
{
   /* The first item (i.e. HTOP) is axis' position, the second (i.i. UP) is
      the positive direction of the tick length. */
   GLG_HTOP_UP = 0,
   GLG_HTOP_DOWN,
   GLG_HCENTER_UP,
   GLG_HCENTER_DOWN,
   GLG_HBOTTOM_UP,
   GLG_HBOTTOM_DOWN,

   GLG_VLEFT_LEFT = 8,
   GLG_VLEFT_RIGHT,
   GLG_VCENTER_LEFT,
   GLG_VCENTER_RIGHT,
   GLG_VRIGHT_LEFT,
   GLG_VRIGHT_RIGHT

} GlgAxisDirection;

/* Axis types */
typedef enum _GlgAxisType
{
   GLG_VALUE_AXIS = 0,           /* Value axis and XY plot */
   GLG_RULER_AXIS,               /* Ruler based on pixels or other units */
   GLG_CENTER_RULER_AXIS,        /* Ruler with labels centered in the middle
                                    of the unit slot. */

   /* Scrolling axis types. */
   GLG_LOCAL_TIME_SCROLL_AXIS,   
   GLG_REL_TIME_SCROLL_AXIS,
   GLG_INDEX_SCROLL_AXIS,        /* Scrolling sample indices */
   GLG_CENTER_INDEX_SCROLL_AXIS, /* Scrolling sample indices centered in 
                                    the middle of a sample's slot. */
   GLG_VALUE_SCROLL_AXIS,        /* Scrolling values other than time. */

   /* Ruler variants: keep a constant pixel per unit ratio. */
   GLG_RULER_SCROLL_AXIS,
   GLG_CENTER_RULER_SCROLL_AXIS,
   GLG_LOCAL_TIME_RULER_SCROLL_AXIS,
   GLG_REL_TIME_RULER_SCROLL_AXIS,
   GLG_INDEX_RULER_SCROLL_AXIS,
   GLG_CENTER_INDEX_RULER_SCROLL_AXIS,

   /* UTC versions */
   GLG_UTC_TIME_SCROLL_AXIS,   
   GLG_UTC_TIME_RULER_SCROLL_AXIS

} GlgAxisType;

/* Backward compatibility, deprecated. */
#define GLG_ABS_TIME_SCROLL_AXIS         GLG_LOCAL_TIME_SCROLL_AXIS
#define GLG_ABS_TIME_RULER_SCROLL_AXIS   GLG_LOCAL_TIME_RULER_SCROLL_AXIS  

/* Axis outline binary flags.*/
typedef enum _GlgAxisOutlineFlags
{
   GLG_AXIS_ELEM   = 1,
   GLG_BOX_ELEM    = 2,
   GLG_MINOR_ELEM  = 4,
   GLG_MAJOR_ELEM  = 8
} GlgAxisOutlineElem;

/* Types of the axis outline */
typedef enum _GlgAxisOutlineType
{
   GLG_NO_OUTLINE = 0,
   GLG_AXIS_LINE   = GLG_AXIS_ELEM,
   GLG_LINES_MINOR = ( GLG_AXIS_ELEM | GLG_MINOR_ELEM ),
   GLG_LINES_MAJOR = ( GLG_AXIS_ELEM | GLG_MAJOR_ELEM ),
   GLG_LINES_ALL   = ( GLG_AXIS_ELEM | GLG_MINOR_ELEM | GLG_MAJOR_ELEM ),
   GLG_BOX_MINOR   = ( GLG_AXIS_ELEM | GLG_BOX_ELEM | GLG_MINOR_ELEM ),
   GLG_BOX_MAJOR   = ( GLG_AXIS_ELEM | GLG_BOX_ELEM | GLG_MAJOR_ELEM ),
   GLG_BOX_ALL     = ( GLG_AXIS_ELEM | GLG_BOX_ELEM | 
                     GLG_MINOR_ELEM | GLG_MAJOR_ELEM )
} GlgAxisOutlineType;

/* Legend layout types */
typedef enum _GlgLegendLayoutType
{
   GLG_HORIZONTAL_LEGEND = 0,
   GLG_HORIZONTAL_WRAPPED_LEGEND,
   GLG_VERTICAL_LEGEND,
   GLG_VERTICAL_WRAPPED_LEGEND

} GlgLegendLayoutType;

/* Spline subtypes */
typedef enum _GlgSplineType
{
   GLG_UNDEFINED_SPLINE = 0,
   GLG_B_SPLINE,
   GLG_C_SPLINE
} GlgSplineType;

/* Image subtypes */
typedef enum _GlgImageType
{
   GLG_UNDEFINED_IMAGE_TYPE = 0,
   GLG_FIXED_IMAGE,
   GLG_SCALED_IMAGE
} GlgImageType;

typedef enum _GlgImageFormat
{
   GLG_GIF = 0,
   GLG_BMP,
   GLG_JPEG,
   GLG_RAW,
   GLG_PNG
} GlgImageFormat;

/* GIS Projections, map server support */
typedef enum _GlgProjectionType
{
   GLG_UNDEFINED_PROJECTION = 0,
   GLG_RECTANGULAR_PROJECTION,
   GLG_ORTHOGRAPHIC_PROJECTION
} GlgProjectionType;

/* A negative value for Z coordinates of converted points outside of the 
   visible part of the globe in the orthographic projection.
   */
#define GLG_GIS_OUTSIDE_VALUE     -2000.

/* Timer Xform Type */
typedef enum _GlgWaveformType
{
   GLG_SAWTOOTH_WAVEFORM = 0,
   GLG_TRIANGLE_WAVEFORM,
   GLG_CIRCULAR_WAVEFORM,
   GLG_SINE_WAVEFORM
} GlgWaveformType;

/* Path Rotation Type */
typedef enum _GlgPathRotationType
{
   GLG_DONT_ROTATE_PATH = 0,
   GLG_ROTATE_PATH,
   GLG_ROTATE_DONT_MOVE_PATH
} GlgPathRotationType;

/* ZSort types */
typedef enum _GlgZSortType
{
   GLG_ZS_INHERIT = 0,
   GLG_ZS_YES,
   GLG_ZS_NO,
   GLG_ZS_BY_PARENT,
   GLG_ZS_SPECIAL,
   GLG_ZS_NO_GDI
} GlgZSortType;

/* Backward compatibility, deprecated. */
#define GLG_INHERIT        GLG_ZS_INHERIT
#define GLG_YES            GLG_ZS_YES
#define GLG_NO             GLG_ZS_NO
#define GLG_BY_PARENT      GLG_ZS_BY_PARENT
#define GLG_SPECIAL        GLG_ZS_SPECIAL

typedef enum _GlgHistoryScrollType
{
   GLG_WRAPPED = 0,
   GLG_SCROLLED
} GlgHistoryScrollType;

typedef enum _GlgTagType
{
   GLG_DATA_TAG = 1,
   GLG_EXPORT_TAG = 2,
   GLG_EXPORT_DYN_TAG = 4
} GlgTagType;

typedef enum _GlgTagAccessType
{
   GLG_INPUT_TAG = 0,
   GLG_INIT_ONLY_TAG = 1,
   GLG_OUTPUT_TAG = 2,

   GLG_CUSTOM_ACCESS_TAG = 8      /* Custom flag for programmatic use */
} GlgTagAccessType;

/* Viewport lighting types */
typedef enum _GlgLightType
{
   GLG_NO_LIGHT = 0,
   GLG_FLAT_LIGHT = 1,
   GLG_FLAT_LIGHTPOINT_LIGHT = 2
} GlgLightType;

#define GLG_EQUATOR_RADIUS     6378136.
#define GLG_POLAR_RADIUS       6356752.

/* Type of the container's elements. */
typedef enum _GlgContainerType
{
   GLG_UNDEFINED_CONTAINER_TYPE = 0,
   GLG_OBJECT,
   GLG_STRING,
   GLG_LONG,
   GLG_RESERVED_DATA_TYPE_0,
   GLG_RESERVED_DATA_TYPE_1,
   GLG_NON_DRAWABLE_OBJECT,         /* For creating custom property groups. */
   GLG_RESERVED_DATA_TYPE_2,
   GLG_RESERVED_DATA_TYPE_3,
   GLG_RESERVED_DATA_TYPE_4,
   GLG_ALLOCATED_POINTER            /* Pointers will be freed with GlgFree when
                                       the container is destroyed. */
} GlgContainerType;

typedef enum _GlgCloneType
{
   GLG_WEAK_CLONE = 0,
   GLG_STRONG_CLONE,
   GLG_FULL_CLONE,
   GLG_CONSTRAINED_CLONE,
   GLG_SHALLOW_CLONE
} GlgCloneType;

typedef enum _GlgFrameType
{
   GLG_UNDEFINED_FRAME_TYPE = 0,
   GLG_1D,
   GLG_2D,
   GLG_3D
} GlgFrameType;

typedef enum _GlgXformType
{
   GLG_UNDEFINED_XF =  0,
   GLG_RESERVED_XF_1,
   GLG_TRANSLATE_XF,
   GLG_RESERVED_XF_2,
   GLG_RESERVED_XF_3,
   GLG_MATRIX_XF,
   GLG_PATH_XF,
   GLG_TRANSLATE_X_XF,
   GLG_TRANSLATE_Y_XF,
   GLG_TRANSLATE_Z_XF,
   GLG_TRANSLATE_XYZ_XF,
   GLG_SCALE_X_XF,
   GLG_SCALE_Y_XF,
   GLG_SCALE_Z_XF,
   GLG_SCALE_XYZ_XF,
   GLG_ROTATE_X_XF,
   GLG_ROTATE_Y_XF,
   GLG_ROTATE_Z_XF,
   GLG_CONCATENATE_XF,
   GLG_MULTIPLY_XF,       /* Deprecated, use GLG_LINEAR3 */
   GLG_D_FORMAT_XF,
   GLG_S_FORMAT_XF,
   GLG_LIST_XF,
   GLG_DIVIDE_XF,
   GLG_TRANSFER_XF,
   GLG_RANGE_XF,          /* Deprecated, use GLG_RANGE_CONVERSION */
   GLG_THRESHOLD_XF,
   GLG_SCALE_TRANSLATE_XF,
   GLG_SCALE_TRANSLATE_XYZ_XF,
   GLG_LINEAR2_XF,       /* Deprecated (Linear Sum), use GLG_LINEAR3 */
   GLG_SHEAR_X_XF,
   GLG_SHEAR_Y_XF,
   GLG_SHEAR_Z_XF,
   GLG_TIMER_XF,
   GLG_RESERVED_XF_4,
   GLG_BOOLEAN_XF,
   GLG_SMAP_XF,
   GLG_STRING_CONCAT_XF,
   GLG_RESERVED_XF_5,
   GLG_RESERVED_XF_6,
   GLG_RANGE_CHECK_XF,
   GLG_RANGE_ALARM_XF,
   GLG_RANGE2_ALARM_XF,
   GLG_CHANGE_ALARM_XF,
   GLG_WORLD_OFFSET_XF,
   GLG_SCREEN_OFFSET_XF,
   /* Scale to screen coordinates */
   GLG_SCALE_SX_XF,
   GLG_SCALE_SY_XF,
   GLG_SCALE_SZ_XF,   
   GLG_TIME_FORMAT_XF,
   GLG_RESERVED_XF_7,
   GLG_RANGE_CONVERSION_XF,
   GLG_IDENTITY_XF,
   GLG_LINEAR3_XF,
   GLG_COMPARE_XF,
   GLG_BITMASK_XF,
   GLG_JAVA_SCRIPT_XF,
   GLG_DMAP_XF,
   GLG_G_FROM_D_XF,
   GLG_D_FROM_G_XF,
   GLG_SCREEN_FACTOR_XF,

   GLG_MAX_XFORM_TYPE
} GlgXformType;

#define GLG_COMBO_XF  GLG_LINEAR2_XF 
#define GLG_LINEAR_XF GLG_MULTIPLY_XF 
#define GLG_SLIST_XF  GLG_SMAP_XF

typedef enum _GlgXformRole
{
   GLG_UNDEFINED_XR = 0,
   GLG_GEOM_XR,
   GLG_COLOR_XR,
   GLG_RESERVED_XR_1,
   GLG_RESERVED_XR_2,
   GLG_RESERVED_XR_3,
   GLG_RESERVED_XR_4,
   GLG_GDATA_XR,
   GLG_DDATA_XR,
   GLG_SDATA_XR
} GlgXformRole;

/* Type of a boolean transformation. */
typedef enum _GlgBoolXformType
{
   GLG_BOOL0_XT = 0,  /*   A ||  B   ||  C */
   GLG_BOOL1_XT,      /*   A ||  B   || !C */
   GLG_BOOL2_XT,      /*   A || !B   || !C */
   GLG_BOOL3_XT,      /*  !A || !B   || !C */
   GLG_BOOL4_XT,      /* ( A ||  B ) &&  C */
   GLG_BOOL5_XT,      /* ( A ||  B ) && !C */
   GLG_BOOL6_XT,      /* ( A || !B ) &&  C */
   GLG_BOOL7_XT,      /* ( A || !B ) && !C */
   GLG_BOOL8_XT,      /*   A &&  B   &&  C */
   GLG_BOOL9_XT,      /*   A &&  B   && !C */
   GLG_BOOL10_XT,     /*   A && !B   && !C */
   GLG_BOOL11_XT,     /*  !A && !B   && !C */
   GLG_BOOL12_XT,     /*   A &&  B   ||  C */
   GLG_BOOL13_XT,     /*   A && !B   ||  C */
   GLG_BOOL14_XT,     /*   A &&  B   || !C */
   GLG_BOOL15_XT,     /*   A && !B   || !C */
   GLG_BOOL16_XT,     /*  !A */
   GLG_BOOL17_XT,     /*   A */
   GLG_BOOL18_XT,     /*   A || !B   ||  C */
   GLG_BOOL19_XT,     /*  !A ||  B   ||  C */
   GLG_BOOL20_XT,     /*  !A || !B   ||  C */
   GLG_BOOL21_XT,     /*  !A ||  B   || !C */
   GLG_BOOL22_XT,     /*   A && !B   &&  C */
   GLG_BOOL23_XT,     /*  !A &&  B   &&  C */
   GLG_BOOL24_XT,     /*  !A && !B   &&  C */
   GLG_BOOL25_XT,     /*  !A &&  B   && !C */
   GLG_BOOL26_XT,     /*( !A ||  B ) &&  C */
   GLG_BOOL27_XT,     /*( !A ||  B ) && !C */
   GLG_BOOL28_XT,     /*( !A || !B ) &&  C */
   GLG_BOOL29_XT,     /*( !A || !B ) && !C */
   GLG_BOOL30_XT,     /*  !A &&  B   ||  C */
   GLG_BOOL31_XT,     /*  !A &&  B   || !C */
   GLG_BOOL32_XT,     /*  !A && !B   ||  C */
   GLG_BOOL33_XT      /*  !A && !B   || !C */

} GlgBoolXformType;

/* Conversion type for parameters of a boolean transformation. */
typedef enum _GlgBoolConversionType
{
   GLG_EQUAL_ZERO_BOOL = 0,
   GLG_GREATER_THAN_ZERO_BOOL,
   GLG_GREATER_THAN_HALF_BOOL,
   GLG_ABS_GREATER_THAN_HALF_BOOL 

} GlgBoolConversionType;

/* Comparison operator of a compare transformation. */
typedef enum _GlgCompareXformType
{
   GLG_COMPARE_EQ_XT = 0,  /* A == B */
   GLG_COMPARE_NE_XT,      /* A != B */
   GLG_COMPARE_LT_XT,      /* A <  B  */
   GLG_COMPARE_LE_XT,      /* A <= B */
   GLG_COMPARE_GT_XT,      /* A >  B  */
   GLG_COMPARE_GE_XT       /* A >= B */
     
} GlgCompareXformType;

/* Move Flag of the offset transformations. */
typedef enum _GlgOffsetXformMoveFlag
{
   GLG_CHANGE_OFFSETS = 0,
   GLG_MOVE_ANCHOR_POINT

} GlgOffsetXformMoveFlag;

/* Reference object subtypes. */
typedef enum _GlgRefType
{
   GLG_UNDEFINED_REF_TYPE = 0,
   GLG_SUBDRAWING_REF,
   GLG_CONTAINER_REF,
   GLG_SUBWINDOW_REF
} GlgRefType;

/* Backward compatibility, deprecated. */
#define GLG_REFERENCE_REF   GLG_SUBDRAWING_REF

/* Storage types for the GLG_REFERENCE_REF. */
typedef enum _GlgStorageType
{
   GLG_USE_INCLUDED = 0,
   GLG_USE_FILE,
   GLG_USE_PALETTE
} GlgStorageType;

/* Backward compatibility, deprecated. */
#define GLG_USE_TEMPLATE       GLG_USE_INCLUDED

typedef enum _GlgModifierType
{
   GLG_UNDEFINED_MOD = 0,
   GLG_SHIFT_MOD,
   GLG_CONTROL_MOD,
   GLG_DOUBLE_CLICK_MOD
} GlgModifierType;

/* Backward compatibility - deprecated. */
#define GlgModifierKey             GlgModifierType
#define GLG_UNDEFINED_MODIFIER_KEY GLG_UNDEFINED_MOD
#define GLG_SHIFT_KEY              GLG_SHIFT_MOD
#define GLG_CONTROL_               KEY GLG_CONTROL_MOD

typedef enum _GlgSearchType
{
   GLG_UNDEFINED_SEARCH_TYPE = 0,
   GLG_FIND_OBJECT,
   GLG_FIND_BY_NAME,
   GLG_RESERVED_SEARCH_TYPE_1,
   GLG_FIND_STRING,  /* For arrays containing strings: compare the actual
		      * strings instead of just matching the string ptr.
		      */
   GLG_RESERVED_SEARCH_TYPE_2,
   GLG_RESERVED_SEARCH_TYPE_3,
   GLG_RESERVED_SEARCH_TYPE_4,
   GLG_RESERVED_SEARCH_TYPE_5,
   GLG_RESERVED_SEARCH_TYPE_6,
   GLG_FIND_BY_INDEX = 100
} GlgSearchType;

typedef enum _GlgAccessType
{
   GLG_UNDEFINED_ACCESS_TYPE = 0,
   GLG_BOTTOM,
   GLG_TOP,
   GLG_CURRENT
} GlgAccessType;

typedef enum _GlgPositionModifier
{
   GLG_UNDEFINED_POSITION_MODIFIER = 0,
   GLG_BEFORE,
   GLG_AFTER
} GlgPositionModifier;

typedef enum _GlgMoveMode
{
   GLG_MOVE_POINTS = 0,
   GLG_MOVE_BY_XFORM,
   GLG_STICKY_CENTER_MODE
} GlgMoveMode;

typedef enum _GlgGlobalType
{
   GLG_LOCAL = 0,
   GLG_SEMI_GLOBAL,
   GLG_GLOBAL,
   GLG_UNCONSTRAINED,
   GLG_BOUND
} GlgGlobalType;

typedef enum _GlgCallbackType
{
   GLG_UNDEFINED_CALLBACK_TYPE = 0,
   GLG_INPUT_CB,
   GLG_SELECT_CB,
   GLG_TRACE_CB,
   GLG_TRACE2_CB,
   GLG_MOTIF_INPUT_CB,
   GLG_MOTIF_SELECT_CB,
   GLG_CUSTOM_DRAW_CB,
   GLG_HIERARCHY_CB
} GlgCallbackType;

typedef enum _GlgHierarchyCallbackType
{
   GLG_UNDEFINED_CALLBACK_REASON = 0,
   GLG_BEFORE_SETUP_CB,
   GLG_AFTER_SETUP_CB,
} GlgHierarchyCallbackType;

typedef enum _GlgWidgetType
{
   GLG_DRAWING_AREA_WIDGET = 0,
   GLG_PUSH_BUTTON_WIDGET,
   GLG_DRAWN_BUTTON_WIDGET,
   GLG_TOGGLE_BUTTON_WIDGET,
   GLG_MAIN_WINDOW_WIDGET,
   GLG_BULLETIN_WIDGET,
   GLG_FORM_WIDGET,
   GLG_ROW_COLUMN_WIDGET,
   GLG_LEFT_ARROW_WIDGET,
   GLG_RIGHT_ARROW_WIDGET,
   GLG_UP_ARROW_WIDGET,
   GLG_DOWN_ARROW_WIDGET,
   GLG_HORIZONTAL_SCALE_WIDGET,
   GLG_VERTICAL_SCALE_WIDGET,
   GLG_HORIZONTAL_SCROLL_WIDGET,
   GLG_VERTICAL_SCROLL_WIDGET,
   GLG_MENU_BAR_WIDGET,
   GLG_RESERVED_WIDGET_TYPE1,
   GLG_RESERVED_WIDGET_TYPE2,
   GLG_TEXT_WIDGET,
   GLG_LABEL_WIDGET,
   GLG_OPTION_MENU_WIDGET,
   GLG_PULL_DOWN_MENU_WIDGET,
   GLG_VERTICAL_SEPARATOR_WIDGET,
   GLG_HORIZONTAL_SEPARATOR_WIDGET,
   GLG_LIST_WIDGET,
   GLG_TEXT_EDIT_WIDGET,
   GLG_MULTI_LIST_WIDGET,
   GLG_EXT_LIST_WIDGET,
   GLG_DIALOG_AREA_WIDGET,
   GLG_MAX_WIDGET_TYPE     
} GlgWidgetType;

typedef enum _GlgShellType
{
   GLG_NO_TOP_SHELL = 0,
   GLG_DIALOG_SHELL,
   GLG_APPLICATION_SHELL
} GlgShellType;

typedef enum _GlgComponentQueryType
{
   GLG_WIDGET_QUERY = 0,
   GLG_SHELL_QUERY,
   GLG_CHILD_WIDGET_QUERY,
   GLG_V_SCROLLBAR_QUERY,   /* X Windows only */
   GLG_H_SCROLLBAR_QUERY,   /* X Windows only */
   GLG_DISPLAY_QUERY        /* X Windows only */
} GlgComponentQueryType;

typedef enum _GlgShadowType
{
   GLG_SHADOW_OUT = 0,
   GLG_SHADOW_IN
} GlgShadowType;

typedef enum _GlgStretchType
{
   GLG_NO_STRETCH = 0,
   GLG_RESIZE_STRETCH = 1,
   GLG_RESIZE_AND_ZOOM_STRETCH = 2
} GlgStretchType;

/* Coordinate types for transforming objects */
typedef enum _GlgCoordType
{
   GLG_SCREEN_COORD = 0,
   GLG_WORLD_COORD,     /* Includes only the zooming xform of the viewport */
   GLG_DRAWING_COORD,   /* Includes all viewing xforms of the viewport */
   GLG_PARENT_COORD,
   GLG_OBJECT_COORD
} GlgCoordType;

/* Backward compatibility, deprecated. */
#define GLG_VIEW_COORD    GLG_WORLD_COORD

/* Scaling adjustment type for the screen offset and screen scaling xforms */
typedef enum _GlgScaleAdjustmentType
{
   GLG_WORLD_SCALING = 0,  /* Does not apply any adjustments. */
   GLG_SCREEN_SCALING,     /* Adjusts by a screen scale factor to use
                              screen coordinates. */
   GLG_RATIO_SCALING,      /* Adjusts by a ratio of the x and y screen scale 
                              factors to maintain X/Y ratio. */
   GLG_INVERSED_WORLD_SCALING,
   GLG_INVERSED_SCREEN_SCALING,
   GLG_INVERSED_RATIO_SCALING
} GlgScaleAdjustmentType;

/* Control point type. */
typedef enum _GlgControlPointType
{
   GLG_DEFAULT_POINT_TYPE = 0
} GlgControlPointType;

/* Drawing coordinate system (CoordSystem) that defines the drawing's 
   resize policy.
 */
typedef enum _GlgDrawingCoordSystem
{
   GLG_WORLD_COORD_SYSTEM = 0,
   GLG_SCREEN_COORD_SYSTEM,         /* Y axis is down */
   GLG_FLIPPED_SCREEN_COORD_SYSTEM, /* Y axis is up */
   GLG_SCREEN_CENTER_COORD_SYSTEM   /* Y axis is up, origin is in the center */
} GlgDrawingCoordSystem;

/* Object coordinate system flag (CoordFlag): defines the coordinate system 
   used to interpret point coordinates of an object.
*/
typedef enum _GlgObjectCoordSystem
{
   GLG_INHERIT_COORD_SYSTEM = 0,        /* Use drawing's coord. system */
   GLG_ABS_SCREEN_COORD_SYSTEM,         /* Y axis is down */
   GLG_ABS_FLIPPED_SCREEN_COORD_SYSTEM, /* Y axis is up */
   GLG_LVP_SCREEN_COORD_SYSTEM = 4,     /* Inside light viewport */
   GLG_LVP_FLIPPED_SCREEN_COORD_SYSTEM  /* Inside light viewport */
} GlgObjectCoordSystem;

/* X and Y pan flags may be ORed to form the pan type. */
typedef enum _GlgPanType
{
   GLG_NO_PAN        = 0,
   GLG_PAN_X         = 1,
   GLG_PAN_Y         = 2,
   GLG_PAN_XY        = 3,
   GLG_PAN_X_AUTO    = 4,
   GLG_PAN_Y_AUTO    = 8,
   GLG_PAN_XY_AUTO   = 12
} GlgPanType;

/* Chart Grid and CrossHair flags. */
typedef enum _GlgEnableXY
{
   GLG_DISABLE_XY = 0,
   GLG_ENABLE_X   = 1,
   GLG_ENABLE_Y   = 2,
   GLG_ENABLE_XY  = 3
} GlgEnableXY;

/* Chart elements drawing order flags (may be ORed together). */
typedef enum _GlgChartElemDrawOrder
{
   GLG_GRID_FOREGROUND    = 1,
   GLG_OUTLINE_FOREGROUND = 2,
   GLG_LEVEL_FOREGROUND   = 4
} GlgChartElemDrawOrder;

/* Plot types. */
typedef enum _GlgPlotType
{
   GLG_LINE_PLOT = 1,
   GLG_STEP_PLOT = 2,
   GLG_BAR_PLOT = 3,
   GLG_FLOATING_BAR_PLOT = 4,

   GLG_MARKERS_PLOT = 32,

   GLG_LINE_AND_MARKERS_PLOT = 33,
   GLG_STEP_AND_MARKERS_PLOT = 34
} GlgPlotType;

/* Chart filter types. */
typedef enum _GlgChartFilterType
{
   GLG_NULL_FILTER = 0,
   GLG_MIN_MAX_FILTER,
   GLG_AVERAGE_FILTER,
   GLG_DISCARD_FILTER,
   GLG_BAR_MIN_MAX_FILTER,

   GLG_CUSTOM_FILTER = 101

} GlgChartFilterType;

typedef enum _GlgChartFilterRval
{
   GLG_SKIP_DATA = 0,
   GLG_USE_DATA1 = 2,
   GLG_USE_DATA2
} GlgChartFilterRval;

typedef enum _GlgTabNavigationType
{
   GLG_TAB_NONE             = 0,
   GLG_TAB_TEXT_BOXES       = 1,
   GLG_TAB_BUTTONS          = 2,
   GLG_TAB_TEXT_AND_BUTTONS = 3

} GlgTabNavigationType;

/* Parallelogram subtype. */
typedef enum _GlgParallType
{
   GLG_PA_PA = 0,   /* 3 points */
   GLG_RECT_PA      /* 2 points */

} GlgParallType;

typedef enum _GlgMapServerUsage
{
   GLG_AUTO_MAP_SERVER = 0,
   GLG_LOCAL_MAP_SERVER,
   GLG_REMOTE_MAP_SERVER
} GlgMapServerUsage;

typedef enum _GlgGradientType
{
   GLG_NO_GRADIENT = 0,
   GLG_NO_GRADIENT1,     /* Placeholder */
   GLG_LINEAR,
   GLG_INVERSED_LINEAR,
   GLG_LINEAR2,
   GLG_INVERSED_LINEAR2,
   GLG_CONICAL,
   GLG_INVERSED_CONICAL,
   GLG_SPHERICAL,
   GLG_INVERSED_SPHERICAL,
   GLG_ELLIPTICAL,
   GLG_INVERSED_ELLIPTICAL,
   GLG_CONICAL_ABS,
   GLG_INVERSED_CONICAL_ABS,

   GLG_LINE_WIDTH_GRADIENT = 256,
   GLG_INVERSED_LINE_WIDTH_GRADIENT = 768     /* 256 + 512 */
} GlgGradientType;

typedef enum _GlgArrowType
{
   GLG_NO_ARROW = 0,
   GLG_START_ARROW = 17,
   GLG_END_ARROW,
   GLG_START_END_ARROW,
   GLG_MIDDLE_ARROW,
   GLG_MIDDLE_INVERSED_ARROW,
   GLG_START_FILL_ARROW = 33,
   GLG_END_FILL_ARROW,
   GLG_START_END_FILL_ARROW,
   GLG_MIDDLE_FILL_ARROW,
   GLG_MIDDLE_INVERSED_FILL_ARROW,
   GLG_START_FILL_EDGE_ARROW = 49,
   GLG_END_FILL_EDGE_ARROW,
   GLG_START_END_FILL_EDGE_ARROW,
   GLG_MIDDLE_FILL_EDGE_ARROW,
   GLG_MIDDLE_INVERSED_FILL_EDGE_ARROW
} GlgArrowType;

#define GLG_ARROW_POSITION_MASK         0x0f
#define GLG_ARROW_TYPE_MASK             0xf0

/* Arrow type is formed by OR'ing together the arrow's position and fill type 
   flags.
   */

/* Arrow position flags */
#define GLG_START_APOS             1
#define GLG_END_APOS               2
#define GLG_START_END_APOS         3
#define GLG_MIDDLE_APOS            4
#define GLG_MIDDLE_INVERSED_APOS   5

/* Arrow fill type flags */
#define GLG_LINE_ATYPE             0x10
#define GLG_FILL_ATYPE             0x20
#define GLG_FILL_EDGE_ATYPE        0x30

/* Units type for Rounded object. */
typedef enum _GlgUnitsType
{
   GLG_RELATIVE_UNITS = 0,
   GLG_WORLD_UNITS,
   GLG_SCREEN_UNITS
} GlgUnitsType;

/* Subtype of the LineAttributes object. */
typedef enum _GlgLineAttrsType
{
   GLG_LINE_NO_FILL = 0,
   GLG_LINE_AND_FILL

} GlgLineAttrsType;

typedef enum _GlgLayoutType
{
   GLG_UNDEFINED_LAYOUT = 0,
   GLG_ALIGN_LEFT,
   GLG_ALIGN_RIGHT,
   GLG_ALIGN_HCENTER,
   GLG_ALIGN_TOP,
   GLG_ALIGN_BOTTOM,
   GLG_ALIGN_VCENTER,
   GLG_SET_EQUAL_VSIZE,     /* Set same height */
   GLG_SET_EQUAL_HSIZE,     /* Set same width */
   GLG_SET_EQUAL_SIZE,      /* Set same width and height */
   GLG_SET_EQUAL_VDISTANCE, /* Distribute (centers) evenly down */
   GLG_SET_EQUAL_HDISTANCE, /* Distribute (centers) evenly across */
   GLG_SET_EQUAL_VSPACE,    /* Space evenly down */
   GLG_SET_EQUAL_HSPACE,    /* Space evenly across */
   GLG_SET_VSIZE,           /* Set height */
   GLG_SET_HSIZE,           /* Set width */
   GLG_SET_VDISTANCE,       /* Set vertical distance (between centers) */
   GLG_SET_HDISTANCE,       /* Set horizontal distance (between centers) */
   GLG_SET_VSPACE,          /* Set vertical space (between objects) */
   GLG_SET_HSPACE,          /* Set horizontal space (between objects) */
   /* Using the viewport's span extent */
   GLG_POSITION_LEFT,       /* Position at the left edge of the viewport's 
                               span. */
   GLG_POSITION_RIGHT,      /* Position at the right edge of the viewport's 
                               span. */
   GLG_POSITION_HCENTER,    /* Position in the center of the viewport's
                               horizontal span. */
   GLG_POSITION_TOP,        /* Position at the top edge of the viewport's 
                               span. */
   GLG_POSITION_BOTTOM,     /* Position at the bottom edge of the viewport's
                               span. */
   GLG_POSITION_VCENTER,    /* Position in the center of the viewport's
                               vertical span. */
   /* Using the viewport's visible extent */
   GLG_POSITION_LEFT_VIS,   /* Position at the left edge of the viewport. */
   GLG_POSITION_RIGHT_VIS,  /* Position at the right edge of the viewport. */
   GLG_POSITION_HCENTER_VIS,/* Center in the viewport horizontally. */
   GLG_POSITION_TOP_VIS,    /* Position at the top edge of the viewport. */
   GLG_POSITION_BOTTOM_VIS, /* Position at the bottom edge of the viewport. */
   GLG_POSITION_VCENTER_VIS,/* Center in the viewport vertically. */
   GLG_POSITION_CENTER_VIS  /* Center in the viewport both vertically and
                               horizontally. */

} GlgLayoutType;

/* Event types for creating custom event selection messages. */
typedef enum _GlgSelectionEventType
{
   GLG_MOVE_SELECTION  = 8,               /* MouseMove selection */
   GLG_CLICK_SELECTION = 16,              /* MouseClick selection */
   GLG_TOOLTIP_SELECTION = ( 2 | 64 )     /* Tooltip selection */
} GlgSelectionEventType;

/* Viewport's ProcessMouse masks (may be ORed together). */
typedef enum _GlgProcessMouseMask
{
   GLG_NO_MOUSE_EVENTS      =  0, /* Disable all events if no other masks */
   GLG_MOUSE_OVER_SELECTION =  1, /* Highlight and selection on mouse over */
   GLG_MOUSE_OVER_TOOLTIP   =  2, /* Enables tooltips on mouse over when used 
				     together with GLG_MOUSE_OVER_SELECTION. */
   GLG_MOUSE_CLICK          =  4, /* Highlight and selection on mouse click */
   GLG_NAMED_TOOLTIP        =  8  /* Use object names instead of TooltipString
				     resources for tooltips */
} GlgProcessMouseMask;

typedef enum _GlgMBType
{
   GLG_SINGLE_BYTE = 0,
   GLG_MULTI_BYTE,
   GLG_UTF8
} GlgMBType;

typedef enum _GlgXFontType
{
   GLG_X_FONT = 0,
   GLG_XFT_FONT
} GlgXFontType;

typedef enum _GlgLocaleType
{
   GLG_DEFAULT_LOCALE = 0,
   GLG_INHERIT_LOCALE,
   GLG_UTF8_LOCALE,
   GLG_XFT_CHAR8_LOCALE     /* Latin1 */
} GlgLocaleType;

/* Compatibility modes. */
typedef enum _GlgCompatibilityMode
{
   GLG_PRE_2_9 = 0,    /* Name change of the TextColor attribute. */
   GLG_PRE_3_1,        /* UTF-8 flag handling. */
   GLG_PRE_3_5,        /* GIS: ortho clipping and Z coord use for elevation. */
   GLG_LATEST_RELEASE = 0x7ffffff
} GlgCompatibilityMode;

typedef enum _GlgErrorType
{
   GLG_INTERNAL_ERROR = 1,
   GLG_USER_ERROR,
   GLG_WARNING,
   GLG_INFO,

   /* On Windows, this flag can be ORed with the above error types to generate
      an error message asynchronously using a timer.
   */     
   GLG_ASYNC_ERROR = 256

} GlgErrorType;

/* Each log level also logs all messages with smaller log level above it. */
typedef enum _GlgLogLevel
{
   GLG_DISABLE_LOGGING = 0,
   GLG_LOG_INTERNAL_ERRORS = 1,
   GLG_LOG_USER_ERRORS,
   GLG_LOG_WARNINGS,
   GLG_LOG_INFO_MESSAGES,
   GLG_LOG_ALL

} GlgLogLevel;

typedef enum _GlgPreAllocType
{
   GLG_PREALLOC_MEMORY = 0,
   GLG_PREALLOC_POLYGON_POINT_BUFFERS,
   GLG_PREALLOC_GRAPH_POINT_BUFFERS,
   GLG_PREALLOC_TESS_BUFFERS
} GlgPreAllocType;

typedef enum _GlgEventType
{
   GLG_EVENT_TYPE_NONE = GLG_EVENT_BASE,
   GLG_LV_ENTER_NOTIFY,
   GLG_LV_LEAVE_NOTIFY,
   GLG_LV_RESIZE,
   GLG_MSW_ENTER_NOTIFY,   /* MS Windows only */
   GLG_MSW_LEAVE_NOTIFY    /* MS Windows only */
} GlgEventType;

#ifndef GLG_INTERNAL
typedef struct _GlgPoint
{
   double x, y, z;
} GlgPoint;

typedef struct _GlgPoint2
{
   double x, y;
} GlgPoint2;

typedef struct _GlgCube
{
   GlgPoint p1; /* Lower left */
   GlgPoint p2; /* Upper right */
} GlgCube;

typedef struct _GlgRectangle
{
   GlgPoint2 p1; /* Lower left */
   GlgPoint2 p2; /* Upper right */
} GlgRectangle;
#endif

typedef struct _GlgMinMax
{
   double min, max;
} GlgMinMax;

typedef struct _GlgMatrixData
{
   GlgLong type;
   double matrix[ 4 ][ 4 ];
} GlgMatrixData;

/* Rendering mode for CustomDraw callback. */
typedef enum _GlgRenderingMode
{
   GLG_DRAW_GRAPHICS = 0,
   GLG_WINDOWS_PRINT,
   GLG_DRAW_POSTSCRIPT,
   GLG_META_DRAW,
   GLG_DRAW_IMAGE
} GlgRenderingMode;

typedef enum _GlgZoomMode
{
   GLG_DRAWING_ZOOM_MODE = 0,
   GLG_GIS_ZOOM_MODE,
   GLG_CHART_ZOOM_MODE

} GlgZoomMode;

typedef enum _GlgZoomState
{
   GLG_NO_ZOOM_STATE    = 0,
   GLG_X_STATE          = 1,
   GLG_Y_STATE          = 2,
   GLG_PAN_DRAG_STATE   = 4,
   GLG_ZOOM_TO_STATE    = 8

} GlgZoomState;

/*** Deprecated ***/
#define GLG_NO_ZOOM_MODE   GLG_NO_ZOOM_STATE
#define GLG_ZOOM_TO_MODE   ( GLG_ZOOM_TO_STATE | GLG_X_STATE | GLG_Y_STATE )
#define GLG_PAN_DRAG_MODE  ( GLG_PAN_DRAG_STATE | GLG_X_STATE | GLG_Y_STATE )

typedef enum _GlgOpenGLType
{
   GLG_NO_OPENGL = 0,
   GLG_HARDWARE_OPENGL,
   GLG_SOFTWARE_OPENGL
} GlgOpenGLType;

typedef enum _GlgLabelType
{
   GLG_TICK_LABEL_TYPE = 0,
   GLG_SELECTION_LABEL_TYPE
} GlgLabelType;

typedef enum _GlgValueType
{
   GLG_NUMERICAL_VALUE = 0,
   GLG_TIME_VALUE
} GlgValueType;

typedef enum _GlgTriggerType
{
   GLG_NO_TRIGGER = 0,
   GLG_MOUSE_CLICK_TRIGGER,
   GLG_MOUSE_OVER_TRIGGER,
   GLG_INPUT_TRIGGER
} GlgTriggerType;

typedef enum _GlgActionType
{
   GLG_NO_ACTION = 0,
   GLG_TOOLTIP_ACTION,
   GLG_SEND_COMMAND_ACTION,
   GLG_SEND_EVENT_ACTION,
   GLG_TRACE_STATE_ACTION,
   GLG_SET_STATE_ACTION,
   GLG_RESET_STATE_ACTION,
   GLG_TOGGLE_STATE_ACTION
} GlgActionType;

/* Choices of the Action object's ProcessArmed attribute. */
typedef enum _GlgProcessArmedType
{
   GLG_ARMED_NONE = 0,
   GLG_ARMED_ONLY,
   GLG_UNARMED_ONLY,
   GLG_ARMED_AND_UNARMED
} GlgProcessArmedType;

/*** Deprecated ***/
#define GLG_ARMED_AND_DISARMED GLG_ARMED_AND_UNARMED

/* Choices of the armed_state parameter of the GlgGetAction() method. */
typedef enum _GlgArmedStateType
{
   GLG_ANY_ARMED_STATE = 0,
   GLG_ARMED_ONLY_STATE,
   GLG_UNARMED_ONLY_STATE,
   GLG_SKIP_ARMED_ONLY_STATE,
   GLG_SKIP_UNARMED_ONLY_STATE
} GlgArmedStateType;

/* Choices of the Action object's ProcessDoubleClick attribute. */
typedef enum _GlgProcessDoubleClickType
{
   GLG_DOUBLE_CLICK_NONE = 0,
   GLG_DOUBLE_CLICK_ONLY,
   GLG_SIGNLE_CLICK_ONLY,
   GLG_SINGLE_AND_DOUBLE_CLICK
} GlgProcessDoubleClickType;

/* Choices of the double_click_state parameter of the GlgGetAction() method. */
typedef enum _GlgDoubleClickStateType
{
   GLG_ANY_DOUBLE_CLICK_STATE = 0,
   GLG_DOUBLE_CLICK_ONLY_STATE,
   GLG_SINGLE_CLICK_ONLY_STATE,
   GLG_SKIP_DOUBLE_CLICK_ONLY_STATE,
   GLG_SKIP_SINGLE_CLICK_ONLY_STATE,
} GlgDoubleClickStateType;

/* Assign a new object value to the object_holder, dereferencing the 
   old value and referencing the new one. */
#define GlgSetObjectMacro( object_holder, new_value ) \
  ( (object_holder) == (new_value) ? (object_holder) : \
   ( GlgDropObject( object_holder ), \
    (object_holder) = GlgReferenceObject( new_value ) ) )

/* Assigns a NULL object to the object holder and dereferences the old 
   value. */
#define GlgNullObjectMacro( object_holder ) \
  ( !(object_holder) ? (GlgObject)0 : \
   ( GlgDropObject( object_holder ), object_holder = (GlgObject)0 ) )

/* Initializes structure with 0s. */
#define GlgInitStruct( ptr, size )  memset( ptr, 0, size )

/* Allocates a structure and initializes it with 0s.  */
#define GlgAllocStruct( size )   GlgInitStruct( GlgAlloc( size ), size )

/************ Glg Typedefs *******************************************/
#ifndef GLG_INTERNAL
typedef void * GlgObject;
#endif
typedef GlgLong GlgBoolean;
typedef void * GlgAnyType;

#ifdef X_WINDOWS

typedef XtAppContext GlgAppContext;
typedef XtIntervalId GlgIntervalID;
typedef XtCallbackProc GlgCallbackProc;
typedef XtWorkProc GlgWorkProc;
typedef XtTimerCallbackProc GlgTimerProc;

typedef GlgLong (*GlgAddTimerFunc)( GlgLong, GlgTimerProc, void * );
typedef void (*GlgRemoveTimerFunc)( GlgLong );
typedef void (*GlgDrawTooltipFunc)( CCONST char *, GlgObject, GlgObject,
                                    GlgLong, GlgLong, GlgLong, GlgLong );

#else

typedef HANDLE GlgAppContext;
typedef GlgLong GlgIntervalID;
typedef void (*GlgCallbackProc)( GlgObject, GlgAnyType, GlgAnyType );
typedef GlgBoolean (*GlgWorkProc)( GlgAnyType );
typedef void (*GlgTimerProc)( GlgAnyType, GlgIntervalID * );

#define False  0
#define True   1

#endif

/* Reason of the chart filter callback. */
typedef enum _GlgChartFilterCBReason
{
   GLG_FILTER_CB_START = 1,  /* Invoked before drawing the plot data. */
   GLG_FILTER_CB_ADD_SAMPLE, /* Invoked to add one sample to the filter. */
   GLG_FILTER_CB_FLUSH,      /* Invoked to flush accumulated data. */
   GLG_FILTER_CB_FINISH,     /* Invoked after the plot drawing has finished. */
  GLG_FILTER_CB_DESTROY     /* Invoked when the plot is destroyed. */

} GlgChartFilterCBReason;

#ifndef GLG_INTERNAL
typedef struct _GlgChartFilterData GlgChartFilterData;
#endif

typedef GlgLong (*GlgChartFilter)( GlgChartFilterCBReason, 
                                   GlgChartFilterData *, void * );

#define GLG_CHART_FILTER_VERSION     2

typedef enum _GlgExtDataType
{
   GLG_NO_EXT_DATA = 0,
   GLG_MARKER_EXT_DATA,
   GLG_BAR_EXT_DATA
} GlgExtDataType;

#ifndef GLG_INTERNAL
/* Chart data sample structure */
typedef struct _GlgDataSample
{
   double value;
   double time;
   float marker_vis;

   unsigned valid : 1;
   unsigned reserved : 1;
   /* True if extended data are supplied and the data sample is an instance of 
      GlgDataSampleExt.
   */
   unsigned extended_data : 1;

} GlgDataSample;

/* Chart extended data sample structure */
typedef struct _GlgDataSampleExt
{
   double value;
   double time;
   float marker_vis;

   unsigned valid : 1;
   unsigned reserved : 1;
   unsigned extended_data : 1;

   double y_low;
   unsigned int fill_color;
   unsigned int edge_color;
   GlgExtDataType saved_type;    /* Marker or bar */
   unsigned char drawn_size;
   unsigned char drawn_type;
   unsigned char line_width;   

} GlgDataSampleExt;

/* Data passed to the chart filter callback. */
struct _GlgChartFilterData
{
   GlgChartFilterType filter_type; /* FilterType (read-only) */
   GlgLong total_points;           /* Total number of points, 
                                      valid and non-valid (read-only) */
   GlgLong num_accum;              /* Number of accumulated points. */

   double filter_interval;         /* Filter interval (read-only) */
   double last_added_x;            /* Last added x (read-only) */
   double last_drawn_x;            /* Last drawn x (read-only) */

   double accum_x;                 /* Accumulated x value. */
   double accum_y;                 /* Accumulated y value. */
   double accum_y_low;             /* Accumulated y_low value for a floating 
                                      bar chart. */
   double min_x;                   /* Stored value for MIN_MAX filter. */
   double min_y;                   /* Stored value for MIN_MAX filter. */
   double max_x;                   /* Stored value for MIN_MAX filter. */
   double max_y;                   /* Stored value for MIN_MAX filter. */

   double x1, y1, y1_low;  /* Output filter values for the first point. */
   double x2, y2;          /* Output filter values for the second point. */

   float marker_vis_acc;   /* Stored marker visibility. */
   float marker_vis_min;   /* Stored marker visibility. */
   float marker_vis_max;   /* Stored marker visibility. */
   float marker_vis1;      /* Output marker visibility for the first point. */
   float marker_vis2;      /* Output marker visibility for the second point. */

   GlgDataSample * data_sample_min;   /* Stored min data sample. */
   GlgDataSample * data_sample_max;   /* Stored max data sample. */
   GlgDataSample * data_sample1;  /* Output data sample for the first point. */
   GlgDataSample * data_sample2;  /* Output data sample for the second point. */
   
   unsigned draw_markers : 1;      /* True if plot has markers (read-only). */
   unsigned y1_low_set : 1;        /* True if y1_low value is usable. */
   unsigned disable_attributes : 1;/* If True, disables the use of colors and 
                                      other graphical attributes for filters 
                                      where it does not make sense (such as 
                                      AVERAGE). */
   GlgObject plot;                 /* Plot object the filter is attached to. */
   GlgDataSample * data_sample;    /* The data sample to be added 
                                      (for GLG_FILTER_CB_ADD_SAMPLE only). */
   GlgChartFilter custom_filter;   /* The custom filter (read-only) */
   void * custom_filter_data;      /* Filter's data passed via the data 
                                      parameter of GlgSetChartFilter(). */
};
#endif

typedef enum _GlgObjectMatchType
{
   GLG_UNDEFINED_MATCH_TYPE = 0,
   GLG_OBJECT_TYPE_MATCH    = 1,
   GLG_OBJECT_NAME_MATCH    = 2,
   GLG_RESOURCE_MATCH       = 4,
   GLG_OBJECT_ID_MATCH      = 8,
   GLG_CUSTOM_MATCH         = 16
} GlgObjectMatchType;

typedef struct _GlgFindMatchingObjectsData
{
   GlgObjectMatchType match_type;
   GlgBoolean find_parents;
   GlgBoolean find_first_match;
   GlgBoolean search_inside;
   GlgBoolean search_drawable_only;
   GlgBoolean (*custom_match)( GlgObject object, void * custom_data );
   void * custom_data;
   GlgObjectType object_type;
   char * object_name;
   char * resource_name;
   GlgObject object_id;
   GlgObject found_object;
   GlgBoolean found_multiple;

} GlgFindMatchingObjectsData;

typedef char * (*GlgLabelFormatter)( GlgObject, GlgLong, GlgLong,
                                     double, time_t, double, void * );
typedef char * (*GlgTooltipFormatter)( GlgObject viewport, GlgObject object, 
                                       GlgObject tooltip_obj, 
                                       GlgLong root_x, GlgLong root_y );
typedef void (*GlgAlarmHandler)( GlgObject, GlgObject, 
                                 CCONST char *, CCONST char *, CCONST char*, 
                                 GlgAnyType );
typedef GlgBoolean
   (*GlgGetMapCallback)( GlgObject gis_obj, GlgLong width, GlgLong height, 
                         GlgLong clip, GlgLong clip_min_x, GlgLong clip_max_x,
                         GlgLong clip_min_y, GlgLong clip_max_y, 
                         CCONST char * url, CCONST char * dataset_file, CCONST char * layers,
                         double center_x, double center_y, 
                         double extent_x, double extent_y, double angle,
                         GlgLong projection, GlgLong stretch, 
                         GlgLong verbocity, GlgPoint * color, 
                         void * reserved, void ** file_name, GlgLong * format );
typedef GlgBoolean
   (*GlgGetMapDataCallback)( GlgObject gis_obj, CCONST char * url, 
                             CCONST char * dataset_file, CCONST char * layer, 
                             GlgLong verbocity, void * reserved,
                             double lon, double lat, 
                             CCONST char * data_name, double * data );

typedef GlgBoolean
   (*GlgGetMapSelectionCallback)( GlgObject gis_obj, 
                                  GlgLong width, GlgLong height, 
                                  CCONST char * url, CCONST char * dataset_file, 
                                  CCONST char * layers,
                                  double center_x, double center_y, 
                                  double extent_x, double extent_y, 
                                  double angle, GlgLong projection, 
                                  GlgLong stretch, GlgLong verbocity, 
                                  void * reserved,
                                  GlgLong x, GlgLong y, 
                                  GlgLong pick_resolution, 
                                  GlgLong select_labels,
                                  GlgObject * selection );

typedef struct _GlgGISCallbacks
{
   GlgGetMapCallback get_map_cb;
   GlgGetMapDataCallback get_map_data_cb;
   GlgGetMapSelectionCallback get_map_selection_cb;
} GlgGISCallbacks;

typedef char * (*GlgGetURLCallback)( CCONST char *, void * );

typedef struct _GlgTraceCBStruct
{
   GlgCallbackType reason;         /* GLG_TRACE_CB or GLG_TRACE2_CB */
   GlgObject viewport;             /* Viewport that received the event. */
   GlgObject light_viewport;       /* Light viewport of the event or NULL. */
#ifndef MS_WINDOWS
   XEvent * event;                 /* Event that triggered the callback. */
   Widget widget;                  /* Widget ID of the event viewport. */
   Window window;                  /* Window ID of the event viewport. */
   Display * display;              /* Display of the event viewport. */
   XEvent * event1;
#else
   MSG * event;                    /* Event that triggered the callback. */
   HWND widget;
   HWND window;                    /* Window ID of the event viewport. */
   void * display;
   void * event1;
#endif
} GlgTraceCBStruct;

typedef struct _GlgHierarchyCBStruct
{
   GlgCallbackType reason;               /* GLG_HIERARCHY_CB */
   GlgHierarchyCallbackType condition;   /* GLG_BEFORE_SETUP_CB or 
                                            GLG_AFTER_SETUP_CB */
   GlgObject object;         /* Subwindow or subdrawing object that loaded 
                                its template. */
   GlgObject subobject;      /* Template instance of the subwindow or 
                                subdrawing. */   
} GlgHierarchyCBStruct;

typedef struct _GlgCustomDrawCBStruct
{
   GlgCallbackType reason;   /* GLG_CUSTOM_DRAW_CB */
   GlgObject viewport;       /* Viewport the CustomDraw callback is 
                                attached to. */
#ifndef MS_WINDOWS
   Widget widget;            /* Widget ID of the viewport. */
   Window window;            /* Window ID of the viewport. */   
   Display * display;        /* Display of the viewport. */
   Colormap colormap;        /* Colormap for Xlib mode */
   GC gc;                    /* GC used to render viewport's graphics. Content 
				must be restored if changed during drawing. */
   void * dc;
   Drawable drawable;        /* Drawable the graphics is rendered into in 
				non-OpenGL graphics mode. */
   void * xft_drawable;      /* Xft drawable for drawing XFT fonts. */
   Region region;            /* Clip region if region_mode=True, may be NULL.*/
#else
   HWND widget;
   HWND window;              /* Window ID of the viewport. */
   void * display;
   unsigned long colormap;
   void * gc;
   HDC dc;                   /* DC used to render viewport's graphics. Content 
				must be restored if changed during drawing. */
   HWND drawable;
   HRGN region;              /* Clip region in screen coordinates if 
				region_mode=True, may be NULL when drawing 
				all. */
#endif
   GlgBoolean region_mode;   /* True is clipping is done via Regions, otherwise
				clipping is done using bounding box. Is always
				False in OpenGL mode. */
   GlgRectangle * box;       /* Clip box in screen coordinates, always 
				provided, even if region_mode=False. 
				May be NULL when drawing the whole window. */
   GlgLong width;            /* Drawing width. */
   GlgLong height;           /* Drawing height. */
   GlgRenderingMode mode;    /* Rendering mode: GLG_DRAW_GRAPHICS, 
				GLG_DRAW_IMAGE, GLG_WINDOWS_PRINT, 
				GLG_DRAW_POSTSCRIPT. */
   GlgBoolean open_gl;       /* If True, OpenGL must be used instead of native
				graphics primitives for modes that render
				graphics. */
   GlgBoolean anti_aliasing; /* Viewport anti-aliasing for OpenGL mode */
   GlgBoolean straight_segments;
   GlgBoolean last_pixel;
   GlgBoolean vertex_pixels;
   FILE * ps_file;           /* PostScript file for GLG_DRAW_POSTSCRIPT mode */
   
} GlgCustomDrawCBStruct;

/************ GLG Function Prototypes *******************************/

typedef void (*GlgErrorHandler)( CCONST char *, GlgErrorType );

GlgImport( GlgAppContext ) GlgInit( GlgBoolean tk_initialized, 
                                    GlgAppContext app_context, 
                                    int argc, char ** argv );
GlgImport( GlgBoolean ) GlgInitLocale( CCONST char * locale );
GlgImport( void ) GlgTerminate( void );
GlgImport( GlgErrorHandler ) GlgSetErrorHandler( GlgErrorHandler new_handler );
GlgImport( GlgObject ) GlgLoadWidgetFromFile( CCONST char * filename );
GlgImport( GlgObject ) GlgLoadWidgetFromImage( void * image, GlgLong size );
GlgImport( GlgObject ) GlgLoadWidgetFromObject( GlgObject object );
GlgImport( GlgObject ) GlgLoadObject( CCONST char * filename );
GlgImport( GlgObject ) GlgLoadObjectFromImage( void * image, GlgLong size );
GlgImport( void ) GlgInitialDraw( GlgObject object );
GlgImport( void ) GlgSetupHierarchy( GlgObject object );
GlgImport( void ) GlgResetHierarchy( GlgObject object );

GlgImport( GlgBoolean ) GlgUpdate( GlgObject object );
GlgImport( GlgBoolean ) GlgReset( GlgObject object );
GlgImport( GlgBoolean ) GlgPrint( GlgObject object, CCONST char * file,
                                  double x, double y, 
                                  double width, double height,
                                  GlgBoolean portrait, GlgBoolean stretch );
#ifdef MS_WINDOWS
GlgImport( GlgBoolean ) GlgWinPrint( GlgObject object, HDC print_dc,
                                     double x, double y, 
                                     double width, double height,
                                     GlgBoolean reserved, GlgBoolean stretch );
GlgImport( GlgBoolean ) GlgOnPrint( GlgObject object, HDC print_dc );
GlgImport( GlgBoolean ) GlgOnDrawMetafile( GlgObject object, HDC print_dc );
#endif
GlgImport( GlgBoolean ) GlgSaveImage( GlgObject object, CCONST char * res_name, 
                                      CCONST char * filename, 
                                      GlgImageFormat format );
GlgImport( GlgBoolean ) GlgSaveImageCustom( GlgObject object, 
                                            CCONST char * res_name, 
                                            CCONST char * filename, 
                                            GlgImageFormat format,
                                            GlgLong x, GlgLong y, 
                                            GlgLong width, GlgLong height, 
                                            GlgLong gap );
#ifdef MS_WINDOWS
GlgImport( HBITMAP ) GlgGenerateImage( GlgObject object, CCONST char * res_name, 
                                       HBITMAP bitmap );
GlgImport( HBITMAP ) GlgGenerateImageCustom( GlgObject object, 
                                             CCONST char * res_name,
                                             HBITMAP bitmap, 
                                             GlgLong x, GlgLong y, 
                                             GlgLong width, GlgLong height, 
                                             GlgLong gap );
#else
GlgImport( Pixmap ) GlgGenerateImage( GlgObject object, CCONST char * res_name, 
                                      Pixmap pixmap );
GlgImport( Pixmap ) GlgGenerateImageCustom( GlgObject object, 
                                            CCONST char * res_name, 
                                            Pixmap pixmap, 
                                            GlgLong x, GlgLong y, 
                                            GlgLong width, GlgLong height, 
                                            GlgLong gap );
#endif
GlgImport( void ) GlgMetaDraw( GlgObject object, CCONST char * file,
                               double x, double y, double width, double height,
                               GlgBoolean portrait, GlgBoolean stretch );
GlgImport( void ) GlgFree( void * ptr );
GlgImport( char * ) GlgStrClone( CCONST char * string );
GlgImport( char * ) GlgConcatStrings( CCONST char * string1, 
                                      CCONST char * string2 );
GlgImport( char * ) GlgCreateIndexedName( CCONST char * template_name, 
                                          GlgLong resource_index );
GlgImport( char * ) GlgConcatResNames( CCONST char * res_name1, 
                                       CCONST char * res_name2 ); 
GlgImport( GlgBoolean ) GlgSetLResource( GlgObject object, 
                                         CCONST char * res_name, GlgLong value );
GlgImport( GlgBoolean ) GlgGetLResource( GlgObject object, 
                                         CCONST char * res_name, 
                                         GlgLong * value );
GlgImport( GlgBoolean ) GlgSetDResource( GlgObject object, 
                                         CCONST char * res_name, 
                                         double value );
GlgImport( GlgBoolean ) GlgSetDResourceIf( GlgObject object, 
                                           CCONST char * res_name, double value, 
                                           GlgBoolean if_changed );
GlgImport( GlgBoolean ) GlgGetDResource( GlgObject object, 
                                         CCONST char * res_name, 
                                         double * value );
GlgImport( GlgBoolean ) GlgSetSResource( GlgObject object, 
                                         CCONST char * res_name, 
                                         CCONST char * value );
GlgImport( GlgBoolean ) GlgSetSResourceIf( GlgObject object, 
                                           CCONST char * res_name, 
                                           CCONST char * value, 
                                           GlgBoolean if_changed );
GlgImport( GlgBoolean ) GlgGetSResource( GlgObject object, 
                                         CCONST char * res_name, 
                                         CCONST char ** value );
GlgImport( GlgBoolean ) GlgSetGResource( GlgObject object, 
                                         CCONST char * res_name,
                                         double x_value, double y_value, 
                                         double z_value );
GlgImport( GlgBoolean ) GlgSetGResourceIf( GlgObject object, 
                                           CCONST char * res_name,
                                           double x_value, double y_value, 
                                           double z_value, 
                                           GlgBoolean if_changed );
GlgImport( GlgBoolean ) GlgGetGResource( GlgObject object, 
                                         CCONST char * res_name,
                                         double * x_value, double * y_value,
                                         double * z_value );
GlgImport( GlgBoolean ) GlgSetSResourceFromD( GlgObject object,
                                              CCONST char * res_name, 
                                              CCONST char * format,
                                              double value );
GlgImport( GlgBoolean ) GlgSetSResourceFromDIf( GlgObject object, 
                                                CCONST char * res_name, 
                                                CCONST char * format,
                                                double value, 
                                                GlgBoolean if_changed );
GlgImport( GlgBoolean ) GlgSetResourceFromObject( GlgObject object, 
                                                  CCONST char * res_name, 
                                                  GlgObject from_value );
GlgImport( GlgBoolean ) GlgSetResourceFromObjectIf( GlgObject object, 
                                                    CCONST char * res_name, 
                                                    GlgObject from_value,
                                                    GlgBoolean if_changed );
GlgImport( GlgBoolean ) GlgHasResourceObject( GlgObject object, 
                                              CCONST char * res_name );
GlgImport( GlgBoolean ) GlgSetDTag( GlgObject object, CCONST char * tag_name, 
                                    double value, GlgBoolean if_changed );
GlgImport( GlgBoolean ) GlgGetDTag( GlgObject object, CCONST char * tag_name, 
                                    double * value );
GlgImport( GlgBoolean ) GlgSetSTag( GlgObject object, CCONST char * tag_name, 
                                    CCONST char * value, GlgBoolean if_changed );
GlgImport( GlgBoolean ) GlgGetSTag( GlgObject object, CCONST char * tag_name, 
                                    CCONST char ** value );
GlgImport( GlgBoolean ) GlgSetGTag( GlgObject object, CCONST char * tag_name, 
                                    double x_value, double y_value, 
                                    double z_value, GlgBoolean if_changed );
GlgImport( GlgBoolean ) GlgGetGTag( GlgObject object, CCONST char * tag_name,
                                    double * x_value, double * y_value, 
                                    double * z_value );
GlgImport( GlgBoolean ) GlgSetSTagFromD( GlgObject object, 
                                         CCONST char * tag_name, 
                                         CCONST char * format, double value, 
                                         GlgBoolean if_changed );
GlgImport( GlgBoolean ) GlgHasTagSource( GlgObject object, 
                                         CCONST char * tag_source );
GlgImport( GlgBoolean ) GlgHasTagName( GlgObject object, 
                                       CCONST char * tag_name );
GlgImport( GlgObject ) GlgCreateTagList( GlgObject object, 
                                         GlgBoolean unique_tag_sources );
GlgImport( GlgObject ) GlgCreateAlarmList( GlgObject object );
GlgImport( GlgLong ) GlgGetSelectionButton( void );
GlgImport( GlgBoolean ) GlgGetModifierState( GlgModifierType modifier );
GlgImport( GlgObject ) GlgGetSelectedPlot( void );
GlgImport( GlgBoolean ) GlgSetZoom( GlgObject object, CCONST char * res_name, 
                                    GlgLong type, double value );
/*** Deprecated, use GlgSetZoomMode(). ***/
GlgImport( GlgBoolean ) GlgSetGISZoom( GlgObject object, CCONST char * res_name, 
                                       GlgObject gis_object, 
                                       CCONST char * gis_res_name );
GlgImport( GlgBoolean ) GlgSetZoomMode( GlgObject object, 
                                        CCONST char * res_name, 
                                        GlgObject zoom_object, 
                                        CCONST char * zoom_object_res_name,
                                        GlgZoomMode mode );
GlgImport( GlgObject ) GlgCreateSelectionNames( GlgObject top_vp, 
                                                GlgRectangle * rectangle,
                                                GlgObject selected_vp );
GlgImport( GlgObject ) GlgCreateSelectionMessage( GlgObject top_vp, 
                                                  GlgRectangle * rectangle, 
                                                  GlgObject selected_vp,
                                                  GlgSelectionEventType selection_type, 
                                                  GlgLong button );
GlgImport( GlgLong ) GlgExportStrings( GlgObject object, CCONST char * filename,
                                       GlgLong separator1, GlgLong separator2 );
GlgImport( GlgLong ) GlgImportStrings( GlgObject object, CCONST char * filename, 
                                       GlgBoolean verbose );
GlgImport( GlgLong ) GlgExportTags( GlgObject object, CCONST char * filename,
                                    GlgLong separator1, GlgLong separator2 );
GlgImport( GlgLong ) GlgImportTags( GlgObject object, CCONST char * filename, 
                                    GlgBoolean verbose );
GlgImport( GlgAnyType ) GlgSendMessage( GlgObject object, CCONST char * res_name,
                                        CCONST char * message,
                                        GlgAnyType param1, GlgAnyType param2,
                                        GlgAnyType param3, GlgAnyType param4 );
GlgImport( GlgObject ) GlgReferenceObject( GlgObject object );
GlgImport( void ) GlgDropObject( GlgObject object );
GlgImport( void ) GlgChangeObject( GlgObject object, CCONST char * res_name );
GlgImport( void ) GlgInvalidateViewport( GlgObject object, 
                                         CCONST char * res_name, 
                                         GlgLong type, GlgLong reserved );
GlgImport( GlgLong ) GlgGetSize( GlgObject container );
GlgImport( GlgObject ) GlgGetElement( GlgObject container, GlgLong pos_index );
GlgImport( GlgObject ) GlgGetNamedPlot( GlgObject object, 
                                        CCONST char * res_name, 
                                        CCONST char * plot_name );
GlgImport( GlgObject ) GlgAddPlot( GlgObject object, CCONST char * res_name,
                                   GlgObject plot );
GlgImport( GlgBoolean ) GlgDeletePlot( GlgObject object, 
                                       CCONST char * res_name, 
                                       GlgObject plot );
GlgImport( GlgObject ) GlgAddTimeLine( GlgObject object, 
                                       CCONST char * res_name, 
                                       GlgObject time_line, double time_stamp );
GlgImport( GlgBoolean ) GlgDeleteTimeLine( GlgObject object, 
                                           CCONST char * res_name, 
                                           GlgObject time_line,
                                           double time_stamp );
GlgImport( GlgBoolean ) GlgClearDataBuffer( GlgObject object, 
                                            CCONST char * res_name );
GlgImport( GlgBoolean ) GlgSetLinkedAxis( GlgObject object, 
                                          CCONST char * res_name, 
                                          GlgObject axis,
                                          CCONST char * axis_res_name );
GlgImport( GlgBoolean ) GlgGetDataExtent( GlgObject object, 
                                          CCONST char * res_name, 
                                          GlgMinMax * min_max, 
                                          GlgBoolean pan_x );
GlgImport( GlgBoolean ) GlgSetLabelFormatter( GlgObject object, 
                                              CCONST char * res_name,
                                              GlgLabelFormatter formatter );
GlgImport( GlgTooltipFormatter ) GlgSetTooltipFormatter( GlgTooltipFormatter formatter);
GlgImport( GlgBoolean ) GlgSetChartFilter( GlgObject object, 
                                           CCONST char * res_name, 
                                           GlgChartFilter chart_filter, 
                                           void * data );
GlgImport( GlgAnyType ) GlgGetNativeComponent( GlgObject object, 
                                               CCONST char * res_name,
                                               GlgComponentQueryType type );

/******* Function prototypes for the generic cross-platform API ******/

GlgImport( void ) GlgSetDefaultViewport( GlgObject viewport );
GlgImport( GlgLong ) GlgMainLoop( GlgAppContext context );
GlgImport( void ) GlgAddCallback( GlgObject viewport, GlgCallbackType type,
                                  GlgCallbackProc callback, GlgAnyType data );
GlgImport( GlgAlarmHandler ) GlgSetAlarmHandler( GlgAlarmHandler handler );
GlgImport( GlgLong ) GlgAddWorkProc( GlgAppContext context, GlgWorkProc proc,
					       GlgAnyType client_data );
GlgImport( void ) GlgRemoveWorkProc( GlgLong id );
GlgImport( GlgLong ) GlgAddTimeOut( GlgAppContext context, GlgLong interval,
                                    GlgTimerProc proc, GlgAnyType client_data );
GlgImport( void ) GlgRemoveTimeOut( GlgLong id );
GlgImport( GlgBoolean  ) GlgGetTime( GlgULong * sec, GlgULong * microsec );
GlgImport( void ) GlgSleep( GlgLong millisec );
GlgImport( void ) GlgBell( GlgObject viewport );
GlgImport( double ) GlgRand( double low, double high );
GlgImport( GlgBoolean ) GlgSync( GlgObject object );

/************* Function prototypes for ICC API *************/
GlgImport( GlgObject ) GlgCreateLink( CCONST char * display_name, 
                                      CCONST char * comm_name );
GlgImport( GlgBoolean ) GlgLinkActive( GlgObject link );
GlgImport( void ) GlgDestroyLink( GlgObject link );

/********** Function prototypes for Map Server support functions **********/
GlgImport( GlgBoolean ) GlgGISConvert( GlgObject gis_object, 
                                       CCONST char * res_name, 
                                       GlgCoordType corrd_type, 
                                       GlgBoolean coord_to_lat_lon, 
                                       GlgPoint * in_point, 
                                       GlgPoint * out_point);
GlgImport( void ) GlmConvert( GlgProjectionType projection, GlgBoolean stretch,
                              GlgCoordType coord_type, 
                              GlgBoolean coord_to_lat_lon,
                              GlgPoint * center, GlgPoint * extent, 
                              double angle,
                              double min_x, double max_x, 
                              double min_y, double max_y, 
                              GlgPoint * in_point, GlgPoint * out_point );
GlgImport( GlgBoolean ) GlgGISGetElevation( GlgObject gis_object, 
                                            CCONST char * res_name, 
                                            CCONST char * layer_name, 
                                            double lon, double lat, 
                                            double * elevation ); 
GlgImport( GlgObject ) GlgGISCreateSelection( GlgObject gis_object, 
                                              CCONST char * res_name,
                                              CCONST char * layers, 
                                              GlgLong x, GlgLong y, 
                                              GlgLong select_labels ); 
GlgImport( GlgObject ) GlgGISGetDataset( GlgObject gis_object, 
                                         CCONST char * res_name );

/************ Fast Access Functions *********************************/
GlgImport( GlgObjectType ) GlgGetObjectType( GlgObject object );
GlgImport( CCONST char * ) GlgGetObjectName( GlgObject object );
GlgImport( GlgDataType ) GlgGetDataType( GlgObject data );

/************ Function prototypes for the Intermediate and Extended APIs ******/
#if !defined GLG_INTERNAL || defined GLG_INTERNAL_EXT_PROTO
GlgImport( GlgObject ) GlgCreateObject( GlgObjectType type, CCONST char * name,
                                        GlgAnyType param1, GlgAnyType param2,
                                        GlgAnyType param3, GlgAnyType param4 );
GlgImport( GlgObject ) GlgCopyObject( GlgObject object );
GlgImport( GlgObject ) GlgCloneObject( GlgObject object, GlgCloneType type );
GlgImport( GlgObject ) GlgConvertViewportType( GlgObject object );
GlgImport( GlgBoolean ) GlgSaveObject( GlgObject object, 
                                       CCONST char * filename );
GlgImport( GlgBoolean ) GlgAddObjectToTop( GlgObject container, 
                                           GlgObject object );
GlgImport( GlgBoolean ) GlgAddObjectToBottom( GlgObject container, 
                                              GlgObject object );
GlgImport( GlgBoolean ) GlgAddObjectAt( GlgObject container, GlgObject object,
                                        GlgLong pos_index );
GlgImport( GlgBoolean ) GlgAddObject( GlgObject container, GlgAnyType object,
                                      GlgAccessType access_type,
                                      GlgPositionModifier pos_modifier );
GlgImport( GlgBoolean ) GlgDeleteTopObject( GlgObject container );
GlgImport( GlgBoolean ) GlgDeleteBottomObject( GlgObject container );
GlgImport( GlgBoolean ) GlgDeleteThisObject( GlgObject container, 
                                             GlgObject object );
GlgImport( GlgBoolean ) GlgDeleteObjectAt( GlgObject container, 
                                           GlgLong pos_index );
GlgImport( GlgBoolean ) GlgDeleteObject( GlgObject container, GlgAnyType object,
                                         GlgAccessType access_type,
                                         GlgPositionModifier pos_modifier );
GlgImport( GlgBoolean ) GlgContainsObject( GlgObject container, 
                                           GlgObject object );
GlgImport( GlgObject ) GlgGetNamedObject( GlgObject container, 
                                          CCONST char * name );
GlgImport( GlgBoolean ) GlgSetElement( GlgObject container, GlgLong pos_index, 
                                       GlgObject new_object );
GlgImport( GlgLong ) GlgGetIndex( GlgObject container, GlgObject object );
GlgImport( GlgLong ) GlgGetStringIndex( GlgObject container, 
                                        CCONST char * string );
GlgImport( GlgObject ) GlgFindObject( GlgObject container, GlgAnyType object,
                                      GlgSearchType search_type, 
                                      GlgAnyType reserved );
GlgImport( void ) GlgSetStart( GlgObject container );
GlgImport( GlgObject ) GlgIterate( GlgObject container );
GlgImport( GlgBoolean ) GlgReorderElement( GlgObject container, 
                                           GlgLong current_index,
                                           GlgLong new_index );
GlgImport( void ) GlgFlush( GlgObject container, GlgLong size );
GlgImport( void ) GlgInverse( GlgObject container );
GlgImport( GlgBoolean ) GlgSetXform( GlgObject object, GlgObject xform );
GlgImport( GlgObject ) GlgGetResourceObject( GlgObject object, 
                                             CCONST char * res_name );
GlgImport( GlgBoolean ) GlgSetResourceObject( GlgObject object, 
                                              CCONST char * res_name,
                                              GlgObject value );
GlgImport( GlgObject ) GlgGetTagObject( GlgObject object, 
                                        CCONST char * search_string, 
                                        GlgBoolean by_name, 
                                        GlgBoolean unique_tags, 
                                        GlgBoolean single_tag, 
                                        GlgTagType tag_type_mask );
GlgImport( GlgBoolean ) GlgHasTag( GlgObject object, 
                                   CCONST char * tag_name,
                                   GlgTagType tag_type_mask );
GlgImport( GlgObject ) GlgQueryTags( GlgObject object, 
                                     GlgTagType tag_type_mask );
GlgImport( GlgObject ) GlgGetAlarmObject( GlgObject object, 
                                          CCONST char * alarm_label, 
                                          GlgBoolean single_alarm, 
                                          GlgLong reserved );
GlgImport( GlgBoolean ) GlgConstrainObject( GlgObject from_attribute, 
                                            GlgObject to_object );
GlgImport( GlgBoolean ) GlgUnconstrainObject( GlgObject attribute );
GlgImport( GlgObject ) GlgSuspendObject( GlgObject object );
GlgImport( void ) GlgReleaseObject( GlgObject object, GlgObject suspend_info );
GlgImport( GlgObject ) GlgGetParent( GlgObject object, GlgLong * num_parents );
GlgImport( GlgBoolean ) GlgFindMatchingObjects( GlgObject object, 
                                                GlgFindMatchingObjectsData * data );
GlgImport( void ) GlgTraverseObjects( GlgObject object, 
                                      GlgBoolean (*func)( GlgObject object,
                                                          void * data ),
                                      void * data );
GlgImport( GlgBoolean ) GlgIsDrawable( GlgObject object );

GlgImport( GlgCube * ) GlgGetBoxPtr( GlgObject object );
GlgImport( GlgObject ) GlgGetDrawingMatrix( GlgObject object );
GlgImport( GlgObject ) GlgCreateInversedMatrix( GlgObject object );
GlgImport( void ) GlgTransformPoint( GlgObject matrix, GlgPoint * in_point, 
                                     GlgPoint * out_point );
GlgImport( GlgObject ) GlgCreateSelection( GlgObject top_vp, 
                                           GlgRectangle * rectangle,
                                           GlgObject selected_vp );
GlgImport( GlgObject ) GlgCreateResourceList( GlgObject object, 
                                              GlgBoolean list_named_res, 
                                              GlgBoolean list_def_attr,
                                              GlgBoolean list_aliases );
GlgImport( GlgBoolean ) GlgTransformObject( GlgObject object, GlgObject xform,
                                            GlgCoordType coord_type, 
                                            GlgObject parent );
GlgImport( GlgObject ) GlgCreatePointArray( GlgObject object, 
                                            GlgControlPointType type );
GlgImport( GlgBoolean ) GlgMoveObjectBy( GlgObject object, 
                                         GlgCoordType coord_type,
                                         double x, double y, double z );
GlgImport( GlgBoolean ) GlgMoveObject( GlgObject object, 
                                       GlgCoordType coord_type, 
                                       GlgPoint * start_point, 
                                       GlgPoint * end_point );
GlgImport( GlgBoolean ) GlgScaleObject( GlgObject object, 
                                        GlgCoordType coord_type, 
                                        GlgPoint * center, double x_scale, 
                                        double y_scale, double z_scale );
GlgImport( GlgBoolean ) GlgRotateObject( GlgObject object, 
                                         GlgCoordType coord_type, 
                                         GlgPoint * center, double x_angle, 
                                         double y_angle, double z_angle );
GlgImport( GlgBoolean ) GlgPositionObject( GlgObject object, 
                                           GlgCoordType coord_type, 
                                           GlgLong anchoring,
                                           double x, double y, double z );
GlgImport( GlgBoolean ) GlgFitObject( GlgObject object, GlgCoordType coord_type,
                                      GlgCube * box, GlgBoolean keep_ratio );
GlgImport( GlgBoolean ) GlgLayoutObjects( GlgObject object, GlgObject sel_elem,
                                          GlgLayoutType type, double distance, 
                                          GlgBoolean use_box, 
                                          GlgBoolean process_subobjects );
GlgImport( GlgBoolean ) GlgScreenToWorld( GlgObject object, 
                                          GlgBoolean inside_vp, 
                                          GlgPoint * in_point, 
                                          GlgPoint * out_point );
GlgImport( GlgBoolean ) GlgWorldToScreen( GlgObject object, 
                                          GlgBoolean inside_vp,
                                          GlgPoint * in_point, 
                                          GlgPoint * out_point );
GlgImport( void ) GlgTranslatePointOrigin( GlgObject from_viewport, 
                                           GlgObject to_viewport, 
                                           GlgPoint * point );
GlgImport( GlgObject ) GlgGetParentViewport( GlgObject object, 
                                             GlgBoolean heavy_weight );
GlgImport( GlgBoolean ) GlgRootToScreenCoord( GlgObject viewport, 
                                              GlgPoint2 * root_coord );
GlgImport( GlgBoolean ) GlgPositionToValue( GlgObject object, 
                                            CCONST char * res_name, 
                                            double x, double y, 
                                            GlgBoolean outside_x, 
                                            GlgBoolean outside_y, 
                                            double * value );
GlgImport( GlgObject ) GlgGetLegendSelection( GlgObject object, 
                                              double x, double y );
GlgImport( GlgObject ) GlgCreateChartSelection( GlgObject object, 
                                                GlgObject plot, 
                                                double x, double y, 
                                                double dx, double dy, 
                                                GlgBoolean screen_coord, 
                                                GlgBoolean include_invalid,
                                                GlgBoolean x_priority );
GlgImport( char * ) GlgCreateTooltipString( GlgObject object, double x, 
                                            double y, double dx, double dy, 
                                            CCONST char * format );
GlgImport( GlgObject ) GlgGetAction( GlgObject object, 
                                     GlgActionType action_type,
                                     GlgTriggerType trigger_type, 
                                     GlgLong button, 
                                     GlgArmedStateType armed_state,
                                     GlgDoubleClickStateType double_click_state,
                                     CCONST char * input_action, 
                                     CCONST char * input_subaction, 
                                     GlgBoolean enabled_only );
GlgImport( void ) GlgGetMatrixData( GlgObject matrix, 
                                    GlgMatrixData * matrix_data );
GlgImport( void ) GlgSetMatrixData( GlgObject matrix, 
                                    GlgMatrixData * matrix_data );
#endif

/******* Function prototypes for support functions ******/
GlgImport( GlgLong ) GlgGetMajorVersion( void );
GlgImport( GlgLong ) GlgGetMinorVersion( void );
/* Deprecated, use GlgSetLParameter */
GlgImport( GlgBoolean ) GlgSetReadOnlyStrings( GlgBoolean read_only );
GlgImport( GlgBoolean ) GlgPreAlloc( GlgLong size, GlgPreAllocType type );
GlgImport( GlgBoolean ) GlgSetLParameter( CCONST char * name, 
                                          GlgLong parameter );
GlgImport( GlgBoolean ) GlgGetLParameter( CCONST char * name, 
                                          GlgLong * parameter );
GlgImport( GlgObject ) GlgParseArgs( CCONST char * program_name, 
                                     CCONST char * args_string, 
                                     GlgLong * argc, char *** argv );
GlgImport( void * ) GlgAlloc( GlgLong size );
GlgImport( void ) GlgSetBrowserObject( GlgObject browser, GlgObject object );

GlgImport( GlgBoolean ) GlgSetBrowserSelection( GlgObject object, 
                                                CCONST char * res_name, 
                                                CCONST char * selection, 
                                                CCONST char * filter );
GlgImport( GlgBoolean ) GlgSetEditMode( GlgObject viewport, 
                                        CCONST char * res_name, 
                                        GlgBoolean edit_mode );
GlgImport( GlgBoolean ) GlgResetSizeConstraint( GlgObject text_obj );
GlgImport( void ) GlgSetParentWidget( GlgObject viewport, void * display, 
                                      void * xscreen, void * widget );
GlgImport( void ) GlgSetGeometry( GlgObject viewport, GlgLong x, GlgLong y, 
                                  GlgLong width, GlgLong height );
GlgImport( GlgObject ) GlgFindScreen( GlgLong window );
GlgImport( GlgLong ) GlgSetTopViewportGeometry( GlgObject viewport, 
                                                GlgLong x, GlgLong y, 
                                                GlgLong width, GlgLong height, 
                                                GlgLong border_width );
#ifndef MS_WINDOWS
GlgImport( GlgBoolean ) GlgProcessXCBEvent( Display * display, void * w_event );
GlgImport( GlgBoolean ) GlgSetX11Flush( GlgBoolean flush );
GlgImport( GlgBoolean ) GlgXPrintDefaultError( Display *dpy, XErrorEvent *event,
                                               FILE * file );
#endif
GlgImport( char * ) GlgGetURLCB( CCONST char * request, void * reserved );
GlgImport( GlgGetURLCallback ) GlgSetGetURLCallback( GlgGetURLCallback callback );
GlgImport( GlgGISCallbacks * ) GlgSetGISCallbacks( GlgGISCallbacks * callbacks );
GlgImport( char * ) GlgGetRelativePath( CCONST char * exe_path, 
                                        CCONST char * filename );
/* If check_if_exists=True, and the file does not exists, returns NULL if 
   return_null_if_not_found=True, otherwise returns filename.
   If check_if_exists=False, always returns the created path name. */
GlgImport( char * ) GlgCreateRelativePath( CCONST char * exe_path, 
                                           CCONST char * filename,
                                           GlgBoolean return_null_if_not_found,
                                           GlgBoolean check_if_exists );
GlgImport( char * ) GlgFindFile( CCONST char * filename, 
                                 CCONST char * path, GlgBoolean check_glg_path );
GlgImport( void ) GlgSetFocus( GlgObject object, CCONST char * res_name );
GlgImport( void ) GlgError( GlgErrorType type, CCONST char * message );
GlgImport( void ) GlgRestartTimers( void );

#ifdef MS_WINDOWS
GlgImport( char * ) GlgGetExePath( void );
GlgImport( void ) GlgLoadExeIcon( GlgObject viewport, GlgLong icon_id );
GlgImport( GlgObject ) GlgGetWindowViewport( HWND window );
GlgImport( void ) GlgSetWindowViewport( HWND window, GlgObject viewport );
GlgImport( GlgLong ) CALLBACK GlgControlWindowProc( HWND hwnd, UINT msg, 
                                                    WPARAM wParam, 
                                                    LPARAM lParam );
GlgImport( GlgBoolean ) GlgSetCursor( GlgObject object, 
                                      CCONST char * res_name, GlgLong cursor );
#else
GlgImport( void ) GlgSetAddTimerFunc( GlgAddTimerFunc func );
GlgImport( void ) GlgSetRemoveTimerFunc( GlgRemoveTimerFunc func );
GlgImport( void ) GlgSetDrawTooltipFunc( GlgDrawTooltipFunc func );
#endif

/* Function prototypes for simple array functions that can be used with the
   GLG Standard API. They have the same functionality as the corresponding
   Extended API functions, but can be used in the absense of the extended API.   
   The functions work only with with an array created by GlgArrayCreate()
   and can't be used to add or delete objects from the drawing.
*/
GlgImport( GlgObject ) GlgArrayCreate( GlgLong init_size, GlgLong increment );
GlgImport( void ) GlgArrayFlush( GlgObject object, GlgLong size );
GlgImport( GlgLong ) GlgArrayGetSize( GlgObject object );
GlgImport( GlgAnyType ) GlgArrayGetElement( GlgObject object, 
                                            GlgLong elem_index );
GlgImport( GlgBoolean ) GlgArraySetElement( GlgObject object, 
                                            GlgLong elem_index,
                                            GlgAnyType elem );
GlgImport( GlgBoolean ) GlgArrayAddToBottom( GlgObject object, 
                                             GlgAnyType elem );
GlgImport( GlgBoolean ) GlgArrayAddToTop( GlgObject object, GlgAnyType elem );
GlgImport( GlgBoolean ) GlgArrayAddAt( GlgObject object, GlgAnyType elem, 
                                       GlgLong add_index );
GlgImport( GlgBoolean ) GlgArrayDeleteLast( GlgObject object );
GlgImport( GlgBoolean ) GlgArrayDeleteFirst( GlgObject object );
GlgImport( GlgBoolean ) GlgArrayDeleteAt( GlgObject object, GlgLong del_index );
GlgImport( GlgBoolean ) GlgArrayContains( GlgObject object, GlgAnyType elem );
GlgImport( GlgLong ) GlgArrayGetIndex( GlgObject object, GlgAnyType elem );

/******* Function prototypes for IH utility functions. ******/
typedef enum _GlgCallEventType
{
   GLG_UNDEFINED_CALL_EVENT_TYPE = 0,
   GLG_HI_SETUP_EVENT,
   GLG_HI_RESETUP_EVENT,
   GLG_CLEANUP_EVENT,
   GLG_MESSAGE_EVENT

} GlgCallEventType;

typedef GlgObject GlgIH;
typedef void * GlgCallEvent;
typedef void (*GlgIHEntryPoint)( GlgIH ih, GlgCallEvent call_event );
typedef GlgLong GlgIHToken;

#define GLG_IH_NEW     NULL   /* The just installed IH */
#define GLG_IH_CURR    NULL   /* The curr ih, when no ih parameter is avail. */
#define GLG_IH_GLOBAL  GlgIHGlobalData()    /* Global data storage. */

GlgImport( void ) GlgIHInit( void );
GlgImport( void ) GlgIHTerminate( void );
GlgImport( GlgIH ) GlgIHGlobalData( void );
GlgImport( GlgIH ) GlgIHInstall( GlgIHEntryPoint );
GlgImport( void ) GlgIHStart( void );
GlgImport( void ) GlgIHResetup( GlgIH ih );
GlgImport( void ) GlgIHUninstall( void );
GlgImport( void ) GlgIHUninstallWithToken( GlgIHToken token );
GlgImport( void ) GlgIHUninstallWithEvent( GlgCallEvent call_event );
GlgImport( GlgIHEntryPoint ) GlgIHGetFunction( GlgIH ih );
GlgImport( GlgIHEntryPoint ) GlgIHGetPrevFunction( void );
GlgImport( GlgIHToken ) GlgIHGetToken( GlgCallEvent call_event );
GlgImport( GlgCallEventType ) GlgIHGetType( GlgCallEvent call_event );
GlgImport( void ) GlgIHPassToken( GlgIHEntryPoint ih_ep, GlgIHToken token, 
                                  GlgBoolean uninstall );

GlgImport( GlgBoolean ) GlgIHCallCurrIH( GlgCallEvent call_event );
GlgImport( GlgBoolean ) GlgIHCallCurrIHWithToken( GlgIHToken token );
GlgImport( GlgBoolean ) GlgIHCallCurrIHWithModifToken( GlgCallEvent call_event, 
                                                       GlgIHToken token );
GlgImport( void ) GlgIHCallPrevIHWithToken( GlgIHToken token );
GlgImport( void ) GlgIHCallPrevIHWithModifToken( GlgCallEvent call_event, 
                                                 GlgIHToken token );

GlgImport( void ) GlgIHSetIParameter( GlgIH ih, char * name, GlgLong value );
GlgImport( void ) GlgIHSetPParameter( GlgIH ih, char * name, void * value );
GlgImport( void ) GlgIHSetSParameter( GlgIH ih, char * name, char * value );
GlgImport( void ) GlgIHSetOParameter( GlgIH ih, char * name, GlgObject value );
GlgImport( void ) GlgIHSetDParameter( GlgIH ih, char * name, double value );
GlgImport( void ) GlgIHSetOParameterFromD( GlgIH ih, char * name, double value );
GlgImport( void ) GlgIHSetOParameterFromG( GlgIH ih, char * name, double value1,
                                           double value2, double value3 );

GlgImport( GlgLong ) GlgIHGetIParameter( GlgIH ih, char * name );
GlgImport( void ) * GlgIHGetPParameter( GlgIH ih, char * name );
GlgImport( char ) * GlgIHGetSParameter( GlgIH ih, char * name );
GlgImport( GlgObject ) GlgIHGetOParameter( GlgIH ih, char * name );
GlgImport( double ) GlgIHGetDParameter( GlgIH ih, char * name );

GlgImport( GlgLong ) GlgIHGetOptIParameter( GlgIH ih, char * name, 
                                            GlgLong default_value );
GlgImport( void ) * GlgIHGetOptPParameter( GlgIH ih, char * name, 
                                           void * default_value );
GlgImport( char ) * GlgIHGetOptSParameter( GlgIH ih, char * name, 
                                           char * default_value );
GlgImport( GlgObject ) GlgIHGetOptOParameter( GlgIH ih, char * name, 
                                              GlgObject default_value );
GlgImport( double ) GlgIHGetOptDParameter( GlgIH ih, char * name, 
                                           double default_value );

GlgImport( void ) GlgIHChangeIParameter( GlgIH ih, char * name, GlgLong value );
GlgImport( void ) GlgIHChangePParameter( GlgIH ih, char * name, void * value );
GlgImport( void ) GlgIHChangeSParameter( GlgIH ih, char * name, char * value );
GlgImport( void ) GlgIHChangeOParameter( GlgIH ih, char * name, 
                                         GlgObject value );
GlgImport( void ) GlgIHChangeDParameter( GlgIH ih, char * name, double value );

#ifdef __cplusplus
}
#endif

#endif
