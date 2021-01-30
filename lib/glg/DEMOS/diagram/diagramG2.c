#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#ifdef _WINDOWS
#include "resource.h"
#endif
#include "GlgApi.h"

#ifdef _WINDOWS
#define sleep( time )    Sleep( 1000 * time )
#endif

/* True to save data, False to save the drawing only with no connectivity 
   information. */
#define SAVE_DATA       False

#define POINT_SELECTION_RESOLUTION     3

/* Default scale factor for icon buttons. */
#define DEFAULT_ICON_ZOOM_FACTOR    10.

/* Percentage of the button area to use for the icon. */
#define ICON_FIT_FACTOR    0.75

#ifndef MAX
#define MAX(x,y)   ( (x) > (y) ? (x) : (y) )
#endif

/* Number of palette buttons to skip: the first button with the "select" icon
   is already in the palette. */
#define PALETTE_START_INDEX    1

typedef enum _InteractionMode
{
   SELECT_OBJECT = 0,
   ADD_NODE,
   ADD_LINK1,
   ADD_LINK2,
   DRAGGING
} InteractionMode;

typedef enum _EventType
{
   MOUSE_PRESSED = 0,
   MOUSE_MOVED_EVENT,
   MOUSE_RELEASED
} EventType;

typedef enum _ObjectType
{
   NO_OBJ = 0,
   NODE,
   LINK
} ObjectType;

long StickyCreateMode = False;
long AllowUnconnectedLinks = True;
GlgObject Viewport = (GlgObject)0;
GlgObject MainArea = (GlgObject)0;
GlgObject SelectedObject = (GlgObject)0;
GlgObject PointMarker = (GlgObject)0;
GlgObject AttachmentMarker = (GlgObject)0;
GlgObject AttachmentArray = (GlgObject)0;
GlgObject AttachmentNode = (GlgObject)0;
ObjectType SelectedObjectType = NO_OBJ;
long NodeType;
long LinkType;
long EdgeType;
GlgObject DragLink = (GlgObject)0;
GlgObject Point1 = (GlgObject)0;
GlgObject StoredColor = (GlgObject)0;
GlgObject CutBuffer = (GlgObject)0;
ObjectType CutBufferType = NO_OBJ;
GlgObject Rect = (GlgObject)0;
InteractionMode Mode;
long NumLinkPoints = 0;
long MiddlePointAdded = False;
char * LastButton = NULL;
GlgPoint start_point;
/* Icon arrays holds node or link icons. Object arrays hold the objects
   to use in the drawing. In case of connectors, only a part of the icon 
   (the connector object) is used in the drawing, without the end markers.
   */
GlgObject NodeIconArray = (GlgObject)0;
GlgObject NodeObjectArray = (GlgObject)0;
GlgObject LinkIconArray = (GlgObject)0;
GlgObject LinkObjectArray = (GlgObject)0;
GlgObject ButtonTemplate = (GlgObject)0;
GlgObject PaletteTemplate = (GlgObject)0;
double
  NumRows,
  NumColumns,
  IconScale = 1.;      /* May be used to scale the icons */
GlgPoint 
  start_point,
  end_point;
GlgAppContext AppContext;
long ProcessDiagram = False;     /* Defines the type of the diagram. */
long FirstMove = False;

/* If set to True, icons are automatically fit to fill the button.
   If set to False, the default zoom factor will be used. */
long FitIcons = False;

/* Used by process diagram */
long DataSourceCounter = 0;
long NumDatasources = 20;
#define UPDATE_INTERVAL   1000     /* Update once per second */

#include "diagram_proto2.h"          /* Function prototypes */

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*=======================================================================
| A diagramming editor: example of using Extended API.
| The type of the diagram is selected by the first command-line argument:
|   -process-diagram or -diagram.
|
| The  AddObjectCB, SelectObjectCB, CutObjectCB and DeleteObjectCB 
| callbacks at the end of this file may be used to interface with 
| application-specific functionality.
*/
int GlgMain( argc, argv, InitAppContext )
     int argc;
     char *argv[];
     GlgAppContext InitAppContext;
{
   long skip;
   char * full_path;

   AppContext = GlgInit( False, InitAppContext, argc, argv );

#ifndef _WINDOWS
   /* Start as a process diagram if invoked through the process_diagram 
      symbolic link on Unix. 
      */
   if( strstr( argv[0], "process_diagram_no_opengl" ) )
   {
      ProcessDiagram = True;
      GlgSetDResource( NULL, "$config/GlgOpenGLMode", 0. );
   }
   else if( strstr( argv[0], "process_diagram" ) )
     ProcessDiagram = True;
#endif

   for( skip = 1; skip < argc; ++skip )
   {
      /* Handle options to start as either diagram or process diagram. */
      else if( strcmp( argv[ skip ], "-process-diagram" ) == 0 )
        ProcessDiagram = True;
      else if( strcmp( argv[ skip ], "-diagram" ) == 0 )
        ProcessDiagram = False;
   }

   DisplayUsage();

   /* If set to True, icons are automatically fit to fill the button.
      If set to False, the default zoom factor will be used. */
   if( ProcessDiagram )
     FitIcons = True;
   else
     FitIcons = False;

   full_path = GlgGetRelativePath( argv[0], ProcessDiagram ? 
                                  "process_diagram.g" : "diagram.g" );

   Viewport = GlgLoadWidgetFromFile( full_path );
   if( !Viewport )
   {
      fprintf( stderr, "\007Can't load <%s> drawing.\n", full_path );
      exit( GLG_EXIT_ERROR );
   }
   GlgFree( full_path );

   MainArea = GlgGetResourceObject( Viewport, "MainArea" );
   if( !MainArea )
   {
      fprintf( stderr, "\007Can't find MainArea viewport.\n" );
      exit( GLG_EXIT_ERROR );
   }

   PaletteTemplate = GlgGetResourceObject( Viewport, "PaletteTemplate" );
   if( !PaletteTemplate )
   {
      fprintf( stderr, "\007Can't find PaletteTemplate viewport.\n" );
      exit( GLG_EXIT_ERROR );
   }
   GlgReferenceObject( PaletteTemplate );
   GlgDeleteThisObject( Viewport, PaletteTemplate );

   GlgAddCallback( Viewport, GLG_INPUT_CB, (GlgCallbackProc)InputCB, NULL );

   /* Add trace callback to the main area to trace only its events. */
   GlgAddCallback( MainArea, GLG_TRACE_CB, (GlgCallbackProc)TraceCB, NULL );

   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( Viewport, "Point1", -700., -700., 0. );
   GlgSetGResource( Viewport, "Point2",  700.,  700., 0. );
   
   /* Setting the window name (title). */
   GlgSetSResource( Viewport, "ScreenName", 
                   ( ProcessDiagram ? 
                    "GLG Process Diagram Demo" : "GLG Diagram Editor Demo" );

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
                   (GlgTimerProc)UpdateProcessDiagram, NULL );

   return (int) GlgMainLoop( AppContext );
}

