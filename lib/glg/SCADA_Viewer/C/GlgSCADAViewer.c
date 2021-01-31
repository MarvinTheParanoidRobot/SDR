#include "scada.h"   
#include "HMIPage.h"
#include "EmptyPage.h"
#include "DefaultHMIPage.h"
#include "ProcessPage.h"
#include "RTChartPage.h"
#include "GlgSCADAViewer.h"

/* Page type table, used to determine the type of a loaded page based
   on the PageType property of the drawing. May be extended by an 
   application to add custom pages.
*/
TypeRecord PageTypeTable[] = {
   { "Default",       DEFAULT_PAGE_TYPE },
   { "Process",       PROCESS_PAGE },
   { "Aeration",      AERATION_PAGE },
   { "Circuit",       CIRCUIT_PAGE },
   { "RealTimeChart", RT_CHART_PAGE },
   { "TestCommands",  TEST_COMMANDS_PAGE },
   { NULL, 0 }
};

/* Default configuration file. */
char * DEFAULT_CONFIG_FILENAME = "scada_config_menu.txt";

/* Predefined menu table, used if the configuration file is not supplied, 
   or if the config file cannot be successfully parsed. 
*/
#define NUM_MENU_ITEMS 5
MenuRecord MenuTable[ NUM_MENU_ITEMS ] = 
{
   /* label               drawing name        tooltip             title */
   { "Solvent\nRecovery", "process.g", "process.g", 
     "Solvent Recovery System" },

   { "Water\nTreatment", "scada_aeration.g", "scada_aeration.g", 
     "Aeration Monitoring" },

   { "Eectrical\nCircuit", "scada_electric.g", "scada_electric.g", 
     "Electrical Circuit Monitoring" },

   { "Real-Time\nStrip-Chart", "scada_chart.g", "scada_chart.g", 
     "Real-Time Strip-Chart Sample" },

   { "Test Object\nCommands", "scada_test_commands.g", "scada_test_commands.g", 
     "Test Object Commands" }
};

GlgAppContext AppContext;
GlgSCADAViewer Viewer;

/* Flag indicating how to supply a time stamp for a
   RealTimeChart embedded into an HMI page: if set to 1, 
   the application will supply a time stamp explicitly.
   Otherwise, a time stamp will be supplied automatically by the chart 
   using current time. 
*/
#define SUPPLY_PLOT_TIME_STAMP 0

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*----------------------------------------------------------------------
|
*/
int GlgMain( int argc, char * argv[], GlgAppContext InitAppContext )
{
   /* Filename of the menu configuration file. May be supplied as the first
      command line argument. If not supplied, DEFAULT_CONFIG_FILENAME is used.
   */
   char * config_filename = DEFAULT_CONFIG_FILENAME;
   char * exe_path = argv[0];          /* Full path of the executable */
   char * full_path;
   long skip;

   AppContext = GlgInit( False, InitAppContext, argc, argv );

   GlgInitStruct( &Viewer, sizeof( Viewer ) );     /* Fill with zeros */
   Viewer.AlarmUpdateInterval = 1000;        /* Query alarms once a second. */

   /* By default, use simulated data to demo or to test without live data.
      Set to false or use -live-data command-line argument to use 
      custom live data feed.
   */
   Viewer.RANDOM_DATA = GlgTrue;

   /* Process command line arguments. */
   for( skip = 1; skip < argc; ++skip )
   {
      /* Use configuration file supplied as a command-line argument. */ 
      if( strcmp( argv[ skip ], "-config" ) == 0 )
      {
	 ++skip;
	 if( argc <= skip )
	 {
	    GlgError( GLG_USER_ERROR, "Missing configuration file name." );
	    break;
	 }
	 config_filename = argv[ skip ];
      }
      else if( strcmp( argv[ skip ], "-random-data" ) == 0 )
        Viewer.RANDOM_DATA = GlgTrue;
      else if( strcmp( argv[ skip ], "-live-data" ) == 0 )
        Viewer.RANDOM_DATA = GlgFalse;
   }

   if( Viewer.RANDOM_DATA )
     printf( "Using random demo DataFeed.\n" );
   else
     printf( "Using live DataFeed.\n" );
 
   /* Initialize MenuArray from the configuration file. */
   FillMenuArray( config_filename, exe_path );

   /* Initialize ActiveDialogs array and ActivePopupMenu. */
   InitActivePopups();

   /* Load the main top level drawing using the full file path. */    
   full_path = GlgGetRelativePath( exe_path, "scada_main.g" );
   Viewer.MainViewport = GlgLoadWidgetFromFile( full_path );
   GlgFree( full_path );

   if( !Viewer.MainViewport )
   {
      GlgError( GLG_USER_ERROR, "Can't load scada_main.g" );
      exit( GLG_EXIT_ERROR );
   }

   /* Set widget dimensions. If not set, default dimensions will 
      be used as set in the GLG editor.
   */
   GlgSetGResource( Viewer.MainViewport, "Point1", 0., 0., 0. );
   GlgSetGResource( Viewer.MainViewport, "Point2", 0., 0., 0. );

   GlgSetDResource( Viewer.MainViewport, "Screen/XHint", 0. );
   GlgSetDResource( Viewer.MainViewport, "Screen/YHint", 0. );
   GlgSetDResource( Viewer.MainViewport, "Screen/WidthHint", 900. );
   GlgSetDResource( Viewer.MainViewport, "Screen/HeightHint", 700. );

   /* Setting the window name (title). */
   GlgSetSResource( Viewer.MainViewport, "ScreenName", "GLG SCADA Viewer" );

   /* Add Input callback to handle input events */
   GlgAddCallback( Viewer.MainViewport, GLG_INPUT_CB, 
                   (GlgCallbackProc) InputCB, NULL );
   
   /* Add Hierarchy callback to handle events when a new drawing is loaded
      into a Subwindow object.
      */
   GlgAddCallback( Viewer.MainViewport, GLG_HIERARCHY_CB, 
                   (GlgCallbackProc) DrawingLoadedCB, NULL );
   
   /* Add Trace callback, used to handle low level events. */
   GlgAddCallback( Viewer.MainViewport, GLG_TRACE_CB, 
                   (GlgCallbackProc) TraceCB, NULL );

   /* Initialization before hierarchy setup. */
   InitBeforeH();

   /* Setup object hierarchy for the top  level viewport. */
   GlgSetupHierarchy( Viewer.MainViewport );

   /* Initialization after hierarchy setup: loads the first screen and starts
      data updates.
   */
   InitAfterH();

   /* Paint the GLG display. */
   GlgInitialDraw( Viewer.MainViewport );

#ifdef _WINDOWS 
   GlgLoadExeIcon( Viewer.MainViewport, IDI_ICON1 );
#endif

   return (int) GlgMainLoop( AppContext );
}

/*--------------------------------------------------------------------
| Initialize top level drawing before hierarchy setup took place.
*/
void InitBeforeH()
{
   double num_rows_d;

   CreateDataFeed();

   /* Store an object ID of the viewport named Menu, containing navigation
      buttons allowing to switch between drawings.
   */
   Viewer.Menu = GlgGetResourceObject( Viewer.MainViewport, "Menu" );

   /* Store an object ID of the Subwindow object named DrawingArea. */
   Viewer.DrawingArea = 
     GlgGetResourceObject( Viewer.MainViewport, "DrawingArea" );

   if( !Viewer.DrawingArea )
   {
      GlgError( GLG_USER_ERROR, "Can't find DrawigngArea Subwindow object." );
      exit( GLG_EXIT_ERROR );
   }
   
   /* Store an object ID of the AlarmDialog viewport. */
   Viewer.AlarmDialog = 
     GlgGetResourceObject( Viewer.MainViewport, "AlarmDialog" );
   Viewer.AlarmListVP = GlgGetResourceObject( Viewer.AlarmDialog, "Table" );

   GlgGetDResource( Viewer.AlarmListVP, "NumRows", &num_rows_d );
   Viewer.NumAlarmRows = num_rows_d;
   Viewer.AlarmStartRow = 0;

   /* Make AlarmDialog a free floating dialog. ShellType property
      can be also set at design time.
   */
   GlgSetDResource( Viewer.AlarmDialog, "ShellType", (double) GLG_DIALOG_SHELL );

   /* Initialize alarm list (set initial values in the template). */
   GlgSetSResource( Viewer.AlarmListVP, "Row/ID", "" );
   GlgSetSResource( Viewer.AlarmListVP, "Row/Description", "" );
   GlgSetDResource( Viewer.AlarmListVP, "Row/UseStringValue", 0. );
   GlgSetDResource( Viewer.AlarmListVP, "Row/DoubleValue", 0. );
   GlgSetSResource( Viewer.AlarmListVP, "Row/StringValue", "" );
   GlgSetDResource( Viewer.AlarmListVP, "Row/AlarmStatus", 0. );
   GlgSetDResource( Viewer.AlarmListVP, "Row/RowVisibility", 0. );

   /* Make AlarmDialog initially invisible. */
   GlgSetDResource( Viewer.AlarmDialog, "Visibility", 0. );
   Viewer.AlarmDialogVisible = GlgFalse;

   /* Set title for the AlarmDialog. */
   GlgSetSResource( Viewer.AlarmDialog, "ScreenName", "Alarm List" );

   /* Set initial state of the alarm button. */
   GlgSetDResource( Viewer.MainViewport, "AlarmButton/Blinking", 0. );

   /* Make Global PopupDialog a floating DialogShell type. */
   GlgSetDResource( Viewer.MainViewport, "PopupDialog/ShellType", 
                    (double) GLG_DIALOG_SHELL );

   /* Make Global PopupDialog initially invisible. */
   GlgSetDResource( Viewer.MainViewport, "PopupDialog/Visibility", 0. );

   /* Make Global PopupMenu initially invisible. */
   GlgSetDResource( Viewer.MainViewport, "PopupMenu/Visibility", 0. );

   /* Make message dialog invisible on startup. */
   Viewer.MessageDialog = 
     GlgGetResourceObject( Viewer.MainViewport, "MessageDialog" );
   GlgSetDResource( Viewer.MessageDialog, "Visibility", 0. );

   /* Make MessageDialog a free floating dialog. ShellType property
      can be also set at design time.
   */
   GlgSetDResource( Viewer.MessageDialog, "ShellType", 
                    (double) GLG_DIALOG_SHELL );

   /* Set the number of menu items. It must be done BEFORE hierarchy setup. */
   GlgSetDResource( Viewer.Menu, "NumRows", Viewer.NumMenuItems );
}

/*--------------------------------------------------------------------
| Initialize top level drawing after hierarchy setup took place.
*/
void InitAfterH()
{
   int i;
   char * resource_name;

   /* Initialize the menu. */
   InitMenu();

   /* Store object ID's of alarm rows for faster access. */
   Viewer.AlarmRows = GlgAlloc( sizeof( GlgObject ) * Viewer.NumAlarmRows );
   for( i=0; i<Viewer.NumAlarmRows; ++i )
   {
      resource_name = GlgCreateIndexedName( "Row%", i );
      Viewer.AlarmRows[i] = 
        GlgGetResourceObject( Viewer.AlarmListVP, resource_name );
      GlgFree( resource_name );
   }

   /* Load drawing corresponding to the first menu item. */
   SelectMainMenuItem( 0, /* update menu object */ GlgTrue );
   LoadDrawingFromMenu( 0 );   
   StartUpdates();
}

/*--------------------------------------------------------------------*/
void CreateDataFeed()
{
   if( !Viewer.DemoDataFeedPtr )
     Viewer.DemoDataFeedPtr = CreateDemoDataFeed();

   if( !Viewer.LiveDataFeedPtr )
     Viewer.LiveDataFeedPtr = CreateLiveDataFeed();
}

