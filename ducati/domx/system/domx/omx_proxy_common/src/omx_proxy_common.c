/*=========================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 *  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied.
 * ==========================================================================*/

/**
 *  @file  omx_proxy_common.c
 *         This file contains methods that provides the functionality for
 *         the OpenMAX1.1 DOMX Framework OMX Common Proxy .
 *
 *  @path \WTSD_DucatiMMSW\framework\domx\omx_proxy_common\src
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

/* ------compilation control switches ----------------------------------------*/
#define TILER_BUFF

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
/* ----- system and platform files ----------------------------*/
//#include <stdio.h>
//#include <stdlib.h>
#include "timm_osal_memory.h"
#include "OMX_TI_Common.h"
#include "OMX_TI_Index.h"
/*-------program files ----------------------------------------*/
#include "omx_proxy_common.h"
#include "omx_rpc.h"
#include "omx_rpc_stub.h"
#include "omx_rpc_utils.h"
#ifdef _Android
#include <utils/Log.h>
#undef LOG_TAG
#define LOG_TAG "OMX_PROXYCOMMON"
//#define DOMX_DEBUG LOGD
#endif
#ifdef TILER_BUFF
#include <ProcMgr.h>
#include <SysLinkMemUtils.h>
#include <mem_types.h>
#include <phase1_d2c_remap.h>
#include <memmgr.h>
#endif

#ifdef TILER_BUFF
#define PortFormatIsNotYUV 0
static OMX_ERRORTYPE RPC_PrepareBuffer_Remote(PROXY_COMPONENT_PRIVATE *pCompPrv, OMX_COMPONENTTYPE *hRemoteComp, OMX_U32 nPortIndex, OMX_U32 nSizeBytes, OMX_BUFFERHEADERTYPE *pChironBuf, OMX_BUFFERHEADERTYPE *pDucBuf);
static OMX_ERRORTYPE RPC_PrepareBuffer_Chiron(PROXY_COMPONENT_PRIVATE *pCompPrv, OMX_COMPONENTTYPE *hRemoteComp, OMX_U32 nPortIndex, OMX_U32 nSizeBytes, OMX_BUFFERHEADERTYPE *pDucBuf, OMX_BUFFERHEADERTYPE *pChironBuf);
static OMX_ERRORTYPE RPC_UTIL_GetNumLines(OMX_COMPONENTTYPE *hComp, OMX_U32 nPortIndex, OMX_U32 * nNumOfLines);
static OMX_ERRORTYPE RPC_MapBuffer_Ducati(OMX_U8 *pBuf, OMX_U32 nBufLineSize, OMX_U32 nBufLines, OMX_U8 **pMappedBuf);
#endif

/* ===========================================================================*/
/**
 * @name PROXY_EventHandler()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_EventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
                                        OMX_EVENTTYPE eEvent, OMX_U32 nData1,
                                        OMX_U32 nData2, OMX_PTR pEventData)
{
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  PROXY_COMPONENT_PRIVATE* pCompPrv;
  OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *)hComponent;

  OMX_U16 count;
  OMX_BUFFERHEADERTYPE * pLocalBufHdr = NULL;

  DOMX_DEBUG("\nEntered Proxy event handler__________________________________________PROXY EH");

  PROXY_assert((hComp->pComponentPrivate != NULL),
                OMX_ErrorBadParameter, NULL);

  pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

  DOMX_DEBUG("hComponent:0x%x,hComp->pApplicationPrivate: 0x%x,eEvent: 0x%x,nData1: 0x%x,nData2: 0x%x,pEventData: 0x%x\n",
               hComponent,pCompPrv->pILAppData,eEvent,nData1,nData2,pEventData);

  switch(eEvent)
{

    case OMX_TI_EventBufferRefCount:
          DOMX_DEBUG("\nReceived Ref Count Event");
          /*nData1 will be pBufferHeader, nData2 will be present count. Need to find local
          buffer header for nData1 which is remote buffer header*/

          PROXY_assert((nData1 != 0), OMX_ErrorBadParameter,
          "Received NULL buffer header from OMX component");

          /*find local buffer header equivalent*/
          for(count = 0; count<pCompPrv->nNumOfBuffers;++count)
	{
            if(pCompPrv->tBufList[count].pBufHeaderRemote == nData1)
		{
                pLocalBufHdr = pCompPrv->tBufList[count].pBufHeader;
                pLocalBufHdr->pBuffer = (OMX_U8 *)pCompPrv->tBufList[count].pBufferActual;
			break;
		}
	}
          PROXY_assert((count != pCompPrv->nNumOfBuffers), OMX_ErrorBadParameter,
          "Received invalid-buffer header from OMX component");

          /*update local buffer header*/
          nData1 = pLocalBufHdr;
          break;

    default:
          break;
}

  pCompPrv->tCBFunc.EventHandler(hComponent,pCompPrv->pILAppData,eEvent,nData1,nData2,pEventData);

EXIT:
  DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
	return OMX_ErrorNone;
}

