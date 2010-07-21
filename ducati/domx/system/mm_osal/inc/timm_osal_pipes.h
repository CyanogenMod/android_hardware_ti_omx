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

#ifndef _TIMM_OSAL_PIPES_H_
#define _TIMM_OSAL_PIPES_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "timm_osal_types.h"

/*
* Defined for Pipe timeout value
*/
TIMM_OSAL_ERRORTYPE TIMM_OSAL_CreatePipe (TIMM_OSAL_PTR *pPipe,
                                          TIMM_OSAL_U32  pipeSize,
                                          TIMM_OSAL_U32  messageSize,
                                          TIMM_OSAL_U8   isFixedMessage);
                                          
TIMM_OSAL_ERRORTYPE TIMM_OSAL_DeletePipe (TIMM_OSAL_PTR pPipe);

TIMM_OSAL_ERRORTYPE TIMM_OSAL_WriteToPipe (TIMM_OSAL_PTR pPipe,
                                           void *pMessage,
                                           TIMM_OSAL_U32 size,
                                           TIMM_OSAL_S32 timeout);

TIMM_OSAL_ERRORTYPE TIMM_OSAL_WriteToFrontOfPipe (TIMM_OSAL_PTR pPipe,
                                                  void *pMessage,
                                                  TIMM_OSAL_U32 size,
                                                  TIMM_OSAL_S32 timeout);

TIMM_OSAL_ERRORTYPE TIMM_OSAL_ReadFromPipe (TIMM_OSAL_PTR pPipe,
                                            void *pMessage,
                                            TIMM_OSAL_U32 size,
                                            TIMM_OSAL_U32 *actualSize,
                                            TIMM_OSAL_S32 timeout);
                                                  
TIMM_OSAL_ERRORTYPE TIMM_OSAL_ClearPipe (TIMM_OSAL_PTR pPipe);

TIMM_OSAL_ERRORTYPE TIMM_OSAL_IsPipeReady (TIMM_OSAL_PTR pPipe);

TIMM_OSAL_ERRORTYPE TIMM_OSAL_GetPipeReadyMessageCount (TIMM_OSAL_PTR pPipe,
                                                        TIMM_OSAL_U32 *count);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TIMM_OSAL_PIPES_H_ */
