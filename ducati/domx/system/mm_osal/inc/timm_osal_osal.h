/* ====================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 * (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied.
 * ==================================================================== */
/*
*  @file timm_osal_osal.h
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

#ifndef _TIMM_OSAL_OSAL_H_
#define _TIMM_OSAL_OSAL_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
* Includes
*******************************************************************************/

#include "timm_osal_types.h"

TIMM_OSAL_ERRORTYPE TIMM_OSAL_Init (void);
TIMM_OSAL_ERRORTYPE TIMM_OSAL_Deinit (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TIMM_OSAL_OSAL_H_ */
