
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
* @file OMX_G723_1Enc_Utils.c
*
* This file implements G723_1 Encoder Component Specific APIs and its functionality
* that is fully compliant with the Khronos OpenMAX (TM) 1.0 Specification
*
* @path  $(CSLPATH)\OMAPSW_MPU\linux\audio\src\openmax_il\g723_enc\src
*
* @rev  1.0
*/
/* ----------------------------------------------------------------------------
*!
*! Revision History
*! ===================================
*! Gyancarlo Garcia: Initial Verision
*! 05-Oct-2007
*!
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
#include <errno.h>
#endif

#include <dbapi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/*-------program files ----------------------------------------*/
#include "OMX_G723_1Enc_Utils.h"
#include "g723_1enc_sn_uuid.h"
#include "usn_ti.h"
#include "usn.h"

#ifdef RESOURCE_MANAGER_ENABLED
    #include <ResourceManagerProxyAPI.h>
#endif

#ifdef UNDER_CE
#define HASHINGENABLE 1
#endif

#ifdef G723_1ENC_DEBUGMEM
extern void *arr[500] = {NULL};
extern int lines[500] = {0};
extern int bytes[500] = {0};
extern char file[500][50] = {""};

void * DebugMalloc(int line, char *s, int size);
int DebugFree(void *dp, int line, char *s);

#define SafeMalloc(x) DebugMalloc(__LINE__,__FILE__,x)
#define SafeFree(z) DebugFree(z,__LINE__,__FILE__)

void * DebugMalloc(int line, char *s, int size)
{
   void *p=NULL;    
   int e=0;
   p = calloc(1,size);
   if(p==NULL){
       printf("__ Memory not available\n");
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
         printf("__ Allocating %d bytes on address %p, line %d file %s\n", size, p, line, s);
         return p;
   }
}

int DebugFree(void *dp, int line, char *s){
    int q = 0;
    if(dp==NULL){
                 printf("__ NULL can't be deleted\n");
                 return 0;
    }
    for(q=0;q<500;q++){
        if(arr[q]==dp){
           printf("__ Deleting %d bytes on address %p, line %d file %s\n", bytes[q],dp, line, s);
           lines[q]=0;
           strcpy(file[q],"");           
           free(dp);
           dp = NULL;
           break;
        }            
     }    
     if(500==q)
         printf("\n\n__ Pointer not found. Line:%d    File%s!!\n\n",line, s);
}
#else
#define SafeMalloc(x) calloc(1,x)
#define SafeFree(z) free(z)
#endif

OMX_ERRORTYPE OMX_DmmUnMap(DSP_HPROCESSOR ProcHandle, void* pMapPtr, void* pResPtr);
OMX_ERRORTYPE OMX_DmmMap(DSP_HPROCESSOR ProcHandle,
                     int size,
                     void* pArmPtr,
                     DMM_BUFFER_OBJ* pDmmBuf);

/* ========================================================================== */
/**
* @G723_1ENC_FillLCMLInitParams () This function is used by the component thread to
* fill the all of its initialization parameters, buffer deatils  etc
* to LCML structure,
*
* @param pComponent  handle for this instance of the component
* @param plcml_Init  pointer to LCML structure to be filled
*
* @pre
*
* @post
*
* @return none
*/
/* ========================================================================== */

OMX_ERRORTYPE G723_1ENC_FillLCMLInitParams(OMX_HANDLETYPE pComponent,
                                  LCML_DSP *plcml_Init, OMX_U16 arr[])
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 nIpBuf = 0,nIpBufSize = 0,nOpBuf = 0,nOpBufSize = 0;
    OMX_BUFFERHEADERTYPE *pTemp = NULL;
    LCML_DSP_INTERFACE *pHandle = (LCML_DSP_INTERFACE *)pComponent;
    G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate = pHandle->pComponentPrivate;
    G723_1ENC_LCML_BUFHEADERTYPE *pTemp_lcml = NULL;
    OMX_U32 i = 0;
    OMX_U32 size_lcml = 0;
    OMX_U8 *pstrTemp = NULL;
    G723_1ENC_DPRINT("%d :: Entering G723_1ENC_FillLCMLInitParams\n",__LINE__);

    nIpBuf = pComponentPrivate->pInputBufferList->numBuffers;
    nIpBufSize = pComponentPrivate->pPortDef[G723_1ENC_INPUT_PORT]->nBufferSize;
    nOpBuf = pComponentPrivate->pOutputBufferList->numBuffers;
    nOpBufSize = pComponentPrivate->pPortDef[G723_1ENC_OUTPUT_PORT]->nBufferSize;
    
    pComponentPrivate->nRuntimeInputBuffers = (OMX_U8)nIpBuf;
    pComponentPrivate->nRuntimeOutputBuffers = (OMX_U8) nOpBuf;
    
    G723_1ENC_DPRINT("%d :: ------ Buffer Details -----------\n",__LINE__);
    G723_1ENC_DPRINT("%d :: Input  Buffer Count = %ld\n",__LINE__,nIpBuf);
    G723_1ENC_DPRINT("%d :: Input  Buffer Size = %ld\n",__LINE__,nIpBufSize);
    G723_1ENC_DPRINT("%d :: Output Buffer Count = %ld\n",__LINE__,nOpBuf);
    G723_1ENC_DPRINT("%d :: Output Buffer Size = %ld\n",__LINE__,nOpBufSize);
    G723_1ENC_DPRINT("%d :: ------ Buffer Details ------------\n",__LINE__);
    /* Fill Input Buffers Info for LCML */
    plcml_Init->In_BufInfo.nBuffers = nIpBuf;
    plcml_Init->In_BufInfo.nSize = nIpBufSize;
    plcml_Init->In_BufInfo.DataTrMethod = DMM_METHOD;

    /* Fill Output Buffers Info for LCML */
    plcml_Init->Out_BufInfo.nBuffers = nOpBuf;
    plcml_Init->Out_BufInfo.nSize = nOpBufSize;
    plcml_Init->Out_BufInfo.DataTrMethod = DMM_METHOD;
    
    /*Copy the node information*/
    plcml_Init->NodeInfo.nNumOfDLLs = 3;

    plcml_Init->NodeInfo.AllUUIDs[0].uuid = &G723_1ENCSOCKET_TI_UUID;
    strcpy ((char*)plcml_Init->NodeInfo.AllUUIDs[0].DllName,G723_1ENC_DLL_NAME);
    plcml_Init->NodeInfo.AllUUIDs[0].eDllType = DLL_NODEOBJECT;

    plcml_Init->NodeInfo.AllUUIDs[1].uuid = &G723_1ENCSOCKET_TI_UUID;
    strcpy ((char*)plcml_Init->NodeInfo.AllUUIDs[1].DllName,G723_1ENC_DLL_NAME);
    plcml_Init->NodeInfo.AllUUIDs[1].eDllType = DLL_DEPENDENT;

    /*plcml_Init->NodeInfo.AllUUIDs[2].uuid = &USN_TI_UUID;*/
    plcml_Init->NodeInfo.AllUUIDs[2].uuid = &ENCODE_COMMON_TI_UUID;
    strcpy ((char*)plcml_Init->NodeInfo.AllUUIDs[2].DllName,G723_1ENC_USN_DLL_NAME);
    plcml_Init->NodeInfo.AllUUIDs[2].eDllType = DLL_DEPENDENT;

    plcml_Init->DeviceInfo.TypeofDevice = 0;

    if(pComponentPrivate->dasfMode == 1) {
        G723_1ENC_DPRINT("%d :: Codec is configuring to DASF mode\n",__LINE__);
        pstrTemp = (OMX_U8*)SafeMalloc(sizeof(LCML_STRMATTR) + 256);
        if(pstrTemp == NULL){
                        G723_1ENC_EPRINT("***********************************\n");
                        G723_1ENC_EPRINT("%d :: Malloc Failed\n",__LINE__);
                        G723_1ENC_EPRINT("***********************************\n");
                        eError = OMX_ErrorInsufficientResources;
                        goto EXIT;
        }
        pComponentPrivate->strmAttr = (LCML_STRMATTR*)(pstrTemp + 128);                
        pComponentPrivate->strmAttr->uSegid = G723_1ENC_DEFAULT_SEGMENT;
        pComponentPrivate->strmAttr->uAlignment = 0;
        pComponentPrivate->strmAttr->uTimeout = G723_1ENC_SN_TIMEOUT;
        pComponentPrivate->strmAttr->uBufsize = nIpBufSize;

        pComponentPrivate->strmAttr->uNumBufs = G723_1ENC_NUM_INPUT_BUFFERS_DASF;
        pComponentPrivate->strmAttr->lMode = STRMMODE_PROCCOPY;
        /* Device is Configuring to DASF Mode */
        plcml_Init->DeviceInfo.TypeofDevice = 1;
        /* Device is Configuring to Record Mode */
        plcml_Init->DeviceInfo.TypeofRender = 1;

        if(pComponentPrivate->acdnMode == 1) {
            /* ACDN mode */
            plcml_Init->DeviceInfo.AllUUIDs[0].uuid = &ACDN_TI_UUID;
        }
        else {
            /* DASF/TeeDN mode */
            plcml_Init->DeviceInfo.AllUUIDs[0].uuid = &DCTN_TI_UUID;
        }
        plcml_Init->DeviceInfo.DspStream = pComponentPrivate->strmAttr;
    }

    /*copy the other information*/
    plcml_Init->SegID = G723_1ENC_DEFAULT_SEGMENT;
    plcml_Init->Timeout = G723_1ENC_SN_TIMEOUT;
    plcml_Init->Alignment = 0;
    plcml_Init->Priority = G723_1ENC_SN_PRIORITY;
    plcml_Init->ProfileID = 0;

    /* Setting Creat Phase Parameters here */
    arr[0] = G723_1ENC_STREAM_COUNT;                                           /* Number of communication paths      */
    arr[1] = G723_1ENC_INPUT_PORT;                                             /* Input Path id                      */

    if(pComponentPrivate->dasfMode == 1) {
        arr[2] = G723_1ENC_INSTRM;                                             /* Streaming on input                 */
        arr[3] = G723_1ENC_NUM_INPUT_BUFFERS_DASF;                             /* Audio Devices are connected        */
    }
    else {
            arr[2] = G723_1ENC_DMM;                                                /* DMM buffers will be used           */                                              if (pComponentPrivate->pInputBufferList->numBuffers) {
            arr[3] = (OMX_U16) pComponentPrivate->pInputBufferList->numBuffers;/* #Bufs exchanged between SN and GPP */
        }
    }

    arr[4] = G723_1ENC_OUTPUT_PORT;                                            /* Output Path id                     */
    arr[5] = G723_1ENC_DMM;                                                    /* DMM buffers will be used           */
    if (pComponentPrivate->pOutputBufferList->numBuffers) {
        arr[6] = (OMX_U16) pComponentPrivate->pOutputBufferList->numBuffers;   /* #Bufs exchanged between SN and GPP */
    }
    else {
        arr[6] = (OMX_U16)1;                                                   /* 1 Buf exchanged between SN and GPP */
    }

    arr[7] = END_OF_CR_PHASE_ARGS;

    plcml_Init->pCrPhArgs = arr;

    /* Allocate memory for all input buffer headers..
     * This memory pointer will be sent to LCML */
    size_lcml = nIpBuf * sizeof(G723_1ENC_LCML_BUFHEADERTYPE);
    pTemp_lcml = (G723_1ENC_LCML_BUFHEADERTYPE *)SafeMalloc(size_lcml);    
    
    G723_1ENC_MEMPRINT("%d :: ALLOCATING MEMORY = %p\n",__LINE__,pTemp_lcml);
    if(pTemp_lcml == NULL) {
        G723_1ENC_DPRINT("%d :: Memory Allocation Failed\n",__LINE__);
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    pComponentPrivate->pLcmlBufHeader[G723_1ENC_INPUT_PORT] = pTemp_lcml;
    for (i=0; i<nIpBuf; i++) {
        G723_1ENC_DPRINT("%d :: INPUT--------- Inside Ip Loop\n",__LINE__);
        pTemp = pComponentPrivate->pInputBufferList->pBufHdr[i];
        pTemp->nSize = sizeof(OMX_BUFFERHEADERTYPE);
        /*pTemp->nAllocLen = nIpBufSize;*/
        pTemp->nFilledLen = nIpBufSize;
        pTemp->nVersion.s.nVersionMajor = G723_1ENC_MAJOR_VER;
        pTemp->nVersion.s.nVersionMinor = G723_1ENC_MINOR_VER;
        pTemp->pPlatformPrivate = pHandle->pComponentPrivate;
        pTemp->nTickCount = G723_1ENC_NOT_USED;
        pTemp_lcml->buffer = pTemp;
        G723_1ENC_DPRINT("%d :: pTemp_lcml->buffer->pBuffer = %p \n",__LINE__,pTemp_lcml->buffer->pBuffer);
        pTemp_lcml->eDir = OMX_DirInput;
        OMX_NBMALLOC_STRUCT(pTemp_lcml->pIpParam, G723ENC_ParamStruct);
        pTemp_lcml->pFrameParam = NULL;
        pTemp_lcml->pDmmBuf = (DMM_BUFFER_OBJ*) SafeMalloc(sizeof(DMM_BUFFER_OBJ));
        /*pTemp_lcml->pIpParam->usLastFrame = 0;*/
        /* This means, it is not a last buffer. This flag is to be modified by
         * the application to indicate the last buffer */
        pTemp->nFlags = G723_1ENC_NORMAL_BUFFER;
        pTemp++;
        pTemp_lcml++;
    }

    /* Allocate memory for all output buffer headers..
     * This memory pointer will be sent to LCML */
    size_lcml = nOpBuf * sizeof(G723_1ENC_LCML_BUFHEADERTYPE);
    pTemp_lcml = (G723_1ENC_LCML_BUFHEADERTYPE *)SafeMalloc(size_lcml);
    G723_1ENC_MEMPRINT("%d :: ALLOCATING MEMORY = %p\n",__LINE__,pTemp_lcml);
    if(pTemp_lcml == NULL) {
        G723_1ENC_DPRINT("%d :: Memory Allocation Failed\n",__LINE__);
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    pComponentPrivate->pLcmlBufHeader[G723_1ENC_OUTPUT_PORT] = pTemp_lcml;
    for (i=0; i<nOpBuf; i++) {
        G723_1ENC_DPRINT("%d :: OUTPUT--------- Inside Op Loop\n",__LINE__);
        pTemp = pComponentPrivate->pOutputBufferList->pBufHdr[i];
        pTemp->nSize = sizeof(OMX_BUFFERHEADERTYPE);
        /*pTemp->nAllocLen = nOpBufSize;*/
        pTemp->nFilledLen = nOpBufSize;
        pTemp->nVersion.s.nVersionMajor = G723_1ENC_MAJOR_VER;
        pTemp->nVersion.s.nVersionMinor = G723_1ENC_MINOR_VER;
        pComponentPrivate->nVersion = pTemp->nVersion.nVersion;
        pTemp->pPlatformPrivate = pHandle->pComponentPrivate;
        pTemp->nTickCount = G723_1ENC_NOT_USED;
        pTemp_lcml->buffer = pTemp;
        G723_1ENC_DPRINT("%d :: pTemp_lcml->buffer->pBuffer = %p \n",__LINE__,pTemp_lcml->buffer->pBuffer);
        pTemp_lcml->eDir = OMX_DirOutput;
        OMX_NBMALLOC_STRUCT(pTemp_lcml->pOpParam, G723ENC_ParamStruct);
        pTemp_lcml->pOpParam->usNbFrames = 0;
        /* This means, it is not a last buffer. This flag is to be modified by
         * the application to indicate the last buffer */
        pTemp->nFlags = G723_1ENC_NORMAL_BUFFER;
        pTemp++;
        pTemp_lcml++;
    }

    pComponentPrivate->bPortDefsAllocated = 1;
    pComponentPrivate->bInitParamsInitialized = 1;
EXIT:
    G723_1ENC_DPRINT("%d :: Exiting G723_1ENC_FillLCMLInitParams\n",__LINE__);
    G723_1ENC_DPRINT("%d :: Returning = 0x%x\n",__LINE__,eError);
    return eError;
}

/* ========================================================================== */
/**
* @G723_1ENC_StartComponentThread() This function is called by the component to create
* the component thread, command pipes, data pipes and LCML Pipes.
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
OMX_ERRORTYPE G723_1ENC_StartComponentThread(OMX_HANDLETYPE pComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate =
                        (G723_1ENC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;
#ifdef UNDER_CE
    pthread_attr_t attr;
    memset(&attr, 0, sizeof(attr));
    attr.__inheritsched = PTHREAD_EXPLICIT_SCHED;
    attr.__schedparam.__sched_priority = OMX_AUDIO_ENCODER_THREAD_PRIORITY;
#endif

    G723_1ENC_DPRINT ("%d :: Enetering  G723_1ENC_StartComponentThread\n", __LINE__);
    /* Initialize all the variables*/
    pComponentPrivate->bIsStopping = 0;

    /* create the pipe used to send buffers to the thread */
    eError = pipe (pComponentPrivate->cmdDataPipe);
    if (eError) {
       eError = OMX_ErrorInsufficientResources;
       G723_1ENC_DPRINT("%d :: Error while creating cmdDataPipe\n",__LINE__);
       goto EXIT;
    }
    /* create the pipe used to send buffers to the thread */
    eError = pipe (pComponentPrivate->dataPipe);
    if (eError) {
       eError = OMX_ErrorInsufficientResources;
       G723_1ENC_DPRINT("%d :: Error while creating dataPipe\n",__LINE__);
       goto EXIT;
    }

    /* create the pipe used to send commands to the thread */
    eError = pipe (pComponentPrivate->cmdPipe);
    if (eError) {
       eError = OMX_ErrorInsufficientResources;
       G723_1ENC_DPRINT("%d :: Error while creating cmdPipe\n",__LINE__);
       goto EXIT;
    }

    /* Create the Component Thread */
#ifdef UNDER_CE
    eError = pthread_create (&(pComponentPrivate->ComponentThread), &attr,
                                       G723_1ENC_CompThread, pComponentPrivate);
#else
    eError = pthread_create (&(pComponentPrivate->ComponentThread), NULL,
                                       G723_1ENC_CompThread, pComponentPrivate);
#endif
    if (eError || !pComponentPrivate->ComponentThread) {
       eError = OMX_ErrorInsufficientResources;
       goto EXIT;
    }

    pComponentPrivate->bCompThreadStarted = 1;
EXIT:
    G723_1ENC_DPRINT("%d :: Exiting G723_1ENC_StartComponentThread\n", __LINE__);
    G723_1ENC_DPRINT("%d :: Returning = 0x%x\n",__LINE__,eError);
    return eError;
}

