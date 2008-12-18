
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
* @file OMX_AacDec_Utils.c
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
*!
*! 17-Aug-2006 mf:
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
#include <malloc.h>
#include <memory.h>
#include <fcntl.h>
#endif

#include <dbapi.h>
#include <string.h>
#include <stdio.h>

/*------- Program Header Files -----------------------------------------------*/
#include "LCML_DspCodec.h"
#include "OMX_AacDec_Utils.h"
#include "Aacdecsocket_ti.h"
#include "decode_common_ti.h"
#include "usn.h"

#ifdef RESOURCE_MANAGER_ENABLED
#include <ResourceManagerProxyAPI.h>
#endif

#ifdef UNDER_CE
#define HASHINGENABLE 1
#endif

#ifdef AACDEC_DEBUGMEM
void *arr[500];
int lines[500];
int bytes[500];
char file[500][50];

void * mymalloc(int line, char *s, int size);
int myfree(void *dp, int line, char *s);
#endif
/* ================================================================================= * */
/**
* @fn AACDEC_Fill_LCMLInitParams() fills the LCML initialization structure.
*
* @param pHandle This is component handle allocated by the OMX core.
*
* @param plcml_Init This structure is filled and sent to LCML.
*
* @pre          None
*
* @post         None
*
*  @return      OMX_ErrorNone = Successful Inirialization of the LCML struct.
*               OMX_ErrorInsufficientResources = Not enough memory
*
*  @see         None
*/
/* ================================================================================ * */
OMX_ERRORTYPE AACDEC_Fill_LCMLInitParams(OMX_HANDLETYPE pComponent,
                                  LCML_DSP *plcml_Init, OMX_U16 arr[])
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 nIpBuf,nIpBufSize,nOpBuf,nOpBufSize;
    OMX_U16 i;
    OMX_BUFFERHEADERTYPE *pTemp;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    AACDEC_COMPONENT_PRIVATE *pComponentPrivate = (AACDEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;
    AACD_LCML_BUFHEADERTYPE *pTemp_lcml;
    OMX_U32 size_lcml;
    char *pTemp_char = NULL;
    char *ptr;
    pComponentPrivate->nRuntimeInputBuffers = 0;
    pComponentPrivate->nRuntimeOutputBuffers = 0;

    AACDEC_DPRINT("%d:::pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bPopulated = %d\n",
                  __LINE__,pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bPopulated);
    AACDEC_DPRINT("%d:::pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bEnabled = %d\n",
                  __LINE__,pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bEnabled);
    AACDEC_DPRINT("%d:::pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bPopulated = %d\n",
                  __LINE__,pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bPopulated);
    AACDEC_DPRINT("%d:::pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bEnabled = %d\n",
                  __LINE__,pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bEnabled);

    pComponentPrivate->strmAttr = NULL;
    nIpBuf = pComponentPrivate->pInputBufferList->numBuffers;
    pComponentPrivate->nRuntimeInputBuffers = nIpBuf;
    nOpBuf = pComponentPrivate->pOutputBufferList->numBuffers;
    pComponentPrivate->nRuntimeOutputBuffers = nOpBuf;
    nIpBufSize = pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->nBufferSize;
    nOpBufSize = pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->nBufferSize;

    AACDEC_DPRINT("Input Buffer Count = %ld\n",nIpBuf);
    AACDEC_DPRINT("Input Buffer Size = %ld\n",nIpBufSize);
    AACDEC_DPRINT("Output Buffer Count = %ld\n",nOpBuf);
    AACDEC_DPRINT("Output Buffer Size = %ld\n",nOpBufSize);

    plcml_Init->In_BufInfo.nBuffers = nIpBuf;
    plcml_Init->In_BufInfo.nSize = nIpBufSize;
    plcml_Init->In_BufInfo.DataTrMethod = DMM_METHOD;
    plcml_Init->Out_BufInfo.nBuffers = nOpBuf;
    plcml_Init->Out_BufInfo.nSize = nOpBufSize;
    plcml_Init->Out_BufInfo.DataTrMethod = DMM_METHOD;

    plcml_Init->NodeInfo.nNumOfDLLs = 3;

    memset(plcml_Init->NodeInfo.AllUUIDs[0].DllName,0, sizeof(plcml_Init->NodeInfo.AllUUIDs[0].DllName));
    memset(plcml_Init->NodeInfo.AllUUIDs[1].DllName,0, sizeof(plcml_Init->NodeInfo.AllUUIDs[1].DllName));
    memset(plcml_Init->NodeInfo.AllUUIDs[2].DllName,0, sizeof(plcml_Init->NodeInfo.AllUUIDs[1].DllName));
    memset(plcml_Init->NodeInfo.AllUUIDs[0].DllName,0, sizeof(plcml_Init->DeviceInfo.AllUUIDs[1].DllName));

    plcml_Init->NodeInfo.AllUUIDs[0].uuid = (struct DSP_UUID*)&MPEG4AACDEC_SN_UUID;
    strcpy ((char*)plcml_Init->NodeInfo.AllUUIDs[0].DllName, AACDEC_DLL_NAME);
    plcml_Init->NodeInfo.AllUUIDs[0].eDllType = DLL_NODEOBJECT;

    plcml_Init->NodeInfo.AllUUIDs[1].uuid = (struct DSP_UUID*)&MPEG4AACDEC_SN_UUID;
    strcpy ((char*)plcml_Init->NodeInfo.AllUUIDs[1].DllName, AACDEC_DLL_NAME);
    plcml_Init->NodeInfo.AllUUIDs[1].eDllType = DLL_DEPENDENT;

    plcml_Init->NodeInfo.AllUUIDs[2].uuid = &USN_TI_UUID;
    strcpy ((char*)plcml_Init->NodeInfo.AllUUIDs[2].DllName, AACDEC_USN_DLL_NAME);
    plcml_Init->NodeInfo.AllUUIDs[2].eDllType = DLL_DEPENDENT;

    plcml_Init->SegID = OMX_AACDEC_DEFAULT_SEGMENT;
    plcml_Init->Timeout = OMX_AACDEC_SN_TIMEOUT;
    plcml_Init->Alignment = 0;
    plcml_Init->Priority = OMX_AACDEC_SN_PRIORITY;
    plcml_Init->ProfileID = -1; /* Previously 0 */

    AACDEC_DPRINT("DLL name0 = %s\n",plcml_Init->NodeInfo.AllUUIDs[0].DllName);
    AACDEC_DPRINT("DLL name1 = %s\n",plcml_Init->NodeInfo.AllUUIDs[1].DllName);
    AACDEC_DPRINT("DLL name2 = %s\n",plcml_Init->NodeInfo.AllUUIDs[2].DllName);

    plcml_Init->DeviceInfo.TypeofDevice = 0; /*Initialisation for F2F mode*/
    plcml_Init->DeviceInfo.TypeofRender = 0;
    if(pComponentPrivate->dasfmode == 1) {

#ifndef DSP_RENDERING_ON
        AACDEC_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                              "Flag DSP_RENDERING_ON Must Be Defined To Use Rendering");
#else

        LCML_STRMATTR *strmAttr;
        AACD_OMX_MALLOC(strmAttr, LCML_STRMATTR);
        pComponentPrivate->strmAttr = strmAttr;
        AACDEC_DPRINT("%d :: AAC DECODER IS RUNNING UNDER DASF MODE \n",__LINE__);

        strmAttr->uSegid = 0;
        strmAttr->uAlignment = 0;
        strmAttr->uTimeout = -1;
        strmAttr->uBufsize = pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->nBufferSize;/*Changed for DASF AAC*/
        AACDEC_DPRINT("%d::strmAttr->uBufsize:%d\n",__LINE__,strmAttr->uBufsize);
        strmAttr->uNumBufs = 2;
        strmAttr->lMode = STRMMODE_PROCCOPY;
        plcml_Init->DeviceInfo.TypeofDevice = 1;
        plcml_Init->DeviceInfo.TypeofRender = 0;
        plcml_Init->DeviceInfo.AllUUIDs[0].uuid = &DCTN_TI_UUID;
        plcml_Init->DeviceInfo.DspStream = strmAttr;
#endif
    }

    if (pComponentPrivate->dasfmode == 0){
        AACDEC_DPRINT("%d :: FILE MODE CREATE PHASE PARAMETERS\n",__LINE__);
        arr[0] = STREAM_COUNT_AACDEC;                        /*Number of Streams*/
        arr[1] = INPUT_PORT_AACDEC;                          /*ID of the Input Stream*/
        arr[2] = 0;                                          /*Type of Input Stream */
        arr[3] = 4;                                          /*Number of buffers for Input Stream*/
        arr[4] = OUTPUT_PORT_AACDEC;                         /*ID of the Output Stream*/
        arr[5] = 0;                                          /*Type of Output Stream */
        arr[6] = 4;                                          /*Number of buffers for Output Stream*/
        arr[7] = 0;                                          /*Decoder Output PCM width is 24-bit or 16-bit*/
        if(pComponentPrivate->nOpBit == 1){
            arr[7] = 1;
        }
        arr[8] = pComponentPrivate->framemode;               /*Frame mode enable */
        arr[9] = END_OF_CR_PHASE_ARGS;
    } else {
        AACDEC_DPRINT("%d :: DASF MODE CREATE PHASE PARAMETERS\n",__LINE__);
        arr[0] = STREAM_COUNT_AACDEC;                         /*Number of Streams*/
        arr[1] = INPUT_PORT_AACDEC;                           /*ID of the Input Stream*/
        arr[2] = 0;                                           /*Type of Input Stream */
        arr[3] = 4;                                           /*Number of buffers for Input Stream*/
        arr[4] = OUTPUT_PORT_AACDEC;                          /*ID of the Output Stream*/
        arr[5] = 2;                                           /*Type of Output Stream */
        arr[6] = 2;                                           /*Number of buffers for Output Stream*/
        arr[7] = 0;                                           /*Decoder Output PCM width is 24-bit or 16-bit*/
        if(pComponentPrivate->nOpBit == 1) {
            arr[7] = 1;
        }
        arr[8] = pComponentPrivate->framemode;          /*Frame mode enable */
        arr[9] = END_OF_CR_PHASE_ARGS;
    }


    plcml_Init->pCrPhArgs = arr;

    AACDEC_DPRINT("%d :: bufAlloced = %lu\n",__LINE__,pComponentPrivate->bufAlloced);
    size_lcml = nIpBuf * sizeof(AACD_LCML_BUFHEADERTYPE);

    AACDEC_OMX_MALLOC_SIZE(ptr,size_lcml,char);
    pTemp_lcml = (AACD_LCML_BUFHEADERTYPE *)ptr;

    pComponentPrivate->pLcmlBufHeader[INPUT_PORT_AACDEC] = pTemp_lcml;

    for (i=0; i<nIpBuf; i++) {
        pTemp = pComponentPrivate->pInputBufferList->pBufHdr[i];
        pTemp->nSize = sizeof(OMX_BUFFERHEADERTYPE);
        pTemp->nAllocLen = nIpBufSize;
        pTemp->nFilledLen = nIpBufSize;
        pTemp->nVersion.s.nVersionMajor = AACDEC_MAJOR_VER;
        pTemp->nVersion.s.nVersionMinor = AACDEC_MINOR_VER;

        pComponentPrivate->nVersion = pTemp->nVersion.nVersion;

        pTemp->pPlatformPrivate = pHandle->pComponentPrivate;
        pTemp->nTickCount = NOT_USED_AACDEC;

        pTemp_lcml->pBufHdr = pTemp;
        pTemp_lcml->eDir = OMX_DirInput;
        pTemp_lcml->pOtherParams[i] = NULL;

        AACDEC_OMX_MALLOC_SIZE(pTemp_lcml->pIpParam,
                             (sizeof(AACDEC_UAlgInBufParamStruct) + DSP_CACHE_ALIGNMENT),
                             AACDEC_UAlgInBufParamStruct);
        pTemp_char = (char*)pTemp_lcml->pIpParam;
        pTemp_char += EXTRA_BYTES;
        pTemp_lcml->pIpParam = (AACDEC_UAlgInBufParamStruct*)pTemp_char;
        pTemp_lcml->pIpParam->bLastBuffer = 0;
        pTemp_lcml->pIpParam->bConcealBuffer = 0;

        pTemp->nFlags = NORMAL_BUFFER_AACDEC;
        ((AACDEC_COMPONENT_PRIVATE *) pTemp->pPlatformPrivate)->pHandle = pHandle;

        AACDEC_DPRINT("%d ::Comp: InBuffHeader[%d] = %p\n", __LINE__, i, pTemp);
        AACDEC_DPRINT("%d ::Comp:  >>>> InputBuffHeader[%d]->pBuffer = %p\n",
                      __LINE__, i, pTemp->pBuffer);
        AACDEC_DPRINT("%d ::Comp: Ip : pTemp_lcml[%d] = %p\n", __LINE__, i, pTemp_lcml);

        pTemp_lcml++;
    }

    size_lcml = nOpBuf * sizeof(AACD_LCML_BUFHEADERTYPE);
    AACDEC_OMX_MALLOC_SIZE(pTemp_lcml,size_lcml,AACD_LCML_BUFHEADERTYPE);
    pComponentPrivate->pLcmlBufHeader[OUTPUT_PORT_AACDEC] = pTemp_lcml;

    for (i=0; i<nOpBuf; i++) {
        pTemp = pComponentPrivate->pOutputBufferList->pBufHdr[i];
        pTemp->nSize = sizeof(OMX_BUFFERHEADERTYPE);

        pTemp->nAllocLen = nOpBufSize;
        pTemp->nVersion.s.nVersionMajor = AACDEC_MAJOR_VER;
        pTemp->nVersion.s.nVersionMinor = AACDEC_MINOR_VER;
        pTemp->pPlatformPrivate = pHandle->pComponentPrivate;
        pTemp->nTickCount = NOT_USED_AACDEC;

        pTemp_lcml->pBufHdr = pTemp;
        pTemp_lcml->eDir = OMX_DirOutput;
        pTemp_lcml->pOtherParams[i] = NULL;

        AACDEC_OMX_MALLOC_SIZE(pTemp_lcml->pOpParam,
                             (sizeof(AACDEC_UAlgOutBufParamStruct) + DSP_CACHE_ALIGNMENT),
                             AACDEC_UAlgOutBufParamStruct);
        pTemp_char = (char*)pTemp_lcml->pOpParam;
        pTemp_char += EXTRA_BYTES;
        pTemp_lcml->pOpParam = (AACDEC_UAlgOutBufParamStruct*)pTemp_char;
        pTemp_lcml->pOpParam->ulFrameCount = DONT_CARE;

        pTemp->nFlags = NORMAL_BUFFER_AACDEC;
        ((AACDEC_COMPONENT_PRIVATE *)pTemp->pPlatformPrivate)->pHandle = pHandle;
        AACDEC_DPRINT("%d ::Comp:  >>>>>>>>>>>>> OutBuffHeader[%d] = %p\n",
                      __LINE__, i, pTemp);
        AACDEC_DPRINT("%d ::Comp:  >>>> OutBuffHeader[%d]->pBuffer = %p\n",
                      __LINE__, i, pTemp->pBuffer);
        AACDEC_DPRINT("%d ::Comp: Op : pTemp_lcml[%d] = %p\n", __LINE__, i, pTemp_lcml);
        pTemp_lcml++;
    }
    pComponentPrivate->bPortDefsAllocated = 1;
    if (pComponentPrivate->aacParams->eAACProfile == OMX_AUDIO_AACObjectMain){
        pComponentPrivate->nProfile = 0;
    } else if (pComponentPrivate->aacParams->eAACProfile == OMX_AUDIO_AACObjectLC){
        pComponentPrivate->nProfile = 1;
    } else if (pComponentPrivate->aacParams->eAACProfile == OMX_AUDIO_AACObjectSSR){
        pComponentPrivate->nProfile = 2;
    } else if (pComponentPrivate->aacParams->eAACProfile == OMX_AUDIO_AACObjectLTP){
        pComponentPrivate->nProfile = 3;
    } else if (pComponentPrivate->aacParams->eAACProfile == OMX_AUDIO_AACObjectHE){
        pComponentPrivate->nProfile = 1;
        pComponentPrivate->SBR = 1;
    } else if (pComponentPrivate->aacParams->eAACProfile == OMX_AUDIO_AACObjectHE_PS){
        pComponentPrivate->nProfile = 1;
        pComponentPrivate->parameteric_stereo = PARAMETRIC_STEREO_AACDEC;
    }

    AACDEC_OMX_MALLOC_SIZE(pComponentPrivate->pParams,(sizeof (USN_AudioCodecParams) + DSP_CACHE_ALIGNMENT),
                           USN_AudioCodecParams);
    pTemp_char = (char*)pComponentPrivate->pParams;
    pTemp_char += EXTRA_BYTES;
    pComponentPrivate->pParams = (USN_AudioCodecParams*)pTemp_char;

    AACDEC_OMX_MALLOC_SIZE(pComponentPrivate->AACDEC_UALGParam,(sizeof (MPEG4AACDEC_UALGParams) + DSP_CACHE_ALIGNMENT),
                           MPEG4AACDEC_UALGParams);
    pTemp_char = (char*)pComponentPrivate->AACDEC_UALGParam;
    pTemp_char += EXTRA_BYTES;
    pComponentPrivate->AACDEC_UALGParam = (MPEG4AACDEC_UALGParams*)pTemp_char;

#ifdef __PERF_INSTRUMENTATION__
    pComponentPrivate->nLcml_nCntIp = 0;
    pComponentPrivate->nLcml_nCntOpReceived = 0;
#endif

    pComponentPrivate->bInitParamsInitialized = 1;

 EXIT:

    return eError;
}

/* ================================================================================= * */
/**
* @fn AacDec_StartCompThread() starts the component thread. This is internal
* function of the component.
*
* @param pHandle This is component handle allocated by the OMX core.
*
* @pre          None
*
* @post         None
*
*  @return      OMX_ErrorNone = Successful Inirialization of the component\n
*               OMX_ErrorInsufficientResources = Not enough memory
*
*  @see         None
*/
/* ================================================================================ * */
OMX_ERRORTYPE AacDec_StartCompThread(OMX_HANDLETYPE pComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    AACDEC_COMPONENT_PRIVATE *pComponentPrivate =
        (AACDEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;
    int nRet = 0;
#ifdef UNDER_CE
    pthread_attr_t attr;
    memset(&attr, 0, sizeof(attr));
    attr.__inheritsched = PTHREAD_EXPLICIT_SCHED;
    attr.__schedparam.__sched_priority = OMX_AUDIO_DECODER_THREAD_PRIORITY;
#endif

    pComponentPrivate->lcml_nOpBuf = 0;
    pComponentPrivate->lcml_nIpBuf = 0;
    pComponentPrivate->app_nBuf = 0;
    pComponentPrivate->num_Op_Issued = 0;
    pComponentPrivate->num_Sent_Ip_Buff = 0;
    pComponentPrivate->num_Reclaimed_Op_Buff = 0;
    pComponentPrivate->bIsEOFSent = 0;

    nRet = pipe (pComponentPrivate->dataPipe);
    if (0 != nRet) {
        AACDEC_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,"Pipe Creation Failed");
    }

    nRet = pipe (pComponentPrivate->cmdPipe);
    if (0 != nRet) {
        AACDEC_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,"Pipe Creation Failed");
    }

    nRet = pipe (pComponentPrivate->cmdDataPipe);
    if (0 != nRet) {
        AACDEC_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,"Pipe Creation Failed");
    }


