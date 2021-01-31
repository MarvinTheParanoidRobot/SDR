#include "GlgSCADAViewer.h"

#include "EmptyPage.h"
#include "DefaultHMIPage.h"
#include "ProcessPage.h"
#include "RTChartPage.h"
#include "AlarmRecord.h"
#include "GlgTagRecord.h"
#include "DemoDataFeed.h"
#include "LiveDataFeed.h"

#ifdef _WINDOWS
# pragma warning( disable : 4244 )
# pragma warning( disable : 4996 )    /* Allow cross-platform localtime() */
#endif

// Function prototypes
void PollTagData( GlgSCADAViewer * glg_viewer, GlgLong * timer_id );
void PollAlarms( GlgSCADAViewer * glg_viewer, GlgLong * timer_id);

/* Page type table, used to determine the type of a loaded page based
   on the PageType property of the drawing. May be extended by an 
   application to add custom pages. 
   Format:  PageType string,   PageTypeEnum
   PageTypeEnum is defined in PageType.h.
*/
TypeRecord PageTypeTable[] = {
   /* PageType string,  PageTypeEnum */
   { "Default",         DEFAULT_PAGE_TYPE },
   { "Process",         PROCESS_PAGE },
   { "Aeration",        AERATION_PAGE },
   { "Circuit",         CIRCUIT_PAGE },
   { "RealTimeChart",   RT_CHART_PAGE },
   { "TestCommands",    TEST_COMMANDS_PAGE },
   { NULL, 0 }
};

// Predefined menu table, used if the configuration file is not supplied,
// or if the config file cannot be successfully parsed. 
//
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

/* Flag indicating how to supply a time stamp for a RealTimeChart embedded
   into an HMI page: if set to 1, the application will supply a time stamp 
   explicitly. Otherwise, a time stamp will be supplied automatically 
   by the chart using current time. 
*/
#define SUPPLY_PLOT_TIME_STAMP 0

/*----------------------------------------------------------------------
| Constructor
*/
GlgSCADAViewer::GlgSCADAViewer( void )
{
   RANDOM_DATA = GlgTrue;
   PageType = UNDEFINED_PAGE_TYPE;
   TimerID = 0;
   AlarmTimerID = 0;
   MenuArray = NULL;
   NumMenuItems = 0;
   has_parent = GlgFalse;
   AlarmDialogVisible = GlgFalse;
   AlarmRows = NULL;
   UpdateInterval = 0;
   AlarmUpdateInterval = 0;
   DataFeed = NULL;
   LiveDataFeedObj = NULL;
   DemoDataFeedObj = NULL;
   
   // Create empty page.
   HMIPage = new EmptyPage( this );

   // Create data feed objects for simulated data as well as live data.
   DemoDataFeedObj = new DemoDataFeed( this );
   LiveDataFeedObj = new LiveDataFeed( this );
}

/*----------------------------------------------------------------------
| Destructor
*/
GlgSCADAViewer::~GlgSCADAViewer(void)
{
   /* Clear TagRecordArray. */
   DeleteTagRecords();

   /* Clear AlarmList */
   if( DataFeed )
     DataFeed->FreeAlarms( AlarmList );

   delete HMIPage;
   delete DemoDataFeedObj;
   delete LiveDataFeedObj;
   delete [] AlarmRows;
   
#if 0
   // Enable if the code for FreeMenuArray is enabled. 
   FreeMenuArray();
#endif
}

/*----------------------------------------------------------------------
| Set viewer size in screen cooridnates. 
*/
void GlgSCADAViewer::SetSize( GlgLong x, GlgLong y, 
			      GlgLong width, GlgLong height )
{
   SetResource( "Point1", 0., 0., 0. );
   SetResource( "Point2", 0., 0., 0. );

   SetResource( "Screen/XHint", (double) x );
   SetResource( "Screen/YHint", (double) y );
   SetResource( "Screen/WidthHint", (double) width );
   SetResource( "Screen/HeightHint", (double) height );
}

/*----------------------------------------------------------------------
| 
*/
void GlgSCADAViewer::Init( void )
{
   /* Initialize ActiveDialogs array and ActivePopupMenu. */
   InitActivePopups();

   /* Initialization before hierarchy setup. */
   InitBeforeH();

   /* Setup object hierarchy for the GLG control and its top
      top level viewport. 
   */
   SetupHierarchy();
   
   /* Initialization after hierarchy setup: loads the first screen and starts
      data updates.
   */
   InitAfterH();
}

/*----------------------------------------------------------------------
| Initialize top level drawing before hierarchy setup.
*/
void GlgSCADAViewer::InitBeforeH()
{
   /* Callbacks must be enabled before  hierarchy setup.
      Enable callbacks only if the viewer object is a top level window and 
      there is no parent. 

      When this class is used in an MFC application, GlgSCADAViewer is 
      displayed in a GLG MFC control, has_parent=true and callbacks are 
      enabled at the parent level in GlgViewerControl::Init() before 
      the hierarchy setup.
   */
   if( !has_parent )
   {
      EnableCallback( GLG_INPUT_CB, NULL );

      /* Hierarchy callback is invoked when a new drawing 
         is loaded into the subwindow object. 
      */
      EnableCallback( GLG_HIERARCHY_CB, NULL );

      /* Trace callback is used to process native events. */
      EnableCallback( GLG_TRACE_CB, NULL );
   }
      
   /* Store an object ID of the viewport named Menu, containing navigation
      buttons allowing to switch drawings.
   */
   Menu = GetResourceObject( "Menu" );

   /* Store object ID of the DrawingArea subwindow object. */
   DrawingArea = GetResourceObject( "DrawingArea" );
     
   if( DrawingArea.IsNull() ) // no DrawingArea found
   {
      GlgError( GLG_USER_ERROR, "Can't find DrawigngArea Subwindow object." );
      Quit();
   }

   /* Store an object ID of the AlarmDialog viewport. */
   AlarmDialog = GetResourceObject( "AlarmDialog" );

   /* Store an object ID of the viewport containing alarm table. */
   AlarmListVP = AlarmDialog.GetResourceObject( "Table" );
   
   double num_rows_d;
   AlarmListVP.GetResource( "NumRows", &num_rows_d );
   NumAlarmRows = num_rows_d;
   AlarmStartRow = 0;

   /* Make AlarmDialog a free floating dialog. ShellType property
      can be also set at design time.
   */
   AlarmDialog.SetResource( "ShellType", GLG_DIALOG_SHELL );

   /* Initialize alarm list (set initial values in the template). */
   AlarmListVP.SetResource( "Row/ID", "" );
   AlarmListVP.SetResource( "Row/Description", "" );
   AlarmListVP.SetResource( "Row/UseStringValue", 0. );
   AlarmListVP.SetResource( "Row/DoubleValue", 0. );
   AlarmListVP.SetResource( "Row/StringValue", "" );
   AlarmListVP.SetResource( "Row/AlarmStatus", 0. );
   AlarmListVP.SetResource( "Row/RowVisibility", 0. );
   
   /* Make AlarmDialog initially invisible. */
   AlarmDialog.SetResource( "Visibility", 0.0 );
   AlarmDialogVisible = GlgFalse;

   /* Set title for the AlarmDialog. */
   AlarmDialog.SetResource( "ScreenName", "Alarm List" );

   /* Set initial state of the alarm button. */
   SetResource( "AlarmButton/Blinking", 0. );

   /* Make Global PopupDialog a floating DialogShell type. */
   SetResource( "PopupDialog/ShellType", GLG_DIALOG_SHELL );

   /* Make Global PopupDialog initially invisible. */
   SetResource( "PopupDialog/Visibility", 0.0 );

   /* Make Global PopupMenu initially invisible. */
   SetResource( "PopupMenu/Visibility", 0.0 );

   /* Store object ID of the MesageDialog and make it invisible on startup. */
   MessageDialog = GetResourceObject( "MessageDialog" );
   MessageDialog.SetResource( "Visibility", 0. );

   /* Make MessageDialog a free floating dialog. ShellType property
      can be also set at design time.
   */
   MessageDialog.SetResource( "ShellType", (double) GLG_DIALOG_SHELL );

   /* Set the number of menu items. It must be done BEFORE hierarchy setup. */
   Menu.SetResource( "NumRows", NumMenuItems );
}

/*----------------------------------------------------------------------
| Initialize top level drawing after hierarchy setup took place.
*/
void GlgSCADAViewer::InitAfterH()
{
   // Initialize the navigation menu.
   InitMenu();

   // Store object IDs of alarm rows for faster access.
   AlarmRows = new GlgObjectC[ NumAlarmRows ];
   for( int i=0; i<NumAlarmRows; ++i )
   {
      char * resource_name = GlgCreateIndexedName( (char *) "Row%", i );
      AlarmRows[i] = AlarmListVP.GetResourceObject( resource_name );
      GlgFree( resource_name );
   }
   
   // Load drawing corresponding to the first menu item.
   SelectMainMenuItem( 0, /* update menu object */ GlgTrue );
   LoadDrawingFromMenu( 0 );
}

/*----------------------------------------------------------------------
| Initialize the navigation menu, a viewport named "Menu".
*/
void GlgSCADAViewer::InitMenu()
{
   GlgObjectC button;

   /* Populate menu buttons based on the MenuArray. */
   for( int i=0; i<NumMenuItems; ++i )
   {
      char * button_name = GlgCreateIndexedName( (char*) "Button%", i );
      button = Menu.GetResourceObject( button_name );
      GlgFree( button_name );

      button.SetResource( "LabelString", MenuArray[ i ].label_string );
      button.SetResource( "TooltipString", MenuArray[ i ].tooltip_string );
   }

   SelectMainMenuItem( NO_SCREEN, GlgTrue );
}

