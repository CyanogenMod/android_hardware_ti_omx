
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
* @file OMX_PcmDec_Utils.c
*
* This file implements various utilitiy functions for various activities 
* like handling command from application, callback from LCML etc.
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
#include <wchar.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <malloc.h>
#include <memory.h>
#include <fcntl.h>
#endif

#include <pthread.h>
#include <dbapi.h>
#include <string.h>
#include <stdio.h>

/*------- Program Header Files -----------------------------------------------*/
#include "LCML_DspCodec.h"

#include "OMX_PcmDec_Utils.h"
#include "pcmdecsocket_ti.h"

#include "usn_ti.h"
#include "usn.h"

#ifdef RESOURCE_MANAGER_ENABLED
#include <ResourceManagerProxyAPI.h>
#endif




#ifdef UNDER_CE
void sleep(DWORD Duration)
{
    Sleep(Duration);

}
#endif

#ifndef UNDER_CE
#define PCMDEC_DPRINT_ON(...)  fprintf(stdout, "%s %d::  ",__FUNCTION__, __LINE__);\
                                fprintf(stdout, __VA_ARGS__);\
                                fprintf(stdout, "\n");
#endif


/* ================================================================================= * */
/**
* @fn PCMDEC_Fill_LCMLInitParams() fills the LCML initialization structure.
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
OMX_ERRORTYPE PCMDEC_Fill_LCMLInitParams(OMX_HANDLETYPE pComponent,
                                  LCML_DSP *plcml_Init, OMX_U16 arr[])
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U16 nIpBuf,nOpBuf;
    OMX_U32 nIpBufSize, nOpBufSize;
    OMX_U32 i;
    OMX_BUFFERHEADERTYPE *pTemp;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate = (PCMDEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;
    
    PCMD_LCML_BUFHEADERTYPE *pTemp_lcml;
    OMX_U32 size_lcml;
    LCML_STRMATTR *strmAttr = NULL;
    char *pTemp_char = NULL;
    char *pTemp_char2 = NULL;

    PCMDEC_DPRINT(":: Entered Fill_LCMLInitParams");

    pComponentPrivate->strmAttr = NULL;


    nIpBuf = (OMX_U16)pComponentPrivate->pInputBufferList->numBuffers;
    pComponentPrivate->nRuntimeInputBuffers = nIpBuf;
    nOpBuf = (OMX_U16)pComponentPrivate->pOutputBufferList->numBuffers;
    pComponentPrivate->nRuntimeOutputBuffers = nOpBuf;
    nIpBufSize = pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->nBufferSize;
    nOpBufSize = pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->nBufferSize;
    
    PCMDEC_DPRINT("Input Buffer Count = %ld\n",nIpBuf);
    PCMDEC_DPRINT("Input Buffer Size = %ld\n",nIpBufSize);
    PCMDEC_DPRINT("Output Buffer Count = %ld\n",nOpBuf);
    PCMDEC_DPRINT("Output Buffer Size = %ld\n",nOpBufSize);

    PCMDEC_BUFPRINT("Input Buffer Count = %ld\n",nIpBuf);
    PCMDEC_BUFPRINT("Input Buffer Size = %ld\n",nIpBufSize);
    PCMDEC_BUFPRINT("Output Buffer Count = %ld\n",nOpBuf);
    PCMDEC_BUFPRINT("Output Buffer Size = %ld\n",nOpBufSize);

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

    plcml_Init->NodeInfo.AllUUIDs[0].uuid = &PCMDECSOCKET_TI_UUID;
    strcpy ((char*)plcml_Init->NodeInfo.AllUUIDs[0].DllName, PCMDEC_DLL_NAME);
    plcml_Init->NodeInfo.AllUUIDs[0].eDllType = DLL_NODEOBJECT;

    plcml_Init->NodeInfo.AllUUIDs[1].uuid = &PCMDECSOCKET_TI_UUID;
    strcpy ((char*)plcml_Init->NodeInfo.AllUUIDs[1].DllName, PCMDEC_DLL_NAME);
    plcml_Init->NodeInfo.AllUUIDs[1].eDllType = DLL_DEPENDENT;

    plcml_Init->NodeInfo.AllUUIDs[2].uuid = &USN_TI_UUID;
    strcpy ((char*)plcml_Init->NodeInfo.AllUUIDs[2].DllName, USN_DLL_NAME);
    plcml_Init->NodeInfo.AllUUIDs[2].eDllType = DLL_DEPENDENT;

    plcml_Init->SegID = OMX_PCMDEC_DEFAULT_SEGMENT;
    plcml_Init->Timeout = OMX_PCMDEC_SN_TIMEOUT;
    plcml_Init->Alignment = 0;
    plcml_Init->Priority = OMX_PCMDEC_SN_PRIORITY;
    plcml_Init->ProfileID = -1;

    pComponentPrivate->pUalgParam = malloc (sizeof(PCMDEC_SN_UALGInputParams) + 256);
    pTemp_char = (char*)pComponentPrivate->pUalgParam;
    pTemp_char += 128;
    pComponentPrivate->pUalgParam = (PCMDEC_SN_UALGInputParams*)pTemp_char;

    
    /*RESERVE MEMORY FOR THE    PCMDEC_UlNumOfBitsStatus    STRUCTURE*/
    pComponentPrivate->pUSetNumOfBits = malloc (sizeof(PCMDEC_UlNumOfBitsStatus) + 256);
    pTemp_char2 = (char*)pComponentPrivate->pUSetNumOfBits;
    pTemp_char2 += 128;
    pComponentPrivate->pUSetNumOfBits = (PCMDEC_UlNumOfBitsStatus*)pTemp_char2;
    
/*The code for reserving memory for the PCMD_USN_AudioCodecParams STRUCTURE  was tested to be outisde of the condition below 
if(pComponentPrivate->dasfmode == 1) , this way the memory was alwasy allocated not only when the dasfmode was selected
this has an impact with the CleanUpInitParams function, where the memory is liberated in case of dasf mode selected.
A couple of dasf mode and file mode test passed succesfully, so this is just a precedent of what could be modifyed in the future
*/ 


   

    /* For L89FLUSH */

    if(pComponentPrivate->dasfmode == 1) {
    /*RESERVE MEMORY FOR THE PCMD_USN_AudioCodecParams STRUCTURE*/
    pComponentPrivate->pParams = (PCMD_USN_AudioCodecParams *) malloc(sizeof(PCMD_USN_AudioCodecParams) + 256);
        if(NULL == pComponentPrivate->pParams) {
            PCMD_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                                "Memory Allocation Failed");
        }
        pTemp_char = (char*)pComponentPrivate->pParams;
        pTemp_char += 128;
        pComponentPrivate->pParams = (PCMD_USN_AudioCodecParams*)pTemp_char;  
 /*    End of:  RESERVE MEMORY FOR THE PCMD_USN_AudioCodecParams STRUCTURE*/

 
#ifndef DSP_RENDERING_ON
          
        PCMD_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                            "Flag DSP_RENDERING_ON Must Be Defined To Use Rendering");

#else
        strmAttr = malloc(sizeof(LCML_STRMATTR));
        PCMDEC_MEMPRINT("%d:[ALLOC] %p\n",__LINE__,strmAttr);

        if (strmAttr == NULL) {
            PCMDEC_DPRINT("strmAttr - failed to malloc\n");
            PCMD_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                                "Memory Allocation Failed");
        }        

        PCMDEC_DPRINT(": Malloc strmAttr = %p\n",strmAttr);
        pComponentPrivate->strmAttr = strmAttr;
        PCMDEC_DPRINT(":: PCM DECODER IS RUNNING UNDER DASF MODE \n");

        strmAttr->uSegid = 0;
        strmAttr->uAlignment = 0;
        strmAttr->uTimeout = -1;
        strmAttr->uBufsize = 1920;
        strmAttr->uNumBufs = 2;
        strmAttr->lMode = STRMMODE_PROCCOPY;
        plcml_Init->DeviceInfo.TypeofDevice = 1;
        plcml_Init->DeviceInfo.TypeofRender = 0;
        plcml_Init->DeviceInfo.AllUUIDs[0].uuid = &DCTN_TI_UUID;
        plcml_Init->DeviceInfo.DspStream = strmAttr;
#endif
    } else {
        plcml_Init->DeviceInfo.TypeofDevice = 0;
    }

    if (pComponentPrivate->dasfmode == 0){
        PCMDEC_DPRINT(":: FILE MODE CREATE PHASE PARAMETERS\n");
        arr[0] = 2;
        arr[1] = 0;
        arr[2] = 0;

        if(pComponentPrivate->pInputBufferList->numBuffers == 0) {
            arr[3] = PCMD_NUM_INPUT_BUFFERS; 
        } else {
            arr[3] = nIpBuf;
        }

        arr[4] = 1;
        arr[5] = 0;
        if(pComponentPrivate->pOutputBufferList->numBuffers == 0) {
            arr[6] = PCMD_NUM_OUTPUT_BUFFERS; 
        } else {
            arr[6] = nOpBuf;
        }

    } else {
        PCMDEC_DPRINT(":: DASF MODE CREATE PHASE PARAMETERS\n");
        arr[0] = 2;
        arr[1] = 0;
        arr[2] = 0;

        if(pComponentPrivate->pInputBufferList->numBuffers == 0) {
            arr[3] = PCMD_NUM_INPUT_BUFFERS; 
        } else {
            arr[3] = nIpBuf;
        }

        arr[4] = 1;
        arr[5] = 2;
        arr[6] = 2;
    }

    arr[7] = END_OF_CR_PAHSE_ARGS;
    plcml_Init->pCrPhArgs = arr;


    PCMDEC_DPRINT(":: bufAlloced = %d\n",pComponentPrivate->bufAlloced);
    size_lcml = nIpBuf * sizeof(PCMD_LCML_BUFHEADERTYPE);
    pTemp_lcml = (PCMD_LCML_BUFHEADERTYPE *)malloc(size_lcml);
    PCMDEC_MEMPRINT("%d:[ALLOC] %p\n",__LINE__,pTemp_lcml);    
    if(pTemp_lcml == NULL) {
        /* Free previously allocated memory before bailing */
        if (strmAttr) {
            PCMDEC_MEMPRINT("%d:::[FREE] %p\n",__LINE__,strmAttr);
            free(strmAttr);
            strmAttr = NULL;
        }
        PCMD_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                            "Memory Allocation Failed");
    }

    pComponentPrivate->pLcmlBufHeader[PCMD_INPUT_PORT] = pTemp_lcml;

    for (i=0; i<nIpBuf; i++) {
        pTemp = pComponentPrivate->pInputBufferList->pBufHdr[i];

        pTemp->nSize = sizeof(OMX_BUFFERHEADERTYPE);
        PCMDEC_DPRINT("IP: pTemp->nSize = %ld\n",pTemp->nSize);

        pTemp->nAllocLen = nIpBufSize;
        pTemp->nFilledLen = nIpBufSize;
        pTemp->nVersion.s.nVersionMajor = PCMDEC_MAJOR_VER;
        pTemp->nVersion.s.nVersionMinor = PCMDEC_MINOR_VER;

        pTemp->pPlatformPrivate = pHandle->pComponentPrivate;
        pTemp->nTickCount = NOT_USED;

        pTemp_lcml->pBufHdr = pTemp;
        pTemp_lcml->eDir = OMX_DirInput;
        pTemp_lcml->pOtherParams[i] = NULL;

        pTemp_lcml->pIpParam =
            (PCMDEC_UAlgInBufParamStruct *)malloc(sizeof(PCMDEC_UAlgInBufParamStruct));
        PCMDEC_MEMPRINT("%d:[ALLOC] %p\n",__LINE__,pTemp_lcml->pIpParam);        
        if (pTemp_lcml->pIpParam == NULL) {
            /* Free previously allocated memory before bailing */
            if (strmAttr) {
                PCMDEC_MEMPRINT("%d:::[FREE] %p\n",__LINE__,strmAttr);
                free(strmAttr);
                strmAttr = NULL;
            }
    
            if (pTemp_lcml) {
                PCMDEC_MEMPRINT("%d:::[FREE] %p\n",__LINE__,pTemp_lcml);
                free(pTemp_lcml);
                pTemp_lcml = NULL;
            }

            PCMD_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                                "Memory Allocation Failed");
        }        
        pTemp_lcml->pIpParam->bLastBuffer = 0;

        pTemp->nFlags = NORMAL_BUFFER;
        ((PCMDEC_COMPONENT_PRIVATE *) pTemp->pPlatformPrivate)->pHandle = pHandle;

        PCMDEC_DPRINT("::Comp: InBuffHeader[%ld] = %p\n", i, pTemp);
        PCMDEC_DPRINT("::Comp:  >>>> InputBuffHeader[%ld]->pBuffer = %p\n", i, pTemp->pBuffer);
        PCMDEC_DPRINT("::Comp: Ip : pTemp_lcml[%ld] = %p\n", i, pTemp_lcml);

        pTemp_lcml++;
    }

    size_lcml = nOpBuf * sizeof(PCMD_LCML_BUFHEADERTYPE);
    pTemp_lcml = (PCMD_LCML_BUFHEADERTYPE *)malloc(size_lcml);
    if(pTemp_lcml == NULL) {
        /* Free previously allocated memory before bailing */
        if (strmAttr) {
            PCMDEC_MEMPRINT("%d:::[FREE] %p\n",__LINE__,strmAttr);
            free(strmAttr);
            strmAttr = NULL;
        }
    
        if (pTemp_lcml) {
            if (pTemp_lcml->pIpParam) {
                PCMDEC_MEMPRINT("%d:::[FREE] %p\n",__LINE__,pTemp_lcml->pIpParam);
                free(pTemp_lcml->pIpParam);
                pTemp_lcml->pIpParam = NULL;
            }                        
                        
            PCMDEC_MEMPRINT("%d:::[FREE] %p\n",__LINE__,pTemp_lcml);
            free(pTemp_lcml);
            pTemp_lcml = NULL;
        }

        PCMD_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                            "Memory Allocation Failed");                           
    }
    pComponentPrivate->pLcmlBufHeader[PCMD_OUTPUT_PORT] = pTemp_lcml;

    for (i=0; i<nOpBuf; i++) {
        pTemp = pComponentPrivate->pOutputBufferList->pBufHdr[i];

        pTemp->nSize = sizeof(OMX_BUFFERHEADERTYPE);

        pTemp->nAllocLen = nOpBufSize;
        /*        pTemp->nFilledLen = nOpBufSize; */
        pTemp->nVersion.s.nVersionMajor = PCMDEC_MAJOR_VER;
        pTemp->nVersion.s.nVersionMinor = PCMDEC_MINOR_VER;


        pTemp->pPlatformPrivate = pHandle->pComponentPrivate;
        pTemp->nTickCount = NOT_USED;

        pTemp_lcml->pBufHdr = pTemp;
        pTemp_lcml->eDir = OMX_DirOutput;
        pTemp_lcml->pOtherParams[i] = NULL;

        pTemp_lcml->pOpParam =
            (PCMDEC_UAlgOutBufParamStruct *)malloc(sizeof(PCMDEC_UAlgOutBufParamStruct));
        PCMDEC_MEMPRINT("%d:[ALLOC] %p\n",__LINE__,pTemp_lcml->pOpParam);        
        if (pTemp_lcml->pOpParam == NULL) {
            /* Free previously allocated memory before bailing */
            if (strmAttr) {
                PCMDEC_MEMPRINT("%d:::[FREE] %p\n",__LINE__,strmAttr);
                free(strmAttr);
                strmAttr = NULL;
            }
    
            if (pTemp_lcml) {
                PCMDEC_MEMPRINT("%d:::[FREE] %p\n",__LINE__,pTemp_lcml);
                free(pTemp_lcml);
                pTemp_lcml = NULL;
            }

            PCMD_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                                "Memory Allocation Failed");
        }        

        pTemp_lcml->pOpParam->ulFrameCount = DONT_CARE;

        pTemp->nFlags = NORMAL_BUFFER;
        ((PCMDEC_COMPONENT_PRIVATE *)pTemp->pPlatformPrivate)->pHandle = pHandle;
        PCMDEC_DPRINT("::Comp:  >>>>>>>>>>>>> OutBuffHeader[%ld] = %p\n", i, pTemp);
        PCMDEC_DPRINT("::Comp:  >>>> OutBuffHeader[%ld]->pBuffer = %p\n", i, pTemp->pBuffer);
        PCMDEC_DPRINT("::Comp: Op : pTemp_lcml[%ld] = %p\n", i, pTemp_lcml);
        pTemp_lcml++;
    }
    pComponentPrivate->bPortDefsAllocated = 1;
