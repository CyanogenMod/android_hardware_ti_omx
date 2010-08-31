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
 *  @file  omx_rpc_utils.h
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

#ifndef OMX_RPC_UTILSH
#define OMX_RPC_UTILSH

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
#include "omx_rpc.h"

#include <timm_osal_trace.h>

#define DOMX_ERROR(fmt,...)  TIMM_OSAL_Error(fmt, ##__VA_ARGS__)
#define DOMX_WARN(fmt,...)   TIMM_OSAL_Warning(fmt, ##__VA_ARGS__)
#define DOMX_INFO(fmt,...)   TIMM_OSAL_Info(fmt, ##__VA_ARGS__)
#define DOMX_DEBUG(fmt,...)  TIMM_OSAL_Debug(fmt, ##__VA_ARGS__)
#define DOMX_ENTER(fmt,...)  TIMM_OSAL_Entering(fmt, ##__VA_ARGS__)
#define DOMX_EXIT(fmt,...)   TIMM_OSAL_Exiting(fmt, ##__VA_ARGS__)


/******************************************************************
 *   MACROS - ASSERTS
 ******************************************************************/
#define RPC_assert  RPC_paramCheck
#define RPC_require RPC_paramCheck
#define RPC_ensure  RPC_paramCheck

#define RPC_paramCheck(C, V, S) do { \
    if (!(C)) { eRPCError = V;\
    DOMX_ERROR("failed check: " #C);\
    DOMX_ERROR(" - returning error: " #V);\
    if(S) DOMX_ERROR(" - %s", S);\
    goto EXIT; } \
    } while(0)

/* ************************* OFFSET DEFINES ******************************** */
#define GET_PARAM_DATA_OFFSET    (sizeof(RPC_OMX_HANDLE) + sizeof(OMX_INDEXTYPE) + sizeof(OMX_U32) /*4 bytes for offset*/ )
#define USE_BUFFER_DATA_OFFSET   (sizeof(OMX_U32)*5)

/******************************************************************
 *   MACROS
 ******************************************************************/
#define RPC_UTIL_GETSTRUCTSIZE(PTR) *((OMX_U32*)PTR)

/******************************************************************
 *   MACROS - COMMON MARSHALLING UTILITIES
 ******************************************************************/
#define RPC_SETFIELDVALUE(MSGBODY, POS, VALUE, TYPE) do { \
    *((TYPE *) ((OMX_U32)MSGBODY+POS)) = VALUE; \
    POS+=sizeof(TYPE); \
    } while(0)

#define RPC_SETFIELDOFFSET(MSGBODY, POS, OFFSET, TYPE) do { \
    *((TYPE *) ((OMX_U32)MSGBODY+POS)) = OFFSET; \
    POS+=sizeof(TYPE); \
    } while(0)

#define RPC_SETFIELDCOPYGEN(MSGBODY, POS, PTR, SIZE) \
TIMM_OSAL_Memcpy((OMX_U8*) ((OMX_U32)MSGBODY+POS),PTR,SIZE);

#define RPC_SETFIELDCOPYTYPE(MSGBODY, POS, PSTRUCT, TYPE) \
*((TYPE *) ((OMX_U32)MSGBODY+POS)) = *PSTRUCT;

/******************************************************************
 *   MACROS - COMMON UNMARSHALLING UTILITIES
 ******************************************************************/
#define RPC_GETFIELDVALUE(MSGBODY, POS, VALUE, TYPE) do { \
    VALUE = *((TYPE *) ((OMX_U32)MSGBODY+POS)); \
    POS+=sizeof(TYPE); \
    } while(0)

#define RPC_GETFIELDOFFSET(MSGBODY, POS, OFFSET, TYPE) do { \
    OFFSET = *((TYPE *) ((OMX_U32)MSGBODY+POS)); \
    POS+=sizeof(TYPE); \
    } while(0)

#define RPC_GETFIELDCOPYGEN(MSGBODY, POS, PTR, SIZE) \
TIMM_OSAL_Memcpy(PTR,(OMX_U8*) ((OMX_U32)MSGBODY+POS),SIZE);

#define RPC_GETFIELDCOPYTYPE(MSGBODY, POS, PSTRUCT, TYPE) \
*PSTRUCT = *((TYPE *) ((OMX_U32)MSGBODY+POS));

#define RPC_GETFIELDPATCHED(MSGBODY, OFFSET, PTR, TYPE) \
PTR = (TYPE *) (MSGBODY+OFFSET);

/******************************************************************
 *   FUNCTIONS
 ******************************************************************/
	RPC_OMX_ERRORTYPE RPC_UnMapBuffer(OMX_U32 mappedBuffer);
	RPC_OMX_ERRORTYPE RPC_MapBuffer(OMX_U32 mappedBuffer);
	RPC_OMX_ERRORTYPE RPC_FlushBuffer(OMX_U8 * pBuffer, OMX_U32 size,
	    OMX_U32 nTargetCoreId);
	RPC_OMX_ERRORTYPE RPC_InvalidateBuffer(OMX_U8 * pBuffer,
	    OMX_U32 size, OMX_U32 nTargetCoreId);

	RPC_OMX_ERRORTYPE RPC_UTIL_GetTargetServerName(OMX_STRING
	    ComponentName, OMX_STRING ServerName);
	RPC_OMX_ERRORTYPE RPC_UTIL_GetLocalServerName(OMX_STRING
	    ComponentName, OMX_STRING * ServerName);
	RPC_OMX_ERRORTYPE RPC_UTIL_GenerateLocalServerName(OMX_STRING
	    cServerName);
	RPC_OMX_ERRORTYPE RPC_UTIL_GetTargetCore(OMX_STRING cComponentName,
	    OMX_U32 * nCoreId);

#endif