/* ========================================================================== */
/**
* @G723_1ENC_FreeCompResources() This function is called by the component during
* de-init , to free Command pipe, data pipe & LCML pipe.
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

OMX_ERRORTYPE G723_1ENC_FreeCompResources(OMX_HANDLETYPE pComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate = (G723_1ENC_COMPONENT_PRIVATE *)
                                                     pHandle->pComponentPrivate;
    G723_1ENC_DPRINT("%d :: Entering G723_1ENC_FreeCompResources\n",__LINE__);

    if (pComponentPrivate->bCompThreadStarted) {
        OMX_NBCLOSE_PIPE(pComponentPrivate->dataPipe[0],err);
        OMX_NBCLOSE_PIPE(pComponentPrivate->dataPipe[1],err);
        OMX_NBCLOSE_PIPE(pComponentPrivate->cmdPipe[0],err);
        OMX_NBCLOSE_PIPE(pComponentPrivate->cmdPipe[1],err);
        OMX_NBCLOSE_PIPE(pComponentPrivate->cmdDataPipe[0],err);
        OMX_NBCLOSE_PIPE(pComponentPrivate->cmdDataPipe[1],err);
    }

/*    if (pComponentPrivate->bPortDefsAllocated) {*/
        OMX_NBMEMFREE_STRUCT(pComponentPrivate->pPortDef[G723_1ENC_INPUT_PORT]);
        OMX_NBMEMFREE_STRUCT(pComponentPrivate->pPortDef[G723_1ENC_OUTPUT_PORT]);
        OMX_NBMEMFREE_STRUCT(pComponentPrivate->G723_1Params[G723_1ENC_INPUT_PORT]);
        OMX_NBMEMFREE_STRUCT(pComponentPrivate->G723_1Params[G723_1ENC_OUTPUT_PORT]);

        OMX_NBMEMFREE_STRUCT(pComponentPrivate->pCompPort[G723_1ENC_INPUT_PORT]->pPortFormat);
        OMX_NBMEMFREE_STRUCT(pComponentPrivate->pCompPort[G723_1ENC_OUTPUT_PORT]->pPortFormat);
        OMX_NBMEMFREE_STRUCT(pComponentPrivate->pCompPort[G723_1ENC_INPUT_PORT]);
        OMX_NBMEMFREE_STRUCT(pComponentPrivate->pCompPort[G723_1ENC_OUTPUT_PORT]);

        OMX_NBMEMFREE_STRUCT(pComponentPrivate->sPortParam);
        OMX_NBMEMFREE_STRUCT(pComponentPrivate->sPriorityMgmt);
        OMX_NBMEMFREE_STRUCT(pComponentPrivate->pInputBufferList);
        OMX_NBMEMFREE_STRUCT(pComponentPrivate->pOutputBufferList);

 /*   }*/
    pComponentPrivate->bPortDefsAllocated = 0;
EXIT:
    G723_1ENC_DPRINT("%d :: Exiting G723_1ENC_FreeCompResources()\n",__LINE__);
    G723_1ENC_DPRINT("%d :: Returning = 0x%x\n",__LINE__,eError);
    return eError;
}

/* ========================================================================== */
/**
* @G723_1ENC_CleanupInitParams() This function is called by the component during
* de-init to free structues that are been allocated at intialization stage
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

OMX_ERRORTYPE G723_1ENC_CleanupInitParams(OMX_HANDLETYPE pComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 i = 0;
    OMX_U8 *pTemp = NULL;
    LCML_DSP_INTERFACE *pLcmlHandle=NULL;
    LCML_DSP_INTERFACE *pLcmlHandleAux=NULL;
    G723_1ENC_LCML_BUFHEADERTYPE *pTemp_lcml = NULL;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate = (G723_1ENC_COMPONENT_PRIVATE *)
                                                     pHandle->pComponentPrivate;
    G723_1ENC_DPRINT("%d :: Entering G723_1ENC_CleanupInitParams()\n", __LINE__);

    if(pComponentPrivate->dasfMode == 1) {
        
        pTemp = (OMX_U8*)pComponentPrivate->strmAttr;
        if(pTemp!=NULL)
               pTemp -=128;
        OMX_NBMEMFREE_STRUCT(pTemp);
    }

    pTemp_lcml = pComponentPrivate->pLcmlBufHeader[G723_1ENC_INPUT_PORT];
         
    for(i=0; i<pComponentPrivate->nRuntimeInputBuffers; i++) {
          if( pTemp_lcml->pFrameParam != NULL ){
                 pLcmlHandle = (LCML_DSP_INTERFACE *)pComponentPrivate->pLcmlHandle;
                 pLcmlHandleAux = (LCML_DSP_INTERFACE *)(((LCML_CODEC_INTERFACE *)pLcmlHandle->pCodecinterfacehandle)->pCodec);
                                  
                 OMX_DmmUnMap(pLcmlHandleAux->dspCodec->hProc,
                             (void*)pTemp_lcml->pIpParam->pParamElem,
                             pTemp_lcml->pDmmBuf->pReserved);         
                 pTemp = (OMX_U8*)pTemp_lcml->pFrameParam;
                 pTemp -= 128;
                 SafeFree(pTemp);
                 pTemp = NULL;
                 pTemp_lcml->pFrameParam = NULL;  
          }

          if(pTemp_lcml->pDmmBuf!=NULL){
                 SafeFree(pTemp_lcml->pDmmBuf);
                 pTemp_lcml->pDmmBuf = NULL;
          }

          OMX_NBMEMFREE_STRUCT(pTemp_lcml->pIpParam);                          
          pTemp_lcml++;
    }

    pTemp_lcml = pComponentPrivate->pLcmlBufHeader[G723_1ENC_OUTPUT_PORT];

    for(i=0; i<pComponentPrivate->nRuntimeOutputBuffers; i++) {
/*        OMX_NBMEMFREE_STRUCT(pTemp_lcml->pOpParam);*/ /* according to the SN guide, the params on the */
                                                        /* output buffer shoul not be needed                 */
        pTemp_lcml++;
    }

    OMX_NBMEMFREE_STRUCT(pComponentPrivate->pLcmlBufHeader[G723_1ENC_INPUT_PORT]);
    OMX_NBMEMFREE_STRUCT(pComponentPrivate->pLcmlBufHeader[G723_1ENC_OUTPUT_PORT]);

    G723_1ENC_DPRINT("%d :: Exiting G723_1ENC_CleanupInitParams()\n",__LINE__);
    G723_1ENC_DPRINT("%d :: Returning = 0x%x\n",__LINE__,eError);
    return eError;
}

/* ========================================================================== */
/**
* @G723_1ENC_StopComponentThread() This function is called by the component during
* de-init to close component thread.
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

OMX_ERRORTYPE G723_1ENC_StopComponentThread(OMX_HANDLETYPE pComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_ERRORTYPE threadError = OMX_ErrorNone;
    int pthreadError = 0;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate = (G723_1ENC_COMPONENT_PRIVATE *)
                                                     pHandle->pComponentPrivate;
    G723_1ENC_DPRINT("%d :: Entering G723_1ENC_StopComponentThread\n",__LINE__);
    G723_1ENC_DPRINT("%d :: About to call pthread_join\n",__LINE__);
    pthreadError = pthread_join (pComponentPrivate->ComponentThread,
                                 (void*)&threadError);
    if (0 != pthreadError) {
        eError = OMX_ErrorHardware;
        G723_1ENC_DPRINT("%d :: Error closing ComponentThread - pthreadError = %d\n",__LINE__,pthreadError);
        goto EXIT;
    }
    if (OMX_ErrorNone != threadError && OMX_ErrorNone != eError) {
        eError = OMX_ErrorInsufficientResources;
        G723_1ENC_DPRINT("%d :: Error while closing Component Thread\n",__LINE__);
        goto EXIT;
    }
EXIT:
   G723_1ENC_DPRINT("%d :: Exiting G723_1ENC_StopComponentThread\n",__LINE__);
   G723_1ENC_DPRINT("%d :: Returning = 0x%x\n",__LINE__,eError);
   return eError;
}


/* ========================================================================== */
/**
* @G723_1ENC_HandleCommand() This function is called by the component when ever it
* receives the command from the application
*
* @param pComponentPrivate  Component private data
*
* @pre
*
* @post
*
* @return none
*/
/* ========================================================================== */

