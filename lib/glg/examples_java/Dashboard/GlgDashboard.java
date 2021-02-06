import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgDashboard extends JApplet implements ActionListener
{ 
   static final long serialVersionUID = 0;

   /* In this example, Glg light weight Swing component GlgJLWBean is used
      to display a Glg drawing. If light weight Swing components features
      are not required, Glg heavy weight Swing component GlgJBean
      may be used instead to increase update performance.
   */
   GlgJLWBean glg_bean;   

   /* If set to true, tags defined in the drawing are used for animation.
      Otherwise, object resources are used to push real-time values 
      into the drawing.
   */
   boolean USE_TAGS = true;
   
   Timer timer = null;                // Timer for periodic updates.
   int UpdateInterval = 100;          // Update rate in msec.
   
   /* Ready flag is set to true in Ready callback to indicate the 
      drawing is ready to be updated with dynamic data.
   */
   boolean Ready = false;
   
   // Used to provide simulated data.
   int counter = 0;
   
   ///////////////////////////////////////////////////////////////////////
   // Constructor, where Glg bean components are created and added to a 
   // native Java container, an Applet in this case.
   ///////////////////////////////////////////////////////////////////////
   public GlgDashboard()
   {
      getContentPane().setLayout( new GridLayout( 1, 1 ) );

      glg_bean = new GlgJLWBean();   
      getContentPane().add( glg_bean );

      /* Add listeners to the Glg bean to handle events.
         Listeners should be added BEFORE setting the drawing name to
         be displayed in the bean ( before calling SetDrawingName() ).
      */

      // Ready Listener is invoked after first draw.
      glg_bean.AddListener( GlgObject.READY_CB, new ReadyListener() );

      // H Listener is invoked after hierarchy setup, but before first draw.
      glg_bean.AddListener( GlgObject.H_CB, new HListener( this ) );

      // Input Listener is used to handle user interaction.
      glg_bean.AddListener( GlgObject.INPUT_CB, new InputListener( this ) );
   }

   ///////////////////////////////////////////////////////////////////////
   // Define a class HListener which implements GlgHListener 
   // interface. This class should provide an HCallback() method which
   // will be invoked after a Glg drawing is loaded but before hierarchy
   // setup and initial draw took place.
   ///////////////////////////////////////////////////////////////////////
   class HListener implements GlgHListener
   {
      GlgDashboard parent;

      public HListener( GlgDashboard parent_p )
      {
         parent = parent_p;
      }

      public void HCallback( GlgObject viewport )
      {
         // Set initial patameters as needed.
         viewport.SetDResource( "DialPressure/Low", 0.0 );
         viewport.SetDResource( "DialVoltage/Low", 0.0 );
         viewport.SetDResource( "DialAmps/Low", 0.0 );
         viewport.SetDResource( "SliderPressure/Low", 0.0 );
         
         viewport.SetDResource( "DialPressure/High", 50.0 );
         viewport.SetDResource( "DialVoltage/High", 120.0 );
         viewport.SetDResource( "DialAmps/High", 10.0 );
         viewport.SetDResource( "SliderPressure/High", 50.0 );
      }
   }

   ///////////////////////////////////////////////////////////////////////
   // ReadyListener implements GlgReadyListener interface. 
   // This class should provide a ReadyCallback() method which
   // will be invoked after the drawing is initially drawn. It may be used 
   // to place initialization code, for example starting updates.
   //////////////////////////////////////////////////////////////////////
   class ReadyListener implements GlgReadyListener
   {
      public void ReadyCallback( GlgObject viewport )
      {
         Ready = true;
         StartUpdates();
      }
   }

   ///////////////////////////////////////////////////////////////////////
   // InputListener implements GlgInputListener interface. 
   // This class should provide an InputCallback() method which
   // will be invoked when an input event occurred in a Glg drawing,
   // such as clicking on a button, a slider or any other object
   // with an attached InputHandler.
   ///////////////////////////////////////////////////////////////////////
   class InputListener implements GlgInputListener
   {
      GlgDashboard parent;
      
      public InputListener( GlgDashboard parent_p )
      {
         parent = parent_p;
      }

      public void InputCallback( GlgObject viewport, GlgObject message_obj )
      {
         String origin = message_obj.GetSResource( "Origin" );
         String format = message_obj.GetSResource( "Format" );
         String action = message_obj.GetSResource( "Action" );
          
         // In this example, process events only from the named input widgets.
         if( origin == null )
           return;

         if( format.equals( "Button" ) ) // Button events.
         {
            if( action.equals( "Activate" ) )  //Push button events.
            {
               if( origin.equals( "QuitButton" ) )
                 System.exit( 0 );    //Exit
            }
            else if( action.equals( "ValueChanged" ) ) //Toggle button events.
            {
               if( origin.equals( "StartButton" ) )
               {
                  int value = message_obj.GetDResource("OnState").intValue();
                  switch (value)
                  {
                   case 0:
                     StopUpdates();
                     break;
                   case 1:
                     StartUpdates();
                     break;
                   default: break;
                  }
               }
            }
            
            // Refresh display.
            viewport.Update();
         }
         
         // Input occurred in a slider named SliderPressure.
         else if( format.equals( "Slider" ) &&
                  origin.equals( "SliderPressure" ) )
         {
            // Retrieve current slider value from the message object.
            double slider_value = 
              message_obj.GetDResource("ValueY").doubleValue();
            
            // Set a data value for a dial control DialPressure.
            viewport.SetDResource( "DialPressure/Value", slider_value);
            viewport.Update();
         }
      }
   }
    //////////////////////////////////////////////////////////////////////
   // StartUpdates() creates a timer to perform periodic updates.
   // The timer invokes the bean's UpdateGraphProc() method to update
   // drawing's resoures with new data values.
   //////////////////////////////////////////////////////////////////////
   public void StartUpdates()
   {
      if( timer == null )
      {
         timer = new Timer( UpdateInterval, this );
         timer.setRepeats( true );
         timer.start();
      }
   }

   //////////////////////////////////////////////////////////////////////
   // StopUpdate() method stops periodic updates
   //////////////////////////////////////////////////////////////////////
   public void StopUpdates()
   {      
      if( timer != null )
      {
         timer.stop();
         timer = null;
      }
   }

   //////////////////////////////////////////////////////////////////////
   // timer's ActionListener method to be invoked peridically 
   //////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateDrawing();
   }

   ///////////////////////////////////////////////////////////////////////
   // Update the drawing with new dynamic data values.
   ///////////////////////////////////////////////////////////////////////
   public void UpdateDrawing()
   {
      // Perform dynamic updates only if the drawing is ready.
      if( !Ready )
        return;
      
      /* Obtain simulated demo data values in a specified range.
         The application should provide a custom implementation
         of the data acquisition interface to obtain real-time
         data values.
      */
      double voltage = GetData( 0.0, 120.0 );
      double current = GetData( 0.0, 10.0 );
      
      if( USE_TAGS ) // Use tags for animation.
      {
         // Push values to the objects using tags defined in the drawing.
         glg_bean.SetDTag( "Voltage", voltage, true /*if_changed*/ );
         glg_bean.SetDTag( "Current", current, true /*if_changed*/ );
      }
      else // Use resources for animation.
      {
         // Push values to the objects using resource paths.
         glg_bean.SetDResource("DialVoltage/Value", voltage );
         glg_bean.SetDResource("DialAmps/Value", current );
      }
      
      glg_bean.Update();
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
      frame.setSize( 500, 500 );
      frame.addWindowListener( new DemoQuit() );
      
      GlgDashboard glg_dashboard = new GlgDashboard();   
      frame.getContentPane().add( glg_dashboard );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      //
      glg_dashboard.glg_bean.SetDrawingName( "dashboard.g" );
   }

   /////////////////////////////////////////////////////////////////////// 
   // Generates demo data value within a specified range. 
   // An application can replace code in this method to supply 
   // real-time data from a custom data source.
   ///////////////////////////////////////////////////////////////////////
   double GetData( double low, double high )
   {
      double
        half_amplitude, center,
                period,
        value,
        alpha;
      
      half_amplitude = ( high - low ) / 2.0;
      center = low + half_amplitude;
      
      period = 100.0;
      alpha = 2.0 * Math.PI * counter / period;
      
      value = center +
        half_amplitude * Math.sin( alpha ) * Math.sin( alpha / 30.0 );
      
      ++counter;
      return value;
   }
}
