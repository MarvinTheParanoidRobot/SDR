/////////////////////////////////////////////////////////////////////////
// A diagramming editor: example of using Extended API.
//
// This demo uses GLG as a bean and may be used in a browser or
// stand-alone.
//
// The type of the diagram is selected by the first command-line argument
// in the stand-alone mode: -diagram or -process-diagram.
// When used as an applet, the diagram type is defined by the settings of
// the ProcessDiagram applet parameter.
//
// The CustomHandler class provides callbacks for interfacing with
// application-specific functionality.
//
// The DiagramData class uses an array list of type <Object> for the lists 
// of nodes and links to be able to handle a list generically in the code 
// regardless of its element type.
//////////////////////////////////////////////////////////////////////////
import java.awt.*;
import java.awt.event.*;
import java.lang.Math;
import java.util.ArrayList;
import javax.swing.*;
import java.lang.reflect.*;
import com.genlogic.*;
import static com.genlogic.GlgObject.*;

// Allow fallthrough.
@SuppressWarnings("fallthrough")

//////////////////////////////////////////////////////////////////////////
public class GlgDiagram extends GlgJBean implements ActionListener
{
   static final long serialVersionUID = 0;

   // Constants

   static final String SELECT_BUTTON_NAME = "IconButton0";

   // Selection sensitivity in pixels
   static final int SELECTION_RESOLUTION = 5;
   static final int POINT_SELECTION_RESOLUTION = 2;

   // Number of palette buttons to skip: the first button with the 
     // "select" icon is already in the palette.
   static final int PALETTE_START_INDEX = 1;

   // Default scale factor for icon buttons.
   static final double DEFAULT_ICON_ZOOM_FACTOR = 10.0;

   // Percentage of the button area to use for the icon.
   static final double ICON_FIT_FACTOR = 0.6;

   // Scale factor when placed in the drawing
   static final double IconScale = 1.0;

   // Object types
   static final int
     NO_OBJ = 0,
     NODE = 1,
     LINK = 2;

   // IH tokens
   static final int
     IH_UNDEFINED_TOKEN = 0,
     IH_ICON_SELECTED = 1,
     IH_SAVE = 2,
     IH_INSERT = 3,
     IH_PRINT = 4,
     IH_CUT = 5,
     IH_PASTE = 6,
     IH_EXIT = 7,
     IH_ZOOM_IN = 8,
     IH_ZOOM_OUT = 9,
     IH_ZOOM_TO = 10,
     IH_ZOOM_RESET = 11,
     IH_PROPERTIES = 12,
     IH_CREATION_MODE = 13,
     IH_DIALOG_APPLY = 14,
     IH_DIALOG_CLOSE = 15,
     IH_DIALOG_CANCEL = 16,
     IH_DIALOG_CONFIRM_DISCARD = 17,
     IH_DATASOURCE_SELECT = 18,
     IH_DATASOURCE_SELECTED = 19,
     IH_DATASOURCE_CLOSE = 20,
     IH_DATASOURCE_APPLY = 21,
     IH_DATASOURCE_LIST_SELECTION = 22,
     IH_MOUSE_PRESSED = 23,
     IH_MOUSE_RELEASED = 24,
     IH_MOUSE_MOVED = 25,
     IH_MOUSE_BUTTON3 = 26,
     IH_FINISH_LINK = 27,
     IH_TEXT_INPUT_CHANGED = 28,
     IH_OK = 29,
     IH_CANCEL = 30,
     IH_ESC = 31;

   class ButtonToken
   {
      String name;
      int token;
      
      ButtonToken( String name, int token )
      {
         this.name = name;
         this.token = token;
      }
   }

   ButtonToken [] ButtonTokenTable =
   {
      new ButtonToken( "Save",             IH_SAVE ),
      new ButtonToken( "Insert",           IH_INSERT ),
      new ButtonToken( "Print",            IH_PRINT ),
      new ButtonToken( "Cut",              IH_CUT ),
      new ButtonToken( "Paste",            IH_PASTE ),
      new ButtonToken( "Exit",             IH_EXIT ),
      new ButtonToken( "ZoomIn",           IH_ZOOM_IN ),
      new ButtonToken( "ZoomOut",          IH_ZOOM_OUT ),
      new ButtonToken( "ZoomTo",           IH_ZOOM_TO ),
      new ButtonToken( "ZoomReset",        IH_ZOOM_RESET ),
      new ButtonToken( "Properties",       IH_PROPERTIES ),
      new ButtonToken( "CreateMode",       IH_CREATION_MODE ),
      new ButtonToken( "DialogApply",      IH_DIALOG_APPLY ),
      new ButtonToken( "DialogClose",      IH_DIALOG_CLOSE ),
      new ButtonToken( "DialogCancel",     IH_DIALOG_CANCEL ),
          /* process diagrams only */
      new ButtonToken( "DataSourceSelect", IH_DATASOURCE_SELECT ),
          /* process diagrams only */
      new ButtonToken( "DataSourceClose",  IH_DATASOURCE_CLOSE ),
          /* process diagrams only */
      new ButtonToken( "DataSourceApply",  IH_DATASOURCE_APPLY ),
      new ButtonToken( "OKDialogOK",       IH_OK ),
      new ButtonToken( "OKDialogCancel",   IH_CANCEL ),
      new ButtonToken( null,               0 )
   };

   boolean TraceMouseMove = false;
   boolean TraceMouseRelease = false;
   boolean StickyCreateMode = false;  /* If set to true, multple instances of
                                         the selected item can be added to 
                                         the drawing by clicking in the 
                                         drawing area. */
   static boolean StandAlone;
   GlgObject Viewport;
   GlgObject DrawingArea;
   GlgObject SelectedObject = null;
   GlgObject PointMarker = null;
   GlgObject AttachmentMarker = null;
   GlgObject AttachmentArray = null;
   GlgObject AttachmentNode = null;
   boolean AllowUnconnectedLinks = true;
   int SelectedObjectType;
   GlgObject StoredColor;
   GlgObject CutBuffer;
   int CutBufferType;
   boolean DialogDataChanged = false;
   String LastButton;
   GlgDiagramData CurrentDiagram = new GlgDiagramData();
   GlgDiagramData SavedDiagram;
   GlgObject NodeIconArray;
   GlgObject NodeObjectArray;
   GlgObject LinkIconArray;
   GlgObject LinkObjectArray;
   GlgObject PaletteTemplate;
   GlgObject ButtonTemplate;
   int NumColumns;

   GlgPoint cursor_pos = new GlgPoint();
   GlgPoint world_coord = new GlgPoint();
   GlgCube select_rect = new GlgCube();

   GlgObject last_color;   // Stores color during selection.

   boolean ProcessDiagram = false;  // Defines the type of the diagram.

   // Used by process diagram.
   int DataSourceCounter = 0;
   int NumDatasources = 20;
   static int UPDATE_INTERVAL = 1000;     // Update once per second.

   // If set to true, icons are automatically fit to fill the button.
     // If set to false, the default zoom factor will be used.
   boolean FitIcons = false;

   javax.swing.Timer timer = null;
   boolean IsReady = false;

   //////////////////////////////////////////////////////////////////////////
   // Utility class used to return several values.
   //////////////////////////////////////////////////////////////////////////
   class ObjectInfo
   {
      GlgObject glg_object;
      int type;

      // Constructors
      ObjectInfo() 
      {}

