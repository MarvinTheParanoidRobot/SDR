//////////////////////////////////////////////////////////////////////////////
// GLG Diagram Demo: an example of using the GLG Extended API.
//
// The demo is written in pure HTML5 and JavaScript. The source code of the
// demo uses the GLG Toolkit JavaScript Library supplied by the included
// Glg*.js and GlgToolkit*.js files. The GLG library loads a GLG drawing
// and renders it on a web page, providing an API to handle user interaction
// with graphical objects in the drawing.
//
// The source code in the diagram_demo.js uses the GLG Extended API to
// implement the diagram editor functionality. The same source code is used
// by both the DIAGRAM DEMO and the PROCESS DIAGRAM demo. The type of the
// diagram is selected by the ProcessDiagram variable that is set in HTML
// outside and defines the type of the demo: Diagram if false, Process Diagram
// if true.
//
// The drawings are created using the GLG Graphics Builder, an interactive
// editor that allows to create grahical objects and define their dynamic
// behavior without any programming. The icons used in the diagram are
// dynamic GLG objects created with the Graphics Builder.
//
// Except for the changes to comply with the JavaScript syntax, this source
// is identical to the source code of the corresponding C/C++/C# and Java
// desktop versions of the demo.
//////////////////////////////////////////////////////////////////////////////

// Get a handle to the GLG Toolkit library.
var GLG = new GlgToolkit();

// Debugging aid: uncomment the next line to throw an exception on a GLG error.
//GLG.ThrowExceptionOnError( true, true, true );

// Set initial size of the drawing.
SetDrawingSize( false );

// Increase canvas resoultion for mobile devices.
var CoordScale = SetCanvasResolution();

/* Loads all drawings used by the program and invokes the StartDiagram function
   when done.
*/
LoadAssets( StartDiagram );

var Viewport = null;         /* Main drawing */
var TemplateDrawing = null;  /* Icon and dialog template drawing */

/* Is used to update the Process Diagram. */
var UPDATE_INTERVAL = 1000;  /* Update once per second. */
var timer = null;

//////////////////////////////////////////////////////////////////////////////
function StartDiagram()
{
   if( Viewport == null || TemplateDrawing == null )
   {
      window.alert( "Can't load drawings, check console message for details." );
      return;
   }

   // Define the element in the HTML page to display the drawing in.
   Viewport.SetParentElement( "glg_area" );
   
   // Disable viewport border to use the border of the glg_area.
   Viewport.SetDResource( "LineWidth", 0 );

   InitBeforeHierarchySetup();

   Viewport.SetupHierarchy();

   InitAfterHierarchySetup();
   
   Viewport.Update();    // Draw

   if( ProcessDiagram )
     // Start periodic updates.
     timer = setTimeout( UpdateProcessDiagram, UPDATE_INTERVAL );
}

//////////////////////////////////////////////////////////////////////////////
// Control variables and constants
//////////////////////////////////////////////////////////////////////////////

const SELECT_BUTTON_NAME = "IconButton0";

// Selection sensitivity in pixels
const SELECTION_RESOLUTION = 5;
const POINT_SELECTION_RESOLUTION = 2;

/* Number of palette buttons to skip: the first button with the "select" icon 
   is already in the palette.
*/
const PALETTE_START_INDEX = 1;

// Default scale factor for icon buttons.
const DEFAULT_ICON_ZOOM_FACTOR = 10.0;

// Percentage of the button area to use for the icon.
const ICON_FIT_FACTOR = 0.6;

// Scale factor when placed in the drawing.
const IconScale = 1.0;

// Object types
const
  NO_OBJ = 0,
  NODE = 1,
  LINK = 2;

// IH tokens
const
  IH_UNDEFINED_TOKEN = 0,
  IH_ICON_SELECTED = 1,
  IH_SAVE = 2,
  IH_INSERT = 3,
  IH_PRINT = 4,
  IH_CUT = 5,
  IH_PASTE = 6,
  IH_ZOOM_IN = 7,
  IH_ZOOM_OUT = 8,
  IH_ZOOM_TO = 9,
  IH_ZOOM_RESET = 10,
  IH_PROPERTIES = 11,
  IH_CREATION_MODE = 12,
  IH_DIALOG_APPLY = 13,
  IH_DIALOG_CLOSE = 14,
  IH_DIALOG_CANCEL = 15,
  IH_DIALOG_CONFIRM_DISCARD = 16,
  IH_DATASOURCE_SELECT = 17,
  IH_DATASOURCE_SELECTED = 18,
  IH_DATASOURCE_CLOSE = 19,
  IH_DATASOURCE_APPLY = 20,
  IH_DATASOURCE_LIST_SELECTION = 21,
  IH_MOUSE_PRESSED = 22,
  IH_MOUSE_RELEASED = 23,
  IH_MOUSE_MOVED = 24,
  IH_MOUSE_BUTTON3 = 25,
  IH_FINISH_LINK = 26,
  IH_TEXT_INPUT_CHANGED = 27,
  IH_OK = 28,
  IH_CANCEL = 29,
  IH_ESC = 30;

function ButtonToken( name, token )
{
   this.name = name;
   this.token = token;
}

var ButtonTokenTable =
[
   new ButtonToken( "Save",             IH_SAVE ),
   new ButtonToken( "Insert",           IH_INSERT ),
   new ButtonToken( "Print",            IH_PRINT ),
   new ButtonToken( "Cut",              IH_CUT ),
   new ButtonToken( "Paste",            IH_PASTE ),
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
];

var TraceMouseMove = false;
var TraceMouseRelease = false;

/* If set to true, multple instances of the selected item can be added to the 
   drawing by clicking in the drawing area. 
*/
var StickyCreateMode = false;

var DrawingArea;                    /* GLG object */
var PaletteTemplate = null;         /* GLG object */
var ButtonTemplate;                 /* GLG object */
var SelectedObject = null;          /* GLG object */
var StoredColor = null;             /* GLG object */
// Stores color during selection.
var LastColor = null;               /* GLG object */
var CutBuffer = null;               /* GLG object */
var PointMarker = null;             /* GLG object */
var AttachmentMarker = null;        /* GLG object */
var AttachmentNode = null;          /* GLG object */
var AttachmentArray = null;         /* GLG group */
var NodeIconArray;                  /* GLG group */
var NodeObjectArray;                /* GLG group */
var LinkIconArray;                  /* GLG group */
var LinkObjectArray;                /* GLG group */
/* If set to true, icons are automatically fit to fill the button.
   If set to false, the default zoom factor will be used. */
var FitIcons = false;               /* bool, is set to true for ProcessDiagram */
var AllowUnconnectedLinks = true;   /* bool */
var DialogDataChanged = false;      /* bool */
var NumColumns;                     /* int */
var SelectedObjectType = NO_OBJ;    /* int */
var CutBufferType = NO_OBJ;         /* int */
var LastButton = null;              /* String */
var CurrentDiagram = new GlgDiagramData();         /* GlgDiagramData */
var SavedDiagramData = null;                       /* JSON string */
var cursor_pos  = GLG.CreateGlgPoint( 0, 0, 0 );   /* GlgPoint */
var world_coord = GLG.CreateGlgPoint( 0, 0, 0 );   /* GlgPoint */
var select_rect = GLG.CreateGlgCube( null, null ); /* GlgCube */
// Used by the process diagram.
var DataSourceCounter = 0;
var NumDatasources = 20;
var TouchStart = false;
var TouchDevice = -1;

//////////////////////////////////////////////////////////////////////////////
// Loads drawings and other assets required by the application and
// invokes the specified callback when done.
//
// A simple application, such as controls_demo.js, may need just one drawing,
// which would be loaded directly without a need for the LoadAssets()
// function.
//////////////////////////////////////////////////////////////////////////////
function LoadAssets( callback )
{
   /* Load drawings used in the demo. For each loaded asset, the AssetLoaded
      callback is invoked with the supplied data.
   */
   GLG.LoadWidgetFromURL( ProcessDiagram ? "process_diagram.g" : "diagram.g",
                          null, AssetLoaded,
                          { name: "main_drawing", callback: callback } );
   
   GLG.LoadObjectFromURL( ( ProcessDiagram ?
                            "process_template.g" : "diagram_template.g" ),
                          null, AssetLoaded,
                          { name: "template_drawing", callback: callback } );
   
   /* HTML5 doesn't provide a scrollbar input element (only a range input 
      html element is available). This application needs to load GLG 
      scrollbars used for integrated scrolling. 
   */
   GLG.LoadWidgetFromURL( "scrollbar_h.g", null, AssetLoaded,
                          { name: "scrollbar_h", callback: callback } );
   GLG.LoadWidgetFromURL( "scrollbar_v.g", null, AssetLoaded,
                          { name: "scrollbar_v", callback: callback } );
}

//////////////////////////////////////////////////////////////////////////////
function AssetLoaded( glg_object, data, path )
{
   if( data.name == "main_drawing" )
   {
       Viewport = glg_object;
   }
   else if( data.name == "template_drawing" )
   {
      TemplateDrawing = glg_object;
   }
   else if( data.name == "scrollbar_h" )
   {
      if( glg_object != null )
        glg_object.SetResourceObject( "$config/GlgHScrollbar", glg_object );
   }
   else if( data.name == "scrollbar_v" )
   {
      if( glg_object != null )
        glg_object.SetResourceObject( "$config/GlgVScrollbar", glg_object );
   }
   else
     console.error( "Unexpected asset name" );

   /* Define an internal variable to keep the number of loaded assets. */
   if( AssetLoaded.num_loaded == undefined )
     AssetLoaded.num_loaded = 1;
   else
     ++AssetLoaded.num_loaded;

   // Invoke the callback after all assets have been loaded.
   if( AssetLoaded.num_loaded == 4 )
     data.callback();
}

//////////////////////////////////////////////////////////////////////////////
// Changes drawing size while maintaining width/height aspect ratio.
//////////////////////////////////////////////////////////////////////////////
function SetDrawingSize( next_size )
{
   const ASPECT_RATIO = 900 / 700;

   // Settings for desktop displays.
   const MIN_WIDTH = 600;
   const MAX_WIDTH = 1000;
   const SCROLLBAR_WIDTH = 15;
   
   if( SetDrawingSize.size_index == undefined )   // first time
   {
      SetDrawingSize.size_index = 0;

      SetDrawingSize.small_sizes       = [ 1, 1.5,  2.,   2.5 ];
      SetDrawingSize.medium_sizes      = [ 1, 0.75, 1.25, 1.5 ];
      SetDrawingSize.large_sizes       = [ 1, 0.6,  1.25, 1.5 ];
      SetDrawingSize.num_sizes = SetDrawingSize.small_sizes.length;
      SetDrawingSize.is_mobile = ( screen.width <= 760 );

      window.addEventListener( "resize", ()=>{ SetDrawingSize( false ) } );
   }
   else if( next_size )
   {
      ++SetDrawingSize.size_index;
      SetDrawingSize.size_index %= SetDrawingSize.num_sizes;
   }

   var drawing_area = document.getElementById( "glg_area" );
   if( SetDrawingSize.is_mobile )
   {
      /* Mobile devices use constant device-width, adjust only the height 
         of the drawing to keep the aspect ratio.
      */
      drawing_area.style.height =
        "" + Math.trunc( drawing_area.clientWidth / ASPECT_RATIO ) + "px";
   }
   else   /* Desktop */
   {
      var span = document.body.clientWidth; 
      if( !SetDrawingSize.is_mobile )
        span -= SCROLLBAR_WIDTH;

      var start_width;
      if( span < MIN_WIDTH )
        start_width = MIN_WIDTH;
      else if( span > MAX_WIDTH )
        start_width = MAX_WIDTH;
      else
        start_width = span;

      var size_array;
      if( span < 600 )
        size_array = SetDrawingSize.small_sizes;
      else if( span < 800 )
        size_array = SetDrawingSize.medium_sizes;
      else
        size_array = SetDrawingSize.large_sizes;

      var size_coeff = size_array[ SetDrawingSize.size_index ];
      var width = Math.trunc( Math.max( start_width * size_coeff, MIN_WIDTH ) );
   
      drawing_area.style.width = "" + width + "px";
      drawing_area.style.height = "" + Math.trunc( width / ASPECT_RATIO ) + "px";
   }
}

//////////////////////////////////////////////////////////////////////////////
// Increases canvas resolution for mobile devices with HiDPI displays.
// Returns chosen coordinate scale factor.
//////////////////////////////////////////////////////////////////////////////
function SetCanvasResolution()
{
   // Set canvas resolution only for mobile devices with devicePixelRatio != 1.
   if( window.devicePixelRatio == 1. || !SetDrawingSize.is_mobile )
     return 1.0;   // Use coord scale = 1.0 for desktop.
   
   /* The first parameter defines canvas coordinate scaling with values 
      between 1 and devicePixelRatio. Values greater than 1 increase 
      canvas resolution and result in sharper rendering. The value of 
      devicePixelRatio may be used for very crisp rendering with very thin lines.

      Canvas scale > 1 makes text smaller, and the second parameter defines
      the text scaling factor used to increase text size.

      The third parameter defines the scaling factor that is used to
      scale down text in native widgets (such as native buttons, toggles, etc.)
      to match the scale of the drawing.
   */
   var coord_scale = 2.0;
   GLG.SetCanvasScale( coord_scale, 1.5, 0.75 );
   
   // Mobile devices use fixed device-width: disable Change Drawing Size button.
   var change_size_button = document.getElementById( "change_size" );
   if( change_size_button != null )
     change_size_button.parentNode.removeChild( change_size_button );

   return coord_scale;      // Chosen coord scale for mobile devices.
}

