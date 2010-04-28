/*=========================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 *  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied.
 * ==========================================================================*/

/**
 *  @file  omx_rpc_skel.h
 *         This file contains methods that provides the functionality for
 *         the OpenMAX1.1 DOMX Framework RPC.
 *
 *  @path \WTSD_DucatiMMSW\framework\domx\omx_rpc\inc
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


#ifndef OMX_RPC_SKELH
#define OMX_RPC_SKELH

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/

#include "../inc/omx_rpc_internal.h"
#include "../inc/omx_rpc_stub.h"

RPC_OMX_ERRORTYPE RPC_SKEL_EmptyBufferDone(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_FillBufferDone(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_EventHandler(UInt32 size, UInt32 *data);

/*Empty SKEL*/
RPC_OMX_ERRORTYPE RPC_SKEL_GetHandle(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_SetParameter(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_GetParameter(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_FreeHandle(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_EmptyThisBuffer(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_FillThisBuffer(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_UseBuffer(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_FreeBuffer(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_SetConfig(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_GetConfig(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_GetState(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_SendCommand(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_GetComponentVersion(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_GetExtensionIndex(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_AllocateBuffer(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE RPC_SKEL_ComponentTunnelRequest(UInt32 size, UInt32 *data);

#endif
