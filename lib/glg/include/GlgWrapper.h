#ifndef _Glg_Wrapper_h
#define _Glg_Wrapper_h

#ifdef __cplusplus
extern "C" {
#endif

#include "GlgApi.h"

/********** GLG Wrapper Widget types and resources. ***********/

typedef struct _GlgInputCBStruct
{
   GlgCallbackType reason;
   GlgObject viewport;      /* Wrapper widget's viewport */
   Widget widget;           /* Wrapper's widget ID */
   GlgObject message_obj;   /* Same as in the old input callback */   
} GlgInputCBStruct;

typedef struct _GlgSelectCBStruct
{
   GlgCallbackType reason;
   XEvent * event;            /* Event that triggered the callback */
   GlgObject viewport;        /* Wrapper widget's viewport */
   Widget widget;             /* Wrapper's widget ID */
   GlgLong num_selected;      /* The length of the selection list */
   char ** selection_list;    /* Same as in the old selection callback */   
} GlgSelectCBStruct;

#define XtNglgDrawingFile   "glgDrawingFile"
#define XtNglgDrawingImage  "glgDrawingImage"
#define XtNglgImageSize     "glgImageSize"
#define XtNglgDrawingObject "glgDrawingObject"

#define XtNglgHInitCB        XtNglgNumInitCB
#define XtNglgVInitCB        XtNglgValInitCB

/* Obsolete, kept for backward compatibility */
#define XtNglgNumInitCB     "glgNumInitCB"
#define XtNglgValInitCB     "glgValInitCB"

#define XtNglgMotifSelectCB "glgMotifSelectCB"
#define XtNglgMotifInputCB  "glgMotifInputCB"
#define XtNglgSelectCB      "glgSelectCB"
#define XtNglgInputCB       "glgInputCB"
#define XtNglgTraceCB       "glgTraceCB"
#define XtNglgTrace2CB      "glgTrace2CB"
#define XtNglgHierarchyCB   "glgHierarchyCB"
#define XtNglgPickResol     "glgPickResol"

#define XtNglgHResource0 "glgHResource0"
#define XtNglgHResource1 "glgHResource1"
#define XtNglgHResource2 "glgHResource2"
#define XtNglgHResource3 "glgHResource3"
#define XtNglgHResource4 "glgHResource4"
#define XtNglgHResource5 "glgHResource5"
#define XtNglgHResource6 "glgHResource6"
#define XtNglgHResource7 "glgHResource7"
#define XtNglgHResource8 "glgHResource8"
#define XtNglgHResource9 "glgHResource9"

#define XtNglgHResource10 "glgHResource10"
#define XtNglgHResource11 "glgHResource11"
#define XtNglgHResource12 "glgHResource12"
#define XtNglgHResource13 "glgHResource13"
#define XtNglgHResource14 "glgHResource14"
#define XtNglgHResource15 "glgHResource15"
#define XtNglgHResource16 "glgHResource16"
#define XtNglgHResource17 "glgHResource17"
#define XtNglgHResource18 "glgHResource18"
#define XtNglgHResource19 "glgHResource19"

#define XtNglgVResource0 "glgVResource0"
#define XtNglgVResource1 "glgVResource1"
#define XtNglgVResource2 "glgVResource2"
#define XtNglgVResource3 "glgVResource3"
#define XtNglgVResource4 "glgVResource4"
#define XtNglgVResource5 "glgVResource5"
#define XtNglgVResource6 "glgVResource6"
#define XtNglgVResource7 "glgVResource7"
#define XtNglgVResource8 "glgVResource8"
#define XtNglgVResource9 "glgVResource9"

#define XtNglgVResource10 "glgVResource10"
#define XtNglgVResource11 "glgVResource11"
#define XtNglgVResource12 "glgVResource12"
#define XtNglgVResource13 "glgVResource13"
#define XtNglgVResource14 "glgVResource14"
#define XtNglgVResource15 "glgVResource15"
#define XtNglgVResource16 "glgVResource16"
#define XtNglgVResource17 "glgVResource17"
#define XtNglgVResource18 "glgVResource18"
#define XtNglgVResource19 "glgVResource19"

#ifndef EXCEED_MIX
externalref 
#endif
   WidgetClass glgWrapperWidgetClass;

GlgObject XglgGetWidgetViewport( Widget widget );

/* Define Xglg functions as corresponding Glg functions */

#define XglgInit( initialized, context, argc, argv ) \
  GlgInit( initialized, context, argc, argv )
#define XglgTerminate()                   GlgTerminate()
#define XglgSync( object )                GlgSync( object )
#define XglgCreateLink( display, server ) GlgCreateLink( display, server ) 
#define XglgLinkActive( object )          GlgLinkActive( object )
#define XglgDestroyLink( object )         GlgDestroyLink( object ) 
#define XglgSetErrorHandler( handler )    GlgSetErrorHandler( handler )
#define XglgUpdate( object )              GlgUpdate( object )
#define XglgReset( object )               GlgReset( object );
#define XglgPrint( object, file, x, y, width, height, portrait, stretch ) \
  GlgPrint( object, file, x, y, width, height, portrait, stretch )
#define XglgFree( ptr )                   GlgFree( ptr )
#define XglgStrClone( string )            GlgStrClone( string )
#define XglgCreateIndexedName( string, index ) \
  GlgCreateIndexedName( string, index )
#define XglgConcatResNames( string1, string2 ) \
  GlgConcatResNames( string1, string2 )
#define XglgSetDResource( object, res_name, value ) \
  GlgSetDResource( object, res_name, value )
#define XglgGetDResource( object, res_name, ret_value ) \
  GlgGetDResource( object, res_name, ret_value )
#define XglgSetSResource( object, res_name, value ) \
  GlgSetSResource( object, res_name, value )
#define XglgGetSResource( object, res_name, ret_value ) \
  GlgGetSResource( object, res_name, ret_value )
#define XglgSetGResource( object, res_name, value1, value2, value3 ) \
  GlgSetGResource( object, res_name, value1, value2, value3 )
#define XglgGetGResource(object, res_name, ret_value1, ret_value2, ret_value3)\
  GlgGetGResource( object, res_name, ret_value1, ret_value2, ret_value3 )
#define XglgSetSResourceFromD( object, res_name, format, value ) \
  GlgSetSResourceFromD( object, res_name, format, value )

/* Obsolete, for backward compatibility only. */
#define XglgInitComm( initialized, context, argc, argv ) \
  GlgInit( initialized, context, argc, argv )
#define XglgTerminateComm()               GlgTerminate()

#ifdef __cplusplus
}
#endif

#endif /* _glg_Wrapper_h */
