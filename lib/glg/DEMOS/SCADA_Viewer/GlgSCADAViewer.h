#ifndef _GlgSCADAViewer_h_
#define _GlgSCADAViewer_h_

typedef struct _GlgSCADAViewer GlgSCADAViewer;

#include "GlgApi.h"
#include "scada.h"
#include "DataTypes.h"
#include "LiveDataFeed.h"
#include "DemoDataFeed.h"
#include "HMIPage.h"

struct _GlgSCADAViewer
{
   /* Set to true to demo with simulated data or to test without live data.
      Set to false or use -live-data command-line argument to use custom 
      live data feed.
   */
   GlgBoolean RANDOM_DATA;

   /* Will be set to true if random data are used for the current page. */
   GlgBoolean RandomData;

   PageTypeEnum PageType;
   HMIPage * HMIPagePtr;
 
   /* Array of tag records. */
   TagRecord * TagRecordArray;
   
   /* Number of tags records in the TagRecordArray. */
   GlgLong NumTagRecords;
   
   DataFeed * DataFeedPtr;        /* Used data feed */
   DataFeed * LiveDataFeedPtr;    /* Real data feed */
   DataFeed * DemoDataFeedPtr;    /* Demo data feed for simulated data */
   
   GlgObject DrawingArea;   /* Subwindow object in top level drawing */
   GlgObject DrawingAreaVP; /* Viewport loaded into the Subwindow */
   GlgObject MainViewport;  /* Main viewport of the top level drawing. */
   GlgObject Menu;          /* Navigation menu viewport */
   GlgObject AlarmDialog;   /* AlarmDialog viewport  */
   GlgObject AlarmListVP;   /* Viewport containing the alarm list */
   int NumAlarmRows;        /* Number of visible alarms on one alarm page. */
   GlgObject * AlarmRows;   /* Keep object ID's of alarm rows for faster 
                               access. */
   int AlarmStartRow;       /* Scrolled state of the alarm list. */
   GlgObject AlarmList;     /* List of alarms (contains alarm records). */
   GlgBoolean AlarmDialogVisible;

   GlgObject MessageDialog; /* Popup dialog used for messages. */
   
   /* Array of active dialogs. Used to open/close active dialogs. */
   ActiveDialogRecord ActiveDialogs[ MAX_DIALOG_TYPE ];
   
   /* Active popup menu. It is assumed that only one popup menu
      is active at a time.
   */
   ActivePopupMenuRecord ActivePopupMenu;
   
   GlgLong UpdateInterval;      /* Update rate in msec for drawing animation. */
   GlgLong AlarmUpdateInterval; /* Alarm update interval in msec. */
   GlgLong TimerID;
   GlgLong AlarmTimerID;
    
   MenuRecord * MenuArray;
   GlgLong NumMenuItems;   

};

extern GlgAppContext AppContext;
extern GlgSCADAViewer Viewer;

/* FUNCTION PROTOTYPES */

/* Initialization functions */
void InitBeforeH( void );
void InitAfterH( void );
void InitMenu( void );
void CreateDataFeed( void );

/* Callbacks */
void InputCB( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void TraceCB( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void DrawingLoadedCB( GlgObject viewport, GlgAnyType client_data, 
                      GlgAnyType call_data );
/* Load */
GlgBoolean LoadDrawingFromMenu( GlgLong screen );
GlgObject LoadDrawing( GlgObject subwindow, char * drawing_file );
void SetupLoadedPage( char * title );
void SetupHMIPage( void );

/* Tags processing. */
void CreateTagRecords( GlgObject drawing_vp );
void DeleteTagRecords( void ); 
void QueryTags( GlgObject new_drawing );
void RemapTags( GlgObject drawing_vp );
void AssignTagSource( GlgObject tag_obj, char * new_tag_source );
GlgLong RemapNamedTags( GlgObject object, char * tag_name, char * tag_source );

/* Periodic updates */
void StartUpdates( void );
void StopUpdates( void );
void PollTagData( void * client_data, GlgLong * timer_id );
void PollAlarms( void * client_data, GlgLong * timer_id );
void UpdateData( void );
void ProcessAlarms( GlgBoolean query_new_list );

/* Commands processing */
void ProcessObjectCommand( GlgObject command_vp, GlgObject selected_obj, 
			   GlgObject action_obj );
void ShowAlarms( void );
void DisplayPopupDialog( GlgObject command_vp, GlgObject selected_obj, 
			 GlgObject command_obj );
void ClosePopupDialog( DialogType dialog_type );
void DisplayPopupMenu( GlgObject command_vp, GlgObject selected_obj, 
		       GlgObject command_obj );
void ClosePopupMenu( PopupMenuType menu_type );
void GoTo( GlgObject command_vp, GlgObject selected_obj, 
	   GlgObject command_obj );
void WriteValue( GlgObject command_vp, GlgObject selected_obj, 
		 GlgObject command_obj );
void WriteValueFromInputWidget( GlgObject command_vp, GlgObject selected_obj, 
				GlgObject command_obj );
void TransferTags( GlgObject selected_obj, GlgObject viewport, 
                   GlgBoolean unset );
void PositionPopupMenu( void );

/* Misc functions */
void ShowMessageDialog( char * message, GlgBoolean error );
GlgBoolean ToggleResource( char * resource_name );
void CloseActivePopups( DialogType allow_dialog );
void InitActivePopups( void );
PageTypeEnum GetPageType( GlgObject drawing );
CommandType GetCommandType( GlgObject command_obj );
DialogType GetDialogType( GlgObject command_obj );
PopupMenuType GetPopupMenuType( GlgObject command_obj );
void Zoom( GlgObject viewport, char zoom_type, double scale );
GlgBoolean IsUndefined( char * string );
GlgLong LookUpMenuArray( char * drawing_name );
void SelectMainMenuItem( GlgLong menu_index, GlgBoolean update_menu );
void FreeAlarmData( AlarmRecord * alarm_record );
double GetCurrTime( void );
GlgObject FindMatchingTimeEP( GlgObject tag_obj );
void SetPlotTimeEP( GlgObject viewport );
TagRecord * LookupTagRecords( char * match_tag_source );

#endif