OMX_U32 G723_1ENC_HandleCommand (G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMMANDTYPE command;
    OMX_STATETYPE commandedState = OMX_StateInvalid;
    OMX_HANDLETYPE pLcmlHandle;
    LCML_CALLBACKTYPE cb;
    LCML_DSP *pLcmlDsp = NULL;
    OMX_U32 pValues[4] = {0};
    OMX_U32 commandData = 0;
    OMX_U16 arr[100] = {0};
    OMX_STRING p = "damedesuStr";
    OMX_U32 i = 0;
    OMX_U32 ret = 0;
    OMX_U32 nTimeout = 0;
    G723_1ENC_LCML_BUFHEADERTYPE *pLcmlHdr = NULL;
    OMX_U8 inputPortFlag=0,outputPortFlag=0;
    OMX_U8* pParamsTemp = NULL;        
    OMX_U8* pAlgParmTemp = NULL;
   
#ifdef RESOURCE_MANAGER_ENABLED
    OMX_ERRORTYPE rm_error = OMX_ErrorNone;
#endif

    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *) pComponentPrivate->pHandle;
    pLcmlHandle = pComponentPrivate->pLcmlHandle;

    ret = read(pComponentPrivate->cmdPipe[0], &command, sizeof (command));
    if (ret == -1) {
        G723_1ENC_DPRINT("%d :: Error in Reading from the Data pipe\n", __LINE__);
        eError = OMX_ErrorHardware;
        goto EXIT;
    }
    ret = read(pComponentPrivate->cmdDataPipe[0], &commandData, sizeof (commandData));
    if (ret == -1) {
        G723_1ENC_DPRINT("%d :: Error in Reading from the Data pipe\n", __LINE__);
        eError = OMX_ErrorHardware;
        goto EXIT;
    }

    if (command == OMX_CommandStateSet) {
        commandedState = (OMX_STATETYPE)commandData;
        switch(commandedState) {
        case OMX_StateIdle:
            G723_1ENC_DPRINT("%d :: G723_1ENC_HandleCommand :: OMX_StateIdle \n",__LINE__);
            G723_1ENC_DPRINT("%d :: pComponentPrivate->curState = %d\n",__LINE__,pComponentPrivate->curState);
            if (pComponentPrivate->curState == commandedState){
                pComponentPrivate->cbInfo.EventHandler ( pHandle,
                                                         pHandle->pApplicationPrivate,
                                                         OMX_EventError,
                                                         OMX_ErrorSameState,
                                                         0,
                                                         NULL);
                G723_1ENC_DPRINT("%d :: Error: Same State Given by Application\n",__LINE__);
            }
            else if (pComponentPrivate->curState == OMX_StateLoaded || pComponentPrivate->curState == OMX_StateWaitForResources) {
                          
                        if (pComponentPrivate->pPortDef[G723_1ENC_INPUT_PORT]->bPopulated &&  pComponentPrivate->pPortDef[G723_1ENC_INPUT_PORT]->bEnabled){
                                        inputPortFlag = 1;
                        }
                        if (pComponentPrivate->pPortDef[G723_1ENC_OUTPUT_PORT]->bPopulated && pComponentPrivate->pPortDef[G723_1ENC_OUTPUT_PORT]->bEnabled){
                                        outputPortFlag = 1;
                        }
                        if(( pComponentPrivate->dasfMode && !outputPortFlag) ||
                           (!pComponentPrivate->dasfMode && (!inputPortFlag || !outputPortFlag)))                                   
                        {
                              /* Sleep for a while, so the application thread can allocate buffers */
                              G723_1ENC_DPRINT("%d :: Sleeping...\n",__LINE__);
                              pComponentPrivate->InLoaded_readytoidle = 1;
#ifndef UNDER_CE
                              pthread_mutex_lock(&pComponentPrivate->InLoaded_mutex);
                              pthread_cond_wait(&pComponentPrivate->InLoaded_threshold, &pComponentPrivate->InLoaded_mutex);
                              pthread_mutex_unlock(&pComponentPrivate->InLoaded_mutex);
#endif
                        }
                    
                
                cb.LCML_Callback = (void *) G723_1ENC_LCMLCallback;

                pLcmlHandle = (OMX_HANDLETYPE) G723_1ENC_GetLCMLHandle(pComponentPrivate);

                if (pLcmlHandle == NULL) {
                    G723_1ENC_DPRINT("%d :: LCML Handle is NULL........exiting..\n",__LINE__);
                    goto EXIT;
                }

                /* Got handle of dsp via phandle filling information about DSP Specific things */
                pLcmlDsp = (((LCML_DSP_INTERFACE*)pLcmlHandle)->dspCodec);
                eError = G723_1ENC_FillLCMLInitParams(pHandle, pLcmlDsp, arr);

                if(eError != OMX_ErrorNone) {
                    G723_1ENC_DPRINT("%d :: Error from G723_1ENCFill_LCMLInitParams()\n",__LINE__);
                    goto EXIT;
                }

                pComponentPrivate->pLcmlHandle = (LCML_DSP_INTERFACE *)pLcmlHandle;
                cb.LCML_Callback = (void *) G723_1ENC_LCMLCallback;

#ifndef UNDER_CE
                eError = LCML_InitMMCodecEx(((LCML_DSP_INTERFACE *)pLcmlHandle)->pCodecinterfacehandle,
                                          p,&pLcmlHandle,(void *)p,&cb, (OMX_STRING)pComponentPrivate->sDeviceString);

#else
                eError = LCML_InitMMCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                                    p,&pLcmlHandle, (void *)p, &cb);
#endif                
                
                if(eError != OMX_ErrorNone) {
                    G723_1ENC_DPRINT("%d :: Error returned from LCML_Init()\n",__LINE__);
                    goto EXIT;
                }

#ifdef RESOURCE_MANAGER_ENABLED
                /* Need check the resource with RM */
                pComponentPrivate->rmproxyCallback.RMPROXY_Callback = (void *) G723_ResourceManagerCallback;
                if (pComponentPrivate->curState != OMX_StateWaitForResources) {
                    rm_error = RMProxy_NewSendCommand(pHandle, RMProxy_RequestResource, OMX_G723_Encoder_COMPONENT, 
                                                               G723_1ENC_CPU, 3456,&(pComponentPrivate->rmproxyCallback));
                
                if(rm_error == OMX_ErrorNone) {
                    /* resource is available */
                    pComponentPrivate->curState = OMX_StateIdle;
                    rm_error = RMProxy_NewSendCommand(pHandle,
                                                      RMProxy_StateSet,
                                                      OMX_G723_Encoder_COMPONENT,
                                                      OMX_StateIdle, 3456,NULL);
                }
                else if(rm_error == OMX_ErrorInsufficientResources) {
                    /* resource is not available, need set state to OMX_StateWaitForResources */
                    pComponentPrivate->curState = OMX_StateWaitForResources;
                    pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                            pHandle->pApplicationPrivate,
                                                            OMX_EventCmdComplete,
                                                            OMX_CommandStateSet,
                                                            pComponentPrivate->curState,
                                                            NULL);
                    G723_1ENC_DPRINT("%d :: Comp: OMX_ErrorInsufficientResources\n", __LINE__);
                }
                }
                else {
                    rm_error = RMProxy_NewSendCommand(pHandle, RMProxy_StateSet, OMX_G723_Encoder_COMPONENT, OMX_StateIdle, 3456,NULL);
                
                }                
                pComponentPrivate->curState = OMX_StateIdle;
                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandStateSet,
                                                        pComponentPrivate->curState,
                                                        NULL);                
#else                            
                pComponentPrivate->curState = OMX_StateIdle;
                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandStateSet,
                                                        pComponentPrivate->curState,
                                                        NULL);                
#endif

            }
            else if (pComponentPrivate->curState == OMX_StateExecuting) {
                G723_1ENC_DPRINT("%d :: Setting Component to OMX_StateIdle\n",__LINE__);

#ifdef HASHINGENABLE
        /*Hashing Change*/
        pLcmlHandle = (LCML_DSP_INTERFACE*)pComponentPrivate->pLcmlHandle;
        eError = LCML_FlushHashes(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle);
        if (eError != OMX_ErrorNone) {
            G723_1ENC_DPRINT("Error occurred in Codec mapping flush!\n");
            break;
        }
#endif
                G723_1ENC_DPRINT("%d :: G723_1ENC: About to Call MMCodecControlStop\n", __LINE__);
                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                                    MMCodecControlStop,(void *)p);
                if(eError != OMX_ErrorNone) {
                    G723_1ENC_DPRINT("%d :: Error from LCML_ControlCodec MMCodecControlStop..\n",__LINE__);
                    goto EXIT;
                }
                
                pAlgParmTemp = (OMX_U8*)pComponentPrivate->pAlgParam;
                if(pAlgParmTemp!=NULL)
                       pAlgParmTemp -=128;
                OMX_NBMEMFREE_STRUCT(pAlgParmTemp);         
                
                if(pComponentPrivate->dasfMode == 1) {
                      pParamsTemp = (OMX_U8*)pComponentPrivate->pParams;
                      if (pParamsTemp != NULL)
                            pParamsTemp -= 128;
                      OMX_NBMEMFREE_STRUCT(pParamsTemp);
                }

                if(pComponentPrivate->ptempBuffer!=NULL){
                      OMX_NBMEMFREE_STRUCT(pComponentPrivate->ptempBuffer);
                }

                if(pComponentPrivate->pHoldBuffer!=NULL){
                      OMX_NBMEMFREE_STRUCT(pComponentPrivate->pHoldBuffer);
                }

                pComponentPrivate->nHoldLength = 0;
                pComponentPrivate->lastOutBufArrived=NULL;
                pComponentPrivate->LastBufSent = 0;

            }
            else if(pComponentPrivate->curState == OMX_StatePause) {

#ifdef HASHINGENABLE
        /*Hashing Change*/
        pLcmlHandle = (LCML_DSP_INTERFACE*)pComponentPrivate->pLcmlHandle;
        /* clear out any mappings that might have accumulated */
        eError = LCML_FlushHashes(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle);
        if (eError != OMX_ErrorNone) {
            G723_1ENC_DPRINT("Error occurred in Codec mapping flush!\n");
            break;
        }
#endif              
        
                pComponentPrivate->curState = OMX_StateIdle;

#ifdef RESOURCE_MANAGER_ENABLED
                rm_error = RMProxy_NewSendCommand(pHandle, RMProxy_StateSet, OMX_G723_Encoder_COMPONENT, OMX_StateIdle, 3456,NULL);
#endif
                pComponentPrivate->cbInfo.EventHandler ( pHandle,
                                                         pHandle->pApplicationPrivate,
                                                         OMX_EventCmdComplete,
                                                         OMX_CommandStateSet,
                                                         pComponentPrivate->curState,
                                                         NULL);
            } else {    /* This means, it is invalid state from application */
                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventError,
                                                        OMX_ErrorIncorrectStateTransition,
                                                        0,
                                                        NULL);
                G723_1ENC_DPRINT("%d :: Comp: OMX_ErrorIncorrectStateTransition\n",__LINE__);
            }
            break;

        case OMX_StateExecuting:
            G723_1ENC_DPRINT("%d :: G723_1ENC_HandleCommand :: OMX_StateExecuting \n",__LINE__);
            if (pComponentPrivate->curState == commandedState){
                pComponentPrivate->cbInfo.EventHandler ( pHandle,
                                                         pHandle->pApplicationPrivate,
                                                         OMX_EventError,
                                                         OMX_ErrorSameState,
                                                         0,
                                                         NULL);
                G723_1ENC_DPRINT("%d :: Comp: OMX_ErrorSameState Given by Comp\n",__LINE__);
            }
            else if (pComponentPrivate->curState == OMX_StateIdle) 
                {                                  
/*-------------------------------------------*/
               
            pAlgParmTemp = (OMX_U8*)SafeMalloc(sizeof(G723_1ENC_TALGCtrl) + 256);
                if(pAlgParmTemp == NULL){
                       G723_1ENC_EPRINT("***********************************\n");
                       G723_1ENC_EPRINT("%d :: Malloc Failed\n",__LINE__);
                       G723_1ENC_EPRINT("***********************************\n");
                       eError = OMX_ErrorInsufficientResources;
                       goto EXIT;
                }

                pComponentPrivate->pAlgParam = (G723_1ENC_TALGCtrl*)(pAlgParmTemp + 128);
                pComponentPrivate->pAlgParam->frameSize = 480;/* needs to be reviewed!! */
                pComponentPrivate->pAlgParam->bitRate = pComponentPrivate->G723_1Params[G723_1ENC_OUTPUT_PORT]->eBitRate;
                /*pComponentPrivate->pAlgParam->bitRate = OMX_AUDIO_G723ModeHigh;*/
                pComponentPrivate->pAlgParam->vadFlag = pComponentPrivate->G723_1Params[G723_1ENC_OUTPUT_PORT]->bDTX; 
                /*pComponentPrivate->pAlgParam->vadFlag = 1;*/
                pComponentPrivate->pAlgParam->noiseSuppressionMode = pComponentPrivate->G723_1Params[G723_1ENC_OUTPUT_PORT]->bHiPassFilter; 
                /*pComponentPrivate->pAlgParam->noiseSuppressionMode = 1;*/


                pComponentPrivate->pAlgParam->size = sizeof (G723_1ENC_TALGCtrl);
                    
                pValues[0] = ALGCMD_NOISE_SUPP_MODE;                    /*setting the noise suppression mode*/
                pValues[1] = (OMX_U32)pComponentPrivate->pAlgParam;
                pValues[2] = sizeof (G723_1ENC_TALGCtrl);
                p = (void *)&pValues;
                G723_1ENC_DPRINT("%d :: EMMCodecControlAlgCtrl-1 Sending...\n",__LINE__);
                /* Sending ALGCTRL MESSAGE DTX to DSP via LCML_ControlCodec*/
                G723_1ENC_DPRINT("--->Setting ALGCMD_NOISE_SUPP_MODE to %d\n",pComponentPrivate->pAlgParam->noiseSuppressionMode);
                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                EMMCodecControlAlgCtrl, (void *)p);
                if (eError != OMX_ErrorNone) {
                    G723_1ENC_EPRINT("%d :: Error from LCML_ControlCodec EMMCodecControlAlgCtrl-1 failed = %x\n",__LINE__,eError);
                    goto EXIT;
                }
                
                pValues[0] = ALGCMD_BITRATE;                    /*setting the bit-rate*/
                pValues[1] = (OMX_U32)pComponentPrivate->pAlgParam;
                pValues[2] = sizeof (G723_1ENC_TALGCtrl);
                p = (void *)&pValues;
                G723_1ENC_DPRINT("%d :: EMMCodecControlAlgCtrl-1 Sending...\n",__LINE__);
                /* Sending ALGCTRL MESSAGE DTX to DSP via LCML_ControlCodec*/
                G723_1ENC_DPRINT("--->Setting ALGCMD_BITRATE to %s\n",((pComponentPrivate->pAlgParam->bitRate == OMX_AUDIO_G723ModeLow) ? "5.3" : "6.3"));
                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                EMMCodecControlAlgCtrl, (void *)p);
                if (eError != OMX_ErrorNone) {
                    G723_1ENC_EPRINT("%d :: Error from LCML_ControlCodec EMMCodecControlAlgCtrl-1 failed = %x\n",__LINE__,eError);
                    goto EXIT;
                }
                
                pValues[0] = ALGCMD_DTX;                    /*setting DTX mode*/
                pValues[1] = (OMX_U32)pComponentPrivate->pAlgParam;
                pValues[2] = sizeof (G723_1ENC_TALGCtrl);
                p = (void *)&pValues;
                G723_1ENC_DPRINT("%d :: EMMCodecControlAlgCtrl-2 Sending...\n",__LINE__);
                /* Sending ALGCTRL MESSAGE BITRATE to DSP via LCML_ControlCodec*/
                G723_1ENC_DPRINT("--->Setting ALGCMD_DTX to %d\n",pComponentPrivate->pAlgParam->vadFlag);
                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                EMMCodecControlAlgCtrl, (void *)p);
                if (eError != OMX_ErrorNone) {
                    G723_1ENC_EPRINT("%d :: Error from LCML_ControlCodec EMMCodecControlAlgCtrl-2 failed = %x\n",__LINE__,eError);
                    goto EXIT;
                }
