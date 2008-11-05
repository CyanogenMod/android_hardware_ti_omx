
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
* @file OMX_VideoEnc_Debug.c
*
* This file implements OMX Component for MPEG-4 encoder that 
* is fully compliant with the OMX specification 1.5.
*
* @path  $(CSLPATH)\src
*
* @rev  0.1
*/
/* -------------------------------------------------------------------------- */
/* ============================================================================= 
*! 
*! Revision History 
*! ================================================================
*!
*! 24-Jul-2005 mf: Revisions appear in reverse chronological order; 
*! that is, newest first.  The date format is dd-Mon-yyyy.  
* =========================================================================== */


/* ------compilation control switches ----------------------------------------*/
/******************************************************************************
*  INCLUDE FILES                                                 
*******************************************************************************/
/* ----- system and platform files -------------------------------------------*/
#ifdef UNDER_CE
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/time.h>
    #include <sys/types.h>
    #include <sys/ioctl.h>
    #include <sys/select.h>
    #include <string.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include <stdarg.h> 
#endif

/*------- Program Header Files -----------------------------------------------*/
#include "OMX_VideoEnc_Debug.h"

/******************************************************************************
*  EXTERNAL REFERENCES NOTE : only use if not found in header file
*******************************************************************************/
/*--------data declarations --------------------------------------------------*/

/*--------function prototypes ------------------------------------------------*/

/******************************************************************************
*  PUBLIC DECLARATIONS Defined here, used elsewhere
*******************************************************************************/
/*--------data declarations --------------------------------------------------*/

/*--------function prototypes ------------------------------------------------*/

/******************************************************************************
*  PRIVATE DECLARATIONS Defined here, used only here
*******************************************************************************/
/*--------data declarations --------------------------------------------------*/

/*--------macro definitions --------------------------------------------------*/

/*--------function prototypes ------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  * VIDENC_Log() 
  *
  * 
  *
  * @param 
  *
  * @retval 
  *         
  **/
/*----------------------------------------------------------------------------*/
void VIDENC_Log(const char *szFileName, int iLineNum, const char *szFunctionName, const char *strFormat, ...)
{
#ifndef UNDER_CE 
    va_list list;
        
    if (strcmp(szFileName, "OMX_VideoEncoder.c")==0) {
        fprintf(stdout, "%s", DBG_TEXT_GREEN);
    }
    else if (strcmp(szFileName, "OMX_VideoEnc_Thread.c")==0) {
        fprintf(stdout, "%s", DBG_TEXT_YELLOW);
    }
    else if (strcmp(szFileName, "OMX_VideoEnc_Utils.c")==0) {
        fprintf(stdout, "%s", DBG_TEXT_CYAN);
    }
    else {
        fprintf(stdout, "%s", DBG_TEXT_WHITE);
    }

    fprintf(stdout, "%s::", szFileName);
    fprintf(stdout, "%s", DBG_TEXT_WHITE);
    fprintf(stdout, "%s():%d: ", szFunctionName, iLineNum);
    fprintf(stdout, "%s", DBG_TEXT_WHITE);
    va_start(list, strFormat);
    vfprintf(stdout, strFormat, list);
    va_end(list);
#endif
}
void VIDENC_Trace(const char *szFunctionName, const char *strFormat, ...)
{
#ifndef UNDER_CE 
    va_list list;
        
    fprintf(stdout, "%s: ", szFunctionName);
    fprintf(stdout, "%s", DBG_TEXT_WHITE);
    va_start(list, strFormat);
    vfprintf(stdout, strFormat, list);
    va_end(list);
#endif
}
void VIDENC_eprint(int iLineNum, const char *szFunctionName, const char *strFormat, ...)
{
#ifndef UNDER_CE 
    va_list list;
        
    fprintf(stdout, "ERROR::%s():%d: ", szFunctionName, iLineNum);
    fprintf(stdout, "%s", DBG_TEXT_WHITE);
    va_start(list, strFormat);
    vfprintf(stdout, strFormat, list);
    va_end(list);
#endif
}
