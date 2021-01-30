
//////////////////////////////////////////////////////////////////////////
// Provide custom code to read and write real-time data values.
//////////////////////////////////////////////////////////////////////////

public class LiveDataFeed extends DataFeedBase
{

   ///////////////////////////////////////////////////////////////////////
   public LiveDataFeed( GlgViewer viewer ) 
   {
      super( viewer );
   }

   ///////////////////////////////////////////////////////////////////////
   @Override
   void Init()
   {
      // Place custom data initialization code here as needed.
   }

   ///////////////////////////////////////////////////////////////////////
   @Override
   void Terminate()
   {
      // Place custom data termination code here as needed.
   }

   ///////////////////////////////////////////////////////////////////////
   // Provide a custom implementation of this method to process incoming
   // data and store application specific data structures in the 
   // AccumulatedData vector. 
   // Thread synchronization is used inside DataFeedBase.StoreRawData() 
   // to ensure synchronization for AccumulatedData object.
   ///////////////////////////////////////////////////////////////////////
   @Override
   void ProcessData()
   {
      /*
        Get data and pass it to the GUI thread using StoreRawData() function.
        For example:
        
        MyData data = GetMyData();
        if( data != null )
          StoreRawData( data );
        
        Adjust code in GlgViewer.PushData() to process application 
        specific data structures.
        
        Add code to put the data thread to sleep after processing data if
        the method that receives data is not blocking.
      */
   }
   
   
   ///////////////////////////////////////////////////////////////////////
   @Override
   boolean WriteDValue( String tag_source, double d_value )
   {
      // Place code here to write double tag value.
      
      return true;
   }

   ///////////////////////////////////////////////////////////////////////
   @Override
   boolean WriteSValue( String tag_source, String s_value )
   {
      // Place code here to write string tag value.
      
      return true;
   }
}
