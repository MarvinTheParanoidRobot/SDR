#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "GlgApi.h"

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
void Input ( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void InitBeforeH( void );
void InitAfterH( void );
void InitTag( char * tag_source );
void CreateTagRecords( GlgObject viewport );
void UpdateDrawing( GlgAnyType data, GlgLong * timer_id );
GlgBoolean DemoReadDValue( TagRecord * tag_record, double * d_value );
GlgBoolean DemoReadSValue( TagRecord* tag_record, char ** s_value );
GlgBoolean DemoWriteDValue( char *tag_source, double value );
GlgBoolean DemoWriteSValue( char *tag_source, char * str );

GlgBoolean ReadDValue( TagRecord* tag_record, double *d_value );
GlgBoolean ReadSValue( TagRecord* tag_record, char ** s_value );
GlgBoolean WriteDValue( char *tag_source, double value );
GlgBoolean WriteSValue( char *tag_source, char * str );

void StopUpdates( void );
void StartUpdates( void );
