/*=========================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 *  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied.
 * ==========================================================================*/

/**
 *  @file  omx_rpc_stub.c
 *         This file contains methods that provides the functionality for
 *         the OpenMAX1.1 DOMX Framework RPC Stub implementations.
 *
 *  @path \WTSD_DucatiMMSW\framework\domx\omx_rpc\src
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
/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
#include <string.h>
#include <stdio.h>

#include "omx_rpc.h"
#include "omx_rpc_utils.h"
#include "omx_proxy_common.h"
#include "omx_rpc_stub.h"
#include <OMX_TI_Common.h>
#include <timm_osal_interfaces.h>
#include <MultiProc.h>

/******************************************************************
 *   EXTERNS
 ******************************************************************/
extern OMX_U32 PACKET_SIZE; // different packet sizes required for INTER-M3 case and MPU-APPM3
extern RPC_Object rpcHndl[CORE_MAX];
extern COREID TARGET_CORE_ID;

/******************************************************************
 *   MACROS - LOCAL
 ******************************************************************/
#define RPC_getPacket(HRCM, nPacketSize, pPacket) \
pPacket = RcmClient_alloc(HRCM, nPacketSize); \
RPC_assert(pPacket != NULL, RPC_OMX_ErrorInsufficientResources, \
           "Error Allocating RCM Message Frame");

#define RPC_sendPacket_sync(HRCM, pPacket, fxnIdx) \
pPacket->fxnIdx = fxnIdx; \
status = RcmClient_exec(HRCM, pPacket); \
if(status < 0) { \
RcmClient_free(HRCM, pPacket); \
pPacket = NULL; \
RPC_assert(0, RPC_OMX_RCM_ErrorExecFail, \
           "RcmClient_exec failed"); \
}

#define RPC_sendPacket_async(HRCM, pPacket, fxnIdx) \
pPacket->fxnIdx = fxnIdx; \
status = RcmClient_execNoReply(HRCM, pPacket); \
if(status < 0) { \
RcmClient_free(HRCM, pPacket); \
pPacket = NULL; \
RPC_assert(0, RPC_OMX_RCM_ErrorExecFail, \
           "RcmClient_exec failed"); \
}

#define RPC_freePacket(HRCM, pPacket) \
if(pPacket!=NULL) RcmClient_free(HRCM, pPacket);

