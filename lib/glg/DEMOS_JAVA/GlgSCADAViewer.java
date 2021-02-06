import java.awt.event.*;
import java.util.ArrayList;
import javax.swing.*;
import java.io.*;
import java.net.*;
import java.lang.reflect.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
// GlgSCADAViewer is used as a top level applet in this example.
// Usage: If this example is executed as a standalone java program, 
// the top level drawing name is assigned in the main method.
// If this example is used as an applet in a browser, the DrawingURL
// parameter of the applet is used in the .html file to specify the
// name of the top level drawing.  
//////////////////////////////////////////////////////////////////////////
public class GlgSCADAViewer extends GlgJBean 
   implements ActionListener, GlgHierarchyListener 
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   /* Set to true to demo with simulated data or to test without live data.
      Set to false or use -live-data command-line argument to use custom 
      live data feed.
   */
   public boolean RANDOM_DATA = true;

   /* Will be set to true if random data are used for the current page. */
   public boolean RandomData;

   // Page type constants.
   public static final int 
     UNDEFINED_PAGE_TYPE = -1,
     // Is used in the absence of PageType property in the loaded drawing.
     DEFAULT_PAGE_TYPE = 0,
     PROCESS_PAGE = 1,
     AERATION_PAGE = 2,
     CIRCUIT_PAGE = 3,
     RT_CHART_PAGE =4,
     TEST_COMMANDS_PAGE = 5;

   /* Page type table, is used to determine the type of a loaded page based
      on the PageType property of the drawing. May be extended by an 
      application to add custom pages.
   */
   TypeRecord[] PageTypeTable = {
      new TypeRecord( "Default",       DEFAULT_PAGE_TYPE ),
      new TypeRecord( "Process",       PROCESS_PAGE ),
      new TypeRecord( "Aeration",      AERATION_PAGE ),
      new TypeRecord( "Circuit",       CIRCUIT_PAGE ),
      new TypeRecord( "RealTimeChart", RT_CHART_PAGE ),
      new TypeRecord( "TestCommands",  TEST_COMMANDS_PAGE ),
      new TypeRecord( null, 0 )
   };

   static final int NO_SCREEN = -1;    // Invalid menu screen index constant.

   // CommandType contants.
   static final int 
     UNDEFINED_COMMAND_TYPE = -1,
     SHOW_ALARMS = 0,
     GOTO = 1,
     POPUP_DIALOG = 2,
     POPUP_MENU = 3,
     CLOSE_POPUP_DIALOG = 4,
     CLOSE_POPUP_MENU = 5,
     WRITE_VALUE = 6,
     WRITE_VALUE_FROM_WIDGET = 7,
     QUIT = 8;

   // DialogType contants.
   static final int
     UNDEFINED_DIALOG_TYPE = -1,
     GLOBAL_POPUP_DIALOG = 0,
     CUSTOM_DIALOG = 1,
     MAX_DIALOG_TYPE = 2;

   static final int CLOSE_ALL = UNDEFINED_DIALOG_TYPE;

   // PopupMenuType contants.
   static final int 
     UNDEFINED_POPUP_MENU_TYPE = -1,
     GLOBAL_POPUP_MENU = 0;

   // Predefined tables, can be extended by the application as needed.

   TypeRecord[] DialogTypeTable = {
      new TypeRecord( "Popup", GLOBAL_POPUP_DIALOG ),
      new TypeRecord( "CustomDialog", CUSTOM_DIALOG ),
      new TypeRecord( null, 0 )
   };
   
   TypeRecord[] PopupMenuTypeTable = {
      new TypeRecord( "PopupMenu", GLOBAL_POPUP_MENU ),
      new TypeRecord( null, 0 )
   };
   
   TypeRecord[] CommandTypeTable = {
      new TypeRecord( "ShowAlarms", SHOW_ALARMS ),
      new TypeRecord( "GoTo", GOTO ),
      new TypeRecord( "PopupDialog", POPUP_DIALOG ),
      new TypeRecord( "PopupMenu", POPUP_MENU ),
      new TypeRecord( "ClosePopupDialog", CLOSE_POPUP_DIALOG ),
      new TypeRecord ( "ClosePopupMenu", CLOSE_POPUP_MENU ),
      new TypeRecord( "WriteValue", WRITE_VALUE ),
      new TypeRecord( "WriteValueFromWidget", WRITE_VALUE_FROM_WIDGET ),
      new TypeRecord( "Quit", QUIT ),
      new TypeRecord( null, 0 )
   };

   /* StandAlone = false if this example is used as an applet in a browser;
      if StandAlone = true if it is executed as a standalone java application.
   */
   static boolean StandAlone = false;

   // Default configuration file.
   static final String DEFAULT_CONFIG_FILENAME = "scada_config_menu.txt";

   /* Filename of the menu configuration file. May be supplied using the
      -config-file command line argument. If not supplied, 
      DEFAULT_CONFIG_FILENAME is used. 
   */
   String ConfigFilename = DEFAULT_CONFIG_FILENAME;

   /* A subclass of a base class that implements functionality of an HMI page. 
      A default HMI page subclass is provided; custom HMI page subclasses
      may be used to define custom page logic, as shown in the RealTimeChart 
      and Process pages.
   */
   HMIPageBase HMIPage;

   // The type of the current page.
   public int PageType = UNDEFINED_PAGE_TYPE;

   /* Is used for supplying data for animation, either random data for a demo,
      or live data in a real application.
   */
   public DataFeedInterface DataFeed;  

   // Stores datafeed instances to create them just once.
   static public DataFeedInterface DemoDataFeed;
   static public DataFeedInterface LiveDataFeed;

   // Dynamically created array of tag records.
   ArrayList<GlgTagRecord> TagRecordArray = new ArrayList<GlgTagRecord>();

   // Number of tags records in the TagRecordArray.
   public int NumTagRecords = 0;

   GlgObject MainViewport;  /* Main viewport of the GLG bean 
                               (viewport of the top level drawing). 
                            */
   GlgObject DrawingArea;   /* Subwindow object in top level drawing */
   public GlgObject DrawingAreaVP; /* Viewport loaded into the Subwindow */
   GlgObject Menu;          /* Navigation menu viewport */
   GlgObject AlarmDialog;   /* AlarmDialog viewport  */
   GlgObject AlarmListVP;   /* Viewport containing the alarm list */
   int NumAlarmRows;        /* Number of visible alarms on one alarm page. */
   GlgObject [] AlarmRows;  /* Keeps object ID's of alarm rows for faster 
                               access. */
   int AlarmStartRow = 0;   /* Scrolled state of the alarm list. */

   ArrayList<AlarmRecord> AlarmList;   /* List of alarms. */

   boolean AlarmDialogVisible;
   boolean FirstAlarmDialog = true; /* Is used to show a help message when the
                                       alarm dialog is shown the first time. */
   GlgObject MessageDialog; /* Popup dialog used for messages. */
   
   // Array of active dialogs. Is used to open/close active dialogs.
   GlgActiveDialogRecord[] ActiveDialogs;
   
   /* Active popup menu. It is assumed that only one popup menu
      is active at a time.
   */
   GlgActivePopupMenuRecord ActivePopupMenu;
   
   int UpdateInterval;      /* Update rate in msec for drawing animation. */
   int AlarmUpdateInterval = 1000; /* Alarm update interval in msec. */

   Timer timer = null;
   Timer alarm_timer = null;

   /* Predefined menu table, used if the configuration file is not supplied, 
      or if the config file cannot be successfully parsed. 
   */
   GlgMenuRecord[] MenuTable = 
   {
      /* label, drawing name, tooltip, title */
      new GlgMenuRecord( "Solvent\nRecovery", "process.g", 
                      "process.g", "Solvent Recovery System" ),

      new GlgMenuRecord( "Water\nTreatment", "scada_aeration.g", 
                      "scada_aeration.g", "Aeration Monitoring" ),
      
      new GlgMenuRecord( "Eectrical\nCircuit", "scada_electric.g", 
                      "scada_electric.g", "Electrical Circuit Monitoring" ),
      
      new GlgMenuRecord( "Real-Time\nStrip-Chart", "scada_chart.g", 
                         "scada_chart.g", "Real-Time Strip-Chart Sample" ),

      new GlgMenuRecord( "Test Object\nCommands", "scada_test_commands.g", 
                         "scada_test_commands.g", "Test Object Commands" )

   };

   /* MenuArray is built dynamically by reading a configuration file.
      If a configuration file is not supplied or cannot be successfully 
      parsed, a predefined MenuTable is used to populate MenuArray.
   */
   ArrayList<GlgMenuRecord> MenuArray = new ArrayList<GlgMenuRecord>();
   int NumMenuItems = 0;
      
   /* Flag indicating how to supply a time stamp for a RealTimeChart embedded 
      into an HMI page: if set to 1, the application will supply a time stamp   
      explicitly. Otherwise, a time stamp will be supplied automatically by
      chart using current time. 
   */
   public boolean SUPPLY_PLOT_TIME_STAMP = false;

   // Used by DataFeed to return data values.
   DataPoint d_data_point = new DataPoint( 'D' );
   DataPoint s_data_point = new DataPoint( 'S' );

   //////////////////////////////////////////////////////////////////////////
   public GlgSCADAViewer()
   {
      super();

      /* Add Hierarchy Listener (HierarchyCallback) to handle events
         when a new drawing is loaded into a Subwindow object.
      */
      AddListener( GlgObject.HIERARCHY_CB, this );

      // Add Trace Listener used to handle low level events.
      AddListener( GlgObject.TRACE_CB, this );

      // Disable not used old-style select callback.
      SelectEnabled = false;

      /* Disable automatic update for input events to avoid slowing down 
         real-time chart updates.
      */
      SetAutoUpdateOnInput( false );

      HMIPage = new EmptyHMIPage( this );    // Initialize to empty page.

      CreateDataFeed();
   }

   //////////////////////////////////////////////////////////////////////////
   // For use as a stand-alone application
   //////////////////////////////////////////////////////////////////////////
   public static void main( final String [] arg )
   {
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Main( arg ); } } );
   }

   //////////////////////////////////////////////////////////////////////////
   public static void Main( final String [] arg )
   {
      // Program is executed as a standalone program.
      StandAlone = true;

      class DemoQuit extends WindowAdapter
      {
         public void windowClosing( WindowEvent e ) { System.exit( 0 ); }
      } 

      JFrame frame = new JFrame( "GLG SCADA Viewer" );

      frame.setResizable( true );
      frame.setSize( 900, 700 );
      frame.setLocation( 0, 0 );

      GlgSCADAViewer glg_viewer = new GlgSCADAViewer();
      frame.getContentPane().add( glg_viewer );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );

      // Process command line arguments which can supply configuration file.
      glg_viewer.ProcessArgs( arg );

      /* Assign a drawing filename after the frame became visible and 
         determined its client size to avoid unnecessary resizing of 
         the drawing.
         Loading the drawing triggers ReadyCallback which starts updates.
      */
      glg_viewer.SetDrawingName( "scada_main.g" );
   }

   //////////////////////////////////////////////////////////////////////
   // HCallback() is invoked after the drawing is loaded, but before
   // object hierarchy has been set up.
   /////////////////////////////////////////////////////////////////////
   public void HCallback( GlgObject viewport )
   {
      /* If used as an applet in the browser, get ConfigFile from 
         the applet parameters. For a stand-alone program, command line 
         arguments are used.
      */
      if( !StandAlone )
      {
         /* If used as an applet in the browser, get the configuration filename
            from the applet's parameter.
         */
         String param = getParameter( "ConfigFile" );
         if( param != null )
           ConfigFilename = param;

         param = getParameter( "RandomData" );
         if( param != null )
           RANDOM_DATA = param.equalsIgnoreCase( "true" );

         if( RANDOM_DATA )
           System.out.println( "Using random demo DataFeed." );
         else
           System.out.println( "Using live DataFeed." );
      }

      // Initialize MenuArray from the configuration file.
      FillMenuArray( ConfigFilename );

      /* Initialize ActiveDialogs array and ActivePopupMenu. */
      InitActivePopups();
      
      /* Store object ID of the bean's viewport, i.e. a viewport of the 
         loaded top level drawing.
      */
      MainViewport = viewport;

      /* Store an object ID of the viewport named Menu, containing navigation
         buttons allowing to switch between drawings.
      */
      Menu = viewport.GetResourceObject( "Menu" );
      
      // Store an object ID of the Subwindow object named DrawingArea.
      DrawingArea = viewport.GetResourceObject( "DrawingArea" );
      if( DrawingArea == null ) // no DrawingArea found
      {
         System.out.println( "Can't find DrawingArea Subwindow object." );
         System.exit( 1 );
      }

      // Store an object ID of the AlarmDialog viewport.
      AlarmDialog = viewport.GetResourceObject( "AlarmDialog" );
      AlarmListVP = AlarmDialog.GetResourceObject( "Table" );
      NumAlarmRows = AlarmListVP.GetDResource( "NumRows" ).intValue();

      /* Make AlarmDialog a free floating dialog. ShellType property
         can be also set at design time.
      */
      AlarmDialog.SetDResource( "ShellType", 
                                (double) GlgObject.DIALOG_SHELL );
      
      // Initialize alarm list (set initial values in the template).
      AlarmListVP.SetSResource( "Row/ID", "" );
      AlarmListVP.SetSResource( "Row/Description", "" );
      AlarmListVP.SetDResource( "Row/UseStringValue", 0.0 );
      AlarmListVP.SetDResource( "Row/DoubleValue", 0.0 );
      AlarmListVP.SetSResource( "Row/StringValue", "" );
      AlarmListVP.SetDResource( "Row/AlarmStatus", 0.0 );
      AlarmListVP.SetDResource( "Row/RowVisibility", 0.0 );

      // Make AlarmDialog initially invisible.
      AlarmDialog.SetDResource( "Visibility", 0.0 );
      AlarmDialogVisible = false;

      // Set title for the AlarmDialog.
      AlarmDialog.SetSResource( "ScreenName", "Alarm List" );

      // Set initial state of the alarm button.
      viewport.SetDResource( "AlarmButton/Blinking", 0.0 );

      /* Make Global PopupDialog a floating DialogShell type. */
      viewport.SetDResource( "PopupDialog/ShellType", 
                             (double) GlgObject.DIALOG_SHELL );
      
      // Make Global PopupDialog initially invisible.
      viewport.SetDResource( "PopupDialog/Visibility", 0.0 );
      
      // Make Global PopupMenu initially invisible.
      viewport.SetDResource( "PopupMenu/Visibility", 0.0 );
      
      // Make message dialog invisible on startup.      
      MessageDialog = viewport.GetResourceObject( "MessageDialog" );
      MessageDialog.SetDResource( "Visibility", 0.0 );

      /* Make MessageDialog a free floating dialog. ShellType property
         can be also set at design time.
      */
      MessageDialog.SetDResource( "ShellType", 
                                  (double) GlgObject.DIALOG_SHELL );

      // If it is an applet, hide the QuitButton -- QuitButton is used
      // in a standalone program only to exit the application.
      if( !StandAlone )
        viewport.SetDResource( "QuitButton/Visibility", 0.0 );

      // Set the number of menu items. It must be done BEFORE hierarchy setup.
      SetDResource( Menu, "NumRows", NumMenuItems );
   }

   //////////////////////////////////////////////////////////////////////
   // VCallback() is invoked after the drawing is loaded and setup,
   // but before it is drawn for the first time.
   ///////////////////////////////////////////////sg//////////////////////
   public void VCallback( GlgObject viewport )
   {
      // Initialize the navigation menu.
      InitMenu();
      
      // Store object ID's of alarm rows for faster access.
      AlarmRows = new GlgObject[ NumAlarmRows ];
      for( int i=0; i<NumAlarmRows; ++i )
        AlarmRows[i] = AlarmListVP.GetResourceObject( "Row" + i );

      // Load drawing corresponding to the first menu item.
      LoadDrawingFromMenu( 0, /* update menu object */ true );
   }

   //////////////////////////////////////////////////////////////////////////
   // Data feeds are reused for different pages - create just once.
   //////////////////////////////////////////////////////////////////////////
   void CreateDataFeed()
   {      
      DemoDataFeed = new DemoDataFeed( this );
      
      LiveDataFeed = new LiveDataFeed( this );
   }

   //////////////////////////////////////////////////////////////////////////
   // Ready callback is invoked after the drawing is drawn for the
   // first time.
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );

      StartUpdates();
   }

   //////////////////////////////////////////////////////////////////////////
   // Processes command line arguments.
   //////////////////////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////////
   // Processes command line arguments.
   //////////////////////////////////////////////////////////////////////////
   public void ProcessArgs( String [] arg )
   {
      if( arg == null )
        return;

      int num_args = arg.length;
      int curr_arg = 0;

      while( curr_arg < num_args )
      {
         // Use configuration file supplied as a command-line argument.
         if( arg[ curr_arg ].equals( "-config-file" ) )
         {
            if( curr_arg + 1 < num_args )
              ConfigFilename = arg[ curr_arg + 1 ];
            else
              GlgObject.Error( GlgObject.WARNING, 
                               "Missing configuration file name", null );
         }
         else if( arg[ curr_arg ].equals( "-random-data" ) )
           RANDOM_DATA = true;
         else if( arg[ curr_arg ].equals( "-live-data" ) )
           RANDOM_DATA = false;

         ++curr_arg;
      }

      if( RANDOM_DATA )
        System.out.println( "Using random demo DataFeed." );
      else
        System.out.println( "Using live DataFeed." );
   }

   //////////////////////////////////////////////////////////////////////
   // Read MenuArray from a configuration file. If configuration file 
   // is not supplied, use predefined MenuTable array.
   //////////////////////////////////////////////////////////////////////
   void FillMenuArray( String config_filename )
   {
      MenuArray = ReadMenuConfig( config_filename );       
      if( MenuArray == null )
      {
         System.out.println( "Can't read config file: using predefined Menu Table." );
         
         // Fill MenuArry from a predefined array MenuTable.
         MenuArray = new ArrayList<GlgMenuRecord>();
         int num_items = MenuTable.length;
         for( int i=0; i < num_items; ++i )
           MenuArray.add( MenuTable[ i ] );
      }
      
      NumMenuItems = MenuArray.size();
   }
   
   //////////////////////////////////////////////////////////////////////
   // Initialize the navigation menu, a viewport named "Menu", based on
   // the menu records from the supplied configuration file.
   //////////////////////////////////////////////////////////////////////
   void InitMenu()
   {
      GlgObject button;
      
      GlgMenuRecord menu_record;
      // Populate menu buttons based on the MenuArray.
      for( int i=0; i<NumMenuItems; ++i )
      {
         button = Menu.GetResourceObject( "Button" + i );

         menu_record = MenuArray.get( i );
         button.SetSResource( "LabelString", menu_record.label_string );
         button.SetSResource( "TooltipString", menu_record.tooltip_string );
      }

      SelectMainMenuItem( NO_SCREEN, true );
   }
   
   //////////////////////////////////////////////////////////////////////
   // StartUpdates() create a timer to perform periodic updates.
   // The timer invokes the bean's UpdateData method to update
   // dynamic attributes with the new data values.
   // A separate timer is used to update alarms with a different 
   // frequency.
   /////////////////////////////////////////////////////////////////////
   public void StartUpdates()
   {
      /* Get new update interval depending on the displayed screen. */
      UpdateInterval = HMIPage.GetUpdateInterval();
      if( UpdateInterval == 0 )
        return;

      if( timer == null )
      {
         timer = new Timer( UpdateInterval, this );
         timer.setRepeats( false );
         timer.start();
      }

      if( alarm_timer == null )
      {
         alarm_timer = 
           new Timer( AlarmUpdateInterval, new AlarmActionListener() );
         alarm_timer.setRepeats( false );
         alarm_timer.start();
      }

      UpdateData();   // Fill data for the initial appearance.
   }

   ///////////////////////////////////////////////////////////////////////
   // StopUpdate() method stops periodic updates
   ///////////////////////////////////////////////////////////////////////
   public void StopUpdates()
   {
      if( timer != null )
      {
         timer.stop();
         timer = null;
      }

      if( alarm_timer != null )
      {
         alarm_timer.stop();
         alarm_timer = null;
      }
   }

   ///////////////////////////////////////////////////////////////////////
   // Invoked by the browser to stop the applet
   //////////////////////////////////////////////////////////////////////
   public void stop()
   {
      StopUpdates();
      super.stop();
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked periodically by the timer to update the drawing with data.
   //////////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      if( timer == null )
        return;   // Prevents race conditions

      UpdateData();
      
      timer.start();   // Restart the update timer. 
   }

   //////////////////////////////////////////////////////////////////////////
   // AlarmActionListener class used by the alarm timer.
   //////////////////////////////////////////////////////////////////////////
   public class AlarmActionListener implements ActionListener
   {
      ///////////////////////////////////////////////////////////////////////
      // Invoked periodically by the alarm timer.
      //////////////////////////////////////////////////////////////////////
      public void actionPerformed( ActionEvent e )
      {	 
         if( alarm_timer == null )
           return;   // Prevents race conditions

         ProcessAlarms( true );

         alarm_timer.start();    // Restart alarms timer
      }
   }

   //////////////////////////////////////////////////////////////////////
   // Traverses an array of tag records, gets new data for each tag and 
   // updates the drawing with new values.
   //////////////////////////////////////////////////////////////////////
   public void UpdateData()
   {
      /* Invoke a page's custom update method, if implemented.
         Don't update any tags if the custom method returns true.
      */         
      if( HMIPage.UpdateData() )
        return;

      /* Always update all tags defined in the current page, as well as 
         any additional tags defined in popup dialogs in the main drawing, 
         outside of the current page.
      */
      int num_tag_records = TagRecordArray.size();
      for( int i=0; i<num_tag_records; ++i )
      {
         GlgTagRecord tag_record = TagRecordArray.get( i );
         
         switch( tag_record.data_type )
         {
          case GlgObject.D:
            // Obtain a new numerical data value for a given tag. 
            if( DataFeed.ReadDTag( tag_record, d_data_point ) )
            {
               /* Push a new data value into a given tag. If the last argument 
                  (if_changed flag) is true, the value is pushed into graphics
                  only if it changed. Otherwise, a new value is always 
                  pushed into graphics. The if_changed flag is ignored for tags
                  attached to the plots in a real time chart, and the new value
                  is always pushed to the chart even if it is the same.
               */
               SetDTag( tag_record.tag_source, d_data_point.d_value, true );

               /* Push a time stamp to the TimeEntryPoint of a plot in 
                  a real-time chart, if found.
               */ 
               if( tag_record.plot_time_ep != null )
                 tag_record.plot_time_ep.SetDResource( null, d_data_point.time_stamp );
            }
            break;
            
          case GlgObject.S:    
            // Obtain a new string data value for a given tag. 
            if( DataFeed.ReadSTag( tag_record, s_data_point ) )            
            {
               // Push new data.
               SetSTag( tag_record.tag_source, s_data_point.s_value, true );
            }
            break;
         }
      }

      // Update the drawing with new data.
      Update();
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Fills AlarmList dialog with received alarm data.
   //////////////////////////////////////////////////////////////////////////
   public void ProcessAlarms( boolean query_new_list )
   {
      if( query_new_list )
        // Get a new alarm list.
        AlarmList = DataFeed.GetAlarms();

      int num_alarms;
      if( AlarmList == null )
        num_alarms = 0;
      else
        num_alarms = AlarmList.size();

      // Activate Alarms button's blinking if there are unacknowledged alarms.
      boolean has_active_alarms = false;
      for( int i=0; i<num_alarms; ++i )
      {
         AlarmRecord alarm = AlarmList.get( i );
         if( !alarm.ack )
         {
            has_active_alarms = true;
            break;
         }
      }
      SetDResource( "AlarmButton/Blinking", has_active_alarms ? 1.0 : 0.0, 
                    true );         

      if( !AlarmDialogVisible )
      {
         Update();
         return;
      }

      /* Fill alarm rows starting with the AlarmStartRow that controls
         scrolling.
      */
      int num_visible = num_alarms - AlarmStartRow;
      if( num_visible < 0 )
        num_visible = 0;
      else if( num_visible > NumAlarmRows )
        num_visible = NumAlarmRows;

      // Fill alarm rows.
      GlgObject alarm_row;
      for( int i=0; i<num_visible; ++i )
      {         
         AlarmRecord alarm = AlarmList.get( i );
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
         alarm_row.SetDResource( "AlarmStatus", (double) alarm.status, true );

         /* Enable blinking: will be disabled when alarm is ACK'ed. */
         alarm_row.SetDResource( "BlinkingEnabled", alarm.ack ? 0.0 : 1.0, 
                                 true );
      }

      /* Empty the rest of the rows. Use true as the last parameter to update
         only if the value changes.
      */
      for( int i=num_visible; i<NumAlarmRows; ++i )
      {
         alarm_row = AlarmRows[i];

         alarm_row.SetDResource( "AlarmIndex", AlarmStartRow + i + 1, true );
         alarm_row.SetSResource( "ID", "", true );

         // Set status to normal to unhighlight the rightmost alarm field.
         alarm_row.SetDResource( "AlarmStatus", 0.0, true );

         // Make all text labels invisible.
         alarm_row.SetDResource( "RowVisibility", 0.0, true );

         alarm_row.SetDResource( "BlinkingEnabled", 0.0, true );
       }

      AlarmDialog.Update();
   }

   //////////////////////////////////////////////////////////////////////
   // Input callback is invoked when the user interacts with objects in a
   // GLG drawing. It is used to handle events occurred in input objects,
   // such as a menu, as well as Commands or Custom Mouse Events attached 
   // to objects at design time.
   //////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject viewport, 
                                        GlgObject message_obj )
   {
      String
        origin,
        format,
        action;      
      int menu_index;       
      GlgObject selected_obj;
      GlgObject action_obj;
      
      /* Return if the page's custom input handler processed the input.
         Otherwise, continue to process common events and commands.
      */
      if( HMIPage.InputCallback( viewport, message_obj ) )
        return;

      origin = message_obj.GetSResource( "Origin" );
      format = message_obj.GetSResource( "Format" );
      action = message_obj.GetSResource( "Action" );
      
      // Retrieve selected object.
      selected_obj = message_obj.GetResourceObject( "Object" );
      
      // Handle events from the screen navigation menu, named "Menu".
      if( format.equals( "Menu" ) )
      {
         if( !action.equals( "Activate" ) || !origin.equals( "Menu" ) )
           return;
      
         /* User selected a button from the menu object named Menu. 
            Load a new drawing associated with the selected button.
         */
         menu_index = message_obj.GetDResource( "SelectedIndex" ).intValue();

         StopUpdates();

         /* Load the drawing associated with the selected menu button and
            display it in the DrawingArea.
         */
         LoadDrawingFromMenu( menu_index,
                              /* don't update menu object */ false );
         StartUpdates();
         viewport.Update();
      }
      
      /* Handle custom commands attached to objects in the drawing at 
         design time.
      */
      else if( format.equals( "Command" ) )
      {
         action_obj = message_obj.GetResourceObject( "ActionObject" ); 
         ProcessObjectCommand( viewport, selected_obj, action_obj );
         viewport.Update();
      }

      // Handle zoom controls on the left.
      else if( format.equals( "Button" ) )
      {
         if( !action.equals( "Activate" ) &&      /* Not a push button */
             !action.equals( "ValueChanged" ) )   /* Not a toggle button */
           return;
          
         if( origin.equals( "MessageDialogOK" ) )
         {
            // Close message dialog.
            MessageDialog.SetDResource( "Visibility", 0.0 );
            MessageDialog.Update();
         }
     
         /* Zoom and pan buttons. */
         else if( origin.equals( "Left" ) )
           Zoom( DrawingAreaVP, 'l', 0.1 );
         else if( origin.equals( "Right" ) )
           Zoom( DrawingAreaVP, 'r', 0.1 );
         else if( origin.equals( "Up" ) )
           Zoom( DrawingAreaVP, 'u', 0.1 );
         else if( origin.equals( "Down" ) )
           Zoom( DrawingAreaVP, 'd', 0.1 );
         else if( origin.equals( "ZoomIn" ) )
           Zoom( DrawingAreaVP, 'i', 1.5 );
         else if( origin.equals( "ZoomOut" ) )
           Zoom( DrawingAreaVP, 'o', 1.5 );
         else if( origin.equals( "ZoomTo" ) )
           Zoom( DrawingAreaVP, 't', 0.0 );
         else if( origin.equals( "ZoomReset" ) )
           Zoom( DrawingAreaVP, 'n', 0.0 );

         /* Alarm scrolling buttons. */
         else if( origin.equals( "ScrollToTop" ) )
         {
            AlarmStartRow = 0;
            ProcessAlarms( false );
         }
         else if( origin.equals( "ScrollUp" ) )
         {
            --AlarmStartRow;
            if( AlarmStartRow < 0 )
              AlarmStartRow = 0;
            ProcessAlarms( false );
         }
         else if( origin.equals( "ScrollUp2" ) )
         {
            AlarmStartRow -= NumAlarmRows;
            if( AlarmStartRow < 0 )
              AlarmStartRow = 0;
            ProcessAlarms( false );
         }
         else if( origin.equals( "ScrollDown" ) )
         {
            ++AlarmStartRow;
            ProcessAlarms( false );
         }
         else if( origin.equals( "ScrollDown2" ) )
         {
            AlarmStartRow += NumAlarmRows;
            ProcessAlarms( false );
         }
         else
           return;

         viewport.Update();
      }
            
      // Handle custom events.
      else if( format.equals( "CustomEvent" ) )
      {
         String event_label = message_obj.GetSResource( "EventLabel" );
         GlgObject action_data = null;

         if( event_label == null || event_label.isEmpty() )
           return;    // don't process events with empty EventLabel.

         /* Retrieve action object. It will be null for custom events 
            added prior to GLG v.3.5. 
            If action object is present, retrieve its ActionData object.
            If Action Data object present, use its properties for custom event 
            processing as needed.
         */
         action_obj = message_obj.GetResourceObject( "ActionObject" ); 
         if( action_obj != null )
           action_data = action_obj.GetResourceObject( "ActionData" );

         if( event_label.equals( "AlarmRowACK" ) )
         {
            // The object ID of the alarm row selected by Ctrl-click.
            GlgObject alarm_row = selected_obj;

            // Retrieve the tag source.
            String tag_source = alarm_row.GetSResource( "ID" );
            
            DataFeed.ACKAlarm( tag_source );
         }
         else
         {
            /* Place custom code here to handle custom events as needed. */
         }

         viewport.Update();
      }
            
      /* Handle events from a Real-Time Chart. */
      else if( format.equals( "Chart" ) && action.equals( "CrossHairUpdate" ) )
      {
         /* To avoid slowing down real-time chart updates, invoke Update() 
            to redraw cross-hair only if the chart is not updated fast 
            enough by the timer.
         */
         if( UpdateInterval > 100 )
           viewport.Update();
      }
      
      // Handle Timer events, generated by objects with blinking dynamics.
      else if( format.equals( "Timer" ) )  
        viewport.Update();

      // Handle window closing events.
      else if( format.equals( "Window" ) && action.equals( "DeleteWindow" ) )
      {
         if( selected_obj == null )
           return;

         /* If the event came from the top level application window,
            exit the process.
         */
         if( selected_obj == MainViewport )
         {
            // If it is a standalone program, exit the application; 
            if( StandAlone )
              System.exit( 0 );
         }
         else if( selected_obj == AlarmDialog )
           ShowAlarms();   // Toggle the state of alarms dialog to erase it.

         /* If the closing window is found in the ActiveDialogs array, 
            close the active dialog. 
         */
         else
         {
            for( int i=0; i<MAX_DIALOG_TYPE; ++i )
              if( selected_obj == ActiveDialogs[ i ].dialog )
              {
                 ClosePopupDialog( i );
                 break;
              }
         }

         viewport.Update();
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Is used to handle low level events, such as obtaining coordinates of 
   // the mouse click, or keyboard events. 
   //////////////////////////////////////////////////////////////////////////
   public void TraceCallback( GlgObject viewport, 
                                        GlgTraceData trace_info )
   {      
      double x, y;
      int key;
      GlgObject event_vp = trace_info.viewport;
      
      /* Return if the page's custom trace callback processed the event.
         Otherwise, continue to process common events.
      */
      if( HMIPage.TraceCallback( viewport, trace_info ) )
        return;

      int event_type = trace_info.event.getID();
            
      switch( event_type )
      {
       case MouseEvent.MOUSE_PRESSED:
       case MouseEvent.MOUSE_MOVED:
       case MouseEvent.MOUSE_DRAGGED:
         // Obtain mouse coordinates. 
         x = (double) ((MouseEvent)trace_info.event).getX();
         y = (double) ((MouseEvent)trace_info.event).getY();
         break;

       case KeyEvent.KEY_PRESSED:
         // Obtain key code.
         key = ((KeyEvent)trace_info.event).getKeyCode();

         // Handle key codes as needed.
         switch( key )
         {
          case KeyEvent.VK_ESCAPE:
            break;
          default: break;
         }
         break;

       default: return;
      }
   }

   ///////////////////////////////////////////////////////////////////////
   // Hierarchy callback, added to the top level viewport and invoked when 
   // a new drawing is loaded into any subwindow or subdrawing object. 
   // In this demo, this callback processes events only from the reference
   // of type "Subwindow" (such as DrawingArea).
   //////////////////////////////////////////////////////////////////////
   public void HierarchyCallback( GlgObject viewport, 
                                            GlgHierarchyData info_data )
   {
      // Handle events only from Subwindow-type reference objects.
      int obj_type = 
        info_data.object.GetDResource( "ReferenceType" ).intValue();

      if( obj_type != GlgObject.SUBWINDOW_REF )
        return;
      
      /* This callback is invoked twice: one time before hierarchy setup
         for the new drawing, and second time after hierarchy setup.
         Drawing initialization can be done here if needed.
      */
      switch( info_data.condition )
      {
       case GlgObject.BEFORE_SETUP_CB:
         GlgObject drawing_vp = info_data.subobject;
         if( drawing_vp == null )
         {
            System.out.println( "Drawing loading failed" ); 
            if( info_data.object == DrawingArea )
              DrawingAreaVP = null;
            return;
         }

         /* Set "ProcessMouse" attribute for the loaded viewport, to
            process custom events and tooltips.
         */
         drawing_vp.SetDResource( "ProcessMouse", 
                                ( GlgObject.MOUSE_CLICK | 
                                  GlgObject.MOUSE_OVER_TOOLTIP ) );
         
         /* Set "OwnsInputCB" attribute for the loaded viewport,
            so that Input callback is invoked with this viewport ID.
         */
         drawing_vp.SetDResource( "OwnsInputCB", 1.0 ); 

         if( info_data.object == DrawingArea )
         {
            DrawingAreaVP = drawing_vp;
            SetupHMIPage();

            /* Initialize loaded drawing before setup. */
            HMIPage.InitBeforeSetup();
         }
         break;
         
       case GlgObject.AFTER_SETUP_CB:
         /* Initialize loaded drawing after setup. */
         if( info_data.object == DrawingArea )
           HMIPage.InitAfterSetup();
         break;
 
       default: break;
      }
   }

   //////////////////////////////////////////////////////////////////////
   // Assigns a new page to HMIPage variable, either as a default or 
   // custom HMI page, based on the PageType property of the loaded 
   // drawing. The HMI page class handles the page logic and user 
   // interaction.
   // 
   // If the PageType property doesn't exist in the drawing or is set to 
   // "Default", DefaultHMIPage class is used for the new page.
   // Otherwise, a page class type corresponding to the PageType property 
   // is assigned to HMIPage variable. For example, RTChartPage class is 
   // used if PageType property of the loaded drawing is set to 
   // "RealTimeChart".
   // 
   // The method also assigns DataFeed for the new page.
   //////////////////////////////////////////////////////////////////////
   void SetupHMIPage()
   { 
      PageType = GetPageType( DrawingAreaVP );     // Get page type.

      /* Use live data if requested with the -live-data command-line option, 
         otherwise use simulated random data for testing. 
         RANDOM_DATA may be set to false to use live data by default.
      */
      if( RANDOM_DATA )
        DataFeed = DemoDataFeed;
      else
        DataFeed = LiveDataFeed;

      switch( PageType )
      {
       case UNDEFINED_PAGE_TYPE:
         HMIPage = new EmptyHMIPage( this );
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
         // Test page: always use demo data.
         DataFeed = DemoDataFeed;
                                            
         HMIPage = new DefaultHMIPage( this );
         break;

       default:
         /* New custom page: use live data if requested with the -live-data 
            command-line option, otherwise use simulated random data for 
            testing. RANDOM_DATA may be set to false to use live data by 
            default.
         */
         HMIPage = new DefaultHMIPage( this );
         break;
      }

      // True if DemoDataFeed is used for the current page.
      RandomData = ( DataFeed == DemoDataFeed );
   }

   //////////////////////////////////////////////////////////////////////
   // Load a new drawing into the DrawingArea when the user selects an item
   // from the navigation menu object (Menu object).
   //////////////////////////////////////////////////////////////////////
   boolean LoadDrawingFromMenu( int screen, boolean update_menu )
   {
      if( !SelectMainMenuItem( screen, update_menu ) )
        return false;    // Invalid page index - don't load.

      // Close active popup dialogs and popup menu, if any.
      CloseActivePopups( CLOSE_ALL );

      GlgMenuRecord menu_record = MenuArray.get( screen  );

      /* Loads a new drawing into the DrawingArea subwindow object.
         DrawingAreaVP is assigned in the Hierarchy callback to the 
         ID of the $Widget viewport of the loaded drawing.
      */
      LoadDrawing( DrawingArea, menu_record.drawing_name );
      if( DrawingAreaVP == null )
      {
         DeleteTagRecords();
         PageType = UNDEFINED_PAGE_TYPE;
         HMIPage = new EmptyHMIPage( this );
         return false;
      }
      
      /* Query a list of tags from the loaded drawing and build a new
         TagRecordArray.
      */
      QueryTags( DrawingAreaVP );
      
      // Set title from the selected menu record, etc.
      SetupLoadedPage( menu_record.drawing_title );

      HMIPage.Ready();
      return true;
   }

   //////////////////////////////////////////////////////////////////////
   void SetupLoadedPage( String title )
   {
      // Set new title. 
      MainViewport.SetSResource( "Title", title );

      /* Set the color of the top level window and menus could
         to match the color of the loaded drawing. 
      */
      GlgPoint color = DrawingAreaVP.GetGResource( "FillColor" );
      MainViewport.SetGResource( "FillColor", color.x, color.y, color.z );      
   }
   
   //////////////////////////////////////////////////////////////////////
   // Load a new drawing into the specified subwindow object and return 
   // a newly loaded viewport. 
   //////////////////////////////////////////////////////////////////////
   public GlgObject LoadDrawing( GlgObject subwindow, String drawing_file )
   {
      if( subwindow == null || drawing_file == null )
      {
         System.out.println( "Drawing loading failed" );
         return null;
      }

      /* Assign a new drawing name to the subwindow object, 
         if the current drawing name has changed.
      */
      subwindow.SetSResource( "SourcePath", drawing_file, 
                              true /* if changed */ );
  
      /* Setup hierarchy for the subwindow object, which causes the 
         new drawing to be loaded into the subwindow. 
         HierarchyCallback will be invoked before and after hierarchy 
         setup for newly loaded drawing. This callback can be used 
         to invoke code for initializing the newly loaded drawing.
      */
      SetupHierarchy( subwindow );

      // Return newly loaded viewport.
      return( subwindow.GetResourceObject( "Instance" ) );
   }

   //////////////////////////////////////////////////////////////////////
   // Query tags for a given viewport and rebuild TagRecordArray. 
   // The new_drawing parameter is the viewport of the new drawing. 
   // TagRecordArray will include tags for the top level viewport 
   // of the viewer, including tags for the loaded page, as well as
   // tags for the popup dialogs, if any.
   //////////////////////////////////////////////////////////////////////
   public void QueryTags( GlgObject new_drawing )
   {
      // Delete existing tag records from TagRecordArray.
      DeleteTagRecords();
      
      /* Remap tags in the loaded drawing if needed.
         Will invoke HMIPage's RemapTagObject() for each tag.
      */
      if( new_drawing != null && HMIPage.NeedTagRemapping() )
        RemapTags( new_drawing );
      
      /* Build an array of tag records containing tags information and
         store it in TagRecordArray. TagRecordArray will be used for 
         unimating the drawing with data.
         Always create data for the top level viewport (MainViewport),
         to keep a global list of tags that include tags in any dynamically
         loaded dialogs.
      */
      CreateTagRecords( MainViewport );
   }

   //////////////////////////////////////////////////////////////////////
   // Create an array of tag records containing tag information.
   //////////////////////////////////////////////////////////////////////
   public void CreateTagRecords( GlgObject drawing_vp )
   {
      GlgObject tag_obj;
      String 
        tag_source, 
        tag_name,
        tag_comment;
      int data_type;
      int access_type;

      // Obtain a list of tags with unique tag sources.
      GlgObject tag_list = 
         drawing_vp.CreateTagList( /* List each tag source only once */ true );
      if( tag_list == null )
         return;   
      
      int size = tag_list.GetSize();
      if( size == 0 )
        return; // no tags found 
      
      for( int i=0; i<size; ++i )
      {
         tag_obj = (GlgObject) tag_list.GetElement( i );
         
         // Skip OUTPUT tags.
         access_type = tag_obj.GetDResource( "TagAccessType" ).intValue();
         if( access_type == GlgObject.OUTPUT_TAG )
           continue;

         /* Retrieve TagSource, the name of the database field used as a 
            data source. */
         tag_source = tag_obj.GetSResource( "TagSource" );

         // Skip undefined tag sources, such as "" or "unset".
         if( IsUndefined( tag_source ) )
           continue; 
         
         if( RandomData )
         {
            /* For demo purposes only, skip tags that have:
               - TagName contains "Speed" or "Test";
               - TagSource contains "Test";
               - TagComment contains "Test".
               Such tags are present in motor_info.g displayed in the 
               PopupDialog, as well as scada_test_commands.g. 
               The demo shows how to set the value for these tags 
               using commands WriteValue or WriteValueFromWidget. 
            */
            tag_name = tag_obj.GetSResource( "TagName" );
            tag_comment = tag_obj.GetSResource( "TagComment" );
            if( !IsUndefined( tag_name ) && 
                ( tag_name.contains( "Speed" ) || tag_name.contains( "Test" ) ) )
              continue;
            if( tag_source.contains( "Test" ) )
              continue;
            if( !IsUndefined( tag_comment ) && tag_comment.contains( "Test" ) )
              continue;
         }

         // Get tag object's data type: GLG_D, GLG_S or GLG_G
         data_type = 
           tag_obj.GetDResource( "DataType" ).intValue();
         
         // Create a new tag record.
         GlgTagRecord tag_record = new GlgTagRecord();
         tag_record.tag_source = tag_source;
         tag_record.data_type = data_type;
         tag_record.tag_obj = tag_obj;
         
         /* Set plot_time_ep to null by default. It will be assigned
            in  SetPlotTimeEP() if needed.
         */
         tag_record.plot_time_ep = null;

         // Add a new tag record to TagRecordArray
         TagRecordArray.add( tag_record );
      }
      
      // Store number of tag records.
      NumTagRecords = TagRecordArray.size();

      /* If a drawing contains a chart, ValueEntryPoint of each plot 
         may have a tag to push data values using the common tag mechanism. 
         The time stamp for the corresponding TimeEntryPoint of the plot
         may be supplied either automatically by the chart 
         (SUPPLY_PLOT_TIME_STAMP=false), or the application may supply the
         time stamp explicitly for each data sample 
         (SUPPLY_PLOT_TIME_STAMP=true).
         
         If SUPPLY_PLOT_TIME_STAMP=1, each tag record in TagRecordArray
         should store corresponding plot's TimeEntryPoint (plot_time_ep).
         If not found, plot_time_ep will be null.
      */
      if( SUPPLY_PLOT_TIME_STAMP )
        SetPlotTimeEP( drawing_vp );
   }
   
   //////////////////////////////////////////////////////////////////////
   // Delete tag records frogm TagRecordArray.
   //////////////////////////////////////////////////////////////////////
   public void DeleteTagRecords()
   {
      if( TagRecordArray.size() > 0 )
        TagRecordArray.clear();

      NumTagRecords = 0;      
   }

   //////////////////////////////////////////////////////////////////////
   // Store TimeEntryPoint (plot_time_ep) in each tag record, if found.
   //////////////////////////////////////////////////////////////////////
   void SetPlotTimeEP( GlgObject drawing_vp )
   {
      if( NumTagRecords == 0 )
        return;
      
      // Obtain a list of all tags, including non-unique tag sources.
      GlgObject tag_list = 
        drawing_vp.CreateTagList( /* List all tags */ false );
      
      if( tag_list == null )
        return;
      
      int size = tag_list.GetSize();
      if( size == 0 )
        return; /* no tags found */
      
      /* For each tag in the list, check if there is a plot object in the
         drawing with a matching TagSource for the plot's ValueEntryPoint.
         If found, obtain plot's TimeEntryPoint and store it in the
         TagRecordArray for a tag record with a matching tag_source.
      */
      for( int i=0; i<size; ++i )
      {
         GlgObject tag_obj = (GlgObject) tag_list.GetElement( i );
         
         /* Check if the tag belongs to a chart's Entry Point.
            If yes, proceed with finding the PLOT object the tag belongs to.
            Otherwise, skip this tag object.
         */
         if( tag_obj.GetDResource( "AlwaysChanged" ).doubleValue() == 0 )
           continue;
         
         /* Retrieve TagSource and TagComment. In the demo, TagComment is
            not used, but the application may use it as needed.
         */
         String tag_comment = tag_obj.GetSResource( "TagComment" );
         String tag_source = tag_obj.GetSResource( "TagSource" );
         
         if( IsUndefined( tag_source ) )
           return;
         
         /* Find a TimeEntryPoint of a plot in a RealTimeChart with
            a matching TagSource assigned for the plot's ValueEntryPoint.
            It is assumed that there is ONLY ONE plot in the drawing 
            with a given TagSource. 
         */
         GlgObject plot_time_ep = FindMatchingTimeEP( tag_obj );
         
         if( plot_time_ep == null )
           continue;   /* There is no plot for this tag source. */
         
         /* Find a tag record in TagRecordArray with a matching tag_source.
            If found, assign plot_time_ep. Otherwise, generate an error.
         */
         GlgTagRecord tag_record = LookupTagRecords( tag_source );
         
         if( tag_record != null )
           tag_record.plot_time_ep = plot_time_ep;
         else /* shouldn't happen */
           GlgObject.Error( GlgObject.USER_ERROR, 
                           "No matching tag record, TimeEntryPoint not stored.",
                            null );
      }
   }

   //////////////////////////////////////////////////////////////////////
   // For a given tag object, find a parent plot object (PLOT object type). 
   // If found, return the plot's TimeEntryPoint.
   // It is assumed that there is ONLY ONE plot in the drawing with a given
   // TagSource. 
   //////////////////////////////////////////////////////////////////////
   GlgObject FindMatchingTimeEP( GlgObject tag_obj )
   {
      /* Fill in match data structure to search for a plot object type,
         which is a parent of the tag_obj (ValueEntryPoint) .
      */
      GlgFindMatchingObjectsData match_data = new GlgFindMatchingObjectsData(); 

      match_data.match_type = GlgObject.OBJECT_TYPE_MATCH;
      match_data.find_parents = true;
      match_data.find_first_match = true;
      match_data.search_inside = false;
      match_data.object_type = GlgObject.PLOT;

      if( !tag_obj.FindMatchingObjects( match_data ) || 
          match_data.found_object == null )
        return null; /* matching object not found */
      
      GlgObject plot = match_data.found_object;
      return plot.GetResourceObject( "TimeEntryPoint" );
   }

   //////////////////////////////////////////////////////////////////////
   // Lookup TagRecordArray and return a matching tag record with 
   // tag_source=match_tag_source.
   //////////////////////////////////////////////////////////////////////
   GlgTagRecord LookupTagRecords( String match_tag_source )
   {
      if( IsUndefined( match_tag_source ) )
        return null;

      int num_tag_records = TagRecordArray.size();
      for( int i=0; i<num_tag_records; ++i )
      {
         GlgTagRecord tag_record = TagRecordArray.get( i );

         if( tag_record.tag_source.equals( match_tag_source ) )
           return tag_record;
      }
      
      return null; // not found
   }
   
   //////////////////////////////////////////////////////////////////////
   // Close all active popups, including popup dialogs and popup menus.
   //////////////////////////////////////////////////////////////////////
   public void CloseActivePopups( int allow_dialog )
   {
      /* Close all active dialogs, except the ones which may remain
         visible until closed by the user, as specified by the allow_dialog 
         parameter.
      */
      for( int i=0; i<MAX_DIALOG_TYPE; ++i )
      {
         if( i == allow_dialog )
           continue; 
         
         ClosePopupDialog( i );
      }
      
      /* Close Global PopupMenu, if any. */
      ClosePopupMenu( GLOBAL_POPUP_MENU );
   }

   //////////////////////////////////////////////////////////////////////
   // Process commands attached to objects at design time.
   // Command types are defined in CommandTypeTable.
   // CommandType strings in the table must match the CommandType strings 
   // defined in the drawing.
   //////////////////////////////////////////////////////////////////////
   public void ProcessObjectCommand( GlgObject command_vp, 
                                     GlgObject selected_obj, 
                                     GlgObject action_obj )
   {
      GlgObject command_obj;
      int 
        command_type,
        dialog_type,
        menu_type;
      
      if( selected_obj == null || action_obj == null )
        return;
      
      /* Retrieve Command object. */
      command_obj = action_obj.GetResourceObject( "Command" );
      if( command_obj == null )
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
         // If it is a standalone program, exit the application.
         if( StandAlone )
           System.exit( 0 );
         break;
       default: 
         System.out.println( "Command failed: Undefined CommandType." );
         break;
      }
   }

   //////////////////////////////////////////////////////////////////////   
   // Opens or closes the alarm window.
   //////////////////////////////////////////////////////////////////////   
   void ShowAlarms()
   {
      AlarmDialogVisible = ToggleResource( "AlarmDialog/Visibility" );
      
      /* If the alarm dialog is becoming visible, fill alarms to show them
         right away.
      */
      if( AlarmDialogVisible )
        ProcessAlarms( true );

      if( FirstAlarmDialog )   // Show the help message the first time only.
      {         
         ShowMessageDialog( "Ctrl-click on the alarm row to acknowledge " +
                            "an alarm.", false );
         FirstAlarmDialog = false;
      }
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   void ShowMessageDialog( String message, boolean error )
   {
      MessageDialog.SetSResource( "MessageString", message );

      /* Set to 1. to highlight the message in red. */
      MessageDialog.SetDResource( "ShowErrorColor", error ? 1.0 : 0.0 );

      MessageDialog.SetDResource( "Visibility", 1.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   // Toggles resource value between 0 and 1 and returns the new value
   // as a boolean.
   //////////////////////////////////////////////////////////////////////////
   boolean ToggleResource( String resource_name )
   {
      double current_value = 
        MainViewport.GetDResource( resource_name ).doubleValue();

      current_value = ( current_value == 0.0 ? 1.0 : 0.0 );

      MainViewport.SetDResource( resource_name, current_value );
      return ( current_value == 1.0 );      
   }

   //////////////////////////////////////////////////////////////////////   
   // Process command "PopupDialog". The requested dialog can be embedded 
   // into the currently loaded drawing, or a global PopupDialog can be used 
   // as a popup. Command parameters DialogType and DialogResource are
   // mandatory. DialogResource specifies the resource path of the dialog 
   // to be displayed. If DialogResource starts with a forward slash ('/'), 
   // the dialog name is relative to the top level viewport MainViewport; 
   // otherwise, it is relative to the currently loaded viewport where the 
   // command occurred, i.e. command_vp.
   //
   // The popup dialog is initialized as needed, using parameters of the 
   // selected object. In this demo, the tag sources are transfered from the 
   // selected object to the dialog viewport for the tags with a matching
   // TagName.
   //////////////////////////////////////////////////////////////////////   
   public void DisplayPopupDialog( GlgObject command_vp, GlgObject selected_obj, 
                                   GlgObject command_obj )     
   {
      GlgObject 
        subwindow = null, 
        popup_vp = null,
        dialog = null;
      
      /* Retrieve command parameters. */
      String dialog_res = command_obj.GetSResource( "DialogResource" );
      String drawing_file = command_obj.GetSResource( "DrawingFile" );
      String destination_res = command_obj.GetSResource( "Destination" );
      String title = command_obj.GetSResource( "Title" );
      
      /* Obtain DialogType. */
      int dialog_type = GetDialogType( command_obj );
      if( dialog_type == UNDEFINED_DIALOG_TYPE )
      {
         System.out.println( "PopupDialog Command failed: Unknown DialogType." );
         return;
      }
      
      /* Close active popups, if any. To avoid flickering, do not close the 
         dialog with the same dialog type that is requested by this command.
      */
      CloseActivePopups( dialog_type );
      
      /* DialogResource specifies resource path of the dialog to be displayed.
         If the path starts with '/', it is relative to the top level 
         $Widget viewport. Otherwise, the path is relative to the 
         viewport of the Input callback (command_vp).
      */
      if( IsUndefined( dialog_res ) )
      {
         System.out.println( "PopupDialog Command failed: Invalid DialogResource." );
         ClosePopupDialog( dialog_type );
         return;
      }
      
      /* Obtain an object ID of the requested popup dialog. 
         If invalid, abort the command. 
      */
      if( dialog_res.startsWith("/") )
        /* skip '/' */
        dialog = MainViewport.GetResourceObject( dialog_res.substring( 1 ) );
      else 
        dialog = command_vp.GetResourceObject( dialog_res );
      
      if( dialog == null )
      {
         System.out.println( "PopupDialog Command failed: Dialog not found." );
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
         String subwindow_name;

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
         if( subwindow == null )
         {
            System.out.println( "PopupDialog Command failed: Destination object not found." );
            ClosePopupDialog( dialog_type );
            return;
         }
         
         /* Load new drawing and obtain an object id of the newly loaded 
            viewport. If drawing loading failed, the error gets 
            reported in the HierarchyCallback.
         */
         popup_vp = LoadDrawing( subwindow, drawing_file );
         if( popup_vp == null )
         {
            System.out.println( "PopupDialog Command failed." );
            ClosePopupDialog( dialog_type );
            return;
         }
      }
      
      /* For the tags with matching TagName, transfer tag sources from the 
         selected object to the loaded popup viewport.
      */
      TransferTags( selected_obj, popup_vp, false );
      
      /* If a new dialog drawing was loaded, rebuild TagRecordArray to 
         include tags both for the drawing displayed in the main drawing area 
         and drawing displayed in the popup dialog.
      */
      if( popup_vp != dialog )
        QueryTags( popup_vp );
      
      /* Poll new data to fill the popup dialog with current values. */
      UpdateData();
      
      /* Display title in the loaded viewport, if Title resource is found. */
      if( popup_vp.HasResourceObject( "Title" ) )
        popup_vp.SetSResource( "Title", title );
      
      /* Display title as the dialog caption. */
      dialog.SetSResource( "ScreenName", title );

      // If running as an applet in a browser: if the dialog is visible,
      // erase and reopen it again to make sure it is not hidden under other 
      // windows. Browser's Java security agent reparents the applet's popup
      // dialog in a top-level application shell which does not stay on top 
      // of the applet's window and may be hidden. Erase and reopen to avoid
      // confusing the user. 
      if( !StandAlone && ActiveDialogs[ dialog_type ].isVisible )
      {
         dialog.SetDResource( "Visibility", 0.0 );
         dialog.Update();
         ActiveDialogs[ dialog_type ].isVisible = false;
      }

      /* Display the dialog if it is not up already. */
      dialog.SetDResource( "Visibility", 1.0 );
      
      /* Store dialog information in ActiveDialogs array */
      ActiveDialogs[ dialog_type ].dialog_type = dialog_type;
      ActiveDialogs[ dialog_type ].dialog = dialog;
      ActiveDialogs[ dialog_type ].subwindow = subwindow;
      ActiveDialogs[ dialog_type ].popup_vp = popup_vp;
      ActiveDialogs[ dialog_type ].isVisible = true;

      dialog.Update();
   }

   ////////////////////////////////////////////////////////////////////// 
   // Close active popup dialog of a given type.
   //////////////////////////////////////////////////////////////////////      
   public void ClosePopupDialog( int dialog_type )
   {
      if( dialog_type == UNDEFINED_DIALOG_TYPE || 
          dialog_type >= MAX_DIALOG_TYPE )
      {
         System.out.println( "Dialog closing failed." ); 
         return;
      }

      if( ActiveDialogs[ dialog_type ].dialog == null )
        return; /* nothing to do. */
      
      if( ActiveDialogs[ dialog_type ].subwindow != null && 
          ActiveDialogs[ dialog_type ].popup_vp != null )
      {
         /* Destroy currently loaded popup drawing and load empty drawing. */
         LoadDrawing( ActiveDialogs[ dialog_type ].subwindow, 
                      "empty_drawing.g" );
         
         /* Rebuild a list of tags to exclude the tags from the previously
            loaded popup viewport.
         */
         QueryTags( null );
      }
      
      /* Hide the dialog */
      ActiveDialogs[ dialog_type ].dialog.SetDResource( "Visibility", 0.0 );
      
      /* Clear a dialog record with a specified index (dialog_type)
         in the ActiveDialogs array.
      */
      ActiveDialogs[ dialog_type ].dialog_type = UNDEFINED_DIALOG_TYPE;
      ActiveDialogs[ dialog_type ].dialog = null;
      ActiveDialogs[ dialog_type ].subwindow = null;
      ActiveDialogs[ dialog_type ].popup_vp = null;
      ActiveDialogs[ dialog_type ].isVisible = false;
   }

   ////////////////////////////////////////////////////////////////////// 
   // Transfer tag sources for tags with a matching TagName from the
   // selected object to the specified viewport.
   ////////////////////////////////////////////////////////////////////// 
   public void TransferTags( GlgObject selected_obj, GlgObject viewport, 
                             boolean unset_tags )
   {
      GlgObject 
        tag_list,
        tag_obj;      
      String 
        widget_type,
        tag_source,
        tag_name;
      int num_remapped_tags;

      /* Retrieve WidgetType from the selected objects, if any. */
      if( selected_obj.HasResourceObject( "WidgetType" ) )
        widget_type = selected_obj.GetSResource( "WidgetType" );
      
      /* Place custom code here to initialize the drawing based on WidgetType, 
         if needed. In this demo, the initialization code below is executed 
         regardless of WidgetType.
      */
      
      /* Obtain a list of tags defined in the selected object. */
      tag_list = selected_obj.CreateTagList( /* List all tags */ false );        
      if( tag_list == null )
        return;
   
      int size = tag_list.GetSize();
      if( size == 0 )
        return; /* no tags found */
      
      /* Traverse the tag list. For each tag, transfer the TagSource
         defined in the selected object to the tag in the loaded 
         popup drawing that has a matching TagName.
      */
      for( int i=0; i<size; ++i )
      {
         tag_obj = (GlgObject) tag_list.GetElement( i );
         
         /* Obtain TagName. */
         tag_name = tag_obj.GetSResource( "TagName" );
         
         /* Skip tags with undefined TagName */
         if( IsUndefined( tag_name ) )
           continue;
         
         /* Obtain TagSource. */
         tag_source = tag_obj.GetSResource( "TagSource" );
         
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
      }
   }
   
   ////////////////////////////////////////////////////////////////////// 
   // Process command "PopupMenu". The requested popup menu can be embedded 
   // in the currently loaded drawing, or a global PopupMenu can be used 
   // as a popup. Command parameters MenuType and MenuResource are
   // mandatory. MenuResource specifies the resource path of the menu object 
   // to be displayed. If MenuResource starts with a forward slash ('/'), 
   // the menu name is relative to the top level viewport MainViewport; 
   // otherwise, it is relative to the currently loaded viewport where the 
   // command occurred, i.e. command_vp.
   //
   // The popup menu is initialized as needed, using parameters of the 
   // selected object. In this demo, the tag sources are transfered from the 
   // selected object to the menu viewport for the tags with a matching
   // TagName.
   ////////////////////////////////////////////////////////////////////// 
   public void DisplayPopupMenu( GlgObject command_vp, GlgObject selected_obj, 
                                 GlgObject command_obj )
   {
      GlgObject 
        subwindow = null, 
        popup_vp = null,
        menu_obj = null;
      
      /* Retrieve command parameters. */
      String menu_res = command_obj.GetSResource( "MenuResource" );
      String drawing_file = command_obj.GetSResource( "DrawingFile" );
      String destination_res = command_obj.GetSResource( "Destination" );
      String title = command_obj.GetSResource("Title" );
      
      /* Obtain MenuType. */
      int menu_type = GetPopupMenuType( command_obj );
      if( menu_type == UNDEFINED_POPUP_MENU_TYPE )
      {
         System.out.println( "PopupMenu Command failed: Unknown MenuType." );
         return;
      }
      
      /* Close active popups, if any. */
      CloseActivePopups( CLOSE_ALL );
      
      /* MenuResource specifies resource path of the menu object to be 
         displayed. If the path starts with '/', it is relative to the 
         top level $Widget viewport. Otherwise, the path is relative to 
         the viewport of the Input callback (command_vp).
      */
      if( IsUndefined( menu_res ) )
      {
         System.out.println( "PopupMenu Command failed: Invalid MenuResource." );
         return;
      }
      
      /* Obtain an object ID of the requested popup menu. 
         If invalid, abort the command. 
      */
      if( menu_res.startsWith("/") )
        /* skip '/'*/
        menu_obj = MainViewport.GetResourceObject( menu_res.substring( 1 ) );
      else 
        menu_obj = command_vp.GetResourceObject( menu_res );
      
      if( menu_obj == null )
      {
         System.out.println( "PopupMenu Command failed: Menu object not found." );
         return;
      }
      
      if( IsUndefined( drawing_file ) )
        /* DrawingFile is not defined, use menu_obj as popup viewport. */
        popup_vp = menu_obj;
      else
      {
         String subwindow_name;

         /* DrawingFile is defined, use it to load the corresponding 
            drawing into the subwindow object defined by the Destination 
            parameter. It is assumed that Destination points to the 
            subwindow object inside the menu object.
         */
         if( IsUndefined( destination_res ) ) 
           subwindow_name = "DrawingArea"; /* Use default name. */
         else
           subwindow_name = destination_res;
         
         /* Obtain an object ID of the subwindow inside the menu object. */
         subwindow = menu_obj.GetResourceObject( subwindow_name );
         if( subwindow == null )
         {
            System.out.println( "PopupDialog Command failed: Destination object not found." );
            return;
         }
         
         /* Load new drawing and store an object id of the newly loaded 
            viewport. If drawing loading failed, the error gets reported 
            in the HierarchyCallback.
         */
         popup_vp = LoadDrawing( subwindow, drawing_file );
         if( popup_vp == null )
         {
            System.out.println( "PopupMenu Command failed." );
            return;
         }
         
         /* If the viewport has Width and Height resources that define
            its size in pixels, adjust the size of the menu object 
            to match the size of the loaded viewport.
         */
         if( popup_vp.HasResourceObject( "Width" ) && 
             menu_obj.HasResourceObject( "Width" ) )
         {
            double menu_width;
            menu_width = popup_vp.GetDResource( "Width" ).doubleValue();
            menu_obj.SetDResource( "Width", menu_width );
         }

         if( popup_vp.HasResourceObject( "Height" ) && 
             menu_obj.HasResourceObject( "Height" ) )
         {
            double menu_height;
            menu_height =  popup_vp.GetDResource( "Height" ).doubleValue();
            menu_obj.SetDResource( "Height", menu_height );
         }
      }
      
      /* Transfer tag sources from the selected object to the loaded 
         popup viewport, using tags with a matching TagName.
      */
      TransferTags( selected_obj, popup_vp, false );
      
      /* Display title in the loaded viewport, if Title resource is found. */
      if( popup_vp.HasResourceObject( "Title" ) )
        popup_vp.SetSResource( "Title", title );
      
      /* Show the menu. */
      menu_obj.SetDResource( "Visibility", 1.0 );
      
      /* Store menu information in the global ActivePopupMenu structure, 
         is used to close the active popup menu.
      */
      ActivePopupMenu.menu_type = menu_type;
      ActivePopupMenu.menu_obj = menu_obj;
      ActivePopupMenu.subwindow = subwindow;
      ActivePopupMenu.menu_vp = popup_vp;
      ActivePopupMenu.selected_obj = selected_obj; 
      ActivePopupMenu.isVisible = true; 
      
      /* Position the menu next to the selected object. */
      PositionPopupMenu();

      menu_obj.Update();
   }

   ////////////////////////////////////////////////////////////////////// 
   // Position ActivePopupMenu at the upper right corner of the selected object,
   // if possible. Otherwise, position the menu close to the selected object
   // such that it is displayed within the current viewport.
   ////////////////////////////////////////////////////////////////////// 
   public void PositionPopupMenu()
   {
      GlgObject 
        selected_obj_vp,  // Viewport that contains selected object.
        menu_parent_vp;   // Parent viewport that contains the popup menu.      
      double 
        x, y,
        offset = 5.0, // offset in pixels.
        menu_width, menu_height,
        parent_width, parent_height; 
      int
        x_anchor,
        y_anchor; 
      
      if( ActivePopupMenu.selected_obj == null || 
          ActivePopupMenu.menu_obj == null )
        return;
      
      selected_obj_vp = ActivePopupMenu.selected_obj.GetParentViewport( true );
      menu_parent_vp = ActivePopupMenu.menu_obj.GetParentViewport( true );
      
      /* Obtain the object's bounding box in screen coordinates. */
      GlgCube sel_obj_box = ActivePopupMenu.selected_obj.GetBox();   
      GlgCube converted_box = new GlgCube( sel_obj_box );
      
      /* If the menu is located in a different viewport from the viewport
         of the selected object, convert screen coordinates of the 
         selected object box from the viewport of the selected object to the 
         viewport that contains the popup menu.
      */
      if( selected_obj_vp != menu_parent_vp )
      {
         GlgObject.TranslatePointOrigin( selected_obj_vp, menu_parent_vp, 
                                         converted_box.p1 );
         GlgObject.TranslatePointOrigin( selected_obj_vp, menu_parent_vp, 
                                         converted_box.p2 );
      }
      
      /* Obtain width and height in pixels of the parent viewport 
         of the menu. 
      */
      parent_width = 
        menu_parent_vp.GetDResource( "Screen/Width" ).doubleValue();
      parent_height = 
        menu_parent_vp.GetDResource( "Screen/Height" ).doubleValue();
      
      /* Obtain width and height of the menu object. */
      GlgCube menu_obj_box = ActivePopupMenu.menu_obj.GetBox(); 
      menu_width = menu_obj_box.p2.x - menu_obj_box.p1.x;
      menu_height = menu_obj_box.p2.y - menu_obj_box.p1.y;
      
      /* Position the popup at the upper right or lower left corner of 
         the selected object, if possible. Otherwise (viewport is too small), 
         position it in the center of the viewport.
      */   
      if( converted_box.p2.x + menu_width + offset > parent_width )
      {
         /* Outside of the window right edge.
            Position the right edge of the popup to the left of the selected
            object. 
            Always use HLEFT anchor to simplify out-of-the-window check.
         */
         x =  converted_box.p1.x - offset - menu_width;
         x_anchor = GlgObject.HLEFT;
      }
      else 
      {
         /* Position the left edge of the popup to the right of the selected
            object. 
         */
         x = converted_box.p2.x + offset; 
         x_anchor = GlgObject.HLEFT;
      }
      
      /* Anchor is always HLEFT here to make checks simpler. */
      if( x < 0 || x + menu_width > parent_width )
      {
         /* Not enough space: place in the center. */
         x = parent_width / 2.0;
         x_anchor = GlgObject.HCENTER;
      }
      
      if( converted_box.p1.y - menu_height - offset < 0.0 ) 
      {
         /* Outside of the window top edge.
            Position the top edge of the popup below the selected object.
         */
         y =  converted_box.p2.y + offset;
         y_anchor = GlgObject.VTOP;
      }
      else 
      {
         /* Position the bottom edge of the popup above the selected object.
            Always use GLG_VTOP anchor to simplify out-of-the-window check.
         */
         y = converted_box.p1.y - offset - menu_height; 
         y_anchor = GlgObject.VTOP;
      }
      
      /* Anchor is always GLG_VTOP here to make checks simpler. */
      if( y < 0 || y + menu_height > parent_height )
      {
         /* Not enough space: place in the center. */
         y = parent_height / 2.0;
         y_anchor = GlgObject.HCENTER;
      }
      
      ActivePopupMenu.menu_obj.PositionObject( GlgObject.SCREEN_COORD, 
                                               x_anchor | y_anchor, x, y, 0.0 );
   }
   
   ////////////////////////////////////////////////////////////////////// 
   // Close active popup menu. In this demo, menu_type is not used, since
   // there is only a single ActivePopupMenu object. The code can be 
   // extended by the application developer as needed.
   ////////////////////////////////////////////////////////////////////// 
   public void ClosePopupMenu( int menu_type )
   {
      if( ActivePopupMenu.menu_obj == null )
        return; /* Nothing to do. */
      
      /* Hide active popup. */
      ActivePopupMenu.menu_obj.SetDResource( "Visibility", 0.0 );
      
      if( ActivePopupMenu.subwindow != null )
      {
         /* Destroy currently loaded popup drawing and load empty drawing. */
         LoadDrawing( ActivePopupMenu.subwindow, "empty_drawing.g" );
      }
      else
      {
         /* Unset tags in the menu object, which were previously
            transfered and assigned from the selected object. 
         */ 
         if( ActivePopupMenu.selected_obj != null )
           TransferTags( ActivePopupMenu.selected_obj, 
                         ActivePopupMenu.menu_obj, true );
      }
      
      /* Clear menu record. */
      ActivePopupMenu.menu_type = UNDEFINED_POPUP_MENU_TYPE;
      ActivePopupMenu.menu_obj = null;
      ActivePopupMenu.subwindow = null;
      ActivePopupMenu.menu_vp = null;
      ActivePopupMenu.selected_obj = null;
      ActivePopupMenu.isVisible = false;
   }
   
   ////////////////////////////////////////////////////////////////////// 
   // Process "GoTo" command. The command loads a new drawing specified
   // by the DrawingFile parameter into the subwindow object specified
   // by the Destination parameter. If Destination is omitted, uses
   // main DrawingArea subwindow object.  
   ////////////////////////////////////////////////////////////////////// 
   public void GoTo( GlgObject command_vp, GlgObject selected_obj, 
                     GlgObject command_obj )
   {
      GlgObject subwindow;
      GlgObject drawing_vp;
      int screen_index;
      
      /* Close active popup dialogs and popup menu. */
      CloseActivePopups( CLOSE_ALL );
      
      /* Retrieve command parameters. */
      String drawing_file = command_obj.GetSResource( "DrawingFile" );
      String destination_res = command_obj.GetSResource( "Destination" );
      String title = command_obj.GetSResource( "Title" );
      
      /* If DrawingFile is not valid, abort the command. */
      if( IsUndefined( drawing_file ) )
      {
         System.out.println( "GoTo Command failed: Invalid DrawingFile." );
         return;
      }
         
      /* Use Destination resource, if any, to display the specified drawing. 
         It is assumed that Destination points to the subwindow object.
         If not defined, use top level DrawingArea subwindow by default.
      */
      if( IsUndefined( destination_res ) ) 
        subwindow = DrawingArea;
      else
      {
         if( destination_res.startsWith("/") )
           /* Destination is relative to the top level viewport (MainViewport).
              Omit the first '/' when using the resource path to obtain the 
              subwindow object.
           */
           subwindow = 
             MainViewport.GetResourceObject( destination_res.substring( 1 ) );
         else 
           /* Destination is relative to the current viewport, where the
              command occurred.
           */
           subwindow = command_vp.GetResourceObject( destination_res );
         
         if( subwindow == null )
         {
            System.out.println( "GoTo Command failed: Invalid Destination." );
            return;
         }
      }
         
      /* Load new drawing and obtain an object id of the newly loaded viewport. 
         If drawing loading failed, the error gets reported in the 
         HierarchyCallback.
      */
      drawing_vp = LoadDrawing( subwindow, drawing_file );
      if( drawing_vp == null )
      {
         /* If drawing loading fails, it will be reported in 
            HierarchyCallback. Generate an additional error 
            indicating command failing.
         */
         System.out.println( "GoTo Command failed." );
         return;
      }      
      
      /* Rebuild TagRecordArray for the newly loaded drawing. */
      QueryTags( drawing_vp );

      if( subwindow == DrawingArea )
      {
         /* Reset main menu selection. If the new drawing matches one of 
            the drawings defined in the MenuArray, update 
            main menu with the corresponding index.
         */
         screen_index = LookUpMenuArray( drawing_file );
         SelectMainMenuItem( screen_index, /* update menu */ true );
         
         SetupLoadedPage( title );   // Use title from the command, etc.

         HMIPage.Ready();
      }
   }
   
   //////////////////////////////////////////////////////////////////////    
   // Process command "WriteValue". The command writes a new value specified
   // by the Value parameter into the tag in the back-end system
   // specfied by the OutputTagHolder.
   ////////////////////////////////////////////////////////////////////// 
   public void WriteValue( GlgObject command_vp, GlgObject selected_obj, 
                           GlgObject command_obj )
   {
      /* Retrieve tag source to write data to. */
      String tag_source = 
        command_obj.GetSResource( "OutputTagHolder/TagSource" );
      
      /* Validate. */
      if( IsUndefined( tag_source ) )
      {
         System.out.println( "WriteValue Command failed: Invalid TagSource." );
         return;
      }
      
      /* Retrieve the value to be written to the tag source. */
      double value = command_obj.GetDResource( "Value" ).doubleValue();
      
      /* Place custom code here as needed, to validate the value specified
         in the command.
      */
      
      /* Write new value to the specified tag source. */
      d_data_point.d_value = value;
      DataFeed.WriteDTag( tag_source, d_data_point );
      
      if( RandomData )
      {
         /* For demo purposes, update the tag value in the drawing. 
            In an application, the input tag will be updated 
            from the back-end system.
         */
         SetDTag( tag_source, value, true );
         Update();
      }
   }
   
   ////////////////////////////////////////////////////////////////////// 
   // Process command "WriteValueFromWidget". The command allows writing
   // a new value into the tag in the back-end system using an input
   // widget, such as a toggle or a spinner.
   ////////////////////////////////////////////////////////////////////// 
   public void WriteValueFromInputWidget( GlgObject command_vp, 
                                          GlgObject selected_obj, 
                                          GlgObject command_obj )
   {
      GlgObject write_tag_obj, read_tag_obj;

      String value_res = command_obj.GetSResource( "ValueResource" );
   
      /* Obtain object ID of the read tag object. FOr example, in case of a
         spinner widget, it will be "Value" resource.
      */
      read_tag_obj = selected_obj.GetResourceObject( value_res );
      if( read_tag_obj == null )
        return;
      
      /* Obtain object ID of the write tag object. */
      write_tag_obj = command_obj.GetResourceObject( "OutputTagHolder" ); 
      if( write_tag_obj == null )
        return;
      
      /* Obtain TagSource from the write tag. */
      String output_tag_source = write_tag_obj.GetSResource( "TagSource" );
      
      /* Validate. */
      if( IsUndefined( output_tag_source ) )
      {
         System.out.println( "Write Command failed: Invalid Output TagSource." );
         return;
      }
      
      /* Retrieve new value from the input widget. */
      double value = read_tag_obj.GetDResource( null ).doubleValue();
      
      /* Write the obtained value from the widget to the output tag. */
      d_data_point.d_value = value;
      DataFeed.WriteDTag( output_tag_source, d_data_point );
      
      if( RandomData )
      {
         /* Update the tag value in the drawing. In an application,
            the read tag will be updated from the back-end system.
         */
         String read_tag_source = read_tag_obj.GetSResource( "TagSource" ); 
         if( !IsUndefined( read_tag_source ) )
         {
            SetDTag( read_tag_source, value, true );
            Update();
         }
      }
   }

   ////////////////////////////////////////////////////////////////////// 
   // Performs zoom/pan operations of the specified type.
   ////////////////////////////////////////////////////////////////////// 
   public void Zoom( GlgObject viewport, char zoom_type, double scale )
   {
      char zoom_reset_type = 'n';
      
      if( viewport == null )
        return;
   
      switch( zoom_type )
      {
       default: 
         viewport.SetZoom( null, zoom_type, scale );
         break;
         
       case 'n':
         /* If a viewport is a chart with the chart zoom mode, use 'N'
            to reset both Time and Y ranges. For a chart, 'n' would reset 
            only the Time range.
         */
         GlgObject zoom_mode_obj = 
           viewport.GetResourceObject( "ZoomModeObject" );
         if( zoom_mode_obj != null )
         {
            int object_type = 
              zoom_mode_obj.GetDResource( "Type" ).intValue();
            if( object_type == GlgObject.CHART )
              zoom_reset_type = 'N';
         }
         
         viewport.SetZoom( null, zoom_reset_type, 0.0 );
         break;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Initialize ActiveDialogs array and ActivePopupMenu.
   //////////////////////////////////////////////////////////////////////////
   public void InitActivePopups()
   {
      ActiveDialogs = new GlgActiveDialogRecord[ MAX_DIALOG_TYPE ];

      /* Initialize ActiveDialogs array. */
      for( int i=0; i<MAX_DIALOG_TYPE; ++i )
         ActiveDialogs[ i ] = 
           new GlgActiveDialogRecord( UNDEFINED_DIALOG_TYPE, 
                                      null, null, null, false );
       
      /* Initialize ActivePopupMenu. */
      ActivePopupMenu = 
        new GlgActivePopupMenuRecord( UNDEFINED_POPUP_MENU_TYPE,
                                      null, null, null, null, false );
   }

   //////////////////////////////////////////////////////////////////////////
   // Returns index of the MenuArray item with a matching drawing_name,
   // if any. If not found, returns NO_SCREEN.
   //////////////////////////////////////////////////////////////////////////
   int LookUpMenuArray( String drawing_name )
   {
      GlgMenuRecord menu_record;

      for( int i=0; i<NumMenuItems; ++i )
      {
         menu_record = MenuArray.get( i );
         if( drawing_name.equals( menu_record.drawing_name ) )
          return i;
      }
      
      return NO_SCREEN;
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Select MainMenu item with a specified index.
   // NO_SCREEN value (-1) unselects a previously selected menu item, if any.
   //////////////////////////////////////////////////////////////////////////
   public boolean SelectMainMenuItem( int menu_index, boolean update_menu )
   {
      /* Validate menu_index. */
      if( menu_index < NO_SCREEN || menu_index >= NumMenuItems )
      {
         System.out.println( "Invalid main menu index." );
         return false;
      }
      
      if( update_menu )
        Menu.SetDResource( "SelectedIndex", menu_index );
      return true;
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Utility function, queries a PageType property of the drawing and 
   // converts it to a PageType integer constant using PageTypeTable.
   //////////////////////////////////////////////////////////////////////////
   public int GetPageType( GlgObject drawing )
   {
      if( drawing == null )
        return DEFAULT_PAGE_TYPE;

      GlgObject type_obj = drawing.GetResourceObject( "PageType" );
      if( type_obj == null )
        return DEFAULT_PAGE_TYPE;

      String type_str = type_obj.GetSResource( null );
 
      int page_type = ConvertStringToType( PageTypeTable, type_str,
                                           UNDEFINED_PAGE_TYPE,
                                           UNDEFINED_PAGE_TYPE );
      if( page_type == UNDEFINED_PAGE_TYPE )
      {
         GlgObject.Error( GlgObject.USER_ERROR, "Invalid PageType.", null );
         return DEFAULT_PAGE_TYPE;
      }
      return page_type;
   }

   //////////////////////////////////////////////////////////////////////////
   // Returns CommandType integer constant using CommandTypeTable.
   //////////////////////////////////////////////////////////////////////////
   public int GetCommandType( GlgObject command_obj )
   {
      String command_type_str = command_obj.GetSResource( "CommandType" );
 
      return ConvertStringToType( CommandTypeTable, command_type_str,
                                  UNDEFINED_COMMAND_TYPE,
                                  UNDEFINED_COMMAND_TYPE );
   }

   //////////////////////////////////////////////////////////////////////////
   // Returns PopupMenuType integer constant using PopupMenuTypeTable.
   //////////////////////////////////////////////////////////////////////////
   int GetPopupMenuType( GlgObject command_obj )
   {
      String menu_type_str = command_obj.GetSResource( "MenuType" );
      
      return ConvertStringToType( PopupMenuTypeTable, menu_type_str,
                                  GLOBAL_POPUP_MENU,
                                  UNDEFINED_POPUP_MENU_TYPE );
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Returns DialogType integer constant using DialogTypeTable.
   //////////////////////////////////////////////////////////////////////////
   public int GetDialogType( GlgObject command_obj )
   {
      String dialog_type_str = command_obj.GetSResource( "DialogType" );
      
      return ConvertStringToType( DialogTypeTable, dialog_type_str,
                                  GLOBAL_POPUP_DIALOG, UNDEFINED_DIALOG_TYPE );
   }
   
   //////////////////////////////////////////////////////////////////////
   // Remap all object tags with the specified tag_name to use a new 
   // tag_source. 
   //////////////////////////////////////////////////////////////////////
   int RemapNamedTags( GlgObject glg_object, 
                       String tag_name, String tag_source )
   {
      GlgObject tag_obj;
      int size;

      /* Obtain a list of tags with TagName attribute matching 
         the specified tag_name.
      */
      GlgObject tag_list = 
        glg_object.GetTagObject( tag_name, /* by name */ true, 
                                 /* list all tags */ false, 
                                 /* multiple tags mode */ false, 
                                 GlgObject.DATA_TAG );
      if( tag_list == null )
         size = 0;
      else
         size = tag_list.GetSize();
      
      for( int i=0; i<size; ++i )
      {
         tag_obj = (GlgObject) tag_list.GetElement( i );
         
         /* In simulation demo mode, assign new tag source unconditionally,
            including INIT ONLY tags. In live data mode, handle INIT ONLY tags 
            using real-time data as shown in the code below. 
         */
         if( RandomData )
         {
            AssignTagSource( tag_obj, tag_source );

            /* For demo purposes, initialize tags with TagName="Speed" 
               using a random data value. These tags are present for motor 
               ebjects in scada_aeration.g, as well as a gauge and slider 
               in scada_motor_info.g. 
            */
            if( tag_name.equals( "Speed" ) )
              SetDTag( tag_source, GlgObject.Rand( 300.0, 1500.0 ), true );
            
            continue;
         }

         /* If tag is INIT_ONLY, initialize its value based on the current 
            data value for the given tag_source. Don't reassign TagSource 
            for this tag_obj, it is initilaized only once and will not be 
            subject to periodic updates.
         */
         int access_type = tag_obj.GetDResource( "TagAccessType" ).intValue();

         if( access_type == GlgObject.INIT_ONLY_TAG )
         {
            int data_type = tag_obj.GetDResource( "DataType" ).intValue();
         
            if( data_type == GlgObject.D )
            {
               double d_value = GetDTag( tag_source );
               tag_obj.SetDResource( null, d_value );
            }
         }
         else
           AssignTagSource( tag_obj, tag_source );
      }
   
      return size;
   }

   //////////////////////////////////////////////////////////////////////
   // Remap tags in the loaded drawing if needed.
   // In demo mode, it assigns unset tag sources to be the same as 
   // tag names. 
   //////////////////////////////////////////////////////////////////////
   public void RemapTags( GlgObject drawing_vp )
   {
      GlgObject tag_obj;
      String 
        tag_source,
        tag_name;

      /* Obtain a list of all tags defined in the drawing and remap them
         as needed.
      */
      GlgObject tag_list = 
        drawing_vp.CreateTagList( /* List all tags */ false );  
      if( tag_list == null )
        return;

      int size = tag_list.GetSize();
      if( size == 0 )
        return; // no tags found
      
      // Traverse the tag list and remap each tag as needed.
      for( int i=0; i<size; ++i )
      {
         tag_obj = (GlgObject) tag_list.GetElement( i );
         
         /* Retrieve TagName and TagSource attributes from the
            tag object. TagSource represents the data source variable
            used to supply real-time data. This function demonstrates
            how to reassign the TagSource at run-time.
         */
         tag_name = tag_obj.GetSResource( "TagName" );
         tag_source = tag_obj.GetSResource( "TagSource" );

         HMIPage.RemapTagObject( tag_obj, tag_name, tag_source );
      }
   }

   //////////////////////////////////////////////////////////////////////
   // Assigns new TagSource to the given tag object.
   //////////////////////////////////////////////////////////////////////
   public void AssignTagSource( GlgObject tag_obj, String new_tag_source )
   {
      tag_obj.SetSResource( "TagSource", new_tag_source );
   } 

   //////////////////////////////////////////////////////////////////////////
   // Utility function to validate the string. Returns true if the string
   // is undefined (invalid).
   //////////////////////////////////////////////////////////////////////////
   public boolean IsUndefined( String str )
   {
      if( str == null || str.isEmpty() || str.equals( "unset" ) ||
          str.equals( "$unnamed" ) )
        return true;
      
      return false;
   }      

   //////////////////////////////////////////////////////////////////////////
   // Utility function to convert a string to a corresponding int value
   // using a provided table.
   //////////////////////////////////////////////////////////////////////////
   public int ConvertStringToType( TypeRecord[] table, String type_str, 
                                   int empty_type, int undefined_type )
   {
      if( type_str == null || type_str.isEmpty() )
        return empty_type;
      
      for( int i=0; table[i].type_str != null; ++i )
        if( type_str.equals( table[i].type_str ) )
          return table[i].type_int;
      
      return undefined_type;
   }

   //////////////////////////////////////////////////////////////////////////
   // TypeRecord class is used to populate predefined tables, such as 
   // CommandTypeTable, DialogTypeTable, etc.
   //////////////////////////////////////////////////////////////////////////
   public class TypeRecord
     
   {
      public String type_str;
      public int type_int;

      public TypeRecord( String str, int int_const )
      {
         type_str = str;
         type_int = int_const;
      }

      public TypeRecord() {} // Empty constructor.
   }
   
   //////////////////////////////////////////////////////////////////////////
   // ReadMenuConfig is used by GlgSCADAViewer, to read and parse a
   // configuration file. The method stores information in the
   // MenuArray, which is an array of classes of type GlgMenuRecord. 
   // Return a number of read records. 
   //////////////////////////////////////////////////////////////////////////
   public ArrayList<GlgMenuRecord> ReadMenuConfig( String config_filename )
   {
      if( config_filename == null || config_filename.length() == 0 )
        return null;
      
      ArrayList<GlgMenuRecord> menu_array = new ArrayList<GlgMenuRecord>();
      
      BufferedReader reader_stream = OpenStream( config_filename );
      if( reader_stream == null )
        return null;
      
      String line;
      int num_read_records = 0;
      
      while( true )
      {
         try
         {
            line = reader_stream.readLine();
         }
         catch( IOException e  )
         {
            System.out.println( "Error reading config file: " + config_filename );
            break;
         }
         
         if( line == null )
           break;   /* Last line */
         
         if( line.length() == 0 || line.charAt( 0 ) == '#' )
           continue;   // Skip comments and empty lines.
         
         String [] entries = line.split( "," );
         int num_entries = entries.length;
         if( num_entries != 4 )
         {         
            System.out.println( "Missing entries, line: " + line );
            continue;
         }
         
         GlgMenuRecord menu_item = new GlgMenuRecord();
         for( int i=0; i<4; ++i )
         {
            // Remove heading and trailing spaces
            String entry = entries[i].trim();
            
            entry = HandleLineFeedChar( entry );
            
            // Create and fill table elements.
            switch( i )
            {
             case 0: 
               menu_item.label_string = entry;
               break;
             case 1: 
               menu_item.drawing_name = entry;
               break;
             case 2: 
               menu_item.tooltip_string = entry;
               break;
             case 3: 
               menu_item.drawing_title = entry;
               break;
            }
         }
         
         menu_array.add( menu_item );
         ++num_read_records;	       
      }
      
      try 
      {
         reader_stream.close(); 
      } 
      catch( IOException e  ){}
      
      return menu_array;
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Replace all occurences of "\\n" with "\n".
   // \n was used in the config file to create multi-line strings.
   //////////////////////////////////////////////////////////////////////////
   static String HandleLineFeedChar( String in_string )
   {
      return in_string.replaceAll( "\\\\n", "\n" );
   }

   //////////////////////////////////////////////////////////////////////////
   BufferedReader OpenStream( String filename )
   {
      filename = GetFullPath( filename );

      if( StandAlone )
      {
         try
         {
            File file = new File( filename );
            FileInputStream file_stream = new FileInputStream( file );
            return new BufferedReader( new InputStreamReader( file_stream ) );
         }
         catch( NullPointerException e )
         {
            System.out.println( "null filename" );
            return null;
         }
         catch( FileNotFoundException e )
         {
            System.out.println( "Can't open file: " + filename );
            return null;
         }
         catch( SecurityException e )
         {
            System.out.println( "Secutity Exception opening file: " + filename );
            return null;
         }
      }
      else
      {
         URL url;
         
         try
         {
            url = new URL( filename );
         }
         catch( MalformedURLException e )
         {
            System.out.println( "Invalid URL: " + filename );
            return null;
         }
         
         try
         {
            URLConnection connection = url.openConnection();
            connection.setUseCaches( false ); 
            connection.setDefaultUseCaches( false ); 
            
            InputStream url_stream = url.openStream();
            return new BufferedReader( new InputStreamReader( url_stream ) );
         }
         catch( IOException e )
         {
            System.out.println( "Can't open URL: " + filename );
            return null;
         }         
      }
   }

   //////////////////////////////////////////////////////////////////////////
   public class EmptyHMIPage extends HMIPageBase
   {
      public EmptyHMIPage( GlgSCADAViewer viewer ) 
      {
         super( viewer );
      }
      
      public int GetUpdateInterval()
      {
         return 0;   // Empty page: no data.
      }
   }

   /////////////////////////////////////////////////////////////////////// 
   // Return exact time including fractions of seconds.
   /////////////////////////////////////////////////////////////////////// 
   public double GetCurrTime()
   {
      return System.currentTimeMillis() / 1000.0;
   }
}
