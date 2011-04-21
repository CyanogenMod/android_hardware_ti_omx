/*
 * Copyright (c) 2010, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 *  @file  omx_proxy_h264dec.c
 *         This file contains methods that provides the functionality for
 *         the OpenMAX1.1 DOMX Framework Tunnel Proxy component.
 ******************************************************************************
 This is the proxy specific wrapper that passes the component name to the
 generic proxy init()
 The proxy wrapper also does some runtime/static time onfig on per proxy basis
 This is a thin wrapper that is called when componentiit() of the proxy is
 called static OMX_ERRORTYPE PROXY_Wrapper_init(OMX_HANDLETYPE hComponent,
 OMX_PTR pAppData); this layer gets called first whenever a proxy s get handle
 is called
 ******************************************************************************
 *  @path WTSD_DucatiMMSW\omx\omx_il_1_x\omx_proxy_component\src
 *
 *  @rev 1.0
 */

/*==============================================================
 *! Revision History
 *! ============================
 *! 18-june-2010 Dandawate Saket dsaket@ti.com: Initial Version
 *================================================================*/

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "omx_proxy_common.h"
#include <timm_osal_interfaces.h>


#ifdef _Android
#include <utils/Log.h>
#include <PVHeader.h>

#undef LOG_TAG
#define LOG_TAG "OMX_PROXYENC"
#define DOMX_DEBUG LOGE
#endif
#ifdef __H264_ENC__
#define COMPONENT_NAME "OMX.TI.DUCATI1.VIDEO.H264E"    // needs to be specific for every configuration wrapper
#elif __MPEG4_ENC__
#define COMPONENT_NAME "OMX.TI.DUCATI1.VIDEO.MPEG4E"    // needs to be specific for every configuration wrapper
#endif

#define OMX_INIT_STRUCT(_s_, _name_)    \
    memset(&(_s_), 0x0, sizeof(_name_));         \
    (_s_).nSize = sizeof(_name_);               \
    (_s_).nVersion.s.nVersionMajor = 0x1;    \
    (_s_).nVersion.s.nVersionMinor = 0x1;     \
    (_s_).nVersion.s.nRevision = 0x0;            \
    (_s_).nVersion.s.nStep = 0x0


/* Params needed for Dynamic Frame Rate Control*/
#define FRAME_RATE_THRESHOLD 1 /* Change in Frame rate to configure the encoder */
OMX_U32 nFrameRateThreshold = 0;/* Frame Rate threshold for every frame rate update */
int nFrameCounter = 0; /* Number of input frames recieved since last framerate calculation */
OMX_TICKS nVideoTime = 0; /* Video duration since last framerate calculation */
OMX_TICKS nLastFrameRateUpdateTime = 0; /*Time stamp at last frame rate update */
OMX_U16 nBFrames = 0; /* Number of B Frames in H264 Encoder */


#ifdef _OPENCORE
static RPC_OMX_ERRORTYPE ComponentPrivateGetParameters(OMX_IN OMX_HANDLETYPE
    hComponent, OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR pComponentParameterStructure)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    if (nParamIndex == PV_OMX_COMPONENT_CAPABILITY_TYPE_INDEX)
    {
        DOMX_DEBUG("PV_OMX_COMPONENT_CAPABILITY_TYPE_INDEX\n");
        PV_OMXComponentCapabilityFlagsType *pPVCapaFlags =
            (PV_OMXComponentCapabilityFlagsType *)
            pComponentParameterStructure;
        /*Set PV (opencore) capability flags */
        pPVCapaFlags->iIsOMXComponentMultiThreaded = OMX_TRUE;
        pPVCapaFlags->iOMXComponentSupportsExternalOutputBufferAlloc =
            OMX_TRUE;//OMX_FALSE;
        pPVCapaFlags->iOMXComponentSupportsExternalInputBufferAlloc =
            OMX_TRUE;
        pPVCapaFlags->iOMXComponentSupportsMovableInputBuffers =
            OMX_TRUE;
        pPVCapaFlags->iOMXComponentSupportsPartialFrames = OMX_FALSE;
        pPVCapaFlags->iOMXComponentCanHandleIncompleteFrames =
            OMX_FALSE;
#ifdef __H264_ENC__
        pPVCapaFlags->iOMXComponentUsesNALStartCodes = OMX_TRUE;
#else
        pPVCapaFlags->iOMXComponentUsesNALStartCodes = OMX_FALSE;
#endif
        pPVCapaFlags->iOMXComponentUsesFullAVCFrames = OMX_TRUE;
        return OMX_ErrorNone;
    }
    return PROXY_GetParameter(hComponent, nParamIndex,
        pComponentParameterStructure);
}
#endif