/* ===========================================================================*/
/**
 * @name RPC_GetHandle()
 * @brief Remote invocation stub for OMX_AllcateBuffer()
 * @param hComp: This is the handle into which the remote Component handle is returned.
                        it's handle with actual OMX Component handle that recides on the specified core
 * @param cComponentName: Name of the component that is to be created on Remote core
 * @param pCallBacks:
 * @param pAppData: equal to pAppData passed in original GetHandle.
 * @param eCompReturn: This is return value that will be supplied by Proxy to the caller.
 *                    This is actual return value returned by the Remote component
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_GetHandle(RPC_OMX_HANDLE hRPCCtx, OMX_STRING cComponentName, OMX_PTR pAppData, OMX_CALLBACKTYPE* pCallBacks, OMX_ERRORTYPE * eCompReturn)
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    RPC_OMX_MESSAGE* pRPCMsg=NULL;
    RPC_OMX_BYTE * pMsgBody = NULL;
    OMX_U32 dataOffset = 0;
    OMX_U32 dataOffset2 = 0;
    OMX_U32 offset=0;
    OMX_PTR * testVal;
    OMX_U32 nPacketSize = PACKET_SIZE;
    RcmClient_Message * pPacket=NULL;
    OMX_U32 nPos = 0;
    RPC_OMX_CONTEXT *hCtx = hRPCCtx;
    RPC_OMX_HANDLE hComp = NULL;

    OMX_S16 status;
    RPC_INDEX fxnIdx;
    OMX_U8 CallingCore = 0;
    OMX_STRING CallingCorercmServerName;

    DOMX_DEBUG("\nEntering: %s\n", __FUNCTION__);
    DOMX_DEBUG("\nRPC_GetHandle: Recieved GetHandle request from %s\n", cComponentName);

    RPC_UTIL_GetLocalServerName(cComponentName,&CallingCorercmServerName);
    DOMX_DEBUG("\n RCM Server Name Calling on Current Core: %s",CallingCorercmServerName);

    CallingCore = MultiProc_getId(NULL);
    fxnIdx = rpcHndl[TARGET_CORE_ID].rpcFxns[RPC_OMX_FXN_IDX_GET_HANDLE].rpcFxnIdx;

    //Allocating remote command message
    RPC_getPacket(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], nPacketSize, pPacket);
    pRPCMsg = (RPC_OMX_MESSAGE*)(&pPacket->data);
    pMsgBody = &pRPCMsg->msgBody[0];

    //Marshalled:[>offset(cParameterName)|>pAppData|>CallingCore|>offset(CallingCorercmServerName)|
    //>--cComponentName--|>--CallingCorercmServerName--|
    //<hComp]

    dataOffset = sizeof(OMX_U32) + sizeof(OMX_PTR) + sizeof(OMX_U8) + sizeof(OMX_U32);
    RPC_SETFIELDOFFSET(pMsgBody, nPos,  dataOffset, OMX_U32);
    //To update with RPC macros
    strcpy((char *)(pMsgBody + dataOffset),cComponentName);

    RPC_SETFIELDVALUE(pMsgBody, nPos, pAppData, OMX_PTR);
    RPC_SETFIELDVALUE(pMsgBody, nPos, CallingCore, OMX_U8);

    dataOffset2 = dataOffset + 128; //max size of omx comp name
    RPC_SETFIELDOFFSET(pMsgBody, nPos,  dataOffset2, OMX_U32);
    //To update with RPC macros
    strcpy((char *)(pMsgBody + dataOffset2),CallingCorercmServerName);

    RPC_sendPacket_sync(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket, fxnIdx);

    *eCompReturn = pRPCMsg->msgHeader.nOMXReturn;

    if(*eCompReturn == OMX_ErrorNone) {
        offset = dataOffset2 + 32; //max size of rcm server name
        RPC_GETFIELDVALUE(pMsgBody, offset, hComp, RPC_OMX_HANDLE);
        DOMX_DEBUG("%s: Received Remote Handle 0x%x\n",__FUNCTION__, hComp);
        hCtx->remoteHandle = hComp;
    }

    RcmClient_free(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket);

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
    return eRPCError;
}




/* ===========================================================================*/
/**
 * @name RPC_FreeHandle()
 * @brief Remote invocation stub for OMX_AllcateBuffer()
 * @param size   : Size of the incoming RCM message (parameter used in the RCM alloc call)
 * @param *data  : Pointer to the RCM message/buffer received
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_FreeHandle(RPC_OMX_HANDLE hRPCCtx, OMX_ERRORTYPE * eCompReturn)
{
   RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
   RPC_OMX_MESSAGE* pRPCMsg=NULL;
   RPC_OMX_BYTE * pMsgBody;
   OMX_U32 nPacketSize = PACKET_SIZE;
   RcmClient_Message * pPacket=NULL;
   OMX_S16 status;
   RPC_INDEX fxnIdx;
   OMX_U32 nPos = 0;
   RPC_OMX_CONTEXT *hCtx = hRPCCtx;
   RPC_OMX_HANDLE hComp = hCtx->remoteHandle;

   DOMX_DEBUG("Entering: %s\n", __FUNCTION__);

   fxnIdx = rpcHndl[TARGET_CORE_ID].rpcFxns[RPC_OMX_FXN_IDX_FREE_HANDLE].rpcFxnIdx;

   //Allocating remote command message
   RPC_getPacket(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], nPacketSize, pPacket);
   pRPCMsg = (RPC_OMX_MESSAGE*)(&pPacket->data);
   pMsgBody = &pRPCMsg->msgBody[0];

   //Marshalled:[>hComp]
   RPC_SETFIELDVALUE(pMsgBody, nPos, hComp, RPC_OMX_HANDLE);

   RPC_sendPacket_sync(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket, fxnIdx);

    *eCompReturn = pRPCMsg->msgHeader.nOMXReturn;

    RcmClient_free(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket);

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
    return eRPCError;

}

/* ===========================================================================*/
/**
 * @name RPC_SetParameter()
 * @brief
 * @param hComp: This is the handle on the Remote core, the proxy will replace
 *        it's handle with actual OMX Component handle that recides on the specified core
 * @param nParamIndex: same as nParamIndex received at the Proxy
 * @param pCompParam: same as nPCompParam recieved at the Proxy
 * @param eCompReturn: This is return value that will be supplied by Proxy to the caller.
          This is actual return value returned by the Remote component
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_SetParameter(RPC_OMX_HANDLE hRPCCtx, OMX_INDEXTYPE nParamIndex,
                                   OMX_PTR pCompParam, OMX_ERRORTYPE * eCompReturn)
{

    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    RcmClient_Message * pPacket=NULL;
    OMX_U32 nPacketSize = PACKET_SIZE;
    RPC_OMX_MESSAGE* pRPCMsg=NULL;
    RPC_OMX_BYTE * pMsgBody;
    RPC_INDEX fxnIdx;
    OMX_U32 nPos = 0;
    OMX_S16 status;
    RPC_OMX_CONTEXT *hCtx = hRPCCtx;
    RPC_OMX_HANDLE hComp = hCtx->remoteHandle;

    OMX_U32 structSize=0;
    OMX_U32 offset=0;

    fxnIdx = rpcHndl[TARGET_CORE_ID].rpcFxns[RPC_OMX_FXN_IDX_SET_PARAMETER].rpcFxnIdx;

    RPC_getPacket(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], nPacketSize, pPacket);

    pRPCMsg = (RPC_OMX_MESSAGE*)(&pPacket->data);
    pMsgBody = &pRPCMsg->msgBody[0];

    //Marshalled:[>hComp|>nParamIndex|>offset(pCompParam)|>--pCompParam--]
    RPC_SETFIELDVALUE(pMsgBody, nPos, hComp, RPC_OMX_HANDLE);
    RPC_SETFIELDVALUE(pMsgBody, nPos, nParamIndex, OMX_INDEXTYPE);

    offset = sizeof(RPC_OMX_HANDLE) + sizeof(OMX_INDEXTYPE) + sizeof(OMX_U32);
    RPC_SETFIELDOFFSET(pMsgBody, nPos,  offset, OMX_U32);

    structSize = RPC_UTIL_GETSTRUCTSIZE(pCompParam);
    RPC_SETFIELDCOPYGEN(pMsgBody, offset,  pCompParam, structSize);

    RPC_sendPacket_sync(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket, fxnIdx);

    *eCompReturn = pRPCMsg->msgHeader.nOMXReturn;

    RcmClient_free(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket);

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
    return eRPCError;
}

/* ===========================================================================*/
/**
 * @name RPC_GetParameter()
 * @brief
 * @param hComp: This is the handle on the Remote core, the proxy will replace
 *        it's handle with actual OMX Component handle that recides on the specified core
 * @param nParamIndex: same as nParamIndex received at the Proxy
 * @param pCompParam: same as nPCompParam recieved at the Proxy
 * @param eCompReturn: This is return value that will be supplied by Proxy to the caller.
          This is actual return value returned by the Remote component
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_GetParameter(RPC_OMX_HANDLE hRPCCtx,OMX_INDEXTYPE nParamIndex,OMX_PTR pCompParam, OMX_ERRORTYPE * eCompReturn)
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    RcmClient_Message * pPacket=NULL;
    OMX_U32 nPacketSize = PACKET_SIZE;
    RPC_OMX_MESSAGE* pRPCMsg=NULL;
    RPC_OMX_BYTE * pMsgBody;
    RPC_INDEX fxnIdx;
    OMX_U32 nPos = 0;
    OMX_S16 status;
    RPC_OMX_CONTEXT *hCtx = hRPCCtx;
    RPC_OMX_HANDLE hComp = hCtx->remoteHandle;

    OMX_U32 structSize=0;
    OMX_U32 offset=0;

    DOMX_DEBUG("Entering: %s\n", __FUNCTION__);

    fxnIdx = rpcHndl[TARGET_CORE_ID].rpcFxns[RPC_OMX_FXN_IDX_GET_PARAMETER].rpcFxnIdx;

    //Allocating remote command message
    RPC_getPacket(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], nPacketSize, pPacket);
    pRPCMsg = (RPC_OMX_MESSAGE*)(&pPacket->data);
    pMsgBody = &pRPCMsg->msgBody[0];

    //Marshalled:[>hComp|>nParamIndex|>offset(pCompParam)|><--pCompParam--]
    RPC_SETFIELDVALUE(pMsgBody, nPos, hComp, RPC_OMX_HANDLE);
    RPC_SETFIELDVALUE(pMsgBody, nPos, nParamIndex, OMX_INDEXTYPE);

    offset = sizeof(RPC_OMX_HANDLE) + sizeof(OMX_INDEXTYPE) + sizeof(OMX_U32);
    RPC_SETFIELDOFFSET(pMsgBody, nPos,  offset, OMX_U32);

    structSize = RPC_UTIL_GETSTRUCTSIZE(pCompParam);
    RPC_SETFIELDCOPYGEN(pMsgBody, offset,  pCompParam, structSize);

    RPC_sendPacket_sync(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket, fxnIdx);

    *eCompReturn = pRPCMsg->msgHeader.nOMXReturn;

    if(pRPCMsg->msgHeader.nOMXReturn == OMX_ErrorNone) {
    RPC_GETFIELDCOPYGEN(pMsgBody, offset, pCompParam, structSize);
    }

    RcmClient_free(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket);

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
    return eRPCError;
}

/* ===========================================================================*/
/**
 * @name RPC_SetConfig()
 * @brief
 * @param hComp: This is the handle on the Remote core, the proxy will replace
 *        it's handle with actual OMX Component handle that recides on the specified core
 * @param nParamIndex: same as nParamIndex received at the Proxy
 * @param pCompParam: same as nPCompParam recieved at the Proxy
 * @param eCompReturn: This is return value that will be supplied by Proxy to the caller.
          This is actual return value returned by the Remote component
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_SetConfig(RPC_OMX_HANDLE hRPCCtx,OMX_INDEXTYPE nConfigIndex,OMX_PTR pCompConfig, OMX_ERRORTYPE * eCompReturn)
{

    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    RcmClient_Message * pPacket=NULL;
    OMX_U32 nPacketSize = PACKET_SIZE;
    RPC_OMX_MESSAGE* pRPCMsg=NULL;
    RPC_OMX_BYTE * pMsgBody;
    RPC_INDEX fxnIdx;
    OMX_U32 nPos = 0;
    OMX_S16 status;
    RPC_OMX_CONTEXT *hCtx = hRPCCtx;
    RPC_OMX_HANDLE hComp = hCtx->remoteHandle;

    OMX_U32 structSize=0;
    OMX_U32 offset=0;

    DOMX_DEBUG("Entering: %s\n", __FUNCTION__);

    fxnIdx = rpcHndl[TARGET_CORE_ID].rpcFxns[RPC_OMX_FXN_IDX_SET_CONFIG].rpcFxnIdx;

    //Allocating remote command message
    RPC_getPacket(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], nPacketSize, pPacket);
    pRPCMsg = (RPC_OMX_MESSAGE*)(&pPacket->data);
    pMsgBody = &pRPCMsg->msgBody[0];

    //Marshalled:[>hComp|>nConfigIndex|>offset(pCompConfig)|>--pCompConfig--]

    RPC_SETFIELDVALUE(pMsgBody, nPos, hComp, RPC_OMX_HANDLE);
    RPC_SETFIELDVALUE(pMsgBody, nPos, nConfigIndex, OMX_INDEXTYPE);

    offset = sizeof(RPC_OMX_HANDLE) + sizeof(OMX_INDEXTYPE) + sizeof(OMX_U32);
    RPC_SETFIELDOFFSET(pMsgBody, nPos,  offset, OMX_U32);

    structSize = RPC_UTIL_GETSTRUCTSIZE(pCompConfig);
    RPC_SETFIELDCOPYGEN(pMsgBody, offset, pCompConfig, structSize);

    RPC_sendPacket_sync(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket, fxnIdx);

    *eCompReturn = pRPCMsg->msgHeader.nOMXReturn;

    RcmClient_free(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket);

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
    return eRPCError;
}

/* ===========================================================================*/
/**
 * @name RPC_GetConfig()
 * @brief Remote invocation stub for OMX_AllcateBuffer()
 * @param size   : Size of the incoming RCM message (parameter used in the RCM alloc call)
 * @param *data  : Pointer to the RCM message/buffer received
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_GetConfig(RPC_OMX_HANDLE hRPCCtx,OMX_INDEXTYPE nConfigIndex,OMX_PTR pCompConfig, OMX_ERRORTYPE * eCompReturn)
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    RcmClient_Message * pPacket=NULL;
    OMX_U32 nPacketSize = PACKET_SIZE;
    RPC_OMX_MESSAGE* pRPCMsg=NULL;
    RPC_OMX_BYTE * pMsgBody;
    RPC_INDEX fxnIdx;
    OMX_U32 nPos = 0;
    OMX_S16 status;
    RPC_OMX_CONTEXT *hCtx = hRPCCtx;
    RPC_OMX_HANDLE hComp = hCtx->remoteHandle;

    OMX_U32 structSize=0;
    OMX_U32 offset=0;

    DOMX_DEBUG("Entering: %s\n", __FUNCTION__);

    fxnIdx = rpcHndl[TARGET_CORE_ID].rpcFxns[RPC_OMX_FXN_IDX_GET_CONFIG].rpcFxnIdx;

    //Allocating remote command message
    RPC_getPacket(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], nPacketSize, pPacket);
    pRPCMsg = (RPC_OMX_MESSAGE*)(&pPacket->data);
    pMsgBody = &pRPCMsg->msgBody[0];

    //Marshalled:[>hComp|>nConfigIndex|>offset(pCompConfig)|><--pCompConfig--]

    RPC_SETFIELDVALUE(pMsgBody, nPos, hComp, RPC_OMX_HANDLE);
    RPC_SETFIELDVALUE(pMsgBody, nPos, nConfigIndex, OMX_INDEXTYPE);

    offset = sizeof(RPC_OMX_HANDLE) + sizeof(OMX_INDEXTYPE) + sizeof(OMX_U32);
    RPC_SETFIELDOFFSET(pMsgBody, nPos,  offset, OMX_U32);

    structSize = RPC_UTIL_GETSTRUCTSIZE(pCompConfig);
    RPC_SETFIELDCOPYGEN(pMsgBody, offset,  pCompConfig, structSize);

    RPC_sendPacket_sync(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket, fxnIdx);

    *eCompReturn = pRPCMsg->msgHeader.nOMXReturn;

    if(pRPCMsg->msgHeader.nOMXReturn == RPC_OMX_ErrorNone) {
        RPC_GETFIELDCOPYGEN(pMsgBody, offset, pCompConfig, structSize);
    }

    RcmClient_free(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket);

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
    return eRPCError;
}



/* ===========================================================================*/
/**
 * @name RPC_SendCommand()
 * @brief Remote invocation stub for OMX_AllcateBuffer()
 * @param size   : Size of the incoming RCM message (parameter used in the RCM alloc call)
 * @param *data  : Pointer to the RCM message/buffer received
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_SendCommand(RPC_OMX_HANDLE hRPCCtx,OMX_COMMANDTYPE eCmd,OMX_U32 nParam,OMX_PTR pCmdData,OMX_ERRORTYPE * eCompReturn)
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    RcmClient_Message * pPacket=NULL;
    OMX_U32 nPacketSize = PACKET_SIZE;
    RPC_OMX_MESSAGE* pRPCMsg=NULL;
    RPC_OMX_BYTE * pMsgBody;
    RPC_INDEX fxnIdx;
    OMX_U32 nPos = 0;
    OMX_S16 status;
    RPC_OMX_CONTEXT *hCtx = hRPCCtx;
    RPC_OMX_HANDLE hComp = hCtx->remoteHandle;

    OMX_U32 structSize=0;
    OMX_U32 offset=0;

    DOMX_DEBUG("\nEntering: %s", __FUNCTION__);

    fxnIdx = rpcHndl[TARGET_CORE_ID].rpcFxns[RPC_OMX_FXN_IDX_SEND_CMD].rpcFxnIdx;

    //Allocating remote command message
    RPC_getPacket(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], nPacketSize, pPacket);
    pRPCMsg = (RPC_OMX_MESSAGE*)(&pPacket->data);
    pMsgBody = &pRPCMsg->msgBody[0];

    //Marshalled:[>hComp|>eCmd|nParam|>offset(pCmdData)|!><--pCmdData--]

    RPC_SETFIELDVALUE(pMsgBody, nPos, hComp, RPC_OMX_HANDLE);
    RPC_SETFIELDVALUE(pMsgBody, nPos, eCmd, OMX_COMMANDTYPE);
    RPC_SETFIELDVALUE(pMsgBody, nPos, nParam, OMX_U32);

    offset = sizeof(RPC_OMX_HANDLE) + sizeof(OMX_COMMANDTYPE) + sizeof(OMX_U32) +
             sizeof(OMX_U32);

    RPC_SETFIELDOFFSET(pMsgBody, nPos,  offset, OMX_U32);

    if(pCmdData != NULL) {
        structSize = RPC_UTIL_GETSTRUCTSIZE(pCmdData);
        RPC_SETFIELDCOPYGEN(pMsgBody, offset,  pCmdData, structSize);
    }

    RPC_sendPacket_sync(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket, fxnIdx);

    *eCompReturn =pRPCMsg->msgHeader.nOMXReturn;

    RcmClient_free(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket);

EXIT:
   DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
   return eRPCError;
}


/* ===========================================================================*/
/**
 * @name RPC_AllocateBuffer()
 * @brief Remote invocation stub for OMX_AllcateBuffer()
 * @param size   : Size of the incoming RCM message (parameter used in the RCM alloc call)
 * @param *data  : Pointer to the RCM message/buffer received
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_AllocateBuffer(RPC_OMX_HANDLE hRPCCtx, OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,OMX_IN OMX_U32 nPortIndex,OMX_U32* pBufHeaderRemote,OMX_U32* pBufferMapped, OMX_PTR pAppPrivate, OMX_U32 nSizeBytes,OMX_ERRORTYPE * eCompReturn)
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    RcmClient_Message * pPacket=NULL;
    OMX_U32 nPacketSize = PACKET_SIZE;
    RPC_OMX_MESSAGE* pRPCMsg=NULL;
    RPC_OMX_BYTE * pMsgBody;
    RPC_INDEX fxnIdx;
    OMX_U32 nPos = 0;
    OMX_S16 status;
    RPC_OMX_CONTEXT *hCtx = hRPCCtx;
    RPC_OMX_HANDLE hComp = hCtx->remoteHandle;

    OMX_U32 offset=0;
    OMX_TI_PLATFORMPRIVATE *pPlatformPrivate = NULL;
    OMX_BUFFERHEADERTYPE* pBufferHdr = *ppBufferHdr;

    DOMX_DEBUG("Entering: %s\n", __FUNCTION__);

    fxnIdx = rpcHndl[TARGET_CORE_ID].rpcFxns[RPC_OMX_FXN_IDX_ALLOCATE_BUFFER].rpcFxnIdx;

    RPC_getPacket(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], nPacketSize, pPacket);
    pRPCMsg = (RPC_OMX_MESSAGE*)(&pPacket->data);
    pMsgBody = &pRPCMsg->msgBody[0];

    //Marshalled:[>hComp|>nPortIndex|>pAppPrivate|>nSizeBytes|
    //<pBufHeaderRemote|<offset(BufHeaderRemotecontents)|<offset(platformprivate)|
    //<--BufHeaderRemotecontents--|!<--pPlatformPrivate--]

    RPC_SETFIELDVALUE(pMsgBody, nPos, hComp, RPC_OMX_HANDLE);
    RPC_SETFIELDVALUE(pMsgBody, nPos, nPortIndex, OMX_U32);
    RPC_SETFIELDVALUE(pMsgBody, nPos, pAppPrivate, OMX_PTR);
    RPC_SETFIELDVALUE(pMsgBody, nPos, nSizeBytes, OMX_U32);

    RPC_sendPacket_sync(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket, fxnIdx);

    *eCompReturn = pRPCMsg->msgHeader.nOMXReturn;

    if(pRPCMsg->msgHeader.nOMXReturn == OMX_ErrorNone){

        RPC_GETFIELDVALUE(pMsgBody, nPos, *pBufHeaderRemote, OMX_U32);

        RPC_GETFIELDOFFSET(pMsgBody, nPos, offset, OMX_U32);
        //save platform private before overwriting
        pPlatformPrivate = (*ppBufferHdr)->pPlatformPrivate;
        RPC_GETFIELDCOPYTYPE(pMsgBody, offset, pBufferHdr, OMX_BUFFERHEADERTYPE);
        (*ppBufferHdr)->pPlatformPrivate = pPlatformPrivate;

        #ifdef TILER_BUFF
        DOMX_DEBUG("\n Copying plat pvt. \n");
        RPC_GETFIELDOFFSET(pMsgBody, nPos, offset, OMX_U32);
        if(offset !=0) {
        RPC_GETFIELDCOPYTYPE(pMsgBody, offset, (OMX_TI_PLATFORMPRIVATE *)((*ppBufferHdr)->pPlatformPrivate), OMX_TI_PLATFORMPRIVATE);
           DOMX_DEBUG("\nDone copying plat pvt., aux buf = 0x%x\n", ((OMX_TI_PLATFORMPRIVATE *)((*ppBufferHdr)->pPlatformPrivate))->pAuxBuf1);
        }
        #endif

        *pBufferMapped = (OMX_U32) (*ppBufferHdr )->pBuffer;
}
else {
    DOMX_DEBUG("%s: OMX Error received: 0x%x",__FUNCTION__,pRPCMsg->msgHeader.nOMXReturn);
}

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
    RcmClient_free(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket);
    return eRPCError;
}

/* ===========================================================================*/
/**
 * @name RPC_UseBuffer()
 * @brief Remote invocation stub for OMX_AllcateBuffer()
 * @param hComp: This is the handle on the Remote core, the proxy will replace
                 it's handle with actual OMX Component handle that recides on the specified core
 * @param ppBufferHdr:
 * @param nPortIndex:
 * @param pAppPrivate:
 * @param eCompReturn: This is return value that will be supplied by Proxy to the caller.
 *                    This is actual return value returned by the Remote component
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_UseBuffer(RPC_OMX_HANDLE hRPCCtx,OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr, OMX_U32 nPortIndex, OMX_PTR pAppPrivate, OMX_U32 nSizeBytes, OMX_U8* pBuffer,OMX_U32* pBufferMapped,OMX_U32* pBufHeaderRemote, OMX_ERRORTYPE * eCompReturn)
{

    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    RcmClient_Message * pPacket=NULL;
    OMX_U32 nPacketSize = PACKET_SIZE;
    RPC_OMX_MESSAGE* pRPCMsg=NULL;
    RPC_OMX_BYTE * pMsgBody;
    RPC_INDEX fxnIdx;
    OMX_U32 nPos = 0;
    OMX_S16 status;
    RPC_OMX_CONTEXT *hCtx = hRPCCtx;
    RPC_OMX_HANDLE hComp = hCtx->remoteHandle;

    OMX_U32 offset=0;
    OMX_U32 mappedAddress =0x00;
    OMX_U32 mappedAddress2 =0x00;
    OMX_TI_PLATFORMPRIVATE *pPlatformPrivate = NULL;
    OMX_BUFFERHEADERTYPE* pBufferHdr = *ppBufferHdr;

    DOMX_DEBUG("Entering: %s\n", __FUNCTION__);

    fxnIdx = rpcHndl[TARGET_CORE_ID].rpcFxns[RPC_OMX_FXN_IDX_USE_BUFFER].rpcFxnIdx;

    RPC_getPacket(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], nPacketSize, pPacket);
    pRPCMsg = (RPC_OMX_MESSAGE*)(&pPacket->data);
    pMsgBody = &pRPCMsg->msgBody[0];

    mappedAddress = (OMX_U32) (*ppBufferHdr)->pBuffer;

    #ifdef TILER_BUFF
    DOMX_DEBUG("\n Getting aux buf\n");
    mappedAddress2 = (OMX_U32) ((OMX_TI_PLATFORMPRIVATE *) ((*ppBufferHdr)->pPlatformPrivate))->pAuxBuf1;
    #endif

    DOMX_DEBUG("\n DEBUG - MAPPING - pBuffer = %x",pBuffer);
    DOMX_DEBUG("\n DEBUG - MAPPING - mappedAddress = %x",mappedAddress);

    //Marshalled:[>hComp|>nPortIndex|>pAppPrivate|>nSizeBytes|
    //>mappedAddress|!>mappedAddress2|<pBufHeaderRemote|
    //<offset(BufHeaderRemotecontents)|<offset(platformprivate)|
    //<--pBufferHdr--|!<--pPlatformPrivate--]

    RPC_SETFIELDVALUE(pMsgBody, nPos, hComp, RPC_OMX_HANDLE);
    RPC_SETFIELDVALUE(pMsgBody, nPos, nPortIndex, OMX_U32);
    RPC_SETFIELDVALUE(pMsgBody, nPos, pAppPrivate, OMX_PTR);
    RPC_SETFIELDVALUE(pMsgBody, nPos, nSizeBytes, OMX_U32);
    RPC_SETFIELDVALUE(pMsgBody, nPos, mappedAddress, OMX_U32);

    #ifdef TILER_BUFF
    RPC_SETFIELDVALUE(pMsgBody, nPos, mappedAddress2, OMX_U32);
    #endif

    RPC_sendPacket_sync(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket, fxnIdx);

    *eCompReturn = pRPCMsg->msgHeader.nOMXReturn;

     //If you are here then Send is successful and you have go the return value
    if(pRPCMsg->msgHeader.nOMXReturn == OMX_ErrorNone){

        RPC_GETFIELDVALUE(pMsgBody, nPos, *pBufHeaderRemote, OMX_U32);

        DOMX_DEBUG("\n Getting offset for buffer header:\n");
        RPC_GETFIELDOFFSET(pMsgBody, nPos, offset, OMX_U32);
        DOMX_DEBUG("\n Copying buffer header at offset:%d\n", offset);

        //save platform private before overwriting
        pPlatformPrivate = (*ppBufferHdr)->pPlatformPrivate;

        DOMX_DEBUG("\n Copying buffer header at offset:%d\n", offset);
        RPC_GETFIELDCOPYTYPE(pMsgBody, offset, pBufferHdr, OMX_BUFFERHEADERTYPE);
        (*ppBufferHdr)->pPlatformPrivate = pPlatformPrivate;
        *pBufferMapped = mappedAddress;

        #ifdef TILER_BUFF
        DOMX_DEBUG("\n Copying plat pvt. \n");

        RPC_GETFIELDOFFSET(pMsgBody, nPos, offset, OMX_U32);
        if(offset !=0) {
        RPC_GETFIELDCOPYTYPE(pMsgBody, offset, (OMX_TI_PLATFORMPRIVATE *)((*ppBufferHdr)->pPlatformPrivate), OMX_TI_PLATFORMPRIVATE);
        DOMX_DEBUG("\nDone copying plat pvt., aux buf = 0x%x\n", ((OMX_TI_PLATFORMPRIVATE *)((*ppBufferHdr)->pPlatformPrivate))->pAuxBuf1);
        }
        #endif
    }
    else {
        DOMX_DEBUG("\n %s: OMX Error received: 0x%x",__FUNCTION__,pRPCMsg->msgHeader.nOMXReturn);
        *pBufferMapped = 0;
    }

        RcmClient_free(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket);

EXIT:
        DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
        return eRPCError;
}

/* ===========================================================================*/
/**
 * @name RPC_FreeBuffer()
 * @brief Remote invocation stub for OMX_AllcateBuffer()
 * @param size   : Size of the incoming RCM message (parameter used in the RCM alloc call)
 * @param *data  : Pointer to the RCM message/buffer received
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_FreeBuffer(RPC_OMX_HANDLE hRPCCtx,OMX_IN  OMX_U32 nPortIndex, OMX_IN  OMX_U32 BufHdrRemote, OMX_ERRORTYPE * eCompReturn)
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    RcmClient_Message * pPacket=NULL;
    OMX_U32 nPacketSize = PACKET_SIZE;
    RPC_OMX_MESSAGE* pRPCMsg=NULL;
    RPC_OMX_BYTE * pMsgBody;
    RPC_INDEX fxnIdx;
    OMX_U32 nPos = 0;
    OMX_S16 status;
    RPC_OMX_CONTEXT *hCtx = hRPCCtx;
    RPC_OMX_HANDLE hComp = hCtx->remoteHandle;

    DOMX_DEBUG("Entering: %s\n", __FUNCTION__);

    fxnIdx = rpcHndl[TARGET_CORE_ID].rpcFxns[RPC_OMX_FXN_IDX_FREE_BUFFER].rpcFxnIdx;
    RPC_getPacket(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], nPacketSize, pPacket);
    pRPCMsg = (RPC_OMX_MESSAGE*)(&pPacket->data);
    pMsgBody = &pRPCMsg->msgBody[0];

    //Marshalled:[>hComp|>nPortIndex|>BufHdrRemote|

    RPC_SETFIELDVALUE(pMsgBody, nPos, hComp, RPC_OMX_HANDLE);
    RPC_SETFIELDVALUE(pMsgBody, nPos, nPortIndex, OMX_U32);

    RPC_SETFIELDVALUE(pMsgBody, nPos, BufHdrRemote, OMX_U32);

    RPC_sendPacket_sync(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket, fxnIdx);

    *eCompReturn = pRPCMsg->msgHeader.nOMXReturn;

    RcmClient_free(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket);

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
    return eRPCError;
}


/* ===========================================================================*/
/**
 * @name RPC_EmptyThisBuffer()
 * @brief
 * @param size   : Size of the incoming RCM message (parameter used in the RCM alloc call)
 * @param *data  : Pointer to the RCM message/buffer received
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/

RPC_OMX_ERRORTYPE RPC_EmptyThisBuffer(RPC_OMX_HANDLE hRPCCtx, OMX_BUFFERHEADERTYPE* pBufferHdr, OMX_U32 BufHdrRemote, OMX_ERRORTYPE * eCompReturn)
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    RcmClient_Message * pPacket=NULL;
    OMX_U32 nPacketSize = PACKET_SIZE;
    RPC_OMX_MESSAGE* pRPCMsg=NULL;
    RPC_OMX_BYTE * pMsgBody;
    RPC_INDEX fxnIdx;
    OMX_U32 nPos = 0;
    OMX_S16 status;
    RPC_OMX_CONTEXT *hCtx = hRPCCtx;
    RPC_OMX_HANDLE hComp = hCtx->remoteHandle;

    OMX_U8* pAuxBuf1=NULL;

    DOMX_DEBUG("\n Entering: %s __________ BUFFER READ-WRITE ", __FUNCTION__);

    fxnIdx = rpcHndl[TARGET_CORE_ID].rpcFxns[RPC_OMX_FXN_IDX_EMPTYTHISBUFFER].rpcFxnIdx;

    RPC_getPacket(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], nPacketSize, pPacket);
    pRPCMsg = (RPC_OMX_MESSAGE*)(&pPacket->data);
    pMsgBody = &pRPCMsg->msgBody[0];

    //Marshalled:[>hComp|>BufHdrRemote|>nFilledLen|>nOffset|>nFlags|>nTimeStamp]

    RPC_SETFIELDVALUE(pMsgBody, nPos, hComp, RPC_OMX_HANDLE);
    RPC_SETFIELDVALUE(pMsgBody, nPos, (OMX_BUFFERHEADERTYPE*)BufHdrRemote, OMX_BUFFERHEADERTYPE*);

    RPC_SETFIELDVALUE(pMsgBody, nPos, pBufferHdr->pBuffer, OMX_U8*);
    /*Check if valid platformPrivate exists, if so set AuxBuf1. It is always marshalled
    even though it could be NULL*/
    if(pBufferHdr->pPlatformPrivate != NULL) {
        pAuxBuf1 = ((OMX_TI_PLATFORMPRIVATE *) (pBufferHdr->pPlatformPrivate))->pAuxBuf1;
    }
    RPC_SETFIELDVALUE(pMsgBody, nPos, pAuxBuf1, OMX_U8*);

    RPC_SETFIELDVALUE(pMsgBody, nPos, pBufferHdr->nFilledLen, OMX_U32);
    RPC_SETFIELDVALUE(pMsgBody, nPos, pBufferHdr->nOffset, OMX_U32);
    RPC_SETFIELDVALUE(pMsgBody, nPos, pBufferHdr->nFlags, OMX_U32);
    RPC_SETFIELDVALUE(pMsgBody, nPos, pBufferHdr->nTimeStamp, OMX_TICKS);

    DOMX_DEBUG("\n pBufferHdr = %x BufHdrRemote %x",pBufferHdr,BufHdrRemote);

    RPC_sendPacket_async(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket, fxnIdx);

    *eCompReturn = OMX_ErrorNone;//pRPCMsg->msgHeader.nOMXReturn;

EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
    return eRPCError;
}


/* ===========================================================================*/
/**
 * @name RPC_FillThisBuffer()
 * @brief Remote invocation stub for OMX_AllcateBuffer()
 * @param size   : Size of the incoming RCM message (parameter used in the RCM alloc call)
 * @param *data  : Pointer to the RCM message/buffer received
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_FillThisBuffer(RPC_OMX_HANDLE hRPCCtx, OMX_BUFFERHEADERTYPE* pBufferHdr, OMX_U32 BufHdrRemote, OMX_ERRORTYPE * eCompReturn)
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    RcmClient_Message * pPacket=NULL;
    OMX_U32 nPacketSize = PACKET_SIZE;
    RPC_OMX_MESSAGE* pRPCMsg=NULL;
    RPC_OMX_BYTE * pMsgBody;
    RPC_INDEX fxnIdx;
    OMX_U32 nPos = 0;
    OMX_S16 status;
    RPC_OMX_CONTEXT *hCtx = hRPCCtx;
    RPC_OMX_HANDLE hComp = hCtx->remoteHandle;

    OMX_U8* pAuxBuf1;

    DOMX_DEBUG("\n Entering: %s __________ BUFFER READ-WRITE ", __FUNCTION__);

    fxnIdx = rpcHndl[TARGET_CORE_ID].rpcFxns[RPC_OMX_FXN_IDX_FILLTHISBUFFER].rpcFxnIdx;

    RPC_getPacket(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], nPacketSize, pPacket);
    pRPCMsg = (RPC_OMX_MESSAGE*)(&pPacket->data);
    pMsgBody = &pRPCMsg->msgBody[0];

    //Marshalled:[>hComp|>BufHdrRemote|>nFilledLen|>nOffset|>nFlags]

    RPC_SETFIELDVALUE(pMsgBody, nPos, hComp, RPC_OMX_HANDLE);
    RPC_SETFIELDVALUE(pMsgBody, nPos, (OMX_BUFFERHEADERTYPE*)BufHdrRemote, OMX_BUFFERHEADERTYPE*);

    RPC_SETFIELDVALUE(pMsgBody, nPos, pBufferHdr->pBuffer, OMX_U8*);
    /*Check if valid platformPrivate exists, if so set AuxBuf1. It is always marshalled
    even though it could be NULL*/
    if(pBufferHdr->pPlatformPrivate != NULL) {
        pAuxBuf1 = ((OMX_TI_PLATFORMPRIVATE *) (pBufferHdr->pPlatformPrivate))->pAuxBuf1;
    }
    RPC_SETFIELDVALUE(pMsgBody, nPos, pAuxBuf1, OMX_U8*);

    RPC_SETFIELDVALUE(pMsgBody, nPos, pBufferHdr->nFilledLen, OMX_U32);
    RPC_SETFIELDVALUE(pMsgBody, nPos, pBufferHdr->nOffset, OMX_U32);
    RPC_SETFIELDVALUE(pMsgBody, nPos, pBufferHdr->nFlags, OMX_U32);

    DOMX_DEBUG("\n pBufferHdr = %x BufHdrRemote %x",pBufferHdr,BufHdrRemote);

