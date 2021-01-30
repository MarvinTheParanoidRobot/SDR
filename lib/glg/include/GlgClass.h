#ifndef GlgClass_h
#define GlgClass_h

#include <stdio.h>
#include "GlgApi.h"
#ifndef _WINDOWS
#include "GlgWrapper.h"
#endif

/* Define one of the following macros to specify the API to be used:
     - GLG_CPP_STANDARD_API
     - GLG_CPP_INTERMEDIATE_API
     - GLG_CPP_EXTENDED_API     (default)
*/

/* Define CONST_CHAR_PTR as 0 for compatibility with previous releases.
*/   
#ifndef CONST_CHAR_PTR
#  define CONST_CHAR_PTR     1
#endif

#if CONST_CHAR_PTR
#  define SCONST           const
#  if !GLG_C_CONST_CHAR_PTR
     /* String casts when calling C API if it does not use CONST. */ 
#    define STRING_CAST      (char*)
#    define STRING_PTR_CAST  (char**)
#  endif
#else
#  define SCONST
#  define STRING_CAST
#  define STRING_PTR_CAST
#endif

/* For compatibility with previous releases. */
#ifndef CONST
#  define CONST SCONST
#endif

////////////////////////////////////////////////////////////////////////////
// Global alarm handler methods
////////////////////////////////////////////////////////////////////////////
typedef void 
  (*GlgAlarmHandlerCPP)( GlgObject data_object, GlgObject alarm, 
                         SCONST char * alarm_label, SCONST char * action, 
                         SCONST char * subaction, void * reserved );

GlgAlarmHandlerCPP GlgSetAlarmHandlerCPP( GlgAlarmHandlerCPP alarm_handler );

////////////////////////////////////////////////////////////////////////////
// Class: GlgSessionC
//	Provides an interface for initializing the GLG Toolkit.
//      Also provides a generic interface to a window system specific
//      event loop.
////////////////////////////////////////////////////////////////////////////

class GlgSessionC
{
 private:
   // Disallows assignments and copying
   GlgSessionC& operator= ( const GlgSessionC& object );
   GlgSessionC( GlgSessionC& object );

 public:
   GlgAppContext app_context;

   // Default constructor
   GlgSessionC( void );

   // Constructor: calls GlgInit. Is different from a default one.
   GlgSessionC( GlgBoolean initialized, GlgAppContext application_context,
                int argc = 0, char ** argv = NULL );

   // Calls GlgInit
   void Create( GlgBoolean initialized = False,
                GlgAppContext application_context = NULL,
                int argc = 0, char ** argv = NULL );

   // Destructor
   virtual ~GlgSessionC( void );

   // Returns the application context
   GlgAppContext GetAppContext( void );

   // Provides a generic interface to the event loop.
   GlgBoolean MainLoop( void );
};

////////////////////////////////////////////////////////////////////////////
// GlgObjectC
//
// This is the main class of the GLG C++ bindings. It has functionality
// of the regular and Extended API.
////////////////////////////////////////////////////////////////////////////

class GlgObjectC
{
 public:

   // An object ID of associated GLG object	
   GlgObject glg_obj;

   // Suspension info object. Is not NULL is the object was suspended for
   // editing. 
   GlgObject suspend_obj;
   
   // Default constructor: Creates a null object
   GlgObjectC( void );

   // g++ gets confused without this constructor.
   GlgObjectC( GlgObjectC * );

   // Constructor: creates a new object of the type using default attributes
   GlgObjectC( GlgObjectType type, SCONST char * name = NULL,
               GlgAnyType data1 = NULL, GlgAnyType data2 = NULL,
               GlgAnyType data3 = NULL, GlgAnyType data4 = NULL );

   // Constructor: loads an object from a file (interface to GlgLoadFromFile)
   GlgObjectC( SCONST char * filename );

   // Constructor: loads an object from a generated memory image (interface
   // to GlgLoadFromImage)
   GlgObjectC( GlgLong object_data[], GlgLong object_data_size );

   // Copy constructor: creates a copy of the object. The type of copying
   // is defined by a clone_type parameter. Default type is GLG_FULL_CLONE
   //
   GlgObjectC( GlgObjectC& object, GlgCloneType clone_type = GLG_FULL_CLONE );

   // Constructor: associates with a named resource object inside another
   // and references it (interface to GlgGetResourceObject).
   //
   GlgObjectC( GlgObjectC& object, SCONST char * res_name );

