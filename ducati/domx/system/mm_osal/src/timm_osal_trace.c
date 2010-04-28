/* ====================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 * (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied.
 * ==================================================================== */
/*
*   @file  timm_osal_trace.c
*   This file contains methods that provides the functionality
*   for logging errors/warings/information/etc.
*
*  @path \
*
*/
/* -------------------------------------------------------------------------- */
/* =========================================================================
 *!
 *! Revision History
 *! ===================================
 *!
 * ========================================================================= */

/******************************************************************************
* Includes
******************************************************************************/

/*#include "typedefs.h"*/
#include <stdarg.h>
#include <stdio.h>
#include "timm_osal_trace.h"

/* ========================================================================== */
/**
* @fn TIMM_OSAL_TraceFunction function
*
*
*/
/* ========================================================================== */
#ifndef TIMM_OSAL_LOG_OVER_TTIF
void TIMM_OSAL_TraceFunction (char *format, ...)
{
	va_list args;
      /* emit trace */
       va_start(args, format);
       vprintf(format, args);
       va_end(args);
	return;
}
#endif
