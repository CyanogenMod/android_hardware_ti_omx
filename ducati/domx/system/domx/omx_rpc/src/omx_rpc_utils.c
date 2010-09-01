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
 *  @file  omx_rpc_utils.c
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
#include <string.h>
#include <stdio.h>

#include "omx_rpc.h"
#include "omx_rpc_internal.h"
#include "omx_rpc_utils.h"
#include <MultiProc.h>
#include <ProcMgr.h>

extern char rcmservertable[MAX_PROC][MAX_SERVER_NAME_LENGTH];
extern char Core_Array[MAX_PROC][MAX_CORENAME_LENGTH];

/* ===========================================================================*/
/**
 * @name EMPTY-STUB
 * @brief
 * @param
 * @return
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_UTIL_GetTargetServerName(OMX_STRING ComponentName,
    OMX_STRING ServerName)
{
	OMX_U8 count = 0;
	OMX_U8 i = 0;
	OMX_U8 servertable_idx = 0;
	OMX_STRING str;
	char Core[MAX_CORENAME_LENGTH];

	DOMX_ENTER("");
	DOMX_DEBUG(" Calling Component Name %s", ComponentName);

	while (*ComponentName != '\0')
	{
		if (*ComponentName == '.')
		{
			count++;
			if (count == 2)
			{
				ComponentName++;
				str = ComponentName;

				while (*str != '.')
				{
					Core[i] = *str;
					i++;
					str++;
				}
				Core[i] = '\0';
				break;
			}

		}

		ComponentName++;
	}

	DOMX_DEBUG(" CORE NAME RECOVERED: %s", Core);
	DOMX_DEBUG
	    ("____________________________________________________________");
	DOMX_DEBUG("Recovering Server Table Index");
	for (i = 0; i < CORE_MAX; i++)
	{

		if (strcmp(Core, Core_Array[i]) == 0)
		{
			servertable_idx = i;
			DOMX_DEBUG("Recovered Server table index = %d", i);
			break;
		}
	}
	strncpy(ServerName, (OMX_STRING) rcmservertable[servertable_idx],
	    MAX_SERVER_NAME_LENGTH);
	DOMX_DEBUG(" ServerName recovered = %s", ServerName);

	return RPC_OMX_ErrorNone;
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
RPC_OMX_ERRORTYPE RPC_UTIL_GetLocalServerName(OMX_STRING ComponentName,
    OMX_STRING * ServerName)
{
/* Implementation returns only current core ID - But this is a place holder to abstract out the
default server and other additional servers available on the current core. This additional servers
should be available in the RPC global that is indexed using the calling component name*/
	OMX_U8 servertable_idx = 0;

	servertable_idx = MultiProc_getId(NULL);	//This can be replace with the mechanism to obtain new addition rcm servers
	*ServerName = rcmservertable[servertable_idx];
	return RPC_OMX_ErrorNone;
}



/* ===========================================================================*/
/**
 * @name RPC_UTIL_GenerateLocalServerName()
 * @brief This function generates a server name to be used while creating the
 *        RCM server. The name is based on the pid so that eacj process gets a
 *        unique server name.
 * @param cServerName - This is filled in and returned with a valid value.
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_UTIL_GenerateLocalServerName(OMX_STRING cServerName)
{
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	OMX_S32 pid = 0;
	OMX_U32 nProcId = 0;

	pid = getpid();
	/*Using pid as the server name, thus limiting to only 1 server per process.
	   This limitaion is partly enforce by syslink as well since max size of server
	   name is 32 so only pid can fit into the name */
	sprintf(cServerName, "%ld", pid);

	nProcId = MultiProc_getId(NULL);
	/*Fill the server table with the newly generated name */
	strncpy(rcmservertable[nProcId], cServerName, MAX_SERVER_NAME_LENGTH);

	return eRPCError;
}



