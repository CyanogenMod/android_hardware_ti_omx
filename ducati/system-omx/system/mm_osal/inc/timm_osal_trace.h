/* ====================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 * (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied.
 * ==================================================================== */
/*
*  @file timm_osal_trace.h
*  The timm_osal_types header file defines the primative osal type definitions.
*  @path
*
*/
/* -------------------------------------------------------------------------- */
/* =========================================================================
 *!
 *! Revision History
 *! ===================================
 *! 0.1: Created the first draft version, ksrini@ti.com
 * ========================================================================= */

#ifndef _TIMM_OSAL_TRACES_H_
#define _TIMM_OSAL_TRACES_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
* Traces
*******************************************************************************/

#ifdef TIMM_OSAL_LOG_OVER_TTIF
#  include <string.h>
	void ttif_trace_fprintf( int id, unsigned int mask, const char *format, ...);
	static int __get_trace_id(char *file) {
	  int ttif_trace_open(const char *name);
      static int trace_id = -1;
      if( trace_id == -1 ) {
          const char *name = strrchr( file, '/');
          if(!name) name = strrchr( file, '\\');
          if(!name) name = file;
          else      name++;
          trace_id = ttif_trace_open(name);
      }
      return trace_id;
   }
#  define TIMM_OSAL_TraceFunction(format, args...) ttif_trace_fprintf(__get_trace_id(__FILE__), 1, format, ##args )
#  define TIMM_OSAL_DEBUG_TRACE_DETAIL 0
#else
void TIMM_OSAL_TraceFunction (char *format, ...);
#endif

#define TIMM_OSAL_Debug(ARGS...)    TIMM_OSAL_TraceFunction(##ARGS)

/******************************************************************************
* Debug Trace defines
******************************************************************************/
/**
* The OSAL debug trace detail can be set at compile time by defining the flag
* TIMM_OSAL_DEBUG_TRACE_DETAIL=<Details>
* detail - 0 - no detail
*          1 - function name
*          2 - function name, line number
* Prefix is added to every debug trace message
*/
#ifndef TIMM_OSAL_DEBUG_TRACE_DETAIL
#define TIMM_OSAL_DEBUG_TRACE_DETAIL 2
#endif

#if ( TIMM_OSAL_DEBUG_TRACE_DETAIL > 0 )
#define DL1 "%s:"
#define DR1 ,__FILE__
#else
#define DL1
#define DR1
#endif
#if ( TIMM_OSAL_DEBUG_TRACE_DETAIL > 1 )
#define DL2 "[%d]:"
#define DR2 ,__LINE__
#else
#define DL2
#define DR2
#endif


typedef enum TIMM_OSAL_TRACEGRP_TYPE
{
  TIMM_OSAL_TRACEGRP_SYSTEM=1,
  TIMM_OSAL_TRACEGRP_OMXBASE=(1<<1),
  TIMM_OSAL_TRACEGRP_DOMX=(1<<2),
  TIMM_OSAL_TRACEGRP_OMXVIDEOENC=(1<<3),
  TIMM_OSAL_TRACEGRP_OMXVIDEODEC=(1<<4),
  TIMM_OSAL_TRACEGRP_OMXCAM=(1<<5),
  TIMM_OSAL_TRACEGRP_OMXIMGDEC=(1<<6),
  TIMM_OSAL_TRACEGRP_DRIVERS=(1<<7),
  TIMM_OSAL_TRACEGRP_SIMCOPALGOS=(1<<8)
} TIMM_OSAL_TRACEGRP;


/**
* The OSAL debug trace level can be set at compile time by defining the flag
* TIMM_OSAL_DEBUG_TRACE_LEVEL=<Level>
* The debug levels are:
* Level 0 - No trace
* Level 1 - Error   [Errors]
* Level 2 - Warning [Warnings that are useful to know about]
* Level 3 - Info    [General information]
* Level 4 - Debug   [most-commonly used statement for us developers]
* Level 5 - Trace   ["ENTERING <function>" and "EXITING <function>" statements]
*
* Example: if TIMM_OSAL_DEBUG_TRACE_LEVEL=3, then level 1,2 and 3 traces messages
* are enabled.
*/
#ifndef TIMM_OSAL_DEBUG_TRACE_LEVEL
#define TIMM_OSAL_DEBUG_TRACE_LEVEL 4
#endif