   // Constructor: creates a C++ class from GlgObject to allow assigning 
   // GlgObject to a GlgObjectC object class:
   //    GlgObjectC object = GetResourceObject( ... )
   GlgObjectC( GlgObject object );

   // Destructor: Deletes or dereferences the GLG object
   virtual ~GlgObjectC( void );

   // Type converter to GlgObject
   operator GlgObject( void );
   
   // Assignment operator: associates the GLG object and references it
   GlgObjectC& operator= ( const GlgObjectC& object );

   // Overloaded ++; --, both infix and postfix. Overloaded to ref/deref
   GlgObjectC& operator++( void );  // prefix  ++
   GlgObjectC& operator++( int );   // postfix ++
   GlgObjectC& operator--( void );  // prefix  --
   GlgObjectC& operator--( int );   // postfix --

   // Explicit ref/deref
   void Reference( void );
   void Drop( void );   

   // Saves the associated GLG object into a file.
   GlgBoolean Save( SCONST char * filename );

   // Explicit Load. Replaces the associated GLG object with the loaded one.
   // Two sets of overloaded load functions: one for loading the whole
   // drwaing or its named components, and another for extracting just the
   // "$Widget" viewport.

   // Loads an object from a file. If object_name is not NULL, extracts and
   // loads just that named resource of the object.
   //
   GlgBoolean Load( SCONST char * filename, SCONST char * object_name = NULL );

   // Loads an object from a generated memory image
   GlgBoolean Load( void * object_data, GlgLong object_data_size,
                    SCONST char * object_name = NULL );

   // Associates with an object inside another object and references it.
   // If object_name is NULL, uses the parameter object itself.
   GlgBoolean Load( GlgObjectC& object, SCONST char * object_name = NULL );

   // Loads a "$Widget" viewport from a file.
   GlgBoolean LoadWidget( SCONST char * filename );

   // Loads a "$Widget" viewport from a generated memory image.
   GlgBoolean LoadWidget( void * object_data, GlgLong object_data_size );

   // Loads a "$Widget" viewport from another object.
   GlgBoolean LoadWidget( GlgObjectC& object );

   // Dissociates and sets the GLG object ID to NULL.
   void LoadNullObject( void );

   // Replace the current object with a created object.
   void Create( GlgObjectType type, SCONST char * name = NULL,
                GlgAnyType data1 = NULL, GlgAnyType data2 = NULL,
                GlgAnyType data3 = NULL, GlgAnyType data4 = NULL );

   // Replace the current object with a copy of it.
   void Copy( GlgCloneType clone_type = GLG_FULL_CLONE );

   // Get resources
   GlgBoolean GetResource( SCONST char * res_name, double * value );  // D-type
   GlgBoolean GetResource( SCONST char * res_name, 
                           SCONST char ** s_value );                  // S-type
   GlgBoolean GetResource( SCONST char * res_name, double * x_value, 
                           double * y_value, double * z_value );     // G-type
   
   // Set resources
   GlgBoolean SetResource( SCONST char * res_name, double value );    // D-type
   GlgBoolean SetResource( SCONST char * res_name, double value, 
                           GlgBoolean if_changed );
   GlgBoolean SetResource( SCONST char * res_name, 
                           SCONST char * s_value );                   // S-type
   GlgBoolean SetResource( SCONST char * res_name, SCONST char * s_value,
                           GlgBoolean if_changed );
   GlgBoolean SetResource( SCONST char * res_name, double x_value, 
                           double y_value, double z_value );          // G-type
   GlgBoolean SetResource( SCONST char * res_name, double x_value, 
                           double y_value, double z_value, 
                           GlgBoolean if_changed );
   // Set S resource from D
   GlgBoolean SetResource( SCONST char * res_name, SCONST char * format, 
                           double value );   
   GlgBoolean SetResource( SCONST char * res_name, SCONST char * format, 
                           double value, GlgBoolean if_changed );   

   // Set resource from object
   GlgBoolean SetResource( SCONST char * res_name, GlgObjectC& object );
   GlgBoolean SetResource( SCONST char * res_name, GlgObjectC& object,
                           GlgBoolean if_changed );

   // Checks if a resource object with the specified name exists.
   GlgBoolean HasResourceObject( SCONST char * res_name );