//////////////////////////////////////////////////////////////////////////////
function InitBeforeHierarchySetup()
{
   /* Add Input callback used to handle user interaction. */
   Viewport.AddListener( GLG.GlgCallbackType.INPUT_CB, InputCallback );

   /* Add Trace callback used to handle mouse operations: selection, dragging,
      connection point highlight.
   */
   Viewport.AddListener( GLG.GlgCallbackType.TRACE_CB, TraceCallback );

   GLG.IHInit();

   FitIcons = ( ProcessDiagram ? true : false );

   // Fill out the palette and change the dialog type before hierarchy setup.

   DrawingArea = Viewport.GetResourceObject( "DrawingArea" );
   if( DrawingArea == null )
   {
      window.alert( "Can't find DrawingArea viewport." );
      return;
   }

   PaletteTemplate = TemplateDrawing.GetResourceObject( "PaletteTemplate" );
   if( PaletteTemplate == null )
   {
      window.alert( "Can't find PaletteTemplate viewport." );
      return;
   }

   AddDialog( TemplateDrawing, "Dialog", "Object Properties", -300, -500 );
   AddDialog( TemplateDrawing, "OKDialog", null, 0, 0 );
   if( ProcessDiagram )
     AddDialog( TemplateDrawing, "DataSourceDialog", null, 600, 200 );

   // Disable the exit button for the browser version of the demo.
   Viewport.SetDResource( "Exit/Visibility", 0 );
   
   SetupDiagramDrawing();
}

//////////////////////////////////////////////////////////////////////////////
function InitAfterHierarchySetup()
{
   // Position node icons inside the palette buttons.
   SetupObjectPalette( "IconButton", PALETTE_START_INDEX );

   // Install and start the top level interface handler.
   GLG.IHInstallAsInterface( MainIH );
   GLG.IHStart();
}

//////////////////////////////////////////////////////////////////////////////
function SetupDiagramDrawing()
{
   SetPrompt( "" );

   /* Create a color object to store original node color during node 
      selection. */
   StoredColor = Viewport.GetResourceObject( "FillColor" ).CopyObject();

   // Set grid color (configuration parameter) to grey.
   Viewport.SetGResource( "$config/GlgGridPolygon/EdgeColor",
                          0.632441, 0.632441, 0.632441 );

   // Create a separate group to hold objects.
   var group =
     GLG.CreateObject( GLG.GlgObjectType.ARRAY,
                       GLG.GlgContainerType.GLG_OBJECT, 0, 0, null, null );
   group.SetSResource( "Name", "ObjectGroup" );
   DrawingArea.AddObjectToBottom( group );

   // Create groups to hold nodes and links.
   NodeIconArray =
     GLG.CreateObject( GLG.GlgObjectType.ARRAY,
                       GLG.GlgContainerType.GLG_OBJECT, 0, 0, null );
   NodeObjectArray =
     GLG.CreateObject( GLG.GlgObjectType.ARRAY,
                       GLG.GlgContainerType.GLG_OBJECT, 0, 0, null );
   LinkIconArray =
     GLG.CreateObject( GLG.GlgObjectType.ARRAY,
                       GLG.GlgContainerType.GLG_OBJECT, 0, 0, null );
   LinkObjectArray =
     GLG.CreateObject( GLG.GlgObjectType.ARRAY,
                       GLG.GlgContainerType.GLG_OBJECT, 0, 0, null );
   
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

//////////////////////////////////////////////////////////////////////////////
// Sets create mode based on the state of the CreateMode button.
//////////////////////////////////////////////////////////////////////////////
function SetCreateMode( set_button )
{
   if( !set_button )
   {
      var create_mode =
        Math.trunc( Viewport.GetDResource( "CreateMode/OnState" ) );
      StickyCreateMode = ( create_mode != 0 );
   }
   else   /* Restore button state from StickyCreateMode. */
     Viewport.SetDResource( "CreateMode/OnState", StickyCreateMode ? 1.0 : 0.0 );
}

//////////////////////////////////////////////////////////////////////////////
function AddDialog( drawing, dialog_name, title, x_offset, y_offset )
{
   var dialog = drawing.GetResourceObject( dialog_name );
   if( dialog == null )
   {
      window.alert( "Can't find dialog: " + dialog_name );
      return;
   }

   /* Make the dialog a top-level window and make it invisible on startup. */
   dialog.SetDResource( "ShellType", GLG.GlgShellType.DIALOG_SHELL );
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
      dialog.SetGResource( "AnchorPointCenter", x_offset, y_offset, 0.0 );
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
      dialog.SetDResource( "Screen/WidthHint", width * CoordScale );
      dialog.SetDResource( "Screen/HeightHint", height * CoordScale );
      dialog.SetDResource( "Screen/XHint", x_offset * CoordScale );
      dialog.SetDResource( "Screen/YHint", y_offset * CoordScale );
   }
   */

   AdjustForMobileDevices( dialog, dialog_name );
   
   Viewport.AddObjectToBottom( dialog );
}

////////////////////////////////////////////////////////////////////////
function AdjustForMobileDevices( dialog, dialog_name )
{
   if( CoordScale == 1.0 )
     return;   /* Desktop version. */

   /* Adjust dialog size on mobile devices with canvas scaling. */
   if( dialog_name == "Dialog" )
   {
      dialog.SetDResource( "Width", 600 );
      dialog.SetDResource( "Height", 400 );
   }
   else if( dialog_name == "OKDialog" )
   {
      dialog.SetDResource( "Width", 500 );
      dialog.SetDResource( "Height", 180 );
   }
   else if( dialog_name == "DataSourceDialog" )
   {
      dialog.SetDResource( "Width", 400 );
      dialog.SetDResource( "Height", 550 );

      /* Make sure the dialog doesn't extend outside of the device-width area:
         that would cause the browser to scale down the web page.
      */
      dialog.SetGResource( "AnchorPointCenter", 200, 0, 0 );
   }
}

////////////////////////////////////////////////////////////////////////
function InputCallback( viewport, message_obj )
{
   var format      = message_obj.GetSResource( "Format" );
   var origin      = message_obj.GetSResource( "Origin" );
   var full_origin = message_obj.GetSResource( "FullOrigin" );
   var action      = message_obj.GetSResource( "Action" );
   var subaction   = message_obj.GetSResource( "SubAction" );
   
   var token = IH_UNDEFINED_TOKEN;

   // Handle the Dialog window closing.
   if( format == "Window" )
   {
      if( action == "DeleteWindow" )
        if( origin == "Dialog" )
          token = IH_DIALOG_CLOSE;
        else if( origin == "DataSourceDialog" )
          token = IH_DATASOURCE_CLOSE;
        else if( origin == "OKDialog" )
          token = IH_ESC;
        else
          token = IH_EXIT;
   }
   else if( format == "Button" )
   {
      if( action != "Activate" && action != "ValueChanged" )
        return;

      else if( origin.startsWith( "IconButton" ) )
      {
         var button = viewport.GetResourceObject( full_origin );
         var icon = button.GetResourceObject( "Icon" );
         if( icon == null )
           GLG.Error( GLG.GlgErrorType.USER_ERROR, "Can't find icon." );
         else
         {
            GLG.IHSetOParameter( GLG.IH_GLOBAL, "$selected_icon", icon );
            GLG.IHSetSParameter( GLG.IH_GLOBAL, "$selected_button",
                                 full_origin );
            token = IH_ICON_SELECTED;
         }
      }
      else
        token = ButtonToToken( origin );
   }
   else if( format == "Text" )
   {
      if( action == "ValueChanged" )
        token = IH_TEXT_INPUT_CHANGED;
   }
   else if( format == "List" )
   {
      if( action == "Select" && subaction == "DoubleClick" &&
          origin == "DSList" )
        token = IH_DATASOURCE_LIST_SELECTION;
   }

   if( token != IH_UNDEFINED_TOKEN )
     GLG.IHCallCurrIHWithToken( token );

   if( format != "Window" )
     Viewport.Update();
}