#ifdef __PERF_INSTRUMENTATION__
    pComponentPrivate->nLcml_nCntIp = 0;
    pComponentPrivate->nLcml_nCntOpReceived = 0;
#endif    

    PCMDEC_DPRINT(":: Exiting Fill_LCMLInitParams\n");

    pComponentPrivate->bInitParamsInitialized = 1;

 EXIT:
    PCMDEC_DPRINT("Exiting PCMDEC_Fill_LCMLInitParams\n");
    return eError;
}


/* ================================================================================= * */
/**
* @fn PcmDec_StartCompThread() starts the component thread. This is internal
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
OMX_ERRORTYPE PcmDec_StartCompThread(OMX_HANDLETYPE pComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate =
        (PCMDEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;
    int nRet = 0;

    PCMDEC_DPRINT (":: Enetering  PcmDec_StartCompThread()\n");

    pComponentPrivate->bIsStopping = 0;
    pComponentPrivate->lcml_nOpBuf = 0;
    pComponentPrivate->lcml_nIpBuf = 0;
    pComponentPrivate->app_nBuf = 0;
    pComponentPrivate->num_Op_Issued = 0;
    pComponentPrivate->num_Sent_Ip_Buff = 0;
    pComponentPrivate->num_Reclaimed_Op_Buff = 0;
    pComponentPrivate->bIsEOFSent = 0;

    nRet = pipe (pComponentPrivate->dataPipe);
    if (0 != nRet) {
        PCMD_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                            "Pipe Creation Failed");
    }

    nRet = pipe (pComponentPrivate->cmdPipe);
    if (0 != nRet) {
        PCMD_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                            "Pipe Creation Failed");
    }

    nRet = pipe (pComponentPrivate->cmdDataPipe);
    if (0 != nRet) {
        PCMD_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                            "Pipe Creation Failed");
    }

    nRet = pthread_create (&(pComponentPrivate->ComponentThread), NULL,
                           PCMDEC_ComponentThread, pComponentPrivate);
    if ((0 != nRet) || (!pComponentPrivate->ComponentThread)) {
        PCMD_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                            "Thread Creation Failed");
    }

    pComponentPrivate->bCompThreadStarted = 1;

    PCMDEC_DPRINT (":: Exiting from PcmDec_StartCompThread()\n");

 EXIT:
    return eError;
}


/* ================================================================================= * */
/**
* @fn PCMDEC_FreeCompResources() function frees the component resources.
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
OMX_ERRORTYPE PCMDEC_FreeCompResources(OMX_HANDLETYPE pComponent)
{
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate = (PCMDEC_COMPONENT_PRIVATE *)
        pHandle->pComponentPrivate;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 nIpBuf=0, nOpBuf=0;
    int nRet=0;

    PCMDEC_DPRINT (":: PcmDec_FreeCompResources\n");

    PCMDEC_DPRINT(":::pComponentPrivate->bPortDefsAllocated = %ld\n",pComponentPrivate->bPortDefsAllocated);
    if (pComponentPrivate->bPortDefsAllocated) {
        nIpBuf = pComponentPrivate->pInputBufferList->numBuffers;
        nOpBuf = pComponentPrivate->pOutputBufferList->numBuffers;
    }
    PCMDEC_DPRINT(":: Closing pipess.....\n");

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
        PCMD_OMX_FREE(pComponentPrivate->pPortDef[PCMD_INPUT_PORT]);
        PCMD_OMX_FREE(pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]);
        PCMD_OMX_FREE(pComponentPrivate->pcmParams[PCMD_INPUT_PORT]);
        PCMD_OMX_FREE(pComponentPrivate->pcmParams[PCMD_OUTPUT_PORT]);
        PCMD_OMX_FREE(pComponentPrivate->pCompPort[PCMD_INPUT_PORT]->pPortFormat);
        PCMD_OMX_FREE (pComponentPrivate->pCompPort[PCMD_OUTPUT_PORT]->pPortFormat);
        PCMD_OMX_FREE (pComponentPrivate->pCompPort[PCMD_INPUT_PORT]);
        PCMD_OMX_FREE (pComponentPrivate->pCompPort[PCMD_OUTPUT_PORT]);
        PCMD_OMX_FREE (pComponentPrivate->sPortParam);
        PCMD_OMX_FREE (pComponentPrivate->pPriorityMgmt);
        PCMD_OMX_FREE(pComponentPrivate->pInputBufferList);
        PCMD_OMX_FREE(pComponentPrivate->pOutputBufferList);
    }

    pComponentPrivate->bPortDefsAllocated = 0;
#ifndef UNDER_CE
    PCMDEC_DPRINT("\n\n FreeCompResources: Destroying mutexes.\n\n");
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
* @fn PCMDEC_HandleCommand() function handles the command sent by the application.
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
OMX_U32 PCMDEC_HandleCommand (PCMDEC_COMPONENT_PRIVATE *pComponentPrivate)
{
    OMX_U32 i;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    char *pArgs = "damedesuStr";
    char *p = "damedesuStr";
    OMX_U32 cmdValues[4];
    OMX_U32 bitValues[4];
    
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *) pComponentPrivate->pHandle;
    
    OMX_COMMANDTYPE command;
    OMX_STATETYPE commandedState;
    OMX_U32 commandData;
    OMX_HANDLETYPE pLcmlHandle = pComponentPrivate->pLcmlHandle;

    /* L89FLUSH */
    OMX_U16 arr[8];    


#ifdef RESOURCE_MANAGER_ENABLED
    OMX_ERRORTYPE rm_error = OMX_ErrorNone;
#endif
    PCMDEC_DPRINT (":: >>> Entering HandleCommand Function\n");

    read (pComponentPrivate->cmdPipe[0], &command, sizeof (command));
    read (pComponentPrivate->cmdDataPipe[0], &commandData, sizeof (commandData));
    PCMDEC_DPRINT("---------------------------------------------\n");
    PCMDEC_DPRINT(":: command = %d\n",command);
    PCMDEC_DPRINT(":: commandData = %ld\n",commandData);
    PCMDEC_DPRINT("---------------------------------------------\n");

#ifdef __PERF_INSTRUMENTATION__
    PERF_ReceivedCommand(pComponentPrivate->pPERFcomp,
                         command,
                         commandData,
                         PERF_ModuleLLMM);
#endif    
    if (command == OMX_CommandStateSet)
        {
            commandedState = (OMX_STATETYPE)commandData;
            if (pComponentPrivate->curState == commandedState) {
                pComponentPrivate->cbInfo.EventHandler (
                                                        pHandle, pHandle->pApplicationPrivate,
                                                        OMX_EventError, OMX_ErrorSameState,0,
                                                        NULL);

                PCMDEC_DPRINT(":: Error: Same State Given by Application\n");
            } 
            else{
            switch(commandedState) 
            {

                case OMX_StateIdle:
                    PCMDEC_DPRINT(": HandleCommand: Cmd Idle \n");
                    if (pComponentPrivate->curState == OMX_StateLoaded || pComponentPrivate->curState == OMX_StateWaitForResources) {
                        LCML_CALLBACKTYPE cb;
                        LCML_DSP *pLcmlDsp;
                        char *p = "damedesuStr";
#ifdef __PERF_INSTRUMENTATION__
                        PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryStart | PERF_BoundarySetup);
#endif        
                        int inputPortFlag=0,outputPortFlag=0;
        
                        if (pComponentPrivate->dasfmode == 1) {
                            pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bEnabled= FALSE;
                            pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bPopulated= FALSE;
                            if(pComponentPrivate->streamID == 0)
                                {
                                    PCMDEC_EPRINT("**************************************\n");
                                    PCMDEC_EPRINT(":: Error = OMX_ErrorInsufficientResources\n");
                                    PCMDEC_EPRINT("**************************************\n");
                                    eError = OMX_ErrorInsufficientResources;
                                    pComponentPrivate->curState = OMX_StateInvalid;
                                    pComponentPrivate->cbInfo.EventHandler(
                                                                           pHandle, pHandle->pApplicationPrivate,
                                                                           OMX_EventError, OMX_ErrorInvalidState,0, NULL);                
                                    goto EXIT;
                                }
                        }

                        if (pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bPopulated &&  
                            pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bEnabled)  {
                            inputPortFlag = 1;
                        }

                        if (!pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bPopulated && 
                            !pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bEnabled) {
                            inputPortFlag = 1;
                        }

                        if (pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bPopulated && 
                            pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bEnabled) {
                            outputPortFlag = 1;
                        }

                        if (!pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bPopulated && 
                            !pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bEnabled) {
                            outputPortFlag = 1;
                        }

                        if (!(inputPortFlag && outputPortFlag)){
                            pComponentPrivate->InLoaded_readytoidle = 1;
#ifndef UNDER_CE            
                            pthread_mutex_lock(&pComponentPrivate->InLoaded_mutex); 
                            pthread_cond_wait(&pComponentPrivate->InLoaded_threshold, &pComponentPrivate->InLoaded_mutex);
                            pthread_mutex_unlock(&pComponentPrivate->InLoaded_mutex);
#else        
                            OMX_WaitForEvent(&(pComponentPrivate->InLoaded_event));
#endif
                        }

                        pLcmlHandle = (OMX_HANDLETYPE) PCMDEC_GetLCMLHandle(pComponentPrivate);
                        if (pLcmlHandle == NULL) {
                            PCMDEC_EPRINT(":: LCML Handle is NULL........exiting..\n");
#if 1
                            pComponentPrivate->curState = OMX_StateInvalid;
                   
                            pComponentPrivate->cbInfo.EventHandler(
                                                                   pHandle, pHandle->pApplicationPrivate,
                                                                   OMX_EventError, OMX_ErrorInvalidState,0, NULL);
#endif
                            goto EXIT;
                        }

                        pLcmlDsp = (((LCML_DSP_INTERFACE*)pLcmlHandle)->dspCodec);

                        eError = PCMDEC_Fill_LCMLInitParams(pHandle, pLcmlDsp, arr);
                        if(eError != OMX_ErrorNone) {
                            PCMDEC_EPRINT(":: Error returned from Fill_LCMLInitParams()\n");
#if 1
                            pComponentPrivate->curState = OMX_StateInvalid;
                    
                            pComponentPrivate->cbInfo.EventHandler(
                                                                   pHandle, pHandle->pApplicationPrivate,
                                                                   OMX_EventError, OMX_ErrorInvalidState,0, NULL);
#endif
                            goto EXIT;
                        }

                        pComponentPrivate->pLcmlHandle = (LCML_DSP_INTERFACE *)pLcmlHandle;
                        cb.LCML_Callback = (void *) PCMDEC_LCML_Callback;

#ifndef UNDER_CE
                        eError = LCML_InitMMCodecEx(((LCML_DSP_INTERFACE *)pLcmlHandle)->pCodecinterfacehandle,
                                                    p,&pLcmlHandle,(void *)p,&cb, (OMX_STRING)pComponentPrivate->sDeviceString);
                        if (eError != OMX_ErrorNone)
                            {
                                PCMDEC_DPRINT("%d :: Error : InitMMCodec failed...>>>>>> \n",__LINE__);
                                goto EXIT;
                            }
#else
                        eError = LCML_InitMMCodec(((LCML_DSP_INTERFACE *)pLcmlHandle)->pCodecinterfacehandle,
                                                  p,&pLcmlHandle,(void *)p,&cb);
                        if (eError != OMX_ErrorNone)
                            {
                                PCMDEC_DPRINT("%d :: Error : InitMMCodec failed...>>>>>> \n",__LINE__);
                                goto EXIT;
                            }

#endif

#ifdef RESOURCE_MANAGER_ENABLED
			pComponentPrivate->rmproxyCallback.RMPROXY_Callback =
                                        (void *) PCMD_ResourceManagerCallback;
                    if (pComponentPrivate->curState != OMX_StateWaitForResources){
                            rm_error = RMProxy_NewSendCommand(pHandle,
                                                       RMProxy_RequestResource,
                                                       OMX_PCM_Decoder_COMPONENT, PCM_CPU, NEWSENDCOMMAND_MEMORY, NULL);

                        if(rm_error == OMX_ErrorNone) {
                            PCMDEC_STATEPRINT("****************** Component State Set to Idle\n\n");
                            pComponentPrivate->curState = OMX_StateIdle;
#ifdef __PERF_INSTRUMENTATION__
                            PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundarySetup);
#endif                    
                            pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                                   pHandle->pApplicationPrivate,
                                                                   OMX_EventCmdComplete,
                                                                   OMX_CommandStateSet,
                                                                   pComponentPrivate->curState,
                                                                   NULL);
                            rm_error = RMProxy_NewSendCommand(pHandle, RMProxy_StateSet, OMX_PCM_Decoder_COMPONENT, OMX_StateIdle, NEWSENDCOMMAND_MEMORY, NULL);
                                                       
                        } else if(rm_error == OMX_ErrorInsufficientResources) {
                            pComponentPrivate->curState = OMX_StateWaitForResources;
                            pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                               pHandle->pApplicationPrivate,
                                                               OMX_EventCmdComplete,
                                                               OMX_CommandStateSet,
                                                               pComponentPrivate->curState,
                                                               NULL);
                        }
                    } else {
                        rm_error = RMProxy_NewSendCommand(pHandle,
                                                       RMProxy_StateSet,
                                                       OMX_PCM_Decoder_COMPONENT,
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

                        PCMDEC_DPRINT(":: Control Came Here\n");
                        PCMDEC_STATEPRINT("****************** Component State Set to Idle\n\n");
                        pComponentPrivate->curState = OMX_StateIdle;
#ifdef __PERF_INSTRUMENTATION__
                        PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundarySetup);
#endif                    
                        pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                               pHandle->pApplicationPrivate,
                                                               OMX_EventCmdComplete,
                                                               OMX_CommandStateSet,
                                                               pComponentPrivate->curState,
                                                               NULL);
#endif

                        PCMDEC_DPRINT("PCMDEC: State has been Set to Idle\n");
                    } else if (pComponentPrivate->curState ==
                               OMX_StateExecuting)
                        {
#ifdef __PERF_INSTRUMENTATION__
                            PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundarySteadyState);