/*----------------------------------------------------------------------
| Start dynamic updates.
*/
void GlgSCADAViewer::StartUpdates()
{
   UpdateInterval = HMIPage->GetUpdateInterval();
   if( !UpdateInterval )
     return;

   /* Start update timer. */
   TimerID = GlgAddTimeOut( AppContext, UpdateInterval, 
			    (GlgTimerProc) PollTagData, this );

   AlarmTimerID = GlgAddTimeOut( AppContext, AlarmUpdateInterval, 
				 (GlgTimerProc) PollAlarms, this );

   UpdateData();    /* Fill data for the initial appearance. */
}

/*----------------------------------------------------------------------
| Stop dynamic updates.
*/
void GlgSCADAViewer::StopUpdates()
{
   /* Stop update timers */
   if( TimerID )
   {
      GlgRemoveTimeOut( TimerID );
      TimerID = 0;
   }

   if( AlarmTimerID )
   {
      GlgRemoveTimeOut( AlarmTimerID );
      AlarmTimerID = 0;
   }
}

/*----------------------------------------------------------------------
| Animates the drawing. 
*/
void PollTagData( GlgSCADAViewer * viewer, GlgLong * timer_id )
{
   GlgULong sec1, microsec1;

   GlgGetTime( &sec1, &microsec1 );  /* Start time */
  
   viewer->UpdateData();   // Update the drawing with new data.

   viewer->Update();       // Repaint the drawing.
   viewer->Sync( );        // Improves interactive response.

   // Get adjusted time interval.
   GlgLong timer_interval = 
     GetAdjustedTimeout( sec1, microsec1, viewer->UpdateInterval );

   /* Reinstall the timeout to continue updating.
      Reinstall even if no tags to keep the timer going: the next loaded 
      drawing may have tags, need to keep updating. Most SCADA drawings 
      will have tags anyway.
   */
   viewer->TimerID = 
     GlgAddTimeOut( viewer->AppContext, timer_interval, 
                    (GlgTimerProc)PollTagData, viewer );
}

/*----------------------------------------------------------------------
| Polls for alarms and displays them in the alarm dialog.
*/
void PollAlarms( GlgSCADAViewer* viewer, GlgLong * timer_id )
{
   viewer->ProcessAlarms( GlgTrue );

   /* Reinstall the timeout to continue updating. */
   viewer->AlarmTimerID = 
     GlgAddTimeOut( viewer->AppContext, 
		    viewer->AlarmUpdateInterval, 
		    (GlgTimerProc)PollAlarms, viewer );
}


/*----------------------------------------------------------------------
| Traverses the array of tag records, 
| gets new data for each tag and updates the drawing with new values.
*/
void GlgSCADAViewer::UpdateData()
{
   /* Invoke a page's custom UpdateData() method, if implemented.
      Don't update any tags if the custom method returns true.
   */   
   if( HMIPage->UpdateData() )
     return;

   if( TagRecordArray.empty() )
     return;    // No tags in the current page.

   /* Always update all tags defined in the current page, as well as 
      any additional tags defined in popup dialogs in the main drawing, 
      outside of the current page.
   */
   TagRecordArrayType::iterator it;
   for( it = TagRecordArray.begin(); it != TagRecordArray.end(); ++it )
   {
      GlgTagRecord * tag_record = *it;
      
      switch( tag_record->data_type )
      {
       case GLG_D:
         // Obtain a new numerical data value for a given tag.  
         double 
           d_value,
           time_stamp;

         if( DataFeed->ReadDTag( tag_record, &d_value, &time_stamp ) )
         {
            /* Push a new data value into a given tag. The last argument 
               indicates whether or not to set the value depending if the 
               value has changed.
               If set to true, push the value only if it has changed.
               Otherwise, a new value is always pushed into the object.
            */
            SetTag( tag_record->tag_source, d_value, GlgFalse );

            /* Push a time stamp to the TimeEntryPoint of a plot in 
               a real-time chart, if found.
            */ 
            if( !tag_record->plot_time_ep.IsNull() )
              tag_record->plot_time_ep.SetResource( NULL, time_stamp );
         }
         break;
         
       case GLG_S:        
         // Obtain a new numerical data value for a given tag.  
         SCONST char * s_value;
         if( DataFeed->ReadSTag( tag_record, &s_value ) )
         {
            SetTag( tag_record->tag_source, s_value, GlgFalse );
         }
         break;
      }
   }
}

/*----------------------------------------------------------------------
| Fills AlarmList dialog with received alarm data.
*/
void GlgSCADAViewer::ProcessAlarms( GlgBoolean query_new_list )
{
   AlarmRecord * alarm;
   GlgObjectC alarm_row;
   int i;

   if( query_new_list )
   {
      /* Free previous alarm list. */
      DataFeed->FreeAlarms( AlarmList );

      /* Get a new alarm list. */
      if( !DataFeed->GetAlarms( AlarmList ) )
        return;  //nothing to do if no alarms
   }

   /* Activate Alarm's button blinking if there are unacknowledged 
      (non-ACK'ed) alarms. 
   */
   GlgBoolean has_active_alarms = GlgFalse;
   AlarmRecordArrayType::iterator it;
   for( it = AlarmList.begin(); it != AlarmList.end(); ++it )
   {
      alarm = *it; 
      if( !alarm->ack )
      {
         has_active_alarms = GlgTrue;
         break;
      }
   }
   
   // Activate AlarmButton blinking.
   SetResource( "AlarmButton/Blinking", (double) has_active_alarms, GlgTrue ); 

   if( !AlarmDialogVisible )
   {
      Update();
      return;
   }

   /* Fill alarm rows starting with the AlarmStartRow that controls
      scrolling.
   */
   GlgLong num_visible = AlarmList.size() - AlarmStartRow;
   if( num_visible < 0 )
     num_visible = 0;
   else if( num_visible > NumAlarmRows )
     num_visible = NumAlarmRows;

   /* Fill alarm rows. */
   for( i=0; i<num_visible; ++i )
   {         
      alarm = AlarmList[ i ];
      alarm_row = AlarmRows[ i ];

      alarm_row.SetResource( "AlarmIndex", AlarmStartRow + i + 1, GlgTrue );
      alarm_row.SetResource( "TimeInput", alarm->time, GlgTrue );
      alarm_row.SetResource( "ID", alarm->tag_source, GlgTrue );
      alarm_row.SetResource( "Description", alarm->description, GlgTrue );
      
      // Set to 1 to supply string value via the StringValue resource.
      // Set to 0 to supply double value via the DoubleValue resource.
      alarm_row.SetResource( "UseStringValue", !alarm->string_value ? 0. : 1., 
                             GlgTrue );
      if( !alarm->string_value )
        alarm_row.SetResource( "DoubleValue", alarm->double_value, GlgTrue );
      else
        alarm_row.SetResource( "StringValue", alarm->string_value, 
                               (GlgBoolean) GlgTrue );
      
      alarm_row.SetResource( "RowVisibility", 1., GlgTrue  );
      alarm_row.SetResource( "AlarmStatus", (double) alarm->status, GlgTrue );
      
      /* Enable blinking: will be disabled when alarm is ACK'ed. */
      alarm_row.SetResource( "BlinkingEnabled", alarm->ack ? 0. : 1., GlgTrue );
   }
   
   /* Empty the rest of the rows. Use GlgTrue as the last parameter to update
      only if the value changes.
   */
   for( i=num_visible; i<NumAlarmRows; ++i )
   {
      alarm_row = AlarmRows[i];
      alarm_row.SetResource( "AlarmIndex", AlarmStartRow + i + 1, GlgTrue );
      alarm_row.SetResource( "ID", "", GlgTrue );      
      // Set status to normal to unhighlight the rightmost alarm field.
      alarm_row.SetResource( "AlarmStatus", 0., GlgTrue );

      // Make all text labels invisible.
      alarm_row.SetResource( "RowVisibility", 0., GlgTrue );
      
      alarm_row.SetResource( "BlinkingEnabled", 0., GlgTrue );
   }
   
   AlarmDialog.Update();
}