   // Get tags
   GlgBoolean GetTag( SCONST char * tag_name, double * value );    // D-type
   GlgBoolean GetTag( SCONST char * tag_name, 
                      SCONST char ** s_value );                    // S-type
   GlgBoolean GetTag( SCONST char * tag_name, double * x_value, 
                      double * y_value, double * z_value );       // G-type
   
   // Set tags
   GlgBoolean SetTag( SCONST char * tag_name, double value, 
                      GlgBoolean if_changed );                    // D-type
   GlgBoolean SetTag( SCONST char * tag_name, SCONST char * s_value, 
                      GlgBoolean if_changed );                    // S-type
   GlgBoolean SetTag( SCONST char * tag_name, double x_value, double y_value,
                      double z_value, GlgBoolean if_changed );    // G-type
   
   GlgBoolean SetTag( SCONST char * tag_name, SCONST char * format, double value,
                      GlgBoolean if_changed );          // Set S tag from D

   // Checks for an existence of a tag source 
   GlgBoolean HasTagSource( SCONST char * tag_source );

   // Checks for an existence of a tag name 
   GlgBoolean HasTagName( SCONST char * tag_name );

   // Returns a list of tags of the object.
   // Requires an explicit Drop of the returned value.
   GlgObject CreateTagList( GlgBoolean unique_tag_sources );

   // Returns a list of alarms attached to the object.
   // Requires an explicit Drop of the returned value.
   GlgObject CreateAlarmList( void );

   // Returns an object ID of the associated GLG object
   GlgObject GetGlgObject( void );

   // Low-level object manipulation finction: sets the associated GLG object
   void SetGlgObject( GlgObject object );

   // NULL check to use after Load/GetResourceObject/etc 
   GlgBoolean IsNull( void );
   GlgBoolean operator!( void );
   
   // Returns True if the object refers to the same object ID as the parameter
   // object.
   GlgBoolean Same( GlgObjectC& object );

   // Other viewport-related GLG API functions.
   void InitialDraw( void );
   void SetupHierarchy( void );
   void ResetHierarchy( void );
   GlgBoolean Reset( void );

   // Update/Sync viewports and links
   GlgBoolean Sync( void );
   GlgBoolean Update( void );
   
   // Printing
   GlgBoolean Print( SCONST char * filename="out.ps",
                     double x=-1000., double y=-1000.,
                     double width = 2000., double height = 2000.,
                     GlgBoolean portrait = True, GlgBoolean stretch = True );

   // GLG Metafile support
   void MetaDraw( SCONST char * filename="out.meta",
                  double x=-1000., double y=-1000.,
                  double width = 2000., double height = 2000.,
                  GlgBoolean portrait = True, GlgBoolean stretch = True );

#if defined _WINDOWS && defined _AFX
   // Native windows printing, GLG controlled page layout.
   GlgBoolean Print( CDC * dc,
                     double x=-1000., double y=-1000.,
                     double width = 2000., double height = 2000.,
                     GlgBoolean portrait = True, GlgBoolean stretch = True );

   // Native window printing, windows-controlled page layout.
   void OnPrint( CDC * dc );

   // Native window printing with clipping disabled.
   void OnDrawMetafile( CDC * dc );
#endif

   // Viewport zoom interface.
   GlgBoolean SetZoom( SCONST char * res_name, GlgLong type, 
                       double value = 0. );
   GlgBoolean SetZoomMode( SCONST char * res_name, GlgObjectC * zoom_object, 
                           SCONST char * zoom_object_name, GlgZoomMode mode );
   /*** Deprecated, use SetZoomMode(). ***/
   GlgBoolean SetGISZoom( SCONST char * res_name, GlgObjectC& gis_object, 
                          SCONST char * gis_name );

   GlgBoolean SaveImage( SCONST char * res_name, SCONST char * file, 
                         GlgImageFormat format = GLG_JPEG );

   GlgBoolean SaveImageCustom( SCONST char * res_name, SCONST char * file, 
                               GlgImageFormat format, 
                               GlgLong x, GlgLong y, 
                               GlgLong width, GlgLong height, GlgLong gap );

