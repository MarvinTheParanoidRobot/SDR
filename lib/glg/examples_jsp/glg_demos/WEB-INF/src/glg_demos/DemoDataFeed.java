package glg_demos;

import java.util.ArrayList;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
// DemoDataFeed provides simulated data for demo, as well as for testing 
// with no LiveDataFeed.
// In an application, data will be coming from LiveDataFeed.
//////////////////////////////////////////////////////////////////////////

public class DemoDataFeed implements DataFeedInterface
{
   ServletLogInterface servlet;

   long counter = 0;
   double High = 100.;
   double Low = 0.;

   public DemoDataFeed( ServletLogInterface servlet ) 
   {
      this.servlet = servlet;
   }

   /////////////////////////////////////////////////////////////////////// 
   // Fill values of the provided tag list in one batch.
   /////////////////////////////////////////////////////////////////////// 
   public void ReadValues( ArrayList<GlgTagRecord> tag_records )
   {
      int size = tag_records.size();
      for( int i=0; i<size; ++i )
      {
         GlgTagRecord tag_record = tag_records.get(i);
         switch( tag_record.data_type )
         {
            /* In the demo mode, just invoke methods that fill individial
               tag records. In a real application, the code can issue a single
               query to a database to get data for filling all tags in the list.
            */
          case GlgObject.D: ReadDValue( tag_record ); break;
          case GlgObject.S: ReadSValue( tag_record ); break;
          default: servlet.Log( "Unsupported tag type." ); break;
         }
      }
   }

   /////////////////////////////////////////////////////////////////////// 
   // Generate demo data value of type D (double)
   ///////////////////////////////////////////////////////////////////////
   public boolean ReadDValue( GlgTagRecord tag_record )
   {
      double 
        half_amplitude, center,
        period = 100.0,
        alpha;
      
      half_amplitude = ( High - Low ) / 2.0;
      center = Low + half_amplitude;
      
      alpha = 2.0 * Math.PI * counter / period;
      
      tag_record.d_value = center + 
        half_amplitude * Math.sin( alpha ) * Math.sin( alpha / 30.0 );
      tag_record.data_filled = true;
      
      ++counter;
      return tag_record.data_filled;
   }

   /////////////////////////////////////////////////////////////////////// 
   public boolean ReadSValue( GlgTagRecord tag_record )
   {
      tag_record.s_value = "";
      tag_record.data_filled = true;

      return tag_record.data_filled;
   }

   /////////////////////////////////////////////////////////////////////// 
   public boolean WriteDValue( String tag_source, double d_value )
   {
      return true;
   }

   /////////////////////////////////////////////////////////////////////// 
   public boolean WriteSValue( String tag_source, String s_value )
   {
      return true;
   }
}