#ifdef UNDER_CE
    nRet = pthread_create (&(pComponentPrivate->ComponentThread), &attr, AACDEC_ComponentThread, pComponentPrivate);
#else
    nRet = pthread_create (&(pComponentPrivate->ComponentThread), NULL, AACDEC_ComponentThread, pComponentPrivate);
#endif
    if ((0 != nRet) || (!pComponentPrivate->ComponentThread)) {
        AACDEC_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,"Thread Creation Failed");
    }

    pComponentPrivate->bCompThreadStarted = 1;

 EXIT:
    return eError;
}


/* ================================================================================= * */
/**
* @fn AACDEC_FreeCompResources() function frees the component resources.
*
* @param pComponent This is the component handle.
*
* @pre          None
*
* @post         None
*
*  @return      OMX_ErrorNone = Successful Inirialization of the component\n
*               OMX_ErrorHardware = Hardware error has occured.
*
*  @see         None
*/
/* ================================================================================ * */

OMX_ERRORTYPE AACDEC_FreeCompResources(OMX_HANDLETYPE pComponent)
{
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    AACDEC_COMPONENT_PRIVATE *pComponentPrivate = (AACDEC_COMPONENT_PRIVATE *)
        pHandle->pComponentPrivate;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 nIpBuf=0, nOpBuf=0;
    int nRet=0;

    AACDEC_DPRINT("%d:::pComponentPrivate->bPortDefsAllocated = %ld\n", __LINE__,pComponentPrivate->bPortDefsAllocated);
    if (pComponentPrivate->bPortDefsAllocated) {
        nIpBuf = pComponentPrivate->pInputBufferList->numBuffers;
        nOpBuf = pComponentPrivate->pOutputBufferList->numBuffers;
    }
    AACDEC_DPRINT("%d :: Closing pipes.....\n",__LINE__);

    nRet = close (pComponentPrivate->dataPipe[0]);
    if (0 != nRet && OMX_ErrorNone == eError) {
        eError = OMX_ErrorHardware;
    }

    nRet = close (pComponentPrivate->dataPipe[1]);
    if (0 != nRet && OMX_ErrorNone == eError) {
        eError = OMX_ErrorHardware;
    }

    nRet = close (pComponentPrivate->cmdPipe[0]);
    if (0 != nRet && OMX_ErrorNone == eError) {
        eError = OMX_ErrorHardware;
    }

    nRet = close (pComponentPrivate->cmdPipe[1]);
    if (0 != nRet && OMX_ErrorNone == eError) {
        eError = OMX_ErrorHardware;
    }

    nRet = close (pComponentPrivate->cmdDataPipe[0]);
    if (0 != nRet && OMX_ErrorNone == eError) {
        eError = OMX_ErrorHardware;
    }

    nRet = close (pComponentPrivate->cmdDataPipe[1]);
    if (0 != nRet && OMX_ErrorNone == eError) {
        eError = OMX_ErrorHardware;
    }

    if (pComponentPrivate->bPortDefsAllocated) {
        AACDEC_OMX_FREE(pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]);
        AACDEC_OMX_FREE(pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]);
        AACDEC_OMX_FREE(pComponentPrivate->aacParams);
        AACDEC_OMX_FREE(pComponentPrivate->pcmParams);
        AACDEC_OMX_FREE(pComponentPrivate->pCompPort[INPUT_PORT_AACDEC]->pPortFormat);
        AACDEC_OMX_FREE(pComponentPrivate->pCompPort[OUTPUT_PORT_AACDEC]->pPortFormat);
        AACDEC_OMX_FREE(pComponentPrivate->pCompPort[INPUT_PORT_AACDEC]);
        AACDEC_OMX_FREE(pComponentPrivate->pCompPort[OUTPUT_PORT_AACDEC]);
        AACDEC_OMX_FREE(pComponentPrivate->sPortParam);
        AACDEC_OMX_FREE(pComponentPrivate->pPriorityMgmt);
        AACDEC_OMX_FREE(pComponentPrivate->pInputBufferList);
        AACDEC_OMX_FREE(pComponentPrivate->pOutputBufferList);
        AACDEC_OMX_FREE(pComponentPrivate->componentRole);
    }


    pComponentPrivate->bPortDefsAllocated = 0;

#ifndef UNDER_CE
    AACDEC_DPRINT("\n\n FreeCompResources: Destroying threads.\n\n");
    pthread_mutex_destroy(&pComponentPrivate->InLoaded_mutex);
    pthread_cond_destroy(&pComponentPrivate->InLoaded_threshold);

    pthread_mutex_destroy(&pComponentPrivate->InIdle_mutex);
    pthread_cond_destroy(&pComponentPrivate->InIdle_threshold);

    pthread_mutex_destroy(&pComponentPrivate->AlloBuf_mutex);
    pthread_cond_destroy(&pComponentPrivate->AlloBuf_threshold);
#else
    OMX_DestroyEvent(&(pComponentPrivate->InLoaded_event));
    OMX_DestroyEvent(&(pComponentPrivate->InIdle_event));
    OMX_DestroyEvent(&(pComponentPrivate->AlloBuf_event));
#endif
    return eError;
}


/* ================================================================================= * */
/**
* @fn AACDEC_HandleCommand() function handles the command sent by the application.
* All the state transitions, except from nothing to loaded state, of the
* component are done by this function.
*
* @param pComponentPrivate  This is component's private date structure.
*
* @pre          None
*
* @post         None
*
*  @return      OMX_ErrorNone = Successful processing.
*               OMX_ErrorInsufficientResources = Not enough memory
*               OMX_ErrorHardware = Hardware error has occured lile LCML failed
*               to do any said operartion.
*
*  @see         None
*/
/* ================================================================================ * */

OMX_U32 AACDEC_HandleCommand (AACDEC_COMPONENT_PRIVATE *pComponentPrivate)
{
    OMX_U32 i;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *) pComponentPrivate->pHandle;
    OMX_COMMANDTYPE command;
    OMX_STATETYPE commandedState;
    OMX_U32 commandData;
    OMX_HANDLETYPE pLcmlHandle = pComponentPrivate->pLcmlHandle;
    OMX_U32 ret = 0;
    OMX_U16 arr[10];
    OMX_U32 aParam[3] = {0};
    int inputPortFlag = 0;
    int outputPortFlag = 0;
    char *pArgs = "damedesuStr";

#ifdef RESOURCE_MANAGER_ENABLED
    OMX_ERRORTYPE rm_error = OMX_ErrorNone;
#endif

    AACDEC_DPRINT ("%d :: >>> Entering HandleCommand Function\n",__LINE__);
    AACDEC_DPRINT ("%d :: UTIL: pComponentPrivate->curState = %d\n",__LINE__,pComponentPrivate->curState);

    ret = read (pComponentPrivate->cmdPipe[0], &command, sizeof (command));
    if (ret == -1) {
        AACDEC_DPRINT ("%d :: Error in Reading from the Data pipe\n", __LINE__);
        eError = OMX_ErrorHardware;
        pComponentPrivate->cbInfo.EventHandler (pHandle, 
                                                pHandle->pApplicationPrivate,
                                                OMX_EventError, 
                                                eError,
                                                OMX_TI_ErrorSevere,
                                                NULL);
        goto EXIT;
    }
    ret = read (pComponentPrivate->cmdDataPipe[0], &commandData, sizeof (commandData));
    if (ret == -1) {
        AACDEC_DPRINT ("%d :: Error in Reading from the Data pipe\n", __LINE__);
        eError = OMX_ErrorHardware;
        pComponentPrivate->cbInfo.EventHandler (pHandle, 
                                                pHandle->pApplicationPrivate,
                                                OMX_EventError, 
                                                eError,
                                                OMX_TI_ErrorSevere,
                                                NULL);
        goto EXIT;
    }
    AACDEC_DPRINT("---------------------------------------------\n");
    AACDEC_DPRINT("%d :: command = %d\n",__LINE__,command);
    AACDEC_DPRINT("%d :: commandData = %ld\n",__LINE__,commandData);
    AACDEC_DPRINT("---------------------------------------------\n");

#ifdef __PERF_INSTRUMENTATION__
    PERF_ReceivedCommand(pComponentPrivate->pPERFcomp,
                         command,
                         commandData,
                         PERF_ModuleLLMM);
#endif

    if (command == OMX_CommandStateSet) {
        commandedState = (OMX_STATETYPE)commandData;
        if (pComponentPrivate->curState == commandedState) {
            pComponentPrivate->cbInfo.EventHandler ( pHandle,
                                                     pHandle->pApplicationPrivate,
                                                     OMX_EventError,
                                                     OMX_ErrorSameState,
                                                     OMX_TI_ErrorMinor,
                                                     NULL);

            AACDEC_DPRINT("%d :: Error: Same State Given by \
                       Application\n",__LINE__);
        }
        else {
            switch(commandedState) {
            case OMX_StateIdle:
                AACDEC_DPRINT("%d: HandleCommand: Cmd Idle \n",__LINE__);
                if (pComponentPrivate->curState == OMX_StateLoaded || pComponentPrivate->curState == OMX_StateWaitForResources) {
                    LCML_CALLBACKTYPE cb;
                    LCML_DSP *pLcmlDsp;
                    char *p = "damedesuStr";
#ifdef __PERF_INSTRUMENTATION__
                    PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryStart | PERF_BoundarySetup);
#endif
                    if (pComponentPrivate->dasfmode == 1) {
                        pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bEnabled= FALSE;
                        pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bPopulated= FALSE;
                        if(pComponentPrivate->streamID == 0) {
                            AACDEC_DPRINT("**************************************\n");
                            AACDEC_DPRINT(":: Error = OMX_ErrorInsufficientResources\n");
                            AACDEC_DPRINT("**************************************\n");
                            eError = OMX_ErrorInsufficientResources;
                            pComponentPrivate->curState = OMX_StateInvalid;
                            pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                                   pHandle->pApplicationPrivate,
                                                                   OMX_EventError, 
                                                                   eError,
                                                                   OMX_TI_ErrorMajor, 
                                                                   "AM: No Stream ID Available");
                            goto EXIT;
                        }
                    }

                    AACDEC_DPRINT("%d:::pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bPopulated = %d\n",
                                  __LINE__,pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bPopulated);
                    AACDEC_DPRINT("%d:::pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bEnabled = %d\n",
                                  __LINE__,pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bEnabled);
                    AACDEC_DPRINT("%d:::pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bPopulated = %d\n",
                                  __LINE__,pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bPopulated);
                    AACDEC_DPRINT("%d:::pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bEnabled = %d\n",
                                  __LINE__,pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bEnabled);

                    if (pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bPopulated &&
                        pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bEnabled)  {
                        inputPortFlag = 1;
                    }

                    if (!pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bPopulated &&
                        !pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bEnabled) {
                        inputPortFlag = 1;
                    }

                    if (pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bPopulated &&
                        pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bEnabled) {
                        outputPortFlag = 1;
                    }

                    if (!pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bPopulated &&
                        !pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bEnabled) {
                        outputPortFlag = 1;
                    }

                    if (!(inputPortFlag && outputPortFlag)) {
                        pComponentPrivate->InLoaded_readytoidle = 1;
#ifndef UNDER_CE
                        pthread_mutex_lock(&pComponentPrivate->InLoaded_mutex);
                        pthread_cond_wait(&pComponentPrivate->InLoaded_threshold, &pComponentPrivate->InLoaded_mutex);
                        pthread_mutex_unlock(&pComponentPrivate->InLoaded_mutex);
#else
                        OMX_WaitForEvent(&(pComponentPrivate->InLoaded_event));
#endif
                    }

                    pLcmlHandle = (OMX_HANDLETYPE) AACDEC_GetLCMLHandle(pComponentPrivate);
                    if (pLcmlHandle == NULL) {
                        AACDEC_EPRINT(":: LCML Handle is NULL........exiting..\n");
                        pComponentPrivate->curState = OMX_StateInvalid;
                        pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                                pHandle->pApplicationPrivate,
                                                                OMX_EventError,
                                                                OMX_ErrorHardware,
                                                                OMX_TI_ErrorSevere,
                                                                NULL);
                        goto EXIT;
                    }

                    pLcmlDsp = (((LCML_DSP_INTERFACE*)pLcmlHandle)->dspCodec);
                    eError = AACDEC_Fill_LCMLInitParams(pHandle, pLcmlDsp, arr);
                    if(eError != OMX_ErrorNone) {
                        AACDEC_EPRINT(":: Error returned from Fill_LCMLInitParams()\n");
                        pComponentPrivate->curState = OMX_StateInvalid;
                        pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                                pHandle->pApplicationPrivate,
                                                                OMX_EventError,
                                                                eError,
                                                                OMX_TI_ErrorSevere,
                                                                NULL);
                        goto EXIT;
                    }

                    pComponentPrivate->pLcmlHandle = (LCML_DSP_INTERFACE *)pLcmlHandle;
                    cb.LCML_Callback = (void *) AACDEC_LCML_Callback;

#ifndef UNDER_CE
                    eError = LCML_InitMMCodecEx(((LCML_DSP_INTERFACE *)pLcmlHandle)->pCodecinterfacehandle,
                                                p,&pLcmlHandle,(void *)p,&cb, (OMX_STRING)pComponentPrivate->sDeviceString);
                    if (eError != OMX_ErrorNone) {
                        AACDEC_DPRINT("%d :: Error : InitMMCodec failed...>>>>>> \n",__LINE__);
                        goto EXIT;
                    }
#else
                    eError = LCML_InitMMCodec(((LCML_DSP_INTERFACE *)pLcmlHandle)->pCodecinterfacehandle,
                                              p,&pLcmlHandle,(void *)p,&cb);
                    if (eError != OMX_ErrorNone) {
                        AACDEC_DPRINT("%d :: Error : InitMMCodec failed...>>>>>> \n",__LINE__);
                        goto EXIT;
                    }
#endif

#ifdef HASHINGENABLE
                    /* Enable the Hashing Code */
                    eError = LCML_SetHashingState(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle, OMX_TRUE);
                    if (eError != OMX_ErrorNone) {
                        AACDEC_EPRINT("Failed to set Mapping State\n");
                        goto EXIT;
                    }
#endif

#ifdef RESOURCE_MANAGER_ENABLED
				/* Need check the resource with RM */
					pComponentPrivate->rmproxyCallback.RMPROXY_Callback =
											(void *) AACDEC_ResourceManagerCallback;
                    if (pComponentPrivate->curState != OMX_StateWaitForResources){
                        rm_error = RMProxy_NewSendCommand(pHandle,
                                                       RMProxy_RequestResource,
                                                       OMX_AAC_Decoder_COMPONENT,
                                                       AACDEC_CPU_USAGE,
                                                       3456,
                                                       &(pComponentPrivate->rmproxyCallback));
                        if(rm_error == OMX_ErrorNone) {
                            /* resource is available */
#ifdef __PERF_INSTRUMENTATION__
							PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundarySetup);
#endif
                            pComponentPrivate->curState = OMX_StateIdle;
                            rm_error = RMProxy_NewSendCommand(pHandle,
                                                           RMProxy_StateSet,
                                                           OMX_AAC_Decoder_COMPONENT,
                                                           OMX_StateIdle,
                                                           3456,
                                                           NULL);
                            pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                               pHandle->pApplicationPrivate,
                                                               OMX_EventCmdComplete,
                                                               OMX_CommandStateSet,
                                                               pComponentPrivate->curState,
                                                               NULL);
                        }
                        else if(rm_error == OMX_ErrorInsufficientResources) {
                            /* resource is not available, need set state to
                               OMX_StateWaitForResources */
                            pComponentPrivate->curState = OMX_StateWaitForResources;
                            pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                                   pHandle->pApplicationPrivate,
                                                                   OMX_EventCmdComplete,
                                                                   OMX_CommandStateSet,
                                                                   pComponentPrivate->curState,
                                                                   NULL);
                        }
                    }else{
                        rm_error = RMProxy_NewSendCommand(pHandle,
                                                       RMProxy_StateSet,
                                                       OMX_AAC_Decoder_COMPONENT,
                                                       OMX_StateIdle,
                                                       3456,
                                                       NULL);

                        pComponentPrivate->curState = OMX_StateIdle;
                        pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                               pHandle->pApplicationPrivate,
                                                               OMX_EventCmdComplete,
                                                               OMX_CommandStateSet,
                                                               pComponentPrivate->curState,
                                                               NULL);
                    }

#else
#ifdef __PERF_INSTRUMENTATION__
                    PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundarySetup);
#endif
					pComponentPrivate->curState = OMX_StateIdle;
                    pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                           pHandle->pApplicationPrivate,
                                                           OMX_EventCmdComplete,
                                                           OMX_CommandStateSet,
                                                           pComponentPrivate->curState,
                                                           NULL);
#endif

                } else if (pComponentPrivate->curState == OMX_StateExecuting) {
#ifdef __PERF_INSTRUMENTATION__
                    PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundarySteadyState);
#endif
                    AACDEC_DPRINT("%d :: In HandleCommand: Stopping the codec\n",__LINE__);
                    pComponentPrivate->bDspStoppedWhileExecuting = OMX_TRUE;
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               MMCodecControlStop,(void *)pArgs);
                    if(eError != OMX_ErrorNone) {
                        AACDEC_DPRINT(": Error Occurred in Codec Stop..\n");
                        pComponentPrivate->curState = OMX_StateInvalid;
                        pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                                pHandle->pApplicationPrivate,
                                                                OMX_EventError,
                                                                eError,
                                                                OMX_TI_ErrorSevere,
                                                                NULL);
                        goto EXIT;
                    }
#ifdef HASHINGENABLE
                    /*Hashing Change*/
                    pLcmlHandle = (LCML_DSP_INTERFACE*)pComponentPrivate->pLcmlHandle;
                    eError = LCML_FlushHashes(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle);
                    if (eError != OMX_ErrorNone) {
                        AACDEC_EPRINT("Error occurred in Codec mapping flush!\n");
                        break;
                    }
#endif
                } else if (pComponentPrivate->curState == OMX_StatePause) {
                    char *pArgs = "damedesuStr";
#ifdef HASHINGENABLE
                    /*Hashing Change*/
                    pLcmlHandle = (LCML_DSP_INTERFACE*)pComponentPrivate->pLcmlHandle;
                    eError = LCML_FlushHashes(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle);
                    if (eError != OMX_ErrorNone) {
                        AACDEC_EPRINT("Error occurred in Codec mapping flush!\n");
                        break;
                    }
#endif
#ifdef __PERF_INSTRUMENTATION__
                    PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundarySteadyState);
