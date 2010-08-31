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
 *  @file  omx_rpc_skel.c
 *         This file contains methods that provides the functionality for
 *         the OpenMAX1.1 DOMX Framework RPC Skeleton implementations.
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

#include <stdio.h>
#include <string.h>

#include <timm_osal_interfaces.h>
#include <OMX_TI_Common.h>

#include "omx_rpc.h"
#include "omx_proxy_common.h"
#include "omx_rpc_skel.h"
#include "omx_rpc_utils.h"
#include "omx_rpc_internal.h"

/* FUNCTION LIST*/
/*
RPC_OMX_ERRORTYPE RPC_SKEL_EmptyBufferDone(UInt32 size, UInt32 *data)
RPC_OMX_ERRORTYPE RPC_SKEL_FillBufferDone(UInt32 size, UInt32 *data)
RPC_OMX_ERRORTYPE RPC_SKEL_EventHandler(UInt32 size, UInt32 *data)
*/


/* ===========================================================================*/
/**
 * @name RPC_SKEL_FillBufferDone()
 * @brief Skeleton for OMX_AllocateBuffer()
 * @param size   : Size of the incoming RCM message (parameter used in the RCM alloc call)
 * @param *data  : Pointer to the RCM message/buffer received
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_SKEL_EmptyBufferDone(UInt32 size, UInt32 * data)
{

	OMX_HANDLETYPE hComp;
	RPC_OMX_ERRORTYPE tRPCError = RPC_OMX_ErrorNone;
	OMX_U32 nfilledLen = 0;
	OMX_U32 nOffset = 0;
	OMX_U32 nFlags = 0;
	OMX_U32 bufferHdr;
	OMX_COMPONENTTYPE *pHandle = NULL;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;
	OMX_U32 nPos = 0;
	RPC_OMX_MESSAGE *recdMsg;
	RPC_OMX_BYTE *pMsgBody = NULL;
	recdMsg = (RPC_OMX_MESSAGE *) (data);

	pMsgBody = &recdMsg->msgBody[0];

	DOMX_ENTER("");

	//Marshalled:[>hComp|>bufferHdr|>nFilledLen|>nOffset|>nFlags]

	RPC_GETFIELDVALUE(pMsgBody, nPos, hComp, OMX_HANDLETYPE);
	pHandle = (OMX_COMPONENTTYPE *) hComp;
	pCompPrv = (PROXY_COMPONENT_PRIVATE *) pHandle->pComponentPrivate;

	RPC_GETFIELDVALUE(pMsgBody, nPos, bufferHdr, OMX_U32);

	RPC_GETFIELDVALUE(pMsgBody, nPos, nfilledLen, OMX_U32);
	RPC_GETFIELDVALUE(pMsgBody, nPos, nOffset, OMX_U32);
	RPC_GETFIELDVALUE(pMsgBody, nPos, nFlags, OMX_U32);

	tRPCError =
	    pCompPrv->proxyEmptyBufferDone(hComp, bufferHdr, nfilledLen,
	    nOffset, nFlags);

	DOMX_EXIT("");
	return tRPCError;
}

/* ===========================================================================*/
/**
 * @name RPC_SKEL_FillBufferDone()
 * @brief Skeleton for OMX_AllocateBuffer()
 * @param size   : Size of the incoming RCM message (parameter used in the RCM alloc call)
 * @param *data  : Pointer to the RCM message/buffer received
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_SKEL_FillBufferDone(UInt32 size, UInt32 * data)
{

	OMX_HANDLETYPE hComp;
	RPC_OMX_ERRORTYPE tRPCError = RPC_OMX_ErrorNone;
	OMX_U32 nfilledLen = 0;
	OMX_U32 nOffset = 0;
	OMX_U32 nFlags = 0;
	OMX_TICKS nTimeStamp;
	OMX_U32 bufferHdr;
	OMX_COMPONENTTYPE *pHandle = NULL;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;
	OMX_U32 nPos = 0;
	RPC_OMX_MESSAGE *recdMsg;
	RPC_OMX_BYTE *pMsgBody = NULL;
	OMX_HANDLETYPE hMarkTargetComponent = NULL;
	OMX_PTR pMarkData = NULL;
	recdMsg = (RPC_OMX_MESSAGE *) (data);

	pMsgBody = &recdMsg->msgBody[0];

	DOMX_ENTER("");

	//Marshalled:[>hComp|>bufferHdr|>nFilledLen|>nOffset|>nFlags|>nTimeStamp]

	RPC_GETFIELDVALUE(pMsgBody, nPos, hComp, OMX_HANDLETYPE);
	pHandle = (OMX_COMPONENTTYPE *) hComp;
	pCompPrv = (PROXY_COMPONENT_PRIVATE *) pHandle->pComponentPrivate;

	RPC_GETFIELDVALUE(pMsgBody, nPos, bufferHdr, OMX_U32);
	RPC_GETFIELDVALUE(pMsgBody, nPos, nfilledLen, OMX_U32);
	RPC_GETFIELDVALUE(pMsgBody, nPos, nOffset, OMX_U32);
	RPC_GETFIELDVALUE(pMsgBody, nPos, nFlags, OMX_U32);
	RPC_GETFIELDVALUE(pMsgBody, nPos, nTimeStamp, OMX_TICKS);
	RPC_GETFIELDVALUE(pMsgBody, nPos, hMarkTargetComponent,
	    OMX_HANDLETYPE);
	RPC_GETFIELDVALUE(pMsgBody, nPos, pMarkData, OMX_PTR);

	tRPCError =
	    pCompPrv->proxyFillBufferDone(hComp, bufferHdr, nfilledLen,
	    nOffset, nFlags, nTimeStamp, hMarkTargetComponent, pMarkData);

	DOMX_EXIT("");
	return tRPCError;
}



/* ===========================================================================*/
/**
 * @name RPC_SKEL_EventHandler()
 * @brief Skeleton for OMX_AllocateBuffer()
 * @param size   : Size of the incoming RCM message (parameter used in the RCM alloc call)
 * @param *data  : Pointer to the RCM message/buffer received
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_SKEL_EventHandler(UInt32 size, UInt32 * data)
{
	OMX_HANDLETYPE hComp;
	RPC_OMX_ERRORTYPE tRPCError = RPC_OMX_ErrorNone;
	OMX_COMPONENTTYPE *pHandle = NULL;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;
	OMX_EVENTTYPE eEvent;
	OMX_U32 nData1;
	OMX_U32 nData2;
	OMX_PTR pEventData;
	OMX_U32 nPos = 0;
	RPC_OMX_MESSAGE *recdMsg;
	RPC_OMX_BYTE *pMsgBody = NULL;
	recdMsg = (RPC_OMX_MESSAGE *) (data);
	pMsgBody = &recdMsg->msgBody[0];

	DOMX_ENTER("");

	//Marshalled:[>hComp|>eEvent|>nData1|>nData2|>pEventData]

	RPC_GETFIELDVALUE(pMsgBody, nPos, hComp, OMX_HANDLETYPE);
	pHandle = (OMX_COMPONENTTYPE *) hComp;
	pCompPrv = (PROXY_COMPONENT_PRIVATE *) pHandle->pComponentPrivate;

	RPC_GETFIELDVALUE(pMsgBody, nPos, eEvent, OMX_EVENTTYPE);
	RPC_GETFIELDVALUE(pMsgBody, nPos, nData1, OMX_U32);
	RPC_GETFIELDVALUE(pMsgBody, nPos, nData2, OMX_U32);
	RPC_GETFIELDVALUE(pMsgBody, nPos, pEventData, OMX_PTR);

	DOMX_DEBUG
	    ("\n**__**Got eventhandler from Remote Core succesffuly - values are : **__**");
	DOMX_DEBUG
	    ("hComp :0x%x,eEvent: 0x%x,nData1: 0x%x,nData2: 0x%x,pEventData: 0x%x",
	    hComp, eEvent, nData1, nData2, pEventData);
	DOMX_DEBUG("Just Before Entering Proxy Event handler call");

	tRPCError =
	    pCompPrv->proxyEventHandler(hComp, pCompPrv->pILAppData, eEvent,
	    nData1, nData2, pEventData);

	DOMX_EXIT("");
	return tRPCError;

}

/* ===========================================================================*/
/**
 * @name EMPTY-SKEL
 * @brief
 * @param
 * @return
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_SKEL_GetHandle(UInt32 size, UInt32 * data)
{
	return RPC_OMX_ErrorNone;
}

RPC_OMX_ERRORTYPE RPC_SKEL_SetParameter(UInt32 size, UInt32 * data)
{
	return RPC_OMX_ErrorNone;
}

RPC_OMX_ERRORTYPE RPC_SKEL_GetParameter(UInt32 size, UInt32 * data)
{
	return RPC_OMX_ErrorNone;
}

RPC_OMX_ERRORTYPE RPC_SKEL_FreeHandle(UInt32 size, UInt32 * data)
{
	return RPC_OMX_ErrorNone;
}

RPC_OMX_ERRORTYPE RPC_SKEL_EmptyThisBuffer(UInt32 size, UInt32 * data)
{
	return RPC_OMX_ErrorNone;
}

RPC_OMX_ERRORTYPE RPC_SKEL_FillThisBuffer(UInt32 size, UInt32 * data)
{
	return RPC_OMX_ErrorNone;
}

RPC_OMX_ERRORTYPE RPC_SKEL_UseBuffer(UInt32 size, UInt32 * data)
{
	return RPC_OMX_ErrorNone;
}

RPC_OMX_ERRORTYPE RPC_SKEL_FreeBuffer(UInt32 size, UInt32 * data)
{
	return RPC_OMX_ErrorNone;
}

RPC_OMX_ERRORTYPE RPC_SKEL_SetConfig(UInt32 size, UInt32 * data)
{
	return RPC_OMX_ErrorNone;
}

RPC_OMX_ERRORTYPE RPC_SKEL_GetConfig(UInt32 size, UInt32 * data)
{
	return RPC_OMX_ErrorNone;
}

RPC_OMX_ERRORTYPE RPC_SKEL_GetState(UInt32 size, UInt32 * data)
{
	return RPC_OMX_ErrorNone;
}

RPC_OMX_ERRORTYPE RPC_SKEL_SendCommand(UInt32 size, UInt32 * data)
{
	return RPC_OMX_ErrorNone;
}

RPC_OMX_ERRORTYPE RPC_SKEL_GetComponentVersion(UInt32 size, UInt32 * data)
{
	return RPC_OMX_ErrorNone;
}

RPC_OMX_ERRORTYPE RPC_SKEL_GetExtensionIndex(UInt32 size, UInt32 * data)
{
	return RPC_OMX_ErrorNone;
}

RPC_OMX_ERRORTYPE RPC_SKEL_AllocateBuffer(UInt32 size, UInt32 * data)
{
	return RPC_OMX_ErrorNone;
}

RPC_OMX_ERRORTYPE RPC_SKEL_ComponentTunnelRequest(UInt32 size, UInt32 * data)
{
	return RPC_OMX_ErrorNone;
}
