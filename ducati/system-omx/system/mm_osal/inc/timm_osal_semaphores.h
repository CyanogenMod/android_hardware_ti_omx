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

#ifndef _TIMM_OSAL_SEMAPHORE_H_
#define _TIMM_OSAL_SEMAPHORE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
* Includes
*******************************************************************************/

#include "timm_osal_types.h"

TIMM_OSAL_ERRORTYPE TIMM_OSAL_SemaphoreCreate(TIMM_OSAL_PTR *pSemaphore, TIMM_OSAL_U32 uInitCount);
TIMM_OSAL_ERRORTYPE TIMM_OSAL_SemaphoreDelete(TIMM_OSAL_PTR pSemaphore);
TIMM_OSAL_ERRORTYPE TIMM_OSAL_SemaphoreObtain(TIMM_OSAL_PTR pSemaphore, TIMM_OSAL_U32 uTimeOut);
TIMM_OSAL_ERRORTYPE TIMM_OSAL_SemaphoreRelease(TIMM_OSAL_PTR pSemaphore);
TIMM_OSAL_ERRORTYPE TIMM_OSAL_SemaphoreReset(TIMM_OSAL_PTR pSemaphore, TIMM_OSAL_U32 uInitCount);
TIMM_OSAL_ERRORTYPE TIMM_OSAL_GetSemaphoreCount(TIMM_OSAL_PTR pSemaphore, TIMM_OSAL_U32 *count);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TIMM_OSAL_SEMAPHORE_H_ */
