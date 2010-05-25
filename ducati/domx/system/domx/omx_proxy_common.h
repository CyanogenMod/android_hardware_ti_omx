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
/*=========================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 *  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied.
 * ==========================================================================*/

/**
 *  @file  omx_proxy_common.h
 *         This file contains methods that provides the functionality for
 *         the OpenMAX1.1 DOMX Framework OMX Common Proxy.
 *
 *  @path \WTSD_DucatiMMSW\framework\domx\omx_proxy_common\ 
 *
 *  @rev 1.0
 */

/*==============================================================
 *! Revision History
 *! ============================
 *! 29-Mar-2010 Abhishek Ranka : Revamped DOMX implementation
 *!
 *! 19-August-2009 B Ravi Kiran ravi.kiran@ti.com: Initial Version
 *================================================================*/
 
#ifndef OMX_PROXY_H
#define OMX_PROXY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ------compilation control switches ----------------------------------------*/

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
/* ----- system and platform files ----------------------------*/ 
#include <OMX_Core.h>
/*-------program files ----------------------------------------*/
#include "omx_rpc.h"
#include "omx_rpc_internal.h"

/****************************************************************
 * PUBLIC DECLARATIONS Defined here, used elsewhere
 ****************************************************************/
/*--------data declarations -----------------------------------*/
#define MAX_NUM_PROXY_BUFFERS             25
#define MAX_COMPONENT_NAME_LENGTH         128
#define PROXY_MAXNUMOFPORTS               8

/******************************************************************
 *   MACROS - ASSERTS
 ******************************************************************/
#define PROXY_assert  PROXY_paramCheck
#define PROXY_require PROXY_paramCheck
#define PROXY_ensure  PROXY_paramCheck

#define PROXY_paramCheck(C, V, S) do {\
    if (!(C)) { eError = V;\
    TIMM_OSAL_TraceFunction("##Error:: %s::in %s::line %d \n",S,__FUNCTION__, __LINE__);\
    goto EXIT; }\
    } while(0)


typedef OMX_ERRORTYPE (*PROXY_EMPTYBUFFER_DONE)(OMX_HANDLETYPE hComponent, OMX_U32 remoteBufHdr,
                                                 OMX_U32 nfilledLen, OMX_U32 nOffset,
                                                 OMX_U32 nFlags);
                                                 
typedef OMX_ERRORTYPE (*PROXY_FILLBUFFER_DONE)(OMX_HANDLETYPE hComponent, OMX_U32 remoteBufHdr,
                                                OMX_U32 nfilledLen, OMX_U32 nOffset,
                                                OMX_U32 nFlags, OMX_TICKS nTimeStamp);

typedef OMX_ERRORTYPE (*PROXY_EVENTHANDLER)(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
                                             OMX_EVENTTYPE eEvent, OMX_U32 nData1,
                                             OMX_U32 nData2, OMX_PTR pEventData);

/*******************************************************************************
* Structures
*******************************************************************************/
/*===============================================================*/
/** PROXY_BUFFER_INFO        : This structure maintains a table of A9 and 
 *                             Ducati side buffers and headers.
 *
 * @param pBufHeader         : This is a pointer to the A9 bufferheader.
 *
 * @param pBufHeaderRemote   : This is pointer to Ducati side bufferheader.
 *
 * @param pBufferMapped      : This is the Ducati side buffer.
 *
 * @param pBufferActual      : This is the actual buffer sent by the client.
 *
 * @param actualContent      : Unknown. Remove?
 *
 * @param pAlloc_localBuffCopy : Unknown. Remove?
 *
 * @param pBufferToBeMapped  : This is the pointer that will be used for 
 *                             mapping the buffer to Ducati side. For TILER
 *                             buffers, this and pBufferActual will  be the
 *                             same. However for NON TILER buffers, this'll
 *                             be an intermediate pointer. This pointer should 
 *                             not be used for r/w or cache operations. It can
 *                             only be used for mapping/unmapping to Ducati 
 *                             space.
 * @param bRemoteAllocatedBuffer : True if buffer is allocated by remote core
 *                                 (as in AllocateBuffer case). This is needed
 *                                 to maintain context since in this case the
 *                                 buffer needs to be unmapped during FreeBuffer
 */
/*===============================================================*/
typedef struct PROXY_BUFFER_INFO{
    OMX_BUFFERHEADERTYPE* pBufHeader;
    OMX_U32 pBufHeaderRemote;
    OMX_U32 pBufferMapped;
    OMX_U32 pBufferActual;
    OMX_U32 actualContent;
    OMX_U32 pAlloc_localBuffCopy;
    OMX_U32 pBufferToBeMapped;
    OMX_BOOL bRemoteAllocatedBuffer;
}PROXY_BUFFER_INFO;

/* ========================================================================== */
/**
* PROXY_COMPONENT_PRIVATE
*
*/
/* ========================================================================== */
typedef struct PROXY_COMPONENT_PRIVATE {
    /* OMX Related Information */
    OMX_CALLBACKTYPE tCBFunc;
    OMX_PTR pILAppData;
    RPC_OMX_HANDLE hRemoteComp; 
    
    PROXY_BUFFER_INFO tBufList[MAX_NUM_PROXY_BUFFERS];
    OMX_U32 nTotalBuffers;
    OMX_U32 nAllocatedBuffers;
    
    /* PROXY specific data - PROXY PRIVATE DATA */
    char *cCompName;
    
    PROXY_EMPTYBUFFER_DONE proxyEmptyBufferDone;
    PROXY_FILLBUFFER_DONE proxyFillBufferDone;
    PROXY_EVENTHANDLER proxyEventHandler;
    
    OMX_U32 nNumOfLines[PROXY_MAXNUMOFPORTS];
}PROXY_COMPONENT_PRIVATE;


/*******************************************************************************
* Functions
*******************************************************************************/
OMX_ERRORTYPE OMX_ProxyCommonInit(OMX_HANDLETYPE hComponent);
OMX_ERRORTYPE PROXY_GetParameter(OMX_IN  OMX_HANDLETYPE hComponent,OMX_IN  OMX_INDEXTYPE nParamIndex,OMX_INOUT OMX_PTR pParamStruct);

#endif
