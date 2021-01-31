/**************************************************************************
  This example demonstrates how to subclass a GlgJBean and use it as a top
  level applet. Subclassing also may be used to create custom components
  with additional custom methods and functionality. This example may be 
  run in a browser using graph_applet.html file or as a standalone Java 
  program. 

  GlgGraphApplet class is subclassed from GlgJBean and displays a GLG 
  drawing containing a bar graph which is updated periodically using the 
  UpdateGraphProc method.

  The drawing name "bar1.g" is set in the start() method in case of 
  running in a browser, or in the main() method if running a standalone 
  Java application. When running in a browser, the drawing is "unset" 
  in the stop() method so that it can be set again in a start() method 
  when reloading an applet.

  Dynamic updates are perfromed in the UpdateGraphProc method, which
  is invoked periodically by a timer. IsReady flag controls whether
  the drawing is ready to be updated. This flag is set to true in the
  ReadyCallback, and is set to false in the StopUpdates method, which
  stops periodic updates.

  GetData method supplies  random data values to update the graph with 
  new data, and GetLabel method generates numeric label strings for the 
  X axis. In a real application, these methods can be substituted to supply 
  data from a given source, such as a database, socket, data file, etc.
***************************************************************************/

import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgGraphApplet extends GlgJBean implements ActionListener
{
   static final long serialVersionUID = 0;

   int label_counter = 0;
   final double high = 10.;  // Graph's high range
   final double low = 0.;    // Graph's low range

   Timer timer = null;
   boolean IsReady = false;

   //////////////////////////////////////////////////////////////////////
   // Constructor
   //////////////////////////////////////////////////////////////////////
   public GlgGraphApplet()
   {
      // Turn on Java diagnostics
      SetJavaLog( true );
   }

   /////////////////////////////////////////////////////////////////////
   // Invoked by the browser to start the applet
   /////////////////////////////////////////////////////////////////////
   public void start()
   {
      super.start();

      // Set a Glg drawing URL to be displayed in the Glg bean, 
      // "bar1.g" in this case. The drawing URL is relative to 
      // applet's document base directory.
      SetDrawingURL( "bar1.g" );

      // Start periodic updates 
      StartUpdates();
   }

   ///////////////////////////////////////////////////////////////////////
   // Invoked by the browser asynchronously to stop the applet.
   ///////////////////////////////////////////////////////////////////////
   public void stop()
   {
      // Stop periodic updates
      StopUpdates();

      // GlgJBean handles asynchronous invocation when used as an applet.
      super.stop();
   }

   //////////////////////////////////////////////////////////////////////
   // HCallback is a Hierarchy Setup callback which is invoked 
   // after the drawing is loaded, but before it is setup and drawn
   // for the fisrt time. The code for initializing graph parameters such 
   // as number of data samples, range, etc. may be placed here. Since 
   // HCallback is invoked before hierarchy setup takes place, individual 
   // datasamples of a graph are not available yet; to access individual 
   // datasamples (DataGroup/DataSample%), use ReadyCallback or VCallback.
   //////////////////////////////////////////////////////////////////////
   public void HCallback( GlgObject viewport )
   {
      // Set initial graph parameters, including titles, datasample
      // and label parameters.

      viewport.SetSResource( "Title/String", "Bar Graph" );
      viewport.SetSResource( "XAxisLabel/String", "Sample" );
      viewport.SetSResource( "YAxisLabel/String", "Value" );

      // Set initial DataSample value
      viewport.SetDResource( "DataGroup/DataSample/Value", 0. );  

      // Set High and Low range
      viewport.SetDResource( "DataGroup/DataSample/High", high ); 
      viewport.SetDResource( "DataGroup/DataSample/Low", low );   

      // Set number of data samples
      viewport.SetDResource( "DataGroup/Factor", 20. );           

      // Set initial label value for an X axis 
      viewport.SetSResource( "XLabelGroup/XLabel/String", "" ); 

      // Set number of labels and ticks for an X axis
      viewport.SetDResource( "XLabelGroup/Factor", 5. );        
      viewport.SetDResource( "XLabelGroup/MinorFactor", 4. );   

      // Set the scroll type
      viewport.SetDResource( "DataGroup/ScrollType", GlgObject.WRAPPED );

      // Make the level line invisible
      viewport.SetDResource( "LevelObjectGroup/Visibility", 0. );          
   }

   //////////////////////////////////////////////////////////////////////
   // VCallback() is invoked after the drawing is loaded and setup,
   // but before it is drawn for the first time.
   // Graph's individual datasamples DataSample% can be accessed here.
   /////////////////////////////////////////////////////////////////////
   public void VCallback( GlgObject viewport )
   {
   }

   //////////////////////////////////////////////////////////////////////
   // ReadyCallback() is invoked after the drawing is loaded, setup
   //  and initially drawn.
   /////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      // Enable dynamic updates 
      IsReady = true;

      // Place custom initialization code here, for example accessing
      // graph's individual datasamples.       
   }
   
   //////////////////////////////////////////////////////////////////////
   // UpdateGraphProc mthod performs dynamic updates. This method
   // is invoked periodically by a timer. It updates the graph with 
   // random data values.
   //////////////////////////////////////////////////////////////////////
   void UpdateGraphProc()
   {
      // Perform dynamic updates only if the drawing is ready and 
      // the timer is active.
      if( !IsReady )
         return;

      // Push next data value to a graph.
      SetDResource( "DataGroup/EntryPoint", GetData() );

      // Push next label to a graph.
      SetSResource( "XLabelGroup/EntryPoint", GetLabel() );
         
      Update();   // Make changes visible
   }

   ///////////////////////////////////////////////////////////////////////
   // GetData method returns random double values to be pushed into the
   // graph. An application should provide a custom data acquisition 
   // mechanism to supply data values for updating dynamics.
   ///////////////////////////////////////////////////////////////////////
   double GetData()
   {
      return high * Math.random();    // Random value
   }

   //////////////////////////////////////////////////////////////////////
   // GetLabel() method returns a label counter to be used for labeling
   // the X axis.
   //////////////////////////////////////////////////////////////////////
   String GetLabel()
   {
      // Increase label counter
      ++label_counter;
      if( label_counter > 9999 )
        label_counter = 0;
      
      return "#" + label_counter;   // Numerical label
   }

   //////////////////////////////////////////////////////////////////////
   // StartUpdates() creates a timer to perform periodic updates.
   // The timer invokes the bean's UpdateGraphProc() method to update
   // drawing's resoures with new data values.
   //////////////////////////////////////////////////////////////////////
   void StartUpdates()
   {
      if( timer == null )
      {
         timer = new Timer( 30, this );
         timer.setRepeats( true );
         timer.start();
      }
   }

   //////////////////////////////////////////////////////////////////////
   // StopUpdate() method stops periodic updates
   //////////////////////////////////////////////////////////////////////
   void StopUpdates()
   {      
      if( timer != null )
      {
         timer.stop();
         timer = null;
      }

      // Disable dynamic updates of the drawing
      IsReady = false;
   }

   //////////////////////////////////////////////////////////////////////
   // timer's ActionListener method to be invoked peridically 
   //////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateGraphProc();
   }

   //////////////////////////////////////////////////////////////////////////
   // For use as a stand-alone application
   //////////////////////////////////////////////////////////////////////////
   public static void main( final String arg[] )
   {
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Main( arg ); } } );
   }

   public static void Main( final String arg[] )
   {
      class DemoQuit extends WindowAdapter
      {
         public void windowClosing( WindowEvent e ) { System.exit( 0 ); }
      } 

      JFrame frame = new JFrame();
      frame.setResizable( true );
      frame.setSize( 600, 400 );
      frame.addWindowListener( new DemoQuit() );

      GlgGraphApplet graph_example = new GlgGraphApplet(); 
      frame.getContentPane().add( graph_example );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      //
      graph_example.SetDrawingName( "bar1.g" );

      // Start periodic updates.
      graph_example.StartUpdates();
   }
}
