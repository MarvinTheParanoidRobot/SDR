import com.genlogic.*;

///////////////////////////////////////////////////////////////////////
// Class GlgActivePopupMenuRecord is used by GlgSCADAViewer. It stores
// information for the active popup menu.
//////////////////////////////////////////////////////////////////////
public class GlgActivePopupMenuRecord
{
   public int menu_type;
   public GlgObject menu_obj;       // menu object ID
   public GlgObject subwindow;      // Subwindow object inside a dialog.
   public GlgObject menu_vp;        // Viewport loaded into drawing_area.
   public GlgObject selected_obj;   // Symbol that trigerred popup menu.
   public boolean isVisible;

   public GlgActivePopupMenuRecord( int type, GlgObject menu, 
                                       GlgObject subwindow_obj, 
                                       GlgObject subwindow_vp,
                                       GlgObject obj, boolean visible ) 
   {
      menu_type = type;
      menu_obj = menu;
      subwindow = subwindow_obj;
      menu_vp = subwindow_vp;
      selected_obj = obj;
      isVisible = visible;
   }

   public GlgActivePopupMenuRecord() 
   {
   }
}
