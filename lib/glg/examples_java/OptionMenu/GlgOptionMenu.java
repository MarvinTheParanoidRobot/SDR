/*----------------------------------------------------------------------
| This example demonstrates how to use GLG Option Menu Widget.
*/
import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

public class GlgOptionMenu extends GlgJBean
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////

   static final long serialVersionUID = 0;

   int counter1 = 0;   
   int counter2 = 0;   

   //////////////////////////////////////////////////////////////////////////
   public GlgOptionMenu()
   {
      super();
   }

   // Invoked after hierarchy setup, but before the drawing is displayed. 
   public void VCallback( GlgObject viewport )
   {
      // An example of initializing option widgets programmatically:
      //     defines items in the option menu. 
      // InitializeMenu( "OptionMenu" );
   }
   
   //////////////////////////////////////////////////////////////////////////
   // This callback is invoked when user interacts with input objects in GLG
   // drawing. 
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

      // Handle window closing if run stand-alone
      if( format.equals( "Window" ) && action.equals( "DeleteWindow" ) )
        System.exit( 0 );

      if( counter1 > 5 )
        counter1 = 0;
      if( counter2 > 5 )
        counter2 = 0;

      // Option menu messages.
      if( format.equals( "Option" ) && action.equals( "Select" ) )
      {
         if( origin.equals( "OptionMenu" ) )
         {
             String selected_item = message_obj.GetSResource( "SelectedItem" );
             int selected_index = 
                message_obj.GetDResource( "SelectedIndex" ).intValue();
             System.out.println( "Selected Item = " + selected_item +
                             " SelectedIndex = " + selected_index );
         }
      }
      else if( format.equals( "Button" ) && action.equals( "Activate" ) )
      {
         // Send option menu widget messages
         if( origin.equals( "ItemMessButton" ) )
         {
            SendItemMessages( "OptionMenu", counter1 );
            ++counter1;	 
         }
      }

      // Update the viewport.
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Initialize items list. 
   // InitItemList property of an object menu is meant to be used in the
   // GLG Builder only. If it is present, it can be modified using
   // resources InitItemList/Item0, InitItemList/Item1 and so on.
   // 
   // To set items programmatically, AddItem or SetItemList messages should 
   // be used. AddItem message will add items to the current InitItemList list,
   // if any. SetItemList message will replace the current item list.
   // It requires the use of the Extended API.
   // 
   // In this example, AddItem message is used to add items to the menu.
   // Since the InitItemList property of the option menu widget in the drawing
   // is not empty, the items will be added to the current item list.
   // If the user wants to define an entire item list in the application,
   // without the use of the Extended API, the option menu widgets in the 
   // drawing should not have any items in the InitItemList property.
   //////////////////////////////////////////////////////////////////////////
   void InitializeMenu( String obj_name )
   {
      GlgObject viewport = GetViewport();
      String res_name = GlgObject.ConcatResNames( obj_name, "Handler" );
   
      viewport.SendMessage( res_name, "AddItem", "Dallas", 
                            new Integer(GlgObject.BOTTOM), null, null );

      viewport.SendMessage( res_name, "AddItem", "New York", 
                            new Integer(GlgObject.BOTTOM), null, null );

      viewport.SendMessage( res_name, "AddItem", "Boston", 
                            new Integer(GlgObject.BOTTOM), null, null );

      viewport.SendMessage( res_name, "AddItem", "Chicago", 
                            new Integer(GlgObject.BOTTOM), null, null );

      viewport.SendMessage( res_name, "UpdateItemList", 
                            null, null, null, null );
   }

   //////////////////////////////////////////////////////////////////////////
   // Process the following option menu widget messages:
   // SetInitItemList 
   // SetItemList
   // GetItemList
   // GetItemCount
   // AddItem
   // DeleteItem
   // UpdateItemList
   //////////////////////////////////////////////////////////////////////////
   void SendItemMessages( String obj_name, int case_index )
   {
      GlgObject viewport = GetViewport();
      String handler_res = GlgObject.ConcatResNames( obj_name, "Handler" );
      int i;

      switch( case_index )
      {
      case 0:
         // SetInitItemList
         SetSResource( "MessageString", "SetInitItemList" );
         Update();

         SetSResource( GlgObject.ConcatResNames( obj_name, "InitItemList/Item0" ), 
                       "Green" );
         viewport.SendMessage( handler_res,
                               "SetInitItemList", 
                               null, null, null, null );
         System.out.println( "Message SetInitItemList, set Item0 to Green" );
         break;

      case 1:
         // SetItemList 
         SetSResource( "MessageString", "SetItemList" );
         Update();
         
         GlgObject new_item_list = 
            new GlgDynArray( GlgObject.STRING, 0, 0 );

         new_item_list.AddObjectToBottom( "Red" );
         new_item_list.AddObjectToBottom( "Green" );
         new_item_list.AddObjectToBottom( "Yellow" );
         new_item_list.AddObjectToBottom( "Blue" );
         new_item_list.AddObjectToBottom( "Purple" );

         viewport.SendMessage( handler_res, "SetItemList", 
                               new_item_list, null, null, null );

         System.out.println( " Message SetItemList, set items to Red,Green,Yellow,Blue,Purple" );
         break;

      case 2:
         // GetItemList
         SetSResource( "MessageString", "GetItemList" );
         Update();
         
         GlgObject item_list = (GlgObject)
            viewport.SendMessage( handler_res, "GetItemList", 
                                  null, null, null, null );
         if( item_list == null)
            return;

         System.out.println( "Message GetItemList, menu items are:");
         for( i =0; i < item_list.GetSize(); ++i )
         {
               String elem;
               elem = (String) item_list.GetElement( i );
               System.out.println( elem );
         }
         break;

      case 3:
         // GetItemCount 
         SetSResource( "MessageString", "GetItemCount" );
         Update();

         Integer num_items = (Integer)
            viewport.SendMessage( handler_res, "GetItemCount", 
                                  null, null, null, null );
         System.out.println( " Message GetItemCount, number of items = " +
                                num_items );
         break;

      case 4:
         // AddItem. Adds an item to the bottom of the list.
         SetSResource( "MessageString", "AddItem" );
         Update();

         viewport.SendMessage( handler_res, "AddItem", "Magenda", 
                               new Integer(GlgObject.TOP), null, null );
         viewport.SendMessage( handler_res, "UpdateItemList", 
                               null, null, null, null );

         System.out.println( " Message AddItem, add item Magenda to the top of list" );
         break;

      case 5:
         // DeleteItem
         SetSResource( "MessageString", "DeleteItem" );
         Update();

         viewport.SendMessage( handler_res, "DeleteItem", 
                               new Integer(GlgObject.BOTTOM),
                               null, null, null );
         viewport.SendMessage( handler_res, "UpdateItemList", 
                               null, null, null, null );
      
         System.out.println( " Message DeleteItem, delete last item" );
         break;

       default:
          SetSResource( "MessageString", "" );
          Update();
          break;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // For use as a stand-alone application
   //////////////////////////////////////////////////////////////////////////
   public static void main( final String arg[] )
   {
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Main( arg ); } } );
   }

   //////////////////////////////////////////////////////////////////////////
   public static void Main( final String arg[] )
   {
      class DemoQuit extends WindowAdapter
      {
         public void windowClosing( WindowEvent e ) { System.exit( 0 ); }
      } 

      JFrame frame = new JFrame();
      frame.setResizable( true );
      frame.setSize( 600, 600 );
      frame.setLocation( 20, 20 );
      frame.addWindowListener( new DemoQuit() );

      GlgOptionMenu example = new GlgOptionMenu();      
      frame.getContentPane().add( example );
      frame.setVisible( true );

      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      // Setting the drawing triggers the ReadyCallback which starts updates.
      //
      example.SetDrawingName( "option.g" );
   }
}