//#ifdef _Android
#if 0
       //Workaround since messages getting lost in syslink communication
	RPC_sendPacket_sync(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket, fxnIdx);

	*eCompReturn = pRPCMsg->msgHeader.nOMXReturn;

	RcmClient_free(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket);
#else
    RPC_sendPacket_async(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket, fxnIdx);

    *eCompReturn = OMX_ErrorNone;//pRPCMsg->msgHeader.nOMXReturn;
#endif
EXIT:
    DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
    return eRPCError;
}

/* ===========================================================================*/
/**
 * @name RPC_GetState()
 * @brief Remote invocation stub for OMX_AllcateBuffer()
 * @param size   : Size of the incoming RCM message (parameter used in the RCM alloc call)
 * @param *data  : Pointer to the RCM message/buffer received
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_GetState(RPC_OMX_HANDLE hRPCCtx,OMX_STATETYPE* pState, OMX_ERRORTYPE * eCompReturn)
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    RcmClient_Message * pPacket=NULL;
    OMX_U32 nPacketSize = PACKET_SIZE;
    RPC_OMX_MESSAGE* pRPCMsg=NULL;
    RPC_OMX_BYTE * pMsgBody;
    RPC_INDEX fxnIdx;
    OMX_U32 nPos = 0;
    OMX_S16 status;
    RPC_OMX_CONTEXT *hCtx = hRPCCtx;
    RPC_OMX_HANDLE hComp = hCtx->remoteHandle;

    OMX_U32 offset=0;

    DOMX_DEBUG("Entering: %s\n", __FUNCTION__);

    *pState = OMX_StateInvalid;

    fxnIdx = rpcHndl[TARGET_CORE_ID].rpcFxns[RPC_OMX_FXN_IDX_GET_STATE].rpcFxnIdx;

    RPC_getPacket(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], nPacketSize, pPacket);
    pRPCMsg = (RPC_OMX_MESSAGE*)(&pPacket->data);
    pMsgBody = &pRPCMsg->msgBody[0];

    //Marshalled:[>hComp|>offset(pState)|<--pState--]
    RPC_SETFIELDVALUE(pMsgBody, nPos, hComp, RPC_OMX_HANDLE);

    RPC_sendPacket_sync(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket, fxnIdx);

    *eCompReturn =pRPCMsg->msgHeader.nOMXReturn;

    if(pRPCMsg->msgHeader.nOMXReturn == OMX_ErrorNone) {
       RPC_GETFIELDOFFSET(pMsgBody, nPos,  offset, OMX_U32);
       RPC_GETFIELDCOPYTYPE(pMsgBody, offset, pState, OMX_STATETYPE);
    }

    RcmClient_free(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket);

EXIT:
  DOMX_DEBUG("Exited: %s\n",__FUNCTION__);
  return eRPCError;
}



/* ===========================================================================*/
/**
 * @name RPC_GetComponentVersion()
 * @brief Remote invocation stub for OMX_AllcateBuffer()
 * @param size   : Size of the incoming RCM message (parameter used in the RCM alloc call)
 * @param *data  : Pointer to the RCM message/buffer received
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_GetComponentVersion(RPC_OMX_HANDLE hRPCCtx, OMX_STRING pComponentName, OMX_VERSIONTYPE* pComponentVersion, OMX_VERSIONTYPE* pSpecVersion, OMX_UUIDTYPE* pComponentUUID, OMX_ERRORTYPE * eCompReturn)
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    RPC_OMX_MESSAGE* pRPCMsg = NULL;
    RPC_OMX_BYTE * pMsgBody =NULL;
    OMX_U32 offset = 0;
    VERSION_INFO *pVer;
    OMX_U32 nPacketSize = PACKET_SIZE;
    RcmClient_Message * pPacket=NULL;
    OMX_S16 status;
    RPC_INDEX fxnIdx;
    OMX_U32 nPos = 0;
    RPC_OMX_CONTEXT *hCtx = hRPCCtx;
    RPC_OMX_HANDLE hComp = hCtx->remoteHandle;

    DOMX_DEBUG("Entering: %s\n", __FUNCTION__);

    fxnIdx = rpcHndl[TARGET_CORE_ID].rpcFxns[RPC_OMX_FXN_IDX_GET_VERSION].rpcFxnIdx;

    if ((NULL == pComponentName)
      || (NULL == pComponentVersion)
      || (NULL == pSpecVersion)
      || (NULL == eCompReturn))
    {
      goto EXIT;
    }

    //Allocating remote command message

RPC_getPacket(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], nPacketSize, pPacket);
pRPCMsg = (RPC_OMX_MESSAGE*)(&pPacket->data);
    pMsgBody = &pRPCMsg->msgBody[0];

    //Marshalled:[>hComp|>offset(pComponentName)|>offset(pComponentVersion)|
    //>offset(pSpecVersion)|>offset(pComponentUUID)|
    //<--pComponentName--|<>--pComponentVersion--|
    //<>--pSpecVersion|<--pComponentUUID--]

    RPC_SETFIELDVALUE(pMsgBody, nPos, hComp, RPC_OMX_HANDLE);

    offset = GET_PARAM_DATA_OFFSET; // strange - why this offset vs just 4 bytes?
    RPC_SETFIELDOFFSET(pMsgBody, nPos,  offset, OMX_U32);

    RPC_sendPacket_sync(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket, fxnIdx);

    *eCompReturn = pRPCMsg->msgHeader.nOMXReturn;

    if( pRPCMsg->msgHeader.nOMXReturn == OMX_ErrorNone) {/*** OMX return***/
    pVer = ((VERSION_INFO *) (pMsgBody + (OMX_U32)GET_PARAM_DATA_OFFSET));
    strcpy(pComponentName, pVer->cName);
    TIMM_OSAL_Memcpy(pComponentVersion, &(pVer->sVersion.s), sizeof(pVer->sVersion.s));
    TIMM_OSAL_Memcpy(pSpecVersion, &(pVer->sSpecVersion.s), sizeof(pVer->sSpecVersion.s));
    }

  RcmClient_free(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket);

 EXIT:
  return eRPCError;
}



