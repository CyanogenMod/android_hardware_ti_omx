
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
* @file OMX_PcmDecoder.c
*
* This file implements OpenMAX (TM) 1.0 Specific APIs and its functionality
* that is fully compliant with the Khronos OpenMAX (TM) 1.0 Specification
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

#else
#include <wchar.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <errno.h>
#endif

#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <dbapi.h>

/*------- Program Header Files -----------------------------------------------*/

#include "LCML_DspCodec.h"

#include "OMX_PcmDec_Utils.h"
#include "TIDspOmx.h"

#ifdef DSP_RENDERING_ON
#include <AudioManagerAPI.h>
#endif

#ifdef RESOURCE_MANAGER_ENABLED
#include <ResourceManagerProxyAPI.h>
#define NEWSENDCOMMAND_MEMORY 123
#endif




#ifdef DSP_RENDERING_ON

#define FIFO1 "/dev/fifo.1"
#define FIFO2 "/dev/fifo.2"
#define PERMS 0666

AM_COMMANDDATATYPE cmd_data;
int pcmd_fdwrite, pcmd_fdread;
int errno;
#endif

#define PCM_DEC_ROLE "audio_renderer.pcm"





/****************************************************************
*  EXTERNAL REFERENCES NOTE : only use if not found in header file
****************************************************************/
/*--------data declarations -----------------------------------*/

/*--------function prototypes ---------------------------------*/

/****************************************************************
*  PUBLIC DECLARATIONS Defined here, used elsewhere
****************************************************************/
/*--------data declarations -----------------------------------*/

/*--------function prototypes ---------------------------------*/

/****************************************************************
*  PRIVATE DECLARATIONS Defined here, used only here
****************************************************************/
/*--------data declarations -----------------------------------*/

/*--------function prototypes ---------------------------------*/

static OMX_ERRORTYPE SetCallbacks (OMX_HANDLETYPE hComp,
                                   OMX_CALLBACKTYPE* pCallBacks, OMX_PTR pAppData);
static OMX_ERRORTYPE GetComponentVersion (OMX_HANDLETYPE hComp,
                                          OMX_STRING pComponentName,
                                          OMX_VERSIONTYPE* pComponentVersion,
                                          OMX_VERSIONTYPE* pSpecVersion,
                                          OMX_UUIDTYPE* pComponentUUID);

static OMX_ERRORTYPE SendCommand (OMX_HANDLETYPE hComp, OMX_COMMANDTYPE nCommand,
                                  OMX_U32 nParam, OMX_PTR pCmdData);

static OMX_ERRORTYPE GetParameter(OMX_HANDLETYPE hComp, OMX_INDEXTYPE nParamIndex,
                                  OMX_PTR ComponentParamStruct);
static OMX_ERRORTYPE SetParameter (OMX_HANDLETYPE hComp,
                                   OMX_INDEXTYPE nParamIndex,
                                   OMX_PTR ComponentParamStruct);
static OMX_ERRORTYPE GetConfig (OMX_HANDLETYPE hComp,
                                OMX_INDEXTYPE nConfigIndex,
                                OMX_PTR pComponentConfigStructure);
static OMX_ERRORTYPE SetConfig (OMX_HANDLETYPE hComp,
                                OMX_INDEXTYPE nConfigIndex,
                                OMX_PTR pComponentConfigStructure);

static OMX_ERRORTYPE EmptyThisBuffer (OMX_HANDLETYPE hComp, OMX_BUFFERHEADERTYPE* pBuffer);

static OMX_ERRORTYPE FillThisBuffer (OMX_HANDLETYPE hComp, OMX_BUFFERHEADERTYPE* pBuffer);
static OMX_ERRORTYPE GetState (OMX_HANDLETYPE hComp, OMX_STATETYPE* pState);
static OMX_ERRORTYPE ComponentTunnelRequest (OMX_HANDLETYPE hComp,
                                             OMX_U32 nPort, OMX_HANDLETYPE hTunneledComp,
                                             OMX_U32 nTunneledPort,
                                             OMX_TUNNELSETUPTYPE* pTunnelSetup);

static OMX_ERRORTYPE ComponentDeInit(OMX_HANDLETYPE pHandle);

static OMX_ERRORTYPE AllocateBuffer (OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
                                     OMX_IN OMX_U32 nPortIndex,
                                     OMX_IN OMX_PTR pAppPrivate,
                                     OMX_IN OMX_U32 nSizeBytes);

