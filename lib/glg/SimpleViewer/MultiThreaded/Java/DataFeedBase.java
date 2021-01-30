import java.lang.Thread; 
import java.util.ArrayList;

/* Base class containing methods for data acquisition. An application
   can extend DataFeedBase class and provide a custom implementation
   of these methods.
*/
public class DataFeedBase
{
   GlgViewer Viewer;
   CustomThread DataThread = null;

   boolean StopDataThread = false;        // Used to abort a thread.

   /* Accumulates data obtained from a back-end system in a data thread.
      The data get pushed to the graphics on a timer, and the array gets
      cleared after the data accumulated in between timer intervals
      have been displayed. 
   */
   ArrayList<BaseData> AccumulatedData = new ArrayList<BaseData>();

   // Methods meant to be overriden by the derived class. 
   void Init() {}
   void Terminate() {}
   void ProcessData() {} // Data processing method.

   // Write numerical value into the provided database tag. 
   boolean WriteDValue( String tag_source, double d_value ) { return true; }
 
   // Write string value into the provided database tag. 
   boolean WriteSValue( String tag_source, String s_value ) { return true; }
   
   /////////////////////////////////////////////////////////////////////// 
   // Constructor
   /////////////////////////////////////////////////////////////////////// 
   public DataFeedBase( GlgViewer viewer ) 
   {
      Viewer = viewer;
   }
   
   /////////////////////////////////////////////////////////////////////// 
   // Data thread function.
   /////////////////////////////////////////////////////////////////////// 
   void GetRawData()
   {
      while( !StopDataThread )
      {
         /* Process incoming data structures and store them in AccumulatedData
            vector. Application should provide a custom implementation
            of ProcessData() in a derived class.
         */
         ProcessData();
      }
      
      /* Clean-up when the thread is stopped.
         There is no need for locking here, because this is happening while the
         GUI thread is waiting for this thread to stop.
      */
      AccumulatedData.clear();
   }

   /////////////////////////////////////////////////////////////////////// 
   synchronized
   ArrayList<BaseData> GetAccumulatedData( ArrayList<BaseData> old_gui_data )
   {
      /* Pass accumulated data to the caller to use for GUI updates.
         Will pass all data accumulated since the last call.
         Instead of copying, simply swap the arrays for performance 
         and efficiency.
      */
      ArrayList<BaseData> accumulated_data = AccumulatedData;

      // Clear old_gui_data array.
      old_gui_data.clear();

      AccumulatedData = old_gui_data;

      return accumulated_data;
   }

   ///////////////////////////////////////////////////////////////////////
   // Store provdied data record in the AccumulatedData array.
   // The method is synchronized to ensure synchronized access for the
   // AccumulatedData object.
   ///////////////////////////////////////////////////////////////////////
   synchronized void StoreRawData( BaseData data )
   {
      AccumulatedData.add( data );
   }
   
   /////////////////////////////////////////////////////////////////////// 
   // Start data thread. In this example, it starts a data acquisition
   // thread allowing to query data asynchronously, for example
   // using sockets. It creates a thread (DataThread) that invokes
   // GetRawData() which will obtain real-time data values and 
   // store them in AccumulatedData array. 
   /////////////////////////////////////////////////////////////////////// 
   void StartUpdates()
   {
      if( DataThread != null && DataThread.isAlive() )    // Already active.
        return;
      
      /* Execute any additional custom initialization code in the derived class
         to initialize data connectivity.
      */
      Init();
            
      // Create a new thread.
      DataThread = new CustomThread( this );
      DataThread.start();
   }

   /////////////////////////////////////////////////////////////////////// 
   void StopUpdates()
   {
      if( DataThread == null || !DataThread.isAlive() )
        return;
      
      /* Set StopDataThread flag to true and join data thread with the main 
         thread, to make sure the thread will finish execution before 
         it gets destroyed.
      */
      StopDataThread = true;

      /* Join the data thread with the main thread and wait until
         the data thread finished execution before terminating it.
      */
      try 
      {
         DataThread.join();
      }
      catch( InterruptedException e )
      {
         System.out.println( "DataThread join interrupted." );
      }

      StopDataThread = false;
      DataThread = null;

      /* Execute any additional custom termination code in the derived class
         to free data connectivity resources, if any.
      */
      Terminate();
   }

   /////////////////////////////////////////////////////////////////////// 
   // Return exact time including fractions of seconds.
   /////////////////////////////////////////////////////////////////////// 
   double GetCurrTime()
   {
      return System.currentTimeMillis() / 1000.0;
   }

   /////////////////////////////////////////////////////////////////////// 
   class CustomThread extends Thread
   {
      DataFeedBase parent;

      CustomThread( DataFeedBase _parent )
      {
         parent = _parent;
      }
      
      @Override
      public void run()
      {
         parent.GetRawData();
      }
   } 
}