/*----------------------------------------------------------------------
| Initialize the navigation menu, a viewport named "Menu".
*/
void InitMenu()
{
   int i;
   GlgObject button;
   char* button_name;

   /* Populate menu buttons based on the MenuArray. */
   for( i=0; i<Viewer.NumMenuItems; ++i )
   {
      button_name = GlgCreateIndexedName( "Button%", i );
      button = GlgGetResourceObject( Viewer.Menu, button_name );
      GlgFree( button_name );

      GlgSetSResource( button, "LabelString", 
                       Viewer.MenuArray[ i ].label_string );
      GlgSetSResource( button, "TooltipString", 
                       Viewer.MenuArray[ i ].tooltip_string );
   }

   SelectMainMenuItem( NO_SCREEN, GlgTrue );
}

/*----------------------------------------------------------------------
| Input callback is invoked when the user interacts with objects in a
| GLG drawing. It is used to handle events occurred in input objects,
| such as a menu, as well as Commands or Custom Mouse Events attached 
| to objects at design time.
*/
void InputCB( GlgObject viewport, GlgAnyType client_data, 
              GlgAnyType call_data )
{
   GlgObject 
     message_obj,
     action_obj,
     selected_obj;
   char
     * format,
     * action,
     * origin;
   double menu_index;

   /* Return if the page's custom input handler processed the input.
      Otherwise, continue to process common events and commands.
   */
   if( HMIP_InputCB( Viewer.HMIPagePtr, viewport, client_data, call_data ) ) 
     return;

   message_obj = (GlgObject) call_data;

   /* Retrieve Format, Action and Origin from the message object. */
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "Origin", &origin );

   /* Retrieve selected object. */
   selected_obj = GlgGetResourceObject( message_obj, "Object" );

   /* Handle events from the screen navigation menu, named "Menu" */
   if( strcmp( format, "Menu" ) == 0 )
   {
      if( strcmp( action, "Activate" ) != 0 || strcmp( origin, "Menu" ) != 0 )
        return;

      /* User selected a button from the menu object named Menu. 
	 Load a new drawing associated with the selected button.
      */
      GlgGetDResource( message_obj, "SelectedIndex", &menu_index );
      SelectMainMenuItem( (GlgLong) menu_index, 
                          GlgFalse /* don't update menu object */ );

      /* Close active popup dialogs and popup menu, if any. */
      CloseActivePopups( CLOSE_ALL );

      StopUpdates();
 
      /* Load the drawing associated with the selected menu button and
	 display it in the DrawingArea.
      */
      LoadDrawingFromMenu( (GlgLong) menu_index );

      StartUpdates();
      GlgUpdate( viewport );
   }

   /* Handle custom commands attached to objects in the drawing. */
   else if( strcmp( format, "Command" ) == 0 )
   {
      action_obj = GlgGetResourceObject( message_obj, "ActionObject" ); 
      ProcessObjectCommand( viewport, selected_obj, action_obj );
      GlgUpdate( viewport );
   }

   /* Handle zoom controls on the left. */
   else if( strcmp( format, "Button" ) == 0 )
   {
      if( strcmp( action, "Activate" ) != 0 &&      /* Not a push button */
          strcmp( action, "ValueChanged" ) != 0 )   /* Not a toggle button */
        return;

      if( strcmp( origin, "MessageDialogOK" ) == 0 )
      {
         /* Close alarm dialog. */
         GlgSetDResource( Viewer.MessageDialog, "Visibility", 0. );
         GlgUpdate( Viewer.MessageDialog );
      }
     
      /* Zoom and pan buttons. */
      else if( strcmp( origin, "Left" ) == 0 )
        Zoom( Viewer.DrawingAreaVP, 'l', 0.1 );
      else if( strcmp( origin, "Right" ) == 0 )
        Zoom( Viewer.DrawingAreaVP, 'r', 0.1 );
      else if( strcmp( origin, "Up" ) == 0 )
        Zoom( Viewer.DrawingAreaVP, 'u', 0.1 );
      else if( strcmp( origin, "Down" ) == 0 )
        Zoom( Viewer.DrawingAreaVP, 'd', 0.1 );
      else if( strcmp( origin, "ZoomIn" ) == 0 )
        Zoom( Viewer.DrawingAreaVP, 'i', 1.5 );
      else if( strcmp( origin, "ZoomOut" ) == 0 )
        Zoom( Viewer.DrawingAreaVP, 'o', 1.5 );
      else if( strcmp( origin, "ZoomTo" ) == 0 )
        Zoom( Viewer.DrawingAreaVP, 't', 0. );
      else if( strcmp( origin, "ZoomReset" ) == 0 )
        Zoom( Viewer.DrawingAreaVP, 'n', 0. );

      /* Alarm scrolling buttons. */
      else if( strcmp( origin, "ScrollToTop" ) == 0 )
      {
         Viewer.AlarmStartRow = 0;
         ProcessAlarms( GlgFalse );
      }
      else if( strcmp( origin, "ScrollUp" ) == 0 )
      {
         --Viewer.AlarmStartRow;
         if( Viewer.AlarmStartRow < 0 )
           Viewer.AlarmStartRow = 0;
         ProcessAlarms( GlgFalse );
      }
      else if( strcmp( origin, "ScrollUp2" ) == 0 )
      {
         Viewer.AlarmStartRow -= Viewer.NumAlarmRows;
         if( Viewer.AlarmStartRow < 0 )
           Viewer.AlarmStartRow = 0;
         ProcessAlarms( GlgFalse );
      }
      else if( strcmp( origin, "ScrollDown" ) == 0 )
      {
         ++Viewer.AlarmStartRow;
         ProcessAlarms( GlgFalse );
      }
      else if( strcmp( origin, "ScrollDown2" ) == 0 )
      {
         Viewer.AlarmStartRow += Viewer.NumAlarmRows;
         ProcessAlarms( GlgFalse );
      }
      else
        return;

      GlgUpdate( viewport );
   }

   /* Handle custom events */
   else if( strcmp( format, "CustomEvent" ) == 0 )
   {
      char * event_label;
      GlgObject action_data = NULL;

      GlgGetSResource( message_obj, "EventLabel", &event_label );
      if( strcmp( event_label, "" ) == 0 )
         return;    /* don't process events with empty EventLabel */

      /* Retrieve action object. It will be null for custom events 
         added prior to GLG v.3.5. 
         If action object is present, retrieve its ActionData object.
         If Action Data object present, use its properties for custom event 
         processing as needed.
      */
      action_obj = GlgGetResourceObject( message_obj, "ActionObject" ); 
      if( action_obj )
	action_data = GlgGetResourceObject( action_obj, "ActionData" );
 
      if( strcmp( event_label, "AlarmRowACK" ) == 0 )
      {
         GlgObject alarm_row;
         char * tag_source;

         /* The object ID of the alarm row selected by Ctrl-click. */
         alarm_row = selected_obj;

         /* Retrieve the tag source. */
         GlgGetSResource( alarm_row, "ID", &tag_source );
            
         DF_ACKAlarm( Viewer.DataFeedPtr, tag_source );
      }
      else
      {
         /* Place custom code here to handle custom events as needed. */
      }

      GlgUpdate( viewport );
   }

   /* Handle events from a Real-Time Chart. */
   else if( strcmp( format, "Chart" ) == 0 &&
            strcmp( action, "CrossHairUpdate" ) == 0 )
   {
      /* To avoid slowing down real-time chart updates, invoke Update() 
         to redraw cross-hair only if the chart is not updated fast 
         enough by the timer.
      */
      if( Viewer.UpdateInterval > 100 )
        GlgUpdate( viewport );
   }

   /* Handle Timer events, generated by objects with blinking dynamics. */
   else if( strcmp( format, "Timer" ) == 0 )
   {
      /* Update objects with Blinking (Timer) dynamics. */
      GlgUpdate( viewport );
      GlgSync( viewport );    /* Improves interactive response. */
   }

   /* Handle window closing events. */
   else if( strcmp( format, "Window" ) == 0 &&
       strcmp( action, "DeleteWindow" ) == 0 )
   {
      int i;

      if( !selected_obj )
	return;

      /* If the event came from the top level application window,
	 exit from process.
      */
      if( selected_obj == Viewer.MainViewport )   
        exit( GLG_EXIT_OK );            /* Closing main application window. */

      else if( selected_obj == Viewer.AlarmDialog )
        ShowAlarms();   /*& Toggle the state of alarms dialog to erase it. */

      else
      {
         /* If the closing window is found in the ActiveDialogs array, 
            close the active dialog. 
         */
         for( i=0; i<MAX_DIALOG_TYPE; ++i )
           if( selected_obj == Viewer.ActiveDialogs[ i ].dialog )
           {
              ClosePopupDialog( i );
              break;
           }
      }

      GlgUpdate( viewport );
   } 
}

/*----------------------------------------------------------------------
| Trace callback. Used to handle low level events, such as obtaining 
| coordinates of the mouse click, or keyboard events. 
*/
void TraceCB( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data )
{
   GlgTraceCBStruct * trace_data;
   GlgObject event_vp;
   EventType event_type = 0;
   double x, y;
   int width, height;
#ifndef _WINDOWS  /* X/Linux */
#  define TEXT_BUFFER_LENGTH 128
   XEvent * event;
   KeySym keysym;
   XComposeStatus status;
   char buf[ TEXT_BUFFER_LENGTH ];
   int length;
#endif

   /* Return if the page's custom trace callback processed the event.
      Otherwise, continue to process common events.
   */
   if( HMIP_TraceCB( Viewer.HMIPagePtr, viewport, client_data, call_data ) )
     return;

   trace_data = (GlgTraceCBStruct*) call_data;
   event_vp = trace_data->viewport;

   /* Platform-specific code to extract event information.
      GLG_COORD_MAPPING_ADJ is added to the cursor coordinates for precise
      pixel mapping.
   */
#ifndef _WINDOWS  /* X/Linux */
   event = trace_data->event;
   
   switch( event->type )
   {
    case ButtonPress:
      x = event->xbutton.x + GLG_COORD_MAPPING_ADJ;
      y = event->xbutton.y + GLG_COORD_MAPPING_ADJ;
      event_type = BUTTON_PRESS_EVENT;
      break;
      
    case MotionNotify:
      x = event->xmotion.x + GLG_COORD_MAPPING_ADJ;
      y = event->xmotion.y + GLG_COORD_MAPPING_ADJ;
      event_type = MOUSE_MOVE_EVENT;
      break;

    case ConfigureNotify:
      width = event->xconfigure.width;
      height = event->xconfigure.height;
      event_type = RESIZE_EVENT;
      break;

    case KeyPress:
    case KeyRelease:
      /* Using XLookupString instead of XLookupKeysym to properly handle 
         Shift. */
      length = XLookupString( &event->xkey, buf, TEXT_BUFFER_LENGTH,
                              &keysym, &status );
      buf[ length ] = '\0';
      switch( keysym )
      {
       case XK_a:
         printf( "Pressed: a\n" );
         break;

       case XK_A:
         printf( "Pressed: A\n" );
         break; 

       case XK_Escape:
         break;

       /* Add code here to handle other keys as needed. */
      }
      break;

    default: return;
   }

#else /* Windows */

   switch( trace_data->event->message )
   {
    case WM_LBUTTONDOWN:
      x = LOWORD( trace_data->event->lParam ) + GLG_COORD_MAPPING_ADJ;
      y = HIWORD( trace_data->event->lParam ) + GLG_COORD_MAPPING_ADJ;
      event_type = BUTTON_PRESS_EVENT;
      break;
      
    case WM_MOUSEMOVE:
      x = LOWORD( trace_data->event->lParam ) + GLG_COORD_MAPPING_ADJ;
      y = HIWORD( trace_data->event->lParam ) + GLG_COORD_MAPPING_ADJ;
      event_type = MOUSE_MOVE_EVENT;
      break;

    case WM_SIZE:
      width = LOWORD( trace_data->event->lParam );
      height = HIWORD( trace_data->event->lParam );
      event_type = RESIZE_EVENT;
      break;

    case WM_CHAR:
    case WM_KEYDOWN: 
      switch( trace_data->event->wParam )
      {
       default: break;
       case VK_ESCAPE: /* ESC key  0x1B */
	 /* Place custom code here as needed. */
         break;
       case VK_SHIFT: /* Shift key */
	 break;
	 
	 /* Add custom code to handle other keys as needed. */
      }
    default: return;
   }
#endif
   
   switch( event_type )
   {
    case BUTTON_PRESS_EVENT:
      break;
    case MOUSE_MOVE_EVENT:
      break;
    case RESIZE_EVENT:
      break;
    default: break;
   }
}