/*-------------------------------------------*/
                if(pComponentPrivate->dasfMode == 1) {
                    G723_1ENC_DPRINT("%d :: ---- Comp: DASF Functionality is ON ---\n",__LINE__);
                    pParamsTemp = (OMX_U8*)SafeMalloc(sizeof(G723_1ENC_AudioCodecParams) + 256);
                    if(pParamsTemp == NULL){
                        G723_1ENC_EPRINT("***********************************\n");
                        G723_1ENC_EPRINT("%d :: Malloc Failed\n",__LINE__);
                        G723_1ENC_EPRINT("***********************************\n");
                        eError = OMX_ErrorInsufficientResources;
                        goto EXIT;
                    }

                    pComponentPrivate->pParams = (G723_1ENC_AudioCodecParams*)(pParamsTemp + 128);
                    pComponentPrivate->pParams->iAudioFormat = 1;
                    pComponentPrivate->pParams->iStrmId = pComponentPrivate->streamID;
                    pComponentPrivate->pParams->iSamplingRate = G723_1ENC_SAMPLING_FREQUENCY;
                    pValues[0] = USN_STRMCMD_SETCODECPARAMS;
                    pValues[1] = (OMX_U32)pComponentPrivate->pParams;
                    pValues[2] = sizeof(G723_1ENC_AudioCodecParams);
                    /* Sending STRMCTRL MESSAGE to DSP via LCML_ControlCodec*/
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                              EMMCodecControlStrmCtrl,(void *)pValues);
                    if(eError != OMX_ErrorNone) {
                       G723_1ENC_DPRINT("%d :: Error from LCML_ControlCodec EMMCodecControlStrmCtrl = %x\n",__LINE__,eError);
                       goto EXIT;
                    }
                }

                /* Sending START MESSAGE to DSP via LCML_ControlCodec*/
                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                EMMCodecControlStart, (void *)p);
                if(eError != OMX_ErrorNone) {
                    G723_1ENC_DPRINT("%d :: Error from LCML_ControlCodec EMMCodecControlStart = %x\n",__LINE__,eError);
                    goto EXIT;
                }

            } else if (pComponentPrivate->curState == OMX_StatePause) {
                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                EMMCodecControlStart, (void *)p);
                if (eError != OMX_ErrorNone) {
                    G723_1ENC_DPRINT("%d :: Error While Resuming the codec = %x\n",__LINE__,eError);
                    goto EXIT;
                }

                for (i=0; i < pComponentPrivate->nNumInputBufPending; i++) {
                    if (pComponentPrivate->pInputBufHdrPending[i]) {
                        G723_1ENC_GetCorrespondingLCMLHeader(pComponentPrivate, pComponentPrivate->pInputBufHdrPending[i]->pBuffer, OMX_DirInput, &pLcmlHdr);
                        G723_1ENC_SetPending(pComponentPrivate,pComponentPrivate->pInputBufHdrPending[i],OMX_DirInput,__LINE__);

                        eError = LCML_QueueBuffer(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                    EMMCodecInputBuffer,
                                                    pComponentPrivate->pInputBufHdrPending[i]->pBuffer,
                                                    pComponentPrivate->pInputBufHdrPending[i]->nAllocLen,
                                                    pComponentPrivate->pInputBufHdrPending[i]->nFilledLen,
                                                    (OMX_U8 *) pLcmlHdr->pIpParam,
                                                    sizeof(G723ENC_ParamStruct),
                                                    NULL);
                    }
                }
                pComponentPrivate->nNumInputBufPending = 0;


                for (i=0; i < pComponentPrivate->nNumOutputBufPending; i++) {
                    if (pComponentPrivate->pOutputBufHdrPending[i]) {
                        G723_1ENC_GetCorrespondingLCMLHeader(pComponentPrivate, pComponentPrivate->pOutputBufHdrPending[i]->pBuffer, OMX_DirOutput, &pLcmlHdr);
                        G723_1ENC_SetPending(pComponentPrivate,pComponentPrivate->pOutputBufHdrPending[i],OMX_DirOutput,__LINE__);
                        eError = LCML_QueueBuffer(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                    EMMCodecOuputBuffer,
                                                    pComponentPrivate->pOutputBufHdrPending[i]->pBuffer,
                                                    pComponentPrivate->pOutputBufHdrPending[i]->nAllocLen,
                                                    /*pComponentPrivate->pOutputBufHdrPending[i]->nFilledLen*/0,
                                                    /*(OMX_U8 *) pLcmlHdr->pIpParam*/NULL,
                                                    /*sizeof(G723ENC_ParamStruct)*/0,
                                                    NULL);
                    }
                }
                pComponentPrivate->nNumOutputBufPending = 0;
            } else {
                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventError,
                                                        OMX_ErrorIncorrectStateTransition,
                                                        0,
                                                        NULL);
                G723_1ENC_DPRINT("%d :: Comp: OMX_ErrorIncorrectStateTransition Given by Comp\n",__LINE__);
                goto EXIT;

            }
            pComponentPrivate->curState = OMX_StateExecuting;
#ifdef RESOURCE_MANAGER_ENABLED
            rm_error = RMProxy_NewSendCommand(pHandle, RMProxy_StateSet, OMX_G723_Encoder_COMPONENT, OMX_StateExecuting, 3456,NULL);
#endif
            pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                    pHandle->pApplicationPrivate,
                                                    OMX_EventCmdComplete,
                                                    OMX_CommandStateSet,
                                                    pComponentPrivate->curState,
                                                    NULL);
            G723_1ENC_DPRINT("%d :: Comp: OMX_CommandStateSet Given by Comp\n",__LINE__);
            break;

        case OMX_StateLoaded:
            G723_1ENC_DPRINT("%d :: G723_1ENC_HandleCommand :: OMX_StateLoaded\n",__LINE__);
            if (pComponentPrivate->curState == commandedState){
                pComponentPrivate->cbInfo.EventHandler ( pHandle,
                                                         pHandle->pApplicationPrivate,
                                                         OMX_EventError,
                                                         OMX_ErrorSameState,
                                                         0,
                                                         NULL);
                G723_1ENC_DPRINT("%d :: Comp: OMX_ErrorSameState Given by Comp\n",__LINE__);
                break;
             }
            if (pComponentPrivate->curState == OMX_StateWaitForResources){
                G723_1ENC_DPRINT("%d :: G723_1ENC_HandleCommand :: OMX_StateWaitForResources\n",__LINE__);
                pComponentPrivate->curState = OMX_StateLoaded;
                pComponentPrivate->cbInfo.EventHandler ( pHandle,
                                                         pHandle->pApplicationPrivate,
                                                         OMX_EventCmdComplete,
                                                         OMX_CommandStateSet,
                                                         pComponentPrivate->curState,
                                                         NULL);
                G723_1ENC_DPRINT("%d :: Comp: OMX_CommandStateSet Given by Comp\n",__LINE__);
                break;
            }
            if (pComponentPrivate->curState != OMX_StateIdle &&
                pComponentPrivate->curState != OMX_StateWaitForResources) {
                G723_1ENC_DPRINT("%d :: G723_1ENC_HandleCommand :: OMX_StateIdle && OMX_StateWaitForResources\n",__LINE__);
                pComponentPrivate->cbInfo.EventHandler ( pHandle,
                                                         pHandle->pApplicationPrivate,
                                                         OMX_EventError,
                                                         OMX_ErrorIncorrectStateTransition,
                                                         0,
                                                         NULL);
                G723_1ENC_DPRINT("%d :: Error: OMX_ErrorIncorrectStateTransition Given by Comp\n",__LINE__);
                goto EXIT;
            }
            if (pComponentPrivate->pInputBufferList->numBuffers &&
                    pComponentPrivate->pOutputBufferList->numBuffers) { 
                         pComponentPrivate->InIdle_goingtoloaded = 1;
                         pthread_mutex_lock(&pComponentPrivate->InIdle_mutex);
                         pthread_cond_wait(&pComponentPrivate->InIdle_threshold, &pComponentPrivate->InIdle_mutex);
                         pthread_mutex_unlock(&pComponentPrivate->InIdle_mutex);
                                     
            }

            /* Now Deinitialize the component No error should be returned from
            * this function. It should clean the system as much as possible */
            G723_1ENC_CleanupInitParams(pHandle);
            
            eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                        EMMCodecControlDestroy, (void *)p);
            if (eError != OMX_ErrorNone) {
                G723_1ENC_DPRINT("%d :: Error: LCML_ControlCodec EMMCodecControlDestroy = %x\n",__LINE__, eError);
                goto EXIT;
            }
            eError = G723_1ENC_EXIT_COMPONENT_THRD;
            pComponentPrivate->bInitParamsInitialized = 0;
            break;

        case OMX_StatePause:
            G723_1ENC_DPRINT("%d :: G723_1ENC_HandleCommand :: OMX_StatePause\n",__LINE__);
            if (pComponentPrivate->curState == commandedState){
                pComponentPrivate->cbInfo.EventHandler ( pHandle,
                                                         pHandle->pApplicationPrivate,
                                                         OMX_EventError,
                                                         OMX_ErrorSameState,
                                                         0,
                                                         NULL);
                G723_1ENC_DPRINT("%d :: Error: OMX_ErrorSameState Given by Comp\n",__LINE__);
                break;
            }
            if (pComponentPrivate->curState != OMX_StateExecuting &&
                pComponentPrivate->curState != OMX_StateIdle) {
                pComponentPrivate->cbInfo.EventHandler ( pHandle,
                                                         pHandle->pApplicationPrivate,
                                                         OMX_EventError,
                                                         OMX_ErrorIncorrectStateTransition,
                                                         0,
                                                         NULL);
                G723_1ENC_DPRINT("%d :: Error: OMX_ErrorIncorrectStateTransition Given by Comp\n",__LINE__);
                goto EXIT;
            }
            eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                        EMMCodecControlPause, (void *)p);
            if (eError != OMX_ErrorNone) {
                G723_1ENC_DPRINT("%d :: Error: LCML_ControlCodec EMMCodecControlPause = %x\n",__LINE__,eError);
                goto EXIT;
            }
            pComponentPrivate->curState = OMX_StatePause;
            pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                    pHandle->pApplicationPrivate,
                                                    OMX_EventCmdComplete,
                                                    OMX_CommandStateSet,
                                                    pComponentPrivate->curState,
                                                    NULL);
            G723_1ENC_DPRINT("%d :: Comp: OMX_CommandStateSet Given by Comp\n",__LINE__);
            break;

        case OMX_StateWaitForResources:
            if (pComponentPrivate->curState == commandedState) {
                pComponentPrivate->cbInfo.EventHandler ( pHandle,
                                                         pHandle->pApplicationPrivate,
                                                         OMX_EventError,
                                                         OMX_ErrorSameState,
                                                         0,
                                                         NULL);
                G723_1ENC_DPRINT("%d :: Error: OMX_ErrorSameState Given by Comp\n",__LINE__);
            } else if (pComponentPrivate->curState == OMX_StateLoaded) {
                pComponentPrivate->curState = OMX_StateWaitForResources;
                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandStateSet,
                                                        pComponentPrivate->curState,
                                                        NULL);
                G723_1ENC_DPRINT("%d :: Comp: OMX_CommandStateSet Given by Comp\n",__LINE__);
            } else {
                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventError,
                                                        OMX_ErrorIncorrectStateTransition,
                                                        0,
                                                        NULL);
                G723_1ENC_DPRINT("%d :: Error: OMX_ErrorIncorrectStateTransition Given by Comp\n",__LINE__);
            }
            break;

        case OMX_StateInvalid:
            G723_1ENC_DPRINT("%d :: G723_1ENC_HandleCommand :: OMX_StateInvalid\n",__LINE__);
            if (pComponentPrivate->curState == commandedState){
                pComponentPrivate->cbInfo.EventHandler ( pHandle,
                                                         pHandle->pApplicationPrivate,
                                                         OMX_EventError,
                                                         OMX_ErrorSameState,
                                                         0,
                                                         NULL);
                G723_1ENC_DPRINT("%d :: Error: OMX_ErrorSameState Given by Comp\n",__LINE__);
            } else{
                pComponentPrivate->curState = OMX_StateInvalid;
                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventError,
                                                        OMX_ErrorInvalidState,
                                                        0,
                                                        NULL);
                G723_1ENC_DPRINT("%d :: Comp: OMX_ErrorInvalidState Given by Comp\n",__LINE__);
            }
            break;

        case OMX_StateMax:
            G723_1ENC_DPRINT("%d :: G723_1ENC_HandleCommand :: Cmd OMX_StateMax\n",__LINE__);
            break;
        default:
            break;
        } /* End of Switch */
        } else if (command == OMX_CommandMarkBuffer) {
        G723_1ENC_DPRINT("%d :: command OMX_CommandMarkBuffer received\n",__LINE__);
        if(!pComponentPrivate->pMarkBuf){
            /* TODO Need to handle multiple marks */
            pComponentPrivate->pMarkBuf = (OMX_MARKTYPE *)(commandData);
        }
    } else if (command == OMX_CommandPortDisable) 
        {
        if (!pComponentPrivate->bDisableCommandPending) {
            if(commandData == 0x0 || commandData == -1){
                /* disable port */
                pComponentPrivate->pPortDef[G723_1ENC_INPUT_PORT]->bEnabled = OMX_FALSE;
                G723_1ENC_DPRINT("%d :: command disabled input port\n",__LINE__);
            }
            if(commandData == 0x1 || commandData == -1){
                 /* disable output port */
                pComponentPrivate->pPortDef[G723_1ENC_OUTPUT_PORT]->bEnabled = OMX_FALSE;
                G723_1ENC_DPRINT("%d :: command disabled output port\n",__LINE__);
                if (pComponentPrivate->curState == OMX_StateExecuting) {
                    pComponentPrivate->bNoIdleOnStop = OMX_TRUE;
                    eError = LCML_ControlCodec(
                                      ((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                      MMCodecControlStop,(void *)p);
                }
            }
        }
        G723_1ENC_DPRINT("commandData = %ld\n",commandData);
        G723_1ENC_DPRINT("pComponentPrivate->pPortDef[INPUT_PORT]->bPopulated = %d\n",pComponentPrivate->pPortDef[G723_1ENC_INPUT_PORT]->bPopulated);
        G723_1ENC_DPRINT("pComponentPrivate->pPortDef[OUTPUT_PORT]->bPopulated = %d\n",pComponentPrivate->pPortDef[G723_1ENC_OUTPUT_PORT]->bPopulated);
        if(commandData == 0x0) {
            if(!pComponentPrivate->pPortDef[G723_1ENC_INPUT_PORT]->bPopulated){
                /* return cmdcomplete event if input unpopulated */
                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandPortDisable,
                                                        G723_1ENC_INPUT_PORT,
                                                        NULL);
                pComponentPrivate->bDisableCommandPending = 0;
            } else {
                pComponentPrivate->bDisableCommandPending = 1;
                pComponentPrivate->bDisableCommandParam = commandData;
            }
        }
        if(commandData == 0x1) {
            if (!pComponentPrivate->pPortDef[G723_1ENC_OUTPUT_PORT]->bPopulated){
                 /* return cmdcomplete event if output unpopulated */
                 pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                         pHandle->pApplicationPrivate,
                                                         OMX_EventCmdComplete,
                                                         OMX_CommandPortDisable,
                                                         G723_1ENC_OUTPUT_PORT,
                                                         NULL);
                 pComponentPrivate->bDisableCommandPending = 0;
            } else {
                pComponentPrivate->bDisableCommandPending = 1;
                pComponentPrivate->bDisableCommandParam = commandData;
            }
        }
        if(commandData == -1) {
            if (!pComponentPrivate->pPortDef[G723_1ENC_INPUT_PORT]->bPopulated &&
                !pComponentPrivate->pPortDef[G723_1ENC_OUTPUT_PORT]->bPopulated){
                 /* return cmdcomplete event if inout & output unpopulated */
                 pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                         pHandle->pApplicationPrivate,
                                                         OMX_EventCmdComplete,
                                                         OMX_CommandPortDisable,
                                                         G723_1ENC_INPUT_PORT,
                                                         NULL);
                 pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                         pHandle->pApplicationPrivate,
                                                         OMX_EventCmdComplete,
                                                         OMX_CommandPortDisable,
                                                         G723_1ENC_OUTPUT_PORT,
                                                         NULL);
                 pComponentPrivate->bDisableCommandPending = 0;
            } else {
                pComponentPrivate->bDisableCommandPending = 1;
                pComponentPrivate->bDisableCommandParam = commandData;
            }
#ifndef UNDER_CE
            sched_yield();           
