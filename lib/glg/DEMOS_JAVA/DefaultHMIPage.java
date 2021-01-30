import com.genlogic.*;

public class DefaultHMIPage extends HMIPageBase
{
   /////////////////////////////////////////////////////////////////////
   // Viewer and PageType variables are defined in and assigned by the 
   // base HMIPageBase class.
   /////////////////////////////////////////////////////////////////////
   public DefaultHMIPage( GlgSCADAViewer viewer ) 
   {
      super( viewer );
   }

   // Returns an update interval in msec for animating drawing with data.
   public int GetUpdateInterval()
   {
      switch( PageType )
      {
       case GlgSCADAViewer.AERATION_PAGE:      return 2000;
       case GlgSCADAViewer.CIRCUIT_PAGE:       return 1000;
       case GlgSCADAViewer.RT_CHART_PAGE:      return 30;
       case GlgSCADAViewer.TEST_COMMANDS_PAGE: return 100;
       default: return 500;   /* Use default update interval. */
      }
   }

   //////////////////////////////////////////////////////////////////////
   // Returns true if tag sources need to be remapped for the page.
   //////////////////////////////////////////////////////////////////////
   public boolean NeedTagRemapping()
   {
      // In demo mode, unset tags need to be remapped to enable animation.
      if( Viewer.RandomData )
        return true;
      else
        return false;   // Remap tags only if necessary.
   }

   //////////////////////////////////////////////////////////////////////
   // Reassign TagSource parameter for a given tag object to a new
   // TagSource value. tag_source and tag_name parameters are the current 
   // TagSource and TagName of the tag_obj.
   //////////////////////////////////////////////////////////////////////
   public void RemapTagObject( GlgObject tag_obj, 
                               String tag_name, String tag_source )
   {
      if( Viewer.RandomData )
      {
         // Skip tags with undefined TagName.
         if( Viewer.IsUndefined( tag_name ) )
           return;
            
         /* In demo mode, assign unset tag sources to be the same as tag names
            to enable animation with demo data.
         */
         if( Viewer.IsUndefined( tag_source ) )
           Viewer.AssignTagSource( tag_obj, tag_name );
      }
      else
      {
         // Assign new TagSource as needed.
         // Viewer.AssignTagSource( tag_obj, new_tag_source );
      }
   }
}