/*----------------------------------------------------------------------
| Hierarchy callback, added to the top level viewport and invoked when 
| a new drawing is loaded into any subwindow or subdrawing object. 
| In this demo, this callback processes events only from the reference
| of type "Subwindow" (such as DrawingArea).
*/
void DrawingLoadedCB( GlgObject viewport, GlgAnyType client_data, 
                      GlgAnyType call_data )
{
   GlgHierarchyCBStruct * info_struct;
   GlgObject drawing_vp;
   double obj_type;

   info_struct = (GlgHierarchyCBStruct *) call_data;
   
   /* Handle events only from Subwindow-type reference objects. */
   GlgGetDResource( info_struct->object, "ReferenceType", &obj_type );
   if( obj_type != GLG_SUBWINDOW_REF )
     return;

   /* This callback is invoked twice: one time before hierarchy setup
      for the newly loaded drawing, and the second time, 
      after hierarchy setup. Drawing initialization can be done here
      if needed.
   */
   switch( info_struct->condition )
   {
    case GLG_BEFORE_SETUP_CB:
      drawing_vp = info_struct->subobject;
      if( !drawing_vp )
      {
	 GlgError( GLG_USER_ERROR, "Drawing loading failed" );
         if( info_struct->object == Viewer.DrawingArea )
           Viewer.DrawingAreaVP = NULL;
	 break; 
      }

      /* Set "ProcessMouse" attribute for the loaded viewport, to
	 process custom events and tooltips.
      */
      GlgSetDResource( drawing_vp, "ProcessMouse", 
		       GLG_MOUSE_CLICK | GLG_MOUSE_OVER_TOOLTIP );

      /* Set "OwnsInputCB" attribute for the loaded viewport,
	 so that Input callback is invoked with this viewport ID.
      */
      GlgSetDResource( drawing_vp, "OwnsInputCB", 1. ); 

      if( info_struct->object == Viewer.DrawingArea )
      {
         Viewer.DrawingAreaVP = drawing_vp;
         SetupHMIPage();

         /* Initialize loaded drawing before setup. */
         HMIP_InitBeforeSetup( Viewer.HMIPagePtr );
      }
      break;

    case GLG_AFTER_SETUP_CB:
      /* Initialize loaded drawing after setup. */
      if( info_struct->object == Viewer.DrawingArea )
        HMIP_InitAfterSetup( Viewer.HMIPagePtr );
      break;

    default: break;
   }
}

/*----------------------------------------------------------------------
| Assigns a new page to HMIPagePtr variable, either as a default or 
| custom HMI page, based on the PageType property of the loaded drawing. 
| The HMI page handles the page logic and user interaction.
|
| If the PageType property doesn't exist in the drawing or is set to 
| "Default", DefaultHMIPage is used for the new page.
| Otherwise, a page corresponding to the PageType property is assigned 
| to HMIPage variable. For example, RTChartPage is used if PageType 
| property of the loaded drawing is set to "RealTimeChart".
| 
| The Function also assigns DataFeedPtr for the new page.
*/
void SetupHMIPage()
{ 
   if( Viewer.HMIPagePtr )
   {      
      HMIP_Destroy( Viewer.HMIPagePtr );   /* Destroy previous page, if any. */
      Viewer.HMIPagePtr = NULL;
   }

   Viewer.PageType = GetPageType( Viewer.DrawingAreaVP );   /* Get page type. */

   /* Use live data if requested with the -live-data command-line option, 
      otherwise use simulated random data for testing. 
      RANDOM_DATA may be set to false to use live data by default.
   */
   if( Viewer.RANDOM_DATA )
     Viewer.DataFeedPtr = Viewer.DemoDataFeedPtr;
   else
     Viewer.DataFeedPtr = Viewer.LiveDataFeedPtr;

   switch( Viewer.PageType )
   {
    case UNDEFINED_PAGE_TYPE:
      Viewer.HMIPagePtr = CreateEmptyPage();
      break;

    case DEFAULT_PAGE_TYPE:
    case AERATION_PAGE:
    case CIRCUIT_PAGE:
      Viewer.HMIPagePtr = CreateDefaultHMIPage();
      break;

    case PROCESS_PAGE:
      Viewer.HMIPagePtr = CreateProcessPage();
      break;

    case RT_CHART_PAGE:
      Viewer.HMIPagePtr = CreateRTChartPage();
      break;

    case TEST_COMMANDS_PAGE:
      /* Test page: always use demo data. */
      Viewer.DataFeedPtr = Viewer.DemoDataFeedPtr;
                                            
      Viewer.HMIPagePtr = CreateDefaultHMIPage();
      break;

    default:
      /* New custom page, will use live data if requested with the -live-data 
         command-line option, otherwise use simulated random data for 
         testing. RANDOM_DATA may be set to false to use live data by 
         default.
      */
      Viewer.HMIPagePtr = CreateDefaultHMIPage();
      break;
   }

   /* True if DemoDataFeed is used for the current page. */
   Viewer.RandomData = ( Viewer.DataFeedPtr == Viewer.DemoDataFeedPtr );
}

/*----------------------------------------------------------------------
| Load a new drawing into the DrawingArea when the user selects an item
| from the navigation menu object (Menu object).
*/
GlgBoolean LoadDrawingFromMenu( GlgLong screen )
{
   /* Loads a new drawing into the DrawingArea subwindow object.
      DrawingAreaVP is assigned in the DrawingLoadedCB callback to the 
      ID of the $Widget viewport of the loaded drawing.
   */
   LoadDrawing( Viewer.DrawingArea, Viewer.MenuArray[ screen ].drawing_name );
   if( !Viewer.DrawingAreaVP )
   {
      DeleteTagRecords();
      Viewer.PageType = UNDEFINED_PAGE_TYPE;
      Viewer.HMIPagePtr = CreateEmptyPage();
      return False;
   }
    
   /* Query a list of tags from the loaded drawing and build a new
      TagRecordArray.
   */
   QueryTags( Viewer.DrawingAreaVP );
  
   /* Set title from the selected menu record, etc. */
   SetupLoadedPage( Viewer.MenuArray[ screen ].drawing_title );

   HMIP_Ready( Viewer.HMIPagePtr );
   return GlgTrue;
}

/*----------------------------------------------------------------------*/
void SetupLoadedPage( char * title )
{
   double r, g, b;

   /* Set new Title/ */
   GlgSetSResource( Viewer.MainViewport, "Title", title );

   /* Set background color of the top level window to match the 
      color of the loaded viewport.
   */
   GlgGetGResource( Viewer.DrawingAreaVP, "FillColor", &r, &g, &b );
   GlgSetGResource( Viewer.MainViewport, "FillColor", r, g, b);
}

/*----------------------------------------------------------------------
| Load a new drawing into the specified subwindow object and return 
| a newly loaded viewport. 
*/
GlgObject LoadDrawing( GlgObject subwindow, char * drawing_file )
{
   if( !subwindow || !drawing_file )
   {
      GlgError( GLG_USER_ERROR, "Drawing loading failed" );
      return NULL;
   }

   /* Assign a new drawing name to the subwindow object, 
      if the current drawing name has changed.
   */
   GlgSetSResourceIf( subwindow, "SourcePath", drawing_file, 
		      GlgTrue /* if changed */ );

   /* Setup hierarchy for the subwindow object, which causes the 
      new drawing to be loaded into the subwindow. 
      DrawingLoadedCB will be invoked before and after hierarchy 
      setup for newly loaded drawing. This callback can be used 
      to invoke code for initializing the newly loaded drawing.
   */
   GlgSetupHierarchy( subwindow );

   /* Return newly loaded viewport. */
   return( GlgGetResourceObject( subwindow, "Instance" ) );
}

/*----------------------------------------------------------------------
| Query tags for a given viewport and rebuild TagRecordArray. 
| The new_drawing parameter is the viewport of the new drawing. 
| TagRecordArray will include tags for the top level viewport 
| of the viewer, including tags for the loaded page, as well as
| tags for the popup dialogs, if any.
*/
void QueryTags( GlgObject new_drawing )
{
   /* Delete existing tag records from TagRecordArray */
   DeleteTagRecords();

   /* Remap tags in the loaded drawing if needed.
      Will invoke HMIPage's RemapTagObject() for each tag.
   */
   if( new_drawing != NULL && HMIP_NeedTagRemapping( Viewer.HMIPagePtr ) )
     RemapTags( new_drawing );
    
   /* Build an array of tag records containing tags information and
      store it in TagRecordArray. TagRecordArray will be used for 
      unimating the drawing with data.
      Always create data for the top level viewport (MainViewport),
      to keep a global list of tags that include tags in any dynamically
      loaded dialogs.
   */
   CreateTagRecords( Viewer.MainViewport );
}

/*--------------------------------------------------------------------
| Create an array of tag records containing tag information.
*/
void CreateTagRecords( GlgObject drawing_vp )
{
   GlgObject 
      tag_list,
      tag_obj;
   char 
     * tag_source,
     * tag_name,
     * tag_comment;
   GlgLong 
      i, 
      size;
   double data_type, access_type;

   /* Obtain a list of tags with unique tag sources. */
   tag_list = GlgCreateTagList( drawing_vp, 
                                /* List each tag source only once */ GlgTrue );
   if( !tag_list )
     return;   

   size = GlgGetSize( tag_list );
   if( !size )
   {
      GlgDropObject( tag_list );
      return; /* no tags found */
   }

   Viewer.TagRecordArray = GlgAlloc( sizeof( TagRecord ) * size );

   for( i=0; i<size; ++i )
   {
      tag_obj = GlgGetElement( tag_list, i );

      /* Skip OUTPUT tags. */
      GlgGetDResource( tag_obj, "TagAccessType", &access_type );
      if( access_type == GLG_OUTPUT_TAG )
	continue;

      /* Retrieve TagSource, the name of the database field used as a 
         data source. */
      GlgGetSResource( tag_obj, "TagSource", &tag_source );

      /* Skip undefined tag sources, such as "" or "unset". */
      if( IsUndefined( tag_source ) )
	continue; 

      if( Viewer.RandomData )
      {
         /* For demo purposes only, skip tags that have:
            - TagName contains "Speed" or "Test";
            - TagSource contains "Test";
            - TagComment contains "Test".
            Such tags are present in motor_info.g displayed in the PopupDialog, 
            as well as scada_test_commands.g. The demo shows how to set the 
            value for these tags using commands WriteValue or 
            WriteValueFromWidget. 
         */
         GlgGetSResource( tag_obj, "TagName", &tag_name );
         GlgGetSResource( tag_obj, "TagComment", &tag_comment );
         if( !IsUndefined( tag_name ) && 
             ( strstr( tag_name, "Speed" ) || strstr( tag_name, "Test" ) ) )
           continue;
         if( strstr( tag_source, "Test" ) )
           continue;
         if( !IsUndefined( tag_comment ) && strstr( tag_comment, "Test" ) )
           continue;
      }

      /* Get tag object's data type: GLG_D, GLG_S or GLG_G */
      GlgGetDResource( tag_obj, "DataType", &data_type );

      /* Create a new tag record. */
      Viewer.TagRecordArray[ Viewer.NumTagRecords ].tag_source = 
        GlgStrClone( tag_source );
      Viewer.TagRecordArray[ Viewer.NumTagRecords ].data_type = data_type;
      Viewer.TagRecordArray[ Viewer.NumTagRecords ].tag_obj = tag_obj;

      /* Set plot_time_ep to NULL by default. It will be assigned
         in SetPlotTimeEP() as needed.
      */
      Viewer.TagRecordArray[ Viewer.NumTagRecords ].plot_time_ep = NULL;

      ++Viewer.NumTagRecords;
   }

   if( !Viewer.NumTagRecords )     /* No input tags were found. */
   {
      GlgFree( Viewer.TagRecordArray );
      Viewer.TagRecordArray = NULL;
   }
   
   /* If a drawing contains a chart, ValueEntryPoint of each plot 
      may have a tag to push data values using the common tag mechanism. 
      The time stamp for the corresponding TimeEntryPoint of the plot
      may be supplied either automatically by the chart 
      (SUPPLY_PLOT_TIME_STAMP=0), or the application may supply the
      time stamp explicitly for each data sample (SUPPLY_PLOT_TIME_STAMP=1).

      If SUPPLY_PLOT_TIME_STAMP=1, each tag record in TagRecordArray
      should store corresponding plot's TimeEntryPoint (plot_time_ep).
      If not found, plot_time_ep will be NULL.
   */
   if( SUPPLY_PLOT_TIME_STAMP )
     SetPlotTimeEP( drawing_vp );

   /* Dereference tag_list object */
   GlgDropObject( tag_list );
}

