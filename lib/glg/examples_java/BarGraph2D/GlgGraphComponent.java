/***************************************************************************
  This example demonstrates how to display and update a Glg graph in a
  Glg bean. The program may be used as an applet in a browser using 
  graph_component.html file, or as a standalone Java program. 

  A GlgJLWBean is added as a subcomponent to the parent container (an
  applet in this case) and displays a drawing named "bar1.g". The
  drawing name is set in the start() method in case of running in a
  browser, or in the main() method if running a standalone Java
  application. When running in a browser, the drawing is "unset" in
  the stop() method so that it can be set again in a start() method
  when reloading the applet.

  To update a graph with data, update procedure UpdateGraphProc is
  used, which is invoked periodically by a timer. IsReady flag
  controls whether the drawing is ready to perform dynamic
  updates. This flag is set to true in the ReadyCallback, indicating
  the drawing is ready to be updated. IsReady flag is set to false in
  the StopUpdates method, which stops periodic updates.
  
  GetData method supplies random data values to update the graph 
  with new data, and GetLabel method generates numeric label strings for 
  the X axis. In a real application, these methods can be substituted to 
  supply  data from a given source, such as a database, socket, 
  data file, etc.
***************************************************************************/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgGraphComponent extends JApplet implements ActionListener
{
   static final long serialVersionUID = 0;

   int label_counter = 0;
   final double high = 10.;
   final double low = 0.;

   // In this example, Glg light-weight Swing component GlgJLWBean is used
   // to display a Glg graph. If light-weight Swing components features
   // are not required, Glg heavy-weight Swing component GlgJBean
   // may be used instead to increase update performance.

   GlgJLWBean glg_bean;

   Timer timer = null;
   boolean IsReady = false;

   /////////////////////////////////////////////////////////////////////
   // Constructor, where Glg bean component is created and added to a 
   // native Java container, an Applet in this case.
   /////////////////////////////////////////////////////////////////////
   public GlgGraphComponent()
   {
      getContentPane().setLayout( new GridLayout( 1, 1 ) );

      glg_bean = new GlgJLWBean();
      getContentPane().add( glg_bean );

      // Add listeners to the Glg bean to "listen" to the particular
      // events that occur in the bean and place custom code there.
      // Listeners should be added BEFORE setting the drawing name to
      // be displayed in the bean ( before calling SetDrawingName() ).

      // Add Hierarchy Setup Listener to place drawing initialization
      // code, such as setting initial graph parameters. HListener is 
      // invoked after the drawing is loaded but before it is initially
      // drawn.  
      glg_bean.AddListener( GlgObject.H_CB, new HListener() );

      // Add Value listener which is invoked after drawing is loaded and
      // setup, but before it is initially drawn.
      glg_bean.AddListener( GlgObject.V_CB, new VListener() );

      // Add Ready Listener to the Glg bean. It is invoked after
      // the drawing is initially drawn.
      glg_bean.AddListener( GlgObject.READY_CB, new ReadyListener() );

      // Turn on diagnostics
      glg_bean.SetJavaLog( true );
   }

   //////////////////////////////////////////////////////////////////////
   // Invoked by the browser to start the applet
   //////////////////////////////////////////////////////////////////////
   public void start()
   {
      super.start();
      
      // Set a GLG drawing to be displayed in the GLG bean by specifying
      // its name, "bar1.g" in this case. The drawing name is relative to 
      // the current directory or applet's document base if used in a 
      // browser. Parent applet should be passed to a GLG bean by
      // calling SetParentApplet() method so that the bean knows about
      // the current document base.    
      //
      glg_bean.SetParentApplet( this ); 
      glg_bean.SetDrawingName( "bar1.g" );
   
      // Start periodic updates 
      StartUpdates();
   }

   ///////////////////////////////////////////////////////////////////////
   // Invoked by the browser asynchronously to stop the applet.
   ///////////////////////////////////////////////////////////////////////
   public void stop()
   {
      // Using invokeLater() to make sure the applet is destroyed in the
      // event thread to avoid threading exceptions.
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Stop(); } } );
   }

   ///////////////////////////////////////////////////////////////////////
   // Invoked from the event thread via invokeLater().
   ///////////////////////////////////////////////////////////////////////   
   public void Stop()
   {
      // Stop periodic updates
      StopUpdates();

      super.stop();
   }
   
   ////////////////////////////////////////////////////////////////////////
   // Define a class HListener which implements GlgHListener interface. 
   // This class should provide an implementation of the HCallback method.
   // HCallback is invoked after the Glg drawing is loaded, but before the 
   // hierarchy setup takes place and before the drawing is drawn for the 
   // first time. The code for initializing graph parameters such as number 
   // of data samples, range, etc. may be placed here. Since HCallback is 
   // invoked before hierarchy setup, individual datasamples are not 
   // available yet; to access individual datasamples 
   // (DataGroup/DataSample%), use ReadyCallback or VCallback 
   // (GlgReadyListener and GlgVListener interface respectively).
   ////////////////////////////////////////////////////////////////////////
   class HListener implements GlgHListener
   {
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
   }

   //////////////////////////////////////////////////////////////////////
   // Define a class VListener which implements GlgVListener interface 
   // and implements its VCallback() method. VCallback() is invoked after 
   // the drawing is loaded and setup, but before it is drawn for the
   // first time. Graph's individual datasamples DataSample% can be 
   // accessed here.
   /////////////////////////////////////////////////////////////////////
   class VListener implements GlgVListener
   {
      public void VCallback( GlgObject viewport )
      {
      }
   }

   ////////////////////////////////////////////////////////////////////////
   // Define a class ReadyListener which implements GlgReadyListener 
   // interface. This class should provide an implementation of the
   //  ReadyCallback method. ReadyCallback is invoked after the drawing 
   // is loaded, setup and initially drawn.
   // It may be used to place initialization code, including resource
   // settings for a graph's individual datasamples( DataGroup/DataSample% 
   // in case of a bar graph ).
   //////////////////////////////////////////////////////////////////////
   class ReadyListener implements GlgReadyListener
   {	
      public void ReadyCallback( GlgObject viewport )
      {
         // Enable dynamic updates 
         IsReady = true;

         //Place custom initialization code here
      }
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Update procedure to update a graph with random data. This procedure 
   // is invoked periodically by a timer.
   ///////////////////////////////////////////////////////////////////////
   void UpdateGraphProc()
   {
      // Perform dynamic updates only if the drawing is ready and 
      // the timer is active.
      if( !IsReady )
         return;

      // Push next data value
      glg_bean.SetDResource( "DataGroup/EntryPoint", GetData() );

      // Push next label
      glg_bean.SetSResource( "XLabelGroup/EntryPoint", GetLabel() );
         
      glg_bean.Update();   // Make changes visible

   }

   ///////////////////////////////////////////////////////////////////////
   // GetData() method returns random double values to be pushed into the
   // graph. An application should provide a custom data acquisition 
   // mechanism to supply real data values for dynamic dynamics.
   //////////////////////////////////////////////////////////////////////
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
   /////////////////////////////////////////////////////////////////////
   void StartUpdates()
   {
      if( timer == null )
      {
         timer = new Timer( 30, this );
         timer.setRepeats( true );
         timer.start();
      }
   }

   ///////////////////////////////////////////////////////////////////////
   // StopUpdate() method stops periodic updates
   ///////////////////////////////////////////////////////////////////////
   void StopUpdates()
   {      
      // Stop the timer
      if( timer != null )
      {
         timer.stop();
         timer = null;
      }

      // Disable dynamic updates of the drawing
      IsReady = false;
   }

   ///////////////////////////////////////////////////////////////////////
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

   ///////////////////////////////////////////////////////////////////////
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

      GlgGraphComponent graph_example = new GlgGraphComponent();   
      frame.getContentPane().add( graph_example );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      //
      graph_example.glg_bean.SetDrawingName( "bar1.g" );

      // Start periodic updates.
      graph_example.StartUpdates();
   }
}
