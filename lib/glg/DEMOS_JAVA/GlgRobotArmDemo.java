import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgRobotArmDemo extends GlgJBean implements ActionListener
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   static final int NUM_ELBOWS = 7;
   boolean
     WireFrame = false,
     AutoUpdate = true;
   GlgAnimationValue [] elbow_array = new GlgAnimationValue[ NUM_ELBOWS ];

   Timer timer = null;

   //////////////////////////////////////////////////////////////////////////
   public GlgRobotArmDemo()
   {
      super();
      SetDResource( "$config/GlgAntiAliasing", 1.0 );  // Enable antialiasing
   }

   //////////////////////////////////////////////////////////////////////////
   // Starts the update timer
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );

      InitializeArrays();
      
      SetWireFrame( false );

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

      JFrame frame = new JFrame( "GLG Robot Arm Demo" );

      frame.setResizable( true );
      frame.setSize( 600, 500 );
      frame.setLocation( 20, 20 );

      GlgRobotArmDemo robot_arm = new GlgRobotArmDemo();      
      frame.getContentPane().add( robot_arm );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );

      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers ReadyCallback which starts updates.
      //
      robot_arm.SetDrawingName( "robot_arm.g" );
   }

   //////////////////////////////////////////////////////////////////////////
   public void UpdateRobotArm()
   { 
      if( timer == null )
        return;   // Prevents race conditions

      if( AutoUpdate )
      {
         // Calculate and set new resource values
         // Different elbows of the arm are named "s0" - "s6" in the drawing.
         // The controlling parameters of elbows' rotation transformations
         // are named "Value". 

         // Update all seven elbows of the robot arm
         for( int i=0; i < NUM_ELBOWS; ++i )
           elbow_array[ i ].Iterate();

         Update();   // Show changes
      }

      timer.start();   // Restart the update timer
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

      if( format.equals( "Button" ) )         /* Handle button clicks */
      {
         if( !action.equals( "Activate" ) &&      /* Not a push button */
             !action.equals( "ValueChanged" ) )   /* Not a toggle button */
           return;

         if( origin.equals( "Updates" ) )
         {
            AutoUpdate = 
              ( GetDResource( message_obj, "OnState" ) == 0.0 ? false : true );
         }
         else if( origin.equals( "WireFrame" ) )
         {
            SetWireFrame( GetDResource( message_obj, "OnState" ) == 0.0 ? 
                          false : true );
         }
      } 
      else if( format.equals( "Slider" ) && action.equals( "ValueChanged" ) )
      {
         // Slider was moved: stop updates to allow the user to control
         // the arm with sliders.
         AutoUpdate = false;
         viewport.SetDResource( "Updates/OnState", 0.0 );
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void SetWireFrame( boolean wireframe )
   {
      WireFrame = wireframe;
      SetDResource( "robot_area/robot_arm/ZSort", wireframe ? 0.0 : 1.0 );
      SetDResource( "robot_area/fill_type", (double)
                    ( wireframe ? GlgObject.EDGE : GlgObject.FILL_EDGE ) );

      // Update toggle with the new value if needed.
      SetDResource( "WireFrame/OnState", wireframe ? 1.0 : 0.0, false );
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   public void StartUpdate()
   {
      AutoUpdate = true;      
      SetDResource( "Updates/OnState", 1.0 );    // Update the updates toggle
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   public void StopUpdate()
   {
      AutoUpdate = false;
      SetDResource( "Updates/OnState", 0.0 );    // Update the updates toggle
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   public void ToggleWireFrame()
   {
      SetWireFrame( !WireFrame );
   }

   //////////////////////////////////////////////////////////////////////////
   void InitializeArrays()
   {
      // Initilize simulation controlling parameters
      elbow_array[ 0 ] =
         new GlgAnimationValue( this, GlgAnimationValue.SIN,
                                0,  70, -0.75, 0.75, "s0/Value" );
      elbow_array[ 1 ] =
         new GlgAnimationValue( this, GlgAnimationValue.SIN,
                                0,  60,  -0.75, 0.75, "s1/Value" );
      elbow_array[ 2 ] =
         new GlgAnimationValue( this, GlgAnimationValue.SIN,
                                0, 200,  0.0,   1.0,   "s2/Value" );
      elbow_array[ 3 ] =
         new GlgAnimationValue( this, GlgAnimationValue.SIN,
                                0, 150,  0.0,   1.0,   "s3/Value" );
      elbow_array[ 4 ] =
         new GlgAnimationValue( this, GlgAnimationValue.SIN,
                                0,  30,  0.0,   1.0,   "s4/Value" );
      elbow_array[ 5 ] =
         new GlgAnimationValue( this, GlgAnimationValue.SIN,
                                0, 100,  0.0,   1.0,   "s5/Value" );
      elbow_array[ 6 ] =
         new GlgAnimationValue( this, GlgAnimationValue.SIN,
                                0,  20,  0.0,   1.0,   "s6/Value" );
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
      UpdateRobotArm();
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
      GlgRobotArmDemo bean;
      String request_name;
      int value;

      public GlgBeanRunnable( GlgRobotArmDemo bean_p, 
                             String request_name_p, int value_p )
      {
         bean = bean_p;
         request_name = request_name_p;
         value = value_p;
      }

      public void run()
      {
         if( request_name.equals( "StartUpdate" ) )
           bean.StartUpdate();
         else if( request_name.equals( "StopUpdate" ) )
           bean.StopUpdate();
         else if( request_name.equals( "ToggleWireFrame" ) )
           bean.ToggleWireFrame();
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