   GlgAnyType SendMessage( SCONST char * res_name, SCONST char * message,
                           GlgAnyType param1 = NULL, GlgAnyType param2 = NULL,
                           GlgAnyType param3 = NULL, GlgAnyType param4 = NULL );

#ifdef MS_WINDOWS
   HBITMAP GenerateImage( SCONST char * res_name = NULL, 
                          HBITMAP pixmap = (HBITMAP)0 );
   HBITMAP GenerateImageCustom( SCONST char * res_name, HBITMAP pixmap, 
                                GlgLong x, GlgLong y, 
                                GlgLong width, GlgLong height, GlgLong gap );
#else
   Pixmap GenerateImage( SCONST char * res_name = NULL, 
                         Pixmap pixmap = (Pixmap)0 );
   Pixmap GenerateImageCustom( SCONST char * res_name, Pixmap pixmap,
                               GlgLong x, GlgLong y, 
                               GlgLong width, GlgLong height, GlgLong gap );
#endif

   //////////////////////////////////////////////////////////////////////////
   // Extended API
   //////////////////////////////////////////////////////////////////////////

   // Explicit GetResourceObject. The return value may be assigned to a
   // GlgObjectC class.
   GlgObject GetResourceObject( SCONST char * res_name );

   // Sets the object itself, not its value
   GlgBoolean SetResourceObject( SCONST char * res_name, GlgObjectC * object );

   // Deprecated, use the new function with "GlgObjectC *"
   GlgBoolean SetResourceObject( SCONST char * res_name, GlgObjectC& object );

   // Adds a transformation to the object
   GlgBoolean SetXform( GlgObjectC * xform );

   // Deprecated, use the new function with "GlgObjectC *"
   GlgBoolean SetXform( GlgObjectC& xform );
   
   // Gets a parent object or array of parents if num_parents > 1
   GlgObject GetParent( GlgLong * num_parents );

   // Returns 3D bounding box of an object
   GlgCube * GetBoxPtr( void );

   // Returns a drwaing matrix object used to render the object
   GlgObject GetDrawingMatrix( void );

   // Suspends/Releases object
   void SuspendObject( void );
   void ReleaseObject( void );
   
   // Returns a list of resources of the object.
   // Requires an explicit Drop of the returned value.
   GlgObject CreateResourceList( GlgBoolean list_named,
                                 GlgBoolean list_default,
                                 GlgBoolean list_aliases );

   // Finds a single tag or creates a list of tags matching the wildcard
   // and the tag type mask.
   // In the single tag mode, returns the first found tag with the tag_name.
   // The return value may be assigned to a GlgObjectC class.
   // Requires an explicit Drop of the returned value in multi-tag mode.
   GlgObject GetTagObject( SCONST char * search_string, GlgBoolean by_name, 
                          GlgBoolean unique_tags, GlgBoolean single_tag,
                          GlgTagType tag_type_mask );

   // Finds a single alarm or creates a list of alarms with the alarm label 
   // matching the wildcard. In the single alarm mode, returns the first found 
   // alarm with the matching alarm label. 
   // The return value may be assigned to a GlgObjectC class.   
   // Requires an explicit Drop of the returned value in multi-alarm mode.
   GlgObject GetAlarmObject( SCONST char * search_string, 
                             GlgBoolean single_alarm );

   // Checks for an existence of a tag with the specified name and type.
   GlgBoolean HasTag( SCONST char * tag_name, GlgTagType tag_type_mask );

   // Returns a list of all tags of the specified tag type.
     // Requires an explicit Drop of the returned value.
   GlgObject QueryTags( GlgTagType tag_type_mask );

   // Creates and returns an array containing all objects' control points.
     // Requires an explicit Drop of the returned value.
   GlgObject CreatePointArray( GlgControlPointType type );

   // Transforms the object by the xform. Changes coordinates of every 
     // object's control point.
   GlgBoolean TransformObject( GlgObjectC& xform, GlgCoordType coord_type,
                               GlgObjectC * parent = NULL );

   // Deprecated, use the new function with "GlgObjectC *"
   GlgBoolean TransformObject( GlgObjectC& xform, GlgCoordType coord_type,
                               GlgObjectC& parent );

   // Moves object by a specified x, y and z distance.
   GlgBoolean MoveObjectBy( GlgCoordType coord_type, 
                            double x, double y, double z );

   // Moves object by a specified vector.
   GlgBoolean MoveObject( GlgCoordType coord_type, 
                          GlgPoint * start_point, GlgPoint * end_point );

   // Scales object in x, y and z dimensions.
   GlgBoolean ScaleObject( GlgCoordType coord_type, GlgPoint * center,
                           double x, double y, double z );

