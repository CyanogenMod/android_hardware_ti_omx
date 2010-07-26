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
#include <stdlib.h>
#include "timm_osal_trace.h"

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

#define DEFAULT_TRACE_LEVEL 1

static int trace_level = -1;

/* strip out leading ../ stuff that happens to __FILE__ for out-of-tree builds */
static const char * simplify_path(const char *file)
{
    while (file)
    {
        char c = file[0];
        if ((c != '.') && (c != '/') && (c != '\\'))
            break;
        file++;
    }
    return file;
}

void __TIMM_OSAL_TraceFunction(const __TIMM_OSAL_TRACE_LOCATION *loc, const char *fmt, ...)
{
    if (trace_level == -1)
    {
        char *val = getenv("TIMM_OSAL_DEBUG_TRACE_LEVEL");
        trace_level = val ? strtol(val, NULL, 0) : DEFAULT_TRACE_LEVEL;
    }

    if (trace_level >= loc->level)
    {
        va_list ap;

        va_start(ap, fmt);  /* make ap point to first arg after 'fmt' */

#if ( TIMM_OSAL_DEBUG_TRACE_DETAIL > 1 )
        printf("%s:%d\t%s()\t", simplify_path(loc->file), loc->line, loc->function);
#endif
        vprintf(fmt, ap);

        va_end(ap);
    }
}