#endif
                    AACDEC_DPRINT("%d :: Comp: Stop Command Received\n",__LINE__);
                    AACDEC_DPRINT("%d: AACDECUTILS::About to call LCML_ControlCodec\n",__LINE__);
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               MMCodecControlStop,(void *)pArgs);
                    if(eError != OMX_ErrorNone) {
                        AACDEC_EPRINT(": Error Occurred in Codec Stop..\n");
                        pComponentPrivate->curState = OMX_StateInvalid;
                        pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                                pHandle->pApplicationPrivate,
                                                                OMX_EventError,
                                                                eError,
                                                                OMX_TI_ErrorSevere,
                                                                NULL);
                        goto EXIT;
                    }
                    AACDEC_STATEPRINT("****************** Component State Set to Idle\n\n");
                    pComponentPrivate->curState = OMX_StateIdle;
#ifdef RESOURCE_MANAGER_ENABLED
                    rm_error = RMProxy_NewSendCommand(pHandle, RMProxy_StateSet, OMX_AAC_Decoder_COMPONENT, OMX_StateIdle, 3456, NULL);
#endif
                    AACDEC_DPRINT ("%d :: The component is stopped\n",__LINE__);
                    pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                            pHandle->pApplicationPrivate,
                                                            OMX_EventCmdComplete,
                                                            OMX_CommandStateSet,
                                                            pComponentPrivate->curState,
                                                            NULL);
                } else {
                    AACDEC_DPRINT("%d: Comp: Sending ErrorNotification: Invalid State\n",__LINE__);
                    pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                           pHandle->pApplicationPrivate,
                                                           OMX_EventError,
                                                           OMX_ErrorIncorrectStateTransition, 
                                                           OMX_TI_ErrorMinor,
                                                           "Invalid State Error");
                }
                break;

            case OMX_StateExecuting:
                AACDEC_DPRINT("%d: HandleCommand: Cmd Executing \n",__LINE__);
                if (pComponentPrivate->curState == OMX_StateIdle) {
                    char *pArgs = "damedesuStr";
                    OMX_U32 pValues[4];
                    OMX_U32 pValues1[4];

                    pComponentPrivate->AACDEC_UALGParam->size = sizeof(MPEG4AACDEC_UALGParams);
                    if(pComponentPrivate->dasfmode == 1) {
                        pComponentPrivate->pParams->unAudioFormat = STEREO_NONINTERLEAVED_STREAM_AACDEC;
                        if(pComponentPrivate->aacParams->nChannels == OMX_AUDIO_ChannelModeMono) {
                            pComponentPrivate->pParams->unAudioFormat = MONO_STREAM_AACDEC;
                            AACDEC_DPRINT("MONO MODE\n");
                        }

                        pComponentPrivate->pParams->ulSamplingFreq = pComponentPrivate->aacParams->nSampleRate;
                        pComponentPrivate->pParams->unUUID = pComponentPrivate->streamID;

                        AACDEC_DPRINT("%d ::pComponentPrivate->pParams->unAudioFormat   = %d\n",
                                      __LINE__,pComponentPrivate->pParams->unAudioFormat);
                        AACDEC_DPRINT("%d ::pComponentPrivate->pParams->ulSamplingFreq  = %ld\n",
                                      __LINE__,pComponentPrivate->aacParams->nSampleRate);

                        pValues[0] = USN_STRMCMD_SETCODECPARAMS;
                        pValues[1] = (OMX_U32)pComponentPrivate->pParams;
                        pValues[2] = sizeof(USN_AudioCodecParams);
                        eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                   EMMCodecControlStrmCtrl,(void *)pValues);
                        if(eError != OMX_ErrorNone) {
                            AACDEC_DPRINT("%d: Error Occurred in Codec StreamControl..\n",__LINE__);
                            pComponentPrivate->curState = OMX_StateInvalid;
                            pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                                    pHandle->pApplicationPrivate,
                                                                    OMX_EventError,
                                                                    eError,
                                                                    OMX_TI_ErrorSevere,
                                                                    NULL);
                            goto EXIT;
                        }
                    }

                    AACDEC_DPRINT ("%d :: pComponentPrivate->nProfile %lu \n",__LINE__,pComponentPrivate->nProfile);
                    AACDEC_DPRINT ("%d :: pComponentPrivate->parameteric_stereo  %lu \n",
                                   __LINE__,pComponentPrivate->parameteric_stereo);
                    AACDEC_DPRINT ("%d :: pComponentPrivate->SBR  %lu \n",__LINE__,pComponentPrivate->SBR);

                    if(pComponentPrivate->parameteric_stereo == PARAMETRIC_STEREO_AACDEC){
                        if(pComponentPrivate->dasfmode == 1){
                            pComponentPrivate->AACDEC_UALGParam->lOutputFormat    = EAUDIO_BLOCK;
                        }else{
                            if(pComponentPrivate->pcmParams->bInterleaved){
                                pComponentPrivate->AACDEC_UALGParam->lOutputFormat    = EAUDIO_INTERLEAVED;
                            }else{
                                pComponentPrivate->AACDEC_UALGParam->lOutputFormat    = EAUDIO_BLOCK;
                            }
                        }
                        pComponentPrivate->AACDEC_UALGParam->iEnablePS        = 1;/*Added for eAAC*/
                        pComponentPrivate->AACDEC_UALGParam->nProfile         = pComponentPrivate->nProfile;
                        pComponentPrivate->AACDEC_UALGParam->lSamplingRateIdx = AACDec_GetSampleRateIndexL(pComponentPrivate->aacParams->nSampleRate);
                        pComponentPrivate->AACDEC_UALGParam->bRawFormat       = 0;
                        if(pComponentPrivate->aacParams->eAACStreamFormat == OMX_AUDIO_AACStreamFormatRAW){
                            pComponentPrivate->AACDEC_UALGParam->bRawFormat       = 1;
                        }
                        pComponentPrivate->AACDEC_UALGParam->DownSampleSbr    = 1;
                    }else{
                        AACDEC_DPRINT("Inside the non parametric stereo\n");
                        if(pComponentPrivate->dasfmode == 1){
                            pComponentPrivate->AACDEC_UALGParam->lOutputFormat    = EAUDIO_BLOCK;
                        }else{
                            if(pComponentPrivate->pcmParams->bInterleaved){
                                pComponentPrivate->AACDEC_UALGParam->lOutputFormat    = EAUDIO_INTERLEAVED;
                            }else{
                                pComponentPrivate->AACDEC_UALGParam->lOutputFormat    = EAUDIO_BLOCK;
                            }
                        }
                        pComponentPrivate->AACDEC_UALGParam->iEnablePS        = 0;
                        pComponentPrivate->AACDEC_UALGParam->nProfile         = pComponentPrivate->nProfile;
                        pComponentPrivate->AACDEC_UALGParam->lSamplingRateIdx = AACDec_GetSampleRateIndexL(pComponentPrivate->aacParams->nSampleRate);
                        pComponentPrivate->AACDEC_UALGParam->bRawFormat       = 0;
                        if(pComponentPrivate->aacParams->eAACStreamFormat == OMX_AUDIO_AACStreamFormatRAW){
                            pComponentPrivate->AACDEC_UALGParam->bRawFormat       = 1;
                        }
                        pComponentPrivate->AACDEC_UALGParam->DownSampleSbr    = 0;
                        if(pComponentPrivate->SBR ){
                            pComponentPrivate->AACDEC_UALGParam->DownSampleSbr    = 1;
                        }
                    }

                    AACDEC_DPRINT("%d::pComponentPrivate->AACDEC_UALGParam->lOutputFormat::%ld\n",
                                  __LINE__,pComponentPrivate->AACDEC_UALGParam->lOutputFormat);
                    AACDEC_DPRINT("%d::pComponentPrivate->AACDEC_UALGParam->DownSampleSbr::%ld\n",
                                  __LINE__,pComponentPrivate->AACDEC_UALGParam->DownSampleSbr);
                    AACDEC_DPRINT("%d::pComponentPrivate->AACDEC_UALGParam->iEnablePS::%ld\n",
                                  __LINE__,pComponentPrivate->AACDEC_UALGParam->iEnablePS);
                    AACDEC_DPRINT("%d::pComponentPrivate->AACDEC_UALGParam->lSamplingRateIdx::%ld\n",
                                  __LINE__,pComponentPrivate->AACDEC_UALGParam->lSamplingRateIdx);
                    AACDEC_DPRINT("%d::pComponentPrivate->SBR::%lu\n",__LINE__,pComponentPrivate->SBR);
                    AACDEC_DPRINT("%d::pComponentPrivate->AACDEC_UALGParam->nProfile::%ld\n",
                                  __LINE__,pComponentPrivate->AACDEC_UALGParam->nProfile);
                    AACDEC_DPRINT("%d::pComponentPrivate->AACDEC_UALGParam->bRawFormat::%ld\n",
                                  __LINE__,pComponentPrivate->AACDEC_UALGParam->bRawFormat);
                    pValues1[0] = IUALG_CMD_SETSTATUS;
                    pValues1[1] = (OMX_U32)pComponentPrivate->AACDEC_UALGParam;
                    pValues1[2] = sizeof(MPEG4AACDEC_UALGParams);

/*  if running under Android (file mode), these values are not available during this state transition.
    We will have to set the codec config parameters after receiving the first buffer that carries
    the config data */
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               EMMCodecControlAlgCtrl,(void *)pValues1);
                    if(eError != OMX_ErrorNone) {
                        AACDEC_DPRINT("%d: Error Occurred in Codec StreamControl..\n",__LINE__);
                        pComponentPrivate->curState = OMX_StateInvalid;
                        pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                               pHandle->pApplicationPrivate,
                                                               OMX_EventError, 
                                                               eError,
                                                               OMX_TI_ErrorSevere, 
                                                               NULL);
                        goto EXIT;
                    }
                    AACDEC_DPRINT("%d :: Algcontrol has been sent to DSP\n",__LINE__);
                    pComponentPrivate->bDspStoppedWhileExecuting = OMX_FALSE;
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               EMMCodecControlStart,(void *)pArgs);
                    if(eError != OMX_ErrorNone) {
                        AACDEC_DPRINT("%d: Error Occurred in Codec Start..\n",__LINE__);
                        goto EXIT;
                    }
                } else if (pComponentPrivate->curState == OMX_StatePause) {
                    char *pArgs = "damedesuStr";
                    AACDEC_DPRINT("%d: UTILS: Resume Command Came from App\n",__LINE__);
                    AACDEC_DPRINT("%d: UTILS::About to call LCML_ControlCodec\n",__LINE__);
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               EMMCodecControlStart,(void *)pArgs);
                    if (eError != OMX_ErrorNone) {
                        AACDEC_DPRINT ("Error While Resuming the codec\n");
                        pComponentPrivate->curState = OMX_StateInvalid;
                        pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                                pHandle->pApplicationPrivate,
                                                                OMX_EventError,
                                                                eError,
                                                                OMX_TI_ErrorSevere,
                                                                NULL);
                        goto EXIT;
                    }

                    for (i=0; i < pComponentPrivate->nNumInputBufPending; i++) {
                        AACDEC_DPRINT("%d pComponentPrivate->pInputBufHdrPending[%lu] = %d\n",__LINE__,i,
                                      pComponentPrivate->pInputBufHdrPending[i] != NULL);
                        if (pComponentPrivate->pInputBufHdrPending[i] != NULL) {
                            AACD_LCML_BUFHEADERTYPE *pLcmlHdr;
                            AACDEC_GetCorresponding_LCMLHeader(pComponentPrivate,pComponentPrivate->pInputBufHdrPending[i]->pBuffer,
                                                               OMX_DirInput, &pLcmlHdr);
                                AACDEC_SetPending(pComponentPrivate,pComponentPrivate->pInputBufHdrPending[i],OMX_DirInput,__LINE__);
                                AACDEC_DPRINT("Calling LCML_QueueBuffer Line %d\n",__LINE__);
                                eError = LCML_QueueBuffer(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                          EMMCodecInputBuffer,
                                                          pComponentPrivate->pInputBufHdrPending[i]->pBuffer,
                                                          pComponentPrivate->pInputBufHdrPending[i]->nAllocLen,
                                                          pComponentPrivate->pInputBufHdrPending[i]->nFilledLen,
                                                          (OMX_U8 *) pLcmlHdr->pIpParam,
                                                          sizeof(AACDEC_UAlgInBufParamStruct),
                                                          NULL);
                        }
                    }
                    pComponentPrivate->nNumInputBufPending = 0;
                    for (i=0; i < pComponentPrivate->nNumOutputBufPending; i++) {
                        AACDEC_DPRINT("%d pComponentPrivate->pOutputBufHdrPending[%lu] = %p\n",__LINE__,i,
                                      pComponentPrivate->pOutputBufHdrPending[i]);
                        if (pComponentPrivate->pOutputBufHdrPending[i] != NULL) {
                            AACD_LCML_BUFHEADERTYPE *pLcmlHdr;
                            AACDEC_GetCorresponding_LCMLHeader(pComponentPrivate,pComponentPrivate->pOutputBufHdrPending[i]->pBuffer,
                                                               OMX_DirOutput, &pLcmlHdr);
                                AACDEC_SetPending(pComponentPrivate,pComponentPrivate->pOutputBufHdrPending[i],OMX_DirOutput,__LINE__);
                                AACDEC_DPRINT("Calling LCML_QueueBuffer Line %d\n",__LINE__);
                                eError = LCML_QueueBuffer(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                      EMMCodecOuputBuffer,
                                                      pComponentPrivate->pOutputBufHdrPending[i]->pBuffer,
                                                      pComponentPrivate->pOutputBufHdrPending[i]->nAllocLen,
                                                      0,
                                                      (OMX_U8 *) pLcmlHdr->pOpParam,
                                                      sizeof(AACDEC_UAlgOutBufParamStruct),
                                                      NULL);
                        }
                    }
                    pComponentPrivate->nNumOutputBufPending = 0;
                } else {
                    pComponentPrivate->cbInfo.EventHandler ( pHandle,
                                                             pHandle->pApplicationPrivate,
                                                             OMX_EventError,
                                                             OMX_ErrorIncorrectStateTransition,
                                                             OMX_TI_ErrorMinor,
                                                             "Invalid State");
                    AACDEC_DPRINT("%d :: Error: Invalid State Given by Application\n",__LINE__);
                    goto EXIT;
                }
                AACDEC_STATEPRINT("****************** Component State Set to Executing\n\n");
#ifdef RESOURCE_MANAGER_ENABLED
                rm_error = RMProxy_NewSendCommand(pHandle, 
                                                  RMProxy_StateSet, 
                                                  OMX_AAC_Decoder_COMPONENT, 
                                                  OMX_StateExecuting, 
                                                  3456, 
                                                  NULL);
#endif
                pComponentPrivate->curState = OMX_StateExecuting;
#ifdef __PERF_INSTRUMENTATION__
                PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryStart | PERF_BoundarySteadyState);
#endif
                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandStateSet,
                                                        pComponentPrivate->curState,
                                                        NULL);
                break;

            case OMX_StateLoaded:
                AACDEC_DPRINT("%d: HandleCommand: Cmd Loaded\n",__LINE__);
                if (pComponentPrivate->curState == OMX_StateWaitForResources ){
                    AACDEC_STATEPRINT("****************** Component State Set to Loaded\n\n");
#ifdef __PERF_INSTRUMENTATION__
                    PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryStart | PERF_BoundaryCleanup);
#endif
                    pComponentPrivate->curState = OMX_StateLoaded;
#ifdef __PERF_INSTRUMENTATION__
                    PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundaryCleanup);
#endif
                    pComponentPrivate->cbInfo.EventHandler ( pHandle,
                                                             pHandle->pApplicationPrivate,
                                                             OMX_EventCmdComplete,
                                                             OMX_CommandStateSet,
                                                             pComponentPrivate->curState,
                                                             NULL);
                    AACDEC_DPRINT("%d :: Tansitioning from WaitFor to Loaded\n",__LINE__);
                    break;
                }

                if (pComponentPrivate->curState != OMX_StateIdle) {
                    pComponentPrivate->cbInfo.EventHandler ( pHandle,
                                                             pHandle->pApplicationPrivate,
                                                             OMX_EventError,
                                                             OMX_ErrorIncorrectStateTransition,
                                                             OMX_TI_ErrorMinor,
                                                             "Incorrect State Transition");
                    AACDEC_DPRINT("%d :: Error: Invalid State Given by \
                       Application\n",__LINE__);
                    goto EXIT;
                }
#ifdef __PERF_INSTRUMENTATION__
                PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryStart | PERF_BoundaryCleanup);
#endif

                AACDEC_DPRINT("%d: AACDECUTILS::Current State = %d\n",__LINE__,pComponentPrivate->curState);
                AACDEC_DPRINT("pComponentPrivate->pInputBufferList->numBuffers = %lu\n",
                              pComponentPrivate->pInputBufferList->numBuffers);
                AACDEC_DPRINT("pComponentPrivate->pOutputBufferList->numBuffers = %lu\n",
                              pComponentPrivate->pOutputBufferList->numBuffers);

                if (pComponentPrivate->pInputBufferList->numBuffers || pComponentPrivate->pOutputBufferList->numBuffers) {
                    pComponentPrivate->InIdle_goingtoloaded = 1;
#ifndef UNDER_CE
                    pthread_mutex_lock(&pComponentPrivate->InIdle_mutex);
                    pthread_cond_wait(&pComponentPrivate->InIdle_threshold, &pComponentPrivate->InIdle_mutex);
                    pthread_mutex_unlock(&pComponentPrivate->InIdle_mutex);
#else
                    OMX_WaitForEvent(&(pComponentPrivate->InIdle_event));
#endif
                    pComponentPrivate->bLoadedCommandPending = OMX_FALSE;
                }

                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                           EMMCodecControlDestroy,(void *)pArgs);
#ifdef __PERF_INSTRUMENTATION__
                PERF_SendingCommand(pComponentPrivate->pPERF, -1, 0, PERF_ModuleComponent);
#endif

                eError = EXIT_COMPONENT_THRD;
                pComponentPrivate->bInitParamsInitialized = 0;
                break;

            case OMX_StatePause:
                AACDEC_DPRINT("%d: HandleCommand: Cmd Pause: Cur State = %d\n",__LINE__,
                              pComponentPrivate->curState);

                if ((pComponentPrivate->curState != OMX_StateExecuting) &&
                    (pComponentPrivate->curState != OMX_StateIdle)) {
                    pComponentPrivate->cbInfo.EventHandler ( pHandle,
                                                             pHandle->pApplicationPrivate,
                                                             OMX_EventError,
                                                             OMX_ErrorIncorrectStateTransition,
                                                             OMX_TI_ErrorMinor,
                                                             "Incorrect State Transition");
                    AACDEC_DPRINT("%d :: Error: Invalid State Given by \
                       Application\n",__LINE__);
                    goto EXIT;
                }
