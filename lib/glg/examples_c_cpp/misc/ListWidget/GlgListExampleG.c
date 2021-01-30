/*----------------------------------------------------------------------
| This example demonstrates how to use GLG List Widget.
*/

#include <stdio.h>
#include <stdlib.h>

#include "GlgApi.h"

/* Function prototypes */
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void SendItemMessages( GlgObject viewport, char * obj_name, int case_index );
void SendSelectionMessages( GlgObject viewport, char * obj_name, 
                            int case_index );
void InitializeList( GlgObject viewport, char * obj_name );

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
   viewport = GlgLoadWidgetFromFile( "list.g" );
   if( !viewport )
     exit( 1 );
   
   /* Setting widget dimensions using world coordinates [-1000;1000].
      If not set, default dimensions will be used as set in the GLG editor.
      */
   GlgSetGResource( viewport, "Point1", -400., -450., 0. );
   GlgSetGResource( viewport, "Point2", 400., 450., 0. );

   /* Setting the window name (title). */
   GlgSetSResource( viewport, "ScreenName", "GlgListExample" );
  
   /* Add Input callback to handle List messages */
   GlgAddCallback( viewport, GLG_INPUT_CB, (GlgCallbackProc)Input, NULL );

   /* Set item list. Object hierarchy must be setup fist. */
   GlgSetupHierarchy( viewport );

   InitializeList( viewport, "SList" );
   InitializeList( viewport, "MList" );
   InitializeList( viewport, "EList" );

   GlgUpdate( viewport );

   return GlgMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| This callback is used to handle input events, object selection
| events and blinking events.
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
   int i, size;
   static int counter1 = 0;   
   static int counter2 = 0;   
      
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

   /* List messages. */
   if( strcmp( format , "List" ) == 0  &&
       strcmp( action , "Select" ) == 0 )
   {
      if( strcmp( origin , "SList" ) == 0 )
      {
	 GlgGetSResource( message_obj, "SelectedItem", &selected_item );
	 GlgGetDResource( message_obj, "SelectedIndex", &selected_index );
	 printf( "SList: Selected Item = %s Selected Index = %lf\n",
		  selected_item, selected_index );
	 
      }
      if( strcmp( origin , "MList" ) == 0 ||
	  strcmp( origin , "EList" ) == 0 )
      {
	 item_list = GlgGetResourceObject( message_obj, "ItemList" );
         selected_item_list = GlgGetResourceObject( message_obj, 
						    "SelectedItemList" );
	 item_state_list = GlgGetResourceObject( message_obj, 
						 "ItemStateList" );
	 if( !item_list || !item_state_list )
	   return;
	 	
	 /* Traverse the list to find the selected items */
         size = GlgGetSize( item_state_list );
	 for( i=0; i<size; ++i )
	 {
	    int elem;
	    elem = (int) GlgGetElement( item_state_list, i );
	    if( elem == 1 )
	      printf( "%s Item %d is selected\n", origin, i );
	 }

      }
   }
   else if( strcmp( format, "Button" ) == 0  &&
	    strcmp( action, "Activate" ) == 0 )
   {
      /* Send List widget messages */
      if( strcmp( origin, "ItemMessButton" ) == 0 )
      {
	 SendItemMessages( viewport, "SList", counter1 );
	 SendItemMessages( viewport, "MList", counter1 );
	 SendItemMessages( viewport, "EList", counter1 );
	 ++counter1;	 
      }
      else if( strcmp( origin, "SelMessButton" ) == 0 )
      {
         /* Send selection messages only to multiple selection lists. */
	 SendSelectionMessages( viewport, "MList", counter2 );
	 SendSelectionMessages( viewport, "EList", counter2 );
	 ++counter2;
      }
   }
   /* Update the viewport. */
   GlgUpdate( viewport );
   
}

