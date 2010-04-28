/* ====================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 * (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied.
 * ==================================================================== */
/*
*   @file  timm_osal_semaphores.c
*   This file contains methods that provides the functionality
*
*  @path \
*
*/
/* -------------------------------------------------------------------------- */
/* =========================================================================
 *!
 *! Revision History
 *! ===================================
 *! 30-Oct-2008 Maiya ShreeHarsha: Linux specific changes
 *! 0.1: Created the first draft version, ksrini@ti.com
 * ========================================================================= */

/******************************************************************************
* Includes
******************************************************************************/

#include <stdio.h>

#define __USE_XOPEN2K /* required for sem_timedwait */

#include <semaphore.h>
#include <sys/time.h>


#include "timm_osal_types.h"
#include "timm_osal_trace.h"
#include "timm_osal_error.h"
#include "timm_osal_memory.h"


#define SEMNAME_MAX           7

/*
typedef struct TIMM_OSAL_SEMAPHORE {
    sem_t  sem;
    CHAR name[SEMNAME_MAX];
} TIMM_OSAL_SEMAPHORE;
*/

/* ========================================================================== */
/**
* @fn TIMM_OSAL_SemaphoreCreate function
*
*
*/
/* ========================================================================== */
TIMM_OSAL_ERRORTYPE TIMM_OSAL_SemaphoreCreate(TIMM_OSAL_PTR *pSemaphore,
                                              TIMM_OSAL_U32  uInitCount)
{
	TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;
    *pSemaphore = TIMM_OSAL_NULL;

	sem_t *psem = (sem_t *) TIMM_OSAL_Malloc(sizeof(sem_t), 0, 0, 0);

    if(TIMM_OSAL_NULL == psem ) {
        bReturnStatus = TIMM_OSAL_ERR_ALLOC;
		goto EXIT;
	}

	/*Unnamed semaphore*/
	if(SUCCESS != sem_init(psem , 0, uInitCount)) {
		/*TIMM_OSAL_Error("Semaphore Create failed !");*/
		/*goto EXIT;*/
    }
	else {
	    *pSemaphore = (TIMM_OSAL_PTR)psem;
	    bReturnStatus = TIMM_OSAL_ERR_NONE;
	}
EXIT:
    if ((TIMM_OSAL_ERR_NONE != bReturnStatus) && (TIMM_OSAL_NULL != psem)) {
        TIMM_OSAL_Free(psem);
	}
    return bReturnStatus;
}

/* ========================================================================== */
/**
* @fn TIMM_OSAL_SemaphoreDelete function
*
*
*/
/* ========================================================================== */
TIMM_OSAL_ERRORTYPE TIMM_OSAL_SemaphoreDelete(TIMM_OSAL_PTR pSemaphore)
{
	TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_NONE;
    sem_t *psem = (sem_t *)pSemaphore;

    if (psem == TIMM_OSAL_NULL ){
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
    }
    /* Release the semaphore.  */
	if(SUCCESS != sem_destroy(psem)){
		/*TIMM_OSAL_Error("Semaphore Delete failed !");*/
		bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;
	}

	TIMM_OSAL_Free(psem);
EXIT:
    return bReturnStatus;
}
/* ========================================================================== */
/**
* @fn TIMM_OSAL_SemaphoreObtain function
*
*
*/
/* ========================================================================== */