#ifdef __PERF_INSTRUMENTATION__
                PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundarySteadyState);
#endif

                AACDEC_DPRINT("%d: AACDECUTILS::About to call LCML_ControlCodec\n",__LINE__);
                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                           EMMCodecControlPause,
                                           (void *)pArgs);
                if (eError != OMX_ErrorNone) {
                    pComponentPrivate->curState = OMX_StateInvalid;
                    pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                            pHandle->pApplicationPrivate,
                                                            OMX_EventError,
                                                            eError,
                                                            OMX_TI_ErrorSevere,
                                                            NULL);
                    goto EXIT;
                }
                AACDEC_DPRINT("%d :: Component: Codec Is Paused\n",__LINE__);
                break;

            case OMX_StateWaitForResources:
                AACDEC_DPRINT("%d: HandleCommand: Cmd : OMX_StateWaitForResources\n",__LINE__);
                if (pComponentPrivate->curState == OMX_StateLoaded) {
#ifdef RESOURCE_MANAGER_ENABLED
                    rm_error = RMProxy_NewSendCommand(pHandle, 
                                                      RMProxy_StateSet, 
                                                      OMX_AAC_Decoder_COMPONENT, 
                                                      OMX_StateWaitForResources, 
                                                      3456,
                                                      NULL);
#endif
                    pComponentPrivate->curState = OMX_StateWaitForResources;
                    AACDEC_DPRINT("%d: Transitioning from Loaded to OMX_StateWaitForResources\n",__LINE__);
                    pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                            pHandle->pApplicationPrivate,
                                                            OMX_EventCmdComplete,
                                                            OMX_CommandStateSet,
                                                            pComponentPrivate->curState,
                                                            NULL);
                }
                else {
                    pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                            pHandle->pApplicationPrivate,
                                                            OMX_EventError,
                                                            OMX_ErrorIncorrectStateTransition,
                                                            OMX_TI_ErrorMinor,
                                                            NULL);
                }
                break;

            case OMX_StateInvalid:
                AACDEC_DPRINT("%d: HandleCommand: Cmd OMX_StateInvalid:\n",__LINE__);
                if (pComponentPrivate->curState != OMX_StateWaitForResources &&
                    pComponentPrivate->curState != OMX_StateInvalid &&
                    pComponentPrivate->curState != OMX_StateLoaded) {

                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                EMMCodecControlDestroy, (void *)pArgs);
                }

                pComponentPrivate->curState = OMX_StateInvalid;
                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventError,
                                                        OMX_ErrorInvalidState,
                                                        OMX_TI_ErrorSevere,
                                                        NULL);

                AACDEC_CleanupInitParams(pHandle);

                break;

            case OMX_StateMax:
                AACDEC_DPRINT("%d: HandleCommand: Cmd OMX_StateMax::\n",__LINE__);
                break;
            } /* End of Switch */
        }
    }
    else if (command == OMX_CommandMarkBuffer) {
        if(!pComponentPrivate->pMarkBuf) {
            pComponentPrivate->pMarkBuf = (OMX_MARKTYPE *)(commandData);
        }
    } else if (command == OMX_CommandPortDisable) {
        if (!pComponentPrivate->bDisableCommandPending) {
            if(commandData == 0x0){
                for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
                    AACDEC_DPRINT("pComponentPrivate->pInputBufferList->bBufferPending[%lu] = %lu\n",i,
                                  pComponentPrivate->pInputBufferList->bBufferPending[i]);
                    if (AACDEC_IsPending(pComponentPrivate,pComponentPrivate->pInputBufferList->pBufHdr[i],OMX_DirInput)) {
#ifdef __PERF_INSTRUMENTATION__
                        PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                          PREF(pComponentPrivate->pInputBufferList->pBufHdr[i], pBuffer),
                                          0,
                                          PERF_ModuleHLMM);
#endif
                        AACDEC_DPRINT("Forcing EmptyBufferDone\n");
                        pComponentPrivate->cbInfo.EmptyBufferDone (pComponentPrivate->pHandle,
                                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                                   pComponentPrivate->pInputBufferList->pBufHdr[i]);
                        pComponentPrivate->nEmptyBufferDoneCount++;
                    }
                }
                pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bEnabled = OMX_FALSE;
            }
            if(commandData == -1){
                pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bEnabled = OMX_FALSE;
            }
            if(commandData == 0x1 || commandData == -1){
                char *pArgs = "damedesuStr";
                pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bEnabled = OMX_FALSE;
                if (pComponentPrivate->curState == OMX_StateExecuting) {
                    pComponentPrivate->bNoIdleOnStop = OMX_TRUE;
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               MMCodecControlStop,(void *)pArgs);
                }
            }
        }
        AACDEC_DPRINT("commandData = %ld\n",commandData);
        AACDEC_DPRINT("pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bPopulated = %d\n",
                      pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bPopulated);
        AACDEC_DPRINT("pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bPopulated = %d\n",
                      pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bPopulated);
        if(commandData == 0x0) {
            if(!pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bPopulated){

                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandPortDisable,
                                                        INPUT_PORT_AACDEC,
                                                        NULL);
                pComponentPrivate->bDisableCommandPending = 0;
            }
            else {
                pComponentPrivate->bDisableCommandPending = 1;
                pComponentPrivate->bDisableCommandParam = commandData;
            }
        }

        if(commandData == 0x1) {
            if (!pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bPopulated){

                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandPortDisable,
                                                        OUTPUT_PORT_AACDEC,
                                                        NULL);
                pComponentPrivate->bDisableCommandPending = 0;
            }
            else {
                pComponentPrivate->bDisableCommandPending = 1;
                pComponentPrivate->bDisableCommandParam = commandData;
            }
        }

        if(commandData == -1) {
            if (!pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bPopulated &&
                !pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bPopulated){

                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandPortDisable,
                                                        INPUT_PORT_AACDEC,
                                                        NULL);

                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandPortDisable,
                                                        OUTPUT_PORT_AACDEC,
                                                        NULL);
                pComponentPrivate->bDisableCommandPending = 0;
            }
            else {
                pComponentPrivate->bDisableCommandPending = 1;
                pComponentPrivate->bDisableCommandParam = commandData;
            }
        }
    }
    else if (command == OMX_CommandPortEnable) {
        if(!pComponentPrivate->bEnableCommandPending) {
            if(commandData == 0x0 || commandData == -1){

                AACDEC_DPRINT("setting input port to enabled\n");
                pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bEnabled = OMX_TRUE;
                AACDEC_DPRINT("pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bEnabled = %d\n",
                              pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bEnabled);

                if(pComponentPrivate->AlloBuf_waitingsignal){
                    pComponentPrivate->AlloBuf_waitingsignal = 0;
                }
            }
            if(commandData == 0x1 || commandData == -1){
                char *pArgs = "damedesuStr";

                if(pComponentPrivate->AlloBuf_waitingsignal){
                    pComponentPrivate->AlloBuf_waitingsignal = 0;
#ifndef UNDER_CE
                    pthread_mutex_lock(&pComponentPrivate->AlloBuf_mutex);
                    pthread_cond_signal(&pComponentPrivate->AlloBuf_threshold);
                    pthread_mutex_unlock(&pComponentPrivate->AlloBuf_mutex);
#else
                    OMX_SignalEvent(&(pComponentPrivate->AlloBuf_event));
#endif
                }
                if(pComponentPrivate->curState == OMX_StateExecuting) {
                    pComponentPrivate->bDspStoppedWhileExecuting = OMX_FALSE;
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               EMMCodecControlStart,(void *)pArgs);
                }
                AACDEC_DPRINT("setting output port to enabled\n");
                pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bEnabled = OMX_TRUE;
                AACDEC_DPRINT("pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bEnabled = %d\n",
                              pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bEnabled);
            }
        }

        if(commandData == 0x0){
			if (pComponentPrivate->curState == OMX_StateLoaded ||
                pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bPopulated) {
				pComponentPrivate->cbInfo.EventHandler(pHandle,
					                                   pHandle->pApplicationPrivate,
													   OMX_EventCmdComplete,
													   OMX_CommandPortEnable,
													   INPUT_PORT_AACDEC,
													   NULL);
				pComponentPrivate->bEnableCommandPending = 0;
			}
			else {
				pComponentPrivate->bEnableCommandPending = 1;
				pComponentPrivate->bEnableCommandParam = commandData;
			}
		}
        else if(commandData == 0x1) {
			if (pComponentPrivate->curState == OMX_StateLoaded ||
                pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bPopulated){
			    pComponentPrivate->cbInfo.EventHandler( pHandle,
					                                    pHandle->pApplicationPrivate,
													    OMX_EventCmdComplete,
														OMX_CommandPortEnable,
														OUTPUT_PORT_AACDEC,
														NULL);
				pComponentPrivate->bEnableCommandPending = 0;
			}
			else {
				pComponentPrivate->bEnableCommandPending = 1;
				pComponentPrivate->bEnableCommandParam = commandData;
			}
		}
        else if(commandData == -1) {
			if (pComponentPrivate->curState == OMX_StateLoaded ||
			    (pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->bPopulated
			    && pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->bPopulated)){
			    pComponentPrivate->cbInfo.EventHandler(pHandle,
					                                   pHandle->pApplicationPrivate,
													   OMX_EventCmdComplete,
													   OMX_CommandPortEnable,
													   INPUT_PORT_AACDEC,
													   NULL);
			    pComponentPrivate->cbInfo.EventHandler(pHandle,
					                                   pHandle->pApplicationPrivate,
													   OMX_EventCmdComplete,
													   OMX_CommandPortEnable,
													   OUTPUT_PORT_AACDEC,
													   NULL);
				pComponentPrivate->bEnableCommandPending = 0;
                AACDECFill_LCMLInitParamsEx(pHandle);
			}
			else {
				pComponentPrivate->bEnableCommandPending = 1;
				pComponentPrivate->bEnableCommandParam = commandData;
			}
	    }
#ifndef UNDER_CE
                    pthread_mutex_lock(&pComponentPrivate->AlloBuf_mutex);
                    pthread_cond_signal(&pComponentPrivate->AlloBuf_threshold);
                    pthread_mutex_unlock(&pComponentPrivate->AlloBuf_mutex);
#else
                    OMX_SignalEvent(&(pComponentPrivate->AlloBuf_event));
#endif

    }
    else if (command == OMX_CommandFlush) {
        if(commandData == 0x0 || commandData == -1) {
            if (pComponentPrivate->nUnhandledEmptyThisBuffers == 0)  {
                pComponentPrivate->bFlushInputPortCommandPending = OMX_FALSE;
                pComponentPrivate->first_buff = 0;

                aParam[0] = USN_STRMCMD_FLUSH;
                aParam[1] = 0x0;
                aParam[2] = 0x0;

                AACDEC_DPRINT("Flushing input port\n");
                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                       EMMCodecControlStrmCtrl,
                                       (void*)aParam);
                if (eError != OMX_ErrorNone) {
                    goto EXIT;
                }
            }
            else {
                pComponentPrivate->bFlushInputPortCommandPending = OMX_TRUE;
            }
        }
        if(commandData == 0x1 || commandData == -1){
            if (pComponentPrivate->nUnhandledFillThisBuffers == 0)  {
                pComponentPrivate->bFlushOutputPortCommandPending = OMX_FALSE;
                pComponentPrivate->first_buff = 0;

                aParam[0] = USN_STRMCMD_FLUSH;
                aParam[1] = 0x1;
                aParam[2] = 0x0;

                AACDEC_DPRINT("Flushing output port\n");
                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                       EMMCodecControlStrmCtrl,
                                       (void*)aParam);
                if (eError != OMX_ErrorNone) {
                    goto EXIT;
                }
            }
            else {
                pComponentPrivate->bFlushOutputPortCommandPending = OMX_TRUE;
            }
        }
    }
 EXIT:
    return eError;
}


/* ================================================================================= * */
/**
* @fn AACDEC_HandleDataBuf_FromApp() function handles the input and output buffers
* that come from the application. It is not direct function wich gets called by
* the application rather, it gets called eventually.
*
* @param *pBufHeader This is the buffer header that needs to be processed.
*
* @param *pComponentPrivate  This is component's private date structure.
*
* @pre          None
*
* @post         None
*
*  @return      OMX_ErrorNone = Successful processing.
*               OMX_ErrorInsufficientResources = Not enough memory
*               OMX_ErrorHardware = Hardware error has occured lile LCML failed
*               to do any said operartion.
*
*  @see         None
*/
/* ================================================================================ * */

OMX_ERRORTYPE AACDEC_HandleDataBuf_FromApp(OMX_BUFFERHEADERTYPE* pBufHeader,
                                    AACDEC_COMPONENT_PRIVATE *pComponentPrivate)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_DIRTYPE eDir;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDefIn = NULL;
    char *pArgs = "damedesuStr";
    OMX_U32 pValues[4];
    OMX_U32 pValues1[4];
    int iObjectType = 0;
    int iSampleRateIndex = 0;
    OMX_U32 nBitPosition = 0;
    OMX_U8* pHeaderStream = (OMX_U8*)pBufHeader->pBuffer;

    pBufHeader->pPlatformPrivate  = pComponentPrivate;
    eError = AACDEC_GetBufferDirection(pBufHeader, &eDir);
    if (eError != OMX_ErrorNone) {
        AACDEC_DPRINT  ("%d :: The pBufHeader is not found in the list\n",__LINE__);
        goto EXIT;
    }

    if (eDir == OMX_DirInput) {
        pComponentPrivate->nUnhandledEmptyThisBuffers--;
        LCML_DSP_INTERFACE *pLcmlHandle = (LCML_DSP_INTERFACE *)pComponentPrivate->pLcmlHandle;
        AACD_LCML_BUFHEADERTYPE *pLcmlHdr;
        pPortDefIn = pComponentPrivate->pPortDef[OMX_DirInput];
        eError = AACDEC_GetCorresponding_LCMLHeader(pComponentPrivate,pBufHeader->pBuffer, OMX_DirInput, &pLcmlHdr);
        if (eError != OMX_ErrorNone) {
            AACDEC_DPRINT ("%d :: Error: Invalid Buffer Came ...\n",__LINE__);
            goto EXIT;
        }
        AACDEC_DPRINT  ("%d:::pBufHeader->nFilledLen = %d\n",__LINE__, pBufHeader->nFilledLen);
        if (pBufHeader->nFilledLen > 0 || pBufHeader->nFlags == OMX_BUFFERFLAG_EOS) {
            pComponentPrivate->bBypassDSP = 0;
            AACDEC_DPRINT  ("%d:::Calling LCML_QueueBuffer\n",__LINE__);
#ifdef __PERF_INSTRUMENTATION__
            PERF_SendingFrame(pComponentPrivate->pPERFcomp,PREF(pBufHeader,pBuffer),
                              pPortDefIn->nBufferSize,
                              PERF_ModuleCommonLayer);
#endif
            pLcmlHdr->pIpParam->bLastBuffer = 0;
			pLcmlHdr->pIpParam->bConcealBuffer = 0;
			if (pBufHeader->nFlags == OMX_BUFFERFLAG_DATACORRUPT){
				AACDEC_DPRINT ("%d :: bConcealBuffer Is Set Here....\n",__LINE__);
				pLcmlHdr->pIpParam->bConcealBuffer = 1;
			}

            if(pComponentPrivate->SendAfterEOS == 1){
                pComponentPrivate->AACDEC_UALGParam->size = sizeof(MPEG4AACDEC_UALGParams);
                if(pComponentPrivate->dasfmode == 1) {
                    pComponentPrivate->pParams->unAudioFormat = STEREO_NONINTERLEAVED_STREAM_AACDEC;
                    if(pComponentPrivate->aacParams->nChannels == OMX_AUDIO_ChannelModeMono) {
                        pComponentPrivate->pParams->unAudioFormat = MONO_STREAM_AACDEC;
                    }

                    pComponentPrivate->pParams->ulSamplingFreq = pComponentPrivate->aacParams->nSampleRate;
                    pComponentPrivate->pParams->unUUID = pComponentPrivate->streamID;


                    pValues[0] = USN_STRMCMD_SETCODECPARAMS;
                    pValues[1] = (OMX_U32)pComponentPrivate->pParams;
                    pValues[2] = sizeof(USN_AudioCodecParams);
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               EMMCodecControlStrmCtrl,(void *)pValues);
                    if(eError != OMX_ErrorNone) {
                        AACDEC_DPRINT("%d: Error Occurred in Codec StreamControl..\n",__LINE__);
                        pComponentPrivate->curState = OMX_StateInvalid;
                        pComponentPrivate->cbInfo.EventHandler( pComponentPrivate->pHandle,
                                                                pComponentPrivate->pHandle->pApplicationPrivate,
                                                                OMX_EventError,
                                                                eError,
                                                                OMX_TI_ErrorSevere,
                                                                NULL);
                        goto EXIT;
                    }
                }
                
                
#ifdef ANDROID
/*we will parse the config data here when we know how, then set the codec params per case*/

                if (pComponentPrivate->bConfigData){
                    iObjectType = AACDEC_GetBits(&nBitPosition, 5, pHeaderStream, OMX_TRUE);
                    if(iObjectType == OBJECTTYPE_LC)
                    {
                        pComponentPrivate->aacParams->eAACProfile = OMX_AUDIO_AACObjectLC;
                    }
                    else if (iObjectType == OBJECTTYPE_HE)
                    {
                        pComponentPrivate->aacParams->eAACProfile = OMX_AUDIO_AACObjectHE;
                    }
                    else if (iObjectType == OBJECTTYPE_HE2)
                    {
                        pComponentPrivate->aacParams->eAACProfile = OMX_AUDIO_AACObjectHE_PS;
                    }
                    iSampleRateIndex = AACDEC_GetBits(&nBitPosition, 4, pHeaderStream, OMX_TRUE);
                    pComponentPrivate->AACDEC_UALGParam->lSamplingRateIdx = iSampleRateIndex;
                    __android_log_print (ANDROID_LOG_VERBOSE, __FILE__,"Parsing AudioSpecificConfig() %d\n",__FUNCTION__, __LINE__);
                    __android_log_print(ANDROID_LOG_VERBOSE, __FILE__,"%s: profile=%d", __FUNCTION__, iObjectType);                    
                    __android_log_print(ANDROID_LOG_VERBOSE, __FILE__,"%s: iSampleRateIndex=%d", __FUNCTION__, iSampleRateIndex);                    
                    __android_log_print(ANDROID_LOG_VERBOSE, __FILE__,"%s: nFilledLen=%d", __FUNCTION__, pBufHeader->nFilledLen);
                    
                    // we are done with this config buffer, let the application know
                    pComponentPrivate->cbInfo.EmptyBufferDone (pComponentPrivate->pHandle,
                                                               pComponentPrivate->pHandle->pApplicationPrivate,
                                                               pBufHeader);

                    // should also send PortSettingsChanged event here ?
                    
                    pComponentPrivate->bConfigData = 0;
                    goto EXIT;
                }

                pComponentPrivate->AACDEC_UALGParam->bRawFormat = 1;

/* dasf mode should always be false (for now) under Android */
                pComponentPrivate->AACDEC_UALGParam->lOutputFormat = EAUDIO_INTERLEAVED;

                switch(pComponentPrivate->aacParams->eAACProfile){
                    case OMX_AUDIO_AACObjectLTP:
                        pComponentPrivate->nProfile = EProfileLTP;
                        pComponentPrivate->AACDEC_UALGParam->iEnablePS =  0;
                        pComponentPrivate->AACDEC_UALGParam->DownSampleSbr = 0;
                        break;
                    case OMX_AUDIO_AACObjectHE_PS:
                        pComponentPrivate->AACDEC_UALGParam->nProfile = EProfileLC;
                        pComponentPrivate->AACDEC_UALGParam->iEnablePS =  1;
                        pComponentPrivate->AACDEC_UALGParam->DownSampleSbr = 1;
                        pComponentPrivate->parameteric_stereo = PARAMETRIC_STEREO_AACDEC;
                        break;
                    case OMX_AUDIO_AACObjectHE:
                        pComponentPrivate->AACDEC_UALGParam->nProfile = EProfileLC;
                        pComponentPrivate->AACDEC_UALGParam->iEnablePS =  0;
                        pComponentPrivate->AACDEC_UALGParam->DownSampleSbr = 1;
                    case OMX_AUDIO_AACObjectLC:
                    default: /* we will use LC profile as the default, SSR and Main Profiles are not supported */
                        __android_log_print(ANDROID_LOG_VERBOSE, __FILE__,"%s: IN Switch::ObjectLC", __FUNCTION__);
                        pComponentPrivate->nProfile = EProfileLC;
                        pComponentPrivate->AACDEC_UALGParam->iEnablePS =  0;
                        pComponentPrivate->AACDEC_UALGParam->DownSampleSbr = 0;
                        break;
                }

#else

                if(pComponentPrivate->parameteric_stereo == PARAMETRIC_STEREO_AACDEC){
                    if(pComponentPrivate->dasfmode == 1){
                        pComponentPrivate->AACDEC_UALGParam->lOutputFormat    = EAUDIO_BLOCK;
                    }
                    else{
                        pComponentPrivate->AACDEC_UALGParam->lOutputFormat    = EAUDIO_INTERLEAVED;
                    }
                    pComponentPrivate->AACDEC_UALGParam->iEnablePS        = 1;/*Added for eAAC*/
                    pComponentPrivate->AACDEC_UALGParam->nProfile         = pComponentPrivate->aacParams->eAACProfile;
                    pComponentPrivate->AACDEC_UALGParam->lSamplingRateIdx = AACDec_GetSampleRateIndexL(pComponentPrivate->aacParams->nSampleRate);
                    pComponentPrivate->AACDEC_UALGParam->bRawFormat       = 0;
                    if(pComponentPrivate->aacParams->eAACStreamFormat == OMX_AUDIO_AACStreamFormatRAW){
                        pComponentPrivate->AACDEC_UALGParam->bRawFormat       = 1;
                    }
                    pComponentPrivate->AACDEC_UALGParam->DownSampleSbr    = 1;
                }else{

                    if(pComponentPrivate->dasfmode == 1){
                        pComponentPrivate->AACDEC_UALGParam->lOutputFormat    = EAUDIO_BLOCK;
                    }
                    else{
                        pComponentPrivate->AACDEC_UALGParam->lOutputFormat    = EAUDIO_INTERLEAVED;
                    }
                    pComponentPrivate->AACDEC_UALGParam->iEnablePS        = 0;
                    pComponentPrivate->AACDEC_UALGParam->nProfile         = pComponentPrivate->nProfile;
                    pComponentPrivate->AACDEC_UALGParam->lSamplingRateIdx = AACDec_GetSampleRateIndexL(pComponentPrivate->aacParams->nSampleRate);


                    pComponentPrivate->AACDEC_UALGParam->bRawFormat       = 0;
                    if(pComponentPrivate->aacParams->eAACStreamFormat == OMX_AUDIO_AACStreamFormatRAW){
                        pComponentPrivate->AACDEC_UALGParam->bRawFormat       = 1;
                    }
                    pComponentPrivate->AACDEC_UALGParam->DownSampleSbr    = 0;
                    if(pComponentPrivate->SBR ){
                        pComponentPrivate->AACDEC_UALGParam->DownSampleSbr    = 1;
                    }
                }
#endif
                pValues1[0] = IUALG_CMD_SETSTATUS;
                pValues1[1] = (OMX_U32)pComponentPrivate->AACDEC_UALGParam;
                pValues1[2] = sizeof(MPEG4AACDEC_UALGParams);

                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                           EMMCodecControlAlgCtrl,(void *)pValues1);
                if(eError != OMX_ErrorNone) {
                    pComponentPrivate->curState = OMX_StateInvalid;
                    pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                           pComponentPrivate->pHandle->pApplicationPrivate,
                                                           OMX_EventError,
                                                           eError,
                                                           OMX_TI_ErrorSevere,
                                                           NULL);
                    goto EXIT;
                }

                pComponentPrivate->bDspStoppedWhileExecuting = OMX_FALSE;
                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                           EMMCodecControlStart,(void *)pArgs);
                if(eError != OMX_ErrorNone) {
                    AACDEC_DPRINT("%d: Error Occurred in Codec Start..\n",__LINE__);
                    goto EXIT;
                }

                pComponentPrivate->SendAfterEOS = 0;
            }

            if(pBufHeader->nFlags == OMX_BUFFERFLAG_EOS) {
                AACDEC_DPRINT ("%d :: bLastBuffer Is Set Here....\n",__LINE__);
                pLcmlHdr->pIpParam->bLastBuffer = 1;
                pComponentPrivate->bIsEOFSent = 1;
                pBufHeader->nFlags = 0;
            }
