/*----------------------------------------------------------------------
| This example demonstrates how to use GLG OptionMenu Widget.
*/

#include <stdio.h>
#include <stdlib.h>

#include "GlgApi.h"

/* Function prototypes */
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void SendItemMessages( GlgObject viewport, char * obj_name, long case_index );
void InitializeMenu( GlgObject viewport, char * obj_name );

GlgAppContext AppContext;

/* Defines a platform-specific program entry point */
#include "GlgMain.h"

/*----------------------------------------------------------------------
|
*/
int GlgMain( argc, argv, InitAppContext )
     int argc;
     char *argv[];
     GlgAppContext InitAppContext;
{
   GlgObject viewport;
   GlgObject init_item_list;
   char * item_string;
   
   AppContext = GlgInit( False, InitAppContext, argc, argv );

   /* Take a drawing from the file. */
   viewport = GlgLoadWidgetFromFile( "option.g" );
   if( !viewport )
     exit( 1 );
   
   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( viewport, "Point1", -400., -450., 0. );
   GlgSetGResource( viewport, "Point2", 400., 450., 0. );

   /* Setting the window name (title). */
   GlgSetSResource( viewport, "ScreenName", "GlgOptionMenu Example" );
  
   /* Add Input callback to handle List messages */
   GlgAddCallback( viewport, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );

   /* Set item list. Object hierarchy must be setup fist. */
   GlgSetupHierarchy( viewport );

   InitializeMenu( viewport, "OptionMenu" );

   GlgUpdate( viewport );

   return GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| This callback is used to handle input events, including selection
| events in the OptionMenu widget.
*/
void Input( viewport, client_data, call_data )
     GlgObject viewport;
     GlgAnyType client_data;
     GlgAnyType call_data;
{
   GlgObject 
     message_obj,
     item_list,
     item_state_list,
     selected_item_list;
   char
     * format,
     * action,
     * origin,
     * subaction,
     * selected_item;
   double selected_index;
   int i;
   static long counter1 = 0;   
   static long counter2 = 0;   
      
   message_obj = (GlgObject) call_data;

   /* Get the message's format, action and origin. */
   GlgGetSResource( message_obj, "Format", &format );
   GlgGetSResource( message_obj, "Action", &action );
   GlgGetSResource( message_obj, "Origin", &origin );

   /* Handle window closing. May use viewport's name. */
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );

   if( counter1 > 5 )
     counter1 = 0;
   if( counter2 > 5 )
     counter2 = 0;

   /* OptionMenu messages. */
   if( strcmp( format , "Option" ) == 0  &&
       strcmp( action , "Select" ) == 0 )
   {
      if( strcmp( origin , "OptionMenu" ) == 0 )
      {
	 GlgGetSResource( message_obj, "SelectedItem", &selected_item );
	 GlgGetDResource( message_obj, "SelectedIndex", &selected_index );
	 printf( "Selected Item = %s Selected Index = %lf\n",
		  selected_item, selected_index );
	 
      }
   }
   else if( strcmp( format, "Button" ) == 0  &&
	    strcmp( action, "Activate" ) == 0 )
   {
      /* Send List widget messages */
      if( strcmp( origin, "ItemMessButton" ) == 0 )
      {
	 SendItemMessages( viewport, "OptionMenu", counter1 );
	 ++counter1;	 
      }
   }
   /* Update the viewport. */
   GlgUpdate( viewport );
   
}

/*----------------------------------------------------------------------
| Initialize a list of items in the menu. 
| InitItemList property of a list object is meant to be used in the
| GLG Builder only. If it is present, it can be modified using
| resources InitItemList/Item0, InitItemList/Item1 and so on.
| 
| To set items programmatically, AddItem or SetItemList messages should 
| be used. AddItem message will add items to the current InitItemList list, 
| if any. SetItemList message will replace the current item list.
| It requires the use of the Extended API.
|
| In this example, AddItem message is used to add items to the menu.
| Since the InitItemList property of the option menu widget in the drawing
| is not empty, the items will be added to the current item list.
| If the user wants to define an entire item list in the application,
| without the use of the Extended API, the option menu widget of the drawing
| should not have any items in the InitItemList property.
*/
void InitializeMenu( viewport, obj_name )
     GlgObject viewport;
     char * obj_name;
{
   char *res_name;

   res_name = GlgConcatResNames( obj_name, "Handler" );

   GlgSendMessage( viewport, res_name,
		  "AddItem", (GlgAnyType) "Dallas", 
		  (GlgAnyType) GLG_BOTTOM, NULL, NULL );

   GlgSendMessage( viewport, res_name,
		  "AddItem", (GlgAnyType) "New York", 
		  (GlgAnyType) GLG_BOTTOM, NULL, NULL );

   GlgSendMessage( viewport, res_name,
		  "AddItem", (GlgAnyType) "Boston", 
		  (GlgAnyType) GLG_BOTTOM, NULL, NULL );

   GlgSendMessage( viewport, res_name,
		  "AddItem", (GlgAnyType) "Chicago", 
		  (GlgAnyType) GLG_BOTTOM, NULL, NULL );

   GlgSendMessage( viewport, res_name,
		  "UpdateItemList", 
		  NULL, NULL, NULL, NULL );

   GlgFree( res_name );
}

