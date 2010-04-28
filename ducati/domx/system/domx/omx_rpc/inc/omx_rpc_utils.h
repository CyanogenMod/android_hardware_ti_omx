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
extern "C" {
#endif /* __cplusplus */

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
#include "omx_rpc.h"

#define LINUX_TRACE

#ifndef LINUX_TRACE
#  include <timm_osal_error.h>
#  include <timm_osal_osal.h>
#  include <timm_osal_trace.h>
#  define DOMX_DEBUG(ARGS,...)  TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_DOMX,ARGS,##__VA_ARGS__)
#else
#include <timm_osal_trace.h>
#define DOMX_DEBUG(ARGS,...) TIMM_OSAL_Trace(ARGS,##__VA_ARGS__)
#endif

/******************************************************************
 *   MACROS - ASSERTS
 ******************************************************************/
#define RPC_assert  RPC_paramCheck
#define RPC_require RPC_paramCheck
#define RPC_ensure  RPC_paramCheck

#define RPC_paramCheck(C,V,S)  if (!(C)) { eRPCError = V;\
TIMM_OSAL_TraceFunction("##Error:: %s::in %s::line %d \n",S,__FUNCTION__, __LINE__); \
goto EXIT; }

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
#if 0
	#define RPC_SETFIELDVALUE(MSGBODY, POS, VALUE, TYPE) \
	*((TYPE *) ((OMX_U32)MSGBODY+POS)) = VALUE; \
	POS+=sizeof(TYPE);

	#define RPC_SETFIELDOFFSET(MSGBODY, POS, OFFSET, TYPE) \
	*((TYPE *) ((OMX_U32)MSGBODY+POS)) = OFFSET; \
	POS+=sizeof(TYPE);

	#define RPC_SETFIELDCOPYGEN(MSGBODY, POS, PTR, SIZE) \
	TIMM_OSAL_Memcpy((OMX_U8*) ((OMX_U32)MSGBODY+POS),PTR,SIZE);

	#define RPC_SETFIELDCOPYTYPE(MSGBODY, POS, PSTRUCT, TYPE) \
	*((TYPE *) ((OMX_U32)MSGBODY+POS)) = *PSTRUCT;

#else

	#define RPC_SETFIELDVALUE(MSGBODY, POS, VALUE, TYPE) \
	{ \
		TYPE temp = VALUE; \
		TIMM_OSAL_Memcpy((OMX_U8*)(MSGBODY+POS),(OMX_U8*)&temp,sizeof(TYPE)); \
		POS+=sizeof(TYPE); \
	} \

	#define RPC_SETFIELDOFFSET(MSGBODY, POS, OFFSET, TYPE) \
	{ \
		TYPE temp = OFFSET; \
		TIMM_OSAL_Memcpy((OMX_U8*)(MSGBODY+POS),(OMX_U8*)&temp,sizeof(TYPE)); \
		POS+=sizeof(TYPE); \
	} \

	#define RPC_SETFIELDCOPYGEN(MSGBODY, POS, PTR, SIZE) \
	{ \
		TIMM_OSAL_Memcpy((OMX_U8*) (MSGBODY+POS),PTR,SIZE); \
	} \

	#define RPC_SETFIELDCOPYTYPE(MSGBODY, POS, PSTRUCT, TYPE) \
	{ \
		TIMM_OSAL_Memcpy((OMX_U8*) (MSGBODY+POS),PSTRUCT,sizeof(TYPE)); \
	} \

#endif

/******************************************************************
 *   MACROS - COMMON UNMARSHALLING UTILITIES
 ******************************************************************/
#if 0

#define RPC_GETFIELDVALUE(MSGBODY, POS, VALUE, TYPE) \
VALUE = *((TYPE *) ((OMX_U32)MSGBODY+POS)); \
POS+=sizeof(TYPE);

#define RPC_GETFIELDOFFSET(MSGBODY, POS, OFFSET, TYPE) \
OFFSET = *((TYPE *) ((OMX_U32)MSGBODY+POS)); \
POS+=sizeof(TYPE);

#define RPC_GETFIELDCOPYGEN(MSGBODY, POS, PTR, SIZE) \
TIMM_OSAL_Memcpy(PTR,(OMX_U8*) ((OMX_U32)MSGBODY+POS),SIZE);

#define RPC_GETFIELDCOPYTYPE(MSGBODY, POS, PSTRUCT, TYPE) \
*PSTRUCT = *((TYPE *) ((OMX_U32)MSGBODY+POS));

#define RPC_GETFIELDPATCHED(MSGBODY, OFFSET, PTR, TYPE) \
PTR = (TYPE *) (MSGBODY+OFFSET);

#else

	#define RPC_GETFIELDVALUE(MSGBODY, POS, VALUE, TYPE) \
	{ \
		TYPE temp; \
		TIMM_OSAL_Memcpy((OMX_U8*)&temp,(OMX_U8*)(MSGBODY+POS),sizeof(TYPE)); \
		VALUE = temp; \
		POS+=sizeof(TYPE); \
	} \

	#define RPC_GETFIELDOFFSET(MSGBODY, POS, OFFSET, TYPE) \
	{ \
		TYPE temp; \
		TIMM_OSAL_Memcpy((OMX_U8*)&temp,(OMX_U8*)(MSGBODY+POS),sizeof(TYPE)); \
		OFFSET = temp; \
		POS+=sizeof(TYPE); \
	} \

	#define RPC_GETFIELDCOPYGEN(MSGBODY, POS, PTR, SIZE) \
	{ \
		TIMM_OSAL_Memcpy(PTR,(OMX_U8*) (MSGBODY+POS),SIZE); \
	} \

	#define RPC_GETFIELDCOPYTYPE(MSGBODY, POS, PSTRUCT, TYPE) \
	{ \
		TIMM_OSAL_Memcpy(PSTRUCT,(OMX_U8*) (MSGBODY+POS),sizeof(TYPE)); \
	} \


	#define RPC_GETFIELDPATCHED(MSGBODY, OFFSET, PTR, TYPE) \
	{ \
		PTR = (TYPE *) (MSGBODY+OFFSET); \
	} \

#endif

/******************************************************************
 *   FUNCTIONS
 ******************************************************************/
RPC_OMX_ERRORTYPE RPC_UnMapBuffer(OMX_U32 mappedBuffer );
RPC_OMX_ERRORTYPE RPC_MapBuffer(OMX_U32 mappedBuffer );
RPC_OMX_ERRORTYPE RPC_FlushBuffer(OMX_U8 * pBuffer, OMX_U32 size );

RPC_OMX_ERRORTYPE RPC_UTIL_GetTargetServerName(OMX_STRING ComponentName, OMX_STRING * ServerName);
RPC_OMX_ERRORTYPE RPC_UTIL_GetLocalServerName(OMX_STRING ComponentName, OMX_STRING * ServerName);

#endif
