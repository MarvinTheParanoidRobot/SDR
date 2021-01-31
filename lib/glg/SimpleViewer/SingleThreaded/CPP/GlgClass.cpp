// Need to emulate this even on Unix: Windows does not allow ifdef'ing it
#include "stdafx.h"

#include "GlgClass.h"

static GlgBoolean GlgInitialized = False;
static GlgAlarmHandlerCPP GlgAlarmHandlerCPPPtr = (GlgAlarmHandlerCPP)0;

////////////////////////////////////////////////////////////////////////////
// Global alarm handler methods
////////////////////////////////////////////////////////////////////////////

extern "C" void GlgAlarmStub( GlgObject object, GlgObject alarm, 
                              SCONST char * alarm_label, SCONST char * action, 
                              SCONST char * subaction, void * reserved )
{
   (*GlgAlarmHandlerCPPPtr)( object, alarm, 
                             STRING_CAST alarm_label, STRING_CAST action, 
                             STRING_CAST subaction, reserved );
}

GlgAlarmHandlerCPP GlgSetAlarmHandlerCPP( GlgAlarmHandlerCPP alarm_handler )
{
   GlgAlarmHandlerCPP old_handler;

   old_handler = GlgAlarmHandlerCPPPtr;
   GlgAlarmHandlerCPPPtr = alarm_handler;
   GlgSetAlarmHandler( 
                      (GlgAlarmHandler) ( alarm_handler ? GlgAlarmStub : 0 ) );
   return old_handler;
}

////////////////////////////////////////////////////////////////////////////
// GlgSessionC
////////////////////////////////////////////////////////////////////////////
GlgSessionC::GlgSessionC( void )
{
   app_context = NULL;
}


GlgSessionC::GlgSessionC( GlgBoolean initialized,
                          GlgAppContext application_context,
                          int argc, char ** argv )
{
   Create( initialized, application_context, argc, argv );
}

void GlgSessionC::Create( GlgBoolean initialized,
                          GlgAppContext application_context,
                          int argc, char ** argv )
{
#if defined _WINDOWS && defined _AFX
   if( !application_context )
     application_context = (GlgAppContext) AfxGetInstanceHandle();
#endif

   if( !GlgInitialized )
   {
      app_context = GlgInit( initialized, application_context, argc, argv );
      GlgInitialized = True;
   }
}

GlgSessionC::~GlgSessionC( void )
{
   // Keep it initialized: will be cleaned on application exit.
   // Would need a ref count if wanted to call GlgTerminate.
}

GlgAppContext GlgSessionC::GetAppContext( void )
{
   return app_context;
}

GlgBoolean GlgSessionC::MainLoop( void )
{
   return GlgMainLoop( app_context );
}

////////////////////////////////////////////////////////////////////////////
// GlgObjectC
////////////////////////////////////////////////////////////////////////////
GlgObjectC::GlgObjectC( void )
{
   glg_obj = (GlgObject)0;
   suspend_obj = (GlgObject)0;
}

#if GLG_HAS_CPP_EXT_API
GlgObjectC::GlgObjectC( GlgObjectType type, SCONST char * name,
                        GlgAnyType data1, GlgAnyType data2,
                        GlgAnyType data3, GlgAnyType data4 )
{
   glg_obj = 
     GlgCreateObject( type, STRING_CAST name, data1, data2, data3, data4 );
   suspend_obj = (GlgObject)0;
}
#endif

GlgObjectC::GlgObjectC( SCONST char * filename )
{
   if( filename )
     glg_obj = GlgLoadObject( STRING_CAST filename );
   else
     glg_obj = (GlgObject)0;
   suspend_obj = (GlgObject)0;
}

GlgObjectC::GlgObjectC( GlgLong object_data[], GlgLong object_data_size )
{
   if( object_data )
     glg_obj = GlgLoadObjectFromImage( object_data, object_data_size );
   else
     glg_obj = (GlgObject)0;   
   suspend_obj = (GlgObject)0;
}

#if GLG_HAS_CPP_EXT_API
GlgObjectC::GlgObjectC( GlgObjectC& object, GlgCloneType clone_type )
{
   if( object.glg_obj )
     glg_obj = GlgCloneObject( object.glg_obj, clone_type );
   else
     glg_obj = (GlgObject)0;   
   suspend_obj = (GlgObject)0;
}
#endif

GlgObjectC::GlgObjectC( GlgObjectC& object, SCONST char * res_name )
{
   if( !object || !object.glg_obj )
     glg_obj = (GlgObject)0;
   else
     if( !res_name )
       glg_obj = object.glg_obj;
     else
#if GLG_HAS_CPP_INT_API
       glg_obj = 
         GlgGetResourceObject( object.glg_obj, STRING_CAST res_name );
#else
       glg_obj = (GlgObject)0;
#endif   

   glg_obj = GlgReferenceObject( glg_obj );
   suspend_obj = (GlgObject)0;
}

GlgObjectC::GlgObjectC( GlgObject object )
{
   glg_obj = GlgReferenceObject( object );
   suspend_obj = (GlgObject)0;
}

GlgObjectC::GlgObjectC( GlgObjectC * object )
{
   glg_obj = GlgReferenceObject( object->glg_obj );
   suspend_obj = (GlgObject)0;
}

GlgObjectC::~GlgObjectC( void )
{
   GlgDropObject( glg_obj );
   GlgDropObject( suspend_obj );
}
   
GlgObjectC& GlgObjectC::operator= ( const GlgObjectC& object )
{
   if( object.glg_obj != glg_obj )
   {
      GlgDropObject( glg_obj );
      GlgDropObject( suspend_obj );
      suspend_obj = (GlgObject)0;

      glg_obj = GlgReferenceObject( object.glg_obj );
   }
   return *this;
}

GlgObjectC::operator GlgObject()
{
   return glg_obj;
}

GlgObjectC& GlgObjectC::operator++( void )
{
   GlgReferenceObject( glg_obj );
   return *this;  
}

GlgObjectC& GlgObjectC::operator++( int dummy )
{
   GlgReferenceObject( glg_obj );
   return *this;  
}

GlgObjectC& GlgObjectC::operator--( void )
{
   GlgDropObject( glg_obj );
   return *this;  
}

GlgObjectC& GlgObjectC::operator--( int dummy )
{
   GlgDropObject( glg_obj );
   return *this;  
}

void GlgObjectC::Reference( void )
{
   GlgReferenceObject( glg_obj );
}

void GlgObjectC::Drop( void )
{
   GlgDropObject( glg_obj );
}

#if GLG_HAS_CPP_INT_API
GlgBoolean GlgObjectC::Save( SCONST char * filename )
{
   return GlgSaveObject( glg_obj, STRING_CAST filename ); 
}
#endif

GlgBoolean GlgObjectC::Load( SCONST char * filename, SCONST char * object_name )
{   
   GlgObject loaded_obj;

   GlgDropObject( glg_obj );
   GlgDropObject( suspend_obj );
   suspend_obj = (GlgObject)0;
   if( filename )
   {
      loaded_obj = GlgLoadObject( STRING_CAST filename ); 
      if( !loaded_obj || !object_name )
	glg_obj = loaded_obj;
      else
      {
#if GLG_HAS_CPP_INT_API
	 glg_obj = GlgGetResourceObject( loaded_obj, STRING_CAST object_name );
         GlgReferenceObject( glg_obj );
#else
	 glg_obj = (GlgObject)0;
#endif
	 GlgDropObject( loaded_obj );
      }
   }
   else
     glg_obj = (GlgObject)0;
   return ( glg_obj != (GlgObject)0 );
}

GlgBoolean GlgObjectC::Load( void * object_data, GlgLong object_data_size,
                             SCONST char * object_name )
{
   GlgObject loaded_obj;

   GlgDropObject( glg_obj );
   GlgDropObject( suspend_obj );
   suspend_obj = (GlgObject)0;
   if( object_data )
   {
      loaded_obj = GlgLoadObjectFromImage( object_data, object_data_size );
      if( !loaded_obj || !object_name )
	glg_obj = loaded_obj;
      else
      {
#if GLG_HAS_CPP_INT_API
	 glg_obj = 
           GlgGetResourceObject( loaded_obj, STRING_CAST object_name );
         GlgReferenceObject( glg_obj );
#else
	 glg_obj = (GlgObject)0;
#endif
	 GlgDropObject( loaded_obj );
      }
   }
   else
     glg_obj = (GlgObject)0;
   return ( glg_obj != (GlgObject)0 );
}

GlgBoolean GlgObjectC::Load( GlgObjectC& object, SCONST char * object_name )
{   
   // Protect in case object == this
   GlgReferenceObject( object.glg_obj ); 

   GlgDropObject( glg_obj );
   GlgDropObject( suspend_obj );
   suspend_obj = (GlgObject)0;
   if( !object.glg_obj || !object_name )
     glg_obj = object.glg_obj;
   else
   {
#if GLG_HAS_CPP_INT_API
      glg_obj = GlgGetResourceObject( object.glg_obj, STRING_CAST object_name );
#else
      glg_obj = (GlgObject)0;
#endif
   }
   GlgReferenceObject( glg_obj );

   GlgDropObject( object.glg_obj );
   return ( glg_obj != (GlgObject)0 );
}