static OMX_ERRORTYPE FreeBuffer(
                                OMX_IN  OMX_HANDLETYPE hComponent,
                                OMX_IN  OMX_U32 nPortIndex,
                                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

static OMX_ERRORTYPE UseBuffer (
                                OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                OMX_IN OMX_U32 nPortIndex,
                                OMX_IN OMX_PTR pAppPrivate,
                                OMX_IN OMX_U32 nSizeBytes,
                                OMX_IN OMX_U8* pBuffer);
            
static OMX_ERRORTYPE GetExtensionIndex(
                                       OMX_IN  OMX_HANDLETYPE hComponent,
                                       OMX_IN  OMX_STRING cParameterName,
                                       OMX_OUT OMX_INDEXTYPE* pIndexType);            

static OMX_ERRORTYPE ComponentRoleEnum(
                                       OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_OUT OMX_U8 *cRole,
                                       OMX_IN OMX_U32 nIndex);

/* ================================================================================= * */
/**
* @fn OMX_ComponentInit() function is called by OMX Core to initialize the component
* with default values of the component. Before calling this function OMX_Init
* must have been called.
*
* @param *hComp This is component handle allocated by the OMX core. 
*
* @pre          OMX_Init should be called by application.
*
* @post         Component has initialzed with default values.
*
*  @return      OMX_ErrorNone = Successful Inirialization of the component\n
*               OMX_ErrorInsufficientResources = Not enough memory
*
*  @see          PcmDec_StartCompThread()
*/
/* ================================================================================ * */
OMX_ERRORTYPE OMX_ComponentInit (OMX_HANDLETYPE hComp)
{

    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE*) hComp;
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDef_ip = NULL, *pPortDef_op = NULL;
    OMX_AUDIO_PARAM_PORTFORMATTYPE *pPortFormat = NULL;
    OMX_AUDIO_PARAM_PCMMODETYPE   *pcm_ip = NULL, *pcm_op = NULL;
    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate = NULL;
    PCMD_AUDIODEC_PORT_TYPE *pCompPort = NULL;
    PCMD_BUFFERLIST *pTemp = NULL;
    int i=0;

    PCMDEC_DPRINT ("Entering OMX_ComponentInit\n");

    PCMD_OMX_CONF_CHECK_CMD(pHandle,1,1)

    pHandle->SetCallbacks = SetCallbacks;
    pHandle->GetComponentVersion = GetComponentVersion;
    pHandle->SendCommand = SendCommand;
    pHandle->GetParameter = GetParameter;
    pHandle->SetParameter = SetParameter;
    pHandle->GetConfig = GetConfig;
    pHandle->SetConfig = SetConfig;
    pHandle->GetState = GetState;
    pHandle->EmptyThisBuffer = EmptyThisBuffer;
    pHandle->FillThisBuffer = FillThisBuffer;
    pHandle->ComponentTunnelRequest = ComponentTunnelRequest;
    pHandle->ComponentDeInit = ComponentDeInit;
    pHandle->AllocateBuffer =  AllocateBuffer;
    pHandle->FreeBuffer = FreeBuffer;
    pHandle->UseBuffer = UseBuffer;
    pHandle->GetExtensionIndex = GetExtensionIndex;
    pHandle->ComponentRoleEnum = ComponentRoleEnum;

    PCMD_OMX_MALLOC(pHandle->pComponentPrivate,PCMDEC_COMPONENT_PRIVATE);

    pComponentPrivate = pHandle->pComponentPrivate;
    pComponentPrivate->pHandle = pHandle;
    
#ifdef __PERF_INSTRUMENTATION__
    pComponentPrivate->pPERF = PERF_Create(PERF_FOURCC('P','C','D',' '),
                                           PERF_ModuleLLMM |
                                           PERF_ModuleAudioDecode);
#endif

    PCMD_OMX_MALLOC(pCompPort, PCMD_AUDIODEC_PORT_TYPE);
    pComponentPrivate->pCompPort[PCMD_INPUT_PORT] =  pCompPort;

    PCMD_OMX_MALLOC(pCompPort, PCMD_AUDIODEC_PORT_TYPE);
    pComponentPrivate->pCompPort[PCMD_OUTPUT_PORT] = pCompPort;
    PCMD_OMX_MALLOC(pTemp, PCMD_BUFFERLIST);
    pComponentPrivate->pInputBufferList = pTemp;

    PCMD_OMX_MALLOC(pTemp, PCMD_BUFFERLIST);
    pComponentPrivate->pOutputBufferList = pTemp;

    pComponentPrivate->pInputBufferList->numBuffers = 0;
    pComponentPrivate->pOutputBufferList->numBuffers = 0;

    for (i=0; i < MAX_NUM_OF_BUFS; i++) {
        pComponentPrivate->pInputBufferList->pBufHdr[i] = NULL;
        pComponentPrivate->pOutputBufferList->pBufHdr[i] = NULL;
    }

    pComponentPrivate->bufAlloced = 0;

    PCMD_OMX_MALLOC(pComponentPrivate->sPortParam, OMX_PORT_PARAM_TYPE);
    OMX_CONF_INIT_STRUCT(pComponentPrivate->sPortParam, OMX_PORT_PARAM_TYPE);
    PCMD_OMX_MALLOC(pComponentPrivate->pPriorityMgmt, OMX_PRIORITYMGMTTYPE);
    OMX_CONF_INIT_STRUCT(pComponentPrivate->pPriorityMgmt, OMX_PRIORITYMGMTTYPE);
    pComponentPrivate->sPortParam->nPorts = 0x2;
    pComponentPrivate->sPortParam->nStartPortNumber = 0x0;
    pComponentPrivate->pcmParams[PCMD_INPUT_PORT] = NULL;
    pComponentPrivate->pcmParams[PCMD_OUTPUT_PORT] = NULL;
    
    pComponentPrivate->sMuteType.bMute = FALSE;
    pComponentPrivate->sVolumeType.bLinear = OMX_FALSE;    

    pComponentPrivate->bIsStopping = 0;

    PCMD_OMX_MALLOC(pcm_ip, OMX_AUDIO_PARAM_PCMMODETYPE);
    PCMD_OMX_MALLOC(pcm_op, OMX_AUDIO_PARAM_PCMMODETYPE);

    pComponentPrivate->pcmParams[PCMD_INPUT_PORT] = pcm_ip;
    pComponentPrivate->pcmParams[PCMD_OUTPUT_PORT] = pcm_op;

    pComponentPrivate->bLowLatencyDisabled = 0;
    pComponentPrivate->dasfmode = 0;
    pComponentPrivate->bitMode = 16;
    
    pComponentPrivate->bBufferIsAllocated = 0;
    pComponentPrivate->bPortDefsAllocated = 0;
    pComponentPrivate->bCompThreadStarted = 0;
    pComponentPrivate->bExitCompThrd = 0;

    pComponentPrivate->parameteric_stereo = 0;
    pComponentPrivate->bInitParamsInitialized = 0;
    pComponentPrivate->pMarkBuf = NULL;
    pComponentPrivate->pMarkData = NULL;
    pComponentPrivate->nEmptyBufferDoneCount = 0;
    pComponentPrivate->nEmptyThisBufferCount = 0;
    pComponentPrivate->nFillBufferDoneCount = 0;
    pComponentPrivate->nFillThisBufferCount = 0;
    pComponentPrivate->strmAttr = NULL;
    pComponentPrivate->bDisableCommandParam = 0;
    pComponentPrivate->bEnableCommandParam = 0;
    pComponentPrivate->bIsInvalidState = OMX_FALSE;      /*new change 7/11/07   */
    pComponentPrivate->ptrLibLCML                   = NULL;

    for (i=0; i < MAX_NUM_OF_BUFS; i++) {
        pComponentPrivate->pInputBufHdrPending[i] = NULL;
        pComponentPrivate->pOutputBufHdrPending[i] = NULL;
        pComponentPrivate->arrTickCount[i] = 0;
        pComponentPrivate->arrBufIndex[i] = 0;
    }

    pComponentPrivate->IpBufindex = 0;
    pComponentPrivate->OpBufindex = 0;
    
    
    pComponentPrivate->nInvalidFrameCount = 0;
    pComponentPrivate->bDisableCommandPending = 0;
    pComponentPrivate->bEnableCommandPending = 0;
    
    pComponentPrivate->numPendingBuffers = 0;
    pComponentPrivate->bNoIdleOnStop= OMX_FALSE;
    pComponentPrivate->bDspStoppedWhileExecuting = OMX_FALSE;
    pComponentPrivate->bIdleCommandPending = OMX_FALSE;
    pComponentPrivate->nOutStandingFillDones = 0;
    pComponentPrivate->nOutStandingEmptyDones = 0;

    
    pComponentPrivate->sDeviceString = malloc(100*sizeof(OMX_STRING));

    /* Initialize device string to the default value */
    strcpy((char*)pComponentPrivate->sDeviceString,"/eteedn:i0:o0/codec\0");


    pComponentPrivate->sBufferSupplier.nSize = sizeof (OMX_PARAM_BUFFERSUPPLIERTYPE);
    pComponentPrivate->sBufferSupplier.nPortIndex = 0;
    pComponentPrivate->sBufferSupplier.eBufferSupplier = OMX_BufferSupplyInput;

    strcpy((char*)pComponentPrivate->componentRole.cRole, PCM_DEC_ROLE);
#ifndef UNDER_CE
    pthread_mutex_init(&pComponentPrivate->AlloBuf_mutex, NULL);
    pthread_cond_init (&pComponentPrivate->AlloBuf_threshold, NULL);
    pComponentPrivate->AlloBuf_waitingsignal = 0;
            
    pthread_mutex_init(&pComponentPrivate->InLoaded_mutex, NULL);
    pthread_cond_init (&pComponentPrivate->InLoaded_threshold, NULL);
    pComponentPrivate->InLoaded_readytoidle = 0;
    
    pthread_mutex_init(&pComponentPrivate->InIdle_mutex, NULL);
    pthread_cond_init (&pComponentPrivate->InIdle_threshold, NULL);
    pComponentPrivate->InIdle_goingtoloaded = 0;
#else
    OMX_CreateEvent(&(pComponentPrivate->AlloBuf_event));
    pComponentPrivate->AlloBuf_waitingsignal = 0;
    
    OMX_CreateEvent(&(pComponentPrivate->InLoaded_event));
    pComponentPrivate->InLoaded_readytoidle = 0;
    
    OMX_CreateEvent(&(pComponentPrivate->InIdle_event));
    pComponentPrivate->InIdle_goingtoloaded = 0;
#endif
    
    PCMD_OMX_MALLOC(pPortDef_ip, OMX_PARAM_PORTDEFINITIONTYPE);
    PCMD_OMX_MALLOC(pPortDef_op, OMX_PARAM_PORTDEFINITIONTYPE);

    pComponentPrivate->pPortDef[PCMD_INPUT_PORT] = pPortDef_ip;
    pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT] = pPortDef_op;

    /* Set input port defaults */
    pPortDef_ip->nSize                              = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    pPortDef_ip->nPortIndex                         = PCMD_INPUT_PORT;
    pPortDef_ip->eDir                               = OMX_DirInput;
    pPortDef_ip->nBufferCountActual                 = PCMD_NUM_INPUT_BUFFERS;
    pPortDef_ip->nBufferCountMin                    = PCMD_NUM_INPUT_BUFFERS;
    pPortDef_ip->nBufferSize                        = PCMD_INPUT_BUFFER_SIZE;
    pPortDef_ip->bEnabled                           = OMX_TRUE;
    pPortDef_ip->bPopulated                         = OMX_FALSE;
    pPortDef_ip->eDomain                            = OMX_PortDomainAudio;
    pPortDef_ip->format.audio.eEncoding             = OMX_AUDIO_CodingPCM;  
    pPortDef_ip->format.audio.cMIMEType             = NULL;
    pPortDef_ip->format.audio.pNativeRender         = NULL;
    pPortDef_ip->format.audio.bFlagErrorConcealment = OMX_FALSE;

    /* Set input port defaults */
    pPortDef_op->nSize                              = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    pPortDef_op->nPortIndex                         = PCMD_OUTPUT_PORT;
    pPortDef_op->eDir                               = OMX_DirOutput;
    pPortDef_op->nBufferCountMin                    = PCMD_NUM_OUTPUT_BUFFERS;
    pPortDef_op->nBufferCountActual                 = PCMD_NUM_OUTPUT_BUFFERS;
    pPortDef_op->nBufferSize                        = PCMD_OUTPUT_BUFFER_SIZE;
    pPortDef_op->bEnabled                           = OMX_TRUE;
    pPortDef_op->bPopulated                         = OMX_FALSE;
    pPortDef_op->eDomain                            = OMX_PortDomainAudio;
    pPortDef_op->format.audio.eEncoding             = OMX_AUDIO_CodingPCM;
    pPortDef_op->format.audio.cMIMEType             = NULL;
    pPortDef_op->format.audio.pNativeRender         = NULL;
    pPortDef_op->format.audio.bFlagErrorConcealment = OMX_FALSE;

    PCMD_OMX_MALLOC(pComponentPrivate->pCompPort[PCMD_INPUT_PORT]->pPortFormat, OMX_AUDIO_PARAM_PORTFORMATTYPE);
    PCMD_OMX_MALLOC(pComponentPrivate->pCompPort[PCMD_OUTPUT_PORT]->pPortFormat, OMX_AUDIO_PARAM_PORTFORMATTYPE);
    OMX_CONF_INIT_STRUCT(pComponentPrivate->pCompPort[PCMD_INPUT_PORT]->pPortFormat, OMX_AUDIO_PARAM_PORTFORMATTYPE);
    OMX_CONF_INIT_STRUCT(pComponentPrivate->pCompPort[PCMD_OUTPUT_PORT]->pPortFormat, OMX_AUDIO_PARAM_PORTFORMATTYPE);


    /* Set input port format defaults */
    pPortFormat = pComponentPrivate->pCompPort[PCMD_INPUT_PORT]->pPortFormat;
    OMX_CONF_INIT_STRUCT(pPortFormat, OMX_AUDIO_PARAM_PORTFORMATTYPE);
    pPortFormat->nPortIndex         = PCMD_INPUT_PORT;
    pPortFormat->nIndex             = OMX_IndexParamAudioPcm;
    pPortFormat->eEncoding          = OMX_AUDIO_CodingPCM;

    /* Set output port format defaults */
    pPortFormat = pComponentPrivate->pCompPort[PCMD_OUTPUT_PORT]->pPortFormat;
    OMX_CONF_INIT_STRUCT(pPortFormat, OMX_AUDIO_PARAM_PORTFORMATTYPE);
    pPortFormat->nPortIndex         = PCMD_OUTPUT_PORT;
    pPortFormat->nIndex             = OMX_IndexParamAudioMp3;
    pPortFormat->eEncoding          = OMX_AUDIO_CodingPCM;
    pcm_ip = pComponentPrivate->pcmParams[PCMD_INPUT_PORT];
    OMX_CONF_INIT_STRUCT(pcm_ip, OMX_AUDIO_PARAM_PCMMODETYPE);
    pcm_ip->nPortIndex = PCMD_INPUT_PORT;
    pcm_ip->nChannels = 2; 
    pcm_ip->eNumData= OMX_NumericalDataSigned; 
    pcm_ip->nBitPerSample = 16;  
    pcm_ip->nSamplingRate = 44100;          
    pcm_ip->ePCMMode = OMX_AUDIO_PCMModeLinear;
    pcm_ip->eChannelMapping[0] = OMX_AUDIO_ChannelLF;
    pcm_ip->eChannelMapping[1] = OMX_AUDIO_ChannelRF;    
    pcm_ip->bInterleaved = OMX_TRUE;
    
    pcm_op = pComponentPrivate->pcmParams[PCMD_OUTPUT_PORT];
    OMX_CONF_INIT_STRUCT(pcm_op, OMX_AUDIO_PARAM_PCMMODETYPE);
    pcm_op->nPortIndex = PCMD_OUTPUT_PORT;


#ifdef DSP_RENDERING_ON
    if((pcmd_fdwrite=open(FIFO1,O_WRONLY))<0) {
        PCMDEC_EPRINT("[PCM Component] - failure to open WRITE pipe\n");
        eError = OMX_ErrorHardware;
    }

    if((pcmd_fdread=open(FIFO2,O_RDONLY))<0) {
        PCMDEC_EPRINT("[PCM Component] - failure to open READ pipe\n");
        eError = OMX_ErrorHardware;
    }
#endif

#ifdef RESOURCE_MANAGER_ENABLED
    eError = RMProxy_NewInitalize();
    if (eError != OMX_ErrorNone) {
        PCMDEC_DPRINT ("Error returned from loading ResourceManagerProxy thread\n");
        goto EXIT;
    }
#endif

    eError = PcmDec_StartCompThread(pHandle);
    if (eError != OMX_ErrorNone) {
        PCMDEC_EPRINT ("Error returned from the Component\n");
        goto EXIT;
    }

    /*    eError = OMX_ErrorNone; */

#ifdef __PERF_INSTRUMENTATION__
    PERF_ThreadCreated(pComponentPrivate->pPERF, pComponentPrivate->ComponentThread,
                       PERF_FOURCC('P','C','D','T'));
#endif
 EXIT:
#if 1
    if(OMX_ErrorNone != eError) {
        PCMDEC_EPRINT(":: ************* ERROR: Freeing Other Malloced Resources\n");
        PCMD_OMX_FREE(pPortDef_ip);
        PCMD_OMX_FREE(pPortDef_op);
        PCMD_OMX_FREE(pcm_ip);
        PCMD_OMX_FREE(pcm_op);
        /*  PCMD_OMX_FREE(pComponentPrivate); */
        PCMD_OMX_FREE(pTemp);
    }
#endif
    PCMDEC_DPRINT ("Exiting OMX_ComponentInit\n");
    return eError;
}


