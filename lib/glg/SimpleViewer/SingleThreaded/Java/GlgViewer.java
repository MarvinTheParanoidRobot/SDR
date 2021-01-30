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

   // Parent container.
   SimpleViewer Parent;

   // Dynamically created array of tag records.
   ArrayList<GlgTagRecord> TagRecordArray = new ArrayList<GlgTagRecord>();
   
   // Number of tags records in the TagRecordArray.
   public int NumTags = 0;

   // DataFeed object is used to supply data for animation.
   DataFeedInterface DataFeed;

   Timer timer = null;
   int UpdateInterval = 100; // update interval in msec.
   boolean IsReady = false;

   // Used by DataFeed to return data values.
   DataPoint d_data_point = new DataPoint( GlgObject.D );
   DataPoint s_data_point = new DataPoint( GlgObject.S );

   //////////////////////////////////////////////////////////////////////////
   public GlgViewer( SimpleViewer parent )
   {
      super();
      Parent = parent;
   }
      
   //////////////////////////////////////////////////////////////////////
   // StartUpdates() creates a timer to perform periodic updates.
   // The timer invokes the bean's UpdateGraphProc() method to update
   // drawing's resoures with new data values.
   /////////////////////////////////////////////////////////////////////
   public void StartUpdates()
   {
      if( timer == null )
      {
         timer = new Timer( UpdateInterval, this );
         timer.setRepeats( true );
         timer.start();
      }
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
      if( TagRecordArray.size() > 0 )
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
      
      /* For each tag in the TagRecordArray, obtain a new data value using 
         DataFeed object and push a new value into the graphics.  
      */
      for( int i = 0; i<NumTags; ++i )
      {
         GlgTagRecord tag_record = TagRecordArray.get(i);
         switch( tag_record.data_type )
         {
          case GlgObject.D:
            if( !DataFeed.ReadDValue( tag_record, d_data_point ) )
            {
               System.out.println( "Failed to read data value for " + 
                                   tag_record.tag_source );
               continue;
            }

            // Push new data value into the graphics.
            SetDTag( tag_record.tag_source,  d_data_point.d_value, 
                     tag_record.if_changed );
            break;

          case GlgObject.S:
            if( !DataFeed.ReadSValue( tag_record, s_data_point ) )
            {
               System.out.println( "Failed to read data value for " + 
                                   tag_record.tag_source );
               continue;
            }

            // Push new data value into the graphics.
            SetSTag( tag_record.tag_source, s_data_point.s_value, 
                     tag_record.if_changed );
            break;

          case GlgObject.G:      // Not used in this example.
          default:
            break;
         }
      }
      
      // Update the display
      Update();
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
               if ( RANDOM_DATA )
               {
                  DataFeed.WriteDValue( "State", 1.0 );
                  
                  // Start/Resume dynamic updates.
                  StartUpdates();
               }
               // Place custom code here as needed.
            }
            else if( origin.equals("StopButton") )
            {
               if ( RANDOM_DATA )
               {
                  DataFeed.WriteDValue( "State", 0.0 );
                  
                  // Stop dynamic updates.
                  StopUpdates();
               }
               else
                 // Place custom code here as needed.
                 ; 
            }
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

