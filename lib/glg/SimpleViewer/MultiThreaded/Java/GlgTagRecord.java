import com.genlogic.*;

///////////////////////////////////////////////////////////////////////
// Stores information for a given GLG tag object and can be extended 
// by the application as needed.
//////////////////////////////////////////////////////////////////////
public class GlgTagRecord 
{
   GlgObject tag_obj; 
   int data_type;
   String tag_source;
   boolean if_changed;

   public GlgTagRecord() {}
}