   // Rotates object around x, y and z axis.
   GlgBoolean RotateObject( GlgCoordType coord_type, GlgPoint * center,
                            double x, double y, double z );

   // Moves object to the specified postion.
   GlgBoolean PositionObject( GlgCoordType coord_type, GlgLong anchoring,
                              double x, double y, double z );

   // Fits object to the specified bounding box.
   GlgBoolean FitObject( GlgCoordType coord_type, GlgCube * box, 
                         GlgBoolean keep_ratio );

   // Performs layout of an object or group of objects.
   GlgBoolean LayoutObjects( GlgObject sel_elem, GlgLayoutType type, 
                             double distance, GlgBoolean use_box, 
                             GlgBoolean process_subobjects );
   
   // Converts point from screen to object's world coordinates.
   GlgBoolean ScreenToWorld( GlgBoolean inside_vp,
                             GlgPoint * in_point, GlgPoint * out_point );

   // Converts point from object's world to screen coordinates.
   GlgBoolean WorldToScreen( GlgBoolean inside_vp,
                             GlgPoint * in_point, GlgPoint * out_point );

   // Attribute objects only: Constrain/Unconstrain
   GlgBoolean ConstrainObject( GlgObjectC& to_attribute );
   GlgBoolean UnconstrainObject( void );
   
   ///////////////////////////////////////////////////////////////////////
   // For matrix objects only

   // Creates and returns a new matrix
   GlgObject InverseMatrix( void );

   // Transforms point using the GLG matrix object associated with this
   // class instance.
   void TransformPoint( GlgPoint * in_point, GlgPoint * out_point );

   // Gets matrix's coeeficients
   void GetMatrixData( GlgMatrixData * matrix_data );

   // Sets matrix's coeeficients
   void SetMatrixData( GlgMatrixData * matrix_data );

   ///////////////////////////////////////////////////////////////////////
   // Container functionality (groups, viewports, polygons, etc.)
     
   // Returns True if the associated GLG object is a GLG container.
   GlgBoolean IsContainer( void );

   // Initializes container for traversing.
   void SetStart( void );

   // Returns the next subobject of the associated container object.
   GlgObject Iterate( void );

   // Returns the size of the associated container object.
   GlgLong GetSize( void );

   // Add object to container
   GlgBoolean AddToTop( GlgObjectC& object );
   GlgBoolean AddToBottom( GlgObjectC& object );
   GlgBoolean AddAt( GlgObjectC& object, GlgLong index  );
   GlgBoolean Add( GlgObjectC& object, GlgAccessType position = GLG_BOTTOM,
                   GlgPositionModifier adjust_next = GLG_AFTER );

   // Delete this object from container
   GlgBoolean DeleteTopObject( void );
   GlgBoolean DeleteBottomObject( void );
   GlgBoolean DeleteObjectAt( GlgLong index );
   GlgBoolean DeleteObject( GlgObjectC& object );
   GlgBoolean Delete( void * reserved = NULL,
                      GlgAccessType position = GLG_BOTTOM,
                      GlgPositionModifier delete_next = GLG_AFTER );

   GlgBoolean ReorderElement( GlgLong old_index, GlgLong new_index );

   // Find functions: the Find's return value may be assigned to
   // a GlgObjectC object
   GlgBoolean ContainsObject( GlgObjectC& object );
   GlgObject GetElement( GlgLong index );
   GlgObject GetNamedObject( SCONST char * name );
   GlgLong GetIndex( GlgObjectC& object );
   GlgLong GetStringIndex( SCONST char * string );

   // Deprecated
   GlgObject Find( SCONST char * name );           // Find named object
   GlgObject Find( GlgLong index );         // Find by index
   GlgObject Find( GlgObjectC& object );    // Find the object

   // Inverses the order of container's elements
   void Inverse();

