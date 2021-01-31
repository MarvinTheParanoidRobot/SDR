import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgAnimationDemo extends GlgJBean implements ActionListener
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   // Animation parameters.
   static final double DELTA = ( 2.0 * Math.PI / 500.0 );
   static final double XIncrement = DELTA * 8.0;
   static final double YIncrement = DELTA * 7.0;
   
   // Static variables used by GetPosition() method that generates
   // animation values. Values change in range 0-2*PI.
   double   
      XValue = 0.0,
      YValue = 0.0;    

   // Time interval for periodic dynamic updates, in millisec.
   int TimeInterval = 20; 

   double Radius;      // Moving ball's radius
   double
        XMin, XMax,    // Ball movement's area
        YMin, YMax;

   Timer timer = null;

   //////////////////////////////////////////////////////////////////////////
   public GlgAnimationDemo()
   {
      super();
   }

   //////////////////////////////////////////////////////////////////////////
   // Starts updates
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );

      // Query the ball's radius
      Radius = GetDResource( "CatchMe/Radius" );

      // Query the extent of the ball's movement area
      GlgPoint point = viewport.GetGResource( "Area/LLPoint" );
      XMin = point.x;
      YMin = point.y;

      point = viewport.GetGResource( "Area/URPoint" );
      XMax = point.x;
      YMax = point.y;

      if( timer == null )
      {
         // Restart the timer after each update (instead of using repeats)
         // to avoid flooding the event queue with timer events on slow 
         // machines.
         timer = new Timer( TimeInterval, this );
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

      JFrame frame = new JFrame( "GLG Animation Demo" );

      frame.setResizable( true );
      frame.setSize( 600, 600 );
      frame.setLocation( 20, 20 );

      GlgAnimationDemo animation = new GlgAnimationDemo();      
      frame.getContentPane().add( animation );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );

      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers ReadyCallback which starts updates.
      //
      animation.SetDrawingName( "animation.g" );
   }

   //////////////////////////////////////////////////////////////////////////
   public void UpdateAnimation()
   { 
      if( timer == null )
        return;   // Prevents race conditions

      // Calculate new coordinates of the ball
      GlgPoint position = GetNewPosition();

      // Set coordinates of the ball in the drawing
      SetDResource( "CatchMe/XValue", position.x );
      SetDResource( "CatchMe/YValue", position.y );

      Update();    // Make changes visible.

      timer.start();   // Restart the update timer
   }

   //////////////////////////////////////////////////////////////////////////
   // This callback is invoked when user selects some object in the drawing
   // with the mouse. In this program, it's used to change the color of the 
   // moving object (named "CatchMe") when the user selects it with the mouse.
   //////////////////////////////////////////////////////////////////////////
   public void SelectCallback( GlgObject viewport, Object[] name_array, int button )
   {
      String name; 

      super.SelectCallback( viewport, name_array, button );

      if( name_array != null )
        for( int i=0; ( name = (String) name_array[i] ) != null; ++i )
        {
           if( name.equals( "Faster" ) || name.equals( "Slower" ) )
             return;   // Ignore buttons selection
           else if( name.equals( "CatchMe" ) )
           {
              // Toggle the ball's color to indicate a hit.
              int color_index = 
                viewport.GetDResource( "CatchMe/ColorIndex" ).intValue();
              viewport.SetDResource( "CatchMe/ColorIndex", 
                                    color_index == 0 ? 1.0 : 0.0 );
              viewport.Update();
              return;
           }
        }	  

      GlgObject.Bell();   // Missed: beep'em up!
   }

   //////////////////////////////////////////////////////////////////////////
   // This callback is invoked when user interacts with input objects in GLG
   // drawing. In this program, it is used to increase or decrease the 
   // animation speed.
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

      if( format.equals( "Button" ) && action.equals( "Activate" ) )
      {	 
         // Act based on the selected button.
         if( origin.equals( "Faster" ) )
         {  
            // Increase animation speed
            if( TimeInterval > 10 )
              TimeInterval /= 2;
         }
         else if( origin.equals( "Slower" ) )
         {
            // Decrease animation speed
            if( TimeInterval < 5000 )
               TimeInterval *= 2;
         }

         // Restart the timer after setting its delay to avoid waiting 
         // for an expiration of a (possibly long) prevous timer interval.
         if( timer != null )
         {
            timer.stop();
            timer.setInitialDelay( TimeInterval );
            timer.setDelay( TimeInterval );
            timer.restart();
         }
      }
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
      UpdateAnimation();
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
   // Simulation: calculates the new position of the ball.
   //////////////////////////////////////////////////////////////////////////
   public GlgPoint GetNewPosition()
   {
      GlgPoint point = new GlgPoint();
 
      double
         x_center,
         y_center,
         x_amplitude,
         y_amplitude;
   
      // Increase x value counter
      XValue += XIncrement;
      if( XValue > 2.0 * Math.PI )
         XValue -= 2.0 * Math.PI;

      // Increase y value counter
      YValue += YIncrement;
      if( YValue > 2.0 * Math.PI )
         YValue -= 2.0 * Math.PI;

      // Find the center of the ball's movement area
      x_center = ( XMax + XMin ) / 2.0;
      y_center = ( YMax + YMin ) / 2.0;

      // The extent of the ball's movements
      x_amplitude = ( XMax - XMin ) / 2.0 - Radius;
      y_amplitude = ( YMax - YMin ) / 2.0 - Radius;
   
      // Calculate the ball's current position */
      point.x = x_center + x_amplitude * Math.sin( XValue );
      point.y = y_center + y_amplitude * Math.cos( YValue );

      return point;
  }
}