GlgBoolean GlgObjectC::LoadWidget( SCONST char * filename )
{   
   GlgDropObject( glg_obj );
   GlgDropObject( suspend_obj );
   suspend_obj = (GlgObject)0;
   if( filename )
     glg_obj = GlgLoadWidgetFromFile( STRING_CAST filename );   
   else
     glg_obj = (GlgObject)0;
   return ( glg_obj != (GlgObject)0 );
}

GlgBoolean GlgObjectC::LoadWidget( void * object_data, GlgLong object_data_size )
{
   GlgDropObject( glg_obj );
   GlgDropObject( suspend_obj );
   suspend_obj = (GlgObject)0;
   if( object_data )
     glg_obj = GlgLoadWidgetFromImage( object_data, object_data_size );
   else
     glg_obj = (GlgObject)0;
   return ( glg_obj != (GlgObject)0 );
}

GlgBoolean GlgObjectC::LoadWidget( GlgObjectC& object )
{   
   // make this->LoadWidgetFromObject( this ) safe
   if( object )
     GlgReferenceObject( object.glg_obj );

   GlgDropObject( glg_obj );
   GlgDropObject( suspend_obj );
   suspend_obj = (GlgObject)0;
   if( object.glg_obj )
     glg_obj = GlgLoadWidgetFromObject( object.glg_obj );
   else
     glg_obj = (GlgObject)0;

   if( object )
     GlgDropObject( object.glg_obj );

   return ( glg_obj != (GlgObject)0 );
}

void GlgObjectC::LoadNullObject( void )
{
   GlgDropObject( glg_obj );
   GlgDropObject( suspend_obj );
   glg_obj = (GlgObject)0;
   suspend_obj = (GlgObject)0;
}

#if GLG_HAS_CPP_EXT_API
void GlgObjectC::Create( GlgObjectType type, SCONST char * name,
                         GlgAnyType data1, GlgAnyType data2,
                         GlgAnyType data3, GlgAnyType data4 )
{
   GlgDropObject( glg_obj );
   GlgDropObject( suspend_obj );
   suspend_obj = (GlgObject)0;   
   glg_obj = 
     GlgCreateObject( type, STRING_CAST name, data1, data2, data3, data4 );
}
#endif

#if GLG_HAS_CPP_EXT_API
void GlgObjectC::Copy( GlgCloneType clone_type )
{
   GlgObject copy_obj;

   if( !glg_obj )
     return;

   copy_obj = GlgCloneObject( glg_obj, clone_type );

   GlgDropObject( glg_obj );
   GlgDropObject( suspend_obj );
   glg_obj = copy_obj;
   suspend_obj = (GlgObject)0;   
}
#endif

GlgBoolean GlgObjectC::GetResource( SCONST char * res_name, double * value )
{
   if( !glg_obj )
     return False;
   return GlgGetDResource( glg_obj, STRING_CAST res_name, value );
}

GlgBoolean GlgObjectC::GetResource( SCONST char * res_name, 
                                    SCONST char ** s_value )
{
   if( !glg_obj )
     return False;
   return GlgGetSResource( glg_obj, 
                           STRING_CAST res_name, STRING_PTR_CAST s_value );
}

GlgBoolean GlgObjectC::GetResource( SCONST char * res_name, double * x_value,
                                    double * y_value, double * z_value )
{
   if( !glg_obj )
     return False;
   return GlgGetGResource( glg_obj, STRING_CAST res_name, 
                           x_value, y_value, z_value );
}

GlgBoolean GlgObjectC::SetResource( SCONST char * res_name, double value )
{
   if( !glg_obj )
     return False;
   return GlgSetDResource( glg_obj, STRING_CAST res_name, value );
}

GlgBoolean GlgObjectC::SetResource( SCONST char * res_name, double value, 
                                    GlgBoolean if_changed )
{
   if( !glg_obj )
     return False;
   return 
     GlgSetDResourceIf( glg_obj, STRING_CAST res_name, value, if_changed );
}

GlgBoolean GlgObjectC::SetResource( SCONST char * res_name, 
                                    SCONST char * s_value )
{
   if( !glg_obj )
     return False;
   return 
     GlgSetSResource( glg_obj, STRING_CAST res_name, STRING_CAST s_value );
}

GlgBoolean GlgObjectC::SetResource( SCONST char * res_name, 
                                    SCONST char * s_value, 
                                    GlgBoolean if_changed )
{
   if( !glg_obj )
     return False;
   return 
     GlgSetSResourceIf( glg_obj, STRING_CAST res_name, STRING_CAST s_value, 
                        if_changed );
}

GlgBoolean GlgObjectC::SetResource( SCONST char * res_name, double x_value,
                                    double y_value, double z_value )
{
   if( !glg_obj )
     return False;
   return GlgSetGResource( glg_obj,
                           STRING_CAST res_name, x_value, y_value, z_value );
}

GlgBoolean GlgObjectC::SetResource( SCONST char * res_name, double x_value,
                                    double y_value, double z_value,
                                    GlgBoolean if_changed )
{
   if( !glg_obj )
     return False;
   return GlgSetGResourceIf( glg_obj, STRING_CAST res_name, 
                             x_value, y_value, z_value, if_changed );
}

GlgBoolean GlgObjectC::SetResource( SCONST char * res_name, SCONST char * format,
                                    double value )
{
   if( !glg_obj )
     return False;
   return 
     GlgSetSResourceFromD( glg_obj, 
                           STRING_CAST res_name, STRING_CAST format, value );
}

GlgBoolean GlgObjectC::SetResource( SCONST char * res_name, SCONST char * format,
                                    double value, GlgBoolean if_changed )
{
   if( !glg_obj )
     return False;
   return 
     GlgSetSResourceFromDIf( glg_obj, STRING_CAST res_name, STRING_CAST format,
                             value, if_changed );
}

GlgBoolean GlgObjectC::SetResource( SCONST char * res_name, 
                                    GlgObjectC& from_object )
{
   if( !glg_obj || !from_object || !from_object.glg_obj )
     return False;
   return GlgSetResourceFromObject( glg_obj, STRING_CAST res_name,
                                    from_object.glg_obj );
}

GlgBoolean GlgObjectC::SetResource( SCONST char * res_name, 
                                    GlgObjectC& from_object, 
                                    GlgBoolean if_changed )
{
   if( !glg_obj || !from_object || !from_object.glg_obj )
     return False;
   return GlgSetResourceFromObjectIf( glg_obj, STRING_CAST res_name, 
                                      from_object.glg_obj, if_changed );
}

GlgBoolean GlgObjectC::HasResourceObject( SCONST char * res_name )
{
   if( !glg_obj )
     return False;
   return GlgHasResourceObject( glg_obj, STRING_CAST res_name );
}

#if GLG_HAS_CPP_INT_API
GlgObject GlgObjectC::GetResourceObject( SCONST char * res_name )
{
   if( !glg_obj )
     return (GlgObject)0;
   return GlgGetResourceObject( glg_obj, STRING_CAST res_name );
}
#endif

#if GLG_HAS_CPP_EXT_API
GlgBoolean GlgObjectC::SetResourceObject( SCONST char * res_name, 
                                          GlgObjectC * object )
{
   if( !glg_obj )
     return False;
   return 
     GlgSetResourceObject( glg_obj, STRING_CAST res_name, 
                           GET_PTR_OBJECT( object ) );
}

// Deprecated, use the new function with "GlgObjectC *"
GlgBoolean GlgObjectC::SetResourceObject( SCONST char * res_name, 
                                          GlgObjectC& object )
{
   if( !glg_obj )
     return False;
   return 
     GlgSetResourceObject( glg_obj, STRING_CAST res_name, object.glg_obj );
}
#endif

GlgBoolean GlgObjectC::GetTag( SCONST char * tag_name, double * value )
{
   if( !glg_obj )
     return False;
   return GlgGetDTag( glg_obj, STRING_CAST tag_name, value );
}

GlgBoolean GlgObjectC::GetTag( SCONST char * tag_name, SCONST char ** s_value )
{
   if( !glg_obj )
     return False;
   return GlgGetSTag( glg_obj, STRING_CAST tag_name, STRING_PTR_CAST s_value );
}

GlgBoolean GlgObjectC::GetTag( SCONST char * tag_name, double * x_value,
                               double * y_value, double * z_value )
{
   if( !glg_obj )
     return False;
   return GlgGetGTag( glg_obj, STRING_CAST tag_name, 
                      x_value, y_value, z_value );
}

GlgBoolean GlgObjectC::SetTag( SCONST char * tag_name, double value,
                               GlgBoolean if_changed )
{
   if( !glg_obj )
     return False;
   return GlgSetDTag( glg_obj, STRING_CAST tag_name, value, if_changed );
}

GlgBoolean GlgObjectC::SetTag( SCONST char * tag_name, SCONST char * s_value,
                               GlgBoolean if_changed )
{
   if( !glg_obj )
     return False;
   return GlgSetSTag( glg_obj, STRING_CAST tag_name, STRING_CAST s_value, 
                      if_changed );
}

