import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgAvionicsDemo extends GlgJBean implements ActionListener
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   static final int NUM_VALUES = 15;
   boolean PerformUpdates = true;
   GlgAnimationValue [] animation_array = new GlgAnimationValue[ NUM_VALUES ];
   static boolean AntiAliasing = true;

   Timer timer = null;

   //////////////////////////////////////////////////////////////////////////
   public GlgAvionicsDemo()
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
         timer = new Timer( 50, this );
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

      JFrame frame = new JFrame( "GLG Avionics Dashboard Demo" );

      frame.setResizable( true );
      frame.setSize( 750, 600 );
      frame.setLocation( 20, 20 );

      GlgAvionicsDemo avionics = new GlgAvionicsDemo();
      frame.getContentPane().add( avionics );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );

      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers ReadyCallback which starts updates.
      //
      avionics.SetDrawingName( "avionics.g" );
   }

   //////////////////////////////////////////////////////////////////////////
   public void UpdateAvionics()
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
   public void StopUpdates()
   {
      if( timer != null )
      {
         timer.stop();
         timer = null;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   public void Start()
   {
      PerformUpdates = true;
      if( timer != null )
        timer.start();
   }

   //////////////////////////////////////////////////////////////////////////
   public void Stop()
   {      
      PerformUpdates = false;
      if( timer != null )
        timer.stop();
   }

   //////////////////////////////////////////////////////////////////////////
   public void ToggleAntiAliasing()
   {
      AntiAliasing = !AntiAliasing;
      SetDResource( "$config/GlgAntiAliasing", AntiAliasing ? 1.0 : 0.0 );

      // Restart with new AntiAliasing setting
      stop();
      start();
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
   // ActionListener method to use the bean as update timer's ActionListener.
   //////////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateAvionics();
   }


   //////////////////////////////////////////////////////////////////////////
   void InitializeArrays()
   {
      int k = 5;   // Speed factor

      // Initilize simulation controlling parameters
      animation_array[ 0 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 1000 / k, 20.0, 80.0, "RPM/Value" );
      animation_array[ 1 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 1100 / k, 30.0, 85.0, "RPM/Value2" );
      animation_array[ 2 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 1400 / k, 3.5, 7.5, "EGT/Value" );
      animation_array[ 3 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 1500 / k, 4.5, 7.8, "EGT/Value2" );
      animation_array[ 4 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 2500 / k, 2000.0, 8000.0, "FUEL/Value" );
      animation_array[ 5 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 2200 / k, 0.6, 1.0, "MACH/Value" );
      animation_array[ 6 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 2300 / k, 0.7, 1.2, "MACH/Value2" );
      animation_array[ 7 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 2100 / k, 0.0, 20.0, "ADA/Value" );
      animation_array[ 8 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 1600 / k, 0.0, 1.0, "NOZ/Value" );
      animation_array[ 9 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 1700 / k, 0.0, 1.0, "NOZ/Value2" );
      animation_array[ 10 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 2600 / k, -10.0, 10.0, "HORIZON/Pitch" );
      animation_array[ 11 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 2700 / k, -10.0, 10.0, "HORIZON/Roll" );
      animation_array[ 12 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 2700 / k, -20.0, 20.0, "HORIZON/LeftRudder" );
      animation_array[ 13 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 1900 / k, -10.0, 10.0, "HORIZON/RightRudder" );
      animation_array[ 14 ] =
        new GlgAnimationValue( this, GlgAnimationValue.SIN,
                              0, 2800 / k, 30.0, 160.0, "COMPASS/Value" );
   }

   //////////////////////////////////////////////////////////////////////////
   // Inner class for a Runnable interface.
   // Provides an interface between JavaScript and Java: invokes applet's 
   // methods in a synchronous way as required by Swing.
   //////////////////////////////////////////////////////////////////////////
   class GlgBeanRunnable implements Runnable
   {
      GlgAvionicsDemo bean;
      String request_name;
      int value;

      public GlgBeanRunnable( GlgAvionicsDemo bean_p, 
                             String request_name_p, int value_p )
      {
         bean = bean_p;
         request_name = request_name_p;
         value = value_p;
      }

      public void run()
      {
         if( request_name.equals( "Start" ) )
           bean.Start();
         else if( request_name.equals( "Stop" ) )
           bean.Stop();
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

