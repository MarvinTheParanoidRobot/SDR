import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
// Class AlarmRecord is used by GlgSCADAViewer. It stores 
// information for an individual alarm and can be extended as needed.
//////////////////////////////////////////////////////////////////////
public class AlarmRecord
{
   public double time;    // Epoch time in seconds.
   public String tag_source;
   public String description;

   /* If string_value is set to null, double_value will be displayed as alarm 
      value; otherwise string_value will be displayed.
   */
   public String string_value;
   public double double_value;

   public int status;
   public boolean ack;
   public int age;       // Used for demo alarm simulation only.
}
