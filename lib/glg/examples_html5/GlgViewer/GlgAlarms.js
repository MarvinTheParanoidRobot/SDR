var AlarmDialog = null;   /* GlgObject */
var AlarmListVP;          /* GlgObject : Viewport containing the alarm list */
var NumAlarmRows;         /* int : Number of visible alarms on one alarm page. */
var AlarmRows;            /* GlgObject[] : Keeps object ID's of alarm rows 
                             for faster access. */

var AlarmStartRow = 0;    /* int: Scrolled state of the alarm list. */
var AlarmList = null;     /* AlarmRecord[] : List of alarms. */
var AlarmDialogVisible = false;  /*boolean*/

var ALARM_UPDATE_INTERVAL = 1000;  /* Update interval, msec */

/* Is used to show a help message when the alarm dialog is shown 
   for the first time. 
*/
var FirstAlarmDialog = true;       /* boolean */

// MessageDialog object used to show a help message. 
var MessageDialog;                 /* GlgObject */

//////////////////////////////////////////////////////////////////////////
// Used to store information for an individual alarm, can be extended
// as needed.
//////////////////////////////////////////////////////////////////////
function AlarmRecord()
{
   this.time = null;          /* double : Epoch time in seconds. */
   this.tag_source = null;    /* String */
   this.description = null;   /* String */

   /* If string_value is set to null, double_value will be displayed as alarm 
      value; otherwise string_value will be displayegd.
   */
   this.string_value = null;  /* String  */
   this.double_value = 0;     /* double */

   this.status = 0;           /* int */   
   this.ack = false;          /* boolean */

   this.age = 0;              /* int: Used for demo alarm simulation only. */
}

//////////////////////////////////////////////////////////////////////////
// Display alarm table as a top level floating dialog.
//////////////////////////////////////////////////////////////////////////
function ShowAlarms( title )
{
    /* Load alarm drawing alarms.g if it hasn't been loaded already. 
       Otherwise, make previously stored AlarmDialog visible.
    */
    if( AlarmDialog == null )
        GLG.LoadWidgetFromURL( "alarms.g", null, AlarmLoadCB, 
                               /*user data*/ title );
    else
    {
        // Make AlarmDialog visible.
        ShowAlarmDialog( true );
        AlarmDialog.Update();
    }   
}

//////////////////////////////////////////////////////////////////////////
function AlarmLoadCB( /*GlgObject*/ drawing, /*String*/ title, path )
{
    if( drawing == null )
    {     
        AppAlert( "Can't load alarm drawing, check console message for details." );
        return;
    }

    // Store loaded viewport.
    AlarmDialog = drawing;

    // Adjust dialog parameters for mobile devices.
    AdjustAlarmDialogForMobileDevices();

    // Initialize alarm dialog before hierarchy setup.
    InitAlarmDialogBeforeH();

    // Setup hierarchy.
    AlarmDialog.SetupHierarchy();

    // Initialize alarm dialog.
    InitAlarmDialogAfterH();

    // Set title for the AlarmDialog.
    AlarmDialog.SetSResource( "ScreenName", title );

    // Display alarm dialog as a floating dialog.
    AlarmDialog.InitialDraw();
}

//////////////////////////////////////////////////////////////////////////
// Initialize before hierarchy setup.
//////////////////////////////////////////////////////////////////////////
function InitAlarmDialogBeforeH()
{
    // Add event listeners.
    AlarmDialog.AddListener( GLG.GlgCallbackType.INPUT_CB, AlarmInputCallback );
    AlarmDialog.AddListener( GLG.GlgCallbackType.TRACE_CB, AlarmTraceCallback );
    
    // Store an object ID of the AlarmDialog viewport.
    AlarmListVP = AlarmDialog.GetResourceObject( "Table" );
    NumAlarmRows = Math.trunc( AlarmListVP.GetDResource( "NumRows" ) );

    // Set "ProcessMouse" for the viewport to enable custom events on MouseClick.
    AlarmListVP.SetDResource( "ProcessMouse", 
                              GLG.GlgProcessMouseMask.MOUSE_CLICK );
    
    // Initialize alarm list (set initial values in the template).
    AlarmListVP.SetSResource( "Row/ID", "" );
    AlarmListVP.SetSResource( "Row/Description", "" );
    AlarmListVP.SetDResource( "Row/UseStringValue", 0.0 );
    AlarmListVP.SetDResource( "Row/DoubleValue", 0.0 );
    AlarmListVP.SetSResource( "Row/StringValue", "" );
    AlarmListVP.SetDResource( "Row/AlarmStatus", 0.0 );
    AlarmListVP.SetDResource( "Row/RowVisibility", 0.0 );

    // Set up message dialog used to display help messages.
    SetupMessageDialog();

    // Set dialog size and position.
    PositionAlarmDialog();

    // Make dialog visible.
    ShowAlarmDialog( true );
}