#endif            
                            PCMDEC_DPRINT(":: In HandleCommand: Stopping the codec\n");
                            eError = LCML_ControlCodec(
                                                       ((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                       MMCodecControlStop,(void *)pArgs);
                            if(eError != OMX_ErrorNone) {
                                PCMDEC_EPRINT(": Error Occurred in Codec Stop..\n");
#if 1
                                pComponentPrivate->curState = OMX_StateInvalid;
                                pComponentPrivate->cbInfo.EventHandler(
                                                                       pHandle, pHandle->pApplicationPrivate,
                                                                       OMX_EventError, OMX_ErrorInvalidState,0, NULL);
#endif
                                goto EXIT;
                            }
                            /*pComponentPrivate->bIsStopping = 1;*/
                            PCMDEC_DPRINT(":: Send Stop Command...........Sleeping here for 1 sec\n");
                            PCMDEC_DPRINT(":: pComponentPrivate->lcml_nIpBuf = %ld\n",pComponentPrivate->lcml_nIpBuf);
                            PCMDEC_DPRINT(":: pComponentPrivate->lcml_nOpBuf = %ld\n", pComponentPrivate->lcml_nOpBuf);
                            PCMDEC_DPRINT(":: pComponentPrivate->nCntIp = %ld\n", pComponentPrivate->lcml_nCntIp);
                            PCMDEC_DPRINT(":: pComponentPrivate->nCntOp = %ld\n", pComponentPrivate->lcml_nCntOp);
                            PCMDEC_DPRINT(":: pComponentPrivate->nCntOpReceived = %ld\n", pComponentPrivate->lcml_nCntOpReceived);
                            PCMDEC_DPRINT(":: pComponentPrivate->nCntIpRes = %ld\n", pComponentPrivate->lcml_nCntIpRes);

                        } else if(pComponentPrivate->curState == OMX_StatePause) {
                        char *pArgs = "damedesuStr";
                        PCMDEC_DPRINT(":: Comp: Stop Command Received\n");
#ifdef __PERF_INSTRUMENTATION__
                        PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundarySteadyState);
#endif                

                        PCMDEC_DPRINT(": PCMDECUTILS::About to call LCML_ControlCodec\n");
                        eError = LCML_ControlCodec(
                                                   ((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                   MMCodecControlStop,(void *)pArgs);
                        if(eError != OMX_ErrorNone) {
                            PCMDEC_EPRINT(": Error Occurred in Codec Stop..\n");
#ifdef RESOURCE_MANAGER_ENABLED
                            rm_error = RMProxy_NewSendCommand(pHandle, RMProxy_StateSet, OMX_PCM_Decoder_COMPONENT, OMX_StateIdle, NEWSENDCOMMAND_MEMORY, NULL);
#endif                    
                            pComponentPrivate->curState = OMX_StateInvalid;
                            pComponentPrivate->cbInfo.EventHandler(
                                                                   pHandle, pHandle->pApplicationPrivate,
                                                                   OMX_EventError, OMX_ErrorInvalidState,0, NULL);
                            goto EXIT;
                        }
                        PCMDEC_STATEPRINT("****************** Component State Set to Idle\n\n");
                        pComponentPrivate->curState = OMX_StateIdle;
                        pComponentPrivate->cbInfo.EventHandler(pHandle, pHandle->pApplicationPrivate,
                                                               OMX_EventCmdComplete, OMX_CommandStateSet, pComponentPrivate->curState, NULL);

                    } else {
                        PCMDEC_DPRINT(": Comp: Sending ErrorNotification: Invalid State\n");
                        pComponentPrivate->cbInfo.EventHandler(
                                                               pHandle,
                                                               pHandle->pApplicationPrivate,
                                                               OMX_EventError,
                                                               OMX_ErrorIncorrectStateTransition, 0,
                                                               "Invalid State Error");
                    }
                    break;
                

                case OMX_StateExecuting:
                    PCMDEC_DPRINT(": HandleCommand: Cmd Executing \n");

                    if (pComponentPrivate->curState == OMX_StateIdle)
                    {
                        char *pArgs = "damedesuStr";
                        char *pBitSel = NULL;
                       
                        pComponentPrivate->pUalgParam->size = sizeof(PCMDEC_SN_UALGInputParams);
                        pComponentPrivate->pUalgParam->nChannels = pComponentPrivate->pcmParams[PCMD_INPUT_PORT]->nChannels;
                            
                        if ((pComponentPrivate->pUalgParam->nChannels ==  PCMD_STEREO_INTERLEAVED_STREAM) && (pComponentPrivate->dasfmode == 0)) {
                            PCMDEC_DPRINT(":: Set to Mono in File Mode\n");
                            pComponentPrivate->pUalgParam->nChannels = PCMD_MONO_STREAM;
                        }

                        cmdValues[0] = IUALG_CMD_SETSTREAMTYPE;
                        cmdValues[1] = (OMX_U32)pComponentPrivate->pUalgParam;
                        cmdValues[2] = sizeof(PCMDEC_SN_UALGInputParams);

                        p = (void *)&cmdValues;
                        PCMDEC_DPRINT (":: Comp :: After LCML_AlgCtrlCodec function \n");
                        eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle, EMMCodecControlAlgCtrl, (void *)p);
                                                  
                        if (eError != OMX_ErrorNone) {
                            PCMDEC_EPRINT(":: LCML_ControlCodec failed...\n");
                            goto EXIT;
                        }
                        /* Support for 8 or 16 bits */
                        pComponentPrivate->pUSetNumOfBits->size = sizeof(PCMDEC_UlNumOfBitsStatus);
                        pComponentPrivate->pUSetNumOfBits->ulNumOfBits =  pComponentPrivate->pcmParams[PCMD_INPUT_PORT]->nBitPerSample;
                                              
                        bitValues[0] = IUALG_CMD_NUMOFBITS;
                        bitValues[1] = (OMX_U32)pComponentPrivate->pUSetNumOfBits;
                        bitValues[2] = sizeof(PCMDEC_UlNumOfBitsStatus);
                        
                        pBitSel = (void *)&bitValues;
                        
                        PCMDEC_DPRINT (":: Comp :: After LCML_AlgCtrlCodec function \n");
                        
                        eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                   EMMCodecControlAlgCtrl, (void *)pBitSel);
                                                
                        if (eError != OMX_ErrorNone) {
                            PCMDEC_EPRINT(":: LCML_ControlCodec failed in BIT SELECTION , pcmdecUtils.c ...\n");
                            goto EXIT;
                        }
                        PCMDEC_DPRINT ("using %ld bits\n",pComponentPrivate->pUSetNumOfBits->ulNumOfBits);
                        PCMDEC_DPRINT (":: Comp :: After LCML_AlgCtrlCodec function \n");
                        
                        if(pComponentPrivate->dasfmode == 1) {
                            OMX_U32 pValues[4];

                            if (pComponentPrivate->pUalgParam->nChannels == PCMD_MONO_STREAM) {
                                pComponentPrivate->pParams->unAudioFormat = PCMD_MONO_STREAM;
                            } else {
                                pComponentPrivate->pParams->unAudioFormat = PCMD_STEREO_NONINTERLEAVED_STREAM;
                            }
                            pComponentPrivate->pParams->unUUID = pComponentPrivate->streamID;
                            pComponentPrivate->pParams->ulSamplingFreq = pComponentPrivate->pcmParams[PCMD_INPUT_PORT]->nSamplingRate;

                            PCMDEC_DPRINT("::pComponentPrivate->pParams->unAudioFormat   = %d\n",pComponentPrivate->pParams->unAudioFormat);
                            PCMDEC_DPRINT("::pComponentPrivate->pParams->ulSamplingFreq  = %ld\n",pComponentPrivate->pParams->ulSamplingFreq);
                            PCMDEC_DPRINT("::pComponentPrivate->pParams->unUUID = %ld\n",pComponentPrivate->pParams->unUUID);

                            pValues[0] = USN_STRMCMD_SETCODECPARAMS;
                            pValues[1] = (OMX_U32)pComponentPrivate->pParams;
                            pValues[2] = sizeof(PCMD_USN_AudioCodecParams);

                            eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                       EMMCodecControlStrmCtrl,(void *)pValues);
                            if(eError != OMX_ErrorNone) {
                                PCMDEC_EPRINT(": Error Occurred in Codec StreamControl..\n");
                                pComponentPrivate->curState = OMX_StateInvalid;
                                pComponentPrivate->cbInfo.EventHandler(
                                                                       pHandle, pHandle->pApplicationPrivate,
                                                                       OMX_EventError, OMX_ErrorInvalidState,0, NULL);
                                goto EXIT;
                            }
                        }

                        eError = LCML_ControlCodec(
                                                   ((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                   EMMCodecControlStart,(void *)pArgs);
                        if(eError != OMX_ErrorNone) {
                            PCMDEC_EPRINT("Error Occurred in Codec Start..\n");
#if 1
                            pComponentPrivate->curState = OMX_StateInvalid;
                            pComponentPrivate->cbInfo.EventHandler(
                                                                   pHandle, pHandle->pApplicationPrivate,
                                                                   OMX_EventError, OMX_ErrorInvalidState,0, NULL);
#endif
                            goto EXIT;
                        }
                    } else if (pComponentPrivate->curState == OMX_StatePause) {
                        char *pArgs = "damedesuStr";
                        PCMDEC_DPRINT(": Comp: Resume Command Came from App\n");

                        PCMDEC_DPRINT(": PCMDECUTILS::About to call LCML_ControlCodec\n");
                        eError = LCML_ControlCodec(
                                                   ((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                   EMMCodecControlStart,(void *)pArgs);

                        if (eError != OMX_ErrorNone) {
                            PCMDEC_DPRINT ("Error While Resuming the codec\n");
#if 1
                            pComponentPrivate->curState = OMX_StateInvalid;
                            pComponentPrivate->cbInfo.EventHandler(
                                                                   pHandle, pHandle->pApplicationPrivate,
                                                                   OMX_EventError, OMX_ErrorInvalidState,0, NULL);
#endif
                            goto EXIT;
                        }
                        if (pComponentPrivate->nNumInputBufPending < pComponentPrivate->pInputBufferList->numBuffers) {
                            pComponentPrivate->nNumInputBufPending = pComponentPrivate->pInputBufferList->numBuffers;
                        }
                        for (i=0; i < pComponentPrivate->nNumInputBufPending; i++) {
                            if (pComponentPrivate->pInputBufHdrPending[i] != NULL) {
                                PCMD_LCML_BUFHEADERTYPE *pLcmlHdr;

                                PCMDEC_GetCorresponding_LCMLHeader(pComponentPrivate, pComponentPrivate->pInputBufHdrPending[i]->pBuffer, OMX_DirInput, &pLcmlHdr);
                                PCMDEC_SetPending(pComponentPrivate,pComponentPrivate->pInputBufHdrPending[i],OMX_DirInput,__LINE__);
                                eError = LCML_QueueBuffer(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                          EMMCodecInputBuffer,
                                                          pComponentPrivate->pInputBufHdrPending[i]->pBuffer,
                                                          pComponentPrivate->pInputBufHdrPending[i]->nAllocLen,
                                                          pComponentPrivate->pInputBufHdrPending[i]->nFilledLen,
                                                          (OMX_U8 *) pLcmlHdr->pIpParam,
                                                          sizeof(PCMDEC_UAlgInBufParamStruct),
                                                          NULL);
                            }
                        }
                        pComponentPrivate->nNumInputBufPending = 0;
                        if (pComponentPrivate->nNumOutputBufPending < pComponentPrivate->pOutputBufferList->numBuffers) {
                            pComponentPrivate->nNumOutputBufPending = pComponentPrivate->pOutputBufferList->numBuffers;
                        }  
                        for (i=0; i < pComponentPrivate->nNumOutputBufPending; i++) {
                            if (pComponentPrivate->pOutputBufHdrPending[i]) {
                                PCMD_LCML_BUFHEADERTYPE *pLcmlHdr;

                                PCMDEC_GetCorresponding_LCMLHeader(pComponentPrivate, pComponentPrivate->pOutputBufHdrPending[i]->pBuffer, OMX_DirOutput, &pLcmlHdr);

                                if (!PCMDEC_IsPending(pComponentPrivate, pComponentPrivate->pOutputBufHdrPending[i], OMX_DirOutput)) {

                                    PCMDEC_SetPending(pComponentPrivate,pComponentPrivate->pOutputBufHdrPending[i],OMX_DirOutput,__LINE__);

                                    eError = LCML_QueueBuffer(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                              EMMCodecOuputBuffer,
                                                              pComponentPrivate->pOutputBufHdrPending[i]->pBuffer,
                                                              pComponentPrivate->pOutputBufHdrPending[i]->nAllocLen,
                                                              /*pComponentPrivate->pOutputBufHdrPending[i]->nFilledLen,*/
                                                              0,
                                                              /*NULL,*/
                                                              (OMX_U8 *)pLcmlHdr->pOpParam,
                                                              sizeof(PCMDEC_UAlgInBufParamStruct),
                                                              NULL);
                                }
                            }
                        }
                        pComponentPrivate->nNumOutputBufPending = 0;
                    }else {
                        pComponentPrivate->cbInfo.EventHandler (
                                                                pHandle, pHandle->pApplicationPrivate,
                                                                OMX_EventError, OMX_ErrorIncorrectStateTransition, 0,
                                                                "Invalid State");
                        PCMDEC_DPRINT(":: Error: Invalid State Given by \
                       Application\n");
                        goto EXIT;
                    }
#ifdef RESOURCE_MANAGER_ENABLED            
                    rm_error = RMProxy_NewSendCommand(pHandle, RMProxy_StateSet, OMX_PCM_Decoder_COMPONENT, OMX_StateExecuting, NEWSENDCOMMAND_MEMORY, NULL);
#endif            

                    PCMDEC_STATEPRINT("****************** Component State Set to Executing\n\n");
                    pComponentPrivate->curState = OMX_StateExecuting;
#ifdef __PERF_INSTRUMENTATION__
                    PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryStart | PERF_BoundarySteadyState);
#endif            
                    pComponentPrivate->cbInfo.EventHandler(pHandle, pHandle->pApplicationPrivate,
                                                           OMX_EventCmdComplete,
                                                           OMX_CommandStateSet, pComponentPrivate->curState, NULL);
                    break;
                
                case OMX_StateLoaded:
                    PCMDEC_DPRINT(": HandleCommand: Cmd Loaded\n");

                    if (pComponentPrivate->curState == OMX_StateWaitForResources ){
                        PCMDEC_STATEPRINT("***************** Component State Set to Loaded\n\n");
#ifdef __PERF_INSTRUMENTATION__
                        PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryStart | PERF_BoundaryCleanup); 
#endif                
                        pComponentPrivate->curState = OMX_StateLoaded;
#ifdef __PERF_INSTRUMENTATION__
                        PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundaryCleanup);
#endif                
                        pComponentPrivate->cbInfo.EventHandler (
                                                                pHandle, pHandle->pApplicationPrivate,
                                                                OMX_EventCmdComplete, OMX_CommandStateSet,pComponentPrivate->curState,
                                                                NULL);
                        PCMDEC_DPRINT(":: Tansitioning from WaitFor to Loaded\n");
                        break;
                    }

                    if (pComponentPrivate->curState != OMX_StateIdle) {
                        pComponentPrivate->cbInfo.EventHandler (
                                                                pHandle, pHandle->pApplicationPrivate,
                                                                OMX_EventError, OMX_ErrorIncorrectStateTransition, 0,
                                                                "Invalid State");
                        PCMDEC_DPRINT(":: Error: Invalid State Given by \
                       Application\n");
                        goto EXIT;
                    }
#ifdef __PERF_INSTRUMENTATION__
                    PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryStart | PERF_BoundaryCleanup);