void DisplayUsage()
{
#ifndef _WINDOWS            
   printf( "Use the -process-diagram or -diagram options\n" );
   printf( "    to select the type of the diagram.\n" );
#endif
}

/*----------------------------------------------------------------------
| Performs initial setup of the drawing.
*/
void Initialize( viewport )
     GlgObject viewport;
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

   /* Make the properties dialog a top-level window, set its title and make
      it invisible on startup. */
   GlgSetDResource( viewport, "Dialog/ShellType", (double) GLG_DIALOG_SHELL );
   GlgSetSResource( viewport, "Dialog/ScreenName", "Object Properties" );
   GlgSetDResource( viewport, "Dialog/Visibility", 0. );

   if( ProcessDiagram )   /* Same for the datasource dialog. */
   {
      GlgSetDResource( viewport, "DataSourceDialog/Visibility", 0. );
      GlgSetDResource( viewport, "DataSourceDialog/ShellType", 
                      (double) GLG_DIALOG_SHELL );
      GlgSetSResource( viewport, "DataSourceDialog/ScreenName", 
                      "DataSource List" );
   }

   /* Create a separate group to hold objects. */
   group = GlgCreateObject( GLG_GROUP, "ObjectGroup", NULL, NULL, NULL, NULL );
   GlgAddObjectToBottom( MainArea, group );
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

   Mode = SELECT_OBJECT;
   SetRadioBox( "IconButton0" );   /* Highlight Select button */

   SetCreateMode();    /* Query initial create mode from the drawing. */
}

/*----------------------------------------------------------------------
| Sets create mode based on the state of the CreateMode button.
*/
void SetCreateMode()
{
   double create_mode;

   GlgGetDResource( Viewport, "CreateMode/OnState", &create_mode );
   StickyCreateMode = ( create_mode != 0. );
}

/*----------------------------------------------------------------------
| Handles mouse operations: selection, dragging, connection point 
| highlight.
*/
void TraceCB( viewport, client_data, call_data )
     GlgObject viewport;
     GlgAnyType client_data;
     GlgAnyType call_data;
{
   GlgPoint cursor_pos;
   GlgTraceCBStruct * trace_data;
   EventType event_type;
   double zoom_to_mode;

   trace_data = (GlgTraceCBStruct*) call_data;

   /* Use the MainArea's events only. */
   if( trace_data->viewport != MainArea )
     return;

   /* Platform-specific code to extract event information. */
#ifndef _WINDOWS
   switch( trace_data->event->type )
   {
    case ButtonPress:
    case ButtonRelease:
      if( trace_data->event->xbutton.button != 1 )
	return;  /* Use the left button clicks only. */
      cursor_pos.x = trace_data->event->xbutton.x;
      cursor_pos.y = trace_data->event->xbutton.y;
      event_type = ( trace_data->event->type == ButtonPress ? 
		    MOUSE_PRESSED : MOUSE_RELEASED );
      break;
    case MotionNotify:
      cursor_pos.x = trace_data->event->xmotion.x;
      cursor_pos.y = trace_data->event->xmotion.y;
      event_type = MOUSE_MOVED_EVENT;
      break;
    default: return;
   }      
#else
   switch( trace_data->event->message )
   {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
      cursor_pos.x = LOWORD( trace_data->event->lParam );
      cursor_pos.y = HIWORD( trace_data->event->lParam );
      switch( trace_data->event->message )
      {
       case WM_LBUTTONDOWN:
       case WM_LBUTTONDBLCLK: event_type = MOUSE_PRESSED; break;
       case WM_LBUTTONUP:     event_type = MOUSE_RELEASED; break;
       case WM_MOUSEMOVE:     event_type = MOUSE_MOVED_EVENT; break;
      }
      break;
    default: return;
   }
#endif

   GlgGetDResource( MainArea, "ZoomToMode", &zoom_to_mode );
   if( zoom_to_mode )
     return;   /* Don't handle mouse selection in ZoomTo mode. */

   cursor_pos.z = 0.;

   switch( Mode )
   {
    default:
      GlgBell( Viewport );
      ResetModes();
      GlgUpdate( Viewport );
      return;

    case ADD_NODE: AddNodeHandler( event_type, &cursor_pos ); return;

    case ADD_LINK1:
    case ADD_LINK2:      
      AddLinkHandler( event_type, &cursor_pos, viewport, trace_data );
      return;
      
    case DRAGGING:
      DraggingHandler( event_type, &cursor_pos ); return;
      
    case SELECT_OBJECT:
      ObjectHandler( event_type, &cursor_pos, viewport, trace_data ); return;
   }
}

/*----------------------------------------------------------------------
|
*/
void AddNodeHandler( event_type, cursor_pos )
     EventType event_type;
     GlgPoint * cursor_pos;
{
   GlgObject new_node;

   if( event_type != MOUSE_PRESSED )
     return;

   new_node = AddNodeAt( NodeType, cursor_pos );
   AddObjectCB( new_node, True );

   SelectGlgObject( new_node, NODE );

   if( !StickyCreateMode )
     ResetModes();

   GlgUpdate( Viewport );
}