/* ===========================================================================*/
/**
 * @name PROXY_FillBufferDone()
 * @brief
 * @param
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_EmptyBufferDone(OMX_HANDLETYPE hComponent, OMX_U32 remoteBufHdr,
                                           OMX_U32 nfilledLen, OMX_U32 nOffset,
                                           OMX_U32 nFlags)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    PROXY_COMPONENT_PRIVATE* pCompPrv;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *)hComponent;

	OMX_U16 count;
	OMX_BUFFERHEADERTYPE * pBufHdr = NULL;

	DOMX_DEBUG("\n%s Entered",__FUNCTION__);

    PROXY_assert((hComp->pComponentPrivate != NULL),
                 OMX_ErrorBadParameter, NULL);

    pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

    for(count = 0; count<pCompPrv->nNumOfBuffers;++count)
	{
        if(pCompPrv->tBufList[count].pBufHeaderRemote == remoteBufHdr)
		{

            pBufHdr = pCompPrv->tBufList[count].pBufHeader;
			pBufHdr->nFilledLen = nfilledLen;
			pBufHdr->nOffset = nOffset;
			pBufHdr->nFlags = nFlags;
            pBufHdr->pBuffer = (OMX_U8 *)pCompPrv->tBufList[count].pBufferActual;
            pCompPrv->tCBFunc.EmptyBufferDone(hComponent,pCompPrv->pILAppData,pBufHdr);
			break;
		}
	}

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
	return OMX_ErrorNone;
}

/* ===========================================================================*/
/**
 * @name PROXY_FillBufferDone()
 * @brief
 * @param
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_FillBufferDone(OMX_HANDLETYPE hComponent, OMX_U32 remoteBufHdr,
                                          OMX_U32 nfilledLen, OMX_U32 nOffset,
                                          OMX_U32 nFlags, OMX_TICKS nTimeStamp)
{

    OMX_ERRORTYPE eError = OMX_ErrorNone;
    PROXY_COMPONENT_PRIVATE* pCompPrv;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *)hComponent;

    OMX_U16 count;
    OMX_BUFFERHEADERTYPE * pBufHdr = NULL;

    DOMX_DEBUG("\n%s Entered",__FUNCTION__);

    PROXY_assert((hComp->pComponentPrivate != NULL),
                 OMX_ErrorBadParameter, NULL);

    pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

    for(count = 0; count<pCompPrv->nNumOfBuffers;++count)
    {
        if(pCompPrv->tBufList[count].pBufHeaderRemote == remoteBufHdr)
        {
            pBufHdr = pCompPrv->tBufList[count].pBufHeader;
            pBufHdr->nFilledLen = nfilledLen;
            pBufHdr->nOffset = nOffset;
            pBufHdr->nFlags = nFlags;
            pBufHdr->pBuffer = (OMX_U8 *)pCompPrv->tBufList[count].pBufferActual;
            pBufHdr->nTimeStamp = nTimeStamp;
            pCompPrv->tCBFunc.FillBufferDone(hComponent,pCompPrv->pILAppData,pBufHdr);
            break;
	}
	}

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
	return OMX_ErrorNone;
}

/* ===========================================================================*/
/**
 * @name PROXY_EmptyThisBuffer()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_EmptyThisBuffer(OMX_HANDLETYPE hComponent,
                                           OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn;
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    PROXY_COMPONENT_PRIVATE* pCompPrv;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *)hComponent;

    OMX_U32 count=0;
    OMX_U8 isMatchFound = 0;
    OMX_U8 *pBuffer=NULL;

    DOMX_DEBUG("\n%s Entered",__FUNCTION__);

    PROXY_assert((pBufferHdr != NULL) && (hComp->pComponentPrivate != NULL) &&
                 (hComp->pComponentPrivate != NULL),
                 OMX_ErrorBadParameter, NULL);

    pCompPrv=(PROXY_COMPONENT_PRIVATE*)hComp->pComponentPrivate;

    pBuffer = pBufferHdr->pBuffer;

    DOMX_DEBUG("\n%s:  pBufferHdr->pBuffer : 0x%x, pBufferHdr->nFilledLen : %d ",__FUNCTION__,pBufferHdr->pBuffer,pBufferHdr->nFilledLen);

    /*First find the index of this buffer header to retrieve remote buffer header */
    for(count=0;count<pCompPrv->nNumOfBuffers;count++)
	{
        if(pCompPrv->tBufList[count].pBufHeader == pBufferHdr)
	    {
            DOMX_DEBUG("%s: Buffer Index of Match %d \n",__FUNCTION__,count);
            isMatchFound = 1;
            break;
	    }
	 }
    if(!isMatchFound)
	{
        DOMX_DEBUG("\n%s: Could not find the remote header in buffer list",__FUNCTION__);
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }

    /*[NPA] If the buffer is modified in buffer header, force remap.
    TBD: Even if MODIFIED is set, the pBuffer can be a pre-mapped buffer
    */
    if((pBufferHdr->nFlags)&(OMX_BUFFERHEADERFLAG_MODIFIED)){

        /* Same pBufferHdr will get updated with remote pBuffer and pAuxBuf1 if a 2D buffer */
        eError=RPC_PrepareBuffer_Remote(pCompPrv, pCompPrv->hRemoteComp,pBufferHdr->nInputPortIndex,
                                        pBufferHdr->nAllocLen,
                                        pBufferHdr, NULL);

        if(eError != OMX_ErrorNone) {
            PROXY_assert(0, OMX_ErrorUndefined,
                         "Unable to map buffer");
	}

        /*Now update the buffer list with new details*/
        pCompPrv->tBufList[count].pBufferMapped = (OMX_U32)(pBufferHdr->pBuffer);
        pCompPrv->tBufList[count].pBufferActual = (OMX_U32)pBuffer;
    }
    else {
        /*Update pBuffer with pBufferMapped stored in input port private
        AuxBuf1 remains untouched as stored during use or allocate calls*/
        pBufferHdr->pBuffer = (OMX_U8 *) pBufferHdr->pInputPortPrivate;
    }

    RPC_FlushBuffer(pBuffer, pBufferHdr->nAllocLen);

    eRPCError = RPC_EmptyThisBuffer(pCompPrv->hRemoteComp, pBufferHdr, pCompPrv->tBufList[count].pBufHeaderRemote, &eCompReturn);

    //changing back the local buffer address
    pBufferHdr->pBuffer = (OMX_U8 *)pCompPrv->tBufList[count].pBufferActual;

    if(eRPCError == RPC_OMX_ErrorNone) {
      DOMX_DEBUG("RPC_EmptyThisBuffer Successful");
      eError = eCompReturn;
    }
    else {
      DOMX_DEBUG("****RPC_EmptyThisBuffer RPC Error");
      eError = OMX_ErrorUndefined;
    }

	EXIT:
    DOMX_DEBUG("\n%s: Exiting: ",__FUNCTION__);
		return eError;
}


/* ===========================================================================*/
/**
 * @name PROXY_FillThisBuffer()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_FillThisBuffer(OMX_HANDLETYPE hComponent,
                                          OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn;
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    PROXY_COMPONENT_PRIVATE* pCompPrv;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *)hComponent;

    OMX_U32 count=0;
    OMX_U8 isMatchFound = 0;
    OMX_U8 *pBuffer=NULL;

    DOMX_DEBUG("\n%s Entered",__FUNCTION__);

    PROXY_assert((pBufferHdr != NULL) && (hComp->pComponentPrivate != NULL) &&
                 (hComp->pComponentPrivate != NULL),
                 OMX_ErrorBadParameter, NULL);

    pCompPrv=(PROXY_COMPONENT_PRIVATE*)hComp->pComponentPrivate;

    pBuffer = pBufferHdr->pBuffer;

    DOMX_DEBUG("\n%s:  pBufferHdr->pBuffer : 0x%x, pBufferHdr->nFilledLen : %d ",__FUNCTION__,pBufferHdr->pBuffer,pBufferHdr->nFilledLen);

    /*First find the index of this buffer header to retrieve remote buffer header */
    for(count=0;count<pCompPrv->nNumOfBuffers;count++)
    {
        if(pCompPrv->tBufList[count].pBufHeader == pBufferHdr)
        {
            DOMX_DEBUG("%s: Buffer Index of Match %d \n",__FUNCTION__,count);
            isMatchFound = 1;
            break;
        }
    }
    if(!isMatchFound)
	{
        DOMX_DEBUG("\n%s: Could not find the remote header in buffer list",__FUNCTION__);
		eError = OMX_ErrorBadParameter;
		goto EXIT;
	}

    /*[NPA] If the buffer is modified in buffer header, force remap.
    TBD: Even if MODIFIED is set, the pBuffer can be a pre-mapped buffer
    */
    if((pBufferHdr->nFlags)&(OMX_BUFFERHEADERFLAG_MODIFIED)){

        /* Same pBufferHdr will get updated with remote pBuffer and pAuxBuf1 if a 2D buffer */
        eError=RPC_PrepareBuffer_Remote(pCompPrv, pCompPrv->hRemoteComp,pBufferHdr->nOutputPortIndex,
                                        pBufferHdr->nAllocLen,
                                        pBufferHdr, NULL);

        if(eError != OMX_ErrorNone) {
            PROXY_assert(0, OMX_ErrorUndefined,
                         "Unable to map buffer");
        }

        /*Now update the buffer list with new details*/
        pCompPrv->tBufList[count].pBufferMapped = (OMX_U32)(pBufferHdr->pBuffer);
        pCompPrv->tBufList[count].pBufferActual = (OMX_U32)pBuffer;
    }
    else {
        /*Update pBuffer with pBufferMapped stored in input port private
        AuxBuf1 remains untouched as stored during use or allocate calls*/
        pBufferHdr->pBuffer = (OMX_U8 *) pBufferHdr->pInputPortPrivate;
	}

    RPC_FlushBuffer(pBuffer, pBufferHdr->nAllocLen);

    RPC_FillThisBuffer(pCompPrv->hRemoteComp, pBufferHdr, pCompPrv->tBufList[count].pBufHeaderRemote,&eCompReturn);

    //changing back the local buffer address
    pBufferHdr->pBuffer = (OMX_U8 *)pCompPrv->tBufList[count].pBufferActual;

    if(eRPCError == RPC_OMX_ErrorNone) {
      DOMX_DEBUG("RPC_FillThisBuffer Successful");
      eError = eCompReturn;
    }
    else {
      DOMX_DEBUG("****RPC_FillThisBuffer RPC Error");
      eError = OMX_ErrorUndefined;
    }