#endif

                    PCMDEC_DPRINT("Current State = %d\n",pComponentPrivate->curState);
                    PCMDEC_DPRINT("pComponentPrivate->pInputBufferList->numBuffers = %ld\n",pComponentPrivate->pInputBufferList->numBuffers);
                    PCMDEC_DPRINT("pComponentPrivate->pOutputBufferList->numBuffers = %ld\n",pComponentPrivate->pOutputBufferList->numBuffers);

                    if (pComponentPrivate->pInputBufferList->numBuffers && 
                        pComponentPrivate->pOutputBufferList->numBuffers) {
                        pComponentPrivate->InIdle_goingtoloaded = 1;                
#ifndef UNDER_CE                
                        pthread_mutex_lock(&pComponentPrivate->InIdle_mutex); 
                        pthread_cond_wait(&pComponentPrivate->InIdle_threshold, &pComponentPrivate->InIdle_mutex);
                        pthread_mutex_unlock(&pComponentPrivate->InIdle_mutex);
#else
                        OMX_WaitForEvent(&(pComponentPrivate->InIdle_event));
#endif
                    }

                    eError = LCML_ControlCodec(
                                               ((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               EMMCodecControlDestroy,(void *)pArgs);
#ifdef __PERF_INSTRUMENTATION__
                    PERF_SendingCommand(pComponentPrivate->pPERF, -1, 0, PERF_ModuleComponent);
#endif                                  
#ifndef UNDER_CE
                    /*Closing LCML Lib*/
                    if (pComponentPrivate->ptrLibLCML != NULL)
                        {
                            PCMDEC_DPRINT("PCM: About to Close LCML %p \n",pComponentPrivate->ptrLibLCML);
                            dlclose( pComponentPrivate->ptrLibLCML  ); 
                            pComponentPrivate->ptrLibLCML = NULL;
                            PCMDEC_DPRINT("PCM: Closed LCML \n");
                        }
#endif
                    pComponentPrivate->bInitParamsInitialized = 0;
                    eError = EXIT_COMPONENT_THRD;
                    pComponentPrivate->bLoadedCommandPending = 0;  /*setting to OMX_False;*/
                    break;
                
                case OMX_StatePause:
                   PCMDEC_DPRINT("Cmd Pause: Cur State = %d\n", pComponentPrivate->curState);

                    if ((pComponentPrivate->curState != OMX_StateExecuting) &&
                        (pComponentPrivate->curState != OMX_StateIdle)) {
                        pComponentPrivate->cbInfo.EventHandler (
                                                                pHandle, pHandle->pApplicationPrivate,
                                                                OMX_EventError, OMX_ErrorIncorrectStateTransition, 0,
                                                                "Invalid State");
                        PCMDEC_DPRINT(":: Error: Invalid State Given by \
                       Application\n");
                        goto EXIT;
                    }
#ifdef __PERF_INSTRUMENTATION__
                    PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundarySteadyState);
#endif

                    PCMDEC_DPRINT(": PCMDECUTILS::About to call LCML_ControlCodec\n");
                    eError = LCML_ControlCodec(
                                               ((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               EMMCodecControlPause,(void *)pArgs);

                    if (eError != OMX_ErrorNone) {
                        PCMDEC_DPRINT(": Error: in Pausing the codec\n");
#if 1
                        pComponentPrivate->curState = OMX_StateInvalid;
                        pComponentPrivate->cbInfo.EventHandler(
                                                               pHandle, pHandle->pApplicationPrivate,
                                                               OMX_EventError, OMX_ErrorInvalidState,0, NULL);
#endif
                        goto EXIT;
                    }
                    
                    PCMDEC_DPRINT(":: Component: Codec Is Paused\n");
                    PCMDEC_STATEPRINT("****************** Component State Set to Pause\n\n");
#if 0
                    pComponentPrivate->curState = OMX_StatePause;
                    pComponentPrivate->cbInfo.EventHandler(pHandle, pHandle->pApplicationPrivate,
                                                           OMX_EventCmdComplete, OMX_CommandStateSet,
                                                           pComponentPrivate->curState, NULL);
#endif
                    break;
                
                case OMX_StateWaitForResources:
                   PCMDEC_DPRINT(": HandleCommand: Cmd : OMX_StateWaitForResources\n");
                    if (pComponentPrivate->curState == OMX_StateLoaded) 
                    {
                        pComponentPrivate->curState = OMX_StateWaitForResources;
                        PCMDEC_DPRINT(": Transitioning from Loaded to OMX_StateWaitForResources\n");
                        pComponentPrivate->cbInfo.EventHandler(
                                                               pHandle, pHandle->pApplicationPrivate,
                                                               OMX_EventCmdComplete,
                                                               OMX_CommandStateSet,pComponentPrivate->curState, NULL);
                    } else {
                        pComponentPrivate->cbInfo.EventHandler(
                                                               pHandle, pHandle->pApplicationPrivate,
                                                               OMX_EventError, OMX_ErrorIncorrectStateTransition,0, NULL);
                    }
                    break;
                
                case OMX_StateInvalid:
                    PCMDEC_DPRINT(": HandleCommand: Cmd OMX_StateInvalid:\n");

                    if (pComponentPrivate->curState != OMX_StateWaitForResources && pComponentPrivate->curState != OMX_StateLoaded &&
                        pComponentPrivate->curState != OMX_StateInvalid) 
                    {
                        eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                   EMMCodecControlDestroy, (void *)p);    
                    }
        
                    pComponentPrivate->curState = OMX_StateInvalid;
                    pComponentPrivate->cbInfo.EventHandler(
                                                           pHandle, pHandle->pApplicationPrivate,
                                                           OMX_EventError, OMX_ErrorInvalidState,0, NULL);

                    PCMDEC_CleanupInitParams(pHandle);

                    break;
                
                case OMX_StateMax:
                    PCMDEC_DPRINT(": HandleCommand: Cmd OMX_StateMax::\n");
                    break;
                
            } /* End of Switch */
            }
        }
    else if (command == OMX_CommandMarkBuffer) {
        PCMDEC_DPRINT("command OMX_CommandMarkBuffer received\n");
        if(!pComponentPrivate->pMarkBuf){
            PCMDEC_DPRINT("command OMX_CommandMarkBuffer received \n");
            pComponentPrivate->pMarkBuf = (OMX_MARKTYPE *)(commandData);
        }
    }

    else if (command == OMX_CommandPortDisable) {
        if (!pComponentPrivate->bDisableCommandPending) {
            PCMDEC_DPRINT("I'm here Line \n");

            if(commandData == 0x0){ 

                /* disable port */
                for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
                    if (PCMDEC_IsPending(pComponentPrivate,pComponentPrivate->pInputBufferList->pBufHdr[i],OMX_DirInput)) {
                        /* Real solution is flush buffers from DSP.  Until we have the ability to do that 
                           we just call EmptyBufferDone() on any pending buffers */
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
                        pComponentPrivate->nEmptyBufferDoneCount++;
                    }
                }
                pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bEnabled = OMX_FALSE;
            }
            if(commandData == -1){
                pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bEnabled = OMX_FALSE;
            }


            if(commandData == 0x1 || commandData == -1){
                char *pArgs = "damedesuStr";


                pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bEnabled = OMX_FALSE;
                if (pComponentPrivate->curState == OMX_StateExecuting) {
                    pComponentPrivate->bNoIdleOnStop = OMX_TRUE;
                    eError = LCML_ControlCodec(
                                               ((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               MMCodecControlStop,(void *)pArgs);
                }

            }

        }
        PCMDEC_DPRINT("commandData = %ld\n",commandData);
        PCMDEC_DPRINT("pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bPopulated = %d\n",pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bPopulated);
        PCMDEC_DPRINT("pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bPopulated = %d\n",pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bPopulated);
        if(commandData == 0x0) {
            if(!pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bPopulated){
                /* return cmdcomplete event if input unpopulated */ 
                pComponentPrivate->cbInfo.EventHandler(
                                                       pHandle, pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete, OMX_CommandPortDisable,PCMD_INPUT_PORT, NULL);
                pComponentPrivate->bDisableCommandPending = 0;
            }
            else{
                pComponentPrivate->bDisableCommandPending = 1;
                pComponentPrivate->bDisableCommandParam = commandData;
            }
        }

        if(commandData == 0x1) {
            if (!pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bPopulated){
                /* return cmdcomplete event if output unpopulated */ 
                pComponentPrivate->cbInfo.EventHandler(
                                                       pHandle, pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete, OMX_CommandPortDisable,PCMD_OUTPUT_PORT, NULL);
                pComponentPrivate->bDisableCommandPending = 0;
            }
            else {
                pComponentPrivate->bDisableCommandPending = 1;
                pComponentPrivate->bDisableCommandParam = commandData;
            }
        }

        if(commandData == -1) {
            if (!pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bPopulated && 
                !pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bPopulated){

                /* return cmdcomplete event if inout & output unpopulated */ 
                pComponentPrivate->cbInfo.EventHandler(
                                                       pHandle, pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete, OMX_CommandPortDisable,PCMD_INPUT_PORT, NULL);

                pComponentPrivate->cbInfo.EventHandler(
                                                       pHandle, pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete, OMX_CommandPortDisable,PCMD_OUTPUT_PORT, NULL);
                pComponentPrivate->bDisableCommandPending = 0;
            }
            else {
                pComponentPrivate->bDisableCommandPending = 1;
                pComponentPrivate->bDisableCommandParam = commandData;
            }
        }
    }
    else if (command == OMX_CommandPortEnable) {
        if (!pComponentPrivate->bEnableCommandPending) {
            if(commandData == 0x0 || commandData == -1){
                /* enable in port */
                PCMDEC_DPRINT("setting input port to enabled\n");
                pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bEnabled = OMX_TRUE;
                PCMDEC_DPRINT("pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bEnabled = %d\n",pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bEnabled);

                if(pComponentPrivate->AlloBuf_waitingsignal){
                    pComponentPrivate->AlloBuf_waitingsignal = 0;
                }
            }
            if(commandData == 0x1 || commandData == -1){
                /* enable out port */
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

                if(pComponentPrivate->curState == OMX_StateExecuting){
                    pComponentPrivate->bDspStoppedWhileExecuting = OMX_FALSE;
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               EMMCodecControlStart,
                                               (void *)pArgs);
                }
                PCMDEC_DPRINT("setting output port to enabled\n");
                pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bEnabled = OMX_TRUE;
                PCMDEC_DPRINT("pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bEnabled = %d\n",pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bEnabled);
            }
        }

        PCMDEC_DPRINT("commandData = %ld\n",commandData);
        PCMDEC_DPRINT("pComponentPrivate->curState = %d\n",pComponentPrivate->curState);
        PCMDEC_DPRINT("pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bPopulated = %d\n",pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bPopulated);

        if(commandData == 0x0 ){
            if(pComponentPrivate->curState == OMX_StateLoaded ||
               pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bPopulated){
                pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                       pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete, 
                                                       OMX_CommandPortEnable,
                                                       PCMD_INPUT_PORT,
                                                       NULL);
                pComponentPrivate->bEnableCommandPending = 0;
            }
            else {
                pComponentPrivate->bEnableCommandPending = 1;
                pComponentPrivate->bEnableCommandParam = commandData;
            }
        }
        else if(commandData == 0x1){
            if(pComponentPrivate->curState == OMX_StateLoaded ||
               pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bPopulated){
                pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                       pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete, 
                                                       OMX_CommandPortEnable,
                                                       PCMD_OUTPUT_PORT,
                                                       NULL);
                pComponentPrivate->bEnableCommandPending = 0;
            }
            else {
                pComponentPrivate->bEnableCommandPending = 1;
                pComponentPrivate->bEnableCommandParam = commandData;
            }
        }
        else if(commandData == -1 ){
            if (pComponentPrivate->curState == OMX_StateLoaded ||
                (pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bPopulated &&
                 pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bPopulated)) {
                pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                       pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete, 
                                                       OMX_CommandPortEnable,
                                                       PCMD_INPUT_PORT,NULL);

                pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                       pHandle->pApplicationPrivate,OMX_EventCmdComplete, 
                                                       OMX_CommandPortEnable,
                                                       PCMD_OUTPUT_PORT,
                                                       NULL);
                pComponentPrivate->bEnableCommandPending = 0;
                PCMDECFill_LCMLInitParamsEx(pHandle);
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
        OMX_U32 aParam[3] = {0};
        PCMDEC_DPRINT(":: Inside OMX_CommandFlush Command\n");
        if(commandData == 0x0 || commandData == -1) {

            aParam[0] = USN_STRMCMD_FLUSH; 
            aParam[1] = 0x0; 
            aParam[2] = 0x0; 

            PCMDEC_DPRINT("Flushing input port\n");
            eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,EMMCodecControlStrmCtrl, (void*)aParam);
            if (eError != OMX_ErrorNone) {
                goto EXIT;
            }
        }
        if(commandData == 0x1 || commandData == -1){
            PCMDEC_DPRINT("Flushing output queue\n");

            aParam[0] = USN_STRMCMD_FLUSH; 
            aParam[1] = 0x1; 
            aParam[2] = 0x0; 

            PCMDEC_DPRINT("Flushing output port\n");
            eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,EMMCodecControlStrmCtrl, (void*)aParam);
            if (eError != OMX_ErrorNone) {
                goto EXIT;
            }
        }
    }
 EXIT:
    PCMDEC_DPRINT (":: Exiting HandleCommand Function\n");
    return eError;
}



/* ================================================================================= * */
/**
* @fn PCMDEC_HandleDataBuf_FromApp() function handles the input and output buffers
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
OMX_ERRORTYPE PCMDEC_HandleDataBuf_FromApp(OMX_BUFFERHEADERTYPE* pBufHeader,
                                           PCMDEC_COMPONENT_PRIVATE *pComponentPrivate)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_DIRTYPE eDir;
    int retval;
    OMX_U32 nAllocLen;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDefIn = NULL;
    
    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate_CC;
    pComponentPrivate_CC = pComponentPrivate;

    PCMDEC_DPRINT (":: Entering HandleDataBuf_FromApp Function\n");
    PCMDEC_DPRINT (":: pBufHeader->pMarkData = %p\n",pBufHeader->pMarkData);     

    pBufHeader->pPlatformPrivate  = pComponentPrivate;
    eError = PCMDEC_GetBufferDirection(pBufHeader, &eDir);
    PCMDEC_DPRINT (":: HandleDataBuf_FromApp Function\n");
    if (eError != OMX_ErrorNone) {
        PCMDEC_EPRINT (":: The pBufHeader is not found in the list\n");
        goto EXIT;
    }

    if (eDir == OMX_DirInput) {
        LCML_DSP_INTERFACE *pLcmlHandle = (LCML_DSP_INTERFACE *)
            pComponentPrivate->pLcmlHandle;
        PCMD_LCML_BUFHEADERTYPE *pLcmlHdr;
        pPortDefIn = pComponentPrivate->pPortDef[OMX_DirInput];

        eError = PCMDEC_GetCorresponding_LCMLHeader(pComponentPrivate,pBufHeader->pBuffer, OMX_DirInput, &pLcmlHdr);
        if (eError != OMX_ErrorNone) {
            PCMDEC_EPRINT(":: Error: Invalid Buffer Came ...\n");
            goto EXIT;
        }

        PCMDEC_DPRINT(":: pBufHeader->nFilledLen = %ld\n",pBufHeader->nFilledLen);

        if (pBufHeader->nFilledLen > 0) {
            pComponentPrivate->bBypassDSP = 0;
            PCMDEC_DPRINT (":: HandleDataBuf_FromApp Function\n");
            PCMDEC_DPRINT (":::Calling LCML_QueueBuffer\n");
#ifdef __PERF_INSTRUMENTATION__
            /*For Steady State Instumentation*/
            PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                              PREF(pBufHeader,pBuffer),
                              pPortDefIn->nBufferSize, 
                              PERF_ModuleCommonLayer);