static OMX_ERRORTYPE OMX_GetOMXComponentName(OMX_COMPONENTTYPE *hComp, char * compName)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_STRING OMX_ComponentName ;
    OMX_VERSIONTYPE OMX_Version,OMX_SpecVersion;
    OMX_UUIDTYPE OMX_UUID;
    OMX_ComponentName = malloc(128);
    eError = hComp->GetComponentVersion(hComp,OMX_ComponentName,&OMX_Version,&OMX_SpecVersion,&OMX_UUID);
    if ( eError == OMX_ErrorNone)
         strcpy( compName, OMX_ComponentName);
    return eError;
}

static OMX_ERRORTYPE OMX_ConfigureDynamicPFramesInH264E( OMX_HANDLETYPE hComponent,
    OMX_BUFFERHEADERTYPE * pBufferHdr,  OMX_U32 nCurrentFrameRate,  OMX_U32 nTargetFrameRate)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 remainder = 0;
    OMX_U16 nTargetPFrames =0; /* Target Pframes to be provided to Encoder */
    OMX_U16 nCurrentPFrames = 0; /* Current pFrame Value configured to  Encoder */
    OMX_VIDEO_CONFIG_AVCINTRAPERIOD tPframeH264e;
    OMX_VIDEO_PARAM_AVCTYPE h264type;
    OMX_COMPONENTTYPE *pHandle;
    if (hComponent == NULL){
        DOMX_ERROR("Component is invalid/ not present ");
            return OMX_ErrorBadParameter;
    }
    pHandle = (OMX_COMPONENTTYPE *) hComponent;

    /* Initialise the OMX structures */
    OMX_INIT_STRUCT(tPframeH264e, OMX_VIDEO_CONFIG_AVCINTRAPERIOD);
    OMX_INIT_STRUCT(h264type,OMX_VIDEO_PARAM_AVCTYPE);

    /* Read number of B Frames if not read yet */
    if(!nBFrames){
        h264type.nPortIndex = 1;
        eError = pHandle->GetParameter(hComponent,OMX_IndexParamVideoAvc,&h264type);
        if(eError != OMX_ErrorNone) {
            DOMX_ERROR(" Error while reading component info = %d",eError);
            return eError;
        }
        nBFrames = h264type.nBFrames;
    }

    /* Read Current pFrames set in Encoder */
    tPframeH264e.nPortIndex = 1;
    eError = pHandle->GetConfig(hComponent,OMX_IndexConfigVideoAVCIntraPeriod,&tPframeH264e);
    if(eError != OMX_ErrorNone) {
        DOMX_ERROR(" Error while Getting PFrame info, Error code = %d",eError);
        return eError;
    }
    nCurrentPFrames = tPframeH264e.nPFrames;

    /* Calculate Target P Frames */
    nTargetPFrames = (nCurrentPFrames * nTargetFrameRate) /nCurrentFrameRate;
    /* Number of pFrames should be multiple of (B FRAMES + 1) */
    remainder = nTargetPFrames % (nBFrames + 1);
    nTargetPFrames = nTargetPFrames - remainder;
    if(nTargetPFrames == nCurrentPFrames){
        DOMX_DEBUG(" No Change in P Frames, No Update required");
        return OMX_ErrorNone;
    }

    /* Update the new PFrames to the Encoder */
    tPframeH264e.nPFrames = nTargetPFrames;
    eError = pHandle->SetConfig(hComponent,OMX_IndexConfigVideoAVCIntraPeriod,&tPframeH264e);
    if(eError != OMX_ErrorNone) {
        DOMX_ERROR(" Error while setting PFrame info, Error code = %d",eError);
    }
    return eError;
}