/*----------------------------------------------------------------------
|
*/
void AddLinkHandler( event_type, cursor_pos, viewport, trace_data )
     EventType event_type;
     GlgPoint * cursor_pos;
     GlgObject viewport;
     GlgTraceCBStruct * trace_data;
{
   GlgObject
     selection,
     sel_object,
     pt_array, 
     point,
     sel_node;
   ObjectType selection_type;
   double d_type;
   long 
     i,
     num_selected;

   if( event_type != MOUSE_PRESSED && event_type != MOUSE_MOVED_EVENT )
     return;

   /* Drag the link's last point. */
   if( event_type == MOUSE_MOVED_EVENT && Mode == ADD_LINK2 )
   {
      /* First time: set link direction depending of the 
         direction of the first mouse move, then make the link visible.
         */
 if( FirstMove )
      {
         SetEdgeDirection( DragLink, &start_point, cursor_pos );
         GlgSetDResource( DragLink, "Visibility", 1. );
         FirstMove = False;
      }

      SetLastPoint( DragLink, cursor_pos, False );

      if( !MiddlePointAdded )
	SetArcMiddlePoint( DragLink );
      GlgUpdate( Viewport );
   }

   selection = 
     GetObjectsAtCursor( viewport, trace_data->viewport, cursor_pos );

   if( selection && ( num_selected = (long) GlgGetSize( selection ) ) )
   {
      /* Some object were selected, process the selection to find the point 
	 to connect to */
      point = (GlgObject)0;
      pt_array = (GlgObject)0;
      sel_node = (GlgObject)0;
      for( i=0; i < num_selected; ++i )
      {
	 sel_object = GlgGetElement( selection, i );

	 /* Find if the object itself is a link or a node, or if it's a part
	    of a node. If it's a part of a node, get the node object ID.
	    */
	 sel_object = GetSelectedObject( sel_object, &selection_type );

	 if( selection_type == NODE )
	 {
	    GlgGetDResource( sel_object, "Type", &d_type );
	    if( d_type == GLG_REFERENCE )
	    {
	       /* Use ref's point as a connector. */
	       point = GlgGetResourceObject( sel_object, "Point" );
	    }
	    else /* Node has multiple attachment point: get the points. */
	    {
	       pt_array = GetAttachmentPoints( sel_object, "CP" );
               if( !pt_array )
                 continue;

               point = GetSelectedPoint( pt_array, cursor_pos );               

               /* Use attachment points array to highlight all attachment 
                  points only if no specific point is selected, and only
                  on the mouse move. On the mouse press, the specific point
                  is used to connect to.
                  */
               if( point || event_type != MOUSE_MOVED_EVENT )
                 SetObject( &pt_array, (GlgObject)0 );
               else
                 sel_node = sel_object;
	    }

            /* If found a point to connect to, stop search and use it.
               If found a node with attachment points, stop search and
               highlight the points.
               */
	    if( point || pt_array )
            {
               if( point )   /* If found the point, reset pt_array */
                 SetObject( &pt_array, (GlgObject)0 );
               break;
            }
	 }
	 
	 /* No point to connect to: continue searching all selected objects. */
      }

      if( point )    /* Use point if found. */
      {
	 switch( event_type )
	 {
	    /* Show feedback at the connector's position. */
	  case MOUSE_MOVED_EVENT: 
            ShowAttachmentPoints( point, (GlgObject)0, (GlgObject)0, 0 );
	    GlgUpdate( Viewport );	    
	    break;
	    
	  case MOUSE_PRESSED:	    /* Connect */
	    switch( Mode )
	    {	       
	     case ADD_LINK1:
	       Point1 = point;

               DragLink = AddLinkObject( LinkType );
               NumLinkPoints = 1;
               
               /* First point */
               ConstrainLinkPoint( DragLink, Point1, False );
               AttachFramePoints( DragLink );

	       /* Store cursor position for setting direction based on the
		  first mouse move. */
	       start_point = *cursor_pos;
               FirstMove = True;
               GlgSetDResource( DragLink, "Visibility", 0. );
	       
	       Mode = ADD_LINK2;
	       SetPrompt( "Select the second node." );

	       GlgUpdate( Viewport );
	       break;
	       
	     case ADD_LINK2:
	       if( point == Point1 )
	       {
		  SetError(
		"The two nodes are the same, chose a different second node." );
		  break;
	       }
	       
	       ++NumLinkPoints;
	       
	       /* Last point */
	       ConstrainLinkPoint( DragLink, point, True ); 
	       AttachFramePoints( DragLink );
	       if( !MiddlePointAdded )
		 SetArcMiddlePoint( DragLink );
	       
               FinalizeLink( DragLink );
               DragLink = (GlgObject)0;
       
               if( StickyCreateMode )
               {
                  /* Start over to create more links. */
                  Mode = ADD_LINK1;
                  SetPrompt( "Select the first node." );
                  MiddlePointAdded = False;
               }
               else
                 ResetModes();

	       GlgUpdate( Viewport );
	       break;
	    }
	    break;
	 }      /* switch( event_type ) */

	 GlgDropObject( selection );
	 return;    /* We are done with the point. */

      }  /* if( point ) */
      /* Mouse is over a node: highlight all connection points. */
      else if( pt_array )
      {
         ShowAttachmentPoints( (GlgObject)0, pt_array, sel_node, 1 );
         GlgDropObject( pt_array );         
         GlgUpdate( Viewport );	    
	 return;    /* We are done with pt_array. */
      }
   }

   /* No point or no selection: handle erasing attachment point feedback and 
      adding middle points for arc links. */

   GlgDropObject( selection );

   if( EraseAttachmentPoints() )
     GlgUpdate( Viewport );

   if( event_type == MOUSE_PRESSED )    /* Add middle link point */
   {	     
      if( Mode == ADD_LINK1 )
      {
         SetError( "Invalid connection point!" );
         return;  /* No first point yet */
      }

      ++NumLinkPoints;

      /* Add middle link point */
      AddLinkPoints( DragLink, 1 );
      MiddlePointAdded = True;

      SetLastPoint( DragLink, cursor_pos, EdgeType == GLG_ARC );
      AttachFramePoints( DragLink );

      if( EdgeType == GLG_ARC )
      {
	 /* Offset the last point after setting the middle one. */
	 cursor_pos->x += 10;
	 cursor_pos->y += 10;
	 SetLastPoint( DragLink, cursor_pos, False );
      }
      GlgUpdate( Viewport );	    
   }
}

/*----------------------------------------------------------------------
| 
*/
void ShowAttachmentPoints( point, pt_array, sel_node, highlight_type )
     GlgObject point, pt_array, sel_node;    
     long highlight_type;
{
   GlgObject marker;
   GlgPoint
     world_point,
     screen_point;
   long i, size;

   if( point )
   {
      if( AttachmentArray )
        EraseAttachmentPoints();   

      /* Get the screen coords of the connector point, not the cursor
         position: may be a few pixels off. */
      GlgGetGResource( point, "XfValue", &screen_point.x,
                      &screen_point.y, &screen_point.z );
	    
      GlgScreenToWorld( MainArea, True, &screen_point, &world_point );
      
      if( !AttachmentMarker )
      {
         AttachmentMarker = PointMarker;
         GlgAddObjectToBottom( MainArea, AttachmentMarker );
      }

      /* Position the feedback marker over the connector */
      GlgSetGResource( AttachmentMarker, "Point", 
                      world_point.x, world_point.y, world_point.z );

      GlgSetDResource( AttachmentMarker, "HighlightType", 
                      (double)highlight_type );
   }
   else if( pt_array )
   {
      if( sel_node = AttachmentNode )
        return;    /* Attachment points are already shown for this node. */
      
      /* Erase previous attachment feedback if shown. */
      EraseAttachmentPoints();   

      size = GlgGetSize( pt_array );
      AttachmentArray =
        GlgCreateObject( GLG_GROUP, NULL, NULL, (GlgAnyType)size, NULL, NULL );
      AttachmentNode = sel_node;

      for( i=0; i<size; ++i )
      {
         marker = GlgCopyObject( PointMarker );

         point = GlgGetElement( pt_array, i );
         
         /* Get the screen coords of the connector point */
         GlgGetGResource( point, "XfValue", &screen_point.x,
                         &screen_point.y, &screen_point.z );
	    
         GlgScreenToWorld( MainArea, True, &screen_point, &world_point );
         
         /* Position the feedback marker over the connector */
         GlgSetGResource( marker, "Point", 
                         world_point.x, world_point.y, world_point.z );

         GlgSetDResource( marker, "HighlightType", (double)highlight_type );

         GlgAddObjectToBottom( AttachmentArray, marker );
         GlgDropObject( marker );
      }
      GlgAddObjectToBottom( MainArea, AttachmentArray );
      GlgDropObject( AttachmentArray );
   }
}

