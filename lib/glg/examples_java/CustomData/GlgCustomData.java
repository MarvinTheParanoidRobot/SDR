/*-----------------------------------------------------------------------
| This example demonstrates how to programmatically add custom properties
| to an object, such as custom MouseClickEvent and TooltipString, as well
| as a custom DataValue property of the D (double) type.
|
| Drawing name is custom_data.g.
------------------------------------------------------------------------*/

import java.awt.event.*;
import java.lang.reflect.*;
import javax.swing.*;
import com.genlogic.*;

public class GlgCustomData extends GlgJBean
{
   static final long serialVersionUID = 0;

   //////////////////////////////////////////////////////////////////////
   // Constructor
   //////////////////////////////////////////////////////////////////////
   public GlgCustomData()
   {
   }

   //////////////////////////////////////////////////////////////////////////
   // For use as a stand-alone application
   //////////////////////////////////////////////////////////////////////////
   public static void main( final String arg[] )
   {
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Main( arg ); } } );
   }

   //////////////////////////////////////////////////////////////////////
   public static void Main( final String arg[] )
   {
      String filename;

      class DemoQuit extends WindowAdapter
      {
         public void windowClosing( WindowEvent e ) { System.exit( 0 ); }
      } 

      if( Array.getLength( arg ) == 0 || arg[ 0 ] == null )
         filename = "custom_data.g";
      else
         filename = arg[ 0 ];

      JFrame frame = new JFrame();
      frame.setResizable( true );
      frame.setSize( 500, 400 );
      frame.addWindowListener( new DemoQuit() );

      GlgCustomData glg_component = new GlgCustomData(); 
      frame.getContentPane().add( glg_component );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      //
      glg_component.SetDrawingName( filename );
   }

   /////////////////////////////////////////////////////////////////
   // ReadyCallback() is invoked after the drawing is loaded, setup
   //  and initially drawn.
   /////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      GlgObject circle = viewport.GetResourceObject( "Circle" );
      AddCustomData( circle, "Circle", 1. );

      GlgObject triangle = viewport.GetResourceObject( "Triangle" );
      AddCustomData( triangle, "Triangle", 2. );
   }

   ///////////////////////////////////////////////////////////////////
   void AddCustomData( GlgObject object, String obj_name, double value )
   {
      // Suspend the object since it has been already drawn.
      GlgObject suspend_info = object.SuspendObject();

      // Create a group that holds all object's custom properties.
      GlgObject group = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );

      // Add tooltip property
      GlgObject tooltip_prop = 
        new GlgSDataPoint( GlgObject.UNDEFINED_XR, null );
      tooltip_prop.SetSResource( "Name", "TooltipString" );  // Set name
      tooltip_prop.SetSResource( null, obj_name );           // Set value
      group.AddObjectToBottom( tooltip_prop );        // add to the group

      // Add custom mouse click event property
      GlgObject event_prop = 
        new GlgSDataPoint( GlgObject.UNDEFINED_XR, null );
      event_prop.SetSResource( "Name", "MouseClickEvent" );  // Set name
      event_prop.SetSResource( null, obj_name + "Event" );   // Set value
      group.AddObjectToBottom( event_prop );          // add to the group

      // Add custom property of D type
      GlgObject data_value = 
        new GlgDDataPoint( GlgObject.UNDEFINED_XR, null );
      data_value.SetSResource( "Name", "DataValue" );  // Set name
      data_value.SetDResource( null, value );          // Set value
      group.AddObjectToBottom( data_value );    // add to the group

      // Set object's HasResources=1 so that the custom properties are
        // visible as object's resources.
      object.SetDResource( "HasResources", 1. );

      // Attach the group containing custom properties to the object.
      object.SetResourceObject( "CustomData", group );      

      object.ReleaseObject( suspend_info );
   }

   ///////////////////////////////////////////////////////////////////
   // Input callback. Process Glg button selection here.
   ///////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject viewport, GlgObject message_obj )
   {
      String format = message_obj.GetSResource( "Format" );
      String action = message_obj.GetSResource( "Action" );
      // String subaction = message_obj.GetSResource( "SubAction" );
      // String origin = message_obj.GetSResource( "Origin" );
      
      if( format.equals( "CustomEvent" ) )
      {
         String event_label = message_obj.GetSResource( "EventLabel" );

         // Process mouse release and tooltip erase: invoked with
         // an empty label and null object.
         //
         if( event_label.equals( "" ) )
         { 
            if( action.equals( "ObjectTooltip" ) )
              System.out.println( "Custom event: Tooltip Erased\n" );
            else if( action.equals( "MouseRelease" ) )
              System.out.println( "Custom event: Mouse Released\n" );
            return;
         }

         // An example of getting an object id of the selected object.
         // GlgObject selected_object;
         // selected_object = message_obj.GetResourceObject( "Object" );

         String object_name = message_obj.GetSResource( "Object/Name" );
         double data_value = 
           message_obj.GetDResource( "Object/DataValue" ).doubleValue();

         System.out.println( "Custom event: " );
         System.out.println( "   action: " + action );
         System.out.println( "   event label: " + event_label );
         System.out.println( "   selected object name: " + object_name );
         System.out.println( "   data value = " + data_value );
         System.out.println( "" );
      }                            
   }
}