/*----------------------------------------------------------------------
| GLG Input callback, invoked when the user interacts with objects in a
| GLG drawing. It is used to handle events occurred in input objects,
| such as a menu, as well as Commands or Custom Mouse Events attached 
| to objects at design time.
*/
void GlgSCADAViewer::Input( GlgObjectC& viewport, GlgObjectC& message )
{
   SCONST char
     * format,
     * action,
     * origin;
   
   GlgObjectC selected_obj;
   GlgObjectC action_obj;
   double menu_index;
   
   /* Return if the page's custom input handler processed the input.
      Otherwise, continue to process common events and commands.
   */
   if( HMIPage->Input( viewport, message ) )
     return;

   /* Get the message's format, action and origin. */
   message.GetResource( "Format", &format );
   message.GetResource( "Action", &action );
   message.GetResource( "Origin", &origin );

   /* Retrieve selected object. */
   selected_obj = message.GetResourceObject( "Object" );
      
   /* Handle events from the screen navigation menu, named "Menu" */
   if( strcmp( format, "Menu" ) == 0 &&
       strcmp( action, "Activate" ) == 0 &&
       strcmp( origin, "Menu" ) == 0 )
   {
      /* User selected a button from the menu object named Menu. 
	 Load a new drawing associated with the selected button.
      */
      message.GetResource( "SelectedIndex", &menu_index );
      SelectMainMenuItem( (GlgLong) menu_index, 
                          /* don't update menu object */ GlgFalse );

      /* Close active popup dialogs and popup menu, if any. */
      CloseActivePopups( CLOSE_ALL );

      StopUpdates();

      /* Load the drawing associated with the selected menu button and
	 display it in the DrawingArea.
      */
      LoadDrawingFromMenu( (GlgLong) menu_index );

      StartUpdates();
      viewport.Update();
   }

   /* Handle custom commands attached to objects in the drawing. */
   else if( strcmp( format, "Command" ) == 0 )
   {
      action_obj = message.GetResourceObject( "ActionObject" ); 
      ProcessObjectCommand( viewport, selected_obj, action_obj );
      viewport.Update();
   }

   // Handle zoom controls on the left.
   else if( strcmp( format, "Button" ) == 0 )
   {
      if( !origin ||
          strcmp( action, "Activate" ) != 0 &&      /* Not a push button */
          strcmp( action, "ValueChanged" ) != 0 )   /* Not a toggle button */
        return;

      if( strcmp( origin, "MessageDialogOK" ) == 0 )
      {
         /* Close alarm dialog. */
         MessageDialog.SetResource( "Visibility", 0. );
         MessageDialog.Update();
      }
     
      /* Zoom and pan buttons. */
      else if( strcmp( origin, "Left" ) == 0 )
        Zoom( DrawingAreaVP, 'l', 0.1 );
      else if( strcmp( origin, "Right" ) == 0 )
        Zoom( DrawingAreaVP, 'r', 0.1 );
      else if( strcmp( origin, "Up" ) == 0 )
        Zoom( DrawingAreaVP, 'u', 0.1 );
      else if( strcmp( origin, "Down" ) == 0 )
        Zoom( DrawingAreaVP, 'd', 0.1 );
      else if( strcmp( origin, "ZoomIn" ) == 0 )
        Zoom( DrawingAreaVP, 'i', 1.5 );
      else if( strcmp( origin, "ZoomOut" ) == 0 )
        Zoom( DrawingAreaVP, 'o', 1.5 );
      else if( strcmp( origin, "ZoomTo" ) == 0 )
        Zoom( DrawingAreaVP, 't', 0. );
      else if( strcmp( origin, "ZoomReset" ) == 0 )
        Zoom( DrawingAreaVP, 'n', 0. );

      /* Alarm scrolling buttons. */
      else if( strcmp( origin, "ScrollToTop" ) == 0 )
      {
         AlarmStartRow = 0;
         ProcessAlarms( GlgFalse );
      }
      else if( strcmp( origin, "ScrollUp" ) == 0 )
      {
         --AlarmStartRow;
         if( AlarmStartRow < 0 )
           AlarmStartRow = 0;
         ProcessAlarms( GlgFalse );
      }
      else if( strcmp( origin, "ScrollUp2" ) == 0 )
      {
         AlarmStartRow -= NumAlarmRows;
         if( AlarmStartRow < 0 )
           AlarmStartRow = 0;
         ProcessAlarms( GlgFalse );
      }
      else if( strcmp( origin, "ScrollDown" ) == 0 )
      {
         ++AlarmStartRow;
         ProcessAlarms( GlgFalse );
      }
      else if( strcmp( origin, "ScrollDown2" ) == 0 )
      {
         AlarmStartRow += NumAlarmRows;
         ProcessAlarms( GlgFalse );
      }
      else
        return;

      viewport.Update( );
   }

   /* Handle custom events */
   else if( strcmp( format, "CustomEvent" ) == 0 )
   {
      SCONST char * event_label;
      GlgObjectC action_data;

      message.GetResource( "EventLabel", &event_label );
      if( strcmp( event_label, "" ) == 0 )
         return;    /* Don't process events with empty EventLabel */

      /* Retrieve action object. It will be NULL for custom events 
	 added prior to GLG v.3.5. 
      */
      action_obj = message.GetResourceObject( "ActionObject" ); 

      /* Retrieve ActionData. If present, use its properties for 
	 custom event processing as needed. 
      */
      if( !action_obj.IsNull() )
	action_data = action_obj.GetResourceObject( "ActionData" );
      
      if( strcmp( event_label, "AlarmRowACK" ) == 0 )
      {
         SCONST char * tag_source;
         
         /* selected_obj is object ID of the alarm row selected by Ctrl-click.
            Retrieve the tag source from it. 
         */
         selected_obj.GetResource( "ID", &tag_source );
            
         /* Send Alarm ACK message to the back-end system for the specified 
            tag source.
         */
         DataFeed->ACKAlarm( tag_source );
      }
      else
      {
         /* Place custom code here to handle custom events as needed. */
      }
      
      viewport.Update();
   }

   /* Handle Timer events, generated by objects with blinking dynamics. */
   else if( strcmp( format, "Timer" ) == 0 )
   {
      /* Update objects with Blinking (Timer) dynamics. */
      viewport.Update( );
      Sync();    /* Improves interactive response. */
   }

   /* Handle events from a Real-Time Chart. */
   else if( strcmp( format, "Chart" ) == 0 &&
            strcmp( action, "CrossHairUpdate" ) == 0 )
   {
      /* To avoid slowing down real-time chart updates, invoke Update() 
         to redraw cross-hair only if the chart is not updated fast 
         enough by the timer.
      */
      if( UpdateInterval > 100 )
        viewport.Update();
   }

   /* Handle window closing events. */
   else if( strcmp( format, "Window" ) == 0 &&
            strcmp( action, "DeleteWindow" ) == 0 )
   {
      if( selected_obj.IsNull() )
	return;

      /* If the event came from the top level application window,
	 exit from process.
      */
      if( selected_obj.Same( *this ) )
        Quit();            /* Closing main application window. */

      else if( selected_obj.Same( AlarmDialog ) )
        ShowAlarms();   /* Toggle the state of alarms dialog to erase it. */
      
      else
      {
         /* If the closing window is found in the ActiveDialogs array, 
            close the active dialog. 
         */
         for( int i=0; i<MAX_DIALOG_TYPE; ++i )
           if( selected_obj.Same( ActiveDialogs[i].dialog ) )
           {
              ClosePopupDialog( (DialogType) i );
              break;
           }
      }
      
      viewport.Update();
   }
}


