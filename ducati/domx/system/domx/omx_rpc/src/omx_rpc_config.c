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
 *  @file  omx_rpc_config.c
 *         This file contains methods that provides the functionality for
 *         the OpenMAX1.1 DOMX Framework RPC.
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
 /* ----- system and platform files ----------------------------*/ 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <Std.h>

#include <OMX_Types.h>
#include <timm_osal_interfaces.h>
#include <timm_osal_trace.h>

#include <MultiProc.h>
#include <RcmClient.h>
#include <RcmServer.h>

/*-------program files ----------------------------------------*/
#include "omx_rpc.h"
#include "omx_rpc_stub.h"
#include "omx_rpc_skel.h"
#include "omx_rpc_internal.h"
#include "omx_rpc_utils.h"

/* contains configurations or structures to be passed to omx_rpc layer */
char rpcFxns[][MAX_FUNCTION_NAME_LENGTH]= {
          "RPC_SKEL_SetParameter",
          "RPC_SKEL_GetParameter",
          "RPC_SKEL_GetHandle",
          "RPC_SKEL_UseBuffer",
          
          "RPC_SKEL_FreeHandle",
          
          "RPC_SKEL_SetConfig",
          "RPC_SKEL_GetConfig",
          "RPC_SKEL_GetState",
          "RPC_SKEL_SendCommand",
          "RPC_SKEL_GetComponentVersion",
          "RPC_SKEL_GetExtensionIndex",
          "RPC_SKEL_FillThisBuffer",
          "RPC_SKEL_FillBufferDone",
          "RPC_SKEL_FreeBuffer",
         
          "RPC_SKEL_EmptyThisBuffer",
          "RPC_SKEL_EmptyBufferDone",
          "RPC_SKEL_EventHandler",
          "RPC_SKEL_AllocateBuffer",
          "RPC_SKEL_ComponentTunnelRequest"
};
          
rpcSkelArr rpcSkelFxns[] =
{
          RPC_SKEL_SetParameter,
          RPC_SKEL_GetParameter,
          RPC_SKEL_GetHandle,
          RPC_SKEL_UseBuffer,
          RPC_SKEL_FreeHandle,
          RPC_SKEL_SetConfig,
          RPC_SKEL_GetConfig,
          RPC_SKEL_GetState,
          RPC_SKEL_SendCommand,
          RPC_SKEL_GetComponentVersion,
          RPC_SKEL_GetExtensionIndex,
          RPC_SKEL_FillThisBuffer,
          RPC_SKEL_FillBufferDone,
          RPC_SKEL_FreeBuffer,
          RPC_SKEL_EmptyThisBuffer,
          RPC_SKEL_EmptyBufferDone,
          RPC_SKEL_EventHandler,
          RPC_SKEL_AllocateBuffer,
          RPC_SKEL_ComponentTunnelRequest
};
