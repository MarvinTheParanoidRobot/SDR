import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgFighterJetDemo extends GlgJBean implements ActionListener
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   boolean AutoUpdate = true;
   double TParam = 0.0;       // Trajectory parameter, changing from 0 to 1
                              // moves from the beginning to the end of the
                              // path.
   double UpdateSpeed = 10.0; // Distance the jet moves along the trajectory
                              // on each update.

   Timer timer = null;

   //////////////////////////////////////////////////////////////////////////
   public GlgFighterJetDemo()
   {
      super();
      SetDResource( "$config/GlgAntiAliasing", 1.0 );  // Enable antialiasing
   }

   //////////////////////////////////////////////////////////////////////////
   // Used to initialize the jet before the first draw.
   //////////////////////////////////////////////////////////////////////////
   public void VCallback( GlgObject viewport )
   {
      InitFighterJet();
   }

   //////////////////////////////////////////////////////////////////////////
   // Starts the update timer
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );

      if( timer == null )
      {
         // Restart the timer after each update (instead of using repeats)
         // to avoid flooding the event queue with timer events on slow 
         // machines.
         timer = new Timer( 30, this );
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

      JFrame frame = new JFrame( "GLG F-35 Lightning II Demo" );

      frame.setResizable( true );
      frame.setSize( 700, 600 );
      frame.setLocation( 20, 20 );

      GlgFighterJetDemo fighter_jet = new GlgFighterJetDemo();      
      frame.getContentPane().add( fighter_jet );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );

      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers ReadyCallback which starts updates.
      //
      fighter_jet.SetDrawingName( "F35-Lightning.g" );
   }

   //////////////////////////////////////////////////////////////////////////
   public void UpdateFighterJet()
   { 
      if( timer == null )
        return;   // Prevents race conditions

      if( AutoUpdate )
      {
         // Calculate and set new resource values

         int PERIOD = 500;
         double X_COEFF   = 4.0 * Math.PI;
         double Y_COEFF   = 2.0 * Math.PI;
         double YAW_COEFF = 1.0 * Math.PI;

         double 
           pos_x, pos_y,
           roll, pitch, yaw,
           dx, dy;

         TParam += UpdateSpeed / PERIOD;
         if( TParam > 1.0 )
           TParam -= 1.0;
         
         pos_x = 500.0 * Math.sin( X_COEFF * TParam );
         pos_y = 500.0 * Math.sin( Y_COEFF * TParam );
         
         // Using derivative to determine pitch angle 
         // (dTParam, dropping common 500. coeff.)
         //
         dy = Y_COEFF * Math.cos( Y_COEFF * TParam );
         dx = X_COEFF * Math.cos( X_COEFF * TParam );
         if( dx == 0.0 ) 
           if( dy > 0.0 )
             pitch = -90.0;
           else
             pitch = 90.0;
         else
         {
            pitch = RadToDeg( Math.atan( dy / dx ) );
            if( dx > 0.0 && dy > 0.0 )
              pitch *= -1.0;
            else if( dx < 0.0 )
              pitch = 180.0 - pitch;
            else if( dx > 0.0 && dy < 0.0 )
              pitch *= -1.0;
         }
         
         roll = 2.0 * 360.0 * TParam;
         while( roll > 180.0 )
           roll -= 360.0;
         while( roll < -180.0 )
           roll += 180.0;
         
         yaw = 50.0 * Math.sin( YAW_COEFF * TParam );
         
         SetDResource( "FighterJet/PositionX", pos_x );
         SetDResource( "FighterJet/PositionY", pos_y );
         SetDResource( "FighterJet/PitchValue", pitch );
         SetDResource( "FighterJet/RollValue", roll );
         SetDResource( "FighterJet/YawValue", yaw );

         Update();   // Show changes
      }

      timer.start();   // Restart the update timer
   }

   //////////////////////////////////////////////////////////////////////////
   double RadToDeg( double angle )
   {
      return angle / Math.PI * 180.0;
   }

   //////////////////////////////////////////////////////////////////////////
   // Handle user interaction.
   //////////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject viewport, GlgObject message_obj )
   {
      String
        format,
        origin,
        action;

      format = message_obj.GetSResource( "Format" );
      origin = message_obj.GetSResource( "Origin" );
      action = message_obj.GetSResource( "Action" );

      if( format.equals( "Button" ) )
      {
         if( !action.equals( "ValueChanged" ) )
           return;

         if( origin.equals( "WireFrame" ) )
         {
            // Disable ZSort for the wire frame mode.
            boolean wire_frame = 
              ( message_obj.GetDResource( "OnState" ).doubleValue() != 0.0 );
            double zsort_type = (double)
              ( wire_frame ? GlgObject.ZS_NO : GlgObject.ZS_SPECIAL );
            
            SetDResource( "FighterJet/ZSort", zsort_type );
            SetDResource( "FighterJet/Glass/ZSort", zsort_type );
         }
      }
      else if( format.equals( "Slider" ) )
      {
         if( !action.equals( "ValueChanged" ) )
           return;

         if( origin.equals( "SpeedSlider" ) )
         {
            UpdateSpeed = message_obj.GetDResource( "ValueY" ).doubleValue();
            if( UpdateSpeed < 0.01 )
              UpdateSpeed = 0.0;
            else if( UpdateSpeed > 100.0 )
              UpdateSpeed = 100.0;

            if( AutoUpdate != ( UpdateSpeed != 0.0 ) )
            {
               AutoUpdate = ( UpdateSpeed != 0.0 );
               ChangeAutoMode();
            }
         }
         else if( !origin.equals( "ScaleSlider" ) )
         {
            // Some slider (other then speed or scale sliders) was moved: 
            // stop updates to allow the user to manually control the jet 
            // with sliders.
            if( AutoUpdate )
            {
               AutoUpdate = false;
               ChangeAutoMode();
            }
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void InitFighterJet()
   {
      SetDResource( "FighterJet/PositionX", 0.0 );
      SetDResource( "FighterJet/PositionY", 0.0 );

      SetDResource( "RollSlider/ValueX", 0.0 );
      SetDResource( "PitchSlider/ValueX", -27.0 );
      SetDResource( "YawSlider/ValueX", 0.0 );

      SetDResource( "ScaleSlider/ValueX", 1.0 );
      SetDResource( "SpeedSlider/ValueY", UpdateSpeed );

      // OpenGL note for the Java version of the demo.
      String OpenGLNote = 
        "This demo requires OpenGL for fast and accurate 3D rendering in the Fill Mode.\n" +
        "Use the Binary Version of the demo for accelerated OpenGL rendering.\n";

      SetSResource( "OpenGLWarning/String", OpenGLNote );
      
      // Make the Dog Fight button invisible - is used only in binary demos.
      SetDResource( "DogFightDemo/Visibility", 0.0 );
   }

   //////////////////////////////////////////////////////////////////////////
   void ChangeAutoMode()
   {
      if( !AutoUpdate )
        SetDResource( "SpeedSlider/ValueY", 0.0 );

      // Show a note for the manual mode.
      SetDResource( "ManualMode", AutoUpdate ? 0.0 : 1.0 );
      if( !AutoUpdate )
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
   // ActionListener method to use the bean as update timer's ActionListener.
   //////////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateFighterJet();
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked by the browser to stop the applet
   //////////////////////////////////////////////////////////////////////////
   public void stop()
   {
      StopUpdates();
      super.stop();
   }
}