static OMX_ERRORTYPE OMX_ConfigureDynamicFrameRate( OMX_HANDLETYPE hComponent,
    OMX_BUFFERHEADERTYPE * pBufferHdr, OMX_STRING hCompName)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 nTargetFrameRate = 0; /* Target Frame Rate to be provided to Encoder */
    OMX_U32 nCurrentFrameRate = 0; /* Current Frame Rate currently set  in Encoder */
    OMX_CONFIG_FRAMERATETYPE tFrameRate;
    OMX_COMPONENTTYPE *pHandle;
    if (hComponent == NULL){
        DOMX_ERROR("Component is invalid/ not present ");
        return OMX_ErrorBadParameter;
    }
    pHandle = (OMX_COMPONENTTYPE *) hComponent;

    /* Initialise the OMX structures */
    OMX_INIT_STRUCT(tFrameRate,OMX_CONFIG_FRAMERATETYPE);

    /* Intialise nLastFrameRateUpdateTime for the 1st frame */
    if((!nFrameCounter) && (!nLastFrameRateUpdateTime)){
        nLastFrameRateUpdateTime = pBufferHdr-> nTimeStamp;
    }

    /* Increment the Frame Counter and Calculate Frame Rate*/
    nFrameCounter++;
    nVideoTime = pBufferHdr->nTimeStamp - nLastFrameRateUpdateTime;

    if(nVideoTime <= 0) {
        return OMX_ErrorBadParameter;
    }

    /*Get Port Frame Rate if not read yet*/
    if(!nFrameRateThreshold) {
        tFrameRate.nPortIndex = 1; /* Output Port Index */
        /* Read Current FrameRate */
        eError = pHandle->GetConfig(hComponent,OMX_IndexConfigVideoFramerate,&tFrameRate);
        nFrameRateThreshold = tFrameRate.xEncodeFramerate >>16;
        DOMX_DEBUG(" Port Frame Rate is %d ", nFrameRateThreshold);
    }
    nCurrentFrameRate = nFrameRateThreshold;

    /* If Number of frames is less than the Threshold
     *  Frame Rate udpate is not necessary
     */
    if(nFrameCounter < nFrameRateThreshold){
            DOMX_EXIT(" Threshold not reached, no update necessary");
            return OMX_ErrorNone;
    }

    /*Calculate the new target Frame Rate*/
    nTargetFrameRate = nFrameCounter * 1000000 / nVideoTime;

    /* Difference in Frame Rate is more than Threshold - Only then update Frame Rate*/
    if((( (OMX_S32)nTargetFrameRate) -((OMX_S32) nCurrentFrameRate) >= FRAME_RATE_THRESHOLD) ||
        (((OMX_S32) nCurrentFrameRate) - ( (OMX_S32)nTargetFrameRate) >= FRAME_RATE_THRESHOLD)) {

        /* Update pFrames in case of H264 Encoder.
          * Currently IDR Frames in H264E is 0.
          * In OMX H264E, for dynamic pFrmae update, IDR is expected to be 1.
          * Disabling the pFrame Update till fix is available from Ducati Team
          */
        #if 0
        if (! strcmp(hCompName,"OMX.TI.DUCATI1.VIDEO.H264E")) {
            eError = OMX_ConfigureDynamicPFramesInH264E(hComponent,pBufferHdr, nCurrentFrameRate, nTargetFrameRate);
            if(eError != OMX_ErrorNone){
                return eError;
            }
        }
        #endif
        /* Now Send the new Frame Rate */
        tFrameRate.nPortIndex = 1; /* Output Port Index */
        tFrameRate.xEncodeFramerate = (OMX_U32)(nTargetFrameRate * (1 << 16));
        eError = pHandle->SetConfig(hComponent,OMX_IndexConfigVideoFramerate,&tFrameRate);
        if(eError != OMX_ErrorNone)    {
            DOMX_ERROR(" Error while configuring Dynamic Frame Rate,Error info = %d",eError);
            return eError;
        }
        nFrameRateThreshold = nTargetFrameRate; /*Update the threshold */
    }
    /* reset all params */
    nFrameCounter = 0 ;
    nVideoTime = 0;
    nLastFrameRateUpdateTime = pBufferHdr->nTimeStamp;
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE ComponentrPrivateEmptyThisBuffer(OMX_HANDLETYPE hComponent,
    OMX_BUFFERHEADERTYPE * pBufferHdr)
{
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
    OMX_STRING hCompName ;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    hCompName = malloc(128);

    /* Get the name of OMX Component */
    eError = OMX_GetOMXComponentName(hComp, hCompName);
    if (eError != OMX_ErrorNone){
        DOMX_ERROR(" Error while Reading Component Name");
        return eError;
    }

    /* For Video Encoder, configure FrameRate Dynamically */
    if((!strcmp(hCompName,"OMX.TI.DUCATI1.VIDEO.H264E") )||
        (!strcmp(hCompName,"OMX.TI.DUCATI1.VIDEO.MPEG4E"))) {
        OMX_ERRORTYPE eError = OMX_ConfigureDynamicFrameRate(hComponent, pBufferHdr, hCompName);
        if( eError != OMX_ErrorNone)
            DOMX_ERROR(" Error while configuring FrameRate Dynamically.Error  info = %d",eError);
    }
    DOMX_DEBUG("Redirection from ComponentPricateEmptyThisBuffer to PROXY_EmptyThisBuffer");
    return PROXY_EmptyThisBuffer( hComponent,pBufferHdr);
}