GlgBoolean GlgObjectC::SetTag( SCONST char * tag_name, double x_value,
                               double y_value, double z_value, 
                               GlgBoolean if_changed )
{
   if( !glg_obj )
     return False;
   return GlgSetGTag( glg_obj, STRING_CAST tag_name, 
                      x_value, y_value, z_value, if_changed );
}

GlgBoolean GlgObjectC::SetTag( SCONST char * tag_name, SCONST char * format,
                               double value, GlgBoolean if_changed )
{
   if( !glg_obj )
     return False;
   return GlgSetSTagFromD( glg_obj, STRING_CAST tag_name, STRING_CAST format, 
                           value, if_changed );
}

GlgBoolean GlgObjectC::HasTagSource( SCONST char * tag_source )
{
   if( !glg_obj )
     return False;
   return GlgHasTagSource( glg_obj, STRING_CAST tag_source );
}

GlgBoolean GlgObjectC::HasTagName( SCONST char * tag_name )
{
   if( !glg_obj )
     return False;
   return GlgHasTagName( glg_obj, STRING_CAST tag_name );
}

GlgObject GlgObjectC::CreateTagList( GlgBoolean unique_tag_sources )
{
   if( !glg_obj )
     return (GlgObject)0;
   return GlgCreateTagList( glg_obj, unique_tag_sources );
}

GlgObject GlgObjectC::CreateAlarmList()
{
   if( !glg_obj )
     return (GlgObject)0;
   return GlgCreateAlarmList( glg_obj );
}

#if GLG_HAS_CPP_INT_API
GlgBoolean GlgObjectC::HasTag( SCONST char * tag_name, 
                               GlgTagType tag_type_mask )
{
   if( !glg_obj )
     return 0;
   return GlgHasTag( glg_obj, STRING_CAST tag_name, tag_type_mask );
}

GlgObject GlgObjectC::QueryTags( GlgTagType tag_type_mask )
{
   if( !glg_obj )
     return (GlgObject)0;
   return GlgQueryTags( glg_obj, tag_type_mask );
}

GlgObject GlgObjectC::GetTagObject( SCONST char * search_string, 
                                    GlgBoolean by_name,
                                    GlgBoolean unique_tags, 
                                    GlgBoolean single_tag, 
                                    GlgTagType tag_type_mask )
{
   if( !glg_obj )
     return (GlgObject)0;
   return GlgGetTagObject( glg_obj, STRING_CAST search_string, by_name, 
                           unique_tags, single_tag, tag_type_mask );
}

GlgObject GlgObjectC::GetAlarmObject( SCONST char * search_string, 
                                      GlgBoolean single_alarm )
{
   if( !glg_obj )
     return (GlgObject)0;
   return 
     GlgGetAlarmObject( glg_obj, STRING_CAST search_string, single_alarm, 0 );
}
#endif

GlgObject GlgObjectC::GetGlgObject( void )
{
   return glg_obj;
}

void GlgObjectC::SetGlgObject( GlgObject object )
{
   if( object == glg_obj )
     return;

   GlgDropObject( glg_obj );
   GlgDropObject( suspend_obj );
   suspend_obj = (GlgObject)0;

   glg_obj = GlgReferenceObject( object );
}

GlgBoolean GlgObjectC::IsNull( void )
{
   return ( glg_obj == (GlgObject)0 );
}

GlgBoolean GlgObjectC::operator!( void )
{
   return this->IsNull();
}

GlgBoolean GlgObjectC::Same( GlgObjectC& object )
{
   if( !object )
     return !glg_obj;
   else
     return ( glg_obj == object.glg_obj );
}

void GlgObjectC::InitialDraw( void )
{
   if( glg_obj )
     GlgInitialDraw( glg_obj );
}

void GlgObjectC::SetupHierarchy( void )
{
   if( glg_obj )
     GlgSetupHierarchy( glg_obj );
}

void GlgObjectC::ResetHierarchy( void )
{
   if( glg_obj )
     GlgResetHierarchy( glg_obj );
}

GlgBoolean GlgObjectC::Reset( void )
{
   if( !glg_obj )
     return False;
   return GlgReset( glg_obj );
}

GlgBoolean GlgObjectC::Sync( void )
{
   if( !glg_obj )
     return False;
   return GlgSync( glg_obj );
}

GlgBoolean GlgObjectC::Update( void )
{
   if( !glg_obj )
     return False;
   return GlgUpdate( glg_obj );
}

GlgBoolean GlgObjectC::Print( SCONST char * filename, double x, double y,
                              double width, double height,
                              GlgBoolean portrait, GlgBoolean stretch )
{
   if( !glg_obj )
     return False;
   return
     GlgPrint( glg_obj, STRING_CAST filename, 
               x, y, width, height, portrait, stretch );
}

void GlgObjectC::MetaDraw( SCONST char * filename, double x, double y,
                           double width, double height,
                           GlgBoolean portrait, GlgBoolean stretch )
{
   if( glg_obj )
     GlgMetaDraw( glg_obj, STRING_CAST filename, 
                  x, y, width, height, portrait, stretch ); 
}

#if defined _WINDOWS && defined _AFX

GlgBoolean GlgObjectC::Print( CDC * dc, double x, double y,
                              double width, double height,
                              GlgBoolean portrait, GlgBoolean stretch )
{
   if( !glg_obj )
     return False;
   return
     GlgWinPrint( glg_obj, dc->GetSafeHdc(), x, y, width, height,
                  portrait, stretch );
}

void GlgObjectC::OnPrint( CDC * dc )
{
   GlgWinPrint( glg_obj, dc->GetSafeHdc(), 0., 0., -1., -1., 0, 0 );
}

void GlgObjectC::OnDrawMetafile( CDC * dc )
{
   GlgWinPrint( glg_obj, dc->GetSafeHdc(), 0., 0., -1., -1., 0, 0 );
}

#endif

GlgBoolean GlgObjectC::SetZoom( SCONST char * res_name, GlgLong type, 
                                double value )
{
   return GlgSetZoom( glg_obj, STRING_CAST res_name, type, value );
}

GlgBoolean GlgObjectC::SetZoomMode( SCONST char * res_name, 
                                    GlgObjectC * zoom_object, 
                                    SCONST char * zoom_object_name, 
                                    GlgZoomMode mode )
{
   return GlgSetZoomMode( glg_obj, STRING_CAST res_name, 
                          GET_PTR_OBJECT( zoom_object ), 
                          STRING_CAST zoom_object_name, 
                          mode );
}

/*** Deprecated, use SetZoomMode(). ***/
GlgBoolean GlgObjectC::SetGISZoom( SCONST char * res_name, 
                                   GlgObjectC& gis_object, 
                                   SCONST char * gis_name )
{
   return GlgSetGISZoom( glg_obj, STRING_CAST res_name, 
                         gis_object.glg_obj, STRING_CAST gis_name );
}

GlgBoolean GlgObjectC::SaveImage( SCONST char * res_name, SCONST char * file,
                                  GlgImageFormat format )
{
   if( !glg_obj )
     return False;
   return 
     GlgSaveImage( glg_obj, STRING_CAST res_name, STRING_CAST file, format );
}

GlgBoolean GlgObjectC::SaveImageCustom( SCONST char * res_name, 
                                        SCONST char * file,
                                        GlgImageFormat format,
                                        GlgLong x, GlgLong y, 
                                        GlgLong width, GlgLong height,
                                        GlgLong gap )
{
   if( !glg_obj )
     return False;
   return GlgSaveImageCustom( glg_obj, STRING_CAST res_name, STRING_CAST file,
                              format, x, y, width, height, gap );
}

GlgAnyType GlgObjectC::SendMessage( SCONST char * res_name, 
                                    SCONST char * message,
                                    GlgAnyType param1, GlgAnyType param2, 
                                    GlgAnyType param3, GlgAnyType param4 )
{
   if( !glg_obj )
     return False;
   return GlgSendMessage( glg_obj, STRING_CAST res_name, STRING_CAST message, 
                          param1, param2, param3, param4 );
}

#ifdef MS_WINDOWS
HBITMAP GlgObjectC::GenerateImage( SCONST char * res_name, HBITMAP pixmap)
#else
Pixmap GlgObjectC::GenerateImage( SCONST char * res_name, Pixmap pixmap )
#endif
{
   if( !glg_obj )
#ifdef MS_WINDOWS
     return (HBITMAP)0;
#else
     return (Pixmap)0;
#endif
   return GlgGenerateImage( glg_obj, STRING_CAST res_name, pixmap );
}

#ifdef MS_WINDOWS
HBITMAP GlgObjectC::GenerateImageCustom( SCONST char * res_name, HBITMAP pixmap,
                                         GlgLong x, GlgLong y, 
                                         GlgLong width, GlgLong height,
                                         GlgLong gap )
#else
Pixmap GlgObjectC::GenerateImageCustom( SCONST char * res_name, Pixmap pixmap,
                                        GlgLong x, GlgLong y, 
                                        GlgLong width, GlgLong height, 
                                        GlgLong gap )
#endif
{
   if( !glg_obj )
#ifdef MS_WINDOWS
     return (HBITMAP)0;
#else
     return (Pixmap)0;
#endif
   return GlgGenerateImageCustom( glg_obj, STRING_CAST res_name, pixmap, 
                                  x, y, width, height, gap );
}

