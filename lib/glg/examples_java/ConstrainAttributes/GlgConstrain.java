/*----------------------------------------------------------------------
|  This example demonstrates the following features:
|  - constraining object attributes
|  - unconstraining object attributes
|
|  Drawing name, constrain.g is passed as an argument:
|  java GlgConstrain constrain.g
|
|  This example uses methods of the Extended API.
-----------------------------------------------------------------------*/

import java.awt.event.*;
import java.lang.reflect.*;
import javax.swing.*;
import com.genlogic.*;

public class GlgConstrain extends GlgJBean
{
   static final long serialVersionUID = 0;

   //////////////////////////////////////////////////////////////////////
   // Constructor
   //////////////////////////////////////////////////////////////////////
   public GlgConstrain()
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
         filename = "constrain.g";
      else
         filename = arg[ 0 ];

      JFrame frame = new JFrame();
      frame.setResizable( true );
      frame.setSize( 500, 400 );
      frame.addWindowListener( new DemoQuit() );

      GlgConstrain glg_component = new GlgConstrain(); 
      frame.getContentPane().add( glg_component );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      //
      glg_component.SetDrawingName( filename );
   }

   ///////////////////////////////////////////////////////////////////
   // Input callback. Process Glg button selection here.
   ///////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject viewport, GlgObject message_obj )
   {
      String format = message_obj.GetSResource( "Format" );
      String action = message_obj.GetSResource( "Action" );
      // String origin = message_obj.GetSResource( "Origin" );
      // String subaction = message_obj.GetSResource( "SubAction" );
      
      if( format.equals( "Button" ) )
      {
         if( action.equals( "ValueChanged" ) )
         {
            int value = message_obj.GetDResource( "OnState" ).intValue();

            // Object2 has been drawn, need to suspend and then release it
              // to constrain or unconstrain its attributes.
            GlgObject object2 = 
              viewport.GetResourceObject( "DrawingArea/Object2" );
            GlgObject suspend_info = object2.SuspendObject();
               
            if( value == 1 )   // Constrain
            {
               // Constrain factor of the Object2's xform to the factor
                 // of the Object1's xform.

               // Get attribute to be constrained: must use default attribute
               // name for the last element of the resource path.
               // For the Move xform, Factor is XformAttr3. Xform is 
               // the default attribute name for the transformation attached 
                 // to an object.
               GlgObject from_attr = 
                 object2.GetResourceObject( "Xform/XformAttr3" );

               // Get the attribute to constrain to.
               GlgObject to_attr =
                 viewport.GetResourceObject( 
                                   "DrawingArea/Object1/Xform/XformAttr3" );

               // Constrain "from" attribute to "to" attribute.
               from_attr.ConstrainObject( to_attr );	       
            }
            else // value == 0 :  Unconstrain
            {
               // Unconstrain factor of the Object2's xform

               // Get attribute to be unconstrained: must use default
                 // attribute name for the last element of the resource path.
               GlgObject attr = 
                 object2.GetResourceObject( "Xform/XformAttr3" );

               attr.UnconstrainObject();	       
            }

            object2.ReleaseObject( suspend_info ); // Release
            Update();
         }
      }                            
   }
}
