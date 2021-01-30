//import java.awt.*;
//import java.util.*;
import java.awt.event.*;
import java.net.*;
import java.lang.reflect.*;
import com.genlogic.*;
import javax.swing.*;

/////////////////////////////////////////////////////////////////////////////
// Loads a drawing and displays it. Prints diagnostic messages about
// Input and Selection events. 
//
// To activate dynamic updates, set UpdateInterval to a positive value
// (in seconds) and place update code in the actionPerformed() method 
// of the timer's action listener.
//
// Command line parameters:
//     Player  drawing_file  [-debug]
/////////////////////////////////////////////////////////////////////////////
public class Player
{
   static GlgObject viewport;
   static String filename = null;
   static boolean debug = false;
   javax.swing.Timer timer = null;   

   // Set to a positive value (in seconds) to enable updates
   double UpdateInterval = -1.;

   public static void main( String arg[] )
   {
      if( Array.getLength( arg ) == 0 || arg[ 0 ] == null )
      {
	 System.out.println( "No drawing file." );
	 System.exit( 1 );
      }
      filename = arg[ 0 ];

      if( Array.getLength( arg ) == 2 &&
	 arg[ 1 ] != null && arg[ 1 ].equals( "-debug" ) )
	debug = true;

      SwingUtilities.
	invokeLater( new Runnable(){ public void run() { Main(); } } );
   }

   public static void Main()
   {
      Player player = new Player();
      player.init();      
   }

   void init()
   {
      GlgObject.Init();

      GlgObject dummy = new GlgDDataValue( 0. );

      //  Set to 1 use Swing components for viewports.
      dummy.SetDResource( "$config/GlgSwingUsage", 1. );

      // Set to 0 to disable anti-aliasing and increase drawing speed
      dummy.SetDResource( "$config/GlgAntiAliasing", 1. );

      GlgObject.SetAlarmHandler( new AlarmHandler() );

      try
      {
	 new URL( filename );

	 // If url is successfully created, filename is a url.
	 if( debug )
	   System.out.println( "Loading drawing URL" );
	 viewport = GlgObject.LoadWidget( filename, GlgObject.URL );
      }
      catch( MalformedURLException e )
      {
	 if( debug )
	   System.out.println( "Loading drawing file" );

	 // Not a valid url, filename is a local file name.
	 viewport = GlgObject.LoadWidget( filename, GlgObject.FILE );
      }

      if( debug )
	System.out.println( "Loaded the drawing" );

      if( viewport == null )
	System.exit( 1 );

      viewport.AddListener( GlgObject.INPUT_CB, new Feedback( this ) );
      viewport.AddListener( GlgObject.SELECT_CB, new Feedback( this ) );

      viewport.InitialDraw();
      
      if( debug )
	System.out.println( "Displayed the drawing" );

      // Starts dynamic updates. Place update code in the actionPerformed() 
      // method of the timer's action listener.
      StartUpdates();
   }

   void StartUpdates()
   {
      if( UpdateInterval >= 0. && timer == null )
      {
	 timer = 
	   new javax.swing.Timer( (int)( UpdateInterval * 1000. ),
				 new UpdateListener( this ) );
	 timer.setRepeats( true );
	 timer.start();
      }
   }

   void StopUpdates()
   {
      if( timer != null )
      {
	 timer.stop();
	 timer = null;
      }
   }
}

class UpdateListener implements ActionListener
{
   Player player;
   
   UpdateListener( Player player_p )
   {
      player = player_p;
   }
   
   /////////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      // Place update code here
   }
}

class Feedback implements GlgInputListener, GlgSelectListener
{
   Player player;

   Feedback( Player player_p )
   {
      player = player_p;
   }

   //////////////////////////////////////////////////////////////////////////
   // Handles user input. Do at least update here.
   //////////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject vp, GlgObject message_obj )
   {
      String
	origin,
	full_origin,
	format,
	action,
	subaction,
	event_label = "",
	name;

      origin = message_obj.GetSResource( "Origin" );
      full_origin = message_obj.GetSResource( "FullOrigin" );
      format = message_obj.GetSResource( "Format" );
      action = message_obj.GetSResource( "Action" );
      subaction = message_obj.GetSResource( "SubAction" );

      // Custom events have EventLabel property.
      if( ( format.equals( "CustomEvent" ) || format.equals( "Command" ) ) &&
          message_obj.GetResourceObject( "EventLabel" ) != null )
        event_label = message_obj.GetSResource( "EventLabel" );

      if( !format.equals( "ObjectSelection" ) )
        System.out.println( "Origin: " + origin + 
                            " FullOrigin: " + full_origin +
                            " Format: " + format + 
                            " Action: " + action + " SubAction: " + subaction +
                            " EventLabel: " + event_label );

      GlgObject object = message_obj.GetResourceObject( "Object" );      
      if( object != null )
      {
	 name = object.GetSResource( "Name" );
	 if( name != null )
	   System.out.println( "Object Name: " + name ); 
      }

      // Handles window closing.
      if( format.equals( "Window" ) && action.equals( "DeleteWindow" ) )
          System.exit( 0 );
      Player.viewport.Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Handles user selection, prints names of the selected objects.
   //////////////////////////////////////////////////////////////////////////
   public void SelectCallback( GlgObject vp, Object[] name_array, int button )
   {
      int i;

      if( name_array != null )
	for( i=0; name_array[ i ] != null; ++i )
	  System.out.println( "Selected object: " + name_array[ i ] );

      // Uncomment the next line to save a PostScript file on a mouse click.
      // vp.Print( "Player.ps", -900., -900., 1800., 1800., true, false );

      // Uncomment next line to save the drawing on a mouse click.
      // vp.SaveObject( "test.g" );
   }
}

////////////////////////////////////////////////////////////////////////
// A sample alarm handler that just prints alarm information.
////////////////////////////////////////////////////////////////////////
class AlarmHandler implements GlgAlarmHandler
{
   public void Alarm( GlgObject data_obj, GlgObject alarm_obj, 
                     String alarm_label, String action, String subaction, 
                     Object reserved )
   {
      System.out.println( "Alarm:" +
                         " obj_name=" + alarm_obj.GetSResource( "Name" ) +
                         " label=" + alarm_label +
                         " action=" + action + " subaction=" + subaction );
   }
}
