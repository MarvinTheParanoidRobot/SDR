import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgControlsDemo extends GlgJBean implements ActionListener
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   static final int NUM_VALUES = 25;
   boolean PerformUpdates = true;
   GlgAnimationValue [] animation_array = new GlgAnimationValue[ NUM_VALUES ];
   static boolean AntiAliasing = true;

   Timer timer = null;

   //////////////////////////////////////////////////////////////////////////
   public GlgControlsDemo()
   {
      super();
      SetDResource( "$config/GlgAntiAliasing", AntiAliasing ? 1.0 : 0.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   // Starts updates
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );

      InitializeArrays();
      
      if( timer == null )
      {
         // Restart the timer after each update (instead of using repeats)
         // to avoid flooding the event queue with timer events on slow 
         // machines.
         timer = new Timer( 350, this );
         timer.setRepeats( false );
         timer.start();
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // For use as a stand-alone java demo
   //////////////////////////////////////////////////////////////////////////
   public static void main( final String [] arg )
   {
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Main( arg ); } } );
   }

   //////////////////////////////////////////////////////////////////////////
   public static void Main( final String [] arg )
   {
      class DemoQuit extends WindowAdapter
      {
         public void windowClosing( WindowEvent e ) { System.exit( 0 ); }
      } 

      JFrame frame = new JFrame( "GLG Dials and Gauges Demo" );

      frame.setResizable( true );
      frame.setSize( 750, 600 );
      frame.setLocation( 20, 20 );

      GlgControlsDemo controls = new GlgControlsDemo();
      frame.getContentPane().add( controls );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );

      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers ReadyCallback which starts updates.
      //
      controls.SetDrawingName( "controls.g" );
   }

   //////////////////////////////////////////////////////////////////////////
   public void UpdateControls()
   { 
      if( timer == null )
        return;   // Prevents race conditions

      if( PerformUpdates )
      {
         // Update all animation_values
         for( int i=0; i < NUM_VALUES; ++i )
           if( animation_array[ i ] != null )
             animation_array[ i ].Iterate();

         Update();   // Show changes
      }

      timer.start();   // Restart the update timer
   }

   //////////////////////////////////////////////////////////////////////////
   public void StartUpdate()
   {
      if( !PerformUpdates )
      {
         PerformUpdates = true;
         SetDResource( "ManualMode", 0.0 );
         Update();
      }
   }

   //////////////////////////////////////////////////////////////////////////
   public void StopUpdate()
   {
      if( PerformUpdates )
      {
         PerformUpdates = false;
         SetDResource( "ManualMode", 1.0 );
         Update();
      }
   }

   //////////////////////////////////////////////////////////////////////////
   public void ToggleAntiAliasing()
   {
      AntiAliasing = !AntiAliasing;
      SetDResource( "$config/GlgAntiAliasing", AntiAliasing ? 1.0 : 0.0 );

      // Restart with the new AntiAliasing setting
      stop();
      start();
   }

   //////////////////////////////////////////////////////////////////////////
   void InitializeArrays()
   {
      // Initilize simulation controlling parameters

      /* 3 top gages */
      animation_array[ 0 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 47, 50.0, 180.0, "Dial1/Value" );
      animation_array[ 1 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 63, 30.0, 90.0, "Dial2/Value" );
      animation_array[ 2 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 91, 1.2, 3.5, "Dial3/Value" );

      /* Two small gauges in the middle. */
      animation_array[ 3 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 36, 2.0, 8.0, "Dial4/Value" );
      animation_array[ 4 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 59, 2.0, 8.0, "Dial5/Value" );

      /* Sliders */
      animation_array[ 5 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 75, 0.2, 0.8, "Slider1/ValueY" );
      animation_array[ 6 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 45, 0.2, 0.8, "Slider2/ValueY" );
      animation_array[ 7 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 84, 0.2, 0.8, "Slider3/ValueY" );
      animation_array[ 8 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 65, 0.2, 0.8, "Slider4/ValueY" );
      animation_array[ 9 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 32, 0.2, 0.8, "Slider5/ValueY" );
      animation_array[ 10 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 58, 0.2, 0.8, "Slider6/ValueY" );

      /* Thermometer */
      animation_array[ 11 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 80, -20.0, 105.0, "Thermometer/ValueY" );

      /* 3 knobs to the right of the thermometer */
      animation_array[ 12 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 46, 5.0, 80.0, "Knob1/Value" );
      animation_array[ 13 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 65, 1.0, 9.0, "Knob2/Value" );
      animation_array[ 14 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 38, 1.0, 9.0, "Knob3/Value" );

      /* 2 switches */
      animation_array[ 15 ] =
        new GlgAnimationValue( this, GlgAnimationValue.RANDOM_INT,
                              0, 100, 0.0, 2.99, "Switch1/ValueX" );
      animation_array[ 16 ] =
        new GlgAnimationValue( this, GlgAnimationValue.RANDOM_INT,
                              0, 100, 0.0, 1.99, "Switch2/OnState" );

      /* Lights */
      animation_array[ 17 ] =
        new GlgAnimationValue( this, GlgAnimationValue.RANDOM,
                              0, 100, 0.0, 3.0, "Light1/Value" );
      animation_array[ 18 ] =
        new GlgAnimationValue( this, GlgAnimationValue.RANDOM,
                              0, 100, 0.0, 3.0, "Light2/Value" );
      animation_array[ 19 ] =
        new GlgAnimationValue( this, GlgAnimationValue.RANDOM,
                              0, 100, 0.0, 3.0, "Light3/Value" );

      /* Toggle switches */
      animation_array[ 20 ] =
        new GlgAnimationValue( this, GlgAnimationValue.RANDOM_INT,
                              0, 100, 0.0, 1.99, "Toggle1/OnState" );
      animation_array[ 21 ] =
        new GlgAnimationValue( this, GlgAnimationValue.RANDOM_INT,
                              0, 100, 0.0, 1.99, "Toggle2/OnState" );
      animation_array[ 22 ] =
        new GlgAnimationValue( this, GlgAnimationValue.RANDOM_INT,
                              0, 100, 0.0, 1.99, "Toggle3/OnState" );

      /* Joystick */
      animation_array[ 23 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 23, 0.2, 0.8, "Joystick/ValueX" );
      animation_array[ 24 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 19, 0.2, 0.8, "Joystick/ValueY" );
   }


   //////////////////////////////////////////////////////////////////////////
   // ActionListener method to use the bean as update timer's ActionListener.
   //////////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateControls();
   }

   //////////////////////////////////////////////////////////////////////////
   // Handles user input: in this demo, it's pressing the start and stop 
   // buttons.
   //////////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject vp, GlgObject message_obj )
   {
      String
        origin,
        format,
        action;

      super.InputCallback( vp, message_obj );

      origin = message_obj.GetSResource( "Origin" );
      format = message_obj.GetSResource( "Format" );
      action = message_obj.GetSResource( "Action" );
      // subaction = message_obj.GetSResource( "SubAction" );

      if( action.equals( "ValueChanged" ) )
      {
         // Stop automatic updates to let the user play with controls
         // when a control's value is adjusted with the mouse.
         StopUpdate();
      }
      else if( format.equals( "Button" ) )
      {
         if( !action.equals( "Activate" ) )
           return;
         
         if( origin.equals( "Start" ) )
           StartUpdate();
         else if( origin.equals( "Stop" ) )
           StopUpdate();
      }

      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   public void StopUpdates()
   {
      if( timer != null )
      {
         timer.stop();
         timer = null;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked by the browser to stop the applet
   //////////////////////////////////////////////////////////////////////////
   public void stop()
   {
      StopUpdates();
      super.stop();
   }

   //////////////////////////////////////////////////////////////////////////
   // Inner class for a Runnable interface.
   // Provides an interface between JavaScript and Java: invokes applet's 
   // methods in a synchronous way as required by Swing.
   //////////////////////////////////////////////////////////////////////////
   class GlgBeanRunnable implements Runnable
   {
      GlgControlsDemo bean;
      String request_name;
      int value;

      public GlgBeanRunnable( GlgControlsDemo bean_p, 
                             String request_name_p, int value_p )
      {
         bean = bean_p;
         request_name = request_name_p;
         value = value_p;
      }

      public void run()
      {
         if( request_name.equals( "Start" ) )
           bean.StartUpdate();
         else if( request_name.equals( "Stop" ) )
           bean.StopUpdate();
         else if( request_name.equals( "ToggleAntiAliasing" ) )
           bean.ToggleAntiAliasing();
         else
           PrintToJavaConsole( "Invalid request name: " + 
                              request_name + "\n" );
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Provides an interface between JavaScript and Java: invokes applet's 
   // methods in a synchronous way as required by Swing.
   //////////////////////////////////////////////////////////////////////////
   public void SendRequest( String request_name, int value )
   {
      GlgBeanRunnable runnable = 
        new GlgBeanRunnable( this, request_name, value );

      SwingUtilities.invokeLater( runnable );
   }
}

