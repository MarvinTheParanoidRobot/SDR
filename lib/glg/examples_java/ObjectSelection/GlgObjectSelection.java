import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

///////////////////////////////////////////////////////////////////////
public class GlgObjectSelection extends JApplet 
{ 
   static final long serialVersionUID = 0;

   // GLG bean.
   GlgJLWBean glg_bean;   

   /* Store information for the object selected with the mouse, used to
      process selection events on MouseRelease instead of MouseClick.  
   */
   GlgObject SelectedObject = null;
   String SelectedEventLabel = null;
   
   /* Flag indicating whether to process obeject selection on MouseRelease
      or MouseClick. In this example, the flag is set based
      on the resource "OnRelease" defined in the object at design time.
   */
   boolean ProcessOnReleaseFlag = false;
      
   // Set to true to print debugging information.
   boolean DEBUG = false;

   ////////////////////////////////////////////////////////////////////
   // Constructor, where Glg bean components are created and added to a 
   // native Java container, an Applet in this case.
   ////////////////////////////////////////////////////////////////////
   public GlgObjectSelection()
   {
      getContentPane().setLayout( new GridLayout( 1, 1 ) );

      glg_bean = new GlgJLWBean();   
      getContentPane().add( glg_bean );

      // Add InputListener to handle user interaction.
      glg_bean.AddListener( GlgObject.INPUT_CB, new InputListener() );

      /* Add H and V Listeners to the Glg bean, used to initialize the
         drawing. HListener is invoked before hierarchy setup, and 
         VListener is invoked after hierarchy setup.
      */
      glg_bean.AddListener( GlgObject.H_CB, new HListener() );
      glg_bean.AddListener( GlgObject.V_CB, new VListener() );

      // Set pick resolution for object selection to 1 pixel
      glg_bean.SetDResource( "$config/GlgPickResolution", 1. );
   }

