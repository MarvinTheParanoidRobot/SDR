function GlgToolkit()
{
this.glg_handle = GetGLGHandle();
if( this.glg_handle.glg_object != null ) return this;
this.glg_handle.glg_object = function(){};
this.glg_handle.glg_point = function(){};
this.glg_handle.glg_cube = function(){};
var tproto = Object.getPrototypeOf( this );
var oproto = this.glg_handle.glg_object.prototype;
var gproto = this.glg_handle.glg_point.prototype;
var cproto = this.glg_handle.glg_cube.prototype;
gproto.CopyFrom = function( point )
{
this.x = point.x; this.y = point.y; this.z = point.z;
}
gproto.Equals = function( point )
{
return point != null && this.x == point.x && this.y == point.y && this.z == point.z;
}
cproto.CopyFrom = function( cube )
{
this.p1.CopyFrom( cube.p1 );
this.p2.CopyFrom( cube.p2 );
}
cproto.Equals = function( cube )
{
return this.p1.Equals( cube.p1 ) && this.p2.Equals( cube.p2 );
}
tproto.ObjectsEqual = function( object1, object2 )
{
if( object1 == null ) return ( object2 == null );
else return object1.Equals( object2 );
}
oproto.Equals = function( object )
{ return object != null && this.obj == object.obj; }
tproto.StartTime = function( ) { this.glg_handle._StartTime( ); }
tproto.PrintTime = function( label ) { this.glg_handle._PrintTime( label ); }
tproto.ThrowExceptionOnError = function( user_error, internal_error, null_strings ) { this.glg_handle._ThrowExceptionOnError( user_error, internal_error, null_strings ); }
tproto.SetErrorHandler = function( handler ) { return this.glg_handle._SetErrorHandler( handler ); }
tproto.SetAlarmHandler = function( handler ) { return this.glg_handle._SetAlarmHandler( handler ); }
tproto.LoadAsset = function( url, request_type, js_callback, user_data ) { this.glg_handle._LoadAsset( url, request_type, js_callback, user_data ); }
tproto.LoadObject = function( data, encoding, path, rebind_ref ) { return this.glg_handle._LoadObject( data, encoding, path, rebind_ref ); }
tproto.LoadWidget = function( data, encoding, path ) { return this.glg_handle._LoadWidget( data, encoding, path ); }
tproto.LoadObjectFromURL = function( url, encoding, js_callback, user_data, js_abort_func, rebind_ref ) { this.glg_handle._LoadObjectFromURL( url, encoding, js_callback, user_data, js_abort_func, rebind_ref ); }
tproto.LoadWidgetFromURL = function( url, encoding, js_callback, user_data, js_abort_func ) { this.glg_handle._LoadWidgetFromURL( url, encoding, js_callback, user_data, js_abort_func ); }
tproto.LoadWidgetFromObject = function( glg_object ) { return this.glg_handle._LoadWidgetFromObject( glg_object ); }
tproto.CreateIndexedName = function( template_name, resource_index ) { return this.glg_handle._CreateIndexedName( template_name, resource_index ); }
tproto.ConcatResNames = function( resource_name1, resource_name2 ) { return this.glg_handle._ConcatResNames( resource_name1, resource_name2 ); }
tproto.Error = function( error_type, message, e ) { this.glg_handle._Error( error_type, message, e ); }
tproto.Init = function( ) { this.glg_handle._Init( ); }
tproto.Terminate = function( ) { this.glg_handle._Terminate( ); }
tproto.SetLocale = function( locale ) { this.glg_handle._SetLocale( locale ); }
tproto.Rand = function( low, high ) { return this.glg_handle._Rand( low, high ); }
tproto.PrintfD = function( format, value ) { return this.glg_handle._PrintfD( format, value ); }
tproto.PrintfI = function( format, value ) { return this.glg_handle._PrintfI( format, value ); }
tproto.PrintfS = function( format, value ) { return this.glg_handle._PrintfS( format, value ); }
tproto.SetEditMode = function( viewport, res_name, edit_mode ) { return this.glg_handle._SetEditMode( viewport, res_name, edit_mode ); }
tproto.GlmConvert = function( projection, stretch, coord_type, coord_to_lat_lon, center, extent, angle, min_x, max_x, min_y, max_y, in_point, out_point ) { this.glg_handle._GlmConvert( projection, stretch, coord_type, coord_to_lat_lon, center, extent, angle, min_x, max_x, min_y, max_y, in_point, out_point ); }
tproto.CreateSelectionNames = function( top_vp, rectangle, selected_vp ) { return this.glg_handle._CreateSelectionNames( top_vp, rectangle, selected_vp ); }
tproto.CreateSelectionNamesFromEvent = function( x, y, delta, top_vp, selected_vp ) { return this.glg_handle._CreateSelectionNamesFromEvent( x, y, delta, top_vp, selected_vp ); }
tproto.CreateSelectionMessage = function( top_vp, rectangle, selected_vp, selection_type, button ) { return this.glg_handle._CreateSelectionMessage( top_vp, rectangle, selected_vp, selection_type, button ); }
tproto.SetTooltipFormatter = function( formatter ) { return this.glg_handle._SetTooltipFormatter( formatter ); }
tproto.IsDemo = function( ) { return this.glg_handle._IsDemo( ); }
tproto.GetStackTraceAsString = function( ) { return this.glg_handle._GetStackTraceAsString( ); }
tproto.GetSelectedPlot = function( ) { return this.glg_handle._GetSelectedPlot( ); }
tproto.GetModifierState = function( modifier ) { return this.glg_handle._GetModifierState( modifier ); }
tproto.SetTouchMode = function( ) { this.glg_handle._SetTouchMode( ); }
tproto.GetTouchMode = function( ) { return this.glg_handle._GetTouchMode( ); }
tproto.GetPendingInstances = function( ) { return this.glg_handle._GetPendingInstances( ); }
tproto.GetPendingTemplates = function( ) { return this.glg_handle._GetPendingTemplates( ); }
tproto.AddGlobalListener = function( type, callback ) { this.glg_handle._AddGlobalListener( type, callback ); }
tproto.SetCanvasScale = function( scale, text_scale, native_text_scale ) { this.glg_handle._SetCanvasScale( scale, text_scale, native_text_scale ); }
oproto.SetResourceObject = function( resource_name, value ) { return this.glg_handle._SetResourceObject( this.obj, resource_name, value ); }
tproto.CreateObject = function( type, param1, param2, param3, param4 ) { return this.glg_handle._CreateObject( type, param1, param2, param3, param4 ); }
tproto.ConvertViewportType = function( obj ) { return this.glg_handle._ConvertViewportType( obj ); }
tproto.RootToScreenCoord = function( viewport, root_coord ) { return this.glg_handle._RootToScreenCoord( viewport, root_coord ); }
tproto.CreateSelection = function( top_vp, rectangle, selected_vp ) { return this.glg_handle._CreateSelection( top_vp, rectangle, selected_vp ); }
tproto.CreateSelectionFromEvent = function( x, y, delta, top_vp, selected_vp ) { return this.glg_handle._CreateSelectionFromEvent( x, y, delta, top_vp, selected_vp ); }
tproto.TranslatePointOrigin = function( from_viewport, to_viewport, point ) { this.glg_handle._TranslatePointOrigin( from_viewport, to_viewport, point ); }
tproto.CreateDataSample = function( value, time_stamp, valid, marker_visibility ) { return this.glg_handle._CreateDataSample( value, time_stamp, valid, marker_visibility ); }
tproto.AddDataSample = function( plot, datasample ) { this.glg_handle._AddDataSample( plot, datasample ); }
tproto.TraceObject = function( object, state, is_widget, top_parent, func ) { return this.glg_handle._TraceObject( object, state, is_widget, top_parent, func ); }
tproto.IHTerminate = function( ) { this.glg_handle._IHTerminate( ); }
tproto.IHGetCurrIH = function( ) { this.glg_handle._IHGetCurrIH( ); }
tproto.IHGetPrevIH = function( ) { this.glg_handle._IHGetPrevIH( ); }
tproto.IHGlobalData = function( ) { this.glg_handle._IHGlobalData( ); }
tproto.IHGetType = function( call_event ) { return this.glg_handle._IHGetType( call_event ); }
tproto.IHGetToken = function( call_event ) { return this.glg_handle._IHGetToken( call_event ); }
tproto.IHInstall = function( interface_handler ) { return this.glg_handle._IHInstall( interface_handler ); }
tproto.IHInstallAsInterface = function( js_function ) { return this.glg_handle._IHInstallAsInterface( js_function ); }
tproto.IHStart = function( ) { this.glg_handle._IHStart( ); }
tproto.IHResetup = function( ih ) { this.glg_handle._IHResetup( ih ); }
tproto.IHUninstall = function( ) { this.glg_handle._IHUninstall( ); }
tproto.IHUninstallWithToken = function( token ) { this.glg_handle._IHUninstallWithToken( token ); }
tproto.IHUninstallWithEvent = function( call_event ) { this.glg_handle._IHUninstallWithEvent( call_event ); }
tproto.IHGetHandlerInterface = function( ih ) { return this.glg_handle._IHGetHandlerInterface( ih ); }
tproto.IHGetHandlerInterfaceFunction = function( handler_interface ) { return this.glg_handle._IHGetHandlerInterfaceFunction( handler_interface ); }
tproto.IHGetPrevHandlerInterface = function( ) { return this.glg_handle._IHGetPrevHandlerInterface( ); }
tproto.IHPassToken = function( handler_interface, token, uninstall ) { this.glg_handle._IHPassToken( handler_interface, token, uninstall ); }
tproto.IHCallCurrIH = function( call_event ) { return this.glg_handle._IHCallCurrIH( call_event ); }
tproto.IHCallCurrIHWithToken = function( token ) { return this.glg_handle._IHCallCurrIHWithToken( token ); }
tproto.IHCallCurrIHWithModifToken = function( call_event, token ) { return this.glg_handle._IHCallCurrIHWithModifToken( call_event, token ); }
tproto.IHCallPrevIHWithToken = function( token ) { this.glg_handle._IHCallPrevIHWithToken( token ); }
tproto.IHCallPrevIHWithModifToken = function( call_event, token ) { this.glg_handle._IHCallPrevIHWithModifToken( call_event, token ); }
tproto.IHSetIParameter = function( ih, name, value ) { this.glg_handle._IHSetIParameter( ih, name, value ); }
tproto.IHSetBParameter = function( ih, name, value ) { this.glg_handle._IHSetBParameter( ih, name, value ); }
tproto.IHSetNParameter = function( ih, name, value ) { this.glg_handle._IHSetNParameter( ih, name, value ); }
tproto.IHSetSParameter = function( ih, name, value ) { this.glg_handle._IHSetSParameter( ih, name, value ); }
tproto.IHSetOParameter = function( ih, name, value ) { this.glg_handle._IHSetOParameter( ih, name, value ); }
tproto.IHSetDParameter = function( ih, name, value ) { this.glg_handle._IHSetDParameter( ih, name, value ); }
tproto.IHSetOParameterFromD = function( ih, name, value ) { this.glg_handle._IHSetOParameterFromD( ih, name, value ); }
tproto.IHSetOParameterFromGPoint = function( ih, name, value ) { this.glg_handle._IHSetOParameterFromGPoint( ih, name, value ); }
tproto.IHSetOParameterFromG = function( ih, name, value1, value2, value3 ) { this.glg_handle._IHSetOParameterFromG( ih, name, value1, value2, value3 ); }
tproto.IHGetIParameter = function( ih, name ) { return this.glg_handle._IHGetIParameter( ih, name ); }
tproto.IHGetBParameter = function( ih, name ) { return this.glg_handle._IHGetBParameter( ih, name ); }
tproto.IHGetNParameter = function( ih, name ) { return this.glg_handle._IHGetNParameter( ih, name ); }
tproto.IHGetSParameter = function( ih, name ) { return this.glg_handle._IHGetSParameter( ih, name ); }
tproto.IHGetOParameter = function( ih, name ) { return this.glg_handle._IHGetOParameter( ih, name ); }
tproto.IHGetDParameter = function( ih, name ) { return this.glg_handle._IHGetDParameter( ih, name ); }
tproto.IHGetOptIParameter = function( ih, name, default_value ) { return this.glg_handle._IHGetOptIParameter( ih, name, default_value ); }
tproto.IHGetOptBParameter = function( ih, name, default_value ) { return this.glg_handle._IHGetOptBParameter( ih, name, default_value ); }
tproto.IHGetOptNParameter = function( ih, name, default_value ) { return this.glg_handle._IHGetOptNParameter( ih, name, default_value ); }
tproto.IHGetOptSParameter = function( ih, name, default_value ) { return this.glg_handle._IHGetOptSParameter( ih, name, default_value ); }
tproto.IHGetOptOParameter = function( ih, name, default_value ) { return this.glg_handle._IHGetOptOParameter( ih, name, default_value ); }
tproto.IHGetOptDParameter = function( ih, name, default_value ) { return this.glg_handle._IHGetOptDParameter( ih, name, default_value ); }
tproto.IHChangeIParameter = function( ih, name, value ) { this.glg_handle._IHChangeIParameter( ih, name, value ); }
tproto.IHChangeBParameter = function( ih, name, value ) { this.glg_handle._IHChangeBParameter( ih, name, value ); }
tproto.IHChangeNParameter = function( ih, name, value ) { this.glg_handle._IHChangeNParameter( ih, name, value ); }
tproto.IHChangeSParameter = function( ih, name, value ) { this.glg_handle._IHChangeSParameter( ih, name, value ); }
tproto.IHChangeOParameter = function( ih, name, value ) { this.glg_handle._IHChangeOParameter( ih, name, value ); }
tproto.IHChangeDParameter = function( ih, name, value ) { this.glg_handle._IHChangeDParameter( ih, name, value ); }
tproto.CreateGlgPoint = function( x, y, z ) { return this.glg_handle._CreateGlgPoint( x, y, z ); }
tproto.CreateGlgPointFromPoint = function( point ) { return this.glg_handle._CreateGlgPointFromPoint( point ); }
tproto.CopyGlgPoint = function( glg_point ) { return this.glg_handle._CopyGlgPoint( glg_point ); }
tproto.CreateGlgCube = function( point1, point2 ) { return this.glg_handle._CreateGlgCube( point1, point2 ); }
tproto.CopyGlgCube = function( cube ) { return this.glg_handle._CopyGlgCube( cube ); }
tproto.CreateGlgMatrixData = function( type, array ) { return this.glg_handle._CreateGlgMatrixData( type, array ); }
tproto.CreateGlgErrorHandler = function( error_func ) { return this.glg_handle._CreateGlgErrorHandler( error_func ); }
tproto.CreateGlgAlarmHandler = function( error_func ) { return this.glg_handle._CreateGlgAlarmHandler( error_func ); }
tproto.CreateGlgTooltipFormatter = function( formatter ) { return this.glg_handle._CreateGlgTooltipFormatter( formatter ); }
tproto.CreateGlgGISRequestObserver = function( request_update_func, request_adjustement_func, adjust_request_func ) { return this.glg_handle._CreateGlgGISRequestObserver( request_update_func, request_adjustement_func, adjust_request_func ); }
tproto.CreateGlgIHHandlerInterface = function( handler ) { return this.glg_handle._CreateGlgIHHandlerInterface( handler ); }
tproto.GetLibraryVersion = function( ) { return this.glg_handle._GetLibraryVersion( ); }
tproto.ReleaseToCache = function( object ) { this.glg_handle._ReleaseToCache( object ); }
tproto.GetReference = function( object ) { return this.glg_handle._GetReference( object ); }
oproto.SetupHierarchy = function() { this.glg_handle._SetupHierarchy( this.obj ); }
oproto.ResetHierarchy = function() { this.glg_handle._ResetHierarchy( this.obj ); }
oproto.InitialDraw = function() { this.glg_handle._InitialDraw( this.obj ); }
oproto.Update = function() { return this.glg_handle._Update( this.obj ); }
oproto.UpdateImmediately = function() { return this.glg_handle._UpdateImmediately( this.obj ); }
oproto.Reset = function() { this.glg_handle._Reset( this.obj ); }
oproto.GetObjectName = function() { return this.glg_handle._GetObjectName( this.obj ); }
oproto.GetObjectType = function() { return this.glg_handle._GetObjectType( this.obj ); }
oproto.GetDataType = function() { return this.glg_handle._GetDataType( this.obj ); }
oproto.SetDResource = function( resource_name, value ) { return this.glg_handle._SetDResource( this.obj, resource_name, value ); }
oproto.SetDResourceIf = function( resource_name, value, if_changed ) { return this.glg_handle._SetDResourceIf( this.obj, resource_name, value, if_changed ); }
oproto.GetDResource = function( resource_name ) { return this.glg_handle._GetDResource( this.obj, resource_name ); }
oproto.SetGResource = function( resource_name, g_value1, g_value2, g_value3 ) { return this.glg_handle._SetGResource( this.obj, resource_name, g_value1, g_value2, g_value3 ); }
oproto.SetGResourceIf = function( resource_name, g_value1, g_value2, g_value3, if_changed ) { return this.glg_handle._SetGResourceIf( this.obj, resource_name, g_value1, g_value2, g_value3, if_changed ); }
oproto.SetGResourceFromPoint = function( resource_name, value ) { return this.glg_handle._SetGResourceFromPoint( this.obj, resource_name, value ); }
oproto.SetGResourceFromPointIf = function( resource_name, value, if_changed ) { return this.glg_handle._SetGResourceFromPointIf( this.obj, resource_name, value, if_changed ); }
oproto.GetGResource = function( resource_name ) { return this.glg_handle._GetGResource( this.obj, resource_name ); }
oproto.SetSResource = function( resource_name, value ) { return this.glg_handle._SetSResource( this.obj, resource_name, value ); }
oproto.SetSResourceIf = function( resource_name, value, if_changed ) { return this.glg_handle._SetSResourceIf( this.obj, resource_name, value, if_changed ); }
oproto.GetSResource = function( resource_name ) { return this.glg_handle._GetSResource( this.obj, resource_name ); }
oproto.SetSResourceFromD = function( resource_name, format, value ) { return this.glg_handle._SetSResourceFromD( this.obj, resource_name, format, value ); }
oproto.SetSResourceFromDIf = function( resource_name, format, value, if_changed ) { return this.glg_handle._SetSResourceFromDIf( this.obj, resource_name, format, value, if_changed ); }
oproto.SetResourceFromObject = function( resource_name, value ) { return this.glg_handle._SetResourceFromObject( this.obj, resource_name, value ); }
oproto.SetResourceFromObjectIf = function( resource_name, value, if_changed ) { return this.glg_handle._SetResourceFromObjectIf( this.obj, resource_name, value, if_changed ); }
oproto.HasResourceObject = function( resource_name ) { return this.glg_handle._HasResourceObject( this.obj, resource_name ); }
oproto.SetDTag = function( tag_source, value, if_changed ) { return this.glg_handle._SetDTag( this.obj, tag_source, value, if_changed ); }
oproto.GetDTag = function( tag_source ) { return this.glg_handle._GetDTag( this.obj, tag_source ); }
oproto.SetGTag = function( tag_source, g_value1, g_value2, g_value3, if_changed ) { return this.glg_handle._SetGTag( this.obj, tag_source, g_value1, g_value2, g_value3, if_changed ); }
oproto.SetGTagObj = function( tag_source, value, if_changed ) { return this.glg_handle._SetGTagObj( this.obj, tag_source, value, if_changed ); }
oproto.GetGTag = function( tag_source ) { return this.glg_handle._GetGTag( this.obj, tag_source ); }
oproto.SetSTag = function( tag_source, value, if_changed ) { return this.glg_handle._SetSTag( this.obj, tag_source, value, if_changed ); }
oproto.GetSTag = function( tag_source ) { return this.glg_handle._GetSTag( this.obj, tag_source ); }
oproto.SetSTagFromD = function( tag_source, format, value, if_changed ) { return this.glg_handle._SetSTagFromD( this.obj, tag_source, format, value, if_changed ); }
oproto.HasTagSource = function( tag_source ) { return this.glg_handle._HasTagSource( this.obj, tag_source ); }
oproto.HasTagName = function( tag_name ) { return this.glg_handle._HasTagName( this.obj, tag_name ); }
oproto.CreateTagList = function( unique_tag_sources ) { return this.glg_handle._CreateTagList( this.obj, unique_tag_sources ); }
oproto.CreateAlarmList = function() { return this.glg_handle._CreateAlarmList( this.obj ); }
oproto.SetParentElement = function( element_name ) { this.glg_handle._SetParentElement( this.obj, element_name ); }
oproto.UpdateSize = function() { this.glg_handle._UpdateSize( this.obj ); }
oproto.AddListener = function( type, callback ) { this.glg_handle._AddListener( this.obj, type, callback ); }
oproto.ResetSizeConstraint = function() { return this.glg_handle._ResetSizeConstraint( this.obj ); }
oproto.ChangeObject = function( res_name ) { this.glg_handle._ChangeObject( this.obj, res_name ); }
oproto.InvalidateViewport = function( res_name, type, pass ) { this.glg_handle._InvalidateViewport( this.obj, res_name, type, pass ); }
oproto.SetZoom = function( res_name, type, value ) { return this.glg_handle._SetZoom( this.obj, res_name, type, value ); }
oproto.SetZoomMode = function( res_name, zoom_object, zoom_object_name, zoom_mode ) { return this.glg_handle._SetZoomMode( this.obj, res_name, zoom_object, zoom_object_name, zoom_mode ); }
oproto.GISGetElevation = function( res_name, layer_name, lon, lat, js_callback ) { return this.glg_handle._GISGetElevation( this.obj, res_name, layer_name, lon, lat, js_callback ); }
oproto.GISCreateSelection = function( res_name, layers, x, y, select_labels, js_callback ) { return this.glg_handle._GISCreateSelection( this.obj, res_name, layers, x, y, select_labels, js_callback ); }
oproto.GISConvert = function( res_name, coord_type, coord_to_lat_lon, in_point, out_point ) { return this.glg_handle._GISConvert( this.obj, res_name, coord_type, coord_to_lat_lon, in_point, out_point ); }
oproto.ExportStrings = function( separator1, separator2 ) { return this.glg_handle._ExportStrings( this.obj, separator1, separator2 ); }
oproto.ImportStrings = function( data, verbose ) { return this.glg_handle._ImportStrings( this.obj, data, verbose ); }
oproto.ExportTags = function( separator1, separator2 ) { return this.glg_handle._ExportTags( this.obj, separator1, separator2 ); }
oproto.ImportTags = function( data, verbose ) { return this.glg_handle._ImportTags( this.obj, data, verbose ); }
oproto.SendMessageToObject = function( res_name, message, param1, param2, param3, param4 ) { return this.glg_handle._SendMessageToObject( this.obj, res_name, message, param1, param2, param3, param4 ); }
oproto.GetNamedPlot = function( res_name, plot_name ) { return this.glg_handle._GetNamedPlot( this.obj, res_name, plot_name ); }
oproto.DeletePlot = function( res_name, plot ) { return this.glg_handle._DeletePlot( this.obj, res_name, plot ); }
oproto.AddPlot = function( res_name, plot ) { return this.glg_handle._AddPlot( this.obj, res_name, plot ); }
oproto.AddTimeLine = function( res_name, time_line, time_stamp ) { return this.glg_handle._AddTimeLine( this.obj, res_name, time_line, time_stamp ); }
oproto.DeleteTimeLine = function( res_name, time_line, time_stamp ) { return this.glg_handle._DeleteTimeLine( this.obj, res_name, time_line, time_stamp ); }
oproto.AddAnnotation = function( res_name, annotation, position_x, position_y, add_box ) { return this.glg_handle._AddAnnotation( this.obj, res_name, annotation, position_x, position_y, add_box ); }
oproto.DeleteAnnotation = function( res_name, annotation, position_x, position_y ) { return this.glg_handle._DeleteAnnotation( this.obj, res_name, annotation, position_x, position_y ); }
oproto.ClearDataBuffer = function( res_name ) { return this.glg_handle._ClearDataBuffer( this.obj, res_name ); }
oproto.SetLinkedAxis = function( res_name, axis_object, axis_res_name ) { return this.glg_handle._SetLinkedAxis( this.obj, res_name, axis_object, axis_res_name ); }
oproto.GetDataExtent = function( res_name, time ) { return this.glg_handle._GetDataExtent( this.obj, res_name, time ); }
oproto.SetLabelFormatter = function( res_name, formatter ) { return this.glg_handle._SetLabelFormatter( this.obj, res_name, formatter ); }
oproto.SetFocus = function( res_name ) { this.glg_handle._SetFocus( this.obj, res_name ); }
oproto.GetNativeComponent = function( res_name, type ) { return this.glg_handle._GetNativeComponent( this.obj, res_name, type ); }
oproto.GetSize = function() { return this.glg_handle._GetSize( this.obj ); }
oproto.GetElement = function( index ) { return this.glg_handle._GetElement( this.obj, index ); }
oproto.GetResource = function( resource_name ) { return this.glg_handle._GetResource( this.obj, resource_name ); }
oproto.CopyObject = function() { return this.glg_handle._CopyObject( this.obj ); }
oproto.CloneObject = function( clone_type ) { return this.glg_handle._CloneObject( this.obj, clone_type ); }
oproto.SetElement = function( index, new_object ) { return this.glg_handle._SetElement( this.obj, index, new_object ); }
oproto.AddObjectToTop = function( obj ) { return this.glg_handle._AddObjectToTop( this.obj, obj ); }
oproto.AddObjectToBottom = function( obj ) { return this.glg_handle._AddObjectToBottom( this.obj, obj ); }
oproto.AddObjectAt = function( obj, index ) { return this.glg_handle._AddObjectAt( this.obj, obj, index ); }
oproto.AddObject = function( object, access_type, pos_modifier ) { return this.glg_handle._AddObject( this.obj, object, access_type, pos_modifier ); }
oproto.DeleteTopObject = function() { return this.glg_handle._DeleteTopObject( this.obj ); }
oproto.DeleteBottomObject = function() { return this.glg_handle._DeleteBottomObject( this.obj ); }
oproto.DeleteThisObject = function( obj ) { return this.glg_handle._DeleteThisObject( this.obj, obj ); }
oproto.DeleteObjectAt = function( index ) { return this.glg_handle._DeleteObjectAt( this.obj, index ); }
oproto.DeleteObject = function( access_type, pos_modifier, replace, replace_obj ) { return this.glg_handle._DeleteObject( this.obj, access_type, pos_modifier, replace, replace_obj ); }
oproto.SetXform = function( xform ) { return this.glg_handle._SetXform( this.obj, xform ); }
oproto.Flush = function( size ) { this.glg_handle._Flush( this.obj, size ); }
oproto.SaveObject = function( encoding ) { return this.glg_handle._SaveObject( this.obj, encoding ); }
oproto.ContainsObject = function( obj ) { return this.glg_handle._ContainsObject( this.obj, obj ); }
oproto.GetNamedObject = function( name ) { return this.glg_handle._GetNamedObject( this.obj, name ); }
oproto.GetIndex = function( obj ) { return this.glg_handle._GetIndex( this.obj, obj ); }
oproto.GetStringIndex = function( search_string ) { return this.glg_handle._GetStringIndex( this.obj, search_string ); }
oproto.SetStart = function() { this.glg_handle._SetStart( this.obj ); }
oproto.Iterate = function() { return this.glg_handle._Iterate( this.obj ); }
oproto.Inverse = function() { this.glg_handle._Inverse( this.obj ); }
oproto.ReorderElement = function( current_index, new_index ) { return this.glg_handle._ReorderElement( this.obj, current_index, new_index ); }
oproto.SetTemplate = function( template ) { this.glg_handle._SetTemplate( this.obj, template ); }
oproto.SetResourceExact = function( resource_name, type, value, if_changed, is_attr ) { return this.glg_handle._SetResourceExact( this.obj, resource_name, type, value, if_changed, is_attr ); }
oproto.GetResourceExact = function( resource_name, type, mand, is_attr, dont_hash ) { return this.glg_handle._GetResourceExact( this.obj, resource_name, type, mand, is_attr, dont_hash ); }
oproto.GetResourceObject = function( resource_name ) { return this.glg_handle._GetResourceObject( this.obj, resource_name ); }
oproto.SetResource = function( resource_name, value ) { return this.glg_handle._SetResource( this.obj, resource_name, value ); }
oproto.GetTagObject = function( search_string, by_name, unique_tags, single_tag, tag_type_mask ) { return this.glg_handle._GetTagObject( this.obj, search_string, by_name, unique_tags, single_tag, tag_type_mask ); }
oproto.GetAlarmObject = function( alarm_label, single_alarm ) { return this.glg_handle._GetAlarmObject( this.obj, alarm_label, single_alarm ); }
oproto.HasTag = function( tag_name, tag_type_mask ) { return this.glg_handle._HasTag( this.obj, tag_name, tag_type_mask ); }
oproto.QueryTags = function( tag_type_mask ) { return this.glg_handle._QueryTags( this.obj, tag_type_mask ); }
oproto.ConstrainObject = function( to_attribute ) { return this.glg_handle._ConstrainObject( this.obj, to_attribute ); }
oproto.UnconstrainObject = function() { return this.glg_handle._UnconstrainObject( this.obj ); }
oproto.UnconstrainObjectTyped = function( clone_type ) { return this.glg_handle._UnconstrainObjectTyped( this.obj, clone_type ); }
oproto.SuspendObject = function() { return this.glg_handle._SuspendObject( this.obj ); }
oproto.ReleaseObject = function( suspend_info ) { this.glg_handle._ReleaseObject( this.obj, suspend_info ); }
oproto.GetNumParents = function() { return this.glg_handle._GetNumParents( this.obj ); }
oproto.GetParent = function() { return this.glg_handle._GetParent( this.obj ); }
oproto.GetBox = function() { return this.glg_handle._GetBox( this.obj ); }
oproto.GetParentViewport = function( heavy_weight ) { return this.glg_handle._GetParentViewport( this.obj, heavy_weight ); }
oproto.GetDrawingMatrix = function() { return this.glg_handle._GetDrawingMatrix( this.obj ); }
oproto.CreateInversedMatrix = function() { return this.glg_handle._CreateInversedMatrix( this.obj ); }
oproto.TransformPoint = function( in_point, out_point ) { this.glg_handle._TransformPoint( this.obj, in_point, out_point ); }
oproto.CreateResourceList = function( list_named_res, list_def_attr, list_aliases ) { return this.glg_handle._CreateResourceList( this.obj, list_named_res, list_def_attr, list_aliases ); }
oproto.TransformObject = function( xform, coord_type, parent ) { return this.glg_handle._TransformObject( this.obj, xform, coord_type, parent ); }
oproto.CreatePointArray = function( type ) { return this.glg_handle._CreatePointArray( this.obj, type ); }
oproto.MoveObjectBy = function( coord_type, x, y, z ) { return this.glg_handle._MoveObjectBy( this.obj, coord_type, x, y, z ); }
oproto.MoveObjectByPoint = function( coord_type, point ) { return this.glg_handle._MoveObjectByPoint( this.obj, coord_type, point ); }
oproto.MoveObject = function( coord_type, start_point, end_point ) { return this.glg_handle._MoveObject( this.obj, coord_type, start_point, end_point ); }
oproto.ScaleObject = function( coord_type, center, x, y, z ) { return this.glg_handle._ScaleObject( this.obj, coord_type, center, x, y, z ); }
oproto.RotateObject = function( coord_type, center, x, y, z ) { return this.glg_handle._RotateObject( this.obj, coord_type, center, x, y, z ); }
oproto.PositionObjectByPoint = function( coord_type, anchoring, position ) { return this.glg_handle._PositionObjectByPoint( this.obj, coord_type, anchoring, position ); }
oproto.PositionObject = function( coord_type, anchoring, x, y, z ) { return this.glg_handle._PositionObject( this.obj, coord_type, anchoring, x, y, z ); }
oproto.FitObject = function( coord_type, box, keep_ratio ) { return this.glg_handle._FitObject( this.obj, coord_type, box, keep_ratio ); }
oproto.LayoutObjects = function( sel_elem, type, distance, use_box, process_subobjects ) { return this.glg_handle._LayoutObjects( this.obj, sel_elem, type, distance, use_box, process_subobjects ); }
oproto.WorldToScreen = function( inside_vp, in_point, out_point ) { return this.glg_handle._WorldToScreen( this.obj, inside_vp, in_point, out_point ); }
oproto.ScreenToWorld = function( inside_vp, in_point, out_point ) { return this.glg_handle._ScreenToWorld( this.obj, inside_vp, in_point, out_point ); }
oproto.GetMatrixData = function( matrix_data ) { return this.glg_handle._GetMatrixData( this.obj, matrix_data ); }
oproto.SetMatrixData = function( matrix_data ) { this.glg_handle._SetMatrixData( this.obj, matrix_data ); }
oproto.PositionToValue = function( res_name, x, y, outside_x, outside_y ) { return this.glg_handle._PositionToValue( this.obj, res_name, x, y, outside_x, outside_y ); }
oproto.PositionToValueObj = function( res_name, x, y, outside_x, outside_y ) { return this.glg_handle._PositionToValueObj( this.obj, res_name, x, y, outside_x, outside_y ); }
oproto.CreateChartSelection = function( plot, x, y, dx, dy, screen_coord, include_invalid, x_priority ) { return this.glg_handle._CreateChartSelection( this.obj, plot, x, y, dx, dy, screen_coord, include_invalid, x_priority ); }
oproto.GetLegendSelection = function( x, y ) { return this.glg_handle._GetLegendSelection( this.obj, x, y ); }
oproto.CreateTooltipString = function( x, y, dx, dy, format ) { return this.glg_handle._CreateTooltipString( this.obj, x, y, dx, dy, format ); }
oproto.GetAction = function( action_type, trigger_type, button, armed_state, double_click_state, action, subaction, enabled_only ) { return this.glg_handle._GetAction( this.obj, action_type, trigger_type, button, armed_state, double_click_state, action, subaction, enabled_only ); }
oproto.IsDrawable = function() { return this.glg_handle._IsDrawable( this.obj ); }
oproto.TraverseObjects = function( action_func ) { this.glg_handle._TraverseObjects( this.obj, action_func ); }
oproto.FindMatchingObjects = function( match_type, find_parents, find_first_match, search_inside, search_drawable_only, object_type, object_name, resource_name, object_id, custom_match ) { return this.glg_handle._FindMatchingObjects( this.obj, match_type, find_parents, find_first_match, search_inside, search_drawable_only, object_type, object_name, resource_name, object_id, custom_match ); }
oproto.RequestGISZoom = function( res_name, type, value, request_observer ) { return this.glg_handle._RequestGISZoom( this.obj, res_name, type, value, request_observer ); }
oproto.RequestGISMap = function( res_name, extent_x, extent_y, center_x, center_y, angle, projection, layers, flags, request_observer ) { return this.glg_handle._RequestGISMap( this.obj, res_name, extent_x, extent_y, center_x, center_y, angle, projection, layers, flags, request_observer ); }
oproto.InstallGISRequest = function( res_name ) { return this.glg_handle._InstallGISRequest( this.obj, res_name ); }
oproto.AbortGISRequest = function( res_name ) { this.glg_handle._AbortGISRequest( this.obj, res_name ); }
oproto.GetGISRequestInfo = function( res_name ) { return this.glg_handle._GetGISRequestInfo( this.obj, res_name ); }
oproto.SetScrollbarObserver = function( res_name, request_observer ) { return this.glg_handle._SetScrollbarObserver( this.obj, res_name, request_observer ); }
 tproto. MAJOR_VERSION = 4;
 tproto. MINOR_VERSION = 0;
 tproto. COORD_MAPPING_ADJ = 0.5;
 tproto. EQUATOR_RADIUS = 6378136.0;
 tproto. POLAR_RADIUS = 6356752.0;
 tproto. GIS_OUTSIDE_VALUE = -2000.0;
 tproto. CHART_FILTER_VERSION = 2;
tproto.GlgErrorType = new function(){
   this.INTERNAL_ERROR = 1;
   this.USER_ERROR = 2;
   this.WARNING = 4;
   this.LOGGING = 5;
   this.INFO = 5;
};
tproto.GlgCallEventType = new function(){
   this.UNDEFINED_CALL_EVENT_TYPE = 0;
   this.HI_SETUP_EVENT = 1;
   this.HI_RESETUP_EVENT = 7;
   this.CLEANUP_EVENT = 4;
   this.MESSAGE_EVENT = 6;
};
tproto.GlgCompatibilityMode = new function(){
   this.PRE_2_9 = 0;
   this.PRE_3_5 = 2;
   this.LATEST_RELEASE = 0x7fffffff;
};
tproto.GlgMediumType = new function(){
   this.FILE = 0;
   this.URL = 1;
   this.STREAM = 2;
};
tproto.GlgFillType = new function(){
   this.EDGE = 1;
   this.FILL = 2;
   this.FILL_EDGE = (1 | 2);
   this.LINE_FILL = 4;
   this.EDGE_LINE_FILL = (1 | 4);
};
tproto.GlgOpenType = new function(){
   this.CLOSED = 0;
   this.OPEN = 1;
   this.OPEN_INVERSED = 2;
};
tproto.GlgArcFillType = new function(){
   this.CHORD = 0;
   this.SECTOR = 1;
   this.BAND = 2;
};
tproto.GlgArcAngleDefType = new function(){
   this.START_AND_ANGLE = 0;
   this.START_AND_END = 1;
};
tproto.GlgMarkerType = new function(){
   this.CROSS = 1;
   this.BOX = 2;
   this.FILLED_BOX = 4;
   this.CIRCLE = 8;
   this.FILLED_CIRCLE = 16;
   this.DOT = 32;
   this.DIAMOND = 64;
   this.FILLED_DIAMOND = 128;
   this.CROSS_BOX = 3;
   this.CROSS_CIRCLE = 9;
};
tproto.GlgTextType = new function(){
   this.FIXED_TEXT = 1;
   this.FIT_TO_BOX_TEXT = 2;
   this.SCROLLED_TEXT = 3;
   this.SPACED_TEXT = 4;
   this.TRUNCATED_TEXT = 5;
   this.WRAPPED_TEXT = 6;
   this.WRAPPED_TRUNCATED_TEXT = 7;
   this.AUTOSCALED_TEXT = 2;
};
tproto.GlgScalingType = new function(){
   this.NO_SCALING = 0;
   this.ZOOM_SCALING = 1;
   this.RESIZE_SCALING = 2;
   this.ZOOM_AND_RESIZE_SCALING = 3;
};
tproto.GlgOrientationType = new function(){
   this.HORIZONTAL = 0;
   this.VERTICAL = 1;
};
tproto.GlgTextDirection = new function(){
   this.HORIZONTAL_TEXT = 0;
   this.VERTICAL_TEXT = 1;
   this.VERTICAL_ROTATED_RIGHT = 2;
   this.VERTICAL_ROTATED_LEFT = 3;
};
tproto.GlgAnchoringType = new function(){
   this.HCENTER = 0;
   this.HLEFT = 1;
   this.HRIGHT = 2;
   this.VCENTER = ( 0 << 4 );
   this.VTOP = ( 1 << 4 );
   this.VBOTTOM = ( 2 << 4 );
};
tproto.GlgLineAnchoringType = new function(){
   this.LCENTER = 0;
   this.LLEFT = 1;
   this.LRIGHT = 2;
   this.LINHERIT = 4;
};
tproto.GlgAxisDirectionMask = new function(){
  this.POSITIVE_AXIS_MASK = 0x1;
  this.VERTICAL_AXIS_MASK = 0x8;
};
tproto.GlgAxisDirection = new function(){
   this.HTOP_UP = 0;
   this.HTOP_DOWN = 1;
   this.HCENTER_UP = 2;
   this.HCENTER_DOWN = 3;
   this.HBOTTOM_UP = 4;
   this.HBOTTOM_DOWN = 5;
   this.VLEFT_LEFT = 8;
   this.VLEFT_RIGHT = 9;
   this.VCENTER_LEFT = 10;
   this.VCENTER_RIGHT = 11;
   this.VRIGHT_LEFT = 12;
   this.VRIGHT_RIGHT = 13;
};
tproto.GlgTimeConversionType = new function(){
   this.LOCAL_TIME = 0;
   this.UTC_TIME = 1;
   this.RELATIVE_TIME = 2;
};
tproto.GlgAxisType = new function(){
   this.VALUE_AXIS = 0;
   this.RULER_AXIS = 1;
   this.CENTER_RULER_AXIS = 2;
   this.LOCAL_TIME_SCROLL_AXIS = 3;
   this.UTC_TIME_SCROLL_AXIS = 14;
   this.REL_TIME_SCROLL_AXIS = 4;
   this.INDEX_SCROLL_AXIS = 5;
   this.CENTER_INDEX_SCROLL_AXIS = 6;
   this.VALUE_SCROLL_AXIS = 7;
   this.RULER_SCROLL_AXIS = 8;
   this.CENTER_RULER_SCROLL_AXIS = 9;
   this.LOCAL_TIME_RULER_SCROLL_AXIS = 10;
   this.UTC_TIME_RULER_SCROLL_AXIS = 15;
   this.REL_TIME_RULER_SCROLL_AXIS = 11;
   this.INDEX_RULER_SCROLL_AXIS = 12;
   this.CENTER_INDEX_RULER_SCROLL_AXIS = 13;
   this.ABS_TIME_SCROLL_AXIS = 3;
   this.ABS_TIME_RULER_SCROLL_AXIS = 10;
};
tproto.GlgAxisOutlineFlags = new function(){
   this.AXIS_ELEM = 1;
   this.BOX_ELEM = 2;
   this.MINOR_ELEM = 4;
   this.MAJOR_ELEM = 8;
};
tproto.GlgAxisOutlineType = new function(){
   this.NO_OUTLINE = 0;
   this.AXIS_LINE = 1;
   this.LINES_MINOR = ( 1 | 4 );
   this.LINES_MAJOR = ( 1 | 8 );
   this.LINES_ALL = ( 1 | 4 | 8 );
   this.BOX_MINOR = ( 1 | 2 | 4 );
   this.BOX_MAJOR = ( 1 | 2 | 8 );
   this.BOX_ALL = ( 1 | 2 | 4 | 8 );
};
tproto.GlgLegendLayoutType = new function(){
   this.HORIZONTAL_LEGEND = 0;
   this.HORIZONTAL_WRAPPED_LEGEND = 1;
   this.VERTICAL_LEGEND = 2;
   this.VERTICAL_WRAPPED_LEGEND = 3;
};
tproto.GlgSplineType = new function(){
   this.B_SPLINE = 1;
   this.C_SPLINE = 2;
};
tproto.GlgImageType = new function(){
   this.FIXED_IMAGE = 1;
   this.SCALED_IMAGE = 2;
};
tproto.GlgProjectionType = new function(){
   this.UNDEFINED_PROJECTION = 0;
   this.RECTANGULAR_PROJECTION = 1;
   this.ORTHOGRAPHIC_PROJECTION = 2;
};
tproto.GlgGISRequestFlags = new function(){
   this.GIS_REQUEST_EXTENT = 1;
   this.GIS_REQUEST_CENTER = 2;
   this.GIS_REQUEST_ANGLE = 4;
   this.GIS_REQUEST_PROJECTION = 8;
   this.GIS_REQUEST_LAYERS = 0x10;
};
tproto.GlgGISRequestStatus = new function(){
   this.GIS_ABORT_ON_GIS_CHANGE = 1;
   this.GIS_ABORT_ON_NEW_REQUEST = 2;
   this.GIS_ABORT_OF_ZOOM_MODE = 3;
   this.GIS_ABORT_ON_LOAD_ERROR = 4;
   this.GIS_ABORT_ON_RESET = 5;
   this.GIS_ABORT_BY_API = 6;
   this.GIS_REQUEST_READY = 7;
};
tproto.GlgWaveformType = new function(){
   this.SAWTOOTH_WAVEFORM = 0;
   this.TRIANGLE_WAVEFORM = 1;
   this.CIRCULAR_WAVEFORM = 2;
   this.SINE_WAVEFORM = 3;
};
tproto.GlgPathRotationType = new function(){
   this.DONT_ROTATE_PATH = 0;
   this.ROTATE_PATH = 1;
   this.ROTATE_DONT_MOVE_PATH = 2;
};
tproto.GlgReferenceType = new function(){
   this.SUBDRAWING_REF = 1;
   this.CONTAINER_REF = 2;
   this.SUBWINDOW_REF = 3;
};
tproto.GlgStorageType = new function(){
   this.USE_INCLUDED = 0;
   this.USE_FILE = 1;
   this.USE_PALETTE = 2;
};
tproto.GlgZSortType = new function(){
   this.ZS_INHERIT = 0;
   this.ZS_YES = 1;
   this.ZS_NO = 2;
   this.ZS_BY_PARENT = 3;
   this.ZS_SPECIAL = 4;
   this.ZS_NO_GDI = 5;
};
tproto.GlgUnitsType = new function(){
   this.RELATIVE_UNITS = 0;
   this.WORLD_UNITS = 1;
   this.SCREEN_UNITS = 2;
};
tproto.GlgHistoryScrollType = new function(){
   this.WRAPPED = 0;
   this.SCROLLED = 1;
};
tproto.GlgTagType = new function(){
   this.DATA_TAG = 1;
   this.EXPORT_TAG = 2;
   this.EXPORT_DYN_TAG = 4;
};
tproto.GlgTagAccessType = new function(){
   this.INPUT_TAG = 0;
   this.INIT_ONLY_TAG = 1;
   this.OUTPUT_TAG = 2;
   this.CUSTOM_ACCESS_TAG = 8;
};
tproto.GlgLightType = new function(){
   this.NO_LIGHT = 0;
   this.FLAT_LIGHT = 1;
   this.POINT_LIGHT = 2;
};
tproto.GlgObjectType = new function(){
   this.REFERENCE = 1;
   this.DATA = 2;
   this.ATTRIBUTE = 3;
   this.MATRIX = 4;
   this.FUNCTION = 5;
   this.XFORM = 6;
   this.VECTOR = 7;
   this.POLYGON = 8;
   this.ARRAY = 9;
   this.LIST = 10;
   this.SCREEN = 11;
   this.DISPLAY = 12;
   this.VIEWPORT = 13;
   this.MARKER = 14;
   this.TEXT = 15;
   this.FONTTABLE = 16;
   this.ALIAS = 17;
   this.ARC = 18;
   this.PARALLELOGRAM = 19;
   this.SQUARE_SERIES = 20;
   this.SERIES = 21;
   this.COLORTABLE = 22;
   this.HISTORY = 23;
   this.POLYLINE = 24;
   this.POLYSURFACE = 25;
   this.FRAME = 26;
   this.FONT = 27;
   this.IMAGE = 29;
   this.CONNECTOR = 30;
   this.SPLINE = 31;
   this.RENDERING = 32;
   this.BOX_ATTR = 33;
   this.LIGHT = 34;
   this.GIS = 35;
   this.TAG = 36;
   this.ROUNDED = 37;
   this.CHART = 38;
   this.PLOT = 39;
   this.LINE_ATTR = 40;
   this.AXIS = 41;
   this.LEVEL_LINE = 42;
   this.LEGEND = 43;
   this.CHART_AREA = 44;
   this.ACTION = 45;
   this.LIGHT_VIEWPORT = 46;
   this.GROUP = 9;
};
tproto.GlgDataType = new function(){
   this.S = 1;
   this.D = 2;
   this.G = 3;
   this.O = 4;
};
tproto.GlgContainerType = new function(){
   this.GLG_OBJECT = 1;
   this.STRING = 2;
   this.INT_VALUE = 3;
   this.NATIVE_OBJECT = 10;
   this.GLG_NON_DRAWABLE_OBJECT = 6;
};
tproto.GlgCloneType = new function(){
   this.WEAK_CLONE = 0;
   this.STRONG_CLONE = 1;
   this.FULL_CLONE = 2;
   this.CONSTRAINED_CLONE = 3;
   this.SHALLOW_CLONE = 4;
};
tproto.GlgFrameType = new function(){
   this.FRAME_1D = 1;
   this.FRAME_2D = 2;
   this.FRAME_3D = 3;
   this.FREE_FRAME = 4;
};
tproto.GlgXformType = new function(){
   this.TRANSLATE_XF = 2;
   this.MATRIX_XF = 5;
   this.PATH_XF = 6;
   this.TRANSLATE_X_XF = 7;
   this.TRANSLATE_Y_XF = 8;
   this.TRANSLATE_Z_XF = 9;
   this.TRANSLATE_XYZ_XF = 10;
   this.SCALE_X_XF = 11;
   this.SCALE_Y_XF = 12;
   this.SCALE_Z_XF = 13;
   this.SCALE_XYZ_XF = 14;
   this.ROTATE_X_XF = 15;
   this.ROTATE_Y_XF = 16;
   this.ROTATE_Z_XF = 17;
   this.CONCATENATE_XF = 18;
   this.D_FORMAT_XF = 20;
   this.S_FORMAT_XF = 21;
   this.LIST_XF = 22;
   this.DIVIDE_XF = 23;
   this.TRANSFER_XF = 24;
   this.RANGE_XF = 25;
   this.THRESHOLD_XF = 26;
   this.SCALE_TRANSLATE_XF = 27;
   this.SCALE_TRANSLATE_XYZ_XF = 28;
   this.SHEAR_X_XF = 30;
   this.SHEAR_Y_XF = 31;
   this.SHEAR_Z_XF = 32;
   this.TIMER_XF = 33;
   this.BOOLEAN_XF = 35;
   this.SMAP_XF = 36;
   this.STRING_CONCAT_XF = 37;
   this.RANGE_CHECK_XF = 40;
   this.RANGE_ALARM_XF = 41;
   this.RANGE2_ALARM_XF = 42;
   this.CHANGE_ALARM_XF = 43;
   this.WORLD_OFFSET_XF = 44;
   this.SCREEN_OFFSET_XF = 45;
   this.SCALE_SX_XF = 46;
   this.SCALE_SY_XF = 47;
   this.SCALE_SZ_XF = 48;
   this.TIME_FORMAT_XF = 49;
   this.RANGE_CONVERSION_XF = 51;
   this.IDENTITY_XF = 52;
   this.LINEAR3_XF = 53;
   this.COMPARE_XF = 54;
   this.BITMASK_XF = 55;
   this.JAVA_SCRIPT_XF = 56;
   this.DMAP_XF = 57;
   this.G_FROM_D_XF = 58;
   this.D_FROM_G_XF = 59;
   this.SCREEN_FACTOR_XF = 60;
   this.SLIST_XF = 36;
};
tproto.GlgBoolXformType = new function(){
   this.BOOL0_XF = 0;
   this.BOOL1_XF = 1;
   this.BOOL2_XF = 2;
   this.BOOL3_XF = 3;
   this.BOOL4_XF = 4;
   this.BOOL5_XF = 5;
   this.BOOL6_XF = 6;
   this.BOOL7_XF = 7;
   this.BOOL8_XF = 8;
   this.BOOL9_XF = 9;
   this.BOOL10_XF = 10;
   this.BOOL11_XF = 11;
   this.BOOL12_XF = 12;
   this.BOOL13_XF = 13;
   this.BOOL14_XF = 14;
   this.BOOL15_XF = 15;
   this.BOOL16_XF = 16;
   this.BOOL17_XF = 17;
   this.BOOL18_XF = 18;
   this.BOOL19_XF = 19;
   this.BOOL20_XF = 20;
   this.BOOL21_XF = 21;
   this.BOOL22_XF = 22;
   this.BOOL23_XF = 23;
   this.BOOL24_XF = 24;
   this.BOOL25_XF = 25;
   this.BOOL26_XT = 26;
   this.BOOL27_XT = 27;
   this.BOOL28_XT = 28;
   this.BOOL29_XT = 29;
   this.BOOL30_XT = 30;
   this.BOOL31_XT = 31;
   this.BOOL32_XT = 32;
   this.BOOL33_XT = 33;
};
tproto.GlgBoolConversionType = new function(){
   this.EQUAL_ZERO_BOOL = 0;
   this.GREATER_THAN_ZERO_BOOL = 1;
   this.GREATER_THAN_HALF_BOOL = 2;
   this.ABS_GREATER_THAN_HALF_BOOL = 3;
};
tproto.GlgCompareXformType = new function(){
   this.COMPARE_EQ_XF = 0;
   this.COMPARE_NE_XF = 1;
   this.COMPARE_LT_XF = 2;
   this.COMPARE_LE_XF = 3;
   this.COMPARE_GT_XF = 4;
   this.COMPARE_GE_XF = 5;
   this.COMPARE_MIN_XF = 6;
   this.COMPARE_MAX_XF = 7;
};
tproto.GlgOffsetXformMoveFlag = new function(){
   this.CHANGE_OFFSETS = 0;
   this.MOVE_ANCHOR_POINT = 1;
};
tproto.GlgRole = new function(){
   this.UNDEFINED_XR = 0;
   this.GEOM_XR = 1;
   this.COLOR_XR = 2;
   this.RESERVED_XR_1 = 3;
   this.RESERVED_XR_2 = 4;
   this.RESERVED_XR_3 = 5;
   this.RESERVED_XR_4 = 6;
   this.GDATA_XR = 7;
   this.DDATA_XR = 8;
   this.SDATA_XR = 9;
};
tproto.GlgMoveMode = new function(){
   this.MOVE_POINTS = 0;
   this.MOVE_BY_XFORM = 1;
   this.STICKY_CENTER_MODE = 2;
};
tproto.GlgGlobalType = new function(){
   this.LOCAL = 0;
   this.SEMI_GLOBAL = 1;
   this.GLOBAL = 2;
   this.UNCONSTRAINED = 3;
   this.BOUND = 4;
};
tproto.GlgCallbackType = new function(){
   this.INPUT_CB = 1;
   this.SELECT_CB = 2;
   this.TRACE_CB = 3;
   this.READY_CB = 4;
   this.H_CB = 5;
   this.V_CB = 6;
   this.TRACE2_CB = 8;
   this.HIERARCHY_CB = 9;
   this.TEMPLATE_LOAD_CB = 10;
};
tproto.GlgHierarchyCallbackType = new function(){
   this.BEFORE_SETUP_CB = 1;
   this.AFTER_SETUP_CB = 0;
};
tproto.GlgWidgetType = new function(){
   this.DRAWING_AREA_WIDGET = 0;
   this.PUSH_BUTTON_WIDGET = 1;
   this.DRAWN_BUTTON_WIDGET = 2;
   this.TOGGLE_BUTTON_WIDGET = 3;
   this.MAIN_WINDOW_WIDGET = 4;
   this.BULLETIN_WIDGET = 5;
   this.FORM_WIDGET = 6;
   this.ROW_COLUMN_WIDGET = 7;
   this.LEFT_ARROW_WIDGET = 8;
   this.RIGHT_ARROW_WIDGET = 9;
   this.UP_ARROW_WIDGET = 10;
   this.DOWN_ARROW_WIDGET = 11;
   this.HORIZONTAL_SCALE_WIDGET = 12;
   this.VERTICAL_SCALE_WIDGET = 13;
   this.HORIZONTAL_SCROLL_WIDGET = 14;
   this.VERTICAL_SCROLL_WIDGET = 15;
   this.MENU_BAR_WIDGET = 16;
   this.MS_FRAME_WIDGET = 18;
   this.TEXT_WIDGET = 19;
   this.TEXT_EDIT_WIDGET = 26;
   this.LABEL_WIDGET = 20;
   this.OPTION_MENU_WIDGET = 21;
   this.PULL_DOWN_MENU_WIDGET = 22;
   this.VERTICAL_SEPARATOR_WIDGET = 23;
   this.HORIZONTAL_SEPARATOR_WIDGET = 24;
   this.LIST_WIDGET = 25;
   this.MULTI_LIST_WIDGET = 27;
   this.EXT_LIST_WIDGET = 28;
   this.DIALOG_AREA_WIDGET = 29;
};
tproto.GlgShellType = new function(){
   this.NO_TOP_SHELL = 0;
   this.DIALOG_SHELL = 1;
   this.APPLICATION_SHELL = 2;
};
tproto.GlgStretchType = new function(){
   this.NO_STRETCH = 0;
   this.RESIZE_STRETCH = 1;
   this.RESIZE_AND_ZOOM_STRETCH = ( 1 | 2 );
};
tproto.GlgZoomMode = new function(){
   this.DRAWING_ZOOM_MODE = 0;
   this.GIS_ZOOM_MODE = 1;
   this.CHART_ZOOM_MODE = 2;
};
tproto.GlgZoomState = new function(){
   this.NO_ZOOM_STATE = 0;
   this.PAN_X_STATE = 1;
   this.PAN_Y_STATE = 2;
   this.PAN_DRAG_STATE = 4;
   this.ZOOM_TO_STATE = 8;
};
tproto.GlgModifierType = new function(){
   this.SHIFT_MOD = 1;
   this.CONTROL_MOD = 2;
   this.DOUBLE_CLICK_MOD = 3;
};
tproto.GlgLabelType = new function(){
   this.TICK_LABEL_TYPE = 0;
   this.SELECTION_LABEL_TYPE = 1;
};
tproto.GlgValueType = new function(){
   this.NUMERICAL_VALUE = 0;
   this.TIME_VALUE = 1;
};
tproto.GlgTriggerType = new function(){
   this.NO_TRIGGER = 0;
   this.MOUSE_CLICK_TRIGGER = 1;
   this.MOUSE_OVER_TRIGGER = 2;
   this.INPUT_TRIGGER = 3;
};
tproto.GlgActionType = new function(){
   this.NO_ACTION = 0;
   this.SEND_COMMAND_ACTION = 2;
   this.SEND_EVENT_ACTION = 3;
   this.TRACE_STATE_ACTION = 4;
   this.SET_STATE_ACTION = 5;
   this.RESET_STATE_ACTION = 6;
   this.TOGGLE_STATE_ACTION = 7;
};
tproto.GlgProcessArmedType = new function(){
   this.ARMED_NONE = 0;
   this.ARMED_ONLY = 1;
   this.UNARMED_ONLY = 2;
   this.ARMED_AND_UNARMED = 3;
   this.ARMED_AND_DISARMED = 3;
};
tproto.GlgArmedStateType = new function(){
   this.ANY_ARMED_STATE = 0;
   this.ARMED_ONLY_STATE = 1;
   this.UNARMED_ONLY_STATE = 2;
   this.SKIP_ARMED_ONLY_STATE = 3;
   this.SKIP_UNARMED_ONLY_STATE = 4;
};
tproto.GlgProcessDoubleClickType = new function(){
   this.DOUBLE_CLICK_NONE = 0;
   this.DOUBLE_CLICK_ONLY = 1;
   this.SINGLE_CLICK_ONLY = 2;
   this.SINGLE_AND_DOUBLE_CLICK = 3;
};
tproto.GlgDoubleClickStateType = new function(){
   this.ANY_DOUBLE_CLICK_STATE = 0;
   this.DOUBLE_CLICK_ONLY_STATE = 1;
   this.SINGLE_CLICK_ONLY_STATE = 2;
   this.SKIP_DOUBLE_CLICK_ONLY_STATE = 3;
   this.SKIP_SINGLE_CLICK_ONLY_STATE = 4;
};
tproto.GlgCoordType = new function(){
   this.SCREEN_COORD = 0;
   this.PARENT_COORD = 3;
   this.OBJECT_COORD = 4;
};
tproto.GlgScaleAdjustmentType = new function(){
   this.WORLD_SCALING = 0;
   this.SCREEN_SCALING = 1;
   this.INVERSED_WORLD_SCALING = 3;
   this.INVERSED_SCREEN_SCALING = 4;
};
tproto.GlgControlPointType = new function(){
   this.DEFAULT_POINT_TYPE = 0;
};
tproto.GlgDrawingCoordSystem = new function(){
   this.WORLD_COORD_SYSTEM = 0;
   this.SCREEN_COORD_SYSTEM = 1;
   this.FLIPPED_SCREEN_COORD_SYSTEM = 2;
   this.SCREEN_CENTER_COORD_SYSTEM = 3;
};
tproto.GlgObjectCoordSystem = new function(){
   this.INHERIT_COORD_SYSTEM = 0;
   this.ABS_SCREEN_COORD_SYSTEM = 1;
   this.ABS_FLIPPED_SCREEN_COORD_SYSTEM = 2;
   this.LVP_SCREEN_COORD_SYSTEM = 4;
   this.LVP_FLIPPED_SCREEN_COORD_SYSTEM = 5;
};
tproto.GlgPanType = new function(){
   this.NO_PAN = 0;
   this.PAN_X = 1;
   this.PAN_Y = 2;
   this.PAN_XY = 3;
   this.PAN_X_AUTO = 4;
   this.PAN_Y_AUTO = 8;
   this.PAN_XY_AUTO = 12;
};
tproto.GlgEnableXY = new function(){
   this.DISABLE_XY = 0;
   this.ENABLE_X = 1;
   this.ENABLE_Y = 2;
   this.ENABLE_XY = 3;
};
tproto.GlgChartElemDrawOrder = new function(){
   this.GRID_FOREGROUND = 1;
   this.OUTLINE_FOREGROUND = 2;
   this.LEVEL_FOREGROUND = 4;
};
tproto.GlgPlotType = new function(){
   this.LINE_PLOT = 1;
   this.STEP_PLOT = 2;
   this.BAR_PLOT = 3;
   this.FLOATING_BAR_PLOT = 4;
   this.MARKERS_PLOT = 32;
   this.LINE_AND_MARKERS_PLOT = 33;
   this.STEP_AND_MARKERS_PLOT = 34;
};
tproto.GlgAnnotationType = new function(){
   this.LABEL_ANNOTATION = 1;
   this.MARKER_ANNOTATION = 2;
   this.LABEL_AND_MARKER_ANNOTATION = 3;
};
tproto.GlgChartFilterType = new function(){
   this.NULL_FILTER = 0;
   this.MIN_MAX_FILTER = 1;
   this.AVERAGE_FILTER = 2;
   this.DISCARD_FILTER = 3;
   this.BAR_MIN_MAX_FILTER = 4;
   this.CUSTOM_FILTER = 101;
};
tproto.GlgChartFilterRval = new function(){
   this.SKIP_DATA = 0;
   this.USE_DATA1 = 2;
   this.USE_DATA2 = 3;
};
tproto.GlgExtDataType = new function(){
   this.NO_EXT_DATA = 0;
   this.MARKER_EXT_DATA = 1;
   this.BAR_EXT_DATA = 2;
};
tproto.GlgParallType = new function(){
   this.PA_PA = 0;
   this.RECT_PA = 1;
};
tproto.GlgGradientType = new function(){
   this.NO_GRADIENT = 0;
   this.LINEAR = 2;
   this.INVERSED_LINEAR = 3;
   this.LINEAR2 = 4;
   this.INVERSED_LINEAR2 = 5;
   this.CONICAL = 6;
   this.INVERSED_CONICAL = 7;
   this.CONICAL_ABS = 12;
   this.INVERSED_CONICAL_ABS = 13;
   this.SPHERICAL = 8;
   this.INVERSED_SPHERICAL = 9;
   this.ELLIPTICAL = 10;
   this.INVERSED_ELLIPTICAL = 11;
   this.LINE_WIDTH_GRADIENT = 256;
   this.INVERSED_LINE_WIDTH_GRADIENT = ( 256 + 512 );
};
tproto.GlgArrowType = new function(){
   this.NO_ARROW = 0;
   this.START_ARROW = 17;
   this.END_ARROW = 18;
   this.START_END_ARROW = 19;
   this.MIDDLE_ARROW = 20;
   this.MIDDLE_INVERSED_ARROW = 21;
   this.START_FILL_ARROW = 33;
   this.END_FILL_ARROW = 34;
   this.START_END_FILL_ARROW = 35;
   this.MIDDLE_FILL_ARROW = 36;
   this.MIDDLE_INVERSED_FILL_ARROW = 37;
};
tproto.GlgArrowPositionMask = new function(){
   this.ARROW_POSITION_MASK = 0x0f;
   this.ARROW_TYPE_MASK = 0xf0;
};
tproto.GlgArrowPositionFlags = new function(){
   this.START_APOS = 1;
   this.END_APOS = 2;
   this.START_END_APOS = 3;
   this.MIDDLE_APOS = 4;
   this.MIDDLE_INVERSED_APOS = 5;
};
tproto.GlgArrowFillTypeFlags = new function(){
   this.LINE_ATYPE = 1;
   this.FILL_ATYPE = 2;
   this.FILL_EDGE_ATYPE = 3;
};
tproto.GlgConvexType = new function(){
   this.AUTO_CONVEX = 0;
   this.CONVEX = 1;
   this.NON_CONVEX = 2;
};
tproto.GlgAntiAliasingType = new function(){
   this.ANTI_ALIASING_OFF = 0;
   this.ANTI_ALIASING_INT = 1;
   this.ANTI_ALIASING_DBL = 2;
   this.ANTI_ALIASING_UNSET = 15;
};
tproto.GlgPolygonShadingType = new function(){
   this.NO_SHADING = 0;
   this.EDGE_SHADING = 1;
   this.FILL_SHADING = 2;
   this.FILL_EDGE_SHADING = 3;
};
tproto.GlgSelectionType = new function(){
   this.DEFAULT_SELECTION_TYPE = 0;
   this.SELECT_AS_FILLED = 1;
};
tproto.GlgLineAttrType = new function(){
   this.LINE_NO_FILL = 0;
   this.LINE_AND_FILL = 1;
};
tproto.GlgSelectionEventType = new function(){
   this.MOVE_SELECTION = 8;
   this.CLICK_SELECTION = 16;
   this.TOOLTIP_SELECTION = ( 2 | 64 );
};
tproto.GlgProcessMouseMask = new function(){
   this.NO_MOUSE_EVENTS = 0;
   this.MOUSE_OVER_SELECTION = 1;
   this.MOUSE_OVER_TOOLTIP = 2;
   this.MOUSE_CLICK = 4;
   this.NAMED_TOOLTIP = 8;
   this.MOUSE_MOVE_AND_CLICK = ( 1 | 4 | 2 );
   this.MOUSE_MOVE_AND_CLICK_NAMED = ( ( 1 | 4 | 2 ) | 8 );
};
tproto.GlgMBType = new function(){
   this.SINGLE_BYTE = 0;
   this.MULTI_BYTE = 1;
   this.UTF8 = 2;
};
tproto.GlgXFontType = new function(){
   this.X_FONT = 0;
   this.XFT_FONT = 1;
};
tproto.GlgLocaleType = new function(){
   this.DEFAULT_LOCALE = 0;
   this.INHERIT_LOCALE = 1;
   this.UTF8_LOCALE = 2;
   this.XFT_CHAR8_LOCALE = 3;
};
tproto.GlgLayoutType = new function(){
   this.ALIGN_LEFT = 1;
   this.ALIGN_RIGHT = 2;
   this.ALIGN_HCENTER = 3;
   this.ALIGN_TOP = 4;
   this.ALIGN_BOTTOM = 5;
   this.ALIGN_VCENTER = 6;
   this.SET_EQUAL_VSIZE = 7;
   this.SET_EQUAL_HSIZE = 8;
   this.SET_EQUAL_SIZE = 9;
   this.SET_EQUAL_VDISTANCE = 10;
   this.SET_EQUAL_HDISTANCE = 11;
   this.SET_EQUAL_VSPACE = 12;
   this.SET_EQUAL_HSPACE = 13;
   this.SET_VSIZE = 14;
   this.SET_HSIZE = 15;
   this.SET_VDISTANCE = 16;
   this.SET_HDISTANCE = 17;
   this.SET_VSPACE = 18;
   this.SET_HSPACE = 19;
   this.POSITION_LEFT = 20;
   this.POSITION_RIGHT = 22;
   this.POSITION_HCENTER = 21;
   this.POSITION_TOP = 23;
   this.POSITION_BOTTOM = 25;
   this.POSITION_VCENTER = 24;
   this.POSITION_LEFT_VIS = 26;
   this.POSITION_RIGHT_VIS = 28;
   this.POSITION_HCENTER_VIS = 27;
   this.POSITION_TOP_VIS = 29;
   this.POSITION_BOTTOM_VIS = 31;
   this.POSITION_VCENTER_VIS = 30;
   this.POSITION_CENTER_VIS = 32;
};
tproto.GlmLabelSelectionMode = new function(){
   this.GIS_LBL_SEL_NONE = 0;
   this.GIS_LBL_SEL_IN_TILE_PRECISION = 1;
   this.GIS_LBL_SEL_MAX_PRECISION = 2;
};
tproto.GlgNativeTooltipType = new function(){
   this.GLG_TOOLTIP = 0;
   this.BOX_TOOLTIP = 1;
};
tproto.GlgComponentQueryType = new function(){
   this.WIDGET_QUERY = 0;
   this.SHELL_QUERY = 1;
   this.CHILD_WIDGET_QUERY = 2;
   this.CHILD_WIDGET2_QUERY = 3;
   this.SHELL_TITLE_QUERY = 4;
   this.SHELL_CLOSE_BUTTON_QUERY = 5;
};
tproto.GlgEventType = new function(){
   this.PAINT = 0;
   this.UPDATE = 1;
   this.COMPONENT_SHOWN = 2;
   this.COMPONENT_HIDDEN = 3;
   this.COMPONENT_MOVED = 4;
   this.COMPONENT_RESIZED = 5;
   this.MOUSE_ENTERED = 6;
   this.MOUSE_EXITED = 7;
   this.MOUSE_MOVED = 8;
   this.MOUSE_WHEEL = 9;
   this.MOUSE_PRESSED = 10;
   this.MOUSE_RELEASED = 11;
   this.KEY_DOWN = 12;
   this.KEY_PRESSED = 13;
   this.KEY_UP = 14;
   this.FOCUS_LOST = 15;
   this.FOCUS_GAINED = 16;
   this.TOUCH_START = 17;
   this.TOUCH_END = 18;
   this.TOUCH_MOVED = 19;
   this.TOUCH_CANCEL = 20;
   this.LV_ENTER_NOTIFY = (( 1000 + 2000 ) + 1 );
   this.LV_LEAVE_NOTIFY = (( 1000 + 2000 ) + 2 );
   this.LV_RESIZE = (( 1000 + 2000 ) + 3 );
};
tproto.GlgObjectMatchType = new function(){
   this.OBJECT_TYPE_MATCH = 1;
   this.OBJECT_NAME_MATCH = 2;
   this.RESOURCE_MATCH = 4;
   this.OBJECT_ID_MATCH = 8;
   this.CUSTOM_MATCH = 16;
};
tproto.GlgHTTPRequestResponseType = new function(){
   this.GLG_DRAWING = 0;
   this.TEXT = 1;
   this.BLOB = 2;
   this.JSON = 3;
   this.INT8_ARRAY = 4;
   this.UINT8_ARRAY = 5;
   this.UINT8_CLAMPED_ARRAY = 6;
   this.INT16_ARRAY = 7;
   this.UINT16_ARRAY = 8;
   this.INT32_ARRAY = 9;
   this.UINT32_ARRAY = 10;
   this.FLOAT32_ARRAY = 11;
   this.FLOAT64_ARRAY = 12;
   this.RAW = 13;
};
tproto.IH_NEW = null;
tproto.IH_CURR = null;
tproto.IH_GLOBAL = null;
tproto.IHInit = function()
{
this.glg_handle._IHInit();
this.IH_GLOBAL = this.glg_handle._IHGlobalData();
}
tproto.BellExt = function( volume, freq, duration )
{
try
{
var AudioContext = window.AudioContext || window.webkitAudioContext;
var a_context = new AudioContext();
var a_oscill = a_context.createOscillator();
var a_gain = a_context.createGain();
a_oscill.connect( a_gain );
a_gain.connect( a_context.destination );
a_oscill.type = "square";
a_gain.gain.value = volume;
a_oscill.frequency.value = freq;
a_oscill.start( a_context.currentTime );
a_oscill.stop( a_context.currentTime + duration );
} catch( e ) { console.log( "Audio API is not supported in this browser." ); }
}
tproto.Bell = function(){ tproto.BellExt( 0.03, 520, 0.1 ) };
tproto.__glg_gunzip_hook__ = function( data )
{
var gunzip = new Zlib.Gunzip( data );
return gunzip.decompress();
}
this.glg_handle._SetJSHandle( this );
return this;
}