#if GLG_HAS_CPP_EXT_API
GlgBoolean GlgObjectC::SetXform( GlgObjectC * xform )
{
   if( !glg_obj )
     return False;
   return GlgSetXform( glg_obj, GET_PTR_OBJECT( xform ) );
}

// Deprecated, use the new function with "GlgObjectC *"
GlgBoolean GlgObjectC::SetXform( GlgObjectC& xform )
{
   if( !glg_obj )
     return False;
   return GlgSetXform( glg_obj, xform.glg_obj );
}
#endif
   
#if GLG_HAS_CPP_INT_API
GlgObject GlgObjectC::GetParent( GlgLong * num_parents )
{
   if( !glg_obj )
     return (GlgObject)0;
   return GlgGetParent( glg_obj, num_parents );
}

GlgCube * GlgObjectC::GetBoxPtr( void )
{
   if( !glg_obj )
     return (GlgCube*)0;
   return GlgGetBoxPtr( glg_obj );
}

GlgObject GlgObjectC::GetDrawingMatrix( void )
{
   if( !glg_obj )
     return (GlgObject)0;
   return GlgGetDrawingMatrix( glg_obj );
}

void GlgObjectC::SuspendObject( void )
{
   if( !glg_obj )
     return;
   GlgDropObject( suspend_obj );
   suspend_obj = GlgReferenceObject( GlgSuspendObject( glg_obj ) );
}

void GlgObjectC::ReleaseObject( void )
{
   if( !glg_obj )
     return;
   GlgReleaseObject( glg_obj, suspend_obj );
   GlgDropObject( suspend_obj );
   suspend_obj = 0;
}
   
GlgObject GlgObjectC::CreateResourceList( GlgBoolean list_named,
                                          GlgBoolean list_default,
                                          GlgBoolean list_aliases )
{
   if( !glg_obj )
     return (GlgObject)0;
   return GlgCreateResourceList( glg_obj, list_named, list_default,
                                 list_aliases );
}

GlgObject GlgObjectC::CreatePointArray( GlgControlPointType type )
{
   if( !glg_obj )
     return (GlgObject)0;
   return GlgCreatePointArray( glg_obj, type );
}

GlgBoolean GlgObjectC::TransformObject( GlgObjectC& xform,
                                        GlgCoordType coord_type,
                                        GlgObjectC * parent )
{
   if( !glg_obj )
     return False;
   return GlgTransformObject( glg_obj, xform.glg_obj, 
                              coord_type, GET_PTR_OBJECT( parent ) );
}

// Deprecated, use the new function with "GlgObjectC *"
GlgBoolean GlgObjectC::TransformObject( GlgObjectC& xform,
                                        GlgCoordType coord_type,
                                        GlgObjectC& parent )
{
   if( !glg_obj )
     return False;
   return GlgTransformObject( glg_obj, xform.glg_obj, 
                              coord_type, parent.glg_obj );
}

GlgBoolean GlgObjectC::MoveObjectBy( GlgCoordType coord_type, 
				     double x, double y, double z )
{
   if( !glg_obj )
     return False;
   return GlgMoveObjectBy( glg_obj, coord_type, x, y, z );
}

GlgBoolean GlgObjectC::MoveObject( GlgCoordType coord_type, 
				   GlgPoint * start_point, 
				   GlgPoint * end_point )
{
   if( !glg_obj )
     return False;
   return GlgMoveObject( glg_obj, coord_type, start_point, end_point );
}

GlgBoolean GlgObjectC::ScaleObject( GlgCoordType coord_type,
				    GlgPoint * center,
				    double x, double y, double z )
{
   if( !glg_obj )
     return False;
   return GlgScaleObject( glg_obj, coord_type, center, x, y, z );
}

GlgBoolean GlgObjectC::RotateObject( GlgCoordType coord_type,
				     GlgPoint * center,
				     double x, double y, double z )
{
   if( !glg_obj )
     return False;
   return GlgRotateObject( glg_obj, coord_type, center, x, y, z );
}

GlgBoolean GlgObjectC::PositionObject( GlgCoordType coord_type,
				       GlgLong anchoring,
				       double x, double y, double z )
{
   if( !glg_obj )
     return False;
   return GlgPositionObject( glg_obj, coord_type, anchoring, x, y, z );
}

GlgBoolean GlgObjectC::FitObject( GlgCoordType coord_type, GlgCube * box,
                                  GlgBoolean keep_ratio )
{
   if( !glg_obj )
     return False;
   return GlgFitObject( glg_obj, coord_type, box, keep_ratio );
}

GlgBoolean GlgObjectC::LayoutObjects( GlgObject sel_elem,
                                      GlgLayoutType type, double distance, 
                                      GlgBoolean use_box, 
                                      GlgBoolean process_subobjects )
{
   if( !glg_obj )
     return False;
   return GlgLayoutObjects( glg_obj, sel_elem, type, distance, use_box, 
                            process_subobjects );
}

GlgBoolean GlgObjectC::ScreenToWorld( GlgBoolean inside_vp,
				      GlgPoint * in_point,
				      GlgPoint * out_point )
{
   if( !glg_obj )
     return False;
   return GlgScreenToWorld( glg_obj, inside_vp, in_point, out_point );
}

GlgBoolean GlgObjectC::WorldToScreen( GlgBoolean inside_vp,
				      GlgPoint * in_point,
				      GlgPoint * out_point )
{
   if( !glg_obj )
     return False;
   return GlgWorldToScreen( glg_obj, inside_vp, in_point, out_point );
}

void GlgObjectC::GetMatrixData( GlgMatrixData * matrix_data )
{
   if( !glg_obj )
      return;
   GlgGetMatrixData( glg_obj, matrix_data );
}

void GlgObjectC::SetMatrixData( GlgMatrixData * matrix_data )
{
   if( !glg_obj )
     return;
   GlgSetMatrixData( glg_obj, matrix_data );
}

GlgBoolean GlgObjectC::ConstrainObject( GlgObjectC& to_attribute )
{
   if( !glg_obj )
     return False;
   return GlgConstrainObject( glg_obj, to_attribute.glg_obj );
}

GlgBoolean GlgObjectC::UnconstrainObject( void )
{
   if( !glg_obj )
     return False;
   return GlgUnconstrainObject( glg_obj );
}
   
GlgObject GlgObjectC::InverseMatrix( void )
{
   if( !glg_obj )
     return (GlgObject)0;
   return GlgCreateInversedMatrix( glg_obj );
}

void GlgObjectC::TransformPoint( GlgPoint * in_point,
                                 GlgPoint * out_point )
{
   if( glg_obj )
     GlgTransformPoint( glg_obj, in_point, out_point );
}

GlgBoolean GlgObjectC::IsContainer()
{
   if( glg_obj )
     switch( GlgGetObjectType( glg_obj ) )
     {
      case GLG_ARRAY:
      case GLG_LIST:
      case GLG_VIEWPORT:
      case GLG_POLYGON:
      case GLG_SPLINE:
      case GLG_CONNECTOR:
	return True;
     }
   return False;
}

void GlgObjectC::SetStart( void )
{
   if( glg_obj )
     GlgSetStart( glg_obj );
}

GlgObject GlgObjectC::Iterate( void )
{
   if( !glg_obj )
     return (GlgObject)0;
   return GlgIterate( glg_obj );
}
#endif

#if GLG_HAS_CPP_EXT_API
GlgBoolean GlgObjectC::Add( GlgObjectC& object, GlgAccessType position,
                            GlgPositionModifier adjust_next )
{
   if( !glg_obj )
     return False;
   return GlgAddObject( glg_obj, object.glg_obj, position, adjust_next );
}

GlgBoolean GlgObjectC::AddToTop( GlgObjectC& object )
{
   if( !glg_obj )
     return False;
   return GlgAddObjectToTop( glg_obj, object.glg_obj );
}

GlgBoolean GlgObjectC::AddToBottom( GlgObjectC& object )
{
   if( !glg_obj )
     return False;
   return GlgAddObjectToBottom( glg_obj, object.glg_obj );
}

GlgBoolean GlgObjectC::AddAt( GlgObjectC& object, GlgLong index )
{
   if( !glg_obj )
     return False;
   return GlgAddObjectAt( glg_obj, object.glg_obj, index );
}

GlgBoolean GlgObjectC::Delete( void * reserved, GlgAccessType position,
                               GlgPositionModifier delete_next )
{
   if( !glg_obj )
     return False;
   return GlgDeleteObject( glg_obj, reserved, position, delete_next );
}

GlgBoolean GlgObjectC::DeleteTopObject()
{
   if( !glg_obj )
     return False;
   return GlgDeleteTopObject( glg_obj );
}

GlgBoolean GlgObjectC::DeleteBottomObject()
{
   if( !glg_obj )
     return False;
   return GlgDeleteBottomObject( glg_obj );
}

GlgBoolean GlgObjectC::DeleteObject( GlgObjectC& object )
{
   if( !glg_obj )
     return False;
   return GlgDeleteThisObject( glg_obj, object.glg_obj );
}

GlgBoolean GlgObjectC::DeleteObjectAt( GlgLong index )
{
   if( !glg_obj )
     return False;
   return GlgDeleteObjectAt( glg_obj, index );
}
#endif

