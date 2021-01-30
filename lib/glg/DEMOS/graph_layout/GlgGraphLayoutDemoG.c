#include <stdio.h>
#include <stdlib.h>
#ifdef _WINDOWS
#include "resource.h"
#endif
#include "GlgApi.h"
#include "GlgGraphLayout.h"

typedef enum _EventType
{
   NO_EVENT = 0,
   MOUSE_MOVE,
   BUTTON_PRESS,
   BUTTON_RELEASE
} EventType;

long
  IsReady = False,
  NumNodes = 20,   /* Default: may be changed by the "-num_nodes N"
		      command line option. */
  NumNodeTypes = 2,
  Untangle = True,
  Star = False;

GlgObject drawing = (GlgObject)0;
GlgObject SelectedNode = (GlgObject)0;
GlgGraphNode SelectedGraphNode = (GlgGraphNode)0;     
GlgAppContext AppContext;

/* Function prototypes. */
void Trace( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
GlgBoolean Update( GlgGraphLayout graph );
GlgObject GetSelectedNode( GlgObject object );
void SelectNode( GlgGraphLayout graph, GlgObject node );
long GetUpdateRate( GlgObject drawing );
long GetNumNodes( GlgObject drawing );
void SetIconSize( GlgGraphLayout graph, GlgObject drawing );

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*------------------------------------------------------------------------
| Optional Args:
|   NumNodes num_nodes
|   NumNodeTypes num_node_types
*/
int GlgMain( argc, argv, InitAppContext )
     int argc;
     char *argv[];
     GlgAppContext InitAppContext;
{
   GlgGraphLayout graph;
   GlgObject 
     viewport,     
     palette;
   long skip;
   char * full_path;

   AppContext = GlgInit( False, InitAppContext, argc, argv );

   /* Scan options */
   for( skip = 1; skip < argc; ++skip )
   {
      if( strcmp( argv[ skip ], "-NumNodes" ) == 0 )
      {
	 ++skip;
	 if( argc <= skip || sscanf( argv[ skip ], "%ld", &NumNodes ) != 1 )
	   GlgGraphError( "Invalid number of nodes." );
	 printf( "Using NumNodes = %ld\n", NumNodes );
      }
      else if( strcmp( argv[ skip ], "-NumNodeTypes" ) == 0 )
      {
	 ++skip;
	 if( argc <= skip || 
	    sscanf( argv[ skip ], "%ld", &NumNodeTypes ) != 1 )
	   GlgGraphError( "Invalid number of node types." );
	 printf( "Using NumNodes = %ld\n", NumNodeTypes );
      }
      else
	break;
   }

   /* Drawing for the demo with interface, etc. */
   full_path = GlgGetRelativePath( argv[0], "graph.g" );
   drawing = GlgLoadWidgetFromFile( full_path );
   if( !drawing )
     exit( GLG_EXIT_ERROR );

   GlgFree( full_path );

   viewport = GlgGetResourceObject( drawing, "Graph" );
   if( !viewport )
   {
      GlgGraphError( "Can't find <Graph> viewport." );
      exit( GLG_EXIT_ERROR );
   }

   /* Check for a icon palette in the drawing file: "Graph/Viewport" */
   palette = GlgGetResourceObject( viewport, "Palette" );

   if( palette )
   {
      GlgReferenceObject( palette );

      /* Delete palette from the drawing. */
      GlgDeleteThisObject( viewport, palette );
   }
   else
   {
      /* Load a palette of icons from a separate file if not in the drawing. */
      full_path = GlgGetRelativePath( argv[0], "palette.g" );

      palette = GlgLoadObject( full_path );
      if( !palette )
      {
         GlgGraphError( "Can't load node palette" );
         exit( GLG_EXIT_ERROR );
      }
      GlgFree( full_path );
   }

   /* Palettes may be set on per graph basis using GlgGraphSetPalette function.
      Here, setting the same palette for all graphs with GlgGraphSetDefPalette.
      */
   GlgGraphSetDefPalette( palette );
   GlgDropObject( palette );

   /* Set initial values of toggle buttons and sliders. */
   GlgSetDResource( drawing, "Untangle/OnState", (double)Untangle );
   GlgSetDResource( drawing, "Star/OnState", (double)Star );
   GlgSetDResource( drawing, "NumNodes/Value", (double)NumNodes );

   /* A graph may also be created using GlgGraphCreate(), GlgGraphAddNode() 
      and GlgGraphAddEdge() functions. */
   graph = GlgGraphCreateRandom( NumNodes, NumNodeTypes, 
				Star ? STAR_GRAPH : CIRCULAR_GRAPH );

   GlgGraphSetUntangle( graph, Untangle );
   GlgGraphUpdateRate( graph ) = GetUpdateRate( drawing );

   GlgGraphCreateGraphics( graph, viewport, (GlgObject)0 );

   SetIconSize( graph, drawing );

   GlgAddCallback( drawing,
		  GLG_INPUT_CB, (GlgCallbackProc)Input, (GlgAnyType)graph );

   GlgAddCallback( viewport, GLG_TRACE_CB, 
		   (GlgCallbackProc)Trace, graph );

   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( drawing, "Point1", -600., -770., 0. );
   GlgSetGResource( drawing, "Point2", 600., 770., 0. );

   /* Setting the window name (title). */
   GlgSetSResource( drawing, "ScreenName", "GLG Graph Layout Demo" );

   GlgInitialDraw( drawing );

#ifdef _WINDOWS            
   GlgLoadExeIcon( drawing, IDI_ICON1 );
#endif

   GlgMainLoop( AppContext );

   GlgGraphDestroy( graph );
   return 0;
}

/*----------------------------------------------------------------------
| 
*/
GlgBoolean Update( GlgGraphLayout graph )
{
   GlgBoolean finished;

   finished = GlgGraphSpringIterate( graph );
   return finished;   /* Continue to invoke if not finished; stop otherwise. */
}

/*----------------------------------------------------------------------
|
*/
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{
   GlgObject message_obj;
   GlgGraphLayout graph;
   char
     * format,
     * action,
     * origin;
   double on_state;

   message_obj = (GlgObject) call_data;
   graph = (GlgGraphLayout) client_data;

   /* Get the message's format and action. */
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "Origin", &origin );

   /* Handle window closing. May use viewport's name. */
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( GLG_EXIT_OK );

   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "FirstExposure" ) == 0 )
   {      
      if( !IsReady )   /* Fisrt time. */
      {
	 IsReady = True;

	 /* Start layout. */
	 GlgAddWorkProc( AppContext, (GlgWorkProc)Update, (GlgAnyType)graph );
      }
   }

   if( strcmp( format, "Button" ) == 0 )
   {
      if( strcmp( action, "Activate" ) == 0 )
      {
	 if( strcmp( origin, "New" ) == 0 )
	 {
	    GlgGraphDestroy( graph );
	    graph = GlgGraphCreateRandom( NumNodes, NumNodeTypes, 
					 Star ? STAR_GRAPH : CIRCULAR_GRAPH );
	    GlgGraphCreateGraphics( graph, 
				   GlgGetResourceObject( viewport, "Graph" ),
				   (GlgObject)0 );
	    GlgGraphSetUntangle( graph, Untangle );
	    GlgGraphUpdateRate( graph ) = GetUpdateRate( drawing );

	    GlgGraphUpdate( graph );
	    GlgAddWorkProc( AppContext, (GlgWorkProc)Update,
			   (GlgAnyType)graph );
	 }
	 else if( strcmp( origin, "Scramble" ) == 0 )
	 {
	    GlgGraphScramble( graph );
	    GlgGraphUpdate( graph );
	    GlgAddWorkProc( AppContext, (GlgWorkProc)Update,
			   (GlgAnyType)graph );
	 }
	 else if( strcmp( origin, "Circular" ) == 0 )
	 {
	    GlgGraphCircularLayout( graph );
	    GlgGraphUpdate( graph );
	 }
	 else if( strcmp( origin, "Quit" ) == 0 )
	   exit( GLG_EXIT_OK );
      }
      else if( strcmp( action, "ValueChanged" ) == 0 )
      {
	 if( strcmp( origin, "Untangle" ) == 0 )
	 {
	    GlgGetDResource( drawing, "Untangle/OnState", &on_state );
	    Untangle = (long)on_state;
	    GlgGraphSetUntangle( graph, Untangle );
	 }
	 else if( strcmp( origin, "Star" ) == 0 )
	 {
	    GlgGetDResource( drawing, "Star/OnState", &on_state );
	    Star = (long)on_state;
	 }
      }
   }
   else if( strcmp( format, "Slider" ) == 0 && 
	   strcmp( action, "ValueChanged" ) == 0 )
   {
      if( strcmp( origin, "UpdateRate" ) == 0 )
	GlgGraphUpdateRate( graph ) = GetUpdateRate( viewport );
      else if( strcmp( origin, "NumNodes" ) == 0 )
	NumNodes = GetNumNodes( viewport );
      else if( strcmp( origin, "IconSize" ) == 0 )
	SetIconSize( graph, viewport );
   }
}

