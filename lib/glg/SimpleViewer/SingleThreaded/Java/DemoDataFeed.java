import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
// DemoDataFeed provides simulated data for demo, as well as for testing 
// with no LiveDataFeed.
// In an application, data will be coming from LiveDataFeed.
//////////////////////////////////////////////////////////////////////////

public class DemoDataFeed implements DataFeedInterface
{
   GlgViewer Viewer;

   long counter = 0;
   long tag_index = 0;
   double High = 100.;
   double Low = 0;

   public DemoDataFeed( GlgViewer viewer ) 
   {
      Viewer = viewer;
   }

   /////////////////////////////////////////////////////////////////////// 
   // Generate demo data value of type D (double)
   ///////////////////////////////////////////////////////////////////////
   public boolean ReadDValue( GlgTagRecord tag_record, DataPoint data_point )
   {
      double 
        half_amplitude, center,
        period = 100.0,
        alpha;
      
      half_amplitude = ( High - Low ) / 2.0;
      center = Low + half_amplitude;
      
      alpha = 2.0 * Math.PI * counter / period;
      
      data_point.d_value = center + 
        half_amplitude * Math.sin( alpha ) * Math.sin( alpha / 30.0 );
      
      ++counter;
      return true;
   }

   /////////////////////////////////////////////////////////////////////// 
   public boolean ReadSValue( GlgTagRecord tag_record, DataPoint data_point )
   {
      data_point.s_value = "";
      return true;
   }

   /////////////////////////////////////////////////////////////////////// 
   public boolean WriteDValue( String tag_source, double d_value )
   {
      /* In the demo mode, set the tag in the drawing to simulate round trip
         after the write operation.
      */
      return Viewer.SetDTag( tag_source, d_value, false );
   }

   /////////////////////////////////////////////////////////////////////// 
   public boolean WriteSValue( String tag_source, String s_value )
   {
      /* In the demo mode, set the tag in the drawing to simulate round trip
         after the write operation.
      */
      return Viewer.SetSTag( tag_source, s_value, false );
   }
}