#if GLG_HAS_CPP_INT_API
GlgBoolean GlgObjectC::ReorderElement( GlgLong old_index, GlgLong new_index )
{
   if( !glg_obj )
     return False;
   return GlgReorderElement( glg_obj, old_index, new_index );
}

GlgBoolean GlgObjectC::ContainsObject( GlgObjectC& object )
{
   if( !glg_obj )
     return False;
   return GlgContainsObject( glg_obj, object.glg_obj );
}

GlgObject GlgObjectC::GetNamedObject( SCONST char * name )
{
   if( !glg_obj )
     return (GlgObject)0;
   return GlgGetNamedObject( glg_obj, STRING_CAST name);
}

GlgLong GlgObjectC::GetIndex( GlgObjectC& object )
{
   if( !glg_obj )
     return -1;
   return GlgGetIndex( glg_obj, object.glg_obj );
}

GlgLong GlgObjectC::GetStringIndex( SCONST char * string )
{
   if( !glg_obj )
     return -1;
   return GlgGetStringIndex( glg_obj, STRING_CAST string );
}

// Deprecated
GlgObject GlgObjectC::Find( GlgLong index ){ return GetElement( index ); }
GlgObject GlgObjectC::Find( SCONST char * name )
{ 
   return GetNamedObject( STRING_CAST name );
}
GlgObject GlgObjectC::Find( GlgObjectC& object )
{
   return ( ContainsObject( object ) ? object.glg_obj : (GlgObject)NULL );
}

void GlgObjectC::Inverse()
{
   if( glg_obj )
     GlgInverse( glg_obj );
}
#endif

GlgLong GlgObjectC::GetSize( void )
{
   if( !glg_obj )
     return 0;
   return GlgGetSize( glg_obj );
}

GlgObject GlgObjectC::GetElement( GlgLong index )
{
   if( !glg_obj )
     return (GlgObject)0;
   return GlgGetElement( glg_obj, index );
}


GlgObject GlgObjectC::GetNamedPlot( SCONST char * res_name, 
                                    SCONST char * plot_name )
{
   if( !glg_obj )
     return (GlgObject)0;   
   return
     GlgGetNamedPlot( glg_obj, STRING_CAST res_name, STRING_CAST plot_name );
}

GlgObject GlgObjectC::AddPlot( SCONST char * res_name, GlgObjectC& plot )
{
   if( !glg_obj )
     return (GlgObject)0;   
   return GlgAddPlot( glg_obj, STRING_CAST res_name, plot.glg_obj );
}

GlgBoolean GlgObjectC::DeletePlot( SCONST char * res_name, GlgObjectC& plot )
{
   if( !glg_obj )
     return False;
   return GlgDeletePlot( glg_obj, STRING_CAST res_name, plot.glg_obj );
}

GlgBoolean GlgObjectC::GetDataExtent( SCONST char * res_name, 
                                      GlgMinMax * min_max, GlgBoolean pan_x )
{
   if( !glg_obj )
     return False;   
   return GlgGetDataExtent( glg_obj, STRING_CAST res_name, min_max, pan_x );
}

#if GLG_HAS_CPP_INT_API
GlgBoolean GlgObjectC::PositionToValue( SCONST char * res_name, 
                                        double x, double y, 
                                        GlgBoolean outside_x, 
                                        GlgBoolean outside_y, double * value )
{
   if( !glg_obj )
     return False;
   return GlgPositionToValue( glg_obj, STRING_CAST res_name, x, y, 
                              outside_x, outside_y, value );
}
#endif

#if GLG_HAS_CPP_INT_API
GlgObject GlgObjectC::CreateChartSelection( GlgObjectC * data_group,
                                            double x, double y, 
                                            double dx, double dy,
                                            GlgBoolean screen_coord, 
                                            GlgBoolean include_invalid, 
                                            GlgBoolean x_priority )
{
   if( !glg_obj )
     return (GlgObject)0;   
   return GlgCreateChartSelection( glg_obj, GET_PTR_OBJECT( data_group ), 
                                   x, y, dx, dy,
                                   screen_coord, include_invalid, x_priority );
}
#endif

#if GLG_HAS_CPP_INT_API
char * GlgObjectC::CreateTooltipString( double x, double y, 
                                        double dx, double dy, 
                                        SCONST char * reserved )
{
   if( !glg_obj )
     return NULL;   
   return STRING_CAST 
     GlgCreateTooltipString( glg_obj, x, y, dx, dy, STRING_CAST reserved );
}
#endif

extern "C" void GlgObjectInputStub( GlgObject callback_viewport_obj,
                                    GlgAnyType client_data, 
                                    GlgAnyType call_data )
{
   GlgObjectC
     * initiator = (GlgObjectC*)client_data,
     callback_viewport( callback_viewport_obj ),
     message( (GlgObject)call_data );

   initiator->Input( callback_viewport, message );
}

extern "C" void GlgObjectSelectStub( GlgObject callback_viewport_obj,
                                     GlgAnyType client_data, 
                                     GlgAnyType call_data )
{
   GlgObjectC
     * initiator = (GlgObjectC*)client_data,
     callback_viewport( callback_viewport_obj );     

   initiator->Select( callback_viewport, (SCONST char **)call_data );
}

extern "C" void GlgObjectTraceStub( GlgObject callback_viewport_obj,
                                    GlgAnyType client_data, 
                                    GlgAnyType call_data )
{
   GlgObjectC
     * initiator = (GlgObjectC*)client_data,
     callback_viewport( callback_viewport_obj );     

   initiator->Trace( callback_viewport, (GlgTraceCBStruct*) call_data );
}

extern "C" void GlgObjectTrace2Stub( GlgObject callback_viewport_obj,
                                     GlgAnyType client_data, 
                                     GlgAnyType call_data )
{
   GlgObjectC
     * initiator = (GlgObjectC*)client_data,
     callback_viewport( callback_viewport_obj );     

   initiator->Trace2( callback_viewport, (GlgTraceCBStruct*) call_data );
}

extern "C" void GlgObjectHierarchyStub( GlgObject callback_viewport_obj,
                                        GlgAnyType client_data, 
                                        GlgAnyType call_data )
{
   GlgObjectC
     * initiator = (GlgObjectC*)client_data,
     callback_viewport( callback_viewport_obj );     

   initiator->Hierarchy( callback_viewport, 
                        (GlgHierarchyCBStruct*) call_data );
}

void GlgObjectC::Input( GlgObjectC& callback_viewport, GlgObjectC& message )
{
}

void GlgObjectC::Select( GlgObjectC& callback_viewport, 
                         SCONST char ** name_array )
{
}

void GlgObjectC::Trace( GlgObjectC& callback_viewport, 
                        GlgTraceCBStruct * trace_info )
{
}

void GlgObjectC::Trace2( GlgObjectC& callback_viewport, 
                         GlgTraceCBStruct * trace_info )
{
}

void GlgObjectC::Hierarchy( GlgObjectC& callback_viewport, 
                            GlgHierarchyCBStruct * hierarchy_info )
{
}

void GlgObjectC::EnableCallback( GlgCallbackType callback_type,
                                 GlgObject callback_viewport )
{
   if( !callback_viewport )
     callback_viewport = glg_obj;

   if( !callback_viewport )
   {
      GlgError( GLG_WARNING, 
                STRING_CAST "Ignoring EnableCallback: no viewport yet." );
      return;
   }

   switch( callback_type )
   {
    case GLG_INPUT_CB:
      GlgAddCallback( callback_viewport, callback_type, 
                     (GlgCallbackProc)GlgObjectInputStub, (GlgAnyType)this );
      break;

    case GLG_SELECT_CB:
      GlgAddCallback( callback_viewport, callback_type,
                     (GlgCallbackProc)GlgObjectSelectStub, (GlgAnyType)this );
	break;

    case GLG_TRACE_CB:
      GlgAddCallback( callback_viewport, callback_type,
                     (GlgCallbackProc)GlgObjectTraceStub, (GlgAnyType)this );
      break;
      
    case GLG_TRACE2_CB:
      GlgAddCallback( callback_viewport, callback_type,
                     (GlgCallbackProc)GlgObjectTrace2Stub, (GlgAnyType)this );
      break;
      
    case GLG_HIERARCHY_CB:
      GlgAddCallback( callback_viewport, callback_type,
                     (GlgCallbackProc)GlgObjectHierarchyStub, 
                     (GlgAnyType)this );
      break;
      
    default:
      GlgError( GLG_WARNING, STRING_CAST "Invalid callback type." );
      break;
   }
}

////////////////////////////////////////////////////////////////////////////
// GlgLinkC
////////////////////////////////////////////////////////////////////////////
GlgLinkC::GlgLinkC( void ){}

GlgLinkC::GlgLinkC( SCONST char * display_name, SCONST char * server_name )
{
   glg_obj = 
     GlgCreateLink( STRING_CAST display_name, STRING_CAST server_name );
}

GlgLinkC::GlgLinkC( GlgObjectC& object )
{
   if( object )
     glg_obj = GlgReferenceObject( object.glg_obj );
   else
     glg_obj = (GlgObject)0;
}