EXIT:
    DOMX_DEBUG("\n%s: Exiting:\n ",__FUNCTION__);
    return eError;
}


/* ===========================================================================*/
/**
 * @name PROXY_AllocateBuffer()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                          OMX_IN OMX_U32 nPortIndex,
                                          OMX_IN OMX_PTR pAppPrivate,
                                          OMX_IN OMX_U32 nSizeBytes)
{
        OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn;
        RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
        PROXY_COMPONENT_PRIVATE* pCompPrv;
        OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *)hComponent;

	OMX_BUFFERHEADERTYPE * pBufferHeader = NULL;
	OMX_U32 pBufferMapped;
	OMX_U32 pBufHeaderRemote;
	OMX_U32 currentBuffer;
        OMX_U8* pBuffer;
	OMX_TI_PLATFORMPRIVATE * pPlatformPrivate;

        DOMX_DEBUG("\n Entered %s ____ \n",__FUNCTION__);

        PROXY_assert((hComp->pComponentPrivate != NULL),
                      OMX_ErrorBadParameter, NULL);

        pCompPrv=(PROXY_COMPONENT_PRIVATE*)hComp->pComponentPrivate;

        currentBuffer = pCompPrv->nNumOfBuffers;
        DOMX_DEBUG("\nIn UB, no. of buffers = %d\n",pCompPrv->nNumOfBuffers);
        PROXY_assert((pCompPrv->nNumOfBuffers < MAX_NUM_PROXY_BUFFERS),
                     OMX_ErrorInsufficientResources,
                     "Proxy cannot handle more than MAX buffers");

	//Allocating Local bufferheader to be maintained locally within proxy
	pBufferHeader = (OMX_BUFFERHEADERTYPE*)TIMM_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE),TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_INT);
        PROXY_assert((pBufferHeader != NULL),
                      OMX_ErrorInsufficientResources,
                      "Allocation of Buffer Header structure failed");

       pPlatformPrivate = (OMX_TI_PLATFORMPRIVATE *)TIMM_OSAL_Malloc(sizeof(OMX_TI_PLATFORMPRIVATE),TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_INT);
	if(pPlatformPrivate==NULL) {
        TIMM_OSAL_Free(pBufferHeader);
        PROXY_assert(0, OMX_ErrorInsufficientResources, NULL);
	}
	pBufferHeader->pPlatformPrivate = pPlatformPrivate;

        DOMX_DEBUG("\n Calling RPC \n");

        eRPCError = RPC_AllocateBuffer(pCompPrv->hRemoteComp,&pBufferHeader,nPortIndex,
                               &pBufHeaderRemote,&pBufferMapped,pAppPrivate,nSizeBytes,&eCompReturn);

         if(eRPCError == RPC_OMX_ErrorNone) {
            DOMX_DEBUG("\n%s Allocate Buffer Successful\n", __FUNCTION__);
            DOMX_DEBUG("\n%s Value of pBufHeaderRemote: 0x%x   LocalBufferHdr :0x%x\n",__FUNCTION__, pBufHeaderRemote,pBufferHeader);

            if(eCompReturn == OMX_ErrorNone)
            {
                  //pBuffer = pBufferHeader->pBuffer;

                  eError = RPC_PrepareBuffer_Chiron(pCompPrv, pCompPrv->hRemoteComp, nPortIndex, nSizeBytes, pBufferHeader, NULL);

	 if(eError!=OMX_ErrorNone) {
                  TIMM_OSAL_Free(pBufferHeader);
                  TIMM_OSAL_Free(pPlatformPrivate);

                  PROXY_assert(0, OMX_ErrorUndefined,
                  "ERROR WHILE GETTING FRAME HEIGHT");
	}
                 pBuffer = pBufferHeader->pBuffer;
/*
pBufferMapped here will contain the Y pointer (basically the unity mapped pBuffer)
pBufferHeaderRemote is the header that contains both Y, UV pointers
*/
            pCompPrv->tBufList[currentBuffer].pBufHeader = pBufferHeader;
            pCompPrv->tBufList[currentBuffer].pBufHeaderRemote= pBufHeaderRemote;
            pCompPrv->tBufList[currentBuffer].pBufferMapped = pBufferMapped;
            pCompPrv->tBufList[currentBuffer].pBufferActual =  (OMX_U32)pBuffer;

			//caching actual content of pInportPrivate
            pCompPrv->tBufList[currentBuffer].actualContent = (OMX_U32)pBufferHeader->pInputPortPrivate;
			//filling pInportPrivate with the mapped address to be later used during ETB and FTB calls
			//Need to think on if we need a global actual buffer to mapped buffer data.
			pBufferHeader->pInputPortPrivate = (OMX_PTR )pBufferMapped;

			//keeping track of number of Buffers
            pCompPrv->nNumOfBuffers++;

			*ppBufferHdr = pBufferHeader;
		}
		else
		{
            TIMM_OSAL_Free(pPlatformPrivate);
			TIMM_OSAL_Free((void *)pBufferHeader);
            eError = eCompReturn;
		}
	}

	else
	{
		eError = OMX_ErrorUndefined;
	}

EXIT:
	return eError;
}


