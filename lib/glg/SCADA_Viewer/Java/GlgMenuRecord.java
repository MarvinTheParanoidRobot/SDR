import com.genlogic.*;
   
//////////////////////////////////////////////////////////////////////////
// Class GlgMenuRecord is used by GlgSCADAViewer. It stores information 
// for an individual menu item and can be extended as needed.
//////////////////////////////////////////////////////////////////////////
public class GlgMenuRecord
{
   public String label_string;
   public String drawing_name;
   public String tooltip_string;
   public String drawing_title;
   
   public GlgMenuRecord( String label_p, String drawing_p,
                            String tooltip_p, String title_p ) 
   {
      label_string = label_p;
      drawing_name = drawing_p;
      tooltip_string = tooltip_p;
      drawing_title = title_p;
   }
   
   public GlgMenuRecord()
   {
   }
}