/*----------------------------------------------------------------------
| Trace callback. Used to handle low level events.
*/
void GlgSCADAViewer::Trace( GlgObjectC& viewport, 
                            GlgTraceCBStruct * trace_data )
{
   GlgObjectC event_vp;
   int
     event_type = 0,
     x, y,
     width, height;

   /* Return if the page's custom trace callback processed the event.
      Otherwise, continue to process common events.
   */
   if( HMIPage->Trace( viewport, trace_data ) )
     return;

   event_vp = trace_data->viewport;

   /* Platform-specific code to extract event information. */
#ifndef _WINDOWS  /* X/Linux */

#define TEXT_BUFFER_LENGTH 128

   XEvent * event;
   KeySym keysym;
   XComposeStatus status;
   char buf[ TEXT_BUFFER_LENGTH ];
   int length;

   event = trace_data->event;
   
   switch( event->type )
   {
    case ButtonPress:
      x = event->xbutton.x;
      y = event->xbutton.y;
      event_type = BUTTON_PRESS_EVENT;
      break;
      
    case MotionNotify:
      x = event->xmotion.x;
      y = event->xmotion.y;
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
      x = LOWORD( trace_data->event->lParam );
      y = HIWORD( trace_data->event->lParam );
      event_type = BUTTON_PRESS_EVENT;
      break;
      
    case WM_MOUSEMOVE:
      x = LOWORD( trace_data->event->lParam );
      y = HIWORD( trace_data->event->lParam );
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
       case VK_ESCAPE: // ESC key  0x1B
	 /* Place custom code here as needed. */
         break;
       case VK_SHIFT: // Shift key
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
| Hierarchy callback, added to the top level viewport and 
| invoked when a new drawing is loaded into any subwindow or subdrawing
| object. In this demo, this callback processes events only from the 
| reference of type "Subwindow" (such as DrawingArea).
*/
void GlgSCADAViewer::Hierarchy( GlgObjectC& viewport, 
				GlgHierarchyCBStruct * info_struct )
{
   GlgObjectC 
     subwindow,
     drawing_vp;
   double obj_type;

   subwindow = info_struct->object;
   drawing_vp = info_struct->subobject;

   /* Handle events only from Subwindow-type reference objects. */
   subwindow.GetResource( "ReferenceType", &obj_type );
   if( obj_type != GLG_SUBWINDOW_REF )
     return;

   /* This callback is invoked twice: one time before hierarchy setup
      for the new drawing, and the second time after hierarchy setup.
      Drawing initialization can be done here if needed.
   */
   switch( info_struct->condition )
   {
    case GLG_BEFORE_SETUP_CB:
      if( drawing_vp.IsNull() )
      {
	 /* Drawing loading error */
	 GlgError( GLG_USER_ERROR, (char*) "Drawing loading failed" );
         if( DrawingArea.Same( subwindow ) )
             DrawingAreaVP.LoadNullObject();
         break; 
      }
      
      /* Set "ProcessMouse" attribute for the loaded viewport, to
         process custom events and tooltips.
      */
      drawing_vp.SetResource( "ProcessMouse", 
                              GLG_MOUSE_CLICK | GLG_MOUSE_OVER_TOOLTIP );
         
      /* Set "OwnsInputCB" attribute for the loaded viewport,
         so that Input callback is invoked with this viewport ID.
      */
      drawing_vp.SetResource( "OwnsInputCB", 1. ); 
      
      // Set up a new HMIPage and initialize it.
      if( DrawingArea.Same( subwindow ) )
      {
         DrawingAreaVP = drawing_vp;
         SetupHMIPage();   // Create new HMIPage of an appropriate type.

         /* Initialize loaded drawing before setup. */
         HMIPage->InitBeforeSetup();
      }
      break;

    case GLG_AFTER_SETUP_CB:
      /* Initialize loaded drawing after setup. */
      if( DrawingArea.Same( subwindow ) )
        HMIPage->InitAfterSetup();
      break;

       default: break;
   }
}

/*----------------------------------------------------------------------
| Assigns a new page to HMIPage variable, either as a default or 
| custom HMI page, based on the PageType property of the loaded drawing. 
| The HMI page class handles the page logic and user interaction.
|
| If the PageType property doesn't exist in the drawing or is set to 
| "Default", DefaultHMIPage class is used for the new page.
| Otherwise, a page class type corresponding to the PageType property 
| is assigned to HMIPage variable. For example, RTChartPage class is used
| if PageType property of the loaded drawing is set to "RealTimeChart".
| 
| The method also assigns DataFeed for the new page.
*/
void GlgSCADAViewer::SetupHMIPage( void )
{ 
   if( HMIPage )
   {      
      delete HMIPage; /* Destroy previous page, if any. */
      HMIPage = new EmptyPage( this );
   }

   /* Get page type, using PageTypeTable enums. */
   PageType = GetPageType( DrawingAreaVP );   

   /* Use live data if requested with the -live-data command-line option, 
      otherwise use simulated random data for testing. 
      RANDOM_DATA may be set to false to use live data by default.
   */
   if( RANDOM_DATA )
     DataFeed = DemoDataFeedObj;
   else
     DataFeed = LiveDataFeedObj;

   switch( PageType )
   {
    case UNDEFINED_PAGE_TYPE:
      HMIPage = new EmptyPage( this );
      break;

    case DEFAULT_PAGE_TYPE:
    case AERATION_PAGE:
    case CIRCUIT_PAGE:
      HMIPage = new DefaultHMIPage( this );
      break;

    case PROCESS_PAGE:
      HMIPage = new ProcessPage( this );
      break;

    case RT_CHART_PAGE:
      HMIPage = new RTChartPage( this );
      break;

    case TEST_COMMANDS_PAGE:
      HMIPage = new DefaultHMIPage( this );

      /* Test page: always use demo data. */
      DataFeed = DemoDataFeedObj;
      break;

    default:
      /* New custom page, will use live data if requested with the -live-data 
         command-line option, otherwise use simulated random data for 
         testing. RANDOM_DATA may be set to false to use live data by 
         default.
      */
      HMIPage = new DefaultHMIPage( this );
      break;
   }

   /* True if DemoDataFeed is used for the current page. */
   RandomData = ( DataFeed == DemoDataFeedObj );
}

/*----------------------------------------------------------------------
| Load a new drawing into the DrawingArea when the user selects an item
| from the navigation menu object (Menu object).
*/
GlgBoolean GlgSCADAViewer::LoadDrawingFromMenu( GlgLong screen )
{
   /* Loads a new drawing into the DrawingArea subwindow object.
      DrawingAreaVP is assigned in the Hierarchy callback to the 
      ID of the $Widget viewport of the loaded drawing.
   */
   LoadDrawing( DrawingArea, MenuArray[ screen ].drawing_name );
   if( DrawingAreaVP.IsNull() )
   {
      DeleteTagRecords();
      PageType = UNDEFINED_PAGE_TYPE;
      HMIPage = new EmptyPage( this );
      return False;
   }
 
   /* Query a list of tags using a newly loaded drawing and build a new
      TagRecordsArray.
   */
   QueryTags( DrawingAreaVP );
    
   // Set title from the selected menu record, etc.
   SetupLoadedPage( MenuArray[ screen ].drawing_title );
      
   HMIPage->Ready();
   return GlgTrue;
}

/*----------------------------------------------------------------------
| Load a new drawing into the specified subwindow object and return 
| newly loaded viewport. 
*/
GlgObject GlgSCADAViewer::LoadDrawing( GlgObjectC& subwindow, 
                                       SCONST char * drawing_file )
{
   if( subwindow.IsNull() || !drawing_file )
   {
      GlgError( GLG_USER_ERROR, "Drawing loading failed");
      return NULL;
   }
   
   /* Assign a new drawing name to the subwindow object, 
      if the current drawing name has changed.
   */
   subwindow.SetResource( "SourcePath", drawing_file, GlgTrue /* if changed */ );
   
   /* Setup hierarchy for the subwindow object, which causes the 
      new drawing to be loaded into the subwindow. 
      Hierarchy callback will be invoked before and after hierarchy 
      setup for newly loaded drawing. This callback can be used 
      to invoke code for initializing the newly loaded drawing.
   */
   subwindow.SetupHierarchy();
   
   // Return newly loaded viewport.
   return( subwindow.GetResourceObject( "Instance" ) );
}

/*----------------------------------------------------------------------
| Set viewer parameters based on the loaded drawing, such
| as background color, title, etc.
*/
void GlgSCADAViewer::SetupLoadedPage( SCONST char * title )
{
   double r, g, b;
   
   // Set new Title. 
   SetResource( "Title", title );
   
   /* Set the color of the top level window to match the color 
      of the loaded drawing. 
   */
   DrawingAreaVP.GetResource( "FillColor", &r, &g, &b );
   SetResource( "FillColor", r, g, b );      
}

/*----------------------------------------------------------------------
| Query tags for a given viewport and rebuild TagRecordArray. 
| The new_drawing parameter is the viewport of the new drawing. 
| TagRecordArray will include tags for the top level viewport 
| of the viewer, including tags for the loaded page, as well as
| tags for the popup dialogs, if any.
*/
void GlgSCADAViewer::QueryTags( GlgObjectC& new_drawing )
{
   /* Delete existing tag records from TagRecordArray */
   DeleteTagRecords();

   /* Remap tags in the loaded drawing if needed.
      Will invoke HMIPage's RemapTagObject() for each tag.
   */
   if( !new_drawing.IsNull() && HMIPage->NeedTagRemapping() )
     RemapTags( new_drawing );
   
   /* Build an array of tag records containing tags information and
      store it in TagRecordArray. TagRecordArray will be used for 
      animating the drawing with data.
      Always create data for the top level viewport, to keep a global 
      list of tags that include tags in any dynamically
      loaded dialogs.
   */
   CreateTagRecords( *this );
}

/*--------------------------------------------------------------------
| Create an array of tag records containing tag information.
*/
void GlgSCADAViewer::CreateTagRecords( GlgObjectC& drawing_vp )
{
   SCONST char  
     * tag_source,
     * tag_name,
     * tag_comment;
   double 
     dtype, 
     access_type;
   GlgObjectC 
     tag_list,
     tag_obj;

   /* Obtain a list of tags with unique tag sources. */
   tag_list = 
     drawing_vp.CreateTagList( /* List each tag source only once */ true );

   /* CreateTagList creates an object (a group containing a list of tags).
      The returned object has to be explicitly dereferenced to prevent a 
      memory leak. The object is still referenced by the tag_list variable 
      instance.
   */
   tag_list.Drop();

   if( tag_list.IsNull() )
      return;   
   
   int size = tag_list.GetSize();
   if( !size )
      return; /* no tags found */

   for( int i=0; i<size; ++i )
   {
      tag_obj = tag_list.GetElement( i );
      
      /* Skip OUTPUT tags. */
      tag_obj.GetResource( "TagAccessType", &access_type );
      if( access_type == GLG_OUTPUT_TAG )
	continue;

      /* Retrieve TagSource attribute from the tag object.
	 TagSource represents the data source variable used 
	 for data animation.
      */
      tag_obj.GetResource( "TagSource", &tag_source );
      
      /* Skip undefined tag sources, such as "" or "unset". */
      if( IsUndefined( tag_source ) )
	continue; 
      
      if( RandomData )
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
         tag_obj.GetResource( "TagName", &tag_name );
         tag_obj.GetResource( "TagComment", &tag_comment );
         
         if( !IsUndefined( tag_name ) && 
             ( strstr( tag_name, "Speed" ) || strstr( tag_name, "Test" ) ) )
           continue;
         if( strstr( tag_source, "Test" ) )
           continue;
         if( !IsUndefined( tag_comment ) && strstr( tag_comment, "Test" ) )
           continue;
      }

      /* Get tag object's data type: GLG_D, GLG_S or GLG_G */
      tag_obj.GetResource( "DataType", &dtype );
      
      /* Create a new tag record. */
      GlgTagRecord * tag_record = new GlgTagRecord();
      tag_record->tag_source = GlgStrClone( (char*) tag_source );
      tag_record->data_type = (int) dtype;
      tag_record->tag_obj = tag_obj;
      
      /* Add a new tag record to TagRecordArray */
      TagRecordArray.push_back( tag_record );
   }
   
   NumTagRecords = TagRecordArray.size();

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
}

/*-----------------------------------------------------------------------
| Clear TagRecordArray
*/
void GlgSCADAViewer::DeleteTagRecords()
{
   if( !TagRecordArray.empty() )
   {
      TagRecordArrayType::iterator it;
      /* Free memory for the array elements  */
      for( it = TagRecordArray.begin(); it != TagRecordArray.end(); ++it )
        delete *it;
      
      TagRecordArray.clear();
   }

   NumTagRecords = 0;
}

/*----------------------------------------------------------------------
| Store TimeEntryPoint (plot_time_ep) in each tag record, if found.
*/
void GlgSCADAViewer::SetPlotTimeEP( GlgObjectC& drawing_vp )
{
   GlgObjectC 
     tag_list,
     tag_obj,
     plot_time_ep;
   GlgLong 
     i, 
     size;
   SCONST char
     * tag_comment,
     * tag_source;
   GlgTagRecord * tag_record;
 
   if( !NumTagRecords )
     return;

   /* Obtain a list of all tags, including non-unique tag sources. */
   tag_list = drawing_vp.CreateTagList( /* List all tags */ false );

   /* Dereference tag_list. It will still be referenced by its
      GlgObjectC instance.
   */ 
   tag_list.Drop();

   if( tag_list.IsNull() )
     return;

   size = tag_list.GetSize();
   if( !size )
      return; /* no tags found */

   /* For each tag in the list, check if there is a plot object in the
      drawing with a matching TagSource for the plot's ValueEntryPoint.
      If found, obtain plot's TimeEntryPoint and store it in the
      TagRecordArray for a tag record with a matching tag_source.
   */
   for( i=0; i<size; ++i )
   {
      tag_obj = tag_list.GetElement( i );

      /* Retrieve TagSource and TagComment. In the demo, TagComment is
         not used, but the application may use it as needed.
      */
      tag_obj.GetResource( "TagComment", &tag_comment );
      tag_obj.GetResource( "TagSource", &tag_source );

      if( IsUndefined( tag_source ) )
        return;

      /* Find a TimeEntryPoint of a plot in a RealTimeChart with
         a matching TagSource assigned for the plot's ValueEntryPoint.
         It is assumed that there is ONLY ONE plot in the drawing 
         with a given TagSource. 
      */
      plot_time_ep = FindMatchingTimeEP( tag_obj );
      
      if( plot_time_ep.IsNull() )
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
}

/*----------------------------------------------------------------------
|  For a given tag object, find a parent plot object (GLG_PLOT object type).
|  If found, return the plot's TimeEntryPoint.
|  It is assumed that there is ONLY ONE plot in the drawing with a given
|  TagSource. 
*/
GlgObject GlgSCADAViewer::FindMatchingTimeEP( GlgObjectC& tag_obj )
{
   GlgObjectC plot;

   /* Fill in match data structure to search for a plot object type,
      which is a parent of the tag_obj (ValueEntryPoint) .
   */
   GlgFindMatchingObjectsData match_data;

   match_data.match_type = GLG_OBJECT_TYPE_MATCH;
   match_data.find_parents = GlgTrue;
   match_data.find_first_match = GlgTrue;
   match_data.search_inside = GlgFalse;
   match_data.object_type = GLG_PLOT;

   if( !GlgFindMatchingObjects( (GlgObject) tag_obj, &match_data ) || 
       !match_data.found_object )
     return NULL; /* matching object not found */

   plot = match_data.found_object;
   return plot.GetResourceObject( "TimeEntryPoint" );
}

/*----------------------------------------------------------------------
| Lookup TagRecordArray and return a matching tag record with 
| tag_source=match_tag_source.
*/
GlgTagRecord * GlgSCADAViewer::LookupTagRecords( SCONST char * match_tag_source )
{
   GlgTagRecord * tag_record;

   if( IsUndefined( match_tag_source ) || TagRecordArray.empty() )
     return NULL;

   TagRecordArrayType::iterator it;
   for( it = TagRecordArray.begin(); it != TagRecordArray.end(); ++it )
   {
      tag_record = *it;
      if( strcmp( tag_record->tag_source, match_tag_source ) == 0 )
        return tag_record;
   }

   return NULL; // not found
}

/*----------------------------------------------------------------------
| Close all active popups, including popup dialogs and popup menus.
*/
void GlgSCADAViewer::CloseActivePopups( DialogType allow_dialog )
{
   /* Close all active dialogs, except AlarmDialog which may remain
      visible until closed by the user, and the dialog type specified 
      by the allow_dialog parameter.
   */
   for( int i=0; i<MAX_DIALOG_TYPE; ++i )
   {
      if( i == allow_dialog || i == ALARM_DIALOG )
	continue; 
      
      /* Close a dialog of a given type, if any. */
      ClosePopupDialog( (DialogType) i ); 
   }
   
   /* Close Global ActivePopupMenu, if any. */
   ClosePopupMenu( GLOBAL_POPUP_MENU );
}

/*----------------------------------------------------------------------
| Process commands attached to objects at design time.
| Command types enums and strings are defined in CommandTypeTable
| in util.c. CommandType strings in the table must match the 
| CommandType strings defined in the drawing.
*/
void GlgSCADAViewer::ProcessObjectCommand( GlgObjectC& command_vp, 
					   GlgObjectC& selected_obj, 
					   GlgObjectC& action_obj )
{
   GlgObjectC command_obj;
   CommandType command_type;
   DialogType dialog_type;
   PopupMenuType menu_type;

   if( selected_obj.IsNull() || action_obj.IsNull() )
     return;

   /* Retrieve Command object. */
   command_obj = action_obj.GetResourceObject( "Command" );
   if( command_obj.IsNull() )
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
      Quit();
      break;
    default: 
      GlgError( GLG_USER_ERROR, (char*) "Undefined CommandType." );
      break;
   }
}

/*----------------------------------------------------------------------
| Opens or closes the alarm window.
*/
void GlgSCADAViewer::ShowAlarms()
{
   static GlgBoolean FirstAlarmDialog = GlgTrue;
   
   AlarmDialogVisible = ToggleResource( "AlarmDialog/Visibility" );
   
   /* If the alarm dialog becomes visible, fill alarms to show them
      right away.
   */
   if( AlarmDialogVisible )
     ProcessAlarms( GlgTrue );
   
   if( FirstAlarmDialog )   // Show help message the first time only.
   {         
      ShowMessageDialog( "Ctrl-click on the alarm row to acknowledge "
                         "the alarm.", GlgFalse );
      FirstAlarmDialog = GlgFalse;
   }

   Update();
}

/*----------------------------------------------------------------------
| Shows/hides MessageDialog.
*/
void GlgSCADAViewer::ShowMessageDialog( SCONST char * message, 
                                        GlgBoolean error )
{
   MessageDialog.SetResource( "MessageString", message );
   
   /* Set to 1. to highlight the message in red. */
   MessageDialog.SetResource( "ShowErrorColor", error ? 1.0 : 0.0 );
   
   MessageDialog.SetResource( "Visibility", 1.0 );
}

/*----------------------------------------------------------------------
| Toggles resource value between 0 and 1 and returns the new value
| as a boolean.
*/
GlgBoolean GlgSCADAViewer::ToggleResource( SCONST char * resource_name )
{
   double current_value = 0.;

   GetResource( resource_name, &current_value );
   
   current_value = ( !current_value ? 1. : 0. );
   
   SetResource( resource_name, current_value );
   return ( current_value == 1. );      
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
void GlgSCADAViewer::DisplayPopupDialog( GlgObjectC& command_vp, 
					 GlgObjectC& selected_obj, 
					 GlgObjectC& command_obj )

{
   SCONST char 
     * title = NULL,
     * drawing_file = NULL,
     * dialog_res = NULL,
     * destination_res = NULL,
     * subwindow_name = NULL;
   DialogType dialog_type;
   GlgObjectC 
     subwindow,
     popup_vp,
     dialog;

   /* Retrieve command parameters. */
   command_obj.GetResource( "DialogResource", &dialog_res );
   command_obj.GetResource( "DrawingFile", &drawing_file );
   command_obj.GetResource( "Destination", &destination_res );
   command_obj.GetResource( "Title", &title );

   /* Obtain DialogType. */
   dialog_type = GetDialogType( command_obj );
   if( dialog_type == UNDEFINED_DIALOG_TYPE )
   {
      GlgError( GLG_USER_ERROR, 
                (char*) "PopupDialog Command failed: Unknown DialogType." );
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
		(char*) "PopupDialog Command failed: Invalid DialogResource." );
      ClosePopupDialog( dialog_type );
      return;
   }
   
   /* Obtain an object ID of the requested popup dialog. 
      If invalid, abort the command. 
   */
   if( StartsWith( dialog_res, "/" ) )
     dialog = GetResourceObject( dialog_res+1 );
   else 
     dialog = command_vp.GetResourceObject( dialog_res );

   if( dialog.IsNull() )
   {
      GlgError( GLG_USER_ERROR, 
		(char*) "Dialog not found, PopupDialog Command failed." );
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
      subwindow = dialog.GetResourceObject( subwindow_name );
      if( subwindow.IsNull() )
      {
	 GlgError( GLG_USER_ERROR, (char * )
                  "PopupDialog Command failed: Destionation object not found." );
         ClosePopupDialog( dialog_type );
	 return;
      }

      /* Load new drawing and obtain an object id of the newly loaded viewport. 
	 If drawing loading failed, the error gets reported in the 
         Hierarchy callback.
      */
      popup_vp = LoadDrawing( subwindow, drawing_file );
      if( popup_vp.IsNull() )
      {
	 /* If drawing loading fails, it will be reported in Hierarchy callback.
	    Generate an additional error indicating command failing.
	 */
	 GlgError( GLG_USER_ERROR, (char*) "PopupDialog Command failed." );
         ClosePopupDialog( dialog_type );
	 return;
      }
   }

   /* For the tags with matching TagName, transfer tag sources from the 
      selected object to the loaded popup viewport.
   */
   TransferTags( selected_obj, popup_vp, GlgFalse );
   
   if( !popup_vp.Same( dialog ) )
     /* Rebuild TagRecordArray to include tags both for the
	drawing displayed in the main drawing area and drawing 
	displayed in the popup dialog. Pass loaded popup (popup_vp)
        to QueryTags, where the tags may need to be reassigned
        if needed.
     */
     QueryTags( popup_vp );
   
   /* Poll new data to fill the popup dialog with current values. */
   UpdateData();
   
   /* Display title in the loaded viewport, if Title resource is found. */
   if( popup_vp.HasResourceObject( "Title" ) )
     popup_vp.SetResource( "Title", title );
   
   /* Display title as the dialog caption. */
   dialog.SetResource( "ScreenName", title );
   
   /* Display the dialog if it is not up already. */
   dialog.SetResource( "Visibility", 1.0 );

  /* Store dialog information in ActiveDialogs array */
   ActiveDialogs[ dialog_type ].dialog_type = dialog_type;
   ActiveDialogs[ dialog_type ].dialog = dialog;
   ActiveDialogs[ dialog_type ].subwindow = subwindow;
   ActiveDialogs[ dialog_type ].popup_vp = popup_vp;
   ActiveDialogs[ dialog_type ].isVisible = GlgTrue;

   Update(); /* update top level viewport. */
   Sync();    /* Improves interactive response. */
}

/*----------------------------------------------------------------------
| Close active popup dialog of a given type.
*/
void GlgSCADAViewer::ClosePopupDialog( DialogType dialog_type )
{
   if( dialog_type == UNDEFINED_DIALOG_TYPE || 
       dialog_type >= MAX_DIALOG_TYPE )
   {
      GlgError( GLG_USER_ERROR, (char*) "Dialog closing failed." ); 
      return;
   }

   if( ActiveDialogs[ dialog_type ].dialog.IsNull() )
     return; //nothing to do

   if( !ActiveDialogs[ dialog_type ].subwindow.IsNull() )
   {
      /* Destroy currently loaded popup drawing and load empty drawing. */
      LoadDrawing( ActiveDialogs[ dialog_type ].subwindow, 
                   "empty_drawing.g" );

      /* Rebuild a list of tags to exclude the tags from the previously
	 loaded popup viewport.
      */
      QueryTags( *this ); 
   }

   /* Hide the dialog */
   ActiveDialogs[ dialog_type ].dialog.SetResource( "Visibility", 0.0 );
   
   /* Clear a dialog record with a specified index (dialog_type)
      in the ActiveDialogs array.
   */
   ActiveDialogs[ dialog_type ].dialog_type = UNDEFINED_DIALOG_TYPE;
   ActiveDialogs[ dialog_type ].dialog.LoadNullObject();
   ActiveDialogs[ dialog_type ].subwindow.LoadNullObject();
   ActiveDialogs[ dialog_type ].popup_vp.LoadNullObject();
   ActiveDialogs[ dialog_type ].isVisible = GlgFalse;
}

/*----------------------------------------------------------------------
| Transfer tag sources for tags with a matching TagName from the
| selected object to the specified viewport.
*/
void GlgSCADAViewer::TransferTags( GlgObjectC& selected_obj, 
				   GlgObjectC& viewport, 
				   GlgBoolean unset_tags )
{
   SCONST char 
     * widget_type,
     * tag_name,
     * tag_source;
   GlgObjectC 
     tag_list,
     tag_obj;
   GlgLong num_remapped_tags;
   
   /* Retrieve WidgetType from the selected objects, if any. */
   if( selected_obj.HasResourceObject( "WidgetType" ) )
     selected_obj.GetResource( "WidgetType", &widget_type );
   
   /* Place custom code here to initialize the drawing based on WidgetType, 
      if needed. In this demo, the initialization code below is executed 
      regardless of WidgetType.
   */

   /* Obtain a list of tags defined in the selected object. */
   tag_list = selected_obj.CreateTagList( /* List all tags */ false );  

   /* CreateTagList creates an object (a group containing a list of tags).
      The returned object has to be explicitly dereferenced to prevent a 
      memory leak. The object is still referenced by the tag_list variable 
      instance.
   */
   tag_list.Drop();

   if( tag_list.IsNull() )
     return;
   
   GlgLong size = tag_list.GetSize();
   if( !size )
     return; /* no tags found */
   
   /* Traverse the tag list. For each tag, transfer the TagSource
      defined in the selected object to the tag in the loaded 
      popup drawing that has a matching TagName.
   */
   for( int i=0; i<size; ++i )
   {
      tag_obj = tag_list.GetElement( i );
      
      /* Obtain TagName. */
      tag_obj.GetResource( "TagName", &tag_name );

      /* Skip tags with undefined TagName */
      if( IsUndefined( tag_name ) )
	continue;

      /* Obtain TagSource. */
      tag_obj.GetResource( "TagSource", &tag_source );
      
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
      
      if( RandomData )
      {
        /* For demo purposes only, initialize input tags with TagName="Speed"
           present for motor objects in scada_aeration.g and scada_motir_info.g.
        */
        if( strcmp( tag_name, "Speed" ) == 0 )
          SetTag( tag_source, GlgRand( 300., 1500. ), GlgTrue );
      }
   }
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
void GlgSCADAViewer::DisplayPopupMenu( GlgObjectC& command_vp, 
				       GlgObjectC& selected_obj, 
				       GlgObjectC& command_obj )
{
   SCONST char 
     * title = NULL,
     * drawing_file = NULL,
     * menu_res = NULL,
     * destination_res = NULL,
     * subwindow_name = NULL;
   PopupMenuType menu_type;
   GlgObjectC
     subwindow,
     popup_vp,
     menu_obj;
   double 
     menu_width, 
     menu_height;

   /* Retrieve command parameters. */
   command_obj.GetResource( "MenuResource", &menu_res );
   command_obj.GetResource( "DrawingFile", &drawing_file );
   command_obj.GetResource( "Destination", &destination_res );
   command_obj.GetResource( "Title", &title );

   /* Obtain MenuType. */
   menu_type = GetPopupMenuType( command_obj );
   if( menu_type == UNDEFINED_POPUP_MENU_TYPE )
   {
      GlgError( GLG_USER_ERROR, (char*) 
                "Unknown MenuType, PopupMenu Command failed." );
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
		(char*) "Invalid MenuResource, PopupMenu Command failed." );
      return;
   }
   
   /* Obtain an object ID of the requested popup menu. 
      If invalid, abort the command. 
   */
   if( StartsWith( menu_res, "/" ) )
     menu_obj = GetResourceObject( menu_res+1 );
   else 
     menu_obj = command_vp.GetResourceObject( menu_res );

   if( menu_obj.IsNull() )
   {
      GlgError( GLG_USER_ERROR, 
		(char*) "Menu object not found, PopupMenu Command failed." );
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
      subwindow = menu_obj.GetResourceObject( subwindow_name );
      if( subwindow.IsNull() )
      {
	 GlgError( GLG_USER_ERROR, (char*)
                 "Destionation object not found, PopupDialog Command failed." );
	 return;
      }
      
      /* Load new drawing and store an object id of the newly loaded viewport. 
	 If drawing loading failed, the error gets reported in the 
	 Hierarchy callback.
      */
      popup_vp = LoadDrawing( subwindow, drawing_file );
      if( popup_vp.IsNull() )
      {
	 /* If drawing loading fails, it will be reported in Hierarchy callback.
	    Generate an additional error indicating command failing.
	 */
	 GlgError( GLG_USER_ERROR, (char*) "PopupMenu Command failed." );
	 return;
      }

      /* If the viewport has Width and Height resources that define
	 its size in pixels, adjust the size of the menu object 
	 to match the size of the loaded viewport.
      */
      if( popup_vp.HasResourceObject( "Width" ) &&
	  menu_obj.HasResourceObject( "Width" ) )
      {
	 popup_vp.GetResource( "Width", &menu_width );
	 menu_obj.SetResource( "Width", menu_width );
      } 

      if( popup_vp.HasResourceObject( "Height" ) &&
	  menu_obj.HasResourceObject( "Height" ) )
      {
	 popup_vp.GetResource( "Height", &menu_height );
	 menu_obj.SetResource( "Height", menu_height );
      } 
   }

   /* Transfer tag sources from the selected object to the loaded 
      popup viewport, using tags with a matching TagName.
   */
   TransferTags( selected_obj, popup_vp, GlgFalse );
      
   /* Display title in the loaded viewport, if Title resource is found. */
   if( popup_vp.HasResourceObject( "Title" ) )
     popup_vp.SetResource( "Title", title );

   /* Show the menu. */
   menu_obj.SetResource( "Visibility", 1.0 );

   /* Store menu information in the global ActivePopupMenu structure, 
      used to close the active popup menu.
   */
   ActivePopupMenu.menu_type = menu_type;
   ActivePopupMenu.menu_obj = menu_obj;
   ActivePopupMenu.subwindow = subwindow;
   ActivePopupMenu.menu_vp = popup_vp;
   ActivePopupMenu.selected_obj = selected_obj; 
   ActivePopupMenu.isVisible = GlgTrue; 
      
   /* Position the menu next to the selected object. */
   PositionPopupMenu();

   menu_obj.Update( );
}

/*----------------------------------------------------------------------
| Position ActivePopupMenu at the upper right corner of the selected object,
| if possible. Otherwise, position the menu close to the selected object
| such that it is displayed within the current viewport.
*/
void GlgSCADAViewer::PositionPopupMenu()
{
   GlgObjectC
     selected_obj_vp, /* Viewport that contains selected object. */
     menu_parent_vp;  /* Parent viewport that contains the popup menu. */
   GlgCube
     * sel_obj_box,
     converted_box,
     * menu_obj_box;
   double 
     x, y,
     offset = 5., // offset in pixels.
     menu_width, menu_height,
     parent_width, parent_height;
   int 
     x_anchor, 
     y_anchor; 
   
   if( ActivePopupMenu.selected_obj.IsNull() || 
       ActivePopupMenu.menu_obj.IsNull() )
     return;

   selected_obj_vp = 
     GlgGetParentViewport( ActivePopupMenu.selected_obj, GlgTrue );
   menu_parent_vp = GlgGetParentViewport( ActivePopupMenu.menu_obj, GlgTrue );

   /* Obtain the object's bounding box in screen coordinates. */
   sel_obj_box = ActivePopupMenu.selected_obj.GetBoxPtr();   
   converted_box = *sel_obj_box;

   /* If the menu is located in a different viewport from the viewport
      of the selected object, convert screen coordinates of the 
      selected object box from the viewport of the selected object to the 
      viewport that contains the popup menu.
   */
   if( !selected_obj_vp.Same( menu_parent_vp ) )
   {
      GlgTranslatePointOrigin( selected_obj_vp, menu_parent_vp, 
			       &converted_box.p1 );
      GlgTranslatePointOrigin( selected_obj_vp, menu_parent_vp, 
			       &converted_box.p2 );
   }

   /* Obtain width and height in pixels of the parent viewport of the menu. */
   menu_parent_vp.GetResource( "Screen/Width", &parent_width );
   menu_parent_vp.GetResource( "Screen/Height", &parent_height );

   /* Obtain width and height of the menu object. */
   menu_obj_box = ActivePopupMenu.menu_obj.GetBoxPtr(); 
   menu_width = menu_obj_box->p2.x - menu_obj_box->p1.x;
   menu_height = menu_obj_box->p2.y - menu_obj_box->p1.y;
   
   /* Position the popup at the upper right or lower left corner of 
      the selected object, if possible. Otherwise (viewport is too small), 
      position it in the center of the viewport.
   */   
   if( converted_box.p2.x + menu_width > parent_width )
   {
      /* Outside of window right edge.
	 Position right edge of the popup to the left of the selected object.
	 Always use GLG_HLEFT anchor to simplify out-of-the-window check.
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

   ActivePopupMenu.menu_obj.PositionObject( GLG_SCREEN_COORD,
                                            x_anchor | y_anchor, x, y, 0. );
}

/*----------------------------------------------------------------------
| Close active popup menu. In this demo, menu_type is not used, since
| there is only a single ActivePopupMenu object. The code can be extended 
| by the application developer as needed.
*/
void GlgSCADAViewer::ClosePopupMenu( PopupMenuType menu_type )
{
   if( ActivePopupMenu.menu_obj.IsNull() )
     return; /* Nothing to do. */

   /* Hide active popup. */
   ActivePopupMenu.menu_obj.SetResource( "Visibility", 0.0 );

   if( !ActivePopupMenu.subwindow.IsNull() )
   {
      /* Destroy currently loaded popup drawing and load empty drawing. */
      LoadDrawing( ActivePopupMenu.subwindow, "empty_drawing.g" );
   }
   else
   {
      /* Unset tags in the menu object, which were previously
	 transfered and assigned from  the selected object. 
      */ 
      if( !ActivePopupMenu.selected_obj.IsNull() )
	TransferTags( ActivePopupMenu.selected_obj, 
		      ActivePopupMenu.menu_obj, GlgTrue );
   }
   
   /* Clear menu record. */
   ActivePopupMenu.menu_type = UNDEFINED_POPUP_MENU_TYPE;
   ActivePopupMenu.menu_obj.LoadNullObject();
   ActivePopupMenu.subwindow.LoadNullObject();
   ActivePopupMenu.menu_vp.LoadNullObject();
   ActivePopupMenu.selected_obj.LoadNullObject();
   ActivePopupMenu.isVisible = GlgFalse;
}

/*----------------------------------------------------------------------
| Process command "GoTo". The command loads a new drawing specified
| by the DrawingFile parameter into the subwindow object specified
| by the Destination parameter. If Destination is omitted, uses
| main DrawingArea subwindow object.  
*/
void GlgSCADAViewer::GoTo( GlgObjectC& command_vp, 
			   GlgObjectC& selected_obj, 
			   GlgObjectC& command_obj )
{
   GlgObjectC 
     subwindow,
     drawing_vp;
   SCONST char 
     * drawing_file,
     * destination_res,
     * title;

   /* Close active popup dialogs and popup menu. */
   CloseActivePopups( CLOSE_ALL );
   
   /* Retrieve command parameters. */
   command_obj.GetResource( "DrawingFile", &drawing_file );
   command_obj.GetResource( "Destination", &destination_res );
   command_obj.GetResource( "Title", &title );
   
   /* If DrawingFile is not valid, abort the command. */
   if( !drawing_file || !*drawing_file )
   {
      GlgError( GLG_USER_ERROR, (char*) 
                "Invalid DrawingFile, GoTo Command failed." );
      return;
   }

   /* Use Destination resource, if any, to display the specified drawing. 
      It is assumed that Destination points to the subwindow object.
      If not defined, use top level DrawingArea subwindow by default.
   */
   if( !destination_res || !*destination_res ) 
     subwindow = DrawingArea;
   else
   {
      if( StartsWith( destination_res, "/" ) )
	/* Destination is relative to the top level viewport MainViewport. */
	subwindow = GetResourceObject( destination_res+1 );
      else 
	/* Destination is relative to the current viewport, where the
	   command occurred.
	*/
	subwindow = command_vp.GetResourceObject( destination_res );
   
      if( subwindow.IsNull() )
      {
	 GlgError( GLG_USER_ERROR, 
		   (char*) "Invalid Destionation object, GoTo Command failed." );
	 return;
      }
   }

   /* Load new drawing and obtain an object id of the newly loaded viewport. 
      If drawing loading failed, the error gets reported in the Hierarchy
      callback.
   */
   drawing_vp = LoadDrawing( subwindow, drawing_file );
   if( drawing_vp.IsNull() )
   {
      /* If drawing loading fails, it will be reported in Hierarchy callback.
	 Generate an additional error indicating command failing.
      */
      GlgError( GLG_USER_ERROR, (char*) "GoTo Command failed." );
      return;
   }

   /* Rebuild TagRecordArray for the newly loaded drawing. */
   QueryTags( drawing_vp );
   
   /* If  the new drawing is loaded into the main DrawingArea,
      newly loaded viewport is stored as DrawingAreaVP.
      Display a new title. Set background color of the top level 
      viewport to match the background color of the loaded drawing.
   */
   if( subwindow.Same( DrawingArea ) )
   {
      /* Reset main menu selection. If the new drawing matches one of the 
	 drawings defined in the MenuArray, update main menu using
	 the corresponding menu index.
      */
      int screen_index = LookUpMenuArray( drawing_file );
      SelectMainMenuItem( screen_index, GlgTrue );

      /* Use title from the command, set background olcor, etc. */
      SetupLoadedPage( title );   

      HMIPage->Ready();
   }
}

/*----------------------------------------------------------------------
| Process command "WriteValue". The command writes a new value specified
| by the Value parameter into the tag in the back-end system
| specfied by the OutputTagHolder.
*/
void GlgSCADAViewer::WriteValue( GlgObjectC& command_vp, 
				 GlgObjectC& selected_obj, 
				 GlgObjectC& command_obj )
{
   SCONST char * tag_source;
   double value;

   /* Retrieve tag source to write data to. */
   command_obj.GetResource( "OutputTagHolder/TagSource", &tag_source );

   /* Validate. */
   if( IsUndefined( tag_source ) )
   {
      GlgError( GLG_USER_ERROR, 
		(char*) "Invalid TagSource. WriteValue Command failed." );
      return;
   }

   /* Retrieve the value to be written to the tag source. */
   command_obj.GetResource( "Value", &value );
   
   /* Place custom code here as needed, to validate the value specified
      in the command.
   */

   /* Write new value to the specified tag source. */
   DataFeed->WriteDTag( tag_source, value );

   if( RandomData )
   {
      /* For demo purposes, update the tag value in the drawing. 
         In an application, the read tag will be updated from 
         the back-end system.
      */
      SetTag( tag_source, value, GlgTrue );
      Update();
   }
}

/*----------------------------------------------------------------------
| Process command "WriteValueFromWidget". The command allows writing
| a new value into the tag in the back-end system using an input
| widget, such as a toggle or a spinner.
*/
void GlgSCADAViewer::WriteValueFromInputWidget( GlgObjectC& command_vp, 
						GlgObjectC& selected_obj, 
						GlgObjectC& command_obj )
{
   
   GlgObjectC write_tag_obj, read_tag_obj;
   double value;
   SCONST char 
     * output_tag_source,
     * value_res;

   command_obj.GetResource( "ValueResource", &value_res );
   
   /* Obtain object ID of the read tag object. FOr example, in case of a
      spinner widget, it will be "Value" resource.
   */
   read_tag_obj = selected_obj.GetResourceObject( value_res );
   
   if( read_tag_obj.IsNull() )
     return;

   /* Obtain object ID of the write tag object. */
   write_tag_obj = command_obj.GetResourceObject( "OutputTagHolder" ); 
   if( write_tag_obj.IsNull() )
     return;

   /* Obtain TagSource from the write tag. */
   write_tag_obj.GetResource( "TagSource", &output_tag_source );
   
   /* Validate. */
   if( IsUndefined( output_tag_source ) )
   {
      GlgError( GLG_USER_ERROR, 
		(char*) "Write Command failed: Invalid Output TagSource." );
      return;
   }

   /* Disable read tag for the duration of the Write command. 
      It is expected that the input widget had a valid input/read tag, 
      otherwise generate an error.
   */
   if( !read_tag_obj.SetResource( "TagEnabled", 0.0 ) )
     GlgError( GLG_USER_ERROR, (char*) "Invalid Input Tag." );

   /* Retrieve new value from the input widget. */
   read_tag_obj.GetResource( NULL, &value );

   /* Write the obtained value from the widget to the output tag. */
   DataFeed->WriteDTag( output_tag_source, value );

   /* Enable read tag, so that it is included in the global
      tag  data updates.
   */
   read_tag_obj.SetResource( "TagEnabled", 1.0 );

   if( RandomData )
   {
      /* Update the tag value in the drawing. In an application,
         the read tag will be updated from the back-end system.
      */
      
      SCONST char * read_tag_source;
      
      read_tag_obj.GetResource( "TagSource", &read_tag_source ); 
      if( !IsUndefined( read_tag_source ) )
      {
         SetTag( read_tag_source, value, GlgTrue );
         Update();
      }
   }
}

/*----------------------------------------------------------------------
| Returns index of the MenuArray item with a matching drawing_name,
| if any. If not found, returns NO_SCREEN.
*/
GlgLong GlgSCADAViewer::LookUpMenuArray( SCONST char * drawing_name )
{
   for( int i=0; i<NumMenuItems; ++i )
     if( strcmp( drawing_name, MenuArray[ i ].drawing_name ) == 0 )
	return i;

   return NO_SCREEN;
}


/*----------------------------------------------------------------------
| Select MainMenu item with a specified index.
| NO_SCREEN value (-1) unselects a previously selected menu item, if any.
*/
void GlgSCADAViewer::SelectMainMenuItem( GlgLong menu_index, 
					 GlgBoolean update_menu )
{
   /* Validate menu_index. */
   if( menu_index < NO_SCREEN || menu_index >= NumMenuItems )
     GlgError( GLG_WARNING, (char*) "Invalid main menu index." );
     
   if( update_menu )
     Menu.SetResource( "SelectedIndex", menu_index );
}

/*----------------------------------------------------------------------
| Performs zoom/pan operations of the specified type.
*/
void GlgSCADAViewer::Zoom( GlgObjectC& viewport, char zoom_type, 
                           double scale )
{
   GlgLong zoom_reset_type = 'n';

   if( viewport.IsNull() )
     return;
   
   switch( zoom_type )
   {
    default: 
      viewport.SetZoom( NULL, zoom_type, scale );
      break;

    case 'n':
      /* If a viewport is a chart with the chart zoom mode, use 'N'
	 to reset both Time and Y ranges. For a chart, 'n' would reset 
	 only the Time range.
      */
      GlgObjectC zoom_mode_obj;
      zoom_mode_obj = 
           viewport.GetResourceObject( "ZoomModeObject" );
      if( !zoom_mode_obj.IsNull() )
      {
         double object_type;
	 zoom_mode_obj.GetResource( "Type", &object_type );
	 if( ( (int) object_type ) == GLG_CHART )
	   zoom_reset_type = 'N';
      }
      
      viewport.SetZoom( NULL, zoom_reset_type, 0. );
      break;
   }
}

/*--------------------------------------------------------------------
| Initialize ActiveDialogs array and ActivePopupMenu.
*/
void GlgSCADAViewer::InitActivePopups()
{
   /* Initialize ActiveDialogs array. */
   for( int i=0; i<MAX_DIALOG_TYPE; ++i )
   {
      ActiveDialogs[ i ].dialog_type = UNDEFINED_DIALOG_TYPE;
      ActiveDialogs[ i ].dialog.LoadNullObject();
      ActiveDialogs[ i ].subwindow.LoadNullObject();
      ActiveDialogs[ i ].popup_vp.LoadNullObject();
      ActiveDialogs[ i ].isVisible = GlgFalse;      
   }

   /* Initialize ActivePopupMenu. */
   ActivePopupMenu.menu_type = UNDEFINED_POPUP_MENU_TYPE;
   ActivePopupMenu.menu_obj.LoadNullObject();
   ActivePopupMenu.subwindow.LoadNullObject();
   ActivePopupMenu.menu_vp.LoadNullObject();
   ActivePopupMenu.selected_obj.LoadNullObject();
   ActivePopupMenu.isVisible = GlgFalse;   
}

/*--------------------------------------------------------------------
| Remap tags as needed in a given viewport (drawing_vp).
*/
void GlgSCADAViewer::RemapTags( GlgObjectC& drawing_vp )
{
   GlgObjectC 
     tag_list,
     tag_obj;
   int
     i,
     size;
   SCONST char 
     * tag_name,
     * tag_source;
   
   /* Obtain a list of all tags defined in the drawing and remap them
      as needed. 
   */
   tag_list = drawing_vp.CreateTagList( /* List all tags */ false );  

   /* CreateTagList creates an object (a group containing a list of tags).
      The returned object has to be explicitly dereferenced to prevent a 
      memory leak. The object is still referenced by the tag_list variable 
      instance.
   */
   tag_list.Drop();

   if( tag_list.IsNull() )
     return;

   size = tag_list.GetSize();
   if( !size )
     return; /* no tags found */
   
   /* Traverse the tag list and remap each tag as needed. */
   for( i=0; i<size; ++i )
   {
      tag_obj = tag_list.GetElement( i );
      
      /* Retrieve TagName and TagSource attributes from the
	 tag object. TagSource represents the data source variable
	 used to supply real-time data. This function demonstrates
	 how to reassign the TagSource at run-time.
      */
      tag_obj.GetResource( "TagName", &tag_name );
      tag_obj.GetResource( "TagSource", &tag_source );

      HMIPage->RemapTagObject( tag_obj, tag_name, tag_source );
   }
}

/*--------------------------------------------------------------------
| Assigns new TagSource parameter for a given tag object.
*/
void GlgSCADAViewer::AssignTagSource( GlgObjectC& tag_obj, 
                                      SCONST char * new_tag_source )
{
   tag_obj.SetResource( "TagSource", new_tag_source );
}

/*----------------------------------------------------------------------
| Remap all object tags with the specified tag_name to use a new tag_source. 
*/
GlgLong GlgSCADAViewer::RemapNamedTags( GlgObjectC& object, 
                                        SCONST char * tag_name,
                                        SCONST char * tag_source )
{
   GlgObjectC 
     tag_obj, 
     tag_list;

   /* Obtain a list of tags with TagName attribute matching 
      the specified tag_name.
   */ 
   tag_list = object.GetTagObject( tag_name, /* by name */ true, 
				   /* list all tags */ false, 
				   /* multiple tags mode */ false, 
				   /* Data tag */ GLG_DATA_TAG );

   /* In the multiple tags mode, GetTagObject creates and returns a list of 
      tags. The returned list object has to be explicitly dereferenced to 
      prevent a memory leak. The object is still referenced by the tag_list 
      variable instance.
   */
   tag_list.Drop();

   int size;
   if( tag_list.IsNull() )
     size = 0;
   else
     size = tag_list.GetSize( );
   
   for( int i=0; i<size; ++i )
   {
      tag_obj = tag_list.GetElement( i );
      AssignTagSource( tag_obj, tag_source );
   }
   
   return size;
}

/*----------------------------------------------------------------------
| Utility function, queries a PageType property of the drawing and 
| converts it to a PageType enum using PageTypeTable defined in util.c.
*/
PageTypeEnum GlgSCADAViewer::GetPageType( GlgObjectC& drawing )
{
   if( !drawing )
     return DEFAULT_PAGE_TYPE;

   GlgObjectC type_obj;
   type_obj = drawing.GetResourceObject( "PageType" );
   if( type_obj.IsNull() )
     return DEFAULT_PAGE_TYPE;

   SCONST char * type_str;
   type_obj.GetResource( NULL, &type_str );
 
   PageTypeEnum page_type = (PageTypeEnum) 
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
CommandType GlgSCADAViewer::GetCommandType( GlgObjectC& command_obj )
{
   SCONST char * command_type_str;

   command_obj.GetResource( "CommandType", &command_type_str );
   return 
     (CommandType) ConvertStringToType( CommandTypeTable, command_type_str,
					UNDEFINED_COMMAND_TYPE,
					UNDEFINED_COMMAND_TYPE);
}

/*----------------------------------------------------------------------
| Returns DialogType enum using DialogTypeTable defined in util.c.
*/
DialogType GlgSCADAViewer::GetDialogType( GlgObjectC& command_obj )
{
   SCONST char *dialog_type_str;

   /* Retrieve DialogType resource from the command object. */
   command_obj.GetResource( "DialogType", &dialog_type_str );
   return 
     (DialogType) ConvertStringToType( DialogTypeTable, dialog_type_str,
				       GLOBAL_POPUP_DIALOG, 
                                       UNDEFINED_DIALOG_TYPE);
}

/*----------------------------------------------------------------------
| Returns PopupMenuType enum using PopupMenuTypeTable defined in util.c.
*/
PopupMenuType GlgSCADAViewer::GetPopupMenuType( GlgObjectC& command_obj )
{
   SCONST char *menu_type_str;
   
   /* Retrieve MenuType resource from the command object. */
   command_obj.GetResource( "MenuType", &menu_type_str );
   return 
     (PopupMenuType) ConvertStringToType( PopupMenuTypeTable, menu_type_str,
					  GLOBAL_POPUP_MENU,
					  UNDEFINED_POPUP_MENU_TYPE);
}

/*----------------------------------------------------------------------
| Quit the application.
*/
void GlgSCADAViewer::Quit( void )
{
#ifdef WINDOWS
   if( has_parent ) // Quit MFC application.
     PostQuitMessage( 0 );
   else
     exit( GLG_EXIT_OK );
#else
   exit( GLG_EXIT_OK );
#endif
}

/*--------------------------------------------------------------------
| Fill MenuArray from a configuration file. If configuration file 
| is not supplied, use predefined MenuTable array.
*/
void GlgSCADAViewer::FillMenuArray( SCONST char * exe_path, 
                                    SCONST char * config_filename )
{
   MenuArray = ReadMenuConfig( exe_path, config_filename, &NumMenuItems );

   if( !MenuArray )
   { 
      GlgError( GLG_INFO, 
            (char*) "Can't read config file: using predefined Menu Table." );

      /* Use predefined MenuTable */
      MenuArray = MenuTable;
      NumMenuItems = NUM_MENU_ITEMS;
   }
}

#if 0
/*--------------------------------------------------------------------
| Frees MenuArray if allocated, may be used if there is a need to 
| reload configuration file.
*/
void GlgSCADAViewer::FreeMenuArray()
{
   int i;

   if( MenuArray != MenuTable )
   {
      /* MenuArray was allocated - free it. */
      for( i=0; i<NumMenuItems; ++i )
      {
         GlgFree( MenuArray[i].label_string );
         GlgFree( MenuArray[i].drawing_name );
         GlgFree( MenuArray[i].tooltip_string );
         GlgFree( MenuArray[i].drawing_title );
      }

      GlgFree( MenuArray );   /* Handles NULL */
      MenuArray = NULL;
      NumMenuItems = 0;
   }
}
#endif

// Assignment operator
GlgSCADAViewer& GlgSCADAViewer::operator= ( const GlgObjectC& object )
{
   GlgObjectC::operator=( object );
   return *this;
}