/* ===========================================================================*/
/**
 * @name PROXY_UseBuffer()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_UseBuffer (OMX_IN OMX_HANDLETYPE hComponent,
                                      OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                      OMX_IN OMX_U32 nPortIndex,OMX_IN OMX_PTR pAppPrivate,
                                      OMX_IN OMX_U32 nSizeBytes, OMX_IN OMX_U8* pBuffer)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE * pBufferHeader = NULL;
    OMX_ERRORTYPE eCompReturn;
    OMX_U32 pBufferMapped;
    OMX_U32 pBufHeaderRemote;
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    OMX_U32 currentBuffer;
    PROXY_COMPONENT_PRIVATE* pCompPrv= NULL;
    OMX_TI_PLATFORMPRIVATE * pPlatformPrivate;
    OMX_COMPONENTTYPE * hComp =(OMX_COMPONENTTYPE *) hComponent;

    PROXY_assert((hComp->pComponentPrivate != NULL),
                  OMX_ErrorBadParameter, NULL);

    pCompPrv=(PROXY_COMPONENT_PRIVATE*)hComp->pComponentPrivate;

    currentBuffer = pCompPrv->nNumOfBuffers;
    DOMX_DEBUG("\nIn UB, no. of buffers = %d\n",pCompPrv->nNumOfBuffers);

    PROXY_assert((pCompPrv->nNumOfBuffers < MAX_NUM_PROXY_BUFFERS),
                  OMX_ErrorInsufficientResources,
                  "Proxy cannot handle more than MAX buffers");

    //Allocating Local bufferheader to be maintained locally within proxy
    pBufferHeader = (OMX_BUFFERHEADERTYPE*) TIMM_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE),TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_INT);
    PROXY_assert((pBufferHeader != NULL),
                  OMX_ErrorInsufficientResources,
                  "Allocation of Buffer Header structure failed");

    pPlatformPrivate = (OMX_TI_PLATFORMPRIVATE *)TIMM_OSAL_Malloc(sizeof(OMX_TI_PLATFORMPRIVATE),TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_INT);
    if(pPlatformPrivate == NULL) {
       TIMM_OSAL_Free(pBufferHeader);
       PROXY_assert(0, OMX_ErrorInsufficientResources, NULL);
    }
    pBufferHeader->pPlatformPrivate = pPlatformPrivate;

    DOMX_DEBUG("Preparing buffer to Remote Core...");

    pBufferHeader->pBuffer = pBuffer;

    DOMX_DEBUG("Prepared buffer header for preparebuffer function...");

    /*[NPA] With NPA, pBuffer can be NULL*/
    if(pBuffer != NULL) {
    eError=RPC_PrepareBuffer_Remote(pCompPrv, pCompPrv->hRemoteComp,nPortIndex,nSizeBytes, pBufferHeader, NULL);

        if(eError != OMX_ErrorNone) {
            TIMM_OSAL_Free(pPlatformPrivate);
            TIMM_OSAL_Free(pBufferHeader);
            PROXY_assert(0, OMX_ErrorUndefined,
                         "ERROR WHILE GETTING FRAME HEIGHT");
        }
}

    DOMX_DEBUG("Making Remote call...");
    eRPCError = RPC_UseBuffer(pCompPrv->hRemoteComp,&pBufferHeader,nPortIndex,pAppPrivate,nSizeBytes,pBuffer,&pBufferMapped,&pBufHeaderRemote, &eCompReturn);

    if(eRPCError == RPC_OMX_ErrorNone)
    {
        DOMX_DEBUG("\n%s Yahoo!!Use Buffer Successful", __FUNCTION__);
        DOMX_DEBUG("\n %s Value of pBufHeaderRemote: 0x%x   LocalBufferHdr :0x%x",__FUNCTION__, pBufHeaderRemote,pBufferHeader);

        if(eCompReturn == OMX_ErrorNone)
{
            //Storing details of pBufferHeader/Mapped/Actual buffer address locally.
            pCompPrv->tBufList[currentBuffer].pBufHeader = pBufferHeader;
            pCompPrv->tBufList[currentBuffer].pBufHeaderRemote= pBufHeaderRemote;
            pCompPrv->tBufList[currentBuffer].pBufferMapped = pBufferMapped;
            pCompPrv->tBufList[currentBuffer].pBufferActual =  (OMX_U32)pBuffer;
            //caching actual content of pInportPrivate
            pCompPrv->tBufList[currentBuffer].actualContent = (OMX_U32)pBufferHeader->pInputPortPrivate;

            //keeping track of number of Buffers
            pCompPrv->nNumOfBuffers++;
            DOMX_DEBUG("\nUpdating no. of buffer to %d\n",pCompPrv->nNumOfBuffers);

            //Restore back original pBuffer
            pBufferHeader->pBuffer = pBuffer;
            //pBufferMapped in pInputPortPrivate acts as key during ETB/FTB calls
            pBufferHeader->pInputPortPrivate = (OMX_PTR )pBufferMapped;

            *ppBufferHdr = pBufferHeader;
        }
        else
	{
            DOMX_DEBUG("\n***ERROR in UseBuffer return value, freeing buffer header");
            TIMM_OSAL_Free(pPlatformPrivate);
            TIMM_OSAL_Free((void *)pBufferHeader);
            eError = eCompReturn;
		}
	}
    else
	{
        eError = OMX_ErrorUndefined;
	}

     DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
EXIT:
	return eError;
}

/* ===========================================================================*/
/**
 * @name PROXY_FreeBuffer()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_FreeBuffer(OMX_IN  OMX_HANDLETYPE hComponent,
                                      OMX_IN  OMX_U32 nPortIndex,
                                      OMX_IN  OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    OMX_ERRORTYPE eCompReturn;
    OMX_COMPONENTTYPE * hComp =(OMX_COMPONENTTYPE *) hComponent;
    PROXY_COMPONENT_PRIVATE* pCompPrv= NULL;
    OMX_U8 *pBufferMapped = NULL;
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;

    OMX_U32 count=0;
    OMX_U8 isMatchFound = 0;
	OMX_ERRORTYPE eError = OMX_ErrorNone;

    PROXY_assert((pBufferHdr != NULL) && (hComp->pComponentPrivate != NULL) &&
                 (pBufferHdr->pInputPortPrivate != NULL),
                 OMX_ErrorBadParameter, NULL);

    pCompPrv=(PROXY_COMPONENT_PRIVATE*)hComp->pComponentPrivate;
    pBufferMapped =  (OMX_U8 *)pBufferHdr->pInputPortPrivate;

    for(count=0;count<pCompPrv->nNumOfBuffers;count++)
	{
        if(pCompPrv->tBufList[count].pBufHeader == pBufferHdr)
		{
			DOMX_DEBUG("%s: Buffer Index of Match %d \n",__FUNCTION__,count);
			isMatchFound = 1;
			break;
		}
	}
	if(!isMatchFound)
	{
		DOMX_DEBUG("\n%s: Could not find the mapped address in component private buffer list",__FUNCTION__);
		return OMX_ErrorBadParameter;
	}

    eRPCError = RPC_FreeBuffer(pCompPrv->hRemoteComp,nPortIndex,pCompPrv->tBufList[count].pBufHeaderRemote , &eCompReturn);

/*
TODO : Demap although not very critical
Unmapping
#ifdef TILER_BUFF

SysLinkMemUtils_unmap (UInt32 mappedAddr, ProcMgr_ProcId procId)
How do you access UV mapped buffers from here. (Y is accessbile from tBufList under pBuffermapped.
The UV is not, may be consider adding this to the table
*/

    if(eRPCError == RPC_OMX_ErrorNone)
	{
		DOMX_DEBUG("%s Yahoo!! Free Buffer Successful\n", __FUNCTION__);
        eError = eCompReturn;
	}
	else{
		DOMX_DEBUG("%s [%d]: Warning: RPC Error",__FUNCTION__,__LINE__);
		eError=OMX_ErrorUndefined;
	}

EXIT:
	return eError;
}


