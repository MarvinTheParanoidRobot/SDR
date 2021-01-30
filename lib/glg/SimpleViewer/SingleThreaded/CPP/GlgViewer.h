#pragma once

#include "GlgClass.h"
#include "TagRecord.h"
#include "DataFeed.h"

class GlgViewer : public GlgObjectC
{
 public:
   GlgViewer( void );
   ~GlgViewer(void);

   // True if simulated demo data are used; False for live data. 
   bool RANDOM_DATA;
   
   /* Used for supplying data for animation. */
   DataFeedC * DataFeed;     

   /* Array of tag records. */
   TagRecordC ** TagRecordArray;
   
   /* Number of tags records in TagRecordArray. */
   GlgLong NumTagRecords;

   /* This GlgViewer class is used in both the cross-platform version
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

   virtual void Input( GlgObjectC& callback_viewport, GlgObjectC& message);
   void Init( void );
   void InitBeforeH( void );
   void InitAfterH( void );
   void InitTag( CONST char * tag_source );
   void CreateTagRecords( void );
   void DeleteTagRecords( void );
   void UpdateDrawing( void );
   void SetSize( GlgLong x, GlgLong y, GlgLong width, GlgLong height );
   void StopUpdates( void );
   void StartUpdates( void );
   void AddDataFeed( DataFeedC * data_feed );
   GlgViewer& operator= ( const GlgObjectC& object );
};
