/***************************************************************************
  This example demonstrates how to display and update a GLG realtime
  stripchart widget and use its integrated interactive behavior.
  The program may be used as an applet in a browser or as a standalone 
  Java program. 
   
  The example also shows how to handle alarms using a global Alarm handler.
  
  GlgChart is added to the parent container and displays a GLG
  drawing "chart_with_alarm.g", which includes a stripchart widget 
  with 2 plots, as well as a toolbar containing some interface widgets 
  and two alarm indicators, one for each plot.

  The alarm indicators are green when the corresponding plot's value 
  is within the defined "soft" low/high limits; the indicators turn 
  yellow when the value is outside of the "soft" limits, but within the
  "hard" limits, and turn red when the data is outside of the "hard" limits. 
  Once an alarm indicator turns red, it is locked and can be cleared 
  only when the user clicks on the Clear button (acknowledging the alarm).

  For demo purposes, the chart displays simulated data. 
  The application may provide a custom data feed supplying 
  real-time application specific data to the chart, which can be
  done via a custom implementation of the DataFeedInterace. 
  A sample of a custom data feed class is available in
  DataFeedSample.java.
***************************************************************************/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

//////////////////////////////////////////////////////////////////////////
public class GlgRTChartAlarm extends JApplet
{
   static final long serialVersionUID = 0;

   GlgChart glg_bean;
   int UpdateInterval = 100;

   /////////////////////////////////////////////////////////////////////
   // Constructor, where Glg bean component is created and added to a 
   // native Java container, an Applet in this case.
   /////////////////////////////////////////////////////////////////////
   public GlgRTChartAlarm()
   {
      getContentPane().setLayout( new GridLayout( 1, 1 ) );
      
      // Instantiate GlgChart, without prefilling it with data.
      glg_bean = new GlgChart( false /* don't prefill with data */ );
      
      // Initialize chart parameters as needed.
      glg_bean.YLow = -5.;
      glg_bean.YHigh = 5.;
      
      // Level lines. 
      glg_bean.LowHard= -4.5;
      glg_bean.LowSoft = -4.;
      glg_bean.HighHard = 4.5;
      glg_bean.HighSoft = 4.;
      
      // Add custom data feed. The application may provide a 
      // custom implementation of the DataFeedInterace to plot
      // real-time application specific data in the chart.
      // glg_bean.AddDataFeed( new DataFeedSample( this ) );
      
      getContentPane().add( glg_bean );
   }
   
   //////////////////////////////////////////////////////////////////////
   // Invoked by the browser to start the applet
   //////////////////////////////////////////////////////////////////////
   public void start()
   {
      super.start();

      // Load a GLG drawing to be displayed in the glg_bean.
      // The drawing name is relative to the current directory 
      // or applet's document base if used in a browser. 
      // SetParentApplet() must be called to pass the document base
      // directory of the applet to the bean.
      //
      glg_bean.SetParentApplet( this );
      glg_bean.LoadDrawing();
      glg_bean.StartUpdates( UpdateInterval );
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
      glg_bean.StopUpdates();

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

   ///////////////////////////////////////////////////////////////////////
   public static void Main( final String arg[] )
   {
      class DemoQuit extends WindowAdapter
      {
         public void windowClosing( WindowEvent e ) { System.exit( 0 ); }
      }

      JFrame frame = new JFrame();
      frame.setResizable( true );
      frame.setSize( 800, 600 );
      frame.addWindowListener( new DemoQuit() );

      GlgRTChartAlarm graph = new GlgRTChartAlarm();
      frame.getContentPane().add( graph );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      //
      graph.glg_bean.LoadDrawing();
      
      // Start periodic updates.
      graph.glg_bean.StartUpdates( graph.UpdateInterval );
   }
}