//////////////////////////////////////////////////////////////////////////
// Initialize after hierarchy setup.
//////////////////////////////////////////////////////////////////////////
function InitAlarmDialogAfterH()
{
    // Store object ID's of alarm rows for faster access.
    AlarmRows = new Array( NumAlarmRows );
    for( var i=0; i<NumAlarmRows; ++i )
        AlarmRows[i] = AlarmListVP.GetResourceObject( "Row" + i );
}

//////////////////////////////////////////////////////////////////////////
function GetAlarmData()
{
    DataFeed.GetAlarms( /*callback*/ AlarmDataCB, /*user data*/ null );
}

//////////////////////////////////////////////////////////////////////////
function AlarmDataCB( alarm_list )
{
    if( alarm_list != null )
    {   
        /* Store the obtained alarm list, it will be used to scroll the alarm
           list using scrollbars.
        */
        AlarmList = alarm_list;

        /* Highlight the Alarms button if there are unacknowledged alarms
           in the obtained alarm list. Push new alarm data to graphics 
           if the user clicked on the Alarms button to show the 
           alarm dialog.
        */
        ProcessAlarms();
    }
    
    // Send new data query request.
    setTimeout( GetAlarmData, ALARM_UPDATE_INTERVAL );
}

//////////////////////////////////////////////////////////////////////////
// Show/Hide alarm dialog.
//////////////////////////////////////////////////////////////////////////
function ShowAlarmDialog( /*boolean*/ show )
{
    if( AlarmDialog == null )
        return;

    /* Show the help message only when the alarm dialog is drawn for the
       first time.
    */
    if( FirstAlarmDialog )
    {
        FirstAlarmDialog = false;
        ShowMessageDialog( ( TouchDevice ? "Click" : "Ctrl-click" ) +
                           " on the alarm row to acknowledge an alarm.", false );
    }
    
    AlarmDialog.SetDResource( "Visibility", show ? 1 : 0 );
    AlarmDialogVisible = show;
}

//////////////////////////////////////////////////////////////////////////
function AlarmInputCallback( /*GlgObject*/ viewport, /*GlgObject*/ message_obj )
{
    var origin = message_obj.GetSResource( "Origin" );   /*String*/
    var format = message_obj.GetSResource( "Format" );   /*String*/
    var action = message_obj.GetSResource( "Action" );   /*String*/
    
    // Retrieve selected object ID from the message object.
    var selected_obj = message_obj.GetResourceObject( "Object" );

    // Handle window closing.
    if( format == "Window" && action == "DeleteWindow" )
    {
        if( selected_obj.Equals( AlarmDialog ) )
        {
            // Erase AlarmDialog.
            CloseAlarmDialog();
        }
        else if( selected_obj.Equals( MessageDialog ) )
        {
            MessageDialog.SetDResource( "Visibility", 0.0 );
            MessageDialog.Update();
        }
    }

    // Handle custom events.
    else if( format == "CustomEvent" )
    {
        var event_label = message_obj.GetSResource( "EventLabel" );
        var action_data = null;
        
        if( event_label == null || event_label.length == 0 )
            return;    // don't process events with empty EventLabel.
        
        var action_obj = message_obj.GetResourceObject( "ActionObject" ); 
        if( action_obj != null )
            action_data = action_obj.GetResourceObject( "ActionData" );
        
        if( event_label == "AlarmRowACK" )
        {
            // The object ID of the alarm row selected by Ctrl-click.
            var alarm_row = selected_obj;
            
            // Retrieve the tag source.
            var tag_source = alarm_row.GetSResource( "ID" );
            
            DataFeed.ACKAlarm( tag_source );
        }
        else
        {
            /* Place custom code here to handle custom events as needed. */
        }
        
        viewport.Update();
    }

    else if( format == "Button" && action == "Activate" )
    {
        /* Alarm scrolling buttons. */
        if( origin == "ScrollToTop" )
        {
            AlarmStartRow = 0;
            ProcessAlarms();
        }
        else if( origin == "ScrollUp" )
        {
            --AlarmStartRow;
            if( AlarmStartRow < 0 )
                AlarmStartRow = 0;
            ProcessAlarms();
        }
        else if( origin == "ScrollUp2" )
        {
            AlarmStartRow -= NumAlarmRows;
            if( AlarmStartRow < 0 )
                AlarmStartRow = 0;
            ProcessAlarms();
         }
        else if( origin == "ScrollDown" )
        {
            ++AlarmStartRow;
            ProcessAlarms();
        }
        else if( origin == "ScrollDown2" )
        {
            AlarmStartRow += NumAlarmRows;
            ProcessAlarms();
        }
        else if( origin == "MessageDialogOK" )
        {
            // Close message dialog.
            MessageDialog.SetDResource( "Visibility", 0.0 );
            MessageDialog.Update();
            return;
        }
        else
           return;

        viewport.Update();
    }

    else if( format == "Timer" )
        viewport.Update();
}

