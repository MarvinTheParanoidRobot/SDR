/*************************************************************************
**************************************************************************/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

///////////////////////////////////////////////////////////////////////
public class GlgActionsExample extends JApplet 
{ 
   static final long serialVersionUID = 0;

   GlgJBean glg_bean;   
   boolean IsReady = false;

   GlgObject DrawingArea;
   GlgObject PopupDialog;

   ////////////////////////////////////////////////////////////////////
   // Constructor, where Glg bean components are created and added to a 
   // native Java container, an Applet in this case.
   ////////////////////////////////////////////////////////////////////
   public GlgActionsExample()
   {
      getContentPane().setLayout( new GridLayout( 1, 1 ) );

      glg_bean = new GlgJBean();   
      getContentPane().add( glg_bean );

      // Add InputListener to the GLG bean to handle user interaction.
      glg_bean.AddListener( GlgObject.INPUT_CB, new InputListener() );

      // Add Ready Listener to the GLG bean. It is invoked after
      // the drawing is initially drawn.
      glg_bean.AddListener( GlgObject.READY_CB, new ReadyListener() );

      // Add HListener to the GLG. It is invoked before hierarchy setup
      // and before the drawing is drawn for the first time.
      glg_bean.AddListener( GlgObject.H_CB, new HListener() );
   }

