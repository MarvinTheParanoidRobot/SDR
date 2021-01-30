/* Base class containing methods for data acquisition. An application
   can extend DataFeedBase class and provide a custom implementation
   of these methods.
*/

#include "DataFeedBase.h"

/* C wrapper function, used by pthread_create().
   Takes a pointer to (DataFeedBase *) and invokes virtual method GetRawData()
   which may be implemented by the derived class.
*/
#ifndef _WINDOWS
void * GetRawDataEntry( void * This ) 
#else
DWORD WINAPI GetRawDataEntry( LPVOID This )
#endif
{
   ((DataFeedBase *) This)->GetRawData();
   return NULL;
}

// Constructor
DataFeedBase::DataFeedBase( GlgViewer * viewer )
{
   Viewer = viewer;
   StopDataThread = GlgFalse;
   ThreadActive = GlgFalse;
   AccumulatedData = new BaseDataVectorType();
   
   // Initialize mutex.
   bool error;
#ifndef _WINDOWS
   error = ( pthread_mutex_init( &ThreadLock, NULL ) != 0 );
#else
   ThreadLock = CreateMutex( NULL, FALSE, NULL );
   error = ( ThreadLock == NULL );
#endif

   if( error )
   {
      GlgError( GLG_USER_ERROR, "Can't create mutex.\n" );
      exit( GLG_EXIT_ERROR );
   }
}

// Destructor
DataFeedBase::~DataFeedBase( void )
{
   StopUpdates();

   BaseData::ClearVector( AccumulatedData );
   delete AccumulatedData;

#ifndef _WINDOWS
   pthread_mutex_destroy( &ThreadLock );
#else
   CloseHandle( ThreadLock );
#endif
}

/*--------------------------------------------------------------------
| This virtual method should be overriden by a derived class to 
| provide any additional custom initialization code in the derived 
| class to initialize data connectivity.
*/
void DataFeedBase::Init( void )
{
}

/*--------------------------------------------------------------------
| This virtual method should be overriden by a derived class to 
| provide any additional custom termination code in the derived 
| class to free data connectivity resources, if any.
*/
void DataFeedBase::Terminate( void )
{
}

/*--------------------------------------------------------------------
| Start data thread. In this example, it starts a data acquisition
| thread allowing to query data asynchronously, for example
| using sockets. It creates a thread (DataThread) that invokes
| GetRawData() which will obtain real-time data values and 
| store them in AccumulatedData array. 
*/
void DataFeedBase::StartUpdates( void )
{
   if( ThreadActive )       // Already active.
     return;

   /* Execute any additional custom initialization code in the derived class
      to initialize data connectivity.
   */
   Init();

   /* Create DataThread. C function GetRawDataEntry() invokes GetRawData() 
      class member function. It is a virtual function that may be overriden
      by the derived class.
   */
   bool error;
#ifndef _WINDOWS
   error = ( pthread_create( &DataThread, NULL, GetRawDataEntry, this ) != 0 );
#else
   DataThread = CreateThread( NULL, 0, GetRawDataEntry, this, 0, NULL );
   error = ( DataThread == 0 );
#endif   

   if( error )
   {
      GlgError( GLG_USER_ERROR, "Can't create DataThread.\n" );
      exit( GLG_EXIT_ERROR );
   }
   ThreadActive = GlgTrue;
}

/*--------------------------------------------------------------------
| Stop data acquisition thread.
*/
void DataFeedBase::StopUpdates( void )
{
   if( !ThreadActive )
     return;

   /* Set StopDataThread flag to true and join data thread with the main thread,
      to make sure the thread will finish execution before it gets destroyed.
   */
   StopDataThread = GlgTrue;

#ifndef _WINDOWS
   pthread_join( DataThread, NULL );
#else
   WaitForSingleObject( DataThread, INFINITE );
   CloseHandle( DataThread );
#endif

   StopDataThread = GlgFalse;
   ThreadActive = GlgFalse;

   /* Execute any additional custom termination code in the derived class
      to free data connectivity resources, if any.
   */
   Terminate();
}

/*--------------------------------------------------------------------
| Query data and store them in the AccumulatedData vector. 
*/
void DataFeedBase::GetRawData( void ) 
{
   while( !StopDataThread )
   {
      /* Process incoming data structures and store them in AccumulatedData
         vector. Application should provide a custom implementation
         of ProcessData() in a derived class.
      */
      ProcessData();
   }

   /* Clean-up when the tread is stopped.
      There is no need for locking here, because this is happening while the
      GUI thread is waiting for this thread to stop.
   */
   BaseData::ClearVector( AccumulatedData );
}

/*--------------------------------------------------------------------
| Invoked synchronously on a timer from GlgViewer::GetGUIData(). 
| Returns the data accumulated since the last call.
| For efficiency, it simply swaps the vector constaining the new data
| with the vector of old data, and use the old vector to accumulate the next
| batch of data.
| Use locking to ensure the vectors will not be accessed while being swapped.
*/
BaseDataVectorType * 
  DataFeedBase::GetAccumulatedData( BaseDataVectorType * old_gui_data )
{
   LockThread();

   /* Pass accumulated data to the caller to use for GUI updates.
      Will pass all data accumulated since the last call.
      Instead of copying, simply swap the arrays for performance and efficiency.
   */
   BaseDataVectorType * accumulated_data = AccumulatedData;

   BaseData::ClearVector( old_gui_data );
   AccumulatedData = old_gui_data;

   UnlockThread();
   return accumulated_data;
}

/*--------------------------------------------------------------------
| Add BaseData object to the AccumulatedData vector.
*/
void DataFeedBase::StoreRawData( BaseData * data )
{
   LockThread();

   AccumulatedData->push_back( data );

   UnlockThread();   
}


/*--------------------------------------------------------------------
| Must be overriden by the derived class.
*/
void DataFeedBase::ProcessData( void )
{
}

/*--------------------------------------------------------------------
|
*/
void DataFeedBase::LockThread()
{
#ifndef _WINDOWS
   pthread_mutex_lock( &ThreadLock );
#else
   WaitForSingleObject( ThreadLock, INFINITE );
#endif
}

/*--------------------------------------------------------------------
|
*/
void DataFeedBase::UnlockThread()
{
#ifndef _WINDOWS
   pthread_mutex_unlock( &ThreadLock );
#else
   ReleaseMutex( ThreadLock );
#endif
}

/*--------------------------------------------------------------------
|
*/
void BaseData::ClearVector( BaseDataVectorType * data_vector )
{
   BaseDataVectorType::iterator it;
   
   /* Free memory for the vector elements  */
   for( it = data_vector->begin(); it != data_vector->end(); ++it )
     delete *it;
   
   data_vector->clear();
}