   //////////////////////////////////////////////////////////////////
   //Invoked by a browser to start the applet
   //////////////////////////////////////////////////////////////////
   public void start()
   {
      super.start();

      /* Set a GLG drawing name to be displayed in the GLG bean, 
         "obj_selection.g" in this case. The drawing name is relative to 
         the applet's document base. Parent applet should be passed 
         to a GLG bean by calling SetParentApplet() method so that 
         the bean knows about the current document base.
      */
      glg_bean.SetParentApplet( this ); 
      LoadDrawing( "obj_selection.g" );
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
   class HListener implements GlgHListener
   {
      public void HCallback( GlgObject viewport )
      {
         // Place custom code for drawing initialization.
      }
   }
   
   ////////////////////////////////////////////////////////////////////
   class VListener implements GlgVListener
   {
      public void VCallback( GlgObject viewport )
      {
         /* Set ProcessMouse property to receive MouseClick, MouseOver and Tooltip
            events in the Input callback. This property may be set in the drawing
            as well.
         */
         viewport.SetDResource( "ProcessMouse", (double) 
                                ( GlgObject.MOUSE_OVER_SELECTION |
                                  GlgObject.MOUSE_OVER_SELECTION | 
                                  GlgObject.MOUSE_OVER_TOOLTIP | 
                                  GlgObject.MOUSE_CLICK ) );
         
         // Initialize the comment string, if found.
         if( viewport.HasResourceObject( "Comment" ) )
           viewport.SetSResource( "Comment/String", "" );
         
         // Place additional custom code as needed.
      }
   }
   
   ////////////////////////////////////////////////////////////////////
   public void LoadDrawing( String filename )
   {
      glg_bean.SetDrawingName( filename );
   }

   ////////////////////////////////////////////////////////////////////
   public void ProcessCommandGoTo( GlgObject command_obj )
   {
      if( command_obj == null )
        return;

      /* Retrieve the drawing file from the command. */
      String drawing_file = command_obj.GetSResource( "DrawingFile" );
      
      /* If DrawingFile is not valid, abort the command. */
      if( drawing_file == null || drawing_file.isEmpty() )
      {
         System.out.println( "Invalid DrawingFile, GoTo Command failed." );
         return;
      }
      
      LoadDrawing( drawing_file );
   }
   
   ////////////////////////////////////////////////////////////////////
   // Handle  Handle input events.
   ////////////////////////////////////////////////////////////////////
   class InputListener implements GlgInputListener
   {
     public void InputCallback( GlgObject viewport, GlgObject message_obj )
     {

        String 
          obj_name,
          event_label;
        
        GlgObject 
          selected_obj,
          action_obj,
          command_obj;
        
        String format = message_obj.GetSResource( "Format" );
        String action = message_obj.GetSResource( "Action" );

        /* Process object selection via Actions attached to an object in
           the GLG Builder (at design time). Actions may be of type Command
           (ActionType=SEND COMMAND) or CustomEvent (ActioType=SEND EVENT).
        */
        
        if( format.equals( "Command" ) || format.equals( "CustomEvent" ) )
        {
           /* Retrieve ActionObject */
           action_obj = message_obj.GetResourceObject( "ActionObject" ); 
           
           /* Retrieve selected object. */
           selected_obj = message_obj.GetResourceObject( "Object" );
           
           /* Extract EventLabel */
           event_label = message_obj.GetSResource( "EventLabel" );
           
           if( DEBUG ) /* print selected_obj and event_label */
             System.out.println( "selected_obj = " + selected_obj + 
                                " event_label = " + event_label );
           
           /* Process Actions with ActionType = SEND COMMAND */      
           if( format.equals( "Command" ) )
           {
              /* Retrieve Command object. */
              command_obj = action_obj.GetResourceObject( "Command" );
              if( command_obj == null  )
                return;
              
              /* Process command */
              ProcessCommand( viewport, selected_obj, command_obj, event_label );
              
              /* Update comment string in the drawing. */
              UpdateComment( viewport, selected_obj, 
                             "Processing Command\nObject Name: " );
              
              /* Refresh the display. */
              viewport.Update();
           }
           
           /* Process Actions with ActionType = SEND EVENT */ 
           else if( format.equals( "CustomEvent" ) )
           {
              if( action.equals( "MouseClick" ) )
              {
                 /* Determine if the event should be processed on MouseRelease
                    or MouseClick.
                 */
                 ProcessOnReleaseFlag = ProcessOnRelease( selected_obj, action_obj );
                 if( !ProcessOnReleaseFlag  )
                 {
                    /* Process selection on MouseClick */
                    ProcessSelectionEvent( viewport, selected_obj, event_label );  
                    
                    /* Update comment string in the drawing. */
                    UpdateComment( viewport, selected_obj, 
                           "Processing Custom Event on MouseClick\nObject Name: " );
                 }
                 
                 /* Process selection event on MouseRelease. */
                 else 
                 {
                    /* On mouse click, store selected object and EventLabel. 
                       The event will be processed on MouseRelease, using the
                       stored object and event label.
                    */
                    SelectedObject = selected_obj;
                    SelectedEventLabel = event_label;
                    
                    if( DEBUG )
                    {
                       System.out.println( "Process selection on MouseRelease" );
                       System.out.println( 
                                      "Stored SelectedObject = " + SelectedObject + 
                                      " SelectedEventLabel = " + SelectedEventLabel );
                    }
                 }
              }
              else if( action.equals( "MouseOver" ) && ProcessOnReleaseFlag )
              {
                 /* Mouse may be moved outside of the object, or moved to 
                    another (intersecting) object, in which case reset 
                    SelectedObject and do not process the event.
                 */
                 if( SelectedObject != null && SelectedObject != selected_obj )
                 {
                    ResetSelectedObject();
                    ProcessOnReleaseFlag = false;
                    
                    /* Update comment string in the drawing. */
                    UpdateComment( viewport, SelectedObject, 
                     "Custom Event not processed,\nmouse is moved away from object " );
                    return;
                 }
              }
              else if( action.equals( "MouseRelease" ) && ProcessOnReleaseFlag )
              {
                 if( SelectedObject == null )  /* nothing to do */
                   return;
                 
                 if( DEBUG )
                   System.out.println( "MouseRelease, SelectedObject = " + 
                                       SelectedObject );
                 
                 /* Ready to execute selection event: MouseRelease occurred 
                    inside the object.
                 */
                 ProcessSelectionEvent( viewport, SelectedObject, SelectedEventLabel ); 
                 
                 /* Update comment string in the drawing. */
                 UpdateComment( viewport, SelectedObject, 
                        "Processing Custom Event on MouseRelease\nObject Name: " ); 
                 
                 /* Reset SelectedObject and SelectedEventLabel. */
                 ResetSelectedObject();
                 ProcessOnReleaseFlag = false;
              }
              
              viewport.Update();    /* Make changes visible. */
           } /* format=CustomEvent */
        }
        else if( format.equals( "Tooltip" ) && action.equals( "ObjectTooltip" ) )
        {
           /* Extract TooltipString. It may be obtained either as 
              EventLabel from message_obj, or as "Tooltip" resource 
              from the ActionObject for tooltips added in GLG v.3.5 and later.
           */
           String tooltip_str = message_obj.GetSResource( "EventLabel" );
           if( DEBUG )
             System.out.println( "Tooltip String from EventLabel = " + 
                                tooltip_str );
           
           action_obj = message_obj.GetResourceObject( "ActionObject" );
           if( action_obj != null )
           {
              tooltip_str = action_obj.GetSResource( "Tooltip" );
              if( DEBUG )
                System.out.println( "Tooltip String from ActionObject = " + 
                                    tooltip_str );
           }
           
           /* Retrieve selected object. */
           selected_obj = message_obj.GetResourceObject( "Object" );
           
           String comment;
           if( selected_obj != null )
             comment = "Tooltip displayed for object ";
           else
             comment = "";
           
           /* Update the comment string in the drawing. */
           UpdateComment( viewport, selected_obj, comment );
           viewport.Update();
        }
        
        /* Handle object selection on MouseClick for top level
           objects without Actions. 
        */
        else if( format.equals( "ObjectSelection" ) && 
                 action.equals( "MouseClick" ) )
        {
           /* Retrieve mouse button index and process selection as needed. */
           int button_index = 
             message_obj.GetDResource( "ButtonIndex" ).intValue();
           
           switch( button_index )
           {
            case 1:
            case 3:
              /* Retrieve SelectionArray resource containing an array of object
                 IDs potentially selected with the mouse */
              GlgObject selection_array = 
                message_obj.GetResourceObject( "SelectionArray" );
              
              if( selection_array == null )
                return;
              
              /* Traverse an array of selected objects and print their 
                 names (for the demonstration purposes). */
              for( int i=0; i < selection_array.GetSize(); ++i )
              {
                 selected_obj = (GlgObject) selection_array.GetElement( i );
                 obj_name = selected_obj.GetSResource( "Name" );
                 
                 if( DEBUG )
                   /* Print selected object name */
                   System.out.println( 
                      "ObjectSelection event; selected object name: " + obj_name );
                 
              }
              
              /* Obtain object ID of the top most object (widget) which is
                 a direct child of the top level viewport.
              */
              
              /* Obtain the first selected object (the object drawn on top). */
              selected_obj = (GlgObject) selection_array.GetElement( 0 );
              
              /* Obtain an ID of the object's parent which is a direct child 
                 of the given viewport.
              */
              GlgObject widget = GetDirectChild( viewport, selected_obj );
              if( widget != null )
              {
                 /* In this example, process generic selection events only
                    for widgets with no Actions.
                 */
                 if( widget.HasResourceObject( "Actions" ) )
                   return;
                 
                 /* Place custom code here to process selection. */
                 
                 /* Print widget name (ford emo purposes). */
                 obj_name = widget.GetSResource( "Name" );
                 System.out.println( 
                           "Processing generic ObjectSelection for object Name=" + 
                           obj_name ); 
                 
                 /* Update the comment string in the drawing. */
                 UpdateComment( viewport, widget, "Selected Widget Name: " ); 
              }
              break;
              
            default: return;
           }
           
           viewport.Update();
        } /* format="ObjectSelection" */
     } 
   }

   ///////////////////////////////////////////////////////////////////////////
   // Reset previously stored SelectedObject and SelectedEventLabel.
   ///////////////////////////////////////////////////////////////////////////
   void ResetSelectedObject()
   {
      if( DEBUG )
        System.out.println( "ResetSelectedObject" );
      
      SelectedObject = null;
      SelectedEventLabel = null;
   }
   
   ///////////////////////////////////////////////////////////////////////////
   // Process commands attached to an object via Actions with
   // ActionType = "SEND COMMAND".
   ///////////////////////////////////////////////////////////////////////////
   void ProcessCommand( GlgObject command_vp, GlgObject selected_obj, 
                        GlgObject command_obj, String event_label )
   {
      String tag_source;
      double value;
      
      if( command_obj == null || selected_obj == null )
        return;
      
      String command_type = command_obj.GetSResource( "CommandType" );
      
      /* Process command with CommandType = "WriteValue" */
      if( command_type.equals( "WriteValue" ) )
      {
         /* Retrieve output tag source (output data variable) from the command. */
         tag_source = 
           command_obj.GetSResource( "OutputTagHolder/TagSource" );
         
         /* Validate. */
         if( tag_source == null || tag_source.isEmpty() || 
             tag_source.equals( "unset" ) )
         {
            System.out.println( "Invalid TagSource. WriteValue Command failed." );
            return;
         }
         
         /* Retrieve the value to be written to the specified tag source. */
         value = command_obj.GetDResource( "Value" ).doubleValue();
         
         /* Place custom code here to write new value to the specified 
            tag source. For demo purposes, set the value of the specified 
            tag in the drawing.
         */
         glg_bean.SetDTag( tag_source, value, true );
      }
      
      /* Process command with CommandType = "WriteValueFromWidget" */
      else if( command_type.equals( "WriteValueFromWidget" ) )
      {
         /* Obtain ValueResource from the command, an S-type resource
            indicating the resource name of the input widget holds the
            new widget value. For example, for a toggle, it will be resource
            "OnState"; for a swith widget, a spinner or a slider, 
            it will be resource named "Value". 
         */
         String value_res = command_obj.GetSResource( "ValueResource" );
         
         /* Obtain new value from the input widget. */
         value = selected_obj.GetDResource( value_res ).doubleValue();
         
         /* Retrieve output tag source (output data variable) from the command. */
         tag_source = command_obj.GetSResource( "OutputTagHolder/TagSource" );
         
         /* Validate. */
         if( tag_source == null || tag_source.isEmpty() || 
             tag_source.equals( "unset" ) )
         {
            System.out.println( "Invalid TagSource. WriteValueFromWidget Command failed." );
            return;
         }
         
         /* Place custom code here to write new value to the specified 
            tag source. For demo purposes, set the value of the 
            specified tag in the drawing.
         */
         glg_bean.SetDTag( tag_source, value, true );
      }
      
      /* Process command with CommandType = "GoTo" */
      else if(  command_type.equals( "GoTo" ) )
      {
         /* GoTo command will destroy the current drawing and replace it
            with a new drawing. Since the click event is processed within the
            callback triggered on the viewport itself, the
            viewport cannot be destroyed within its own callback,
            and the GoTo command should be processed on a
            timer. The command_obj passed as a parameter to the 
            timer procedure.
         */
         
         Timer timer = new Timer( 1, new ActionListener(){ 
                  public void actionPerformed( ActionEvent e )
                  { ProcessCommandGoTo( command_obj ); } } );

         timer.setRepeats( false );
         timer.start();
      }
      
      /* Process custom command with CommandType = "TankSelected" */
      else if( command_type.equals( "TankSelected" ) )
      {
         /* Place custom code here to handle the command as needed. 
            selected_obj argument passed to this function holds the
            object ID of the selected tank.
         */
         String obj_name = selected_obj.GetSResource( "Name" );
         System.out.println( 
                   "Processing custom command TankSelected for object Name=" + 
                    obj_name );
      }
   }
   
   ///////////////////////////////////////////////////////////////////////////
   // Process custom events attached to an object via Actions with
   // ActionType = "SEND EVENT". 
   ///////////////////////////////////////////////////////////////////////////
   void ProcessSelectionEvent( GlgObject event_vp, GlgObject selected_obj, 
                               String event_label )
   {
      /* Add custom code here to process custom event as needed. */
      
      if( selected_obj == null )
        return;
      
      /* Obtain selected object name. */
      String obj_name = selected_obj.GetSResource( "Name" );
      System.out.println( "Processing selection event for object " + obj_name ); 
   }
   
   ///////////////////////////////////////////////////////////////////////////
   // Returns true if object selection event should be processed
   // on MouseRelease as opposed to MouseClick. In this example, this
   // condition is defined in the drawing using resource "OnRelease",
   // which is added as a custom resource to an Action object, via
   // "Add Data". It is expected that the ActionObject is named 
   // and its HasResource=ON, so that we can access resource "OnRelease"
   // from the action_obj.
   ///////////////////////////////////////////////////////////////////////////
   boolean ProcessOnRelease( GlgObject selected_obj, GlgObject action_obj )
   {
      if( selected_obj == null )
        return false;
      
      /* Pre-3.5 Custom MouseClickEvent, process on MouseClick by default. 
         It may changed as needed.
      */
      if( action_obj == null ) 
        return false;
      
      /* Events added as Actions (GLG v.3.5 and later): 
         Retrieve OnRelease resource, if any. If set to 0, process selection on
         MouseClick. Otherwise, process selection on MouseRelease.
      */
      GlgObject res_obj = action_obj.GetResourceObject( "OnRelease" );
      if( res_obj != null )
      {
         double on_release = res_obj.GetDResource( null ).doubleValue();
         if( on_release != 0. ) /* OnRelease != 0 */
           return true;       // Process on MouseRelease.
      }
      
      // Process on MouseClick.
      return false;           
     }
   
   ///////////////////////////////////////////////////////////////////////////
   // Returns the object/widget which is a parent of specified child and
   // is a direct child of the specified viewport (it may be the object itself).
   ///////////////////////////////////////////////////////////////////////////
   GlgObject GetDirectChild( GlgObject viewport, GlgObject child )
   {
      /* Get the viewport's container (Array resource). */
      GlgObject container = viewport.GetResourceObject( "Array" );
      
      while( child != null  )
      {
         GlgObject parent = child.GetParent();	
         if( parent == container )
           return child;
         
         child = parent;
      }
        
      return null;    /* Not found: should not happen in this example. */
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Update the comment string in the viewport using the resource
   // Comment/String, if found.
   //////////////////////////////////////////////////////////////////////////
   void UpdateComment( GlgObject viewport, GlgObject selected_obj, String str )
   {
      if( !viewport.HasResourceObject( "Comment" ) )
        return;
      
      String comment;
      
      /* Retrieve selected object name and update the comment 
         string in the drawing, appending the object name, if any.
      */
      if( selected_obj != null )
        comment = str + selected_obj.GetSResource( "Name" );
      else 
        comment = str;
      
      viewport.SetSResource( "Comment/String", comment );  
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
      frame.setSize( 700, 600 );
      frame.addWindowListener( new DemoQuit() );
      
      GlgObjectSelection obj_sel_example = new GlgObjectSelection();   
      frame.getContentPane().add( obj_sel_example );
      frame.setVisible( true );
      
      /* Set a GLG drawing to be displayed in the GLG bean
         Set after layout negotiation has finished.
      */
      obj_sel_example.LoadDrawing( "obj_selection.g" ); 
   }
}
   
