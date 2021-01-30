#ifndef _glg_custom_dll_h
#define _glg_custom_dll_h

#include <stdio.h>
#if defined _WINDOWS || defined _WIN32 || defined _WIN64
#define MS_WINDOWS
#include <windows.h>
#endif
#include "GlgApi.h"

#ifdef MS_WINDOWS
/* On Windows, need to export the entry point. */
#define glg_export  __declspec( dllexport ) 
#else
#define glg_export 
#endif

void GlgActivateCustomDialogEvents( GlgObject dialog );

#endif