/* ===========================================================================*/
/**
 * @name RPC_UTIL_GetTargetCore()
 * @brief This function gets the target core id by parsing the component name.
 *        It is assumed that component names follow the convention
 *        <OMX>.<Company Name>.<Core Name>.<Domain>.<Component Details> with
 *        all characters in upper case for e.g. OMX.TI.DUCATI1.VIDEO.H264E
 * @param cComponentName - Name of the component.
 * @param nCoreId - Core ID, this is filled in and returned.
 * @return RPC_OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_UTIL_GetTargetCore(OMX_STRING cComponentName,
    OMX_U32 * nCoreId)
{
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	OMX_S8 cCoreName[MAX_SERVER_NAME_LENGTH] = { 0 };
	OMX_S32 ret = 0;
	OMX_U32 i = 0;

	ret =
	    sscanf(cComponentName, "%*[^'.'].%*[^'.'].%[^'.'].%*s",
	    cCoreName);
	RPC_assert(ret == 1, RPC_OMX_ErrorBadParameter,
	    "Incorrect component name");

	for (i = 0; i < CORE_MAX; i++)
	{
		if (strcmp((char *)cCoreName, Core_Array[i]) == 0)
			break;
	}
	RPC_assert(i < CORE_MAX, RPC_OMX_ErrorBadParameter,
	    "Unknown core name");
	*nCoreId = i;

      EXIT:
	return eRPCError;
}



/* ===========================================================================*/
/**
 * @name EMPTY-STUB
 * @brief  the Api creates the RCM client on the target core(where the component sits).
This happens in the context of the Default RCM server on the target core.
The RCM server name to connect for this client will be the default RCM server on the calling core.
This can be provided as an option as the name that you pass in the string server is used as the RCM server name
input to the client create call.
@Default_RcmServer - The default rcm server on the target core
@Server - The name of the server on the calling core to connect to
 * @param
 * @return
 *
 */
/* ===========================================================================*/
/*
RPC_OMX_ERRORTYPE RPC_GetTargetClient(OMX_STRING Default_RcmServer, OMX_STRING server, rcmhHndl)
{


}
*/

/* ===========================================================================*/
/**
 * @name EMPTY-STUB
 * @brief
 * @param
 * @return
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_MapBuffer(OMX_U32 mappedBuffer)
{
	DOMX_ENTER("");
	DOMX_DEBUG("Empty implementation ");
	//PlaceHolder for Mapping Buffers - Cuurently no implementation here
	return RPC_OMX_ErrorNone;
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
RPC_OMX_ERRORTYPE RPC_UnMapBuffer(OMX_U32 mappedBuffer)
{
	DOMX_ENTER("");
	DOMX_DEBUG("Empty implementation ");
	//PlaceHolder for UnMapping Buffers - Cuurently no implementation here
	return RPC_OMX_ErrorNone;
}

/* ===========================================================================*/
/**
 * @name RPC_FlushBuffer
 * @brief Used to flush buffers from cache to memory. Used when buffers are
 *        being transferred across processor boundaries.
 * @param pBuffer       : Pointer to the data that has to be flushed.
 *        size          : Size of the data to be flushed.
 *        nTargetCoreId : Core to which buffer is being transferred.
 * @return RPC_OMX_ErrorNone      : Success.
 *         RPC_OMX_ErrorUndefined : Flush operation failed.
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_FlushBuffer(OMX_U8 * pBuffer, OMX_U32 size,
    OMX_U32 nTargetCoreId)
{
	DOMX_ENTER("");
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	OMX_S32 nStatus = 0;

	DOMX_DEBUG("About to flush %d bytes", size);
	nStatus = ProcMgr_flushMemory((OMX_PTR) pBuffer, size,
	    (ProcMgr_ProcId) nTargetCoreId);
	RPC_assert(nStatus >= 0, RPC_OMX_ErrorUndefined,
	    "Cache flush failed");

      EXIT:
	return eRPCError;
}



/* ===========================================================================*/
/**
 * @name RPC_InvalidateBuffer
 * @brief Used to flush buffers from cache to memory. Used when buffers are
 *        being transferred across processor boundaries.
 * @param pBuffer       : Pointer to the data that has to be flushed.
 *        size          : Size of the data to be flushed.
 *        nTargetCoreId : Core to which buffer is being transferred.
 * @return RPC_OMX_ErrorNone      : Success.
 *         RPC_OMX_ErrorUndefined : Invalidate operation failed.
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_InvalidateBuffer(OMX_U8 * pBuffer, OMX_U32 size,
    OMX_U32 nTargetCoreId)
{
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	OMX_S32 nStatus = 0;
	DOMX_ENTER("");

	DOMX_DEBUG("About to invalidate %d bytes", size);
	nStatus = ProcMgr_invalidateMemory((OMX_PTR) pBuffer, size,
	    (ProcMgr_ProcId) nTargetCoreId);
	RPC_assert(nStatus >= 0, RPC_OMX_ErrorUndefined,
	    "Cache invalidate failed");

      EXIT:
	return eRPCError;
}