   // Real-Time Chart's methods.
   GlgObject GetNamedPlot( SCONST char * res_name, SCONST char * plot_name );
   GlgObject AddPlot( SCONST char * res_name, GlgObjectC& plot );
   GlgBoolean DeletePlot( SCONST char * res_name, GlgObjectC& plot );
   GlgBoolean GetDataExtent( SCONST char * res_name, 
                             GlgMinMax * min_max, GlgBoolean pan_x );
   GlgBoolean PositionToValue( SCONST char * res_name, double x, double y, 
                               GlgBoolean outside_x, GlgBoolean outside_y, 
                               double * value );
   // Requires an explicit Drop of the returned value.
   GlgObject CreateChartSelection( GlgObjectC * plot,
                                   double x, double y, double dx, double dy,
                                   GlgBoolean screen_coord, 
                                   GlgBoolean include_invalid, 
                                   GlgBoolean x_priority );
   char * CreateTooltipString( double x, double y, double dx, double dy, 
                               SCONST char * reserved );

   // Enables selection, input, trace and hierarchy callbacks of the object.
   // If callback_viewport is not NULL, adds GLG callbacks to it, 
   // otherwise adds it to the viewport associated with the object.
   //
   void EnableCallback( GlgCallbackType callback_type,
                        GlgObject callback_viewport = NULL );

   // Virtual callback functions to be overridden by a derived class. 
   virtual void Input( GlgObjectC& callback_viewport, GlgObjectC& message );
   virtual void Select( GlgObjectC& callback_viewport, 
                        SCONST char ** name_array );
   virtual void Trace( GlgObjectC& callback_viewport, 
                       GlgTraceCBStruct * trace_info );
   virtual void Trace2( GlgObjectC& callback_viewport, 
                        GlgTraceCBStruct * trace_info );
   virtual void Hierarchy( GlgObjectC& callback_viewport, 
                           GlgHierarchyCBStruct * hierarchy_info );
};

////////////////////////////////////////////////////////////////////////////
// GlgLinkC
//
// GlgLinkC - is subclassed from GlgObjectC to inherit Set/Get Resource
// and other methods. Provides methods for establishing links between GLG
// processes by using GLG ICC. Some inherited methods of extended API are not
// implemented yet for links.
////////////////////////////////////////////////////////////////////////////

class GlgLinkC: public GlgObjectC
{
 public:
   // Default constructor: creates a null link object
   GlgLinkC( void );

   // Constructor: creates a link object and connects it to a named GLG ICC
   // Server
   GlgLinkC( SCONST char * display_name, SCONST char * server_name );

   // Constructor: creates a link from a GlgObjectClass:
   //    GlgObjectC some_object;
   //    GlgLinkC link = some_object;
   GlgLinkC( GlgObjectC& object );

   // Constructor: creates a link from a GlgObject
   GlgLinkC( GlgObject object );

   // Copy constructor. The server connection is not copied.
   GlgLinkC( GlgLinkC& object );

   // Destructor, calls GlgDestroyLink 
   virtual ~GlgLinkC( void );

   // Establish a connection to a named GLG ICC Server
   GlgBoolean Connect( SCONST char * display_name, SCONST char * server_name );

   // Explicit link closing, calls GlgDestroyLink
   void Disconnect( void );

   // Assignment operator, the server connection is not copied.
   GlgLinkC& operator= ( const GlgLinkC& object );

   // Type converter to GlgObject is inherited from GlgObjectC

   // Interface to GlgLinkActive: returns True is the link is active.
   GlgBoolean IsActive();
};

#ifdef _WINDOWS

#ifdef _AFX

/////////////////////////////////////////////////////////////////////////////
// GlgControlC
//
// MFC CWnd derived class providing a window for displaying a GLG drawing.
/////////////////////////////////////////////////////////////////////////////

class GlgControlC : public CWnd
{
   // Attributes
 public:

   // An object ID of the associated GLG viewport object with the drawing.
   GlgObjectC viewport;

 private:
   GlgBoolean got_initial_draw;
   GlgBoolean drawn;

   // Construction
 public:
   
   // Default constructor: creates an empty control
   GlgControlC( void );

   // Destructor. Destroys the window and dereferences the viewport object.
   virtual ~GlgControlC( void );

 private:
   // Disallow assignments and copying
   GlgControlC& operator= ( const GlgControlC& object );
   GlgControlC( GlgControlC& object );   

   // Operations
 public:

   // Create control's window, load drawing from a file. 
   void Create( SCONST char * drawing_file,
                CWnd * parent, SCONST char * control_name = "GlgControl",
                CRect * in_rect = NULL, int IDC_id = 0 );

   // Create control's window, load drawing from a generated memory image. 
   void Create( void * drawing_image, GlgLong image_size,
                CWnd * parent, SCONST char * control_name = "GlgControl",
                CRect * in_rect = NULL, int IDC_id = 0 );