/*----------------------------------------------------------------------
| Erases attachment points feedback if shown. Returns True if feedback
| was erased, of False if there was nothing to erase.
*/
long EraseAttachmentPoints()
{
   if( AttachmentMarker )
   {
      GlgDeleteThisObject( MainArea, AttachmentMarker );
      AttachmentMarker = (GlgObject)0;
      return True;
   }

   if( AttachmentArray )
   {
      GlgDeleteThisObject( MainArea, AttachmentArray );
      AttachmentArray = (GlgObject)0;
      AttachmentNode = (GlgObject)0;
      return True;
   }

   return False;    /* Nothing to erase. */
}

/*----------------------------------------------------------------------
| 
*/
void FinalizeLink( link )
     GlgObject link;
{
   GlgObject arrow_type;

   arrow_type = GlgGetResourceObject( DragLink, "ArrowType" );
   if( arrow_type )
     GlgSetDResource( arrow_type, 
                     NULL, (double) GLG_MIDDLE_FILL_ARROW );
   
   /* After storing color: changes color to select */
   SelectGlgObject( link, LINK );
   
   AddObjectCB( link, False );	
}

/*----------------------------------------------------------------------
| Handles object selection and dragging start.
*/
void ObjectHandler( event_type, cursor_pos, viewport, trace_data )
     EventType event_type;
     GlgPoint * cursor_pos;
     GlgObject viewport;
     GlgTraceCBStruct * trace_data;
{
   GlgObject
     selection,
     sel_object;
   long 
     i,
     num_selected;
   ObjectType selection_type;

   if( event_type != MOUSE_PRESSED )
     return;

   selection = 
     GetObjectsAtCursor( viewport, trace_data->viewport, cursor_pos );

   if( selection && ( num_selected = (long) GlgGetSize( selection ) ) )
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
	    
	    GlgUpdate( Viewport );

	    SelectObjectCB( sel_object, selection_type == NODE );

	    /* Prepare for dragging */
	    Mode = DRAGGING;

	    /* Store the start point */
	    start_point = *cursor_pos;

	    GlgDropObject( selection );
	    return;
	 }
      }
   }

   GlgDropObject( selection );

   SelectGlgObject( (GlgObject)0, 0 );    /* Unselect. */

   GlgUpdate( Viewport );
}

/*----------------------------------------------------------------------
|
*/
void DraggingHandler( event_type, cursor_pos )
     EventType event_type;
     GlgPoint * cursor_pos;
{
   switch( event_type )
   {
    case MOUSE_RELEASED:
      ResetModes();
      return;

    case MOUSE_MOVED_EVENT:
      if( SelectedObjectType == NODE )
	GlgMoveObject( SelectedObject, GLG_SCREEN_COORD,
		      &start_point, cursor_pos );
#if 0
      /* Disallow dragging links with the mouse: no connectivity information, 
         can't properly handle moving links attached to icons with multiple 
         attached points without connectivity information.
         Enable only if all icons use reference objects with a single
         attachment point.
         */         
      else
	MoveLink( SelectedObject, &start_point, cursor_pos );
#endif

      GlgUpdate( Viewport );

      /* Update the start point for the next move. */
      start_point = *cursor_pos;
      break;
   }
}

/*----------------------------------------------------------------------
| If the link is attached to nodes that use reference objects, moving the
| link moves the nodes, with no extra actions required. However, the link
| can be connected to a node with multiple attachment points which doesn't
| use reference object. Moving such a link would move just the attachment
| points, but not the nodes. To avoid this, we need to unsconstrain the 
| end points of the link not to spoil the attachment points' geometry, 
| move the link, move the nodes, then constrain the link points back.
| This is done in the diagram demi that keeps connectivity information.
| In this simpler version, we just draw and save the graphics with
| no diagram logic.
*/
void MoveLink( link, start_point, end_point )
     GlgObject link;
     GlgPoint
       * start_point,
       * end_point;
{
   /* Just move the link. For nodes that are reference objects, moving 
      the link moves the nodes. */
   GlgMoveObject( link, GLG_SCREEN_COORD, start_point, end_point );
}

/*----------------------------------------------------------------------
| 
*/
GlgObject GetObjectsAtCursor( viewport, event_vp, cursor_pos )
     GlgObject viewport, event_vp;
     GlgPoint * cursor_pos;
{
   GlgRectangle select_rect;

#define SELECTION_RESOLUTION  5    /* Selection sensitivity in pixels */

   /* Select all object in the vicinity of the +-SELECTION_RESOLUTION pixels
      from the actual mouse click position. */
   select_rect.p1.x = cursor_pos->x - SELECTION_RESOLUTION;
   select_rect.p1.y = cursor_pos->y - SELECTION_RESOLUTION;
   select_rect.p2.x = cursor_pos->x + SELECTION_RESOLUTION;
   select_rect.p2.y = cursor_pos->y + SELECTION_RESOLUTION;

   return
     GlgCreateSelection( /** top viewport **/ viewport, &select_rect,
			/** event viewport **/ event_vp );
}