/*-----------------------------------------------------------------------
| Clear TagRecordArray
*/
void DeleteTagRecords()
{
   GlgLong i;

   if( !Viewer.NumTagRecords )
      return;

   /* Free memory for the tag_source */
   for( i=0; i<Viewer.NumTagRecords; ++i )
     GlgFree( Viewer.TagRecordArray[i].tag_source );
   
   /* Free memory allocated for the TagRecordArray */
   GlgFree( Viewer.TagRecordArray );
   
   Viewer.TagRecordArray = NULL;
   Viewer.NumTagRecords = 0;
}

/*----------------------------------------------------------------------
| Store TimeEntryPoint (plot_time_ep) in each tag record, if found.
*/
void SetPlotTimeEP( GlgObject drawing_vp )
{
   GlgObject 
     tag_list,
     tag_obj,
     plot_time_ep;
   GlgLong 
     i, 
     size;
   char 
     * tag_comment,
     * tag_source;
   TagRecord * tag_record;
 
   if( !Viewer.NumTagRecords )
     return;

   /* Obtain a list of all tags, including non-unique tag sources. */
   tag_list = GlgCreateTagList( drawing_vp, /* List all tags */ GlgFalse );
   if( !tag_list )
     return;   

   size = GlgGetSize( tag_list );
   if( !size )
   {
      GlgDropObject( tag_list );
      return; /* no tags found */
   }

   /* For each tag in the list, check if there is a plot object in the
      drawing with a matching TagSource for the plot's ValueEntryPoint.
      If found, obtain plot's TimeEntryPoint and store it in the
      TagRecordArray for a tag record with a matching tag_source.
   */
   for( i=0; i<size; ++i )
   {
      tag_obj = GlgGetElement( tag_list, i );

      /* Retrieve TagSource and TagComment. In the demo, TagComment is
         not used, but the application may use it as needed.
      */
      GlgGetSResource( tag_obj, "TagComment", &tag_comment );
      GlgGetSResource( tag_obj, "TagSource", &tag_source );

      if( IsUndefined( tag_source ) )
        return;

      /* Find a TimeEntryPoint of a plot in a RealTimeChart with
         a matching TagSource assigned for the plot's ValueEntryPoint.
         It is assumed that there is ONLY ONE plot in the drawing 
         with a given TagSource. 
      */
      plot_time_ep = FindMatchingTimeEP( tag_obj );
      
      if( !plot_time_ep )
        continue;   /* There is no plot for this tag source. */
      
      /* Find a tag record in TagRecordArray with a matching tag_source.
         If found, assign plot_time_ep. Otherwise, generate an error.
      */
      tag_record = LookupTagRecords( tag_source );
      
      if( tag_record )
        tag_record->plot_time_ep = plot_time_ep;
      else /* shouldn't happen */
        GlgError( GLG_USER_ERROR, 
                  "No matching tag record, TimeEntryPoint not stored." );
   }

   /* Dereference tag_list after it is no longer needed. */
   GlgDropObject( tag_list );
}

/*----------------------------------------------------------------------
|  For a given tag object, find a parent plot object (GLG_PLOT object type).
|  If found, return the plot's TimeEntryPoint.
|  It is assumed that there is ONLY ONE plot in the drawing with a given
|  TagSource. 
*/
GlgObject FindMatchingTimeEP( GlgObject tag_obj )
{
   GlgObject plot;

   /* Fill in match data structure to search for a plot object type,
      which is a parent of the tag_obj (ValueEntryPoint) .
   */
   GlgFindMatchingObjectsData match_data;

   match_data.match_type = GLG_OBJECT_TYPE_MATCH;
   match_data.find_parents = GlgTrue;
   match_data.find_first_match = GlgTrue;
   match_data.search_inside = GlgFalse;
   match_data.object_type = GLG_PLOT;

   if( !GlgFindMatchingObjects( tag_obj, &match_data ) || 
       !match_data.found_object )
     return NULL; /* matching object not found */

   plot = match_data.found_object;
   return GlgGetResourceObject( plot, "TimeEntryPoint" );
}

/*----------------------------------------------------------------------
| Lookup TagRecordArray and return a matching tag record with 
| tag_source=match_tag_source.
*/
TagRecord * LookupTagRecords( char * match_tag_source )
{
   GlgLong i;
   TagRecord * tag_record;

   if( IsUndefined( match_tag_source ) || !Viewer.NumTagRecords )
     return NULL;

   for( i=0; i<Viewer.NumTagRecords; ++i )
   {
      tag_record = &Viewer.TagRecordArray[ i ];
      if( strcmp( tag_record->tag_source, match_tag_source ) == 0 )
        return tag_record;
   }

   return NULL;    /* not found */
}

/*----------------------------------------------------------------------
| Close all active popups, including popup dialogs and popup menus.
*/
void CloseActivePopups( DialogType allow_dialog )
{
   int i;

   /* Close all active dialogs, except the ones which may remain
      visible until closed by the user, as specified by the allow_dialog 
      parameter.
   */
   for( i=0; i<MAX_DIALOG_TYPE; ++i )
   {
      if( i == allow_dialog )
	continue; 

      /* Close a dialog of a given type, if any. */
      ClosePopupDialog( i );
   }
   
   /* Close Global PopupMenu, if any. */
   ClosePopupMenu( GLOBAL_POPUP_MENU );
}

/*----------------------------------------------------------------------
| Process commands attached to objects at design time.
| Command types enums and strings are defined in CommandTypeTable in util.c. 
| CommandType strings in the table must match the CommandType strings 
| defined in the drawing.
*/
void ProcessObjectCommand( GlgObject command_vp, GlgObject selected_obj, 
			   GlgObject action_obj )
{
   GlgObject command_obj;
   CommandType command_type;
   DialogType dialog_type;
   PopupMenuType menu_type;

   if( !selected_obj || !action_obj )
     return;

   /* Retrieve Command object. */
   command_obj = GlgGetResourceObject( action_obj, "Command" );
   if( !command_obj )
     return;

   command_type = GetCommandType( command_obj );
   switch( command_type )
   {
    case SHOW_ALARMS:
      ShowAlarms();
      break;
    case POPUP_DIALOG:
      DisplayPopupDialog( command_vp, selected_obj, command_obj );
      break;
    case CLOSE_POPUP_DIALOG:
      dialog_type = GetDialogType( command_obj );
      ClosePopupDialog( dialog_type );
      break;
    case POPUP_MENU:
      DisplayPopupMenu( command_vp, selected_obj, command_obj );
      break;
    case CLOSE_POPUP_MENU:
      menu_type = GetPopupMenuType( command_obj );
      ClosePopupMenu( menu_type );
      break;
    case GOTO:
      StopUpdates();
      GoTo( command_vp, selected_obj, command_obj );
      StartUpdates();
     break;
    case WRITE_VALUE:
      WriteValue( command_vp, selected_obj, command_obj );
      break;
    case WRITE_VALUE_FROM_WIDGET:
      WriteValueFromInputWidget( command_vp, selected_obj, command_obj );
      break;
    case QUIT:
      exit( GLG_EXIT_OK );
      break;
    default: 
      GlgError( GLG_USER_ERROR, "Command failed: Undefined CommandType." );
      break;
   }
}

/*----------------------------------------------------------------------
| Opens or closes the alarm window.
*/
void ShowAlarms()
{
   static GlgBoolean FirstAlarmDialog = GlgTrue;

   Viewer.AlarmDialogVisible = ToggleResource( "AlarmDialog/Visibility" );
      
   /* If the alarm dialog is becoming visible, fill alarms to show them
      right away.
   */
   if( Viewer.AlarmDialogVisible )
     ProcessAlarms( GlgTrue );

   if( FirstAlarmDialog )   /* Show the help message the first time only. */
   {         
      ShowMessageDialog( "Ctrl-click on the alarm row to acknowledge "
                         "an alarm.", GlgFalse );
      FirstAlarmDialog = GlgFalse;
   }

   GlgUpdate( Viewer.MainViewport );
}

/*----------------------------------------------------------------------*/
void ShowMessageDialog( char * message, GlgBoolean error )
{
   GlgSetSResource( Viewer.MessageDialog, "MessageString", message );
   
   /* Set to 1. to highlight the message in red. */
   GlgSetDResource( Viewer.MessageDialog, "ShowErrorColor", error ? 1. : 0. );
   
   GlgSetDResource( Viewer.MessageDialog, "Visibility", 1. );
}

/*----------------------------------------------------------------------
| Toggles resource value between 0 and 1 and returns the new value
| as a boolean.
*/
GlgBoolean ToggleResource( char * resource_name )
{
   double current_value = 0.;

   GlgGetDResource( Viewer.MainViewport, resource_name, &current_value );

   current_value = ( !current_value ? 1. : 0. );

   GlgSetDResource( Viewer.MainViewport, resource_name, current_value );
   return ( current_value == 1.0 );    
}