/*----------------------------------------------------------------------
| Implements moving the node with the mouse.
*/
void Trace( viewport, client_data, call_data )
     GlgObject viewport;
     GlgAnyType client_data;
     GlgAnyType call_data;
{
   GlgObject 
     selection,
     sel_object;
   GlgTraceCBStruct * trace_data;
   GlgRectangle select_rect;   
   GlgGraphLayout graph;
   GlgGraphNode node;   
   GlgPoint
     screen_point,
     world_point;
   EventType event_type;
   long
     i,
     x, y;
   GlgLong num_selected;

   trace_data = (GlgTraceCBStruct*) call_data;
   graph = (GlgGraphLayout) client_data;

   /* Platform-specific code to extract event information. */
#ifndef _WINDOWS
   switch( trace_data->event->type )
   {
    case ButtonRelease:
      event_type = BUTTON_RELEASE;
      break;

    case ButtonPress:
      if( trace_data->event->xbutton.button != 1 )
	return;  /* Use the left button clicks only. */
      event_type = BUTTON_PRESS;
      x = trace_data->event->xbutton.x;
      y = trace_data->event->xbutton.y;
      break;
    case MotionNotify:
      event_type = MOUSE_MOVE;
      x = trace_data->event->xmotion.x;
      y = trace_data->event->xmotion.y;
      break;
    default: return;
   }      
#else
   switch( trace_data->event->message )
   {
    case WM_LBUTTONUP:
      event_type = BUTTON_RELEASE;
      break;

    case WM_LBUTTONDOWN:  event_type = BUTTON_PRESS;   goto process1;
    case WM_MOUSEMOVE:    event_type = MOUSE_MOVE;     goto process1;
    process1:
      x = LOWORD( trace_data->event->lParam );
      y = HIWORD( trace_data->event->lParam );
      break;
    default: return;
   }
#endif

#define SELECTION_RESOLUTION  2    /* Selection sensitivity in pixels */

   switch( event_type )
   {
    case BUTTON_PRESS: 
      /* Select all object in the vicinity of the +-SELECTION_RESOLUTION pixels
	 from the actual mouse click position. */
      select_rect.p1.x = x - SELECTION_RESOLUTION;
      select_rect.p1.y = y - SELECTION_RESOLUTION;
      select_rect.p2.x = x + SELECTION_RESOLUTION;
      select_rect.p2.y = y + SELECTION_RESOLUTION;
      
      selection = 
	GlgCreateSelection( /** top viewport **/ viewport, &select_rect,
			   /** event viewport **/ trace_data->viewport );
      
      if( selection && ( num_selected = GlgGetSize( selection ) ) )
      {
	 /* Some object were selected, process the selection. */
	 GlgSetStart( selection );
	 for( i=0; i < num_selected; ++i )
	 {
	    sel_object = GlgIterate( selection );
	    
	    /* Find the node the selected object belongs to. */
	    sel_object = GetSelectedNode( sel_object );	 
	    if( sel_object )
	    {
	       SelectNode( graph, sel_object );  /* Sets SelectedNode */
	       return;
	    }
	 }
      }

      /* No nodes were selected. */
      SelectNode( graph, (GlgObject)0 );    /* Unselect. */
      break;

    case MOUSE_MOVE:
      if( !SelectedNode )
	break;

      node = GlgGraphFindNode( graph, SelectedNode );
      if( !node )
      {
	 GlgGraphError( "Can't find the node in the graph." );
	 break;
      }

      if( GlgGraphFinished( graph ) )
      {
	 GlgGraphIncreaseTemperature( graph, False );
	 GlgAddWorkProc( AppContext, (GlgWorkProc)Update, (GlgAnyType)graph );
      }

      screen_point.x = x;
      screen_point.y = y;
      screen_point.z = 0;
      GlgScreenToWorld( viewport, True, &screen_point, &world_point );
      GlgGraphSetNodePosition( graph, node, world_point.x, world_point.y, 0. );

      /* If invoked in the middle of a graph layout, updates the nodes
	 with the latest layout results before updating the graphics.
	 */
      GlgGraphUpdate( graph );
      break;

    case BUTTON_RELEASE:
      SelectNode( graph, (GlgObject)0 );

      if( GlgGraphFinished( graph ) )
      {
	 GlgGraphIncreaseTemperature( graph, False );
	 GlgAddWorkProc( AppContext, (GlgWorkProc)Update, (GlgAnyType)graph );
      }
      break;
   }
}

