/* ====================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 * (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied.
 * ==================================================================== */
/*
*  @file timm_osal_defines.h
*  The osal header file defines
*  @path
*
*/
/* -------------------------------------------------------------------------- */
/* =========================================================================
 *!
 *! Revision History
 *! ===================================
 *! 0.1: Created the first draft version, ksrini@ti.com
 * ========================================================================= */

#ifndef _TIMM_OSAL_EVENTS_H_
#define _TIMM_OSAL_EVENTS_H_

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

#include "timm_osal_types.h"

	typedef enum TIMM_OSAL_EVENT_OPERATION
	{
		TIMM_OSAL_EVENT_AND,
		TIMM_OSAL_EVENT_AND_CONSUME,
		TIMM_OSAL_EVENT_OR,
		TIMM_OSAL_EVENT_OR_CONSUME
	} TIMM_OSAL_EVENT_OPERATION;

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_EventCreate(TIMM_OSAL_PTR * pEvents);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_EventDelete(TIMM_OSAL_PTR pEvents);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_EventSet(TIMM_OSAL_PTR pEvents,
	    TIMM_OSAL_U32 uEventFlag, TIMM_OSAL_EVENT_OPERATION eOperation);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_EventRetrieve(TIMM_OSAL_PTR pEvents,
	    TIMM_OSAL_U32 uRequestedEvents,
	    TIMM_OSAL_EVENT_OPERATION eOperation,
	    TIMM_OSAL_U32 * pRetrievedEvents, TIMM_OSAL_U32 uTimeOut);

#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif				/* _TIMM_OSAL_EVENTS_H_ */
