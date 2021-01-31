#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#ifdef _WINDOWS
#include "resource.h"
#pragma warning( disable : 4244 )
#endif
#include "GlgApi.h"

#ifdef _WINDOWS
#define sleep( time )    Sleep( 1000 * time )
#else
#include <X11/keysym.h>
#endif

#define POINT_SELECTION_RESOLUTION     3

/* Default scale factor for icon buttons. */
#define DEFAULT_ICON_ZOOM_FACTOR    10.

/* Percentage of the button area to use for the icon. */
#define ICON_FIT_FACTOR    0.75

#ifndef MAX
#define MAX(x,y)   ( (x) > (y) ? (x) : (y) )
#endif

#define SELECT_BUTTON_NAME  "IconButton0"

/* Number of palette buttons to skip: the first button with the "select" icon
   is already in the palette. */
#define PALETTE_START_INDEX    1

typedef enum _ObjectType
{
   NO_OBJ = 0,
   NODE,
   LINK
} ObjectType;

typedef struct GlgDiagramDataStructDef GlgDiagramDataStruct;
typedef struct GlgNodeDataStructDef GlgNodeDataStruct;
typedef struct GlgLinkDataStructDef GlgLinkDataStruct;

typedef GlgDiagramDataStruct * GlgDiagramData;
typedef GlgNodeDataStruct * GlgNodeData;
typedef GlgLinkDataStruct * GlgLinkData;

struct GlgNodeDataStructDef
{   
   GlgLong node_type;
   GlgPoint position;
   char * object_label;
   char * object_data;
   GlgObject graphics;
   char * datasource;  /* Used by process diagram to supply dynamic data. */
};

struct GlgLinkDataStructDef
{   
   GlgLong link_type;
   GlgLong link_direction;
   GlgPoint link_color;
   GlgObject graphics;
   GlgNodeData start_node;
   GlgNodeData end_node;
   char * start_point_name;
   char * end_point_name;
   char * object_label;
   char * object_data;
   GlgObject point_array;
   GlgLong first_move;
   char * datasource;    /* Used by process diagram to supply dynamic data. */
};

struct GlgDiagramDataStructDef
{
   GlgObject node_list;
   GlgObject link_list;
};

typedef enum _IHToken
{
   IH_UNDEFINED_TOKEN = 0,
   IH_ICON_SELECTED,
   IH_SAVE,
   IH_INSERT,
   IH_PRINT,
   IH_CUT,
   IH_PASTE,
   IH_EXIT,
   IH_ZOOM_IN,
   IH_ZOOM_OUT,
   IH_ZOOM_TO,
   IH_ZOOM_RESET,
   IH_PROPERTIES,
   IH_CREATION_MODE,
   IH_DIALOG_APPLY,
   IH_DIALOG_CLOSE,
   IH_DIALOG_CANCEL,
   IH_DIALOG_CONFIRM_DISCARD,
   IH_DATASOURCE_SELECT,
   IH_DATASOURCE_SELECTED,
   IH_DATASOURCE_CLOSE,
   IH_DATASOURCE_APPLY,
   IH_DATASOURCE_LIST_SELECTION,
   IH_MOUSE_PRESSED,
   IH_MOUSE_RELEASED,
   IH_MOUSE_MOVED,
   IH_MOUSE_BUTTON3,
   IH_FINISH_LINK,
   IH_TEXT_INPUT_CHANGED,
   IH_OK,
   IH_CANCEL,
   IH_ESC,
} IHToken;

typedef struct _ButtonToken
{
   char * name;
   IHToken token;
} ButtonToken;

ButtonToken ButtonTokenTable[] =
{
   "Save",              IH_SAVE,
   "Insert",            IH_INSERT,
   "Print",             IH_PRINT,
   "Cut",               IH_CUT,
   "Paste",             IH_PASTE,
   "Exit",              IH_EXIT,
   "ZoomIn",            IH_ZOOM_IN,
   "ZoomOut",           IH_ZOOM_OUT,
   "ZoomTo",            IH_ZOOM_TO,
   "ZoomReset",         IH_ZOOM_RESET,
   "Properties",        IH_PROPERTIES,
   "CreateMode",        IH_CREATION_MODE,
   "DialogApply",       IH_DIALOG_APPLY,
   "DialogClose",       IH_DIALOG_CLOSE,
   "DialogCancel",      IH_DIALOG_CANCEL,
   "DataSourceSelect",  IH_DATASOURCE_SELECT,  /* process diagrams only */
   "DataSourceClose",   IH_DATASOURCE_CLOSE,   /* process diagrams only */
   "DataSourceApply",   IH_DATASOURCE_APPLY,   /* process diagrams only */
   "OKDialogOK",        IH_OK,
   "OKDialogCancel",    IH_CANCEL,
   NULL,                0
};

GlgBoolean TraceMouseMove = False;
GlgBoolean TraceMouseRelease = False;
GlgBoolean StickyCreateMode = False;  /* If set to True, multple instances of
                                         the selected item can be added to the
                                         drawing by clicking in the drawing 
                                         area. */
GlgLong AllowUnconnectedLinks = True;
GlgObject Viewport = NULL;
GlgObject DrawingArea = NULL;
GlgObject SelectedObject = NULL;
GlgObject PointMarker = NULL;
ObjectType SelectedObjectType = NO_OBJ;
GlgObject StoredColor = NULL;
GlgObject CutBuffer = NULL;
ObjectType CutBufferType = NO_OBJ;
char * LastButton = NULL;
GlgDiagramData CurrentDiagram = NULL;
GlgDiagramData SavedDiagram = NULL;
/* Icon arrays holds node or link icons. Object arrays hold the objects
   to use in the drawing. In case of connectors, only a part of the icon 
   (the connector object) is used in the drawing, without the end markers.
   */
GlgObject NodeIconArray = NULL;
GlgObject NodeObjectArray = NULL;
GlgObject LinkIconArray = NULL;
GlgObject LinkObjectArray = NULL;
GlgObject ButtonTemplate = NULL;
GlgObject PaletteTemplate = NULL;
double
  NumRows,
  NumColumns,
  IconScale = 1.;      /* May be used to scale the icons */
GlgAppContext AppContext;
GlgBoolean ProcessDiagram = False;     /* Defines the type of the diagram. */
GlgBoolean DialogDataChanged = False;

/* If set to True, icons are automatically fit to fill the button.
   If set to False, the default zoom factor will be used. */
GlgLong FitIcons = False;

/* Used by process diagram */
GlgLong DataSourceCounter = 0;
GlgLong NumDatasources = 20;
#define UPDATE_INTERVAL   1000     /* Update once per second */

