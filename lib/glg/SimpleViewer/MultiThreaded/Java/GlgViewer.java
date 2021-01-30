import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.lang.reflect.*;
import java.util.ArrayList;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgViewer extends GlgJBean implements ActionListener
{
   /* Set to true to use simulated data for animation. 
      Set to false to use live real-time data.
      May be overriden by command line option -random-data or -live-data in
      a standalone program, or RandomData applet parameter if used
      inside an applet. 
   */
   public boolean RANDOM_DATA = true;

   /* If GUI is not keeping up with refreshing graphics due to very high rate 
      of incoming data, the size of the accumulated data will grow exponentially.
      This example generates a warning if it happens.
   */
   final int HIGH_WATER_MARK_LIMIT = 100000;

   // Parent container.
   SimpleViewer Parent;
   
   // DataFeed object is used to supply data for animation.
   DataFeedBase DataFeed;
   
   // Dynamically created array of tag records.
   ArrayList<GlgTagRecord> TagRecordArray = new ArrayList<GlgTagRecord>();
   
   // Number of tags records in the TagRecordArray.
   public int NumTags = 0;

   // An array containing accumulated raw data from the data thread. */
   ArrayList<BaseData> GUIData = new ArrayList<BaseData>();
   
   Timer timer = null;
   int UpdateInterval = 100; // update interval in msec.
   boolean IsReady = false;

   // Set to true to print debugging information. 
   static boolean DEBUG = false;

   //////////////////////////////////////////////////////////////////////////
   public GlgViewer( SimpleViewer parent )
   {
      super();
      Parent = parent;
   }
      
   //////////////////////////////////////////////////////////////////////
   //  Start dynamic updates.
   /////////////////////////////////////////////////////////////////////
   public void StartUpdates()
   {
      if( DataFeed == null )
      {
         System.out.println( "No DataFeed object, can't start data updates." );
         return;
      }
      
      DataFeed.StartUpdates();    // Start data thread.

      // Start a timer to update the graphics with data.
      if( timer == null )
      {
         timer = new Timer( UpdateInterval, this );
         timer.setRepeats( true );
         timer.start();
      }
   }

   ///////////////////////////////////////////////////////////////////////
   // Stop data updates.
   ///////////////////////////////////////////////////////////////////////
   public void StopUpdates()
   {
      if( DataFeed == null )
      {
         System.out.println( "No DataFeed object, can't stop data updates." );
         return;
      }
      
      DataFeed.StopUpdates();    // Stop data thread.

      if( timer != null )
      {
         timer.stop();
         timer = null;
      }
   }
   
   //////////////////////////////////////////////////////////////////////////
   public void LoadDrawing( String drawing_filename )
   {
      if( drawing_filename == null )
      {
         System.out.println( "No drawing file: drawing loading failed." );
         return;
      }

      SetDrawingName( drawing_filename );
   }

   //////////////////////////////////////////////////////////////////////
   public void HCallback( GlgObject viewport )
   {
      AddDataFeed();
   }

   //////////////////////////////////////////////////////////////////////
   // VCallback() is invoked after the drawing is loaded and setup, 
   // but before it is drawn for the first time. 
   /////////////////////////////////////////////////////////////////////
   public void VCallback( GlgObject viewport )
   {
      /* Clear TagRecordArray, in case the drawing
         is replaced in the GlgViewer, so that the previous 
         TagRecordArray is cleared before the new one gets
         created.
      */
      DeleteTagRecords();

      // Build TagRecordArray, a list of glg tag records.
      CreateTagRecords( viewport );
   }
   
   ////////////////////////////////////////////////////////////////////////
   // ReadyCallback is invoked after the drawing is loaded, setup and 
   // initially drawn.
   ////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      IsReady = true;
   }

   //////////////////////////////////////////////////////////////////////////
   // Add DataFeed object to a glg_bean.
   //////////////////////////////////////////////////////////////////////////
   public void AddDataFeed()
   {
      if( RANDOM_DATA )
      {
        DataFeed = new DemoDataFeed( this );
        System.out.println( "Using random demo DataFeed." );
      }
      else
      {
        DataFeed = new LiveDataFeed( this );
        System.out.println( "Using live DataFeed." );
      }
   }

   /////////////////////////////////////////////////////////////////////
   // Create and populate TagRecordArray
   /////////////////////////////////////////////////////////////////////
   public void CreateTagRecords( GlgObject viewport )
   {
      GlgObject tag_obj;
      String tag_source;
      String tag_name;
      String tag_comment;
      int tag_access_type;
      GlgObject tag_list;

      /* Retrieve a tag list from the drawing. Include tags with unique
         tag sources.
      */
      tag_list = viewport.CreateTagList( /*unique tag sources*/ true );
      if( tag_list == null )
        return;
      
      int size = tag_list.GetSize();
      if( size == 0 )
        return; // no tags found 
            
      /* Create an array of tag records by traversing the tag list
         and retrieving information from each tag object
         in the list.
      */
      for( int i=0; i<size; ++i )
      {
         tag_obj = (GlgObject) tag_list.GetElement( i );
         tag_source = tag_obj.GetSResource( "TagSource" );
         tag_name = tag_obj.GetSResource( "TagName" );
         tag_comment = tag_obj.GetSResource( "TagComment" );
         
         // Skip undefined tags.
         if( tag_source == null || tag_source.equals("") || 
             tag_source.equals("unset") )
           continue;
         
         tag_access_type = tag_obj.GetDResource( "TagAccessType" ).intValue();
         
         // Handle special tags according to their tag access type.
         switch( tag_access_type )
         {
          case GlgObject.OUTPUT_TAG: continue;   // Skip OUTPUT tags.
            
          case GlgObject.INIT_ONLY_TAG:
            /* Initialize INIT_ONLY tags; do not include them in 
               TagRecordArray, so that they will not be subject to 
               the continous data animation. 
            */
            InitTag( tag_source );
            continue;
          
          default: break;
         }
         
         // Tag source is valid, add a new tag record to TagRecordArray.
         GlgTagRecord tag_record = new GlgTagRecord();
         tag_record.tag_obj = tag_obj;
         tag_record.tag_source = tag_source;
         tag_record.data_type = tag_obj.GetDResource( "DataType" ).intValue();
         
         /* For further performance optimization, set if_changed=true which 
            will push the value into the tag only if the value has changed. 
            Set if_changed = false for the tags assigned to the entry 
            points of a chart object.
         */
         if( tag_comment != null && tag_comment.contains("Chart") )
           tag_record.if_changed = false;
         else
           tag_record.if_changed = true;      
         
         // Add a valid tag record to the list.
         TagRecordArray.add( tag_record );
      }
      
      // Store number of tag records in TagRecordArray.
      NumTags = TagRecordArray.size();
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Clear TagRecordArray
   //////////////////////////////////////////////////////////////////////////
   public void DeleteTagRecords()
   {
      TagRecordArray.clear();
      NumTags = 0;
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Initialize tag with a given tag source.
   //////////////////////////////////////////////////////////////////////////
   public void InitTag( String tag_source )
   {
      double value = -1.0;
      
      if( RANDOM_DATA )
      {
         // Initialize "State" tag.
         if( tag_source.equals( "State" ) )
           value = 1.0;
      }
      else
      {
         // Place custom code here to set value as needed.
      }
      
      // Set the tag value in the drawing.
      SetDTag( tag_source, value, false );
   }

   ///////////////////////////////////////////////////////////////////////
   // timer's ActionListener method to be invoked periodically 
   //////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateDrawing();
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Update the drawing with new dynamic data values.
   //////////////////////////////////////////////////////////////////////////
   public void UpdateDrawing()
   {
      if( !IsReady )
        return;
      
      // Obtain data values and push them into graphics.
      GetGUIData();
      ProcessGUIData();    
            
      // Update display.
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Access real-time data accumulated so far in a buffer.
   //////////////////////////////////////////////////////////////////////////
   void GetGUIData()
   {
      if( NumTags == 0 )
        return;
      
      /* Get data accumulated in the data thread in the AccumulatedData array
         since the last query, and store them in GUIData array.
         We simply swap the arrays containing the old and new accumulated data,
         and use the old array to accumulate the next portion of the data.
      */
      GUIData = DataFeed.GetAccumulatedData( GUIData );

      if( GUIData.size() > HIGH_WATER_MARK_LIMIT )
        System.out.println( "GUI is overwhelmed with data, use merging or drop data!" );
      
      if( DEBUG )
        System.out.println( "GUIData size: " + GUIData.size() );
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Push all data accumulated in the GUIData vector into graphics. 
   //////////////////////////////////////////////////////////////////////////
   void ProcessGUIData()
   {
      int size = GUIData.size();

      if( size == 0 )
        return;
      
      for( int i = 0; i <  size; ++i )
        PushData( GUIData.get( i ) );
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Process data in a provided data structure based on the structure type
   // (data.type). For each field in a data structure, push the data value into
   // a tag with a matching TagSource and data type, if found. 
   //
   // !!!!! IMPORTANT: TagSource in the drawing MUST MATCH hardcoded 
   // tag source strings below. For example, the tag sources in the drawing 
   // must be "LAT" and "LON" to push lat and lon values from the
   // GPS data structure. 
   //
   // If the application navigates through multiple drawings, not all
   // data fields in the incoming data structures may have corresponding
   // tags in the currently loaded drawing. 
   //
   // PushTagData function performs a check to make sure a data value
   // is pushed to the graphics only if the tag with a matching TagSource
   // exists in the currently loaded drawing.
   //
   // The application can modify the tag sources and data structures as needed.
   //////////////////////////////////////////////////////////////////////////
   void PushData( BaseData data )
   {
      switch( data.type )
      {
       case BaseData.GPS:
         GPSData gps_data = (GPSData) data;
         PushTagData( "LAT", gps_data.lat );
         PushTagData( "LON", gps_data.lon );
         PushTagData( "SPEED", gps_data.speed );
         PushTagData( "ALTITUDE", gps_data.altitude );
         PushTagData( "PITCH", gps_data.pitch );
         PushTagData( "ROLL", gps_data.roll );
         PushTagData( "YAW", gps_data.yaw );
         PushTagData( "POSITION", gps_data.lat, gps_data.lon, 
                      gps_data.altitude );
         break;
         
       case BaseData.TELEMETRY:
         TelemetryData telem_data = (TelemetryData) data;
         PushTagData( "POWER", telem_data.power );
         PushTagData( "VOLTAGE", telem_data.voltage );
         PushTagData( "CURRENT", telem_data.current );
         PushTagData( "TEMPERATURE", telem_data.temperature );
         PushTagData( "PRESSURE", telem_data.pressure );
         PushTagData( "STATE_HEALTH", telem_data.state_health );
         break;
         
       default:
         System.out.println( "Unknown Data Structure Type." );
         System.exit( 1 );
      }
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Push a double value into a D-type tag, if found.
   // Verify that the tag with a specified tag source is present in the drawing
   // before pushing the value to the graphics.
   //////////////////////////////////////////////////////////////////////////
   boolean PushTagData( String data_source, double d_value )
   {
      // Find a tag record with a matching tag source and data type.
      GlgTagRecord tag_record = LookupTagRecords( data_source, GlgObject.D );
      
      if( tag_record == null ) 
        return false;
      
      // Push value into graphics.
      SetDTag( tag_record.tag_source, d_value, tag_record.if_changed );
      
      return true;
   }

   //////////////////////////////////////////////////////////////////////////
   // Push x,y,z values into a G-type tag, if found. 
   // Verify that the tag with a specified tag source is present in the drawing
   // before pushing the value to the graphics.
   //////////////////////////////////////////////////////////////////////////
   boolean PushTagData( String data_source, double x, double y, double z )
   {
      // Find a tag record with a matching tag source and data type.
      GlgTagRecord tag_record = LookupTagRecords( data_source, GlgObject.G );
      
      if( tag_record == null ) 
        return false;

      // Push value into graphics.
      GlgPoint position = new GlgPoint( x, y, z );
      SetGTag( tag_record.tag_source, position, tag_record.if_changed );
      
      return true;
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Push a string value into an S-type tag, if found.
   // Verify that the tag with a specified tag source is present in the drawing
   // before pushing the value to the graphics.
   //////////////////////////////////////////////////////////////////////////
   boolean PushTagData( String data_source, String s_value )
   {
      // Find a tag record with a matching tag source and data type.
      GlgTagRecord tag_record = LookupTagRecords( data_source, GlgObject.S );
      
      if( tag_record == null ) 
        return false;
      
      // Push value into graphics.
      SetSTag( tag_record.tag_source, s_value, tag_record.if_changed );
      
      return true;
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Returns a tag record with a matching tag_source and data_type.
   ///////////////////////////////////////////////////////////////////////
   GlgTagRecord LookupTagRecords( String tag_source, long data_type )
   {
      for( int i = 0; i < NumTags; ++i )
      {
         GlgTagRecord tag_record = TagRecordArray.get( i );
         if( tag_record.tag_source.equals( tag_source ) &&
             tag_record.data_type == data_type )
           return tag_record;
      }
      
      return null; // not found
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Handle user interaction.
   ///////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject viewport, GlgObject message_obj )
   {
      String
        origin,
        format,
        action,
        subaction;
      
      origin = message_obj.GetSResource( "Origin" );
      format = message_obj.GetSResource( "Format" );
      action = message_obj.GetSResource( "Action" );
      subaction = message_obj.GetSResource( "SubAction" );
      
      // Handle window closing if run stand-alone. 
      if( format.equals( "Window" ) && action.equals( "DeleteWindow" ) )
        System.exit( 0 );
      
      // Handle button events based on the button names.
      if( format.equals("Button") )
      {
         if( !action.equals("Activate") && !action.equals("ValueChanged") )
           return;
         
         if( action.equals("Activate") )  //Push button event
         {
            if( origin.equals("QuitButton") )
            {  
               if( Parent.IsStandalone )
                 System.exit( 0 );
            }
            else if( origin.equals("StartButton") )
            {
               // For demo purposes, the StartButton starts dynamic updates. 
               StartUpdates();
            }
            else if( origin.equals("StopButton") )
            {
               // For demo purposes, Stop button stops dynamic updates.
               StopUpdates();
            }
            else
              // Place custom code here as needed.
              ; 
         }
         
         else if( action.equals("ValueChanged") ) // Toggle button
         {
            double state = message_obj.GetDResource( "OnState" ).doubleValue();

            // Place code here to handle events from a toggle button
            // and write a new value to a given tag_source.
            // DataFeed.WriteDValue( (tag_source, state );
         }
         
         // Make changes visible.
         viewport.Update();
      }
      else if( format.equals( "Timer" ) )
      {
         // Refresh the display for the objects with integrated Timer dynamics.
         viewport.Update();
      }
   }
}