GlgLinkC::GlgLinkC( GlgObject object )
{
   glg_obj = GlgReferenceObject( object );
}

#if GLG_HAS_CPP_EXT_API
GlgLinkC::GlgLinkC( GlgLinkC& object )
{
   if( object.glg_obj )
     glg_obj = GlgCopyObject( object.glg_obj );
   else
     glg_obj = (GlgObject)0;
}
#endif

GlgLinkC::~GlgLinkC( void )
{
   if( glg_obj && GlgLinkActive( glg_obj ) )
   {
      GlgDestroyLink( glg_obj );
      glg_obj = (GlgObject)0;
   }
}

GlgBoolean GlgLinkC::Connect( SCONST char * display_name, 
                              SCONST char * server_name )
{
   if( glg_obj )
   {
      if( GlgLinkActive( glg_obj ) )
         GlgDestroyLink( glg_obj );
      GlgDropObject( glg_obj );
   }
   glg_obj = GlgCreateLink( STRING_CAST display_name, STRING_CAST server_name );
   return glg_obj && GlgLinkActive( glg_obj );
}

void GlgLinkC::Disconnect( void )
{
   if( glg_obj && GlgLinkActive( glg_obj ) )
     GlgDestroyLink( glg_obj );
}

GlgLinkC& GlgLinkC::operator= ( const GlgLinkC& object )
{
   GlgObject old_obj;

   old_obj = glg_obj;   // Protect in case object == this
   glg_obj = GlgReferenceObject( object.glg_obj );
   GlgDropObject( old_obj );
   suspend_obj = (GlgObject)0;
   return *this;
}

GlgBoolean GlgLinkC::IsActive()
{
   return GlgLinkActive( glg_obj );
}

#ifdef _WINDOWS

#ifdef _AFX

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// GlgControlC
/////////////////////////////////////////////////////////////////////////////

GlgControlC::GlgControlC( void )
{
   if( !GlgInitialized )
   {
      GlgInit( False, (GlgAppContext) AfxGetInstanceHandle(), 0, NULL );
      GlgInitialized = True;
   }

   viewport = (GlgObject)0;  // Creates a viewport object with glg_obj=null
   got_initial_draw = False;
   drawn = False;
}

GlgControlC::~GlgControlC()
{
   if( this->m_hWnd )
     DestroyWindow();

   got_initial_draw = False;
   drawn = False;
}

void GlgControlC::Create( SCONST char * drawing_file, CWnd * parent,
                          SCONST char * control_name, CRect *in_rect, 
                          int IDC_id )
{
   CRect rect;

   if( in_rect )
     rect = *in_rect;
   else
     parent->GetClientRect( &rect );
   
   if( drawing_file )
   {
      GlgDropObject( viewport.glg_obj );        
      viewport.glg_obj = GlgLoadWidgetFromFile( STRING_CAST drawing_file );
   }

   ((CWnd*)this)->Create( "GlgControl", STRING_CAST control_name, 
                          WS_CHILD | WS_VISIBLE, rect, parent, IDC_id );
   got_initial_draw = False;
   drawn = False;
}

void GlgControlC::Create( void * image_data, GlgLong image_size, CWnd * parent,
                          SCONST char * control_name, CRect *in_rect, 
                          int IDC_id )
{
   CRect rect;

   if( in_rect )
     rect = *in_rect;
   else
     parent->GetClientRect( &rect );
   
   if( image_data )
   {
      GlgDropObject( viewport.glg_obj );        
      viewport.glg_obj = GlgLoadWidgetFromImage( image_data, image_size );
   }

   ((CWnd*)this)->Create( "GlgControl", STRING_CAST control_name, 
                          WS_CHILD | WS_VISIBLE, rect, parent, IDC_id );
   got_initial_draw = False;
   drawn = False;
}

void GlgControlC::Create( GlgObjectC& object, CWnd * parent,
                          SCONST char * control_name, CRect *in_rect, 
                          int IDC_id )
{
   CRect rect;

   if( in_rect )
     rect = *in_rect;
   else
     parent->GetClientRect( &rect );
   
   if( object )
   {
      GlgDropObject( viewport.glg_obj );        
      viewport.glg_obj = GlgLoadWidgetFromObject( object.glg_obj );
   }

   ((CWnd*)this)->Create( "GlgControl", STRING_CAST control_name, 
                          WS_CHILD | WS_VISIBLE, rect, parent, IDC_id );
   got_initial_draw = False;
   drawn = False;
}

void GlgControlC::InitialDraw( void )
{
   if( !got_initial_draw )
   {
      /* Draw the first time */
      got_initial_draw = True;

      drawn = False;    /* Request to attach it */
   }

   /* Do it only if has window, otherwise it'll be done later,
      on the first draw. */
   if( this->m_hWnd && viewport.glg_obj )
   {      
      if( !drawn )
        DrawFirstTime();      
      else
        GlgUpdate( viewport.glg_obj );
   }
}

void GlgControlC::DrawFirstTime()
{
   if( viewport.glg_obj )
   {
      GlgSetWindowViewport( this->m_hWnd, viewport.glg_obj );
      
      HCallback();
      GlgSetupHierarchy( viewport.glg_obj );
      
      VCallback();   
      GlgUpdate( viewport.glg_obj );
   }
   drawn = True;
}

void GlgControlC::OnShowWindow( BOOL bShow, UINT nStatus )
{
   CWnd::OnShowWindow( bShow, nStatus );

   /* Attach when shown first time. */
   if( bShow && got_initial_draw && !drawn )
     DrawFirstTime();
}

void GlgControlC::SetupHierarchy( void )
{
   if( drawn )
   {
      GlgSetupHierarchy( viewport.glg_obj );
   }
   else   /* First time */
   {
      if( this->m_hWnd && viewport.glg_obj )
      {
         GlgSetWindowViewport( this->m_hWnd, viewport.glg_obj );
         
         HCallback();
         GlgSetupHierarchy( viewport.glg_obj );
         VCallback(); 
         
         got_initial_draw = True;
         drawn = True;
      }
   }
}

void GlgControlC::ResetHierarchy( void )
{
   if( viewport.glg_obj )
   {
      GlgResetHierarchy( viewport.glg_obj );
      got_initial_draw = False;
      drawn = False;
   }
}

GlgBoolean GlgControlC::Update( void )
{
   if( !this->m_hWnd || !viewport.glg_obj || !drawn )
     return False;   
   return GlgUpdate( viewport.glg_obj );
}

void GlgControlC::SetDrawing( SCONST char * filename )
{
   GlgObject new_viewport;

   if( !this->m_hWnd )  // Hasn't been created yet
     return;

   if( filename )
     new_viewport = GlgLoadWidgetFromFile( STRING_CAST filename );
   else
     new_viewport = (GlgObject)0;

   GlgSetWindowViewport( this->m_hWnd, new_viewport );

   GlgDropObject( viewport.glg_obj );
   viewport.glg_obj = new_viewport;

   GlgDropObject( viewport.suspend_obj );
   viewport.suspend_obj = (GlgObject)0;

   got_initial_draw = False;
   drawn = False;
}

void GlgControlC::SetDrawing( void * drawing_image, 
                              GlgLong drawing_image_size )
{
   GlgObject new_viewport;

   if( !this->m_hWnd )  // Hasn't been created yet
     return;

   if( drawing_image )
     new_viewport = 
       GlgLoadWidgetFromImage( drawing_image, drawing_image_size );
   else
     new_viewport = (GlgObject)0;

   GlgSetWindowViewport( this->m_hWnd, new_viewport );

   GlgDropObject( viewport.glg_obj );
   viewport.glg_obj = new_viewport;

   GlgDropObject( viewport.suspend_obj );
   viewport.suspend_obj = (GlgObject)0;

   got_initial_draw = False;
   drawn = False;
}

void GlgControlC::SetDrawing( GlgObjectC& object )
{
   GlgObject new_viewport;

   if( !this->m_hWnd )  // Hasn't been created yet
     return;

   if( object.glg_obj )
     new_viewport = GlgLoadWidgetFromObject( object.glg_obj );
   else
     new_viewport = (GlgObject)0;

   GlgSetWindowViewport( this->m_hWnd, new_viewport );

   GlgDropObject( viewport.glg_obj );
   viewport.glg_obj = new_viewport;

   GlgDropObject( viewport.suspend_obj );
   viewport.suspend_obj = (GlgObject)0;

   got_initial_draw = False;
   drawn = False;
}

void GlgControlC::Print( CDC * pDC, DWORD dwFlags )
{
   if( !this->m_hWnd || !viewport )  // Hasn't been created yet
     return;

   GlgWinPrint( viewport, pDC->GetSafeHdc(), 0., 0., -1., -1., 0, 0 );
}

void GlgControlInputStub( GlgObject callback_viewport_obj,
                          GlgAnyType client_data, GlgAnyType call_data )
{
   GlgControlC * initiator = (GlgControlC*)client_data;
   GlgObjectC
     callback_viewport( callback_viewport_obj ),
   message( (GlgObject)call_data );
   
   initiator->Input( callback_viewport, message );
}

void GlgControlSelectStub( GlgObject callback_viewport_obj,
                           GlgAnyType client_data, GlgAnyType call_data )
{
   GlgControlC * initiator = (GlgControlC*)client_data;
   GlgObjectC callback_viewport( callback_viewport_obj );
    
   initiator->Select( callback_viewport, (SCONST char **)call_data );
}