#include "diagram_proto.h"          /* Function prototypes */

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*=======================================================================
| A diagramming editor: example of using Extended API.
| The type of the diagram is selected by the first command-line argument:
|   -diagram or -process-diagram.
|
| The  AddObjectCB, SelectObjectCB, CutObjectCB and DeleteObjectCB 
| callbacks at the end of this file may be used to interface with 
| application-specific functionality.
*/
int GlgMain( int argc, char * argv[], GlgAppContext InitAppContext )
{
   GlgLong skip;
   char * full_path;
   GlgObject template_drawing;

   AppContext = GlgInit( False, InitAppContext, argc, argv );

   GlgIHInit();    /* Init interaction handlers. */

#ifndef _WINDOWS
   /* Start as a process diagram if invoked through the process_diagram 
      symbolic link on Unix. 
      */
   if( strstr( argv[0], "process_diagram" ) ||
       strstr( argv[0], "process_diagram_no_opengl" ) )
     ProcessDiagram = True;
#endif

   for( skip = 1; skip < argc; ++skip )
   {
      /* Handle options to start as either diagram or process diagram. */
      if( strcmp( argv[ skip ], "-process-diagram" ) == 0 )
        ProcessDiagram = True;
      else if( strcmp( argv[ skip ], "-diagram" ) == 0 )
        ProcessDiagram = False;
   }

   DisplayUsage();

   /* If FitIcons is set to True, icons are automatically fit to fill the 
      button. If set to False, the default zoom factor will be used. 
      */
   if( ProcessDiagram )
     FitIcons = True;
   else
     FitIcons = False;

   /* Load the main drawing. */
   full_path = GlgGetRelativePath( argv[0], ProcessDiagram ? 
                                  "process_diagram.g" : "diagram.g" );
   Viewport = GlgLoadWidgetFromFile( full_path );
   if( !Viewport )
     error( "Can't load main drawing.", True );  
   GlgFree( full_path );

   /* Load the template drawing containing icons and dialogs. */
   full_path = GlgGetRelativePath( argv[0], ProcessDiagram ? 
                                   "process_template.g" : "diagram_template.g" );
   template_drawing = GlgLoadObject( full_path );
   if( !template_drawing )
     error( "Can't load template drawing.", True );
   GlgFree( full_path );

   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( Viewport, "Point1", 0., 0., 0. );
   GlgSetGResource( Viewport, "Point2", 0., 0., 0. );
   GlgSetDResource( Viewport, "Screen/WidthHint", 850. );
   GlgSetDResource( Viewport, "Screen/HeightHint", 600. );
   
   DrawingArea = GlgGetResourceObject( Viewport, "DrawingArea" );
   if( !DrawingArea )
     error( "Can't find DrawingArea viewport.", True );

   PaletteTemplate = 
     GlgGetResourceObject( template_drawing, "PaletteTemplate" );
   if( !PaletteTemplate )
     error( "Can't find PaletteTemplate viewport.", True );
   GlgReferenceObject( PaletteTemplate );

   AddDialog( template_drawing, "Dialog", "Object Properties", 400, 0 );
   AddDialog( template_drawing, "OKDialog", NULL, 0, 0 );
   if( ProcessDiagram )
     AddDialog( template_drawing, "DataSourceDialog", NULL, 700, 200 );

   GlgDropObject( template_drawing );

   GlgAddCallback( Viewport, GLG_INPUT_CB, (GlgCallbackProc) InputCB, NULL );

   /* Add trace callback to the top viewport to be able to trace the ESC key
      pressed anywhere.
   */
   GlgAddCallback( Viewport, GLG_TRACE_CB, (GlgCallbackProc) TraceCB, NULL );

   /* Setting the window title. */
   GlgSetSResource( Viewport, "ScreenName", 
                    ( ProcessDiagram ? "GLG Process Diagram Demo" :
                      "GLG Diagram Editor Demo" ) );

   Initialize( Viewport );

   GlgSetupHierarchy( Viewport );

   /* Position node icons inside the palette buttons. */
   SetupObjectPalette( "IconButton", PALETTE_START_INDEX );

#ifdef _WINDOWS            
   /* Change icon depending on the usage: diagram or process diagram. */
   GlgLoadExeIcon( Viewport, ProcessDiagram ? IDI_ICON2 : IDI_ICON1 );
#endif

   GlgUpdate( Viewport );

   if( ProcessDiagram )
     GlgAddTimeOut( AppContext, UPDATE_INTERVAL, 
                   (GlgTimerProc) UpdateProcessDiagram, DrawingArea );

   /* Install top level interface handler. EK */
   GlgIHInstall( MainIH );

   /* Start the installed interface handler. EK */
   GlgIHStart();

   return (int) GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| Top level main interface handler. 
| Parameters:
| ih - interface handler handle
| call_event - event the handler is invoked with
| EK
*/
void MainIH( GlgIH ih, GlgCallEvent call_event )
{
   GlgIHToken token;
   GlgCallEventType event_type;
   GlgObject
     icon,
     object;
   char
     * icon_type,
     * button_name;

   /* Retrieve the event type the handler has been invoked with.
     GLG_HI_SETUP_EVENT - triggered when handler is started by GlgIHStart 
     GLG_MESSAGE_EVENT  - trigerred when handler is called by 
                          GlgIHCallCurrIHWithToken or GlgIHCallCurrIH
     GLG_CLEANUP_EVENT  - trigerred when handler is uninstalled by GlgIHUninstall
     EK
   */
   event_type = GlgIHGetType( call_event );
   switch( event_type )
   {
    case GLG_HI_SETUP_EVENT:
      break;

    case GLG_MESSAGE_EVENT:
      /* Retrieve the token from the event and nandle known tokens as needed. 
         EK 
      */
      token = GlgIHGetToken( call_event );
      switch( token )
      {
       case IH_EXIT:
         GlgIHInstall( ConfirmIH );
         GlgIHSetSParameter( GLG_IH_NEW, "title", "Confirm Exiting" );
         GlgIHSetSParameter( GLG_IH_NEW, "message", "OK to quit?" );
         GlgIHSetIParameter( GLG_IH_NEW, "requested_op", token );
         GlgIHStart();
         break;

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
         GlgSetZoom( DrawingArea, NULL, 'i', 0. );
         GlgUpdate( Viewport );
         break;

       case IH_ZOOM_OUT:
         GlgSetZoom( DrawingArea, NULL, 'o', 0. );
         GlgUpdate( Viewport );
         break;

       case IH_ZOOM_TO:
         GlgSetZoom( DrawingArea, NULL, 't', 0. );
         GlgUpdate( Viewport );
         break;

       case IH_ZOOM_RESET:
         GlgSetZoom( DrawingArea, NULL, 'n', 0. );
         GlgUpdate( Viewport );
         break;

       case IH_ICON_SELECTED:
         /* Retrieve handler parameters. 
            $selected_icon   - parameter of type GlgObject
            $selected_button - parameter of type S (string) 
            These parameters are global and are assigned in Input callback 
            InputCB.
            EK
         */
         icon = GlgIHGetOParameter( GLG_IH_GLOBAL, "$selected_icon" );
         button_name = GlgIHGetSParameter( GLG_IH_GLOBAL, "$selected_button" );
         if( !icon || !button_name )
         {
            SetError( "NULL icon or icon button name." );
            break;
         }

         /* Object to use in the drawing. In case of connectors, uses only a
            part of the icon (the connector object) without the end markers.
            */
         object = GlgGetResourceObject( icon, "Object" );
         if( !object )
           object = icon;

	 GlgGetSResource( object, "IconType", &icon_type );
	 if( !icon_type )
	 {
	    SetError( "Can't find icon type." );
	    break;
	 }

 	 if( strcmp( icon_type, "Select" ) == 0 )
         {
            SetRadioBox( SELECT_BUTTON_NAME );   /* Highlight Select button */
            SetPrompt( "" );
         }

         /* For an icon type "Link" or "Node", install a corresponding 
            Set new parameters for the newly installed handler, using
            name/value pair:
            "template" parameter is of type GlgObject and holds the icon object
            id to be added to the drawing area, either a link object or a node;
            "button_name" parameter is a string and holds the icon button name.
            
            GlgIHStart() will start the handler, triggering the handler to
            to be called with the event type GLG_HI_SETUP_EVENT. 
            The parameters assigned here are passed to the hadler function
            and can be retrieved using GlgIHSet%Parameter.
            EK
         */
         else if( strcmp( icon_type, "Link" ) == 0 )
         {
            GlgIHInstall( AddLinkIH );
            GlgIHSetOParameter( GLG_IH_NEW, "template", object );
            GlgIHSetSParameter( GLG_IH_NEW, "button_name", button_name );
            GlgIHStart();
         }
	 else if( strcmp( icon_type, "Node" ) == 0 )
         {
            GlgIHInstall( AddNodeIH );
            GlgIHSetOParameter( GLG_IH_NEW, "template", object );
            GlgIHSetSParameter( GLG_IH_NEW, "button_name", button_name );
            GlgIHStart();
         }
         GlgUpdate( Viewport );
         break;

       case IH_CREATION_MODE:
         /* Set sticky creation mode from the button. */
         SetCreateMode( False );
         break;

       case IH_MOUSE_PRESSED:
         /* Selects the object and installs MoveObjectIH to drag the object 
            with the mouse.
            "$cursor_pos" is a global parameter assigned in Trace callback
            or when the object is moved or dragged.  EK 
         */
         SelectObjectWithMouse( GlgIHGetOParameter( GLG_IH_GLOBAL, 
                                                    "$cursor_pos" ) );

         /* All tokens that originate from the TraceCB require an explicit 
            update. For tokens originating from the InputCB, update is done
            at the end of the InputCB.
         */            
         GlgUpdate( Viewport );
         break;

       case IH_ESC:
       case IH_MOUSE_BUTTON3:
         break;    /* Allow: do nothing. */

       default: 
         /* EK: New comment
            Handle unrecognized tokens: in this demo, the unrecognized
            token is passed to the special "pass-through" handler 
            EditPropertiesIH, which is used to handle the Properties dialog. 
            Properties dialog is a floating dialog that can remain open, 
            and its content is changed to show properties
            of the selected object. A "pass-through" handler is a special
            handler type allowing to handle floating dialogs.
         */

         /* EK: old comment, remove
            Pass these tokens to be processed by a pass-through 
            EditPropertiesIH for dialog that can stay open to show 
            properties while selecting different objects in the drawing.
          */

         /* Set a global flag indicating the current handler is invoked
            as a "pass-through" handler with a token passed from the
            previous handler. EK
         */
         GlgIHSetIParameter( GLG_IH_GLOBAL, "fall_through_call", True );

         /* Installs EditPropertiesIH handler, start it and invoke
            it with a given token. EK
         */ 
         GlgIHPassToken( EditPropertiesIH, token, False );

         /* Reset flag */
         GlgIHSetIParameter( GLG_IH_GLOBAL, "fall_through_call", False );

         if( !GlgIHGetIParameter( GLG_IH_GLOBAL, "token_used" ) )
           SetError( "Invalid token." );
         break;
      }
      break;

    case GLG_CLEANUP_EVENT:
      /* Invoked when the handler is uninstalled via GlgIHUninstall. EK */
      SetError( "Main ih handler should never be uninstalled." );
      break;
   }
}

/*----------------------------------------------------------------------
| Handles object selection and prepares for moving the object with 
| the mouse.
*/
void SelectObjectWithMouse( GlgObject cursor_pos_obj )
{
   GlgObject
     selection,
     sel_object;
   GlgLong 
     i,
     num_selected;
   ObjectType selection_type;

   selection = GetObjectsAtCursor( cursor_pos_obj );

   if( selection && ( num_selected = GlgGetSize( selection ) ) )
   {
      /* Some object were selected, process the selection. */
      for( i=0; i < num_selected; ++i )
      {
	 sel_object = GlgGetElement( selection, i );

	 /* Find if the object itself is a link or a node, of if it's a part
	    of a node. If it's a part of a node, get the node object ID.
         */
	 sel_object = GetSelectedObject( sel_object, &selection_type );
	 
	 if( selection_type )
	 {
	    SelectGlgObject( sel_object, selection_type );

            SelectObjectCB( sel_object, GetData( sel_object ),
                            selection_type == NODE );

	    /* Prepare for dragging the object with the mouse. 
               Install new handler MoveObjectIH. EK
            */
            GlgIHInstall( MoveObjectIH );

	    /* Store the start point using current cursor position as
               a parameter of the newly installed handler MoveObjectIH.  EK
            */
            GlgIHSetOParameter( GLG_IH_NEW, "start_point", cursor_pos_obj );

            GlgIHStart();

	    GlgDropObject( selection );
	    return;
	 }
      }
   }

   GlgDropObject( selection );

   SelectGlgObject( NULL, 0 );    /* Unselect. */
}

/*----------------------------------------------------------------------
| Handler parameters:    EK
|   start_point (G data obj)
*/
void MoveObjectIH( GlgIH ih, GlgCallEvent call_event )
{
   GlgIHToken token;
   GlgCallEventType event_type;
   GlgObject start_point_obj, cursor_pos_obj;
   void * data;

   event_type = GlgIHGetType( call_event );
   switch( event_type )
   {
    case GLG_HI_SETUP_EVENT:
      TraceMouseMove = True;
      TraceMouseRelease = True;
      break;

    case GLG_MESSAGE_EVENT:
      token = GlgIHGetToken( call_event );
      switch( token )
      {
       case IH_MOUSE_MOVED:
         data = GetData( SelectedObject );

         start_point_obj = GlgIHGetOParameter( ih, "start_point" );
         cursor_pos_obj = GlgIHGetOParameter( GLG_IH_GLOBAL, "$cursor_pos" );

         if( SelectedObjectType == NODE )
           MoveObject( SelectedObject, start_point_obj, cursor_pos_obj );
         else
           MoveLink( SelectedObject, start_point_obj, cursor_pos_obj );
         
         GlgUpdate( Viewport );

         if( SelectedObjectType == NODE )
         {
            /* Update the X and Y in the node's data struct. */
            UpdateNodePosition( SelectedObject, (GlgNodeData) data );
            
            /* Don't need to update the attached links' points, since 
               the stored positions of the first and last points are
               not used: they are constrained to nodes and positioned 
               by them. */
         }
         else   /* LINK */
         {
            GlgLinkData link_data = (GlgLinkData) data;
            if( link_data->start_node )
              UpdateNodePosition( link_data->start_node->graphics, NULL );
            if( link_data->end_node )
              UpdateNodePosition( link_data->end_node->graphics, NULL );
	 
            /* Update stored point values */
            StorePointData( link_data, SelectedObject );
         }

         /* Update the start point for the next move. */
         GlgIHChangeOParameter( ih, "start_point", cursor_pos_obj );

         GlgUpdate( Viewport );
         break;

       case IH_MOUSE_RELEASED:
         /* Uninstall the current handler on mouse release. EK  */
         GlgIHUninstall();
         break;

       default:
         /* Unrecognized token: uninstall the current handler and
            invoke the parent handler, passing call_event to it. EK
         */
         GlgIHUninstallWithEvent( call_event ); /* EK: old comment -- Pass to the parent IH. */
         break;
      }
      break;

    case GLG_CLEANUP_EVENT:
      TraceMouseMove = False;
      TraceMouseRelease = False;
      break;
   }
}

/*----------------------------------------------------------------------
| Handler parameters:   EK
|   template    (GlgObject)
|   button_name (string)
|  
| The AddNodeIH handler is triggered via GlgIHCallCurrIHWithToken from the
| Trace callback. Passed tokens: IH_MOUSE_PRESSED, IH_MOUSE_MOVED.
| EK
*/
void AddNodeIH( GlgIH ih, GlgCallEvent call_event )
{
   GlgIHToken token;
   GlgCallEventType event_type;
   GlgObject
     template, 
     new_node,
     cursor_pos_obj;
   double node_type;  

   event_type = GlgIHGetType( call_event );
   switch( event_type )
   {
    case GLG_HI_SETUP_EVENT:
      TraceMouseMove = True;
      SetRadioBox( GlgIHGetSParameter( ih, "button_name" ) );
      SetPrompt( "Position the node." );
      GlgUpdate( Viewport );      
      break;

    case GLG_MESSAGE_EVENT:
      token = GlgIHGetToken( call_event );
      switch( token )
      {
       case IH_MOUSE_PRESSED:
         /* Query node type. */  
         template = GlgIHGetOParameter( ih, "template" );
         GlgGetDResource( template, "Index", &node_type );

         cursor_pos_obj = GlgIHGetOParameter( GLG_IH_GLOBAL, "$cursor_pos" );
         new_node = AddNodeAt( (GlgLong) node_type, NULL, cursor_pos_obj, 
                               GLG_SCREEN_COORD );
         AddObjectCB( new_node, GetData( new_node ), True );

         SelectGlgObject( new_node, NODE );

         /* In StickyCreateMode, keep adding nodes at each mouse click position.
          */
         if( !StickyCreateMode )
           GlgIHUninstall();

         GlgUpdate( Viewport );
         break;

       case IH_MOUSE_MOVED:
         break;   /* Allow: do nothing. */

       default:
         /* Unrecognized token: uninstall current handler and invoke 
            the parent handler, passing the call_event to it. EK
         */
         GlgIHUninstallWithEvent( call_event ); /* EK: old..Pass to the parent IH. */
         break;
      }
      break;

    case GLG_CLEANUP_EVENT:  /* Triggered when handler is uninstalled. EK */
      TraceMouseMove = False;
      SetRadioBox( SELECT_BUTTON_NAME );   /* Highlight Select button */
      SetPrompt( "" );
      GlgUpdate( Viewport );
      break;
   }
}

/*----------------------------------------------------------------------
| Handler Parameters:   EK
|   template (obj)
|   button_name (string)
|
| The AddLinkIH handler is triggered via GlgIHCallCurrIHWithToken passing
| the following tokens:
| IH_MOUSE_PRESSED, IH_MOUSE_MOVED, IH_ESC and IH_MOUSE_BUTTON3 are 
|   passed from the Trace callback;
| IH_FINISH_LINK is passed from the AddLinkIH handler itself when the
|   user finished creating a link.
| EK
*/
void AddLinkIH( GlgIH ih, GlgCallEvent call_event )
{
   GlgIHToken token;
   GlgCallEventType event_type;
   GlgObject 
     template,
     cursor_pos_obj,
     start_point_obj,
     sel_node,
     point, 
     pt_array,
     drag_link;
   GlgLinkData link_data;
   double link_type;
   GlgLong 
     edge_type,
     middle_point_added;
   GlgBoolean first_node;

   event_type = GlgIHGetType( call_event );
   switch( event_type )
   {
    case GLG_HI_SETUP_EVENT:
      TraceMouseMove = True;
      SetRadioBox( GlgIHGetSParameter( ih, "button_name" ) );

      /* Store link type */  
      template = GlgIHGetOParameter( ih, "template" );
      GlgGetDResource( template, "Index", &link_type );
      GlgIHSetIParameter( ih, "link_type", (GlgLong) link_type );

      /* Store edge type */
      GetCPContainer( template, &edge_type );      /* Get edge type */
      GlgIHSetIParameter( ih, "edge_type", edge_type );

      GlgIHSetOParameter( ih, "drag_link", NULL );      
      /* Fall through */

    case GLG_HI_RESETUP_EVENT:  
      /* Triggered by GlgIHResetup, invoked in AddLinkIH handler itself
         allowing to create more links, in case StickyCreateMode=True. EK
      */
      GlgIHSetIParameter( ih, "first_node", True );
      GlgIHSetIParameter( ih, "middle_point_added", False );

      SetPrompt( "Select the first node or attachment point." );
      GlgUpdate( Viewport );
      break;

    case GLG_MESSAGE_EVENT:
      first_node = GlgIHGetIParameter( ih, "first_node" );
      drag_link = GlgIHGetOParameter( ih, "drag_link" );

      token = GlgIHGetToken( call_event );
      switch( token )
      {
       case IH_MOUSE_MOVED:
         cursor_pos_obj = GlgIHGetOParameter( GLG_IH_GLOBAL, "$cursor_pos" );
         StoreAttachmentPoints( cursor_pos_obj, token );

         point = GlgIHGetOParameter( ih, "attachment_point" );
         pt_array = GlgIHGetOParameter( ih, "attachment_array" );
         sel_node = GlgIHGetOParameter( ih, "attachment_node" );

         if( point )
           ShowAttachmentPoints( point, NULL, NULL, 0 );
         else if( pt_array )
         {
            ShowAttachmentPoints( NULL, pt_array, sel_node, 1 );
            GlgDropObject( pt_array );
         }
         else
           /* No point or no selection: erasing attachment points feedback. */
           EraseAttachmentPoints();

         /* Drag the link's last point. */
         if( !first_node && token == IH_MOUSE_MOVED )
         {
            link_data = (GlgLinkData) GetData( drag_link );
            
            /* First time: set link direction depending of the 
               direction of the first mouse move, then make the link visible.
            */
            if( link_data->first_move )
            {
               start_point_obj = GlgIHGetOParameter( ih, "start_point" );
               SetEdgeDirection( drag_link, start_point_obj, cursor_pos_obj );
               GlgSetDResource( drag_link, "Visibility", 1. );
               link_data->first_move = False;
            }

            SetLastPoint( drag_link, cursor_pos_obj, False, False );

            middle_point_added = GlgIHGetIParameter( ih, "middle_point_added" );
            if( !middle_point_added )
              SetArcMiddlePoint( drag_link );
         }
         GlgUpdate( Viewport );
         break;

       case IH_MOUSE_PRESSED:
         cursor_pos_obj = GlgIHGetOParameter( GLG_IH_GLOBAL, "$cursor_pos" );
         StoreAttachmentPoints( cursor_pos_obj, token );

         point = GlgIHGetOParameter( ih, "attachment_point" );
         pt_array = GlgIHGetOParameter( ih, "attachment_array" );
         sel_node = GlgIHGetOParameter( ih, "attachment_node" );

         if( point )
         {
            if( first_node )
            {	       
               GlgIHSetOParameter( ih, "first_point", point );
               
               link_type = GlgIHGetIParameter( ih, "link_type" );
               drag_link = AddLinkObject( link_type, NULL );
               GlgIHSetOParameter( ih, "drag_link", drag_link );
               
               /* First point */
               ConstrainLinkPoint( drag_link, point, False );
               AttachFramePoints( drag_link );
               
               /* Wire up the start node */
               link_data = (GlgLinkData) GetData( drag_link );
               link_data->start_node = (GlgNodeData) GetData( sel_node );
               
               /* Store cursor position for setting direction based on the
                  first mouse move.
               */
               GlgIHSetOParameter( ih, "start_point", cursor_pos_obj );
               link_data->first_move = True;
               GlgSetDResource( drag_link, "Visibility", 0. );
               
               GlgIHChangeIParameter( ih, "first_node", False );
               SetPrompt( "Select the second node or additional points." );
            }
            else
            {  
               GlgObject first_point;

               first_point = GlgIHGetOptOParameter( ih, "first_point", NULL );
               if( point == first_point )
               {
                  SetError( "The two nodes are the same, "
                            "chose a different second node." );
                  break;
               }
	       
               /* Last point */
               ConstrainLinkPoint( drag_link, point, True ); 
               AttachFramePoints( drag_link );

               middle_point_added = 
                 GlgIHGetIParameter( ih, "middle_point_added" );
               if( !middle_point_added )
                 SetArcMiddlePoint( drag_link );
               
               /* Wire up the end node */
               link_data = (GlgLinkData) GetData( drag_link );
               link_data->end_node = (GlgNodeData) GetData( sel_node );
               
               FinalizeLink( drag_link );
               GlgIHChangeOParameter( ih, "drag_link", NULL );
               
               if( StickyCreateMode )
               {
                  GlgIHCallCurrIHWithToken( IH_FINISH_LINK );
                  GlgIHResetup( ih );   /* Start over to create more links. */
               }
               else
                 GlgIHUninstall();   /* Will call IH_FINISH_LINK */
            }
         }
         else if( pt_array )  /* !point */
         {
            ShowAttachmentPoints( NULL, pt_array, sel_node, 1 );
            GlgDropObject( pt_array );
         }
         else
         {
            /* No point or no selection: erase attachment point feedback and 
               add middle link points.
            */
            EraseAttachmentPoints();

            if( first_node )
            {
               /* No first point yet: can't connect. */
               SetError( "Invalid connection point!" );  
               break;
            }

            /* Add middle link point */
            AddLinkPoints( drag_link, 1 );
            GlgIHChangeIParameter( ih, "middle_point_added", True );
            
            /* Set the last point of a linear link or the middle point of 
               the arc link.
            */
            edge_type = GlgIHGetIParameter( ih, "edge_type" );            
            SetLastPoint( drag_link, cursor_pos_obj, False, 
                          edge_type == GLG_ARC );
            AttachFramePoints( drag_link );
            
            /* Set the last point of the arc link, offsetting it from the 
               middle point.
            */
            if( edge_type == GLG_ARC )
              SetLastPoint( drag_link, cursor_pos_obj, True, False );
         }
         GlgUpdate( Viewport );
         break;  

       case IH_FINISH_LINK:    /* Finish the current link. */
         drag_link = GlgIHGetOptOParameter( ih, "drag_link", NULL );
         if( drag_link )
         {
            /* Finish the last link */
            if( AllowUnconnectedLinks && FinishLink( drag_link ) )
              ;   /* Keep the link even if its second end is not connected. */
            else
            {
               /* Delete the link if its second end is not connected. */
               GlgObject group;               
               group = GlgGetResourceObject( DrawingArea, "ObjectGroup" );
               GlgDeleteThisObject( group, drag_link );
            }
            GlgIHChangeOParameter( ih, "drag_link", NULL );
         }
         EraseAttachmentPoints();   
         GlgUpdate( Viewport );
         break;

       case IH_ESC:
       case IH_MOUSE_BUTTON3:
         drag_link = GlgIHGetOptOParameter( ih, "drag_link", NULL );
         if( drag_link && StickyCreateMode )
         {
            /* Stop adding points to this link. */
            GlgIHCallCurrIHWithToken( IH_FINISH_LINK );
            GlgIHResetup( ih );   /* Start over to create more links. */
         }
         else
           /* No curr link or !StickyCreateMode: finish the current link 
              if any and stop adding links.
           */
           GlgIHUninstall();      /* Will call IH_FINISH_LINK */
         break;

       default:
         GlgIHUninstallWithEvent( call_event ); /* Pass to the parent IH. */
         break;
      }
      break;

    case GLG_CLEANUP_EVENT:
      GlgIHCallCurrIHWithToken( IH_FINISH_LINK ); /* Finish the current link */

      TraceMouseMove = False;
      SetRadioBox( SELECT_BUTTON_NAME );   /* Highlight Select button */
      SetPrompt( "" );
      GlgUpdate( Viewport );
      break;
   }
}

/*----------------------------------------------------------------------
| Finds attachment point(s) of a node under the cursor.
|
| Stores the node and either the selected attachment point or all 
| attachment points as parameters of the invoking IH: attachment_point, 
| attachment_array and attachment_node.
|
| Stores NULLs is no node is selected.
*/
void StoreAttachmentPoints( GlgObject cursor_pos_obj, GlgIHToken event_type )
{
   GlgObject
     selection,
     sel_object,
     point = NULL,
     pt_array = NULL,
     sel_node = NULL;
   ObjectType selection_type;
   GlgLong i, num_selected;
   
   selection = GetObjectsAtCursor( cursor_pos_obj );

   if( selection && ( num_selected = GlgGetSize( selection ) ) )
   {
      /* Some object were selected, process the selection to find the point 
	 to connect to */
      for( i=0; i < num_selected; ++i )
      {
	 sel_object = GlgGetElement( selection, i );

	 /* Find if the object itself is a link or a node, or if it's a part
	    of a node. If it's a part of a node, get the node object ID.
	    */
	 sel_object = GetSelectedObject( sel_object, &selection_type );

	 if( selection_type == NODE )
	 {
            double d_type;

	    GlgGetDResource( sel_object, "Type", &d_type );
	    if( d_type == GLG_REFERENCE )
	    {
	       /* Use ref's point as a connector. */
	       point = GlgGetResourceObject( sel_object, "Point" );
	    }
	    else /* Node has multiple attachment points: get an array of 
                    attachment points. */
	    {
	       pt_array = GetNodeAttachmentPoints( sel_object, "CP" );
               if( !pt_array )
                 continue;

               point = GetSelectedPoint( pt_array, cursor_pos_obj );

               /* Use attachment points array to highlight all attachment 
                  points only if no specific point is selected, and only
                  on the mouse move. On the mouse press, the specific point
                  is used to connect to.
                  */
               if( point || event_type != IH_MOUSE_MOVED )
                 SetObject( &pt_array, NULL );
	    }

            /* If found a point to connect to, stop search and use it.
               If found a node with attachment points, stop search and
               highlight the points.
               */
	    if( point || pt_array )
            {
               if( point )
                 /* If found attachment point, reset pt_array */
                 SetObject( &pt_array, NULL );

               sel_node = sel_object;
               break;
            }
	 }
	 
	 /* No point to connect to: continue searching all selected objects. */
      }

      GlgDropObject( selection );
   }

   /* Store as parameters of the invoking handler. */
   GlgIHSetOParameter( GLG_IH_CURR, "attachment_point", point );
   GlgIHSetOParameter( GLG_IH_CURR, "attachment_array", pt_array );
   GlgIHSetOParameter( GLG_IH_CURR, "attachment_node", sel_node );
}

/*----------------------------------------------------------------------
| 
*/
void EditPropertiesIH( GlgIH ih, GlgCallEvent call_event )
{
   GlgIHToken token;
   GlgCallEventType event_type;
   char * string;

   event_type = GlgIHGetType( call_event );
   switch( event_type )
   {
    case GLG_HI_SETUP_EVENT:
      break;

    case GLG_MESSAGE_EVENT:
      GlgIHSetIParameter( GLG_IH_GLOBAL, "token_used", True );

      token = GlgIHGetToken( call_event );
      switch( token )
      {
       case IH_TEXT_INPUT_CHANGED:
         if( !SelectedObject )
           break;

         DialogDataChanged = True;
         break;

       case IH_PROPERTIES:
         FillData();
         GlgSetDResource( Viewport, "Dialog/Visibility", 1. ); 
         GlgUpdate( Viewport );
         break;

       case IH_DATASOURCE_SELECT:   /* process diagram only */
         if( !SelectedObject )
         {
            SetPrompt( "Select an object in the drawing first." );
            GlgBell( Viewport );
            GlgUpdate( Viewport );
            break;
         }

         /* Returns with IH_DATASOURCE_SELECTED and $rval containing selected
            datasource string. 
         */
         GlgIHInstall( GetDataSourceIH );
         GlgIHStart();
         break;

       case IH_DATASOURCE_SELECTED:   /* process diagram only */
         string = GlgIHGetSParameter( ih, "$rval" );  /* Get the selection. */
         GlgSetSResource( Viewport, "Dialog/DialogDataSource/TextString",
                          string );
         DialogDataChanged = True;
         break;

       case IH_DIALOG_CANCEL:
         DialogDataChanged = False;
         FillData();
         GlgUpdate( Viewport );
         break;

       case IH_DIALOG_APPLY:
         ApplyDialogData();
         DialogDataChanged = False;
         GlgUpdate( Viewport );
         break;

       case IH_DIALOG_CLOSE:
         if( !DialogDataChanged )    /* No changes: close the dialog. */
         {
            GlgSetDResource( Viewport, "Dialog/Visibility", 0. );
            GlgUpdate( Viewport );
            break;
         }

         /* Data changed: confirm discarding changes. */

         /* Store the CLOSE action that initiated the confirmation,
            to close the data dialog when confirmed.
         */
         GlgIHSetIParameter( ih, "op", token );
         
         GlgIHCallCurrIHWithToken( IH_DIALOG_CONFIRM_DISCARD );
         break;

       case IH_DIALOG_CONFIRM_DISCARD:
         /* Returns with IH_OK with IH_CANCEL.
            All parameters are optional, except for the message parameter.
         */
         GlgIHInstall( ConfirmIH );
         GlgIHSetSParameter( GLG_IH_NEW, "title",
                             "Confirm Discarding Changes" );
         GlgIHSetSParameter( GLG_IH_NEW, "message", 
                             "Do you want to save dialog changes?" );
         GlgIHSetSParameter( GLG_IH_NEW, "ok_label", "Save" );
         GlgIHSetSParameter( GLG_IH_NEW, "cancel_label", "Discard" );
         GlgIHSetIParameter( GLG_IH_NEW, "modal_dialog", True );            
         GlgIHStart();
         break;

       case IH_OK:       /* Save changes. */
       case IH_CANCEL:   /* Discard changes. */
         GlgIHCallCurrIHWithToken( token == IH_OK ? 
                                   IH_DIALOG_APPLY : IH_DIALOG_CANCEL );
         
         /* Close the data dialog if that's what initiated the confirmation. */
         if( GlgIHGetOptIParameter( ih, "op", 0 ) == IH_DIALOG_CLOSE )
           GlgIHCallCurrIHWithToken( IH_DIALOG_CLOSE );
         break;

       case IH_ESC: 
         break;     /* Allow: do nothing. */

       default: 
         if( !DialogDataChanged )    /* No changes. */
         {
            GlgIHSetIParameter( GLG_IH_GLOBAL, "token_used", False );
            UninstallPassTroughIH( call_event );
            break;
         }

         /* Data changed: ignore the action and confirm discarding changes.
            Alternatively, data changes could be applied automatically when 
            the text field looses focus, the way it is done in the GLG editors,
            which would eliminate a need for a confirmation dialog for 
            discarding changed data.
         */

         /* Restore state of any ignored toggles. */
         RestoreToggleStateWhenDisabled( token );

         GlgIHCallCurrIHWithToken( IH_DIALOG_CONFIRM_DISCARD );
         break;
      }
      break;

    case GLG_CLEANUP_EVENT:
      DialogDataChanged = False;
      break;
   }
}

/*----------------------------------------------------------------------
|
*/
void GetDataSourceIH( GlgIH ih, GlgCallEvent call_event )
{
   GlgIHToken token;
   GlgCallEventType event_type;
   char * string;

   event_type = GlgIHGetType( call_event );
   switch( event_type )
   {
    case GLG_HI_SETUP_EVENT:
      GlgSetDResource( Viewport, "DataSourceDialog/Visibility", 1. ); 
      GlgUpdate( Viewport );
      break;

    case GLG_MESSAGE_EVENT:
      token = GlgIHGetToken( call_event );
      switch( token )
      {
       case IH_DATASOURCE_APPLY:
       case IH_DATASOURCE_LIST_SELECTION:
         GlgGetSResource( Viewport,
                          "DataSourceDialog/DSList/SelectedItem", &string );
         GlgIHUninstall();

         /* Set the return value in the parent datastore and call the parent. */
         GlgIHSetSParameter( GLG_IH_CURR, "$rval", string );
         GlgIHCallCurrIHWithToken( IH_DATASOURCE_SELECTED );         
         break;

       case IH_DATASOURCE_CLOSE:
         GlgIHUninstall();
         break;

       default: 
         GlgIHUninstallWithEvent( call_event );
         break;
      }
      break;

    case GLG_CLEANUP_EVENT:
      GlgSetDResource( Viewport, "DataSourceDialog/Visibility", 0. ); 
      GlgUpdate( Viewport );
      break;
   }
}

/*----------------------------------------------------------------------
| OK/Cancel confirmation dialog. 
|
| Parameters:
|   message
|   title (optional, default "Confirm")
|   ok_label (optional, def. "OK")
|   cancel_label (optional, def. "Cancel")
|   modal_dialog (optional, def. True)
|   allow_ESC (optional, def. True )
|   requested_op (optional, default - undefined (0) )
*/
void ConfirmIH( GlgIH ih, GlgCallEvent call_event )
{
   GlgIHToken token;
   GlgCallEventType event_type;
   GlgObject dialog;

   event_type = GlgIHGetType( call_event );
   switch( event_type )
   {
    case GLG_HI_SETUP_EVENT:
      dialog = GlgGetResourceObject( Viewport, "OKDialog" );
      GlgSetSResource( dialog, "ScreenName",
                       GlgIHGetOptSParameter( ih, "title", "Confirm" ) );
      GlgSetSResource( dialog, "OKDialogOK/LabelString",
                       GlgIHGetOptSParameter( ih, "ok_label", "OK" ) );
      GlgSetSResource( dialog, "OKDialogCancel/LabelString",
                       GlgIHGetOptSParameter( ih, "cancel_label", "Cancel" ) );
      GlgSetSResource( dialog, "DialogMessage/String", 
                       GlgIHGetSParameter( ih, "message" ) );
      GlgSetDResource( dialog, "Visibility", 1. );
      GlgUpdate( Viewport );
      break;

    case GLG_MESSAGE_EVENT:
      token = GlgIHGetToken( call_event );
      switch( token )
      {
       case IH_OK:
       case IH_CANCEL:
         if( GlgIHGetOptIParameter( ih, "requested_op", 0 ) == IH_EXIT )
         {
            if( token == IH_OK )
              exit( GLG_EXIT_OK );
            else  /* IH_CANCEL */
              GlgIHUninstall();     /* Do nothing */
            break;
         }

         GlgIHUninstallWithToken( token );   /* Pass selection to the parent. */
         break;

       case IH_EXIT:
         /* Allow to quit the application in the modal mode if Exit 
            is pressed twice.
         */
         exit( GLG_EXIT_OK );
         break;

       case IH_ESC:
         if( GlgIHGetOptIParameter( ih, "allow_ESC", 1 ) )
           GlgIHUninstall();
         break;

       default: 
         if( GlgIHGetOptIParameter( ih, "modal_dialog", True ) )
         {
            RestoreToggleStateWhenDisabled( token );
            SetPrompt( "Please select one of the choices from the dialog." );
            GlgBell( Viewport );
            GlgUpdate( Viewport );
         }
         else
           GlgIHUninstallWithEvent( call_event );
         break;
      }
      break;

    case GLG_CLEANUP_EVENT:
      GlgSetDResource( Viewport, "OKDialog/Visibility", 0. );
      GlgUpdate( Viewport );
      break;
   }
}

/*----------------------------------------------------------------------
| Restore state of any pressed toggles ignored or disabled by the 
| confirmation dialog.
*/
void RestoreToggleStateWhenDisabled( GlgIHToken token )
{
   switch( token )
   {
    case IH_ICON_SELECTED:
      DeselectButton( GlgIHGetSParameter( GLG_IH_GLOBAL, 
                                          "$selected_button" ) );
      break;
      
    case IH_CREATION_MODE:
      SetCreateMode( True );
      break;
   }
}

/*----------------------------------------------------------------------
| 
*/
void UninstallPassTroughIH( GlgCallEvent call_event )
{
   if( GlgIHGetIParameter( GLG_IH_GLOBAL, "fall_through_call" ) )
     /* A fall-through invokation: a parent handler passed an unused event 
        to this IH for possible processing, discard The event. Passing 
        the event to the parent IH would cause infinite recursion.
     */
     GlgIHUninstall();
   else
     /* Not a pass-through invokation: the IH was not uninstalled and 
        is current. Pass an unused event to the parent handler for
        processing.
     */              
     GlgIHUninstallWithEvent( call_event );           
}

void DisplayUsage()
{
#ifndef _WINDOWS            
   printf( "Use the -process-diagram or -diagram command-line options\n" );
   printf( "    to select the type of the diagram.\n" );
#endif
}

/*----------------------------------------------------------------------
| Performs initial setup of the drawing.
*/
void Initialize( GlgObject viewport )
{
   GlgObject group;

   SetPrompt( "" );

   /* Create a color object to store original node color during node 
      selection. */
   StoredColor = 
     GlgCopyObject( GlgGetResourceObject( viewport, "FillColor" ) );

#ifndef _WINDOWS
   /* Make exit button visible. */
   GlgSetDResource( viewport, "Exit/Visibility", 1. );
#else
   /* Make exit button invisible. */
   GlgSetDResource( viewport, "Exit/Visibility", 0. );
#endif

   /* Set grid color (configuration parameter) to grey. */
   GlgSetGResource( viewport, "$config/GlgGridPolygon/EdgeColor",
		   0.632441, 0.632441, 0.632441 );

   /* Create a separate group to hold objects. */
   group = GlgCreateObject( GLG_GROUP, "ObjectGroup", NULL, NULL, NULL, NULL );
   GlgAddObjectToBottom( DrawingArea, group );
   GlgDropObject( group );
   
   /* Create groups to hold nodes and links. */
   NodeIconArray = GlgCreateObject( GLG_GROUP, NULL, NULL, NULL, NULL, NULL );
   NodeObjectArray = GlgCreateObject( GLG_GROUP, NULL, NULL, NULL, NULL, NULL );
   LinkIconArray = GlgCreateObject( GLG_GROUP, NULL, NULL, NULL, NULL, NULL );
   LinkObjectArray = GlgCreateObject( GLG_GROUP, NULL, NULL, NULL, NULL, NULL );

   /* Scan palette template and extract icon and link objects, adding them
      to the buttons in the object palette. */
   GetPaletteIcons( PaletteTemplate, "Node", 
                   NodeIconArray, NodeObjectArray );
   GetPaletteIcons( PaletteTemplate, "Link", 
                   LinkIconArray, LinkObjectArray );
   
   FillObjectPalette( "ObjectPalette", "IconButton", PALETTE_START_INDEX );

   SetRadioBox( SELECT_BUTTON_NAME );   /* Highlight Select button */

   /* Set initial sticky creation mode from the button state in the drawing. */
   SetCreateMode( False );

   CurrentDiagram = CreateDiagramData();
}

/*----------------------------------------------------------------------
| Sets create mode based on the state of the CreateMode button.
*/
void SetCreateMode( GlgBoolean set_button )
{
   if( !set_button )   /* Set StickyCreateMode from the button */
   {
      double create_mode;

      GlgGetDResource( Viewport, "CreateMode/OnState", &create_mode );
      StickyCreateMode = ( create_mode != 0. );
   }
   else   /* Restore button state from StickyCreateMode. */
     GlgSetDResource( Viewport,
                      "CreateMode/OnState", (double) StickyCreateMode );
}

/*----------------------------------------------------------------------
| 
*/
void AddDialog( GlgObject drawing, char * dialog_name, char * title,
                GlgLong x_offset, GlgLong y_offset )
{
   GlgObject dialog;

   dialog = GlgGetResourceObject( drawing, dialog_name );
   if( !dialog )
     error( "Can't find dialog.", True );

   /* Make the dialog a top-level window, set its title and make
      it invisible on startup. */
   GlgSetDResource( dialog, "ShellType", (double) GLG_DIALOG_SHELL );
   if( title )
     GlgSetSResource( dialog, "ScreenName", title );
   GlgSetDResource( dialog, "Visibility", 0. );

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
#if 1
   /* Predefined layout widget is used as a dialog.
      Center the dialog relatively the parent window, but with a supplied 
      offset. The offset is defined in world coordinates and is relative to
      the center of the dialog's parent.
   */
   GlgSetGResource( dialog, "AnchorPointCenter", 
                    (double) x_offset, (double) y_offset, 0. );
#else

   /* Alternatively, a simple viewport could be used as a dialog window.
      In this case, the below code could be used to set the dialog's size
      in pixels as well as the pixel offset of its upper left corner 
      relatively to the parent's upper left corner.
   */
   GlgSetGResource( dialog, "Point1", 0.,0.,0. );
   GlgSetGResource( dialog, "Point2", 0.,0.,0. );
   GlgSetDResource( dialog, "Screen/WidthHint", (double) width );
   GlgSetDResource( dialog, "Screen/HeightHint", (double) height );
   GlgSetDResource( dialog, "Screen/XHint", (double) x_offset );
   GlgSetDResource( dialog, "Screen/YHint", (double) y_offset );
#endif

   GlgAddObjectToBottom( Viewport, dialog );
}

/*----------------------------------------------------------------------
|
*/
void InputCB( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{
   GlgObject message_obj;
   char
     * format,
     * origin,
     * full_origin,
     * action,
     * subaction;
   GlgIHToken token = 0;

   message_obj = (GlgObject) call_data;

   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Origin", &origin );
   GlgGetSResource( message_obj, "FullOrigin", &full_origin );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "SubAction", &subaction );

   /* Handle window closing. */
   if( strcmp( format, "Window" ) == 0 )
   {
      if( strcmp( action, "DeleteWindow" ) == 0 )
	if( strcmp( origin, "Dialog" ) == 0 )
          token = IH_DIALOG_CLOSE;
        else if( strcmp( origin, "DataSourceDialog" ) == 0 )
          token = IH_DATASOURCE_CLOSE;
        else if( strcmp( origin, "OKDialog" ) == 0 )
          token = IH_ESC;
        else
          token = IH_EXIT;
   }
   else if( strcmp( format, "Button" ) == 0 )
   {
      if( strcmp( action, "Activate" ) != 0 && 
	 strcmp( action, "ValueChanged" ) != 0 )
	return;
      
      if( strncmp( origin, "IconButton", strlen( "IconButton" ) ) == 0 )
      {
	 GlgObject button, icon;

	 button = GlgGetResourceObject( viewport, full_origin );
	 icon = GlgGetResourceObject( button, "Icon" );
	 if( !icon )
           SetError( "Can't find icon." );
         else
         {
            GlgIHSetOParameter( GLG_IH_GLOBAL, "$selected_icon", icon );
            GlgIHSetSParameter( GLG_IH_GLOBAL, "$selected_button", full_origin );
            token = IH_ICON_SELECTED;
         }
      }
      else
        token = ButtonToToken( origin );
   }
   else if( strcmp( format, "Text" ) == 0 )
   {
      if( strcmp( action, "ValueChanged" ) == 0 )
        token = IH_TEXT_INPUT_CHANGED;
   }
   else if( strcmp( format, "List" ) == 0 )
   {
      if( strcmp( action, "Select" ) == 0 &&
          strcmp( subaction, "DoubleClick" ) == 0 &&
          strcmp( origin, "DSList" ) == 0 )
        token = IH_DATASOURCE_LIST_SELECTION;
   }

   if( token )
     GlgIHCallCurrIHWithToken( token );

   GlgUpdate( Viewport );
}

#define STRING_LENGTH 32

/*----------------------------------------------------------------------
| Handles mouse operations: selection, dragging, connection point 
| highlight.
*/
void TraceCB( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{
   GlgPoint cursor_pos;
   GlgTraceCBStruct * trace_data;
   GlgIHToken event_type = IH_UNDEFINED_TOKEN;
   double zoom_to_mode;
   GlgBoolean use_coords = False;
#ifndef _WINDOWS
   char buf[ STRING_LENGTH + 1 ];
   int length;
   KeySym keysym;
   XComposeStatus status;
#endif

   trace_data = (GlgTraceCBStruct*) call_data;

   /* Platform-specific code to extract event information.
      GLG_COORD_MAPPING_ADJ is added to the cursor coordinates for precise
      pixel mapping.
   */
   cursor_pos.z = 0.;
#ifndef _WINDOWS
   switch( trace_data->event->type )
   {
    case ButtonPress:
      cursor_pos.x = trace_data->event->xbutton.x + GLG_COORD_MAPPING_ADJ;
      cursor_pos.y = trace_data->event->xbutton.y + GLG_COORD_MAPPING_ADJ;
      switch( trace_data->event->xbutton.button )
      {
       case 1: event_type = IH_MOUSE_PRESSED; break;
       case 3: event_type = IH_MOUSE_BUTTON3; break;
       default: return;   /* Report only buttons 1 and 3 */
      }
      use_coords = True;
      break;

    case ButtonRelease:
      if( trace_data->event->xbutton.button != 1 )
	return;  /* Trace only the left button releases. */
      cursor_pos.x = trace_data->event->xbutton.x + GLG_COORD_MAPPING_ADJ;
      cursor_pos.y = trace_data->event->xbutton.y + GLG_COORD_MAPPING_ADJ;
      event_type = IH_MOUSE_RELEASED;
      use_coords = True;
      break;

    case MotionNotify:
      cursor_pos.x = trace_data->event->xmotion.x + GLG_COORD_MAPPING_ADJ;
      cursor_pos.y = trace_data->event->xmotion.y + GLG_COORD_MAPPING_ADJ;
      event_type = IH_MOUSE_MOVED;
      use_coords = True;
      break;

    case KeyPress:
      length = XLookupString( &trace_data->event->xkey, 
                              buf, STRING_LENGTH, &keysym, &status );
      buf[ length ] = '\0';
      switch( keysym )
      {
       case XK_Escape:   /* ESC key */
         event_type = IH_ESC;
         break;
      }
      break;

    default: return;
   }      
#else
   switch( trace_data->event->message )
   {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
      cursor_pos.x = LOWORD( trace_data->event->lParam ) + GLG_COORD_MAPPING_ADJ;
      cursor_pos.y = HIWORD( trace_data->event->lParam ) + GLG_COORD_MAPPING_ADJ;
      switch( trace_data->event->message )
      {
       case WM_LBUTTONDOWN:
       case WM_LBUTTONDBLCLK: event_type = IH_MOUSE_PRESSED; break;
       case WM_RBUTTONDOWN:
       case WM_RBUTTONDBLCLK: event_type = IH_MOUSE_BUTTON3; break;
       case WM_LBUTTONUP:     event_type = IH_MOUSE_RELEASED; break;
       case WM_MOUSEMOVE:     event_type = IH_MOUSE_MOVED; break;
       default: return;
      }
      use_coords = True;
      break;

    case WM_CHAR:
      switch( trace_data->event->lParam )
      {
       case 0x1B:    /* ESC key */
         event_type = IH_ESC;
         break;
      }
      break;

    default: return;
   }
#endif

   switch( event_type )
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

   GlgGetDResource( DrawingArea, "ZoomToMode", &zoom_to_mode );
   if( zoom_to_mode )
     return;   /* Don't handle mouse selection in ZoomTo mode. */

   if( use_coords )
   {
      /* If nodes use viewports (buttons, gauges, etc.), need to convert 
         coordinates inside the selected viewport to the coordinates of the 
         drawing area.
      */
      if( trace_data->viewport != DrawingArea )
      {
         if( !IsChildOf( DrawingArea, trace_data->viewport ) )
           return;   /* Mouse event outside of the drawing area. */

         GlgTranslatePointOrigin( trace_data->viewport, viewport, 
                                  &cursor_pos );
      }

      GlgIHSetOParameterFromG( GLG_IH_GLOBAL, "$cursor_pos",
                               cursor_pos.x, cursor_pos.y, cursor_pos.z );
   }
   
   /* Pass to the current IH. */
   GlgIHCallCurrIHWithToken( event_type );
}

/*----------------------------------------------------------------------
| Can be used only for drawable objects, and not for data objects that 
| can be constrained.
*/
GlgBoolean IsChildOf( GlgObject grand, GlgObject object )
{
   if( !object )
     return False;

   if( object == grand )
     return True;

   return IsChildOf( grand, GlgGetParent( object, NULL ) );
}

/*----------------------------------------------------------------------
| Is always invoked when AddLinkIH is current: can use its parameters.
*/
void ShowAttachmentPoints( GlgObject point, GlgObject pt_array, 
                           GlgObject sel_node, GlgLong highlight_type )
{
   GlgObject 
     marker,
     attachment_marker,
     attachment_array,
     attachment_node;
   GlgPoint
     world_point,
     screen_point;
   GlgLong i, size;

   if( point )
   {
      attachment_array = GlgIHGetOParameter( GLG_IH_CURR, "attachment_array" );
      if( attachment_array )
        EraseAttachmentPoints();   

      /* Get the screen coords of the connector point, not the cursor
         position: may be a few pixels off. */
      GlgGetGResource( point, "XfValue", &screen_point.x,
                      &screen_point.y, &screen_point.z );
	    
      GlgScreenToWorld( DrawingArea, True, &screen_point, &world_point );
      
      attachment_marker = 
        GlgIHGetOptOParameter( GLG_IH_CURR, "attachment_marker", NULL );
      if( !attachment_marker )
      {
         attachment_marker = PointMarker;
         GlgAddObjectToBottom( DrawingArea, attachment_marker );
         GlgIHSetOParameter( GLG_IH_CURR, "attachment_marker", 
                             attachment_marker );
      }

      /* Position the feedback marker over the connector */
      GlgSetGResource( attachment_marker, "Point", 
                      world_point.x, world_point.y, world_point.z );

      GlgSetDResource( attachment_marker, "HighlightType", 
                      (double)highlight_type );
   }
   else if( pt_array )
   {
      attachment_node = GlgIHGetOParameter( GLG_IH_CURR, "attachment_node" );
      if( sel_node = attachment_node )
        return;    /* Attachment points are already shown for this node. */
      
      /* Erase previous attachment feedback if shown. */
      EraseAttachmentPoints();   

      size = GlgGetSize( pt_array );
      attachment_array =
        GlgCreateObject( GLG_GROUP, NULL, NULL, (GlgAnyType)size, NULL, NULL );
      attachment_node = sel_node;

      GlgIHSetOParameter( GLG_IH_CURR, "attachment_array", attachment_array );
      GlgIHSetOParameter( GLG_IH_CURR, "attachment_node", attachment_node );

      for( i=0; i<size; ++i )
      {
         marker = GlgCopyObject( PointMarker );

         point = GlgGetElement( pt_array, i );
         
         /* Get the screen coords of the connector point */
         GlgGetGResource( point, "XfValue", &screen_point.x,
                         &screen_point.y, &screen_point.z );
	    
         GlgScreenToWorld( DrawingArea, True, &screen_point, &world_point );
         
         /* Position the feedback marker over the connector */
         GlgSetGResource( marker, "Point", 
                         world_point.x, world_point.y, world_point.z );

         GlgSetDResource( marker, "HighlightType", (double)highlight_type );

         GlgAddObjectToBottom( attachment_array, marker );
         GlgDropObject( marker );
      }
      GlgAddObjectToBottom( DrawingArea, attachment_array );
      GlgDropObject( attachment_array );
   }
}

/*----------------------------------------------------------------------
| Erases attachment points feedback if it's shown. 
| Is always invoked when AddLinkIH is current: can use its parameters.
*/
void EraseAttachmentPoints()
{
   GlgObject attachment_marker, attachment_array;

   attachment_marker = 
     GlgIHGetOptOParameter( GLG_IH_CURR, "attachment_marker", NULL );
   if( attachment_marker )
   {
      GlgDeleteThisObject( DrawingArea, attachment_marker );
      GlgIHChangeOParameter( GLG_IH_CURR, "attachment_marker", NULL );
      return;
   }

   attachment_array = 
     GlgIHGetOptOParameter( GLG_IH_CURR, "attachment_array", NULL );
   if( attachment_array )
   {
      GlgDeleteThisObject( DrawingArea, attachment_array );
      GlgIHChangeOParameter( GLG_IH_CURR, "attachment_array", NULL );
      GlgIHChangeOParameter( GLG_IH_CURR, "attachment_node", NULL );
   }
}

/*----------------------------------------------------------------------
| 
*/
void FinalizeLink( GlgObject link )
{
   GlgObject arrow_type;
   GlgLinkData link_data;

   arrow_type = GlgGetResourceObject( link, "ArrowType" );
   if( arrow_type )
     GlgSetDResource( arrow_type, 
                     NULL, (double) GLG_MIDDLE_FILL_ARROW );
   
   /* Store points */
   link_data = (GlgLinkData) GetData( link );
   StorePointData( link_data, link );
   
   /* Add link data to the link list */
   GlgAddObjectToBottom( CurrentDiagram->link_list, link_data );
   
   /* After storing color: changes color to select */
   SelectGlgObject( link, LINK );
   
   AddObjectCB( link, GetData( link ), False );	
}

/*----------------------------------------------------------------------
| Stores point coordinates in the link data structure as a vector.
*/
void StorePointData( GlgLinkData link_data, GlgObject link )
{
   GlgObject
     point_container,
     point;
   GlgLong
     i,
     num_points,
     edge_type;

   point_container = GetCPContainer( link, &edge_type );      
   num_points = GlgGetSize( point_container );

   /* Always create a new array and discard the old one for simplicity
      (not much to gain any way). */
   GlgDropObject( link_data->point_array );

   link_data->point_array = 
     GlgCreateObject( GLG_GROUP, NULL,
		     NULL, (GlgAnyType)num_points, NULL, NULL );

   for( i=0; i<num_points; ++i )
   {
      point = GlgGetElement( point_container, i );
      /* Keep the actual link points. */
      GlgAddObjectToBottom( link_data->point_array, point );
   }
}
      
/*----------------------------------------------------------------------
| Restores link's middle points from the link data's stored vector.
| The first and last point's values are not used: they are constrained 
| to nodes and positioned/controlled by them.
*/
void RestorePointData( GlgLinkData link_data, GlgObject link )
{
   GlgObject
     point_container,
     point,
     saved_point;
   GlgLong
     i,
     start, end,
     num_points;
   
   /* Set middle point values */
   if( link_data->point_array )
   {
      num_points = GlgGetSize( link_data->point_array );
      point_container = GetCPContainer( link, NULL );
      
      /* Skip the first and last point if they are constrained to nodes.
	 Set only the unconnected ends and middle points. */
      start = ( link_data->start_node ? 1 : 0 );
      end = ( link_data->end_node ? num_points - 1 : num_points );

      /* Skip the first and last point: constrained to nodes.
	 Set only the middle points. */
      for( i=start; i<end; ++i )
      {
	 point = GlgGetElement( point_container, i );
	 saved_point = GlgGetElement( link_data->point_array, i );
	 GlgSetResourceFromObject( point, NULL, saved_point );
      }
   }
}

/*----------------------------------------------------------------------*/
void GetPointFromObj( GlgObject point_obj, GlgPoint * point )
{
   GlgGetGResource( point_obj, NULL, &point->x, &point->y, &point->z );
}

/*----------------------------------------------------------------------*/
void MoveObject( GlgObject object, GlgObject start_point_obj,
                 GlgObject end_point_obj )
{
   GlgPoint start_point, end_point;

   GetPointFromObj( start_point_obj, &start_point );
   GetPointFromObj( end_point_obj, &end_point );

   GlgMoveObject( object, GLG_SCREEN_COORD, &start_point, &end_point );
}

/*----------------------------------------------------------------------
| If the link is attached to nodes that use reference objects, moving the
| link moves the nodes, with no extra actions required. However, the link
| can be connected to a node with multiple attachment points which doesn't
| use reference object. Moving such a link would move just the attachment
| points, but not the nodes. To avoid this, unsconstrain the end points
| of the link not to spoil the attachment points' geometry, move the link,
| move the nodes, then constrain the link points back.
*/
void MoveLink( GlgObject link, 
               GlgObject start_point_obj, GlgObject end_point_obj )
{
   GlgObject
     point1, point2,
     start_node = NULL, 
     end_node = NULL;
   GlgLinkData link_data;
   GlgPoint start_point, end_point;
   double 
     object_type1 = GLG_REFERENCE, 
     object_type2 = GLG_REFERENCE;

   link_data = (GlgLinkData) GetData( link );

   if( link_data->start_node )
   {
      start_node = link_data->start_node->graphics;
      GlgGetDResource( start_node, "Type", &object_type1 );
   }

   if( link_data->end_node )
   {
      end_node = link_data->end_node->graphics;
      GlgGetDResource( end_node, "Type", &object_type2 );
   }

   GetPointFromObj( start_point_obj, &start_point );
   GetPointFromObj( end_point_obj, &end_point );
     
   if( object_type1 == GLG_REFERENCE && object_type2 == GLG_REFERENCE )
   {
      /* Nodes are reference objects (or null), moving the link moves nodes. */
      GlgMoveObject( link, GLG_SCREEN_COORD, &start_point, &end_point );
   }
   else   /* Nodes with multiple attachment points. */
   {
      /* Unconstrain link points */
      if( start_node )
        point1 = UnConstrainLinkPoint( link, False );
      if( end_node )
        point2 = UnConstrainLinkPoint( link, True );

      DetachFramePoints( link );
      
      /* Move the link */
      GlgMoveObject( link, GLG_SCREEN_COORD, &start_point, &end_point );
      
      /* Move start node, then reattach the link. */
      if( start_node )
      {
         GlgMoveObject( start_node, GLG_SCREEN_COORD, &start_point, &end_point );
         ConstrainLinkPoint( link, point1, False );
      }

      /* Move end node, then reattach the link. */
      if( end_node )
      {
         GlgMoveObject( end_node, GLG_SCREEN_COORD, &start_point, &end_point );
         ConstrainLinkPoint( link, point2, True );
      }

      AttachFramePoints( link );
   }
}

/*----------------------------------------------------------------------
|
*/
void UpdateNodePosition( GlgObject node, GlgNodeData node_data )
{
   GlgPoint world_coord;

   if( !node_data )
     node_data = (GlgNodeData) GetData( node );

   GetPosition( node, &world_coord );
   node_data->position.x = world_coord.x;
   node_data->position.y = world_coord.y;
}

/*----------------------------------------------------------------------
| 
*/
GlgObject GetObjectsAtCursor( GlgObject cursor_pos_obj )
{
   GlgRectangle select_rect;
   GlgPoint cursor_pos;

#define SELECTION_RESOLUTION  5    /* Selection sensitivity in pixels */

   GetPointFromObj( cursor_pos_obj, &cursor_pos );

   /* Select all object in the vicinity of the +-SELECTION_RESOLUTION pixels
      from the actual mouse click position. */
   select_rect.p1.x = cursor_pos.x - SELECTION_RESOLUTION;
   select_rect.p1.y = cursor_pos.y - SELECTION_RESOLUTION;
   select_rect.p2.x = cursor_pos.x + SELECTION_RESOLUTION;
   select_rect.p2.y = cursor_pos.y + SELECTION_RESOLUTION;

   return GlgCreateSelection( DrawingArea, &select_rect, DrawingArea );
}

/*----------------------------------------------------------------------
|
*/
void SelectGlgObject( GlgObject object, ObjectType selected_type )
{
   static GlgObject last_color = NULL;
   char * name;

   if( object == SelectedObject )
     return;   /* No change */

   if( last_color )    /* Restore the color of previously selected node. */
   {
      GlgSetResourceFromObject( last_color, NULL, StoredColor );
      SetObject( &last_color, NULL );
   }

   SetObject( &SelectedObject, object );
   SelectedObjectType = selected_type;

   if( object )
   {
      if( GlgHasResourceObject( object, "SelectColor" ) )
      {
         /* Change color to highlight selected node or link. */
         last_color = GlgGetResourceObject( object, "SelectColor" );
         GlgReferenceObject( last_color );
         
         /* Store original color */
         GlgSetResourceFromObject( StoredColor, NULL, last_color );
         
         /* Set color to red to highlight selection. */
         GlgSetGResource( last_color, NULL, 1., 0., 0. );
      }
      name = GetObjectLabel( SelectedObject, SelectedObjectType );
   }
   else
     name = "NONE";

   /* Display selected object name at the bottom. */
   GlgSetSResource( Viewport, "SelectedObject", name );

   FillData();
}

/*----------------------------------------------------------------------
|
*/
void Cut()
{
   GlgObject
     group,
     list;
   void * data;

   if( NoSelection() )
     return;

   /* Disallow deleting a node without deleting the link first. */
   if( SelectedObjectType == NODE && NodeConnected( SelectedObject ) )
   {
      SetError( "Remove links connected to the node before removing the node!" );
      return;
   }

   group = GlgGetResourceObject( Viewport, "DrawingArea/ObjectGroup" );

   if( GlgContainsObject( group, SelectedObject ) )
   {
      /* If previous cut object is stored, destroy it's data. */
      if( CutBuffer )
      {
	 data = GetData( CutBuffer );
	 if( CutBufferType == NODE )
	   DestroyNodeData( data );
	 else
	   DestroyLinkData( data );
      }

      /* Store the node in the cut buffer. It also keeps it referenced while
	 deleting link, etc. */
      SetObject( &CutBuffer, SelectedObject );
      CutBufferType = SelectedObjectType;

      /* Delete the node */
      GlgDeleteThisObject( group, SelectedObject );

      data = GetData( SelectedObject );
      CutObjectCB( SelectedObject, data, SelectedObjectType == NODE );

      /* Delete the data from the list, but don't destroy the data. */
      if( SelectedObjectType == NODE )
	list = CurrentDiagram->node_list;
      else  /* Link */
	list = CurrentDiagram->link_list;
            
      if( !GlgDeleteThisObject( list, data ) )
	SetError( "Deleting data failed!" );
      
      SelectGlgObject( NULL, 0 );
   }
   else
     SetError( "Cut failed." ); 

   GlgUpdate( Viewport );
}

/*----------------------------------------------------------------------
|
*/
void Paste()
{
   GlgObject 
     group,
     list;
   void * data;

   if( !CutBuffer )
   {
      SetError( "Empty cut buffer, cut some object first." );
      return;
   }
	    
   group = GlgGetResourceObject( Viewport, "DrawingArea/ObjectGroup" );

   data = GetData( CutBuffer );
   PasteObjectCB( CutBuffer, data, CutBufferType == NODE );

   if( CutBufferType == NODE )
   {
      GlgAddObjectToBottom( group, CutBuffer );     /* In front */
      list = CurrentDiagram->node_list;     
   }
   else
   {
      GlgAddObjectToTop( group, CutBuffer );        /* Behind */
      list = CurrentDiagram->link_list;
   }
   GlgAddObjectToBottom( list, data );

   SelectGlgObject( CutBuffer, CutBufferType );

   /* Allow pasting just once to avoid handling the data copy */
   CutBuffer = NULL;

   GlgUpdate( Viewport );
}

/*----------------------------------------------------------------------
| Saves the diagram data. 
*/
void Save( GlgDiagramData diagram )
{
   GlgLong i;
   GlgLinkData link;

   /* Print all nodes and edges as a test */
   GlgObject node_list = diagram->node_list;
   GlgObject link_list = diagram->link_list;

   for( i=0; i<GlgGetSize( node_list ); ++i )
     printf( "Node %ld, type %ld\n", (long) i,
             (long) ((GlgNodeData) GlgGetElement( node_list, i ))->node_type );

   for( i=0; i<GlgGetSize( link_list ); ++i )
   {
      link = (GlgLinkData) GlgGetElement( link_list, i );
      printf( "Link %ld, type %ld", (long) i, (long) link->link_type );
      PrintLinkInfo( ", from node ", node_list,
                    link->start_node, link->start_point_name );
      PrintLinkInfo( " to node ", node_list,
                    link->end_node, link->end_point_name );
      printf( "\n" );
   }

   /*  Save the DiagramData class by writing out individual records, 
       or using any other custom save method. */

   /* Save the current diagram to use it as test for loading. */
   if( SavedDiagram )
     DestroyDiagramData( SavedDiagram );
   SavedDiagram = diagram;

   /* Empty the drawing area. Don't destroy the data - will be used for 
      loading test.  */
   UnsetDiagram( diagram, False );

   GlgUpdate( Viewport );
}

/*----------------------------------------------------------------------
| 
*/
void PrintLinkInfo( char * label, GlgObject node_list, GlgNodeData node, 
                    char * point_name )
{ 
   if( !node )
     printf( "%s%s", label, "null" );
   else
   {
      printf( "%s%ld", label, (long) GlgGetIndex( node_list, node ) );

      /* Print connection point info if not the default point. */
      if( strcmp( point_name, "Point" ) != 0 )
        printf( ":%s", point_name );
   }
}

/*----------------------------------------------------------------------
| Loads the diagram data and recreates the graphics from the data.
*/
void Load()
{
   /* Load the DiagramClass by reading individual records or any other 
      custom load method matching the save method used above. */

   /* In the demo, load the saved diagram and use it as a Save/Load test. */
   if( !SavedDiagram )
     SetError( "Save the diagram first." );
   else
   {
      UnsetDiagram( CurrentDiagram, True );
      
      /* Load saved diagram */
      SetDiagram( SavedDiagram );
      SavedDiagram = NULL;
   }
   GlgUpdate( Viewport );
}

/*----------------------------------------------------------------------
|
*/
void Print()
{
   /* Set Level 2 PostScript for compatibility with old printers. */
   GlgSetDResource( Viewport, "$config/GlgPSLevel", 2. );
      
   if( !GlgPrint( Viewport, "diagram.ps",
		 -900., -900., 1800., 1800., True, False ) )
   {
      GlgBell( Viewport );
      SetPrompt( "Printing failed: can't write <editor.ps> file." );
   }
   else
     SetPrompt( "Saved postscript output as <editor.ps>." );

   GlgUpdate( Viewport );
}

/*----------------------------------------------------------------------
| If AllowUnconnectedLinks=true, keep the link if it has at least two 
| points.
*/
GlgLong FinishLink( GlgObject link )
{
   GlgObject 
     point_container,
     suspend_info;
   GlgLong 
     edge_type,
     size;

   point_container = GetCPContainer( link, &edge_type );
   if( edge_type == GLG_ARC )
     return False;      /* Disconnected arc links are not allowed. */

   size = GlgGetSize( point_container );

   /* The link must have at least two points already defined, and one extra
      point that was added to drag the next point. */
   if( size < 3 )
     return False;

   /* Delete the unfinished, unconnected point. */
   suspend_info = GlgSuspendObject( link );
   GlgDeleteBottomObject( point_container );
   GlgReleaseObject( link, suspend_info );

   FinalizeLink( link );
   return True;
}

/*----------------------------------------------------------------------
|
*/
void SetPrompt( char * string )
{
   GlgSetSResource( Viewport, "Prompt/String", string );
}

/*----------------------------------------------------------------------
|
*/
void SetError( char * string )
{
   GlgError( GLG_USER_ERROR, string );

   SetPrompt( string );
}

/*----------------------------------------------------------------------
|
*/
void SetObject( GlgObject * object_ptr, GlgObject object )
{
   /* No NULL check is required for Glg Ref/Drop functions. */
   GlgDropObject( *object_ptr );
   *object_ptr = GlgReferenceObject( object );
}

/*----------------------------------------------------------------------
|
*/
void SetString( char ** string_ptr, char * string )
{
   if( *string_ptr == string )
     return;   /* Same string ptr. */

   /* No NULL check is required for Glg string functions. */
   GlgFree( *string_ptr );
   *string_ptr = GlgStrClone( string );
}

/*----------------------------------------------------------------------
|
*/
GlgLong NoSelection()
{
   if( SelectedObject )
     return False;
   else
   {
      SetError( "Select some object first." );
      return True;
   }
}

/*----------------------------------------------------------------------
|
*/
GlgObject AddNodeAt( GlgLong node_type, GlgNodeData node_data, 
                     GlgObject position_obj, GlgCoordType coord_type )
{     
   GlgObject
     new_node,
     node_list,
     group;
   GlgPoint 
     position,
     world_coord;
   double object_type;
   GlgLong store_position;   

   if( !node_data )
   {
      node_data = CreateNodeData();
      node_data->node_type = node_type;

      /* Add node data to the node list */
      node_list = CurrentDiagram->node_list;
      GlgAddObjectToBottom( node_list, node_data );

      if( ProcessDiagram )
      {
         /* Assign an arbitrary datasource initially. */
         node_data->datasource = 
           GlgCreateIndexedName( "DataSource", DataSourceCounter );
         
         ++DataSourceCounter;
         if( DataSourceCounter >= NumDatasources )
           DataSourceCounter = 0;
      }
   }

   /* Create the node based on the node type */
   new_node = CreateNode( node_data );
   GlgGetDResource( new_node, "Type", &object_type );      

   /* Make label visible and set its string */
   if( GlgHasResourceObject( new_node, "Label" ) )
   {
      GlgSetDResource( new_node, "Label/Visibility", 1. );
      GlgSetSResource( new_node, "Label/String", node_data->object_label );
   }

   /* Store datasource as a tag of the node's Value resource, if it exists. */
   if( ProcessDiagram )
   {
      GlgObject value_obj;
      char * datasource;

      value_obj = GlgGetResourceObject( new_node, "Value" );
      datasource = node_data->datasource;
      if( value_obj && datasource && *datasource )
      {         
         GlgObject tag_obj;

         tag_obj = 
           GlgCreateObject( GLG_TAG, NULL, "Value", datasource, NULL, NULL );

         GlgSetResourceObject( value_obj, "TagObject", tag_obj );
         GlgDropObject( tag_obj );
      }
   }

   /* No cursor pos: get pos from the data struct. */
   if( !position_obj )
   {
      position = node_data->position;
      store_position = False;
   }
   else
   {
      GetPointFromObj( position_obj, &position );
      store_position = True;
   }
   node_data->graphics = new_node;  /* Pointer from data struct to graphics */

   /* Add the object to the drawing first, so that it's hierarchy is setup
      for postioning it.
   */
   group = GlgGetResourceObject( DrawingArea, "ObjectGroup" );
   GlgAddObjectToBottom( group, new_node );

   /* Transform the object to set its size and position. */
   PlaceObject( new_node, &position, coord_type, &world_coord );

   if( store_position )
   {
      node_data->position.x = world_coord.x;
      node_data->position.y = world_coord.y;
   }
   return new_node;
}

/*----------------------------------------------------------------------
|
*/
GlgObject CreateNode( GlgNodeData node_data )
{
   GlgObject new_node;

   /* Get node template from the palette */
   new_node = GlgGetElement( NodeObjectArray, node_data->node_type );
	 
   /* Create a new node instance  */
   new_node = GlgCloneObject( new_node, GLG_STRONG_CLONE );

   /* Name node using an "object" prefix (used to distiguish
      nodes from links on selection). */
   GlgSetSResource( new_node, "Name", "object" );

   AddCustomData( new_node, node_data );

   if( ProcessDiagram )
   {
      /* Init label data using node's InitLabel if exists. */
      if( GlgHasResourceObject( new_node, "InitLabel" ) )
        GlgGetSResource( new_node, "InitLabel", &node_data->object_label );
   }

   return new_node;
}

/*----------------------------------------------------------------------
|
*/
GlgObject CreateLink( GlgLinkData link_data )
{
   GlgObject new_link;
   GlgLong num_points;

   /* Get link template from the palette */
   new_link = GlgGetElement( LinkObjectArray, link_data->link_type );
	 
   /* Create a new link instance */
   new_link = GlgCloneObject( new_link, GLG_STRONG_CLONE );

   /* Name link using a "link" prefix (used to distiguish
      links from nodes on selection). */
   GlgSetSResource( new_link, "Name", "link" );

   /* If point_array exists, create/add middle link points
      If not an arc, it's created with 2 points by default, 
      add ( num_points - 2 ) more. If it's an arc, AddLinkPoints
      will do nothing. */
   if( link_data->point_array &&
       ( num_points = GlgGetSize( link_data->point_array ) ) > 2 )
     AddLinkPoints( new_link, num_points - 2 );

   AddCustomData( new_link, link_data );

   return new_link;
}

/*----------------------------------------------------------------------
| Connects the first or last point of the link.
*/
void ConstrainLinkPoint( GlgObject link, GlgObject point, GlgBoolean last_point )
{
   GlgObject
     link_point,
     point_container,
     suspend_info;
   GlgLinkData link_data;
   char * point_name;
   GlgLong edge_type;

   point_container = GetCPContainer( link, &edge_type );

   link_point = 
     GlgGetElement( point_container, 
		( last_point ? GlgGetSize( point_container ) - 1 : 0 ) );
   
   suspend_info = GlgSuspendObject( link );
   GlgConstrainObject( link_point, point );
   GlgReleaseObject( link, suspend_info );

   /* Store the point name for save/load */
   GlgGetSResource( point, "Name", &point_name );
   if( !point_name || !*point_name )
     point_name = "Point";
   
   link_data = (GlgLinkData) GetData( link );
   if( last_point )
   {
      GlgFree( link_data->end_point_name );
      link_data->end_point_name = GlgStrClone( point_name );
   }
   else
   {
      GlgFree( link_data->start_point_name );
      link_data->start_point_name = GlgStrClone( point_name );
   }
}    

/*----------------------------------------------------------------------
| Positions the arc's middle point if it's not explicitly defined.
*/
void SetArcMiddlePoint( GlgObject link )
{
   GlgObject point_container;
   GlgLong edge_type;
   GlgObject start_point, middle_point, end_point;
   double x1, y1, x2, y2, z;

   point_container = GetCPContainer( link, &edge_type );
   if( edge_type != GLG_ARC )
     return;

   /* Offset the arc's middle point if wasn't set. */
   start_point = GlgGetElement( point_container, 0 );
   middle_point = GlgGetElement( point_container, 1 );
   end_point = GlgGetElement( point_container, 2 );
   
   GlgGetGResource( start_point, NULL, &x1, &y1, &z );
   GlgGetGResource( end_point, NULL, &x2, &y2, &z );
   
   /* Offset the middle point. */
   GlgSetGResource( middle_point, NULL,
		   ( x1 + x2 ) / 2. + ( y1 - y2 ? 50. : 0. ),
		   ( y1 + y2 ) / 2. + ( y1 - y2 ? 0. : 50. ), z );
}

/*----------------------------------------------------------------------
| Handles links with labels: constrains frame's points to the link's 
| points.
*/
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

   frame = GlgGetResourceObject( link, "Frame" );
   if( !frame ) /* Link without label/frame: nothing to do. */
     return;

   link_point_container = GetCPContainer( link, NULL );

   /* Always use the first segment of the link to attach the frame. */
   link_start_point = GlgGetElement( link_point_container, 0 );
   link_end_point = GlgGetElement( link_point_container, 1 );

   frame_point_container = GlgGetResourceObject( frame, "CPArray" );
   frame_start_point = GlgGetElement( frame_point_container, 0 );
   frame_end_point = GlgGetElement( frame_point_container,
				   GlgGetSize( frame_point_container ) - 1 );
   
   suspend_info = GlgSuspendObject( link );

   GlgConstrainObject( frame_start_point, link_start_point );
   GlgConstrainObject( frame_end_point, link_end_point );

   GlgReleaseObject( link, suspend_info );
}    

/*----------------------------------------------------------------------
| Disconnects the first or last point of the link. Returns the object 
| the link is connected to.
*/
GlgObject UnConstrainLinkPoint( GlgObject link, GlgBoolean last_point )
{
   GlgObject
     point_container,
     link_point,
     attachment_point,
     suspend_info;
   GlgLong edge_type;
   
   point_container = GetCPContainer( link, &edge_type );

   link_point = 
     GlgGetElement( point_container,
		   ( last_point ? GlgGetSize( point_container ) - 1 : 0 ) );
   attachment_point = GlgGetResourceObject( link_point, "Data" );

   suspend_info = GlgSuspendObject( link );
   GlgUnconstrainObject( link_point );
   GlgReleaseObject( link, suspend_info );

   return attachment_point;
}

/*----------------------------------------------------------------------
| Detaches the first and last points of the frame.
*/
void DetachFramePoints( GlgObject link )
{
   GlgObject
     frame,
     frame_point_container,
     frame_start_point,
     frame_end_point,
     suspend_info;
   
   frame = GlgGetResourceObject( link, "Frame" );
   if( !frame ) /* Link without label and frame */
     return;

   frame_point_container = GlgGetResourceObject( frame, "CPArray" );
   frame_start_point = GlgGetElement( frame_point_container, 0 );
   frame_end_point = GlgGetElement( frame_point_container,
				   GlgGetSize( frame_point_container ) - 1 );
   
   suspend_info = GlgSuspendObject( link );

   GlgUnconstrainObject( frame_start_point );
   GlgUnconstrainObject( frame_end_point );

   GlgReleaseObject( link, suspend_info );
}

/*----------------------------------------------------------------------
| Set last point of the link (dragging).
*/
void SetLastPoint( GlgObject link, GlgObject cursor_pos_obj, 
                   GlgBoolean offset, GlgBoolean arc_middle_point )
{
   GlgObject 
     point,
     point_container;
   GlgPoint 
     cursor_pos,
     world_coord;
   GlgLong edge_type;
   
   point_container = GetCPContainer( link, &edge_type );
   GetPointFromObj( cursor_pos_obj, &cursor_pos );

   /* Offset the point: used to offset the arc's last point from the middle one
      while dragging.
   */
   if( offset )
   {
      cursor_pos.x += 10.;
      cursor_pos.y += 10.;
   }

   if( arc_middle_point )
     /* Setting the middle point of an arc. */
     point = GlgGetElement( point_container, 1 );
   else
     /* Setting the last point. */
     point = GlgGetElement( point_container, GlgGetSize( point_container ) - 1 );

   GlgScreenToWorld( DrawingArea, True, &cursor_pos, &world_coord );
   GlgSetGResource( point, NULL, world_coord.x, world_coord.y, world_coord.z );
}

/*----------------------------------------------------------------------
| 
*/
void AddLinkPoints( GlgObject link, GlgLong num_points )
{
   GlgObject 
     point,
     add_point,
     point_container,
     suspend_info;
   GlgLong
     i,
     edge_type;

   point_container = GetCPContainer( link, &edge_type );

   if( edge_type == GLG_ARC )
     return; /* Arc connectors have fixed number of points: don't add. */

   point = GlgGetElement( point_container, 0 );

   suspend_info = GlgSuspendObject( link );
   for( i=0; i<num_points; ++i )
   {      
      add_point = GlgCloneObject( point, GLG_FULL_CLONE );
      GlgAddObjectToBottom( point_container, add_point );
      GlgDropObject( add_point );      
   }
   GlgReleaseObject( link, suspend_info );   
}

/*----------------------------------------------------------------------
| Set the direction of the recta-linera connector depending on the direction
| of the first mouse move.
*/
void SetEdgeDirection( GlgObject link, 
                       GlgObject start_pos_obj, GlgObject end_pos_obj )
{
   GlgLinkData link_data;
   GlgPoint start_pos, end_pos;
   GlgLong
     edge_type,
     direction;

   GetCPContainer( link, &edge_type );
   if( edge_type == 0 || edge_type == GLG_ARC )   /* Arc or polygon */
     return;

   GetPointFromObj( start_pos_obj, &start_pos );
   GetPointFromObj( end_pos_obj, &end_pos );

   if( fabs( start_pos.x - end_pos.x ) > fabs( start_pos.y - end_pos.y ) )
     direction = GLG_HORIZONTAL;
   else
     direction = GLG_VERTICAL;

   GlgSetDResource( link, "EdgeDirection", (double) direction );   

   link_data = (GlgLinkData) GetData( link );
   link_data->link_direction = direction;
}

/*----------------------------------------------------------------------
|
*/
GlgObject AddLinkObject( GlgLong link_type, GlgLinkData link_data )
{     
   GlgObject
     link,
     arrow_type,
     direction,
     node1, node2,
     point1, point2,
     group;

   if( !link_data )    /* Creating a new link interactively */
   {	 
      link_data = CreateLinkData();
      link_data->link_type = link_type;
      link = CreateLink( link_data );

      /* Store color */
      GlgGetGResource( link, "EdgeColor",
		      &link_data->link_color.x, &link_data->link_color.y,
		      &link_data->link_color.z );
		  
      /* Don't add link data to the link list or store points: 
	 will be done when finished creating the link. */
   }
   else  /* Creating a link from data on load. */
   {
      link = CreateLink( link_data );
      
      /* Set color */
      GlgSetGResource( link, "EdgeColor", link_data->link_color.x,
		      link_data->link_color.y, link_data->link_color.z );

      /* Enable arrow type if defined */
      arrow_type = GlgGetResourceObject( link, "ArrowType" );
      if( arrow_type )
	GlgSetDResource( arrow_type, NULL, (double) GLG_MIDDLE_FILL_ARROW );

      /* Restore connector direction if recta-linear */
      direction = GlgGetResourceObject( link, "EdgeDirection" );
      if( direction )
	GlgSetDResource( direction, NULL, (double) link_data->link_direction );

      /* Constrain end points to start and end nodes  */
      if( link_data->start_node )
      {
         node1 = link_data->start_node->graphics;
         point1 = GlgGetResourceObject( node1, link_data->start_point_name );
         ConstrainLinkPoint( link, point1, False ); /* First point */
      }

      if( link_data->end_node )
      {
         node2 = link_data->end_node->graphics;
         point2 = GlgGetResourceObject( node2, link_data->end_point_name );
         ConstrainLinkPoint( link, point2, True ); /* Last point */
      }

      AttachFramePoints( link );

      RestorePointData( link_data, link );
   }

   /* Display the label if it's a link with a label. */
   if( GlgHasResourceObject( link, "Label" ) )
     GlgSetSResource( link, "Label/String", link_data->object_label );

   link_data->graphics = link;     /* Pointer from data struct to graphics */

   /* Add to the top of the draw list to be behind other objects. */
   group = GlgGetResourceObject( DrawingArea, "ObjectGroup" );
   GlgAddObjectToTop( group, link );
   
   return link;
}

/*----------------------------------------------------------------------
| Set the object size and position.
*/
void PlaceObject( GlgObject node, GlgPoint * pos, GlgLong coord_type, 
                  GlgPoint * world_coord )
{
   double d_type;

   /* World coordinates of the node are returned to be stored in the node's
      data structure. */
   if( coord_type == GLG_SCREEN_COORD ) 
     GlgScreenToWorld( DrawingArea, True, pos, world_coord );
   else
     *world_coord = *pos;

   GlgGetDResource( node, "Type", &d_type );
   if( d_type == GLG_REFERENCE )
   {
      /* Reference: can use its point to position it. */
      GlgSetGResource( node, "Point", 
		      world_coord->x, world_coord->y, world_coord->z );

      if( IconScale != 1. )   /* Change node size if required. */
	/* Scale object around the origin, which is now located at pos. */
	GlgScaleObject( node, coord_type, pos, IconScale, IconScale, 1. );
   }
   else
   {
      /* Arbitrary object: move its box's center to the cursor position. */
      GlgPositionObject( node, coord_type, GLG_HCENTER | GLG_VCENTER,
			pos->x, pos->y, pos->z );

      if( IconScale != 1. )   /* Change node size if required. */
	/* Scale object around the center of it's bounding box. */
	GlgScaleObject( node, coord_type, NULL, IconScale, IconScale, 1. );
   }
}
      
/*----------------------------------------------------------------------
| Get the link's control points container based on the link type.
*/
GlgObject GetCPContainer( GlgObject link, GlgLong * edge_type )
{
   GlgObject point_container;
   double
     d_link_type,
     d_edge_type;
   
   GlgGetDResource( link, "Type", &d_link_type );
   switch( (int) d_link_type )
   {
    case GLG_POLYGON:
      point_container = link;
      if( edge_type )
	*edge_type = 0;
      break;

    case GLG_GROUP:    /* Group containing a Link with a Label */
      point_container = 
	GetCPContainer( GlgGetResourceObject( link, "Link" ), edge_type );
      break;

    case GLG_CONNECTOR:
      point_container = link;
      if( edge_type )
      {
	 GlgGetDResource( link, "EdgeType", &d_edge_type );
	 *edge_type = (GlgLong)d_edge_type;
      }
      break;

    default: SetError( "Invalid link type." ); return NULL;
   }
   return point_container;
}

/*----------------------------------------------------------------------
| Determines what node or link the object belongs to and returns it. 
| Also returns type of the object: NODE or LINK.
*/
GlgObject GetSelectedObject( GlgObject object, ObjectType * selection_type )
{
   char * type_string;

   while( object )
   {
      /* Check if the object has IconType. */
      if( GlgHasResourceObject( object, "IconType" ) )
      {
	 GlgGetSResource( object, "IconType", &type_string );
	 if( strcmp( type_string, "Link" ) == 0 )
	 {
	    if( selection_type )
	      *selection_type = LINK;
	    return object;
	 }
	 else if( strcmp( type_string, "Node" ) == 0 )
	 {
	    if( selection_type )
	      *selection_type = NODE;
	    return object;
	 }
      }

      object = GlgGetParent( object, NULL );
   }

   /* No node/link parent found - no selection. */
   if( selection_type )
    *selection_type = NO_OBJ;
   return NULL;
}

/*----------------------------------------------------------------------
| Returns an array of all attachment points, i.e. the points whose names 
| start with the name_prefix.
*/
GlgObject GetNodeAttachmentPoints( GlgObject sel_object, char * name_prefix )
{
   GlgObject
     pt_array,
     attachment_pt_array,
     point;
   char * name;
   GlgLong 
     i,
     size;

   pt_array = GlgCreatePointArray( sel_object, 0 );
   if( !pt_array )
     return NULL;
     
   size = GlgGetSize( pt_array );
   attachment_pt_array = 
     GlgCreateObject( GLG_GROUP, NULL, NULL, NULL, NULL, NULL );

   /* Add points that start with the name_prefix to attachment_pt_array. */
   for( i=0; i<size; ++i )
   {
      point = GlgGetElement( pt_array, i );
      GlgGetSResource( point, "Name", &name );
      if( name && strncmp( name, name_prefix, strlen( name_prefix ) ) == 0 )
        GlgAddObjectToBottom( attachment_pt_array, point );
   }

   if( !GlgGetSize( attachment_pt_array ) )
     SetObject( attachment_pt_array, NULL );

   GlgDropObject( pt_array );
   return attachment_pt_array;
}

/*----------------------------------------------------------------------
| Checks if one of the point array's points is under the cursor.
*/
GlgObject GetSelectedPoint( GlgObject point_array, GlgObject cursor_pos_obj )
{
   GlgObject point;
   GlgPoint cursor_pos;
   double x, y, z;
   GlgLong i, size;

   if( !point_array )
     return NULL;

   GetPointFromObj( cursor_pos_obj, &cursor_pos );

   size = GlgGetSize( point_array ) ;
   for( i=0; i<size; ++i )
   {
      point = GlgGetElement( point_array, i );

      /* Get position in screen coords. */
      GlgGetGResource( point, "XfValue", &x, &y, &z );
      if( fabs( cursor_pos.x - x ) < POINT_SELECTION_RESOLUTION &&
         fabs( cursor_pos.y - y ) < POINT_SELECTION_RESOLUTION )
        return point;	
   }
   return NULL;
}


/*----------------------------------------------------------------------
|
*/
void SetDiagram( GlgDiagramData diagram )
{
   GlgObject 
     node_list,
     link_list;
   GlgNodeData node_data;
   GlgLinkData link_data;
   GlgLong i;

   node_list = diagram->node_list;
   link_list = diagram->link_list;
      
   for( i=0; i< GlgGetSize( node_list ); ++i )
   {
      node_data = (GlgNodeData) GlgGetElement( node_list, i );
      AddNodeAt( 0, node_data, NULL, GLG_PARENT_COORD );
   }

   for( i=0; i< GlgGetSize( link_list ); ++i )
   {
      link_data = (GlgLinkData) GlgGetElement( link_list, i );
      AddLinkObject( 0, link_data );
   }

   CurrentDiagram = diagram;
   GlgUpdate( Viewport );
}

/*----------------------------------------------------------------------
|
*/
void UnsetDiagram( GlgDiagramData diagram, GlgLong destroy_data )
{
   GlgObject
     node_list,
     link_list,
     group;
   GlgNodeData node_data;
   GlgLinkData link_data;
   GlgLong i;

   SelectGlgObject( NULL, 0 );
      
   node_list = diagram->node_list;
   link_list = diagram->link_list;
      
   group = GlgGetResourceObject( DrawingArea, "ObjectGroup" );

   for( i=0; i < GlgGetSize( node_list ); ++i )
   {
      node_data = (GlgNodeData) GlgGetElement( node_list, i );
      if( node_data->graphics )
	GlgDeleteThisObject( group, node_data->graphics );
      if( destroy_data )
	DestroyNodeData( node_data );
   }

   for( i=0; i < GlgGetSize( link_list ); ++i )
   {
      link_data = (GlgLinkData) GlgGetElement( link_list, i );
      if( link_data->graphics )
	GlgDeleteThisObject( group, link_data->graphics );
      if( destroy_data )
	DestroyLinkData( link_data );
   }

   if( destroy_data )
     DestroyDiagramData( CurrentDiagram );

   CurrentDiagram = CreateDiagramData();

   GlgUpdate( Viewport );
}

/*----------------------------------------------------------------------
| Fills the object palette with buttons containing node and link icons
| from the palette template. Palette template is a convenient place to 
| edit all icons instead of placing them into the object palette buttons. 
|
| Icons named "Node0", "Node1", etc. were extracted into the NodeIconArray.
| The LinkIconArray contains icons named "Link0", "Link1", etc.
| Here, we place all node and link icons inside the object palette buttons 
| named IconButton<N>, staring with the start_index to skip the first button
| which already contains the select button. 
| The palette buttons are created by copying an empty template button.
|
| Parameters: 
|   palette_name - name of the object palette to add buttons to.
|   button_name  - base name of the object palette buttons.
|   start_index  - Number of buttons to skip (to skip the select button which
|                  is already in the palette).
*/
void FillObjectPalette( char * palette_name, char * button_name, 
                        GlgLong start_index )
{
   GlgObject palette;
   char * res_name;

   palette = GlgGetResourceObject( Viewport, "ObjectPalette" );

   /* Find and store an empty palette button used as a template.
      Search the button at the top viewport level, since palette's
      HasResources=NO. 
      */
   res_name = GlgCreateIndexedName( button_name, start_index );
   ButtonTemplate = GlgGetResourceObject( Viewport, res_name );
   GlgFree( res_name );
   
   if( !ButtonTemplate )
     error( "Can't find palette button to copy!", True );
      
   /* Delete the template button from the palette but keep it around. */
   GlgReferenceObject( ButtonTemplate );
   GlgDeleteThisObject( palette, ButtonTemplate );
   
   /* Store NumRows and NumColumns info. */
   GlgGetDResource( ButtonTemplate, "NumRows", &NumRows );
   GlgGetDResource( ButtonTemplate, "NumColumns", &NumColumns );
    
   /* Add all icons from each array, increasing the start_index. */
   start_index = 
     FillObjectPaletteFromArray( palette, button_name, start_index,
                                LinkIconArray, LinkObjectArray, "Link" );
   start_index = 
     FillObjectPaletteFromArray( palette, button_name, start_index,
                                NodeIconArray, NodeObjectArray, "Node" );

   /* Store the marker template for attachment points feedback. */
   PointMarker = GlgGetResourceObject( PaletteTemplate, "PointMarker" );
   GlgReferenceObject( PointMarker );
	    
   /* Cleanup */
   GlgDropObject( ButtonTemplate );
   ButtonTemplate = NULL;

   GlgDropObject( PaletteTemplate );
   PaletteTemplate = NULL;
   
   GlgDropObject( NodeIconArray );
   NodeIconArray = NULL;
   
   GlgDropObject( LinkIconArray );
   LinkIconArray = NULL;
}

/*----------------------------------------------------------------------
| Adds object palette buttons containing all icons from an array.
*/   
GlgLong FillObjectPaletteFromArray( palette, button_name, start_index, 
                                icon_array, object_array, default_tooltip )
     GlgObject palette;
     char * button_name;
     GlgLong start_index;
     GlgObject 
       icon_array,    /* Array of icon objects to use in the palette button */
       object_array;  /* Array of objects to use in the drawing. */
     char * default_tooltip;
{
   GlgObject
     icon,
     object,
     button,
     tooltip;
   char
     * res_name,
     * string;
   GlgLong i, size, button_index;

   /* Add all icons from the icon array to the palette using a copy of 
      the template button.
      */  
   size = GlgGetSize( icon_array );
   button_index = start_index;
   for( i=0; i<size; ++i )
   {      
      icon = GlgGetElement( icon_array, i );
      object = GlgGetElement( object_array, i );

      /* Set uniform icon name to simplify selection. */
      GlgSetSResource( icon, "Name", "Icon" );

      /* For nodes, set initial label. */
      if( strcmp( default_tooltip, "Node" ) == 0 &&
         GlgHasResourceObject( object, "Label" ) )
      {
         if( GlgHasResourceObject( object, "InitLabel" ) )
           GlgGetSResource( object, "InitLabel", &string );
         else
           string = "";

         GlgSetSResource( object, "Label/String", string );
      }

      /* Create a button to hold the icon. */
      button = GlgCloneObject( ButtonTemplate, GLG_STRONG_CLONE ); 

      /* Set button name by appending its index as a suffix (IconButtonN). */
      res_name = GlgCreateIndexedName( button_name, button_index );
      GlgSetSResource( button, "Name", res_name );
      GlgFree( res_name );

      /* Set tooltip string. */
      tooltip = GlgGetResourceObject( icon, "TooltipString" );
      if( tooltip )   /* Use a custom tooltip from the icon if defined. */ 
	GlgSetResourceFromObject( button, "TooltipString", tooltip );
      else   /* Use the supplied default tooltip. */
	GlgSetSResource( button, "TooltipString", "Node" );

      /* Position the button by setting row and column indices. */
      GlgSetDResource( button, "RowIndex",
		      (double) ( button_index / (int) NumColumns ) );
      GlgSetDResource( button, "ColumnIndex",
		      (double) ( button_index % (int) NumColumns ) );

      /* Zoom palette icon button to scale icons displayed in it. 
         Preliminary zoom by 10 for better fitting, will be precisely 
         adjusted later. 
         */
      GlgSetDResource( button, "Zoom", DEFAULT_ICON_ZOOM_FACTOR );

      GlgAddObjectToBottom( button, icon );
      GlgAddObjectToBottom( palette, button );
      GlgDropObject( button );
      ++button_index;
   }

   return button_index; /* Return the next start index. */
}

/*----------------------------------------------------------------------
| Positions node icons inside the palette buttons.
| Invoked after the drawing has been setup, which is required by 
| GlgPositionObject().
| Parameters:
|   button_name - base name of the palette buttons.
|   start_index  - Number of buttons to skip (to skip the select button which
|                  is already in the palette).
*/
void SetupObjectPalette( char * button_name, GlgLong start_index )
{
   GlgObject
     icon,
     button;
   char * res_name;
   double 
     object_type, 
     zoom_factor,
     icon_scale;
   GlgLong i;

   /* Find icons in the palette template and add them to the palette,
      using a copy of the template button. */
   for( i=start_index; ; ++i )
   {      
      res_name = GlgCreateIndexedName( button_name, i );
      button = GlgGetResourceObject( Viewport, res_name );
      GlgFree( res_name );

      if( !button )
	return;    /* No more buttons */

      icon = GlgGetResourceObject( button, "Icon" );
      GlgGetDResource( icon, "Type", &object_type );      

      if( object_type == GLG_REFERENCE )
        GlgSetGResource( icon, "Point", 0., 0., 0. );  /* Center position */
      else
	GlgPositionObject( icon, GLG_PARENT_COORD, GLG_HCENTER | GLG_VCENTER,
			  0., 0., 0. );              /* Center position */

      zoom_factor = GetIconZoomFactor( button, icon );

      /* Query an additional icon scale factor if defined in the icon. */ 
      if( GlgHasResourceObject( icon, "IconScale" ) )
      {
         GlgGetDResource( icon, "IconScale", &icon_scale ); 
         zoom_factor *= icon_scale;
      }

      /* Zoom palette icon button to scale icons displayed in it.*/
      GlgSetDResource( button, "Zoom", zoom_factor );
   }
}

/*----------------------------------------------------------------------
| Returns a proper zoom factor to precisely fit the icon in the button.
| Used for automatic fitting if FitIcons = True.
*/
double GetIconZoomFactor( GlgObject button, GlgObject icon )
{
   GlgCube * box;
   GlgPoint point1, point2;
   double
     extent_x, extent_y, extent,
     zoom_factor;

   if( FitIcons )
   {
      GlgGetDResource( button, "Zoom", &zoom_factor );
      
      box = GlgGetBoxPtr( icon );
      GlgScreenToWorld( button, True, &box->p1, &point1 );
      GlgScreenToWorld( button, True, &box->p2, &point2 );
      
      extent_x = fabs( point1.x - point2.x );
      extent_y = fabs( point1.y - point2.y );
      extent = MAX( extent_x, extent_y );
      
      /* Increase zoom so that the icon fills the percentage of the button
         defined by the ICON_FIT_FACTOR. 
         */
      zoom_factor = 2000. / extent * ICON_FIT_FACTOR;
      return zoom_factor;
   }
   else
     return DEFAULT_ICON_ZOOM_FACTOR;
}

/*----------------------------------------------------------------------
| Queries items in the palette and fills array of node or link icons.
| For each palette item, an icon is added to the icon_array, and the 
| object to be used in the drawing is added to the object_array.
| In case of connectors, the object uses only a part of the icon 
| (the connector object) without the end markers.
*/
void GetPaletteIcons( GlgObject palette, char * icon_name, GlgObject icon_array, 
                      GlgObject object_array )
                       
                           /* Icon base name, same as icon type. */
                                        
{
   GlgObject
     icon,
     object;
   char
     * res_name,
     * type_string;   
   GlgLong
     i,
     size;
   
   for( i=0; ; ++i )
   {
      /* Get icon[i] */
      res_name = GlgCreateIndexedName( icon_name, i );
      icon = GlgGetResourceObject( palette, res_name );
      GlgFree( res_name );

      if( !icon )
	break;

      /* Object to use in the drawing. In case of connectors, uses only a
         part of the icon (the connector object) without the end markers.
         */
      object = GlgGetResourceObject( icon, "Object" );
      if( !object )
        object = icon;

      if( !GlgHasResourceObject( object, "IconType" ) )
      {
         SetError( "Can't find IconType resource." );
         continue;
      }

      GlgGetSResource( object, "IconType", &type_string );

      /* Using icon base name as icon type since they are the same,
         i.e. "Node" and "Node", or "Link" and "Link".
         */
      if( strcmp( type_string, icon_name ) == 0 )
      {
	 /* Found an icon of requested type, add it to the array. */
	 GlgAddObjectToBottom( icon_array, icon );
	 GlgAddObjectToBottom( object_array, object );

         /* Set index to match the index in the icon name, i.e. 0 for Icon0. */
         GlgSetDResource( object, "Index", (double) i );
      }
   }

   size = GlgGetSize( icon_array );
   if( !size )
     SetError( "Can't find any icons of this type." );
   else
     printf( "Scanned %ld %s icons\n", (long) size, icon_name );
}

/*----------------------------------------------------------------------
| Adds custom data to the graphical object
*/
void AddCustomData( GlgObject object, void * data )
{
   GlgObject
     custom_data,
     holder_group;   

   /* Add back-pointer from graphics to the link's data struct,
      keeping the data already attached (if any).
      */
   custom_data = GlgGetResourceObject( object, "CustomData" );
   if( !custom_data )
   {
      /* No custom data attached: create an extra group and attach it 
	 to object as custom data. */
      custom_data = GlgCreateObject( GLG_GROUP, NULL, NULL, NULL, NULL, NULL );
      GlgSetResourceObject( object, "CustomData", custom_data );
      GlgDropObject( custom_data );
   }

   /* To allow using non-glg objects, use a group with element type
      GLG_LONG as a holder. The first element of the group will keep
      the custom data pointer (pointer to the Link or Node structure).
      */
   holder_group = GlgCreateObject( GLG_GROUP, "PtrHolder",
				  (GlgAnyType)GLG_LONG, NULL, NULL, NULL );
   GlgAddObjectToBottom( holder_group, data );

   /* Add it to custom data. */
   GlgAddObjectToBottom( custom_data, holder_group );
   GlgDropObject( holder_group );
}

/*----------------------------------------------------------------------
| Get custom data attached to the graphical object
*/
void * GetData( GlgObject object )
{
   GlgObject holder_group;

   holder_group = GlgGetResourceObject( object, "PtrHolder" );

   return GlgGetElement( holder_group, 0 );
}

/*----------------------------------------------------------------------
| Highlights the new button and unhighlights the previous one.
*/
void SetRadioBox( char * button_name )
{
   GlgObject button;

   if( !button_name )
     return;

   /* Always highlight the new button: the toggle would unhighlight if 
      clicked on twice. */
   button = GlgGetResourceObject( Viewport, button_name );
   GlgSetDResource( button, "OnState", 1. );

   /* Unhighlight the previous button. */
   if( LastButton && strcmp( button_name, LastButton ) != 0 )
   {
      button = GlgGetResourceObject( Viewport, LastButton );
      GlgSetDResource( button, "OnState", 0. );
   }

   SetString( &LastButton, button_name ); /* Store the last button. */
}

/*----------------------------------------------------------------------
| Deselects the button.
*/
void DeselectButton( char * button_name )
{
   GlgObject button;
   
   button = GlgGetResourceObject( Viewport, button_name );
   GlgSetDResource( button, "OnState", 0. );
}

/*----------------------------------------------------------------------
|
*/
void GetPosition( GlgObject object, GlgPoint * coord )
{
   GlgPoint center;
   GlgCube * box;
   double type;

   GlgGetDResource( object, "Type", &type );
   if( type == GLG_REFERENCE )
   {
      /* Reference: can use its point to position it. */
      GlgGetGResource( object, "Point", &coord->x, &coord->y, &coord->z );
   }
   else
   {
      /* Arbitrary object: convert the box's center to the world coords. */
      
      /* Get object center in screen coords. */
      box = GlgGetBoxPtr( object );

      center.x = ( box->p1.x + box->p2.x ) / 2.;
      center.y = ( box->p1.y + box->p2.y ) / 2.;
      center.z = ( box->p1.z + box->p2.z ) / 2.;
      
      GlgScreenToWorld( DrawingArea, True, &center, coord );
   }
}

/*----------------------------------------------------------------------
| Fills Properties dialog with the selected object data.
*/
void FillData()
{
   char
     * label,
     * object_data,
     * datasource;

   switch( SelectedObjectType )
   {
    default:
      label = "NO_OBJECT";
      object_data = "";
      datasource = "";
      break;
      
    case NODE:
    case LINK:
      label = GetObjectLabel( SelectedObject, SelectedObjectType );
      object_data = GetObjectData( SelectedObject, SelectedObjectType );

      if( ProcessDiagram )
      {
         datasource = 
           GetObjectDataSource( SelectedObject, SelectedObjectType );

         /* Substitute an empty string instead of null for display. */
         if( !datasource )
           datasource = "";
      }
      break;
   }   

   GlgSetSResource( Viewport, "Dialog/DialogName/TextString", label );
   GlgSetSResource( Viewport, "Dialog/DialogData/TextString", object_data );

   /* For process diagram also set the datasource field. */
   if( ProcessDiagram )
     GlgSetSResource( Viewport, "Dialog/DialogDataSource/TextString", 
                     datasource );
}

/*----------------------------------------------------------------------
| Stores data from the dialog fields in the object.
*/
GlgLong ApplyDialogData()
{
   char
     * name,
     * object_data,
     * datasource;

   switch( SelectedObjectType )
   {
    case NODE:
    case LINK:
      break;
    default: return True;
   }   

   GlgGetSResource( Viewport, "Dialog/DialogName/TextString", &name );
   SetObjectLabel( SelectedObject, SelectedObjectType, name );

   GlgGetSResource( Viewport, "Dialog/DialogData/TextString", &object_data );
   SetObjectData( SelectedObject, SelectedObjectType, object_data );

   if( ProcessDiagram )
   {
      GlgGetSResource( Viewport, "Dialog/DialogDataSource/TextString", 
                      &datasource );
      SetObjectDataSource( SelectedObject, SelectedObjectType, datasource );
   }

   GlgUpdate( Viewport );
   return True;
}

/*----------------------------------------------------------------------
| 
*/
char * GetObjectLabel( GlgObject object, ObjectType type )
{
   void * data;

   data = GetData( object );
   if( type == NODE )
     return ((GlgNodeData)data)->object_label;
   else
     return ((GlgLinkData)data)->object_label;
}

/*----------------------------------------------------------------------
| 
*/
void SetObjectLabel( GlgObject object, ObjectType type, char * label )
{
   void * data;
   
   /* Display label in the node or link object if it has a label. */
   if( GlgHasResourceObject( object, "Label" ) )
     GlgSetSResource( object, "Label/String", label );

   data = GetData( object );
   switch( type )
   {
    case NODE:
      SetString( &((GlgNodeData)data)->object_label, label );
      break;

    case LINK:
      SetString( &((GlgLinkData)data)->object_label, label );
      break;
   }
}
   
/*----------------------------------------------------------------------
| 
*/
char * GetObjectData( GlgObject object, ObjectType type )
{
   void * data;

   data = GetData( object );
   if( type == NODE )
     return ((GlgNodeData)data)->object_data;
   else
     return ((GlgLinkData)data)->object_data;
}

/*----------------------------------------------------------------------
| 
*/
void SetObjectData( GlgObject object, ObjectType type, char * object_data )
{
   void * data;

   data = GetData( object );
   if( type == NODE )
     SetString( &((GlgNodeData)data)->object_data, object_data );
   else
     SetString( &((GlgLinkData)data)->object_data, object_data );
}

/*----------------------------------------------------------------------
| 
*/
char * GetObjectDataSource( GlgObject object, ObjectType type )
{
   void * data;

   data = GetData( object );
   if( type == NODE )
     return ((GlgNodeData)data)->datasource;
   else
     return ((GlgLinkData)data)->datasource;
}

/*----------------------------------------------------------------------
| 
*/
void SetObjectDataSource( GlgObject object, ObjectType type, char * datasource )
{
   void * data;

   if( datasource && !*datasource )
     datasource = NULL;   /* Substitute NULL for empty datasource strings. */

   data = GetData( object );
   switch( type )
   {
    case NODE:
      if( GlgHasResourceObject( object, "Value" ) )
	GlgSetSResource( object, "Value/Tag", datasource );

      SetString( &((GlgNodeData)data)->datasource, datasource );
      break;

    case LINK:
      SetString( &((GlgLinkData)data)->datasource, datasource );
      break;
   }
}

/*----------------------------------------------------------------------
| 
*/
GlgDiagramData CreateDiagramData()
{
   GlgDiagramData data;

   data = GlgAlloc( sizeof( GlgDiagramDataStruct ) );
   data->node_list =
     GlgCreateObject( GLG_GROUP, NULL,
		     (GlgAnyType)GLG_LONG, NULL, NULL, NULL );
   data->link_list =
     GlgCreateObject( GLG_GROUP, NULL,
		     (GlgAnyType)GLG_LONG, NULL, NULL, NULL );
   return data;
}

/*----------------------------------------------------------------------
| 
*/
void DestroyDiagramData( GlgDiagramData data )
{
   GlgNodeData node;
   GlgLinkData link;
   GlgLong i;   

   for( i=0; i < GlgGetSize( data->node_list ); ++i )
   {
      node = (GlgNodeData) GlgGetElement( data->node_list, i );
      DestroyNodeData( node );
   }

   for( i=0; i < GlgGetSize( data->link_list ); ++i )
   {
      link = (GlgLinkData) GlgGetElement( data->link_list, i );
      DestroyLinkData( link );
   }

   GlgDropObject( data->node_list );
   GlgDropObject( data->link_list );
   GlgFree( data );
}

/*----------------------------------------------------------------------
| 
*/
GlgNodeData CreateNodeData()
{
   GlgNodeData data = GlgAlloc( sizeof( GlgNodeDataStruct ) );

   data->graphics = NULL;
   data->object_label = GlgStrClone( "" );
   data->object_data = GlgStrClone( "" );
   return data;
}

/*----------------------------------------------------------------------
| 
*/
GlgLinkData CreateLinkData()
{
   GlgLinkData data = GlgAlloc( sizeof( GlgLinkDataStruct ) );
   
   data->start_node = NULL;
   data->end_node = NULL;
   data->start_point_name = NULL;
   data->end_point_name = NULL;
   data->graphics = NULL;
   data->point_array = NULL;
   data->object_label = GlgStrClone( "A1" );
   data->object_data = GlgStrClone( "" );
   data->first_move = True;
   return data;
}

/*----------------------------------------------------------------------
| 
*/
void DestroyNodeData( GlgNodeData data )
{
   GlgDropObject( data->graphics );
   GlgFree( data->object_label );
   GlgFree( data->object_data );
   GlgFree( data );
}

/*----------------------------------------------------------------------
| 
*/
void DestroyLinkData( GlgLinkData data )
{
   GlgDropObject( data->graphics );
   GlgDropObject( data->point_array );
   GlgFree( data->start_point_name );
   GlgFree( data->end_point_name );
   GlgFree( data->object_label );
   GlgFree( data->object_data );
   GlgFree( data );
}

/*----------------------------------------------------------------------
| 
*/
GlgLong NodeConnected( GlgObject node )
{
   GlgLinkData link_data;
   GlgLong i;

   for( i=0; i< GlgGetSize( CurrentDiagram->link_list ); ++ i )
   {
      link_data = (GlgLinkData) GlgGetElement( CurrentDiagram->link_list, i );
      if( link_data->start_node && link_data->start_node->graphics == node ||
	 link_data->end_node && link_data->end_node->graphics == node )
	return True;      
   }
   return False;
}

/*----------------------------------------------------------------------
| Custom callback, invoked when a node or link is added.
*/
void AddObjectCB( GlgObject object, void * data, GlgLong is_node )
{}

/*----------------------------------------------------------------------
| Custom callback, invoked when a node or link is selected.
*/
void SelectObjectCB( GlgObject object, void * data, GlgLong is_node )
{}

/*----------------------------------------------------------------------
| Custom callback, invoked when a node or link is deleted.
*/
void CutObjectCB( GlgObject object, void * data, GlgLong is_node )
{}

/*----------------------------------------------------------------------
| Custom callback, invoked when a node or link is pasted.
*/
void PasteObjectCB( GlgObject object, void * data, GlgLong is_node )
{}

/*----------------------------------------------------------------------
| Updates all tags defined in the drawing for a process diagram.
*/
void UpdateProcessDiagram( GlgAnyType data, GlgLong * timer_ptr )
{
   GlgObject drawing, tag_list, data_object;
   char * tag_name;
   double new_value;
   GlgLong i, size;

   if( !ProcessDiagram )
     return;

   drawing = (GlgObject) data;

   /* Since new nodes may be added or removed in the process of the diagram
      editing, get the current list of tags every time. In an application 
      that just displays the diagram without editing, the tag list may be
      obtained just once (initially) and used to subscribe to data.
      Query only unique tags.
      */
   tag_list = GlgCreateTagList( drawing, True );
   if( tag_list )
   {
      size = GlgGetSize( tag_list );
      for( i=0; i<size; ++i )
      {
         data_object = GlgGetElement( tag_list, i );
         GlgGetSResource( data_object, "Tag", &tag_name );

         new_value = GetTagValue( tag_name, data_object );
         GlgSetDTag( drawing, tag_name, new_value, True );
      }
      
      GlgDropObject( tag_list );
      GlgUpdate( drawing );
   }

   /* Re-install the timer to continue updates. */
   GlgAddTimeOut( AppContext, UPDATE_INTERVAL, 
                 (GlgTimerProc)UpdateProcessDiagram, drawing );
}

/*----------------------------------------------------------------------
| Get new value based on a tag name. In a real application, the value
| is obtained from a process database. PLC or another live datasource.
| In the demo, use random data.
*/
double GetTagValue( char * tag_name, GlgObject data_object )
{
   double value, increment, direction;

   /* Get the current value */
   GlgGetDResource( data_object, NULL, &value );
   
   /* Increase it. */
   increment = GlgRand( 0., 0.1 );

   if( value == 0. )
     direction = 1.;
   else if( value == 1. )
     direction = -1.;
   else
     direction = GlgRand( -1., 1. );

   if( direction > 0. )
     value += increment;
   else
     value -= increment;

   if( value > 1. )
     value = 1.;
   else if( value < 0. )
     value = 0.;

   return value;
}

/*----------------------------------------------------------------------
|
*/
GlgIHToken ButtonToToken( char * button_name )
{
   GlgLong i;

   for( i=0; ButtonTokenTable[i].name; ++i )
     if( strcmp( button_name, ButtonTokenTable[i].name ) == 0 )
       return ButtonTokenTable[i].token;

   return IH_UNDEFINED_TOKEN;   /* 0 */
}

/*----------------------------------------------------------------------
|
*/
void error( char * string, GlgBoolean quit )
{
   GlgError( GLG_USER_ERROR, string );
   if( quit )
     exit( GLG_EXIT_ERROR );
}