   // Create control's window, use a "$Widget" viewport of another
   // object  as a drawing.
   void Create( GlgObjectC& object,
                CWnd * parent, SCONST char * control_name = "GlgControl",
                CRect * in_rect = NULL, int IDC_id = 0 );

   // Draw the graphics the first time, invoke H and V callbacks.
   void InitialDraw();

   // Update displayed graphics.
   GlgBoolean Update( void );

   // Setup hierarchy.
   void SetupHierarchy();

   // Reset hierarchy.
   void ResetHierarchy();

   // Change the drawing, load a new drawing from a file.
   void SetDrawing( SCONST char * filename );

   // Change the drawing, load a new drawing from a generated memory image.
   void SetDrawing( void * drawing_image, GlgLong drawing_image_size );

   // Change the drawing, use a "$Widget" viewport of another
   // object  as a new drawing.
   void SetDrawing( GlgObjectC& object );

   // Enables selection, input, trace and hierarchy callbacks of the control.
   // If callback_viewport is not NULL, adds GLG callbacks to it, 
   // otherwise adds it to the control's viewport.
   // 
   virtual void EnableCallback( GlgCallbackType callback_type,
			       GlgObject callback_viewport = NULL );

   // Virtual callback functions to be overridden by a derived class. 
   virtual void Input( GlgObjectC& callback_viewport, GlgObjectC& message );
   virtual void Select( GlgObjectC& callback_viewport, 
                        SCONST char ** name_array );
   virtual void Trace( GlgObjectC& callback_viewport,
                       GlgTraceCBStruct * trace_info );
   virtual void Trace2( GlgObjectC& callback_viewport,
                        GlgTraceCBStruct * trace_info );
   virtual void Hierarchy( GlgObjectC& callback_viewport,
                           GlgHierarchyCBStruct * hierarchy_info );

   // Invoked by the InitialDraw before hierarchy setup for setting 
   // initial resources .
   virtual void HCallback();

   // Invoked by the InitialDraw for setting initial resources 
   // after hierarchy setup, but before drawing graphics.
   virtual void VCallback();

   // Native Windows print method.
   void Print( CDC * pDC, DWORD dwFlags );

   // Returns an object ID of the control's viewport.
   GlgObject GetViewport( void );

 private:
   void DrawFirstTime();

   // Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GlgControlC)
	public:
	protected:
	//}}AFX_VIRTUAL

   // Generated message map functions
 protected:
	//{{AFX_MSG(GlgControlC)
        afx_msg virtual void OnShowWindow( BOOL bShow, UINT nStatus );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

#else

////////////////////////////////////////////////////////////////////////////
// GlgWrapperC
//
// C++ wrapper around the GlgWrapper widget. It is a subclass of a 
// GlgObjectC and inherits GLG API and GLG Extended API from it.
//
// NOTE: GetViewport method has to be called explicitly after the widget
//       has been realized and before any API functions are used. The
//       GlgGetViewport method associates a viewport object with the
//       the instance of a class. The viewport is set to NULL initially
//       and its object ID has to be obtained from the widget after the
//       widget has been realized. The viewport is created by the widget
//       when it is relaizing. Since the widget may be realized by its
//       parent, it's difficult to encapsulate GetViewport functionality
//       into the Glg Wrapper class so that it's transparent to the user.
//       Trying to do so causes a lot of code duplication and will lead
//       problem in the future, so invoking this call is left as
//       responsibility of the user.
//
////////////////////////////////////////////////////////////////////////////
class GlgWrapperC : public GlgObjectC
{
 private:
   // Disallow assignments and copying
   GlgWrapperC& operator= ( const GlgWrapperC& object );
   GlgWrapperC( GlgWrapperC& object );   

 public:
   Widget widget;
   
   // Default constructor: creates an empty wrapper widget with no drawing.
   GlgWrapperC( void );

   // Destructor. Destroys the wrapper widget. Be carefull to set the widget
   // to NULL if it's destroyed by destroying its parent to avoid
   // XtDestroyWidget being called with an invalid widget ID.
   //
   virtual ~GlgWrapperC( void );

   // Create the widget, load drawing from a file.
   void Create( SCONST char * filename, Widget parent,
                SCONST char * widget_name = "GlgWrapper",
                GlgLong width = 400, GlgLong height = 300 );