/*----------------------------------------------------------------------
|
*/
void InputCB( viewport, client_data, call_data )
     GlgObject viewport;
     GlgAnyType client_data;
     GlgAnyType call_data;
{
   GlgObject 
     message_obj,
     object;
   char
     * format,
     * origin,
     * full_origin,
     * action,
     * subaction,
     * string;

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
          /* Close the dialog */
          GlgSetDResource( viewport, "Dialog/Visibility", 0. );
	else
	  exit( GLG_EXIT_OK );
   }
   else if( strcmp( format, "Button" ) == 0 )
   {
      if( strcmp( action, "Activate" ) != 0 && 
	 strcmp( action, "ValueChanged" ) != 0 )
	return;
      
      ResetModes();

      if( strcmp( origin, "Save" ) == 0 )
	Save();
      else if( strcmp( origin, "Insert" ) == 0 )
	Load();
      else if( strcmp( origin, "Print" ) == 0 )
	Print();
      else if( strcmp( origin, "Cut" ) == 0 )
	Cut();
      else if( strcmp( origin, "Paste" ) == 0 )
	Paste();
      else if( strcmp( origin, "Exit" ) == 0 )
	exit( GLG_EXIT_OK );
      else if( strcmp( origin, "ZoomIn" ) == 0 )
      {
	 ResetModes();
	 GlgSetZoom( MainArea, NULL, 'i', 0. );
      }
      else if( strcmp( origin, "ZoomOut" ) == 0 )
      {
	 ResetModes();
	 GlgSetZoom( MainArea, NULL, 'o', 0. );
      }
      else if( strcmp( origin, "ZoomTo" ) == 0 )
      {
	 ResetModes();
	 GlgSetZoom( MainArea, NULL, 't', 0. );
      }
      else if( strcmp( origin, "ZoomReset" ) == 0 )
      {
	 ResetModes();
	 GlgSetZoom( MainArea, NULL, 'n', 0. );
      }
      else if( strncmp( origin, "IconButton", strlen( "IconButton" ) ) ==0 )
      {
	 GlgObject
	   button,
	   icon;
	 char * icon_type;

	 button = GlgGetResourceObject( viewport, full_origin );
	 icon = GlgGetResourceObject( button, "Icon" );
	 if( !icon )
	 {
	    SetError( "Can't find icon." );
	    return;
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
	    return;
	 }

	 if( strcmp( icon_type, "Select" ) == 0 )
	   ResetModes();      

	 else if( strcmp( icon_type, "Link" ) == 0 )
	   AddLink( full_origin, object );
	 
	 else if( strcmp( icon_type, "Node" ) == 0 )
	   AddNode( full_origin, object );
      }      
      else if( strcmp( origin, "Properties" ) == 0 )
      {
         FillData();
         GlgSetDResource( viewport, "Dialog/Visibility", 1. );
      }
      else if( strcmp( origin, "CreateMode" ) == 0 )
      { 
         ResetModes();
         SetCreateMode();
      }
      else if( strcmp( origin, "DialogApply" ) == 0 )
	ApplyData();      
      else if( strcmp( origin, "DialogClose" ) == 0 )
      {
	 if( ApplyData() )
	   /* Close the dialog if successfully applied. */
	   GlgSetDResource( viewport, "Dialog/Visibility", 0. );
      }
      else if( strcmp( origin, "DialogCancel" ) == 0 )
      {
	 /* Close the dialog without applying the data. */
	 GlgSetDResource( viewport, "Dialog/Visibility", 0. );
      }
      /* The rest of buttons exist in and are used for process diagrams only */
      else if( strcmp( origin, "DataSourceSelect" ) == 0 )
        GlgSetDResource( viewport, "DataSourceDialog/Visibility", 1. );
      else if( strcmp( origin, "DataSourceClose" ) == 0 )
        GlgSetDResource( viewport, "DataSourceDialog/Visibility", 0. );
      else if( strcmp( origin, "DataSourceApply" ) == 0 )
      {
         GlgGetSResource( viewport, "DataSourceList/SelectedItem", &string );
         GlgSetSResource( viewport, "Dialog/DialogDataSource/TextString",
                         string );
      }
   }

   GlgUpdate( Viewport );
}

