
/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
/* ====================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
* ==================================================================== */
#ifndef OMX_DEBUG_H
#define OMX_DEBUG_H

#include "OMX_DebugMem.h"

#define OMX_NO_MESSAGES    0x00000000
#define OMX_ALL_MESSAGES   0xffff0000

#define OMX_MASK_FATAL     0x80000000
#define OMX_MASK_ERROR     0x40000000
#define OMX_MASK_WARN      0x20000000
#define OMX_MASK_INFO      0x10000000
#define OMX_MASK_DEBUG     0x08000000  
#define OMX_MASK_TRACE     0x04000000
#define OMX_MASK_RESERVE1  0x02000000
#define OMX_MASK_RESERVE2  0x01000000

#define OMX_MASK_USERMASK  0x00FF0000

#define OMX_OPTION_FILE    0x00000001
#define OMX_OPTION_FUNC    0x00000002
#define OMX_OPTION_LINE    0x00000004

#define OMX_MASK_HANDLES   0x0000FFFF



/*
 *  ANSI escape sequences for outputing text in various colors
 */
#define DBG_TEXT_WHITE   "\x1b[1;37;40m"
#define DBG_TEXT_YELLOW  "\x1b[1;33;40m"
#define DBG_TEXT_MAGENTA "\x1b[1;35;40m"
#define DBG_TEXT_GREEN   "\x1b[1;32;40m"
#define DBG_TEXT_CYAN    "\x1b[1;36;40m"
#define DBG_TEXT_RED     "\x1b[1;31;40m"


