/* ====================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 * (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied.
 * ==================================================================== */
/*
*  @file timm_osal_task.h
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

#ifndef _TIMM_OSAL_TASK_H_
#define _TIMM_OSAL_TASK_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
* Includes
*******************************************************************************/

#include "timm_osal_types.h"

/*******************************************************************************
* Tasks
*******************************************************************************/

/**
* prototype for the task function
*/
/*typedef void (*TIMM_OSAL_TaskProc)(TIMM_OSAL_U32 argc, TIMM_OSAL_PTR argv);*/ /*Nucleus*/

typedef void * (*TIMM_OSAL_TaskProc)(void * arg);		/*Linux*/



TIMM_OSAL_ERRORTYPE TIMM_OSAL_CreateTask (TIMM_OSAL_PTR *pTask,
                                          TIMM_OSAL_TaskProc pFunc,
                                          TIMM_OSAL_U32 uArgc,
                                          TIMM_OSAL_PTR pArgv,
                                          TIMM_OSAL_U32 uStackSize,
                                          TIMM_OSAL_U32 uPriority,
                                          TIMM_OSAL_S8 *pName);

TIMM_OSAL_ERRORTYPE TIMM_OSAL_DeleteTask (TIMM_OSAL_PTR pTask);

TIMM_OSAL_ERRORTYPE TIMM_OSAL_SleepTask (TIMM_OSAL_U32 mSec);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TIMM_OSAL_TASK_H_ */