////////////////////////////////////////////////////////////////////////
// Handles mouse operations: selection, dragging, connection point
// highlight.
////////////////////////////////////////////////////////////////////////
function TraceCallback( viewport, trace_info )
{
   var use_coords = false;
   var token = IH_UNDEFINED_TOKEN;

   // Detect touch device if it hasn't been detected yet.
   if( TouchDevice == -1 )
     if( trace_info.event_type == GLG.GlgEventType.TOUCH_START )
     {
        TouchDevice = 1;

        // Increase pick resolution for touch devices.
        Viewport.SetDResource( "$config/GlgPickResolution", 50 );
     }
     else if( trace_info.event_type == GLG.GlgEventType.MOUSE_PRESSED )
       TouchDevice = 0;
   
   TouchStart = false;
   var event_type = trace_info.event_type;
   switch( event_type )
   {
      /* Handle touch events to enable dragging on mobile devices.
         The SetTouchMode function is used to activate the touch mode on the
         touchStart event. It will be invoked later only if needed 
         (only if an object is selected), not to interfere with page scrolling
         and zooming. The touch mode stays active until the touchEnd or
         touchCancel event.
         If the touch mode is not activated, the browser will generate 
         simulated mouse move events which will enable mouse clicks but not the 
         dragging.
         The trace_info.button is set to 1 for all touch events for better
         compatibility with the mouse events.
      */
    case GLG.GlgEventType.TOUCH_START:
      TouchStart = true;
    case GLG.GlgEventType.MOUSE_PRESSED:
      switch( trace_info.button )
      {
       case 1: token = IH_MOUSE_PRESSED; break;
       case 3: token = IH_MOUSE_BUTTON3; break;
       default: return;   /* Report only buttons 1 and 3 */
      }
      use_coords = true;
      break;
      
    case GLG.GlgEventType.TOUCH_END:
    case GLG.GlgEventType.TOUCH_CANCEL:
      if( !GLG.GetTouchMode() )
        return;
    case GLG.GlgEventType.MOUSE_RELEASED:
      if( trace_info.button != 1 )
        return;  // Trace only the left button releases.
      token = IH_MOUSE_RELEASED;
      use_coords = true;
      break;
      
    case GLG.GlgEventType.TOUCH_MOVED:
      if( !GLG.GetTouchMode() )
        return;
    case GLG.GlgEventType.MOUSE_MOVED:
      token = IH_MOUSE_MOVED;
      use_coords = true;
      break;

    case GLG.GlgEventType.KEY_DOWN:
      if( trace_info.event.keyCode == 27 )
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

   if( Math.trunc( DrawingArea.GetDResource( "ZoomToMode" ) ) != 0 )
     return;   // Don't handle mouse selection in ZoomTo mode.

   if( use_coords )
   {
      /* If nodes use viewports (buttons, gauges, etc.), need to convert 
         coordinates inside the selected viewport to the coordinates of the 
         drawing area.
      */
      if( !trace_info.viewport.Equals( DrawingArea ) && 
          !IsChildOf( DrawingArea, trace_info.viewport ) )
        return;   /* Mouse event outside of the drawing area. */

      cursor_pos.x = trace_info.mouse_x * CoordScale;
      cursor_pos.y = trace_info.mouse_y * CoordScale;
      cursor_pos.z = 0.0;

      /* COORD_MAPPING_ADJ is added to the cursor coordinates for 
         precise pixel mapping.
      */
      cursor_pos.x += GLG.COORD_MAPPING_ADJ;
      cursor_pos.y += GLG.COORD_MAPPING_ADJ;
      
      if( !trace_info.viewport.Equals( DrawingArea  ) )
        GLG.TranslatePointOrigin( trace_info.viewport, viewport, cursor_pos );

      GLG.IHSetOParameterFromGPoint( GLG.IH_GLOBAL, "$cursor_pos", cursor_pos );
   }

   // Pass token to the current IH.
   GLG.IHCallCurrIHWithToken( token );
}

////////////////////////////////////////////////////////////////////////
// Can be used only for drawable objects, and not for data objects that 
// can be constrained.
////////////////////////////////////////////////////////////////////////
function IsChildOf( grand, object )
{
   if( object == null )
     return false;
   
   if( object.Equals( grand ) )
     return true;
   
   return IsChildOf( grand, object.GetParent() );
}

var EditPropertiesHandler;

////////////////////////////////////////////////////////////////////////
// Top level interface handler. 
// Parameters:
//   ih - interface handler handle
//   call_event - event the handler is invoked with
////////////////////////////////////////////////////////////////////////
function MainIH( ih, call_event )
{
   /* A handler can be invoked with the following event types:
      GLG_HI_SETUP_EVENT - triggered when handler is started by IHStart 
      GLG_MESSAGE_EVENT  - trigerred when handler is called by 
                           IHCallCurrIHWithToken, IHCallCurrIH, etc.
      GLG_CLEANUP_EVENT  - trigerred when handler is uninstalled by 
                           IHUninstall
   */
   switch( GLG.IHGetType( call_event ) )
   {
    case GLG.GlgCallEventType.HI_SETUP_EVENT:
      break;
      
    case GLG.GlgCallEventType.MESSAGE_EVENT:
      // Retrieve the token from the event and handle known tokens as needed.
      var token = GLG.IHGetToken( call_event );
      switch( token )
      {
       case IH_SAVE:
         Save( CurrentDiagram );
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
         DrawingArea.SetZoom( null, 'i', 0.0 );
         Viewport.Update();
         break;
         
       case IH_ZOOM_OUT:
         DrawingArea.SetZoom( null, 'o', 0.0 );
         Viewport.Update();
         break;
               
       case IH_ZOOM_TO:
         DrawingArea.SetZoom( null, 't', 0.0 );
         Viewport.Update();
         break;
         
       case IH_ZOOM_RESET:
         DrawingArea.SetZoom( null, 'n', 0.0 );
         Viewport.Update();
         break;

       case IH_ICON_SELECTED:
         /* Retrieve selected icon parameters. 
            $selected_icon   - parameter of type GlgObject
            $selected_button - parameter of type S (string) 
            These parameters are global and are assigned in the input callback
            InputCallback.
         */

         var icon = GLG.IHGetOParameter( GLG.IH_GLOBAL, "$selected_icon" );
         var button_name =
           GLG.IHGetSParameter( GLG.IH_GLOBAL, "$selected_button" );
         if( icon == null || button_name == null )
         {
            SetError( "null icon or icon button name." );
            break;
         }
         
         /* Object to use in the drawing. In case of connectors, uses 
            only a part of the icon (the connector object) without the 
            end markers.
         */
         var object = icon.GetResourceObject( "Object" );
         if( object == null )
           object = icon;
         
         var icon_type = object.GetSResource( "IconType" );
         if( icon_type == null )
         {
            SetError( "Can't find icon type." );
            break;
         }
         
         if( icon_type == "Select" )
         {
            SetRadioBox( SELECT_BUTTON_NAME ); // Highlight Select button
            SetPrompt( "" );
         }

         /* For an icon type "Link" or "Node", install a corresponding 
            handler, set its parameters, and start the handler.

            "template" parameter supplies the GlgObject id of the selected
            icon to be added to the drawing area, either a link or a node.
            "button_name" parameter supplies the name of the selected icon 
            button.
            
            The parameters are passed to the handler function and can be 
            retrieved using GlgIHSet*Parameter.
         */
         else if( icon_type == "Link" )
         {
            GLG.IHInstallAsInterface( AddLinkIH );
            GLG.IHSetOParameter( GLG.IH_NEW, "template", object );
            GLG.IHSetSParameter( GLG.IH_NEW, "button_name", button_name );
            GLG.IHStart();
         }
         else if( icon_type == "Node" )
         {
            GLG.IHInstallAsInterface( AddNodeIH );
            GLG.IHSetOParameter( GLG.IH_NEW, "template", object );
            GLG.IHSetSParameter( GLG.IH_NEW, "button_name", button_name );
            GLG.IHStart();
         }
         Viewport.Update();
         break;
         
       case IH_CREATION_MODE:
         /* Set sticky creation mode from the button. */
         SetCreateMode( false );
         break;
         
       case IH_MOUSE_PRESSED:
         /* Selects the object and installs MoveObjectIH to drag the 
            object with the mouse.
            "$cursor_pos" is a global parameter assigned in TraceCallback
            when the object is moved or dragged.
         */
         SelectObjectWithMouse( GLG.IHGetOParameter( GLG.IH_GLOBAL,
                                                     "$cursor_pos" ) );
         /* All tokens that originate from the TraceCB require an explicit 
            update. For tokens originating from the InputCB, update is 
            done at the end of the InputCB.
         */            
         Viewport.Update();

         /* Set the touch mode on the touchStart event if some object is 
            selected. The touch mode supresses generating simulated mouse events
            and uses touchMove events for dragging.
         */
         if( TouchStart && SelectedObject != null )
           GLG.SetTouchMode();
         break;
         
       case IH_ESC:
       case IH_MOUSE_BUTTON3:
         break;    /* Allow: do nothing. */
         
       default: 
         /* Handle unrecognized tokens. In this demo, unrecognized tokens
            are passed to a special "pass-through" handler 
            EditPropertiesIH, which is used to handle the Properties 
            dialog. 
            
            Properties dialog is a floating dialog that can remain open, 
            and its content is changed to show properties of the selected 
            object. A "pass-through" handler is a special handler type 
            that handles floating dialogs.
         */

         /* Set a global flag indicating the current handler is invoked
            as a "pass-through" handler with a token passed from the
            previous handler.
         */
         GLG.IHSetBParameter( GLG.IH_GLOBAL, "fall_through_call", true );
         
         EditPropertiesHandler =
           GLG.CreateGlgIHHandlerInterface( EditPropertiesIH );

         /* Install EditPropertiesIH handler, start it and invoke it with 
            a given token.
         */ 
         GLG.IHPassToken( EditPropertiesHandler, token, false );
         
         /* Reset the flag */
         GLG.IHSetBParameter( GLG.IH_GLOBAL, "fall_through_call", false );

         if( !GLG.IHGetBParameter( GLG.IH_GLOBAL, "token_used" ) )
           SetError( "Invalid token." );
         break;
      }
      break;
      
    case GLG.GlgCallEventType.CLEANUP_EVENT:
      // Invoked when the handler is uninstalled via IHUninstall.
      SetError( "Main ih handler should never be uninstalled." );
      break;
   }
}

////////////////////////////////////////////////////////////////////////
// Handles object selection and prepares for moving the object with 
// the mouse.
////////////////////////////////////////////////////////////////////////
function SelectObjectWithMouse( cursor_pos_obj )
{
   var selection = GetObjectsAtCursor( cursor_pos_obj );

   var num_selected;
   if( selection != null && ( num_selected = selection.GetSize() ) != 0 )
   {
      // Some object were selected, process the selection.
      for( var i=0; i < num_selected; ++i )
      {
         var sel_object = selection.GetElement( i );
         
         /* Find if the object itself is a link or a node, of if it's a part
            of a node. If it's a part of a node, get the node object ID.
         */
         var selection_info = GetSelectedObject( sel_object );
         sel_object = selection_info.glg_object;
         var selection_type = selection_info.type;
         
         if( selection_type != NO_OBJ )
         {
            SelectGlgObject( sel_object, selection_type );
            
            CustomSelectObjectCB( Viewport, sel_object, GetData( sel_object ),
                                  selection_type == NODE );
            
            // Prepare for dragging the object with the mouse.
            GLG.IHInstallAsInterface( MoveObjectIH );
            
            // Store the start point.
            GLG.IHSetOParameter( GLG.IH_NEW, "start_point", cursor_pos_obj );
            
            GLG.IHStart();
            
            return;
         }
      }
   }
   
   SelectGlgObject( null, 0 );    // Unselect
}

////////////////////////////////////////////////////////////////////////
// Handler parameters:
//   start_point (G data obj)
////////////////////////////////////////////////////////////////////////
function MoveObjectIH( ih, call_event )
{
   switch( GLG.IHGetType( call_event ) )
   {
    case GLG.GlgCallEventType.HI_SETUP_EVENT:
      TraceMouseMove = true;
      TraceMouseRelease = true;
      break;
      
    case GLG.GlgCallEventType.MESSAGE_EVENT:
      var token = GLG.IHGetToken( call_event );
      switch( token )
      {
       case IH_MOUSE_MOVED:
         var data = GetData( SelectedObject );
         
         var start_point_obj = GLG.IHGetOParameter( ih, "start_point" );
         var cursor_pos_obj =
           GLG.IHGetOParameter( GLG.IH_GLOBAL, "$cursor_pos" );
         
         if( SelectedObjectType == NODE )
           MoveObject( SelectedObject, start_point_obj, cursor_pos_obj );
         else
           MoveLink( SelectedObject, start_point_obj, cursor_pos_obj );
         
         Viewport.Update();
         
         if( SelectedObjectType == NODE )
         {
            // Update the X and Y in the node's data struct.
            UpdateNodePosition( SelectedObject, data );
            
            /* Don't need to update the attached links' points, since 
               the stored positions of the first and last points are
               not used: they are constrained to nodes and positioned 
               by them. */
         }
         else   /* LINK */
         {
            var link_data = data;
            if( link_data.start_node != null )
              UpdateNodePosition( link_data.start_node.graphics, null );
            if( link_data.end_node != null )
              UpdateNodePosition( link_data.end_node.graphics, null );
            
            // Update stored point values.
            StorePointData( link_data, SelectedObject );
         }
         
         // Update the start point for the next move.
         GLG.IHChangeOParameter( ih, "start_point", cursor_pos_obj );
         
         Viewport.Update();
         break;
         
       case IH_MOUSE_RELEASED:
         // Uninstall the handler on mouse release.
         GLG.IHUninstall();
         break;
         
       default:
         /* Unrecognized token: uninstall current handler and invoke the 
            parent handler, passing the call_event to it.
         */
         GLG.IHUninstallWithEvent( call_event );
         break;
      }
      break;
      
    case GLG.GlgCallEventType.CLEANUP_EVENT:
      TraceMouseMove = false;
      TraceMouseRelease = false;
      break;
   }
}

////////////////////////////////////////////////////////////////////////
// The AddNodeIH handler is invoked with the IH_MOUSE_PRESSED and
// IH_MOUSE_MOVED tokens by the IHCallCurrIHWithToken function
// in the TraceCallback.
//
// Handler parameters:
//   template    (obj)
//   button_name (string)
////////////////////////////////////////////////////////////////////////
function AddNodeIH( ih, call_event )
{
   switch( GLG.IHGetType( call_event ) )
   {
    case GLG.GlgCallEventType.HI_SETUP_EVENT:
      TraceMouseMove = true;
      SetRadioBox( GLG.IHGetSParameter( ih, "button_name" ) );
      SetPrompt( "Position the node." );
      Viewport.Update(); 
      break;
      
    case GLG.GlgCallEventType.MESSAGE_EVENT:
      var token = GLG.IHGetToken( call_event );
      switch( token )
      {
       case IH_MOUSE_PRESSED:
         // Query node type.
         var template = GLG.IHGetOParameter( ih, "template" );
         var node_type = Math.trunc( template.GetDResource( "Index" ) );
         
         var cursor_pos_obj =
           GLG.IHGetOParameter( GLG.IH_GLOBAL, "$cursor_pos" );
         var new_node = AddNodeAt( node_type, null, cursor_pos_obj, 
                                   GLG.GlgCoordType.SCREEN_COORD );
         CustomAddObjectCB( Viewport, new_node, GetData( new_node ), true );
         SelectGlgObject( new_node, NODE );

         /* In StickyCreateMode, keep adding nodes at each mouse click 
            position.
         */
         if( !StickyCreateMode )
           GLG.IHUninstall();
         
         Viewport.Update();
         break;
         
       case IH_MOUSE_MOVED: 
         break;   // Allow: do nothing.
         
       default:
         /* Unrecognized token: uninstall current handler and invoke the 
            parent handler, passing the call_event to it.
         */
         GLG.IHUninstallWithEvent( call_event );
         break;
      }
      break;

    // Triggered when handler is uninstalled.      
    case GLG.GlgCallEventType.CLEANUP_EVENT:
      TraceMouseMove = false;
      SetRadioBox( SELECT_BUTTON_NAME );   // Highlight Select button
      SetPrompt( "" );
      Viewport.Update();
      break;
   }
}

////////////////////////////////////////////////////////////////////////
// The AddLinkIH handler is invoked via IHCallCurrIHWithToken with
// the following tokens:
//   IH_MOUSE_PRESSED, IH_MOUSE_MOVED, IH_ESC and IH_MOUSE_BUTTON3 are 
//      passed from the TraceCallback;
//   IH_FINISH_LINK is passed from the AddLinkIH handler itself when the
//      user finishes link creation.
//
// Handler parameters:
//   template (obj)
//   button_name (string)
////////////////////////////////////////////////////////////////////////
function AddLinkIH( ih, call_event )
{
   var
     template,
     cursor_pos_obj,
     start_point_obj,
     sel_node,
     point, 
     pt_array,
     drag_link;
   var link_data;
   var
     link_type,
     edge_type;
   var
     first_node,
     middle_point_added;
   
   switch( GLG.IHGetType( call_event ) )
   {
    case GLG.GlgCallEventType.HI_SETUP_EVENT:
      TraceMouseMove = true;
      SetRadioBox( GLG.IHGetSParameter( ih, "button_name" ) );
      
      // Store link type
      template = GLG.IHGetOParameter( ih, "template" );
      link_type = Math.trunc( template.GetDResource( "Index" ) );
      GLG.IHSetIParameter( ih, "link_type", link_type );
      
      // Store edge type
      var link_info = GetCPContainer( template );
      edge_type = link_info.type;
      GLG.IHSetIParameter( ih, "edge_type", edge_type );

      GLG.IHSetOParameter( ih, "drag_link", null );
      /* Fall through */

    case GLG.GlgCallEventType.HI_RESETUP_EVENT:
      /* This event type is triggered by the IHResetup call in this handler,
         which is used to reset the handler to the inital state to continue
         creating new links of this type when StickyCreateMode=True.
      */
      GLG.IHSetBParameter( ih, "first_node", true );
      GLG.IHSetBParameter( ih, "middle_point_added", false );
      
      SetPrompt( "Select the first node or attachment point." );
      Viewport.Update();
      break;
      
    case GLG.GlgCallEventType.MESSAGE_EVENT:
      first_node = GLG.IHGetBParameter( ih, "first_node" );
      drag_link = GLG.IHGetOParameter( ih, "drag_link" );
      
      var token = GLG.IHGetToken( call_event );
      switch( token )
      {
       case IH_MOUSE_MOVED:
         cursor_pos_obj = GLG.IHGetOParameter( GLG.IH_GLOBAL, "$cursor_pos" );
         StoreAttachmentPoints( cursor_pos_obj, token );
         
         point = GLG.IHGetOParameter( ih, "attachment_point" );
         pt_array = GLG.IHGetOParameter( ih, "attachment_array" );
         sel_node = GLG.IHGetOParameter( ih, "attachment_node" );
         
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
            link_data = GetData( drag_link );
            
            /* First time: set link direction depending of the direction
               of the first mouse move, then make the link visible.
            */
            if( link_data.first_move )
            {
               start_point_obj = GLG.IHGetOParameter( ih, "start_point" );
               SetEdgeDirection( drag_link, 
                                 start_point_obj, cursor_pos_obj );
               drag_link.SetDResource( "Visibility", 1.0 );
               link_data.first_move = false;
            }

            SetLastPoint( drag_link, cursor_pos_obj, false, false );
            
            middle_point_added = GLG.IHGetBParameter( ih, "middle_point_added" );
            if( !middle_point_added )
              SetArcMiddlePoint( drag_link );
         }
         Viewport.Update();
         break;
               
       case IH_MOUSE_PRESSED:
         if( TouchStart )
           GLG.SetTouchMode();

         cursor_pos_obj = GLG.IHGetOParameter( GLG.IH_GLOBAL, "$cursor_pos" );
         StoreAttachmentPoints( cursor_pos_obj, token );
         
         point = GLG.IHGetOParameter( ih, "attachment_point" );
         pt_array = GLG.IHGetOParameter( ih, "attachment_array" );
         sel_node = GLG.IHGetOParameter( ih, "attachment_node" );
         
         if( point != null )
         {
            if( first_node )
            {	       
               GLG.IHSetOParameter( ih, "first_point", point );
               
               link_type = GLG.IHGetIParameter( ih, "link_type" );
               drag_link = AddLinkObject( link_type, null );
               GLG.IHSetOParameter( ih, "drag_link", drag_link );
               
               // First point
               ConstrainLinkPoint( drag_link, point, false );
               AttachFramePoints( drag_link );
               
               // Wire up the start node
               link_data = GetData( drag_link );
               link_data.start_node = GetData( sel_node );
               
               /* Store cursor position for setting direction based on the
                  first mouse move.
               */
               GLG.IHSetOParameter( ih, "start_point", cursor_pos_obj );
               link_data.first_move = true;
               drag_link.SetDResource( "Visibility", 0.0 );
               
               GLG.IHChangeBParameter( ih, "first_node", false );
               SetPrompt( "Select the second node or additional points." );
            }
            else
            {  
               var first_point;
               
               first_point = GLG.IHGetOptOParameter( ih, "first_point", null );
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
                 GLG.IHGetBParameter( ih, "middle_point_added" );
               if( !middle_point_added )
                 SetArcMiddlePoint( drag_link );
               
               // Wire up the end node
               link_data = GetData( drag_link );
               link_data.end_node = GetData( sel_node );
               
               FinalizeLink( drag_link );
               GLG.IHChangeOParameter( ih, "drag_link", null );
               
               if( StickyCreateMode )
               {
                  GLG.IHCallCurrIHWithToken( IH_FINISH_LINK );
                  GLG.IHResetup( ih );   // Start over to create more links.
               }
               else
                 GLG.IHUninstall();   /* Will call IH_FINISH_LINK */
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
            GLG.IHChangeBParameter( ih, "middle_point_added", true );
            
            /* Set the last point of a linear link or the middle point of 
               the arc link.
            */
            edge_type = GLG.IHGetIParameter( ih, "edge_type" );            
            SetLastPoint( drag_link, cursor_pos_obj, false, 
                          edge_type == GLG.GlgObjectType.ARC );
            AttachFramePoints( drag_link );
            
            /* Set the last point of the arc link, offsetting it from the 
               middle point.
            */
            if( edge_type == GLG.GlgObjectType.ARC )
              SetLastPoint( drag_link, cursor_pos_obj, true, false );
         }
         Viewport.Update();
         break;  
         
       case IH_FINISH_LINK:    // Finish the current link.
         drag_link = GLG.IHGetOptOParameter( ih, "drag_link", null );
         if( drag_link != null )
         {
            // Finish the last link
            if( AllowUnconnectedLinks && FinishLink( drag_link ) )
              ;   // Keep the link even if its second end is not connected.
            else
            {
               // Delete the link if its second end is not connected.
               var group = DrawingArea.GetResourceObject( "ObjectGroup" );
               group.DeleteThisObject( drag_link );
            }
            
            GLG.IHChangeOParameter( ih, "drag_link", null );
         }
         EraseAttachmentPoints();   
         Viewport.Update();
         break;
         
       case IH_ESC:
       case IH_MOUSE_BUTTON3:
         drag_link = GLG.IHGetOptOParameter( ih, "drag_link", null );
         if( drag_link != null && StickyCreateMode )
         {
            // Stop adding points to this link.
            GLG.IHCallCurrIHWithToken( IH_FINISH_LINK );
            GLG.IHResetup( ih );   // Start over to create more links.
         }
         else
           /* No curr link or !StickyCreateMode: finish the current link 
              if any and stop adding links.
           */
           GLG.IHUninstall();      // Will call IH_FINISH_LINK
         break;
         
       default:
         GLG.IHUninstallWithEvent( call_event ); // Pass to the parent IH.
         break;
      }
      break;
      
    case GLG.GlgCallEventType.CLEANUP_EVENT:
      GLG.IHCallCurrIHWithToken( IH_FINISH_LINK ); // Finish the current link
      
      TraceMouseMove = false;
      SetRadioBox( SELECT_BUTTON_NAME );   // Highlight Select button
      SetPrompt( "" );
      Viewport.Update();
      break;
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
function StoreAttachmentPoints( cursor_pos_obj, event_type )
{
   var
     point = null,
     pt_array = null,
     sel_node = null;
      
   var selection = GetObjectsAtCursor( cursor_pos_obj );
   
   var num_selected;
   if( selection != null && ( num_selected = selection.GetSize() ) != 0 )
   {
      /* Some object were selected, process the selection to find the point 
         to connect to */
      for( var i=0; i < num_selected; ++i )
      {
         var sel_object = selection.GetElement( i );
            
         /* Find if the object itself is a link or a node, or if it's a part
            of a node. If it's a part of a node, get the node object ID.
         */
         var selection_info = GetSelectedObject( sel_object );
         sel_object = selection_info.glg_object;
         var selection_type = selection_info.type;
            
         if( selection_type == NODE )
         {
            var type = Math.trunc( sel_object.GetDResource( "Type" ) );
            if( type == GLG.GlgObjectType.REFERENCE )
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
      GLG.IHSetOParameter( GLG.IH_CURR, "attachment_point", point );
      GLG.IHSetOParameter( GLG.IH_CURR, "attachment_array", pt_array );
      GLG.IHSetOParameter( GLG.IH_CURR, "attachment_node", sel_node );
   }
}
   
////////////////////////////////////////////////////////////////////////
function EditPropertiesIH( ih, call_event )
{
   switch( GLG.IHGetType( call_event ) )
   {
    case GLG.GlgCallEventType.HI_SETUP_EVENT:
      break;
      
    case GLG.GlgCallEventType.MESSAGE_EVENT:
      GLG.IHSetBParameter( GLG.IH_GLOBAL, "token_used", true );
      
      var token = GLG.IHGetToken( call_event );
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
         Viewport.Update();
         break;
               
       case IH_DATASOURCE_SELECT:   /* process diagram only */
         if( SelectedObject == null )
         {
            SetError( "Select an object in the drawing first." );
            Viewport.Update();
            break;
         }
         
         /* Returns with IH_DATASOURCE_SELECTED and $rval containing 
            selected datasource string. 
         */
         GLG.IHInstallAsInterface( GetDataSourceIH );
         GLG.IHStart();
         break;
         
       case IH_DATASOURCE_SELECTED:   /* process diagram only */
         // Get the selection.
         var rval = GLG.IHGetSParameter( ih, "$rval" );
         Viewport.SetSResource( "Dialog/DialogDataSource/TextString", rval );
         DialogDataChanged = true;
         break;
         
       case IH_DIALOG_CANCEL:
         DialogDataChanged = false;
         FillData();
         Viewport.Update();
         break;
         
       case IH_DIALOG_APPLY:
         ApplyDialogData();
         DialogDataChanged = false;
         Viewport.Update();
         break;
         
       case IH_DIALOG_CLOSE:
         if( !DialogDataChanged )    // No changes: close the dialog.
         {
            Viewport.SetDResource( "Dialog/Visibility", 0.0 );
            Viewport.Update();
            break;
         }
         
         // Data changed: confirm discarding changes.
         
         /* Store the CLOSE action that initiated the confirmation,
            to close the data dialog when confirmed.
         */
         GLG.IHSetIParameter( ih, "op", token );
         
         GLG.IHCallCurrIHWithToken( IH_DIALOG_CONFIRM_DISCARD );
         break;
         
       case IH_DIALOG_CONFIRM_DISCARD:
         /* Returns with IH_OK with IH_CANCEL.
            All parameters are optional, except for the message parameter.
         */
         GLG.IHInstallAsInterface( ConfirmIH );
         GLG.IHSetSParameter( GLG.IH_NEW,
                              "title", "Confirmation Dialog" );
         GLG.IHSetSParameter( GLG.IH_NEW, "message", 
                              "Do you want to save dialog changes?" );
         GLG.IHSetSParameter( GLG.IH_NEW, "ok_label", "Save" );
         GLG.IHSetSParameter( GLG.IH_NEW, "cancel_label", "Discard" );
         GLG.IHSetBParameter( GLG.IH_NEW, "modal_dialog", true ); 
         GLG.IHStart();
         break;
               
       case IH_OK:       // Save changes.
       case IH_CANCEL:   // Discard changes.
         GLG.IHCallCurrIHWithToken( token == IH_OK ? 
                                    IH_DIALOG_APPLY : IH_DIALOG_CANCEL );
         
         /* Close the data dialog if that's what initiated the confirmation. */
         if( GLG.IHGetOptIParameter( ih, "op", 0 ) == IH_DIALOG_CLOSE )
           GLG.IHCallCurrIHWithToken( IH_DIALOG_CLOSE );
         break;
               
       case IH_ESC: 
         break;     // Allow: do nothing.
               
       default: 
         if( !DialogDataChanged )    // No changes.
         {
            GLG.IHSetBParameter( GLG.IH_GLOBAL, "token_used", false );
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
         
         GLG.IHCallCurrIHWithToken( IH_DIALOG_CONFIRM_DISCARD );
         break;
      }
      break;
      
    case GLG.GlgCallEventType.CLEANUP_EVENT:
      DialogDataChanged = false;
      break;
   }
}

////////////////////////////////////////////////////////////////////////
function GetDataSourceIH( ih, call_event )
{
   switch( GLG.IHGetType( call_event ) )
   {
    case GLG.GlgCallEventType.HI_SETUP_EVENT:
      Viewport.SetDResource( "DataSourceDialog/Visibility", 1.0 ); 
      Viewport.Update();
      break;
      
    case GLG.GlgCallEventType.MESSAGE_EVENT:
      var token = GLG.IHGetToken( call_event );
      switch( token )
      {
       case IH_DATASOURCE_APPLY:
       case IH_DATASOURCE_LIST_SELECTION:
         var sel_item =
           Viewport.GetSResource( "DataSourceDialog/DSList/SelectedItem" );
         GLG.IHUninstall();

         /* Set the return value in the parent datastore and call the parent. */
         GLG.IHSetSParameter( GLG.IH_CURR, "$rval", sel_item );
         GLG.IHCallCurrIHWithToken( IH_DATASOURCE_SELECTED ); 
         break;
         
       case IH_DATASOURCE_CLOSE:
         GLG.IHUninstall();
         break;
         
       default: 
         GLG.IHUninstallWithEvent( call_event );
         break;
      }
      break;
      
    case GLG.GlgCallEventType.CLEANUP_EVENT:
      Viewport.SetDResource( "DataSourceDialog/Visibility", 0.0 ); 
      Viewport.Update();
      break;
   }
}

////////////////////////////////////////////////////////////////////////
// OK/Cancel confirmation dialog. 
//
// Handler parameters:
//   message
//   title (optional, default "Confirm")
//   ok_label (optional, def. "OK")
//   cancel_label (optional, def. "Cancel")
//   modal_dialog (optional, def. true)
//   allow_ESC (optional, def. true )
//   requested_op (optional, default - undefined (0) )
////////////////////////////////////////////////////////////////////////
function ConfirmIH( ih, call_event )
{
   switch( GLG.IHGetType( call_event ) )
   {
    case GLG.GlgCallEventType.HI_SETUP_EVENT:
      var dialog = Viewport.GetResourceObject( "OKDialog" );
      dialog.SetSResource( "ScreenName",
                           GLG.IHGetOptSParameter( ih, "title", "Confirm" ) );
      dialog.SetSResource( "OKDialogOK/LabelString",
                           GLG.IHGetOptSParameter( ih, "ok_label", "OK" ) );
      dialog.SetSResource( "OKDialogCancel/LabelString",
                           GLG.IHGetOptSParameter( ih, "cancel_label",
                                                   "Cancel" ) );
      dialog.SetSResource( "DialogMessage/String",
                           GLG.IHGetSParameter( ih, "message" ) );
      dialog.SetDResource( "DialogMessage/FontType", 6 );
      dialog.SetDResource( "Visibility", 1.0 );
      Viewport.Update();
      break;
            
    case GLG.GlgCallEventType.MESSAGE_EVENT:
      var token = GLG.IHGetToken( call_event );
      switch( token )
      {
       case IH_OK:
       case IH_CANCEL:
         GLG.IHUninstallWithToken( token );   // Pass selection to the parent.
         break;

       case IH_ESC:
         if( GLG.IHGetOptBParameter( ih, "allow_ESC", true ) )
           GLG.IHUninstall();
         break;
         
       default: 
         if( GLG.IHGetOptBParameter( ih, "modal_dialog", true ) )
         {
            RestoreToggleStateWhenDisabled( token );
            SetError( "Please select one of the choices from the comfirmation dialog." );
            Viewport.Update();
         }
         else
           GLG.IHUninstallWithEvent( call_event );
         break;
      }
      break;
      
    case GLG.GlgCallEventType.CLEANUP_EVENT:
      Viewport.SetDResource( "OKDialog/Visibility", 0.0 );
      Viewport.Update();
      break;
   }
}

////////////////////////////////////////////////////////////////////////
// Restore state of any pressed toggles ignored or disabled by the 
// confirmation dialog.
////////////////////////////////////////////////////////////////////////
function RestoreToggleStateWhenDisabled( token )
{
   switch( token )
   {
    case IH_ICON_SELECTED:
      DeselectButton( GLG.IHGetSParameter( GLG.IH_GLOBAL, "$selected_button" ) );
      break;
      
    case IH_CREATION_MODE:
      SetCreateMode( true );
      break;
   }
}

////////////////////////////////////////////////////////////////////////
function UninstallPassTroughIH( call_event )
{
   if( GLG.IHGetBParameter( GLG.IH_GLOBAL, "fall_through_call" ) )
     /* A fall-through invokation: a parent handler passed an unused event 
        to this IH for possible processing, discard The event. Passing 
        the event to the parent IH would cause infinite recursion.
     */
     GLG.IHUninstall();
   else
     /* Not a pass-through invokation: the IH was not uninstalled and 
        is current. Pass an unused event to the parent handler for
        processing.
     */              
     GLG.IHUninstallWithEvent( call_event );           
}

////////////////////////////////////////////////////////////////////////
function ShowAttachmentPoints( point, pt_array, sel_node, highlight_type )
{
   var screen_point;

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
      AttachmentMarker.SetGResourceFromPoint( "Point", world_coord );
      
      AttachmentMarker.SetDResource( "HighlightType", highlight_type );
   }
   else if( pt_array != null )
   {
      if( sel_node == AttachmentNode )
        return;    // Attachment points are already shown for this node.
      
      // Erase previous attachment feedback if shown.
      EraseAttachmentPoints();   
      
      var size = pt_array.GetSize();
      AttachmentArray =
        GLG.CreateObject( GLG.GlgObjectType.ARRAY,
                          GLG.GlgContainerType.GLG_OBJECT, size, 0, null );
      AttachmentNode = sel_node;
         
      for( var i=0; i<size; ++i )
      {
         var marker = PointMarker.CopyObject();
         
         point = pt_array.GetElement( i );
            
         // Get the screen coords of the connector point.
         screen_point = point.GetGResource( "XfValue" );
         
         DrawingArea.ScreenToWorld( true, screen_point, world_coord );
         
         // Position the feedback marker over the connector.
         marker.SetGResourceFromPoint( "Point", world_coord );
         
         marker.SetDResource( "HighlightType", highlight_type );
         
         AttachmentArray.AddObjectToBottom( marker );
      }
      DrawingArea.AddObjectToBottom( AttachmentArray );
   }
}

////////////////////////////////////////////////////////////////////////
// Erases attachment points feedback if shown. Returns true if feedback
// was erased, of false if there was nothing to erase.
////////////////////////////////////////////////////////////////////////
function EraseAttachmentPoints()
{
   if( AttachmentMarker != null )
   {
      DrawingArea.DeleteThisObject( AttachmentMarker );
      AttachmentMarker = null;
      return true;
   }
   
   if( AttachmentArray != null )
   {
      DrawingArea.DeleteThisObject( AttachmentArray );
      AttachmentArray = null;
      AttachmentNode = null;
      return true;
   }
   
   return false;    // Nothing to erase.
}

////////////////////////////////////////////////////////////////////////
function FinalizeLink( link )
{
   var arrow_type = link.GetResourceObject( "ArrowType" );
   if( arrow_type != null )
     arrow_type.SetDResource( null, GLG.GlgArrowType.MIDDLE_FILL_ARROW );

   var link_data = GetData( link );

   // Store points
   StorePointData( link_data, link );
   
   // Add link data to the link list
   var link_list = CurrentDiagram.getLinkList();
   link_list.push( link_data );
   
   // After storing color: changes color to select.
   SelectGlgObject( link, LINK );
   
   CustomAddObjectCB( Viewport, link, GetData( link ), false );
}

////////////////////////////////////////////////////////////////////////
// Stores point coordinates in the link data structure as an array.
////////////////////////////////////////////////////////////////////////
function StorePointData( link_data, link )
{
   var link_info = GetCPContainer( link );
   var point_container = link_info.glg_object;
      
   var num_points = point_container.GetSize();

   // Create a new array and discard the old one for simplicity.
   link_data.point_array = new Array( num_points );
   for( var i=0; i<num_points; ++i )
   {
      var point_obj = point_container.GetElement( i ); // GLG Data of G type
      var glg_point = point_obj.GetGResource( null );  // GlgPoint
      link_data.point_array[i] = glg_point;
   }
}
      
////////////////////////////////////////////////////////////////////////
// Restores link's middle points from the link data's stored vector.
// The first and last point's values are not used: they are constrained 
// to nodes and positioned/controlled by them.
////////////////////////////////////////////////////////////////////////
function RestorePointData( link_data, link )
{
   // Set middle point values
   if( link_data.point_array != null )
   {
      var num_points = link_data.point_array.length;
      
      var link_info = GetCPContainer( link );
      var point_container = link_info.glg_object;
      
      /* Skip the first and last point if they are constrained to nodes.
         Set only the unconnected ends and middle points.
      */
      var start = ( link_data.getStartNode() != null ? 1 : 0 );
      var end = ( link_data.getEndNode() != null ? num_points - 1 : num_points );

      /* Skip the first and last point: constrained to nodes.
         Set only the middle points.
      */
      for( var i=start; i<end; ++i )
      {
         var point = point_container.GetElement( i );  // GLG data of G type
         var saved_point = link_data.point_array[i];   // GlgPoint
         point.SetGResourceFromPoint( null, saved_point );
      }
   }
}

////////////////////////////////////////////////////////////////////////
function GetPointFromObj( point_obj )
{
   return point_obj.GetGResource( null );
}

////////////////////////////////////////////////////////////////////////
function MoveObject( object, start_point_obj, end_point_obj )
{
   object.MoveObject( GLG.GlgCoordType.SCREEN_COORD,
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
function MoveLink( link, start_point_obj, end_point_obj )
{
   var
     start_node = null,
     end_node = null;
   var
     type1 = GLG.GlgObjectType.REFERENCE, 
     type2 = GLG.GlgObjectType.REFERENCE;
   
   var link_data = GetData( link );
   
   if( link_data.getStartNode() != null )
   {         
      start_node = link_data.getStartNode().graphics;
      type1 = Math.trunc( start_node.GetDResource( "Type" ) );
   }
      
   if( link_data.getEndNode() != null )
   {
      end_node = link_data.getEndNode().graphics;
      type2 = Math.trunc( end_node.GetDResource( "Type" ) );
   }
   
   var start_point = GetPointFromObj( start_point_obj );
   var end_point = GetPointFromObj( end_point_obj );
     
   if( type1 == GLG.GlgObjectType.REFERENCE &&
       type2 == GLG.GlgObjectType.REFERENCE )
   {
      // Nodes are reference objects (or null), moving the link moves nodes.
      link.MoveObject( GLG.GlgCoordType.SCREEN_COORD, start_point, end_point );
   }
   else   // Nodes with multiple attachment points.
   {
      // Unconstrain link points
      var
        point1 = null,
        point2 = null;

      if( start_node != null )
        point1 = UnConstrainLinkPoint( link, false );
      if( end_node != null )
        point2 = UnConstrainLinkPoint( link, true );
      
      DetachFramePoints( link );
      
      // Move the link
      link.MoveObject( GLG.GlgCoordType.SCREEN_COORD, start_point, end_point );
      
      // Move start node, then reattach the link.
      if( start_node != null )
      {
         start_node.MoveObject( GLG.GlgCoordType.SCREEN_COORD, 
                                start_point, end_point );
         ConstrainLinkPoint( link, point1, false );
      }
      
      // Move end node, then reattach the link.
      if( end_node != null )
      {
         end_node.MoveObject( GLG.GlgCoordType.SCREEN_COORD,
                              start_point, end_point );
         ConstrainLinkPoint( link, point2, true );
      }
      
      AttachFramePoints( link );
   }
}

////////////////////////////////////////////////////////////////////////
function UpdateNodePosition( node, node_data )
{
   if( node_data == null )
     node_data = GetData( node );
   
   GetPosition( node, world_coord );
   node_data.position.CopyFrom( world_coord );
}

////////////////////////////////////////////////////////////////////////
function GetObjectsAtCursor( cursor_pos_obj )
{
   var cursor_pos = GetPointFromObj( cursor_pos_obj );

   /* Select all objects in the vicinity of the +-SELECTION_RESOLUTION 
      pixels from the actual mouse click position.
   */
   select_rect.p1.x = cursor_pos.x - SELECTION_RESOLUTION;
   select_rect.p1.y = cursor_pos.y - SELECTION_RESOLUTION;
   select_rect.p2.x = cursor_pos.x + SELECTION_RESOLUTION;
   select_rect.p2.y = cursor_pos.y + SELECTION_RESOLUTION;
   
   return GLG.CreateSelection( DrawingArea, select_rect, DrawingArea );
}

////////////////////////////////////////////////////////////////////////
function SelectGlgObject( glg_object, selected_type )
{
   var name;

   if( GLG.ObjectsEqual( glg_object, SelectedObject ) )
     return;   // No change
   
   if( LastColor != null ) // Restore the color of previously selected node
   {
      LastColor.SetResourceFromObject( null, StoredColor );
      LastColor = null;
   }

   SelectedObject = glg_object;
   SelectedObjectType = selected_type;
   
   // Show object selection
   if( glg_object != null )
   {
      // Change color to highlight selected node or link.
      if( glg_object.HasResourceObject( "SelectColor" ) )
      {
         LastColor = glg_object.GetResourceObject( "SelectColor" );
         
         // Store original color
         StoredColor.SetResourceFromObject( null, LastColor );
         
         // Set color to red to highlight selection.
         LastColor.SetGResource( null, 1.0, 0.0, 0.0 );
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
function Cut()
{
   if( NoSelection() )
     return;
   
   // Disallow deleting a node without deleting the link first.
   if( SelectedObjectType == NODE && NodeConnected( SelectedObject ) )
   {
      SetError( "Remove links connected to the node before removing the node!" );
      return;
   }
   
   var group = DrawingArea.GetResourceObject( "ObjectGroup" );
   
   if( group.ContainsObject( SelectedObject ) )
   {
      // Store the node or link in the cut buffer.
      CutBuffer = SelectedObject;
      CutBufferType = SelectedObjectType;
      
      // Delete the node
      group.DeleteThisObject( SelectedObject );

      // Delete the data
      var data = GetData( SelectedObject );
      var list = null;

      CustomCutObjectCB( Viewport, SelectedObject, data,
                         SelectedObjectType == NODE );

      if( SelectedObjectType == NODE )
        list = CurrentDiagram.getNodeList();
      else  // Link
        list = CurrentDiagram.getLinkList();

      RemoveArrayElement( list, data );

      SelectGlgObject( null, 0 );
   }
   else
     SetError( "Cut failed." );    
}

////////////////////////////////////////////////////////////////////////
function RemoveArrayElement( array, data )
{
   for( var i=0; i<array.length; ++i )
     if( array[i] == data )
     {
        array.splice( i, 1 );
        return;
     }
      
   SetError( "Deleting data failed!" );
}

////////////////////////////////////////////////////////////////////////
function Paste()
{
   if( CutBuffer == null )
   {
      SetError( "Empty cut buffer, cut some object first." );
      return;
   }
   
   var group = DrawingArea.GetResourceObject( "ObjectGroup" );

   var data = GetData( CutBuffer );
   CustomPasteObjectCB( Viewport, CutBuffer, data, CutBufferType == NODE );

   var list = null;
   if( CutBufferType == NODE )
   {
      group.AddObjectToBottom( CutBuffer );     // In front
      list = CurrentDiagram.getNodeList();
      list.push( data );
   }
   else // LINK
   {
      group.AddObjectToTop( CutBuffer );        // Behind
      list = CurrentDiagram.getLinkList();
      list.push( data );
   }
   
   SelectGlgObject( CutBuffer, CutBufferType );
   
   // Allow pasting just once to avoid handling the data copy
   CutBuffer = null;
}

////////////////////////////////////////////////////////////////////////
// Saves a diagram as a JSON string.
// The demo uses this JSON string to demonstrate loading a diagram from
// JSON when the Load button is pressed.
// In an application, the JSON string may be sent to a server to be
// saved.
////////////////////////////////////////////////////////////////////////
function Save( diagram )
{
   /* Using \t for better console output. A real application could omit
      the third parameter for more compact output.
   */
   var json_string = JSON.stringify( diagram, replacer, '\t' );

   /* Save the current diagram to use it as test for loading.
      In a real application, the JSON string will be sent to the server
      to be saved.
   */
   SavedDiagramData = json_string;
   
   // Empty the drawing area.
   UnsetDiagram( diagram );

   // Print to the console for demo and debugging.
   WriteLine( "SAVED DIGRAM DATA" );
   WriteLine( json_string );
}

////////////////////////////////////////////////////////////////////////
function replacer( name, value )
{
   switch( name )
   {
    case "datasource":
      if( ProcessDiagram )
        return value;
      else
        return undefined;         // Save datasource only for ProcessDiagram.

    case "start_node":
    case "end_node":
    if( value == null )
      return value;      // This is an unconnected end of a link.

    // For the start and end nodes, save their indices in the node array. 
    var node_list = CurrentDiagram.getNodeList();
    return node_list.indexOf( value );

    case "position":
    case "link_color": 
      // These fields are GLG points: save only their x, y and z fields.
      return { x : value.x, y : value.y, z : value.z };

    case "point_array":
      if( value.length == 0 )
        return value;
      
      /* Point array contains GLG data objects of G (geometrical) type,
         save their x, y and z fields.
      */
      var array = [];
      for( var i=0; i<value.length; ++i )
      {
         var point = value[ i ];   // GlgPoint
         array.push( { x : point.x, y : point.y, z : point.z } );
      }
      return array;
      
    default: return value;
  
      // These fields are used only at run time and do not get saved.
    case "graphics":    return undefined;
    case "first_move":  return undefined;
   }
}

////////////////////////////////////////////////////////////////////////
// Load a diagram from a JSON string.
// The demo loads a diagram from a diagram previously saved as a
// JSON string by the Save() method.
// In a real application, a previously stored diagram may be loaded
// from a JSON string receved from a server.
////////////////////////////////////////////////////////////////////////
function Load()
{
   /* In the demo, load the previously saved diagram and use it to demonstrate
      how to load a diagram from a JSON string.
   */
   if( SavedDiagramData == null )
     SetError( "Save the diagram first." );
   else
   {
      UnsetDiagram( CurrentDiagram );   // Erase the current diagram.
      
      /* Load a new diagram from a previously saved JSON string.
         In a real application, a prevously saved diagram can be retrieved
         from a server in the JSON format.
      */
      var diagram_data;
      try
      {
         diagram_data = JSON.parse( SavedDiagramData );
      }
      catch( error )
      {
         SetError( "JSON parse error: " + error );
         SavedDiagramData = null;
         return;
      }
      
      SetDiagram( diagram_data );
      SavedDiagramData = null;
   }
}

////////////////////////////////////////////////////////////////////////
function Print()
{
   window.print();
}

////////////////////////////////////////////////////////////////////////
// If AllowUnconnectedLinks=true, keep the link if it has at least two 
// points.
////////////////////////////////////////////////////////////////////////
function FinishLink( link )
{
   var link_info = GetCPContainer( link );
   
   var point_container = link_info.glg_object;
   
   var edge_type = link_info.type;
   if( edge_type == GLG.GlgObjectType.ARC )
     return false;      // Disconnected arc links are not allowed.
   
   var size = point_container.GetSize();
   
   /* The link must have at least two points already defined, and one extra
      point that was added to drag the next point.
   */
   if( size < 3 )
     return false;
   
   // Delete the unfinished, unconnected point.
   var suspend_info = link.SuspendObject();
   point_container.DeleteBottomObject();
   link.ReleaseObject( suspend_info );
   
   FinalizeLink( link );
   return true;
}

////////////////////////////////////////////////////////////////////////
function SetPrompt( message )
{
   Viewport.SetSResource( "Prompt/String", message );
   Viewport.Update();
}

////////////////////////////////////////////////////////////////////////
function SetError( message )
{
   GLG.Bell();
   SetPrompt( message );
   console.log( message );
   alert( message );
}

////////////////////////////////////////////////////////////////////////
function NoSelection()
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
function AddNodeAt( node_type, node_data, position_obj, coord_system )
{     
   var store_position;
   
   if( node_data == null )
   {
      node_data = new GlgNodeData( null );
      node_data.node_type = node_type;
      
      // Add node data to the node list
      var node_list = CurrentDiagram.getNodeList();
      node_list.push( node_data );
      
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
   var new_node = CreateNode( node_data );
   
   // Make label visible and set its string.
   if( new_node.HasResourceObject( "Label" ) )
   {
      new_node.SetDResource( "Label/Visibility", 1.0 );
      new_node.SetSResource( "Label/String", node_data.object_label );
   }
   
   // Store datasource as a tag of the node's Value resource, if it exists.
   if( ProcessDiagram )
   {
      var value_obj = new_node.GetResourceObject( "Value" );
      var datasource = node_data.datasource;
      
      if( value_obj != null && datasource != null && datasource.length != 0  )
      {
         var tag_obj =
           GLG.CreateObject( GLG.GlgObjectType.TAG,
                             "Value", datasource, null, null );
         value_obj.SetResourceObject( "TagObject", tag_obj );
      }
   }
   
   var position;
   
   // No cursor position: get position from the data struct.      
   if( position_obj == null )
   {
      /* Using CreateGlgPointFromPoint() instead of CopyGlgPoint().
         We are loading new diagram: node_data.position is obtained from JSON.
         It has x, y and z properties, but it is not a GLG point.
      */
      position = GLG.CreateGlgPointFromPoint( node_data.position );
      store_position = false;
   }
   else
   {
      position = GetPointFromObj( position_obj );
      store_position = true;
   }
   
   node_data.graphics = new_node;  // Pointer from data struct to graphics

   /* dd the object to the drawing first, so that it's hierarchy is setup
      for positioning it.
   */
   var group = DrawingArea.GetResourceObject( "ObjectGroup" );
   group.AddObjectToBottom( new_node );

   // Transform the object to set its size and position.
   PlaceObject( new_node, position, coord_system, world_coord );
   
   if( store_position )
     node_data.position.CopyFrom( world_coord );

   return new_node;
}

////////////////////////////////////////////////////////////////////////
function CreateNode( node_data )
{
   // Get node template from the palette
   var new_node = NodeObjectArray.GetElement( node_data.node_type );
         
   // Create a new node instance 
   new_node = new_node.CloneObject( GLG.GlgCloneType.STRONG_CLONE );
   
   /* Name node using an "object" prefix (used to distiguish
      nodes from links on selection).
   */
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
function CreateLink( link_data )
{
   // Get link template from the palette
   var new_link = LinkObjectArray.GetElement( link_data.link_type );
   
   // Create a new link instance 
   new_link = new_link.CloneObject( GLG.GlgCloneType.STRONG_CLONE );
   
   /* Name link using a "link" prefix (used to distiguish
      links from nodes on selection).
   */
   new_link.SetSResource( "Name", "link" );

   /* If point_array exists, create/add middle link points
      If not an arc, it's created with 2 points by default, 
      add ( num_points - 2 ) more. If it's an arc, AddLinkPoints
      will do nothing.
   */
   var num_points;
   if( link_data.point_array != null &&
       ( num_points = link_data.point_array.length ) > 2 )
     AddLinkPoints( new_link, num_points - 2 );
   
   AddCustomData( new_link, link_data );
   
   return new_link;
}

/////////////////////////////////////////////////////////////////////////
// Connects the first or last point of the link.
/////////////////////////////////////////////////////////////////////////
function ConstrainLinkPoint( link, point, last_point )
{
   var link_info = GetCPContainer( link );
   
   var point_container = link_info.glg_object;
   
   var link_point =
     point_container.GetElement( last_point ?
                                 point_container.GetSize() - 1 : 0 );
   
   var suspend_info = link.SuspendObject();
   link_point.ConstrainObject( point );
   link.ReleaseObject( suspend_info );

   // Store the point name for save/load
   var point_name = point.GetSResource( "Name" );
   if( point_name == null || point_name.length == 0 )
     point_name = "Point";
   
   var link_data = GetData( link );
   if( last_point )
     link_data.end_point_name = point_name;
   else
     link_data.start_point_name = point_name;
}

/////////////////////////////////////////////////////////////////////////
// Positions the arc's middle point if it's not explicitly defined.
/////////////////////////////////////////////////////////////////////////
function SetArcMiddlePoint( link )
{
   var link_info = GetCPContainer( link );
   var point_container = link_info.glg_object;
   var edge_type = link_info.type;
   
   if( edge_type != GLG.GlgObjectType.ARC )
     return;
   
   // Offset the arc's middle point if wasn't set.
   var start_point = point_container.GetElement( 0 );
   var middle_point = point_container.GetElement( 1 );
   var end_point = point_container.GetElement( 2 );
   
   var pt1 = start_point.GetGResource( null );
   var pt2 = end_point.GetGResource( null );
   
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
function AttachFramePoints( link )
{
   var frame = link.GetResourceObject( "Frame" );
   if( frame == null ) // Link without label and frame
     return;
      
   var link_info = GetCPContainer( link );
   var link_point_container = link_info.glg_object;

   // Always use the first segment of the link to attach the frame.
   var link_start_point = link_point_container.GetElement( 0 );
   var link_end_point = link_point_container.GetElement( 1 );
      
   var frame_point_container = frame.GetResourceObject( "CPArray" );
   var size = frame_point_container.GetSize();
   var frame_start_point = frame_point_container.GetElement( 0 );
   var frame_end_point = frame_point_container.GetElement( size - 1 );
      
   var suspend_info = link.SuspendObject();
      
   frame_start_point.ConstrainObject( link_start_point );
   frame_end_point.ConstrainObject( link_end_point );
   
   link.ReleaseObject( suspend_info );
}

/////////////////////////////////////////////////////////////////////////
// Disconnects the first or last point of the link. Returns the object 
// the link is connected to.
/////////////////////////////////////////////////////////////////////////
function UnConstrainLinkPoint( link, last_point )
{
   var link_info = GetCPContainer( link );
   
   var point_container = link_info.glg_object;
   
   var link_point =
     point_container.GetElement( last_point ?
                                 point_container.GetSize() - 1 : 0 );
   var attachment_point = link_point.GetResourceObject( "Data" );
   
   var suspend_info = link.SuspendObject();
   link_point.UnconstrainObject();
   link.ReleaseObject( suspend_info );
   
   return attachment_point;
}
   
/////////////////////////////////////////////////////////////////////////
// Detaches the first and last points of the frame.
/////////////////////////////////////////////////////////////////////////
function DetachFramePoints( link )
{
   var frame = link.GetResourceObject( "Frame" );
   if( frame == null ) // Link without label and frame
     return;
   
   var frame_point_container = frame.GetResourceObject( "CPArray" );
   var size = frame_point_container.GetSize(); 
   var frame_start_point = frame_point_container.GetElement( 0 );
   var frame_end_point = frame_point_container.GetElement( size - 1 );
      
   var suspend_info = link.SuspendObject();
      
   frame_start_point.UnconstrainObject();
   frame_end_point.UnconstrainObject();
   
   link.ReleaseObject( suspend_info );
}

/////////////////////////////////////////////////////////////////////////
// Set last point of the link (dragging).
/////////////////////////////////////////////////////////////////////////
function SetLastPoint( link, cursor_pos_obj, offset, arc_middle_point )
{
   var link_info = GetCPContainer( link );
   var point_container = link_info.glg_object;
   
   var cursor_pos = GetPointFromObj( cursor_pos_obj );
   
   /* Offset the point: used to offset the arc's last point from the 
      middle one while dragging.
   */
   if( offset )
   {
      cursor_pos.x += 10.0;
      cursor_pos.y += 10.0;
   }
   
   var point;
   if( arc_middle_point )
     // Setting the middle point of an arc.
     point = point_container.GetElement( 1 );
   else
     // Setting the last point.
     point = point_container.GetElement( point_container.GetSize() - 1 );
   
   DrawingArea.ScreenToWorld( true, cursor_pos, world_coord );
   point.SetGResourceFromPoint( null, world_coord );
}

/////////////////////////////////////////////////////////////////////////
function AddLinkPoints( link, num_points )
{
   var link_info = GetCPContainer( link );
   if( link_info.type == GLG.GlgObjectType.ARC )
     return; // Arc connectors have fixed number of points: don't add.
   
   var point_container = link_info.glg_object;
   
   var point = point_container.GetElement( 0 );
   
   var suspend_info = link.SuspendObject();
   for( var i=0; i<num_points; ++i )
   {
      var add_point = point.CloneObject( GLG.GlgCloneType.FULL_CLONE );
      point_container.AddObjectToBottom( add_point );
   }
   link.ReleaseObject( suspend_info );
}

/////////////////////////////////////////////////////////////////////////
// Set the direction of the recta-linera connector depending on the 
// direction of the first mouse move.
/////////////////////////////////////////////////////////////////////////
function SetEdgeDirection( link, start_pos_obj, end_pos_obj )
{
   var direction;
   
   var link_info = GetCPContainer( link );
   var edge_type = link_info.type;
   
   if( edge_type == GLG.GlgObjectType.ARC || edge_type == 0 )
     return;      // Arc or polygon
   
   var start_pos = GetPointFromObj( start_pos_obj );
   var end_pos = GetPointFromObj( end_pos_obj );

   if( Math.abs( start_pos.x - end_pos.x ) > 
       Math.abs( start_pos.y - end_pos.y ) )
     direction = GLG.GlgOrientationType.HORIZONTAL;
   else
     direction = GLG.GlgOrientationType.VERTICAL;
   
   link.SetDResource( "EdgeDirection", direction );
   
   var link_data = GetData( link );
   link_data.link_direction = direction;
}

////////////////////////////////////////////////////////////////////////
function AddLinkObject( link_type, link_data )
{     
   var link;
   
   if( link_data == null )    // Creating a new link interactively
   {	 
      link_data = new GlgLinkData();
      link_data.link_type = link_type;
      
      link = CreateLink( link_data );
      
      // Store color
      link_data.link_color = link.GetGResource( "EdgeColor" );
      
      /* Don't add link data to the link list or store points: 
         will be done when finished creating the link.
      */
   }
   else  // Creating a link from data on load.
   {
      link = CreateLink( link_data );
      
      // Set color
      link.SetGResourceFromPoint( "EdgeColor", link_data.link_color );
      
      // Enable arrow type if defined 
      var arrow_type = link.GetResourceObject( "ArrowType" );
      if( arrow_type != null )
        arrow_type.SetDResource( null, GLG.GlgArrowType.MIDDLE_FILL_ARROW );
      
      // Restore connector direction if recta-linear
      var direction = link.GetResourceObject( "EdgeDirection" );
      if( direction != null )
        direction.SetDResource( null, link_data.link_direction );
      
      // Constrain end points to start and end nodes 
      var start_node = link_data.getStartNode();
      if( start_node != null )
      {
         var node1 = start_node.graphics;
         var point1 = node1.GetResourceObject( link_data.start_point_name  );
         ConstrainLinkPoint( link, point1, false ); // First point
      }
      
      var end_node = link_data.getEndNode();
      if( end_node != null )
      {
         var node2 = end_node.graphics;         
         var point2 = node2.GetResourceObject( link_data.end_point_name );         
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
   var group = DrawingArea.GetResourceObject( "ObjectGroup" );
   group.AddObjectToTop( link );
   
   return link;
}

////////////////////////////////////////////////////////////////////////
// Set the object size and position.
////////////////////////////////////////////////////////////////////////
function PlaceObject( node, pos, coord_type, world_coord )
{
   /* World coordinates of the node are returned to be stored in the node's
      data structure.
   */
   if( coord_type == GLG.GlgCoordType.SCREEN_COORD ) 
     DrawingArea.ScreenToWorld( true, pos, world_coord );
   else
     world_coord.CopyFrom( pos );
   
   var type = Math.trunc( node.GetDResource( "Type" ) );
   if( type == GLG.GlgObjectType.REFERENCE )
   {
      // Reference: can use its point to position it.
      node.SetGResourceFromPoint( "Point", world_coord );
      
      if( IconScale != 1.0 )   // Change node size if required.
        // Scale object around the origin, which is now located at pos.
        node.ScaleObject( coord_type, pos, IconScale, IconScale, 1.0 );
   }
   else
   {
      // Arbitrary object: move its box's center to the cursor position.
      node.PositionObject( coord_type, 
                           ( GLG.GlgAnchoringType.HCENTER |
                             GLG.GlgAnchoringType.VCENTER ),
                           pos.x, pos.y, pos.z );

      if( IconScale != 1.0 )   // Change node size if required.
        // Scale object around the center of it's bounding box.
        node.ScaleObject( coord_type, null, IconScale, IconScale, 1.0 );
   }
}
      
////////////////////////////////////////////////////////////////////////
// Get the link's control points container based on the link type.   
////////////////////////////////////////////////////////////////////////
function GetCPContainer( link )
{
   var link_type = Math.trunc( link.GetDResource( "Type" ) );

   switch( link_type )
   {
    case GLG.GlgObjectType.POLYGON:
      return { glg_object : link, type : 0 };

    case GLG.GlgObjectType.GROUP: // Group containing a polygon with a label
      return GetCPContainer( link.GetResourceObject( "Link" ) );

    case GLG.GlgObjectType.CONNECTOR:
      var type = Math.trunc( link.GetDResource( "EdgeType" ) );
      return { glg_object : link, type : type };

    default: SetError( "Invalid link type." ); return null;
   }
}

////////////////////////////////////////////////////////////////////////
// Determines what node or link the object belongs to and returns it. 
// Also returns type of the object: NODE or LINK.
////////////////////////////////////////////////////////////////////////
function GetSelectedObject( glg_object )
{
   while( glg_object != null )
   {
      // Check if the object has IconType.
      if( glg_object.HasResourceObject( "IconType" ) )
      {
         var type_string = glg_object.GetSResource( "IconType" );
         if( type_string == "Link" )
           return { glg_object : glg_object, type : LINK };	   
         else if( type_string == "Node" )
           return { glg_object : glg_object, type : NODE };
      }

      glg_object = glg_object.GetParent();
   }

   // No node/link parent found - no selection.
   return { glg_object : null, type : NO_OBJ };
}

////////////////////////////////////////////////////////////////////////
// Returns an array of all attachment points, i.e. the points whose 
// names start with the name_prefix.
////////////////////////////////////////////////////////////////////////
function GetAttachmentPoints( sel_object, name_prefix )
{
   var pt_array = sel_object.CreatePointArray( 0 );
   if( pt_array == null )
     return null;
   
   var size = pt_array.GetSize();
   var attachment_pt_array = 
     GLG.CreateObject( GLG.GlgObjectType.ARRAY,
                       GLG.GlgContainerType.GLG_OBJECT, 0, 0, null );
   
   // Add points that start with the name_prefix to attachment_pt_array.
   for( var i=0; i<size; ++i )
   {
      var point = pt_array.GetElement( i );
      var name = point.GetSResource( "Name" );
      if( name != null && name.startsWith( name_prefix ) )
        attachment_pt_array.AddObjectToBottom( point );
   }

   if( attachment_pt_array.GetSize() == 0 )
     attachment_pt_array = null;
   
   return attachment_pt_array;
}

////////////////////////////////////////////////////////////////////////
// Checks if one of the point array's points is under the cursor.
////////////////////////////////////////////////////////////////////////
function GetSelectedPoint( pt_array, cursor_pos_obj )
{
   if( pt_array == null )
     return null;
   
   var cursor_pos = GetPointFromObj( cursor_pos_obj );
   
   var size = pt_array.GetSize();
   
   for( var i=0; i<size; ++i )
   {
      var point = pt_array.GetElement( i );
      
      // Get position in screen coords.
      var screen_pos = point.GetGResource( "XfValue" );
      if( Math.abs( cursor_pos.x - screen_pos.x ) < POINT_SELECTION_RESOLUTION &&
          Math.abs( cursor_pos.y - screen_pos.y ) < POINT_SELECTION_RESOLUTION )
        return point;
   }
   return null;
}

////////////////////////////////////////////////////////////////////////
function SetDiagram( diagram_data )
{
   var i;
   
   CurrentDiagram = new GlgDiagramData();
   var node_list = CurrentDiagram.getNodeList();
   var link_list = CurrentDiagram.getLinkList();

   for( i=0; i<diagram_data.node_list.length; ++i )
   {
      var node_data = new GlgNodeData( diagram_data.node_list[i] );      
      node_list.push( node_data );
      AddNodeAt( 0, node_data, null, GLG.GlgCoordType.PARENT_COORD );
   }
   
   for( i=0; i<diagram_data.link_list.length; ++i )
   {
      var link_data = new GlgLinkData( diagram_data.link_list[i] );
      link_list.push( link_data );
      AddLinkObject( 0, link_data );
   }
   
   Viewport.Update();
}

////////////////////////////////////////////////////////////////////////
function UnsetDiagram( diagram )
{
   SelectGlgObject( null, 0 );
   
   var node_list = diagram.getNodeList();
   var link_list = diagram.getLinkList();
   
   var group = DrawingArea.GetResourceObject( "ObjectGroup" );
   
   for( var i=0; i<node_list.length; ++i )
   {
      var node_data = node_list[i];
      if( node_data.graphics != null )
        group.DeleteThisObject( node_data.graphics );
   }
   
   for( i=0; i<link_list.length; ++i )
   {
      var link_data = link_list[i];
      if( link_data.graphics != null )
        group.DeleteThisObject( link_data.graphics );
      }
   
   CurrentDiagram = new GlgDiagramData();
   Viewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////
function FillObjectPalette( palette_name, button_name, start_index )
{
   var palette = Viewport.GetResourceObject( "ObjectPalette" );
   
   /* Find and store an empty palette button used as a template.
      Search the button at the top viewport level, since palette's
      HasResources=NO.
   */
   ButtonTemplate = Viewport.GetResourceObject( button_name + start_index );
   
   if( ButtonTemplate == null )
   {
      SetError( "Can't find palette button to copy!" );
      return;
   }
   
   // Delete the template button from the palette but keep it around.
   palette.DeleteThisObject( ButtonTemplate );
   
   // Store NumColumns info.
   NumColumns = Math.trunc( ButtonTemplate.GetDResource( "NumColumns" ) );
   
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

//////////////////////////////////////////////////////////////////////////////
// Adds object palette buttons containing all icons from an array.
// icon_array is an array of icon objects to use in the palette button.
// object_array is an array of objects to use in the drawing.
//////////////////////////////////////////////////////////////////////////////
function FillObjectPaletteFromArray( palette, button_name, start_index,
                                     icon_array, object_array, default_tooltip )
{
   /* Add all icons from the icon array to the palette using a copy of 
      the template button.
   */
   var size = icon_array.GetSize();
   var button_index = start_index;
   for( var i=0; i<size; ++i )
   { 
      var icon = icon_array.GetElement( i );
      var glg_object = object_array.GetElement( i );
      
      // Set uniform icon name to simplify selection.
      icon.SetSResource( "Name", "Icon" );
      
      // For nodes, set initial label.
      if( default_tooltip == "Node" && glg_object.HasResourceObject( "Label" ) )
      {
         var label;
         if( glg_object.HasResourceObject( "InitLabel" ) )
           label = glg_object.GetSResource( "InitLabel" );
         else
           label = "";
         
         glg_object.SetSResource( "Label/String", label );
      }
      
      // Create a button to hold the icon.
      var button = ButtonTemplate.CloneObject( GLG.GlgCloneType.STRONG_CLONE ); 

      // Set button name by appending its index as a suffix (IconButtonN).
      button.SetSResource( "Name", button_name + button_index );
      
      // Set tooltip string.
      var tooltip = icon.GetResourceObject( "TooltipString" );
      if( tooltip != null )
        // Use a custom tooltip from the icon if defined.
        button.SetResourceFromObject( "TooltipString", tooltip );
      else   // Use the supplied default tooltip.
        button.SetSResource( "TooltipString", default_tooltip );
      
      // Position the button by setting row and column indices.
      button.SetDResource( "RowIndex", Math.trunc( button_index / NumColumns ) );
      button.SetDResource( "ColumnIndex", button_index % NumColumns );
      
      /* Zoom palette icon button to scale icons displayed in it. 
         Preliminary zoom by 10 for better fitting, will be precisely 
         adjusted later. 
      */
      button.SetDResource( "Zoom", DEFAULT_ICON_ZOOM_FACTOR );
      
      button.AddObjectToBottom( icon );
      
      palette.AddObjectToBottom( button );
      ++button_index;
   }
   
   return button_index; /* Return the next start index. */
}

//////////////////////////////////////////////////////////////////////////////
// Positions node icons inside the palette buttons.
// Invoked after the drawing has been setup, which is required by 
// PositionObject().
// Parameters:
//   button_name        Base name of the palette buttons.
//   start_index        Number of buttons to skip (the select and link
//                      buttons are already in the palette).
//////////////////////////////////////////////////////////////////////////////
function SetupObjectPalette( button_name, start_index )
{
   /* Find icons in the palette template and add them to the palette,
      using a copy of the template button.
   */
   for( var i = start_index; ; ++i )
   {      
      var button = Viewport.GetResourceObject( button_name + i );
      
      if( button == null )
        return;    // No more buttons
      
      var icon = button.GetResourceObject( "Icon" );
      var type = Math.trunc( icon.GetDResource( "Type" ) );      
      
      if( type == GLG.GlgObjectType.REFERENCE )
        icon.SetGResource( "Point", 0.0, 0.0, 0.0 );  // Center position
      else
        icon.PositionObject( GLG.GlgCoordType.PARENT_COORD,
                             ( GLG.GlgAnchoringType.HCENTER |
                               GLG.GlgAnchoringType.VCENTER ),
                             0.0, 0.0, 0.0 );    // Center position
      
      var zoom_factor = GetIconZoomFactor( button, icon );
      
      // Query an additional icon scale factor if defined in the icon.
      if( icon.HasResourceObject( "IconScale" ) )
        zoom_factor *= icon.GetDResource( "IconScale" );
      
      // Zoom palette icon button to scale icons displayed in it.
      button.SetDResource( "Zoom", zoom_factor );
   }
}

//////////////////////////////////////////////////////////////////////////////
// Returns a proper zoom factor to precisely fit the icon in the button.
// Used for automatic fitting if FitIcons = true.
//////////////////////////////////////////////////////////////////////////////
function GetIconZoomFactor( button, icon )
{
   if( !FitIcons )
     return DEFAULT_ICON_ZOOM_FACTOR;

   var
     point1 = GLG.CreateGlgPoint( 0, 0, 0 ), 
     point2 = GLG.CreateGlgPoint( 0, 0, 0 );
   
   zoom_factor = button.GetDResource( "Zoom" );
   
   var box = icon.GetBox();
   button.ScreenToWorld( true, box.p1, point1 );
   button.ScreenToWorld( true, box.p2, point2 );
   
   var extent_x = Math.abs( point1.x - point2.x );
   var extent_y = Math.abs( point1.y - point2.y );
   var extent = Math.max( extent_x, extent_y );

   // Reduce garbage collection.
   GLG.ReleaseToCache( point1 );
   GLG.ReleaseToCache( point2 );
   
   /* Increase zoom so that the icon fills the percentage of the button
      defined by the ICON_FIT_FACTOR. 
   */
   zoom_factor = 2000.0 / extent * ICON_FIT_FACTOR;
   return zoom_factor;
}

//////////////////////////////////////////////////////////////////////////////
// Queries items in the palette and fills array of node or link icons.
// For each palette item, an icon is added to the icon_array, and the 
// object to be used in the drawing is added to the object_array.
// In case of connectors, the object uses only a part of the icon 
// (the connector object) without the end markers.
//////////////////////////////////////////////////////////////////////////////
function GetPaletteIcons( palette, icon_name, icon_array, object_array )
{      
   for( var i=0; ; ++i )
   {
      // Get icon[i]
      var icon = palette.GetResourceObject( icon_name + i );
      if( icon == null )
        break;
      
      /* Object to use in the drawing. In case of connectors, uses only a
         part of the icon (the connector object) without the end markers.
      */
      var glg_object = icon.GetResourceObject( "Object" );
      if( glg_object == null )
        glg_object = icon;
      
      if( !glg_object.HasResourceObject( "IconType" ) )
      {
         SetError( "Can't find IconType resource." );
         continue;
      }
      
      var type_string = glg_object.GetSResource( "IconType" );
      
      /* Using icon base name as icon type since they are the same,
         i.e. "Node" and "Node", or "Link" and "Link".
      */
      if( type_string == icon_name )
      {
         // Found an icon of requested type, add it to the array.
         icon_array.AddObjectToBottom( icon );
         object_array.AddObjectToBottom( glg_object );
         
         // Set index to match the index in the icon name, i.e. 0 for Icon0.
         glg_object.SetDResource( "Index", i );
      }
   }

   var size = icon_array.GetSize();
   if( size == 0 )
     SetError( "Can't find any icons of this type." );
   else
     console.log("Scanned " + size + " " + icon_name +  " icons");
}

////////////////////////////////////////////////////////////////////////
// Adds custom data to the graphical object
////////////////////////////////////////////////////////////////////////
function AddCustomData( glg_object, data )
{
   /* Add back-pointer from graphics to the link's data struct,
      keeping the data already attached (if any).
   */
   var custom_data = glg_object.GetResourceObject( "CustomData" );
   if( custom_data == null )
   {
      /* No custom data attached: create an extra group and attach it 
         to object as custom data.
      */
      custom_data =
        GLG.CreateObject( GLG.GlgObjectType.ARRAY,
                          GLG.GlgContainerType.GLG_OBJECT, 0, 0, null );
      glg_object.SetResourceObject( "CustomData", custom_data );
   }

   /* To allow using non-glg objects, use a group with element type
      NATIVE_OBJECT as a holder. The first element of the group will keep
      the custom data pointer (pointer to the Link or Node structure).
   */
   var holder_group =
     GLG.CreateObject( GLG.GlgObjectType.ARRAY,
                       GLG.GlgContainerType.NATIVE_OBJECT, 0, 0, null );
   holder_group.SetSResource( "Name", "PtrHolder" );
   
   holder_group.AddObjectToBottom( data );
   
   // Add it to custom data.
   custom_data.AddObjectToBottom( holder_group );
}

////////////////////////////////////////////////////////////////////////
// Get custom data attached to the graphical object
////////////////////////////////////////////////////////////////////////
function GetData( glg_object )
{
   var holder_group = glg_object.GetResourceObject( "PtrHolder" );
   return holder_group.GetElement( 0 );
}

////////////////////////////////////////////////////////////////////////
function SetRadioBox( button_name )
{
   /* Always highlight the new button: the toggle would unhighlight if 
      clicked on twice.
   */
   var button = Viewport.GetResourceObject( button_name );
   if( button != null )
     button.SetDResource( "OnState", 1.0 );
   
   // Unhighlight the previous button.
   if( LastButton != null && LastButton != button_name )
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
function DeselectButton( button_name )
{
   var button = Viewport.GetResourceObject( button_name );
   button.SetDResource( "OnState", 0.0 );
}

////////////////////////////////////////////////////////////////////////
function GetPosition( glg_object, coord ) 
{
   var type = Math.trunc( glg_object.GetDResource( "Type" ) );
   if( type == GLG.GlgObjectType.REFERENCE )
   {
      // Reference: can use its point to position it.
      coord.CopyFrom( glg_object.GetGResource( "Point" ) );
   }
   else
   {
      // Arbitrary object: convert the box's center to the world coords.
      
      // Get object center in screen coords.
      var box = glg_object.GetBox();

      var center = GLG.CreateGlgPoint( ( box.p1.x + box.p2.x ) / 2.0,
                                       ( box.p1.y + box.p2.y ) / 2.0,
                                       ( box.p1.z + box.p2.z ) / 2.0 );

      DrawingArea.ScreenToWorld( true, center, coord );
   }
}

////////////////////////////////////////////////////////////////////////
// Fills Properties dialog with the selected object data.
////////////////////////////////////////////////////////////////////////   
function FillData()
{
   var 
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
function ApplyDialogData()
{
   // Store data from the dialog fields in the object.
   var label = Viewport.GetSResource( "Dialog/DialogName/TextString" );
   var object_data = Viewport.GetSResource( "Dialog/DialogData/TextString" );
   
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
      var datasource =
        Viewport.GetSResource( "Dialog/DialogDataSource/TextString" );
      SetObjectDataSource( SelectedObject, datasource );
   }
   
   Viewport.Update();
   return true;
}

////////////////////////////////////////////////////////////////////////
function GetObjectLabel( glg_object )
{
   var data = GetData( glg_object );
   if( data instanceof GlgNodeData )
     return data.object_label;
   else
     return data.object_label;
}

////////////////////////////////////////////////////////////////////////
function SetObjectLabel( glg_object, label )
{
   // Display label in the node or link object if it has a label.
   if( glg_object.HasResourceObject( "Label" ) )
     glg_object.SetSResource( "Label/String", label );
   
   var data = GetData( glg_object );
   if( data instanceof GlgNodeData )
     data.object_label = label;
   else if( data instanceof GlgLinkData )
     data.object_label = label;
}

////////////////////////////////////////////////////////////////////////
function GetObjectData( glg_object )
{
   var data = GetData( glg_object );
   if( data instanceof GlgNodeData )
     return data.object_data;
   else
     return data.object_data;
}

////////////////////////////////////////////////////////////////////////
function SetObjectData( glg_object, object_data )
{
   var data = GetData( glg_object );
   if( data instanceof GlgNodeData )
     data.object_data = object_data;
   else
     data.object_data = object_data;
}

////////////////////////////////////////////////////////////////////////
function GetObjectDataSource( glg_object )
{
   var  data = GetData( glg_object );
   if( data instanceof GlgNodeData )
     return data.datasource;
   else
     return data.datasource;
}

////////////////////////////////////////////////////////////////////////
function SetObjectDataSource( glg_object, datasource )
{
   if( datasource != null && datasource.length == 0 )
     datasource = null;  // Substitute null for empty datasource strings.
   
   var data = GetData( glg_object );
   if( data instanceof GlgNodeData )
   {
      if( glg_object.HasResourceObject( "Value" ) )
        glg_object.SetSResource( "Value/Tag", datasource );
      
      data.datasource = datasource;
   }
   else
     data.datasource = datasource;
}

////////////////////////////////////////////////////////////////////////
function NodeConnected( node )
{
   for( var i=0; i< CurrentDiagram.getLinkList().length; ++ i )
   {
      var link_data = CurrentDiagram.getLinkList()[i];
      
      var start_node = link_data.getStartNode();
      var end_node = link_data.getEndNode();
      if( start_node != null && start_node.graphics == node ||
          end_node != null && end_node.graphics == node )
        return true;      
   }
   return false;
}

////////////////////////////////////////////////////////////////////////
function ButtonToToken( button_name )
{
   for( var i=0; ButtonTokenTable[i].name != null; ++i )
     if( button_name == ButtonTokenTable[i].name )
       return ButtonTokenTable[i].token;
   
   return IH_UNDEFINED_TOKEN;   /* 0 */
}

//////////////////////////////////////////////////////////////////////////////
// Updates all tags defined in the drawing for a process diagram.
//////////////////////////////////////////////////////////////////////////////
function UpdateProcessDiagram()
{
   if( timer == null || !ProcessDiagram )
     return;
   
   var drawing = DrawingArea;
   
   /* Since new nodes may be added or removed in the process of the diagram
      editing, get the current list of tags every time. In an application 
      that just displays the diagram without editing, the tag list may be
      obtained just once (initially) and used to subscribe to data.
      Query only unique tags.
   */
   var tag_list = drawing.CreateTagList( true );
   if( tag_list != null )
   {
      var size = tag_list.GetSize();
      for( var i=0; i<size; ++i )
      {
         var data_object = tag_list.GetElement( i );
         var tag_name = data_object.GetSResource( "Tag" );

         var new_value = GetTagValue( tag_name, data_object );
         drawing.SetDTag( tag_name, new_value, true );
      }
      
      Viewport.Update( drawing );
   }

   // Restart the update timer
   timer = setTimeout( UpdateProcessDiagram, UPDATE_INTERVAL );
}

//////////////////////////////////////////////////////////////////////////////
// Get new value based on a tag name. In a real application, the value
// is obtained from a process database. PLC or another live datasource.
// In the demo, use random data.
//////////////////////////////////////////////////////////////////////////////
function GetTagValue( tag_name, data_object )
{
   // Get the current value
   var value = data_object.GetDResource( null );
   
   // Increase it.
   var increment = GLG.Rand( 0.0, 0.1 );
   
   var direction;
   if( value == 0.0 )
     direction = 1.0;
   else if( value == 1.0 )
     direction = -1.0;
   else
     direction = GLG.Rand( -1.0, 1.0 );
   
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

//////////////////////////////////////////////////////////////////////////////
function WriteLine( line )
{
   console.log( line );
}

//////////////////////////////////////////////////////////////////////////////
// Holds node data.
//////////////////////////////////////////////////////////////////////////////
function GlgNodeData( loaded_data )
{   
   if( loaded_data == null )
   {
      this.node_type = 0;                            /* int */
      this.position = GLG.CreateGlgPoint( 0, 0, 0 ); /* GlgPoint */
      this.object_label = "";                        /* String */
      this.object_data = "";                         /* String */
      this.datasource = "";                          /* String */
   }
   else
   {
      /* Using CreateGlgPointFromPoint() instead of CopyGlgPoint().
         We are loading new diagram: loaded_data.position is obtained from JSON.
         It has x, y and z properties, but it is not a GLG point.
      */
      this.node_type = loaded_data.node_type;
      this.position = GLG.CreateGlgPointFromPoint( loaded_data.position );
      this.object_label = loaded_data.object_label;
      this.object_data = loaded_data.object_data;
      if( ProcessDiagram )
        this.datasource = loaded_data.datasource;
      else
        this.datasource = "";      
   }
   
   this.graphics = null;                             /* GlgObject */
}

//////////////////////////////////////////////////////////////////////////////
// Holds link data.
//////////////////////////////////////////////////////////////////////////////
function GlgLinkData( loaded_data )
{   
   if( loaded_data == null )
   {
      this.link_type = 0;               /* int */
      this.link_direction = 0;          /* int */
      this.link_color = null;           /* GlgPoint */
      this.start_node = null;           /* GlgNodeData */
      this.end_node = null;             /* GlgNodeData */
      this.start_point_name = null;     /* String */
      this.end_point_name = null;       /* String */
      this.object_label = "A1";         /* String */
      this.object_data = "";            /* String */
      this.point_array = [];            /* Array of GlgPoints */
      this.datasource = "";             /* String */
   }
   else
   {
      /* Using CreateGlgPointFromPoint() instead of CopyGlgPoint().
         We are loading new diagram: loaded_data's link_color and elements of 
         point_array are obtained from JSON. They have x, y and z properties,
         but it is not a GLG point.
      */
      this.link_type        = loaded_data.link_type;
      this.link_direction   = loaded_data.link_direction;
      this.link_color       =
        GLG.CreateGlgPointFromPoint( loaded_data.link_color );
      this.start_point_name = loaded_data.start_point_name;
      this.end_point_name   = loaded_data.end_point_name;
      this.object_label     = loaded_data.object_label;
      this.object_data      = loaded_data.object_data;
      this.point_array      = [];

      /* Convert saved indices to node objects. */
      var node_list = CurrentDiagram.getNodeList();
      this.start_node       = node_list[ loaded_data.start_node ];
      this.end_node         = node_list[ loaded_data.end_node ];

      /* Convert to point array elements to GlgPoint. */
      for( var i=0; i<loaded_data.point_array.length; ++i )
      {
         var glg_point =
           GLG.CreateGlgPointFromPoint( loaded_data.point_array[ i ] );
         this.point_array.push( glg_point );
      }
      
      if( ProcessDiagram )
        this.datasource = loaded_data.datasource;
      else
        this.datasource = "";      
   }
   
   this.graphics = null;                /* GlgObject */
   this.first_move = false;             /* bool */
}

GlgLinkData.prototype.getStartNode = function(){ return this.start_node };
GlgLinkData.prototype.getEndNode = function(){ return this.end_node };
GlgLinkData.prototype.setStartNode = function( node ){ this.start_node = node };
GlgLinkData.prototype.setEndNode = function( node ){ this.end_node = node };

//////////////////////////////////////////////////////////////////////////////
// Holds diagram connectivity data as node and link lists.
//////////////////////////////////////////////////////////////////////////////
function GlgDiagramData()
{
   this.node_list = [];
   this.link_list = [];
}

GlgDiagramData.prototype.getNodeList = function(){ return this.node_list };
GlgDiagramData.prototype.getLinkList = function(){ return this.link_list };

//////////////////////////////////////////////////////////////////////////////
// Custom callbacks, place application-specific code into these callbacks.
//////////////////////////////////////////////////////////////////////////////

function CustomAddObjectCB( viewport, icon, data, is_node )
{
}

function CustomSelectObjectCB( viewport, icon, data, is_node )
{
}

function CustomCutObjectCB( viewport, icon, data, is_node )
{
}

function CustomPasteObjectCB( viewport, icon, data, is_node )
{
}