/*----------------------------------------------------------------------
| Process command "PopupDialog". The requested dialog can be embedded 
| into the currently loaded drawing, or a global PopupDialog can be used 
| as a popup. Command parameters DialogType and DialogResource are
| mandatory. DialogResource specifies the resource path of the dialog 
| to be displayed. If DialogResource starts with a forward slash ('/'), 
| the dialog name is relative to the top level viewport MainViewport; 
| otherwise, it is relative to the currently loaded viewport where the 
| command occurred, i.e. command_vp.
|
| The popup dialog is initialized as needed, using parameters of the 
| selected object. In this demo, the tag sources are transfered from the 
| selected object to the dialog viewport for the tags with a matching
| TagName.
*/
void DisplayPopupDialog( GlgObject command_vp, GlgObject selected_obj, 
			 GlgObject command_obj )
{
   char 
     * title = NULL,
     * drawing_file = NULL,
     * dialog_res = NULL,
     * destination_res = NULL,
     * subwindow_name = NULL;

   DialogType dialog_type;
   GlgObject 
     subwindow = NULL, 
     popup_vp = NULL,
     dialog = NULL;

   /* Retrieve command parameters. */
   GlgGetSResource( command_obj, "DialogResource", &dialog_res );
   GlgGetSResource( command_obj, "DrawingFile", &drawing_file );
   GlgGetSResource( command_obj, "Destination", &destination_res );
   GlgGetSResource( command_obj, "Title", &title );

   /* Obtain DialogType. */
   dialog_type = GetDialogType( command_obj );
   if( dialog_type == UNDEFINED_DIALOG_TYPE )
   {
      GlgError( GLG_USER_ERROR, 
                "PopupDialog Command failed: Unknown DialogType." );
      return;
   }
   
   /* Close active popups, if any. To avoid flickering, do not close the 
      dialog with the same dialog type that is requested by this command.
   */
   CloseActivePopups( dialog_type );
 
   /* DialogResource specifies resource path of the dialog to be displayed.
      If the path starts with '/', it is relative to the top level 
      $Widget viewport. Otherwise, the path is relative to the viewport 
      of the Input callback (command_vp).
   */
   if( IsUndefined( dialog_res ) )
   {
      GlgError( GLG_USER_ERROR, 
		"PopupDialog Command failed: Invalid DialogResource." );
      ClosePopupDialog( dialog_type );
      return;
   }
   
   /* Obtain an object ID of the requested popup dialog. 
      If invalid, abort the command. 
   */
   if( StartsWith( dialog_res, "/" ) )
     dialog = GlgGetResourceObject( Viewer.MainViewport, dialog_res+1 );
   else 
     dialog = GlgGetResourceObject( command_vp, dialog_res );

   if( !dialog )
   {
      GlgError( GLG_USER_ERROR, 
		"PopupDialog Command failed: Dialog not found." );
      ClosePopupDialog( dialog_type );
      return;
   }

   if( IsUndefined( drawing_file ) )
     /* DrawingFile is not defined, use dialog as a popup viewport. */
     popup_vp = dialog;

   /* If DrawingFile is present, load the corresponding drawing into 
      the subwindow object defined by the Destination parameter. 
   */
   else
   {
      /* Use Destination resource, if any, to display the specified drawing. 
	 If omitted, use default name "DrawingArea". It is assumed that 
	 Destination points to the subwindow object inside a dialog.
      */
      if( IsUndefined( destination_res ) )
	subwindow_name = "DrawingArea"; /* Use default name. */
      else
	subwindow_name = destination_res;

      /* Obtain an object ID of the subwindow inside a dialog. */
      subwindow = GlgGetResourceObject( dialog, subwindow_name );
      if( !subwindow )
      {
	 GlgError( GLG_USER_ERROR, 
		"PopupDialog Command failed: Destionation object not found." );
         ClosePopupDialog( dialog_type );
	 return;
      }

      /* Load new drawing and obtain an object id of the newly loaded viewport. 
	 If drawing loading failed, the error gets reported in the callback
	 DrawingLoadedCB.
      */
      popup_vp = LoadDrawing( subwindow, drawing_file );
      if( !popup_vp )
      {
	 /* If drawing loading fails, it will be reported in DrawingLoadCB.
	    Generate an additional error indicating command failing.
	 */
	 GlgError( GLG_USER_ERROR, "PopupDialog Command failed." );
         ClosePopupDialog( dialog_type );
	 return;
      }
   }

   /* For the tags with matching TagName, transfer tag sources from the 
      selected object to the loaded popup viewport.
   */
   TransferTags( selected_obj, popup_vp, GlgFalse );
   
   /* If a new dialog drawing was loaded, rebuild TagRecordArray to 
      include tags both for the drawing displayed in the main drawing area 
      and drawing displayed in the popup dialog.
   */
   if( popup_vp != dialog )
     QueryTags( popup_vp );
   
   /* Poll new data to fill the popup dialog with current values. */
   UpdateData();
   
   /* Display title in the loaded viewport, if Title resource is found. */
   if( GlgHasResourceObject( popup_vp, "Title" ) )
     GlgSetSResource( popup_vp, "Title", title );
   
   /* Display title as the dialog caption. */
   GlgSetSResource( dialog, "ScreenName", title );
   
   /* Display the dialog if it is not up already. */
   GlgSetDResource( dialog, "Visibility", 1. );

   /* Store dialog information in ActiveDialogs array */
   Viewer.ActiveDialogs[ dialog_type ].dialog_type = dialog_type;
   Viewer.ActiveDialogs[ dialog_type ].dialog = GlgReferenceObject( dialog );
   Viewer.ActiveDialogs[ dialog_type ].subwindow = 
     GlgReferenceObject( subwindow );
   Viewer.ActiveDialogs[ dialog_type ].popup_vp = GlgReferenceObject( popup_vp );
   Viewer.ActiveDialogs[ dialog_type ].isVisible = GlgTrue;
   
   GlgUpdate( Viewer.MainViewport );
   GlgSync( Viewer.MainViewport );    /* Improves interactive response. */
}

/*----------------------------------------------------------------------
| Close active popup dialog of a given type.
*/
void ClosePopupDialog( DialogType dialog_type )
{
   if( dialog_type == UNDEFINED_DIALOG_TYPE || 
       dialog_type >= MAX_DIALOG_TYPE )
   {
      GlgError( GLG_USER_ERROR, "Dialog closing failed." ); 
      return;
   }

   if( !Viewer.ActiveDialogs[ dialog_type ].dialog )
     return; /* nothing to do. */

   if( Viewer.ActiveDialogs[ dialog_type ].subwindow && 
       Viewer.ActiveDialogs[ dialog_type ].popup_vp )
   {
      /* Destroy currently loaded popup drawing and load empty drawing. */
      LoadDrawing( Viewer.ActiveDialogs[ dialog_type ].subwindow, 
                   "empty_drawing.g" );

      /* Rebuild a list of tags to exclude the tags from the previously
	 loaded popup viewport.
      */
      QueryTags( NULL );
   }

   /* Hide the dialog */
   GlgSetDResource( Viewer.ActiveDialogs[ dialog_type ].dialog, 
                    "Visibility", 0. );
   
   /* Clear a dialog record with a specified index (dialog_type)
      in the ActiveDialogs array.
   */
   GlgDropObject( Viewer.ActiveDialogs[ dialog_type ].dialog );
   GlgDropObject( Viewer.ActiveDialogs[ dialog_type ].subwindow );   
   GlgDropObject( Viewer.ActiveDialogs[ dialog_type ].popup_vp );

   Viewer.ActiveDialogs[ dialog_type ].dialog_type = UNDEFINED_DIALOG_TYPE;
   Viewer.ActiveDialogs[ dialog_type ].dialog = NULL;
   Viewer.ActiveDialogs[ dialog_type ].subwindow = NULL;
   Viewer.ActiveDialogs[ dialog_type ].popup_vp = NULL;
   Viewer.ActiveDialogs[ dialog_type ].isVisible = GlgFalse;
}

/*----------------------------------------------------------------------
| Transfer tag sources for tags with a matching TagName from the
| selected object to the specified viewport.
*/
void TransferTags( GlgObject selected_obj, GlgObject viewport, 
		   GlgBoolean unset_tags )
{
   char 
     * widget_type,
     * tag_name,
     * tag_source;

   GlgObject 
     tag_list,
     tag_obj;

   GlgLong
     size,
     i,
     num_remapped_tags;
   
   /* Retrieve WidgetType from the selected objects, if any. */
   if( GlgHasResourceObject( selected_obj, "WidgetType" ) )
     GlgGetSResource( selected_obj, "WidgetType", &widget_type );
   
   /* Place custom code here to initialize the drawing based on WidgetType, 
      if needed. In this demo, the initialization code below is executed 
      regardless of WidgetType.
   */

   /* Obtain a list of tags defined in the selected object. */
   tag_list = GlgCreateTagList( selected_obj, /* List all tags */ False );  
   if( !tag_list )
     return;
   
   size = GlgGetSize( tag_list );
   if( !size )
   {
     GlgDropObject( tag_list );
     return; /* no tags found */
   }
   
   /* Traverse the tag list. For each tag, transfer the TagSource
      defined in the selected object to the tag in the loaded 
      popup drawing that has a matching TagName.
   */
   for( i=0; i<size; ++i )
   {
      tag_obj = GlgGetElement( tag_list, i );
      
      /* Obtain TagName. */
      GlgGetSResource( tag_obj, "TagName", &tag_name );

      /* Skip tags with undefined TagName */
      if( IsUndefined( tag_name ) )
	continue;

      /* Obtain TagSource. */
      GlgGetSResource( tag_obj, "TagSource", &tag_source );
      
      /* Skip tags with undefined TagSource. */
      if( IsUndefined( tag_source ) )
	continue;
      
      /* Remap all tags with the specified tag name (tag_name)
	 to use a new tag source (tag_source).
      */
      if( unset_tags )
	num_remapped_tags = 
	  RemapNamedTags( viewport, tag_name, "unset" );
      else
	num_remapped_tags = 
	  RemapNamedTags( viewport, tag_name, tag_source );
      
      if( Viewer.RandomData )
      {
         /* For demo purposes only, initialize input tags with 
            TagName="Speed" present for motor objects in 
            scada_aeration.g and scada_motor_info.g.
         */
         if( strcmp( tag_name, "Speed" ) == 0 )
           GlgSetDTag( Viewer.MainViewport, tag_source, GlgRand( 300., 1500. ), 
                       GlgTrue );
      }
   }
   
   /* Dereference tag_list object. */
   GlgDropObject( tag_list );
}
   