      ObjectInfo( GlgObject glg_object_p, int type_p )
      {
         glg_object = glg_object_p;
         type = type_p;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   public GlgDiagram()
   {
      super();

      SelectedObjectType = NO_OBJ;
      CutBufferType = NO_OBJ;
      LastButton = null;

      /* The bean is a default listener for bean events (except for the trace
         events which are disabled by default), but add all listeners
         explicitely anyway as an example. 
      */
      AddListener( GlgObject.H_CB, this );
      AddListener( GlgObject.V_CB, this );
      AddListener( GlgObject.READY_CB, this );
      AddListener( GlgObject.INPUT_CB, this );
      AddListener( GlgObject.TRACE_CB, this );
   }

   //////////////////////////////////////////////////////////////////////////
   // For use as a stand-alone java demo
   // The type of the diagram is selected by the first command-line argument:
   //   -diagram or -process-diagram.
   //////////////////////////////////////////////////////////////////////////
   public static void main( final String [] arg )
   {
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Main( arg ); } } );
   }

   //////////////////////////////////////////////////////////////////////////
   public static void Main( final String [] arg )
   {
      class DemoQuit extends WindowAdapter
      {
         GlgDiagram diagram;

         DemoQuit( GlgDiagram diagram )
         {
            this.diagram = diagram;
         }

         public void windowClosing( WindowEvent e )
         {
            // Get confirmation before exiting.
            if( diagram != null && diagram.IsReady )
              IHCallCurrIHWithToken( IH_EXIT );
         }
      }

      IHInit();

      GlgDiagram.StandAlone = true;
      GlgDiagram diagram = new GlgDiagram();      

      // Process command line arguments.
      diagram.ProcessArgs( arg );

      JFrame frame = 
        new JFrame( diagram.ProcessDiagram ? 
                    "GLG Process Diagram Demo" : "GLG Diagram Demo" );

      frame.setResizable( true );
      frame.setSize( 900, 700 );
      frame.setLocation( 20, 20 );

      frame.getContentPane().add( diagram );

      frame.addWindowListener( new DemoQuit( diagram ) );
      frame.setDefaultCloseOperation( JFrame.DO_NOTHING_ON_CLOSE );

      frame.setVisible( true );

      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers the H, V and Ready callbacks.
      // When used as an applet, the DrawingName parameter is set in HTML.
        //
      diagram.SetDrawingName( diagram.ProcessDiagram ? 
                              "process_diagram.g" : "diagram.g" );
   }

   //////////////////////////////////////////////////////////////////////////
   // Processes command line arguments.
   //////////////////////////////////////////////////////////////////////////
   public void ProcessArgs( String [] arg )
   {
      if( arg == null )
        return;

      int num_arg = arg.length;
      if( num_arg != 0 )
      {
         if( arg[ 0 ].equals( "-process-diagram" ) )
           ProcessDiagram = true;
         else if( arg[ 0 ].equals( "-diagram" ) )
           ProcessDiagram = false;
         else
           DisplayUsage();
      }
      else
        DisplayUsage();
   }

   //////////////////////////////////////////////////////////////////////////
   public void StopUpdates()
   {
      if( timer != null )
      {
         timer.stop();
         timer = null;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   static void DisplayUsage()
   {
      String usage_info = 
        "Use the -process-diagram or -diagram options\n" +
        "   to select the type of the diagram.";

      System.out.println( usage_info );
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked after the drawing has been loaded but before it's set up. 
   //////////////////////////////////////////////////////////////////////////
   public void HCallback( GlgObject viewport )
   {
      if( !StandAlone )
      {
         String prop_string = getParameter( "ProcessDiagram" );
         if( prop_string != null )
           if( prop_string.equalsIgnoreCase( "true" ) )
             ProcessDiagram = true;
           else if( prop_string.equalsIgnoreCase( "false" ) )
             ProcessDiagram = false;
      }

      if( ProcessDiagram )
        FitIcons = true;
      else
        FitIcons = false;

      // Fill out the palette and change the dialog type before the 
        // hierarchy setup

      Viewport = GetViewport();

      DrawingArea = Viewport.GetResourceObject( "DrawingArea" );
      if( DrawingArea == null )
      {
         System.out.println( "Can't find DrawingArea viewport." );
         return;
      }

      String full_path = GetFullPath( ProcessDiagram ? "process_template.g" :
                                      "diagram_template.g" );
      GlgObject template_drawing = 
        GlgObject.LoadObject( full_path, StandAlone ? GlgObject.FILE : GlgObject.URL );

      PaletteTemplate = template_drawing.GetResourceObject( "PaletteTemplate" );
      if( PaletteTemplate == null )
      {
         System.out.println( "Can't find PaletteTemplate viewport." );
         return;
      }

      AddDialog( template_drawing, "Dialog", "Object Properties", 400, 0 );
      AddDialog( template_drawing, "OKDialog", null, 0, 0 );
      if( ProcessDiagram )
        AddDialog( template_drawing, "DataSourceDialog", null, 700, 200 );

      Initialize( Viewport );
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked after the drawing has been loaded and set up, but before it's 
   // displayed. 
   //////////////////////////////////////////////////////////////////////////
   public void VCallback( GlgObject viewport )
   {      
      // Position node icons inside the palette buttons.
      SetupObjectPalette( "IconButton", PALETTE_START_INDEX );

      IHInstall( new MainIH( this ) );
      IHStart();
   }

   //////////////////////////////////////////////////////////////////////////
   // Performs initial setup of the drawing.
   //////////////////////////////////////////////////////////////////////////
   void Initialize( GlgObject viewport )
   {
      SetPrompt( "" );

      /* Create a color object to store original node color during node 
         selection. */
      StoredColor = Viewport.GetResourceObject( "FillColor" ).CopyObject();

      // Make exit button visible for stand-alone.
      if( StandAlone )
        Viewport.SetDResource( "Exit/Visibility", 1.0 );
      else
        Viewport.SetDResource( "Exit/Visibility", 0.0 );

      // Set grid color (configuration parameter) to grey.
      Viewport.SetGResource( "$config/GlgGridPolygon/EdgeColor",
                             0.632441, 0.632441, 0.632441 );

      // Create a separate group to hold objects.
      GlgObject group = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
      group.SetSResource( "Name", "ObjectGroup" );
      DrawingArea.AddObjectToBottom( group );

      // Create groups to hold nodes and links.
      NodeIconArray = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
      NodeObjectArray = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
      LinkIconArray = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
      LinkObjectArray = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );

      /* Scan palette template and extract icon and link objects, adding them
         to the buttons in the object palette.
      */
      GetPaletteIcons( PaletteTemplate, "Node", NodeIconArray, NodeObjectArray );
      GetPaletteIcons( PaletteTemplate, "Link", LinkIconArray, LinkObjectArray );

      FillObjectPalette( "ObjectPalette", "IconButton", PALETTE_START_INDEX );

      SetRadioBox( SELECT_BUTTON_NAME );  // Highlight Select button

      // Set initial sticky creation mode from the button state in the drawing.
      SetCreateMode( false );

      CurrentDiagram = new GlgDiagramData();
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked after the drawing has been drawn for the first time.
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );

      IsReady = true;
      if( ProcessDiagram )
        if( timer == null )
        {
           // Restart the timer after each update (instead of using repeats)
           // to avoid flooding the event queue with timer events on slow 
           // machines.
           timer = new javax.swing.Timer( UPDATE_INTERVAL, this );
           timer.setRepeats( false );
           timer.start();
        }
   }

   //////////////////////////////////////////////////////////////////////////
   // Sets create mode based on the state of the CreateMode button.
   //////////////////////////////////////////////////////////////////////////
   void SetCreateMode( boolean set_button )
   {
      if( !set_button )
      {
         int create_mode = (int) GetDResource( "CreateMode/OnState" );
         StickyCreateMode = ( create_mode != 0 );
      }
      else   /* Restore button state from StickyCreateMode. */
        SetDResource( "CreateMode/OnState", StickyCreateMode ? 1.0 : 0.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   void AddDialog( GlgObject drawing, String dialog_name, String title,
                   int x_offset, int y_offset )
   {
      GlgObject dialog = drawing.GetResourceObject( dialog_name );
      if( dialog == null )
      {
         System.out.println(  "Can't find dialog."  );
         return;
      }

      /* Make the dialog a top-level window, set its title and make
         it invisible on startup. */
      dialog.SetDResource( "ShellType", (double) GlgObject.DIALOG_SHELL );
      if( title != null )
        dialog.SetSResource( "ScreenName", title );
      dialog.SetDResource( "Visibility", 0.0 );

      /* The dialog uses a predefined layout widget with offset transformations 
         attached to its control points.
         
         The dialog widget allows defining the dialog's width and height in 
         the drawing via the Width and Height parameters, and use the dialog's 
         center anchor point to position the dialog.
         
         Alternatively, a simple viewport can be used as a dialog window.
         The below code demonstrates how to position the dialog in both cases.
         
         In either case, the position has to be defined before the dialog
         is set up the first time (before it's added to the drawing).
      */
      if( true )
      {
         /* Predefined layout widget is used as a dialog.
            Center the dialog relatively the parent window, but with a supplied 
            offset. The offset is defined in world coordinates and is relative to
            the center of the dialog's parent.
         */
         dialog.SetGResource( "AnchorPointCenter", 
                              (double) x_offset, (double) y_offset, 0.0 );
      }
      /* 
      else
      {
         // Alternatively, a simple viewport could be used as a dialog window.
         // In this case, the below code could be used to set the dialog's size
         // in pixels as well as the pixel offset of its upper left corner 
         // relatively to the parent's upper left corner.
         
         dialog.SetGResource( "Point1", 0.0, 0.0, 0.0 );
         dialog.SetGResource( "Point2", 0.0, 0.0, 0.0 );
         dialog.SetDResource( "Screen/WidthHint", (double) width );
         dialog.SetDResource( "Screen/HeightHint", (double) height );
         dialog.SetDResource( "Screen/XHint", (double) x_offset );
         dialog.SetDResource( "Screen/YHint", (double) y_offset );
      }
      */
      
      Viewport.AddObjectToBottom( dialog );
   }

   ////////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject viewport, GlgObject message_obj )
   {
      if( !IsReady )
        return;
      
      String format      = message_obj.GetSResource( "Format" );
      String origin      = message_obj.GetSResource( "Origin" );
      String full_origin = message_obj.GetSResource( "FullOrigin" );
      String action      = message_obj.GetSResource( "Action" );
      String subaction   = message_obj.GetSResource( "SubAction" );

      int token = IH_UNDEFINED_TOKEN;

      // Handle the Dialog window closing.
      if( format.equals( "Window" ) )
      {
         if( action.equals( "DeleteWindow" ) )
           if( origin.equals( "Dialog" ) )
             token = IH_DIALOG_CLOSE;
           else if( origin.equals( "DataSourceDialog" ) )
             token = IH_DATASOURCE_CLOSE;
           else if( origin.equals( "OKDialog" ) )
             token = IH_ESC;
           else
             token = IH_EXIT;
      }
      else if( format.equals( "Button" ) )
      {
         if( !action.equals( "Activate" ) && !action.equals( "ValueChanged" ) )
           return;

         else if( origin.startsWith("IconButton") )
         {
            GlgObject button = viewport.GetResourceObject( full_origin );
            GlgObject icon = button.GetResourceObject( "Icon" );
            if( icon == null )
              SetError( "Can't find icon." );
            else
            {
               IHSetOParameter( IH_GLOBAL, "$selected_icon", icon );
               IHSetSParameter( IH_GLOBAL, "$selected_button", full_origin );
               token = IH_ICON_SELECTED;
            }
         }
         else
           token = ButtonToToken( origin );
      }
      else if( format.equals( "Text" ) )
      {
         if( action.equals( "ValueChanged" ) )
           token = IH_TEXT_INPUT_CHANGED;
      }
      else if( format.equals( "List" ) )
      {
         if( action.equals( "Select" ) && subaction.equals( "DoubleClick" ) &&
             origin.equals( "DSList" ) )
           token = IH_DATASOURCE_LIST_SELECTION;
      }

      if( token != IH_UNDEFINED_TOKEN )
        IHCallCurrIHWithToken( token );

      Update();
   }

   ////////////////////////////////////////////////////////////////////////
   // Handles mouse operations: selection, dragging, connection point 
   // highlight.
   ////////////////////////////////////////////////////////////////////////
   public void TraceCallback( GlgObject viewport, GlgTraceData trace_info )
   {
      if( !IsReady )
        return;

      boolean use_coords = false;
      int token = IH_UNDEFINED_TOKEN;

      int event_type = trace_info.event.getID();
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
         switch( GetButton( trace_info.event ) )
         {
          case 1: token = IH_MOUSE_PRESSED; break;
          case 3: token = IH_MOUSE_BUTTON3; break;
          default: return;   /* Report only buttons 1 and 3 */
         }
         use_coords = true;
         break;

       case MouseEvent.MOUSE_RELEASED:
         if( GetButton( trace_info.event ) != 1 )
           return;  // Trace only the left button releases.
         token = IH_MOUSE_RELEASED;
         use_coords = true;
         break;

       case MouseEvent.MOUSE_MOVED:
       case MouseEvent.MOUSE_DRAGGED:
         token = IH_MOUSE_MOVED;
         use_coords = true;
         break;

       case KeyEvent.KEY_PRESSED:
         if( ((KeyEvent)trace_info.event).getKeyCode() == KeyEvent.VK_ESCAPE )
           token = IH_ESC;      // ESC key
         break;
         
       default: return;
      }

      switch( token )
      {
       case IH_UNDEFINED_TOKEN: 
         return;
       case IH_MOUSE_MOVED: 
         if( !TraceMouseMove )
           return;
         break;
       case IH_MOUSE_RELEASED:
         if( !TraceMouseRelease )
           return;
         break;
      }

      if( DrawingArea.GetDResource( "ZoomToMode" ).intValue() != 0 )
        return;   // Don't handle mouse selection in ZoomTo mode.

      if( use_coords )
      {
         /* If nodes use viewports (buttons, gauges, etc.), need to convert 
            coordinates inside the selected viewport to the coordinates of the 
            drawing area.
         */
         if( trace_info.viewport != DrawingArea && 
             !IsChildOf( DrawingArea, trace_info.viewport ) )
           return;   /* Mouse event outside of the drawing area. */

         cursor_pos.x = (double) ((MouseEvent)trace_info.event).getX();
         cursor_pos.y = (double) ((MouseEvent)trace_info.event).getY();
         cursor_pos.z = 0.0;
         
         /* COORD_MAPPING_ADJ is added to the cursor coordinates for 
            precise pixel mapping.
         */
         cursor_pos.x += GlgObject.COORD_MAPPING_ADJ;
         cursor_pos.y += GlgObject.COORD_MAPPING_ADJ;

         if( trace_info.viewport != DrawingArea  )
           GlgObject.TranslatePointOrigin( trace_info.viewport, viewport, 
                                           cursor_pos );

         IHSetOParameterFromG( IH_GLOBAL, "$cursor_pos", cursor_pos );
      }

      // Pass to the current IH.
      IHCallCurrIHWithToken( token );
   }

   ////////////////////////////////////////////////////////////////////////
   // Can be used only for drawable objects, and not for data objects that 
   // can be constrained.
   ////////////////////////////////////////////////////////////////////////
   boolean IsChildOf( GlgObject grand, GlgObject object )
   {
      if( object == null )
        return false;
      
      if( object == grand )
        return true;
      
      return IsChildOf( grand, object.GetParent() );
   }

   ////////////////////////////////////////////////////////////////////////
   class MainIH implements GlgIHHandlerInterface
   {
      GlgDiagram diagram;
      MainIH( GlgDiagram diagram ) { this.diagram = diagram; }

      public void EntryPoint( GlgObject ih, GlgCallEvent call_event )
      {
         int token;
         GlgObject
           icon,
           object;
         String
           icon_type,
           button_name;
         
         switch( IHGetType( call_event ) )
         {
          case HI_SETUP_EVENT:
            break;

          case MESSAGE_EVENT:
            token = IHGetToken( call_event );
            switch( token )
            {
             case IH_EXIT:
               IHInstall( new ConfirmIH( diagram ) );
               IHSetSParameter( IH_NEW, "title", "Confirm Exiting" );
               IHSetSParameter( IH_NEW, "message", "OK to quit?" );
               IHSetIParameter( IH_NEW, "requested_op", token );
               IHStart();
               break;
               
             case IH_SAVE:
               Save( diagram.CurrentDiagram );
               break;
               
             case IH_INSERT:
               Load();
               break;

             case IH_PRINT:
               Print();
               break;

             case IH_CUT:
               Cut();
               break;

             case IH_PASTE:
               Paste();
               break;

             case IH_ZOOM_IN:
               diagram.DrawingArea.SetZoom( null, 'i', 0.0 );
               Update();
               break;

             case IH_ZOOM_OUT:
               diagram.DrawingArea.SetZoom( null, 'o', 0.0 );
               Update();
               break;
               
             case IH_ZOOM_TO:
               diagram.DrawingArea.SetZoom( null, 't', 0.0 );
               Update();
               break;

             case IH_ZOOM_RESET:
               diagram.DrawingArea.SetZoom( null, 'n', 0.0 );
               Update();
               break;

             case IH_ICON_SELECTED:
               icon = IHGetOParameter( IH_GLOBAL, "$selected_icon" );
               button_name = IHGetSParameter( IH_GLOBAL, "$selected_button" );
               if( icon == null || button_name == null )
               {
                  SetError( "null icon or icon button name." );
                  break;
               }
               
               /* Object to use in the drawing. In case of connectors, uses 
                  only a part of the icon (the connector object) without the 
                  end markers.
               */
               object = icon.GetResourceObject( "Object" );
               if( object == null )
                 object = icon;
               
               icon_type = object.GetSResource( "IconType" );
               if( icon_type == null )
               {
                  SetError( "Can't find icon type." );
                  break;
               }
               
               if( icon_type.equals( "Select" ) )
               {
                  SetRadioBox( SELECT_BUTTON_NAME ); // Highlight Select button
                  SetPrompt( "" );
               }
               else if( icon_type.equals( "Link" ) )
               {
                  IHInstall( new AddLinkIH( diagram ) );
                  IHSetOParameter( IH_NEW, "template", object );
                  IHSetSParameter( IH_NEW, "button_name", button_name );
                  IHStart();
               }
               else if( icon_type.equals( "Node" ) )
               {
                  IHInstall( new AddNodeIH( diagram ) );
                  IHSetOParameter( IH_NEW, "template", object );
                  IHSetSParameter( IH_NEW, "button_name", button_name );
                  IHStart();
               }
               Update();
               break;
               
             case IH_CREATION_MODE:
               /* Set sticky creation mode from the button. */
               SetCreateMode( false );
               break;

             case IH_MOUSE_PRESSED:
               /* Selects the object and installs MoveObjectIH to drag the 
                  object with the mouse.
               */
               SelectObjectWithMouse( IHGetOParameter( IH_GLOBAL, 
                                                       "$cursor_pos" ) );
               /* All tokens that originate from the TraceCB require an explicit 
                  update. For tokens originating from the InputCB, update is 
                  done at the end of the InputCB.
               */            
               Update();
               break;

             case IH_ESC:
             case IH_MOUSE_BUTTON3:
               break;    /* Allow: do nothing. */
               
             default: 
               /* Pass these tokens to be processed by a pass-through 
                  EditPropertiesIH for dialog that can stay open to show 
                  properties while selecting different objects in the drawing.
               */
               IHSetBParameter( IH_GLOBAL, "fall_through_call", true );

               IHPassToken( new EditPropertiesIH( diagram ), token, false );

               IHSetBParameter( IH_GLOBAL, "fall_through_call", false );

               if( !IHGetBParameter( IH_GLOBAL, "token_used" ) )
                 SetError( "Invalid token." );
               break;
            }
            break;
            
          case CLEANUP_EVENT:
            SetError( "Main ih handler should never be uninstalled." );
            break;
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////
   // Handles object selection and prepares for moving the object with 
   // the mouse.
   ////////////////////////////////////////////////////////////////////////
   void SelectObjectWithMouse( GlgObject cursor_pos_obj )
   {
      GlgObject
        selection,
        sel_object;
      int i, num_selected;

      selection = GetObjectsAtCursor( cursor_pos_obj );

      if( selection != null && ( num_selected = selection.GetSize() ) != 0 )
      {
         // Some object were selected, process the selection.
         for( i=0; i < num_selected; ++i )
         {
            sel_object = (GlgObject) selection.GetElement( i );

            /* Find if the object itself is a link or a node, of if it's a part
               of a node. If it's a part of a node, get the node object ID.
            */
            ObjectInfo selection_info = GetSelectedObject( sel_object );
            sel_object = selection_info.glg_object;
            int selection_type = selection_info.type;
         
            if( selection_type != NO_OBJ )
            {
               SelectGlgObject( sel_object, selection_type );

               CustomHandler.SelectObjectCB( this, sel_object, 
                                             GetData( sel_object ),
                                             selection_type == NODE );

               // Prepare for dragging the object with the mouse.
               IHInstall( new MoveObjectIH( this ) );

               // Store the start point.
               IHSetOParameter( IH_NEW, "start_point", cursor_pos_obj );

               IHStart();

               return;
            }
         }
      }

      SelectGlgObject( null, 0 );    // Unselect
   }

   ////////////////////////////////////////////////////////////////////////
   class MoveObjectIH implements GlgIHHandlerInterface
   {
      GlgDiagram diagram;
      MoveObjectIH( GlgDiagram diagram ) { this.diagram = diagram; }

      ////////////////////////////////////////////////////////////////////////
      // Parameters:
      //   start_point (G data obj)
      ////////////////////////////////////////////////////////////////////////
      public void EntryPoint( GlgObject ih, GlgCallEvent call_event )
      {
         int token;
         GlgObject start_point_obj, cursor_pos_obj;
         Object data;
         
         switch( IHGetType( call_event ) )
         {
          case HI_SETUP_EVENT:
            TraceMouseMove = true;
            TraceMouseRelease = true;
            break;

          case MESSAGE_EVENT:
            token = IHGetToken( call_event );
            switch( token )
            {
             case IH_MOUSE_MOVED:
               data = GetData( SelectedObject );
               
               start_point_obj = IHGetOParameter( ih, "start_point" );
               cursor_pos_obj = IHGetOParameter( IH_GLOBAL, "$cursor_pos" );
               
               if( SelectedObjectType == NODE )
                 MoveObject( SelectedObject, start_point_obj, cursor_pos_obj );
               else
                 MoveLink( SelectedObject, start_point_obj, cursor_pos_obj );
               
               Update();

               if( SelectedObjectType == NODE )
               {
                  // Update the X and Y in the node's data struct.
                  UpdateNodePosition( SelectedObject, (GlgNodeData) data );
            
                  /* Don't need to update the attached links' points, since 
                     the stored positions of the first and last points are
                     not used: they are constrained to nodes and positioned 
                     by them. */
               }
               else   /* LINK */
               {
                  GlgLinkData link_data = (GlgLinkData) data;
                  if( link_data.start_node != null )
                    UpdateNodePosition( link_data.start_node.graphics, null );
                  if( link_data.end_node != null )
                    UpdateNodePosition( link_data.end_node.graphics, null );
                  
                  // Update stored point values.
                  StorePointData( link_data, SelectedObject );
               }
               
               // Update the start point for the next move.
               IHChangeOParameter( ih, "start_point", cursor_pos_obj );

               Update();
               break;
               
             case IH_MOUSE_RELEASED:
               IHUninstall();
               break;
               
             default:
               IHUninstallWithEvent( call_event );   // Pass to the parent IH.
               break;
            }
            break;
            
          case CLEANUP_EVENT:
            TraceMouseMove = false;
            TraceMouseRelease = false;
            break;
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////
   class AddNodeIH implements GlgIHHandlerInterface
   {
      GlgDiagram diagram;
      AddNodeIH( GlgDiagram diagram ) { this.diagram = diagram; }

      ////////////////////////////////////////////////////////////////////////
      // Parameters:
      //   template    (obj)
      //   button_name (string)
      ////////////////////////////////////////////////////////////////////////
      public void EntryPoint( GlgObject ih, GlgCallEvent call_event )
      {
         int token;
         GlgObject
           template, 
           new_node,
           cursor_pos_obj;
         int node_type;  

         switch( IHGetType( call_event ) )
         {
          case HI_SETUP_EVENT:
            TraceMouseMove = true;
            SetRadioBox( IHGetSParameter( ih, "button_name" ) );
            SetPrompt( "Position the node." );
            Update(); 
            break;

          case MESSAGE_EVENT:
            token = IHGetToken( call_event );
            switch( token )
            {
             case IH_MOUSE_PRESSED:
               // Query node type.
               template = IHGetOParameter( ih, "template" );
               node_type = template.GetDResource( "Index" ).intValue();
               
               cursor_pos_obj = IHGetOParameter( IH_GLOBAL, "$cursor_pos" );
               new_node = AddNodeAt( node_type, null, cursor_pos_obj, 
                                     GlgObject.SCREEN_COORD );
               CustomHandler.AddObjectCB( diagram, new_node, GetData( new_node ),
                                          true );
               SelectGlgObject( new_node, NODE );

               /* In StickyCreateMode, keep adding nodes at each mouse click 
                  position.
               */
               if( !StickyCreateMode )
                 IHUninstall();

               Update();
               break;
               
             case IH_MOUSE_MOVED: 
               break;   // Allow: do nothing.
               
             default:
               IHUninstallWithEvent( call_event ); // Pass to the parent IH.
               break;
            }
            break;
            
          case CLEANUP_EVENT:
            TraceMouseMove = false;
            SetRadioBox( SELECT_BUTTON_NAME );   // Highlight Select button
            SetPrompt( "" );
            Update();
            break;
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////
   class AddLinkIH implements GlgIHHandlerInterface
   {
      GlgDiagram diagram;
      AddLinkIH( GlgDiagram diagram ) { this.diagram = diagram; }

      ////////////////////////////////////////////////////////////////////////
      // Parameters:
      //   template (obj)
      //   button_name (string)
      ////////////////////////////////////////////////////////////////////////
      public void EntryPoint( GlgObject ih, GlgCallEvent call_event )
      {
         int token;
         GlgObject 
           template,
           cursor_pos_obj,
           start_point_obj,
           sel_node,
           point, 
           pt_array,
           drag_link;
         GlgLinkData link_data;
         int
           link_type,
           edge_type;
         boolean
           first_node,
           middle_point_added;

         switch( IHGetType( call_event ) )
         {
          case HI_SETUP_EVENT:
            TraceMouseMove = true;
            SetRadioBox( IHGetSParameter( ih, "button_name" ) );

            // Store link type
            template = IHGetOParameter( ih, "template" );
            link_type = template.GetDResource( "Index" ).intValue();
            IHSetIParameter( ih, "link_type", link_type );

            // Store edge type
            ObjectInfo link_info = GetCPContainer( template );
            edge_type = link_info.type;
            IHSetIParameter( ih, "edge_type", edge_type );

            IHSetOParameter( ih, "drag_link", null );
            /* Fall through */

          case HI_RESETUP_EVENT:
            IHSetBParameter( ih, "first_node", true );
            IHSetBParameter( ih, "middle_point_added", false );
            
            SetPrompt( "Select the first node or attachment point." );
            Update();
            break;

          case MESSAGE_EVENT:
            first_node = IHGetBParameter( ih, "first_node" );
            drag_link = IHGetOParameter( ih, "drag_link" );

            token = IHGetToken( call_event );
            switch( token )
            {
             case IH_MOUSE_MOVED:
               cursor_pos_obj = IHGetOParameter( IH_GLOBAL, "$cursor_pos" );
               StoreAttachmentPoints( cursor_pos_obj, token );

               point = IHGetOParameter( ih, "attachment_point" );
               pt_array = IHGetOParameter( ih, "attachment_array" );
               sel_node = IHGetOParameter( ih, "attachment_node" );

               if( point != null )
                 ShowAttachmentPoints( point, null, null, 0 );
               else if( pt_array != null )
               {
                  ShowAttachmentPoints( null, pt_array, sel_node, 1 );
               }
               else
                 // No point or no selection: erasing attachment points feedback.
                 EraseAttachmentPoints();

               // Drag the link's last point.
               if( !first_node && token == IH_MOUSE_MOVED )
               {
                  link_data = (GlgLinkData) GetData( drag_link );
                  
                  /* First time: set link direction depending of the direction
                     of the first mouse move, then make the link visible.
                  */
                  if( link_data.first_move )
                  {
                     start_point_obj = IHGetOParameter( ih, "start_point" );
                     SetEdgeDirection( drag_link, 
                                       start_point_obj, cursor_pos_obj );
                     drag_link.SetDResource( "Visibility", 1.0 );
                     link_data.first_move = false;
                  }

                  SetLastPoint( drag_link, cursor_pos_obj, false, false );

                  middle_point_added = 
                    IHGetBParameter( ih, "middle_point_added" );
                  if( !middle_point_added )
                    SetArcMiddlePoint( drag_link );
               }
               Update();
               break;
               
             case IH_MOUSE_PRESSED:
               cursor_pos_obj = IHGetOParameter( IH_GLOBAL, "$cursor_pos" );
               StoreAttachmentPoints( cursor_pos_obj, token );

               point = IHGetOParameter( ih, "attachment_point" );
               pt_array = IHGetOParameter( ih, "attachment_array" );
               sel_node = IHGetOParameter( ih, "attachment_node" );

               if( point != null )
               {
                  if( first_node )
                  {	       
                     IHSetOParameter( ih, "first_point", point );
               
                     link_type = IHGetIParameter( ih, "link_type" );
                     drag_link = AddLinkObject( link_type, null );
                     IHSetOParameter( ih, "drag_link", drag_link );
               
                     // First point
                     ConstrainLinkPoint( drag_link, point, false );
                     AttachFramePoints( drag_link );
               
                     // Wire up the start node
                     link_data = (GlgLinkData) GetData( drag_link );
                     link_data.start_node = (GlgNodeData) GetData( sel_node );
               
                     /* Store cursor position for setting direction based on the
                        first mouse move.
                     */
                     IHSetOParameter( ih, "start_point", cursor_pos_obj );
                     link_data.first_move = true;
                     drag_link.SetDResource( "Visibility", 0.0 );
               
                     IHChangeBParameter( ih, "first_node", false );
                     SetPrompt( "Select the second node or additional points." );
                  }
                  else
                  {  
                     GlgObject first_point;
                     
                     first_point = 
                       IHGetOptOParameter( ih, "first_point", null );
                     if( point == first_point )
                     {
                        SetError( "The two nodes are the same, " +
                                  "chose a different second node." );
                        break;
                     }
                     
                     // Last point
                     ConstrainLinkPoint( drag_link, point, true ); 
                     AttachFramePoints( drag_link );

                     middle_point_added = 
                       IHGetBParameter( ih, "middle_point_added" );
                     if( !middle_point_added )
                       SetArcMiddlePoint( drag_link );
               
                     // Wire up the end node
                     link_data = (GlgLinkData) GetData( drag_link );
                     link_data.end_node = (GlgNodeData) GetData( sel_node );
               
                     FinalizeLink( drag_link );
                     IHChangeOParameter( ih, "drag_link", null );
               
                     if( StickyCreateMode )
                     {
                        IHCallCurrIHWithToken( IH_FINISH_LINK );
                        IHResetup( ih );   // Start over to create more links.
                     }
                     else
                       IHUninstall();   /* Will call IH_FINISH_LINK */
                  }
               }
               else if( pt_array != null )  /* !point */
               {
                  ShowAttachmentPoints( null, pt_array, sel_node, 1 );
               }
               else
               {
                  /* No point or no selection: erase attachment point feedback 
                     and add middle link points.
                  */
                  EraseAttachmentPoints();
                  
                  if( first_node )
                  {
                     // No first point yet: can't connect.
                     SetError( "Invalid connection point!" );  
                     break;
                  }
                  
                  // Add middle link point
                  AddLinkPoints( drag_link, 1 );
                  IHChangeBParameter( ih, "middle_point_added", true );
            
                  /* Set the last point of a linear link or the middle point of 
                     the arc link.
                  */
                  edge_type = IHGetIParameter( ih, "edge_type" );            
                  SetLastPoint( drag_link, cursor_pos_obj, false, 
                                edge_type == GlgObject.ARC );
                  AttachFramePoints( drag_link );
            
                  /* Set the last point of the arc link, offsetting it from the 
                     middle point.
                  */
                  if( edge_type == GlgObject.ARC )
                    SetLastPoint( drag_link, cursor_pos_obj, true, false );
               }
               Update();
               break;  
               
             case IH_FINISH_LINK:    // Finish the current link.
               drag_link = IHGetOptOParameter( ih, "drag_link", null );
               if( drag_link != null )
               {


                  // Finish the last link
                  if( AllowUnconnectedLinks && FinishLink( drag_link ) )
                    ;   // Keep the link even if its second end is not connected.
                  else
                  {
                     // Delete the link if its second end is not connected.
                     GlgObject group = 
                       DrawingArea.GetResourceObject( "ObjectGroup" );
                     group.DeleteObject( drag_link );
                  }


                  IHChangeOParameter( ih, "drag_link", null );
               }
               EraseAttachmentPoints();   
               Update();
               break;
               
             case IH_ESC:
             case IH_MOUSE_BUTTON3:
               drag_link = IHGetOptOParameter( ih, "drag_link", null );
               if( drag_link != null && StickyCreateMode )
               {
                  // Stop adding points to this link.
                  IHCallCurrIHWithToken( IH_FINISH_LINK );
                  IHResetup( ih );   // Start over to create more links.
               }
               else
                 /* No curr link or !StickyCreateMode: finish the current link 
                    if any and stop adding links.
                 */
                 IHUninstall();      // Will call IH_FINISH_LINK
               break;
         
             default:
               IHUninstallWithEvent( call_event ); // Pass to the parent IH.
               break;
            }
            break;
            
          case CLEANUP_EVENT:
            IHCallCurrIHWithToken( IH_FINISH_LINK ); // Finish the current link

            TraceMouseMove = false;
            SetRadioBox( SELECT_BUTTON_NAME );   // Highlight Select button
            SetPrompt( "" );
            Update();
            break;
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////
   // Finds attachment point(s) of a node under the cursor.
   //
   // Stores the node and either the selected attachment point or all 
   // attachment points as parameters of the invoking IH: attachment_point, 
   // attachment_array and attachment_node.
   //
   // Stores nulls is no node is selected.
   ////////////////////////////////////////////////////////////////////////
   void StoreAttachmentPoints( GlgObject cursor_pos_obj, int event_type )
   {
      GlgObject
        selection,
        sel_object,
        point = null,
        pt_array = null,
        sel_node = null;
      int i, num_selected;
   
      selection = GetObjectsAtCursor( cursor_pos_obj );

      if( selection != null && ( num_selected = selection.GetSize() ) != 0 )
      {
         /* Some object were selected, process the selection to find the point 
            to connect to */
         for( i=0; i < num_selected; ++i )
         {
            sel_object = (GlgObject) GetElement( selection, i );
            
            /* Find if the object itself is a link or a node, or if it's a part
               of a node. If it's a part of a node, get the node object ID.
            */
            ObjectInfo selection_info = GetSelectedObject( sel_object );
            sel_object = selection_info.glg_object;
            int selection_type = selection_info.type;
            
            if( selection_type == NODE )
            {
               int type = 
                 sel_object.GetDResource( "Type" ).intValue();
               if( type == GlgObject.REFERENCE )
               {
                  // Use ref's point as a connector.
                  point = sel_object.GetResourceObject( "Point" );
               }
               else  /* Node has multiple attachment points: get an array of 
                        attachment points. */
               {
                  pt_array = GetAttachmentPoints( sel_object, "CP" );
                  if( pt_array == null )
                    continue;

                  point = GetSelectedPoint( pt_array, cursor_pos_obj );
                  
                  /* Use attachment points array to highlight all attachment 
                     points only if no specific point is selected, and only
                     on the mouse move. On the mouse press, the specific point
                     is used to connect to.
                  */
                  if( point != null || event_type != IH_MOUSE_MOVED )
                    pt_array = null;
               }
               
               /* If found a point to connect to, stop search and use it.
                  If found a node with attachment points, stop search and
                  highlight the points.
               */
               if( point != null || pt_array != null )
               {
                  if( point != null )
                    // If found attachment point, reset pt_array
                    pt_array = null;
                  
                  sel_node = sel_object;
                  break;
               }
            }
            
            // No point to connect to: continue searching all selected objects.
         }
         
         // Store as parameters of the invoking handler.
         IHSetOParameter( IH_CURR, "attachment_point", point );
         IHSetOParameter( IH_CURR, "attachment_array", pt_array );
         IHSetOParameter( IH_CURR, "attachment_node", sel_node );
      }
   }
   
   ////////////////////////////////////////////////////////////////////////
   class EditPropertiesIH implements GlgIHHandlerInterface
   {
      GlgDiagram diagram;
      EditPropertiesIH( GlgDiagram diagram ) { this.diagram = diagram; }

      ////////////////////////////////////////////////////////////////////////
      public void EntryPoint( GlgObject ih, GlgCallEvent call_event )
      {
         int token;

         switch( IHGetType( call_event ) )
         {
          case HI_SETUP_EVENT:
            break;

          case MESSAGE_EVENT:
            IHSetBParameter( IH_GLOBAL, "token_used", true );

            token = IHGetToken( call_event );
            switch( token )
            {
             case IH_TEXT_INPUT_CHANGED:
               if( SelectedObject == null )
                 break;
               
               DialogDataChanged = true;
               break;
               
             case IH_PROPERTIES:
               FillData();
               Viewport.SetDResource( "Dialog/Visibility", 1.0 ); 
               Update();
               break;
               
             case IH_DATASOURCE_SELECT:   /* process diagram only */
               if( SelectedObject == null )
               {
                  SetPrompt( "Select an object in the drawing first." );
                  Bell();
                  Update();
                  break;
               }

               /* Returns with IH_DATASOURCE_SELECTED and $rval containing 
                  selected datasource string. 
               */
               IHInstall( new GetDataSourceIH( diagram ) );
               IHStart();
               break;
               
             case IH_DATASOURCE_SELECTED:   /* process diagram only */
               // Get the selection.
               String rval = IHGetSParameter( ih, "$rval" );
               Viewport.SetSResource( "Dialog/DialogDataSource/TextString",
                                      rval );
               DialogDataChanged = true;
               break;

             case IH_DIALOG_CANCEL:
               DialogDataChanged = false;
               FillData();
               Update();
               break;
               
             case IH_DIALOG_APPLY:
               ApplyDialogData();
               DialogDataChanged = false;
               Update();
               break;
               
             case IH_DIALOG_CLOSE:
               if( !DialogDataChanged )    // No changes: close the dialog.
               {
                  Viewport.SetDResource( "Dialog/Visibility", 0.0 );
                  Update();
                  break;
               }

               // Data changed: confirm discarding changes.
               
               /* Store the CLOSE action that initiated the confirmation,
                  to close the data dialog when confirmed.
               */
               IHSetIParameter( ih, "op", token );
         
               IHCallCurrIHWithToken( IH_DIALOG_CONFIRM_DISCARD );
               break;

             case IH_DIALOG_CONFIRM_DISCARD:
               /* Returns with IH_OK with IH_CANCEL.
                  All parameters are optional, except for the message parameter.
               */
               IHInstall( new ConfirmIH( diagram ) );
               IHSetSParameter( IH_NEW, "title", "Confirm Discarding Changes" );
               IHSetSParameter( IH_NEW, "message", 
                                "Do you want to save dialog changes?" );
               IHSetSParameter( IH_NEW, "ok_label", "Save" );
               IHSetSParameter( IH_NEW, "cancel_label", "Discard" );
               IHSetBParameter( IH_NEW, "modal_dialog", true ); 
               IHStart();
               break;
               
             case IH_OK:       // Save changes.
             case IH_CANCEL:   // Discard changes.
               IHCallCurrIHWithToken( token == IH_OK ? 
                                      IH_DIALOG_APPLY : IH_DIALOG_CANCEL );
         
               /* Close the data dialog if that's what initiated the 
                  confirmation. */
               if( IHGetOptIParameter( ih, "op", 0 ) == IH_DIALOG_CLOSE )
                 IHCallCurrIHWithToken( IH_DIALOG_CLOSE );
               break;
               
             case IH_ESC: 
               break;     // Allow: do nothing.
               
             default: 
               if( !DialogDataChanged )    // No changes.
               {
                  IHSetBParameter( IH_GLOBAL, "token_used", false );
                  UninstallPassTroughIH( call_event );
                  break;
               }
               
               /* Data changed: ignore the action and confirm discarding changes.
                  Alternatively, data changes could be applied automatically 
                  when the text field looses focus, the way it is done in the 
                  GLG editors, which would eliminate a need for a confirmation 
                  dialog for discarding changed data.
               */
               
               // Restore state of any ignored toggles.
               RestoreToggleStateWhenDisabled( token );
               
               IHCallCurrIHWithToken( IH_DIALOG_CONFIRM_DISCARD );
               break;
            }
            break;
            
          case CLEANUP_EVENT:
            DialogDataChanged = false;
            break;
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////
   class GetDataSourceIH implements GlgIHHandlerInterface
   {
      GlgDiagram diagram;
      GetDataSourceIH( GlgDiagram diagram ) { this.diagram = diagram; }

      ////////////////////////////////////////////////////////////////////////
      public void EntryPoint( GlgObject ih, GlgCallEvent call_event )
      {
         int token;

         switch( IHGetType( call_event ) )
         {
          case HI_SETUP_EVENT:
            Viewport.SetDResource( "DataSourceDialog/Visibility", 1.0 ); 
            Update();
            break;

          case MESSAGE_EVENT:
            token = IHGetToken( call_event );
            switch( token )
            {
             case IH_DATASOURCE_APPLY:
             case IH_DATASOURCE_LIST_SELECTION:
               String sel_item = 
                 Viewport.GetSResource( "DataSourceDialog/DSList/SelectedItem" );
               IHUninstall();

               /* Set the return value in the parent datastore and call the 
                  parent. */
               IHSetSParameter( IH_CURR, "$rval", sel_item );
               IHCallCurrIHWithToken( IH_DATASOURCE_SELECTED ); 
               break;
               
             case IH_DATASOURCE_CLOSE:
               IHUninstall();
               break;

             default: 
               IHUninstallWithEvent( call_event );
               break;
            }
            break;
            
          case CLEANUP_EVENT:
            Viewport.SetDResource( "DataSourceDialog/Visibility", 0.0 ); 
            Update();
            break;
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////
   class ConfirmIH implements GlgIHHandlerInterface
   {
      GlgDiagram diagram;
      ConfirmIH( GlgDiagram diagram ) { this.diagram = diagram; }

      ////////////////////////////////////////////////////////////////////////
      // OK/Cancel confirmation dialog. 
      //
      // Parameters:
      //   message
      //   title (optional, default "Confirm")
      //   ok_label (optional, def. "OK")
      //   cancel_label (optional, def. "Cancel")
      //   modal_dialog (optional, def. true)
      //   allow_ESC (optional, def. true )
      //   requested_op (optional, default - undefined (0) )
      ////////////////////////////////////////////////////////////////////////
      public void EntryPoint( GlgObject ih, GlgCallEvent call_event )
      {
         int token;
         GlgObject dialog;
         
         switch( IHGetType( call_event ) )
         {
          case HI_SETUP_EVENT:
            dialog = Viewport.GetResourceObject( "OKDialog" );
            dialog.SetSResource( "ScreenName",
                                 IHGetOptSParameter( ih, "title", "Confirm" ) );
            dialog.SetSResource( "OKDialogOK/LabelString",
                                 IHGetOptSParameter( ih, "ok_label", "OK" ) );
            dialog.SetSResource( "OKDialogCancel/LabelString",
                                 IHGetOptSParameter( ih, "cancel_label", 
                                                     "Cancel" ) );
            dialog.SetSResource( "DialogMessage/String", 
                                 IHGetSParameter( ih, "message" ) );
            dialog.SetDResource( "Visibility", 1.0 );
            Update();
            break;
            
          case MESSAGE_EVENT:
            token = IHGetToken( call_event );
            switch( token )
            {
             case IH_OK:
             case IH_CANCEL:
               if( IHGetOptIParameter( ih, "requested_op", 0 ) == IH_EXIT )
               {
                  if( token == IH_OK )
                    System.exit( 0 );
                  else  // IH_CANCEL
                    IHUninstall();     // Do nothing
                  break;
               }

               IHUninstallWithToken( token );   // Pass selection to the parent.
               break;

             case IH_EXIT:
               /* Allow to quit the application in the modal mode if Exit 
                  is pressed twice.
               */
               System.exit( 0 );
               break;

             case IH_ESC:
               if( IHGetOptBParameter( ih, "allow_ESC", true ) )
                 IHUninstall();
               break;

             default: 
               if( IHGetOptBParameter( ih, "modal_dialog", true ) )
               {
                  RestoreToggleStateWhenDisabled( token );
                  SetPrompt( "Please select one of the choices from the dialog." );
                  Bell();
                  Update();
               }
               else
                 IHUninstallWithEvent( call_event );
               break;
            }
            break;

          case CLEANUP_EVENT:
            Viewport.SetDResource( "OKDialog/Visibility", 0.0 );
            Update();
            break;
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////
   // Restore state of any pressed toggles ignored or disabled by the 
   // confirmation dialog.
   ////////////////////////////////////////////////////////////////////////
   void RestoreToggleStateWhenDisabled( int token )
   {
      switch( token )
      {
       case IH_ICON_SELECTED:
         DeselectButton( IHGetSParameter( IH_GLOBAL, "$selected_button" ) );
         break;
         
       case IH_CREATION_MODE:
         SetCreateMode( true );
         break;
      }
   }

   ////////////////////////////////////////////////////////////////////////
   void UninstallPassTroughIH( GlgCallEvent call_event )
   {
      if( IHGetBParameter( IH_GLOBAL, "fall_through_call" ) )
        /* A fall-through invokation: a parent handler passed an unused event 
           to this IH for possible processing, discard The event. Passing 
           the event to the parent IH would cause infinite recursion.
        */
        IHUninstall();
      else
        /* Not a pass-through invokation: the IH was not uninstalled and 
           is current. Pass an unused event to the parent handler for
           processing.
        */              
        IHUninstallWithEvent( call_event );           
   }

   ////////////////////////////////////////////////////////////////////////
   void ShowAttachmentPoints( GlgObject point, GlgObject pt_array, 
                              GlgObject sel_node, int highlight_type )
   {
      GlgPoint screen_point;

      if( point != null )
      {
         if( AttachmentArray != null )
           EraseAttachmentPoints();   
         
         // Get screen coords of the connector point, not the cursor
           // position: may be a few pixels off.
         screen_point = point.GetGResource( "XfValue" );

         DrawingArea.ScreenToWorld( true, screen_point, world_coord );

         if( AttachmentMarker == null )
         {
            AttachmentMarker = PointMarker;
            DrawingArea.AddObjectToBottom( AttachmentMarker );
         }

         // Position the feedback marker over the connector.
         AttachmentMarker.SetGResource( "Point", world_coord );

         AttachmentMarker.SetDResource( "HighlightType", 
                                       (double) highlight_type );
      }
      else if( pt_array != null )
      {
         if( sel_node == AttachmentNode )
           return;    // Attachment points are already shown for this node.
             
         // Erase previous attachment feedback if shown.
         EraseAttachmentPoints();   

         int size = pt_array.GetSize();
         AttachmentArray = 
           new GlgDynArray( GlgObject.GLG_OBJECT, size, 0 );
         AttachmentNode = sel_node;
         
         for( int i=0; i<size; ++i )
         {
            GlgObject marker = PointMarker.CopyObject();
            
            point = (GlgObject) pt_array.GetElement( i );
            
            // Get the screen coords of the connector point.
            screen_point = point.GetGResource( "XfValue" );
            
            DrawingArea.ScreenToWorld( true, screen_point, world_coord );
            
            // Position the feedback marker over the connector.
            marker.SetGResource( "Point", world_coord );
            
            marker.SetDResource( "HighlightType", (double)highlight_type );

            AttachmentArray.AddObjectToBottom( marker );
         }
         DrawingArea.AddObjectToBottom( AttachmentArray );
      }
   }

   ////////////////////////////////////////////////////////////////////////
   // Erases attachment points feedback if shown. Returns true if feedback
   // was erased, of false if there was nothing to erase.
   ////////////////////////////////////////////////////////////////////////
   boolean EraseAttachmentPoints()
   {
      if( AttachmentMarker != null )
      {
         DrawingArea.DeleteObject( AttachmentMarker );
         AttachmentMarker = null;
         return true;
      }

      if( AttachmentArray != null )
      {
         DrawingArea.DeleteObject( AttachmentArray );
         AttachmentArray = null;
         AttachmentNode = null;
         return true;
      }

      return false;    // Nothing to erase.
   }

   ////////////////////////////////////////////////////////////////////////
   void FinalizeLink( GlgObject link )
   {
      GlgObject arrow_type = link.GetResourceObject( "ArrowType" );
      if( arrow_type != null )
        arrow_type.SetDResource( null, (double) GlgObject.MIDDLE_FILL_ARROW );

      GlgLinkData link_data = (GlgLinkData) GetData( link );

      // Store points
      StorePointData( link_data, link );
   
      // Add link data to the link list
      ArrayList<Object> link_list = CurrentDiagram.getLinkList();
      link_list.add( link_data );
   
      // After storing color: changes color to select.
      SelectGlgObject( link, LINK );
   
      CustomHandler.AddObjectCB( this, link, GetData( link ), false );
   }

   ////////////////////////////////////////////////////////////////////////
   // Stores point coordinates in the link data structure as an array.
   ////////////////////////////////////////////////////////////////////////
   void StorePointData( GlgLinkData link_data, GlgObject link )
   {
      ObjectInfo link_info = GetCPContainer( link );
      GlgObject point_container = link_info.glg_object;
      
      int num_points = point_container.GetSize();

      // Create a new array and discard the old one for simplicity.
      link_data.point_array = new ArrayList<GlgDiagramPoint>( num_points );
      for( int i=0; i<num_points; ++i )
      {
         GlgObject point = (GlgObject) point_container.GetElement( i );
         GlgPoint xyz = point.GetGResource( null );
         link_data.point_array.add( new GlgDiagramPoint( xyz ) );
      }
   }
      
   ////////////////////////////////////////////////////////////////////////
   // Restores link's middle points from the link data's stored vector.
   // The first and last point's values are not used: they are constrained 
   // to nodes and positioned/controlled by them.
   ////////////////////////////////////////////////////////////////////////
   void RestorePointData( GlgLinkData link_data, GlgObject link )
   {
      // Set middle point values
      if( link_data.point_array != null )
      {
         int num_points = link_data.point_array.size();

         ObjectInfo link_info = GetCPContainer( link );
         GlgObject point_container = link_info.glg_object;

         // Skip the first and last point if they are constrained to nodes.
           // Set only the unconnected ends and middle points.
         int start = ( link_data.getStartNode() != null ? 1 : 0 );
         int end = 
           ( link_data.getEndNode() != null ? num_points - 1 : num_points );

         // Skip the first and last point: constrained to nodes.
           // Set only the middle points.
         for( int i=start; i<end; ++i )
         {
            GlgObject point = (GlgObject) point_container.GetElement( i );
            GlgDiagramPoint xyz = link_data.point_array.get( i );
            point.SetGResource( null, xyz );
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////
   GlgPoint GetPointFromObj( GlgObject point_obj )
   {
      return point_obj.GetGResource( null );
   }

   ////////////////////////////////////////////////////////////////////////
   void MoveObject( GlgObject object, GlgObject start_point_obj,
                    GlgObject end_point_obj )
   {
      object.MoveObject( GlgObject.SCREEN_COORD, 
                         GetPointFromObj( start_point_obj ), 
                         GetPointFromObj( end_point_obj ) );
   }

   ////////////////////////////////////////////////////////////////////////
   // If the link is attached to nodes that use reference objects, moving the
   // link moves the nodes, with no extra actions required. However, the link
   // can be connected to a node with multiple attachment points which doesn't
   // use reference object. Moving such a link would move just the attachment
   // points, but not the nodes. To avoid this, unsconstrain the end points
   // of the link not to spoil the attachment points' geometry, move the link,
   // move the nodes, then constrain the link points back.
   ////////////////////////////////////////////////////////////////////////
   void MoveLink( GlgObject link, GlgObject start_point_obj, 
                  GlgObject end_point_obj )
   {
      GlgObject 
        start_node = null,
        end_node = null;
      int
        type1 = GlgObject.REFERENCE, 
        type2 = GlgObject.REFERENCE;

      GlgLinkData link_data = (GlgLinkData) GetData( link );

      if( link_data.getStartNode() != null )
      {         
         start_node = link_data.getStartNode().graphics;
         type1 = start_node.GetDResource( "Type" ).intValue();
      }
      
      if( link_data.getEndNode() != null )
      {
         end_node = link_data.getEndNode().graphics;
         type2 = end_node.GetDResource( "Type" ).intValue();
      }

      GlgPoint start_point = GetPointFromObj( start_point_obj );
      GlgPoint end_point = GetPointFromObj( end_point_obj );
     
      if( type1 == GlgObject.REFERENCE && type2 == GlgObject.REFERENCE )
      {
         // Nodes are reference objects (or null), moving the link moves nodes.
         link.MoveObject( GlgObject.SCREEN_COORD, start_point, end_point );
      }
      else   // Nodes with multiple attachment points.
      {
         // Unconstrain link points
         GlgObject point1 = null, point2 = null;

         if( start_node != null )
           point1 = UnConstrainLinkPoint( link, false );
         if( end_node != null )
           point2 = UnConstrainLinkPoint( link, true );

         DetachFramePoints( link );
      
         // Move the link
         link.MoveObject( GlgObject.SCREEN_COORD, start_point, end_point );
      
         // Move start node, then reattach the link.
         if( start_node != null )
         {
            start_node.MoveObject( GlgObject.SCREEN_COORD, 
                                  start_point, end_point );
            ConstrainLinkPoint( link, point1, false );
         }

         // Move end node, then reattach the link.
         if( end_node != null )
         {
            end_node.MoveObject( GlgObject.SCREEN_COORD, 
                                start_point, end_point );
            ConstrainLinkPoint( link, point2, true );
         }

         AttachFramePoints( link );
      }
   }

   ////////////////////////////////////////////////////////////////////////
   void UpdateNodePosition( GlgObject node, GlgNodeData node_data )
   {
      if( node_data == null )
        node_data = (GlgNodeData) GetData( node );

      GetPosition( node, world_coord );
      node_data.position.x = world_coord.x;
      node_data.position.y = world_coord.y;
   }

   ////////////////////////////////////////////////////////////////////////
   public GlgObject GetObjectsAtCursor( GlgObject cursor_pos_obj )
   {
      GlgPoint cursor_pos = GetPointFromObj( cursor_pos_obj );

      /* Select all objects in the vicinity of the +-SELECTION_RESOLUTION 
         pixels from the actual mouse click position.
      */
      select_rect.p1.x = cursor_pos.x - SELECTION_RESOLUTION;
      select_rect.p1.y = cursor_pos.y - SELECTION_RESOLUTION;
      select_rect.p2.x = cursor_pos.x + SELECTION_RESOLUTION;
      select_rect.p2.y = cursor_pos.y + SELECTION_RESOLUTION;

      return GlgObject.CreateSelection( DrawingArea, select_rect, DrawingArea );
   }

   ////////////////////////////////////////////////////////////////////////
   void SelectGlgObject( GlgObject glg_object, int selected_type )
   {
      String name;

      if( glg_object == SelectedObject )
        return;   // No change
      
      if( last_color != null ) // Restore the color of previously selected node
      {
         last_color.SetResourceFromObject( null, StoredColor );
         last_color = null;
      }

      SelectedObject = glg_object;
      SelectedObjectType = selected_type;

      // Show object selection
      if( glg_object != null )
      {
         // Change color to highlight selected node or link.
         if( glg_object.HasResourceObject( "SelectColor" ) )
         {
            last_color = glg_object.GetResourceObject( "SelectColor" );

            // Store original color
            StoredColor.SetResourceFromObject( null, last_color );

            // Set color to red to highlight selection.
            last_color.SetGResource( null, 1.0, 0.0, 0.0 );
         }
         name = GetObjectLabel( SelectedObject );
      }
      else
        name = "NONE";

      // Display selected object name at the bottom.
      Viewport.SetSResource( "SelectedObject", name );

      FillData();
   }

   ////////////////////////////////////////////////////////////////////////
   void Cut()
   {
      if( NoSelection() )
        return;

      // Disallow deleting a node without deleting the link first.
      if( SelectedObjectType == NODE && NodeConnected( SelectedObject ) )
      {
         SetError( "Remove links connected to the node before removing the node!" );
         return;
      }

      GlgObject group = DrawingArea.GetResourceObject( "ObjectGroup" );

      if( group.ContainsObject( SelectedObject ) )
      {
         // Store the node or link in the cut buffer.
         CutBuffer = SelectedObject;
         CutBufferType = SelectedObjectType;

         // Delete the node
         group.DeleteObject( SelectedObject );

         // Delete the data
         Object data = GetData( SelectedObject );
         ArrayList<Object> list = null;

         CustomHandler.CutObjectCB( this, SelectedObject, data,
                                    SelectedObjectType == NODE );

         if( SelectedObjectType == NODE )
           list = CurrentDiagram.getNodeList();
         else  // Link
           list = CurrentDiagram.getLinkList();

         if( !list.remove( data ) )
           System.out.println( "Deleting data failed!" );

         SelectGlgObject( null, 0 );
      }
      else
        SetError( "Cut failed." );    
   }

   ////////////////////////////////////////////////////////////////////////
   void Paste()
   {
      if( CutBuffer == null )
      {
         SetError( "Empty cut buffer, cut some object first." );
         return;
      }
      
      GlgObject group = DrawingArea.GetResourceObject( "ObjectGroup" );

      Object data = GetData( CutBuffer );
      CustomHandler.PasteObjectCB( this, CutBuffer, data,
                                   CutBufferType == NODE );
      ArrayList<Object> list = null;

      if( CutBufferType == NODE )
      {
         group.AddObjectToBottom( CutBuffer );     // In front
         list = CurrentDiagram.getNodeList();
         list.add( data );
      }
      else // LINK
      {
         group.AddObjectToTop( CutBuffer );        // Behind
         list = CurrentDiagram.getLinkList();
         list.add( data );
      }

      SelectGlgObject( CutBuffer, CutBufferType );

      // Allow pasting just once to avoid handling the data copy
      CutBuffer = null;
   }

   ////////////////////////////////////////////////////////////////////////
   void Save( GlgDiagramData diagram )
   {
      // Print all nodes and edges as a test
      ArrayList<Object> node_list = diagram.getNodeList();
      ArrayList<Object> link_list = diagram.getLinkList();

      int i;
      for( i=0; i<node_list.size(); ++i )
        WriteLine( "Node " + i + ", type " + 
                   ((GlgNodeData) node_list.get( i )).node_type );

      for( i=0; i<link_list.size(); ++i )
      {
         GlgLinkData link = (GlgLinkData) link_list.get( i );
         WriteLine( "Link " + i + ", type " + link.link_type );
         PrintLinkInfo( " from node ", node_list,
                       link.start_node, link.start_point_name );
         PrintLinkInfo( " to node ", node_list,
                       link.end_node, link.end_point_name );
      }

      // Save the DiagramData class using either Java serialization,
      // writing out individual records, or any other custom save method.

      // Save the current diagram to use it as test for loading.
      SavedDiagram = diagram;

      // Empty the drawing area
      UnsetDiagram( diagram );
   }

   ////////////////////////////////////////////////////////////////////////
   void PrintLinkInfo( String label, ArrayList<Object> node_list, 
                       GlgNodeData node, String point_name )
   { 
      String output_string;      

      if( node == null )
        output_string = label + "null ";
      else
      {
         output_string = label + node_list.indexOf( node );
         
         // Print connection point info if not the default point.
         if( !point_name.equals( "Point" ) )
           output_string += ":" + point_name;
      }

      WriteLine( output_string );
   }

   ////////////////////////////////////////////////////////////////////////
   void Load()
   {
      // Load the DiagramClass using Java de-serialization, reading individual
      // records or any other custom load method matching the save method
        // used above.

      // In the demo, load the saved diagram and use it as a Save/Load test.
      if( SavedDiagram == null )
        SetError( "Save the diagram first." );
      else
      {
         UnsetDiagram( CurrentDiagram );
      
         // Load saved diagram
         SetDiagram( SavedDiagram );
         SavedDiagram = null;
      }
   }

   ////////////////////////////////////////////////////////////////////////
   void Print()
   {
      // May be used to export postscript, generate an image to be saved, 
      // or invoke native Java printing (see Glg Java print examples).

      // In an applet printing is handled by the browser's print button.
   }

   ////////////////////////////////////////////////////////////////////////
   // If AllowUnconnectedLinks=true, keep the link if it has at least two 
   // points.
   ////////////////////////////////////////////////////////////////////////
   boolean FinishLink( GlgObject link )
   {
      ObjectInfo link_info = GetCPContainer( link );

      GlgObject point_container = link_info.glg_object;

      int edge_type = link_info.type;
      if( edge_type == GlgObject.ARC )
        return false;      // Disconnected arc links are not allowed.

      int size = point_container.GetSize();

      // The link must have at least two points already defined, and one extra
        // point that was added to drag the next point.
      if( size < 3 )
        return false;

      // Delete the unfinished, unconnected point.
      GlgObject suspend_info = link.SuspendObject();
      point_container.DeleteBottomObject();
      link.ReleaseObject( suspend_info );

      FinalizeLink( link );
      return true;
   }

   ////////////////////////////////////////////////////////////////////////
   void SetPrompt( String message )
   {
      Viewport.SetSResource( "Prompt/String", message );
      Update();
   }

   ////////////////////////////////////////////////////////////////////////
   void SetError( String message )
   {
      Bell();
      SetPrompt( message );
   }

   ////////////////////////////////////////////////////////////////////////
   boolean NoSelection()
   {
      if( SelectedObject != null )
        return false;
      else
      {
         SetError( "Select some object first." );
         return true;
      }
   }

   ////////////////////////////////////////////////////////////////////////
   GlgObject AddNodeAt( int node_type, GlgNodeData node_data, 
                        GlgObject position_obj, int coord_system )
   {     
      boolean store_position;

      if( node_data == null )
      {
         node_data = new GlgNodeData();
         node_data.node_type = node_type;

         // Add node data to the node list
         ArrayList<Object> node_list = CurrentDiagram.getNodeList();
         node_list.add( node_data );

         if( ProcessDiagram )
         {
            // Assign an arbitrary datasource initially.
            node_data.datasource = "DataSource" + DataSourceCounter;
         
            ++DataSourceCounter;
            if( DataSourceCounter >= NumDatasources )
              DataSourceCounter = 0;
         }
      }

      // Create the node based on the node type
      GlgObject new_node = CreateNode( node_data );

      // Make label visible and set its string.
      if( new_node.HasResourceObject( "Label" ) )
      {
         new_node.SetDResource( "Label/Visibility", 1.0 );
         new_node.SetSResource( "Label/String", node_data.object_label );
      }

      // Store datasource as a tag of the node's Value resource, if it exists.
      if( ProcessDiagram )
      {
         GlgObject value_obj = new_node.GetResourceObject( "Value" );
         String datasource = node_data.datasource;
         
         if( value_obj != null && 
            datasource != null && datasource.length() != 0  )
         {
            GlgObject tag_obj = new GlgTag( "Value", datasource,  null );
            value_obj.SetResourceObject( "TagObject", tag_obj );
         }
      }

      GlgPoint position;

      // No cursor pos: get pos from the data struct.      
      if( position_obj == null )
      {
         position = 
           new GlgPoint( node_data.position.x, node_data.position.y, 0.0 );
         store_position = false;
      }
      else
      {
         position = GetPointFromObj( position_obj );
         store_position = true;
      }

      node_data.graphics = new_node;  // Pointer from data struct to graphics

      // Add the object to the drawing first, so that it's hierarchy is setup
        // for positioning it.
      GlgObject group = DrawingArea.GetResourceObject( "ObjectGroup" );
      group.AddObjectToBottom( new_node );

      // Transform the object to set its size and position.
      PlaceObject( new_node, position, coord_system, world_coord );

      if( store_position )
      {
         node_data.position.x = world_coord.x;
         node_data.position.y = world_coord.y;
      }
      return new_node;
   }

   ////////////////////////////////////////////////////////////////////////
   GlgObject CreateNode( GlgNodeData node_data )
   {
      // Get node template from the palette
      GlgObject new_node = (GlgObject)
        NodeObjectArray.GetElement( node_data.node_type );
         
      // Create a new node instance 
      new_node = new_node.CloneObject( GlgObject.STRONG_CLONE );

      // Name node using an "object" prefix (used to distiguish
        // nodes from links on selection).
      new_node.SetSResource( "Name", "object" );

      AddCustomData( new_node, node_data );

      if( ProcessDiagram )
      {
         // Init label data using node's InitLabel if exists.
         if( new_node.HasResourceObject( "InitLabel" ) )
           node_data.object_label = new_node.GetSResource( "InitLabel" );
      }

      return new_node;
   }

   ////////////////////////////////////////////////////////////////////////
   GlgObject CreateLink( GlgLinkData link_data )
   {
      int num_points;

      // Get link template from the palette
      GlgObject new_link = (GlgObject)
        LinkObjectArray.GetElement( link_data.link_type );
         
      // Create a new link instance 
      new_link = new_link.CloneObject( GlgObject.STRONG_CLONE );

      // Name link using a "link" prefix (used to distiguish
        // links from nodes on selection).
      new_link.SetSResource( "Name", "link" );

      // If point_array exists, create/add middle link points
      // If not an arc, it's created with 2 points by default, 
      // add ( num_points - 2 ) more. If it's an arc, AddLinkPoints
        // will do nothing.
      if( link_data.point_array != null &&
         ( num_points = link_data.point_array.size() ) > 2 )
        AddLinkPoints( new_link, num_points - 2 );

      AddCustomData( new_link, link_data );

      return new_link;
   }

   /////////////////////////////////////////////////////////////////////////
   // Connects the first or last point of the link.
   /////////////////////////////////////////////////////////////////////////
   void ConstrainLinkPoint( GlgObject link, GlgObject point, 
                           boolean last_point )
   {
      ObjectInfo link_info = GetCPContainer( link );

      GlgObject point_container = link_info.glg_object;

      GlgObject link_point = (GlgObject) 
        point_container.GetElement(  
                ( last_point ? point_container.GetSize() - 1 : 0 ) );
   
      GlgObject suspend_info = link.SuspendObject();
      link_point.ConstrainObject( point );
      link.ReleaseObject( suspend_info );

      // Store the point name for save/load
      String point_name = point.GetSResource( "Name" );
      if( point_name == null || point_name.length() == 0 )
        point_name = "Point";
   
      GlgLinkData link_data = (GlgLinkData) GetData( link );
      if( last_point )
        link_data.end_point_name = point_name;
      else
        link_data.start_point_name = point_name;
   }


   /////////////////////////////////////////////////////////////////////////
   // Positions the arc's middle point if it's not explicitly defined.
   /////////////////////////////////////////////////////////////////////////
   void SetArcMiddlePoint( GlgObject link )
   {
      ObjectInfo link_info = GetCPContainer( link );
      GlgObject point_container = link_info.glg_object;
      int edge_type = link_info.type;

      if( edge_type != GlgObject.ARC )
        return;

      // Offset the arc's middle point if wasn't set.
      GlgObject start_point = (GlgObject) point_container.GetElement( 0 );
      GlgObject middle_point = (GlgObject) point_container.GetElement( 1 );
      GlgObject end_point = (GlgObject) point_container.GetElement( 2 );

      GlgPoint pt1 = start_point.GetGResource( null );
      GlgPoint pt2 = end_point.GetGResource( null );

      // Offset the middle point.
      middle_point.SetGResource( null,
              ( pt1.x + pt2.x ) / 2.0 + ( pt1.y - pt2.y != 0.0 ? 50.0 : 0.0 ),
              ( pt1.y + pt2.y ) / 2.0 + ( pt1.y - pt2.y != 0.0 ? 0.0 : 50.0 ),
              ( pt1.z + pt2.z ) / 2.0 );
   }

   /////////////////////////////////////////////////////////////////////////
   // Handles links with labels: constrains frame's points to the link's 
   // points.
   /////////////////////////////////////////////////////////////////////////
   void AttachFramePoints( GlgObject link )
   {
      GlgObject
        frame,
        link_point_container,
        frame_point_container,
        link_start_point,
        link_end_point,
        frame_start_point,
        frame_end_point,
        suspend_info;

      frame = link.GetResourceObject( "Frame" );
      if( frame == null ) // Link without label and frame
        return;
      
      ObjectInfo link_info = GetCPContainer( link );
      link_point_container = link_info.glg_object;

      // Always use the first segment of the link to attach the frame.
      link_start_point = (GlgObject) link_point_container.GetElement( 0 );
      link_end_point = (GlgObject) link_point_container.GetElement( 1 );
      
      frame_point_container = frame.GetResourceObject( "CPArray" );
      int size = frame_point_container.GetSize();
      frame_start_point =(GlgObject)  frame_point_container.GetElement( 0 );
      frame_end_point =
        (GlgObject) frame_point_container.GetElement( size - 1 );
      
      suspend_info = link.SuspendObject();
      
      frame_start_point.ConstrainObject( link_start_point );
      frame_end_point.ConstrainObject( link_end_point );
      
      link.ReleaseObject( suspend_info );
   }

   /////////////////////////////////////////////////////////////////////////
   // Disconnects the first or last point of the link. Returns the object 
   // the link is connected to.
   /////////////////////////////////////////////////////////////////////////
   GlgObject UnConstrainLinkPoint( GlgObject link, boolean last_point )
   {
      ObjectInfo link_info = GetCPContainer( link );

      GlgObject point_container = link_info.glg_object;

      GlgObject link_point = (GlgObject)
        point_container.GetElement(
                ( last_point ? point_container.GetSize() - 1 : 0 ) );
      GlgObject attachment_point = link_point.GetResourceObject( "Data" );

      GlgObject suspend_info = link.SuspendObject();
      link_point.UnconstrainObject();
      link.ReleaseObject( suspend_info );

      return attachment_point;
   }
   
   /////////////////////////////////////////////////////////////////////////
   // Detaches the first and last points of the frame.
   /////////////////////////////////////////////////////////////////////////
   void DetachFramePoints( GlgObject link )
   {
      GlgObject
        frame,
        frame_point_container,
        frame_start_point,
        frame_end_point,
        suspend_info;
      
      frame = link.GetResourceObject( "Frame" );
      if( frame == null ) // Link without label and frame
        return;
      
      frame_point_container = frame.GetResourceObject( "CPArray" );
      int size = frame_point_container.GetSize(); 
      frame_start_point = (GlgObject) frame_point_container.GetElement( 0 );
      frame_end_point =
        (GlgObject) frame_point_container.GetElement( size - 1 );
      
      suspend_info = link.SuspendObject();
      
      frame_start_point.UnconstrainObject();
      frame_end_point.UnconstrainObject();
      
      link.ReleaseObject( suspend_info );
   }

   /////////////////////////////////////////////////////////////////////////
   // Set last point of the link (dragging).
   /////////////////////////////////////////////////////////////////////////
   void SetLastPoint( GlgObject link, GlgObject cursor_pos_obj, 
                      boolean offset, boolean arc_middle_point )
   {
      GlgObject point;

      ObjectInfo link_info = GetCPContainer( link );
      GlgObject point_container = link_info.glg_object;
      
      GlgPoint cursor_pos = GetPointFromObj( cursor_pos_obj );

      /* Offset the point: used to offset the arc's last point from the 
         middle one while dragging.
      */
      if( offset )
      {
         cursor_pos.x += 10.0;
         cursor_pos.y += 10.0;
      }

      if( arc_middle_point )
        // Setting the middle point of an arc.
        point = (GlgObject) point_container.GetElement( 1 );
      else
        // Setting the last point.
        point = (GlgObject)
          point_container.GetElement( point_container.GetSize() - 1 );

      DrawingArea.ScreenToWorld( true, cursor_pos, world_coord );
      point.SetGResource( null, world_coord );
   }

   /////////////////////////////////////////////////////////////////////////
   void AddLinkPoints( GlgObject link, int num_points )
   {
      ObjectInfo link_info = GetCPContainer( link );
      if( link_info.type == GlgObject.ARC )
        return; // Arc connectors have fixed number of points: don't add.

      GlgObject point_container = link_info.glg_object;

      GlgObject point = (GlgObject) point_container.GetElement( 0 );

      GlgObject suspend_info = link.SuspendObject();
      for( int i=0; i<num_points; ++i )
      {
         GlgObject add_point = point.CloneObject( GlgObject.FULL_CLONE );
         point_container.AddObjectToBottom( add_point );
      }
      link.ReleaseObject( suspend_info );
   }

   /////////////////////////////////////////////////////////////////////////
   // Set the direction of the recta-linera connector depending on the 
   // direction of the first mouse move.
   /////////////////////////////////////////////////////////////////////////
   void SetEdgeDirection( GlgObject link, GlgObject start_pos_obj, 
                          GlgObject end_pos_obj )
   {
      int direction;

      ObjectInfo link_info = GetCPContainer( link );
      int edge_type = link_info.type;

      if( edge_type == GlgObject.ARC || edge_type == 0 )   // Arc or polygon
        return;

      GlgPoint start_pos = GetPointFromObj( start_pos_obj );
      GlgPoint end_pos = GetPointFromObj( end_pos_obj );


      if( Math.abs( start_pos.x - end_pos.x ) > 
         Math.abs( start_pos.y - end_pos.y ) )
        direction = GlgObject.HORIZONTAL;
      else
        direction = GlgObject.VERTICAL;

      link.SetDResource( "EdgeDirection", (double) direction );

      GlgLinkData link_data = (GlgLinkData) GetData( link );
      link_data.link_direction = direction;
   }

   ////////////////////////////////////////////////////////////////////////
   GlgObject AddLinkObject( int link_type, GlgLinkData link_data )
   {     
      GlgObject link;

      if( link_data == null )    // Creating a new link interactively
      {	 
         link_data = new GlgLinkData();
         link_data.link_type = link_type;

         link = CreateLink( link_data );

         // Store color
         link_data.link_color = 
           new GlgDiagramPoint( link.GetGResource( "EdgeColor" ) );
                  
         // Don't add link data to the link list or store points: 
         // will be done when finished creating the link.
      }
      else  // Creating a link from data on load.
      {
         link = CreateLink( link_data );

         // Set color
         link.SetGResource( "EdgeColor", link_data.link_color );

         // Enable arrow type if defined 
         GlgObject arrow_type = link.GetResourceObject( "ArrowType" );
         if( arrow_type != null )
           arrow_type.SetDResource( null,
                                   (double) GlgObject.MIDDLE_FILL_ARROW );

         // Restore connector direction if recta-linear
         GlgObject direction = link.GetResourceObject( "EdgeDirection" );
         if( direction != null )
           direction.SetDResource( null, (double) link_data.link_direction );

         // Constrain end points to start and end nodes 
         GlgNodeData start_node = link_data.getStartNode();
         if( start_node != null )
         {
            GlgObject node1 = start_node.graphics;
            GlgObject point1 = 
              node1.GetResourceObject( link_data.start_point_name  );
            ConstrainLinkPoint( link, point1, false ); // First point
         }
         
         GlgNodeData end_node = link_data.getEndNode();
         if( end_node != null )
         {
            GlgObject node2 = end_node.graphics;         
            GlgObject point2 = 
              node2.GetResourceObject( link_data.end_point_name );         
            ConstrainLinkPoint( link, point2, true ); // Last point
         }

         AttachFramePoints( link );

         RestorePointData( link_data, link );
      }

      // Display the label if it's a link with a label.
      if( link.HasResourceObject( "Label" ) )
        link.SetSResource( "Label/String", link_data.object_label );

      link_data.graphics = link;     // Pointer from data struct to graphics

      // Add to the top of the draw list to be behind other objects.
      GlgObject group = DrawingArea.GetResourceObject( "ObjectGroup" );
      group.AddObjectToTop( link );

      return link;
   }

   ////////////////////////////////////////////////////////////////////////
   // Set the object size and position.
   ////////////////////////////////////////////////////////////////////////
   void PlaceObject( GlgObject node, GlgPoint pos, int coord_type, 
                    GlgPoint world_coord )
   {
      int type;

      // World coordinates of the node are returned to be stored in the node's
        // data structure.
      if( coord_type == GlgObject.SCREEN_COORD ) 
        DrawingArea.ScreenToWorld( true, pos, world_coord );
      else
        world_coord.CopyFrom( pos );

      type = node.GetDResource( "Type" ).intValue();
      if( type == GlgObject.REFERENCE )
      {
         // Reference: can use its point to position it.
         node.SetGResource( "Point", world_coord );

         if( IconScale != 1.0 )   // Change node size if required.
           // Scale object around the origin, which is now located at pos.
           node.ScaleObject( coord_type, pos, IconScale, IconScale, 1.0 );
      }
      else
      {
         // Arbitrary object: move its box's center to the cursor position.
         node.PositionObject( coord_type, 
                              GlgObject.HCENTER | GlgObject.VCENTER,
                              pos.x, pos.y, pos.z );

         if( IconScale != 1.0 )   // Change node size if required.
           // Scale object around the center of it's bounding box.
           node.ScaleObject( coord_type, null, IconScale, IconScale, 1.0 );
      }
   }
      
   ////////////////////////////////////////////////////////////////////////
   // Get the link's control points container based on the link type.   
   ////////////////////////////////////////////////////////////////////////
   ObjectInfo GetCPContainer( GlgObject link )
   {
      ObjectInfo rval;
      int link_type;

      link_type = link.GetDResource( "Type" ).intValue();
      switch( link_type )
      {
       case GlgObject.POLYGON:
         rval = new ObjectInfo();
         rval.glg_object = link;
         rval.type = 0;
         break;

       case GlgObject.GROUP: // Group containing a polygon with a label
         return GetCPContainer( link.GetResourceObject( "Link" ) );

       case GlgObject.CONNECTOR:
         rval = new ObjectInfo();
         rval.glg_object = link;
         rval.type = link.GetDResource( "EdgeType" ).intValue();
         break;

       default: SetError( "Invalid link type." ); return null;
      }
      return rval;
   }

   ////////////////////////////////////////////////////////////////////////
   // Determines what node or link the object belongs to and returns it. 
   // Also returns type of the object: NODE or LINK.
   ////////////////////////////////////////////////////////////////////////
   ObjectInfo GetSelectedObject( GlgObject glg_object )
   {
      while( glg_object != null )
      {
         // Check if the object has IconType.
         if( glg_object.HasResourceObject( "IconType" ) )
         {
            String type_string = glg_object.GetSResource( "IconType" );
            if( type_string.equals( "Link" ) )
              return new ObjectInfo( glg_object, LINK );	   
            else if( type_string.equals( "Node" ) )
              return new ObjectInfo( glg_object, NODE );	   
         }

         glg_object = glg_object.GetParent();
      }

      // No node/link parent found - no selection.
      return new ObjectInfo( null, NO_OBJ );
   }

   ////////////////////////////////////////////////////////////////////////
   // Returns an array of all attachment points, i.e. the points whose 
   // names start with the name_prefix.
   ////////////////////////////////////////////////////////////////////////
   GlgObject GetAttachmentPoints( GlgObject sel_object, String name_prefix )
   {
      GlgObject pt_array = sel_object.CreatePointArray( 0 );
      if( pt_array == null )
        return null;

      int size = pt_array.GetSize();
      GlgObject attachment_pt_array = 
        new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );

      // Add points that start with the name_prefix to attachment_pt_array.
      for( int i=0; i<size; ++i )
      {
         GlgObject point = (GlgObject) pt_array.GetElement( i );
         String name = point.GetSResource( "Name" );
         if( name != null && name.startsWith(name_prefix) )
           attachment_pt_array.AddObjectToBottom( point );
      }

      if( attachment_pt_array.GetSize() == 0 )
        attachment_pt_array = null;

      return attachment_pt_array;
   }

   ////////////////////////////////////////////////////////////////////////
   // Checks if one of the point array's points is under the cursor.
   ////////////////////////////////////////////////////////////////////////
   GlgObject GetSelectedPoint( GlgObject point_array, GlgObject cursor_pos_obj )
   {
      if( point_array == null )
        return null;

      GlgPoint cursor_pos = GetPointFromObj( cursor_pos_obj );

      int size = point_array.GetSize();

      for( int i=0; i<size; ++i )
      {
         GlgObject point = (GlgObject) point_array.GetElement( i );

         // Get position in screen coords.
         GlgPoint screen_pos = point.GetGResource( "XfValue" );
         if( Math.abs( cursor_pos.x - screen_pos.x ) < 
                                                 POINT_SELECTION_RESOLUTION &&
             Math.abs( cursor_pos.y - screen_pos.y ) <
                                                 POINT_SELECTION_RESOLUTION )
           return point;	
      }
      return null;
   }

   ////////////////////////////////////////////////////////////////////////
   void SetDiagram( GlgDiagramData diagram )
   {
      int i;

      ArrayList<Object> node_list = diagram.getNodeList();
      ArrayList<Object> link_list = diagram.getLinkList();
      
      for( i=0; i<node_list.size(); ++i )
      {
         GlgNodeData node_data = (GlgNodeData) node_list.get( i );
         AddNodeAt( 0, node_data, null, GlgObject.PARENT_COORD );
      }

      for( i=0; i<link_list.size(); ++i )
      {
         GlgLinkData link_data = (GlgLinkData) link_list.get( i );
         AddLinkObject( 0, link_data );
      }

      CurrentDiagram = diagram;
      Update();
   }

   ////////////////////////////////////////////////////////////////////////
   void UnsetDiagram( GlgDiagramData diagram )
   {
      int i;

      SelectGlgObject( null, 0 );
      
      ArrayList<Object> node_list = diagram.getNodeList();
      ArrayList<Object> link_list = diagram.getLinkList();
      
      GlgObject group = DrawingArea.GetResourceObject( "ObjectGroup" );

      for( i=0; i<node_list.size(); ++i )
      {
         GlgNodeData node_data = (GlgNodeData) node_list.get( i );
         if( node_data.graphics != null )
           group.DeleteObject( node_data.graphics );
      }

      for( i=0; i<link_list.size(); ++i )
      {
         GlgLinkData link_data = (GlgLinkData) link_list.get( i );
         if( link_data.graphics != null )
            group.DeleteObject( link_data.graphics );
      }

      CurrentDiagram = new GlgDiagramData();
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Fills the object palette with buttons containing node and link icons
   // from the palette template. Palette template is a convenient place to 
   // edit all icons instead of placing them into the object palette buttons. 
   //
   // Icons named "Node0", "Node1", etc. were extracted into the NodeIconArray.
   // The LinkIconArray contains icons named "Link0", "Link1", etc.
   // Here, we place all node and link icons inside the object palette buttons 
   // named IconButton<N>, staring with the start_index to skip the first 
   // button which already contains the select button. 
   // The palette buttons are created by copying an empty template button.
   // Parameters:
   //  palette_name       Name of the object palette to add buttons to.
   //  button_name        Base name of the object palette buttons.
   //  start_index        Number of buttons to skip (the first button
   //                     with the select icon is already in the palette).
   //////////////////////////////////////////////////////////////////////////
   void FillObjectPalette( String palette_name, String button_name, 
                          int start_index )
   {
      GlgObject palette = Viewport.GetResourceObject( "ObjectPalette" );

      // Find and store an empty palette button used as a template.
      // Search the button at the top viewport level, since palette's
        // HasResources=NO.
      ButtonTemplate = Viewport.GetResourceObject( button_name + start_index );
      
      if( ButtonTemplate == null )
      {
         SetError( "Can't find palette button to copy!" );
         return;
      }
      
      // Delete the template button from the palette but keep it around.
      palette.DeleteObject( ButtonTemplate );

      // Store NumColumns info.
      NumColumns = ButtonTemplate.GetDResource( "NumColumns" ).intValue();

      // Add all icons from each array, increasing the start_index. */
      start_index = 
        FillObjectPaletteFromArray( palette, button_name, start_index,
                                   LinkIconArray, LinkObjectArray, "Link" );
      start_index = 
        FillObjectPaletteFromArray( palette, button_name, start_index,
                                   NodeIconArray, NodeObjectArray, "Node" );

      // Store the marker template for attachment points feedback.
      PointMarker = PaletteTemplate.GetResourceObject( "PointMarker" );
            
      // Cleanup
      ButtonTemplate = null;
      PaletteTemplate = null;
      NodeIconArray = null;
      LinkIconArray = null;
   }

   //////////////////////////////////////////////////////////////////////////
   // Adds object palette buttons containing all icons from an array.
   // icon_array is an array of icon objects to use in the palette button.
   // object_array is an array of objects to use in the drawing.
   //////////////////////////////////////////////////////////////////////////
   int FillObjectPaletteFromArray( GlgObject palette, String button_name, 
                                  int start_index, 
                                  GlgObject icon_array, GlgObject object_array,
                                  String default_tooltip )
   {
      // Add all icons from the icon array to the palette using a copy of 
        // the template button.
      int size = icon_array.GetSize();
      int button_index = start_index;
      for( int i=0; i<size; ++i )
      { 
         GlgObject icon = (GlgObject) icon_array.GetElement( i );
         GlgObject glg_object = (GlgObject) object_array.GetElement( i );

         // Set uniform icon name to simplify selection.
         icon.SetSResource( "Name", "Icon" );

         // For nodes, set initial label.
         if( default_tooltip.equals( "Node" ) && 
            glg_object.HasResourceObject( "Label" ) )
         {
            String label;
            if( glg_object.HasResourceObject( "InitLabel" ) )
              label = glg_object.GetSResource( "InitLabel" );
            else
              label = "";

            glg_object.SetSResource( "Label/String", label );
         }

         // Create a button to hold the icon.
         GlgObject button = 
           ButtonTemplate.CloneObject( GlgObject.STRONG_CLONE ); 

         // Set button name by appending its index as a suffix (IconButtonN).
         button.SetSResource( "Name", button_name +button_index );

         // Set tooltip string.
         GlgObject tooltip = icon.GetResourceObject( "TooltipString" );
         if( tooltip != null )
           // Use a custom tooltip from the icon if defined.
           button.SetResourceFromObject( "TooltipString", tooltip );
         else   // Use the supplied default tooltip.
           button.SetSResource( "TooltipString", default_tooltip );

         // Position the button by setting row and column indices.
         button.SetDResource( "RowIndex",
                             (double) ( button_index / NumColumns ) );
         button.SetDResource( "ColumnIndex",
                             (double) ( button_index % NumColumns ) );

         // Zoom palette icon button to scale icons displayed in it. 
         // Preliminary zoom by 10 for better fitting, will be precisely 
           // adjusted later. 
         button.SetDResource( "Zoom", DEFAULT_ICON_ZOOM_FACTOR );

         button.AddObjectToBottom( icon );

         palette.AddObjectToBottom( button );
         ++button_index;
      }

      return button_index; /* Return the next start index. */
   }
      
   //////////////////////////////////////////////////////////////////////////
   // Positions node icons inside the palette buttons.
   // Invoked after the drawing has been setup, which is required by 
   // PositionObject().
   // Parameters:
   //  button_name        Base name of the palette buttons.
   //  start_index        Number of buttons to skip (the select and link
   //                     buttons are already in the palette).
   //////////////////////////////////////////////////////////////////////////
   void SetupObjectPalette( String button_name, int start_index )
   {
      // Find icons in the palette template and add them to the palette,
      // using a copy of the template button.
      for( int i = start_index; ; ++i )
      {      
         GlgObject button = Viewport.GetResourceObject( button_name + i );

         if( button == null )
           return;    // No more buttons

         GlgObject icon = button.GetResourceObject( "Icon" );
         int type = icon.GetDResource( "Type" ).intValue();      

         if( type == GlgObject.REFERENCE )
           icon.SetGResource( "Point", 0.0, 0.0, 0.0 );  // Center position
         else
           icon.PositionObject( GlgObject.PARENT_COORD,
                                GlgObject.HCENTER | GlgObject.VCENTER,
                                0.0, 0.0, 0.0 );    // Center position

         double zoom_factor = GetIconZoomFactor( button, icon );

         // Query an additional icon scale factor if defined in the icon.
         if( icon.HasResourceObject( "IconScale" ) )
           zoom_factor *= icon.GetDResource( "IconScale" ).doubleValue();

         // Zoom palette icon button to scale icons displayed in it.
         button.SetDResource( "Zoom", zoom_factor );
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Returns a proper zoom factor to precisely fit the icon in the button.
   // Used for automatic fitting if FitIcons = true.
   //////////////////////////////////////////////////////////////////////////
   double GetIconZoomFactor( GlgObject button, GlgObject icon )
   {
      if( !FitIcons )
        return DEFAULT_ICON_ZOOM_FACTOR;
      
      GlgPoint 
        point1 = new GlgPoint(), 
        point2 = new GlgPoint();
      double
        extent_x, extent_y, extent,
        zoom_factor;
      
      zoom_factor = button.GetDResource( "Zoom" ).doubleValue();
      
      GlgCube box = icon.GetBox();
      button.ScreenToWorld( true, box.p1, point1 );
      button.ScreenToWorld( true, box.p2, point2 );
      
      extent_x = Math.abs( point1.x - point2.x );
      extent_y = Math.abs( point1.y - point2.y );
      extent = Math.max( extent_x, extent_y );
        
      // Increase zoom so that the icon fills the percentage of the button
        // defined by the ICON_FIT_FACTOR. 
      zoom_factor = 2000.0 / extent * ICON_FIT_FACTOR;
      return zoom_factor;
   }

   //////////////////////////////////////////////////////////////////////////
   // Queries items in the palette and fills array of node or link icons.
   // For each palette item, an icon is added to the icon_array, and the 
   // object to be used in the drawing is added to the object_array.
   // In case of connectors, the object uses only a part of the icon 
   // (the connector object) without the end markers.
   //////////////////////////////////////////////////////////////////////////
   void GetPaletteIcons( GlgObject palette, String icon_name, 
                        GlgObject icon_array, GlgObject object_array )
   {      
      for( int i=0; ; ++i )
      {
         // Get icon[i]
         GlgObject icon = palette.GetResourceObject( icon_name + i );
         if( icon == null )
           break;

         // Object to use in the drawing. In case of connectors, uses only a
         // part of the icon (the connector object) without the end markers.
           //
         GlgObject glg_object = icon.GetResourceObject( "Object" );
         if( glg_object == null )
           glg_object = icon;

         if( !glg_object.HasResourceObject( "IconType" ) )
         {
            SetError( "Can't find IconType resource." );
            continue;
         }

         String type_string = glg_object.GetSResource( "IconType" );

         // Using icon base name as icon type since they are the same,
         // i.e. "Node" and "Node", or "Link" and "Link".
           //
         if( type_string.equals( icon_name ) )
         {
            // Found an icon of requested type, add it to the array.
            icon_array.AddObjectToBottom( icon );
            object_array.AddObjectToBottom( glg_object );

            // Set index to match the index in the icon name, i.e. 0 for Icon0.
            glg_object.SetDResource( "Index", (double) i );
         }
      }

      int size = icon_array.GetSize();
      if( size == 0 )
        SetError( "Can't find any icons of this type." );
      else
        System.out.println( "Scanned " + size + " " + icon_name +  " icons" );
   }

   ////////////////////////////////////////////////////////////////////////
   // Adds custom data to the graphical object
   ////////////////////////////////////////////////////////////////////////
   void AddCustomData( GlgObject glg_object, Object data )
   {
      // Add back-pointer from graphics to the link's data struct,
      // keeping the data already attached (if any).
        //
      GlgObject custom_data = glg_object.GetResourceObject( "CustomData" );
      if( custom_data == null )
      {
         // No custom data attached: create an extra group and attach it 
           // to object as custom data.
         custom_data = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
         glg_object.SetResourceObject( "CustomData", custom_data );
      }

      // To allow using non-glg objects, use a group with element type
      // NATIVE_OBJECT as a holder. The first element of the group will keep
        // the custom data pointer (pointer to the Link or Node structure).
      GlgObject holder_group = new GlgDynArray( GlgObject.NATIVE_OBJECT, 0, 0 );
      holder_group.SetSResource( "Name", "PtrHolder" );

      holder_group.AddObjectToBottom( data );

      // Add it to custom data.
      custom_data.AddObjectToBottom( holder_group );
   }

   ////////////////////////////////////////////////////////////////////////
   // Get custom data attached to the graphical object
   ////////////////////////////////////////////////////////////////////////
   Object GetData( GlgObject glg_object )
   {
      GlgObject holder_group = glg_object.GetResourceObject( "PtrHolder" );
      return holder_group.GetElement( 0 );
   }

   ////////////////////////////////////////////////////////////////////////
   void SetRadioBox( String button_name )
   {
      // Always highlight the new button: the toggle would unhighlight if 
        // clicked on twice.
      GlgObject button = Viewport.GetResourceObject( button_name );
      if( button != null )
        button.SetDResource( "OnState", 1.0 );

      // Unhighlight the previous button.
      if( LastButton != null && !LastButton.equals( button_name ) )
      {
         button = Viewport.GetResourceObject( LastButton );
         if( button != null )
           button.SetDResource( "OnState", 0.0 );
      }
         
      LastButton = button_name;   // Store the last button.
   }

   ////////////////////////////////////////////////////////////////////////
   // Deselects the button.
   ////////////////////////////////////////////////////////////////////////
   void DeselectButton( String button_name )
   {
      GlgObject button = Viewport.GetResourceObject( button_name );
      button.SetDResource( "OnState", 0.0 );
   }

   ////////////////////////////////////////////////////////////////////////
   void GetPosition( GlgObject glg_object, GlgPoint coord ) 
   {
      int type = glg_object.GetDResource( "Type" ).intValue();
      if( type == GlgObject.REFERENCE )
      {
         // Reference: can use its point to position it.
         coord.CopyFrom( glg_object.GetGResource( "Point" ) );
      }
      else
      {
         // Arbitrary object: convert the box's center to the world coords.

         // Get object center in screen coords.
         GlgCube box = glg_object.GetBox();

         GlgPoint center = new GlgPoint( ( box.p1.x + box.p2.x ) / 2.0,
                                        ( box.p1.y + box.p2.y ) / 2.0,
                                        ( box.p1.z + box.p2.z ) / 2.0 );

         DrawingArea.ScreenToWorld( true, center, coord );
      }
   }

   ////////////////////////////////////////////////////////////////////////
   // Fills Properties dialog with the selected object data.
   ////////////////////////////////////////////////////////////////////////   
   void FillData()
   {
      String 
        label,
        object_data,
        datasource = null;

      switch( SelectedObjectType )
      {
       default:
         label = "NO_OBJECT";
         object_data = "";
         datasource = "";
         break;
      
       case NODE:	 
       case LINK:	 
         label = GetObjectLabel( SelectedObject );
         object_data = GetObjectData( SelectedObject );
         if( ProcessDiagram )
         {
            datasource = GetObjectDataSource( SelectedObject );

            // Substitute an empty string instead of null for display.
            if( datasource == null )
              datasource = "";
         }
         break;
      }   

      Viewport.SetSResource( "Dialog/DialogName/TextString", label );
      Viewport.SetSResource( "Dialog/DialogData/TextString", object_data );

      // For process diagram also set the datasource field.
      if( ProcessDiagram )
        Viewport.SetSResource( "Dialog/DialogDataSource/TextString", 
                              datasource );
   }

   ////////////////////////////////////////////////////////////////////////
   // Stores data from the dialog fields in the object.
   ////////////////////////////////////////////////////////////////////////
   boolean ApplyDialogData()
   {
      String
        label,
        object_data;

      // Store data from the dialog fields in the object.
      label = Viewport.GetSResource( "Dialog/DialogName/TextString" );
      object_data = Viewport.GetSResource( "Dialog/DialogData/TextString" );

      switch( SelectedObjectType )
      {
       case NODE:
       case LINK:
         break;
       default: return true;
      }   

      // Store data
      SetObjectLabel( SelectedObject, label );
      SetObjectData( SelectedObject, object_data );

      if( ProcessDiagram )
      {
         String datasource = 
           Viewport.GetSResource( "Dialog/DialogDataSource/TextString" );
         SetObjectDataSource( SelectedObject, datasource );
      }

      Update();
      return true;
   }

   ////////////////////////////////////////////////////////////////////////
   String GetObjectLabel( GlgObject glg_object )
   {
      Object data = GetData( glg_object );
      if( data instanceof GlgNodeData )
        return ((GlgNodeData)data).object_label;
      else
        return ((GlgLinkData)data).object_label;
   }

   ////////////////////////////////////////////////////////////////////////
   void SetObjectLabel( GlgObject glg_object, String label )
   {
      // Display label in the node or link object if it has a label.
      if( glg_object.HasResourceObject( "Label" ) )
        glg_object.SetSResource( "Label/String", label );

      Object data = GetData( glg_object );
      if( data instanceof GlgNodeData )
        ((GlgNodeData)data).object_label = label;
      else if( data instanceof GlgLinkData )
        ((GlgLinkData)data).object_label = label;
   }
   
   ////////////////////////////////////////////////////////////////////////
   String GetObjectData( GlgObject glg_object )
   {
      Object data = GetData( glg_object );
      if( data instanceof GlgNodeData )
        return ((GlgNodeData)data).object_data;
      else
        return ((GlgLinkData)data).object_data;
   }

   ////////////////////////////////////////////////////////////////////////
   void SetObjectData( GlgObject glg_object, String object_data )
   {
      Object data = GetData( glg_object );
      if( data instanceof GlgNodeData )
        ((GlgNodeData)data).object_data = object_data;
      else
        ((GlgLinkData)data).object_data = object_data;
   }
   
   ////////////////////////////////////////////////////////////////////////
   String GetObjectDataSource( GlgObject glg_object )
   {
      Object data = GetData( glg_object );
      if( data instanceof GlgNodeData )
        return ((GlgNodeData)data).datasource;
      else
        return ((GlgLinkData)data).datasource;
   }

   ////////////////////////////////////////////////////////////////////////
   void SetObjectDataSource( GlgObject glg_object, String datasource )
   {
      if( datasource != null && datasource.length() == 0 )
        datasource = null;  // Substitute null for empty datasource strings.
      
      Object data = GetData( glg_object );
      if( data instanceof GlgNodeData )
      {
         if( glg_object.HasResourceObject( "Value" ) )
           glg_object.SetSResource( "Value/Tag", datasource );

         ((GlgNodeData)data).datasource = datasource;
      }
      else
        ((GlgLinkData)data).datasource = datasource;
   }
   
   ////////////////////////////////////////////////////////////////////////
   boolean NodeConnected( GlgObject node )
   {
      for( int i=0; i< CurrentDiagram.getLinkList().size(); ++ i )
      {
         GlgLinkData link_data =
           (GlgLinkData) CurrentDiagram.getLinkList().get( i );

         GlgNodeData start_node = link_data.getStartNode();
         GlgNodeData end_node = link_data.getEndNode();
        if( start_node != null && start_node.graphics == node ||
           end_node != null && end_node.graphics == node )
          return true;      
      }
      return false;
   }

   ////////////////////////////////////////////////////////////////////////
   int ButtonToToken( String button_name )
   {

      for( int i=0; ButtonTokenTable[i].name != null; ++i )
        if( button_name.equals( ButtonTokenTable[i].name ) )
          return ButtonTokenTable[i].token;

      return IH_UNDEFINED_TOKEN;   /* 0 */
   }

   ////////////////////////////////////////////////////////////////////////
   int GetButton( AWTEvent event )
   {
      if( ! ( event instanceof InputEvent ) )
        return 0;
      
      InputEvent input_event = (InputEvent) event;
      int modifiers = input_event.getModifiers();
      
      if( ( modifiers & InputEvent.BUTTON1_MASK ) != 0 )
        return 1;
      else if( ( modifiers & InputEvent.BUTTON2_MASK ) != 0 )
        return 2;
      else if( ( modifiers & InputEvent.BUTTON3_MASK ) != 0 )
        return 3;
      else
        return 0;
   }

   //////////////////////////////////////////////////////////////////////////
   // ActionListener method to use the bean as update timer's ActionListener.
   //////////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateProcessDiagram();
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates all tags defined in the drawing for a process diagram.
   //////////////////////////////////////////////////////////////////////////
   void UpdateProcessDiagram()
   {
      if( timer == null || !ProcessDiagram )
        return;

      GlgObject drawing = DrawingArea;

      // Since new nodes may be added or removed in the process of the diagram
      // editing, get the current list of tags every time. In an application 
      // that just displays the diagram without editing, the tag list may be
      // obtained just once (initially) and used to subscribe to data.
        // Query only unique tags.
      GlgObject tag_list = drawing.CreateTagList( true );
      if( tag_list != null )
      {
         int size = tag_list.GetSize();
         for( int i=0; i<size; ++i )
         {
            GlgObject data_object = (GlgObject) tag_list.GetElement( i );
            String tag_name = data_object.GetSResource( "Tag" );

            double new_value = GetTagValue( tag_name, data_object );
            drawing.SetDTag( tag_name, new_value, true );
         }

         Update( drawing );
      }

      timer.start();   // Restart the update timer
   }

   //////////////////////////////////////////////////////////////////////////
   // Get new value based on a tag name. In a real application, the value
   // is obtained from a process database. PLC or another live datasource.
   // In the demo, use random data.
   //////////////////////////////////////////////////////////////////////////
   double GetTagValue( String tag_name, GlgObject data_object )
   {
      // Get the current value
      double value = data_object.GetDResource( null ).doubleValue();
   
      // Increase it.
      double increment = GlgObject.Rand( 0.0, 0.1 );

      double direction;
      if( value == 0.0 )
        direction = 1.0;
      else if( value == 1.0 )
        direction = -1.0;
      else
        direction = GlgObject.Rand( -1.0, 1.0 );

      if( direction > 0.0 )
        value += increment;
      else
        value -= increment;
      
      if( value > 1.0 )
        value = 1.0;
      else if( value < 0.0 )
        value = 0.0;
      
      return value;
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked by the browser to stop the applet
   //////////////////////////////////////////////////////////////////////////
   public void stop()
   {
      StopUpdates();
      IsReady = false;
      super.stop();
   }

   //////////////////////////////////////////////////////////////////////////
   void WriteLine( String line )
   {
      System.out.println( line );
   }
}

class GlgNodeData
{   
   int node_type;
   GlgDiagramPoint position;
   String object_label;
   String object_data;
   String datasource;
   transient GlgObject graphics;

   GlgNodeData()
   {
      object_label = "";
      object_data = "";
      datasource = "";
      position = new GlgDiagramPoint();
   }
}

@SuppressWarnings("rawtypes")
class GlgLinkData
{   
   int link_type;
   int link_direction;
   GlgDiagramPoint link_color;
   transient GlgObject graphics;
   GlgNodeData start_node;
   GlgNodeData end_node;
   String start_point_name;
   String end_point_name;
   String object_label;
   String object_data;
   ArrayList<GlgDiagramPoint> point_array;
   boolean first_move;
   String datasource;

   GlgLinkData()
   {
      object_label = "A1";
      object_data = "";
      datasource = "";
      point_array = new ArrayList<GlgDiagramPoint>();
   }

   GlgNodeData getStartNode()
   {
      return start_node;
   }

   GlgNodeData getEndNode()
   {
      return end_node;
   }

   void setStartNode( GlgNodeData node )
   {
      start_node = node;
   }

   void setEndNode( GlgNodeData node )
   {
      end_node = node;
   }
}

@SuppressWarnings("rawtypes")
class GlgDiagramData
{
   // Using <Object> type to be able to handle a list generically in the code 
   // regardless of its element type.
   //
   ArrayList<Object> node_list;
   ArrayList<Object> link_list;

   GlgDiagramData()
   {
      node_list = new ArrayList<Object>();
      link_list = new ArrayList<Object>();
   }

   ArrayList<Object> getNodeList()
   {
      return node_list;
   }

   ArrayList<Object> getLinkList()
   {
      return link_list;
   }
}
class GlgDiagramPoint extends GlgPoint
{
   // Allowes for custom serialization extensions for saving and loading

   GlgDiagramPoint()
   {
   }

   GlgDiagramPoint( GlgPoint point )
   {
      if( point != null )
      {
         x = point.x;
         y = point.y;
         z = point.z;
      }      
   }
}

// Custom handler, place application-specific code into these callbacks.
class CustomHandler
{
   static void AddObjectCB( GlgDiagram diagram, GlgObject icon, 
                                      Object data, boolean is_node )
   {
   }

   static void SelectObjectCB( GlgDiagram diagram, GlgObject icon, 
                                         Object data, boolean is_node )
   {
   }

   static void CutObjectCB( GlgDiagram diagram, GlgObject icon, 
                                      Object data, boolean is_node )
   {
   }

   static void PasteObjectCB( GlgDiagram diagram, GlgObject icon, 
                                        Object data, boolean is_node )
   {
   }
}
