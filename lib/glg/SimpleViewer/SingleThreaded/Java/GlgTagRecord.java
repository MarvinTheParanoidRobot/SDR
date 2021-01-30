import com.genlogic.*;

///////////////////////////////////////////////////////////////////////
// Stores information for a given GLG tag object and can be extended 
// by the application as needed.
//////////////////////////////////////////////////////////////////////
public class GlgTagRecord 
{
   public GlgObject tag_obj; 
   public int data_type;
   public String tag_source;
   public boolean if_changed;

   public GlgTagRecord()
   {
   }
}
