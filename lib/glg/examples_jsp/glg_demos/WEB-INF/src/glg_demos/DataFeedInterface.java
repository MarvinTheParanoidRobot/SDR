package glg_demos;

import java.util.ArrayList;

// Data feed interface.
public interface DataFeedInterface
{
   // Fill values of the provided tag list.
   void ReadValues( ArrayList<GlgTagRecord> tag_records );

   // Query a D (double) tag value from the database.
   boolean ReadDValue( GlgTagRecord tag_record );

   // Query an S (string) tag value from the database.
   boolean ReadSValue( GlgTagRecord tag_record );

   // Write numerical value into the provided database tag. 
   boolean WriteDValue( String tag_source, double d_value );
 
   // Write string value into the provided database tag. 
   boolean WriteSValue( String tag_source, String s_value );
}