#endif

                                

            if(pBufHeader->nFlags == OMX_BUFFERFLAG_EOS) {
                pLcmlHdr->pIpParam->bLastBuffer = 1;
            }
        
            nAllocLen = pBufHeader->nAllocLen;

            if ( (pComponentPrivate->bLowLatencyDisabled == 1) &&
                 (pBufHeader->nFilledLen < pBufHeader->nAllocLen) &&
                 (pComponentPrivate->dasfmode == 1)
                 )  {    /* Workaround to disable SN low latency feature */
                nAllocLen = pBufHeader->nFilledLen;
            }

            if (pComponentPrivate->dasfmode == 0)
            {
                    /* Store time stamp information */
                    pComponentPrivate->arrBufIndex[pComponentPrivate->IpBufindex] = pBufHeader->nTimeStamp;
                    /* Store nTickCount information */
                    pComponentPrivate->arrTickCount[pComponentPrivate->IpBufindex] = pBufHeader->nTickCount;
                    pComponentPrivate->IpBufindex++;
                    pComponentPrivate->IpBufindex %= pPortDefIn->nBufferCountActual;
                    PCMDEC_DPRINT("Input buffer TS = %lli at header: %p\n",pComponentPrivate->arrBufIndex[pComponentPrivate->IpBufindex],pBufHeader);
            }
        
            if (pComponentPrivate->curState == OMX_StateExecuting) {
                if (!PCMDEC_IsPending(pComponentPrivate,pBufHeader,OMX_DirInput)) {
                    if(!pComponentPrivate->bDspStoppedWhileExecuting) {
                        PCMDEC_SetPending(pComponentPrivate,pBufHeader,OMX_DirInput,__LINE__);
                        eError = LCML_QueueBuffer(pLcmlHandle->pCodecinterfacehandle,
                                                  EMMCodecInputBuffer,
                                                  pBufHeader->pBuffer,
                                                  nAllocLen,
                                                  pBufHeader->nFilledLen,
                                                  (OMX_U8 *) pLcmlHdr->pIpParam,
                                                  sizeof(PCMDEC_UAlgInBufParamStruct),
                                                  pBufHeader->pBuffer);            
                    }
                    else {
#ifdef __PERF_INSTRUMENTATION__
                        PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                          PREF(pBufHeader, pBuffer),
                                          0,
                                          PERF_ModuleHLMM);
#endif    
                        pComponentPrivate->cbInfo.EmptyBufferDone (
                                                                   pComponentPrivate->pHandle,
                                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                                   pBufHeader
                                                                   );
                        pComponentPrivate->nOutStandingEmptyDones--;
                        if (pComponentPrivate->bIdleCommandPending) {
                            TransitionToIdle(pComponentPrivate);
                        }        
                    }
                    if (eError != OMX_ErrorNone) {
                        PCMDEC_DPRINT ("::Comp: SetBuff: IP: Error Occurred\n");
                        eError = OMX_ErrorHardware;
                        goto EXIT;
                    }

                    pComponentPrivate->lcml_nCntIp++;
                    pComponentPrivate->lcml_nIpBuf++;
                    pComponentPrivate->num_Sent_Ip_Buff++;
                    PCMDEC_DPRINT ("Sending Input buffer to Codec\n");
                }
            } 
            else if (pComponentPrivate->curState == OMX_StatePause) {
                pComponentPrivate->pInputBufHdrPending[pComponentPrivate->nNumInputBufPending++] = pBufHeader;
            }
        }
        else {
            pComponentPrivate->bBypassDSP = 1;
            PCMDEC_DPRINT ("Forcing FillBufferDone\n");

            if(pComponentPrivate->dasfmode == 0) {
                if (pComponentPrivate->bIsEOFSent) {
                    pComponentPrivate->pOutputBufferList->pBufHdr[0]->nFlags |= OMX_BUFFERFLAG_EOS;

                    pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                           pComponentPrivate->pHandle->pApplicationPrivate,
                                                           OMX_EventBufferFlag,
                                                           pComponentPrivate->pOutputBufferList->pBufHdr[0]->nOutputPortIndex,
                                                           pComponentPrivate->pOutputBufferList->pBufHdr[0]->nFlags, NULL);

                    pComponentPrivate->bIsEOFSent = 0;        
                }            
#ifdef __PERF_INSTRUMENTATION__
                PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                  PREF(pComponentPrivate->pOutputBufferList->pBufHdr[0],pBuffer),
                                  PREF(pComponentPrivate->pOutputBufferList->pBufHdr[0],nFilledLen),
                                  PERF_ModuleHLMM);
#endif    
                pComponentPrivate->cbInfo.FillBufferDone (
                                                          pComponentPrivate->pHandle,
                                                          pComponentPrivate->pHandle->pApplicationPrivate,
                                                          pComponentPrivate->pOutputBufferList->pBufHdr[0]
                                                          );
                
                pComponentPrivate->nFillBufferDoneCount++;

#ifdef __PERF_INSTRUMENTATION__
                PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                  PREF(pComponentPrivate->pInputBufferList->pBufHdr[0], pBuffer),
                                  0,
                                  PERF_ModuleHLMM);
#endif    
                pComponentPrivate->cbInfo.EmptyBufferDone (
                                                           pComponentPrivate->pHandle,
                                                           pComponentPrivate->pHandle->pApplicationPrivate,
                                                           pComponentPrivate->pInputBufferList->pBufHdr[0]
                                                           );
                pComponentPrivate->nEmptyBufferDoneCount++;
                pComponentPrivate->nOutStandingEmptyDones--;
                if (pComponentPrivate->bIdleCommandPending) {
                    TransitionToIdle(pComponentPrivate);
                }
            }
        }
                                
        if(pBufHeader->nFlags == OMX_BUFFERFLAG_EOS) {
            pComponentPrivate->bIsEOFSent = 1;

            if(pComponentPrivate->dasfmode == 0) {
                /* copy flag to output buffer header */
                pComponentPrivate->pOutputBufferList->pBufHdr[0]->nFlags |= OMX_BUFFERFLAG_EOS;

                pComponentPrivate->cbInfo.EventHandler(
                                                       pComponentPrivate->pHandle,
                                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                                       OMX_EventBufferFlag,
                                                       pComponentPrivate->pOutputBufferList->pBufHdr[0]->nOutputPortIndex,
                                                       pComponentPrivate->pOutputBufferList->pBufHdr[0]->nFlags,
                                                       NULL);
            } 
            else {
                pComponentPrivate->cbInfo.EventHandler(
                                                       pComponentPrivate->pHandle,
                                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                                       OMX_EventBufferFlag,
                                                       pBufHeader->nInputPortIndex,
                                                       pBufHeader->nFlags,
                                                       NULL);                
            }
            PCMDEC_DPRINT(":: Component Setting nFlags to zero\n");            
            pBufHeader->nFlags = 0;            
        }
        if(pBufHeader->pMarkData){
            PCMDEC_DPRINT (":Detected pBufHeader->pMarkData\n");

            pComponentPrivate->pMarkData = pBufHeader->pMarkData;
            pComponentPrivate->hMarkTargetComponent = pBufHeader->hMarkTargetComponent;
            pComponentPrivate->pOutputBufferList->pBufHdr[0]->pMarkData = pBufHeader->pMarkData;
            pComponentPrivate->pOutputBufferList->pBufHdr[0]->hMarkTargetComponent = pBufHeader->hMarkTargetComponent;        

            if(pBufHeader->hMarkTargetComponent == pComponentPrivate->pHandle && pBufHeader->pMarkData){
                pComponentPrivate_CC->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                          pComponentPrivate->pHandle->pApplicationPrivate,
                                                          OMX_EventMark,
                                                          0,
                                                          0,
                                                          pBufHeader->pMarkData);
            }
        }
    }
    else if (eDir == OMX_DirOutput) {

        LCML_DSP_INTERFACE *pLcmlHandle = (LCML_DSP_INTERFACE *)
            pComponentPrivate->pLcmlHandle;
        PCMD_LCML_BUFHEADERTYPE *pLcmlHdr;
        PCMDEC_DPRINT(": pComponentPrivate->lcml_nOpBuf = %ld\n",pComponentPrivate->lcml_nOpBuf);
        PCMDEC_DPRINT(": pComponentPrivate->lcml_nIpBuf = %ld\n",pComponentPrivate->lcml_nIpBuf);
        eError = PCMDEC_GetCorresponding_LCMLHeader(pComponentPrivate, pBufHeader->pBuffer, OMX_DirOutput, &pLcmlHdr);
        if (eError != OMX_ErrorNone) {
            PCMDEC_DPRINT(":: Error: Invalid Buffer Came ...\n");
            goto EXIT;
        }

        PCMDEC_DPRINT (":::Calling LCML_QueueBuffer\n");
#ifdef __PERF_INSTRUMENTATION__
        PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                          PREF(pBufHeader,pBuffer),
                          0,
                          PERF_ModuleCommonLayer);
#endif

        if(!(pComponentPrivate->bIsStopping)) {
            if (pComponentPrivate->bBypassDSP == 0) {
                PCMDEC_DPRINT ("Comp:: Sending Emptied Output buffer=%p to LCML\n",pBufHeader);
                  
                if (pComponentPrivate->curState == OMX_StateExecuting) {
                    PCMDEC_DPRINT ("Comp:: in PCMDEC UTILS pLcmlHandle->pCodecinterfacehandle= %p\n",pLcmlHandle->pCodecinterfacehandle);
                    PCMDEC_DPRINT ("Comp:: in PCMDEC UTILS EMMCodecOuputBuffer = %d\n",EMMCodecOuputBuffer);
                    PCMDEC_DPRINT ("Comp:: in PCMDEC UTILS pBufHeader->pBuffer = %p\n",pBufHeader->pBuffer);
                    PCMDEC_DPRINT ("Comp:: in PCMDEC UTILS pBufHeader->nAllocLen = %ld\n",pBufHeader->nAllocLen);

                    PCMDEC_DPRINT("Supposed to be calling PCMDEC_SetPending for output\n");
                    retval = PCMDEC_IsPending(pComponentPrivate,pBufHeader,OMX_DirOutput);
                    PCMDEC_DPRINT("PCMDEC_IsPending(pComponentPrivate,pBufHeader,OMX_DirOutput) returned %d\n",retval);
                    PCMDEC_DPRINT("PCMDEC_IsPending(pComponentPrivate,pBufHeader,OMX_DirOutput) returned %d\n",retval);
                    PCMDEC_DPRINT("pComponentPrivate->numPendingBuffers = %ld\n",pComponentPrivate->numPendingBuffers);
                    if (!PCMDEC_IsPending(pComponentPrivate,pBufHeader,OMX_DirOutput) && 
                        (pComponentPrivate->numPendingBuffers < pComponentPrivate->pOutputBufferList->numBuffers))  {
                        PCMDEC_SetPending(pComponentPrivate,pBufHeader,OMX_DirOutput,__LINE__);
                        eError = LCML_QueueBuffer(
                                                  pLcmlHandle->pCodecinterfacehandle,
                                                  EMMCodecOuputBuffer,
                                                  pBufHeader->pBuffer,
                                                  pBufHeader->nAllocLen,
                                                  0,
                                                  NULL,
                                                  sizeof(PCMDEC_UAlgInBufParamStruct),
                                                  pBufHeader->pBuffer);
                        PCMDEC_DPRINT ("Comp:: in PCMDEC UTILS = \n");
                        if (eError != OMX_ErrorNone ) {
                            PCMDEC_EPRINT (":: Comp:: SetBuff OP: Error Occurred\n");
                            eError = OMX_ErrorHardware;
                            goto EXIT;
                        }
                        PCMDEC_DPRINT ("Comp:: in PCMDEC UTILS \n");
                        pComponentPrivate->lcml_nCntOp++;
                        pComponentPrivate->lcml_nOpBuf++;
                        pComponentPrivate->num_Op_Issued++;
                        PCMDEC_DPRINT ("Comp:: in PCMDEC UTILS \n");
                    }
                } else {
                    pComponentPrivate->pOutputBufHdrPending[pComponentPrivate->nNumOutputBufPending++] = pBufHeader;
                }
            }
        } else {
            if (pComponentPrivate->curState == OMX_StateExecuting) {
                PCMDEC_DPRINT ("Comp:: in PCMDEC UTILS pLcmlHandle->pCodecinterfacehandle= %p\n",pLcmlHandle->pCodecinterfacehandle);
                PCMDEC_DPRINT ("Comp:: in PCMDEC UTILS EMMCodecOuputBuffer = %d\n",EMMCodecOuputBuffer);
                PCMDEC_DPRINT ("Comp:: in PCMDEC UTILS pBufHeader->pBuffer = %p\n",pBufHeader->pBuffer);
                PCMDEC_DPRINT ("Comp:: in PCMDEC UTILS pBufHeader->nAllocLen = %ld\n",pBufHeader->nAllocLen);

                PCMDEC_DPRINT("Supposed to be calling PCMDEC_SetPending for output\n");
                retval = PCMDEC_IsPending(pComponentPrivate,pBufHeader,OMX_DirOutput);
                PCMDEC_DPRINT("PCMDEC_IsPending(pComponentPrivate,pBufHeader,OMX_DirOutput) returned %d\n",retval);
                PCMDEC_DPRINT("PCMDEC_IsPending(pComponentPrivate,pBufHeader,OMX_DirOutput) returned %d\n",retval);
                PCMDEC_DPRINT("pComponentPrivate->numPendingBuffers = %ld\n",pComponentPrivate->numPendingBuffers);
                if (!PCMDEC_IsPending(pComponentPrivate,pBufHeader,OMX_DirOutput) && 
                    (pComponentPrivate->numPendingBuffers < pComponentPrivate->pOutputBufferList->numBuffers))  {
                    PCMDEC_SetPending(pComponentPrivate,pBufHeader,OMX_DirOutput,__LINE__);
                    eError = LCML_QueueBuffer(
                                              pLcmlHandle->pCodecinterfacehandle,
                                              EMMCodecOuputBuffer,
                                              pBufHeader->pBuffer,
                                              pBufHeader->nAllocLen,
                                              0,
                                              NULL,
                                              sizeof(PCMDEC_UAlgInBufParamStruct),
                                              pBufHeader->pBuffer);
                    PCMDEC_DPRINT ("Comp:: in PCMDEC UTILS = \n");
                    if (eError != OMX_ErrorNone ) {
                        PCMDEC_EPRINT (":: Comp:: SetBuff OP: Error Occurred\n");
                        eError = OMX_ErrorHardware;
                        goto EXIT;
                    }
                    PCMDEC_DPRINT ("Comp:: in PCMDEC UTILS \n");
                    pComponentPrivate->lcml_nCntOp++;
                    pComponentPrivate->lcml_nOpBuf++;
                    pComponentPrivate->num_Op_Issued++;
                    PCMDEC_DPRINT ("Comp:: in PCMDEC UTILS \n");
                }
            } else {
                pComponentPrivate->pOutputBufHdrPending[pComponentPrivate->nNumOutputBufPending++] = pBufHeader;
            }
        } /* end of else */
    } /* end of OMX_DirOutput if struct */
    else {
        PCMDEC_DPRINT(": BufferHeader %p, Buffer %p Unknown ..........\n",pBufHeader, pBufHeader->pBuffer);
        eError = OMX_ErrorBadParameter;
    }
 EXIT:
    PCMDEC_DPRINT(": Exiting from  HandleDataBuf_FromApp: %x \n",eError);
    if(eError == OMX_ErrorBadParameter) {
        PCMDEC_DPRINT(": Error = OMX_ErrorBadParameter\n");
    }
    return eError;
}


