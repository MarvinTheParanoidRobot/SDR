import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
// Class GlgTagRecord is used by GlgSCADAViewer. It stores information 
// for a given GLG tag object and can be extended as needed.
//////////////////////////////////////////////////////////////////////
public class GlgTagRecord 
{
   public int data_type;
   public String tag_source;
   public GlgObject tag_obj; 
   
   /* TimeEntryPoint for a plot in a RealTimeChart, if any.
      This object is valid if SUPPLY_PLOT_TIME_STAMP flag is true, and
      the drawing contains a chart where a plot's ValueEntryPoint 
      has a tag with TagSource=tag_source.
   */
   public GlgObject plot_time_ep;

   public GlgTagRecord() 
   {
   }
}