/**
* TIMM_OSAL_Error() -- Fatal errors
*/
#if (TIMM_OSAL_DEBUG_TRACE_LEVEL >= 1)
#define TIMM_OSAL_Error(ARGS,...)   \
                  TIMM_OSAL_TraceFunction("\nERROR:" DL1 DL2 ARGS "\n"DR1 DR2,##__VA_ARGS__)
#else
#define TIMM_OSAL_Error(ARGS,...)
#endif

/**
* TIMM_OSAL_Warning() -- Warnings that are useful to know about
*/
#if (TIMM_OSAL_DEBUG_TRACE_LEVEL >= 2)
#define TIMM_OSAL_Warning(ARGS,...)   \
                TIMM_OSAL_TraceFunction("\nWARNING:" DL1 DL2 ARGS "\n"DR1 DR2,##__VA_ARGS__)
#else
#define TIMM_OSAL_Warning(ARGS,...)
#endif

/**
* TIMM_OSAL_Info() -- general information
*/
#if (TIMM_OSAL_DEBUG_TRACE_LEVEL >= 3)
#define TIMM_OSAL_Info(ARGS,...)   \
                TIMM_OSAL_TraceFunction("\nINFO:" DL1 DL2 ARGS "\n"DR1 DR2, ##__VA_ARGS__)
#else
#define TIMM_OSAL_Info(ARGS,...)
#endif

/**
* TIMM_OSAL_Debug() -- most-commonly used statement for us developers
*/
#if (TIMM_OSAL_DEBUG_TRACE_LEVEL >= 4)
#define TIMM_OSAL_Trace(ARGS,...)    \
                  TIMM_OSAL_TraceFunction("\nTRACE:"DL1 DL2 ARGS "\n" DR1 DR2, ## __VA_ARGS__)
#else
#define TIMM_OSAL_Trace(ARGS,...)
#endif

/**
* TIMM_OSAL_Entering() -- "ENTERING <function>" statements
* TIMM_OSAL_Exiting()  -- "EXITING <function>" statements
*/
#if (TIMM_OSAL_DEBUG_TRACE_LEVEL >= 5)
#define TIMM_OSAL_Entering() TIMM_OSAL_TraceFunction("\nENTERING: %s",__FUNCTION__)
#define TIMM_OSAL_Exiting(ARG)     \
          TIMM_OSAL_TraceFunction("\nEXITING: %s:Returned(%d)",__FUNCTION__,ARG)
#else
#define TIMM_OSAL_Entering()
#define TIMM_OSAL_Exiting(ARG)
#endif

/*******************************************************************************
** New Trace to be used by Applications
*******************************************************************************/

/**
* TIMM_OSAL_ErrorExt() -- Fatal errors
*/
#if (TIMM_OSAL_DEBUG_TRACE_LEVEL >= 1)
#define TIMM_OSAL_ErrorExt(TRACE_GRP, ARGS, ...)   \
                  TIMM_OSAL_TraceFunction("ERROR:"DL1 DL2 ARGS "\n" DR1 DR2, ##__VA_ARGS__)
#else
#define TIMM_OSAL_ErrorExt(TRACE_GRP, ARGS)
#endif

/**
* TIMM_OSAL_WarningExt() -- Warnings that are useful to know about
*/
#if (TIMM_OSAL_DEBUG_TRACE_LEVEL >= 2)
#define TIMM_OSAL_WarningExt(TRACE_GRP, ARGS, ...)  \
                 TIMM_OSAL_TraceFunction("WARNING:"DL1 DL2 ARGS "\n" DR1 DR2, ##__VA_ARGS__)
#else
#define TIMM_OSAL_WarningExt(TRACE_GRP, ARGS,...)
#endif

/**
* TIMM_OSAL_InfoExt() -- general information
*/
#if (TIMM_OSAL_DEBUG_TRACE_LEVEL >= 3)
#define TIMM_OSAL_InfoExt(TRACE_GRP,ARGS, ...)  \
                 TIMM_OSAL_TraceFunction("INFO:"DL1 DL2 ARGS "\n" DR1 DR2, ##__VA_ARGS__)
#else
#define TIMM_OSAL_InfoExt(TRACE_GRP, ARGS, ...)
#endif

/**
* TIMM_OSAL_DebugExt() -- most-commonly used statement for us developers
*/
#if (TIMM_OSAL_DEBUG_TRACE_LEVEL >= 4)
#define TIMM_OSAL_TraceExt(TRACE_GRP, ARGS, ...)  \
                TIMM_OSAL_TraceFunction("TRACE:"DL1 DL2 ARGS "\n" DR1 DR2, ##__VA_ARGS__)
#else
#define TIMM_OSAL_TraceExt(TRACE_GRP, ARGS, ...)
#endif

/**
* TIMM_OSAL_EnteringExt() -- "ENTERING <function>" statements
* TIMM_OSAL_ExitingExt()  -- "EXITING <function>" statements
*/
#if (TIMM_OSAL_DEBUG_TRACE_LEVEL >= 5)
#define TIMM_OSAL_EnteringExt(TRACE_GRP)   TIMM_OSAL_TraceFunction("ENTERING: %s",__FUNCTION__)
#define TIMM_OSAL_ExitingExt(TRACE_GRP,ARG)   \
                TIMM_OSAL_TraceFunction("EXITING: %s:Returned(%d)",__FUNCTION__,ARG)
#else
#define TIMM_OSAL_EnteringExt(TRACE_GRP)
#define TIMM_OSAL_ExitingExt(TRACE_GRP, ARG)
#endif


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TIMM_OSAL_TRACES_H_ */