/* ================================================================================= * */
/**
* @fn SendCommand() function receives all the commands from the application.
*
* @param phandle This is component handle.
* 
* @param Cmd    This is commnad set that came from application.
*
* @param nParam This is commnad of the commands set that came from application.
*
* @param pCmdData This is command data that came with command.
*
* @pre          OMX_Init should be called by application.
*
* @post         None
*
* @return      OMX_ErrorNone = Successful Inirialization of the component\n
*              OMX_ErrorBadPortIndex = Bad port index specified by application.
*/
/* ================================================================================ * */
static OMX_ERRORTYPE SendCommand (OMX_HANDLETYPE phandle,
                                  OMX_COMMANDTYPE Cmd,
                                  OMX_U32 nParam,OMX_PTR pCmdData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    int nRet;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)phandle;
    PCMDEC_COMPONENT_PRIVATE *pCompPrivate = NULL;

    PCMD_OMX_CONF_CHECK_CMD(pHandle,1,1);
    pCompPrivate = (PCMDEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;
#ifdef _ERROR_PROPAGATION__
    if (pCompPrivate->curState == OMX_StateInvalid){
        eError = OMX_ErrorInvalidState;
        goto EXIT;
    }   
#else
    PCMDEC_DPRINT("PCMDEC: Entered SendCommand\n");
    if(pCompPrivate->curState == OMX_StateInvalid){
        PCMDEC_DPRINT("PCMDEC: Error Notofication Sent to App\n");
#if 0
        pCompPrivate->cbInfo.EventHandler (
                                           pHandle, pHandle->pApplicationPrivate,
                                           OMX_EventError, OMX_ErrorInvalidState,0,
                                           "Invalid State");
#endif

        PCMD_OMX_ERROR_EXIT(eError, OMX_ErrorInvalidState,"OMX_ErrorInvalidState");

    }
#endif    
#ifdef __PERF_INSTRUMENTATION__
    PERF_SendingCommand(pCompPrivate->pPERF,
                        Cmd,
                        (Cmd == OMX_CommandMarkBuffer) ? ((OMX_U32) pCmdData) : nParam,
                        PERF_ModuleComponent);
#endif
    switch(Cmd) {
    case OMX_CommandStateSet:
        PCMDEC_DPRINT("PCMDEC: Entered switch - Command State Set\n");
        if (nParam == OMX_StateLoaded) {
            pCompPrivate->bLoadedCommandPending = OMX_TRUE;
        }
        if(pCompPrivate->curState == OMX_StateLoaded) {
            PCMDEC_DPRINT("PCMDEC: Entered switch - curState == OMX_StateLoaded\n");
            if((nParam == OMX_StateExecuting) || (nParam == OMX_StatePause)) {
                PCMDEC_DPRINT("PCMDEC: Entered switch - nParam == StatExecuting || OMX_StatePause\n");
                pCompPrivate->cbInfo.EventHandler (
                                                   pHandle,
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventError,
                                                   OMX_ErrorIncorrectStateTransition,
                                                   0,
                                                   NULL);
                PCMDEC_DPRINT("Incorrect St Tr fm Loaded to Executing By App\n");
                goto EXIT;
            }

            if(nParam == OMX_StateInvalid) {
                pCompPrivate->curState = OMX_StateInvalid;
                pCompPrivate->cbInfo.EventHandler (
                                                   pHandle,
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventError,
                                                   OMX_ErrorInvalidState,
                                                   0,
                                                   NULL);
                PCMDEC_DPRINT("Incorrect State Tr from Loaded to Invalid by Application\n");
                goto EXIT;
            }
        }
        break;
    case OMX_CommandFlush:
        PCMDEC_DPRINT("PCMDEC: Entered switch - Command Flush\n");
        if(nParam > 1 && nParam != -1) {
            PCMD_OMX_ERROR_EXIT(eError,OMX_ErrorBadPortIndex,"OMX_ErrorBadPortIndex");
        }
        break;
    case OMX_CommandPortDisable:
        break;
    case OMX_CommandPortEnable:
        PCMDEC_DPRINT("PCMDEC: Entered switch - Command Port Enable/Disbale\n");
        break;
    case OMX_CommandMarkBuffer:
        PCMDEC_DPRINT("PCMDEC: Entered switch - Command Mark Buffer\n");

        if(nParam > 0) {
            PCMD_OMX_ERROR_EXIT(eError,OMX_ErrorBadPortIndex,"OMX_ErrorBadPortIndex");
        }
        break;

        break;
    default:
        PCMDEC_DPRINT("PCMDEC: Entered switch - Default\n");
        PCMDEC_DPRINT("PCMDEC: Command Received Default error\n");
#if 1
        pCompPrivate->cbInfo.EventHandler(
                                          pHandle, pHandle->pApplicationPrivate,
                                          OMX_EventError,
                                          OMX_ErrorUndefined,0,
                                          "Invalid Command");
        break;
#endif
        /*            eError = OMX_ErrorUndefined; */
        goto EXIT;
    }


    nRet = write (pCompPrivate->cmdPipe[1], &Cmd, sizeof(Cmd));
    if (nRet == -1) {
        PCMD_OMX_ERROR_EXIT(eError,OMX_ErrorHardware,"write failed: OMX_ErrorHardware");
    }

    
    if (Cmd == OMX_CommandMarkBuffer) {
        nRet = write (pCompPrivate->cmdDataPipe[1], &pCmdData,
                      sizeof(OMX_PTR));
        if (nRet == -1) {
            PCMD_OMX_ERROR_EXIT(eError,OMX_ErrorHardware,"write failed: OMX_ErrorHardware");
        }
    }
    else {
        nRet = write (pCompPrivate->cmdDataPipe[1], &nParam,
                      sizeof(OMX_U32));
        if (nRet == -1) {
            PCMD_OMX_ERROR_EXIT(eError,OMX_ErrorHardware,"write failed: OMX_ErrorHardware");
        }
    }


    PCMDEC_DPRINT("PCMDEC:SendCommand - nRet = %d\n",nRet);
    if (nRet == -1) {
        PCMD_OMX_ERROR_EXIT(eError,OMX_ErrorInsufficientResources,"OMX_ErrorInsufficientResources");
    }
 EXIT:
    return eError;
}



/* ================================================================================= * */
/**
* @fn GetParameter() function gets the various parameter values of the
* component.
*
* @param hComp         This is component handle.
* 
* @param nParamIndex   This is enumerate values which specifies what kind of
*                      information is to be retreived form the component.
*
* @param ComponentParameterStructure      This is output argument which is
*                                         filled by the component component 
*
* @pre          The component should be in loaded state.
*
* @post         None
*
* @return      OMX_ErrorNone = Successful Inirialization of the component\n
*              OMX_ErrorBadPortIndex = Bad port index specified by application.
*/
/* ================================================================================ * */
static OMX_ERRORTYPE GetParameter (OMX_HANDLETYPE hComp,
                                   OMX_INDEXTYPE nParamIndex,
                                   OMX_PTR ComponentParameterStructure)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    PCMDEC_COMPONENT_PRIVATE  *pComponentPrivate;
    OMX_PARAM_PORTDEFINITIONTYPE *pParameterStructure;

    pParameterStructure = (OMX_PARAM_PORTDEFINITIONTYPE*)ComponentParameterStructure;

    PCMDEC_DPRINT("Inside the GetParameter:: %x\n",nParamIndex);

    PCMD_OMX_CONF_CHECK_CMD(hComp,1,1);
    pComponentPrivate = (PCMDEC_COMPONENT_PRIVATE *)(((OMX_COMPONENTTYPE*)hComp)->pComponentPrivate);

    PCMD_OMX_CONF_CHECK_CMD(pComponentPrivate, ComponentParameterStructure, 1);
    PCMDEC_DPRINT(":: Entering the GetParameter\n");
    if (pParameterStructure == NULL) {
        eError = OMX_ErrorBadParameter;
        PCMDEC_DPRINT(":: OMX_ErrorBadPortIndex from GetParameter");
        goto EXIT;
    }
#ifdef _ERROR_PROPAGATION__
    if (pComponentPrivate->curState == OMX_StateInvalid){
        eError = OMX_ErrorInvalidState;
        goto EXIT;
    }   
#else    
    if(pComponentPrivate->curState == OMX_StateInvalid) {
#if 0
        pComponentPrivate->cbInfo.EventHandler(
                                               hComp,
                                               ((OMX_COMPONENTTYPE *)hComp)->pApplicationPrivate,
                                               OMX_EventError,
                                               OMX_ErrorIncorrectStateOperation,
                                               0,
                                               NULL);
#endif
        eError = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }
#endif


    switch(nParamIndex){
    case OMX_IndexParamAudioInit:
        {
            PCMDEC_DPRINT(":: GetParameter OMX_IndexParamAudioInit\n");
            memcpy(ComponentParameterStructure, pComponentPrivate->sPortParam, sizeof(OMX_PORT_PARAM_TYPE));
        }
        break;
    case OMX_IndexParamCompBufferSupplier:
        if(((OMX_PARAM_BUFFERSUPPLIERTYPE *)(ComponentParameterStructure))->nPortIndex == OMX_DirInput) {
            PCMDEC_DPRINT(":: GetParameter OMX_IndexParamCompBufferSupplier \n");
            memcpy(ComponentParameterStructure, &(pComponentPrivate->sBufferSupplier), sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE));
        }
        else if(((OMX_PARAM_BUFFERSUPPLIERTYPE *)(ComponentParameterStructure))->nPortIndex == OMX_DirOutput) {
            PCMDEC_DPRINT(":: GetParameter OMX_IndexParamCompBufferSupplier \n");
            memcpy(ComponentParameterStructure, &(pComponentPrivate->sBufferSupplier), sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE));
        } 
        else {
            PCMDEC_DPRINT(":: OMX_ErrorBadPortIndex from GetParameter");
            eError = OMX_ErrorBadPortIndex;
        }           
        break;                      

    case OMX_IndexParamPortDefinition:
        {
            PCMDEC_DPRINT(": GetParameter OMX_IndexParamPortDefinition \n");
            if(((OMX_PARAM_PORTDEFINITIONTYPE *)(ComponentParameterStructure))->nPortIndex ==
               pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->nPortIndex) {
                memcpy(ComponentParameterStructure, pComponentPrivate->pPortDef[PCMD_INPUT_PORT], sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            } else if(((OMX_PARAM_PORTDEFINITIONTYPE *)(ComponentParameterStructure))->nPortIndex ==
                      pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->nPortIndex) {
                memcpy(ComponentParameterStructure, pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT], sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            } else {
                PCMDEC_DPRINT(":: OMX_ErrorBadPortIndex from GetParameter \n");
                eError = OMX_ErrorBadPortIndex;
            }
        }
        break;

    case OMX_IndexParamAudioPortFormat:   
        {
            PCMDEC_DPRINT(":: GetParameter OMX_IndexParamAudioPortFormat \n");
            if(((OMX_AUDIO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->nPortIndex ==
               pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->nPortIndex) {
                if(((OMX_AUDIO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->nIndex >
                   pComponentPrivate->pCompPort[PCMD_INPUT_PORT]->pPortFormat->nPortIndex) {
                    eError = OMX_ErrorNoMore;
                }
                else {
                    memcpy(ComponentParameterStructure, pComponentPrivate->pCompPort[PCMD_INPUT_PORT]->pPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
                }
            }
            else if(((OMX_AUDIO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->nPortIndex ==
                    pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->nPortIndex){
                if(((OMX_AUDIO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->nIndex >
                   pComponentPrivate->pCompPort[PCMD_OUTPUT_PORT]->pPortFormat->nPortIndex) {
                    eError = OMX_ErrorNoMore;
                }
                else {
                    memcpy(ComponentParameterStructure, pComponentPrivate->pCompPort[PCMD_OUTPUT_PORT]->pPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
                }
            }
            else {
                PCMDEC_DPRINT(":: OMX_ErrorBadPortIndex from GetParameter \n");
                eError = OMX_ErrorBadPortIndex;
            }
        }
        break;

    case OMX_IndexParamAudioPcm:
        {
            PCMDEC_DPRINT(" :: GetParameter OMX_IndexParamAudioMp3 \n");
            if(((OMX_AUDIO_PARAM_PCMMODETYPE *)(ComponentParameterStructure))->nPortIndex ==
               pComponentPrivate->pcmParams[PCMD_INPUT_PORT]->nPortIndex) {
                memcpy(ComponentParameterStructure, pComponentPrivate->pcmParams[PCMD_INPUT_PORT], sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
            } else if(((OMX_AUDIO_PARAM_PCMMODETYPE *)(ComponentParameterStructure))->nPortIndex ==
                      pComponentPrivate->pcmParams[PCMD_OUTPUT_PORT]->nPortIndex) {
                memcpy(ComponentParameterStructure, pComponentPrivate->pcmParams[PCMD_OUTPUT_PORT], sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));

            } else {
                PCMDEC_DPRINT(" :: OMX_ErrorBadPortIndex from GetParameter \n");
                eError = OMX_ErrorBadPortIndex;
            }
        }
        break;

    case OMX_IndexParamPriorityMgmt:
        {
            PCMDEC_DPRINT(" :: GetParameter OMX_IndexParamPriorityMgmt \n");
            memcpy(ComponentParameterStructure, pComponentPrivate->pPriorityMgmt, sizeof(OMX_PRIORITYMGMTTYPE));
        }
        break;

    case OMX_IndexParamVideoInit:
        break;

    case OMX_IndexParamImageInit:
        break;
        
    case OMX_IndexParamOtherInit:
        break;
        

    default:
        {
            PCMDEC_DPRINT(" :: OMX_ErrorUnsupportedIndex GetParameter \n");
            eError = OMX_ErrorUnsupportedIndex;
        }
        break;
    }
 EXIT:
    PCMDEC_DPRINT(" :: Exiting GetParameter\n");
    PCMDEC_DPRINT(" :: Returning = 0x%x\n",eError);
    return eError;
}


/* ================================================================================= * */
/**
* @fn SetParameter() function sets the various parameter values of the
* component.
*
* @param hComp         This is component handle.
* 
* @param nParamIndex   This is enumerate values which specifies what kind of
*                      information is to be set for the component.
*
* @param ComponentParameterStructure      This is input argument which contains
*                                         the values to be set for the component.
*
* @pre          The component should be in loaded state.
*
* @post         None
*
* @return      OMX_ErrorNone = Successful Inirialization of the component\n
*              OMX_ErrorBadPortIndex = Bad port index specified by application.
*/
/* ================================================================================ * */
static OMX_ERRORTYPE SetParameter (
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_INDEXTYPE nIndex,
        OMX_IN  OMX_PTR pCompParam)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE* pHandle= (OMX_COMPONENTTYPE*)hComponent;
    PCMDEC_COMPONENT_PRIVATE  *pComponentPrivate;
    OMX_AUDIO_PARAM_PORTFORMATTYPE* pComponentParam = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE *pComponentParamPort = NULL;
    OMX_AUDIO_PARAM_PCMMODETYPE *pCompPcmParam = NULL;
    OMX_PARAM_COMPONENTROLETYPE  *pRole;

    PCMD_OMX_CONF_CHECK_CMD(hComponent,1,1) 
        pComponentPrivate = (PCMDEC_COMPONENT_PRIVATE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    PCMD_OMX_CONF_CHECK_CMD(pComponentPrivate, pCompParam, 1); 
#ifdef _ERROR_PROPAGATION__
    if (pComponentPrivate->curState == OMX_StateInvalid){
        eError = OMX_ErrorInvalidState;
        goto EXIT;
    }   
#endif

    PCMDEC_DPRINT(" :: Entering the SetParameter\n");
    if (pComponentPrivate->curState != OMX_StateLoaded) {
        eError = OMX_ErrorIncorrectStateOperation;
        PCMDEC_DPRINT(" :: OMX_ErrorIncorrectStateOperation from SetParameter");
        goto EXIT;
    }
#if 0
    if (pCompParam == NULL) {
        eError = OMX_ErrorBadParameter;
        PCMDEC_DPRINT(" :: OMX_ErrorBadParameter from SetParameter");
        goto EXIT;
    }
#endif

    switch(nIndex) {
    case OMX_IndexParamAudioPortFormat:
        {
            PCMDEC_DPRINT(":: SetParameter OMX_IndexParamAudioPortFormat \n");
            pComponentParam = (OMX_AUDIO_PARAM_PORTFORMATTYPE *)pCompParam;
            if ( pComponentParam->nPortIndex == pComponentPrivate->pCompPort[PCMD_INPUT_PORT]->pPortFormat->nPortIndex ) {
                memcpy(pComponentPrivate->pCompPort[PCMD_INPUT_PORT]->pPortFormat, pComponentParam, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
            } else if ( pComponentParam->nPortIndex == pComponentPrivate->pCompPort[PCMD_OUTPUT_PORT]->pPortFormat->nPortIndex ) {
                memcpy(pComponentPrivate->pCompPort[PCMD_OUTPUT_PORT]->pPortFormat, pComponentParam, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
            } else {
                PCMDEC_DPRINT(":: OMX_ErrorBadPortIndex from SetParameter");
                eError = OMX_ErrorBadPortIndex;
            }
        }
        break;
    case OMX_IndexParamAudioPcm:
        {
            PCMDEC_DPRINT(" :: SetParameter OMX_IndexParamAudioPcm \n");

            pCompPcmParam = (OMX_AUDIO_PARAM_PCMMODETYPE *)pCompParam;
                
            /* This if condition is not yet well defined because khronos
             * test suite does not set the sampling frequency. For component
             * test application, it is meant to test that component returns
             * the error on invalid frequecy */ 

#if 1
            if(pCompPcmParam->nSamplingRate == INVALID_SAMPLING_FREQ) {
                eError = OMX_ErrorUnsupportedIndex;
                PCMDEC_EPRINT("Unsupported SampleRate Given By the App\n");
                goto EXIT;
            }
#endif

            if(pCompPcmParam->nPortIndex == 0) { /* 0 means Input port */
                memcpy(((PCMDEC_COMPONENT_PRIVATE*)
                        pHandle->pComponentPrivate)->pcmParams[PCMD_INPUT_PORT], pCompPcmParam, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
            } else if (pCompPcmParam->nPortIndex == 1) { /* 1 means Output port */
                memcpy(((PCMDEC_COMPONENT_PRIVATE *)
                        pHandle->pComponentPrivate)->pcmParams[PCMD_OUTPUT_PORT], pCompPcmParam, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
            }
            else {
                PCMDEC_DPRINT(":: OMX_ErrorBadPortIndex from SetParameter");
                eError = OMX_ErrorBadPortIndex;
            }
        }
        break;
    case OMX_IndexParamPortDefinition:
        {
            pComponentParamPort = (OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam;
            PCMDEC_DPRINT(":: SetParameter OMX_IndexParamPortDefinition \n");
            if(((OMX_PARAM_PORTDEFINITIONTYPE *)(pCompParam))->nPortIndex ==
               pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->nPortIndex) {
                PCMDEC_DPRINT(":: SetParameter OMX_IndexParamPortDefinition \n");
                memcpy(pComponentPrivate->pPortDef[PCMD_INPUT_PORT], pCompParam,sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            }
            else if(((OMX_PARAM_PORTDEFINITIONTYPE *)(pCompParam))->nPortIndex ==
                    pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->nPortIndex) {
                PCMDEC_DPRINT(":: SetParameter OMX_IndexParamPortDefinition \n");
                memcpy(pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT], pCompParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            }
            else {
                PCMDEC_DPRINT(":: OMX_ErrorBadPortIndex from SetParameter");
                eError = OMX_ErrorBadPortIndex;
            }
        }
        break;
    case OMX_IndexParamPriorityMgmt:
        {
            PCMDEC_DPRINT(":: SetParameter OMX_IndexParamPriorityMgmt \n");
            memcpy(pComponentPrivate->pPriorityMgmt, (OMX_PRIORITYMGMTTYPE*)pCompParam, sizeof(OMX_PRIORITYMGMTTYPE));
        }
        break;

    case OMX_IndexParamAudioInit:
        {
            PCMDEC_DPRINT(":: SetParameter OMX_IndexParamAudioInit \n");
            memcpy(pComponentPrivate->sPortParam, (OMX_PORT_PARAM_TYPE*)pCompParam, sizeof(OMX_PORT_PARAM_TYPE));
        }
        break;
            
    case OMX_IndexParamCompBufferSupplier:
        if(((OMX_PARAM_PORTDEFINITIONTYPE *)(pCompParam))->nPortIndex ==
           pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->nPortIndex) {
            PCMDEC_DPRINT(":: SetParameter OMX_IndexParamCompBufferSupplier \n");
            pComponentPrivate->sBufferSupplier.eBufferSupplier = OMX_BufferSupplyInput;
            memcpy(&pComponentPrivate->sBufferSupplier, pCompParam, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE));                                 
        }
        else if(((OMX_PARAM_PORTDEFINITIONTYPE *)(pCompParam))->nPortIndex ==
                pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->nPortIndex) {
            PCMDEC_DPRINT(":: SetParameter OMX_IndexParamCompBufferSupplier \n");
            pComponentPrivate->sBufferSupplier.eBufferSupplier = OMX_BufferSupplyOutput;
            memcpy(&(pComponentPrivate->sBufferSupplier), pCompParam, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE));
        } 
        else {
            PCMDEC_DPRINT(":: OMX_ErrorBadPortIndex from SetParameter");
            eError = OMX_ErrorBadPortIndex;
        }
        break;          
            
    case OMX_IndexParamStandardComponentRole:

        if (pCompParam) {
            pRole = (OMX_PARAM_COMPONENTROLETYPE *)pCompParam;
            memcpy(&(pComponentPrivate->componentRole), (void *)pRole, sizeof(OMX_PARAM_COMPONENTROLETYPE));
            PCMDEC_DPRINT(":: SetParameter OMX_IndexParamAudioInit \n");
                  
        } else {
            eError = OMX_ErrorBadParameter;
        }
        break;              
    default:
        {
            PCMDEC_DPRINT(":: SetParameter OMX_ErrorUnsupportedIndex \n");
            eError = OMX_ErrorUnsupportedIndex;
        }
        break;
    }
 EXIT:
    PCMDEC_DPRINT(":: Exiting SetParameter\n");
    PCMDEC_DPRINT(":: Returning = 0x%x\n",eError);
    return eError;
}


/* ================================================================================= * */
/**
* @fn SetCallbacks() Sets application callbacks to the component
*
* @param pComponent  This is component handle.
* 
* @param pCallBacks  Application callbacks
*
* @param pAppData    Application specified private data.
*
* @pre          None
*
* @post         None
*
* @return      OMX_ErrorNone = Successful Inirialization of the component
*              OMX_ErrorBadParameter = If callback argument is NULL.
*/
/* ================================================================================ * */

static OMX_ERRORTYPE SetCallbacks (OMX_HANDLETYPE pComponent,
                                   OMX_CALLBACKTYPE* pCallBacks,
                                   OMX_PTR pAppData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE*)pComponent;

    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate;


    PCMD_OMX_CONF_CHECK_CMD(pHandle,1,1)

        pComponentPrivate = (PCMDEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;

    PCMD_OMX_CONF_CHECK_CMD(pComponentPrivate,1,1)

        PCMDEC_DPRINT ("Entering SetCallbacks\n");

#if 0
    if (pCallBacks == NULL) {
        eError = OMX_ErrorBadParameter;
        PCMDEC_DPRINT ("%d :: Received the empty callbacks from the \
                application\n",__LINE__);
        goto EXIT;

    }
#endif
    PCMD_OMX_CONF_CHECK_CMD(pCallBacks, pCallBacks->EventHandler, pCallBacks->EmptyBufferDone);
    PCMD_OMX_CONF_CHECK_CMD(pCallBacks->FillBufferDone, 1, 1);

    memcpy (&(pComponentPrivate->cbInfo), pCallBacks, sizeof(OMX_CALLBACKTYPE));
    pHandle->pApplicationPrivate = pAppData;
    PCMDEC_STATEPRINT("****************** Component State Set to Loaded\n\n");
    pComponentPrivate->curState = OMX_StateLoaded;

 EXIT:
    PCMDEC_DPRINT ("Exiting SetCallbacks\n");
    return eError;
}


/* ================================================================================= * */
/**
* @fn GetComponentVersion() Sets application callbacks to the component. Currently this
* function is not implemented.
*
* @param hComp  This is component handle.
* 
* @param pComponentName  This is component name.
*
* @param pComponentVersion  This output argument will contain the component
*                           version when this function exits successfully.
*
* @param pSpecVersion    This is specification version.
* 
* @param pComponentUUID  This specifies the UUID of the component.
*
* @pre          None
*
* @post         None
*
* @return      OMX_ErrorNone = Successful Inirialization of the component
*/
/* ================================================================================ * */
static OMX_ERRORTYPE GetComponentVersion (OMX_HANDLETYPE hComp,
                      OMX_STRING pComponentName,
                                          OMX_VERSIONTYPE* pComponentVersion,
                                          OMX_VERSIONTYPE* pSpecVersion,
                                          OMX_UUIDTYPE* pComponentUUID)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    eError = OMX_ErrorNotImplemented;

    PCMDEC_DPRINT ("Entering GetComponentVersion\n");
    PCMDEC_DPRINT ("Inside  GetComponentVersion\n");
    PCMDEC_DPRINT ("Exiting GetComponentVersion\n");
    return eError;
}



/* ================================================================================= * */
/**
* @fn GetConfig() gets the configuration of the component depending on the value
* of nConfigINdex. This function is currently not implemented.
*
* @param hComp  This is component handle.
* 
* @param nConfigIndex  This is config index to get the configuration of
*                      component.
*
* @param ComponentConfigStructure This is configuration structure that is filled
* by the component depending on the value of nConfigIndex.
*
* @pre          None
*
* @post         None
*
* @return      OMX_ErrorNone = Successful Inirialization of the component
*/
/* ================================================================================ * */
static OMX_ERRORTYPE GetConfig (OMX_HANDLETYPE hComp,
                                OMX_INDEXTYPE nConfigIndex,
                                OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate;
    TI_OMX_STREAM_INFO streamInfo;

    pComponentPrivate = (PCMDEC_COMPONENT_PRIVATE *)
        (((OMX_COMPONENTTYPE*)hComp)->pComponentPrivate);
#ifdef _ERROR_PROPAGATION__
    if (pComponentPrivate->curState == OMX_StateInvalid){
        eError = OMX_ErrorInvalidState;
        goto EXIT;
    }   
#endif
    
    PCMDEC_DPRINT ("Inside   GetConfig\n");
    if(nConfigIndex == OMX_IndexCustomPcmDecStreamIDConfig)
        {
            /* copy component info */
            streamInfo.streamId = pComponentPrivate->streamID;
            memcpy(ComponentConfigStructure,&streamInfo,sizeof(TI_OMX_STREAM_INFO));
        }
    else if(nConfigIndex == OMX_IndexConfigAudioMute){
        memcpy(ComponentConfigStructure,&(pComponentPrivate->sMuteType),sizeof(OMX_AUDIO_CONFIG_MUTETYPE));
    }
    else if(nConfigIndex ==  OMX_IndexConfigAudioVolume){
        memcpy(ComponentConfigStructure,&(pComponentPrivate->sVolumeType),sizeof(OMX_AUDIO_CONFIG_VOLUMETYPE));
    }

 EXIT:
    PCMDEC_DPRINT ("Exiting  GetConfig\n");
    return eError;
}

/* ================================================================================= * */
/**
* @fn SetConfig() Sets the configuration of the component depending on the value
* of nConfigINdex.
*
* @param hComp  This is component handle.
* 
* @param nConfigIndex  This is config index to get the configuration of
*                      component.
*
* @param ComponentConfigStructure This is configuration structure that contains
*                                 the information which the component has to
*                                 configured with.
*
* @pre          None
*
* @post         None
*
* @return      OMX_ErrorNone = Successful Inirialization of the component
*/
/* ================================================================================ * */
static OMX_ERRORTYPE SetConfig (OMX_HANDLETYPE hComp,
                                OMX_INDEXTYPE nConfigIndex,
                                OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate = NULL;
    OMX_AUDIO_CONFIG_MUTETYPE *pMuteStructure = NULL;
    OMX_AUDIO_CONFIG_VOLUMETYPE *pVolumeStructure = NULL;
    TI_OMX_DSP_DEFINITION* pDspDefinition = NULL;
    OMX_U32 *bTemp;
    OMX_S16 *customFlag = NULL;
    TI_OMX_DATAPATH dataPath;

    PCMDEC_DPRINT ("Entering SetConfig\n");

    PCMD_OMX_CONF_CHECK_CMD(hComp,1,1)

        pComponentPrivate = (PCMDEC_COMPONENT_PRIVATE *)
        (((OMX_COMPONENTTYPE*)hComp)->pComponentPrivate);

    PCMD_OMX_CONF_CHECK_CMD(pComponentPrivate,1,1)
#ifdef _ERROR_PROPAGATION__
        if (pComponentPrivate->curState == OMX_StateInvalid){
            eError = OMX_ErrorInvalidState;
            goto EXIT;
        }   
#endif    
    switch (nConfigIndex)
        {
        case OMX_IndexCustomPcmDecHeaderInfoConfig:
            {
                pDspDefinition = (TI_OMX_DSP_DEFINITION *)ComponentConfigStructure;
                pComponentPrivate->dasfmode = pDspDefinition->dasfMode;
                if (pDspDefinition->dasfMode == 0) {
                    pComponentPrivate->dasfmode = 0;
                }
                else if (pDspDefinition->dasfMode == 1) {
                    pComponentPrivate->dasfmode = 1;
                }
                else if (pDspDefinition->dasfMode == 2) {
                    pComponentPrivate->dasfmode = 1;
                }
                pComponentPrivate->streamID = pDspDefinition->streamId;
            
                break;
            }
        
        case  OMX_IndexCustomPcmDecDataPath:
            customFlag = (OMX_S16*)ComponentConfigStructure;
            if (customFlag == NULL) {
                eError = OMX_ErrorBadParameter;
                goto EXIT;
            }

            dataPath = *customFlag;

            switch(dataPath) {
            case DATAPATH_APPLICATION:
                OMX_MMMIXER_DATAPATH(pComponentPrivate->sDeviceString, RENDERTYPE_DECODER, pComponentPrivate->streamID);
                break;
                
            case DATAPATH_APPLICATION_TEE:
                OMX_MMMIXER_DATAPATH_TEE(pComponentPrivate->sDeviceString, RENDERTYPE_DECODER, pComponentPrivate->streamID);
                break;


            case DATAPATH_APPLICATION_RTMIXER:
                strcpy((char*)pComponentPrivate->sDeviceString,(char*)RTM_STRING);
                break;

            case DATAPATH_ACDN:
                strcpy((char*)pComponentPrivate->sDeviceString,(char*)ACDN_STRING);
                break;

            default:
                break;
                
            }
            break;
        
        case OMX_IndexCustomPcmDecLowLatencyConfig:
            {
                bTemp= (OMX_U32 *)ComponentConfigStructure;
                pComponentPrivate->bLowLatencyDisabled = *bTemp;
                PCMDEC_DPRINT ("pComponentPrivate->bLowLatencyDisabled = %d\n",
                               pComponentPrivate->bLowLatencyDisabled);
                break;
            }    

            /* set mute/unmute for playback stream */
        case OMX_IndexConfigAudioMute:
            {
#ifdef DSP_RENDERING_ON
                pMuteStructure = (OMX_AUDIO_CONFIG_MUTETYPE *)ComponentConfigStructure;
                pComponentPrivate->sMuteType.bMute = pMuteStructure->bMute;
                PCMDEC_DPRINT("Set Mute/Unmute for playback stream\n");
                cmd_data.hComponent = hComp;
                if(pMuteStructure->bMute == OMX_TRUE)
                    {
                        PCMDEC_DPRINT("Mute the playback stream\n");
                        cmd_data.AM_Cmd = AM_CommandStreamMute;
                    }
                else
                    {
                        PCMDEC_DPRINT("unMute the playback stream\n");
                        cmd_data.AM_Cmd = AM_CommandStreamUnMute;
                    }
                cmd_data.param1 = 0;
                cmd_data.param2 = 0;
                cmd_data.streamID = pComponentPrivate->streamID;

                if((write(pcmd_fdwrite, &cmd_data, sizeof(cmd_data)))<0)
                    {   
                        PCMDEC_DPRINT("[PCM decoder] - fail to send command to audio manager\n");
                    }
#endif
                break;
            }

            /* set volume for playback stream */
        case OMX_IndexConfigAudioVolume:
            {
#ifdef DSP_RENDERING_ON
                pVolumeStructure = (OMX_AUDIO_CONFIG_VOLUMETYPE *)ComponentConfigStructure;
                pComponentPrivate->sVolumeType.bLinear = pVolumeStructure->bLinear;
                PCMDEC_DPRINT("Set volume for playback stream\n");
                cmd_data.hComponent = hComp;
                cmd_data.AM_Cmd = AM_CommandSWGain;
                cmd_data.param1 = pVolumeStructure->sVolume.nValue;
                cmd_data.param2 = 0;
                cmd_data.streamID = pComponentPrivate->streamID;

                if((write(pcmd_fdwrite, &cmd_data, sizeof(cmd_data)))<0)
                    {   
                        PCMDEC_DPRINT("[PCM decoder] - fail to send command to audio manager\n");
                    }
#endif
                break;
            }
        default:
            eError = OMX_ErrorUnsupportedIndex;
            break;      
        }
        
    PCMDEC_DPRINT ("Exiting  SetConfig\n");
 EXIT:

    return eError;
}

/* ================================================================================= * */
/**
* @fn GetState() Gets the current state of the component.
*
* @param pComponent  This is component handle.
* 
* @param pState      This is the output argument that contains the state of the
*                    component.
*
* @pre          None
*
* @post         None
*
* @return      OMX_ErrorNone = Successful Inirialization of the component
*              OMX_ErrorBadParameter = if output argument is NULL. 
*/
/* ================================================================================ * */
static OMX_ERRORTYPE GetState (OMX_HANDLETYPE pComponent, OMX_STATETYPE* pState)
{
    OMX_ERRORTYPE eError = OMX_ErrorUndefined;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;

    PCMDEC_DPRINT ("Entering GetState\n");

    if (!pState) {
        PCMDEC_DPRINT (":: About to exit with bad parameter\n");
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }

    PCMD_OMX_CONF_CHECK_CMD(pHandle,1,1) 
        if (pHandle && pHandle->pComponentPrivate) {
            PCMDEC_DPRINT (":: In GetState\n");
            *pState =  ((PCMDEC_COMPONENT_PRIVATE*)
                        pHandle->pComponentPrivate)->curState;
            PCMDEC_DPRINT (":: curState = %d\n",(int)*pState);
        } else {
            PCMDEC_DPRINT (":: In GetState\n");
            PCMDEC_STATEPRINT("Component State Set to Loaded\n\n");
            *pState = OMX_StateLoaded;
        }

    eError = OMX_ErrorNone;

 EXIT:
    PCMDEC_DPRINT (":: Exiting GetState\n");
    return eError;
}

/*-------------------------------------------------------------------*/
/**
  *  EmptyThisBuffer() This callback is used to send the input buffer to
  *  component
  *
  * @param pComponent       handle for this instance of the component
  * @param nPortIndex       input port index
  * @param pBuffer          buffer to be sent to codec
  *
  * @retval OMX_NoError              Success, ready to roll
  *         OMX_Error_BadParameter   The input parameter pointer is null
  **/
/*-------------------------------------------------------------------*/

/* ================================================================================= * */
/**
* @fn EmptyThisBuffer() This function is used by application to sent the filled
* input buffers to the component.
*
* @param pComponent  This is component handle.
* 
* @param pBuffer     This is poiter to the buffer header that come from the
*                    application.
*
* @pre          None
*
* @post         None
*
* @return      OMX_ErrorNone = Successful exit of the function
*              OMX_ErrorBadParameter =  Bad input argument
*              OMX_ErrorBadPortIndex = Bad port index supplied by the
*              application
*/
/* ================================================================================ * */

static OMX_ERRORTYPE EmptyThisBuffer (OMX_HANDLETYPE pComponent,
                                      OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate;
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDef;
    int ret=0;

    /*
      int n=0;
      int flag=0;
      int i=0; */
    PCMD_OMX_CONF_CHECK_CMD(pHandle,1,1)

        pComponentPrivate = (PCMDEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;

    PCMD_OMX_CONF_CHECK_CMD(pComponentPrivate,1,1)

        pPortDef = ((PCMDEC_COMPONENT_PRIVATE*) 
                    pComponentPrivate)->pPortDef[PCMD_INPUT_PORT];
#ifdef _ERROR_PROPAGATION__
    if (pComponentPrivate->curState == OMX_StateInvalid){
        eError = OMX_ErrorInvalidState;
        goto EXIT;
    }   
#endif                    
#ifdef __PERF_INSTRUMENTATION__
    PERF_ReceivedFrame(pComponentPrivate->pPERF,
                       pBuffer->pBuffer,
                       pBuffer->nFilledLen,
                       PERF_ModuleHLMM);
#endif

    if(!pPortDef->bEnabled) {
        eError = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    if (pBuffer == NULL) {
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }

    if (pBuffer->nSize != sizeof(OMX_BUFFERHEADERTYPE)) {
        PCMDEC_EPRINT(":: Error: Bad Size = %ld, Add: %p\n",
                      pBuffer->nSize,pBuffer);
        PCMD_OMX_ERROR_EXIT(eError,OMX_ErrorBadParameter,"Bad Size");
    } else {
        /*        PCMDEC_DPRINT("%d :: EmptyThisBuffer: size of OK\n",__LINE__); */
    }

    if (pBuffer->nInputPortIndex != PCMD_INPUT_PORT) {
        PCMD_OMX_ERROR_EXIT(eError,OMX_ErrorBadPortIndex,"OMX_ErrorBadPortIndex");
    }


    if (pBuffer->nVersion.nVersion != pComponentPrivate->nVersion) {
        PCMD_OMX_ERROR_EXIT(eError,OMX_ErrorVersionMismatch,"OMX_ErrorVersionMismatch");
    }


    if(pComponentPrivate->curState != OMX_StateExecuting && pComponentPrivate->curState != OMX_StatePause) {
        eError = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }


    PCMDEC_DPRINT("\n------------------------------------------\n\n");
    PCMDEC_DPRINT (":: Component Sending Filled ip buff %p \
                             to Component Thread\n",pBuffer);
    PCMDEC_DPRINT("\n------------------------------------------\n\n");

    if (pComponentPrivate->bBypassDSP == 0) {
        pComponentPrivate->app_nBuf--;
    }

    pComponentPrivate->pMarkData = pBuffer->pMarkData;
    pComponentPrivate->hMarkTargetComponent = pBuffer->hMarkTargetComponent;
    if (pBuffer->nFlags == OMX_BUFFERFLAG_EOS) {                            
        pComponentPrivate->bIsEOFSent = OMX_TRUE;
    }                   
    ret = write (pComponentPrivate->dataPipe[1], &pBuffer,
                 sizeof(OMX_BUFFERHEADERTYPE*));
    if (ret == -1) {
        PCMD_OMX_ERROR_EXIT(eError,OMX_ErrorHardware,"write failed: OMX_ErrorHardware");
    }

    pComponentPrivate->nEmptyThisBufferCount++;


 EXIT:
    PCMDEC_DPRINT (":: Exiting EmptyThisBuffer\n");
    return eError;
}

/* ================================================================================= * */
/**
* @fn FillThisBuffer() This function is used by application to sent the empty
* output buffers to the component.
*
* @param pComponent  This is component handle.
* 
* @param pBuffer     This is poiter to the output buffer header that come from the
*                    application.
*
* @pre          None
*
* @post         None
*
* @return      OMX_ErrorNone = Successful exit of the function
*              OMX_ErrorBadParameter =  Bad input argument
*              OMX_ErrorBadPortIndex = Bad port index supplied by the
*              application
*/
/* ================================================================================ * */
static OMX_ERRORTYPE FillThisBuffer (OMX_HANDLETYPE pComponent,
                                     OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate = NULL;
    int nRet=0;
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDef;

    PCMDEC_DPRINT("\n------------------------------------------\n\n");
    PCMDEC_DPRINT (" :: Component Sending Emptied op buff %p \
                             to Component Thread\n",pBuffer);
    PCMDEC_DPRINT("\n------------------------------------------\n\n");


    PCMD_OMX_CONF_CHECK_CMD(pHandle,1,1)

        pComponentPrivate = (PCMDEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;

    PCMD_OMX_CONF_CHECK_CMD(pComponentPrivate,1,1)


        pPortDef = ((PCMDEC_COMPONENT_PRIVATE*) 
                    pComponentPrivate)->pPortDef[PCMD_OUTPUT_PORT];
#ifdef _ERROR_PROPAGATION__
    if (pComponentPrivate->curState == OMX_StateInvalid){
        eError = OMX_ErrorInvalidState;
        goto EXIT;
    }   
#endif
#ifdef __PERF_INSTRUMENTATION__
    PERF_ReceivedFrame(pComponentPrivate->pPERF,
                       pBuffer->pBuffer,
                       0,
                       PERF_ModuleHLMM);
#endif

    if(!pPortDef->bEnabled) {
        eError = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    if (pBuffer == NULL) {
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }

    if (pBuffer->nSize != sizeof(OMX_BUFFERHEADERTYPE)) {
        PCMDEC_EPRINT (":: FillThisBuffer: Bad Size: %ld, Add: %p\n",
                       pBuffer->nSize, pBuffer);
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    } else {
        /*    PCMDEC_DPRINT("%d :: FillThisBuffer: size of OK\n",__LINE__); */
    }

    if (pBuffer->nOutputPortIndex != PCMD_OUTPUT_PORT) {
        PCMDEC_DPRINT (":: FillThisBuffer: BadPortIndex\n");
        eError  = OMX_ErrorBadPortIndex;
        goto EXIT;
    }
    PCMDEC_DPRINT("::pBuffer->nVersion.nVersion:%ld\n",pBuffer->nVersion.nVersion);
    PCMDEC_DPRINT("::pComponentPrivate->nVersion:%ld\n",pComponentPrivate->nVersion);
    if (pBuffer->nVersion.nVersion != pComponentPrivate->nVersion) {
        PCMDEC_DPRINT (":: FillThisBuffer: BufferHeader Version Mismatch\n");
        PCMDEC_DPRINT(":pBuffer->nVersion.nVersion:%ld\n",pBuffer->nVersion.nVersion);
        PCMDEC_DPRINT("::pComponentPrivate->nVersion:%ld\n",pComponentPrivate->nVersion);
        eError = OMX_ErrorVersionMismatch;
        goto EXIT;
    }
    if(pComponentPrivate->curState != OMX_StateExecuting && pComponentPrivate->curState != OMX_StatePause) {
        eError = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }


    pBuffer->nFilledLen = 0;

    if (pComponentPrivate->bBypassDSP == 0) {
        pComponentPrivate->app_nBuf--;
    }

    if(pComponentPrivate->pMarkBuf){
        PCMDEC_DPRINT("FillThisBuffer Line\n");
        pBuffer->hMarkTargetComponent = pComponentPrivate->pMarkBuf->hMarkTargetComponent;
        pBuffer->pMarkData = pComponentPrivate->pMarkBuf->pMarkData;
        pComponentPrivate->pMarkBuf = NULL;
    }

    if (pComponentPrivate->pMarkData) {
        PCMDEC_DPRINT("FillThisBuffer Line\n");
        pBuffer->hMarkTargetComponent = pComponentPrivate->hMarkTargetComponent;
        pBuffer->pMarkData = pComponentPrivate->pMarkData;
        pComponentPrivate->pMarkData = NULL;
    }

    nRet = write (pComponentPrivate->dataPipe[1], &pBuffer,
                  sizeof (OMX_BUFFERHEADERTYPE*));
    if (pBuffer->nFlags == OMX_BUFFERFLAG_EOS) {                            
        pComponentPrivate->bIsEOFSent = OMX_TRUE;
    }                   

    if (nRet == -1) {
        PCMD_OMX_ERROR_EXIT(eError,OMX_ErrorHardware,"write failed: OMX_ErrorHardware");
    }

    pComponentPrivate->nFillThisBufferCount++;

 EXIT:
    PCMDEC_DPRINT (":: Exiting FillThisBuffer\n");
    return eError;
}

/* ================================================================================= * */
/**
* @fn ComponentDeInit() This function deinitializes the component. It is called
* from OMX Core, not by application. Albeit, Application does call
* OMX_FreeHandle of OMX Core and which in turn calls this function.
*
* @param pHandle  This is component handle.
* 
* @pre          None
*
* @post        This function should clean or free as much resources as
*              possible.
*
* @return      OMX_ErrorNone = On Success
*              Appropriate error number in case any error happens.
*/
/* ================================================================================ * */
static OMX_ERRORTYPE ComponentDeInit(OMX_HANDLETYPE pHandle)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_ERRORTYPE eError1 = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pComponent = (OMX_COMPONENTTYPE *)pHandle;
    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate = NULL;
    int k=0, k2 = 0;


    PCMDEC_DPRINT("ComponentDeInit\n");

    PCMD_OMX_CONF_CHECK_CMD(pComponent,1,1)

        pComponentPrivate = (PCMDEC_COMPONENT_PRIVATE *)pComponent->pComponentPrivate;

    PCMD_OMX_CONF_CHECK_CMD(pComponentPrivate,1,1)
    
#ifdef __PERF_INSTRUMENTATION__
        PERF_Boundary(pComponentPrivate->pPERF,
                      PERF_BoundaryStart | PERF_BoundaryCleanup);
#endif

#ifdef DSP_RENDERING_ON
    close(pcmd_fdwrite);
    close(pcmd_fdread);
#endif

#ifdef RESOURCE_MANAGER_ENABLED
    eError = RMProxy_NewSendCommand(pHandle, RMProxy_FreeResource, OMX_PCM_Decoder_COMPONENT, 0, NEWSENDCOMMAND_MEMORY, NULL);
    if (eError != OMX_ErrorNone) {
        PCMDEC_DPRINT ("%d ::Error returned from destroy ResourceManagerProxy thread\n",
                       __LINE__);
    }

    eError1 = RMProxy_Deinitalize();
    if (eError1 != OMX_ErrorNone) {
        PCMDEC_EPRINT(":: First Error in ComponentDeinit: From RMProxy_Deinitalize\n");
        eError = eError1;
    }
#endif


    pComponentPrivate->bIsStopping = 1;
    pComponentPrivate->bExitCompThrd = 1;
    write (pComponentPrivate->cmdPipe[1], &pComponentPrivate->bExitCompThrd, sizeof(OMX_U16));    
    k = pthread_join(pComponentPrivate->ComponentThread, (void*)&k2);
    if(0 != k) {
        if (OMX_ErrorNone == eError) {
            PCMDEC_EPRINT(":: First Error in ComponentDeinit: From pthread_join\n");
            eError = OMX_ErrorHardware;
        }
    }

    eError1 = PCMDEC_FreeCompResources(pHandle);
    if (OMX_ErrorNone != eError1) {
        if (OMX_ErrorNone == eError) {
            PCMDEC_EPRINT(":: First Error in ComponentDeinit: From FreeCompResources\n");
            eError = eError1;
        }
    }

#ifdef __PERF_INSTRUMENTATION__
    PERF_Boundary(pComponentPrivate->pPERF,
                  PERF_BoundaryComplete | PERF_BoundaryCleanup);
    PERF_Done(pComponentPrivate->pPERF);
#endif
    if (pComponentPrivate->sDeviceString != NULL) {
        free(pComponentPrivate->sDeviceString);
    }

    PCMDEC_MEMPRINT(":: Freeing: pComponentPrivate = %p\n", pComponentPrivate);
    /*    free(pComponentPrivate); */
    PCMD_OMX_FREE(pComponentPrivate);
    PCMDEC_DPRINT("::*********** ComponentDeinit is Done************** \n");

 EXIT:
    return eError;
}


/* ================================================================================= * */
/**
* @fn ComponentTunnelRequest() This function estabilishes the tunnel between two
* components. This is not implemented currently.
*
* @param hComp  Handle of this component.
*
* @param nPort Port of this component on which tunneling has to be done.
*
* @param hTunneledComp Handle of the component with which tunnel has to be
*                      established.
*
* @param nTunneledPort Port of the tunneling component.
*
* @param pTunnelSetup Tunnel Setuup parameters.
*
* @pre          None
*
* @post        None
*
* @return      OMX_ErrorNone = On Success
*              Appropriate error number in case any error happens.
*/
/* ================================================================================ * */
static OMX_ERRORTYPE ComponentTunnelRequest (OMX_HANDLETYPE hComp,
                     OMX_U32 nPort, OMX_HANDLETYPE hTunneledComp,
                     OMX_U32 nTunneledPort,
                     OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    PCMDEC_DPRINT (":: Entering ComponentTunnelRequest\n");
    PCMDEC_DPRINT (":: Inside   ComponentTunnelRequest\n");
    eError = OMX_ErrorNotImplemented;
    PCMDEC_DPRINT (":: Exiting ComponentTunnelRequest\n");
    return eError;
}



/* ================================================================================= * */
/**
* @fn AllocateBuffer() This function allocated the memory for the buffer onm
* request from application.
*
* @param hComponent  Handle of this component.
*
* @param pBuffer  Pointer to the buffer header.
*
* @param nPortIndex  Input port or Output port
*
* @param pAppPrivate Application private data.
*
* @param nSizeBytes Size of the buffer that is to be allocated.
*
* @pre          None
*
* @post        Requested buffer should get the memory allocated.
*
* @return      OMX_ErrorNone = On Success
*              OMX_ErrorBadPortIndex = Bad port index from app
*/
/* ================================================================================ * */
static OMX_ERRORTYPE AllocateBuffer (OMX_IN OMX_HANDLETYPE hComponent,
                   OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
                   OMX_IN OMX_U32 nPortIndex,
                   OMX_IN OMX_PTR pAppPrivate,
                                     OMX_IN OMX_U32 nSizeBytes)
{
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = NULL;
    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate;           /* 55555555555555555555555555*/
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE *pBufferHeader = NULL;


    PCMDEC_DPRINT (":: Entering AllocateBuffer\n");
    PCMD_OMX_CONF_CHECK_CMD(hComponent,1,1)
        pComponentPrivate = (PCMDEC_COMPONENT_PRIVATE *)
        (((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    PCMD_OMX_CONF_CHECK_CMD(pComponentPrivate, 1, 1)

        pPortDef = ((PCMDEC_COMPONENT_PRIVATE*)
                    pComponentPrivate)->pPortDef[nPortIndex];
#ifdef _ERROR_PROPAGATION__
    if (pComponentPrivate->curState == OMX_StateInvalid){
        eError = OMX_ErrorInvalidState;
        goto EXIT;
    }   
#endif
    PCMD_OMX_CONF_CHECK_CMD(pPortDef, 1, 1)

        while (1) {
            if(pPortDef->bEnabled) {
                break;
            }
            pComponentPrivate->AlloBuf_waitingsignal = 1;  
#ifndef UNDER_CE
            pthread_mutex_lock(&pComponentPrivate->AlloBuf_mutex); 
            pthread_cond_wait(&pComponentPrivate->AlloBuf_threshold, &pComponentPrivate->AlloBuf_mutex);
            pthread_mutex_unlock(&pComponentPrivate->AlloBuf_mutex);
#else
            OMX_WaitForEvent(&(pComponentPrivate->AlloBuf_event));
#endif
            break;
        }

    PCMD_OMX_MALLOC(pBufferHeader, OMX_BUFFERHEADERTYPE);
    memset((pBufferHeader), 0x0, sizeof(OMX_BUFFERHEADERTYPE));

    /* Needed for cache synchronization between ARM and DSP */
    pBufferHeader->pBuffer = (OMX_U8 *)malloc(nSizeBytes + 256);
    if(NULL == pBufferHeader->pBuffer) {
        PCMDEC_DPRINT(":: Malloc Failed\n");
        goto EXIT;
    }
    PCMDEC_MEMPRINT(":: Malloced = %p\n",pBufferHeader->pBuffer);
    pBufferHeader->nVersion.nVersion = PCMDEC_BUFHEADER_VERSION;


    PCMDEC_DPRINT("********************************************\n");
    PCMDEC_DPRINT(":: Allocated BufHeader %p Buffer = %p, on port %ld\n",
                  pBufferHeader,
                  pBufferHeader->pBuffer, nPortIndex);

    PCMDEC_DPRINT(":: Ip Num = %ld\n",
                  pComponentPrivate->pInputBufferList->numBuffers);
    PCMDEC_DPRINT(":: Op Num = %ld\n",
                  pComponentPrivate->pOutputBufferList->numBuffers);
    PCMDEC_DPRINT("********************************************\n");

    pBufferHeader->pBuffer += 128;

    pBufferHeader->pAppPrivate = pAppPrivate;
    pBufferHeader->pPlatformPrivate = pComponentPrivate;
    pBufferHeader->nAllocLen = nSizeBytes;
    
#ifdef __PERF_INSTRUMENTATION__
    PERF_ReceivedBuffer(pComponentPrivate->pPERF,
                        (*pBuffer)->pBuffer, nSizeBytes,
                        PERF_ModuleMemory);
#endif

    if (nPortIndex == PCMD_INPUT_PORT) {
        pBufferHeader->nInputPortIndex = nPortIndex;
        pBufferHeader->nOutputPortIndex = -1;
        pComponentPrivate->pInputBufferList->pBufHdr[pComponentPrivate->pInputBufferList->numBuffers] = pBufferHeader;
        pComponentPrivate->pInputBufferList->bBufferPending[pComponentPrivate->pInputBufferList->numBuffers] = 0;

        PCMDEC_DPRINT("pComponentPrivate->pInputBufferList->pBufHdr[%ld] = %p\n",
                      pComponentPrivate->pInputBufferList->numBuffers,pComponentPrivate->pInputBufferList->pBufHdr[pComponentPrivate->pInputBufferList->numBuffers]);

        pComponentPrivate->pInputBufferList->bufferOwner[pComponentPrivate->pInputBufferList->numBuffers++] = 1;

        PCMDEC_DPRINT("pComponentPrivate->pInputBufferList->numBuffers = %ld\n",pComponentPrivate->pInputBufferList->numBuffers);
        PCMDEC_DPRINT("pPortDef->nBufferCountMin = %ld\n",pPortDef->nBufferCountMin);

        if (pComponentPrivate->pInputBufferList->numBuffers == pPortDef->nBufferCountActual) {
            pPortDef->bPopulated = 1;
        }
    } else if (nPortIndex == PCMD_OUTPUT_PORT) {
        pBufferHeader->nInputPortIndex = -1;
        pBufferHeader->nOutputPortIndex = nPortIndex;
        pComponentPrivate->pOutputBufferList->pBufHdr[pComponentPrivate->pOutputBufferList->numBuffers] = pBufferHeader;

        pComponentPrivate->pOutputBufferList->bBufferPending[pComponentPrivate->pOutputBufferList->numBuffers] = 0;
        PCMDEC_DPRINT("pComponentPrivate->pOutputBufferList->pBufHdr[%ld] = %p\n",pComponentPrivate->pOutputBufferList->numBuffers,pComponentPrivate->pOutputBufferList->pBufHdr[pComponentPrivate->pOutputBufferList->numBuffers]);

        pComponentPrivate->pOutputBufferList->bufferOwner[pComponentPrivate->pOutputBufferList->numBuffers++] = 1;

        if (pComponentPrivate->pOutputBufferList->numBuffers == pPortDef->nBufferCountActual) {
            pPortDef->bPopulated = 1;
        }

    } else {
        PCMD_OMX_ERROR_EXIT(eError,OMX_ErrorBadPortIndex,"OMX_ErrorBadPortIndex");
    }
    
    if((pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bPopulated == pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bEnabled)&&
       (pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bPopulated == pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bEnabled) &&
       (pComponentPrivate->InLoaded_readytoidle))
        {
            pComponentPrivate->InLoaded_readytoidle = 0;                  
#ifndef UNDER_CE        
            pthread_mutex_lock(&pComponentPrivate->InLoaded_mutex);
            pthread_cond_signal(&pComponentPrivate->InLoaded_threshold);
            pthread_mutex_unlock(&pComponentPrivate->InLoaded_mutex);
#else
            OMX_SignalEvent(&(pComponentPrivate->InLoaded_event));
#endif        
        }


    pBufferHeader->pAppPrivate = pAppPrivate;
    pBufferHeader->pPlatformPrivate = pComponentPrivate;
    pBufferHeader->nAllocLen = nSizeBytes;


    pBufferHeader->nVersion.s.nVersionMajor = PCMDEC_MAJOR_VER;
    pBufferHeader->nVersion.s.nVersionMinor = PCMDEC_MINOR_VER;
    pComponentPrivate->nVersion = pBufferHeader->nVersion.nVersion;
    pBufferHeader->nSize = sizeof(OMX_BUFFERHEADERTYPE);

    PCMDEC_DPRINT("Line\n");
    *pBuffer = pBufferHeader;
    pComponentPrivate->bufAlloced = 1;

    if (pComponentPrivate->bEnableCommandPending && pPortDef->bPopulated) {
        SendCommand (pComponentPrivate->pHandle,
                     OMX_CommandPortEnable,
                     pComponentPrivate->bEnableCommandParam,NULL);
    }

 EXIT:

#if 1
    if(OMX_ErrorNone != eError) {
        PCMDEC_DPRINT("%d :: ************* ERROR: Freeing Other Malloced Resources\n",__LINE__);
        PCMD_OMX_FREE(pBufferHeader->pBuffer);
        PCMD_OMX_FREE(pBufferHeader);
    }
#endif

    return eError;
}

/* ================================================================================= * */
/**
* @fn FreeBuffer() This function frees the meomory of the buffer specified.
*
* @param hComponent  Handle of this component.
*
* @param nPortIndex  Input port or Output port
*
* @param pBuffer  Pointer to the buffer header.
*
* @pre          None
*
* @post        Requested buffer should get the memory allocated.
*
* @return      OMX_ErrorNone = On Success
*              OMX_ErrorBadPortIndex = Bad port index from app
*/
/* ================================================================================ * */
static OMX_ERRORTYPE FreeBuffer(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_U32 nPortIndex,
            OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    PCMDEC_COMPONENT_PRIVATE * pComponentPrivate = NULL;
    OMX_U8* buff;
    int i;
    int inputIndex = -1;
    int outputIndex = -1;
    OMX_COMPONENTTYPE *pHandle;

    PCMDEC_DPRINT ("Entering FreeBuffer\n");

    pComponentPrivate = (PCMDEC_COMPONENT_PRIVATE *) (((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    pHandle = (OMX_COMPONENTTYPE *) pComponentPrivate->pHandle;
    PCMDEC_DPRINT ("pComponentPrivate = %p\n",pComponentPrivate);
    for (i=0; i < MAX_NUM_OF_BUFS; i++) {
        buff = (OMX_U8 *)pComponentPrivate->pInputBufferList->pBufHdr[i];
        if (buff == (OMX_U8 *)pBuffer) {
            PCMDEC_DPRINT("Found matching input buffer\n");
            PCMDEC_DPRINT("buff = %p\n",buff);
            PCMDEC_DPRINT("pBuffer = %p\n",pBuffer);

            inputIndex = i;
            break;
        }
    }

    for (i=0; i < MAX_NUM_OF_BUFS; i++) {
        buff = (OMX_U8 *)pComponentPrivate->pOutputBufferList->pBufHdr[i];
        if (buff == (OMX_U8 *)pBuffer) {
            PCMDEC_DPRINT("Found matching output buffer\n");
            PCMDEC_DPRINT("buff = %p\n",buff);
            PCMDEC_DPRINT("pBuffer = %p\n",pBuffer);
            outputIndex = i;
            break;
        }
    }


    if (inputIndex != -1) {
        if (pComponentPrivate->pInputBufferList->bufferOwner[inputIndex] == 1) {
            buff = pComponentPrivate->pInputBufferList->pBufHdr[inputIndex]->pBuffer;
            if (buff != NULL) {
                buff -= 128;
            }
            PCMDEC_DPRINT("[FREE] %p\n",buff);
            PCMDEC_MEMPRINT("\n: Freeing:  %p IP Buffer\n",buff);
            PCMD_OMX_FREE(buff);
            buff = NULL;
        }
#ifdef __PERF_INSTRUMENTATION__
        PERF_SendingBuffer(pComponentPrivate->pPERF,
                           pComponentPrivate->pInputBufferList->pBufHdr[inputIndex]->pBuffer, 
                           pComponentPrivate->pInputBufferList->pBufHdr[inputIndex]->nAllocLen,
                           (pComponentPrivate->bufAlloced == OMX_TRUE) ?
                           PERF_ModuleMemory :
                           PERF_ModuleHLMM);
#endif
        PCMDEC_MEMPRINT("Freeing: %p IP Buf Header\n\n",
                        pComponentPrivate->pInputBufferList->pBufHdr[inputIndex]);

        PCMD_OMX_FREE(pComponentPrivate->pInputBufferList->pBufHdr[inputIndex]);
        pComponentPrivate->pInputBufferList->pBufHdr[inputIndex] = NULL;
        pComponentPrivate->pInputBufferList->numBuffers--;


        if (pComponentPrivate->pInputBufferList->numBuffers <
            pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->nBufferCountMin) {
            pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bPopulated = OMX_FALSE;
        }
        if(pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bEnabled &&
           pComponentPrivate->bLoadedCommandPending == OMX_FALSE &&
           (pComponentPrivate->curState == OMX_StateIdle ||
            pComponentPrivate->curState == OMX_StateExecuting ||
            pComponentPrivate->curState == OMX_StatePause)) {

            pComponentPrivate->cbInfo.EventHandler(
                                                   pHandle, pHandle->pApplicationPrivate,
                                                   OMX_EventError, OMX_ErrorPortUnpopulated,nPortIndex, NULL);
        }
    } else if (outputIndex != -1) {
        if (pComponentPrivate->pOutputBufferList->bBufferPending[outputIndex]) {
            pComponentPrivate->numPendingBuffers++;
        }
        if (pComponentPrivate->pOutputBufferList->bufferOwner[outputIndex] == 1) {
            buff = pComponentPrivate->pOutputBufferList->pBufHdr[outputIndex]->pBuffer;
            if (buff != NULL) {
                buff -= 128;
            }
            PCMDEC_DPRINT ("FreeBuffer\n");
            PCMDEC_DPRINT("[FREE] %p\n",buff);

            PCMDEC_MEMPRINT("Freeing: %p OP Buffer\n",buff);
            PCMD_OMX_FREE(buff);
            buff = NULL;
        }
#ifdef __PERF_INSTRUMENTATION__
        PERF_SendingBuffer(pComponentPrivate->pPERF,
                           pComponentPrivate->pOutputBufferList->pBufHdr[outputIndex]->pBuffer, 
                           pComponentPrivate->pOutputBufferList->pBufHdr[outputIndex]->nAllocLen,
                           (pComponentPrivate->bufAlloced == OMX_TRUE) ?
                           PERF_ModuleMemory :
                           PERF_ModuleHLMM);
#endif

        PCMDEC_MEMPRINT("Freeing: %p OP Buf Header\n\n",
                        pComponentPrivate->pOutputBufferList->pBufHdr[outputIndex]);

        /*  free(pComponentPrivate->pOutputBufferList->pBufHdr[outputIndex]); */
        PCMD_OMX_FREE(pComponentPrivate->pOutputBufferList->pBufHdr[outputIndex]);
        pComponentPrivate->pOutputBufferList->pBufHdr[outputIndex] = NULL;
        pComponentPrivate->pOutputBufferList->numBuffers--;

        PCMDEC_DPRINT("pComponentPrivate->pOutputBufferList->numBuffers = %ld\n",pComponentPrivate->pOutputBufferList->numBuffers);
        PCMDEC_DPRINT("pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->nBufferCountMin = %ld\n",pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->nBufferCountMin);
        if (pComponentPrivate->pOutputBufferList->numBuffers <
            pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->nBufferCountMin) {

            pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bPopulated = OMX_FALSE;
        }

        if(pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bEnabled &&
           pComponentPrivate->bLoadedCommandPending == OMX_FALSE &&
           (pComponentPrivate->curState == OMX_StateIdle ||
            pComponentPrivate->curState == OMX_StateExecuting ||
            pComponentPrivate->curState == OMX_StatePause)) {


            pComponentPrivate->cbInfo.EventHandler(
                                                   pHandle, pHandle->pApplicationPrivate,
                                                   OMX_EventError, OMX_ErrorPortUnpopulated,nPortIndex, NULL);
        }
    } else {
        PCMDEC_DPRINT("Returning OMX_ErrorBadParameter\n");
        eError = OMX_ErrorBadParameter;
    }

    if ((!pComponentPrivate->pInputBufferList->numBuffers &&
         !pComponentPrivate->pOutputBufferList->numBuffers) &&
        pComponentPrivate->InIdle_goingtoloaded)
        {
            pComponentPrivate->InIdle_goingtoloaded = 0;                  
#ifndef UNDER_CE           
            pthread_mutex_lock(&pComponentPrivate->InIdle_mutex);
            pthread_cond_signal(&pComponentPrivate->InIdle_threshold);
            pthread_mutex_unlock(&pComponentPrivate->InIdle_mutex);
#else
            OMX_SignalEvent(&(pComponentPrivate->InIdle_event));
#endif           
        }


    pComponentPrivate->bufAlloced = 0;
    if (pComponentPrivate->bDisableCommandPending && 
        (pComponentPrivate->pInputBufferList->numBuffers + 
         pComponentPrivate->pOutputBufferList->numBuffers == 0)) {
        SendCommand (pComponentPrivate->pHandle,
                     OMX_CommandPortDisable,
                     pComponentPrivate->bDisableCommandParam,
                     NULL);
    }

    PCMDEC_DPRINT ("Exiting FreeBuffer\n");
    return eError;
}


/* ================================================================================= * */
/**
* @fn UseBuffer() This function is called by application when app allocated the
* memory for the buffer and sends it to application for use of component.
*
* @param hComponent  Handle of this component.
*
* @param ppBufferHdr  Double pointer to the buffer header.
*
* @param nPortIndex  Input port or Output port
*
* @param pAppPrivate Application private data.
*
* @param nSizeBytes Size of the buffer that is to be allocated.
*
* @param pBuffer    Pointer to data buffer which was allocated by the
* application.
*
* @pre          None
*
* @post        None
*
* @return      OMX_ErrorNone = On Success
*              OMX_ErrorBadPortIndex = Bad port index from app
*/
/* ================================================================================ * */
static OMX_ERRORTYPE UseBuffer (
            OMX_IN OMX_HANDLETYPE hComponent,
            OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
            OMX_IN OMX_U32 nPortIndex,
            OMX_IN OMX_PTR pAppPrivate,
            OMX_IN OMX_U32 nSizeBytes,
            OMX_IN OMX_U8* pBuffer)
{
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDef;
    PCMDEC_COMPONENT_PRIVATE *pComponentPrivate;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE *pBufferHeader;

    PCMDEC_DPRINT ("Entering UseBuffer\n");
    PCMDEC_DPRINT ("pBuffer = %p\n",pBuffer);

    pComponentPrivate = (PCMDEC_COMPONENT_PRIVATE *)
        (((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

#ifdef __PERF_INSTRUMENTATION__
    PERF_ReceivedBuffer(pComponentPrivate->pPERF,
                        pBuffer, nSizeBytes,
                        PERF_ModuleHLMM);
#endif

    pPortDef = ((PCMDEC_COMPONENT_PRIVATE*)
                pComponentPrivate)->pPortDef[nPortIndex];
#ifdef _ERROR_PROPAGATION__
    if (pComponentPrivate->curState == OMX_StateInvalid){
        eError = OMX_ErrorInvalidState;
        goto EXIT;
    }
    
#endif                    
    PCMDEC_DPRINT ("pPortDef = %p\n", pPortDef);
    PCMDEC_DPRINT ("pPortDef->bEnabled = %d\n",pPortDef->bEnabled);

    if(!pPortDef->bEnabled) {
        PCMD_OMX_ERROR_EXIT(eError,OMX_ErrorIncorrectStateOperation,
                            "Port is Disabled: OMX_ErrorIncorrectStateOperation");
    }

#if 1
    if(nSizeBytes != pPortDef->nBufferSize || pPortDef->bPopulated) {
        PCMD_OMX_ERROR_EXIT(eError,OMX_ErrorBadParameter,
                            "Bad Size or Port Disabled : OMX_ErrorBadParameter");
    }
#endif

#if 0
    if(pPortDef->bPopulated) {
        PCMD_OMX_ERROR_EXIT(eError,OMX_ErrorBadParameter,
                            "Bad Size or Port Disabled : OMX_ErrorBadParameter");
    }
#endif

    PCMD_OMX_MALLOC(pBufferHeader, OMX_BUFFERHEADERTYPE);

    memset((pBufferHeader), 0x0, sizeof(OMX_BUFFERHEADERTYPE));

    if (nPortIndex == PCMD_OUTPUT_PORT) {
        pBufferHeader->nInputPortIndex = -1;
        pBufferHeader->nOutputPortIndex = nPortIndex;

        pComponentPrivate->pOutputBufferList->pBufHdr[pComponentPrivate->pOutputBufferList->numBuffers] = pBufferHeader;
        pComponentPrivate->pOutputBufferList->bBufferPending[pComponentPrivate->pOutputBufferList->numBuffers] = 0;
        pComponentPrivate->pOutputBufferList->bufferOwner[pComponentPrivate->pOutputBufferList->numBuffers++] = 0;
        if (pComponentPrivate->pOutputBufferList->numBuffers == pPortDef->nBufferCountActual) {
            pPortDef->bPopulated = OMX_TRUE;
        }
    }
    else {
        pBufferHeader->nInputPortIndex = nPortIndex;
        pBufferHeader->nOutputPortIndex = -1;


        pComponentPrivate->pInputBufferList->pBufHdr[pComponentPrivate->pInputBufferList->numBuffers] = pBufferHeader;
        pComponentPrivate->pInputBufferList->bBufferPending[pComponentPrivate->pInputBufferList->numBuffers] = 0;
        pComponentPrivate->pInputBufferList->bufferOwner[pComponentPrivate->pInputBufferList->numBuffers++] = 0;
        if (pComponentPrivate->pInputBufferList->numBuffers == pPortDef->nBufferCountActual) {
            pPortDef->bPopulated = OMX_TRUE;
        }
    }

    if((pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bPopulated == pComponentPrivate->pPortDef[PCMD_OUTPUT_PORT]->bEnabled)&&
       (pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bPopulated == pComponentPrivate->pPortDef[PCMD_INPUT_PORT]->bEnabled) &&
       (pComponentPrivate->InLoaded_readytoidle))
        {
            pComponentPrivate->InLoaded_readytoidle = 0;                  
#ifndef UNDER_CE        
            pthread_mutex_lock(&pComponentPrivate->InLoaded_mutex);
            pthread_cond_signal(&pComponentPrivate->InLoaded_threshold);
            pthread_mutex_unlock(&pComponentPrivate->InLoaded_mutex);
#else
            OMX_SignalEvent(&(pComponentPrivate->InLoaded_event));
#endif        
        }

    pBufferHeader->pAppPrivate = pAppPrivate;
    pBufferHeader->pPlatformPrivate = pComponentPrivate;
    pBufferHeader->nAllocLen = nSizeBytes;

    pBufferHeader->nVersion.s.nVersionMajor = PCMDEC_MAJOR_VER;
    pBufferHeader->nVersion.s.nVersionMinor = PCMDEC_MINOR_VER;
    pComponentPrivate->nVersion = pBufferHeader->nVersion.nVersion;

    pBufferHeader->pBuffer = pBuffer;
    pBufferHeader->nSize = sizeof(OMX_BUFFERHEADERTYPE);
    *ppBufferHdr = pBufferHeader;
    pComponentPrivate->bufAlloced = 1;
    PCMDEC_DPRINT("pBufferHeader = %p\n",pBufferHeader);
    
    if (pComponentPrivate->bEnableCommandPending){
        SendCommand (pComponentPrivate->pHandle,
                     OMX_CommandPortEnable,
                     pComponentPrivate->bEnableCommandParam,NULL);
    }
    
 EXIT:
    return eError;
}

/* ================================================================================= */
/**
* @fn GetExtensionIndex() description for GetExtensionIndex  
GetExtensionIndex().  
Returns index for vendor specific settings.   
*
*  @see         OMX_Core.h
*/
/* ================================================================================ */
static OMX_ERRORTYPE GetExtensionIndex(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_STRING cParameterName,
            OMX_OUT OMX_INDEXTYPE* pIndexType) 
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    PCMDEC_DPRINT("GetExtensionIndex\n");
    if (!(strcmp(cParameterName,"OMX.TI.index.config.pcmheaderinfo"))) {
        *pIndexType = OMX_IndexCustomPcmDecHeaderInfoConfig;
        PCMDEC_DPRINT("OMX_IndexCustomPcmDecHeaderInfoConfig\n");
    }
    else if (!(strcmp(cParameterName,"OMX.TI.index.config.lowlatencyinfo")))
        {
            *pIndexType = OMX_IndexCustomPcmDecLowLatencyConfig;
            PCMDEC_DPRINT("OMX_IndexCustomPcmDecLowLatencyConfig\n");
        }
    else if(!(strcmp(cParameterName,"OMX.TI.index.config.pcmstreamIDinfo"))) 
        {
            *pIndexType = OMX_IndexCustomPcmDecStreamIDConfig;
        
        }
    else if(!(strcmp(cParameterName,"OMX.TI.index.config.pcm.datapath"))) 
        {
            *pIndexType = OMX_IndexCustomPcmDecDataPath;
        }
    else {
        eError = OMX_ErrorBadParameter;
    }

    PCMDEC_DPRINT("Exiting GetExtensionIndex\n");
    return eError;
}
/* ================================================================================ */
static OMX_ERRORTYPE ComponentRoleEnum(
                                       OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_OUT OMX_U8 *cRole,
                                       OMX_IN OMX_U32 nIndex)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    /* This is a non standard component and does not support roles */

    eError = OMX_ErrorNotImplemented;

    return eError;
}

#ifdef UNDER_CE
/* ================================================================================= */
/**
* @fns Sleep replace for WIN CE
*/
/* ================================================================================ */
int OMX_CreateEvent(OMX_Event *event){
    int ret = OMX_ErrorNone;   
    HANDLE createdEvent = NULL;
    if(event == NULL){
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    event->event  = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(event->event == NULL)
        ret = (int)GetLastError();
 EXIT:
    return ret;
}

int OMX_SignalEvent(OMX_Event *event){
    int ret = OMX_ErrorNone;     
    if(event == NULL){
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }     
    SetEvent(event->event);
    ret = (int)GetLastError();
 EXIT:
    return ret;
}

int OMX_WaitForEvent(OMX_Event *event) {
    int ret = OMX_ErrorNone;         
    if(event == NULL){
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }     
    WaitForSingleObject(event->event, INFINITE);    
    ret = (int)GetLastError();
 EXIT:
    return ret;
}

int OMX_DestroyEvent(OMX_Event *event) {
    int ret = OMX_ErrorNone;
    if(event == NULL){
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }  
    CloseHandle(event->event);
 EXIT:    
    return ret;
}
#endif
