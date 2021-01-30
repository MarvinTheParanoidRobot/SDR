package glg_demos;

import com.genlogic.*;
import java.util.ArrayList;

//////////////////////////////////////////////////////////////////////////
// Provide custom code to read and write real-time data values.
//////////////////////////////////////////////////////////////////////////

public class LiveDataFeed implements DataFeedInterface
{
   ServletLogInterface servlet;
   
   // Constructor.
   public LiveDataFeed( ServletLogInterface servlet ) 
   {
      this.servlet = servlet;
   }

   public void ReadValues( ArrayList<GlgTagRecord> tag_records )
   {
      int size = tag_records.size();
      for( int i = 0; i < size; ++i )
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

   // Obtain data value of D type (double)
   public boolean ReadDValue( GlgTagRecord tag_record )
   {
      String tag_source = tag_record.tag_source;       

      /* Place custom code here to obtain a real-time data value 
         of D type from the database variable defined by tag_source.
      */
      tag_record.d_value = 0.0;

      tag_record.data_filled = true;
      return tag_record.data_filled;
   }
   
   public boolean ReadSValue( GlgTagRecord tag_record )
   {
      String tag_source = tag_record.tag_source;

      /* place custom code here to obtain a real-time data value 
         of S type from the database variable defined by tag_source.
      */
      tag_record.s_value = "";

      tag_record.data_filled = true;
      return tag_record.data_filled;
   }

   public boolean WriteDValue( String tag_source, double d_value  )
   {
      boolean result = true;
      
      /* Place custom code here to write d_value into the database 
         variable defined by tag_source.
      */

      return result;
    }

   public boolean WriteSValue( String tag_source, String s_value  )
   {
      boolean result = true;
      
      /* Place custom code here to write d_value into the database 
         variable defined by tag_source.
      */

      return result;
   }
}