/* ===========================================================================*/
/**
 * @name PROXY_SetParameter()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN  OMX_INDEXTYPE nParamIndex,
                                        OMX_IN  OMX_PTR pParamStruct)
 {
    OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn;
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    PROXY_COMPONENT_PRIVATE* pCompPrv;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *)hComponent;

	DOMX_DEBUG("Entered: %s\n",__FUNCTION__);

    PROXY_require((pParamStruct != NULL),
                 OMX_ErrorBadParameter, NULL);

    PROXY_assert((hComp->pComponentPrivate != NULL),
                 OMX_ErrorBadParameter, NULL);

    pCompPrv = (PROXY_COMPONENT_PRIVATE*)hComp->pComponentPrivate;

    eRPCError = RPC_SetParameter(pCompPrv->hRemoteComp,nParamIndex,pParamStruct, &eCompReturn);

    if(eRPCError == RPC_OMX_ErrorNone) {
        DOMX_DEBUG("RPC_SetParameter Successful");
        eError = eCompReturn;
    }
    else {
        DOMX_DEBUG("****RPC_SetParameter RPC Error");
        eError = OMX_ErrorUndefined;
    }

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
	return eError;
 }


/* ===========================================================================*/
/**
 * @name PROXY_GetParameter()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_GetParameter(OMX_IN  OMX_HANDLETYPE hComponent,
                                        OMX_IN  OMX_INDEXTYPE nParamIndex,
                                        OMX_INOUT OMX_PTR pParamStruct)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn;
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    PROXY_COMPONENT_PRIVATE* pCompPrv;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *)hComponent;

    DOMX_DEBUG("Entered: %s\n",__FUNCTION__);

    PROXY_require((pParamStruct != NULL),
                 OMX_ErrorBadParameter, NULL);

    PROXY_assert((hComp->pComponentPrivate != NULL),
                 OMX_ErrorBadParameter, NULL);

    pCompPrv = (PROXY_COMPONENT_PRIVATE*)hComp->pComponentPrivate;
#ifdef _Android
	if(pCompPrv->componentprivateGetParameters != NULL)
	{
		eRPCError = pCompPrv->componentprivateGetParameters(hComponent,nParamIndex,pParamStruct);
		if(eRPCError == OMX_ErrorNone)
			{
			DOMX_DEBUG("%s RPC_GetParameter Private Successful\n", __FUNCTION__);
			return eRPCError;
			}
	}
#endif
    eRPCError = RPC_GetParameter(pCompPrv->hRemoteComp,nParamIndex,pParamStruct, &eCompReturn);

    if(eRPCError == RPC_OMX_ErrorNone) {
        DOMX_DEBUG("RPC_GetParameter Successful");
        eError = eCompReturn;
       }
	else{
        DOMX_DEBUG("****RPC_GetParameter RPC Error");
		eError=OMX_ErrorUndefined;
	}

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
	return eError;
}



/* ===========================================================================*/
/**
 * @name PROXY_GetConfig()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_GetConfig (OMX_HANDLETYPE hComponent,
				      OMX_INDEXTYPE nConfigIndex,
                                      OMX_PTR pConfigStruct)
{

    OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn;
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    PROXY_COMPONENT_PRIVATE* pCompPrv;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *)hComponent;

	DOMX_DEBUG("Entered: %s\n",__FUNCTION__);

    PROXY_require((pConfigStruct != NULL),
                 OMX_ErrorBadParameter, NULL);

    PROXY_assert((hComp->pComponentPrivate != NULL),
                 OMX_ErrorBadParameter, NULL);

    pCompPrv = (PROXY_COMPONENT_PRIVATE*)hComp->pComponentPrivate;

    eRPCError = RPC_GetConfig(pCompPrv->hRemoteComp, nConfigIndex, pConfigStruct, &eCompReturn);

    if(eRPCError == RPC_OMX_ErrorNone) {
        DOMX_DEBUG("RPC_GetConfig Successful");
        eError = eCompReturn;
    }
    else {
        DOMX_DEBUG("****RPC_GetConfig RPC Error");
        eError = OMX_ErrorUndefined;
    }

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
	return eError;
}


/* ===========================================================================*/
/**
 * @name PROXY_SetConfig()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_SetConfig(OMX_IN  OMX_HANDLETYPE hComponent,
				     OMX_IN  OMX_INDEXTYPE nConfigIndex,
                                     OMX_IN  OMX_PTR pConfigStruct)
 {

    OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn;
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    PROXY_COMPONENT_PRIVATE* pCompPrv;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *)hComponent;

    DOMX_DEBUG("Entered: %s\n",__FUNCTION__);

    PROXY_require((pConfigStruct != NULL),
                 OMX_ErrorBadParameter, NULL);

    PROXY_assert((hComp->pComponentPrivate != NULL),
                 OMX_ErrorBadParameter, NULL);

    pCompPrv = (PROXY_COMPONENT_PRIVATE*)hComp->pComponentPrivate;

    eRPCError = RPC_SetConfig(pCompPrv->hRemoteComp, nConfigIndex, pConfigStruct, &eCompReturn);

    if(eRPCError == RPC_OMX_ErrorNone) {
        DOMX_DEBUG("RPC_SetConfig Successful");
        eError = eCompReturn;
    }
    else {
        DOMX_DEBUG("****RPC_SetConfig RPC Error");
        eError = OMX_ErrorUndefined;
    }

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
    return eError;
 }


/* ===========================================================================*/
/**
 * @name PROXY_GetState()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_GetState(OMX_IN  OMX_HANDLETYPE hComponent,
				    OMX_OUT OMX_STATETYPE* pState)
{
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  OMX_ERRORTYPE eCompReturn;
  RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
  OMX_COMPONENTTYPE * hComp = hComponent;
  PROXY_COMPONENT_PRIVATE* pCompPrv = NULL;

  DOMX_DEBUG("\nEnter: %s",__FUNCTION__);

  PROXY_require((pState != NULL),
                OMX_ErrorBadParameter, NULL);

  PROXY_assert((hComp->pComponentPrivate != NULL),
                OMX_ErrorBadParameter, NULL);

  pCompPrv = (PROXY_COMPONENT_PRIVATE*)hComp->pComponentPrivate;

  eRPCError = RPC_GetState(pCompPrv->hRemoteComp, pState, &eCompReturn);

  DOMX_DEBUG("\n%s: Returned from RPC_GetState",__FUNCTION__);

  if(eRPCError == RPC_OMX_ErrorNone)
  {
	DOMX_DEBUG("\n%s Yahoo!! Get Parameter Successful", __FUNCTION__);
    eError = eCompReturn;
  }
  else
  {
	DOMX_DEBUG("\n%s [%d]: Warning: RPC Error",__FUNCTION__,__LINE__);
	eError=OMX_ErrorUndefined;
  }

EXIT:
   return eError;
}



/* ===========================================================================*/
/**
 * @name PROXY_SendCommand()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_SendCommand(OMX_IN  OMX_HANDLETYPE hComponent,
				   OMX_IN  OMX_COMMANDTYPE eCmd,
				   OMX_IN  OMX_U32 nParam,
				   OMX_IN  OMX_PTR pCmdData)
{
  OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn;
  RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
  PROXY_COMPONENT_PRIVATE* pCompPrv;
  OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *)hComponent;

  DOMX_DEBUG("\nEnter: %s",__FUNCTION__);

  PROXY_assert((hComp->pComponentPrivate != NULL),
                OMX_ErrorBadParameter, NULL);

  pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

  eRPCError = RPC_SendCommand(pCompPrv->hRemoteComp, eCmd, nParam, pCmdData, &eCompReturn);

  if(eRPCError == RPC_OMX_ErrorNone) {
      DOMX_DEBUG("RPC_SendCommand Successful");
      eError = eCompReturn;
  }
  else {
      DOMX_DEBUG("****RPC_SendCommand RPC Error");
      eError = OMX_ErrorUndefined;
  }

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
  return eError;
}



/* ===========================================================================*/
/**
 * @name PROXY_GetComponentVersion()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_GetComponentVersion(OMX_IN  OMX_HANDLETYPE hComponent,
	    OMX_OUT OMX_STRING pComponentName,
	    OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
	    OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
	    OMX_OUT OMX_UUIDTYPE* pComponentUUID)
{
  OMX_ERRORTYPE eError       = OMX_ErrorNone;
  PROXY_COMPONENT_PRIVATE* pCompPrv = NULL;
  OMX_COMPONENTTYPE * hComp = hComponent;
  RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
  OMX_ERRORTYPE eCompReturn;

  DOMX_DEBUG("\nEnter: %s",__FUNCTION__);

  PROXY_assert((hComp->pComponentPrivate != NULL),
                OMX_ErrorBadParameter, NULL);

  pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

  eRPCError = RPC_GetComponentVersion(pCompPrv->hRemoteComp,
			  pComponentName,
			  pComponentVersion,
			  pSpecVersion,
			  pComponentUUID,
              &eCompReturn);

  if(eRPCError == RPC_OMX_ErrorNone) {
    DOMX_DEBUG("\n%s PROXY_GetComponentVersion Successful", __FUNCTION__);
    eError = eCompReturn;
  }
  else {
	 eError = OMX_ErrorUndefined;
    }

  DOMX_DEBUG("\nLeaving: %s",__FUNCTION__);

EXIT:
  return  eError;
}



/* ===========================================================================*/
/**
 * @name PROXY_GetExtensionIndex()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_GetExtensionIndex( OMX_IN  OMX_HANDLETYPE hComponent,
					      OMX_IN  OMX_STRING cParameterName,
					      OMX_OUT OMX_INDEXTYPE* pIndexType)
{

  OMX_ERRORTYPE eError       = OMX_ErrorNone;
  PROXY_COMPONENT_PRIVATE* pCompPrv = NULL;
  OMX_COMPONENTTYPE * hComp = hComponent;
  OMX_ERRORTYPE eCompReturn;
  RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;

  DOMX_DEBUG("\nEnter: %s",__FUNCTION__);

  PROXY_assert((hComp->pComponentPrivate != NULL),
                OMX_ErrorBadParameter, NULL);

  pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

  eRPCError = RPC_GetExtensionIndex(pCompPrv->hRemoteComp,
			cParameterName,
			pIndexType,
            &eCompReturn);

  if(eRPCError == RPC_OMX_ErrorNone) {
    DOMX_DEBUG("\n%s PROXY_GetComponentVersion Successful", __FUNCTION__);
    eError = eCompReturn;
        }
        else {
    eError=OMX_ErrorUndefined;
        }

  DOMX_DEBUG("\nLeaving: %s",__FUNCTION__);

EXIT:
  return  eError;
}




/* ===========================================================================*/
/**
 * @name PROXY_ComponentRoleEnum()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
					     OMX_OUT OMX_U8 *cRole,
					     OMX_IN OMX_U32 nIndex)
{
  OMX_ERRORTYPE eError = OMX_ErrorNone;

  DOMX_DEBUG("\nEntering: %s",__FUNCTION__);
  DOMX_DEBUG("\n EMPTY IMPLEMENTATION ");
  DOMX_DEBUG("\nLeaving: %s",__FUNCTION__);

EXIT:
    return eError;
}


/* ===========================================================================*/
/**
 * @name PROXY_ComponentTunnelRequest()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_ComponentTunnelRequest(OMX_IN  OMX_HANDLETYPE hComponent,
                                                  OMX_IN  OMX_U32 nPort,
                                                  OMX_IN  OMX_HANDLETYPE hTunneledComp,
                                                  OMX_IN  OMX_U32 nTunneledPort,
                                                  OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
  OMX_ERRORTYPE eError = OMX_ErrorNone;

  DOMX_DEBUG("\nEntering: %s",__FUNCTION__);
  DOMX_DEBUG("\n EMPTY IMPLEMENTATION ");
  DOMX_DEBUG("\nLeaving: %s",__FUNCTION__);

EXIT:
    return eError;
}


/* ===========================================================================*/
/**
 * @name PROXY_SetCallbacks()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_SetCallbacks (OMX_HANDLETYPE hComponent,
                                        OMX_CALLBACKTYPE* pCallBacks,
                                        OMX_PTR pAppData)
{
        OMX_ERRORTYPE eError = OMX_ErrorNone;
    PROXY_COMPONENT_PRIVATE* pCompPrv;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *)hComponent;

    DOMX_DEBUG("Entered : %s\n",__FUNCTION__);

    PROXY_require((pCallBacks != NULL),
                   OMX_ErrorBadParameter, NULL);

    PROXY_assert((hComp->pComponentPrivate != NULL),
                OMX_ErrorBadParameter, NULL);

    pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

    /*Store App callback and data to proxy- managed by proxy*/
    pCompPrv->tCBFunc = *pCallBacks;
    pCompPrv->pILAppData = pAppData;

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
    return eError;
}