#endif
        }
    } else if (command == OMX_CommandPortEnable) {
        if(commandData == 0x0 || commandData == -1){
            /* enable in port */
            G723_1ENC_DPRINT("%d :: setting input port to enabled\n",__LINE__);
            pComponentPrivate->pPortDef[G723_1ENC_INPUT_PORT]->bEnabled = OMX_TRUE;
            if(pComponentPrivate->AlloBuf_waitingsignal)
            {
                 pComponentPrivate->AlloBuf_waitingsignal = 0;
#ifndef UNDER_CE
                 pthread_mutex_lock(&pComponentPrivate->AlloBuf_mutex);
                 pthread_cond_signal(&pComponentPrivate->AlloBuf_threshold);
                 pthread_mutex_unlock(&pComponentPrivate->AlloBuf_mutex);
#endif
            }
            G723_1ENC_DPRINT("pComponentPrivate->pPortDef[G723_1ENC_INPUT_PORT]->bEnabled = %d\n",pComponentPrivate->pPortDef[G723_1ENC_INPUT_PORT]->bEnabled);
        }
        if(commandData == 0x1 || commandData == -1){
            /* enable out port */
            if(pComponentPrivate->AlloBuf_waitingsignal)
            {
                 pComponentPrivate->AlloBuf_waitingsignal = 0;
#ifndef UNDER_CE
                 pthread_mutex_lock(&pComponentPrivate->AlloBuf_mutex);
                 pthread_cond_signal(&pComponentPrivate->AlloBuf_threshold);
                 pthread_mutex_unlock(&pComponentPrivate->AlloBuf_mutex);
#endif
            }
            if (pComponentPrivate->curState == OMX_StateExecuting) {
                pComponentPrivate->bDspStoppedWhileExecuting = OMX_FALSE;
                eError = LCML_ControlCodec(
                                      ((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                      EMMCodecControlStart,(void *)p);
            }
            G723_1ENC_DPRINT("%d :: setting output port to enabled\n",__LINE__);
            pComponentPrivate->pPortDef[G723_1ENC_OUTPUT_PORT]->bEnabled = OMX_TRUE;
            G723_1ENC_DPRINT("pComponentPrivate->pPortDef[G723_1ENC_OUTPUT_PORT]->bEnabled = %d\n",pComponentPrivate->pPortDef[G723_1ENC_OUTPUT_PORT]->bEnabled);
        }

        while (1) {
            G723_1ENC_DPRINT("pComponentPrivate->curState = %d\n",pComponentPrivate->curState);
            G723_1ENC_DPRINT("pComponentPrivate->pPortDef[G723_1ENC_INPUT_PORT]->bPopulated = %d\n",pComponentPrivate->pPortDef[G723_1ENC_INPUT_PORT]->bPopulated);
            if(commandData == 0x0 && (pComponentPrivate->curState == OMX_StateLoaded ||
                pComponentPrivate->pPortDef[G723_1ENC_INPUT_PORT]->bPopulated)){
                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandPortEnable,
                                                        G723_1ENC_INPUT_PORT,
                                                        NULL);
                G723_1ENC_DPRINT("%d :: setting Input port to enabled\n",__LINE__);
                break;
            } else if(commandData == 0x1 && (pComponentPrivate->curState == OMX_StateLoaded ||
              pComponentPrivate->pPortDef[G723_1ENC_OUTPUT_PORT]->bPopulated)){
                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandPortEnable,
                                                        G723_1ENC_OUTPUT_PORT,
                                                        NULL);
                G723_1ENC_DPRINT("%d :: setting output port to enabled\n",__LINE__);
                break;
            } else if(commandData == -1 && (pComponentPrivate->curState == OMX_StateLoaded ||
              (pComponentPrivate->pPortDef[G723_1ENC_INPUT_PORT]->bPopulated
              && pComponentPrivate->pPortDef[G723_1ENC_OUTPUT_PORT]->bPopulated))){
                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandPortEnable,
                                                        G723_1ENC_INPUT_PORT,
                                                        NULL);
                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandPortEnable,
                                                        G723_1ENC_OUTPUT_PORT,
                                                        NULL);
                G723_1ENC_FillLCMLInitParamsEx(pComponentPrivate->pHandle);
                G723_1ENC_DPRINT("%d :: setting Input & Output port to enabled\n",__LINE__);
                break;
            }
            if(nTimeout++ > G723_1ENC_OMX_MAX_TIMEOUTS){
                /* return error as we have waited long enough */
                pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventError,
                                                        OMX_ErrorPortUnresponsiveDuringAllocation,
                                                        0 ,
                                                        NULL);
                break;
            }
#ifndef UNDER_CE
            sched_yield();
#endif
        }
    } else if (command == OMX_CommandFlush) {
        OMX_U32 aParam[3] = {0};
        if(commandData == 0x0 || commandData == -1) {
            aParam[0] = USN_STRMCMD_FLUSH; 
            aParam[1] = 0x0; 
            aParam[2] = 0x0; 

            G723_1ENC_DPRINT("Flushing input port\n");
            eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                        EMMCodecControlStrmCtrl, (void*)aParam);
            if (eError != OMX_ErrorNone) {
                 goto EXIT;
            }
        }
        if(commandData == 0x1 || commandData == -1){

            aParam[0] = USN_STRMCMD_FLUSH; 
            aParam[1] = 0x1; 
            aParam[2] = 0x0; 

            G723_1ENC_DPRINT("Flushing output port\n");
            eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                        EMMCodecControlStrmCtrl, (void*)aParam);
            if (eError != OMX_ErrorNone) {
                 goto EXIT;
            }

        }
    }

EXIT:
    G723_1ENC_DPRINT("%d :: Exiting G723_1ENC_HandleCommand Function\n",__LINE__);
    G723_1ENC_DPRINT("%d :: Returning = 0x%x\n",__LINE__,eError);
    return eError;
}

/* ========================================================================== */
/**
* @G723_1ENC_HandleDataBufFromApp() This function is called by the component when ever it
* receives the buffer from the application
*
* @param pComponentPrivate  Component private data
* @param pBufHeader Buffer from the application
*
* @pre
*
* @post
*
* @return none
*/
/* ========================================================================== */
OMX_ERRORTYPE G723_1ENC_HandleDataBufFromApp(OMX_BUFFERHEADERTYPE* pBufHeader,
                                    G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_DIRTYPE eDir;
    G723_1ENC_LCML_BUFHEADERTYPE *pLcmlHdr=NULL;
    LCML_DSP_INTERFACE *phandle = NULL;
    OMX_U8  nFrames = 0,i = 0;
    OMX_U8* pBufParmsTemp=NULL;
    LCML_DSP_INTERFACE *pLcmlHandle = (LCML_DSP_INTERFACE *) pComponentPrivate->pLcmlHandle;
    OMX_U8* pExtraData=NULL;
    OMX_U32 remainingBytes=0, frameLength=0;


    G723_1ENC_DPRINT ("%d :: Entering G723_1ENC_HandleDataBufFromApp Function\n",__LINE__);
    /*Find the direction of the received buffer from buffer list */
    eError = G723_1ENC_GetBufferDirection(pBufHeader, &eDir);
    if (eError != OMX_ErrorNone) {
        G723_1ENC_DPRINT ("%d :: The pBufHeader is not found in the list\n", __LINE__);
        goto EXIT;
    }

    if (eDir == OMX_DirInput) {
        if(pComponentPrivate->dasfMode == 0) {
            if ((pBufHeader->nFilledLen > 0) || (pBufHeader->nFlags == OMX_BUFFERFLAG_EOS)) {

                eError = G723_1ENC_GetCorrespondingLCMLHeader(pComponentPrivate, pBufHeader->pBuffer, OMX_DirInput, &pLcmlHdr);
                if (eError != OMX_ErrorNone) {
                    G723_1ENC_DPRINT("%d :: Error: Invalid Buffer Came ...\n",__LINE__);
                    goto EXIT;
                }
                
                /* Store time stamp information */
                pComponentPrivate->arrTimestamp[pComponentPrivate->IpBufindex] = pBufHeader->nTimeStamp;
                /* Store nTickCount information */
                pComponentPrivate->arrTickCount[pComponentPrivate->IpBufindex] = pBufHeader->nTickCount;
                pComponentPrivate->IpBufindex++;
                pComponentPrivate->IpBufindex %= pComponentPrivate->pPortDef[G723_1ENC_OUTPUT_PORT]->nBufferCountActual;
                            
                if(pComponentPrivate->nHoldLength && pComponentPrivate->pHoldBuffer){
                        if((pComponentPrivate->nHoldLength+pBufHeader->nFilledLen) > pBufHeader->nAllocLen){
                                /*means that a second Acumulator must be used to insert holdbuffer to pbuffer and save remaining bytes into hold buffer*/
                                remainingBytes = pComponentPrivate->nHoldLength+pBufHeader->nFilledLen-pBufHeader->nAllocLen;
                                if (pComponentPrivate->ptempBuffer == NULL) {
                                        pComponentPrivate->ptempBuffer = SafeMalloc(G723_1ENC_INPUT_BUFFER_SIZE);
                                        if (!pComponentPrivate->ptempBuffer) {
                                                eError = OMX_ErrorInsufficientResources;
                                                goto EXIT;
                                        }

                                }
                                pExtraData = (pBufHeader->pBuffer)+(pBufHeader->nFilledLen-remainingBytes);
                                memcpy(pComponentPrivate->ptempBuffer,pExtraData,remainingBytes);
                                pBufHeader->nFilledLen-=remainingBytes;                                                                                                              }
                        memmove(pBufHeader->pBuffer+pComponentPrivate->nHoldLength, pBufHeader->pBuffer, pBufHeader->nFilledLen);
                        memcpy(pBufHeader->pBuffer,pComponentPrivate->pHoldBuffer, pComponentPrivate->nHoldLength);
                        pBufHeader->nFilledLen+=pComponentPrivate->nHoldLength;
                        if(remainingBytes){
                                memcpy(pComponentPrivate->pHoldBuffer, pComponentPrivate->ptempBuffer, remainingBytes);
                                pComponentPrivate->nHoldLength=remainingBytes;
                                remainingBytes=0;
                        }
                        else{
                                pComponentPrivate->nHoldLength=0;
                        }                         
                }

                frameLength = G723_1ENC_INPUT_BUFFER_SIZE;
                nFrames = (OMX_U8)(pBufHeader->nFilledLen / G723_1ENC_INPUT_BUFFER_SIZE);              
                if( nFrames>=1 || (pBufHeader->nFlags == OMX_BUFFERFLAG_EOS)){ /* At least there is 1 frame in the buffer */
                        if (!pComponentPrivate->nHoldLength){
                             if(pBufHeader->nFlags != OMX_BUFFERFLAG_EOS){
                                     pComponentPrivate->nHoldLength = pBufHeader->nFilledLen - frameLength*nFrames;
                             }
                             if (pComponentPrivate->nHoldLength > 0) {/* something need to be hold in pHoldBuffer */
                                     if (pComponentPrivate->pHoldBuffer == NULL) {
                                             pComponentPrivate->pHoldBuffer = SafeMalloc(G723_1ENC_INPUT_BUFFER_SIZE);
                                             if (!pComponentPrivate->pHoldBuffer) {
                                                  eError = OMX_ErrorInsufficientResources;
                                                  goto EXIT;
                                             }

                                     }
                             memset(pComponentPrivate->pHoldBuffer, 0, G723_1ENC_INPUT_BUFFER_SIZE);
                             /* Copy the extra data into pHoldBuffer. Size will be nHoldLength. */
                             pExtraData = pBufHeader->pBuffer + frameLength*nFrames;
                             memcpy(pComponentPrivate->pHoldBuffer, pExtraData, pComponentPrivate->nHoldLength);
                             pBufHeader->nFilledLen-=pComponentPrivate->nHoldLength;
                             } 
                        }
                }
                else{   /*less than 1 frame on the buffer, needs to be stored for future use*/
                        /* received buffer with less than 1 AMR frame length. Save the data in pHoldBuffer.*/
                        pComponentPrivate->nHoldLength = pBufHeader->nFilledLen;
                        /* save the data into pHoldBuffer */
                        if (pComponentPrivate->pHoldBuffer == NULL) {
                                pComponentPrivate->pHoldBuffer = SafeMalloc(G723_1ENC_INPUT_BUFFER_SIZE);
                                if (!pComponentPrivate->pHoldBuffer) {
                                      eError = OMX_ErrorInsufficientResources;
                                      goto EXIT;
                                }

                        }
                        /* Not enough data to be sent. Copy all received data into pHoldBuffer.*/
                        memset(pComponentPrivate->pHoldBuffer, 0, G723_1ENC_INPUT_BUFFER_SIZE);
                        memcpy(pComponentPrivate->pHoldBuffer, pBufHeader->pBuffer, pComponentPrivate->nHoldLength);
                        /* since not enough data, we shouldn't send anything to SN, but instead request to EmptyBufferDone again.*/
                        if (pComponentPrivate->curState != OMX_StatePause) {
                                G723_1ENC_DPRINT("%d :: Calling EmptyBufferDone\n",__LINE__);
#ifdef __PERF_INSTRUMENTATION__
                                PERF_SendingFrame(pComponentPrivate->pPERFcomp, pBufHeader->pBuffer, 0, PERF_ModuleHLMM);
#endif

                                pComponentPrivate->cbInfo.EmptyBufferDone( pComponentPrivate->pHandle,
                                                                           pComponentPrivate->pHandle->pApplicationPrivate,
                                                                           pBufHeader);
                                pComponentPrivate->nEmptyBufferDoneCount++;
                        }
                        else {
                                 pComponentPrivate->pInputBufHdrPending[pComponentPrivate->nNumInputBufPending++] = pBufHeader;
                             }
                        goto EXIT;
                }
                if(!nFrames)
                      nFrames++;/*In case that EOS come on a Empty or incomplete buffer*/

                if(!pLcmlHdr->pFrameParam ){

                       pBufParmsTemp = (OMX_U8*)SafeMalloc( (sizeof(G723ENC_FrameStruct)*nFrames) + 256);
                       memset(pBufParmsTemp, 0x0, sizeof(G723ENC_FrameStruct)*(nFrames)+256);
                       pLcmlHdr->pFrameParam =  (G723ENC_FrameStruct*)(pBufParmsTemp + 128);
                       phandle = (LCML_DSP_INTERFACE *)(((LCML_CODEC_INTERFACE *)pLcmlHandle->pCodecinterfacehandle)->pCodec); 
                       eError = OMX_DmmMap(phandle->dspCodec->hProc,      
                                        sizeof(G723ENC_FrameStruct),      
                                        (void*)pLcmlHdr->pFrameParam,     
                                        (pLcmlHdr->pDmmBuf));
                       pLcmlHdr->pIpParam->pParamElem = (G723ENC_FrameStruct *)pLcmlHdr->pDmmBuf->pMapped; /*DSP Address*/
                }
                                
                for(i=0;i<nFrames-1;i++){
                        (pLcmlHdr->pFrameParam+i)->usLastFrame = 0;
                }                
                if(pBufHeader->nFlags == OMX_BUFFERFLAG_EOS) {
                    pComponentPrivate->LastBufSent = 1; 
                    (pLcmlHdr->pFrameParam+nFrames-1)->usLastFrame = 1;
                }
                else {
                     (pLcmlHdr->pFrameParam+nFrames-1)->usLastFrame = 0;
                }                


                if (eError != OMX_ErrorNone){
                                G723_1ENC_DPRINT("OMX_DmmMap ERRROR!!!!\n\n");
                                goto EXIT;
                }

                pLcmlHdr->pIpParam->usNbFrames = nFrames;
               
                G723_1ENC_DPRINT("DSP Addres %p\n",pLcmlHdr->pIpParam->pParamElem);
                            
                if (pComponentPrivate->curState == OMX_StateExecuting) {
                    if(!pComponentPrivate->bDspStoppedWhileExecuting) {
                        if (!G723_1ENC_IsPending(pComponentPrivate,pBufHeader,OMX_DirInput)) {
                            G723_1ENC_SetPending(pComponentPrivate,pBufHeader,OMX_DirInput,__LINE__);

                            eError = LCML_QueueBuffer( pLcmlHandle->pCodecinterfacehandle,
                                                       EMMCodecInputBuffer,
                                                       (OMX_U8 *)pBufHeader->pBuffer,
                                                       pBufHeader->nAllocLen,
                                                       pBufHeader->nFilledLen,
                                                       (OMX_U8 *) pLcmlHdr->pIpParam,
                                                       sizeof(G723ENC_ParamStruct),
                                                       NULL);
                            if (eError != OMX_ErrorNone) {
                                eError = OMX_ErrorHardware;
                                goto EXIT;
                            }

                        }
                    } else{
                        pComponentPrivate->cbInfo.EmptyBufferDone (
                                           pComponentPrivate->pHandle,
                                           pComponentPrivate->pHandle->pApplicationPrivate,
                                           pBufHeader
                                           );
                    }

                } else {
                    pComponentPrivate->pInputBufHdrPending[pComponentPrivate->nNumInputBufPending++] = pBufHeader;
                }
            } else {
                
                G723_1ENC_DPRINT("%d :: Calling EmptyBufferDone\n",__LINE__);
                pComponentPrivate->cbInfo.EmptyBufferDone( pComponentPrivate->pHandle,
                                                           pComponentPrivate->pHandle->pApplicationPrivate,
                                                           pComponentPrivate->pInputBufferList->pBufHdr[0]);
                pComponentPrivate->nEmptyBufferDoneCount++;
            }
            if(pBufHeader->nFlags == OMX_BUFFERFLAG_EOS) {

                pComponentPrivate->pOutputBufferList->pBufHdr[0]->nFlags |= OMX_BUFFERFLAG_EOS;
                pComponentPrivate->cbInfo.EventHandler( pComponentPrivate->pHandle,
                                                        pComponentPrivate->pHandle->pApplicationPrivate,
                                                        OMX_EventBufferFlag,
                                                        pComponentPrivate->pOutputBufferList->pBufHdr[0]->nOutputPortIndex,
                                                        pComponentPrivate->pOutputBufferList->pBufHdr[0]->nFlags, NULL);
                pBufHeader->nFlags = 0;
            }
            if(pBufHeader->pMarkData){
                /* copy mark to output buffer header */
                pComponentPrivate->pOutputBufferList->pBufHdr[0]->pMarkData = pBufHeader->pMarkData;
                pComponentPrivate->pOutputBufferList->pBufHdr[0]->hMarkTargetComponent = pBufHeader->hMarkTargetComponent;
                /* trigger event handler if we are supposed to */
                if(pBufHeader->hMarkTargetComponent == pComponentPrivate->pHandle && pBufHeader->pMarkData){
                    pComponentPrivate->cbInfo.EventHandler( pComponentPrivate->pHandle,
                                                            pComponentPrivate->pHandle->pApplicationPrivate,
                                                            OMX_EventMark,
                                                            0,
                                                            0,
                                                            pBufHeader->pMarkData);
                }
            }
        }
    } else if (eDir == OMX_DirOutput) {
        /* Make sure that output buffer is issued to output stream only when
         * there is an outstanding input buffer already issued on input stream
         */
        eError = G723_1ENC_GetCorrespondingLCMLHeader(pComponentPrivate, pBufHeader->pBuffer, OMX_DirOutput, &pLcmlHdr);
        if (eError != OMX_ErrorNone) {
                    G723_1ENC_DPRINT("%d :: Error: Invalid Buffer Came ...\n",__LINE__);
                    goto EXIT;
        }
        pLcmlHdr->pOpParam->usNbFrames =1;   /*<<<<<<<<<<---------------- according SN guide this is not needed!!!!!!*/
                                             /*            The SN should be the one that set this Information            */
                                             /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
       
        if (!(pComponentPrivate->bIsStopping)) {
                G723_1ENC_DPRINT ("%d: Sending Empty OUTPUT BUFFER to Codec = %p\n",__LINE__,pBufHeader->pBuffer);

                if (pComponentPrivate->curState == OMX_StateExecuting) {
                    if (!G723_1ENC_IsPending(pComponentPrivate,pBufHeader,OMX_DirOutput)) {
                        G723_1ENC_SetPending(pComponentPrivate,pBufHeader,OMX_DirOutput,__LINE__);
                        
                        eError = LCML_QueueBuffer( pLcmlHandle->pCodecinterfacehandle,
                                                   EMMCodecOuputBuffer,
                                                   (OMX_U8 *)pBufHeader->pBuffer,
                                                   pBufHeader->nAllocLen,
                                                   pBufHeader->nFilledLen,
/*                                                   NULL,
                                                   0,*/
                                                   (OMX_U8*) pLcmlHdr->pOpParam,  /*<--According SN this should not be necesary*/
                                                   sizeof(G723ENC_ParamStruct),
                                                   NULL);
                        if (eError != OMX_ErrorNone ) {
                            G723_1ENC_DPRINT ("%d :: IssuingDSP OP: Error Occurred\n",__LINE__);
                            eError = OMX_ErrorHardware;
                            goto EXIT;
                        }
                    }
                } else if (pComponentPrivate->curState == OMX_StatePause) {
                    pComponentPrivate->pOutputBufHdrPending[pComponentPrivate->nNumOutputBufPending++] = pBufHeader;
                }
         
        } else {
            if (pComponentPrivate->curState == OMX_StateExecuting) {
                if (!G723_1ENC_IsPending(pComponentPrivate,pBufHeader,OMX_DirOutput)) {
                    G723_1ENC_SetPending(pComponentPrivate,pBufHeader,OMX_DirOutput,__LINE__);
                    G723_1ENC_DPRINT("%d Sending OutputBuffer to SN AllocLen: %d\n", __LINE__,pBufHeader->nAllocLen);
                    eError = LCML_QueueBuffer( pLcmlHandle->pCodecinterfacehandle,
                                               EMMCodecOuputBuffer,
                                               (OMX_U8 *)pBufHeader->pBuffer,
                                               pBufHeader->nAllocLen,
                                               pBufHeader->nFilledLen,
                                               (OMX_U8 *) pLcmlHdr->pOpParam, /*<--According SN this should not be necesary*/
                                               sizeof(G723ENC_ParamStruct),
                                               NULL);
                    if (eError != OMX_ErrorNone ) {
                        G723_1ENC_DPRINT ("%d :: IssuingDSP OP: Error Occurred\n",__LINE__);
                          eError = OMX_ErrorHardware;
                          goto EXIT;
                    }


                }
            } else if (pComponentPrivate->curState == OMX_StatePause) {
                pComponentPrivate->pOutputBufHdrPending[pComponentPrivate->nNumOutputBufPending++] = pBufHeader;
            }
        }
    } else {
        eError = OMX_ErrorBadParameter;
    }

EXIT:
    G723_1ENC_DPRINT("%d :: Exiting from  G723_1ENC_HandleDataBufFromApp \n",__LINE__);
    G723_1ENC_DPRINT("%d :: Returning error %d\n",__LINE__,eError);
    return eError;
}