/* Setup log format (adds newline if no newline provided) */
// do not use this one directly....
#define OMX_LOG_PRINT(HANDLE, STR, ARG...) \
    (OMX_Log(HANDLE, __FILE__, __LINE__, __FUNCTION__, STR, ##ARG))

#ifdef OMX_DEBUG
    #define OMX_DPRINT(HANDLE, STR, ARG...) OMX_LOG_PRINT(OMX_MASK_DEBUG | HANDLE, STR, ##ARG)
    #define OMX_TPRINT(HANDLE, STR, ARG...) OMX_LOG_PRINT(OMX_MASK_TRACE | HANDLE, STR, ##ARG)
#else
    #define OMX_DPRINT(HANDLE, STR, ARG...)  
    #define OMX_TPRINT(HANDLE, STR, ARG...)
#endif

/* Later this will be able to be turned on/off separately as a trace */
#define OMX_DENTER(handle) OMX_TPRINT((handle), "+++ENTERING")
#define OMX_DEXIT(handle,retVal) OMX_TPRINT((handle), "---EXITING(0x%x)", (retVal))

#define OMX_DBG_INT(handle, intVar) OMX_DPRINT(OMX_MASK_DEBUG | (handle), #intVar ": %d", (intVar))
#define OMX_DBG_PTR(handle, ptrVar) OMX_DPRINT(OMX_MASK_DEBUG | (handle), #ptrVar ": 0x%08x", (ptrVar))
#define OMX_DBG_STR(handle, strVar) OMX_DPRINT(OMX_MASK_DEBUG | (handle), #strVar ": %s", (strVar))


/* Error/warning printing defines to be used by all sub-components */
#define OMX_INFOPRINT(handle, str,arg...)  (OMX_LOG_PRINT(OMX_MASK_INFO | (handle), "(INFO) "str, ##arg))
#define OMX_WARNPRINT(handle, str,arg...)  (OMX_LOG_PRINT(OMX_MASK_WARN | (handle), "(WARN) "str, ##arg))
#define OMX_ERRPRINT(handle, str,arg...)   (OMX_LOG_PRINT(OMX_MASK_ERROR | (handle), "(ERROR) "str, ##arg))
#define OMX_FATALPRINT(handle, str,arg...)   (OMX_LOG_PRINT(OMX_MASK_FATAL | (handle), "(FATAL) "str, ##arg))

/* assert macros */
#ifdef OMX_DEBUG
    #define OMX_ASSERT(COND) ((!(COND))?OMX_FATALPRINT(0,"OMX_ASSERT("#COND")"),abort():0)
#else
    #define OMX_ASSERT(COND)
#endif

#define OMX_LOG_ADD_MASK(HANDLE,NEW_BITS) (OMX_Log_SetMask((HANDLE), OMX_Log_GetMask(HANDLE) | (NEW_BITS)))
#define OMX_LOG_CLEAR_MASK(HANDLE,NEW_BITS) (OMX_Log_SetMask((HANDLE), OMX_Log_GetMask(HANDLE) & ~(NEW_BITS)))

#define OMX_LOG_ADD_OPTIONS(HANDLE,NEW_BITS) (OMX_Log_SetOptions((HANDLE), OMX_Log_GetOptions(HANDLE) | (NEW_BITS)))
#define OMX_LOG_CLEAR_OPTIONS(HANDLE,NEW_BITS) (OMX_Log_SetOptions((HANDLE), OMX_Log_GetOptions(HANDLE) & ~(NEW_BITS)))

typedef unsigned int OMX_DBG_HANDLE;

OMX_DBG_HANDLE OMX_Log_GetDebugHandle(const char *description);
void           OMX_Log_ReleaseDebugHandle(OMX_DBG_HANDLE hDebug);
unsigned int   OMX_Log_GetMask(OMX_DBG_HANDLE hDebug);
unsigned int   OMX_Log_SetMask(OMX_DBG_HANDLE hDebug, unsigned int uiNewMask);

unsigned int   OMX_Log_GetOptions(OMX_DBG_HANDLE hDebug);
unsigned int   OMX_Log_SetOptions(OMX_DBG_HANDLE hDebug, unsigned int uiNewOptions);

void OMX_Log(unsigned int mask, const char *szFileName, int iLineNum, 
             const char *szFunctionName, const char *strFormat, ...);

const char *OMX_GetErrorString(OMX_ERRORTYPE error);

OMX_ERRORTYPE OMX_Log_LoadConfigFile(char* szConfigFile);


/*
 * The following macros are intended to make accessing a debug handle easier.
 *
 * For example, for the Util library, you would create a header file called
 *   OMX_Util_Private.h.  This file will be included in all source files 
 *   compiled into the Util library.  The header file uses the 'define' macro
 *   to generate a prototype for the getDebugHandle() function:
 *
 *   DEFINE_DEBUG_HANDLE_FN(UTIL);
 *
 * Now, in your private header file, define easier macros for printing:
 *
 *   #define UTIL_DPRINT(str,args...) OMX_DPRINT(ACCESS_DEBUG_HANDLE(UTIL),str,##args)
 *   #define UTIL_WARNPRINT(str,args...) OMX_WARNPRINT(ACCESS_DEBUG_HANDLE(UTIL),str,##args)
 *   #define UTIL_ERRPRINT(str,args...) OMX_ERRPRINT(ACCESS_DEBUG_HANDLE(UTIL),str,##args)
 *
 * Finally, in a source file which will be compiled into the lib, for example 
 *   OMX_Util_Private.c, you implement the function with the 'implement' macro:
 *
 *   IMPLEMENT_DEBUG_HANDLE_FN(UTIL)
 *
 */
#define DEFINE_DEBUG_HANDLE_FN(MOD) OMX_DBG_HANDLE getDebugHandle_##MOD(void);

#define ACCESS_DEBUG_HANDLE(MOD) getDebugHandle_##MOD()

#define IMPLMENT_DEBUG_HANDLE_FN(MOD) \
OMX_DBG_HANDLE getDebugHandle_##MOD(void)                    \
{                                                            \
    static OMX_DBG_HANDLE hDebug = 0;                        \
    if(!hDebug) {                                            \
        hDebug = OMX_Log_GetDebugHandle(#MOD);               \
        OMX_DPRINT(0,"Component "#MOD": hDebug %d",hDebug);  \
    }                                                        \
    return hDebug;                                           \
}



#endif