   // Create the widget, load drawing from a generated memory image.
   void Create( void * image_data, GlgLong image_data_size,
                Widget parent, SCONST char * widget_name = "GlgWrapper",
                GlgLong width = 400, GlgLong height = 300 );
	       
   // Create the widget, use a "$Widget" viewport of another
   // object  as a drawing.
   void Create( GlgObjectC& object, Widget parent,
                SCONST char * widget_name = "GlgWrapper",
                GlgLong width = 400, GlgLong height = 300 );

   // Change the drawing using a particular wrapper widget resource.
   // The actual drawing displayed is still subject to the priority of
   // a particular wrapper widget drawing resource. Setting drawing
   // using one of the SetDrawing functions may interfere with others.
   //

   // Load a new drawing from a file (uses XtNglgDrawingFile resource).
   void SetDrawing( SCONST char * filename );

   // Load a new drawing from a generated memory image
   // ( uses XtNglgDrawingImage resource).
   //
   void SetDrawing( void * image_data, GlgLong image_data_size );

   // Use a "$Widget" viewport of another object as a new drawing
   // (uses XtNglgDrawingObject resource).
   //
   void SetDrawing( GlgObjectC& object );

   // Returns a widget id.
   Widget GetWidget( void );

   // Associates a viewport object of the wrapper widget with the instance
   // of the class after the widget has been realized.
   GlgObject GetViewport();

   // Enables selection, input, trace and hierarchy callbacks of the control. 
   // If callback_viewport is not NULL, adds GLG callbacks to it, 
   // otherwise adds it to the control's viewport.
   // 
   void EnableCallback( GlgCallbackType callback_type,
                        GlgObject callback_viewport = NULL );

   // Virtual callback functions to be overridden by a derived class. 
   virtual void Input( GlgObjectC& callback_viewport, GlgObjectC& message );
   virtual void Select( GlgObjectC& callback_viewport, 
                        SCONST char ** name_array );
   virtual void Trace( GlgObjectC& callback_viewport,
                       GlgTraceCBStruct * trace_info );
   virtual void Trace2( GlgObjectC& callback_viewport,
                        GlgTraceCBStruct * trace_info );
   virtual void Hierarchy( GlgObjectC& callback_viewport,
                           GlgHierarchyCBStruct * hierarchy_info );

   // Invoked before hierarchy setup for setting initial resources .
   virtual void HCallback( GlgObjectC& callback_viewport );

   // Invoked for setting initial resources after hierarchy setup, 
   //  but before drawing graphics.
   virtual void VCallback( GlgObjectC& callback_viewport );


   // Set and get Xt resources of string type (is primarily used with
   // XtNglgHResource* and XtNglgVResource* resources).
   //
   void SetXtResource( String xt_res_name, SCONST char * res_value );
   void GetXtResource( String xt_res_name, SCONST char ** res_value_ptr );
};

void GlgWrapperHXtStub( Widget callback_widget, XtPointer client_data,
                        XtPointer call_data );
void GlgWrapperVXtStub( Widget callback_widget, XtPointer client_data,
                        XtPointer call_data );

#endif   /* _WINDOWS */

#if !defined GLG_CPP_STANDARD_API && !defined GLG_CPP_INTERMEDIATE_API && !defined GLG_CPP_EXTENDED_API
/* Compatibility with macros from previous releases */
#  ifdef GLG_DISABLE_CPLUS_EXTENDED_API
#    define GLG_CPP_STANDARD_API
#  elif defined GLG_INTERMEDIATE_API_ONLY
#    define GLG_CPP_INTERMEDIATE_API
#  else
#    define GLG_CPP_EXTENDED_API
#  endif
#endif

#ifdef GLG_CPP_STANDARD_API
#  define GLG_HAS_CPP_INT_API     0
#  define GLG_HAS_CPP_EXT_API     0
#elif defined GLG_CPP_INTERMEDIATE_API
#  define GLG_HAS_CPP_INT_API     1
#  define GLG_HAS_CPP_EXT_API     0
#elif defined GLG_CPP_EXTENDED_API
#  define GLG_HAS_CPP_INT_API     1
#  define GLG_HAS_CPP_EXT_API     1
#endif

#define GET_PTR_OBJECT( cpp_object_ptr ) \
  ( cpp_object_ptr ? cpp_object_ptr->glg_obj : NULL )

#endif /* GlgClass_h */