/*-------------------------------------------------------------------*/
/**
* G723_1ENC_GetBufferDirection () This function is used by the component
* to get the direction of the buffer
* @param eDir pointer will be updated with buffer direction
* @param pBufHeader pointer to the buffer to be requested to be filled
*
* @retval none
**/
/*-------------------------------------------------------------------*/

OMX_ERRORTYPE G723_1ENC_GetBufferDirection(OMX_BUFFERHEADERTYPE *pBufHeader,
                                                         OMX_DIRTYPE *eDir)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate = pBufHeader->pPlatformPrivate;
    OMX_U32 nBuf = 0;
    OMX_BUFFERHEADERTYPE *pBuf = NULL;
    OMX_U32 flag = 1,i = 0;
    G723_1ENC_DPRINT("%d :: Entering G723_1ENC_GetBufferDirection Function\n",__LINE__);
    /*Search this buffer in input buffers list */
    nBuf = pComponentPrivate->pInputBufferList->numBuffers;
    for(i=0; i<nBuf; i++) {
        pBuf = pComponentPrivate->pInputBufferList->pBufHdr[i];
        if(pBufHeader == pBuf) {
            *eDir = OMX_DirInput;
            G723_1ENC_DPRINT("%d :: pBufHeader = %p is INPUT BUFFER pBuf = %p\n",__LINE__,pBufHeader,pBuf);
            flag = 0;
            goto EXIT;
        }
    }
    /*Search this buffer in output buffers list */
    nBuf = pComponentPrivate->pOutputBufferList->numBuffers;
    for(i=0; i<nBuf; i++) {
        pBuf = pComponentPrivate->pOutputBufferList->pBufHdr[i];
        if(pBufHeader == pBuf) {
            *eDir = OMX_DirOutput;
            G723_1ENC_DPRINT("%d :: pBufHeader = %p is OUTPUT BUFFER pBuf = %p\n",__LINE__,pBufHeader,pBuf);
            flag = 0;
            goto EXIT;
        }
    }

    if (flag == 1) {
        G723_1ENC_DPRINT("%d :: Buffer %p is Not Found in the List\n",__LINE__, pBufHeader);
        eError = OMX_ErrorUndefined;
        goto EXIT;
    }
EXIT:
    G723_1ENC_DPRINT("%d :: Exiting G723_1ENC_GetBufferDirection Function\n",__LINE__);
    G723_1ENC_DPRINT("%d :: Returning = 0x%x\n",__LINE__,eError);
    return eError;
}

/* -------------------------------------------------------------------*/
/**
  * G723_1ENC_GetCorrespondingLCMLHeader() function will be called by LCML_Callback
  * component to write the msg
  * @param *pBuffer,          Event which gives to details about USN status
  * @param G723_1ENC_LCML_BUFHEADERTYPE **ppLcmlHdr
  * @param  OMX_DIRTYPE eDir this gives direction of the buffer
  * @retval OMX_NoError              Success, ready to roll
  *         OMX_Error_BadParameter   The input parameter pointer is null
 **/
/* -------------------------------------------------------------------*/
OMX_ERRORTYPE G723_1ENC_GetCorrespondingLCMLHeader(G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate, 
                                                  OMX_U8 *pBuffer,
                                                  OMX_DIRTYPE eDir,
                                                  G723_1ENC_LCML_BUFHEADERTYPE **ppLcmlHdr)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    G723_1ENC_LCML_BUFHEADERTYPE *pLcmlBufHeader = NULL;
    OMX_U32 i = 0,nIpBuf = 0,nOpBuf = 0;
    nIpBuf = pComponentPrivate->pInputBufferList->numBuffers;
    nOpBuf = pComponentPrivate->pOutputBufferList->numBuffers;
    G723_1ENC_DPRINT("%d :: Entering G723_1ENC_GetCorrespondingLCMLHeader..\n",__LINE__);
    while (!pComponentPrivate->bInitParamsInitialized) {
        G723_1ENC_DPRINT("%d :: Waiting for init to complete........\n",__LINE__);
#ifndef UNDER_CE
        sched_yield();
#else
        Sleep(0);
#endif
    }
    if(eDir == OMX_DirInput) {
        G723_1ENC_DPRINT("%d :: Entering G723_1ENC_GetCorrespondingLCMLHeader..\n",__LINE__);
        pLcmlBufHeader = pComponentPrivate->pLcmlBufHeader[G723_1ENC_INPUT_PORT];
        for(i = 0; i < nIpBuf; i++) {
            G723_1ENC_DPRINT("%d :: pBuffer = %p\n",__LINE__,pBuffer);
            G723_1ENC_DPRINT("%d :: pLcmlBufHeader->buffer->pBuffer = %p\n",__LINE__,pLcmlBufHeader->buffer->pBuffer);
            if(pBuffer == pLcmlBufHeader->buffer->pBuffer) {
                *ppLcmlHdr = pLcmlBufHeader;
                 G723_1ENC_DPRINT("%d :: Corresponding Input LCML Header Found = %p\n",__LINE__,pLcmlBufHeader);
                 eError = OMX_ErrorNone;
                 goto EXIT;
            }
            pLcmlBufHeader++;
        }
    } else if (eDir == OMX_DirOutput) {
        G723_1ENC_DPRINT("%d :: Entering G723_1ENC_GetCorrespondingLCMLHeader..\n",__LINE__);
        pLcmlBufHeader = pComponentPrivate->pLcmlBufHeader[G723_1ENC_OUTPUT_PORT];
        for(i = 0; i < nOpBuf; i++) {
            G723_1ENC_DPRINT("%d :: pBuffer = %p\n",__LINE__,pBuffer);
            G723_1ENC_DPRINT("%d :: pLcmlBufHeader->buffer->pBuffer = %p\n",__LINE__,pLcmlBufHeader->buffer->pBuffer);
            if(pBuffer == pLcmlBufHeader->buffer->pBuffer) {
                *ppLcmlHdr = pLcmlBufHeader;
                 G723_1ENC_DPRINT("%d :: Corresponding Output LCML Header Found = %p\n",__LINE__,pLcmlBufHeader);
                 eError = OMX_ErrorNone;
                 goto EXIT;
            }
            pLcmlBufHeader++;
        }
    } else {
      G723_1ENC_DPRINT("%d :: Invalid Buffer Type :: exiting...\n",__LINE__);
      eError = OMX_ErrorUndefined;
    }

EXIT:
    G723_1ENC_DPRINT("%d :: Exiting G723_1ENC_GetCorrespondingLCMLHeader..\n",__LINE__);
    G723_1ENC_DPRINT("%d :: Returning = 0x%x\n",__LINE__,eError);
    return eError;
}

/* -------------------------------------------------------------------*/
/**
  *  G723_1ENC_LCMLCallback() will be called LCML component to write the msg
  *
  * @param event                 Event which gives to details about USN status
  * @param void * args        //    args [0] //bufType;
                              //    args [1] //arm address fpr buffer
                              //    args [2] //BufferSize;
                              //    args [3]  //arm address for param
                              //    args [4] //ParamSize;
                              //    args [6] //LCML Handle
  * @retval OMX_NoError              Success, ready to roll
  *         OMX_Error_BadParameter   The input parameter pointer is null
 **/
/*-------------------------------------------------------------------*/

OMX_ERRORTYPE G723_1ENC_LCMLCallback (TUsnCodecEvent event,void * args[10])
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U8 *pBuffer = args[1];
    G723_1ENC_LCML_BUFHEADERTYPE *pLcmlHdr = NULL;
    G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate = NULL;
    OMX_U8 index=0,nFrames=0;
    OMX_U16 i=0,frameLength=0, length=0;
    OMX_COMPONENTTYPE* pHandle = NULL;
#ifdef RESOURCE_MANAGER_ENABLED 
    OMX_ERRORTYPE rm_error = OMX_ErrorNone;
#endif
 
    G723_1ENC_DPRINT("%d :: Entering the G723_1ENC_LCMLCallback Function\n",__LINE__);
    pComponentPrivate = (G723_1ENC_COMPONENT_PRIVATE*)((LCML_DSP_INTERFACE*)args[6])->pComponentPrivate;
    
    pHandle = pComponentPrivate->pHandle;    