/*----------------------------------------------------------------------
|
*/
void SelectGlgObject( object, selected_type )
     GlgObject object;
     ObjectType selected_type;
{
   static GlgObject last_color = (GlgObject)0;
   char * name;

   if( object == SelectedObject )
     return;   /* No change */

   if( last_color )    /* Restore the color of previously selected node. */
   {
      GlgSetResourceFromObject( last_color, NULL, StoredColor );
      SetObject( &last_color, (GlgObject)0 );
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
void AddNode( button_name, node )
     char * button_name;
     GlgObject node;
{
   double node_type;

   Mode = ADD_NODE;
   SetPrompt( "Position the node." );

   SetRadioBox( button_name );

   /* Store Node type */  
   GlgGetDResource( node, "Index", &node_type );
   NodeType = node_type;
}

/*----------------------------------------------------------------------
|
*/
void AddLink( button_name, link )
     char * button_name;
     GlgObject link;
{
   double link_type;

   Mode = ADD_LINK1;
   SetPrompt( "Select the first node." );

   SetRadioBox( button_name );

   /* Store Link type */  
   GlgGetDResource( link, "Index", &link_type );
   LinkType = link_type;

   GetCPContainer( link, &EdgeType );      /* Get EdgeType */
}

/*----------------------------------------------------------------------
|
*/
void Cut()
{
   GlgObject
     group,
     list;

   if( NoSelection() )
     return;

   group = GlgGetResourceObject( Viewport, "MainArea/ObjectGroup" );

   if( GlgContainsObject( group, SelectedObject ) )
   {
      /* Store the node in the cut buffer. It also keeps it referenced while
	 deleting link, etc. */
      SetObject( &CutBuffer, SelectedObject );
      CutBufferType = SelectedObjectType;

      /* Delete the node */
      GlgDeleteThisObject( group, SelectedObject );

      CutObjectCB( SelectedObject, SelectedObjectType == NODE );

      SelectGlgObject( (GlgObject)0, 0 );
   }
   else
     SetError( "Cut failed." );    
}

/*----------------------------------------------------------------------
|
*/
void Paste()
{
   GlgObject 
     group,
     list;

   if( !CutBuffer )
   {
      SetError( "Empty cut buffer, cut some object first." );
      return;
   }
	    
   group = GlgGetResourceObject( Viewport, "MainArea/ObjectGroup" );

   PasteObjectCB( CutBuffer, CutBufferType == NODE );

   if( CutBufferType == NODE )
     GlgAddObjectToBottom( group, CutBuffer );     /* In front */
   else
     GlgAddObjectToTop( group, CutBuffer );        /* Behind */

   SelectGlgObject( CutBuffer, CutBufferType );

   /* Allow pasting just once to avoid handling the data copy */
   CutBuffer = (GlgObject)0;
}

/*----------------------------------------------------------------------
| Save .g drawing: only graphics, no connectivity information.
*/
void Save()
{
   GlgObject selected_object;
   long selected_object_type;

   /* Unselect to avoid saving the object in the selected state. */
   selected_object = SelectedObject;
   selected_object_type = SelectedObjectType;
   SelectGlgObject( (GlgObject)0, 0 );

   GlgSetSResource( MainArea, "Name", "$Widget" ); /* Save as $Widget */

   GlgSaveObject( MainArea, "saved_diagram.g" );

   GlgSetSResource( MainArea, "Name", "MainArea" ); /* Restore name */

   /* Restore selection */
   SelectGlgObject( selected_object, selected_object_type );
}

/*----------------------------------------------------------------------
| Load saved .g drawing.
*/
void Load()
{
   GlgObject new_drawing;

   new_drawing = GlgLoadWidgetFromFile( "saved_diagram.g" );
   if( new_drawing )
   {
      double
        x1, y1, z1,
        x2, y2, z2;

      /* Check if it contains "ObjectGroup" */
      if( !GlgHasResourceObject( new_drawing, "ObjectGroup" ) )
      {
         GlgBell( Viewport );
         SetPrompt( "Load failed: the file was not saved with this application." );
         GlgDropObject( new_drawing );
         return;
      }

      /* Reset selection: new drawing is loaded. */
      SelectGlgObject( (GlgObject)0, 0 );

      /* Get extent of the MainArea. */
      GlgGetGResource( MainArea, "Point1", &x1, &y1, &z1 );
      GlgGetGResource( MainArea, "Point2", &x2, &y2, &z2 );
      
      /* Set the extent to match the extent of the viewport being replaces. */
      GlgSetGResource( new_drawing, "Point1", x1, y1, z1 );
      GlgSetGResource( new_drawing, "Point2", x2, y2, z2 );

      /* Add trace callback */
      GlgAddCallback( new_drawing, GLG_TRACE_CB, (GlgCallbackProc)TraceCB, 
                     NULL );

      /* Name it MainArea */
      GlgSetSResource( new_drawing, "Name", "MainArea" );
      
      /* Replace the MainArea with the loaded object. */
      GlgDeleteThisObject( Viewport, MainArea );
      GlgAddObjectToBottom( Viewport, new_drawing );

      MainArea = new_drawing;
      GlgDropObject( new_drawing );
   }
   else
   {
      GlgBell( Viewport );
      SetPrompt( "Load failed: can't load the drawing from saved_drawing.g" );
   }
}

/*----------------------------------------------------------------------
|
*/
void Print()
{
   ResetModes();

   /* Set Level 1 PostScript for compatibility with old printers. */
   GlgSetSResource( Viewport, "$config/GlgPSLevel", "level1" );
      
   if( !GlgPrint( Viewport, "editor.ps",
		 -900., -900., 1800., 1800., True, False ) )
   {
      GlgBell( Viewport );
      SetPrompt( "Printing failed: can't write <editor.ps> file." );
   }
   else
     SetPrompt( "Saved postscript output as <editor.ps>." );
}

/*----------------------------------------------------------------------
|
*/
void ResetModes()
{
   if( Mode )
   {
      if( DragLink )
      {
         if( AllowUnconnectedLinks && FinishLink( DragLink ) )
           ;   /* Keep the link even if its second end is not connected. */
         else
         {
            GlgObject group;

            /* Delete link if if its second end is not connected. */
            group = GlgGetResourceObject( MainArea, "ObjectGroup" );
            GlgDeleteThisObject( group, DragLink );
         }
         DragLink = (GlgObject)0;
      }

      EraseAttachmentPoints();
   }

   Mode = SELECT_OBJECT;
   SetRadioBox( "IconButton0" );   /* Highlight Select button */

   SetPrompt( "" );
   GlgUpdate( Viewport );
}

/*----------------------------------------------------------------------
| If AllowUnconnectedLinks=true, keep the link if it has at least two 
| points.
*/
long FinishLink( link )
     GlgObject link;
{
   GlgObject 
     point_container,
     suspend_info;
   long 
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
void SetPrompt( string )
     char * string;
{
   GlgSetSResource( Viewport, "Prompt/String", string );
   GlgUpdate( Viewport );
}

/*----------------------------------------------------------------------
|
*/
void SetError( string )
     char * string;
{
   GlgError( GLG_USER_ERROR, string );

   SetPrompt( string );
}

/*----------------------------------------------------------------------
|
*/
void SetObject( object_ptr, object )
     GlgObject * object_ptr;
     GlgObject object;
{
   /* No NULL check is required for Glg Ref/Drop functions. */
   GlgDropObject( *object_ptr );
   *object_ptr = GlgReferenceObject( object );
}

/*----------------------------------------------------------------------
|
*/
void SetString( string_ptr, string )
     char ** string_ptr;
     char * string;
{
   /* No NULL check is required for Glg string functions. */
   GlgFree( *string_ptr );
   *string_ptr = GlgStrClone( string );
}

/*----------------------------------------------------------------------
|
*/
long NoSelection()
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
GlgObject AddNodeAt( node_type, pos )
     long node_type;
     GlgPoint * pos;
{     
   GlgObject
     new_node,
     node_list,
     group;
   double object_type;

   /* Create the node based on the node type */
   new_node = CreateNode( node_type );
   GlgGetDResource( new_node, "Type", &object_type );      

   /* Make label visible and set it to empty string initially. */
   if( GlgHasResourceObject( new_node, "Label" ) )
   {
      GlgSetDResource( new_node, "Label/Visibility", 1. );
      GlgSetSResource( new_node, "Label/String", "" );
   }

   AddCustomData( new_node, "CustomString", "" );

   /* Store datasource as a tag of the node's Value resource, if it exists. */
   if( ProcessDiagram )
   {
      GlgObject value_obj;
      char * datasource;

      /* Assign an arbitrary datasource initially. */
      datasource = GlgCreateIndexedName( "DataSource", DataSourceCounter );
      ++DataSourceCounter;
      if( DataSourceCounter >= NumDatasources )
        DataSourceCounter = 0;

      value_obj = GlgGetResourceObject( new_node, "Value" );
      if( value_obj )
      {         
         GlgObject tag_obj;

         tag_obj = 
           GlgCreateObject( GLG_TAG, NULL, "Value", datasource, NULL, NULL );

         GlgSetResourceObject( value_obj, "TagObject", tag_obj );
         GlgDropObject( tag_obj );
      }
   }

   /* Add the object to the drawing first, so that it's hierarchy is setup
      for postioning it. */
   group = GlgGetResourceObject( MainArea, "ObjectGroup" );
   GlgAddObjectToBottom( group, new_node );

   /* Transform the object to set its size and position. */
   PlaceObject( new_node, pos );

   return new_node;
}

/*----------------------------------------------------------------------
|
*/
GlgObject CreateNode( node_type )
     long node_type;
{
   GlgObject new_node;
   char * label;

   /* Get node template from the palette */
   new_node = GlgGetElement( NodeObjectArray, node_type );
	 
   /* Create a new node instance  */
   new_node = GlgCloneObject( new_node, GLG_STRONG_CLONE );

   /* Name node using an "object" prefix (used to distiguish
      nodes from links on selection). */
   GlgSetSResource( new_node, "Name", "object" );

   if( ProcessDiagram )
   {
      /* Init the label to the initial value if provided. */
      if( GlgHasResourceObject( new_node, "InitLabel" ) &&
         GlgHasResourceObject( new_node, "Label/String" ) )
      {
         GlgGetSResource( new_node, "InitLabel", &label );
         GlgSetSResource( new_node, "Label/String", label );
      }
   }

   return new_node;
}

/*----------------------------------------------------------------------
|
*/
GlgObject CreateLink( link_type )
     long link_type;
{
   GlgObject new_link;
   long num_points;

   /* Get link template from the palette */
   new_link = GlgGetElement( LinkObjectArray, link_type );
	 
   /* Create a new link instance */
   new_link = GlgCloneObject( new_link, GLG_STRONG_CLONE );

   /* Name link using a "link" prefix (used to distiguish
      links from nodes on selection). */
   GlgSetSResource( new_link, "Name", "link" );

   return new_link;
}

/*----------------------------------------------------------------------
| Connects the first or last point of the link.
*/
void ConstrainLinkPoint( link, point, last_point )
     GlgObject link, point;
     GlgBoolean last_point;
{
   GlgObject
     link_point,
     point_container,
     suspend_info;
   char * point_name;
   long edge_type;

   point_container = GetCPContainer( link, &edge_type );

   link_point = 
     GlgGetElement( point_container, 
		( last_point ? GlgGetSize( point_container ) - 1 : 0 ) );
   
   suspend_info = GlgSuspendObject( link );
   GlgConstrainObject( link_point, point );
   GlgReleaseObject( link, suspend_info );
}    

/*----------------------------------------------------------------------
| Positions the arc's middle point if it's not explicitly defined.
*/
void SetArcMiddlePoint( link )
     GlgObject link;
{
   GlgObject point_container;
   long edge_type;
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
void AttachFramePoints( link )
     GlgObject link;
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
   if( !frame ) /* Link without label and frame */
     return;

   link_point_container = GetCPContainer( link, (long*)0 );

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
| Set last point of the link (dragging).
*/
void SetLastPoint( link, cursor_pos, arc_middle_point )
     GlgObject link;
     GlgPoint * cursor_pos;
     GlgBoolean arc_middle_point;
{
   GlgObject 
     point,
     point_container;
   GlgPoint world_coord;
   long edge_type;
   
   point_container = GetCPContainer( link, &edge_type );

   if( arc_middle_point )
     /* Setting the middle point of an arc. */
     point = GlgGetElement( point_container, 1 );
   else
     /* Setting the last point. */
     point =
       GlgGetElement( point_container, GlgGetSize( point_container ) - 1 );

   GlgScreenToWorld( MainArea, True, cursor_pos, &world_coord );
   GlgSetGResource( point, NULL, world_coord.x, world_coord.y, world_coord.z );
}

/*----------------------------------------------------------------------
| 
*/
void AddLinkPoints( link, num_points )
     GlgObject link;
     long num_points;
{
   GlgObject 
     point,
     add_point,
     point_container,
     suspend_info;
   long
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
void SetEdgeDirection( link, start_pos, end_pos )
     GlgObject link;
     GlgPoint * start_pos, * end_pos;
{
   long
     edge_type,
     direction;

   GetCPContainer( link, &edge_type );
   if( edge_type == 0 || edge_type == GLG_ARC )   /* Arc or polygon */
     return;

   if( fabs( start_pos->x - end_pos->x ) > fabs( start_pos->y - end_pos->y ) )
     direction = GLG_HORIZONTAL;
   else
     direction = GLG_VERTICAL;

   GlgSetDResource( link, "EdgeDirection", (double) direction );   
}

/*----------------------------------------------------------------------
|
*/
GlgObject AddLinkObject( link_type )
     long link_type;
{     
   GlgObject link, group;

   link = CreateLink( link_type );

   AddCustomData( link, "CustomString", "" );

   /* Init label to "" */
   if( GlgHasResourceObject( link, "Label" ) )
     GlgSetSResource( link, "Label/String", "" );

   /* Add to the top of the draw list to be behind other objects. */
   group = GlgGetResourceObject( MainArea, "ObjectGroup" );
   GlgAddObjectToTop( group, link );
   
   return link;
}

/*----------------------------------------------------------------------
| Set the object size and position.
*/
void PlaceObject( node, pos )
     GlgObject node;
     GlgPoint * pos;
{
   GlgPoint world_coord;
   double d_type;

   GlgGetDResource( node, "Type", &d_type );
   if( d_type == GLG_REFERENCE )
   {
      /* Reference: can use its point to position it. */

      /* Convert to world coordinates. */
      GlgScreenToWorld( MainArea, True, pos, &world_coord );

      /* Position its point in the world coordinates. */
      GlgSetGResource( node, "Point", 
		      world_coord.x, world_coord.y, world_coord.z );

      if( IconScale != 1. )   /* Change node size if required. */
	/* Scale object around the origin, which is now located at pos. */
	GlgScaleObject( node, GLG_SCREEN_COORD, pos, 
                       IconScale, IconScale, 1. );
   }
   else
   {
      /* Arbitrary object: move its box's center to the cursor position. */
      GlgPositionObject( node, GLG_SCREEN_COORD, GLG_HCENTER | GLG_VCENTER,
			pos->x, pos->y, pos->z );

      if( IconScale != 1. )   /* Change node size if required. */
	/* Scale object around the center of it's bounding box. */
	GlgScaleObject( node, GLG_SCREEN_COORD, NULL, 
                       IconScale, IconScale, 1. );
   }
}
      
/*----------------------------------------------------------------------
| Get the link's control points container based on the link type.
*/
GlgObject GetCPContainer( link, edge_type )
     GlgObject link;
     long * edge_type;
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
	 *edge_type = (long)d_edge_type;
      }
      break;

    default: SetError( "Invalid link type." ); return (GlgObject)0;
   }
   return point_container;
}

/*----------------------------------------------------------------------
| Determines what node or link the object belongs to and returns it. 
| Also returns type of the object: NODE or LINK.
*/
GlgObject GetSelectedObject( object, selection_type )
     GlgObject object;
     ObjectType * selection_type;
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

      object = GlgGetParent( object, (GlgLong*)0 );
   }

   /* No node/link parent found - no selection. */
   if( selection_type )
    *selection_type = NO_OBJ;
   return (GlgObject)0;
}