/* ===========================================================================*/
/**
 * @name RPC_GetExtensionIndex()
 * @brief Remote invocation stub for OMX_AllcateBuffer()
 * @param size   : Size of the incoming RCM message (parameter used in the RCM alloc call)
 * @param *data  : Pointer to the RCM message/buffer received
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_GetExtensionIndex(RPC_OMX_HANDLE hRPCCtx,OMX_STRING cParameterName, OMX_INDEXTYPE* pIndexType, OMX_ERRORTYPE * eCompReturn)
{

  RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
  RPC_OMX_MESSAGE* pRPCMsg=NULL;
  RPC_OMX_BYTE * pMsgBody=NULL;
  OMX_U32  offset = 0;
  OMX_U32 i_offset;
  OMX_U32 nPos = 0;
  RPC_OMX_CONTEXT *hCtx = hRPCCtx;
  RPC_OMX_HANDLE hComp = hCtx->remoteHandle;

  OMX_U32 nPacketSize = PACKET_SIZE;
  RcmClient_Message * pPacket=NULL;

  OMX_S16 status;
  RPC_INDEX fxnIdx;

  fxnIdx = rpcHndl[TARGET_CORE_ID].rpcFxns[RPC_OMX_FXN_IDX_GET_EXT_INDEX].rpcFxnIdx;

  //Allocating remote command message
  RPC_getPacket(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], nPacketSize, pPacket);
  pRPCMsg = (RPC_OMX_MESSAGE*)(&pPacket->data);
  pMsgBody = &pRPCMsg->msgBody[0];

  //Marshalled:[>hComp|>offset(cParameterName)|
  //>--cParameterName--|<offset(IndexType)|<--pIndexType--]

  RPC_SETFIELDVALUE(pMsgBody, nPos, hComp, RPC_OMX_HANDLE);

  offset=sizeof(RPC_OMX_HANDLE) + sizeof(OMX_U32) + sizeof(OMX_U32);
  RPC_SETFIELDOFFSET(pMsgBody, nPos,  offset, OMX_U32);

  //can assert this value at proxy
  if(strlen(cParameterName) <= 128) {
    strcpy((pMsgBody+offset), cParameterName);
  }

  RPC_sendPacket_sync(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket, fxnIdx);

  *eCompReturn = pRPCMsg->msgHeader.nOMXReturn;

  if( pRPCMsg->msgHeader.nOMXReturn == OMX_ErrorNone) {
      RPC_GETFIELDOFFSET(pMsgBody, nPos,  i_offset, OMX_U32);
      RPC_GETFIELDCOPYTYPE(pMsgBody, i_offset, pIndexType, OMX_INDEXTYPE);
  }

  RcmClient_free(hCtx->ClientHndl[RCM_DEFAULT_CLIENT], pPacket);

 EXIT:
  return eRPCError;

}

/* ===========================================================================*/
/**
 * @name EMPTY-STUB
 * @brief
 * @param
 * @return
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE RPC_EventHandler(RPC_OMX_HANDLE hRPCCtx,OMX_PTR pAppData,OMX_EVENTTYPE eEvent,OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData)
{
	 return RPC_OMX_ErrorNone;
}

OMX_ERRORTYPE RPC_EmptyBufferDone(RPC_OMX_HANDLE hRPCCtx, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
	 return RPC_OMX_ErrorNone;
}

OMX_ERRORTYPE RPC_FillBufferDone(RPC_OMX_HANDLE hRPCCtx, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
	 return RPC_OMX_ErrorNone;
}

RPC_OMX_ERRORTYPE RPC_ComponentTunnelRequest(RPC_OMX_HANDLE hRPCCtx, OMX_IN  OMX_U32 nPort,RPC_OMX_HANDLE hTunneledremoteHandle,OMX_U32 nTunneledPort, OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup,OMX_ERRORTYPE * nCmdStatus)
{
	 return RPC_OMX_ErrorNone;
}
