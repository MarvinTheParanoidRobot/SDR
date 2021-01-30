#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "zmq.h"
#include "GlgApi.h"
#include "raw_data.h"

typedef struct _TagRecord
{
   GlgObject tag_obj;
   char * tag_source;
   GlgDataType data_type;

   /* May be used to further optimize performance. */
   GlgBoolean if_changed;

} TagRecord;

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

/* Function prototypes */
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void InitBeforeH( void );
void InitAfterH( void );
void InitTag( char * tag_source );
void CreateTagRecords( GlgObject viewport );
void UpdateDrawing( GlgAnyType data, GlgLong * timer_id );
GlgBoolean DemoWriteDValue( char *tag_source, double value );

void StopUpdates( void );
void StartUpdates( void );
double GetCurrTime( void );
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, 
                            GlgLong interval );

GlgBoolean ProcessNewData( void * receiver_socket );
void PushDataToGUI( BaseData * data );
GlgBoolean PushDTagData( char * data_source, double d_value );
GlgBoolean PushGTagData( char * data_source, double x, double y, double z );
GlgBoolean PushSTagData( char * data_source, char * s_value );
TagRecord * LookupTagRecords( char * tag_source, GlgLong data_type );

void ProcessDemoData( void * sender_socket );
void ProcessRealData( void * sender_socket );

int ReceiveDataType( void * receiver_socket );
GlgBoolean ReceiveDataStruct( void * receiver_socket, DataStructType type, 
                              BaseData * data );
void SendDataToGUIThread( void * sender_socket, BaseData * data );
void * CreateZMQSender( void * zmq_context );
void * CreateZMQReceiver( void * zmq_context );

#ifndef _WINDOWS
void * DataThreadFunc( void * arg );
#else
DWORD WINAPI DataThreadFunc( LPVOID arg );
#endif

void SleepMS( GlgLong millisec );
