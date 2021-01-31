import com.genlogic.*;

// Defines methods used by pages of the GLG SCADAViewer.
public class HMIPageBase
{
   public GlgSCADAViewer Viewer;
   public int PageType;

   public HMIPageBase( GlgSCADAViewer viewer )
   {
      Viewer = viewer;
      PageType = Viewer.PageType;
   }

   // Returns an update interval in msec for animating drawing with data.
   public int GetUpdateInterval()
   {
      return 1000;    // Update once a second by default.
   }

   /* A custom update method for animating drawing with data; may be used
      to implement any additional data update logic.
   */
   public boolean UpdateData()
   {
      /* Return false to automatically update all tags defined in the drawing 
         (via the UpdateData method of the GlgSCADAViewer class).
      */
      return false;
   }

   /* A custom input handler for the page. If it returns false, the default
      input handler of the SCADA Viewer will be used to process common
      events and commands.
   */
   public boolean InputCallback( GlgObject viewport, GlgObject message_obj )
   {
      return false;
   }
 
   /* A custom trace callback for the page. If it returns false, the default 
      trace callback of the SCADA Viewer will be used to process events.
   */   
   public boolean TraceCallback( GlgObject viewport, GlgTraceData trace_info )
   {
      return false;
   }

   /* Returns true if tag sources need to be remapped for the page,
      in which case RemapTagObject() must provide code to perform desired
      remapping logic.
    */
   public boolean NeedTagRemapping()
   {
      return false;
   }

   /* Used if NeedTagRemapping() returns true.
      Reassigns TagSource parameter for a given tag object to a new
      TagSource value. tag_source and tag_name parameters are the current 
      TagSource and TagName of the tag_obj.
   */
   public void RemapTagObject( GlgObject tag_obj, 
                                    String tag_name, String tag_source ){}

   /* Perform any desired initialization of the drawing before and after
      hierarchy setup.
   */
   public void InitBeforeSetup(){}
   public void InitAfterSetup(){}

   // Invoked when the page has been loaded and the tags have been remapped.
   public void Ready(){}
}