/* ===========================================================================*/
/**
 * @name PROXY_ComponentDeInit()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_ComponentDeInit (OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn;
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    PROXY_COMPONENT_PRIVATE* pCompPrv;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *)hComponent;

    OMX_U32 count;

    DOMX_DEBUG("Entered : %s\n",__FUNCTION__);

    PROXY_assert((hComp->pComponentPrivate != NULL),
                OMX_ErrorBadParameter, NULL);

    pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

    eRPCError = RPC_FreeHandle(pCompPrv->hRemoteComp, &eCompReturn);

    //Unmap any buffers previously mapped
    for(count=0;count<pCompPrv->nNumOfBuffers;++count)
    {
        DOMX_DEBUG("%s Cleaning up Buffers\n",__FUNCTION__);
        if(pCompPrv->tBufList[count].pBufferMapped)
            RPC_UnMapBuffer(pCompPrv->tBufList[count].pBufferMapped);

        if(pCompPrv->tBufList[count].pBufHeader){

            if(pCompPrv->tBufList[count].pBufHeader->pPlatformPrivate)
            TIMM_OSAL_Free(pCompPrv->tBufList[count].pBufHeader->pPlatformPrivate);

            TIMM_OSAL_Free(pCompPrv->tBufList[count].pBufHeader);
            pCompPrv->tBufList[count].pBufHeader = NULL;
        }
    }

    RPC_InstanceDeInit(pCompPrv->hRemoteComp);

    if(pCompPrv->cCompName){
        TIMM_OSAL_Free(pCompPrv->cCompName);
  	    }

    if(pCompPrv){
		TIMM_OSAL_Free(pCompPrv);
    }

   if(eRPCError == RPC_OMX_ErrorNone) {
      DOMX_DEBUG("RPC_FreeHandle Successful");
      eError = eCompReturn;
   }
   else {
      DOMX_DEBUG("****RPC_FreeHandle RPC Error");
		 eError = OMX_ErrorUndefined;
	    }

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
    return eError;
}


/* ===========================================================================*/
/**
 * @name OMX_ProxyCommonInit()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE OMX_ProxyCommonInit(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn;
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    PROXY_COMPONENT_PRIVATE* pCompPrv;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *)hComponent;

    RPC_OMX_HANDLE  hRemoteComp;
    OMX_U32 i=0;
    //OMX_CALLBACKTYPE tProxyCBinfo = {PROXY_EventHandler, PROXY_EmptyBufferDone, PROXY_FillBufferDone};

    DOMX_DEBUG("\n_____________________INSIDE PROXY COMMON__________________________\n");
    DOMX_DEBUG("%s:  hComponent: 0x%x\n",__FUNCTION__, hComponent);

    PROXY_assert((hComp->pComponentPrivate != NULL),
                 OMX_ErrorBadParameter, NULL);

    pCompPrv = (PROXY_COMPONENT_PRIVATE*)hComp->pComponentPrivate;

    pCompPrv->nNumOfBuffers = 0;
    pCompPrv->proxyEmptyBufferDone = PROXY_EmptyBufferDone;
    pCompPrv->proxyFillBufferDone = PROXY_FillBufferDone;
    pCompPrv->proxyEventHandler = PROXY_EventHandler;

    /*reset num of lines info per port*/
    for(i=0; i<PROXY_MAXNUMOFPORTS;i++)
    pCompPrv->nNumOfLines[i] = 0;

    eRPCError = RPC_InstanceInit(pCompPrv->cCompName,&hRemoteComp);
    PROXY_assert((eRPCError == RPC_OMX_ErrorNone) && (hRemoteComp!= NULL),
                 OMX_ErrorUndefined, "Error initializing RPC");

    //Send the proxy component handle for pAppData
    eRPCError = RPC_GetHandle(hRemoteComp, pCompPrv->cCompName, (OMX_PTR)hComponent, NULL, &eCompReturn);

    if(eRPCError==RPC_OMX_ErrorNone) {
       if(eCompReturn==OMX_ErrorNone) {
          DOMX_DEBUG("\nRPC_GetHandle Successful");
          pCompPrv->hRemoteComp = hRemoteComp;
       }
       else {
          DOMX_DEBUG("\n ERROR executing OMX_GetHandle remotely");
          eError = eCompReturn;
          RPC_InstanceDeInit(hRemoteComp);
       }
     }
    else {
       DOMX_DEBUG("****RPC_GetHandle RPC Error");
       RPC_InstanceDeInit(hRemoteComp);
       eError = OMX_ErrorHardware;
		 goto EXIT;
	    }

    hComp->SetCallbacks	= PROXY_SetCallbacks;
    hComp->ComponentDeInit = PROXY_ComponentDeInit;
    hComp->UseBuffer = PROXY_UseBuffer;
    hComp->GetParameter = PROXY_GetParameter;
    hComp->SetParameter = PROXY_SetParameter;
    hComp->EmptyThisBuffer= PROXY_EmptyThisBuffer;
    hComp->FillThisBuffer= PROXY_FillThisBuffer;
    hComp->GetComponentVersion= PROXY_GetComponentVersion;
    hComp->SendCommand = PROXY_SendCommand;
    hComp->GetConfig= PROXY_GetConfig;
    hComp->SetConfig= PROXY_SetConfig;
    hComp->GetState= PROXY_GetState;
    hComp->GetExtensionIndex  = PROXY_GetExtensionIndex;
    hComp->FreeBuffer = PROXY_FreeBuffer;
    hComp->ComponentRoleEnum = PROXY_ComponentRoleEnum;
    hComp->AllocateBuffer = PROXY_AllocateBuffer;
    hComp->ComponentTunnelRequest = PROXY_ComponentTunnelRequest;

    pCompPrv->hRemoteComp = hRemoteComp;