void GlgControlTraceStub( GlgObject callback_viewport_obj,
                          GlgAnyType client_data, GlgAnyType call_data )
{
   GlgControlC * initiator = (GlgControlC*)client_data;
   GlgObjectC callback_viewport( callback_viewport_obj );
    
   initiator->Trace( callback_viewport, (GlgTraceCBStruct*) call_data );
}

void GlgControlTrace2Stub( GlgObject callback_viewport_obj,
                           GlgAnyType client_data, GlgAnyType call_data )
{
   GlgControlC * initiator = (GlgControlC*)client_data;
   GlgObjectC callback_viewport( callback_viewport_obj );
    
   initiator->Trace2( callback_viewport, (GlgTraceCBStruct*) call_data );
}

void GlgControlHierarchyStub( GlgObject callback_viewport_obj,
                              GlgAnyType client_data, GlgAnyType call_data )
{
   GlgControlC * initiator = (GlgControlC*)client_data;
   GlgObjectC callback_viewport( callback_viewport_obj );
    
   initiator->Hierarchy( callback_viewport,
                        (GlgHierarchyCBStruct*) call_data );
}

void GlgControlC::Input( GlgObjectC& callback_viewport, GlgObjectC& message )
{
}

void GlgControlC::Select( GlgObjectC& callback_viewport, 
                          SCONST char ** name_array )
{
}

void GlgControlC::Trace( GlgObjectC& callback_viewport,
			GlgTraceCBStruct * trace_info )
{
}

void GlgControlC::Trace2( GlgObjectC& callback_viewport,
                          GlgTraceCBStruct * trace_info )
{
}

void GlgControlC::Hierarchy( GlgObjectC& callback_viewport,
                             GlgHierarchyCBStruct * hierarchy_info )
{
}

void GlgControlC::HCallback()
{
}

void GlgControlC::VCallback()
{
}

void GlgControlC::EnableCallback( GlgCallbackType callback_type,
                                  GlgObject callback_viewport )
{
   if( !callback_viewport )
     callback_viewport = viewport;

   if( !callback_viewport )
   {
      GlgError( GLG_WARNING, 
                STRING_CAST "Ignoring EnableCallback: no viewport yet." );
      return;
   }

   switch( callback_type )
   {
    case GLG_INPUT_CB:
      GlgAddCallback( callback_viewport, callback_type,
                      (GlgCallbackProc)GlgControlInputStub, (GlgAnyType)this );
      break;
      
    case GLG_SELECT_CB:
      GlgAddCallback( callback_viewport, callback_type,
                      (GlgCallbackProc)GlgControlSelectStub, (GlgAnyType)this );
      break;

    case GLG_TRACE_CB:
      GlgAddCallback( callback_viewport, callback_type,
                      (GlgCallbackProc)GlgControlTraceStub, (GlgAnyType)this );
      break;

    case GLG_TRACE2_CB:
      GlgAddCallback( callback_viewport, callback_type,
                      (GlgCallbackProc)GlgControlTrace2Stub, (GlgAnyType)this );
      break;
      
    case GLG_HIERARCHY_CB:
      GlgAddCallback( callback_viewport, callback_type,
                      (GlgCallbackProc)GlgControlHierarchyStub, 
                      (GlgAnyType)this );
      break;
      
    default:
      GlgError( GLG_WARNING, STRING_CAST "Invalid callback type." );
      break;
   }
}

GlgObject GlgControlC::GetViewport( void )
{
   return viewport.glg_obj;
}

BEGIN_MESSAGE_MAP(GlgControlC, CWnd)
	//{{AFX_MSG_MAP(GlgControlC)
        ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#endif

#else

////////////////////////////////////////////////////////////////////////////
// GlgWrapperC
////////////////////////////////////////////////////////////////////////////
GlgWrapperC::GlgWrapperC( void )
{
   widget = (Widget)0;
}

void GlgWrapperC::Create( SCONST char * filename, Widget parent,
                          SCONST char * widget_name, GlgLong width, 
                          GlgLong height )
{
   Arg al[10];
   Cardinal ac;

   ac = 0;
   XtSetArg( al[ac], XtNinitialResourcesPersistent, False ); ac++;
   XtSetArg( al[ac], XtNwidth, width ); ac++;
   XtSetArg( al[ac], XtNheight, height ); ac++;
   XtSetArg( al[ac], STRING_CAST XtNglgDrawingFile, 
	     STRING_CAST filename ); ac++;
   widget = XtCreateWidget ( STRING_CAST widget_name, glgWrapperWidgetClass, 
			    parent, al, ac );

   XtAddCallback( widget, XtNglgHInitCB, 
                 (XtCallbackProc)GlgWrapperHXtStub, (XtPointer)this );
   XtAddCallback( widget, XtNglgVInitCB, 
                 (XtCallbackProc)GlgWrapperVXtStub, (XtPointer)this );
   XtManageChild( widget );
}

void GlgWrapperC::Create( void * image_data, GlgLong image_data_size,
                          Widget parent, SCONST char * widget_name,
                          GlgLong width, GlgLong height )
{
   Arg al[10];
   Cardinal ac;

   ac = 0;
   XtSetArg( al[ac], XtNinitialResourcesPersistent, False ); ac++;
   XtSetArg( al[ac], XtNwidth, width ); ac++;
   XtSetArg( al[ac], XtNheight, height ); ac++;
   XtSetArg( al[ac], STRING_CAST XtNglgDrawingImage, image_data ); ac++;
   XtSetArg( al[ac], STRING_CAST XtNglgImageSize, image_data_size ); ac++;
   widget = XtCreateWidget ( STRING_CAST widget_name, glgWrapperWidgetClass, 
			    parent, al, ac );

   XtAddCallback( widget, XtNglgHInitCB, 
                 (XtCallbackProc)GlgWrapperHXtStub, (XtPointer)this );
   XtAddCallback( widget, XtNglgVInitCB, 
                 (XtCallbackProc)GlgWrapperVXtStub, (XtPointer)this );
   XtManageChild( widget );
}
	       
void GlgWrapperC::Create( GlgObjectC& object, Widget parent,
                          SCONST char * widget_name, GlgLong width, 
                          GlgLong height )
{
   Arg al[10];
   Cardinal ac;

   ac = 0;
   XtSetArg( al[ac], XtNinitialResourcesPersistent, False ); ac++;
   XtSetArg( al[ac], XtNwidth, width ); ac++;
   XtSetArg( al[ac], XtNheight, height ); ac++;
   if( object )
   {
      XtSetArg( al[ac], STRING_CAST XtNglgDrawingObject, 
		object.glg_obj ); ac++;
   }
   widget = XtCreateWidget ( STRING_CAST widget_name, glgWrapperWidgetClass, 
			    parent, al, ac );

   XtAddCallback( widget, XtNglgHInitCB, 
                 (XtCallbackProc)GlgWrapperHXtStub, (XtPointer)this );
   XtAddCallback( widget, XtNglgVInitCB, 
                 (XtCallbackProc)GlgWrapperVXtStub, (XtPointer)this );
   XtManageChild( widget );
}

GlgWrapperC::~GlgWrapperC()
{
   if( widget )
     XtDestroyWidget( widget );
}

void GlgWrapperC::SetDrawing( SCONST char * filename )
{
   Arg al[5];
   Cardinal ac;

   if( !widget )   // Hasn't been created yet.
     return;

   ac = 0;
   XtSetArg( al[ac], STRING_CAST XtNglgDrawingFile, 
	     STRING_CAST filename ); ac++;
   XtSetValues( widget, al, ac );

   GlgDropObject( glg_obj );
   GlgDropObject( suspend_obj );
   glg_obj = GlgReferenceObject( XglgGetWidgetViewport( widget ) );
   suspend_obj = (GlgObject)0;
}

void GlgWrapperC::SetDrawing( void * image_data, GlgLong image_data_size )
{
   Arg al[5];
   Cardinal ac;

   if( !widget )   // Hasn't been created yet.
     return;

   ac = 0;
   XtSetArg( al[ac], STRING_CAST XtNglgDrawingImage, image_data ); ac++;
   XtSetArg( al[ac], STRING_CAST XtNglgImageSize, image_data_size ); ac++;
   XtSetValues( widget, al, ac );

   GlgDropObject( glg_obj );
   GlgDropObject( suspend_obj );
   glg_obj = GlgReferenceObject( XglgGetWidgetViewport( widget ) );
   suspend_obj = (GlgObject)0;
}

void GlgWrapperC::SetDrawing( GlgObjectC& object )
{
   Arg al[5];
   Cardinal ac;

   if( !widget )   // Hasn't been created yet.
     return;

   ac = 0;
   if( object )
   {
      XtSetArg( al[ac], STRING_CAST XtNglgDrawingObject, 
		object.glg_obj ); ac++;
   }
   else
   {
      XtSetArg( al[ac], STRING_CAST XtNglgDrawingObject, NULL ); ac++;
   }
   XtSetValues( widget, al, ac );

   GlgDropObject( glg_obj );
   GlgDropObject( suspend_obj );
   glg_obj = GlgReferenceObject( XglgGetWidgetViewport( widget ) );
   suspend_obj = (GlgObject)0;
}