/*----------------------------------------------------------------------
| Process command "PopupMenu". The requested popup menu can be embedded 
| in the currently loaded drawing, or a global PopupMenu can be used 
| as a popup. Command parameters MenuType and MenuResource are
| mandatory. MenuResource specifies the resource path of the menu object 
| to be displayed. If MenuResource starts with a forward slash ('/'), 
| the menu name is relative to the top level viewport MainViewport; 
| otherwise, it is relative to the currently loaded viewport where the 
| command occurred, i.e. command_vp.
|
| The popup menu is initialized as needed, using parameters of the 
| selected object. In this demo, the tag sources are transfered from the 
| selected object to the menu viewport for the tags with a matching
| TagName.
*/
void DisplayPopupMenu( GlgObject command_vp, GlgObject selected_obj, 
		       GlgObject command_obj )
{
   PopupMenuType menu_type;
   GlgObject 
     subwindow = NULL, 
     popup_vp = NULL,
     menu_obj = NULL;
   double 
     menu_width, 
     menu_height;
   char 
     * title = NULL,
     * drawing_file = NULL,
     * menu_res = NULL,
     * destination_res = NULL,
     * subwindow_name = NULL;

   /* Retrieve command parameters. */
   GlgGetSResource( command_obj, "MenuResource", &menu_res );
   GlgGetSResource( command_obj, "DrawingFile", &drawing_file );
   GlgGetSResource( command_obj, "Destination", &destination_res );
   GlgGetSResource( command_obj, "Title", &title );

   /* Obtain MenuType. */
   menu_type = GetPopupMenuType( command_obj );
   if( menu_type == UNDEFINED_POPUP_MENU_TYPE )
   {
      GlgError( GLG_USER_ERROR, "PopupMenu Command failed: Unknown MenuType." );
      return;
   }
   
   /* Close active popups, if any. */
   CloseActivePopups( CLOSE_ALL );
 
   /* MenuResource specifies resource path of the menu object to be displayed.
      If the path starts with '/', it is relative to the top level 
      $Widget viewport. Otherwise, the path is relative to the viewport 
      of the Input callback (command_vp).
   */
   if( IsUndefined( menu_res ) )
   {
      GlgError( GLG_USER_ERROR, 
		"Invalid MenuResource, PopupMenu Command failed." );
      return;
   }
   
   /* Obtain an object ID of the requested popup menu. 
      If invalid, abort the command. 
   */
   if( StartsWith( menu_res, "/" ) )
     menu_obj = GlgGetResourceObject( Viewer.MainViewport, menu_res+1 );
   else 
     menu_obj = GlgGetResourceObject( command_vp, menu_res );

   if( !menu_obj )
   {
      GlgError( GLG_USER_ERROR, 
		"PopupMenu Command failed: Menu object not found." );
      return;
   }

   if( IsUndefined( drawing_file ) )
     /* DrawingFile is not defined, use menu_obj as popup viewport. */
     popup_vp = menu_obj;
   else
   {
      /* DrawingFile is defined, use it to load the corresponding 
	 drawing into the subwindow object defined by the Destination 
	 parameter. It is assumed that Destination points to the subwindow 
	 object inside the menu object.
      */
      if( IsUndefined( destination_res ) )
	subwindow_name = "DrawingArea"; /* Use default name. */
      else
	subwindow_name = destination_res;

      /* Obtain an object ID of the subwindow inside the menu object. */
      subwindow = GlgGetResourceObject( menu_obj, subwindow_name );
      if( !subwindow )
      {
	 GlgError( GLG_USER_ERROR, 
	   "PopupDialog Command failed: Destionation object not found." );
	 return;
      }

      /* Load new drawing and store an object id of the newly loaded viewport. 
	 If drawing loading failed, the error gets reported in the 
	 DrawingLoadedCB callback.
      */
      popup_vp = LoadDrawing( subwindow, drawing_file );
      if( !popup_vp )
      {
	 GlgError( GLG_USER_ERROR, "PopupMenu Command failed." );
	 return;
      }

      /* If the viewport has Width and Height resources that define
	 its size in pixels, adjust the size of the menu object 
	 to match the size of the loaded viewport.
      */
      if( GlgHasResourceObject( popup_vp, "Width" ) &&
	  GlgHasResourceObject( menu_obj, "Width" ) )
      {
	 GlgGetDResource( popup_vp, "Width", &menu_width );
	 GlgSetDResource( menu_obj, "Width", menu_width );
      } 

      if( GlgHasResourceObject( popup_vp, "Height" ) &&
	  GlgHasResourceObject( menu_obj, "Height" ) )
      {
	 GlgGetDResource( popup_vp, "Height", &menu_height );
	 GlgSetDResource( menu_obj, "Height", menu_height );
      } 
   }

   /* Transfer tag sources from the selected object to the loaded 
      popup viewport, using tags with a matching TagName.
   */
   TransferTags( selected_obj, popup_vp, GlgFalse );
      
   /* Display title in the loaded viewport, if Title resource is found. */
   if( GlgHasResourceObject( popup_vp, "Title" ) )
     GlgSetSResource( popup_vp, "Title", title );

   /* Show the menu. */
   GlgSetDResource( menu_obj, "Visibility", 1. );

   /* Store menu information in the global ActivePopupMenu structure, 
      used to close the active popup menu.
   */
   Viewer.ActivePopupMenu.menu_type = menu_type;
   Viewer.ActivePopupMenu.menu_obj = GlgReferenceObject( menu_obj );
   Viewer.ActivePopupMenu.subwindow = GlgReferenceObject( subwindow );
   Viewer.ActivePopupMenu.menu_vp = GlgReferenceObject( popup_vp );
   Viewer.ActivePopupMenu.selected_obj = GlgReferenceObject( selected_obj ); 
   Viewer.ActivePopupMenu.isVisible = GlgTrue; 
      
   /* Position the menu next to the selected object. */
   PositionPopupMenu();

   GlgUpdate( menu_obj );
}

/*----------------------------------------------------------------------
| Position ActivePopupMenu at the upper right corner of the selected object,
| if possible. Otherwise, position the menu close to the selected object
| such that it is displayed within the current viewport.
*/
void PositionPopupMenu()
{
   GlgObject 
     selected_obj_vp, /* Viewport that contains selected object. */
     menu_parent_vp;  /* Parent viewport that contains the popup menu. */
   GlgCube
     * sel_obj_box,
     converted_box,
     * menu_obj_box;
   double 
     x, y,
     offset = 5., /* offset in pixels. */
     menu_width, menu_height,
     parent_width, parent_height;
   int 
     x_anchor, 
     y_anchor; 
   
   if( !Viewer.ActivePopupMenu.selected_obj || 
       !Viewer.ActivePopupMenu.menu_obj )
     return;

   selected_obj_vp = 
     GlgGetParentViewport( Viewer.ActivePopupMenu.selected_obj, True );
   menu_parent_vp = 
     GlgGetParentViewport( Viewer.ActivePopupMenu.menu_obj, True );

   /* Obtain the object's bounding box in screen coordinates. */
   sel_obj_box = GlgGetBoxPtr( Viewer.ActivePopupMenu.selected_obj );   
   converted_box = *sel_obj_box;

   /* If the menu is located in a different viewport from the viewport
      of the selected object, convert screen coordinates of the 
      selected object box from the viewport of the selected object to the 
      viewport that contains the popup menu.
   */
   if( selected_obj_vp != menu_parent_vp )
   {
      GlgTranslatePointOrigin( selected_obj_vp, menu_parent_vp, 
			       &converted_box.p1 );
      GlgTranslatePointOrigin( selected_obj_vp, menu_parent_vp, 
			       &converted_box.p2 );
   }

   /* Obtain width and height in pixels of the parent viewport of the menu. */
   GlgGetDResource( menu_parent_vp, "Screen/Width", &parent_width );
   GlgGetDResource( menu_parent_vp, "Screen/Height", &parent_height );

   /* Obtain width and height of the menu object. */
   menu_obj_box = GlgGetBoxPtr( Viewer.ActivePopupMenu.menu_obj ); 
   menu_width = menu_obj_box->p2.x - menu_obj_box->p1.x;
   menu_height = menu_obj_box->p2.y - menu_obj_box->p1.y;
   
   /* Position the popup at the upper right or lower left corner of 
      the selected object, if possible. Otherwise (viewport is too small), 
      position it in the center of the viewport.
   */   
   if( converted_box.p2.x + menu_width > parent_width )
   {
      /* Outside of window right edge. Position right edge of the popup
         to the left of the selected object. Always use GLG_HLEFT anchor
         to simplify out-of-the-window check.
      */
      x =  converted_box.p1.x - offset - menu_width;
      x_anchor = GLG_HLEFT;
   }
   else 
   {
      /* Position left edge of the popup to the right of the selected object. */
      x = converted_box.p2.x + offset; 
      x_anchor = GLG_HLEFT;
   }

   /* Anchor is always GLG_HLEFT here to make checks simpler. */
   if( x < 0 || x + menu_width > parent_width )
   {
      /* Not enough space: place in the center. */
      x = parent_width / 2.;
      x_anchor = GLG_HCENTER;
   }

   if( converted_box.p1.y - menu_height < 0. ) 
   {
      /* Outside of window top edge.
	 Position the top edge of the popup below the selected object.
      */
      y =  converted_box.p2.y + offset;
      y_anchor = GLG_VTOP;
   }
   else 
   {
      /* Position bottom edge of the popup above the selected object.
	 Always use GLG_VTOP achor to simplify out-of-the-window check.
      */
      y = converted_box.p1.y - offset - menu_height; 
      y_anchor = GLG_VTOP;
   }

   /* Anchor is always GLG_VTOP here to make checks simpler. */
   if( y < 0 || y + menu_height > parent_height )
   {
      /* Not enough space: place in the center. */
      y = parent_height / 2.;
      y_anchor = GLG_HCENTER;
   }

   GlgPositionObject( Viewer.ActivePopupMenu.menu_obj, 
		      GLG_SCREEN_COORD, x_anchor | y_anchor, x, y, 0. );
}

/*----------------------------------------------------------------------
| Close active popup menu. In this demo, menu_type is not used, since
| there is only a single ActivePopupMenu object. The code can be 
| extended by the application developer as needed.
*/
void ClosePopupMenu( PopupMenuType menu_type )
{
   if( !Viewer.ActivePopupMenu.menu_obj )
     return; /* Nothing to do. */

   /* Hide active popup. */
   GlgSetDResource( Viewer.ActivePopupMenu.menu_obj, "Visibility", 0. );

   if( Viewer.ActivePopupMenu.subwindow )
   {
      /* Destroy currently loaded popup drawing and load empty drawing. */
      LoadDrawing( Viewer.ActivePopupMenu.subwindow, "empty_drawing.g" );
   }
   else
   {
      /* Unset tags in the menu object, which were previously
	 transfered and assigned from the selected object. 
      */ 
      if( Viewer.ActivePopupMenu.selected_obj )
	TransferTags( Viewer.ActivePopupMenu.selected_obj, 
		      Viewer.ActivePopupMenu.menu_obj, GlgTrue );
   }
   
   /* Clear menu record. */
   GlgDropObject( Viewer.ActivePopupMenu.menu_obj );
   GlgDropObject( Viewer.ActivePopupMenu.subwindow );
   GlgDropObject( Viewer.ActivePopupMenu.menu_vp );
   GlgDropObject( Viewer.ActivePopupMenu.selected_obj );

   Viewer.ActivePopupMenu.menu_type = UNDEFINED_POPUP_MENU_TYPE;
   Viewer.ActivePopupMenu.menu_obj = NULL;
   Viewer.ActivePopupMenu.subwindow = NULL;
   Viewer.ActivePopupMenu.menu_vp = NULL;
   Viewer.ActivePopupMenu.selected_obj = NULL;
   Viewer.ActivePopupMenu.isVisible = GlgFalse;
}

/*----------------------------------------------------------------------
| Process "GoTo" command. The command loads a new drawing specified
| by the DrawingFile parameter into the subwindow object specified
| by the Destination parameter. If Destination is omitted, uses
| main DrawingArea subwindow object.  
*/
void GoTo( GlgObject command_vp, GlgObject selected_obj, 
	   GlgObject command_obj )
{
   GlgObject subwindow;
   GlgObject drawing_vp;
   char 
     * drawing_file,
     * destination_res,
     * title;

   /* Close active popup dialogs and popup menu. */
   CloseActivePopups( CLOSE_ALL );
   
   /* Retrieve command parameters. */
   GlgGetSResource( command_obj, "DrawingFile", &drawing_file );
   GlgGetSResource( command_obj, "Destination", &destination_res );
   GlgGetSResource( command_obj, "Title", &title );
   
   /* If DrawingFile is not valid, abort the command. */
   if( !drawing_file || !*drawing_file )
   {
      GlgError( GLG_USER_ERROR, "GoTo Command failed: Invalid DrawingFile." );
      return;
   }

   /* Use Destination resource, if any, to display the specified drawing. 
      It is assumed that Destination points to the subwindow object.
      If not defined, use top level DrawingArea subwindow by default.
   */
   if( !destination_res || !*destination_res ) 
     subwindow = Viewer.DrawingArea;
   else
   {
      if( StartsWith( destination_res, "/" ) )
        /* Destination is relative to the top level viewport (MainViewport).
           Omit the first '/' when using the resource path to obtain the 
           subwindow object.
        */
	subwindow = 
          GlgGetResourceObject( Viewer.MainViewport, destination_res + 1 );
      else 
	/* Destination is relative to the current viewport, where the
	   command occurred.
	*/
	subwindow = GlgGetResourceObject( command_vp, destination_res );
   
      if( !subwindow )
      {
	 GlgError( GLG_USER_ERROR, 
		   "GoTo Command failed: Invalid Destionation object." );
	 return;
      }
   }

   /* Load new drawing and obtain an object id of the newly loaded viewport. 
      If drawing loading failed, the error gets reported in the callback
      DrawingLoadedCB.
   */
   drawing_vp = LoadDrawing( subwindow, drawing_file );
   if( !drawing_vp )
   {
      /* If drawing loading fails, it will be reported in DrawingLoadCB.
	 Generate an additional error indicating command failing.
      */
      GlgError( GLG_USER_ERROR, "GoTo Command failed." );
      return;
   }

   /* Rebuild TagRecordArray for the newly loaded drawing. */
   QueryTags( drawing_vp );

   if( subwindow == Viewer.DrawingArea )
   {
      int screen_index;

      /* Reset main menu selection. If the new drawing matches one of the 
	 drawings defined in the MenuArray, update main menu using
	 the corresponding menu index.
      */
      screen_index = LookUpMenuArray( drawing_file );
      SelectMainMenuItem( screen_index, GlgTrue );
      
      SetupLoadedPage( title );   /* Use title from the command, etc. */

      HMIP_Ready( Viewer.HMIPagePtr );
   }
}

