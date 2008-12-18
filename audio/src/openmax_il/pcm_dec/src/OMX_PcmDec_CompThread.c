
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
/* =============================================================================
*             Texas Instruments OMAP (TM) Platform Software
*  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
*
*  Use of this software is controlled by the terms and conditions found
*  in the license agreement under which this software has been supplied.
* =========================================================================== */
/**
* @file OMX_PcmDec_CompThread.c
*
* This file implements the component thread functionality that receives
* commands and buffers from application and LCML.
*
* @path  $(CSLPATH)\OMAPSW_MPU\linux\audio\src\openmax_il\pcm_dec\src
*
* @rev  1.0
*/
/* ----------------------------------------------------------------------------
*!
*! Revision History
*! ===================================
*! 21-sept-2006 bk: updated some review findings for alpha release
*! 24-Aug-2006 bk: Khronos OpenMAX (TM) 1.0 Conformance tests some more
*! 18-July-2006 bk: Khronos OpenMAX (TM) 1.0 Conformance tests validated for few cases
*! This is newest file
* =========================================================================== */
/* ------compilation control switches -------------------------*/
/****************************************************************
*  INCLUDE FILES
****************************************************************/
/* ----- system and platform files ----------------------------*/

#ifdef UNDER_CE
#include <windows.h>
#include <oaf_osal.h>
#include <omx_core.h>
#include <stdlib.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <sys/select.h>
#include <malloc.h>
#include <memory.h>
#include <fcntl.h>
#include <signal.h>
#endif

#include <dbapi.h>
#include <string.h>
#include <stdio.h>

#include "OMX_PcmDec_Utils.h"
#include "OMX_PcmDec_CompThread.h"

/* ================================================================================= * */
/**
* @fn PCMDEC_ComponentThread() This is component thread that keeps listening for
* commands or event/messages/buffers from application or from LCML.
*
* @param pThreadData This is thread argument.
*
* @pre          None
*
* @post         None
*
*  @return      OMX_ErrorNone = Always
*
*  @see         None
*/
/* ================================================================================ * */
void* PCMDEC_ComponentThread (void* pThreadData)
{
    int status;
    struct timespec  tv;
    int fdmax;
    fd_set rfds;
    OMX_U32 nRet;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    PCMDEC_COMPONENT_PRIVATE* pComponentPrivate = (PCMDEC_COMPONENT_PRIVATE*)pThreadData;
    OMX_COMPONENTTYPE *pHandle = pComponentPrivate->pHandle;

    PCMDEC_DPRINT (":: Entering ComponentThread \n");
#ifdef __PERF_INSTRUMENTATION__
    pComponentPrivate->pPERFcomp = PERF_Create(PERF_FOURCC('P', 'C', 'M', 'D'),
                                               PERF_ModuleComponent |
                                               PERF_ModuleAudioDecode);
#endif

    fdmax = pComponentPrivate->cmdPipe[0];

    if (pComponentPrivate->dataPipe[0] > fdmax) {
        fdmax = pComponentPrivate->dataPipe[0];
    }


    while (1) {
        FD_ZERO (&rfds);
        FD_SET (pComponentPrivate->cmdPipe[0], &rfds);
        FD_SET (pComponentPrivate->dataPipe[0], &rfds);


        tv.tv_sec = 1;
        tv.tv_nsec = 0;

#ifndef UNDER_CE
        sigset_t set;
        sigemptyset (&set);
        sigaddset (&set, SIGALRM);
        status = pselect (fdmax+1, &rfds, NULL, NULL, &tv, &set);
#else
        status = select (fdmax+1, &rfds, NULL, NULL, &tv);
#endif

        if (pComponentPrivate->bExitCompThrd == 1) {
            PCMDEC_DPRINT(":: Comp Thrd Exiting here...\n");
            goto EXIT;
        }



        if (0 == status) {
            PCMDEC_DPRINT("\n\n\n!!!!!  Component Time Out !!!!!!!!!!!! \n");

            if (pComponentPrivate->bExitCompThrd == 1) {
                PCMDEC_EPRINT(":: Comp Thrd Exiting here...\n");
                goto EXIT;
            }


        } else if (-1 == status) {
            PCMDEC_DPRINT (":: Error in Select\n");
            pComponentPrivate->cbInfo.EventHandler (
                             pHandle,pHandle->pApplicationPrivate,
                             OMX_EventError,OMX_ErrorInsufficientResources, 0,
                             "Error from COmponent Thread in select");
            goto EXIT;

        } else if ((FD_ISSET (pComponentPrivate->dataPipe[0], &rfds))) {
/*                   && (pComponentPrivate->curState != OMX_StatePause)) { */
            int ret;
            OMX_BUFFERHEADERTYPE *pBufHeader = NULL;

            if (pComponentPrivate->bExitCompThrd == 1) {
                PCMDEC_EPRINT(":: Comp Thrd Exiting here...\n");
                goto EXIT;
            }

            PCMDEC_DPRINT (":: DATA pipe is set in Component Thread\n");
            ret = read(pComponentPrivate->dataPipe[0], &pBufHeader, sizeof(pBufHeader));
            if (ret == -1) {
                PCMDEC_DPRINT (":: Error while reading from the pipe\n");
            }

            eError = PCMDEC_HandleDataBuf_FromApp (pBufHeader,pComponentPrivate);
            if (eError != OMX_ErrorNone) {
                PCMDEC_DPRINT (":: Error From HandleDataBuf_FromApp\n");
                break;
            }
        }

		else if (FD_ISSET (pComponentPrivate->cmdPipe[0], &rfds)) {

            if (pComponentPrivate->bExitCompThrd == 1) {
                PCMDEC_EPRINT(":: Comp Thrd Exiting here...\n");
                goto EXIT;
            }

            PCMDEC_DPRINT (":: CMD pipe is set in Component Thread\n");

            nRet = PCMDEC_HandleCommand (pComponentPrivate);
            if (nRet == EXIT_COMPONENT_THRD) {
                PCMDEC_DPRINT ("Exiting from Component thread\n");
                PCMDEC_CleanupInitParams(pHandle);
                PCMDEC_STATEPRINT("****************** Component State Set to Loaded\n\n");

                pComponentPrivate->curState = OMX_StateLoaded;
#ifdef __PERF_INSTRUMENTATION__
				PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundaryCleanup);
#endif				
                pComponentPrivate->cbInfo.EventHandler(
                     pHandle, pHandle->pApplicationPrivate,
                     OMX_EventCmdComplete,
                     OMX_ErrorNone,pComponentPrivate->curState, NULL);
            }
        }  

		
    }
EXIT:

    pComponentPrivate->bCompThreadStarted = 0;
#ifdef __PERF_INSTRUMENTATION__
    PERF_Done(pComponentPrivate->pPERFcomp);
#endif

    PCMDEC_DPRINT (":: Exiting ComponentThread \n");
    return (void*)OMX_ErrorNone;
}