#ifdef _Android
    pCompPrv->componentprivateGetParameters = NULL;
#endif
    DOMX_DEBUG("\n Proxy Initted");

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
    return eError;
}




OMX_ERRORTYPE RPC_PrepareBuffer_Chiron(PROXY_COMPONENT_PRIVATE *pCompPrv, OMX_COMPONENTTYPE *hRemoteComp, OMX_U32 nPortIndex, OMX_U32 nSizeBytes, OMX_BUFFERHEADERTYPE *pDucBuf, OMX_BUFFERHEADERTYPE *pChironBuf)
		{
      OMX_ERRORTYPE eError = OMX_ErrorNone;
      OMX_U32 nNumOfLines = 1;
      OMX_U8 *pBuffer;

      DSPtr dsptr[2];
      bytes_t lengths[2],uvPtr;
      OMX_U32 i = 0;
      OMX_U32 numBlocks =0;

      pBuffer = pDucBuf->pBuffer;

     if(((OMX_TI_PLATFORMPRIVATE *)pDucBuf->pPlatformPrivate)->pAuxBuf1 == NULL) {
            DOMX_DEBUG("\nOne component buffer\n");

      if(!(pCompPrv->nNumOfLines[nPortIndex])) {
      pCompPrv->nNumOfLines[nPortIndex] = 1;
      }

            dsptr[0]=pBuffer;
            numBlocks = 1;
      lengths[0] = (4 * 1024) * ((nSizeBytes + ((4 * 1024) - 1)) / (4 * 1024));
         }
      else {
      DOMX_DEBUG("\nTwo component buffers\n");
      dsptr[0]=pBuffer;
      dsptr[1] = ((OMX_TI_PLATFORMPRIVATE *)pDucBuf->pPlatformPrivate)->pAuxBuf1;

      if(!(pCompPrv->nNumOfLines[nPortIndex])) {
      eError=RPC_UTIL_GetNumLines(hRemoteComp,nPortIndex, &nNumOfLines);
      PROXY_assert((eError== OMX_ErrorNone), OMX_ErrorUndefined,
                    "ERROR WHILE GETTING FRAME HEIGHT");

      pCompPrv->nNumOfLines[nPortIndex] = nNumOfLines;
      }
      else {
      nNumOfLines = pCompPrv->nNumOfLines[nPortIndex];
      }

      lengths[0]= nNumOfLines* 4 * 1024;
      lengths[1]= nNumOfLines/2 * 4 * 1024;
           numBlocks = 2;
        }

        //Map back to chiron
        DOMX_DEBUG("\nNumBlocks = %d\n", numBlocks);
        for (i = 0; i < numBlocks; i++)
        {
            DOMX_DEBUG("\ndsptr[%d] = 0x%x\n", i, dsptr[i]);
            DOMX_DEBUG("\nlength[%d] = %d\n", i, lengths[i]);
        }

     pDucBuf->pBuffer = tiler_assisted_phase1_D2CReMap(numBlocks,dsptr,lengths);

     PROXY_assert((pDucBuf->pBuffer != NULL), OMX_ErrorUndefined,
                     "Mapping to Chiron failed");

EXIT:
    return OMX_ErrorNone;
}