/*----------------------------------------------------------------------
| Process command "WriteValue". The command writes a new value specified
| by the Value parameter into the tag in the back-end system
| specfied by the OutputTagHolder.
*/
void WriteValue( GlgObject command_vp, GlgObject selected_obj, 
		 GlgObject command_obj )
{
   char * tag_source;
   double value;

   /* Retrieve tag source to write data to. */
   GlgGetSResource( command_obj, "OutputTagHolder/TagSource", &tag_source );

   /* Validate. */
   if( IsUndefined( tag_source ) )
   {
      GlgError( GLG_USER_ERROR, 
		"WriteValue Command failed: Invalid TagSource." );
      return;
   }

   /* Retrieve the value to be written to the tag source. */
   GlgGetDResource( command_obj, "Value", &value );
   
   /* Place custom code here as needed, to validate the value specified
      in the command.
   */

   /* Write new value to the specified tag source. */
   DF_WriteDTagData( Viewer.DataFeedPtr, tag_source, value );

   if( Viewer.RandomData )
   {
      /* For demo purposes, update the tag value in the drawing. 
         In an application, the read tag will be updated from 
         the back-end system.
      */
      GlgSetDTag( Viewer.MainViewport, tag_source, value, GlgTrue );
      GlgUpdate( Viewer.MainViewport );
   }
}

/*----------------------------------------------------------------------
| Process command "WriteValueFromWidget". The command allows writing
| a new value into the tag in the back-end system using an input
| widget, such as a toggle or a spinner.
*/
void WriteValueFromInputWidget( GlgObject command_vp, GlgObject selected_obj, 
				GlgObject command_obj )
{
   
   GlgObject write_tag_obj, read_tag_obj;
   double value;
   char 
     * output_tag_source,
     * value_res;

   char * read_tag_source;

   GlgGetSResource( command_obj, "ValueResource", &value_res );
   
   /* Obtain object ID of the read tag object. FOr example, in case of a
      spinner widget, it will be "Value" resource.
   */
   read_tag_obj = GlgGetResourceObject( selected_obj, value_res );
   if( !read_tag_obj )
     return;

   /* Obtain object ID of the write tag object. */
   write_tag_obj = GlgGetResourceObject( command_obj, "OutputTagHolder" ); 
   if( !write_tag_obj )
     return;

   /* Obtain TagSource from the write tag. */
   GlgGetSResource( write_tag_obj, "TagSource", &output_tag_source );
   
   /* Validate. */
   if( IsUndefined( output_tag_source ) )
   {
      GlgError( GLG_USER_ERROR, 
                "Write Command failed: Invalid Output TagSource." );
      return;
   }

   /* Retrieve new value from the input widget. */
   GlgGetDResource( read_tag_obj, NULL, &value );

   /* Write the obtained value from the widget to the output tag. */
   DF_WriteDTagData( Viewer.DataFeedPtr, output_tag_source, value );

   if( Viewer.RandomData )
   {
      /* Update the tag value in the drawing. In an application,
         the read tag will be updated from the back-end system.
      */
      GlgGetSResource( read_tag_obj, "TagSource", &read_tag_source ); 
      if( !IsUndefined( read_tag_source ) )
      {
         GlgSetDTag( Viewer.MainViewport, read_tag_source, value, GlgTrue );
         GlgUpdate( Viewer.MainViewport );
      }
   }
}

/*----------------------------------------------------------------------
| Performs zoom/pan operations of the specified type.
*/
void Zoom( GlgObject viewport, char zoom_type, double scale )
{
   GlgObject zoom_mode_obj;
   double object_type;
   GlgLong zoom_reset_type = 'n';

   if( !viewport )
     return;
   
   switch( zoom_type )
   {
    default: 
      GlgSetZoom( viewport, NULL, zoom_type, scale );
      break;

    case 'n':
      /* If a viewport is a chart with the chart zoom mode, use 'N'
	 to reset both Time and Y ranges. For a chart, 'n' would reset 
	 only the Time range.
      */
      zoom_mode_obj = GlgGetResourceObject( viewport, "ZoomModeObject" );
      if( zoom_mode_obj )
      {
	 GlgGetDResource( zoom_mode_obj, "Type", &object_type );
	 if( ( (int) object_type ) == GLG_CHART )
	   zoom_reset_type = 'N';
      }
      
      GlgSetZoom( viewport, NULL, zoom_reset_type, 0. );
      break;
   }
}

/*----------------------------------------------------------------------
| Returns index of the MenuArray item with a matching drawing_name,
| if any. If not found, returns NO_SCREEN.
*/
GlgLong LookUpMenuArray( char * drawing_name )
{
   GlgLong i;

   for( i=0; i<Viewer.NumMenuItems; ++i )
     if( strcmp( drawing_name, Viewer.MenuArray[ i ].drawing_name ) == 0 )
	return i;

   return NO_SCREEN;
}

/*----------------------------------------------------------------------
| Select MainMenu item with a specified index.
| NO_SCREEN value (-1) unselects a previously selected menu item, if any.
*/
void SelectMainMenuItem( GlgLong menu_index, GlgBoolean update_menu )
{
   /* Validate menu_index. */
   if( menu_index < NO_SCREEN || menu_index >= Viewer.NumMenuItems )
     GlgError( GLG_WARNING, "Invalid main menu index." );
   
   if( update_menu )
     GlgSetDResource( Viewer.Menu, "SelectedIndex", menu_index );
}

/*----------------------------------------------------------------------
| Utility function, queries a PageType property of the drawing and 
| converts it to a PageType enum using PageTypeTable defined in util.c.
*/
PageTypeEnum GetPageType( GlgObject drawing )
{
   GlgObject type_obj;
   PageTypeEnum page_type;
   char * type_str;

   if( !drawing )
     return DEFAULT_PAGE_TYPE;

   type_obj = GlgGetResourceObject( drawing, "PageType" );
   if( !type_obj )
     return DEFAULT_PAGE_TYPE;

   GlgGetSResource( type_obj, NULL, &type_str );
 
   page_type = (PageTypeEnum) 
     ConvertStringToType( PageTypeTable, type_str,
                          UNDEFINED_PAGE_TYPE, UNDEFINED_PAGE_TYPE );
   if( page_type == UNDEFINED_PAGE_TYPE )
   {
      GlgError( GLG_USER_ERROR, "Invalid PageType." );
      return DEFAULT_PAGE_TYPE;
   }
   return page_type;
}

/*----------------------------------------------------------------------
| Returns CommandType enum using CommandTypeTable defined in util.c.
*/
CommandType GetCommandType( GlgObject command_obj )
{
   char * command_type_str;

   GlgGetSResource( command_obj, "CommandType", &command_type_str );
   return (CommandType) 
     ConvertStringToType( CommandTypeTable, command_type_str,
                          UNDEFINED_COMMAND_TYPE, UNDEFINED_COMMAND_TYPE );
}

/*----------------------------------------------------------------------
| Returns DialogType enum using DialogTypeTable defined in util.c.
*/
DialogType GetDialogType( GlgObject command_obj )
{
   char * dialog_type_str;

   /* Retrieve DialogType resource from the command object. */
   GlgGetSResource( command_obj, "DialogType", &dialog_type_str );
   return (DialogType) 
     ConvertStringToType( DialogTypeTable, dialog_type_str,
                          GLOBAL_POPUP_DIALOG, UNDEFINED_DIALOG_TYPE );
}

/*----------------------------------------------------------------------
| Returns PopupMenuType enum using PopupMenuTypeTable defined in util.c.
*/
PopupMenuType GetPopupMenuType( GlgObject command_obj )
{
   char * menu_type_str;
   
   /* Retrieve MenuType resource from the command object. */
   GlgGetSResource( command_obj, "MenuType", &menu_type_str );
   return (PopupMenuType) 
     ConvertStringToType( PopupMenuTypeTable, menu_type_str,
                          GLOBAL_POPUP_MENU, UNDEFINED_POPUP_MENU_TYPE );
}

/*----------------------------------------------------------------------
| Remap all object tags with the specified tag_name to use a new tag_source. 
*/
GlgLong RemapNamedTags( GlgObject object, char * tag_name, char * tag_source )
{
   GlgObject 
      tag_list,
      tag_obj;
   GlgLong
      size,
      i;

   /* Obtain a list of tags with TagName attribute matching 
      the specified tag_name.
      */
   tag_list = GlgGetTagObject( object, tag_name, /* by name */ GlgTrue, 
                               /* list all tags */ GlgFalse, 
                               /* multiple tags mode */ GlgFalse, 
                               /* Data tag */ 1 );
   if( !tag_list )
      size = 0;
   else
      size = GlgGetSize( tag_list );

   for( i=0; i<size; ++i )
   {
      tag_obj = GlgGetElement( tag_list, i );
      AssignTagSource( tag_obj, tag_source );
   }
   
   /* Dereference tag_list object to prevent memory leak. */
   GlgDropObject( tag_list );
   return size;
}

/*--------------------------------------------------------------------
| Remap tags in the loaded drawing if needed.
| In demo mode, it assigns unset tag sources to be the same as 
| tag names. 
*/
void RemapTags( GlgObject drawing_vp )
{
   GlgObject 
      tag_list,
      tag_obj;
   GlgLong
      size, 
      i;
   char 
      * tag_name,
      * tag_source;

   /* Obtain a list of all tags defined in the drawing and remap them
      as needed. 
      */
   tag_list = GlgCreateTagList( drawing_vp, /* List all tags */ False );  
   if( !tag_list )
      return;

   size = GlgGetSize( tag_list );
   if( !size )
     return; /* no tags found */

   /* Traverse the tag list and remap each tag as needed. */
   for( i=0; i<size; ++i )
   {
      tag_obj = GlgGetElement( tag_list, i );

      /* Retrieve TagName and TagSource attributes from the
         tag object. TagSource represents the data source variable
         used to supply real-time data. This function demonstrates
         how to reassign the TagSource at run-time.
         */
      GlgGetSResource( tag_obj, "TagName", &tag_name );
      GlgGetSResource( tag_obj, "TagSource", &tag_source );

      HMIP_RemapTagObject( Viewer.HMIPagePtr, tag_obj, tag_name, tag_source );
   }

   /* Dereference tag_list object to prevent memory leak */
   GlgDropObject( tag_list );
}

/*--------------------------------------------------------------------
| Assigns new TagSource parameter for a given tag object.
*/
void AssignTagSource( GlgObject tag_obj, char * new_tag_source )
{
   GlgSetSResource( tag_obj, "TagSource", new_tag_source );
}

/*----------------------------------------------------------------------
| Start dynamic updates.
*/
void StartUpdates()
{
   /* Get new update interval depending on the displayed screen. */
   Viewer.UpdateInterval = HMIP_GetUpdateInterval( Viewer.HMIPagePtr );
   if( !Viewer.UpdateInterval )
     return;

   /* Start update timer. */
   Viewer.TimerID = GlgAddTimeOut( AppContext, Viewer.UpdateInterval, 
                                   (GlgTimerProc) PollTagData, NULL );

   Viewer.AlarmTimerID = GlgAddTimeOut( AppContext, Viewer.AlarmUpdateInterval, 
                                        (GlgTimerProc) PollAlarms, NULL );

   UpdateData();   /* Fill data for the initial appearance. */
}