/* ================================================================================= * */
/**
* @fn PCMDEC_GetBufferDirection() function determines whether it is input buffer or
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

OMX_ERRORTYPE PCMDEC_GetBufferDirection(OMX_BUFFERHEADERTYPE *pBufHeader,
                                        OMX_DIRTYPE *eDir)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate = pBufHeader->pPlatformPrivate;
    OMX_U32 nBuf = pComponentPrivate->pInputBufferList->numBuffers;
    OMX_BUFFERHEADERTYPE *pBuf = NULL;
    int flag = 1;
    OMX_U32 i=0;

    PCMDEC_DPRINT (":: Entering GetBufferDirection Function\n");
    for(i=0; i<nBuf; i++) {
        pBuf = pComponentPrivate->pInputBufferList->pBufHdr[i];
        if(pBufHeader == pBuf) {
            *eDir = OMX_DirInput;
            PCMDEC_DPRINT (":: Buffer %p is INPUT BUFFER\n", pBufHeader);
            flag = 0;
            goto EXIT;
        }
    }

    nBuf = pComponentPrivate->pOutputBufferList->numBuffers;

    for(i=0; i<nBuf; i++) {
        pBuf = pComponentPrivate->pOutputBufferList->pBufHdr[i];
        if(pBufHeader == pBuf) {
            *eDir = OMX_DirOutput;
            PCMDEC_DPRINT (":: Buffer %p is OUTPUT BUFFER\n", pBufHeader);
            flag = 0;
            goto EXIT;
        }
    }

    if (flag == 1) {
        PCMD_OMX_ERROR_EXIT(eError, OMX_ErrorBadParameter,
                            "Buffer Not Found in List : OMX_ErrorBadParameter");
    }
 EXIT:
    PCMDEC_DPRINT (":: Exiting GetBufferDirection Function\n");
    return eError;
}


/* ================================================================================= * */
/**
* @fn PCMDEC_LCML_Callback() function is callback which is called by LCML whenever
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
OMX_ERRORTYPE PCMDEC_LCML_Callback (TUsnCodecEvent event,void * args [10])
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U8 *pBuffer = args[1];
    PCMD_LCML_BUFHEADERTYPE *pLcmlHdr;
    OMX_COMPONENTTYPE *pHandle = NULL;
    LCML_DSP_INTERFACE *pLcmlHandle;
    OMX_U32 i;
    OMX_ERRORTYPE rm_error = OMX_ErrorNone;
    PCMDEC_COMPONENT_PRIVATE* pComponentPrivate_CC = NULL;

    pComponentPrivate_CC = (PCMDEC_COMPONENT_PRIVATE*)((LCML_DSP_INTERFACE*)args[6])->pComponentPrivate;    

    PCMDEC_DPRINT (":: Entering the LCML_Callback() : event = %d\n",event);
    switch(event) {
        
    case EMMCodecDspError:
        PCMDEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecDspError >>>>>>>>>>\n");
        break;

    case EMMCodecInternalError:
        PCMDEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecInternalError >>>>>>>>>> \n");
        break;

    case EMMCodecInitError:
        PCMDEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecInitError>>>>>>>>>> \n");
        break;

    case EMMCodecDspMessageRecieved:
        PCMDEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecDspMessageRecieved>>>>>>>>>> \n");
        break;

    case EMMCodecBufferProcessed:
        PCMDEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecBufferProcessed>>>>>>>>>> \n");
        break;

    case EMMCodecProcessingStarted:
        PCMDEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecProcessingStarted>>>>>>>>>> \n");
        break;
            
    case EMMCodecProcessingPaused:
        PCMDEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecProcessingPaused>>>>>>>>>> \n");
        break;

    case EMMCodecProcessingStoped:
        PCMDEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecProcessingStoped>>>>>>>>>> \n");
        break;

    case EMMCodecProcessingEof:
        PCMDEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecProcessingEof>>>>>>>>>> \n");
        break;

    case EMMCodecBufferNotProcessed:
        PCMDEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecBufferNotProcessed>>>>>>>>>> \n");
        break;

    case EMMCodecAlgCtrlAck:
        PCMDEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecAlgCtrlAck>>>>>>>>>> \n");
        break;

    case EMMCodecStrmCtrlAck:
        PCMDEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecStrmCtrlAck>>>>>>>>>> \n");
        break;
    }
    if(event == EMMCodecBufferProcessed)
        {
            PCMDEC_DPRINT(":: --------- EMMCodecBufferProcessed Here\n");
            if( args[0] == (void *)EMMCodecInputBuffer) {
                PCMDEC_DPRINT (" :: Inside the LCML_Callback EMMCodecInputBuffer\n");
                PCMDEC_DPRINT(":: Input: pBufferr = %p\n", pBuffer);

                eError = PCMDEC_GetCorresponding_LCMLHeader(pComponentPrivate_CC, pBuffer, OMX_DirInput, &pLcmlHdr);
                PCMDEC_DPRINT(":: Output: pLcmlHeader = %p\n", pLcmlHdr);
                PCMDEC_DPRINT(":: Output: pLcmlHdr->eDir = %d\n", pLcmlHdr->eDir);
                PCMDEC_DPRINT(":: Output: *pLcmlHdr->eDir = %d\n", pLcmlHdr->eDir);
                PCMDEC_DPRINT(":: Output: Filled Len = %ld\n", pLcmlHdr->pBufHdr->nFilledLen);
                if (eError != OMX_ErrorNone) {
                    PCMDEC_DPRINT(":: Error: Invalid Buffer Came ...\n");
                    goto EXIT;
                }
                PCMDEC_DPRINT(":: Input: pLcmlHeader = %p\n", pLcmlHdr);

#ifdef __PERF_INSTRUMENTATION__
                PERF_ReceivedFrame(pComponentPrivate_CC->pPERFcomp,
                                   PREF(pLcmlHdr->pBufHdr,pBuffer),
                                   0,
                                   PERF_ModuleCommonLayer);
#endif
                pComponentPrivate_CC->lcml_nCntIpRes++;

                PCMDEC_ClearPending(pComponentPrivate_CC,pLcmlHdr->pBufHdr,OMX_DirInput,__LINE__);
                pComponentPrivate_CC->nOutStandingEmptyDones++;


#ifdef __PERF_INSTRUMENTATION__
            PERF_SendingFrame(pComponentPrivate_CC->pPERFcomp,
                              PREF(pLcmlHdr->pBufHdr,pBuffer),
                              0,
                              PERF_ModuleHLMM);
#endif
            pComponentPrivate_CC->cbInfo.EmptyBufferDone (pComponentPrivate_CC->pHandle,
                                                       pComponentPrivate_CC->pHandle->pApplicationPrivate,
                                                       pLcmlHdr->pBufHdr);
            pComponentPrivate_CC->nEmptyBufferDoneCount++;
            pComponentPrivate_CC->lcml_nIpBuf--;
            pComponentPrivate_CC->app_nBuf++;


            } else if (args[0] == (void *)EMMCodecOuputBuffer) {
                PCMDEC_DPRINT (" :: Inside the LCML_Callback EMMCodecOuputBuffer\n");
                PCMDEC_DPRINT(":: Output: pBufferr = %p\n", pBuffer);
                
                if (!PCMDEC_IsValid(pComponentPrivate_CC,pBuffer,OMX_DirOutput)) {
                    /* If the buffer we get back from the DSP is not valid call FillBufferDone
                       on a valid buffer */
#ifdef __PERF_INSTRUMENTATION__
                    PERF_SendingFrame(pComponentPrivate_CC->pPERFcomp,
                                      PREF(pComponentPrivate_CC->pOutputBufferList->pBufHdr[0],pBuffer),
                                      PREF(pComponentPrivate_CC->pOutputBufferList->pBufHdr[0],nFilledLen),
                                      PERF_ModuleHLMM);