Widget GlgWrapperC::GetWidget()
{
   return widget;
}

GlgObject GlgWrapperC::GetViewport()
{
   if( !widget )
     return (GlgObject)0;

   if( !glg_obj )
     glg_obj = GlgReferenceObject( XglgGetWidgetViewport( widget ) );
   return glg_obj;
}
     
void GlgWrapperInputStub( GlgObject callback_viewport_obj,
                          GlgAnyType client_data, GlgAnyType call_data )
{
   GlgObjectC
     * initiator = (GlgObjectC*)client_data,
     callback_viewport( callback_viewport_obj ),
     message( (GlgObject)call_data );

   initiator->Input( callback_viewport, message );
}

void GlgWrapperSelectStub( GlgObject callback_viewport_obj,
                           GlgAnyType client_data, GlgAnyType call_data )
{
   GlgWrapperC * initiator = (GlgWrapperC*)client_data;
   GlgObjectC callback_viewport( callback_viewport_obj );

   initiator->Select( callback_viewport, (SCONST char **)call_data );
}

void GlgWrapperTraceStub( GlgObject callback_viewport_obj,
                          GlgAnyType client_data, GlgAnyType call_data )
{
   GlgWrapperC * initiator = (GlgWrapperC*)client_data;
   GlgObjectC callback_viewport( callback_viewport_obj );

   initiator->Trace( callback_viewport, (GlgTraceCBStruct*) call_data );
}

void GlgWrapperTrace2Stub( GlgObject callback_viewport_obj,
                           GlgAnyType client_data, GlgAnyType call_data )
{
   GlgWrapperC * initiator = (GlgWrapperC*)client_data;
   GlgObjectC callback_viewport( callback_viewport_obj );

   initiator->Trace2( callback_viewport, (GlgTraceCBStruct*) call_data );
}

void GlgWrapperHierarchyStub( GlgObject callback_viewport_obj,
                              GlgAnyType client_data, GlgAnyType call_data )
{
   GlgWrapperC * initiator = (GlgWrapperC*)client_data;
   GlgObjectC callback_viewport( callback_viewport_obj );

   initiator->Hierarchy( callback_viewport, (GlgHierarchyCBStruct*) call_data );
}

void GlgWrapperHXtStub( Widget callback_widget, XtPointer client_data,
                        XtPointer call_data )
{
   GlgWrapperC * initiator = (GlgWrapperC*)client_data;
   GlgObjectC
     callback_viewport( XglgGetWidgetViewport( callback_widget ) ),
     message( (GlgObject)call_data );

   initiator->HCallback( callback_viewport );
}

void GlgWrapperVXtStub( Widget callback_widget, XtPointer client_data,
                        XtPointer call_data )
{
   GlgWrapperC * initiator = (GlgWrapperC*)client_data;
   GlgObjectC
     callback_viewport( XglgGetWidgetViewport( callback_widget ) ),
     message( (GlgObject)call_data );

   initiator->VCallback( callback_viewport );
}

void GlgWrapperInputXtStub( Widget callback_widget, XtPointer client_data,
                            XtPointer call_data )
{
   GlgWrapperC * initiator = (GlgWrapperC*)client_data;
   GlgObjectC
     callback_viewport( XglgGetWidgetViewport( callback_widget ) ),
     message( (GlgObject)call_data );

   initiator->Input( callback_viewport, message );
}

void GlgWrapperSelectXtStub( Widget callback_widget, XtPointer client_data,
                             XtPointer call_data )
{
   GlgWrapperC * initiator = (GlgWrapperC*)client_data;
   GlgObjectC callback_viewport( XglgGetWidgetViewport( callback_widget ) );

   initiator->Select( callback_viewport, (SCONST char **)call_data );
}

void GlgWrapperTraceXtStub( Widget callback_widget, XtPointer client_data,
                            XtPointer call_data )
{
   GlgWrapperC * initiator = (GlgWrapperC*)client_data;
   GlgObjectC callback_viewport( XglgGetWidgetViewport( callback_widget ) );

   initiator->Trace( callback_viewport, (GlgTraceCBStruct*) call_data );
}

void GlgWrapperTrace2XtStub( Widget callback_widget, XtPointer client_data,
                             XtPointer call_data )
{
   GlgWrapperC * initiator = (GlgWrapperC*)client_data;
   GlgObjectC callback_viewport( XglgGetWidgetViewport( callback_widget ) );

   initiator->Trace2( callback_viewport, (GlgTraceCBStruct*) call_data );
}

void GlgWrapperHierarchyXtStub( Widget callback_widget, XtPointer client_data,
                                XtPointer call_data )
{
   GlgWrapperC * initiator = (GlgWrapperC*)client_data;
   GlgObjectC callback_viewport( XglgGetWidgetViewport( callback_widget ) );

   initiator->Hierarchy( callback_viewport, 
                        (GlgHierarchyCBStruct*) call_data );
}

void GlgWrapperC::HCallback( GlgObjectC& viewport )
{
}

void GlgWrapperC::VCallback( GlgObjectC& viewport )
{
}

void GlgWrapperC::Input( GlgObjectC& callback_viewport, GlgObjectC& message )
{
}

void GlgWrapperC::Select( GlgObjectC& callback_viewport, 
                          SCONST char ** name_array )
{
}

void GlgWrapperC::Trace( GlgObjectC& callback_viewport, 
                         GlgTraceCBStruct * trace_info )
{
}

void GlgWrapperC::Trace2( GlgObjectC& callback_viewport,
                          GlgTraceCBStruct * trace_info )
{
}

void GlgWrapperC::Hierarchy( GlgObjectC& callback_viewport,
                             GlgHierarchyCBStruct * hierarchy_info )
{
}

void GlgWrapperC::EnableCallback( GlgCallbackType callback_type,
                                  GlgObject callback_viewport )
{
   switch( callback_type )
   {
    case GLG_INPUT_CB:
      if( callback_viewport )
	GlgAddCallback( callback_viewport, callback_type,
                        (GlgCallbackProc)GlgWrapperInputStub,
                        (GlgAnyType)this );
      else  /* !callback_viewport: use the wrapper */     
	XtAddCallback( widget, XtNglgInputCB,
                       (XtCallbackProc)GlgWrapperInputXtStub,
                       (GlgAnyType)this );
      break;
      
    case GLG_SELECT_CB:
      if( callback_viewport )
	GlgAddCallback( callback_viewport, callback_type,
                        (GlgCallbackProc)GlgWrapperSelectStub,
                        (GlgAnyType)this );
      else  /* !callback_viewport: use the wrapper */     
	XtAddCallback( widget, XtNglgSelectCB,
                       (XtCallbackProc)GlgWrapperSelectXtStub,
                       (GlgAnyType)this );
      break;

    case GLG_TRACE_CB:
      if( callback_viewport )
	GlgAddCallback( callback_viewport, callback_type,
                        (GlgCallbackProc)GlgWrapperTraceStub,
                        (GlgAnyType)this );
      else  /* !callback_viewport: use the wrapper */     
	XtAddCallback( widget, XtNglgTraceCB,
                       (XtCallbackProc)GlgWrapperTraceXtStub,
                       (GlgAnyType)this );
      break;

    case GLG_TRACE2_CB:
      if( callback_viewport )
	GlgAddCallback( callback_viewport, callback_type,
                        (GlgCallbackProc)GlgWrapperTrace2Stub,
                        (GlgAnyType)this );
      else  /* !callback_viewport: use the wrapper */     
	XtAddCallback( widget, XtNglgTrace2CB,
                       (XtCallbackProc)GlgWrapperTrace2XtStub,
                       (GlgAnyType)this );
      break;

    case GLG_HIERARCHY_CB:
      if( callback_viewport )
	GlgAddCallback( callback_viewport, callback_type,
                        (GlgCallbackProc)GlgWrapperHierarchyStub,
                        (GlgAnyType)this );
      else  /* !callback_viewport: use the wrapper */     
	XtAddCallback( widget, XtNglgHierarchyCB,
                       (XtCallbackProc)GlgWrapperHierarchyXtStub,
                       (GlgAnyType)this );
      break;

    default:
      GlgError( GLG_WARNING, STRING_CAST "Invalid callback type." );
      break;
   }
}

void GlgWrapperC::SetXtResource( String xt_res_name, SCONST char * res_value )
{
   Arg al[ 5 ];
   Cardinal ac;

   if( !widget )   // Hasn't been created yet.
     return;

   ac = 0;
   XtSetArg( al[ac], xt_res_name, (XtArgVal) res_value ); ac++;
   XtSetValues( widget, al, ac );
}

void GlgWrapperC::GetXtResource( String xt_res_name, 
                                 SCONST char ** res_value_ptr )
{
   Arg al[ 5 ];
   Cardinal ac;

   if( !widget )   // Hasn't been created yet.
   {
      *res_value_ptr = NULL;
      return;
   }

   ac = 0;
   XtSetArg( al[ac], xt_res_name, STRING_PTR_CAST res_value_ptr ); ac++;
   XtGetValues( widget, al, ac );
}

#endif