/**/
            if(pBufHeader->nFilledLen <= 0){  /* if no data present, assume its the end of the stream */
		pLcmlHdr->pIpParam->bLastBuffer = 1;
                pComponentPrivate->bIsEOFSent = 1;
            }
/**/
            /* Store time stamp information */
            pComponentPrivate->arrBufIndex[pComponentPrivate->IpBufindex] = pBufHeader->nTimeStamp;
            /*Store tick count information*/
            pComponentPrivate->arrBufIndexTick[pComponentPrivate->IpBufindex] = pBufHeader->nTickCount;
            pComponentPrivate->IpBufindex++;
            pComponentPrivate->IpBufindex %= pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->nBufferCountActual;

			if(!pComponentPrivate->framemode){
				if(pComponentPrivate->first_buff == 0){
					pComponentPrivate->first_TS = pBufHeader->nTimeStamp;
					pComponentPrivate->first_buff = 1;
				}
		        }
		

            AACDEC_DPRINT  ("%d Comp:: Sending Filled Input buffer = %p, %p\
                               to LCML\n", __LINE__,pBufHeader,pBufHeader->pBuffer);
            if (pComponentPrivate->curState == OMX_StateExecuting) {
                if (!AACDEC_IsPending(pComponentPrivate,pBufHeader,OMX_DirInput)) {
                    if(!pComponentPrivate->bDspStoppedWhileExecuting) {
                        AACDEC_SetPending(pComponentPrivate,pBufHeader,OMX_DirInput,__LINE__);
                        AACDEC_DPRINT ("Calling LCML_QueueBuffer Line %d\n",__LINE__);
                        eError = LCML_QueueBuffer(pLcmlHandle->pCodecinterfacehandle,
                                                  EMMCodecInputBuffer,
                                                  pBufHeader->pBuffer,
                                                  pBufHeader->nAllocLen,
                                                  pBufHeader->nFilledLen,
                                                  (OMX_U8 *) pLcmlHdr->pIpParam,
                                                  sizeof(AACDEC_UAlgInBufParamStruct),
                                                  NULL);
                        if (eError != OMX_ErrorNone) {
                            AACDEC_DPRINT  ("%d ::Comp: SetBuff: IP: Error Occurred\n",__LINE__);
                            eError = OMX_ErrorHardware;
                            goto EXIT;
                        }
                    }else {
#ifdef __PERF_INSTRUMENTATION__
                        PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                          PREF(pBufHeader, pBuffer),
                                          0,
                                          PERF_ModuleHLMM);
#endif
                        pComponentPrivate->cbInfo.EmptyBufferDone (pComponentPrivate->pHandle,
                                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                                   pBufHeader
                                                                   );
                    }
                    pComponentPrivate->lcml_nCntIp++;
                    pComponentPrivate->lcml_nIpBuf++;
                    pComponentPrivate->num_Sent_Ip_Buff++;
                    AACDEC_DPRINT  ("Sending Input buffer to Codec\n");
                }
            }else if (pComponentPrivate->curState == OMX_StatePause){
                pComponentPrivate->pInputBufHdrPending[pComponentPrivate->nNumInputBufPending++] = pBufHeader;
            }
        } else {
            pComponentPrivate->bBypassDSP = 1;
                AACDEC_DPRINT  ("Forcing EmptyBufferDone\n");
#ifdef __PERF_INSTRUMENTATION__
                PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                  PREF(pComponentPrivate->pInputBufferList->pBufHdr[0], pBuffer),
                                  0,
                                  PERF_ModuleHLMM);
#endif
                pComponentPrivate->cbInfo.EmptyBufferDone (pComponentPrivate->pHandle,
                                                           pComponentPrivate->pHandle->pApplicationPrivate,
                                                           pComponentPrivate->pInputBufferList->pBufHdr[0]
                                                           );
                pComponentPrivate->nEmptyBufferDoneCount++;
        }
        if(pBufHeader->pMarkData){
            AACDEC_DPRINT  ("%d:Detected pBufHeader->pMarkData\n",__LINE__);
            pComponentPrivate->pMarkData = pBufHeader->pMarkData;
            pComponentPrivate->hMarkTargetComponent = pBufHeader->hMarkTargetComponent;
            pComponentPrivate->pOutputBufferList->pBufHdr[0]->pMarkData = pBufHeader->pMarkData;
            pComponentPrivate->pOutputBufferList->pBufHdr[0]->hMarkTargetComponent = pBufHeader->hMarkTargetComponent;
            if(pBufHeader->hMarkTargetComponent == pComponentPrivate->pHandle && pBufHeader->pMarkData){
                pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                                       OMX_EventMark,
                                                       0,
                                                       0,
                                                       pBufHeader->pMarkData);
            }
        }
        if (pComponentPrivate->bFlushInputPortCommandPending) {
            OMX_SendCommand(pComponentPrivate->pHandle,OMX_CommandFlush,0,NULL);
        }
    }else if (eDir == OMX_DirOutput) {
        pComponentPrivate->nUnhandledFillThisBuffers--;
        LCML_DSP_INTERFACE *pLcmlHandle = (LCML_DSP_INTERFACE *)pComponentPrivate->pLcmlHandle;
        AACD_LCML_BUFHEADERTYPE *pLcmlHdr;
        AACDEC_DPRINT ("%d : pComponentPrivate->lcml_nOpBuf = %ld\n",__LINE__,pComponentPrivate->lcml_nOpBuf);
        AACDEC_DPRINT ("%d : pComponentPrivate->lcml_nIpBuf = %ld\n",__LINE__,pComponentPrivate->lcml_nIpBuf);
        eError = AACDEC_GetCorresponding_LCMLHeader(pComponentPrivate,pBufHeader->pBuffer, OMX_DirOutput, &pLcmlHdr);
        if (eError != OMX_ErrorNone) {
            AACDEC_DPRINT ("%d :: Error: Invalid Buffer Came ...\n",__LINE__);
            goto EXIT;
        }
        AACDEC_DPRINT  ("%d:::Calling LCML_QueueBuffer\n",__LINE__);
#ifdef __PERF_INSTRUMENTATION__
        PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                          PREF(pBufHeader,pBuffer),
                          0,
                          PERF_ModuleCommonLayer);
#endif
        if (pComponentPrivate->bBypassDSP == 0) {
            AACDEC_DPRINT  ("%d Comp:: Sending Emptied Output buffer=%p to LCML\n",__LINE__,pBufHeader);
            if (pComponentPrivate->curState == OMX_StateExecuting) {
                AACDEC_DPRINT  ("%d Comp:: in AACDEC UTILS pLcmlHandle->pCodecinterfacehandle= %p\n",
                                __LINE__,pLcmlHandle->pCodecinterfacehandle);
                AACDEC_DPRINT  ("%d Comp:: in AACDEC UTILS EMMCodecOuputBuffer = %u\n",__LINE__,EMMCodecOuputBuffer);
                AACDEC_DPRINT  ("%d Comp:: in AACDEC UTILS pBufHeader->pBuffer = %s\n",__LINE__,pBufHeader->pBuffer);
                AACDEC_DPRINT  ("%d Comp:: in AACDEC UTILS pBufHeader->nAllocLen = %lu\n",__LINE__,pBufHeader->nAllocLen);
                AACDEC_DPRINT ("pComponentPrivate->numPendingBuffers = %lu\n",pComponentPrivate->numPendingBuffers);

                    if (!AACDEC_IsPending(pComponentPrivate,pBufHeader,OMX_DirOutput) &&
                        (pComponentPrivate->numPendingBuffers < pComponentPrivate->pOutputBufferList->numBuffers))  {
                        if (!pComponentPrivate->bDspStoppedWhileExecuting){
							eError = LCML_QueueBuffer(pLcmlHandle->pCodecinterfacehandle,
                                                  EMMCodecOuputBuffer,
                                                  pBufHeader->pBuffer,
                                                  pBufHeader->nAllocLen,
                                                  0,
                                                  (OMX_U8 *) pLcmlHdr->pOpParam,
												  sizeof(AACDEC_UAlgOutBufParamStruct),
                                                  pBufHeader->pBuffer);
							if (eError != OMX_ErrorNone ) {
								AACDEC_DPRINT  ("%d :: Comp:: SetBuff OP: Error Occurred\n", __LINE__);
								eError = OMX_ErrorHardware;
								goto EXIT;
							}

                        pComponentPrivate->lcml_nCntOp++;
                        pComponentPrivate->lcml_nOpBuf++;
                        pComponentPrivate->num_Op_Issued++;
                    }
                }
            }
            else if (pComponentPrivate->curState == OMX_StatePause) {

                pComponentPrivate->pOutputBufHdrPending[pComponentPrivate->nNumOutputBufPending++] = pBufHeader;
            }
        }
        if (pComponentPrivate->bFlushOutputPortCommandPending) {
            OMX_SendCommand( pComponentPrivate->pHandle,
                                  OMX_CommandFlush,
                                  1,NULL);
        }
    }
    else {
        AACDEC_DPRINT ("%d : BufferHeader %p, Buffer %p Unknown ..........\n",__LINE__,pBufHeader, pBufHeader->pBuffer);
        eError = OMX_ErrorBadParameter;
    }
 EXIT:
    AACDEC_DPRINT ("%d : Exiting from  HandleDataBuf_FromApp: %x \n",__LINE__,eError);
    if(eError == OMX_ErrorBadParameter) {
        AACDEC_DPRINT ("%d : Error = OMX_ErrorBadParameter\n",__LINE__);
    }
    return eError;
}

/* ================================================================================= * */
/**
* @fn AACDEC_GetBufferDirection() function determines whether it is input buffer or
* output buffer.
*
* @param *pBufHeader This is pointer to buffer header whose direction needs to
*                    be determined.
*
* @param *eDir  This is output argument which stores the direction of buffer.
*
* @pre          None
*
* @post         None
*
*  @return      OMX_ErrorNone = Successful processing.
*               OMX_ErrorBadParameter = In case of invalid buffer
*
*  @see         None
*/
/* ================================================================================ * */

OMX_ERRORTYPE AACDEC_GetBufferDirection(OMX_BUFFERHEADERTYPE *pBufHeader,
                                                         OMX_DIRTYPE *eDir)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    AACDEC_COMPONENT_PRIVATE *pComponentPrivate = pBufHeader->pPlatformPrivate;
    OMX_U32 nBuf = pComponentPrivate->pInputBufferList->numBuffers;
    OMX_BUFFERHEADERTYPE *pBuf = NULL;
    int flag = 1;
    OMX_U32 i=0;

    for(i=0; i<nBuf; i++) {
        pBuf = pComponentPrivate->pInputBufferList->pBufHdr[i];
        if(pBufHeader == pBuf) {
            *eDir = OMX_DirInput;
            AACDEC_DPRINT ("%d :: Buffer %p is INPUT BUFFER\n",__LINE__, pBufHeader);
            flag = 0;
            goto EXIT;
        }
    }

    nBuf = pComponentPrivate->pOutputBufferList->numBuffers;

    for(i=0; i<nBuf; i++) {
        pBuf = pComponentPrivate->pOutputBufferList->pBufHdr[i];
        if(pBufHeader == pBuf) {
            *eDir = OMX_DirOutput;
            AACDEC_DPRINT ("%d :: Buffer %p is OUTPUT BUFFER\n",__LINE__, pBufHeader);
            flag = 0;
            goto EXIT;
        }
    }

    if (flag == 1) {
        AACDEC_OMX_ERROR_EXIT(eError, OMX_ErrorBadParameter,
                              "Buffer Not Found in List : OMX_ErrorBadParameter");
    }

 EXIT:
    return eError;
}

/* ================================================================================= * */
/**
* @fn AACDEC_LCML_Callback() function is callback which is called by LCML whenever
* there is an even generated for the component.
*
* @param event  This is event that was generated.
*
* @param arg    This has other needed arguments supplied by LCML like handles
*               etc.
*
* @pre          None
*
* @post         None
*
*  @return      OMX_ErrorNone = Successful processing.
*               OMX_ErrorInsufficientResources = Not enough memory
*
*  @see         None
*/
/* ================================================================================ * */
OMX_ERRORTYPE AACDEC_LCML_Callback (TUsnCodecEvent event,void * args [10])
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U8 *pBuffer = args[1];
#ifdef UNDER_CE
    OMX_U8 i;
#endif
    OMX_U32 pValues[4];
    AACD_LCML_BUFHEADERTYPE *pLcmlHdr;
    OMX_COMPONENTTYPE *pHandle = NULL;
    LCML_DSP_INTERFACE *pLcmlHandle;
    AACDEC_COMPONENT_PRIVATE* pComponentPrivate = NULL;
	OMX_U16 i;
	static OMX_U32 TS = 0;

#ifdef RESOURCE_MANAGER_ENABLED
    OMX_ERRORTYPE rm_error = OMX_ErrorNone;
