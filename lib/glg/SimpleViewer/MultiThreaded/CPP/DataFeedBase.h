#pragma once

#include "viewer.h"
#include "raw_data.h"

#ifndef _WINDOWS
#include <pthread.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

// C wrapper function. Invokes virtual class member DataFeedBase::GetRawData().
extern "C"
{ 
#ifndef _WINDOWS
   // Used by pthread_create().  
   void * GetRawDataEntry( void * data );
#else
   // Used by CreateThread().
   DWORD WINAPI GetRawDataEntry( LPVOID data );
#endif
}

class GlgViewer;

// Base Data feed class, provides virtual methods.
class DataFeedBase
{
 public:
   DataFeedBase( GlgViewer * viewer );
   ~DataFeedBase( void );

   GlgViewer * Viewer;

   // A vector of BaseData structures.
   BaseDataVectorType * AccumulatedData;

   // Data acquisition thread.
#ifndef _WINDOWS
   pthread_t DataThread;
   pthread_mutex_t ThreadLock;            // Used for thread locking.
#else
   HANDLE DataThread;
   HANDLE ThreadLock;                     // Used for thread locking.      
#endif

   GlgBoolean ThreadActive;
   GlgBoolean StopDataThread;             // Used to abort a thread.

   // Virtual methods, may be overriden by the derived class. 
   virtual void Init( void );
   virtual void Terminate( void );

   // Data processing method, MUST be overriden by the derived class.
   virtual void ProcessData( void );

   void GetRawData( void );
   void StoreRawData( BaseData * data );
   void StartUpdates( void );
   void StopUpdates( void );
   BaseDataVectorType * GetAccumulatedData( BaseDataVectorType * old_gui_data ); 
   void LockThread( void );
   void UnlockThread( void );

   inline virtual GlgBoolean WriteDValue( CONST char *tag_source, double value )
   {
      return GlgTrue;
   }

   inline virtual GlgBoolean WriteSValue( CONST char *tag_source, 
                                          CONST char * str )
   {
      return GlgTrue;
   }
};