/*----------------------------------------------------------------------
| Returns an array of all attachment points, i.e. the points whose names 
| start with the name_prefix.
*/
GlgObject GetAttachmentPoints( sel_object, name_prefix )
     GlgObject sel_object;
     char * name_prefix;
{
   GlgObject
     pt_array,
     attachment_pt_array,
     point;
   char * name;
   long 
     i,
     size;

   pt_array = GlgCreatePointArray( sel_object, 0 );
   if( !pt_array )
     return NULL;
     
   size = (long) GlgGetSize( pt_array );
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
     SetObject( attachment_pt_array, (GlgObject)0 );

   GlgDropObject( pt_array );
   return attachment_pt_array;
}

/*----------------------------------------------------------------------
| Checks if one of the point array's points is under the cursor.
*/
GlgObject GetSelectedPoint( point_array, cursor_pos )
     GlgObject point_array;
     GlgPoint * cursor_pos;
{
   GlgObject point;
   double x, y, z;
   long i, size;

   if( !point_array )
     return NULL;

   size = (long) GlgGetSize( point_array ) ;
   for( i=0; i<size; ++i )
   {
      point = GlgGetElement( point_array, i );

      /* Get position in screen coords. */
      GlgGetGResource( point, "XfValue", &x, &y, &z );
      if( fabs( cursor_pos->x - x ) < POINT_SELECTION_RESOLUTION &&
         fabs( cursor_pos->y - y ) < POINT_SELECTION_RESOLUTION )
        return point;	
   }
   return NULL;
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
*/
void FillObjectPalette( palette_name, button_name, start_index )
     char
       * palette_name,     /* Name of the object palette to add buttons to. */
       * button_name;      /* Base name of the object palette buttons. */
     long start_index;     /* Number of buttons to skip (select button is
                              already in the palette). */
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
   {
      SetError( "Can't find palette button to copy!" );
      exit( GLG_EXIT_ERROR );
   }		    
      
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
   ButtonTemplate = (GlgObject)0;

   GlgDropObject( PaletteTemplate );
   PaletteTemplate = (GlgObject)0;
   
   GlgDropObject( NodeIconArray );
   NodeIconArray = (GlgObject)0;
   
   GlgDropObject( LinkIconArray );
   LinkIconArray = (GlgObject)0;
}