//Takes chiron buffer buffer header and updates with ducati buffer ptr and UV ptr
OMX_ERRORTYPE RPC_PrepareBuffer_Remote(PROXY_COMPONENT_PRIVATE *pCompPrv, OMX_COMPONENTTYPE *hRemoteComp, OMX_U32 nPortIndex, OMX_U32 nSizeBytes, OMX_BUFFERHEADERTYPE *pChironBuf, OMX_BUFFERHEADERTYPE *pDucBuf)
{
      OMX_ERRORTYPE eError = OMX_ErrorNone;
      OMX_U32 nNumOfLines = 1;
      OMX_U8 *pBuffer;

      DOMX_DEBUG("\n Entered %s ____ \n",__FUNCTION__);

      pBuffer = pChironBuf->pBuffer;

      if(!MemMgr_Is2DBlock(pBuffer)) {

       if(!(pCompPrv->nNumOfLines[nPortIndex])) {
         pCompPrv->nNumOfLines[nPortIndex] = 1;
       }

        pChironBuf->pBuffer = NULL;
        RPC_MapBuffer_Ducati(pBuffer, nSizeBytes, nNumOfLines, &(pChironBuf->pBuffer));
		}
      else {
      if(!(pCompPrv->nNumOfLines[nPortIndex])) {
        eError=RPC_UTIL_GetNumLines(hRemoteComp,nPortIndex, &nNumOfLines);
        PROXY_assert((eError== OMX_ErrorNone), OMX_ErrorUndefined,
                    "ERROR WHILE GETTING FRAME HEIGHT");

        pCompPrv->nNumOfLines[nPortIndex] = nNumOfLines;
		}
      else {
      nNumOfLines = pCompPrv->nNumOfLines[nPortIndex];
	}

      pChironBuf->pBuffer = NULL;
      ((OMX_TI_PLATFORMPRIVATE *)(pChironBuf->pPlatformPrivate))->pAuxBuf1 = NULL;

      RPC_MapBuffer_Ducati(pBuffer, 4 * 1024, nNumOfLines, &(pChironBuf->pBuffer));
      RPC_MapBuffer_Ducati((OMX_U8*) ((OMX_U32)pBuffer + nNumOfLines*4*1024), 4 * 1024, nNumOfLines/2, &((OMX_TI_PLATFORMPRIVATE *)(pChironBuf->pPlatformPrivate))->pAuxBuf1
);
	}

     DOMX_DEBUG("Exited: %s\n",__FUNCTION__);

EXIT:
    return OMX_ErrorNone;
}

/* ===========================================================================*/
/**
 * @name RPC_UTIL_GetNumLines()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
 OMX_ERRORTYPE RPC_UTIL_GetNumLines(OMX_COMPONENTTYPE *hRemoteComp, OMX_U32 nPortIndex, OMX_U32 * nNumOfLines)
{
    OMX_ERRORTYPE eError       = OMX_ErrorNone;
      OMX_ERRORTYPE eCompReturn;
      RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;

		OMX_PARAM_PORTDEFINITIONTYPE portDef;
		OMX_U8 * pBuffer;
		OMX_PTR pUVBuffer;

        DOMX_DEBUG("Entered: %s\n",__FUNCTION__);

	 /*initializing Structure*/
		portDef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
        portDef.nVersion.s.nVersionMajor = 0x1;
        portDef.nVersion.s.nVersionMinor = 0x1;
        portDef.nVersion.s.nRevision  = 0x0;
        portDef.nVersion.s.nStep   = 0x0;

        portDef.nPortIndex = nPortIndex;

     eRPCError = RPC_GetParameter(hRemoteComp,OMX_IndexParamPortDefinition,(OMX_PTR)&portDef, &eCompReturn);

     if(eRPCError == RPC_OMX_ErrorNone) {
	     DOMX_DEBUG("\n PROXY_UTIL Get Parameter Successful\n", __FUNCTION__);
         eError = eCompReturn;
        }
	 else{
		DOMX_DEBUG("\n%s [%d]: Warning: RPC Error",__FUNCTION__,__LINE__);
	 	eError=OMX_ErrorUndefined;
	 }

     if(eCompReturn==OMX_ErrorNone) {

     //start with 1 meaning 1D buffer
     *nNumOfLines = 1;

        if(portDef.eDomain==OMX_PortDomainVideo) {
            *nNumOfLines = portDef.format.video.nFrameHeight;
           //DOMX_DEBUG("\nPort definition Type is video...");
           //DOMX_DEBUG("\n&&Colorformat is:0x%x", portDef.format.video.eColorFormat);
           //DOMX_DEBUG("\nnFrameHeight is:%d", portDef.format.video.nFrameHeight);
           //*nNumOfLines = portDef.format.video.nFrameHeight;

           //if((portDef.format.video.eColorFormat == OMX_COLOR_FormatYUV420PackedSemiPlanar) ||
             //  (portDef.format.video.eColorFormat == OMX_COLOR_FormatYUV420Planar))
           //{
            //DOMX_DEBUG("\nSetting FrameHeight as Number of lines...");
            //*nNumOfLines = portDef.format.video.nFrameHeight;
           //}
		}
		else if(portDef.eDomain==OMX_PortDomainImage) {
		    DOMX_DEBUG("\nImage DOMAIN TILER SUPPORT");
		}
        else if(portDef.eDomain==OMX_PortDomainAudio) {
		    DOMX_DEBUG("\nAudio DOMAIN TILER SUPPORT");
		}
		else if(portDef.eDomain==OMX_PortDomainOther) {
		    DOMX_DEBUG("\nOther DOMAIN TILER SUPPORT");
		}
		else { //this is the sample component test
        //Temporary - just to get check functionality
		    DOMX_DEBUG("\nSample component TILER SUPPORT");
            *nNumOfLines = 4;
        }
		}
     else {
         DOMX_DEBUG("\n ERROR IN RECOVERING UV POINTER");
     }

     DOMX_DEBUG("Port Number: %d :: NumOfLines %d\n",nPortIndex,*nNumOfLines);
     DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
EXIT:
     return eError;
		}

/* ===========================================================================*/
/**
 * @name RPC_MapBuffer_Ducati()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE RPC_MapBuffer_Ducati(OMX_U8 *pBuf, OMX_U32 nBufLineSize, OMX_U32 nBufLines, OMX_U8 **pMappedBuf)
{
    ProcMgr_MapType mapType;
    SyslinkMemUtils_MpuAddrToMap MpuAddr_list_1D;
    MemAllocBlock block;
    OMX_U32 status;

    DOMX_DEBUG("\n Entered %s ____ \n",__FUNCTION__);

    if(!MemMgr_IsMapped(pBuf) && (nBufLines == 1)) {
        DOMX_DEBUG("\nBuffer is not mapped: Mapping as 1D buffer now..");
        block.pixelFormat = PIXEL_FMT_PAGE;
        block.dim.len = nBufLineSize;
        block.stride = 0;
        block.ptr    = pBuf;

        *pMappedBuf = MemMgr_Map(&block, 1);
	 }

    if(MemMgr_IsMapped(pBuf)) {
    //If Tiler 1D buffer, get corresponding ducati address and send out buffer to ducati
    //For 2D buffers, in phase1, retrive the ducati address (SSPtrs) for Y and UV buffers
    //and send out buffer to ducati
        mapType = ProcMgr_MapType_Tiler;
        MpuAddr_list_1D.mpuAddr = (UInt32)pBuf;
        MpuAddr_list_1D.size = nBufLineSize * nBufLines;

        status = SysLinkMemUtils_map(&MpuAddr_list_1D, 1, pMappedBuf, mapType, PROC_APPM3);
    }

     DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
EXIT:
    return OMX_ErrorNone;
	}