/*----------------------------------------------------------------------
| Process the following option menu messages:
|   SetInitItemList 
|   SetItemList
|   GetItemList
|   GetItemCount
|   AddItem
|   DeleteItem
|   UpdateItemList
*/
void SendItemMessages( viewport, obj_name, case_index )
     GlgObject viewport;
     char * obj_name;
     long case_index;
{

   GlgObject new_item_list;
   char ** item_list;
   long num_items, i, rvalue;
   char * handler_res, * item_res;

   handler_res = GlgConcatResNames( obj_name, "Handler" );

   switch( case_index )
   {
    case 0:
      /* SetInitItemList */
      GlgSetSResource( viewport, "MessageString", "SetInitItemList" );
      GlgUpdate( viewport );

      item_res = GlgConcatResNames( obj_name, "InitItemList/Item0" );
      GlgSetSResource( viewport, item_res, "Green" );
      rvalue = (long)GlgSendMessage( viewport, handler_res,
				    "SetInitItemList", 
				    NULL, NULL, NULL, NULL );
      printf( "Message SetInitItemList, set Item0 to Green\n" );
      GlgFree( item_res );
      break;

    case 1:
      /* SetItemList */
      GlgSetSResource( viewport, "MessageString", "SetItemList" );
      GlgUpdate( viewport );

      new_item_list = GlgCreateObject( GLG_ARRAY, NULL, 
				      (GlgAnyType)GLG_STRING, 
				      NULL, NULL, NULL ); 
      GlgAddObjectToBottom( new_item_list, "Red" );
      GlgAddObjectToBottom( new_item_list, "Green" );
      GlgAddObjectToBottom( new_item_list, "Yellow" );
      GlgAddObjectToBottom( new_item_list, "Blue" );
      GlgAddObjectToBottom( new_item_list, "Purple" );

      rvalue = (long)GlgSendMessage( viewport, handler_res,
				    "SetItemList", 
				    (GlgAnyType) new_item_list, 
				    NULL, NULL, NULL );
      GlgDropObject( new_item_list );

      printf( "Message SetItemList, %s set items to Red,Green,Yellow,Blue,Purple\n", obj_name );
      break;

    case 2:
      /* GetItemList */
      GlgSetSResource( viewport, "MessageString", "GetItemList" );
      GlgUpdate( viewport );

      item_list = GlgSendMessage( viewport, handler_res,
				 "GetItemList", 
				 NULL, NULL, NULL, NULL );
      if( !item_list )
	return;

      printf( "Message GetItemList, menu elements:\n" );
      for( i =0; i < GlgGetSize( item_list ); ++i )
      {
	 char * elem;
	 elem = (char *)GlgGetElement( item_list, i );
	 printf( "  %s\n", elem );
      }

      break;

    case 3:
      /* GetItemCount */
      GlgSetSResource( viewport, "MessageString", "GetItemCount" );
      GlgUpdate( viewport );

      num_items = (long) GlgSendMessage( viewport, handler_res,
					"GetItemCount", 
					NULL, NULL, NULL, NULL );
      printf( "Message GetItemCount, number of items in the menu = %ld\n", 
	       num_items );
      break;

    case 4:
      /* AddItem. Adds an item to the bottom of the list */
      GlgSetSResource( viewport, "MessageString", "AddItem" );
      GlgUpdate( viewport );

      rvalue = (long) GlgSendMessage( viewport, handler_res,
				     "AddItem", 
				     (GlgAnyType) "Magenda", 
				     (GlgAnyType) GLG_TOP, NULL, NULL );
      rvalue = (long) GlgSendMessage( viewport, handler_res,
				     "UpdateItemList", 
				      NULL, NULL, NULL, NULL );

      printf( "Message AddItem, add item Magenda to the top of list %s\n",
               obj_name );
      break;

    case 5:
      /* DeleteItem */
      GlgSetSResource( viewport, "MessageString", "DeleteItem" );
      GlgUpdate( viewport );

      rvalue = (long) GlgSendMessage( viewport, handler_res,
				     "DeleteItem", 
				     (GlgAnyType) GLG_BOTTOM,
				     NULL, NULL, NULL );
      rvalue = (long) GlgSendMessage( viewport, handler_res,
				     "UpdateItemList", 
				      NULL, NULL, NULL, NULL );
      
      printf( "Message DeleteItem, delete last item from menu\n" );
      break;

    default:
      GlgSetSResource( viewport, "MessageString", "" );
      GlgUpdate( viewport );
      break;
   }

   GlgFree( handler_res );
}