#ifdef G723_1ENC_DEBUG
    switch(event) {

        case EMMCodecDspError:
            G723_1ENC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecDspError\n");
            break;

        case EMMCodecInternalError:
            G723_1ENC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecInternalError\n");
            break;

        case EMMCodecInitError:
            G723_1ENC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecInitError\n");
            break;

        case EMMCodecDspMessageRecieved:
            G723_1ENC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecDspMessageRecieved\n");
            break;

        case EMMCodecBufferProcessed:
            G723_1ENC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecBufferProcessed\n");
            break;

        case EMMCodecProcessingStarted:
            G723_1ENC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecProcessingStarted\n");
            break;

        case EMMCodecProcessingPaused:
            G723_1ENC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecProcessingPaused\n");
            break;

        case EMMCodecProcessingStoped:
            G723_1ENC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecProcessingStoped\n");
            break;

        case EMMCodecProcessingEof:
            G723_1ENC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecProcessingEof\n");
            break;

        case EMMCodecBufferNotProcessed:
            G723_1ENC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecBufferNotProcessed\n");
            break;

        case EMMCodecAlgCtrlAck:
            G723_1ENC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecAlgCtrlAck\n");
            break;

        case EMMCodecStrmCtrlAck:
            G723_1ENC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecStrmCtrlAck\n");
            break;
    }
#endif

    if(event == EMMCodecBufferProcessed)
    {
        if((OMX_U32)args[0] == EMMCodecInputBuffer) {
            G723_1ENC_DPRINT("%d :: INPUT: pBuffer = %p\n",__LINE__, pBuffer);
            G723_1ENC_DPRINT("%d Arriving INPUT Buffer from SN\n",__LINE__);
            eError = G723_1ENC_GetCorrespondingLCMLHeader(pComponentPrivate, pBuffer, OMX_DirInput, &pLcmlHdr);
            if (eError != OMX_ErrorNone) {
                G723_1ENC_DPRINT("%d :: Error: Invalid Buffer Came ...\n",__LINE__);
                goto EXIT;
            }
            G723_1ENC_ClearPending(pComponentPrivate,pLcmlHdr->buffer,OMX_DirInput,__LINE__);
            if(pComponentPrivate->dasfMode == 0) {
                G723_1ENC_DPRINT("%d: Component Sending Empty Input buffer%p to App\n",__LINE__,pLcmlHdr->buffer->pBuffer);
                pComponentPrivate->cbInfo.EmptyBufferDone (
                                       pHandle,
                                       pHandle->pApplicationPrivate,
                                       pLcmlHdr->buffer
                                       );

                pComponentPrivate->nEmptyBufferDoneCount++;
            }
        } else if((OMX_U32)args[0] == EMMCodecOuputBuffer) {
            G723_1ENC_DPRINT("%d :: OUTPUT: pBuffer = %p\n",__LINE__, pBuffer);
            pComponentPrivate->nOutStandingFillDones++;
            eError = G723_1ENC_GetCorrespondingLCMLHeader(pComponentPrivate, pBuffer, OMX_DirOutput, &pLcmlHdr);
            if (eError != OMX_ErrorNone) {
                G723_1ENC_DPRINT("%d :: Error: Invalid Buffer Came ...\n",__LINE__);
                goto EXIT;
            }
            G723_1ENC_DPRINT("%d :: Output: pLcmlHdr->buffer->pBuffer = %p\n",__LINE__, pLcmlHdr->buffer->pBuffer);
            pLcmlHdr->buffer->nFilledLen = (OMX_U32)args[8];
            index = (pLcmlHdr->buffer->pBuffer[0] ) & 0x03;
            if( pLcmlHdr->buffer->nFilledLen ) {
                       nFrames = (OMX_U8)( pLcmlHdr->buffer->nFilledLen / G723_1ENC_OUTPUT_BUFFER_SIZE);
                       frameLength=0;
                       length=0;
                       for(i=0;i<nFrames;i++){
                         index = (pLcmlHdr->buffer->pBuffer[i*G723_1ENC_OUTPUT_BUFFER_SIZE] ) & 0x03;
                         if(pLcmlHdr->buffer->nFilledLen == 0)
                                     length = 0;
                         else
                                 length = (OMX_U16)pComponentPrivate->G723_1FrameSize[index];
                             if (i){
                                   memmove( pLcmlHdr->buffer->pBuffer + frameLength,
                                            pLcmlHdr->buffer->pBuffer + (i * G723_1ENC_OUTPUT_BUFFER_SIZE),
                                            length);
                             }
                             frameLength += length;
                        }
                        pLcmlHdr->buffer->nFilledLen= frameLength;
                }
            G723_1ENC_DPRINT("%d :: Output: pBuffer = %ld\n",__LINE__, pLcmlHdr->buffer->nFilledLen);
            pComponentPrivate->lastOutBufArrived = pLcmlHdr->buffer;
            G723_1ENC_ClearPending(pComponentPrivate,pLcmlHdr->buffer,OMX_DirOutput,__LINE__);
            /* Copying time stamp information to output buffer */
            pLcmlHdr->buffer->nTimeStamp = (OMX_TICKS)pComponentPrivate->arrTimestamp[pComponentPrivate->OpBufindex];
            /* Copying nTickCount information to output buffer */
            pLcmlHdr->buffer->nTickCount = pComponentPrivate->arrTickCount[pComponentPrivate->OpBufindex];
        
            pComponentPrivate->OpBufindex++;
            pComponentPrivate->OpBufindex %= pComponentPrivate->pPortDef[G723_1ENC_OUTPUT_PORT]->nBufferCountActual;

            pComponentPrivate->cbInfo.FillBufferDone( pHandle,
                                                  pHandle->pApplicationPrivate,
                                                  pLcmlHdr->buffer);

            pComponentPrivate->nFillBufferDoneCount++;

            pComponentPrivate->nOutStandingFillDones--;
        }
    }
    else if (event == EMMCodecStrmCtrlAck) {
        G723_1ENC_DPRINT("%d :: GOT MESSAGE USN_DSPACK_STRMCTRL \n",__LINE__);
        if (args[1] == (void *)USN_STRMCMD_FLUSH) {
                 if ( args[2] == (void *)EMMCodecInputBuffer) {
                     if (args[0] == (void *)USN_ERR_NONE ) {
                         G723_1ENC_DPRINT("Flushing input port %d\n",__LINE__);
                       
                         for (i=0; i < pComponentPrivate->nNumInputBufPending; i++) {
                              G723_1ENC_DPRINT("%d :: Calling EmptyBufferDone\n", __LINE__);
                              pComponentPrivate->cbInfo.EmptyBufferDone (
                                       pComponentPrivate->pHandle,
                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                       pComponentPrivate->pInputBufHdrPending[i]);
                              pComponentPrivate->pInputBufHdrPending[i] = NULL;                                       
                              pComponentPrivate->nEmptyBufferDoneCount++;
                              G723_1ENC_DPRINT("%d :: Incrementing nEmptyBufferDoneCount = %ld\n",__LINE__,pComponentPrivate->nEmptyBufferDoneCount);
                         }
                         pComponentPrivate->nNumInputBufPending=0;                         
                         /* return all input buffers */
                         pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                    pHandle->pApplicationPrivate,
                                                    OMX_EventCmdComplete,
                                                    OMX_CommandFlush,
                                                    G723_1ENC_INPUT_PORT,
                                                    NULL);  
                     } else {
                         G723_1ENC_DPRINT ("LCML reported error while flushing input port\n");
                         goto EXIT;                            
                     }
                 }
                 else if ( args[2] == (void *)EMMCodecOuputBuffer) { 
                     if (args[0] == (void *)USN_ERR_NONE ) {                      
                         G723_1ENC_DPRINT("Flushing output port %d\n",__LINE__);

                        for (i=0; i < pComponentPrivate->nNumOutputBufPending; i++) {
                             G723_1ENC_DPRINT("%d :: Calling FillBufferDone\n", __LINE__);
                             pComponentPrivate->cbInfo.FillBufferDone (
                                       pComponentPrivate->pHandle,
                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                       pComponentPrivate->pOutputBufHdrPending[i]);
                             pComponentPrivate->pOutputBufHdrPending[i] = NULL;
                             pComponentPrivate->nFillBufferDoneCount++;
                             G723_1ENC_DPRINT("%d :: Incrementing nFillBufferDoneCount = %ld\n",__LINE__,pComponentPrivate->nFillBufferDoneCount);
                        }
                        pComponentPrivate->nNumOutputBufPending=0;                        
                        /* return all output buffers */
                        pComponentPrivate->cbInfo.EventHandler( pHandle,
                                                    pHandle->pApplicationPrivate,
                                                    OMX_EventCmdComplete,
                                                    OMX_CommandFlush,
                                                    G723_1ENC_OUTPUT_PORT,
                                                    NULL);
                     } else {
                         G723_1ENC_DPRINT ("LCML reported error while flushing output port\n");
                         goto EXIT;                            
                     }
                 }
            }
    }
    else if(event == EMMCodecProcessingStoped) {

        G723_1ENC_DPRINT("%d :: GOT MESSAGE USN_DSPACK_STOP \n",__LINE__);
        if (!pComponentPrivate->bNoIdleOnStop) {
            pComponentPrivate->curState = OMX_StateIdle;
#ifdef RESOURCE_MANAGER_ENABLED
            rm_error = RMProxy_NewSendCommand(pHandle,
                                              RMProxy_StateSet,
                                              OMX_G723_Encoder_COMPONENT,
                                              OMX_StateIdle, 3456,NULL);

#endif
            if (pComponentPrivate->bPreempted == 0) {
                pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                pComponentPrivate->pHandle->pApplicationPrivate,
                                                OMX_EventCmdComplete,
                                                OMX_CommandStateSet,
                                                pComponentPrivate->curState,
                                                NULL);
            } else {
                pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                pComponentPrivate->pHandle->pApplicationPrivate,
                                                OMX_EventError,
                                                OMX_ErrorResourcesPreempted,
                                                0,
                                                NULL);
            }                                                
        }
        else {
            pComponentPrivate->bDspStoppedWhileExecuting = OMX_TRUE;
            pComponentPrivate->bNoIdleOnStop= OMX_FALSE;            
        }
    }
    else if(event == EMMCodecDspMessageRecieved) {
        G723_1ENC_DPRINT("%d :: commandedState  = %ld\n",__LINE__,(OMX_U32)args[0]);
        G723_1ENC_DPRINT("%d :: arg1 = %ld\n",__LINE__,(OMX_U32)args[1]);
        G723_1ENC_DPRINT("%d :: arg2 = %ld\n",__LINE__,(OMX_U32)args[2]);

        if(0x0500 == (OMX_U32)args[2]) {
            G723_1ENC_DPRINT("%d :: EMMCodecDspMessageRecieved\n",__LINE__);
        }
    }
    else if(event == EMMCodecAlgCtrlAck) {
        G723_1ENC_DPRINT("%d :: GOT MESSAGE USN_DSPACK_ALGCTRL \n",__LINE__);
    }
    else if (event == EMMCodecDspError) {
        if(((int)args[4] == USN_ERR_WARNING) && ((int)args[5] == IUALG_WARN_PLAYCOMPLETED)) {
            G723_1ENC_DPRINT("%d :: GOT MESSAGE IUALG_WARN_PLAYCOMPLETED\n",__LINE__);
            if(pComponentPrivate->lastOutBufArrived!=NULL && !pComponentPrivate->dasfMode){
                     pComponentPrivate->lastOutBufArrived->nFlags = OMX_BUFFERFLAG_EOS;
                     pComponentPrivate->LastBufSent=0;
            }
        }
    }
EXIT:
    G723_1ENC_DPRINT("%d :: Exiting the G723_1ENC_LCMLCallback Function\n",__LINE__);
    G723_1ENC_DPRINT("%d :: Returning = 0x%x\n",__LINE__,eError);
    return eError;
}

/* ================================================================================= */
/**
  *  G723_1ENC_GetLCMLHandle()
  *
  * @retval OMX_HANDLETYPE
  */
/* ================================================================================= */
#ifndef UNDER_CE
OMX_HANDLETYPE G723_1ENC_GetLCMLHandle(G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_ERRORTYPE (*fpGetHandle)(OMX_HANDLETYPE);
    OMX_HANDLETYPE pHandle = NULL;
    void *handle = NULL;
    char *error = NULL;
    G723_1ENC_DPRINT("%d :: Entering G723_1ENC_GetLCMLHandle..\n",__LINE__);
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
        G723_1ENC_DPRINT("%d :: OMX_ErrorUndefined...\n",__LINE__);
        pHandle = NULL;
        goto EXIT;
    }
   
    ((LCML_DSP_INTERFACE*)pHandle)->pComponentPrivate = pComponentPrivate;
    
    pComponentPrivate->ptrLibLCML=handle;           /* saving LCML lib pointer  */

EXIT:
    G723_1ENC_DPRINT("%d :: Exiting G723_1ENC_GetLCMLHandle..\n",__LINE__);
    G723_1ENC_DPRINT("%d :: Returning = 0x%x\n",__LINE__,eError);
    return pHandle;
}

#else
/*WINDOWS Explicit dll load procedure*/
OMX_HANDLETYPE G723_1ENC_GetLCMLHandle()
{
    typedef OMX_ERRORTYPE (*LPFNDLLFUNC1)(OMX_HANDLETYPE);
    OMX_HANDLETYPE pHandle = NULL;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    HINSTANCE hDLL;               // Handle to DLL
    LPFNDLLFUNC1 fpGetHandle1;
    hDLL = LoadLibraryEx(TEXT("OAF_BML.dll"), NULL,0);
    if (hDLL == NULL) {
        //fputs(dlerror(), stderr);
        G723_1ENC_DPRINT("BML Load Failed!!!\n");
        return pHandle;
    }
    fpGetHandle1 = (LPFNDLLFUNC1)GetProcAddress(hDLL,TEXT("GetHandle"));
    if (!fpGetHandle1) {
      // handle the error
      FreeLibrary(hDLL);
      return pHandle;
    }
    // call the function
    eError = fpGetHandle1(&pHandle);
    if(eError != OMX_ErrorNone) {
        eError = OMX_ErrorUndefined;
        G723_1ENC_DPRINT("eError != OMX_ErrorNone...\n");
        pHandle = NULL;
        return pHandle;
    }
    return pHandle;
}
#endif

