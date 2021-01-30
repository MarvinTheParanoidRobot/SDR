#pragma once

#include "GlgClass.h"
#include "DataFeed.h"

typedef enum
{
   BUTTON_PRESS_EVENT = 0,
   MOUSE_WHEEL_EVENT,
   MOUSE_MOVE_EVENT,   
   RESIZE_EVENT
} EventType;

class GlgMapViewer : public GlgObjectC
{
 public:
   GlgMapViewer( void );
   ~GlgMapViewer( void );

   // True if simulated demo data are used; False for live data. 
   bool RANDOM_DATA;
   
   /* Used for supplying data for animation. */
   DataFeedC * DataFeed;     

   /* This GlgMapViewer class is used in both the cross-platform version
      of the example and the MFC version on Windows. The use of the has_parent 
      member makes it possible to share the class between both versions.
      has_parent=True in case of an MFC application where a parent of the class
      is a GLG MFC Control. 
   */
   bool has_parent; 
   
   // Update interval in msec
   GlgLong UpdateInterval; 

   GlgAppContext AppContext;
   GlgLong TimerID;
   
   GlgObjectC MapVp;      // Map viewport containing a GIS object
   GlgObjectC GISObject;  // GIS object

   // Store icon information. 
   IconData * Icon;
   
   // Store initial extent and center, used to reset the drawing on ZoomReset.
   GlgPoint InitExtent;
   GlgPoint InitCenter;

   virtual void Input( GlgObjectC& callback_viewport, GlgObjectC& message);
   virtual void Trace( GlgObjectC& callback_viewport, 
                       GlgTraceCBStruct * trace_info );

   void Init( void );
   void InitBeforeH( void );
   void InitAfterH( void );
   void UpdateDrawing( void );
   void SetSize( GlgLong x, GlgLong y, GlgLong width, GlgLong height );
   void StopUpdates( void );
   void StartUpdates( void );
   void AddDataFeed( DataFeedC * data_feed );
   GlgBoolean GetLatLonInfo( GlgPoint * in_point, GlgPoint * out_point );
   void ShowInfoDisplay( GlgBoolean show, GlgPoint * lat_lon );
   void GetIconData( IconData * icon );
   void SetIconVisibility( GlgObjectC& icon, GlgBoolean show );
   void PositionIcon( IconData * icon );
   GlgObject GetIconObject( CONST char * icon_name );
   int ZoomToMode( void );
   void Zoom( char type, double value );
   GlgMapViewer& operator= ( const GlgObjectC& object );
};
