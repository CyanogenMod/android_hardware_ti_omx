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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
/*=========================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 *  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied.
 * ==========================================================================*/

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
#define COMPONENT_NAME "OMX.TI.DUCATI1.VIDEO.H264E" // needs to be specific for every configuration wrapper
#elif __MPEG4_ENC__
#define COMPONENT_NAME "OMX.TI.DUCATI1.VIDEO.MPEG4E" // needs to be specific for every configuration wrapper
#endif
#ifdef _OPENCORE
static RPC_OMX_ERRORTYPE ComponentPrivateGetParameters(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_INDEXTYPE nParamIndex,
        OMX_INOUT OMX_PTR pComponentParameterStructure)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    if ( nParamIndex == PV_OMX_COMPONENT_CAPABILITY_TYPE_INDEX){
    DOMX_DEBUG("PV_OMX_COMPONENT_CAPABILITY_TYPE_INDEX\n");
    PV_OMXComponentCapabilityFlagsType* pPVCapaFlags =
    (PV_OMXComponentCapabilityFlagsType*)pComponentParameterStructure;
    /*Set PV (opencore) capability flags*/
    pPVCapaFlags->iIsOMXComponentMultiThreaded = OMX_TRUE;
    pPVCapaFlags->iOMXComponentSupportsExternalOutputBufferAlloc = OMX_FALSE;
    pPVCapaFlags->iOMXComponentSupportsExternalInputBufferAlloc = OMX_TRUE;
    pPVCapaFlags->iOMXComponentSupportsMovableInputBuffers = OMX_TRUE;
    pPVCapaFlags->iOMXComponentSupportsPartialFrames = OMX_FALSE;
    pPVCapaFlags->iOMXComponentCanHandleIncompleteFrames = OMX_FALSE;
#ifdef __H264_ENC__
    pPVCapaFlags->iOMXComponentUsesNALStartCodes = OMX_TRUE;
#else
    pPVCapaFlags->iOMXComponentUsesNALStartCodes = OMX_FALSE;
#endif
    pPVCapaFlags->iOMXComponentUsesFullAVCFrames = OMX_TRUE;
    return OMX_ErrorNone;
    }
return  PROXY_GetParameter(hComponent,nParamIndex,pComponentParameterStructure);
}
#endif

OMX_ERRORTYPE OMX_ComponentInit (OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = NULL;
    PROXY_COMPONENT_PRIVATE *pComponentPrivate;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    DOMX_DEBUG("\n_____________________INSISDE VIDEO ENCODER PROXY WRAPPER__________________________\n");

    pHandle->pComponentPrivate = \
            (PROXY_COMPONENT_PRIVATE *)TIMM_OSAL_Malloc( \
                                sizeof(PROXY_COMPONENT_PRIVATE), \
                                TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_INT);

    pComponentPrivate = (PROXY_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;
    if (pHandle->pComponentPrivate == NULL) {
        DOMX_DEBUG("\n ERROR IN ALLOCATING PROXY COMPONENT PRIVATE STRUCTURE");
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    pComponentPrivate->cCompName = (OMX_U8 *)TIMM_OSAL_Malloc( \
                            MAX_COMPONENT_NAME_LENGTH*sizeof(OMX_U8), \
                            TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_INT);
    if (pComponentPrivate->cCompName == NULL) {
        DOMX_DEBUG("\n ERROR IN ALLOCATING PROXY COMPONENT NAME STRUCTURE");
        TIMM_OSAL_Free(pComponentPrivate);
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    // Copying component Name - this will be picked up in the proxy common
    assert(strlen(COMPONENT_NAME)+1 < MAX_COMPONENT_NAME_LENGTH);
    TIMM_OSAL_Memcpy(pComponentPrivate->cCompName,COMPONENT_NAME, \
                            strlen(COMPONENT_NAME)+1);
    eError = OMX_ProxyCommonInit(hComponent);// Calling Proxy Common Init()

    if(eError != OMX_ErrorNone){
        DOMX_DEBUG("\nError in Initializing Proxy");
        TIMM_OSAL_Free(pComponentPrivate->cCompName);
        TIMM_OSAL_Free(pComponentPrivate);
        }

#ifdef _OPENCORE
    // Make sure private function to component is always assigned
    // after component init.
    pHandle->GetParameter = ComponentPrivateGetParameters;
#endif

    EXIT:
        return eError;
}

