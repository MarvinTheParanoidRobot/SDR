#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>

#ifdef _WINDOWS
# pragma warning( disable : 4996 )    /* Allow cross-platform strcpy */
# pragma warning( disable : 4800 )
#define strcasecmp   _stricmp
#endif

using namespace std;

/* Set GLG_C_CONST_CHAR_PTR to 1 to use constant strings (const char *)
   and pass them to GLG C functions. If not set, a (char*) cast is required
   when passing strings to GLG C functions, such as GlgStrClone, GlgFree, etc.
   Needs to be defined before GlgClass.h.
 */
#define GLG_C_CONST_CHAR_PTR  1
#include "GlgClass.h"

// If set to 1, enable debugging information
#define DEBUG 0

// Set to 1 to debug adjusted time interval for the timers. 
#define DEBUG_TIMER 0

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

// Macros.
#define StartsWith( string1, string2 ) \
   ( strncmp( string1, string2, strlen( string2 ) ) == 0 )

/* If both are NULL or have the same content. */
#define StrEqual( string1, string2 ) \
   ( string1 == string2 || \
     string1 && string2 && strcmp( string1, string2 ) == 0 )

#define RELATIVE_TO_NEW_RANGE( low, high, rel_value ) \
   ( (low) + ((high) - (low)) * rel_value )

// Function Prototypes
void SetSize( GlgObjectC& viewport, 
              GlgLong x, GlgLong y, GlgLong width, GlgLong height );
double GetCurrTime( void );
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, 
                            GlgLong interval );
GlgBoolean EndsWith( SCONST char * string1, SCONST char * string2, 
                     GlgBoolean use_case );
char * RemoveTail( SCONST char * str, SCONST char * tail );
GlgBoolean IsUndefined( SCONST char * str );

