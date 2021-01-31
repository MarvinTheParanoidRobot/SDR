#pragma once

#include "scada.h"
#include "PageType.h"
#include "ActiveDialogRecord.h"
#include "ActivePopupMenuRecord.h"

class HMIPageBase;
class DataFeedBase;

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

class GlgSCADAViewer : public GlgObjectC
{
 public:
   GlgSCADAViewer(void);
   ~GlgSCADAViewer(void);
   
 public:
   GlgAppContext AppContext;

   /* Global flag indicating whether to use simulated data or live
      data for animation. By default, it is initialized to true in main.cpp. 
      To use live data, set RANDOM_DATA to false in main.cpp, or use
      command line argument -live-data. 
   */
   GlgBoolean RANDOM_DATA;
   
   /* Will be set to true if random data (DemoDataFeed) are used for 
      the current page. Allows to animate some pages with live data, and
      others with simulated data, if needed for testing purposes.
   */
   GlgBoolean RandomData;

   /* This GlgSCADAViewer class is used in both the cross-platform version
      of the example and the MFC version on Windows. The use of the has_parent 
      member makes it possible to share the class between both versions.
      has_parent=True in case of an MFC application where a parent of the class
      is a GLG MFC Control. 
   */
   GlgBoolean has_parent; 

   // The type of the current page. PageTypeEnum is defined in PageType.h.
   PageTypeEnum PageType;
   
   /* A subclass of a base class that implements functionality of an HMI page. 
      A default HMI page subclass is provided; custom HMI page subclasses
      may be used to define custom page logic, as shown in the RealTimeChart 
      and Process pages.
   */
   HMIPageBase * HMIPage;
 
   /* Array of tag records. */
   TagRecordArrayType TagRecordArray;   

   /* Used for supplying data for animation, either random data for a demo,
       or live data in a real application.
   */
   DataFeedBase * DataFeed;        

   DataFeedBase * LiveDataFeedObj;   // Live data feed 
   DataFeedBase * DemoDataFeedObj;   // Demo data feed for simulated data
   
   GlgObjectC Menu;               // Navigation menu
   GlgObjectC DrawingArea;        // Subwindow object in top level drawing
   GlgObjectC DrawingAreaVP;      // Viewport loaded into the Subwindow
   GlgObjectC AlarmDialog;        // AlarmDialog viewport
   GlgObjectC AlarmListVP;        // Viewport containing the alarm list
   GlgLong NumAlarmRows;          // Number of visible alarms on one alarm page.
   GlgLong NumTagRecords;             // Number of tag records in TagRecordArray

   // An array to keep object IDs of alarm rows for faster access.
   GlgObjectC * AlarmRows;

   GlgLong AlarmStartRow;             // Scrolled state of the alarm list.
 
   // List of alarm records containing active alarms.
   AlarmRecordArrayType AlarmList; 

   GlgBoolean AlarmDialogVisible;
   GlgObjectC MessageDialog;      // Popup dialog used for messages.

   /* Array of active dialogs. Used to open/close active dialogs. */
   ActiveDialogRecord ActiveDialogs[ MAX_DIALOG_TYPE ];
   
   /* Active popup menu. It is assumed that only one popup menu
      is active at a time.
   */
   ActivePopupMenuRecord ActivePopupMenu;
 
   /* Array of menu items used to populate a navigation menu in the
      top level drawing scada_main.g.
   */
   MenuRecord * MenuArray;

   /* Number of menu items in MenuArray. */
   GlgLong NumMenuItems;

   GlgLong UpdateInterval;       // update interval in msec
   GlgLong AlarmUpdateInterval;  // alarm update interval in msec
   GlgLong TimerID;
   GlgLong AlarmTimerID;

   // Callbacks.
   virtual void Input( GlgObjectC& callback_viewport, GlgObjectC& message);
   virtual void Hierarchy( GlgObjectC& callback_viewport,
                                 GlgHierarchyCBStruct * hierarchy_info );
   virtual void Trace( GlgObjectC& callback_viewport, 
		       GlgTraceCBStruct * trace_data );

   // Sets window size and position.
   void SetSize( GlgLong x, GlgLong y, GlgLong width, GlgLong height );

   // Initialization methods.
   void Init( void );
   void InitBeforeH( void );
   void InitAfterH( void );
   void InitMenu( void );
 
   // Load
   GlgBoolean LoadDrawingFromMenu( GlgLong screen );
   GlgObject LoadDrawing( GlgObjectC& subwindow, SCONST char * drawing_file );
   void SetupLoadedPage( SCONST char * title );
   void SetupHMIPage( void );
   
   // Tags processing.
   void QueryTags( GlgObjectC& new_drawing );
   void CreateTagRecords( GlgObjectC& drawing_vp); 
   void DeleteTagRecords( void ); 
   void RemapTags( GlgObjectC& drawing_vp );
   void AssignTagSource( GlgObjectC& tag_obj, SCONST char * new_tag_source );
   GlgLong RemapNamedTags( GlgObjectC& object, 
                           SCONST char * tag_name, SCONST char * tag_source );

   GlgObject FindMatchingTimeEP( GlgObjectC& tag_obj );
   void SetPlotTimeEP( GlgObjectC& viewport );
   GlgTagRecord * LookupTagRecords( SCONST char * match_tag_source );

   // Periodic updates
   void UpdateData( void );
   void StartUpdates( void );
   void StopUpdates( void );	
   void ProcessAlarms( GlgBoolean query_new_list );

   //  Commands processing
   void ProcessObjectCommand( GlgObjectC& command_vp, GlgObjectC& selected_obj, 
			      GlgObjectC& action_obj );   
   void ShowAlarms( void );
   void DisplayPopupDialog( GlgObjectC& command_vp, GlgObjectC& selected_obj, 
			    GlgObjectC& command_obj );
   void ClosePopupDialog( DialogType dialog_type );
   void TransferTags( GlgObjectC& selected_obj, GlgObjectC& viewport, 
		      GlgBoolean unset_tags );
   void DisplayPopupMenu( GlgObjectC& command_vp, GlgObjectC& selected_obj, 
			  GlgObjectC& command_obj );
   void PositionPopupMenu();
   void ClosePopupMenu( PopupMenuType menu_type );
   void GoTo( GlgObjectC& command_vp, GlgObjectC& selected_obj, 
	 GlgObjectC& command_obj );
   void WriteValue( GlgObjectC& command_vp, GlgObjectC& selected_obj, 
		    GlgObjectC& command_obj );
   void WriteValueFromInputWidget( GlgObjectC& command_vp, GlgObjectC& selected_obj, 
				   GlgObjectC& command_obj );


   // Misc functions
   void CloseActivePopups( DialogType allow_dialog );
   void InitActivePopups( void );
   void ShowMessageDialog( SCONST char * message, GlgBoolean error );
   PageTypeEnum GetPageType( GlgObjectC& drawing );
   GlgBoolean ToggleResource( SCONST char * resource_name );
   GlgLong LookUpMenuArray( SCONST char * drawing_name );
   void SelectMainMenuItem( GlgLong menu_index, GlgBoolean update_menu );
   CommandType GetCommandType( GlgObjectC& command_obj );
   DialogType GetDialogType( GlgObjectC& command_obj );
   PopupMenuType GetPopupMenuType( GlgObjectC& command_obj );
   void Zoom( GlgObjectC& viewport, char zoom_type, double scale );
   void FillMenuArray( SCONST char * exe_path, SCONST char * config_filename );
   void Quit( void );

   GlgSCADAViewer& operator= ( const GlgObjectC& object );
};