#endif    
                    pComponentPrivate_CC->cbInfo.FillBufferDone (
                                                                 pComponentPrivate_CC->pHandle,
                                                                 pComponentPrivate_CC->pHandle->pApplicationPrivate,
                                                                 pComponentPrivate_CC->pOutputBufferList->pBufHdr[pComponentPrivate_CC->nInvalidFrameCount++]
                                                                 );
                    /*pComponentPrivate_CC->nOutStandingFillDones--;                    */ /*Ojo con este comentario*/
                    pComponentPrivate_CC->numPendingBuffers--;
                }
                else {

                    pComponentPrivate_CC->nOutStandingFillDones++;
                    eError = PCMDEC_GetCorresponding_LCMLHeader(pComponentPrivate_CC,pBuffer, OMX_DirOutput, &pLcmlHdr);
                    if (eError != OMX_ErrorNone) {
                        PCMDEC_DPRINT(":: Error: Invalid Buffer Came ...\n");
                        goto EXIT;
                    }
                    pLcmlHdr->pBufHdr->nFilledLen = (int)args[8];
                    PCMDEC_DPRINT(":: Output: pLcmlHeader = %p\n", pLcmlHdr);
                    PCMDEC_DPRINT(":: Output: pLcmlHdr->eDir = %d\n", pLcmlHdr->eDir);
                    PCMDEC_DPRINT(":: Output: Filled Len = %ld\n", pLcmlHdr->pBufHdr->nFilledLen);
                    pComponentPrivate_CC->lcml_nCntOpReceived++;
       
   
#ifdef __PERF_INSTRUMENTATION__
                    PERF_ReceivedFrame(pComponentPrivate_CC->pPERFcomp,
                                       PREF(pLcmlHdr->pBufHdr,pBuffer),
                                       PREF(pLcmlHdr->pBufHdr,nFilledLen),
                                       PERF_ModuleCommonLayer);

                    pComponentPrivate_CC->nLcml_nCntOpReceived++;

                    if ((pComponentPrivate_CC->nLcml_nCntIp >= 1) && (pComponentPrivate_CC->nLcml_nCntOpReceived == 1)) {
                        PERF_Boundary(pComponentPrivate_CC->pPERFcomp,
                                      PERF_BoundaryStart | PERF_BoundarySteadyState);
                    }
#endif

                    if (pComponentPrivate_CC->dasfmode == 0)
                        {
                            /* Copying time stamp information to output buffer */
                            pLcmlHdr->pBufHdr->nTimeStamp = (OMX_TICKS)pComponentPrivate_CC->arrBufIndex[pComponentPrivate_CC->OpBufindex];
                            /* Copying nTickCount information to output buffer */
                            pLcmlHdr->pBufHdr->nTickCount = pComponentPrivate_CC->arrTickCount[pComponentPrivate_CC->OpBufindex];
                            pComponentPrivate_CC->OpBufindex++;
                            pComponentPrivate_CC->OpBufindex %= pComponentPrivate_CC->pPortDef[OMX_DirInput]->nBufferCountActual;
                            PCMDEC_DPRINT("Output buffer TS = %lli at header: %p\n",pLcmlHdr->pBufHdr->nTimeStamp,pLcmlHdr->pBufHdr);
                        }

                    PCMDEC_ClearPending(pComponentPrivate_CC,pLcmlHdr->pBufHdr,OMX_DirOutput,__LINE__);

                    /* Previously in HandleDatabuffer form LCML */
                    if (pComponentPrivate_CC->pMarkData) {
                            pLcmlHdr->pBufHdr->pMarkData = pComponentPrivate_CC->pMarkData;
                            pLcmlHdr->pBufHdr->hMarkTargetComponent = pComponentPrivate_CC->hMarkTargetComponent;
                    }
                    pComponentPrivate_CC->num_Reclaimed_Op_Buff++;

                    if (pComponentPrivate_CC->bIsEOFSent) {
                        pLcmlHdr->pBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
                        pComponentPrivate_CC->cbInfo.EventHandler(pComponentPrivate_CC->pHandle,
                                                      pComponentPrivate_CC->pHandle->pApplicationPrivate,
                                                      OMX_EventBufferFlag,             
                                                      pLcmlHdr->pBufHdr->nOutputPortIndex,
                                                      pLcmlHdr->pBufHdr->nFlags, NULL);
                        pComponentPrivate_CC->bIsEOFSent = 0;
                    }
                   /*   Aqui falta lo del copying  time stamp information to output buffer*/
                   
                   /*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

#ifdef __PERF_INSTRUMENTATION__
                    PERF_SendingBuffer(pComponentPrivate_CC->pPERFcomp,
                               pLcmlHdr->pBufHdr->pBuffer,
                               pLcmlHdr->pBufHdr->nFilledLen,
                               PERF_ModuleHLMM);
#endif
                    pComponentPrivate_CC->cbInfo.FillBufferDone (
                                                      pHandle,
                                                      pHandle->pApplicationPrivate,
                                                      pLcmlHdr->pBufHdr
                                                      );
                    pComponentPrivate_CC->nOutStandingFillDones--;
                    pComponentPrivate_CC->lcml_nOpBuf--;
                    pComponentPrivate_CC->app_nBuf++;
                    pComponentPrivate_CC->nFillBufferDoneCount++;
                }
            }
        }else if(event == EMMCodecProcessingStoped) {
            if (!pComponentPrivate_CC->bNoIdleOnStop) {
                pComponentPrivate_CC->curState = OMX_StateIdle;
                
              

#ifdef RESOURCE_MANAGER_ENABLED
        rm_error = RMProxy_NewSendCommand(pComponentPrivate_CC->pHandle,
                                           RMProxy_StateSet,
                                           OMX_PCM_Decoder_COMPONENT,
                                           OMX_StateIdle,
                                           NEWSENDCOMMAND_MEMORY,
                                           NULL);
#endif  
            pComponentPrivate_CC->cbInfo.EventHandler(pComponentPrivate_CC->pHandle,
                                                   pComponentPrivate_CC->pHandle->pApplicationPrivate,
                                                   OMX_EventCmdComplete,
                                                   OMX_CommandStateSet,
                                                   pComponentPrivate_CC->curState,
                                                   NULL);


                /*  PCMDEC_DPRINT("%d :: Calling TransitionToIdle \n",__LINE__); */
                TransitionToIdle(pComponentPrivate_CC);
            } else {
                pComponentPrivate_CC->bDspStoppedWhileExecuting = OMX_TRUE;
                
                
                for (i=0; i < pComponentPrivate_CC->pInputBufferList->numBuffers; i++) {
                    if (PCMDEC_IsPending(pComponentPrivate_CC,pComponentPrivate_CC->pInputBufferList->pBufHdr[i],OMX_DirInput)) {
                        PCMDEC_DPRINT("Forcing EmptyBufferDone\n");
                        PCMDEC_DPRINT("EmptyBufferDone Line %d\n",__LINE__);
                        pComponentPrivate_CC->cbInfo.EmptyBufferDone (
                                                                      pComponentPrivate_CC->pHandle,
                                                                      pComponentPrivate_CC->pHandle->pApplicationPrivate,
                                                                      pComponentPrivate_CC->pInputBufferList->pBufHdr[i]
                                                                      );
                    }
                }
                for (i=0; i < pComponentPrivate_CC->pOutputBufferList->numBuffers; i++) {
                    if (PCMDEC_IsPending(pComponentPrivate_CC,pComponentPrivate_CC->pOutputBufferList->pBufHdr[i],OMX_DirOutput)) {
                        pComponentPrivate_CC->cbInfo.FillBufferDone (
                                                                     pComponentPrivate_CC->pHandle,
                                                                     pComponentPrivate_CC->pHandle->pApplicationPrivate,
                                                                     pComponentPrivate_CC->pOutputBufferList->pBufHdr[i]
                                                                     );
                    }
                }
                pComponentPrivate_CC->bNoIdleOnStop= OMX_FALSE;     /*Estaba despues de la llave que sigue */
            }
            /*
                            Hasta aqui se modifico todo para cambiar lo del LCML PIPE.
                            */
        } else if(event == EMMCodecAlgCtrlAck) {
            PCMDEC_DPRINT ("GOT MESSAGE USN_DSPACK_ALGCTRL \n");
        } else if (event == EMMCodecDspError) {
#ifdef _ERROR_PROPAGATION__
            /* Cheking for MMU_fault */
            if(((int)args[4] == USN_ERR_UNKNOWN_MSG) && (args[5] == NULL)) {
                PCMDEC_DPRINT("%d :: UTIL: MMU_Fault \n",__LINE__);
                /*pComponentPrivate_CC->bIsInvalidState=OMX_TRUE;*/
                pComponentPrivate_CC->curState = OMX_StateInvalid;
                pHandle = pComponentPrivate_CC->pHandle;
                pComponentPrivate_CC->cbInfo.EventHandler(pHandle, 
                                                          pHandle->pApplicationPrivate,
                                                          OMX_EventError,
                                                          OMX_ErrorInvalidState, 
                                                          0x2,
                                                          NULL);
            }
#endif                     
            PCMDEC_DPRINT(":: commandedState  = %p\n",args[0]);
            PCMDEC_DPRINT(":: arg4 = %p\n",args[4]);
            PCMDEC_DPRINT(":: arg5 = %p\n",args[5]);
            PCMDEC_DPRINT(":: --------- EMMCodecDspError Here\n");
            if(((int)args[4] == USN_ERR_WARNING) && ((int)args[5] == IUALG_WARN_PLAYCOMPLETED)) {
                char *pArgs = "damedesuStr";
                OMX_COMPONENTTYPE *pHandle = pComponentPrivate_CC->pHandle;
                LCML_DSP_INTERFACE *pLcmlHandle;

                PCMDEC_DPRINT (":: Comp: Inside the LCML_Callback: IUALG_WARN_PLAYCOMPLETED \n");
                pLcmlHandle = (LCML_DSP_INTERFACE *)pComponentPrivate_CC->pLcmlHandle;

                PCMDEC_DPRINT(": PCMDECUTILS::About to call LCML_ControlCodec\n");
                eError = LCML_ControlCodec(
                                           pLcmlHandle->pCodecinterfacehandle,
                                           MMCodecControlStop,(void *)pArgs);
                if(eError != OMX_ErrorNone) {
                    PCMDEC_EPRINT(": Error Occurred in Codec Stop..\n");
                    goto EXIT;
                }

                PCMDEC_DPRINT(":: PCMDEC: Codec has been Stopped here\n");

                pComponentPrivate_CC->pLcmlBufHeader[0]->pIpParam->bLastBuffer = 0;

                pComponentPrivate_CC->curState = OMX_StateIdle;
                PCMDEC_DPRINT(":: PCMDEC: State has been Set to Idle\n");
                /* add callback to application to indicate SN/USN has completed playing of current set of date */
#ifdef UNDER_CE
                                    

                pComponentPrivate_CC->cbInfo.EventHandler(pComponentPrivate_CC->pHandle,                  
                                                          pComponentPrivate_CC->pHandle->pApplicationPrivate,
                                                          OMX_EventBufferFlag,
                                                          (OMX_U32)NULL,
                                                          OMX_BUFFERFLAG_EOS,
                                                          NULL);                
#else
                                    

                pComponentPrivate_CC->cbInfo.EventHandler(pHandle, pHandle->pApplicationPrivate,
                                                          OMX_EventCmdComplete, OMX_CommandStateSet,
                                                          pComponentPrivate_CC->curState, NULL);
#endif                                    
            }
            if((int)args[5] == IUALG_WARN_CONCEALED) {
                PCMDEC_DPRINT( "Algorithm issued a warning. But can continue" );
                PCMDEC_DPRINT("%d :: arg5 = %p\n",__LINE__,args[5]);
            }

            if((int)args[5] == IUALG_ERR_GENERAL) {
                PCMDEC_DPRINT( "Algorithm error. Cannot continue" );
                PCMDEC_DPRINT("%d :: arg5 = %p\n",__LINE__,args[5]);
                PCMDEC_DPRINT("%d :: LCML_Callback: IUALG_ERR_GENERAL\n",__LINE__);
                pHandle = pComponentPrivate_CC->pHandle;
                pLcmlHandle = (LCML_DSP_INTERFACE *)pComponentPrivate_CC->pLcmlHandle;
#if 0
                eError = LCML_ControlCodec(
                                           ((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                           MMCodecControlStop,(void *)pArgs);
                if(eError != OMX_ErrorNone) {
                    PCMDEC_DPRINT("%d: Error Occurred in Codec Stop..\n",
                                  __LINE__);
                    goto EXIT;
                }
                PCMDEC_DPRINT("%d :: PCMDEC: Codec has been Stopped here\n",__LINE__);
                pComponentPrivate_CC->curState = OMX_StateIdle;
                pComponentPrivate_CC->cbInfo.EventHandler(
                                                          pHandle, pHandle->pApplicationPrivate,
                                                          OMX_EventCmdComplete, OMX_ErrorNone,0, NULL);
#endif
            }

            if( (int)args[5] == IUALG_ERR_DATA_CORRUPT )
            {
                char *pArgs = "damedesuStr";
                PCMDEC_DPRINT("%d :: arg5 = %p\n",__LINE__,args[5]);
                PCMDEC_DPRINT("%d :: LCML_Callback: IUALG_ERR_DATA_CORRUPT\n",__LINE__);
                pHandle = pComponentPrivate_CC->pHandle;
                pLcmlHandle = (LCML_DSP_INTERFACE *)pComponentPrivate_CC->pLcmlHandle;

                eError = LCML_ControlCodec(
                                           ((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                           MMCodecControlStop,(void *)pArgs);
                if(eError != OMX_ErrorNone) {
                    PCMDEC_DPRINT("%d: Error Occurred in Codec Stop..\n",
                                  __LINE__);
                    goto EXIT;
                }
                PCMDEC_DPRINT("%d :: PCMDEC: Codec has been Stopped here\n",__LINE__);
                pComponentPrivate_CC->curState = OMX_StateIdle;
#ifdef RESOURCE_MANAGER_ENABLED
                rm_error = RMProxy_NewSendCommand(pHandle, RMProxy_StateSet, OMX_PCM_Decoder_COMPONENT, OMX_StateIdle, NEWSENDCOMMAND_MEMORY, NULL);
#endif            
            
                pComponentPrivate_CC->cbInfo.EventHandler(
                                                          pHandle, pHandle->pApplicationPrivate,
                                                              OMX_EventCmdComplete, OMX_ErrorNone,0, NULL);

            }

            if( (int)args[5] == IUALG_WARN_OVERFLOW )
            {
                PCMDEC_DPRINT( "Algorithm error. Overflow" );
            }
            if( (int)args[5] == IUALG_WARN_UNDERFLOW )
            {
                PCMDEC_DPRINT( "Algorithm error. Underflow" );
            }
        }
        else if (event == EMMCodecStrmCtrlAck) {
            PCMDEC_DPRINT(":: GOT MESSAGE USN_DSPACK_STRMCTRL ----\n");

            if (args[1] == (void *)USN_STRMCMD_FLUSH) {
                pHandle = pComponentPrivate_CC->pHandle;                              
                if ( args[2] == (void *)EMMCodecInputBuffer) {
                    if (args[0] ==(void *) USN_ERR_NONE ) {
                        PCMDEC_DPRINT("Flushing input port %d\n",__LINE__);
                        /*for (i=0; i < MAX_NUM_OF_BUFS; i++) {
                                    pComponentPrivate_CC->pInputBufHdrPending[i] = NULL;
                                }
                                pComponentPrivate_CC->nNumInputBufPending=0;                       
                                   */
                        /*for (i=0; i < pComponentPrivate_CC->pInputBufferList->numBuffers; i++) {*/
                        for (i=0; i < pComponentPrivate_CC->nNumInputBufPending; i++) {
#ifdef __PERF_INSTRUMENTATION__
                            PERF_SendingFrame(pComponentPrivate_CC->pPERFcomp,
                                              /*PREF(pComponentPrivate_CC->pInputBufferList->pBufHdr[i],pBuffer),*/
                                              PREF(pComponentPrivate_CC->pInputBufHdrPending[i],pBuffer),
                                              0,
                                              PERF_ModuleHLMM);
#endif
                            pComponentPrivate_CC->cbInfo.EmptyBufferDone (
                                                                          pComponentPrivate_CC->pHandle,
                                                                          pComponentPrivate_CC->pHandle->pApplicationPrivate,
                                                                          /*pComponentPrivate_CC->pInputBufferList->pBufHdr[i]*/
                                                                          pComponentPrivate_CC->pInputBufHdrPending[i]
                                                                          );
                            pComponentPrivate_CC->pInputBufHdrPending[i]  = NULL;
                        }
                        pComponentPrivate_CC->nNumInputBufPending=0;
                        pComponentPrivate_CC->cbInfo.EventHandler(  pHandle, 
                                                                    pHandle->pApplicationPrivate,
                                                                    OMX_EventCmdComplete, OMX_CommandFlush,PCMD_INPUT_PORT, NULL);    
                    } else {
                        PCMDEC_DPRINT ("LCML reported error while flushing input port\n");
                        goto EXIT;                            
                    }
                }
                else if ( args[2] == (void *)EMMCodecOuputBuffer) { 
                    if (args[0] == (void *)USN_ERR_NONE ) {                      
                        /*PCMDEC_DPRINT("Flushing output port %d\n",__LINE__);*/
                        PCMDEC_DPRINT("\tCallback FLUSH OUT %ld\n",pComponentPrivate_CC->nNumOutputBufPending);
                        
                        for (i=0; i < pComponentPrivate_CC->nNumOutputBufPending; i++) {

        #ifdef __PERF_INSTRUMENTATION__
                            PERF_SendingFrame(pComponentPrivate_CC->pPERFcomp,
                                              PREF(pComponentPrivate_CC->pOutputBufHdrPending[i],pBuffer),
                                              PREF(pComponentPrivate_CC->pOutputBufHdrPending[i],nFilledLen),
                                              PERF_ModuleHLMM);
        #endif  
                        }
                        pComponentPrivate_CC->cbInfo.EventHandler(  pComponentPrivate_CC->pHandle, 
                                                                    pComponentPrivate_CC->pHandle->pApplicationPrivate,
                                                                    OMX_EventCmdComplete, 
                                                                    OMX_CommandFlush,
                                                                    PCMD_OUTPUT_PORT, 
                                                                    NULL);
                    } else {
                        PCMDEC_DPRINT ("LCML reported error while flushing output port\n");
                        goto EXIT;                            
                    }
                }
            }
        }
        else if (event ==EMMCodecProcessingPaused) {
            pComponentPrivate_CC->curState = OMX_StatePause;
            pComponentPrivate_CC->cbInfo.EventHandler(pComponentPrivate_CC->pHandle, pComponentPrivate_CC->pHandle->pApplicationPrivate,
                                                      OMX_EventCmdComplete, OMX_CommandStateSet,
                                                      pComponentPrivate_CC->curState, NULL);
            
        }
#ifdef _ERROR_PROPAGATION__

        else if (event ==EMMCodecInitError){
            /* Cheking for MMU_fault */
            if(((int)args[4] == USN_ERR_UNKNOWN_MSG) && (args[5] == NULL)) {
                PCMDEC_DPRINT("%d :: UTIL: MMU_Fault \n",__LINE__);
                /*pComponentPrivate_CC->bIsInvalidState=OMX_TRUE;*/
                pComponentPrivate_CC->curState = OMX_StateInvalid;
                pHandle = pComponentPrivate_CC->pHandle;
                pComponentPrivate_CC->cbInfo.EventHandler(pHandle, 
                                                          pHandle->pApplicationPrivate,
                                                          OMX_EventError,
                                                          OMX_ErrorInvalidState, 
                                                          0x2,
                                                          NULL);
            }    
        }
        else if (event ==EMMCodecInternalError){
            /* Cheking for MMU_fault */
            if(((int)args[4] == USN_ERR_UNKNOWN_MSG) && (args[5] == NULL)) {
                PCMDEC_DPRINT("%d :: UTIL: MMU_Fault \n",__LINE__);
                /*pComponentPrivate_CC->bIsInvalidState=OMX_TRUE;*/
                pComponentPrivate_CC->curState = OMX_StateInvalid;
                pHandle = pComponentPrivate_CC->pHandle;
                pComponentPrivate_CC->cbInfo.EventHandler(pHandle, 
                                                          pHandle->pApplicationPrivate,
                                                          OMX_EventError,
                                                          OMX_ErrorInvalidState, 
                                                          0x2,
                                                          NULL);
            }
        }
#endif
 EXIT:
    PCMDEC_DPRINT (":: Exiting the LCML_Callback() \n");
    return eError;
}

/* ================================================================================ */


/*#ifdef UNDER_CE*/

/* ================================================================================= * */
/**
* @fn PCMDEC_GetCorresponding_LCMLHeader() function gets the corresponding LCML
* header from the actual data buffer for required processing.
*
* @param *pBuffer This is the data buffer pointer. 
*
* @param eDir   This is direction of buffer. Input/Output.
*
* @param *PCMD_LCML_BUFHEADERTYPE  This is pointer to LCML Buffer Header.
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
OMX_ERRORTYPE PCMDEC_GetCorresponding_LCMLHeader(PCMDEC_COMPONENT_PRIVATE *pComponentPrivate,OMX_U8 *pBuffer,
                                          OMX_DIRTYPE eDir,
                                                 PCMD_LCML_BUFHEADERTYPE **ppLcmlHdr)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    PCMD_LCML_BUFHEADERTYPE *pLcmlBufHeader;
    int nIpBuf=0, nOpBuf=0, i=0;
    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate_CC;
    pComponentPrivate_CC = pComponentPrivate;

    PCMDEC_DPRINT (":: Entering the PCMDEC_GetCorresponding_LCMLHeader()\n");

    PCMDEC_DPRINT (":: eDir = %d\n",eDir);


    while (!pComponentPrivate_CC->bInitParamsInitialized) {
        sched_yield();
    }



    if(eDir == OMX_DirInput) {
        PCMDEC_DPRINT (":: In GetCorresponding_LCMLHeader()\n");

        nIpBuf = pComponentPrivate_CC->pInputBufferList->numBuffers;

        pLcmlBufHeader = pComponentPrivate_CC->pLcmlBufHeader[PCMD_INPUT_PORT];

        for(i=0; i<nIpBuf; i++) {
            PCMDEC_DPRINT("pBuffer = %p\n",pBuffer);
            PCMDEC_DPRINT("pLcmlBufHeader->pBufHdr->pBuffer = %p\n",pLcmlBufHeader->pBufHdr->pBuffer);
            if(pBuffer == pLcmlBufHeader->pBufHdr->pBuffer) {
                *ppLcmlHdr = pLcmlBufHeader;
                PCMDEC_DPRINT("::Corresponding LCML Header Found\n");
                goto EXIT;
            }
            pLcmlBufHeader++;
        }
    } else if (eDir == OMX_DirOutput) {
        i = 0;
        nOpBuf = pComponentPrivate_CC->pOutputBufferList->numBuffers;

        pLcmlBufHeader = pComponentPrivate_CC->pLcmlBufHeader[PCMD_OUTPUT_PORT];
        PCMDEC_DPRINT (":: nOpBuf = %d\n",nOpBuf);

        for(i=0; i<nOpBuf; i++) {
            PCMDEC_DPRINT("pBuffer = %p\n",pBuffer);
            PCMDEC_DPRINT("pLcmlBufHeader->pBufHdr->pBuffer = %p\n",pLcmlBufHeader->pBufHdr->pBuffer);
            if(pBuffer == pLcmlBufHeader->pBufHdr->pBuffer) {
                *ppLcmlHdr = pLcmlBufHeader;
                PCMDEC_DPRINT("::Corresponding LCML Header Found\n");
                goto EXIT;
            }
            pLcmlBufHeader++;
        }
    } else {
        PCMDEC_DPRINT(":: Invalid Buffer Type :: exiting...\n");
    }

 EXIT:
    PCMDEC_DPRINT (":: Exiting the GetCorresponding_LCMLHeader() \n");
    return eError;
}

/* ================================================================================= * */
/**
* @fn PCMDEC_GetLCMLHandle() function gets the LCML handle and interacts with LCML
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
OMX_HANDLETYPE PCMDEC_GetLCMLHandle(PCMDEC_COMPONENT_PRIVATE *pComponentPrivate)
{
    /* This must be taken care by WinCE */
    OMX_HANDLETYPE pHandle = NULL;
#ifndef UNDER_CE
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
#ifndef UNDER_CE              
        dlclose(handle);                            /* got error - Close LCML lib  */
        eError = OMX_ErrorUndefined;
        PCMDEC_DPRINT("eError != OMX_ErrorNone...\n");
        pHandle = NULL;
        goto EXIT;
#endif
    }

    ((LCML_DSP_INTERFACE*)pHandle)->pComponentPrivate = pComponentPrivate;
    pComponentPrivate->ptrLibLCML=handle;            /* saving LCML lib pointer  */    

 EXIT:
#endif
    return pHandle;
}

void PCMDEC_CleanupInitParams(OMX_HANDLETYPE pComponent)
{
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate = (PCMDEC_COMPONENT_PRIVATE *)
        pHandle->pComponentPrivate;
    PCMD_LCML_BUFHEADERTYPE *pTemp_lcml;
    OMX_U32 nIpBuf = pComponentPrivate->nRuntimeInputBuffers;
    OMX_U32 nOpBuf = pComponentPrivate->nRuntimeOutputBuffers;
    OMX_U32 i=0;
    char *pTemp = NULL;

    PCMDEC_DPRINT (":: PCMDEC_CleanupInitParams()\n");

    PCMDEC_MEMPRINT(":: Freeing:  pComponentPrivate->strmAttr = %p\n", pComponentPrivate->strmAttr);
   
    PCMD_OMX_FREE(pComponentPrivate->strmAttr); 
    pComponentPrivate->strmAttr = NULL;
    
    PCMDEC_MEMPRINT(":: Freeing: pComponentPrivate->pUalgParam = %p\n",pComponentPrivate->pUalgParam);
    pTemp = (char*)pComponentPrivate->pUalgParam;
    if (pTemp != NULL) {
        pTemp -= 128;
    }
    pComponentPrivate->pUalgParam = (PCMDEC_SN_UALGInputParams*)pTemp;
    PCMD_OMX_FREE(pComponentPrivate->pUalgParam); 
  
 
    PCMDEC_MEMPRINT(":: Freeing: pComponentPrivate->pUSetNumOfBits = %p\n",pComponentPrivate->pUSetNumOfBits);
    pTemp = (char*)pComponentPrivate->pUSetNumOfBits;
    if (pTemp != NULL) {
        pTemp -= 128;
    }
    
    pComponentPrivate->pUSetNumOfBits = (PCMDEC_UlNumOfBitsStatus*)pTemp;
    PCMD_OMX_FREE(pComponentPrivate->pUSetNumOfBits); 

 
    
    if (pComponentPrivate->dasfmode == 1) {
        PCMDEC_MEMPRINT(":: Freeing: pComponentPrivate->pParams = %p\n",pComponentPrivate->pParams);
        pTemp = (char*)pComponentPrivate->pParams;
        if (pTemp != NULL) {        
            pTemp -= 128;
        }
        pComponentPrivate->pParams = (PCMD_USN_AudioCodecParams*)pTemp;        
        PCMD_OMX_FREE(pComponentPrivate->pParams);
        
    }

    pTemp_lcml = pComponentPrivate->pLcmlBufHeader[PCMD_INPUT_PORT];
    for(i=0; i<nIpBuf; i++) {
        PCMDEC_MEMPRINT(":: Freeing: pTemp_lcml->pIpParam = %p\n",pTemp_lcml->pIpParam);
        PCMD_OMX_FREE(pTemp_lcml->pIpParam);
        pTemp_lcml++;
    }

    PCMDEC_MEMPRINT(":: Freeing pComponentPrivate->pLcmlBufHeader[PCMD_INPUT_PORT] = %p\n",pComponentPrivate->pLcmlBufHeader[PCMD_INPUT_PORT]);

    PCMD_OMX_FREE(pComponentPrivate->pLcmlBufHeader[PCMD_INPUT_PORT]);

    pTemp_lcml = pComponentPrivate->pLcmlBufHeader[PCMD_OUTPUT_PORT];
    for(i=0; i<nOpBuf; i++) {
        PCMDEC_MEMPRINT(":: Freeing: pTemp_lcml->pIpParam = %p\n",pTemp_lcml->pOpParam);
        PCMD_OMX_FREE(pTemp_lcml->pOpParam);
        pTemp_lcml++;
    }

    PCMDEC_MEMPRINT(":: Freeing: pComponentPrivate->pLcmlBufHeader[PCMD_OUTPUT_PORT] = %p\n",pComponentPrivate->pLcmlBufHeader[PCMD_OUTPUT_PORT]);
    PCMD_OMX_FREE(pComponentPrivate->pLcmlBufHeader[PCMD_OUTPUT_PORT]);
    pComponentPrivate->pLcmlBufHeader[PCMD_OUTPUT_PORT] = NULL;
    PCMDEC_DPRINT ("Exiting Successfully PCMDEC_CleanupInitParams()\n");
}
/* ========================================================================== */
/**
* @PCMDEC_SetPending() This function marks the buffer as pending when it is sent
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
void PCMDEC_SetPending(PCMDEC_COMPONENT_PRIVATE *pComponentPrivate, OMX_BUFFERHEADERTYPE *pBufHdr, OMX_DIRTYPE eDir, OMX_U32 lineNumber)
{
    OMX_U32 i;

    PCMDEC_DPRINT("Called PCMDEC_SetPending\n");
    PCMDEC_DPRINT("eDir = %d\n",eDir);

    if (eDir == OMX_DirInput) {
        for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pInputBufferList->pBufHdr[i]) {
                pComponentPrivate->pInputBufferList->bBufferPending[i] = 1;
                PCMDEC_DPRINT("*******************INPUT BUFFER %d IS PENDING Line %ld******************************\n",i,lineNumber);
            }
        }
    }
    else {
        for (i=0; i < pComponentPrivate->pOutputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pOutputBufferList->pBufHdr[i]) {
                pComponentPrivate->pOutputBufferList->bBufferPending[i] = 1;
                PCMDEC_DPRINT("*******************OUTPUT BUFFER %d IS PENDING Line %ld******************************\n",i,lineNumber);
            }
        }
    }
}

/* ========================================================================== */
/**
* @PCMDEC_ClearPending() This function clears the buffer status from pending
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

void PCMDEC_ClearPending(PCMDEC_COMPONENT_PRIVATE *pComponentPrivate, OMX_BUFFERHEADERTYPE *pBufHdr, OMX_DIRTYPE eDir, OMX_U32 lineNumber)
{
    OMX_U32 i;

    if (eDir == OMX_DirInput) {
        for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pInputBufferList->pBufHdr[i]) {
                pComponentPrivate->pInputBufferList->bBufferPending[i] = 0;
                PCMDEC_DPRINT("*******************INPUT BUFFER %d IS RECLAIMED Line %ld******************************\n",i,lineNumber);
            }
        }
    }
    else {
        for (i=0; i < pComponentPrivate->pOutputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pOutputBufferList->pBufHdr[i]) {
                pComponentPrivate->pOutputBufferList->bBufferPending[i] = 0;
                PCMDEC_DPRINT("*******************OUTPUT BUFFER %d IS RECLAIMED Line %ld******************************\n",i,lineNumber);
            }
        }
    }
}

/* ========================================================================== */
/**
* @PCMDEC_IsPending() This function checks whether or not a buffer is pending.
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

OMX_U32 PCMDEC_IsPending(PCMDEC_COMPONENT_PRIVATE *pComponentPrivate, OMX_BUFFERHEADERTYPE *pBufHdr, OMX_DIRTYPE eDir)
{
    OMX_U32 i;

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
* @PCMDEC_IsValid() This function identifies whether or not buffer recieved from
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

OMX_U32 PCMDEC_IsValid(PCMDEC_COMPONENT_PRIVATE *pComponentPrivate, OMX_U8 *pBuffer, OMX_DIRTYPE eDir)
{
    OMX_U32 i;
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
* @PCMDECFill_LCMLInitParamsEx() This function initializes the init parameter of
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

OMX_ERRORTYPE PCMDECFill_LCMLInitParamsEx(OMX_HANDLETYPE pComponent)

{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 nIpBuf,nIpBufSize,nOpBuf,nOpBufSize;
    OMX_U32 i;
    OMX_BUFFERHEADERTYPE *pTemp;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate =
        (PCMDEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;
    PCMD_LCML_BUFHEADERTYPE *pTemp_lcml;
    OMX_U32 size_lcml;
    char *ptr;


    PCMDEC_DPRINT(":: Entered Fill_LCMLInitParams");


    nIpBuf = pComponentPrivate->pInputBufferList->numBuffers;
    nOpBuf = pComponentPrivate->pOutputBufferList->numBuffers;
    nIpBufSize = pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->nBufferSize;
    nOpBufSize = pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->nBufferSize;


    PCMDEC_DPRINT("Input Buffer Count = %ld\n",nIpBuf);
    PCMDEC_DPRINT("Input Buffer Size = %ld\n",nIpBufSize);
    PCMDEC_DPRINT("Output Buffer Count = %ld\n",nOpBuf);
    PCMDEC_DPRINT("Output Buffer Size = %ld\n",nOpBufSize);
    PCMDEC_BUFPRINT("Input Buffer Count = %ld\n",nIpBuf);
    PCMDEC_BUFPRINT("Input Buffer Size = %ld\n",nIpBufSize);
    PCMDEC_BUFPRINT("Output Buffer Count = %ld\n",nOpBuf);
    PCMDEC_BUFPRINT("Output Buffer Size = %ld\n",nOpBufSize);



    PCMDEC_DPRINT(":: bufAlloced = %d\n",pComponentPrivate->bufAlloced);
    size_lcml = nIpBuf * sizeof(PCMD_LCML_BUFHEADERTYPE);

    PCMD_OMX_MALLOC_SIZE(ptr,size_lcml);
    pTemp_lcml = (PCMD_LCML_BUFHEADERTYPE *)ptr;

    pComponentPrivate->pLcmlBufHeader[PCMD_INPUT_PORT] = pTemp_lcml;

    for (i=0; i<nIpBuf; i++) {
        pTemp = pComponentPrivate->pInputBufferList->pBufHdr[i];

        pTemp->nSize = sizeof(OMX_BUFFERHEADERTYPE);

        pTemp->nAllocLen = nIpBufSize;
        pTemp->nFilledLen = nIpBufSize;
        pTemp->nVersion.s.nVersionMajor = PCMDEC_MAJOR_VER;
        pTemp->nVersion.s.nVersionMinor = PCMDEC_MINOR_VER;

        pTemp->pPlatformPrivate = pHandle->pComponentPrivate;
        pTemp->nTickCount = NOT_USED;

        pTemp_lcml->pBufHdr = pTemp;
        pTemp_lcml->eDir = OMX_DirInput;
        pTemp_lcml->pOtherParams[i] = NULL;

        PCMD_OMX_MALLOC(pTemp_lcml->pIpParam, PCMDEC_UAlgInBufParamStruct);
        pTemp_lcml->pIpParam->bLastBuffer = 0;

        pTemp->nFlags = NORMAL_BUFFER;
        ((PCMDEC_COMPONENT_PRIVATE *) pTemp->pPlatformPrivate)->pHandle = pHandle;

        PCMDEC_DPRINT("::Comp: InBuffHeader[%ld] = %p\n", i, pTemp);
        PCMDEC_DPRINT("::Comp:  >>>> InputBuffHeader[%ld]->pBuffer = %p\n", i, pTemp->pBuffer);
        PCMDEC_DPRINT("::Comp: Ip : pTemp_lcml[%ld] = %p\n", i, pTemp_lcml);

        pTemp_lcml++;
    }

    size_lcml = nOpBuf * sizeof(PCMD_LCML_BUFHEADERTYPE);
    pTemp_lcml = (PCMD_LCML_BUFHEADERTYPE *)malloc(size_lcml);
    if(pTemp_lcml == NULL) {
        PCMD_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                            "Memory Allocation Failed");
    }
    pComponentPrivate->pLcmlBufHeader[PCMD_OUTPUT_PORT] = pTemp_lcml;

    for (i=0; i<nOpBuf; i++) {
        pTemp = pComponentPrivate->pOutputBufferList->pBufHdr[i];

        pTemp->nSize = sizeof(OMX_BUFFERHEADERTYPE);

        pTemp->nAllocLen = nOpBufSize;
        pTemp->nFilledLen = nOpBufSize;
        pTemp->nVersion.s.nVersionMajor = PCMDEC_MAJOR_VER;
        pTemp->nVersion.s.nVersionMinor = PCMDEC_MINOR_VER;

        pTemp->pPlatformPrivate = pHandle->pComponentPrivate;
        pTemp->nTickCount = NOT_USED;

        pTemp_lcml->pBufHdr = pTemp;
        pTemp_lcml->eDir = OMX_DirOutput;
        pTemp_lcml->pOtherParams[i] = NULL;

        PCMD_OMX_MALLOC(pTemp_lcml->pOpParam,PCMDEC_UAlgOutBufParamStruct);

        pTemp_lcml->pOpParam->ulFrameCount = DONT_CARE;

        pTemp->nFlags = NORMAL_BUFFER;
        ((PCMDEC_COMPONENT_PRIVATE *)pTemp->pPlatformPrivate)->pHandle = pHandle;
        PCMDEC_DPRINT("::Comp:  >>>>>>>>>>>>> OutBuffHeader[%ld] = %p\n", i, pTemp);
        PCMDEC_DPRINT("::Comp:  >>>> OutBuffHeader[%ld]->pBuffer = %p\n", i, pTemp->pBuffer);
        PCMDEC_DPRINT("::Comp: Op : pTemp_lcml[%ld] = %p\n", i, pTemp_lcml);
        pTemp_lcml++;
    }
    pComponentPrivate->bPortDefsAllocated = 1;

#ifdef __PERF_INSTRUMENTATION__
    pComponentPrivate->nLcml_nCntIp = 0;
    pComponentPrivate->nLcml_nCntOpReceived = 0;
#endif
    PCMDEC_DPRINT(":: Exiting Fill_LCMLInitParams");

    pComponentPrivate->bInitParamsInitialized = 1;

 EXIT:
    return eError;
}


/* ========================================================================== */
/**
* @TransitionToIdle() This function sets the pending status of all the buffers
* appropriately before making the idle transition.
*
* @param pComponentPrivate This is component's internal structure.
*
* @pre None
*
* @post None
*
* @return None
*/
/* ========================================================================== */
void TransitionToIdle(PCMDEC_COMPONENT_PRIVATE *pComponentPrivate)
{
    OMX_U32 i;
    OMX_ERRORTYPE rm_error = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *) pComponentPrivate->pHandle;

    PCMDEC_DPRINT("in TransitionToIdle - pComponentPrivate->nOutStandingFillDones = %ld\n",pComponentPrivate->nOutStandingFillDones);
    PCMDEC_DPRINT("in TransitionToIdle - pComponentPrivate->nOutStandingEmptyDones = %ld\n",pComponentPrivate->nOutStandingEmptyDones);

    if (pComponentPrivate->nOutStandingFillDones <= 0 && pComponentPrivate->nOutStandingEmptyDones <= 0) {
        PCMDEC_DPRINT("Went inside if statement\n");

        /* If there are any buffers still marked as pending they must have
           been queued after the socket node was stopped */
        for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
            PCMDEC_DPRINT("Line \n");

            if (pComponentPrivate->pInputBufferList->bBufferPending[i]) {
                PCMDEC_DPRINT("Line \n");

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
                PCMDEC_ClearPending(pComponentPrivate, pComponentPrivate->pInputBufferList->pBufHdr[i], OMX_DirInput, __LINE__);
            }
        }
        PCMDEC_DPRINT("Line \n");

        for (i=0; i < pComponentPrivate->pOutputBufferList->numBuffers; i++) {
            PCMDEC_DPRINT("Line \n");

            if (pComponentPrivate->pOutputBufferList->bBufferPending[i]) {
                PCMDEC_DPRINT("Line \n");

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

                PCMDEC_ClearPending(pComponentPrivate, pComponentPrivate->pOutputBufferList->pBufHdr[i], OMX_DirOutput, __LINE__);
            }
        }

        pComponentPrivate->curState = OMX_StateIdle;
#ifdef RESOURCE_MANAGER_ENABLED
        rm_error = RMProxy_NewSendCommand(pHandle, RMProxy_StateSet, OMX_PCM_Decoder_COMPONENT, OMX_StateIdle, NEWSENDCOMMAND_MEMORY, NULL);
#endif

	if(pComponentPrivate->bPreempted==0){
        					pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                               pComponentPrivate->pHandle->pApplicationPrivate,
                                               OMX_EventCmdComplete,
                                               OMX_CommandStateSet,
                                               pComponentPrivate->curState,
                                               NULL);

        pComponentPrivate->bIdleCommandPending = OMX_FALSE;
	} else {
		pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventError,
                                                   OMX_ErrorResourcesPreempted,
                                                   0,
                                                   NULL);

	}

    }
}

/*
void PCMD_ResourceManagerCallback(RMPROXY_COMMANDDATATYPE cbData){
	OMX_COMMANDTYPE Cmd = OMX_CommandStateSet;
    OMX_STATETYPE state = OMX_StateIdle;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)cbData.hComponent;
    PCMDEC_COMPONENT_PRIVATE *pCompPrivate = NULL;

    pCompPrivate = (PCMDEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;

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
