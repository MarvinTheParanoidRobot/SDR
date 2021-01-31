/************************************************************************
  This example demonstrates how to subclass GlgBean to use it as a
  custom graphics component inside another applet.

  This example may be run in a browser using subclass_example.html file or 
  as a standalone Java program. 
************************************************************************/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgSubclassExample extends JApplet
{
   static final long serialVersionUID = 0;

   // Custom component to display and update a GLG graph
   GlgGraphClass graph_component; 

   // Define IsApplet flag which is set to false if running
   // a standalone java program; it is set to true if running as 
   // an applet in a browser.
   static boolean IsApplet = true;

   //////////////////////////////////////////////////////////////////////
   public GlgSubclassExample()
   {
      // Create a custom component and add it to the applet
      graph_component = new GlgGraphClass();

      if( IsApplet )
        // Define a parent applet for the custom graph component.
        graph_component.SetParentApplet( this );

      getContentPane().setLayout( new GridLayout( 1, 1 ) );
      getContentPane().add( graph_component );

      // Set drawing for an applet.
      // For stand-alone applications, drawing will be set outside after
      // setting initial size.
      if( IsApplet )
        SetGraphDrawing();
   }

   //////////////////////////////////////////////////////////////////
   // Loads the graph drawing.
   //////////////////////////////////////////////////////////////////
   void SetGraphDrawing()
   {
      // Set a GLG drawing to be displayed in this component.
      graph_component.SetDrawingName( graph_component.drawing_name );
   }

   /////////////////////////////////////////////////////////////////////
   // Invoked by the browser to start the applet
   /////////////////////////////////////////////////////////////////////
   public void start()
   {
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
      graph_component.StopUpdates();

      super.stop();
   }

   //////////////////////////////////////////////////////////////////////////
   // For use as a stand-alone application
   //////////////////////////////////////////////////////////////////////////
   public static void main( final String arg[] )
   {
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Main( arg ); } } );
   }

   //////////////////////////////////////////////////////////////////////
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

      // Set IsApplet global flag which is used by the constructor
      // to determine if the bean is used in a browser or standalone 
      // Java program.
      GlgSubclassExample.IsApplet = false;

      GlgSubclassExample graph_example = new GlgSubclassExample();
      frame.getContentPane().add( graph_example );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished to make sure the drawing
      // appears in the right size and does not resize after the initial 
      // appearance.
      //
      graph_example.SetGraphDrawing();
   }

   /////////////////////////////////////////////////////////////////////
   // Subclasses the GlgJBean class to define a custom graphical 
   // componenet which will display and update a GLG graph. The name of
   // the drawing displayed in this custom component is "bar1.g" and 
   // is defined in the constructor of this class.
   /////////////////////////////////////////////////////////////////////
   class GlgGraphClass extends GlgJBean implements ActionListener
   {   
      static final long serialVersionUID = 0;

      int label_counter = 0;
      final double high = 10.;  // Graph's high range
      final double low = 0.;    // Graph's low range
      Timer timer = null;
      boolean IsReady = false;
      String drawing_name;

      //////////////////////////////////////////////////////////////////
      // Constructor
      //////////////////////////////////////////////////////////////////
      public GlgGraphClass()
      {
         // Turn on Java diagnostics
         SetJavaLog( true );

         drawing_name = "bar1.g";
      }

      /////////////////////////////////////////////////////////////////
      // ReadyCallback() is invoked after the drawing is loaded, setup
      //  and initially drawn.
      /////////////////////////////////////////////////////////////////
      public void ReadyCallback( GlgObject viewport )
      {
         // Start periodic updates.
         StartUpdates();

         // The drawing is ready to process dynamic updates.
         IsReady = true;

         // Place custom initialization code here, for example accessing
         // graph's individual datasamples, etc..       
      }

      ////////////////////////////////////////////////////////////////
      // HCallback is a Hierarchy Setup callback which is invoked 
      // after the drawing is loaded but before the drawing is setup 
      // and initially drawn. The code for initializing graph parameters
      // such as number of data samples, range, etc. may be placed here. 
      // Since HCallback() is invoked before hierarchy setup takes place, 
      // individual datasamples of a graph are not available yet; to 
      // access individual datasamples (DataGroup/DataSample%), use 
      // ReadyCallback  or VCallback.
      /////////////////////////////////////////////////////////////////
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

      /////////////////////////////////////////////////////////////////
      // VCallback() is invoked after the drawing is loaded and setup.
      // Graph's individual datasamples DataSample% can be accessed here.
      /////////////////////////////////////////////////////////////////
      public void VCallback( GlgObject viewport )
      {
      }

      ////////////////////////////////////////////////////////////////
      // Update procedure to update the graph with random data. This
      // procedure is invoked periodically by a timer.
      ////////////////////////////////////////////////////////////////
      void UpdateGraphProc()
      {
         // Update the drawing only if it is ready to process dynamic updates.
         if( !IsReady )
            return;

         // Push next data value into a graph
         SetDResource( "DataGroup/EntryPoint", GetData() );

         // Push next label
         SetSResource( "XLabelGroup/EntryPoint", GetLabel() );
         
         Update();   // Make changes visible
      }

      //////////////////////////////////////////////////////////////////
      // GetData() method returns random double values to be pushed into 
      // the graph. An application should provide a custom data 
      // acquisition mechanism to supply data values for updating 
      // dynamics.
      //////////////////////////////////////////////////////////////////
      double GetData()
      {
         return high * Math.random();    // Random value
      }

      //////////////////////////////////////////////////////////////////
      // GetLabel() method returns a label counter to be used for 
      // labeling the X axis.
      //////////////////////////////////////////////////////////////////
      String GetLabel()
      {
         // Increase label counter
         ++label_counter;
         if( label_counter > 9999 )
            label_counter = 0;
      
         return "#" + label_counter;   // Numerical label
      }

      //////////////////////////////////////////////////////////////////
      // StartUpdates() creates a timer to perform periodic updates.
      // The timer invokes the bean's UpdateGraphProc() method to update
      // drawing's resoures with new data values.
      /////////////////////////////////////////////////////////////////
      void StartUpdates()
      {
         if( timer == null )
         {
            timer = new Timer( 30, this );
            timer.setRepeats( true );
            timer.start();
         }
      }

      /////////////////////////////////////////////////////////////////
      // StopUpdate() method stops periodic updates
      /////////////////////////////////////////////////////////////////
      void StopUpdates()
      {      
         if( timer != null )
         {
            timer.stop();
            timer = null;
         }

         // Disable dynamic updates.         
         IsReady = false;
      }

      /////////////////////////////////////////////////////////////////
      // timer's ActionListener method to be invoked peridically 
      /////////////////////////////////////////////////////////////////
      public void actionPerformed( ActionEvent e )
      {
         UpdateGraphProc();
      }

      ////////////////////////////////////////////////////////////////
      // Override removeNotify method of the Java Component class to
      // to stop dynamic updates.
      ////////////////////////////////////////////////////////////////
      public void removeNotify()
      {
         // Stop periodic updates
         StopUpdates();

         super.removeNotify();
      }
   }
}
