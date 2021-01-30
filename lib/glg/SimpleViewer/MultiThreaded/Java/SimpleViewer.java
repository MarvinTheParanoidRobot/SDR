import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class SimpleViewer extends JApplet
{
   static final long serialVersionUID = 0;
   
   /* Define IsStandalone flag which is set to true if running
      a standalone java program; it is set to false if running as 
      an applet in a browser.
   */
   static boolean IsStandalone = false;   
      
   // GLG Viewer instance.
   GlgViewer glg_bean;

   // Use default drawing name unless specified as a command line argument.
   public String DefaultDrawingName = "tags_example.g";
   
   // GLG drawing filename.
   String DrawingFilename = null;
   
   /////////////////////////////////////////////////////////////////////
   // Constructor, where Glg bean component is created and added to a 
   // native Java container, an Applet in this case.
   /////////////////////////////////////////////////////////////////////
   public SimpleViewer()
   {
      getContentPane().setLayout( new GridLayout( 1, 1 ) );
      
      // Instantiate GlgViewer and add it to the parent container.
      glg_bean = new GlgViewer( this );
      getContentPane().add( glg_bean );
   }
   
   //////////////////////////////////////////////////////////////////////
   // Invoked by the browser to start the applet
   //////////////////////////////////////////////////////////////////////
   public void start()
   {
      super.start();
    
      // Process applet parameters.
      String param = getParameter( "DrawingURL" );
      if( param != null )
        DrawingFilename = param;
      else
        DrawingFilename = DefaultDrawingName;
      
      param = getParameter( "RandomData" );
      if( param != null )
        glg_bean.RANDOM_DATA = param.equalsIgnoreCase( "true" );
      
      /* Load a GLG drawing to be displayed in the glg_bean.
         The drawing name is relative to the current directory 
         or applet's document base if used in a browser. 
         SetParentApplet() must be called to pass the document base
         directory of the applet to the bean.
      */
      glg_bean.SetParentApplet( this );
      glg_bean.LoadDrawing( DrawingFilename );
      glg_bean.StartUpdates();
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
      frame.setSize( 700, 700 );
      frame.addWindowListener( new DemoQuit() );

      SimpleViewer viewer = new SimpleViewer();
      viewer.IsStandalone = true;
      frame.getContentPane().add( viewer );
      frame.setVisible( true );
      
      /* Process command line arguments.
         Supported command line options:
         -random-data         (use simulated demo data)
         -live-data           (use live application date from LiveDataFeed)
         drawing_filename     (specifies GLG drawing to be loaded an animated
      */
      viewer.ProcessArgs( arg );

      /* Load a GLG drawing. It will be animated by the glg_bean, an 
         instance of a GlgViewer class.
      */
      viewer.glg_bean.LoadDrawing( viewer.DrawingFilename );

      // Start periodic dynamic updates.
      viewer.glg_bean.StartUpdates();
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
            glg_bean.RANDOM_DATA = true;
            System.out.println( "Using simulated data for animation." );
         }
         else if( args[ skip ].equals( "-live-data" ) )
         {
            // Use live application data for animation.
            glg_bean.RANDOM_DATA = false;
            System.out.println( "Using live application data for animation." );
         }
         else if( args[skip].startsWith( "-" ) )
           continue;
         else
         {
            // Use the drawing file from the command line, if any.
            DrawingFilename = args[ skip ];
         }
      }
      
      /* If drawing file is not supplied on the command line, use 
         default drawing filename defined by DefaultDrawingName.
      */  
      if( DrawingFilename == null )
        DrawingFilename = DefaultDrawingName;     
   }
}
