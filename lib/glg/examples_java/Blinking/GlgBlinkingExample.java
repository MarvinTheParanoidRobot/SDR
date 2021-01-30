/*----------------------------------------------------------------------
| This example demonstrates how to use integrated blinking feature
| of the GLG Toolkit. 
| 
| When water level in the tank reaches 90%, two alarm objects will
| start blinking. 
|
| In order to update blinking objects, Update() must be called
| in the Input callback. Update() can be invoked either at the end of 
| the Input callback for all the messages, or it can be called for 
| blinking messages only.
|
| The "blinking.g" drawing contains two alarm objects, where one of them
| blinks using the color attribute (a circle), and another blinks using 
| the visibility attribute (a text object). In both cases, blinking
| functionality is implemented by attaching a Timer transformation to the
| attribute being blinked (toggled). 
|
| One alarm object is a group named "Alarm" containing a circle whose 
| FillColor has a List transformation. ValueIndex parameter of that 
| transformation has a Timer transformation, causing ValueIndex
| being toggled between 0 and 1 when Timer is enabled (Alarm/AlarmEnabled 
| resource is set to 1.). Toggling the ValueIndex attribute consequently
| causes color to alternate between green and red. 
|
| The second alarm object is a text object named "AlarmText" with a 
| string OVERFLOW, and its Visibility attribute has a Timer transformation 
| attached. When AlarmText/AlarmEnabled resource is set to 1., 
| the text starts blinking by toggling its Visibility resource.
|
| Periodic dynamic updates are handled in the UpdateAnimation method.
*/

import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgBlinkingExample extends GlgJBean implements ActionListener
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////

   static final long serialVersionUID = 0;

   // Low and High range of the water lavel in the tank.
   double Low, High;

   // Increment value for dynamic data simulation.
   double Increment;	
   
   // Static variable used by UpdateAnimation() method that generates
   // animation values. 
   double WaterLevel;

   // Time interval for periodic updates, in millisec.
   int TimeInterval = 20; 

   boolean IsReady = false;

   Timer timer = null;

   //////////////////////////////////////////////////////////////////////////
   public GlgBlinkingExample()
   {
      super();
   }

   //////////////////////////////////////////////////////////////////////////
   // Starts updates
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      if( GetJavaLog() )
        PrintToJavaConsole( "Debug: Ready\n" );

      super.ReadyCallback();

      // Extract the Low and High ranges of the water level value in 
       // the tank.
      Low = GetDResource( "WaterTank/Low" );
      High = GetDResource( "WaterTank/High" );

      // Make 50 iterations before starting an alarm.
      Increment = (High - Low) / 50.; 

      // Initialize water_level variable.
      WaterLevel = Low;

      if( timer == null )
      {
         timer = new Timer( TimeInterval, this );
         timer.setRepeats( true );
         timer.start();
      }

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

      JFrame frame = new JFrame();
      frame.setResizable( true );
      frame.setSize( 400, 450 );
      frame.setLocation( 20, 20 );
      frame.addWindowListener( new DemoQuit() );

      GlgBlinkingExample blinking_example = new GlgBlinkingExample();      
      frame.getContentPane().add( blinking_example );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      // Setting the drawing triggers the ReadyCallback which starts updates.
      //
      blinking_example.SetDrawingName( "blinking.g" );
   }

   //////////////////////////////////////////////////////////////////////////
   public void UpdateAnimation()
   {      
      if( !IsReady() )
        return;

      // Calculate new water level. When water level reaches 90% of the
      // tank, enable alarms. 
      WaterLevel += Increment;	
      if( WaterLevel >= (High - Low) * .9 )
      {
         SetDResource( "Alarm/AlarmEnabled", 1. );
         SetDResource( "AlarmText/AlarmEnabled", 1. );
      }
      else
      {
         // Update dynamic resources of the tank and the meter. 
         SetDResource( "WaterTank/WaterLevel", WaterLevel );
         SetDResource( "WaterMeter/Value", WaterLevel );
      }

      Update();    // Make changes visible.
   }

   //////////////////////////////////////////////////////////////////////////
   // This callback is used to handle input events, object selection
   //  events and blinking events.
   //////////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject viewport, GlgObject message_obj )
   {
      String
        origin,
        format,
        action;

      super.InputCallback( viewport, message_obj );

      origin = message_obj.GetSResource( "Origin" );
      format = message_obj.GetSResource( "Format" );
      action = message_obj.GetSResource( "Action" );

      // Handle window closing if run stand-alone
      if( format.equals( "Window" ) && action.equals( "DeleteWindow" ) )
        System.exit( 0 );

      if( format.equals( "Button" ) && action.equals( "Activate" ) )
      {	 
         // Exit if the user clicks on the Quit button.
         if( origin.equals( "QuitButton" ) )
            System.exit( 0 );
      }
      // Blinking messages.
      else if( format.equals( "Timer" ) && action.equals( "Update" ) )
      {
         // GlgUpdate() may be called here to update blinking objects
         //  only, instead of calling it at the end of the Input callback.
         // Update();
      }
      
      // Update the viewport. It is also necessary for updating 
      // blinking objects.
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked by the browser asynchronously to stop the applet.
   //////////////////////////////////////////////////////////////////////////
   public void stop()
   {
      if( timer != null )
      {
         timer.stop();
         timer = null;
      }

      IsReady = false;

      // GlgJBean handles asynchronous invocation when used as an applet.
      super.stop();
   }

   //////////////////////////////////////////////////////////////////////////
   // ActionListener method to use the bean as update timer's ActionListener.
   //////////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateAnimation();
   }
}