/*----------------------------------------------------------------------
| Adds object palette buttons containing all icons from an array.
*/   
long FillObjectPaletteFromArray( palette, button_name, start_index, 
                                icon_array, object_array, default_tooltip )
     GlgObject palette;
     char * button_name;
     long start_index;
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
   long i, size, button_index;

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
*/
void SetupObjectPalette( button_name, start_index )
     char * button_name;   /* Base name of the palette buttons. */
     long start_index;     /* Number of buttons to skip (the select 
			      icon button is already in the palette). */
{
   GlgObject
     icon,
     button;
   char * res_name;
   double 
     object_type, 
     zoom_factor,
     icon_scale;
   long i;

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
double GetIconZoomFactor( button, icon )
     GlgObject button, icon;
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
void GetPaletteIcons( palette, icon_name, icon_array, object_array )
     GlgObject palette;
     char * icon_name;     /* Icon base name, same as icon type. */
     GlgObject icon_array, object_array;
{
   GlgObject
     icon,
     object;
   char
     * res_name,
     * type_string;   
   long
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

   size = (long) GlgGetSize( icon_array );
   if( !size )
     SetError( "Can't find any icons of this type." );
   else
     printf( "Scanned %ld %s icons\n", size, icon_name );
}

/*----------------------------------------------------------------------
| Highlights the new button and unhighlights the previous one.
*/
void SetRadioBox( button_name )
     char * button_name;
{
   GlgObject button;

   /* Always highlight the new button: the toggle would unhighlight if 
      clicked on twice. */
   button = GlgGetResourceObject( Viewport, button_name );
   if( button )
     GlgSetDResource( button, "OnState", 1. );

   /* Unhighlight the previous button. */
   if( LastButton && strcmp( button_name, LastButton ) != 0 )
   {
      button = GlgGetResourceObject( Viewport, LastButton );
      if( button )
	GlgSetDResource( button, "OnState", 0. );
   }

   SetString( &LastButton, button_name ); /* Store the last button. */
}

/*----------------------------------------------------------------------
|
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
|
*/
long ApplyData()
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

   /* Store data from the dialog fields in the object. */
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
char * GetObjectLabel( object, type )
     GlgObject object;
     ObjectType type;
{
   char * label = "";

   if( GlgHasResourceObject( object, "Label" ) )
     GlgGetSResource( object, "Label/String", &label );

   return label;
}

/*----------------------------------------------------------------------
| 
*/
void SetObjectLabel( object, type, label )
     GlgObject object;
     ObjectType type;
     char * label;
{
   /* Display label in the node or link object if it has a label. */
   if( GlgHasResourceObject( object, "Label" ) )
     GlgSetSResource( object, "Label/String", label );
}
   
/*----------------------------------------------------------------------
| 
*/
char * GetObjectData( object, type )
     GlgObject object;
     ObjectType type;
{
   char * data = "";

   if( GlgHasResourceObject( object, "CustomString" ) )
     GlgGetSResource( object, "CustomString", &data );

   return data;
}

/*----------------------------------------------------------------------
| 
*/
void SetObjectData( object, type, object_data )
     GlgObject object;
     ObjectType type;
     char * object_data;
{
   if( GlgHasResourceObject( object, "CustomString" ) )
     GlgSetSResource( object, "CustomString", object_data );
}

/*----------------------------------------------------------------------
| 
*/
char * GetObjectDataSource( object, type )
     GlgObject object;
     ObjectType type;
{
   char * datasource = "";

   if( type == NODE )
     if( GlgHasResourceObject( object, "Value/Tag" ) )
       GlgGetSResource( object, "Value/Tag", &datasource );

   return datasource;
}

/*----------------------------------------------------------------------
| 
*/
void SetObjectDataSource( object, type, datasource )
     GlgObject object;
     ObjectType type;
     char * datasource;
{
   if( type != NODE )
     return;

   if( datasource && !*datasource )
     datasource = NULL;   /* Substitute NULL for empty datasource strings. */

   if( GlgHasResourceObject( object, "Value/Tag" ) )
     GlgSetSResource( object, "Value/Tag", datasource );
}

/*----------------------------------------------------------------------
| Adds custom data to the graphical object
*/
void AddCustomData( object, name, value )
     GlgObject object;
     char 
       * name,
       * value;
{
   GlgObject
     custom_data,
     data_obj;   

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

   data_obj = 
     GlgCreateObject( GLG_ATTRIBUTE, NULL, 
                     (GlgAnyType) GLG_S, (GlgAnyType) GLG_UNDEFINED_XF, 
                     NULL, NULL );
   GlgSetSResource( data_obj, "Name", name ); 
   GlgSetSResource( data_obj, NULL, value ); 

   /* Add it to custom data. */
   GlgAddObjectToBottom( custom_data, data_obj );
   GlgDropObject( data_obj );
}

/*----------------------------------------------------------------------
| Custom callback, invoked when a node or link is added.
*/
void AddObjectCB( GlgObject object, long is_node )
{}

/*----------------------------------------------------------------------
| Custom callback, invoked when a node or link is selected.
*/
void SelectObjectCB( GlgObject object, long is_node )
{}

/*----------------------------------------------------------------------
| Custom callback, invoked when a node or link is deleted.
*/
void CutObjectCB( GlgObject object, long is_node )
{}

/*----------------------------------------------------------------------
| Custom callback, invoked when a node or link is pasted.
*/
void PasteObjectCB( GlgObject object, long is_node )
{}

/*----------------------------------------------------------------------
| Updates all tags defined in the drawing for a process diagram.
*/
void UpdateProcessDiagram( data, timer_ptr )
     GlgAnyType data;
     GlgLong * timer_ptr;
{
   GlgObject drawing, tag_list, data_object;
   char * tag_name;
   double new_value;
   long i, size;

   if( !ProcessDiagram )
     return;

   drawing = (GlgObject) MainArea;

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
double GetTagValue( tag_name, data_object )
     char * tag_name;
     GlgObject data_object;
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