/* ================================================================================= */
/**
* @fn G723_1ENC_SetPending() description for G723_1ENC_SetPending
G723_1ENC_SetPending().
This component is called when a buffer is queued to the LCML
* @param pComponent  handle for this instance of the component
*
* @pre
*
* @post
*
* @return OMX_ERRORTYPE
*/
/* ================================================================================ */
void G723_1ENC_SetPending(G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate,
                         OMX_BUFFERHEADERTYPE *pBufHdr, OMX_DIRTYPE eDir, OMX_U32 lineNumber)
{
    OMX_U16 i = 0;

    if (eDir == OMX_DirInput) {
        for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pInputBufferList->pBufHdr[i]) {
                pComponentPrivate->pInputBufferList->bBufferPending[i] = 1;
                G723_1ENC_DPRINT("****INPUT BUFFER %d IS PENDING Line %ld******\n",i,lineNumber);
            }
        }
    }
    else {
        for (i=0; i < pComponentPrivate->pOutputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pOutputBufferList->pBufHdr[i]) {
                pComponentPrivate->pOutputBufferList->bBufferPending[i] = 1;
                G723_1ENC_DPRINT("****OUTPUT BUFFER %d IS PENDING Line %ld*****\n",i,lineNumber);
            }
        }
    }
}
/* ================================================================================= */
/**
* @fn G723_1ENC_ClearPending() description for G723_1ENC_ClearPending
G723_1ENC_ClearPending().
This component is called when a buffer is returned from the LCML
* @param pComponent  handle for this instance of the component
*
* @pre
*
* @post
*
* @return OMX_ERRORTYPE
*/
/* ================================================================================ */
void G723_1ENC_ClearPending(G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate,
                           OMX_BUFFERHEADERTYPE *pBufHdr, OMX_DIRTYPE eDir, OMX_U32 lineNumber)
{
    OMX_U16 i = 0;

    if (eDir == OMX_DirInput) {
        for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pInputBufferList->pBufHdr[i]) {
                pComponentPrivate->pInputBufferList->bBufferPending[i] = 0;
                G723_1ENC_DPRINT("****INPUT BUFFER %d IS RECLAIMED Line %ld*****\n",i,lineNumber);
            }
        }
    }
    else {
        for (i=0; i < pComponentPrivate->pOutputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pOutputBufferList->pBufHdr[i]) {
                pComponentPrivate->pOutputBufferList->bBufferPending[i] = 0;
                G723_1ENC_DPRINT("****OUTPUT BUFFER %d IS RECLAIMED Line %ld*****\n",i,lineNumber);
            }
        }
    }
}
/* ================================================================================= */
/**
* @fn G723_1ENC_IsPending() description for G723_1ENC_IsPending
G723_1ENC_IsPending().
This method returns the pending status to the buffer
* @param pComponent  handle for this instance of the component
*
* @pre
*
* @post
*
* @return OMX_ERRORTYPE
*/
/* ================================================================================ */
OMX_U32 G723_1ENC_IsPending(G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate,
                           OMX_BUFFERHEADERTYPE *pBufHdr, OMX_DIRTYPE eDir)
{
    OMX_U16 i = 0;

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
/* ================================================================================= */
/**
* @fn G723_1ENC_IsValid() description for G723_1ENC_IsValid
G723_1ENC_IsValid().
This method checks to see if a buffer returned from the LCML is valid.
* @param pComponent  handle for this instance of the component
*
* @pre
*
* @post
*
* @return OMX_ERRORTYPE
*/
/* ================================================================================ */
OMX_U32 G723_1ENC_IsValid(G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate,
                         OMX_U8 *pBuffer, OMX_DIRTYPE eDir)
{
    OMX_U16 i = 0;
    OMX_U32 found=0;

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
* @G723_1ENC_FillLCMLInitParamsEx() This function is used by the component thread to
* fill the all of its initialization parameters, buffer deatils  etc
* to LCML structure,
*
* @param pComponent  handle for this instance of the component
* @param plcml_Init  pointer to LCML structure to be filled
*
* @pre
*
* @post
*
* @return none
*/
/* ========================================================================== */
OMX_ERRORTYPE G723_1ENC_FillLCMLInitParamsEx(OMX_HANDLETYPE pComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 nIpBuf = 0,nIpBufSize = 0,nOpBuf = 0,nOpBufSize = 0;
    OMX_BUFFERHEADERTYPE *pTemp = NULL;
    LCML_DSP_INTERFACE *pHandle = (LCML_DSP_INTERFACE *)pComponent;
    G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate = pHandle->pComponentPrivate;
    G723_1ENC_LCML_BUFHEADERTYPE *pTemp_lcml = NULL;
    OMX_U32 i = 0;
    OMX_U32 size_lcml = 0;
    G723_1ENC_DPRINT("%d :: G723_1ENC_FillLCMLInitParamsEx\n",__LINE__);
    nIpBuf = pComponentPrivate->pInputBufferList->numBuffers;
    nIpBufSize = pComponentPrivate->pPortDef[G723_1ENC_INPUT_PORT]->nBufferSize;
    nOpBuf = pComponentPrivate->pOutputBufferList->numBuffers;
    nOpBufSize = pComponentPrivate->pPortDef[G723_1ENC_OUTPUT_PORT]->nBufferSize;
    
    pComponentPrivate->nRuntimeInputBuffers = nIpBuf;
    pComponentPrivate->nRuntimeOutputBuffers = nOpBuf;
    
    G723_1ENC_DPRINT("%d :: ------ Buffer Details -----------\n",__LINE__);
    G723_1ENC_DPRINT("%d :: Input  Buffer Count = %ld\n",__LINE__,nIpBuf);
    G723_1ENC_DPRINT("%d :: Input  Buffer Size = %ld\n",__LINE__,nIpBufSize);
    G723_1ENC_DPRINT("%d :: Output Buffer Count = %ld\n",__LINE__,nOpBuf);
    G723_1ENC_DPRINT("%d :: Output Buffer Size = %ld\n",__LINE__,nOpBufSize);
    G723_1ENC_DPRINT("%d :: ------ Buffer Details ------------\n",__LINE__);
    /* Allocate memory for all input buffer headers..
     * This memory pointer will be sent to LCML */
    size_lcml = nIpBuf * sizeof(G723_1ENC_LCML_BUFHEADERTYPE);
    pTemp_lcml = (G723_1ENC_LCML_BUFHEADERTYPE *)SafeMalloc(size_lcml);
    G723_1ENC_MEMPRINT("%d :: [ALLOC] %p\n",__LINE__,pTemp_lcml);
    if(pTemp_lcml == NULL) {
        G723_1ENC_DPRINT("%d :: Memory Allocation Failed\n",__LINE__);
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    pComponentPrivate->pLcmlBufHeader[G723_1ENC_INPUT_PORT] = pTemp_lcml;
    for (i=0; i<nIpBuf; i++) {
        G723_1ENC_DPRINT("%d :: INPUT--------- Inside Ip Loop\n",__LINE__);
        pTemp = pComponentPrivate->pInputBufferList->pBufHdr[i];
        pTemp->nSize = sizeof(OMX_BUFFERHEADERTYPE);
        pTemp->nAllocLen = nIpBufSize;
        pTemp->nFilledLen = nIpBufSize;
        pTemp->nVersion.s.nVersionMajor = G723_1ENC_MAJOR_VER;
        pTemp->nVersion.s.nVersionMinor = G723_1ENC_MINOR_VER;
        pTemp->pPlatformPrivate = pHandle->pComponentPrivate;
        pTemp->nTickCount = G723_1ENC_NOT_USED;
        pTemp_lcml->buffer = pTemp;
        G723_1ENC_DPRINT("%d :: pTemp_lcml->buffer->pBuffer = %p \n",__LINE__,pTemp_lcml->buffer->pBuffer);
        pTemp_lcml->eDir = OMX_DirInput;
        OMX_NBMALLOC_STRUCT(pTemp_lcml->pIpParam, G723ENC_ParamStruct);
        pTemp_lcml->pDmmBuf = (DMM_BUFFER_OBJ*) SafeMalloc(sizeof(DMM_BUFFER_OBJ));

        pTemp_lcml->pFrameParam = NULL;
        /*pTemp_lcml->pIpParam->usLastFrame = 0;*/
        /* This means, it is not a last buffer. This flag is to be modified by
         * the application to indicate the last buffer */
        pTemp->nFlags = G723_1ENC_NORMAL_BUFFER;
        pTemp++;
        pTemp_lcml++;
    }

    /* Allocate memory for all output buffer headers..
     * This memory pointer will be sent to LCML */
    size_lcml = nOpBuf * sizeof(G723_1ENC_LCML_BUFHEADERTYPE);
    pTemp_lcml = (G723_1ENC_LCML_BUFHEADERTYPE *)SafeMalloc(size_lcml);
    G723_1ENC_MEMPRINT("%d :: [ALLOC] %p\n",__LINE__,pTemp_lcml);
    if(pTemp_lcml == NULL) {
        G723_1ENC_DPRINT("%d :: Memory Allocation Failed\n",__LINE__);
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    pComponentPrivate->pLcmlBufHeader[G723_1ENC_OUTPUT_PORT] = pTemp_lcml;
    for (i=0; i<nOpBuf; i++) {
        G723_1ENC_DPRINT("%d :: OUTPUT--------- Inside Op Loop\n",__LINE__);
        pTemp = pComponentPrivate->pOutputBufferList->pBufHdr[i];
        pTemp->nSize = sizeof(OMX_BUFFERHEADERTYPE);
        pTemp->nAllocLen = nOpBufSize;
        pTemp->nFilledLen = nOpBufSize;
        pTemp->nVersion.s.nVersionMajor = G723_1ENC_MAJOR_VER;
        pTemp->nVersion.s.nVersionMinor = G723_1ENC_MINOR_VER;
        pComponentPrivate->nVersion = pTemp->nVersion.nVersion;
        pTemp->pPlatformPrivate = pHandle->pComponentPrivate;
        pTemp->nTickCount = G723_1ENC_NOT_USED;
        pTemp_lcml->buffer = pTemp;
        G723_1ENC_DPRINT("%d :: pTemp_lcml->buffer->pBuffer = %p \n",__LINE__,pTemp_lcml->buffer->pBuffer);
        pTemp_lcml->eDir = OMX_DirOutput;
        OMX_NBMALLOC_STRUCT(pTemp_lcml->pOpParam, G723ENC_ParamStruct);/*According SN guide this variable should be neede*/
        memset(pTemp_lcml->pOpParam,0,sizeof(G723ENC_ParamStruct));
        pTemp_lcml->pOpParam->usNbFrames = 0;
        /* This means, it is not a last buffer. This flag is to be modified by
         * the application to indicate the last buffer */
        pTemp->nFlags = G723_1ENC_NORMAL_BUFFER;
        pTemp++;
        pTemp_lcml++;
    }
    pComponentPrivate->bPortDefsAllocated = 1;
    pComponentPrivate->bInitParamsInitialized = 1;
EXIT:
    G723_1ENC_DPRINT("%d :: Exiting G723_1ENC_FillLCMLInitParamsEx\n",__LINE__);
    G723_1ENC_DPRINT("%d :: Returning = 0x%x\n",__LINE__,eError);
    return eError;
}

/** ========================================================================
*  OMX_DmmMap () method is used to allocate the memory using DMM.
*
*  @param ProcHandle -  Component identification number
*  @param size  - Buffer header address, that needs to be sent to codec
*  @param pArmPtr - Message used to send the buffer to codec
*  @param pDmmBuf - buffer id
*
*  @retval OMX_ErrorNone  - Success
*          OMX_ErrorHardware  -  Hardware Error
** ==========================================================================*/
OMX_ERRORTYPE OMX_DmmMap(DSP_HPROCESSOR ProcHandle,
                     int size,
                     void* pArmPtr,
                     DMM_BUFFER_OBJ* pDmmBuf)
{
    OMX_ERRORTYPE eError = OMX_ErrorUndefined;
    DSP_STATUS status = DSP_SOK;
    int nSizeReserved = 0;

    if(pDmmBuf == NULL)
    {
        G723_1ENC_DPRINT("pBuf is NULL %d\n",__LINE__);
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }

    if(pArmPtr == NULL)
    {
        G723_1ENC_DPRINT("pBuf is NULL %d\n",__LINE__);
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }
    if(ProcHandle == NULL)
    {
        G723_1ENC_DPRINT("ProcHandle is NULL %d\n",__LINE__);
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }

    /* Allocate */
    pDmmBuf->pAllocated = pArmPtr;

    /* Reserve */
    nSizeReserved = ROUND_TO_PAGESIZE(size) + 2*DMM_PAGE_SIZE ;


    status = DSPProcessor_ReserveMemory(ProcHandle, nSizeReserved, &(pDmmBuf->pReserved));

    if(DSP_FAILED(status))
    {
        G723_1ENC_DPRINT("DSPProcessor_ReserveMemory() failed - error 0x%d  line: %d\n", (int)status,__LINE__);
        eError = OMX_ErrorHardware;
        goto EXIT;
    }
    pDmmBuf->nSize = size;
    G723_1ENC_DPRINT(" DMM MAP Reserved: %p, size 0x%x (%d)\n", pDmmBuf->pReserved,nSizeReserved,nSizeReserved);

    /* Map */
    status = DSPProcessor_Map(ProcHandle,
                              pDmmBuf->pAllocated,/* Arm addres of data to Map on DSP*/
                              size , /* size to Map on DSP*/
                              pDmmBuf->pReserved, /* reserved space */
                              &(pDmmBuf->pMapped), /* returned map pointer */
                              0); /* final param is reserved.  set to zero. */
    if(DSP_FAILED(status))
    {
        G723_1ENC_DPRINT("DSPProcessor_Map() failed - error 0x%x line: %d\n", (int)status, __LINE__);
        eError = OMX_ErrorHardware;
        goto EXIT;
    }
    G723_1ENC_DPRINT("DMM Mapped: %p, size 0x%x (%d) line: %d\n",pDmmBuf->pMapped, size,size,__LINE__);
     
    /* Issue an initial memory flush to ensure cache coherency */
    status = DSPProcessor_FlushMemory(ProcHandle, pDmmBuf->pAllocated, size, 0);
    if(DSP_FAILED(status))
    {
        G723_1ENC_DPRINT("Unable to flush mapped buffer: error 0x%x   line: %d\n",(int)status,__LINE__);
        goto EXIT;
    }
    eError = OMX_ErrorNone;

EXIT:
   return eError;
}

/** ========================================================================
*  OMX_DmmUnMap () method is used to de-allocate the memory using DMM.
*
*  @param ProcHandle -  Component identification number
*  @param pMapPtr  - Map address
*  @param pResPtr - reserve adress
*
*  @retval OMX_ErrorNone  - Success
*          OMX_ErrorHardware  -  Hardware Error
** ==========================================================================*/
OMX_ERRORTYPE OMX_DmmUnMap(DSP_HPROCESSOR ProcHandle, void* pMapPtr, void* pResPtr)
{
    DSP_STATUS status = DSP_SOK;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    G723_1ENC_DPRINT("OMX UnReserve DSP: %p\n",pResPtr);
    if(pMapPtr == NULL)
    {
        G723_1ENC_DPRINT("pMapPtr is NULL\n");
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }
    if(pResPtr == NULL)
    {
        G723_1ENC_DPRINT("pResPtr is NULL\n");
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }
    if(ProcHandle == NULL)
    {
        G723_1ENC_DPRINT("--ProcHandle is NULL\n");
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }
    G723_1ENC_DPRINT("%d Antes del Processor_UnMap %d\n",__LINE__);
    status = DSPProcessor_UnMap(ProcHandle,pMapPtr);
    if(DSP_FAILED(status))
    {
        G723_1ENC_DPRINT("DSPProcessor_UnMap() failed - error 0x%x",(int)status);
    }

    G723_1ENC_DPRINT("%d unreserving  structure =0x%p\n",pResPtr ,__LINE__);
    status = DSPProcessor_UnReserveMemory(ProcHandle,pResPtr);
    if(DSP_FAILED(status))
    {
        G723_1ENC_DPRINT("DSPProcessor_UnReserveMemory() failed - error 0x%x", (int)status);
    }

EXIT:
    return eError;
}

#ifdef RESOURCE_MANAGER_ENABLED
void G723_ResourceManagerCallback(RMPROXY_COMMANDDATATYPE cbData)
{
    OMX_COMMANDTYPE Cmd = OMX_CommandStateSet;
    OMX_STATETYPE state = OMX_StateIdle;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)cbData.hComponent;
    G723_1ENC_COMPONENT_PRIVATE *pCompPrivate = NULL;

    pCompPrivate = (G723_1ENC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;

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
#endif

