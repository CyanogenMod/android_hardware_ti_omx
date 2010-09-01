/* ====================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 * (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied.
 * ==================================================================== */
/*
*  @file timm_osal_memory.h
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

#ifndef _TIMM_OSAL_MEMORY_H_
#define _TIMM_OSAL_MEMORY_H_

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

/*******************************************************************************
* Includes
*******************************************************************************/

#include "timm_osal_types.h"
#include "timm_osal_error.h"


/* Enumeration Flag for Memory Segmenation Id */
	typedef enum TIMMOSAL_MEM_SEGMENTID
	{

		TIMMOSAL_MEM_SEGMENT_EXT = 0,
		TIMMOSAL_MEM_SEGMENT_INT,
		TIMMOSAL_MEM_SEGMENT_UNCACHED
	} TIMMOSAL_MEM_SEGMENTID;


/*******************************************************************************
* External interface
*******************************************************************************/

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_CreateMemoryPool(void);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_DeleteMemoryPool(void);

	TIMM_OSAL_PTR TIMM_OSAL_Malloc(TIMM_OSAL_U32 size,
	    TIMM_OSAL_BOOL bBlockContiguous, TIMM_OSAL_U32 unBlockAlignment,
	    TIMMOSAL_MEM_SEGMENTID tMemSegId);

	void TIMM_OSAL_Free(TIMM_OSAL_PTR pData);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_Memset(TIMM_OSAL_PTR pBuffer,
	    TIMM_OSAL_U8 uValue, TIMM_OSAL_U32 uSize);

	TIMM_OSAL_S32 TIMM_OSAL_Memcmp(TIMM_OSAL_PTR pBuffer1,
	    TIMM_OSAL_PTR pBuffer2, TIMM_OSAL_U32 uSize);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_Memcpy(TIMM_OSAL_PTR pBufDst,
	    TIMM_OSAL_PTR pBufSrc, TIMM_OSAL_U32 uSize);

	TIMM_OSAL_U32 TIMM_OSAL_GetMemCounter(void);

#define TIMM_OSAL_MallocExtn(size, bBlockContiguous, unBlockAlignment, tMemSegId, hHeap) \
    TIMM_OSAL_Malloc(size, bBlockContiguous, unBlockAlignment, tMemSegId )

#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif				/* _TIMM_OSAL_DEFINES_H_ */