OMX_ERRORTYPE OMX_ComponentInit(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = NULL;
    PROXY_COMPONENT_PRIVATE *pComponentPrivate;
    pHandle = (OMX_COMPONENTTYPE *) hComponent;

    DOMX_DEBUG
        ("\n_____________________INSISDE VIDEO ENCODER PROXY WRAPPER__________________________\n");

    pHandle->pComponentPrivate =
        (PROXY_COMPONENT_PRIVATE *)
        TIMM_OSAL_Malloc(sizeof(PROXY_COMPONENT_PRIVATE), TIMM_OSAL_TRUE,
        0, TIMMOSAL_MEM_SEGMENT_INT);

    pComponentPrivate =
        (PROXY_COMPONENT_PRIVATE *) pHandle->pComponentPrivate;
    if (pHandle->pComponentPrivate == NULL)
    {
        DOMX_DEBUG
            ("\n ERROR IN ALLOCATING PROXY COMPONENT PRIVATE STRUCTURE");
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    pComponentPrivate->cCompName =
        (OMX_U8 *) TIMM_OSAL_Malloc(MAX_COMPONENT_NAME_LENGTH *
        sizeof(OMX_U8), TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_INT);
    if (pComponentPrivate->cCompName == NULL)
    {
        DOMX_DEBUG
            ("\n ERROR IN ALLOCATING PROXY COMPONENT NAME STRUCTURE");
        TIMM_OSAL_Free(pComponentPrivate);
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    // Copying component Name - this will be picked up in the proxy common
    assert(strlen(COMPONENT_NAME) + 1 < MAX_COMPONENT_NAME_LENGTH);
    TIMM_OSAL_Memcpy(pComponentPrivate->cCompName, COMPONENT_NAME,
        strlen(COMPONENT_NAME) + 1);
    eError = OMX_ProxyCommonInit(hComponent);    // Calling Proxy Common Init()

    if (eError != OMX_ErrorNone)
    {
        DOMX_DEBUG("\nError in Initializing Proxy");
        TIMM_OSAL_Free(pComponentPrivate->cCompName);
        TIMM_OSAL_Free(pComponentPrivate);
    }
#ifdef _OPENCORE
    // Make sure private function to component is always assigned
    // after component init.
    pHandle->GetParameter = ComponentPrivateGetParameters;
#endif
    pHandle->EmptyThisBuffer = ComponentrPrivateEmptyThisBuffer;
    EXIT:
    return eError;
}
