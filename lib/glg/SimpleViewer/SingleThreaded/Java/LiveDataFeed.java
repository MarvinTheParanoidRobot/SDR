import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
// Provide custom code to read and write real-time data values.
//////////////////////////////////////////////////////////////////////////

public class LiveDataFeed implements DataFeedInterface
{
   GlgViewer Viewer;

   ///////////////////////////////////////////////////////////////////////
   public LiveDataFeed( GlgViewer viewer ) 
   {
      Viewer = viewer;

      // Place custom code here to initialize data feed.
      //
      // InitDataBase();
   }

   ///////////////////////////////////////////////////////////////////////
   public boolean ReadDValue( GlgTagRecord tag_record, DataPoint data_point )
   {
      // Place code here to query and return double tag value.
      // The name of the tag is provided in tag_record.tag_source.

      data_point.d_value = 0.0;
      return true;
   }
      
   ///////////////////////////////////////////////////////////////////////
   public boolean ReadSValue( GlgTagRecord tag_record, DataPoint data_point )
   {
      // Place code here to query and return string tag value.
      // The name of the tag is provided in tag_record.tag_source.

      data_point.s_value = "no value";
      return true;
   }

   ///////////////////////////////////////////////////////////////////////
   public boolean WriteDValue( String tag_source, double d_value )
   {
      // Place code here to write double tag value.
      // The tag value is provided in data_point.d_value.
      
      return true;
   }

   ///////////////////////////////////////////////////////////////////////
   public boolean WriteSValue( String tag_source, String s_value )
   {
      // Place code here to write double tag value.
      // The tag value is provided in data_point.s_value.
      
      return true;
   }
}