/*----------------------------------------------------------------------
| Stop dynamic updates.
*/
void StopUpdates()
{
   /* Stop update timers */
   if( Viewer.TimerID )
   {
      GlgRemoveTimeOut( Viewer.TimerID );
      Viewer.TimerID = 0;
   }

   if( Viewer.AlarmTimerID )
   {
      GlgRemoveTimeOut( Viewer.AlarmTimerID );
      Viewer.AlarmTimerID = 0;
   }
}

/*----------------------------------------------------------------------
| Animates the drawing with new data.
*/
void PollTagData( void * client_data, GlgLong * timer_id )
{
   GlgULong sec1, microsec1;
   GlgLong timer_interval;

   GlgGetTime( &sec1, &microsec1 );  /* Start time */

   /* Update the drawing with new data. */
   UpdateData();

   GlgUpdate( Viewer.MainViewport );  /* Repaint the drawing. */
   GlgSync( Viewer.MainViewport );    /* Improves interactive response. */

   timer_interval = GetAdjustedTimeout( sec1, microsec1, Viewer.UpdateInterval );

   /* Reinstall the timeout to continue updating.
      Reinstall even if no tags to keep the timer going: the next loaded 
      drawing may have tags, need to keep updating. Most SCADA drawings 
      will have tags anyway.
      */
   Viewer.TimerID = GlgAddTimeOut( AppContext, timer_interval, 
                                   (GlgTimerProc) PollTagData, NULL );
}

/*----------------------------------------------------------------------
| Polls for alarms and displays them in the alarm dialog.
*/
void PollAlarms( void * client_data, GlgLong * timer_id )
{
   ProcessAlarms( GlgTrue );

   /* Reinstall the timeout to continue updating. */
   Viewer.AlarmTimerID = GlgAddTimeOut( AppContext, Viewer.AlarmUpdateInterval, 
                                        (GlgTimerProc) PollAlarms, NULL );
}

/*----------------------------------------------------------------------
| Traverses the array of tag records, gets new data for each tag and 
| updates the drawing with new values.
*/
void UpdateData()
{
   TagRecord * tag_record;
   double 
     d_value,
     time_stamp;
   char * s_value;
   GlgLong i;

   /* Invoke a page's custom UpdateData method, if implemented.
      Don't update any tags if the custom method returns true.
   */         
   if( HMIP_UpdateData( Viewer.HMIPagePtr ) )
     return;

   for( i=0; i<Viewer.NumTagRecords; ++i )
   {
      tag_record = &Viewer.TagRecordArray[ i ];
      
      switch( tag_record->data_type )
      {
       case GLG_D:
	 /* Get data value. */
         if( DF_ReadDTagData( Viewer.DataFeedPtr, tag_record, 
                              &d_value, &time_stamp ) )
         {
            /* Push a new data value into a given tag. The last argument 
               indicates whether or not to set the value depending if the 
               value has changed.
               If set to true, push the value only if it has changed.
               Otherwise, a new value is always pushed into the object.
            */
            GlgSetDTag( Viewer.MainViewport, tag_record->tag_source, d_value, 
                        GlgFalse );

            /* Push a time stamp to the TimeEntryPoint of a plot in 
               a real-time chart, if found.
            */ 
            if( tag_record->plot_time_ep )
              GlgSetDResource( tag_record->plot_time_ep, NULL, time_stamp );
         }
	 break;
         
       case GLG_S:
         if( DF_ReadSTagData( Viewer.DataFeedPtr, tag_record, &s_value ) )
            GlgSetSTag( Viewer.MainViewport, tag_record->tag_source, s_value, 
                        GlgFalse );
          
	 break;
      }
   }
}

/*----------------------------------------------------------------------
| Fills AlarmList dialog with received alarm data.
*/
void ProcessAlarms( GlgBoolean query_new_list )
{
   GlgObject alarm_row;
   GlgBoolean has_active_alarms = GlgFalse;
   AlarmRecord * alarm;
   int 
     i, 
     num_alarms,
     num_visible;

   if( query_new_list )
   {
      /* Free previous alarm list. */
      DF_FreeAlarms( Viewer.DataFeedPtr, Viewer.AlarmList );

      /* Get a new alarm list. */
      Viewer.AlarmList = DF_GetAlarms( Viewer.DataFeedPtr );
   }

   if( !Viewer.AlarmList )
     num_alarms = 0;
   else
     num_alarms = GlgArrayGetSize( Viewer.AlarmList );

   /* Activate Alarms button's blinking if there are unacknowledged alarms. */
   for( i=0; i<num_alarms; ++i )
   {
      alarm = (AlarmRecord*) GlgArrayGetElement( Viewer.AlarmList, i );
      if( !alarm->ack )
      {
         has_active_alarms = GlgTrue;
         break;
      }
   }
   GlgSetDResourceIf( Viewer.MainViewport, "AlarmButton/Blinking", 
                      (double) has_active_alarms, GlgTrue ); 

   if( !Viewer.AlarmDialogVisible )
   {
      GlgUpdate( Viewer.MainViewport );
      return;
   }

   /* Fill alarm rows starting with the AlarmStartRow that controls
      scrolling.
   */
   num_visible = num_alarms - Viewer.AlarmStartRow;
   if( num_visible < 0 )
     num_visible = 0;
   else if( num_visible > Viewer.NumAlarmRows )
     num_visible = Viewer.NumAlarmRows;

   /* Fill alarm rows. */
   for( i=0; i<num_visible; ++i )
   {         
      alarm = (AlarmRecord*) GlgArrayGetElement( Viewer.AlarmList, i );
      alarm_row = Viewer.AlarmRows[i];

      GlgSetDResourceIf( alarm_row, "AlarmIndex", Viewer.AlarmStartRow + i + 1,
                         GlgTrue );
      GlgSetDResourceIf( alarm_row, "TimeInput", alarm->time, GlgTrue  );
      GlgSetSResourceIf( alarm_row, "ID", alarm->tag_source, GlgTrue );
      GlgSetSResourceIf( alarm_row, "Description", alarm->description, 
                         GlgTrue );
      
      /* Set to 1 to supply string value via the StringValue resource.
         Set to 0 to supply double value via the DoubleValue resource.
      */
      GlgSetDResourceIf( alarm_row, "UseStringValue", 
                         !alarm->string_value ? 0. : 1., GlgTrue );
      if( !alarm->string_value )
        GlgSetDResourceIf( alarm_row, "DoubleValue", alarm->double_value, 
                           GlgTrue );
      else
        GlgSetSResourceIf( alarm_row, "StringValue", alarm->string_value, 
                           GlgTrue );
      
      GlgSetDResourceIf( alarm_row, "RowVisibility", 1., GlgTrue  );
      GlgSetDResourceIf( alarm_row, "AlarmStatus", (double) alarm->status, 
                         GlgTrue );
      
      /* Enable blinking: will be disabled when alarm is ACK'ed. */
      GlgSetDResourceIf( alarm_row, "BlinkingEnabled", alarm->ack ? 0. : 1., 
                         GlgTrue );
   }
   
   /* Empty the rest of the rows. Use GlgTrue as the last parameter to update
      only if the value changes.
   */
   for( i=num_visible; i<Viewer.NumAlarmRows; ++i )
   {
      alarm_row = Viewer.AlarmRows[i];
      GlgSetDResourceIf( alarm_row, "AlarmIndex", Viewer.AlarmStartRow + i + 1,
                       GlgTrue );
      GlgSetSResourceIf( alarm_row, "ID", "", GlgTrue );
      
      /* Set status to normal to unhighlight the rightmost alarm field. */
      GlgSetDResourceIf( alarm_row, "AlarmStatus", 0., GlgTrue );

      /* Make all text labels invisible. */
      GlgSetDResourceIf( alarm_row, "RowVisibility", 0., GlgTrue );
      
      GlgSetDResourceIf( alarm_row, "BlinkingEnabled", 0., GlgTrue );
   }
   
   GlgUpdate( Viewer.AlarmDialog );
}

/*--------------------------------------------------------------------
| Read MenuArray from a configuration file. If configuration file 
| is not supplied, use predefined MenuTable array.
*/
void FillMenuArray( char * config_filename, char * exe_path )
{
   Viewer.MenuArray = 
     ReadMenuConfig( exe_path, config_filename, &Viewer.NumMenuItems );

   if( !Viewer.MenuArray )
   { 
      GlgError( GLG_INFO, 
                "Can't read config file: using predefined Menu Table." );

      /* Use predefined MenuTable */
      Viewer.MenuArray = MenuTable;
      Viewer.NumMenuItems = NUM_MENU_ITEMS;
   }
}

/*----------------------------------------------------------------------
| Utility function to validate the string.
*/
GlgBoolean IsUndefined( char * string )
{
   if( !string || !*string || strcmp( string, "unset" ) == 0 ||
       strcmp( string, "$unnamed" ) == 0 )
     return GlgTrue;

   return GlgFalse;
}

/*--------------------------------------------------------------------
| Initialize ActiveDialogs array and ActivePopupMenu.
*/
void InitActivePopups()
{
   int i;

   /* Initialize ActiveDialogs array. */
   for( i=0; i<MAX_DIALOG_TYPE; ++i )
   {
      Viewer.ActiveDialogs[ i ].dialog_type = UNDEFINED_DIALOG_TYPE;
      Viewer.ActiveDialogs[ i ].dialog = NULL;
      Viewer.ActiveDialogs[ i ].subwindow = NULL;
      Viewer.ActiveDialogs[ i ].popup_vp = NULL;
      Viewer.ActiveDialogs[ i ].isVisible = GlgFalse;      
   }

   /* Initialize ActivePopupMenu. */
   Viewer.ActivePopupMenu.menu_type = UNDEFINED_POPUP_MENU_TYPE;
   Viewer.ActivePopupMenu.menu_obj = NULL;
   Viewer.ActivePopupMenu.subwindow = NULL;
   Viewer.ActivePopupMenu.menu_vp = NULL;
   Viewer.ActivePopupMenu.selected_obj = NULL;
   Viewer.ActivePopupMenu.isVisible = GlgFalse;   
}

/*----------------------------------------------------------------------
| Free alarm data.
*/
void FreeAlarmData( AlarmRecord * alarm_record )
{
   GlgFree( alarm_record->tag_source );
   GlgFree( alarm_record->description );
   GlgFree( alarm_record->string_value );               
}

/*----------------------------------------------------------------------
| Return exact time including fractions of seconds.
*/
double GetCurrTime()
{
   GlgULong sec, microsec;
   
   if( !GlgGetTime( &sec, &microsec ) )
     return 0.;
     
   return sec + microsec / 1000000.;
}

#if 0
/*--------------------------------------------------------------------
| Frees MenuArray if allocated, may be used if there is a need to 
| reload configuration file.
*/
void FreeMenuArray()
{
   int i;

   if( Viewer.MenuArray != MenuTable )
   {
      /* MenuArray was allocated - free it. */
      for( i=0; i<Viewer.NumMenuItems; ++i )
      {
         GlgFree( Viewer.MenuArray[i].label_string );
         GlgFree( Viewer.MenuArray[i].drawing_name );
         GlgFree( Viewer.MenuArray[i].tooltip_string );
         GlgFree( Viewer.MenuArray[i].drawing_title );
      }

      GlgFree( Viewer.MenuArray );   /* Handles NULL */
      Viewer.MenuArray = NULL;
      Viewer.NumMenuItems = 0;
   }
}
#endif