#endif
    static double time_stmp = 0;
  /*  FILE * fOutAAC = NULL;
    FILE * fOutPCM = NULL; */

    pComponentPrivate = (AACDEC_COMPONENT_PRIVATE*)((LCML_DSP_INTERFACE*)args[6])->pComponentPrivate;

    switch(event) {

    case EMMCodecDspError:
        AACDEC_DPRINT ("[LCML CALLBACK EVENT]  EMMCodecDspError\n");
        break;

    case EMMCodecInternalError:
        AACDEC_DPRINT ("[LCML CALLBACK EVENT]  EMMCodecInternalError\n");
        break;

    case EMMCodecInitError:
        AACDEC_DPRINT ("[LCML CALLBACK EVENT]  EMMCodecInitError\n");
        break;

    case EMMCodecDspMessageRecieved:
        AACDEC_DPRINT ("[LCML CALLBACK EVENT]  EMMCodecDspMessageRecieved\n");
        break;

    case EMMCodecBufferProcessed:
        AACDEC_DPRINT ("[LCML CALLBACK EVENT]  EMMCodecBufferProcessed\n");
        break;

    case EMMCodecProcessingStarted:
        AACDEC_DPRINT ("[LCML CALLBACK EVENT]  EMMCodecProcessingStarted\n");
        break;

    case EMMCodecProcessingPaused:
        AACDEC_DPRINT ("[LCML CALLBACK EVENT]  EMMCodecProcessingPaused\n");
        break;

    case EMMCodecProcessingStoped:
        AACDEC_DPRINT ("[LCML CALLBACK EVENT]  EMMCodecProcessingStoped\n");
        break;

    case EMMCodecProcessingEof:
        AACDEC_DPRINT ("[LCML CALLBACK EVENT]  EMMCodecProcessingEof\n");
        break;

    case EMMCodecBufferNotProcessed:
        AACDEC_DPRINT ("[LCML CALLBACK EVENT]  EMMCodecBufferNotProcessed\n");
        break;

    case EMMCodecAlgCtrlAck:
        AACDEC_DPRINT ("[LCML CALLBACK EVENT]  EMMCodecAlgCtrlAck\n");
        break;

    case EMMCodecStrmCtrlAck:
        AACDEC_DPRINT ("[LCML CALLBACK EVENT]  EMMCodecStrmCtrlAck\n");
        break;
    }

    if(event == EMMCodecBufferProcessed){
        if( args[0] == (void *)EMMCodecInputBuffer) {
            AACDEC_DPRINT (" :: Inside the LCML_Callback EMMCodecInputBuffer\n");
            AACDEC_DPRINT  ("%d :: Input: pBuffer = %p\n",__LINE__, pBuffer);
            eError = AACDEC_GetCorresponding_LCMLHeader(pComponentPrivate, pBuffer, OMX_DirInput, &pLcmlHdr);
            if (eError != OMX_ErrorNone) {
                AACDEC_DPRINT ("%d :: Error: Invalid Buffer Came ...\n",__LINE__);
                goto EXIT;
            }
            AACDEC_DPRINT  ("%d :: Input: pLcmlHeader = %p\n",__LINE__, pLcmlHdr);
            AACDEC_DPRINT  ("%d :: Input: pLcmlHdr->eDir = %u\n",__LINE__, pLcmlHdr->eDir);
            AACDEC_DPRINT  ("%d :: Input: *pLcmlHdr->eDir = %u\n",__LINE__, pLcmlHdr->eDir);
            AACDEC_DPRINT  ("%d :: Input: Filled Len = %ld\n",__LINE__, pLcmlHdr->pBufHdr->nFilledLen);
#ifdef __PERF_INSTRUMENTATION__
            PERF_ReceivedFrame(pComponentPrivate->pPERFcomp,
                               PREF(pLcmlHdr->pBufHdr,pBuffer),
                               0,
                               PERF_ModuleCommonLayer);
#endif
            pComponentPrivate->lcml_nCntIpRes++;
            AACDEC_ClearPending(pComponentPrivate,pLcmlHdr->pBufHdr,OMX_DirInput,__LINE__);

#ifdef __PERF_INSTRUMENTATION__
			PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                          PREF(pLcmlHdr->pBufHdr,pBuffer),
                          0,
                          PERF_ModuleHLMM);
#endif
           /* fOutAAC = fopen("captureInput.aac", "a+");
            fwrite(pLcmlHdr->pBufHdr->pBuffer, 1, pLcmlHdr->pBufHdr->nFilledLen, fOutAAC);
            fclose(fOutAAC); */

            pComponentPrivate->cbInfo.EmptyBufferDone (pComponentPrivate->pHandle,
                                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                                       pLcmlHdr->pBufHdr);
            pComponentPrivate->nEmptyBufferDoneCount++;
            pComponentPrivate->lcml_nIpBuf--;
            pComponentPrivate->app_nBuf++;

        } else if (args[0] == (void *)EMMCodecOuputBuffer) {
            AACDEC_DPRINT (" :: Inside the LCML_Callback EMMCodecOuputBuffer\n");
            AACDEC_DPRINT  ("%d :: Output: pBufferr = %p\n",__LINE__, pBuffer);
            if (!AACDEC_IsValid(pComponentPrivate,pBuffer,OMX_DirOutput)) {
#ifdef __PERF_INSTRUMENTATION__
                PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                  pComponentPrivate->pOutputBufferList->pBufHdr[pComponentPrivate->nInvalidFrameCount]->pBuffer,
                                  pComponentPrivate->pOutputBufferList->pBufHdr[pComponentPrivate->nInvalidFrameCount]->nFilledLen,
                                  PERF_ModuleHLMM);
#endif
                pComponentPrivate->cbInfo.FillBufferDone (pComponentPrivate->pHandle,
                                                          pComponentPrivate->pHandle->pApplicationPrivate,
                                                          pComponentPrivate->pOutputBufferList->pBufHdr[pComponentPrivate->nInvalidFrameCount++]
                                                          );
                pComponentPrivate->numPendingBuffers--;
            } else{
                pComponentPrivate->nOutStandingFillDones++;
                eError = AACDEC_GetCorresponding_LCMLHeader(pComponentPrivate, pBuffer, OMX_DirOutput, &pLcmlHdr);
                if (eError != OMX_ErrorNone) {
                    AACDEC_DPRINT("%d :: Error: Invalid Buffer Came ...\n",__LINE__);
                    goto EXIT;
                }
                pLcmlHdr->pBufHdr->nFilledLen = (int)args[8];
                AACDEC_DPRINT  ("%d :: Output: pLcmlHeader = %p\n",__LINE__, pLcmlHdr);
                AACDEC_DPRINT  ("%d :: Output: pLcmlHdr->eDir = %u\n",__LINE__, pLcmlHdr->eDir);
                AACDEC_DPRINT  ("%d :: Output: Filled Len = %ld\n",__LINE__, pLcmlHdr->pBufHdr->nFilledLen);
                AACDEC_DPRINT  ("%d :: Output: pLcmlHeader->pBufHdr = %p\n",__LINE__, pLcmlHdr->pBufHdr);
                pComponentPrivate->lcml_nCntOpReceived++;
#ifdef __PERF_INSTRUMENTATION__
                PERF_ReceivedFrame(pComponentPrivate->pPERFcomp,
                                   PREF(pLcmlHdr->pBufHdr,pBuffer),
                                   PREF(pLcmlHdr->pBufHdr,nFilledLen),
                                   PERF_ModuleCommonLayer);
                pComponentPrivate->nLcml_nCntOpReceived++;
                if ((pComponentPrivate->nLcml_nCntIp >= 1) && (pComponentPrivate->nLcml_nCntOpReceived == 1)) {
                    PERF_Boundary(pComponentPrivate->pPERFcomp,
                                  PERF_BoundaryStart | PERF_BoundarySteadyState);
                }
#endif
                AACDEC_ClearPending(pComponentPrivate,pLcmlHdr->pBufHdr,OMX_DirOutput,__LINE__);
			if (pComponentPrivate->pMarkData) {
				pLcmlHdr->pBufHdr->pMarkData = pComponentPrivate->pMarkData;
				pLcmlHdr->pBufHdr->hMarkTargetComponent = pComponentPrivate->hMarkTargetComponent;
			}
			pComponentPrivate->num_Reclaimed_Op_Buff++;
            if (pComponentPrivate->bIsEOFSent){
				AACDEC_DPRINT  ("%d : UTIL: Adding EOS flag to the output buffer\n",__LINE__);
				pLcmlHdr->pBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
				AACDEC_DPRINT  ("%d : UTIL:: pLcmlHdr->pBufHdr = %p\n",__LINE__,pLcmlHdr->pBufHdr);
				AACDEC_DPRINT  ("%d : UTIL:: pLcmlHdr->pBufHdr->nFlags = %x\n",__LINE__,(int)pLcmlHdr->pBufHdr->nFlags);
				pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventBufferFlag,
                                                   pLcmlHdr->pBufHdr->nOutputPortIndex,
                                                   pLcmlHdr->pBufHdr->nFlags, NULL);
				pComponentPrivate->bIsEOFSent = 0;
				AACDEC_DPRINT  ("%d : UTIL: EOS flag has been propagated\n",__LINE__);
			}

			AACDEC_DPRINT("pLcmlHdr->pBufHdr = 0x%p\n",pLcmlHdr->pBufHdr);

			if(pComponentPrivate->framemode){
				/* Copying time stamp information to output buffer */
				pLcmlHdr->pBufHdr->nTimeStamp = (OMX_TICKS)pComponentPrivate->arrBufIndex[pComponentPrivate->OpBufindex];
			}else{
            if(pComponentPrivate->first_buff == 1){
                pComponentPrivate->first_buff = 2;
                pLcmlHdr->pBufHdr->nTimeStamp = pComponentPrivate->first_TS;
                TS = pLcmlHdr->pBufHdr->nTimeStamp;
            }else{
                if(pComponentPrivate->pcmParams->nChannels == 0) {/* OMX_AUDIO_ChannelModeStereo */
                    time_stmp = pLcmlHdr->pBufHdr->nFilledLen / (2 * (pComponentPrivate->pcmParams->nBitPerSample / 8));
                }else {/* OMX_AUDIO_ChannelModeMono */
                    time_stmp = pLcmlHdr->pBufHdr->nFilledLen / (1 * (pComponentPrivate->pcmParams->nBitPerSample / 8));
                }
                time_stmp = (time_stmp / pComponentPrivate->pcmParams->nSamplingRate) * 1000;
                /* Update time stamp information */
                TS += (OMX_U32)time_stmp;
                pLcmlHdr->pBufHdr->nTimeStamp = TS;
			}
            }

  			/*Copying tick count information to output buffer*/
              pLcmlHdr->pBufHdr->nTickCount = (OMX_U32)pComponentPrivate->arrBufIndexTick[pComponentPrivate->OpBufindex];
              pComponentPrivate->OpBufindex++;
              pComponentPrivate->OpBufindex %= pComponentPrivate->pPortDef[OMX_DirInput]->nBufferCountActual;


#ifdef __PERF_INSTRUMENTATION__
				PERF_SendingBuffer(pComponentPrivate->pPERFcomp,
                               pLcmlHdr->pBufHdr->pBuffer,
                               pLcmlHdr->pBufHdr->nFilledLen,
                               PERF_ModuleHLMM);
#endif

                             /*   fOutPCM = fopen("captureOut.pcm", "a+");
                                fwrite(pLcmlHdr->pBufHdr->pBuffer, 1, pLcmlHdr->pBufHdr->nFilledLen, fOutPCM);
                                fclose(fOutPCM); */

				pComponentPrivate->cbInfo.FillBufferDone (pComponentPrivate->pHandle,
                                                      pComponentPrivate->pHandle->pApplicationPrivate,
                                                      pLcmlHdr->pBufHdr
                                                      );
				pComponentPrivate->nOutStandingFillDones--;
				pComponentPrivate->lcml_nOpBuf--;
				pComponentPrivate->app_nBuf++;
				pComponentPrivate->nFillBufferDoneCount++;
            }
        }
    } else if(event == EMMCodecProcessingStoped) {

        /* If there are any buffers still marked as pending they must have
           been queued after the socket node was stopped */
        for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
            if (pComponentPrivate->pInputBufferList->bBufferPending[i]) {
#ifdef __PERF_INSTRUMENTATION__
                PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                  PREF(pComponentPrivate->pInputBufferList->pBufHdr[i], pBuffer),
                                  0,
                                  PERF_ModuleHLMM);
#endif

                pComponentPrivate->cbInfo.EmptyBufferDone (
                                                           pComponentPrivate->pHandle,
                                                           pComponentPrivate->pHandle->pApplicationPrivate,
                                                           pComponentPrivate->pInputBufferList->pBufHdr[i]
                                                           );
                AACDEC_ClearPending(pComponentPrivate, pComponentPrivate->pInputBufferList->pBufHdr[i], OMX_DirInput, __LINE__);
            }
        }

        for (i=0; i < pComponentPrivate->pOutputBufferList->numBuffers; i++) {

            if (pComponentPrivate->pOutputBufferList->bBufferPending[i]) {
#ifdef __PERF_INSTRUMENTATION__
                PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                  PREF(pComponentPrivate->pOutputBufferList->pBufHdr[i],pBuffer),
                                  PREF(pComponentPrivate->pOutputBufferList->pBufHdr[i],nFilledLen),
                                  PERF_ModuleHLMM);
#endif

                pComponentPrivate->cbInfo.FillBufferDone (
                                                          pComponentPrivate->pHandle,
                                                          pComponentPrivate->pHandle->pApplicationPrivate,
                                                          pComponentPrivate->pOutputBufferList->pBufHdr[i]
                                                          );

                AACDEC_ClearPending(pComponentPrivate, pComponentPrivate->pOutputBufferList->pBufHdr[i], OMX_DirOutput, __LINE__);
            }
        }

        if (!pComponentPrivate->bNoIdleOnStop) {
            pComponentPrivate->curState = OMX_StateIdle;

#ifdef RESOURCE_MANAGER_ENABLED
            rm_error = RMProxy_NewSendCommand(pComponentPrivate->pHandle, RMProxy_StateSet, OMX_AAC_Decoder_COMPONENT, OMX_StateIdle, 3456, NULL);
#endif
            if (pComponentPrivate->bPreempted == 0) {
                pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete,
                                                       OMX_CommandStateSet,
                                                       pComponentPrivate->curState,
                                                       NULL);
            }
            else {
                pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                                       OMX_EventError,
                                                       OMX_ErrorResourcesPreempted,
                                                       OMX_TI_ErrorMajor,
                                                       NULL);
            }
        }else {
            pComponentPrivate->bDspStoppedWhileExecuting = OMX_TRUE;
            pComponentPrivate->bNoIdleOnStop = OMX_FALSE;
        }
    } else if(event == EMMCodecAlgCtrlAck) {
        AACDEC_DPRINT ("GOT MESSAGE USN_DSPACK_ALGCTRL \n");
    } else if (event == EMMCodecDspError) {
        AACDEC_DPRINT("%d :: commandedState  = %d\n",__LINE__,(int)args[0]);
        AACDEC_DPRINT("%d :: arg4 = %d\n",__LINE__,(int)args[4]);
        AACDEC_DPRINT("%d :: arg5 = %d\n",__LINE__,(int)args[5]);
        AACDEC_DPRINT("%d ::UTIL: EMMCodecDspError Here\n",__LINE__);
#ifdef _ERROR_PROPAGATION__
        /* Cheking for MMU_fault */
        if((args[4] == (void*)USN_ERR_UNKNOWN_MSG) && (args[5] == (void*)NULL)) {
            AACDEC_EPRINT("\n%d :: UTIL: MMU_Fault \n",__LINE__);
            pComponentPrivate->bIsInvalidState = OMX_TRUE;
            pComponentPrivate->curState = OMX_StateInvalid;
            pHandle = pComponentPrivate->pHandle;
            pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventError,
                                                   OMX_ErrorHardware, 
                                                   OMX_TI_ErrorSevere,
                                                   NULL);
        }
#endif
        if(((int)args[4] == USN_ERR_WARNING) && ((int)args[5] == IUALG_WARN_PLAYCOMPLETED)) {
            AACDEC_DPRINT("%d :: UTIL: IUALG_WARN_PLAYCOMPLETED/USN_ERR_WARNING event received\n", __LINE__);
#ifndef UNDER_CE
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventBufferFlag,
                                                   (OMX_U32)NULL,
                                                   OMX_BUFFERFLAG_EOS,
                                                   NULL);
            pComponentPrivate->pLcmlBufHeader[0]->pIpParam->bLastBuffer = 0;
#else
            /* add callback to application to indicate SN/USN has completed playing of current set of date */
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventBufferFlag,
                                                   (OMX_U32)NULL,
                                                   OMX_BUFFERFLAG_EOS,
                                                   NULL);
#endif
		}

        if((int)args[5] == IUALG_WARN_CONCEALED) {
            AACDEC_DPRINT( "Algorithm issued a warning. But can continue" );
            AACDEC_DPRINT("%d :: arg5 = %p\n",__LINE__,args[5]);
        }

        if((int)args[5] == IUALG_ERR_GENERAL) {
            AACDEC_EPRINT( "Algorithm error. Cannot continue" );
            AACDEC_EPRINT("%d :: arg5 = %x\n",__LINE__,(int)args[5]);
            AACDEC_EPRINT("%d :: LCML_Callback: IUALG_ERR_GENERAL\n",__LINE__);
            pHandle = pComponentPrivate->pHandle;
            pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventError, 
                                                   OMX_ErrorUndefined,
                                                   OMX_TI_ErrorSevere, 
                                                   NULL);
        }

        if( (int)args[5] == IUALG_ERR_DATA_CORRUPT ) {
            char *pArgs = "damedesuStr";
            AACDEC_EPRINT( "Algorithm error. Corrupt data" );
            AACDEC_EPRINT("%d :: arg5 = %x\n",__LINE__,(int)args[5]);
            AACDEC_EPRINT("%d :: LCML_Callback: IUALG_ERR_DATA_CORRUPT\n",__LINE__);
            pHandle = pComponentPrivate->pHandle;
            pLcmlHandle = (LCML_DSP_INTERFACE *)pComponentPrivate->pLcmlHandle;
#ifndef UNDER_CE
            pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventError, 
                                                   OMX_ErrorStreamCorrupt, 
                                                   OMX_TI_ErrorMajor, 
                                                   NULL);
            eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                       MMCodecControlStop,(void *)pArgs);
            if(eError != OMX_ErrorNone) {
                AACDEC_DPRINT("%d: Error Occurred in Codec Stop..\n",__LINE__);
                goto EXIT;
            }
            AACDEC_DPRINT("%d :: AACDEC: Codec has been Stopped here\n",__LINE__);
            pComponentPrivate->curState = OMX_StateIdle;
            pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventCmdComplete, 
                                                   OMX_ErrorNone,
                                                   0, 
                                                   NULL);
#else
            pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventError, 
                                                   OMX_ErrorUndefined,
                                                   0, 
                                                   NULL);