TIMM_OSAL_ERRORTYPE TIMM_OSAL_SemaphoreObtain(TIMM_OSAL_PTR pSemaphore, TIMM_OSAL_U32 uTimeOut)
{
    TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;
    TIMM_OSAL_U32 ltimenow_us;
    struct timeval ltime_now;
    struct timespec abs_timeout;
	sem_t *psem = (sem_t *)pSemaphore;

	if (psem == TIMM_OSAL_NULL){
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}

    if (TIMM_OSAL_SUSPEND == uTimeOut) {
		if(SUCCESS != sem_wait(psem)) {
			/*TIMM_OSAL_Error("Semaphore Wait failed !");*/
			goto EXIT;
		}

    }
    else if (TIMM_OSAL_NO_SUSPEND == uTimeOut) {
		if(SUCCESS != sem_trywait(psem)){
			/*TIMM_OSAL_Error("Semaphore blocked !");*/
			goto EXIT;
		}
    }
    else {
		gettimeofday(&ltime_now, NULL);
		/*uTimeOut is assumed to be in milliseconds*/
		ltimenow_us = ltime_now.tv_usec + 1000 * uTimeOut;
		abs_timeout.tv_sec = ltime_now.tv_sec + uTimeOut / 1000;
		abs_timeout.tv_nsec = (ltimenow_us % 1000000) * 1000;

	   	if(SUCCESS != sem_timedwait(psem, &abs_timeout)){
			/*TIMM_OSAL_Error("Semaphore Timed Wait failed !");*/
			goto EXIT;
	   	}
    }
	bReturnStatus = TIMM_OSAL_ERR_NONE;

EXIT:
    return bReturnStatus;
}
/* ========================================================================== */
/**
* @fn TIMM_OSAL_SemaphoreRelease function
*
*
*/
/* ========================================================================== */
TIMM_OSAL_ERRORTYPE TIMM_OSAL_SemaphoreRelease(TIMM_OSAL_PTR pSemaphore)
{
    TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;
	sem_t *psem = (sem_t *)pSemaphore;

	if (TIMM_OSAL_NULL == psem){
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}
    /* Release the semaphore.  */
	if(SUCCESS != sem_post(psem)){
		/*TIMM_OSAL_Error("Release failed !");*/
	}
	else {
		bReturnStatus = TIMM_OSAL_ERR_NONE;
	}

EXIT:
    return bReturnStatus;
}

/* ========================================================================== */
/**
* @fn TIMM_OSAL_SemaphoreReset function
*
*
*/
/* ========================================================================== */
TIMM_OSAL_ERRORTYPE TIMM_OSAL_SemaphoreReset(TIMM_OSAL_PTR pSemaphore, TIMM_OSAL_U32 uInitCount)
{
  /*  TIMM_OSAL_SEMAPHORE *pHandle = (TIMM_OSAL_SEMAPHORE *)pSemaphore;
    STATUS  status;
    TIMM_OSAL_ERRORTYPE bReturnStatus;*/


    /* Release the semaphore.  */
    /*status = NU_Reset_Semaphore(&(pHandle->sem),
                                uInitCount);
*/
 /*   switch(status)
    {
    case NU_SUCCESS:
        bReturnStatus = TIMM_OSAL_ERR_NONE;
        break;
    default:
        bReturnStatus = TIMM_OSAL_ERR_CREATE(TIMM_OSAL_ERR, TIMM_OSAL_COMP_SEMAPHORES, status);
        break;
    }
*/
   /* return bReturnStatus;*/
return TIMM_OSAL_ERR_UNKNOWN;
}

/* ========================================================================== */
/**
* @fn TIMM_OSAL_GetSemaphoreCount function
*
*
*/
/* ========================================================================== */
TIMM_OSAL_ERRORTYPE TIMM_OSAL_GetSemaphoreCount(TIMM_OSAL_PTR pSemaphore,
                                                        TIMM_OSAL_U32 *count)
{
    TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;
    int sval = -2;		/*value that is not possible*/
	sem_t *psem = (sem_t *)pSemaphore;

	if (TIMM_OSAL_NULL == psem){
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}

    /* Release the semaphore.  */
	if(SUCCESS != sem_getvalue(psem, &sval)){
		/*TIMM_OSAL_Error("Get Semaphore Count failed !");*/
	}
	else {
		*count = sval;
		bReturnStatus = TIMM_OSAL_ERR_NONE;
	}

EXIT:
    return bReturnStatus;
}

