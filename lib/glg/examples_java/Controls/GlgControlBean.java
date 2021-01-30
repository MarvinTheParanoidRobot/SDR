
/**************************************************************************
  GlgControlBean example may be used as an applet or run as a standalone
  Java program. It demonstrates how to create multiple instances of a
  Glg bean, each displaying its own drawing. In this example, the drawings 
  are control widgets from <glg_dir>/widgets/controls directory and used 
  without modifications. The example shows how to handle user interaction
  in the control widgets, for example reterieving  values from a slider or
  a knob based on the user actions.
***************************************************************************/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgControlBean extends JApplet 
{ 
   static final long serialVersionUID = 0;

   //In this example, Glg light weight Swing component GlgJLWBean is used
   //to display a Glg drawing. If light weight Swing components features
   //are not required, Glg heavy weight Swing component GlgJBean
   //may be used instead to increase update performance.

   GlgJLWBean glg_bean1, glg_bean2;   

   double bean1_low = 10.;
   double bean1_high = 50.;
   double bean1_init_value = 25.;

   double bean2_low = 100.;
   double bean2_high = 500.;
   double bean2_init_value = 300.;

   ///////////////////////////////////////////////////////////////////////
   // Constructor, where Glg bean components are created and added to a 
   // native Java container, an Applet in this case.
   ///////////////////////////////////////////////////////////////////////
   public GlgControlBean()
   {
      // Using no layout.
      getContentPane().setLayout( null );
      getContentPane().setBackground( new Color( 255, 255, 255 ) );

      glg_bean1 = new GlgJLWBean();         
      glg_bean2 = new GlgJLWBean();   

      getContentPane().add( glg_bean1 );
      getContentPane().add( glg_bean2 );

      // Set fixed sizes.
      glg_bean1.setBounds( 50, 70, 150, 150 );
      glg_bean2.setBounds( 240, 50, 80, 220 );

      // Add Ready Listener to the Glg bean 
      glg_bean1.AddListener( GlgObject.READY_CB, new ReadyListener() );
      glg_bean2.AddListener( GlgObject.READY_CB, new ReadyListener() );

      // Add Hierarchy Setup Listener to each Glg bean to set initial
      // parameters of the controls before they are initially drawn.
      glg_bean1.AddListener( GlgObject.H_CB, new HListener( this, 1 ) );
      glg_bean2.AddListener( GlgObject.H_CB, new HListener( this, 2 ) );

      // Add InputListener to a Glg bean to handle user interaction.
      glg_bean1.AddListener( GlgObject.INPUT_CB, new InputListener( 1 ) );
      glg_bean2.AddListener( GlgObject.INPUT_CB, new InputListener( 2 ) );
   }

   ///////////////////////////////////////////////////////////////////////
   // Invoked by the browser to start the applet
   ///////////////////////////////////////////////////////////////////////
   public void start()
   {
      // Set a Glg drawing name to be displayed in each Glg bean.
      // The drawing names are relative to the applet's document base. 
      // Parent applet should be passed to a Glg bean by calling 
      // SetParentApplet() method so that the bean knows about the current 
      // document base.

      glg_bean1.SetParentApplet( this ); 
      glg_bean2.SetParentApplet( this ); 

      glg_bean1.SetDrawingName( "meter5.g" );
      glg_bean2.SetDrawingName( "gauge3.g" );
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
   // will be invoked after a drawing is loaded but before it is initially
   // drawn. It may be used to place initialization code to set initial
   // drawing parameters.
   ///////////////////////////////////////////////////////////////////////
   class HListener implements GlgHListener
   {
      GlgControlBean parent;
      int bean_number;

      public HListener( GlgControlBean parent_p, int bean_number_p )
      {
         parent = parent_p;
         bean_number = bean_number_p;
      }

      public void HCallback( GlgObject viewport )
      {
         //Set initial drawing parameters
         if( bean_number == 1 )
         {
            viewport.SetDResource( "Low", parent.bean1_low );
            viewport.SetDResource( "High", parent.bean1_high );
            viewport.SetDResource( "Value", parent.bean1_init_value );
         }
         else if( bean_number == 2 )
         {
            viewport.SetDResource( "Low", parent.bean2_low );
            viewport.SetDResource( "High", parent.bean2_high );
            viewport.SetDResource( "ValueY", parent.bean2_init_value );
         }
      }
   }

   //////////////////////////////////////////////////////////////////////
   // Define a class ReadyListener which implements GlgReadyListener 
   // interface. This class should provide a ReadyCallback() method which
   // will be invoked after a drawing is loaded and initially 
   // drawn. It may be used to place initialization code.
   //////////////////////////////////////////////////////////////////////
   class ReadyListener implements GlgReadyListener
   {
      public void ReadyCallback( GlgObject viewport )
      {
         // Place custom initialization code here, for example starting
         // dynamic updates, etc.
      }
   }

   ////////////////////////////////////////////////////////////////////////
   // Define a class InputListener which implements GlgInputListener 
   // interface. This class should provide an InputCallback() method which
   // will be invoked when an input event occurred in a Glg drawing,
   // such as clicking on a dial, a slider or any other object
   // with an attached InputHandler.
   ///////////////////////////////////////////////////////////////////////
   class InputListener implements GlgInputListener
   {
      int bean_number;

      public InputListener( int bean_number_p )
      {
         bean_number = bean_number_p;
      }

      public void InputCallback( GlgObject viewport, GlgObject message_obj )
      {
         // String origin = message_obj.GetSResource( "Origin" );
         // String action = message_obj.GetSResource( "Action" );
         // String subaction = message_obj.GetSResource( "SubAction" );

         String format = message_obj.GetSResource( "Format" );
         
         // Input event occurred in a meter or dial
         if( format.equals( "Knob" ) ) 
         {
            if( bean_number == 1 )
            {
               double value = message_obj.GetDResource( "Value" ).doubleValue();
               System.out.println( "meter Value = " + value );
            }
         }
         // Input event occurred in a gauge or slider
         else if( format.equals( "Slider" ) ) 
         {
            if( bean_number == 2 )
            {
               double slider_value;
  
               // Retrieve a current value from a slider (thermometer gauge
               // in this case)
               slider_value = 
                 message_obj.GetDResource( "ValueY" ).doubleValue();
               System.out.println( "gauge Value = " + slider_value );
            }                    
         }
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

      GlgControlBean controls_example = new GlgControlBean();      
      frame.getContentPane().add( controls_example );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      //
      controls_example.glg_bean1.SetDrawingName( "meter5.g" );
      controls_example.glg_bean2.SetDrawingName( "gauge3.g" );
   }
}


