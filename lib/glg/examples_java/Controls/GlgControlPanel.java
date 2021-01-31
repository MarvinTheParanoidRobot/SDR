/*************************************************************************
  This example demonstrates how to use a Glg bean to display and interact
  with a Glg drawing containing a panel of Glg controls. The drawing has
  been composed in the GlgBuilder and contains several Glg control widgets.
  The top level viewport of the drawing, the $Widget viewport, contains
  children viewports named "Dial", "Slider" and "QuitButton" that represent
  a dial, a slider and a GLG button. These controls were inserted 
  into the drawing in the GlgBuilder and their viewports were renamed from 
  $Widget to "Dial", "Slider" and "QuitButton" respectively.

  A Glg bean is added as a subcomponent to the parent container, an
  applet in this case. The drawing to be displayed in the Glg bean is
  specified by setting the drawing name ( "controls.g" ) in the start()
  method, which is invoked by the browser, or in the main() method 
  if running a standalone Java program.

  User interaction in the controls is handled in the InputListener.
  InputListener is added as a listener object to the Glg bean.
**************************************************************************/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgControlPanel extends JApplet 
{ 
   static final long serialVersionUID = 0;

   // In this example, Glg light weight Swing component GlgJLWBean is used
   // to display a Glg drawing. If light weight Swing components features
   // are not required, Glg heavy weight Swing component GlgJBean
   // may be used instead to increase update performance.

   GlgJLWBean glg_bean;   

   double low_range = 0.;
   double high_range = 50.;
   double init_value = 30.;

   // Define IsStandalone flag which is set to true if running
   // a standalone java program; it is set to false if running as 
   // an applet in a browser.
   static boolean IsStandalone = false;   

   ///////////////////////////////////////////////////////////////////////
   // Constructor, where Glg bean components are created and added to a 
   // native Java container, an Applet in this case.
   ///////////////////////////////////////////////////////////////////////
   public GlgControlPanel()
   {
      getContentPane().setLayout( new GridLayout( 1, 1 ) );

      glg_bean = new GlgJLWBean();   
      getContentPane().add( glg_bean );

      // Add listeners to the Glg bean to "listen" to the particular
      // events that occur in the bean and place custom code there.
      // Listeners should be added BEFORE setting the drawing name to
      // be displayed in the bean ( before calling SetDrawingName() ).

      // Add Ready Listener to the Glg bean to place initialization code
      // after intial draw took place.
      glg_bean.AddListener( GlgObject.READY_CB, new ReadyListener() );

      // Add Hierarchy Setup Listener to set initial parameters of the 
      // controls before the drawing is painted for the first time.
      glg_bean.AddListener( GlgObject.H_CB, new HListener( this ) );

      // Add InputListener to handle user interaction.
      glg_bean.AddListener( GlgObject.INPUT_CB, new InputListener( this ) );
   }

   //////////////////////////////////////////////////////////////////
   // Invoked by a browser to start the applet.
   //////////////////////////////////////////////////////////////////
   public void start()
   {
      super.start();

      // Set the GLG drawing name to be displayed in the GLG bean,
      // "controls.g" in this case. The drawing name is relative to 
      // the applet's document base. Parent applet should be passed 
      // to a GLG bean by calling SetParentApplet() method so that 
      // the bean knows about the current document base.
      //
      glg_bean.SetParentApplet( this ); 
      glg_bean.SetDrawingName( "controls.g" );
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
      super.stop();
   }

   ///////////////////////////////////////////////////////////////////////
   // Define a class HListener which implements GlgHListener 
   // interface. This class should provide an HCallback() method which
   // will be invoked after a Glg drawing is loaded but before hierarchy
   // setup and initial draw took place.
   ///////////////////////////////////////////////////////////////////////
   class HListener implements GlgHListener
   {
      GlgControlPanel parent;

      public HListener( GlgControlPanel parent_p )
      {
         parent = parent_p;
      }

      public void HCallback( GlgObject viewport )
      {

         // Disable InputHandler for a dial control so that the dial acts
         // only as an output device and doesn't respond to user input.
         viewport.SetDResource( "Dial/HandlerDisabled", 1. );

         // Set Hign and Low range resources for both Slider and Dial 
         // controls.
         viewport.SetDResource( "Dial/Low", parent.low_range );
         viewport.SetDResource( "Slider/Low", parent.low_range );

         viewport.SetDResource( "Dial/High", parent.high_range );
         viewport.SetDResource( "Slider/High", parent.high_range );

         // Set initial values of a dial and a slider
         viewport.SetDResource( "Dial/Value", parent.init_value );
         viewport.SetDResource( "Slider/ValueY", parent.init_value );
      }
   }

   ///////////////////////////////////////////////////////////////////////
   // Define a class ReadyListener which implements GlgReadyListener 
   // interface. This class should provide a ReadyCallback() method which
   // will be invoked after a drawing is initially drawn. It may be used 
   // to place initialization code, for example starting updates and
   // retrieving values from a given source.
   //////////////////////////////////////////////////////////////////////
   class ReadyListener implements GlgReadyListener
   {
      public void ReadyCallback( GlgObject viewport )
      {
         // Place custom code here
      }
   }

   ///////////////////////////////////////////////////////////////////////
   // Define a class InputListener which implements GlgInputListener 
   // interface. This class should provide an InputCallback() method which
   // will be invoked when an input event occurred in a Glg drawing,
   // such as clicking on a button, a slider or any other object
   // with an attached InputHandler.
   ///////////////////////////////////////////////////////////////////////
   class InputListener implements GlgInputListener
   {
      GlgControlPanel top_applet;
      
      public InputListener( GlgControlPanel top_applet_p )
      {
         top_applet = top_applet_p;
      }

      public void InputCallback( GlgObject viewport, GlgObject message_obj )
      {
         String origin = message_obj.GetSResource( "Origin" );
         String format = message_obj.GetSResource( "Format" );
         String action = message_obj.GetSResource( "Action" );
         // String subaction = message_obj.GetSResource( "SubAction" );
         
         if( format.equals( "Button" ) ) //input event occurred in a button
         {
            if( !action.equals( "Activate" ) )
              return;
            
            if( origin.equals( "QuitButton" ) )
            {
               if( GlgControlPanel.IsStandalone )
                 System.exit( 0 );
               else
                 top_applet.stop();
            }                            
         }
         
         if( format.equals( "Slider" ) ) //input event occurred in a slider
         {
            double slider_value;
            
            // Retrieve a current slider value from a message object
            slider_value = message_obj.GetDResource( "ValueY" ).doubleValue();
            // Set a data value for a dial control
            viewport.SetDResource( "Dial/Value", slider_value );
         }

         // Update the viewport to reflect new resource settings
         viewport.Update();
      }
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
      frame.setSize( 400, 400 );
      frame.addWindowListener( new DemoQuit() );

      // Set IsStandalone global flag which is used by the 
      // GlgControlPanel class to determine if the bean is used
      // in a browser or standalone Java program.
      //
      GlgControlPanel.IsStandalone = true;
      
      GlgControlPanel controls_example = new GlgControlPanel();   
      frame.getContentPane().add( controls_example );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      //
      controls_example.glg_bean.SetDrawingName( "controls.g" );
   }
}