//////////////////////////////////////////////////////////////////////////////
// Trace callback is used to process native events of interest.
//////////////////////////////////////////////////////////////////////////////
function AlarmTraceCallback( /*GlgObject*/ viewport, /*GlgTaceData*/ trace_info )
{   
    // Process events only in the AlarmVP (Table viewport).
    if( !trace_info.viewport.Equals( AlarmListVP ) )
        return;

    var x, y;   // Cursor position.
    var event_type = trace_info.event_type;     // Native event type.

    switch( event_type )
    {
     case GLG.GlgEventType.MOUSE_PRESSED:
     case GLG.GlgEventType.MOUSE_MOVED:
        x = trace_info.mouse_x;
        y = trace_info.mouse_y;
       break;
    }
}

//////////////////////////////////////////////////////////////////////////
// Fills AlarmDialog with received alarm data if the AlarmDialog is
// visible. Highlights the Alarms button if there are unacknowledged 
// alarms in the alarm list. 
//////////////////////////////////////////////////////////////////////////
function ProcessAlarms()
{
    var num_alarms; /*int*/
    if( AlarmList == null )
        num_alarms = 0;
    else
        num_alarms = AlarmList.length;
    
    // Highlight Alarms button if there are unacknowledged alarms.
    var has_active_alarms = false;
    for( i=0; i<num_alarms; ++i )
    {
        alarm = AlarmList[i];
        if( !alarm.ack )
        {
            has_active_alarms = true;
            break;
        }
    }

    // Highlight the Alarms button.
    HighlightAlarmButton( has_active_alarms ? 1.0 : 0.0 );        
    
    if( AlarmDialog == null || !AlarmDialogVisible )
        return;
    
    /* Fill alarm rows starting with the AlarmStartRow that controls
       scrolling.
    */
    var num_visible = num_alarms - AlarmStartRow; /*int*/
    if( num_visible < 0 )
        num_visible = 0;
    else if( num_visible > NumAlarmRows )
        num_visible = NumAlarmRows;
    
    // Fill alarm rows.
    var alarm_row; /*GlgObject*/
    var alarm;     /*AlarmRecord*/
    for( var i=0; i<num_visible; ++i )
    {         
        alarm = AlarmList[i];
        alarm_row = AlarmRows[i];
        
        alarm_row.SetDResource( "AlarmIndex", AlarmStartRow + i + 1, true  );
        alarm_row.SetDResource( "TimeInput", alarm.time, true  );
        alarm_row.SetSResource( "ID", alarm.tag_source, true );
        alarm_row.SetSResource( "Description", alarm.description, true );
        
        // Set to 1 to supply string value via the StringValue resource.
        // Set to 0 to supply double value via the DoubleValue resource.
        alarm_row.SetDResource( "UseStringValue", 
                                 alarm.string_value == null ? 0.0 : 1.0, true );
         if( alarm.string_value == null )
           alarm_row.SetDResource( "DoubleValue", alarm.double_value, true );
         else
           alarm_row.SetSResource( "StringValue", alarm.string_value, true );

         alarm_row.SetDResource( "RowVisibility", 1.0, true  );
         alarm_row.SetDResource( "AlarmStatus", alarm.status, true );
        
        /* Enable blinking: will be disabled when alarm is ACK'ed. */
        alarm_row.SetDResource( "BlinkingEnabled", alarm.ack ? 0.0 : 1.0, 
                                true );
    }
    
    /* Empty the rest of the rows. Use true as the last parameter to update
       only if the value changes.
    */
    for( var i=num_visible; i<NumAlarmRows; ++i )
    {
        alarm_row = AlarmRows[i];
        
        alarm_row.SetDResource( "AlarmIndex", AlarmStartRow + i + 1, true );
        alarm_row.SetSResource( "ID", "", true );
        
        // Set status to normal to unhighlight the rightmost alarm field.
        alarm_row.SetDResource( "AlarmStatus", 0.0, true );
        
        // Make all text labels invisible.
        alarm_row.SetDResourceIf( "RowVisibility", 0.0, true );
        
        alarm_row.SetDResourceIf( "BlinkingEnabled", 0.0, true );
    }
    
    AlarmDialog.Update();
}

