#include "scada.h"

TypeRecord DialogTypeTable[] = {
   { "Popup", GLOBAL_POPUP_DIALOG },
   { "AlarmDialog", ALARM_DIALOG },
   { "CustomDialog", CUSTOM_DIALOG },
   { NULL, 0 }
};

TypeRecord PopupMenuTypeTable[] = {
   { "PopupMenu", GLOBAL_POPUP_MENU },
   { NULL, 0 }
};

TypeRecord CommandTypeTable[] = {
   { "ShowAlarms", SHOW_ALARMS },
   { "GoTo", GOTO },
   { "PopupDialog", POPUP_DIALOG },
   { "PopupMenu", POPUP_MENU },
   { "ClosePopupDialog", CLOSE_POPUP_DIALOG },
   { "ClosePopupMenu", CLOSE_POPUP_MENU },
   { "WriteValue", WRITE_VALUE },
   { "WriteValueFromWidget", WRITE_VALUE_FROM_WIDGET },
   { "Quit", QUIT },
   { NULL, 0 }
};

GlgLong ConvertStringToType( TypeRecord * table, SCONST char * type_str, 
			     GlgLong empty_type,
			     GlgLong undefined_type )
{
   GlgLong i;

   if( !type_str || !*type_str )
     return empty_type;
     
   for( i=0; table[i].type_str; ++i )
     if( strcmp( type_str, table[i].type_str ) == 0 )
       return table[i].type_enum;

   return undefined_type;
}

/*----------------------------------------------------------------------
| 
*/
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, 
                            GlgLong interval )
{
   GlgULong sec2, microsec2;
   GlgLong elapsed_time, adj_interval;

   GlgGetTime( &sec2, &microsec2 );  /* End time */
   
   /* Elapsed time in millisec */
   elapsed_time = 
     ( sec2 - sec1 ) * 1000 + (long) ( microsec2 - microsec1 ) / 1000;

   /* Maintain constant update interval regardless of the system speed. */
   if( elapsed_time + 20 >= interval )
      /* Slow system: update as fast as we can, but allow a small interval 
         for handling input events. */
     adj_interval = 20;
   else
     /* Fast system: keep constant update interval. */
     adj_interval = interval - elapsed_time;

#if DEBUG_TIMER
   printf( "sec= %ld, msec= %ld\n", sec2 - sec1, microsec2 - microsec1 );
   printf( "*** elapsed= %ld, requested= %ld, adjusted= %ld\n",
	  (long) elapsed_time, (long) interval, (long) adj_interval );
#endif

   return adj_interval;
}


/*----------------------------------------------------------------------
| Return exact time including fractions of seconds.
*/
double GetCurrTime()
{
   GlgULong sec, microsec;
   
   if( !GlgGetTime( &sec, &microsec ) )
     return 0.;
     
   return sec + microsec / 1000000.;
}


/*----------------------------------------------------------------------
| Utility function to validate the string.
*/
GlgBoolean IsUndefined( SCONST char * str )
{
   if( !str || !*str || strcmp( str, "unset" ) == 0 ||
       strcmp( str, "$unnamed" ) == 0 )
     return GlgTrue;

   return GlgFalse;
}