#endif

        }

        if ( ( (int)args[4] == USN_ERR_WARNING ) && ( (int)args[5] == AACDEC_SBR_CONTENT)) {
            AACDEC_DPRINT("%d :: LCML_Callback: SBR content detected \n" ,__LINE__);
            if(pComponentPrivate->aacParams->eAACProfile != OMX_AUDIO_AACObjectHE &&
               pComponentPrivate->aacParams->eAACProfile != OMX_AUDIO_AACObjectHE_PS){

                pComponentPrivate->aacParams->eAACProfile = OMX_AUDIO_AACObjectHE;
                pComponentPrivate->AACDEC_UALGParam->nProfile = OMX_AUDIO_AACObjectHE;
                pComponentPrivate->AACDEC_UALGParam->iEnablePS =  0;
                pComponentPrivate->AACDEC_UALGParam->DownSampleSbr = 0;

                pValues[0] = IUALG_CMD_SETSTATUS;
                pValues[1] = (OMX_U32)pComponentPrivate->AACDEC_UALGParam;
                pValues[2] = sizeof(MPEG4AACDEC_UALGParams);

                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pComponentPrivate->pLcmlHandle)->pCodecinterfacehandle,
                                           EMMCodecControlAlgCtrl,(void *)pValues);
                if(eError != OMX_ErrorNone) {
                    AACDEC_DPRINT("%d: Error Occurred in Codec StreamControl..\n",__LINE__);
                    pComponentPrivate->curState = OMX_StateInvalid;
                    pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                           pComponentPrivate->pHandle->pApplicationPrivate,
                                                           OMX_EventError,
                                                           eError,
                                                           OMX_TI_ErrorSevere,
                                                           NULL);
                    goto EXIT;
                }

                pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                                       OMX_EventPortSettingsChanged,
                                                       OMX_DirInput,
                                                       OMX_AUDIO_AACObjectHE,
                                                       NULL);
            }
        }
        if ( ( (int)args[4] == USN_ERR_WARNING ) && ( (int)args[5] == AACDEC_PS_CONTENT )){
            AACDEC_DPRINT("%d :: LCML_Callback: PS content detected \n" ,__LINE__);
            if(pComponentPrivate->aacParams->eAACProfile != OMX_AUDIO_AACObjectHE_PS){

                pComponentPrivate->aacParams->eAACProfile = OMX_AUDIO_AACObjectHE_PS;
                pComponentPrivate->AACDEC_UALGParam->nProfile = OMX_AUDIO_AACObjectHE_PS;
                pComponentPrivate->AACDEC_UALGParam->iEnablePS =  1;
                pComponentPrivate->AACDEC_UALGParam->DownSampleSbr = 0;

                pValues[0] = IUALG_CMD_SETSTATUS;
                pValues[1] = (OMX_U32)pComponentPrivate->AACDEC_UALGParam;
                pValues[2] = sizeof(MPEG4AACDEC_UALGParams);

                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pComponentPrivate->pLcmlHandle)->pCodecinterfacehandle,
                                           EMMCodecControlAlgCtrl,(void *)pValues);
                if(eError != OMX_ErrorNone) {
                    AACDEC_DPRINT("%d: Error Occurred in Codec StreamControl..\n",__LINE__);
                    pComponentPrivate->curState = OMX_StateInvalid;
                    pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                           pComponentPrivate->pHandle->pApplicationPrivate,
                                                           OMX_EventError,
                                                           OMX_ErrorInvalidState,
                                                           0,
                                                           NULL);
                    goto EXIT;
                }

                pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                                       OMX_EventPortSettingsChanged,
                                                       OMX_DirInput,
                                                       OMX_AUDIO_AACObjectHE_PS,
                                                       NULL);
            }

        }
        if( (int)args[5] == IUALG_WARN_OVERFLOW ){
            AACDEC_DPRINT( "Algorithm error. Overflow" );
        }
        if( (int)args[5] == IUALG_WARN_UNDERFLOW ){
            AACDEC_DPRINT( "Algorithm error. Underflow" );
        }
        if( (int)args[4] == USN_ERR_PROCESS ){
            AACDEC_EPRINT( "Algorithm Error Process" );
            pHandle = pComponentPrivate->pHandle;
            char *pArgs = "damedesuStr";

            pLcmlHandle = (LCML_DSP_INTERFACE *)pComponentPrivate->pLcmlHandle;

            eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                       MMCodecControlStop,
                                       (void *)pArgs);
            if(eError != OMX_ErrorNone) {
                AACDEC_DPRINT("%d: Error Occurred in Codec Stop..\n",__LINE__);
                goto EXIT;
            }
            AACDEC_DPRINT("%d :: AACDEC: Codec has been Stopped here\n",__LINE__);
            pComponentPrivate->curState = OMX_StateIdle;
            pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventCmdComplete, 
                                                   OMX_ErrorNone,
                                                   0, 
                                                   NULL);
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventError,
                                                   OMX_ErrorStreamCorrupt,
                                                   OMX_TI_ErrorMajor,
                                                   NULL);
        }
    } else if (event == EMMCodecStrmCtrlAck) {
        AACDEC_DPRINT("%d :: GOT MESSAGE USN_DSPACK_STRMCTRL ----\n",__LINE__);
        if (args[1] == (void *)USN_STRMCMD_FLUSH) {
            pHandle = pComponentPrivate->pHandle;
            if ( args[2] == (void *)EMMCodecInputBuffer) {
                if (args[0] == (void *)USN_ERR_NONE ) {
                    AACDEC_DPRINT("Flushing input port %d\n",__LINE__);

					for (i=0; i < pComponentPrivate->nNumInputBufPending; i++) {
#ifdef __PERF_INSTRUMENTATION__
					PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                      PREF(pComponentPrivate->pInputBufHdrPending[i],pBuffer),
									  0,
									  PERF_ModuleHLMM);
#endif
					pComponentPrivate->cbInfo.EmptyBufferDone (
                                                           pComponentPrivate->pHandle,
                                                           pComponentPrivate->pHandle->pApplicationPrivate,
                                                           pComponentPrivate->pInputBufHdrPending[i]
                                                           );
					pComponentPrivate->pInputBufHdrPending[i] = NULL;
					}
                    pComponentPrivate->nNumInputBufPending=0;
                    pComponentPrivate->cbInfo.EventHandler(pHandle, pHandle->pApplicationPrivate,
                                                           OMX_EventCmdComplete, OMX_CommandFlush,INPUT_PORT_AACDEC, NULL);
                } else {
                    AACDEC_EPRINT ("LCML reported error while flushing input port\n");
                    goto EXIT;
                }
            }
            else if ( args[2] == (void *)EMMCodecOuputBuffer) {
                if (args[0] == (void *)USN_ERR_NONE ) {
				    AACDEC_DPRINT("Flushing output port %d\n",__LINE__);
						for (i=0; i < pComponentPrivate->nNumOutputBufPending; i++) {
#ifdef __PERF_INSTRUMENTATION__
							PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                          PREF(pComponentPrivate->pOutputBufHdrPending[i],pBuffer),
                                          PREF(pComponentPrivate->pOutputBufHdrPending[i],nFilledLen),
                                          PERF_ModuleHLMM);
#endif

							pComponentPrivate->cbInfo.FillBufferDone (pComponentPrivate->pHandle,
                                                                  pComponentPrivate->pHandle->pApplicationPrivate,
                                                                  pComponentPrivate->pOutputBufHdrPending[i]
                                                                  );
							pComponentPrivate->nOutStandingFillDones--;
							pComponentPrivate->pOutputBufHdrPending[i] = NULL;
						}
                    pComponentPrivate->nNumOutputBufPending=0;
                    pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle, pComponentPrivate->pHandle->pApplicationPrivate,
                                                           OMX_EventCmdComplete, OMX_CommandFlush,OUTPUT_PORT_AACDEC, NULL);

                } else {
                    AACDEC_EPRINT("LCML reported error while flushing output port\n");
                    goto EXIT;
                }
            }
        }
    }else if (event == EMMCodecProcessingPaused) {
        pComponentPrivate->nUnhandledFillThisBuffers = 0;
        pComponentPrivate->nUnhandledEmptyThisBuffers = 0;
        pComponentPrivate->curState = OMX_StatePause;
        pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle, pComponentPrivate->pHandle->pApplicationPrivate,
                                               OMX_EventCmdComplete, OMX_CommandStateSet,
                                               pComponentPrivate->curState, NULL);
    }
#ifdef _ERROR_PROPAGATION__
    else if (event == EMMCodecInitError){
        /* Cheking for MMU_fault */
        if((args[4] == (void*)USN_ERR_UNKNOWN_MSG) && (args[5] == (void*)NULL)) {
            AACDEC_EPRINT("%d :: UTIL: MMU_Fault \n",__LINE__);
            pComponentPrivate->bIsInvalidState=OMX_TRUE;
            pComponentPrivate->curState = OMX_StateInvalid;
            pHandle = pComponentPrivate->pHandle;
            pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventError,
                                                   OMX_ErrorStreamCorrupt,
                                                   OMX_TI_ErrorSevere,
                                                   NULL);
        }
    }
    else if (event ==EMMCodecInternalError){
        /* Cheking for MMU_fault */
        if((args[4] == (void*)USN_ERR_UNKNOWN_MSG) && (args[5] == (void*)NULL)) {
            AACDEC_EPRINT("%d :: UTIL: MMU_Fault \n",__LINE__);
            pComponentPrivate->bIsInvalidState=OMX_TRUE;
            pComponentPrivate->curState = OMX_StateInvalid;
            pHandle = pComponentPrivate->pHandle;
            pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventError,
                                                   OMX_ErrorStreamCorrupt,
                                                   OMX_TI_ErrorSevere,
                                                   NULL);
        }
    }
#endif
 EXIT:
    return eError;
}


/* ================================================================================= * */
/**
* @fn AACDEC_GetCorresponding_LCMLHeader() function gets the corresponding LCML
* header from the actual data buffer for required processing.
*
* @param *pBuffer This is the data buffer pointer.
*
* @param eDir   This is direction of buffer. Input/Output.
*
* @param *AACD_LCML_BUFHEADERTYPE  This is pointer to LCML Buffer Header.
*
* @pre          None
*
* @post         None
*
*  @return      OMX_ErrorNone = Successful Inirialization of the component\n
*               OMX_ErrorHardware = Hardware error has occured.
*
*  @see         None
*/
/* ================================================================================ * */
OMX_ERRORTYPE AACDEC_GetCorresponding_LCMLHeader(AACDEC_COMPONENT_PRIVATE* pComponentPrivate,
                                        OMX_U8 *pBuffer,
                                        OMX_DIRTYPE eDir,
                                        AACD_LCML_BUFHEADERTYPE **ppLcmlHdr)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    AACD_LCML_BUFHEADERTYPE *pLcmlBufHeader;
    int nIpBuf=0, nOpBuf=0, i=0;

    while (!pComponentPrivate->bInitParamsInitialized) {
#ifndef UNDER_CE
        sched_yield();
#else
        Sleep(0);
#endif
    }


    if(eDir == OMX_DirInput) {
        AACDEC_DPRINT  ("%d :: In GetCorresponding_LCMLHeader()\n",__LINE__);
        nIpBuf = pComponentPrivate->pInputBufferList->numBuffers;
        pLcmlBufHeader = pComponentPrivate->pLcmlBufHeader[INPUT_PORT_AACDEC];

        for(i=0; i<nIpBuf; i++) {
            AACDEC_DPRINT ("pBuffer = %p\n",pBuffer);
            AACDEC_DPRINT ("pLcmlBufHeader->pBufHdr->pBuffer = %p\n",pLcmlBufHeader->pBufHdr->pBuffer);
            if(pBuffer == pLcmlBufHeader->pBufHdr->pBuffer) {
                *ppLcmlHdr = pLcmlBufHeader;
                AACDEC_DPRINT ("%d::Corresponding LCML Header Found\n",__LINE__);
                goto EXIT;
            }
            pLcmlBufHeader++;
        }
    } else if (eDir == OMX_DirOutput) {
        i = 0;
        nOpBuf = pComponentPrivate->pOutputBufferList->numBuffers;
        pLcmlBufHeader = pComponentPrivate->pLcmlBufHeader[OUTPUT_PORT_AACDEC];
        AACDEC_DPRINT  ("%d :: nOpBuf = %d\n",__LINE__,nOpBuf);

        for(i=0; i<nOpBuf; i++) {
            AACDEC_DPRINT ("pBuffer = %p\n",pBuffer);
            AACDEC_DPRINT ("pLcmlBufHeader->pBufHdr->pBuffer = %p\n",pLcmlBufHeader->pBufHdr->pBuffer);
            if(pBuffer == pLcmlBufHeader->pBufHdr->pBuffer) {
                *ppLcmlHdr = pLcmlBufHeader;
                AACDEC_DPRINT ("%d::Corresponding LCML Header Found\n",__LINE__);
                goto EXIT;
            }
            pLcmlBufHeader++;
        }
    } else {
        AACDEC_DPRINT ("%d:: Invalid Buffer Type :: exiting...\n",__LINE__);
    }

 EXIT:
    return eError;
}

/* ================================================================================= * */
/**
* @fn AACDEC_GetLCMLHandle() function gets the LCML handle and interacts with LCML
* by using this LCML Handle.
*
* @param *pBufHeader This is the buffer header that needs to be processed.
*
* @param *pComponentPrivate  This is component's private date structure.
*
* @pre          None
*
* @post         None
*
*  @return      OMX_HANDLETYPE = Successful loading of LCML library.
*               OMX_ErrorHardware = Hardware error has occured.
*
*  @see         None
*/
/* ================================================================================ * */
#ifndef UNDER_CE
OMX_HANDLETYPE AACDEC_GetLCMLHandle(AACDEC_COMPONENT_PRIVATE* pComponentPrivate)
{
    /* This must be taken care by WinCE */
    OMX_HANDLETYPE pHandle = NULL;
    OMX_ERRORTYPE eError;
    void *handle;
    OMX_ERRORTYPE (*fpGetHandle)(OMX_HANDLETYPE);
    char *error;

    handle = dlopen("libLCML.so", RTLD_LAZY);
    if (!handle) {
        fputs(dlerror(), stderr);
        goto EXIT;
    }

    fpGetHandle = dlsym (handle, "GetHandle");
    if ((error = dlerror()) != NULL) {
        fputs(error, stderr);
        goto EXIT;
    }
    eError = (*fpGetHandle)(&pHandle);
    if(eError != OMX_ErrorNone) {
        eError = OMX_ErrorUndefined;
        AACDEC_DPRINT("eError != OMX_ErrorNone...\n");
        pHandle = NULL;
        goto EXIT;
    }
    ((LCML_DSP_INTERFACE*)pHandle)->pComponentPrivate = pComponentPrivate;

 EXIT:
    return pHandle;
}
#else
OMX_HANDLETYPE AACDEC_GetLCMLHandle(AACDEC_COMPONENT_PRIVATE* pComponentPrivate)
{
    /* This must be taken care by WinCE */
    OMX_HANDLETYPE pHandle = NULL;
    typedef OMX_ERRORTYPE (*LPFNDLLFUNC1)(OMX_HANDLETYPE);
    OMX_ERRORTYPE eError;
    HINSTANCE hDLL;               // Handle to DLL
    LPFNDLLFUNC1 fpGetHandle1;



    hDLL = LoadLibraryEx(TEXT("OAF_BML.dll"), NULL,0);
    if (hDLL == NULL)
        {
            AMRDEC_DPRINT("BML Load Failed!!!\n");
            return pHandle;
        }

    fpGetHandle1 = (LPFNDLLFUNC1)GetProcAddress(hDLL,TEXT("GetHandle"));
    if (!fpGetHandle1)
        {
            // handle the error
            FreeLibrary(hDLL);
            return pHandle;
        }

    // call the function
    eError = fpGetHandle1(&pHandle);
    if(eError != OMX_ErrorNone) {
        eError = OMX_ErrorUndefined;
        AMRDEC_DPRINT("eError != OMX_ErrorNone...\n");
        pHandle = NULL;
        return pHandle;
    }
    ((LCML_DSP_INTERFACE*)pHandle)->pComponentPrivate = pComponentPrivate;

    return pHandle;

}
#endif

/* ========================================================================== */
/**
* @AACDEC_CleanupInitParams() This function is called by the component during
* de-init to close component thread, Command pipe, data pipe & LCML pipe.
*
* @param pComponent  handle for this instance of the component
*
* @pre
*
* @post
*
* @return none
*/
/* ========================================================================== */