/*----------------------------------------------------------------------
| Some part of the node may be selected, find the node it belongs to.
*/
GlgObject GetSelectedNode( GlgObject object )
{
   char * name;

   while( object )
   {
      GlgGetSResource( object, "Name", &name );
      if( name && strncmp( name, "Node", strlen( "Node" ) ) == 0 )
	return object;

      object = GlgGetParent( object, (GlgLong*)0 );
   }
   return (GlgObject)0;
}

/*----------------------------------------------------------------------
| 
*/
void SelectNode( GlgGraphLayout graph, GlgObject node )
{
   static GlgObject 
     last_color = (GlgObject)0,
     stored_color =  (GlgObject)0;
   GlgGraphNode graph_node;

   if( node == SelectedNode )
     return;   /* No change */

   if( last_color )    /* Restore the color of previously selected node. */
   {
      GlgSetResourceFromObject( last_color, NULL, stored_color );
      GlgDropObject( last_color );
      last_color = (GlgObject)0;
   }

   GlgDropObject( SelectedNode );
   SelectedNode = GlgReferenceObject( node );

   /* Change color to highlight selected node. */
   if( node )
   {
      if( last_color )
	GlgDropObject( last_color );
	
      last_color = GlgGetResourceObject( node, "Group/HighlightColor" );
      if( !last_color )
	GlgGraphError( "Can't find Icon's highlight color (Group/HighlightColor)." );
      GlgReferenceObject( last_color );

      /* Store original color */
      if( !stored_color )   
	stored_color = GlgCopyObject( last_color );     /* First time */
      else
	GlgSetResourceFromObject( stored_color, NULL, last_color );

      /* Set color to red to highlight selection. */
      GlgSetGResource( last_color, NULL, 1., 0., 0. );

      graph_node = GlgGraphFindNode( graph, node );
      if( !graph_node )
      {
	 GlgGraphError( "Can't find the graph node." );
	 return;
      }
      GlgNodeAnchor( graph_node ) = True;
      SelectedGraphNode = graph_node;
   }
   else   /* Unselecting */
     if( SelectedGraphNode )
     {
	GlgNodeAnchor( SelectedGraphNode ) = False;
	SelectedGraphNode = (GlgGraphNode)0;   
     }

   GlgUpdate( GlgGraphGetViewport( graph ) );
}

/*----------------------------------------------------------------------
| 
*/
long GetUpdateRate( GlgObject drawing )
{
   double update_rate, high;

   GlgGetDResource( drawing, "UpdateRate/Value", &update_rate );
   GlgGetDResource( drawing, "UpdateRate/High", &high );

   if( update_rate == high )
     return 1000000;    /* Update just once when finished. */
   else
     return (long) update_rate;
}

/*----------------------------------------------------------------------
| 
*/
long GetNumNodes( GlgObject drawing )
{
   double num_nodes;

   GlgGetDResource( drawing, "NumNodes/Value", &num_nodes );
   return (long) num_nodes;
}

/*----------------------------------------------------------------------
| 
*/
void SetIconSize( GlgGraphLayout graph, GlgObject drawing )
{   
   double icon_size;
 
   GlgGetDResource( drawing, "IconSize/Value", &icon_size );

   GlgSetDResource( GlgGraphGetViewport( graph ),
		   "Node%/Group/IconScale", icon_size );
   
   GlgUpdate( GlgGraphGetViewport( graph ) );
}
