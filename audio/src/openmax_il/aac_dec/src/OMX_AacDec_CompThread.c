
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
/* ==============================================================================
*             Texas Instruments OMAP (TM) Platform Software
*  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
*
*  Use of this software is controlled by the terms and conditions found
*  in the license agreement under which this software has been supplied.
* ============================================================================ */
/**
* @file OMX_AacDec_CompThread.c
*
* This file implements OMX Component for AAC Decoder that
* is fully compliant with the OMX Audio specification 1.0.
*
* @path  $(CSLPATH)\
*
* @rev  1.0
*/
/* ----------------------------------------------------------------------------
*!
*! Revision History
*! ===================================
*! 13-Dec-2005 mf:  Initial Version. Change required per OMAPSWxxxxxxxxx
*! to provide _________________.
*!
* ============================================================================= */


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
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <sys/select.h>
#include <memory.h>
#include <fcntl.h>
#include <signal.h>
#endif

#include <dbapi.h>
#include <string.h>
#include <stdio.h>

#include "OMX_AacDec_Utils.h"

/* ================================================================================= * */
/**
* @fn AACDEC_ComponentThread() This is component thread that keeps listening for
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
void* AACDEC_ComponentThread (void* pThreadData)
{
    int status;
    struct timespec tv;
    int fdmax;
    fd_set rfds;
    OMX_U32 nRet;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    AACDEC_COMPONENT_PRIVATE* pComponentPrivate = (AACDEC_COMPONENT_PRIVATE*)pThreadData;
    OMX_COMPONENTTYPE *pHandle = pComponentPrivate->pHandle;

    AACDEC_DPRINT ("%d :: Entering ComponentThread \n",__LINE__);
#ifdef __PERF_INSTRUMENTATION__
    pComponentPrivate->pPERFcomp = PERF_Create(PERF_FOURCC('A', 'C', 'D', '_'),
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
            AACDEC_DPRINT("%d :: Comp Thrd Exiting here...\n",__LINE__);
            goto EXIT;
        }



        if (0 == status) {
            AACDEC_DPRINT("\n\n\n!!!!!  Component Time Out !!!!!!!!!!!! \n");

            if (pComponentPrivate->bExitCompThrd == 1) {
                printf("%d :: Comp Thrd Exiting here...\n",__LINE__);
                goto EXIT;
            }


        } else if (-1 == status) {
            AACDEC_DPRINT ("%d :: Error in Select\n", __LINE__);
            pComponentPrivate->cbInfo.EventHandler (pHandle,
                                                    pHandle->pApplicationPrivate,
                                                    OMX_EventError,
                                                    OMX_ErrorHardware, 
                                                    OMX_TI_ErrorSevere,
                                                    "Error from COmponent Thread in select");
            goto EXIT;

        } else if ((FD_ISSET (pComponentPrivate->dataPipe[0], &rfds))) {
            int ret;
            OMX_BUFFERHEADERTYPE *pBufHeader = NULL;

            AACDEC_DPRINT ("%d :: DATA pipe is set in Component Thread\n",__LINE__);
            ret = read(pComponentPrivate->dataPipe[0], &pBufHeader, sizeof(pBufHeader));
            if (ret == -1) {
                AACDEC_DPRINT ("%d :: Error while reading from the pipe\n",__LINE__);
            }

            eError = AACDEC_HandleDataBuf_FromApp (pBufHeader,pComponentPrivate);
            if (eError != OMX_ErrorNone) {
                AACDEC_DPRINT ("%d :: Error From HandleDataBuf_FromApp\n",__LINE__);
                break;
            }
        } else if (FD_ISSET (pComponentPrivate->cmdPipe[0], &rfds)) {
            AACDEC_DPRINT ("%d :: CMD pipe is set in Component Thread\n",__LINE__);
            nRet = AACDEC_HandleCommand (pComponentPrivate);
            if (nRet == EXIT_COMPONENT_THRD) {
                AACDEC_DPRINT ("Exiting from Component thread\n");
                AACDEC_CleanupInitParams(pHandle);
                AACDEC_STATEPRINT("******************* Component State Set to Loaded\n\n");

                pComponentPrivate->curState = OMX_StateLoaded;
#ifdef __PERF_INSTRUMENTATION__
                PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundaryCleanup);
#endif          
                if(pComponentPrivate->bPreempted==0){
                    pComponentPrivate->cbInfo.EventHandler(
                                                           pHandle, pHandle->pApplicationPrivate,
                                                           OMX_EventCmdComplete,
                                                           OMX_ErrorNone,pComponentPrivate->curState, NULL);
                } else {
                    pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                           pHandle->pApplicationPrivate,
                                                           OMX_EventError,
                                                           OMX_ErrorResourcesLost,
                                                           OMX_TI_ErrorMajor,
                                                           NULL);
                    pComponentPrivate->bPreempted = 0;
                }
            }    
        }  
    }
 EXIT:

    pComponentPrivate->bCompThreadStarted = 0;

#ifdef __PERF_INSTRUMENTATION__
    PERF_Done(pComponentPrivate->pPERFcomp);
#endif
    AACDEC_DPRINT ("%d :: Exiting ComponentThread \n",__LINE__);
    return (void*)OMX_ErrorNone;
}
