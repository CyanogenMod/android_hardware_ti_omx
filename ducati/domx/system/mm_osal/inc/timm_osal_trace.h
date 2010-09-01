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
extern "C"
{
#endif				/* __cplusplus */

/*******************************************************************************
* Traces
*******************************************************************************/


/******************************************************************************
* Debug Trace defines
******************************************************************************/

	typedef enum TIMM_OSAL_TRACEGRP_TYPE
	{
		TIMM_OSAL_TRACEGRP_SYSTEM = 1,
		TIMM_OSAL_TRACEGRP_OMXBASE = (1 << 1),
		TIMM_OSAL_TRACEGRP_DOMX = (1 << 2),
		TIMM_OSAL_TRACEGRP_OMXVIDEOENC = (1 << 3),
		TIMM_OSAL_TRACEGRP_OMXVIDEODEC = (1 << 4),
		TIMM_OSAL_TRACEGRP_OMXCAM = (1 << 5),
		TIMM_OSAL_TRACEGRP_OMXIMGDEC = (1 << 6),
		TIMM_OSAL_TRACEGRP_DRIVERS = (1 << 7),
		TIMM_OSAL_TRACEGRP_SIMCOPALGOS = (1 << 8)
	} TIMM_OSAL_TRACEGRP;


/**
* The OSAL debug trace level can be set at runtime by defining the environment
* variable TIMM_OSAL_DEBUG_TRACE_LEVEL=<Level>.  The default level is 1
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

/**
 * Information about the trace location/type, passed as a single pointer to
 * internal trace function.  Not part of the public API
 */
	typedef struct
	{
		const char *file;
		const char *function;
		const int line;
		const short level;
		const short tracegrp;	/* TIMM_OSAL_TRACEGRP */
	} __TIMM_OSAL_TRACE_LOCATION;

/**
 * Trace implementation function.  Not part of public API.  Default
 * implementation uses printf(), but you can use LD_PRELOAD to plug in
 * alternative trace system at runtime.
 */
	void __TIMM_OSAL_TraceFunction(const __TIMM_OSAL_TRACE_LOCATION * loc,
	    const char *fmt, ...);

/**
 * Internal trace macro.  Not part of public API.
 */
#define __TIMM_OSAL_Trace(level, tracegrp, fmt, ...)                          \
    do {                                                                      \
        static const __TIMM_OSAL_TRACE_LOCATION loc = {                       \
                __FILE__, __FUNCTION__, __LINE__, (level), (tracegrp)         \
        };                                                                    \
        __TIMM_OSAL_TraceFunction(&loc, fmt"\n", ##__VA_ARGS__);              \
    } while(0)

/**
* TIMM_OSAL_Error() -- Fatal errors
*/
#define TIMM_OSAL_Error(fmt,...)  TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_SYSTEM, fmt, ##__VA_ARGS__)

/**
* TIMM_OSAL_Warning() -- Warnings that are useful to know about
*/
#define TIMM_OSAL_Warning(fmt,...)  TIMM_OSAL_WarningExt(TIMM_OSAL_TRACEGRP_SYSTEM, fmt, ##__VA_ARGS__)

/**
* TIMM_OSAL_Info() -- general information
*/
#define TIMM_OSAL_Info(fmt,...)  TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_SYSTEM, fmt, ##__VA_ARGS__)

/**
* TIMM_OSAL_Debug() -- debug traces, most-commonly useful for developers
*/
#define TIMM_OSAL_Debug(fmt,...)  TIMM_OSAL_DebugExt(TIMM_OSAL_TRACEGRP_SYSTEM, fmt, ##__VA_ARGS__)

/**
* TIMM_OSAL_Entering() -- "ENTERING <function>" statements
* TIMM_OSAL_Exiting()  -- "EXITING <function>" statements
*/
#define TIMM_OSAL_Entering(fmt,...)  TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_SYSTEM, fmt, ##__VA_ARGS__)
#define TIMM_OSAL_Exiting(fmt,...)  TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_SYSTEM, fmt, ##__VA_ARGS__)

/*******************************************************************************
** New Trace to be used by Applications
*******************************************************************************/

/**
* TIMM_OSAL_ErrorExt() -- Fatal errors
*/
#define TIMM_OSAL_ErrorExt(tracegrp, fmt, ...)  __TIMM_OSAL_Trace(1, tracegrp, "ERROR: "fmt, ##__VA_ARGS__)

/**
* TIMM_OSAL_WarningExt() -- Warnings that are useful to know about
*/
#define TIMM_OSAL_WarningExt(tracegrp, fmt, ...)  __TIMM_OSAL_Trace(2, tracegrp, "WARNING: "fmt, ##__VA_ARGS__)

/**
* TIMM_OSAL_InfoExt() -- general information
*/
#define TIMM_OSAL_InfoExt(tracegrp, fmt, ...)  __TIMM_OSAL_Trace(3, tracegrp, "INFO: "fmt, ##__VA_ARGS__)

/**
* TIMM_OSAL_DebugExt() -- most-commonly used statement for us developers
*/
#define TIMM_OSAL_DebugExt(tracegrp, fmt, ...)  __TIMM_OSAL_Trace(4, tracegrp, "TRACE: "fmt, ##__VA_ARGS__)

/**
* TIMM_OSAL_EnteringExt() -- "ENTERING <function>" statements
* TIMM_OSAL_ExitingExt()  -- "EXITING <function>" statements
*/
#define TIMM_OSAL_EnteringExt(tracegrp, fmt, ...)  __TIMM_OSAL_Trace(5, tracegrp, "ENTER: "fmt, ##__VA_ARGS__)
#define TIMM_OSAL_ExitingExt(tracegrp, fmt, ...)  __TIMM_OSAL_Trace(5, tracegrp, "EXIT: "fmt, ##__VA_ARGS__)


#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif				/* _TIMM_OSAL_TRACES_H_ */
