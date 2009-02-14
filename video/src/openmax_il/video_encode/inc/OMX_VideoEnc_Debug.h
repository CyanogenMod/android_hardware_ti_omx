
/*
 *  Copyright 2001-2008 Texas Instruments - http://www.ti.com/
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* =============================================================================
*             Texas Instruments OMAP(TM) Platform Software
*  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
*
*  Use of this software is controlled by the terms and conditions found 
*  in the license agreement under which this software has been supplied.
* =========================================================================== */
/**
* @file OMX_VideoEnc_Debug.h
*
* This file implements OMX Component for MPEG-4 encoder that 
* is fully compliant with the OMX specification 1.5.
*
* @path  $(CSLPATH)\inc
*
* @rev  0.1
*/
/* -------------------------------------------------------------------------- */
/* ============================================================================= 
*! 
*! Revision History 
*! ================================================================
*!
*! 02-Feb-2006 mf: Revisions appear in reverse chronological order; 
*! that is, newest first.  The date format is dd-Mon-yyyy.  
* =========================================================================== */

#ifndef OMX_VIDEOENC_DEBUG__H
#define OMX_VIDEOENC_DEBUG__H
#include <utils/Log.h>

/*
 *  PRINT OPTIONS     
 */

/* For printing errors */
#ifndef UNDER_CE
    #define __OMX_EPRINT__

/* For tracing flow and memory allocation/deallocation 
   Add -DOMX_DEBUG to CFLAGS of src/Makefile */
#if OMX_DEBUG 
    /* #define __OMX_PRINT__ */ /* just like printf() */
    #define __OMX_TRACE__
#endif

#if 0
    #define __OMX_DPRINT__
    #define __VIDENC_UTILS_DPRINT__
    #define __VIDENC_DPRINT__
    #define __VIDENC_THREAD_DPRINT__
#endif

#if 0
    #define OMX_PRINT_COLOR
#endif
#endif

#ifndef UNDER_CE /* Start Linux logging definitions */
/*
 *  ANSI escape sequences for outputing text in various colors
 */
#ifdef OMX_PRINT_COLOR
    #define DBG_TEXT_WHITE   "\x1b[1;37;40m"
    #define DBG_TEXT_YELLOW  "\x1b[1;33;40m" /* reserved for OMX_VideoEnc_Thread.c */
    #define DBG_TEXT_MAGENTA "\x1b[1;35;40m" 
    #define DBG_TEXT_GREEN   "\x1b[1;32;40m" /* reserved for OMX_VideoEncoder.c */
    #define DBG_TEXT_CYAN    "\x1b[1;36;40m" /* reserved for OMX_Video_Utils.c */
    #define DBG_TEXT_RED     "\x1b[1;31;40m" /* reserved for test app */ 
#else
    #define DBG_TEXT_WHITE   ""
    #define DBG_TEXT_YELLOW  ""
    #define DBG_TEXT_MAGENTA ""
    #define DBG_TEXT_GREEN   ""
    #define DBG_TEXT_CYAN    ""
    #define DBG_TEXT_RED     ""
#endif

void VIDENC_Log(const char *szFileName, int iLineNum, const char *szFunctionName, const char *strFormat, ...);
void VIDENC_Trace (const char *szFunctionName, const char *strFormat, ...);
void VIDENC_eprint (int iLineNum, const char *szFunctionName, const char *strFormat, ...);

#ifdef __OMX_TRACE__
    /*#define OMX_TRACE(STR, ARG...) VIDENC_Trace(__FUNCTION__, STR, ##ARG)*/
	#define OMX_TRACE LOGD
#else
    #define OMX_TRACE(...)
#endif

#ifdef __OMX_TRACE__
    /*#define OMX_EPRINT(STR, ARG...) VIDENC_eprint(__LINE__, __FUNCTION__, STR, ##ARG)*/
    #define OMX_EPRINT LOGE
#else
    #define OMX_EPRINT(...)
#endif

#ifdef __OMX_TRACE__
    /*#define OMX_PRINT(...) fprintf(stdout, __VA_ARGS__)*/
	#define OMX_PRINT LOGD
#else
    #define OMX_PRINT(...)
#endif

#ifdef __OMX_DPRINT__ 
    #define OMX_DPRINT(STR, ARG...) VIDENC_Log(__FILE__, __LINE__, __FUNCTION__, STR, ##ARG)
#else
    #define OMX_DPRINT(...)
#endif 

#ifdef __VIDENC_UTILS_DPRINT__
    #define VIDENC_UTILS_DPRINT(STR, ARG...) OMX_DPRINT(STR, ##ARG)
#else
    #define VIDENC_UTILS_DPRINT(...)
#endif 

#ifdef __VIDENC_DPRINT__
    #define VIDENC_DPRINT(STR, ARG...) OMX_DPRINT(STR, ##ARG)
#else
    #define VIDENC_DPRINT(...)
#endif 

#ifdef __VIDENC_THREAD_DPRINT__
    #define VIDENC_THREAD_DPRINT(STR, ARG...) OMX_DPRINT(STR, ##ARG)
#else
    #define VIDENC_THREAD_DPRINT(...)
#endif 
#else /* end Linux, start Windows */

#include <oaf_debug.h>
#define VIDENC_EPRINT         printf
#define OMX_PRINT             VIDENC_DPRINT
#define OMX_EPRINT            VIDENC_EPRINT
#define VIDENC_THREAD_DPRINT  VIDENC_DPRINT
#define VIDENC_UTILS_DPRINT   VIDENC_DPRINT
#define OMX_TRACE             VIDENC_DPRINT
#endif /* end Windows */ 

#endif