//////////////////////////////////////////////////////////////////////////
// Highlight Alarm button if there are unacknowledged alarms.
//////////////////////////////////////////////////////////////////////////
function HighlightAlarmButton( highlight )
{
    var button = document.getElementById( "alarms" );
    if( highlight )
        button.style.background = 'rgb(255,0,0)';
    else
        button.style.background = 'rgb(240,240,240)';
}

//////////////////////////////////////////////////////////////////////////
// Set AlarmDialog size and position.
//////////////////////////////////////////////////////////////////////////
function PositionAlarmDialog()
{
    var glg_area = document.getElementById( "glg_area" );

    var offset_x, offset_y;
    var width, height;

    if( CoordScale == 1.0 )    // Desktop
    {
        offset_x = 75;
        offset_y = 150;
        width = 600;
        height = 400;
    }
    else                      // Mobile device.
    {
        offset_x = 10;
        offset_y = 10;
        width = 320;
        height = 250;
    }

    // Set dialog position relatively to the glg_area.
    var x = glg_area.offsetLeft + offset_x;
    var y = glg_area.offsetTop + offset_y;

    // Disable dialog positioning by control points.
    AlarmDialog.SetGResource( "Point1", 0, 0, 0 );
    AlarmDialog.SetGResource( "Point2", 0, 0, 0 );

    // Set dialog initial position.
    AlarmDialog.SetDResource( "Screen/XHint", x * CoordScale );
    AlarmDialog.SetDResource( "Screen/YHint", y * CoordScale );

    // Set dialog width/height in screen coordinates.
    AlarmDialog.SetDResource( "Screen/WidthHint", width * CoordScale );
    AlarmDialog.SetDResource( "Screen/HeightHint", height * CoordScale );
}

//////////////////////////////////////////////////////////////////////////
function AdjustAlarmDialogForMobileDevices()
{
    if( CoordScale == 1.0 )   // Desktop.
        return;

    AlarmDialog.SetDResource( "Table/NumRows", 6. );
    AlarmDialog.SetDResource( "Table/NumVisibleRows", 6. );

    ScaleParameter( AlarmDialog, "ScrollWidth", CoordScale );
    ScaleParameter( AlarmDialog, "TopScrollHeight", CoordScale );
    ScaleParameter( AlarmDialog, "BottomScrollHeight", CoordScale );
    ScaleParameter( AlarmDialog, "HeaderHeight", 1.5 );
}

/////////////////////////////////////////////////////////////////////////////
// Message dialog is used for an alarm acknowledgement help message.
///////////////////////////////////////////////////////////////////////////////
function SetupMessageDialog()
{
    MessageDialog = AlarmDialog.GetResourceObject( "MessageDialog" );
    MessageDialog.SetDResource( "Visibility", 0 );    // Make it invisible.
    
    /* Make MessageDialog a free floating dialog. ShellType property
       can be also set at design time.
    */
    MessageDialog.SetDResource( "ShellType", GLG.GlgShellType.DIALOG_SHELL );
    MessageDialog.SetSResource( "Screen/ScreenName", "Message Dialog" );

    // Increase the size of the message dialog on mobile devices.
    if( CoordScale != 1.0 )
        MessageDialog.SetDResource( "CoordScale", 1.75 );
}

//////////////////////////////////////////////////////////////////////////////
function ShowMessageDialog( /*String*/ message, /*boolean*/ error )
{
    MessageDialog.SetSResource( "MessageString", message );
    
    /* Set to 1. to highlight the message in red. */
    MessageDialog.SetDResource( "ShowErrorColor", error ? 1.0 : 0.0 );
   
    MessageDialog.SetDResource( "Visibility", 1.0 );
}

//////////////////////////////////////////////////////////////////////////////
function CloseAlarmDialog()
{
    if( AlarmDialog != null )
    {
        ShowAlarmDialog( false );  // Hide the dialog.
        AlarmDialog.Update();
    }
}

