import java.awt.event.*;
import java.util.Vector;
import java.lang.reflect.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
// This example demonstrates how to use GLG tags feature in a generic 
// way. The program loads the drawing, queries a list of all tags 
// defined in the drawing and uses tags to subscribe to the data 
// updates. The tags defined the fields in the process database which
// the drawing needs data for. The program then waits for data and
// updates the drawing when the data are received.
//
// The prototype does not actually connect to a process database, 
// since it is very specifiec to the application environment. Instead,
// it shows a sample framework and uses simulated random data. 
// An application developer needs to write application-speciific code 
// for the SubscribeTagData method, using selected data aquisition
// method (a custom framework or 3-rd party tools) to connect to the 
// application's data.
//////////////////////////////////////////////////////////////////////////

interface DataBean
{   
   // Process D data event
   public void ProcessDataEvent( String tag_source, double dvalue );

   // Process S data event
   public void ProcessDataEvent( String tag_source, String svalue );

   // Process G data event
   public void ProcessDataEvent( String tag_source, double gvalue1,
                                double gvalue2, double gvalue3 );

   // A list of active timers for cleanup. 
   public Vector GetTimerArray();
   public void SetTimerArray( Vector timer_array );
}

//////////////////////////////////////////////////////////////////////////

public class GlgTagsExample extends GlgJBean implements DataBean
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////

   static final long serialVersionUID = 0;

   // Time interval for periodic updates, in millisec.
   int TimeInterval = 250; 
   boolean SimulatedData = true;      // Use simulated data to demo.
   boolean UseObjectID = false;
     
   public Vector TimerArray = null;   // A list of active timers for cleanup. 

   // Keep around instead of creating on every G tag access
   GlgPoint point = new GlgPoint();

   boolean IsReady;

   //////////////////////////////////////////////////////////////////////////
   public GlgTagsExample()
   {
      super();

      SetDResource( "$config/GlgSwingUsage", 1. );
   }

   //////////////////////////////////////////////////////////////////////////
   // Starts updates
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback();
      IsReady = true;      
   }

   //////////////////////////////////////////////////////////////////////////
   // For use as a stand-alone application
   //////////////////////////////////////////////////////////////////////////
   public static void main( final String arg[] )
   {
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Main( arg ); } } );
   }

   //////////////////////////////////////////////////////////////////////////
   public static void Main( final String arg[] )
   {
      class DemoQuit extends WindowAdapter
      {
         public void windowClosing( WindowEvent e ) { System.exit( 0 ); }
      } 

      String filename;
      if( Array.getLength( arg ) == 0 || arg[ 0 ] == null )
      {
         System.out.println( "No drawing file supplied on command line, using tags_example.g" );
         filename = "tags_example.g";
      }
      else
        filename = arg[ 0 ];

      JFrame frame = new JFrame();
      frame.setResizable( true );
      frame.setSize( 800, 800 );
      frame.setLocation( 20, 20 );
      frame.addWindowListener( new DemoQuit() );

      GlgTagsExample tags = new GlgTagsExample();      
      frame.getContentPane().add( tags );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      //
      tags.SetDrawingName( filename );
   }

   //////////////////////////////////////////////////////////////////////
   // VCallback() is invoked after the drawing is loaded and setup,
   // but before it is drawn for the first time.
   // Use it to query tags after HierarchySetup: more efficient. 
   /////////////////////////////////////////////////////////////////////
   public void VCallback( GlgObject viewport )
   {
      GlgObject TagList;

      TagList = 
        viewport.CreateTagList( /* List each tag only once */ true );

      if( TagList != null && TagList.GetSize() != 0 )
        SubscribeToData( viewport, TagList );
   }

   //////////////////////////////////////////////////////////////////////
   // Subscribe all tags in the list.
   //////////////////////////////////////////////////////////////////////
   void SubscribeToData( GlgObject viewport, GlgObject tag_list )
   {
      GlgObject tag_obj;
      String tag_source;
      int i, size;
      int dtype;

      size = tag_list.GetSize();
      for( i=0; i<size; ++i )
      {
         tag_obj = (GlgObject) tag_list.GetElement( i );

         // Get the name of the database field to use as a data source
         tag_source = tag_obj.GetSResource( "TagSource" );
         if( tag_source == null || tag_source.length() == 0 )
           continue;

         // Get tag object's data type: D, S or G.
         dtype = tag_obj.GetDResource( "DataType" ).intValue();

         // Subscribe this tag
         SubscribeTagData( dtype, tag_source );
      }
   }

   //////////////////////////////////////////////////////////////////////
   // Subscribe to the data field indicated by the tag source, so that 
   // data_callback is invoked when the data comes.
   //////////////////////////////////////////////////////////////////////
   void SubscribeTagData( int data_type, String tag_source )
   {
      if( SimulatedData )
      {
         // Simulated data example: activate periodic data update.
         // Creating TagDataGenerator adds a timer for it and adds it
           // to a timer array.
         new TagDataGenerator( TimeInterval, this, data_type, tag_source );
      }
      else
      {
         // Fill with application's code to subscribe to data.
      }
   }

   //////////////////////////////////////////////////////////////////////
   // Generates data for selected tag.
   //////////////////////////////////////////////////////////////////////
   @SuppressWarnings("unchecked")
   class TagDataGenerator implements ActionListener
   {
      DataBean bean;
      int data_type;
      String tag_source;
      Timer timer;

      TagDataGenerator( int time_interval, DataBean bean_p,
                       int data_type_p, String tag_source_p )
      {
         bean = bean_p;
         data_type = data_type_p;
         tag_source = tag_source_p;

         timer = new Timer( time_interval, this );
         timer.setRepeats( false );
         timer.start();

         // Keep a list of active timers for cleanup
         if( bean.GetTimerArray() == null )
           bean.SetTimerArray( new Vector() );

         bean.GetTimerArray().add( timer );
      }

      /////////////////////////////////////////////////////////////////////////
      public void actionPerformed( ActionEvent e )
      {
         if( IsReady )
           GenerateTagData();

         timer.start();   // Restart timer
      } 

      /////////////////////////////////////////////////////////////////////////
      void GenerateTagData()
      {
         switch( data_type )
         {
          case GlgObject.D:	 
            // Invoke data callback with random D data 
            bean.ProcessDataEvent( tag_source, GlgObject.Rand( 0., 1. ) );
            break;

          case GlgObject.S:
            // Invoke data callback with random S data
              ProcessDataEvent( tag_source, 
                               Double.toString( GlgObject.Rand( 0., 1. ) ) );
            break;

          case GlgObject.G:
            // Invoke data callback with random G data
            ProcessDataEvent( tag_source, 
                             GlgObject.Rand( 0., 1. ),
                             GlgObject.Rand( 0., 1. ),
                             GlgObject.Rand( 0., 1. ) );
            break;
            
          default: return;
         }
      }   
   }

   //////////////////////////////////////////////////////////////////////
   // DataBean interface: Process D data event
   //////////////////////////////////////////////////////////////////////
   public void ProcessDataEvent( String tag_source, double dvalue )
   {
      SetDTag( tag_source, dvalue, true );

      // If drawing has a lot of tags, change to update just once after
      // each batch of tag data instead of updating after each individual
        // data event.
      Update();
   }

   //////////////////////////////////////////////////////////////////////
   // DataBean interface: Process S data event
   //////////////////////////////////////////////////////////////////////
   public void ProcessDataEvent( String tag_source, String svalue )
   {
      SetSTag( tag_source, svalue, true );

      // If drawing has a lot of tags, change to update just once after
      // each batch of tag data instead of updating after each individual
        // data event.
      Update();
   }

   //////////////////////////////////////////////////////////////////////
   // DataBean interface: Process G data event
   //////////////////////////////////////////////////////////////////////
   public void ProcessDataEvent( String tag_source, double gvalue1,
                                double gvalue2, double gvalue3 )
   {
      point.x = gvalue1;
      point.y = gvalue2;
      point.z = gvalue3;
      SetGTag( tag_source, point, true );

      // If drawing has a lot of tags, change to update just once after
      // each batch of tag data instead of updating after each individual
        // data event.
      Update();
   }

   //////////////////////////////////////////////////////////////////////
   public Vector GetTimerArray()
   {
      return TimerArray;
   }

   //////////////////////////////////////////////////////////////////////
   public void SetTimerArray( Vector timer_array )
   {
      TimerArray = timer_array;
   }

   //////////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject viewport, GlgObject message_obj )
   {
      super.InputCallback( viewport, message_obj );

      String format = message_obj.GetSResource( "Format" );
      String action = message_obj.GetSResource( "Action" );
      // String origin = message_obj.GetSResource( "Origin" );

      // Handle window closing if run stand-alone
      if( format.equals( "Window" ) && action.equals( "DeleteWindow" ) )
        System.exit( 0 );


   }

   ///////////////////////////////////////////////////////////////////////
   // Invoked by the browser asynchronously to stop the applet.
   ///////////////////////////////////////////////////////////////////////
   public void stop()
   {
      IsReady = false;

      if( TimerArray != null )
      {
         int i, size;

         size = TimerArray.size();
         for( i=0; i<size; ++i )
         {
            Timer timer = (Timer) TimerArray.elementAt( i );
            timer.stop();
         }
         TimerArray = null;
      }

      // GlgJBean handles asynchronous invocation when used as an applet.
      super.stop();
   }
}
