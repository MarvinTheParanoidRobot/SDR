import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
// Class GlgActiveDialogRecord is used by GlgSCADAViewer. It stores
// information for each active popup dialog.
//////////////////////////////////////////////////////////////////////
public class GlgActiveDialogRecord
{
   public int dialog_type;
   public GlgObject dialog;       // dialog object ID 
   public GlgObject subwindow;    // Subwindow object inside a dialog.
   public GlgObject popup_vp;     // Viewport loaded into drawing_area.
   public boolean isVisible;

   public GlgActiveDialogRecord( int type, GlgObject dialog_obj, 
                                    GlgObject subwindow_obj, 
                                    GlgObject subwindow_vp, boolean visible ) 
   {
      dialog_type = type;
      dialog = dialog_obj;
      subwindow = subwindow_obj;
      popup_vp = subwindow_vp;
      isVisible = visible;
   }

   public GlgActiveDialogRecord() 
   {
   }
}