void AACDEC_CleanupInitParams(OMX_HANDLETYPE pComponent)
{
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    AACDEC_COMPONENT_PRIVATE *pComponentPrivate = (AACDEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;
    AACD_LCML_BUFHEADERTYPE *pTemp_lcml;
    char *pTemp = NULL;
    OMX_U32 nIpBuf = 0;
    OMX_U32 nOpBuf = 0;
    OMX_U32 i=0;

    AACDEC_MEMPRINT("%d :: Freeing:  pComponentPrivate->strmAttr = %p\n",
                    __LINE__, pComponentPrivate->strmAttr);
    AACDEC_OMX_FREE(pComponentPrivate->strmAttr);

    nIpBuf = pComponentPrivate->nRuntimeInputBuffers;
    pTemp_lcml = pComponentPrivate->pLcmlBufHeader[INPUT_PORT_AACDEC];
    for(i=0; i<nIpBuf; i++) {
        AACDEC_MEMPRINT(":: Freeing: pTemp_lcml->pIpParam = %p\n",pTemp_lcml->pIpParam);
        pTemp = (char*)pTemp_lcml->pIpParam;
        if (pTemp != NULL) {
            pTemp -= EXTRA_BYTES;
        }
        pTemp_lcml->pIpParam = (AACDEC_UAlgInBufParamStruct*)pTemp;
        AACDEC_OMX_FREE(pTemp_lcml->pIpParam);
        pTemp_lcml++;
    }

    AACDEC_MEMPRINT(":: Freeing pComponentPrivate->pLcmlBufHeader[INPUT_PORT_AACDEC] = %p\n",
                    pComponentPrivate->pLcmlBufHeader[INPUT_PORT_AACDEC]);
    AACDEC_OMX_FREE(pComponentPrivate->pLcmlBufHeader[INPUT_PORT_AACDEC]);

    nOpBuf = pComponentPrivate->nRuntimeOutputBuffers;
    pComponentPrivate->pLcmlBufHeader[INPUT_PORT_AACDEC] = NULL;
    pTemp_lcml = pComponentPrivate->pLcmlBufHeader[OUTPUT_PORT_AACDEC];
    for(i=0; i<nOpBuf; i++) {
        AACDEC_MEMPRINT(":: Freeing: pTemp_lcml->pOpParam = %p\n",pTemp_lcml->pOpParam);
        pTemp = (char*)pTemp_lcml->pOpParam;
        if (pTemp != NULL) {
            pTemp -= EXTRA_BYTES;
        }
        pTemp_lcml->pOpParam = (AACDEC_UAlgOutBufParamStruct*)pTemp;
        AACDEC_OMX_FREE(pTemp_lcml->pOpParam);
        pTemp_lcml++;
    }

    AACDEC_MEMPRINT(":: Freeing: pComponentPrivate->pLcmlBufHeader[OUTPUT_PORT_AACDEC] = %p\n",
                    pComponentPrivate->pLcmlBufHeader[OUTPUT_PORT_AACDEC]);
    AACDEC_OMX_FREE(pComponentPrivate->pLcmlBufHeader[OUTPUT_PORT_AACDEC]);

    pTemp = (char*)pComponentPrivate->pParams;
    if (pTemp != NULL) {
        pTemp -= EXTRA_BYTES;
    }
    pComponentPrivate->pParams = (USN_AudioCodecParams*)pTemp;
    AACDEC_OMX_FREE(pComponentPrivate->pParams);

    pTemp = (char*)pComponentPrivate->AACDEC_UALGParam;
    if (pTemp != NULL) {
        pTemp -= EXTRA_BYTES;
    }
    pComponentPrivate->AACDEC_UALGParam = (MPEG4AACDEC_UALGParams*)pTemp;
    AACDEC_OMX_FREE(pComponentPrivate->AACDEC_UALGParam);
}


/*=======================================================================*/
/*! @fn AACDec_GetSampleRateIndexL

 * @brief Gets the sample rate index

 * @param  aRate : Actual Sampling Freq

 * @Return  Index

 */
/*=======================================================================*/
int AACDec_GetSampleRateIndexL( const int aRate)
{
    int index = 0;
    AACDEC_DPRINT("%d::aRate:%d\n",__LINE__,aRate);

    switch( aRate ){
    case 96000:
        index = 0;
        break;
    case 88200:
        index = 1;
        break;
    case 64000:
        index = 2;
        break;
    case 48000:
        index = 3;
        break;
    case 44100:
        index = 4;
        break;
    case 32000:
        index = 5;
        break;
    case 24000:
        index = 6;
        break;
    case 22050:
        index = 7;
        break;
    case 16000:
        index = 8;
        break;
    case 12000:
        index = 9;
        break;
    case 11025:
        index = 10;
        break;
    case 8000:
        index = 11;
        break;
    default:
        AACDEC_DPRINT("Invalid sampling frequency\n");
        break;
    }

    AACDEC_DPRINT("%d::index:%d\n",__LINE__,index);
    return index;
}

/* ========================================================================== */
/**
* @AACDEC_SetPending() This function marks the buffer as pending when it is sent
* to DSP/
*
* @param pComponentPrivate This is component's private date area.
*
* @param pBufHdr This is poiter to OMX Buffer header whose buffer is sent to DSP
*
* @param eDir This is direction of buffer i.e. input or output.
*
* @pre None
*
* @post None
*
* @return none
*/
/* ========================================================================== */
void AACDEC_SetPending(AACDEC_COMPONENT_PRIVATE *pComponentPrivate, OMX_BUFFERHEADERTYPE *pBufHdr, OMX_DIRTYPE eDir, OMX_U32 lineNumber)
{
    OMX_U16 i;

    if (eDir == OMX_DirInput) {
        for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pInputBufferList->pBufHdr[i]) {
                pComponentPrivate->pInputBufferList->bBufferPending[i] = 1;
                AACDEC_DPRINT("*******************INPUT BUFFER %d IS PENDING Line %lu, :%p******************************\n",i,lineNumber,pBufHdr);
            }
        }
    }
    else {
        for (i=0; i < pComponentPrivate->pOutputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pOutputBufferList->pBufHdr[i]) {
                pComponentPrivate->pOutputBufferList->bBufferPending[i] = 1;
                AACDEC_DPRINT("*******************OUTPUT BUFFER %d IS PENDING Line, %lu :%p******************************\n",i,lineNumber,pBufHdr);
            }
        }
    }
}

/* ========================================================================== */
/**
* @AACDEC_ClearPending() This function clears the buffer status from pending
* when it is received back from DSP.
*
* @param pComponentPrivate This is component's private date area.
*
* @param pBufHdr This is poiter to OMX Buffer header that is received from
* DSP/LCML.
*
* @param eDir This is direction of buffer i.e. input or output.
*
* @pre None
*
* @post None
*
* @return none
*/
/* ========================================================================== */
void AACDEC_ClearPending(AACDEC_COMPONENT_PRIVATE *pComponentPrivate,
            OMX_BUFFERHEADERTYPE *pBufHdr, OMX_DIRTYPE eDir, OMX_U32 lineNumber)
{
    OMX_U16 i;

    if (eDir == OMX_DirInput) {
        for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pInputBufferList->pBufHdr[i]) {
                pComponentPrivate->pInputBufferList->bBufferPending[i] = 0;
                AACDEC_DPRINT("*******************INPUT BUFFER %d IS RECLAIMED Line %lu, :%p******************************\n",i,lineNumber,pBufHdr);
            }
        }
    }
    else {
        for (i=0; i < pComponentPrivate->pOutputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pOutputBufferList->pBufHdr[i]) {
                pComponentPrivate->pOutputBufferList->bBufferPending[i] = 0;
                AACDEC_DPRINT("*******************OUTPUT BUFFER %d IS RECLAIMED Line %lu, :%p******************************\n",i,lineNumber,pBufHdr);
            }
        }
    }
}

/* ========================================================================== */
/**
* @AACDEC_IsPending() This function checks whether or not a buffer is pending.
*
* @param pComponentPrivate This is component's private date area.
*
* @param pBufHdr This is poiter to OMX Buffer header of interest.
*
* @param eDir This is direction of buffer i.e. input or output.
*
* @pre None
*
* @post None
*
* @return none
*/
/* ========================================================================== */
OMX_U32 AACDEC_IsPending(AACDEC_COMPONENT_PRIVATE *pComponentPrivate, OMX_BUFFERHEADERTYPE *pBufHdr, OMX_DIRTYPE eDir)
{
    OMX_U16 i;

    if (eDir == OMX_DirInput) {
        for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pInputBufferList->pBufHdr[i]) {
                return pComponentPrivate->pInputBufferList->bBufferPending[i];
            }
        }
    }
    else {
        for (i=0; i < pComponentPrivate->pOutputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pOutputBufferList->pBufHdr[i]) {
                return pComponentPrivate->pOutputBufferList->bBufferPending[i];
            }
        }
    }
    return -1;
}


/* ========================================================================== */
/**
* @AACDEC_IsValid() This function identifies whether or not buffer recieved from
* LCML is valid. It searches in the list of input/output buffers to do this.
*
* @param pComponentPrivate This is component's private date area.
*
* @param pBufHdr This is poiter to OMX Buffer header of interest.
*
* @param eDir This is direction of buffer i.e. input or output.
*
* @pre None
*
* @post None
*
* @return status of the buffer.
*/
/* ========================================================================== */
OMX_U32 AACDEC_IsValid(AACDEC_COMPONENT_PRIVATE *pComponentPrivate, OMX_U8 *pBuffer, OMX_DIRTYPE eDir)
{
    OMX_U16 i;
    int found=0;

    if (eDir == OMX_DirInput) {
        for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
            if (pBuffer == pComponentPrivate->pInputBufferList->pBufHdr[i]->pBuffer) {
                found = 1;
            }
        }
    }
    else {
        for (i=0; i < pComponentPrivate->pOutputBufferList->numBuffers; i++) {
            if (pBuffer == pComponentPrivate->pOutputBufferList->pBufHdr[i]->pBuffer) {
                found = 1;
            }
        }
    }
    return found;
}

/* ========================================================================== */
/**
* @AACDECFill_LCMLInitParamsEx() This function initializes the init parameter of
* the LCML structure when a port is enabled and component is in idle state.
*
* @param pComponent This is component handle.
*
* @pre None
*
* @post None
*
* @return appropriate OMX Error.
*/
/* ========================================================================== */
OMX_ERRORTYPE AACDECFill_LCMLInitParamsEx(OMX_HANDLETYPE pComponent)

{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 nIpBuf,nIpBufSize,nOpBuf,nOpBufSize;
    OMX_U16 i;
    OMX_BUFFERHEADERTYPE *pTemp;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    AACDEC_COMPONENT_PRIVATE *pComponentPrivate =
        (AACDEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;
    AACD_LCML_BUFHEADERTYPE *pTemp_lcml;
    OMX_U32 size_lcml;
    char *ptr;

    nIpBuf = pComponentPrivate->pInputBufferList->numBuffers;
    nOpBuf = pComponentPrivate->pOutputBufferList->numBuffers;
    nIpBufSize = pComponentPrivate->pPortDef[INPUT_PORT_AACDEC]->nBufferSize;
    nOpBufSize = pComponentPrivate->pPortDef[OUTPUT_PORT_AACDEC]->nBufferSize;


    AACDEC_DPRINT("Input Buffer Count = %ld\n",nIpBuf);
    AACDEC_DPRINT("Input Buffer Size = %ld\n",nIpBufSize);
    AACDEC_DPRINT("Output Buffer Count = %ld\n",nOpBuf);
    AACDEC_DPRINT("Output Buffer Size = %ld\n",nOpBufSize);
    AACDEC_DPRINT("Input Buffer Count = %ld\n",nIpBuf);
    AACDEC_DPRINT("Input Buffer Size = %ld\n",nIpBufSize);
    AACDEC_DPRINT("Output Buffer Count = %ld\n",nOpBuf);
    AACDEC_DPRINT("Output Buffer Size = %ld\n",nOpBufSize);



    AACDEC_DPRINT("%d :: bufAlloced = %lu\n",__LINE__,pComponentPrivate->bufAlloced);
    size_lcml = nIpBuf * sizeof(AACD_LCML_BUFHEADERTYPE);

    AACDEC_OMX_MALLOC_SIZE(ptr,size_lcml,char);
    pTemp_lcml = (AACD_LCML_BUFHEADERTYPE *)ptr;

    pComponentPrivate->pLcmlBufHeader[INPUT_PORT_AACDEC] = pTemp_lcml;

    for (i=0; i<nIpBuf; i++) {
        pTemp = pComponentPrivate->pInputBufferList->pBufHdr[i];
        pTemp->nSize = sizeof(OMX_BUFFERHEADERTYPE);

        pTemp->nAllocLen = nIpBufSize;
        pTemp->nFilledLen = nIpBufSize;
        pTemp->nVersion.s.nVersionMajor = AACDEC_MAJOR_VER;
        pTemp->nVersion.s.nVersionMinor = AACDEC_MINOR_VER;

        pComponentPrivate->nVersion = pTemp->nVersion.nVersion;

        pTemp->pPlatformPrivate = pHandle->pComponentPrivate;
        pTemp->nTickCount = NOT_USED_AACDEC;

        pTemp_lcml->pBufHdr = pTemp;
        pTemp_lcml->eDir = OMX_DirInput;
        pTemp_lcml->pOtherParams[i] = NULL;

        AACDEC_OMX_MALLOC_SIZE(pTemp_lcml->pIpParam,
                             (sizeof(AACDEC_UAlgInBufParamStruct) + DSP_CACHE_ALIGNMENT),
                             AACDEC_UAlgInBufParamStruct);
        ptr = (char*)pTemp_lcml->pIpParam;
        ptr += EXTRA_BYTES;
        pTemp_lcml->pIpParam = (AACDEC_UAlgInBufParamStruct*)ptr;

        pTemp_lcml->pIpParam->bLastBuffer = 0;

        pTemp->nFlags = NORMAL_BUFFER_AACDEC;
        ((AACDEC_COMPONENT_PRIVATE *) pTemp->pPlatformPrivate)->pHandle = pHandle;

        AACDEC_DPRINT("%d ::Comp: InBuffHeader[%d] = %p\n", __LINE__, i, pTemp);
        AACDEC_DPRINT("%d ::Comp:  >>>> InputBuffHeader[%d]->pBuffer = %p\n",
                      __LINE__, i, pTemp->pBuffer);
        AACDEC_DPRINT("%d ::Comp: Ip : pTemp_lcml[%d] = %p\n", __LINE__, i, pTemp_lcml);

        pTemp_lcml++;
    }

    size_lcml = nOpBuf * sizeof(AACD_LCML_BUFHEADERTYPE);
    AACDEC_OMX_MALLOC_SIZE(pTemp_lcml,size_lcml,AACD_LCML_BUFHEADERTYPE);
    pComponentPrivate->pLcmlBufHeader[OUTPUT_PORT_AACDEC] = pTemp_lcml;

    for (i=0; i<nOpBuf; i++) {
        pTemp = pComponentPrivate->pOutputBufferList->pBufHdr[i];
        pTemp->nSize = sizeof(OMX_BUFFERHEADERTYPE);

        pTemp->nAllocLen = nOpBufSize;
        pTemp->nFilledLen = nOpBufSize;
        pTemp->nVersion.s.nVersionMajor = AACDEC_MAJOR_VER;
        pTemp->nVersion.s.nVersionMinor = AACDEC_MINOR_VER;

        pComponentPrivate->nVersion = pTemp->nVersion.nVersion;

        pTemp->pPlatformPrivate = pHandle->pComponentPrivate;
        pTemp->nTickCount = NOT_USED_AACDEC;

        pTemp_lcml->pBufHdr = pTemp;
        pTemp_lcml->eDir = OMX_DirOutput;
        pTemp_lcml->pOtherParams[i] = NULL;

        AACDEC_OMX_MALLOC_SIZE(pTemp_lcml->pOpParam,
                             (sizeof(AACDEC_UAlgOutBufParamStruct) + DSP_CACHE_ALIGNMENT),
                             AACDEC_UAlgOutBufParamStruct);
        ptr = (char*)pTemp_lcml->pOpParam;
        ptr += EXTRA_BYTES;
        pTemp_lcml->pOpParam = (AACDEC_UAlgOutBufParamStruct*)ptr;

        pTemp_lcml->pOpParam->ulFrameCount = DONT_CARE;

        pTemp->nFlags = NORMAL_BUFFER_AACDEC;
        ((AACDEC_COMPONENT_PRIVATE *)pTemp->pPlatformPrivate)->pHandle = pHandle;
        AACDEC_DPRINT("%d ::Comp:  >>>>>>>>>>>>> OutBuffHeader[%d] = %p\n",
                      __LINE__, i, pTemp);
        AACDEC_DPRINT("%d ::Comp:  >>>> OutBuffHeader[%d]->pBuffer = %p\n",
                      __LINE__, i, pTemp->pBuffer);
        AACDEC_DPRINT("%d ::Comp: Op : pTemp_lcml[%d] = %p\n", __LINE__, i, pTemp_lcml);
        pTemp_lcml++;
    }
    pComponentPrivate->bPortDefsAllocated = 1;

#ifdef __PERF_INSTRUMENTATION__
    pComponentPrivate->nLcml_nCntIp = 0;
    pComponentPrivate->nLcml_nCntOpReceived = 0;
#endif
    pComponentPrivate->bInitParamsInitialized = 1;

 EXIT:
    return eError;
}

#ifdef AACDEC_DEBUGMEM
/* ========================================================================== */
/**
* @mymalloc() This function is used to debug memory leaks
*
* @param
*
* @pre None
*
* @post None
*
* @return None
*/
/* ========================================================================== */
void * mymalloc(int line, char *s, int size)
{
    void *p;
    int e=0;

    p = malloc(size);
    if(p==NULL){
        AACDEC_DPRINT("Memory not available\n");
        exit(1);
    }
    else{
        while((lines[e]!=0)&& (e<500) ){
            e++;
        }
        arr[e]=p;
        lines[e]=line;
        bytes[e]=size;
        strcpy(file[e],s);
        AACDEC_DPRINT("Allocating %d bytes on address %p, line %d file %s pos %d\n", size, p, line, s, e);
        return p;
    }

}

 /* ========================================================================== */
/**
* @myfree() This function is used to debug memory leaks
*
* @param
*
* @pre None
*
* @post None
*
* @return None
*/
/* ========================================================================== */
int myfree(void *dp, int line, char *s)
{
    int q;
    for(q=0;q<500;q++){
        if(arr[q]==dp){
            AACDEC_DPRINT("Deleting %d bytes on address %p, line %d file %s\n", bytes[q],dp, line, s);
            free(dp);
            dp = NULL;
            lines[q]=0;
            strcpy(file[q],"");
            break;
        }
    }
    if(500==q)
        AACDEC_DPRINT("\n\nPointer not found. Line:%d    File%s!!\n\n",line, s);
}
#endif


/*  =========================================================================*/
/*  func    GetBits                                                          */
/*                                                                           */
/*  desc    Gets aBits number of bits from position aPosition of one buffer  */
/*            and returns the value in a TUint value.                        */
/*  =========================================================================*/
OMX_U32 AACDEC_GetBits(OMX_U32* nPosition, OMX_U8 nBits, OMX_U8* pBuffer, OMX_BOOL bIcreasePosition)
{
    OMX_U32 nOutput;
    OMX_U32 nNumBitsRead = 0;
    OMX_U32 nBytePosition = 0;
    OMX_U8  nBitPosition =  0;
    nBytePosition = *nPosition / 8;
    nBitPosition =  *nPosition % 8;

    if (bIcreasePosition)
        *nPosition += nBits;
    nOutput = ((OMX_U32)pBuffer[nBytePosition] << (24+nBitPosition) );
    nNumBitsRead = nNumBitsRead + (8 - nBitPosition);
    if (nNumBitsRead < nBits)
    {
        nOutput = nOutput | ( pBuffer[nBytePosition + 1] << (16+nBitPosition));
        nNumBitsRead = nNumBitsRead + 8;
    }
    if (nNumBitsRead < nBits)
    {
        nOutput = nOutput | ( pBuffer[nBytePosition + 2] << (8+nBitPosition));
        nNumBitsRead = nNumBitsRead + 8;
    }
    if (nNumBitsRead < nBits)
    {
        nOutput = nOutput | ( pBuffer[nBytePosition + 3] << (nBitPosition));
        nNumBitsRead = nNumBitsRead + 8;
    }
    nOutput = nOutput >> (32 - nBits) ;
    return nOutput;
}
/*
void AACDEC_ResourceManagerCallback(RMPROXY_COMMANDDATATYPE cbData)
{
    OMX_COMMANDTYPE Cmd = OMX_CommandStateSet;
    OMX_STATETYPE state = OMX_StateIdle;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)cbData.hComponent;
    AACDEC_COMPONENT_PRIVATE *pCompPrivate = NULL;

    pCompPrivate = (AACDEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;

    if (*(cbData.RM_Error) == OMX_RmProxyCallback_ResourcesPreempted) {
        if (pCompPrivate->curState == OMX_StateExecuting ||
            pCompPrivate->curState == OMX_StatePause) {
            write (pCompPrivate->cmdPipe[1], &Cmd, sizeof(Cmd));
            write (pCompPrivate->cmdDataPipe[1], &state ,sizeof(OMX_U32));
            pCompPrivate->bPreempted = 1;
        }
    }
    else if (*(cbData.RM_Error) == OMX_RmProxyCallback_ResourcesAcquired){
        pCompPrivate->cbInfo.EventHandler (
	                        pHandle, pHandle->pApplicationPrivate,
	                        OMX_EventResourcesAcquired, 0,0,
	                        NULL);
    }
}

*/
