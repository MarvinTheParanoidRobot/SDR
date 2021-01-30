// Data feed interface.
public interface DataFeedInterface
{
   // Query D (double) tag value from the database.
   boolean ReadDValue( GlgTagRecord tag_record, DataPoint data_point );

   // Query S (string) tag value from the database.
   boolean ReadSValue( GlgTagRecord tag_record, DataPoint data_point );

   // Write numerical value into the provided database tag. 
   boolean WriteDValue( String tag_source, double d_value );
 
   // Write string value into the provided database tag. 
   boolean WriteSValue( String tag_source, String s_value );
}