/*----------------------------------------------------------------------
| Initialize a list of items. 
| InitItemList property of a list object is meant to be used in the
| GLG Builder only. If it is present, it can be modified using
| resources InitItemList/Item0, InitItemList/Item1 and so on.
| 
| To set items programmatically, AddItem or SetItemList messages should 
| be used. AddItem message will add items to the current InitItemList list, 
| if any. SetItemList message will replace the current item list.
| It requires the use of the Extended API.
|
| In this example, AddItem message is used to add items to the list.
| Since the InitItemList property of the list widgets in the drawing
| are not empty, the items will be added to the current item list.
| If the user wants to define an entire item list in the application,
| without the use of the Extended API, the list widgets in the drawing
| should not have any items in the InitItemList property.
*/
void InitializeList( viewport, obj_name )
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
| Process the following List widget messages:
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
     int case_index;
{
   GlgObject list;
   GlgObject new_item_list;

   char ** item_list;
   int i, num_items, size, rvalue;

   list = GlgGetResourceObject( viewport, obj_name );
   if( !list )
      return;
  
   switch( case_index )
   {
    case 0:
      /* SetInitItemList */
      GlgSetSResource( viewport, "MessageString", "SetInitItemList" );
      GlgUpdate( viewport );

      GlgSetSResource( list, "InitItemList/Item0", "Green" );
      rvalue = (int) GlgSendMessage( list, "Handler",
                                     "SetInitItemList", 
                                     NULL, NULL, NULL, NULL );
      printf( "Message SetInitItemList, set Item0 to Green\n" );
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

      rvalue = (int) GlgSendMessage( list, "Handler",
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

      item_list = GlgSendMessage( list, "Handler",
				 "GetItemList", 
				 NULL, NULL, NULL, NULL );
      if( !item_list )
	return;

      printf( "Message GetItemList, %s elements:\n", obj_name );

      size = GlgGetSize( item_list );
      for( i=0; i<size; ++i )
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

      num_items = (int) GlgSendMessage( list, "Handler",
					"GetItemCount", 
					NULL, NULL, NULL, NULL );
      printf( "Message GetItemCount, %s number of items = %d\n", 
	       obj_name, num_items );
      break;

    case 4:
      /* AddItem. Adds an item to the bottom of the list */
      GlgSetSResource( viewport, "MessageString", "AddItem" );
      GlgUpdate( viewport );

      rvalue = (int) GlgSendMessage( list, "Handler",
				     "AddItem", 
				     (GlgAnyType) "Magenda", 
				     (GlgAnyType) GLG_TOP, NULL, NULL );
      rvalue = (int) GlgSendMessage( list, "Handler",
				     "UpdateItemList", 
                                     NULL, NULL, NULL, NULL );

      printf( "Message AddItem, add item Magenda to the top of list %s\n",
               obj_name );
      break;

    case 5:
      /* DeleteItem */
      GlgSetSResource( viewport, "MessageString", "DeleteItem" );
      GlgUpdate( viewport );

      rvalue = (int) GlgSendMessage( list, "Handler",
				     "DeleteItem", 
				     (GlgAnyType) GLG_BOTTOM,
				     NULL, NULL, NULL );
      rvalue = (int) GlgSendMessage( list, "Handler",
				     "UpdateItemList", 
                                     NULL, NULL, NULL, NULL );
      
      printf( "Message DeleteItem, delete last item from list %s\n",
	       obj_name );
      break;

    default:
      GlgSetSResource( viewport, "MessageString", "" );
      GlgUpdate( viewport );
      break;
   }
}

/*----------------------------------------------------------------------
| Process the following List widget messages:
|   ResetAllItemsState
|   SetItemStateList
|   GetItemStateList
|   GetItemState
|   SetItemState
|   GetSelectedItemList
*/
void SendSelectionMessages( viewport, obj_name, case_index )
   GlgObject viewport;
   char *obj_name;
   int case_index;
{
   GlgObject list;
   GlgObject 
      item_state_list, 
      new_item_state_list,
      selected_item_list;
   int
     i,
     size,
     elem,
     item_state,
     num_items, 
     rvalue;
   char * item_label;

   list = GlgGetResourceObject( viewport, obj_name );
   if( !list )
      return;
  
   switch( case_index )
   {
    case 0: /* GetItemState. Get item state of the 2nd element */
      GlgSetSResource( viewport, "MessageString", "GetItemState" );
      GlgUpdate( viewport );

      item_state = (int) GlgSendMessage( list, "Handler",
                                         "GetItemState", (GlgAnyType) 2, 
                                         NULL, NULL, NULL );
      printf( "Message GetItemState, %s item 2 state is %d\n", 
              obj_name, item_state );
      break;

    case 1:  /* GetItemStateList */
      GlgSetSResource( viewport, "MessageString", "GetItemStateList" );
      GlgUpdate( viewport );

      item_state_list = GlgSendMessage( list, "Handler",
					 "GetItemStateList", 
					 NULL, NULL, NULL, NULL );
      /* Traverse the list to find the selected items */
      printf( "Message GetItemStateList, %s indexes of selected items:\n", 
              obj_name );

      size = GlgGetSize( item_state_list );
      for( i=0; i<size; ++i )
      {
         elem = (int) GlgGetElement( item_state_list, i );
         if( elem )
           printf( "  %d\n", i );
      }
      break;

    case 2:  /* SetItemState. Set 3rd item's state to 1 (selected) */
      GlgSetSResource( viewport, "MessageString", "SetItemState" );
      GlgUpdate( viewport );
      rvalue = (int) GlgSendMessage( list, "Handler",
                                     "SetItemState", (GlgAnyType) 3, 
                                     (GlgAnyType) 1, NULL, NULL );
      GlgUpdate( list );
      printf( "Message SetItemState, select the 3rd item of list %s\n", 
               obj_name );
      break;

    case 3:  /* SetItemStateList */
      GlgSetSResource( viewport, "MessageString", "SetItemStateList" );
      GlgUpdate( viewport );

      num_items = (int) GlgSendMessage( list, "Handler",
                                        "GetItemCount", 
                                        NULL, NULL, NULL, NULL );
      new_item_state_list = GlgCreateObject( GLG_ARRAY, NULL, 
                                             (GlgAnyType) GLG_LONG, 
                                             NULL, NULL, NULL ); 
      for( i=0; i<num_items; ++i )
      {
         /* Select items with odd indices. */
	 item_state = ( i % 2 ? 1 : 0 );
         GlgAddObjectToBottom( new_item_state_list, (GlgAnyType) item_state );
      }

      rvalue = (int) GlgSendMessage( list, "Handler",
                                     "SetItemStateList",
                                     (GlgAnyType) new_item_state_list, 
                                     NULL, NULL, NULL );
      printf( "Message SetItemStateList, select odd items of list %s\n",
               obj_name );

      /* Check the current ItemStateList to make sure it was
         set correctly by the previous SetItemStateList message */
      item_state_list = GlgSendMessage( list, "Handler",
					 "GetItemStateList", 
					 NULL, NULL, NULL, NULL );
      /* Traverse the list to find the selected items */
      printf( "Message GetItemStateList, %s has selected items:\n", 
	      obj_name );

      size = GlgGetSize( item_state_list );
      for( i=0; i<size; ++i )
      {
         elem = (int) GlgGetElement( item_state_list, i );
         if( elem )
           printf( "  %d\n", i );
      }
      break;

    case 4:  /* GetSelectedItemList */
      GlgSetSResource( viewport, "MessageString", "GetSelectedItemList" );
      GlgUpdate( viewport );

      selected_item_list = GlgSendMessage( list, "Handler",
                                           "GetSelectedItemList",
                                           NULL, NULL, NULL, NULL );
      if( !selected_item_list )
        return;
     
      printf( "Message GetSelectedItemList, %s selected items are:\n", 
               obj_name );

      size = GlgGetSize( selected_item_list );
      for( i=0; i <size; ++i )
      {
	 item_label = (char *) GlgGetElement( selected_item_list, i );
	 printf( "  %s\n", item_label );
      }
   
      break;

    case 5:  /* ResetAllItemsState */
      GlgSetSResource( viewport, "MessageString", "RestAllItemsState" );
      GlgUpdate( viewport );

      rvalue = (int) GlgSendMessage( list, "Handler",
                                     "ResetAllItemsState",
                                     NULL, NULL, NULL, NULL );

      printf( "Message ResetAllItemsState\n" );
      break;

    default:
      GlgSetSResource( viewport, "MessageString", "" );
      GlgUpdate( viewport );
      break;
      
   }
}





