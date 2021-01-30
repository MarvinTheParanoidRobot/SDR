#pragma once

#include "viewer.h"
#include "TagRecord.h"
#include "DataFeedBase.h"

class GlgViewer : public GlgObjectC
{
 public:
   GlgViewer( void );
   ~GlgViewer( void );

   // True if simulated demo data are used; False for live data. 
   GlgBoolean RANDOM_DATA;
   
   /* Used for supplying data for animation. */
   DataFeedBase * DataFeed;     

   /* Array of tag records. */
   TagRecordsType TagRecords;
   
   /* Number of tags records in TagRecords. */
   GlgLong NumTagRecords;

   /* A vector containing accumulated raw data from the data thread. */
   BaseDataVectorType * GUIData;

   /* This GlgViewer class is used in both the cross-platform version
      of the example and the MFC version on Windows. The use of the has_parent 
      member makes it possible to share the class between both versions.
      has_parent=True in case of an MFC application where a parent of the class
      is a GLG MFC Control. 
   */
   GlgBoolean has_parent; 
   
   GlgLong UpdateInterval;           // data feed and refresh interval in msec
   GlgLong TimerID;                  // TimerID for the screen update timer.
   
   GlgAppContext AppContext;

   virtual void Input( GlgObjectC& callback_viewport, GlgObjectC& message);
   void Init( void );
   void InitBeforeH( void );
   void InitAfterH( void );
   void InitTag( CONST char * tag_source );
   void CreateTagRecords( void );
   void DeleteTagRecords( void );
   void GetGUIData( void );
   void ProcessGUIData( void );
   void StopUpdates( void );
   void StartUpdates( void );
   void AddDataFeed( void );
   void AddDataFeed( DataFeedBase * data_feed );
   void DeleteDataFeed( void );

   /* Sample method to process incoming data structures.
      Application should provide a custom implementation of processing
      application specific data structures.
   */
   void PushData( BaseData * data );

   TagRecord * LookupTagRecords( SCONST char * tag_source, GlgLong data_type );
   GlgBoolean PushTagData( SCONST char * tag_source, double d_value );
   GlgBoolean PushTagData( SCONST char * tag_source, SCONST char * s_value );
   GlgBoolean PushTagData( SCONST char * tag_source, 
                           double x, double y, double z );

   GlgViewer& operator= ( const GlgObjectC& object );
};
