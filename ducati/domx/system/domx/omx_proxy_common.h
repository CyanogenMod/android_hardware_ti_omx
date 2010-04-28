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

#define PROXY_paramCheck(C,V,S)  if (!(C)) { eError = V;\
TIMM_OSAL_TraceFunction("##Error:: %s::in %s::line %d \n",S,__FUNCTION__, __LINE__); \
goto EXIT; }


typedef OMX_ERRORTYPE (*PROXY_EMPTYBUFFER_DONE)(OMX_HANDLETYPE hComponent, OMX_U32 remoteBufHdr,
                                                 OMX_U32 nfilledLen, OMX_U32 nOffset,
                                                 OMX_U32 nFlags);

typedef OMX_ERRORTYPE (*PROXY_FILLBUFFER_DONE)(OMX_HANDLETYPE hComponent, OMX_U32 remoteBufHdr,
                                                OMX_U32 nfilledLen, OMX_U32 nOffset,
                                                OMX_U32 nFlags, OMX_TICKS nTimeStamp);

typedef OMX_ERRORTYPE (*PROXY_EVENTHANDLER)(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
                                             OMX_EVENTTYPE eEvent, OMX_U32 nData1,
                                             OMX_U32 nData2, OMX_PTR pEventData);
#ifdef _Android
typedef RPC_OMX_ERRORTYPE (*COMPONENTPRIVATE_GetParameter)(OMX_IN  OMX_HANDLETYPE hComponent,
                                              OMX_IN  OMX_INDEXTYPE nParamIndex,
                                              OMX_INOUT OMX_PTR pComponentParameterStructure);
#endif
/*******************************************************************************
* Structures
*******************************************************************************/
/* ========================================================================== */
/**
* PROXY_BUFFER_INFO
*
*/
/* ========================================================================== */
typedef struct PROXY_BUFFER_INFO{
	OMX_BUFFERHEADERTYPE* pBufHeader;
	OMX_U32 pBufHeaderRemote;
	OMX_U32 pBufferMapped;
	OMX_U32 pBufferActual;
	OMX_U32 actualContent;
	OMX_U32 pAlloc_localBuffCopy;
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
    OMX_U32 nNumOfBuffers;

	/* PROXY specific data - PROXY PRIVATE DATA */
       char *cCompName;

	PROXY_EMPTYBUFFER_DONE proxyEmptyBufferDone;
	PROXY_FILLBUFFER_DONE proxyFillBufferDone;
	PROXY_EVENTHANDLER proxyEventHandler;
        OMX_U32 nNumOfLines[PROXY_MAXNUMOFPORTS];
#ifdef _Android
	COMPONENTPRIVATE_GetParameter componentprivateGetParameters;
#endif
}PROXY_COMPONENT_PRIVATE;


/*******************************************************************************
* Functions
*******************************************************************************/
OMX_ERRORTYPE OMX_ProxyCommonInit(OMX_HANDLETYPE hComponent);

#endif