   //////////////////////////////////////////////////////////////////
   //Invoked by a browser to start the applet.
   //////////////////////////////////////////////////////////////////
   public void start()
   {
      super.start();

      // Set a GLG drawing name to be displayed in the GLG bean, 
      // "top_level.g" in this case. The drawing name is relative to 
      // the applet's document base. Parent applet should be passed 
      // to a GLG bean by calling SetParentApplet() method so that 
      // the bean knows about the current document base.
      //
      glg_bean.SetParentApplet( this ); 
      glg_bean.SetDrawingName( "top_level.g" );
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

   ////////////////////////////////////////////////////////////////////
   class ReadyListener implements GlgReadyListener
   {
      public void ReadyCallback( GlgObject viewport )
      {
         IsReady=true;
      }
   }

   ////////////////////////////////////////////////////////////////////
   class HListener implements GlgHListener
   {
      public void HCallback( GlgObject viewport )
      {
         // Obtain an object ID of the DrawingArea object, which is a
         // SubWindow object in this example.
         DrawingArea = viewport.GetResourceObject( "DrawingArea" );

         // Obtain object ID of the PopupDialog and make it invisible.
         PopupDialog = viewport.GetResourceObject( "PopupDialog" );
         ClosePopup();
      }
   }

   ////////////////////////////////////////////////////////////////////
   // Define a class InputListener which implements GlgInputListener 
   // interface. This class should provide an InputCalllback() method 
   // which will be invoked when an input event occurred in a Glg 
   // drawing, such as clicking on a control widget or selecting an 
   // object with event properties attached to it.
   ////////////////////////////////////////////////////////////////////
   class InputListener implements GlgInputListener
   {
     public void InputCallback( GlgObject viewport, GlgObject message_obj )
     {
        String tag_source;
        final String target;

        GlgObject 
           selected_obj,   // selected object
           action_res_obj; // object id of OEMAction resource
        
        double value;
        
        String format = message_obj.GetSResource( "Format" );
        String action = message_obj.GetSResource( "Action" );
        String origin = message_obj.GetSResource( "Origin" );

        // Handle window closing. May use viewport's name.
        if( format.equals( "Window" ) &&
            action.equals( "DeleteWindow" ) )
        {
           /* Close PopupDialog */
           if( origin.equals( "PopupDialog" ) )
              ClosePopup();
           else
              // Exit application
              System.exit( 0 );
        }
        
        // Handle object selection events for objects with 
         // Custom MouseClick event
        if( format.equals( "CustomEvent" ) )
        {
           // Retrieve EventLabel
           String event_label = message_obj.GetSResource( "EventLabel" );

           // Prevent recursive GlgUpdate while destroying the drawing in 
           // GoToView action.
           if( event_label.equals("") )
              return;
    
           if( action.equals( "MouseClick" ) )
           {
              if( event_label.equals( "OEMActionEvent" ) )
              {
                 // Handle actions attached to the selected object
                 selected_obj = message_obj.GetResourceObject( "Object" );
                 if( !selected_obj.HasResourceObject( "OEMAction" ) )
                    return;
                 
                 action_res_obj = 
                    selected_obj.GetResourceObject( "OEMAction" );
                 int action_type = 
                    action_res_obj.GetDResource( "ActionType" ).intValue();
                 switch( action_type )
                 {
                  default:
                  case 0: // Undefined action
                     break;
                  case 1: // WriteValue
                     tag_source = action_res_obj.GetSResource( "Tag" );
                     value = 
                        action_res_obj.GetDResource( "Value" ).doubleValue();
                     WriteTagValue( tag_source, value );
                     break;
                  case 2: // WriteCurrent value
                     tag_source = action_res_obj.GetSResource( "Tag" );
                     value = viewport.GetDTag( tag_source ).doubleValue();
                     WriteTagValue( tag_source, value );
                     break;
                  case 3: // Popup
                     target = action_res_obj.GetSResource( "Target" );
                     DisplayPopup( target ); 
                     break;
                  case 4: // GOTO view
                      target = action_res_obj.GetSResource( "Target" );
                      SwingUtilities.invokeLater( new Runnable()
                             { public void run() { GoToView( target ); } } );
                      break;
                 }
              } 
           }
        }
        // Handle events from a toggle button.
        else if( format.equals( "Button" ) &&
                 action.equals( "ValueChanged" ) )
        {
           double button_value = 
              message_obj.GetDResource( "OnState" ).doubleValue();
           
           // If OnState resource of the button has a tag object, 
           // the value of that
           // tag will correspond to the new value of OnState resource.
           // If there are other tag objects in the drawing that have the same
           // TagSource as the button, we need to synchronize these tags 
           // with the new value from the button. 
           selected_obj = message_obj.GetResourceObject( "Object" );
           tag_source = selected_obj.GetSResource( "OnState/TagSource" );
           
           // Synchronize all tags that have the same TagSource as the
           // toggle button which generated the event.
           if( tag_source != null && !tag_source.equals("") 
               && !tag_source.equals( "unset" ) )
              viewport.SetDTag( tag_source, button_value, true );
        }
        // Handle events from a push button.
        else if( format.equals( "Button" ) &&
                 action.equals( "Activate" ) )
        {
           if( origin.equals( "PopupOKButton" ) )
              // Close PopupDialog if the dialog's OK button was pressed.
              ClosePopup();
           else if( origin.equals( "MainButton" ) )
              // Display main_drawing.g in the DrawingArea
              viewport.SetSResource( "DrawingArea/SourcePath", 
                                     "main_drawing.g" );
           else if( origin.equals( "QuitButton" ) )
              /* Exit application */
              System.exit( 0 );
        }

        viewport.Update();
     }
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Functions to be executed for custom actions
   //////////////////////////////////////////////////////////////////////////
   public void WriteTagValue( String tag_source, double value )
   {
      System.out.println( "Write action, tag_source=" + tag_source +
                          " value=" + value );
      
      if( tag_source == null )
         return;
      
      /* Comment out the following line and add code to write tag value 
         to the database */
      glg_bean.SetDTag( tag_source, value, true );
   }
   
   public void DisplayPopup( String target )
   {
      System.out.println( "Popup action, target=" + target );
      PopupDialog.SetSResource( "DrawingArea/SourcePath", target );
      PopupDialog.SetSResource( "ScreenName", target );
      PopupDialog.SetDResource( "Visibility", 1.0 );
   }
   
   public void ClosePopup()
   {
      PopupDialog.SetDResource( "Visibility", 0.0 );
   }

   public void GoToView( String drawing_name )
   {
      System.out.println( "GoTo action, drawing_name = " + drawing_name );
      DrawingArea.SetSResource( "SourcePath", drawing_name );

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
      frame.setSize( 700, 700 );
      frame.addWindowListener( new DemoQuit() );
       
      GlgActionsExample oem_action_example = new GlgActionsExample();   
      frame.getContentPane().add( oem_action_example );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      //
      oem_action_example.glg_bean.SetDrawingName( "top_level.g" );   
   }
}
