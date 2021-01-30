/***************************************************************************
  This example demonstrates how to display and update a GLG realtime
  stripchart widget and use its integrated interactive behavior.
  The program may be used as an applet in a browser using 
  realtime_chart.html, or as a standalone Java program. 

  GlgChart is added to the parent container and displays a stripchart
  drawing that may be supplied as a command line argument.
  
  Supported command line options:
    -random-data         (use simulated demo data)
    -live-data           (use live application date from LiveDataFeed)
    drawing_name         (specifies GLG drawing to be loaded an animated

  By default, the example uses "stripchart.g", containing a horizontal 
  stripchart and a toolbar with controls allowing to zoom and scroll the chart.

  "stripchart_vertical.g" is similar to stripchart.g, but uses a
  vertical stripchart. 

  To run the example using a vertical stripchart:
     java GlgRTChartExample stripchart_vertical.g
***************************************************************************/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

//////////////////////////////////////////////////////////////////////////
public class GlgRTChartExample extends JApplet
{
   static final long serialVersionUID = 0;

   /* To use live data, set  RANDOM_DATA=false, or use
      -live-data command line option.
   */
   public boolean RANDOM_DATA = true;
   
   // Use default drawing name unless specified as a command line argument.
   String DefaultDrawingName = "stripchart.g";
   
   // GLG drawing filename.
   String DrawingName = null;
   
   GlgChart glg_bean;
   
   /////////////////////////////////////////////////////////////////////
   // Constructor, where Glg bean component is created and added to a 
   // native Java container, an Applet in this case.
   /////////////////////////////////////////////////////////////////////
   public GlgRTChartExample()
   {
      getContentPane().setLayout( new GridLayout( 1, 1 ) );
      
      // Create GlgChart, passing a parent container.
      glg_bean = new GlgChart( this );
      glg_bean.RandomData = RANDOM_DATA;
      
      // Add the bean to the parent container.
      getContentPane().add( glg_bean );
   }
   
   //////////////////////////////////////////////////////////////////////
   // Invoked by the browser to start the applet.
   //////////////////////////////////////////////////////////////////////
   public void start()
   {
      super.start();
      
      // Process applet parameters.
      String param = getParameter( "DrawingURL" );
      if( param != null )
        DrawingName = param;
      else
        DrawingName = DefaultDrawingName;
      
      param = getParameter( "RandomData" );
      if( param != null )
        glg_bean.RandomData = param.equalsIgnoreCase( "true" );
      
      /* Load a GLG drawing to be displayed in the glg_bean.
         The drawing name is relative to the current directory 
         or applet's document base if used in a browser. 
         SetParentApplet() must be called to pass the document base
         directory of the applet to the bean.
      */
      glg_bean.SetParentApplet( this );
      glg_bean.LoadDrawing( DrawingName );
      glg_bean.StartUpdates();
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Invoked by the browser asynchronously to stop the applet.
   ///////////////////////////////////////////////////////////////////////
   public void stop()
   {
      /* Using invokeLater() to make sure the applet is destroyed in the
         event thread to avoid threading exceptions.
      */
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
      frame.setSize( 700, 600 );
      frame.addWindowListener( new DemoQuit() );

      GlgRTChartExample panel = new GlgRTChartExample();
      frame.getContentPane().add( panel );
      frame.setVisible( true );
      
      /* Process command line arguments.
         Supported command line options:
         -random-data         (use simulated demo data)
         -live-data           (use live application date from LiveDataFeed)
         drawing_name         (specifies GLG drawing to be loaded an animated
      */
      panel.ProcessArgs( arg );

      /* Set a GLG drawing to be displayed in the GLG bean
         Set after layout negotiation has finished.
      */
      panel.glg_bean.LoadDrawing( panel.DrawingName );

      // Start periodic updates.
      panel.glg_bean.StartUpdates();
   }

   //////////////////////////////////////////////////////////////////////////
   // Process command line arguments.
   //////////////////////////////////////////////////////////////////////////
   public void ProcessArgs( String [] args )
   {
      if( args == null )
        return;
          
      for( int skip = 0; skip < args.length; ++skip )
      {
         if( args[ skip ].equals( "-random-data" ) )
         {
            // Use simulated demo data for animation.
            glg_bean.RandomData = true;
            System.out.println( "Using simulated data for animation." );
         }
         else if( args[ skip ].equals( "-live-data" ) )
         {
            // Use live application data for animation.
            glg_bean.RandomData = false;
            System.out.println( "Using live application data for animation." );
         }
         else if( args[skip].startsWith( "-" ) )
           continue;
         else
         {
            // Use the drawing file from the command line, if any.
            DrawingName = args[ skip ];
         }
      }
      
      /* If drawing file is not supplied on the command line, use 
         default drawing filename defined by DefaultDrawingName.
      */  
      if( DrawingName == null )
        DrawingName = DefaultDrawingName;     
   }
}
