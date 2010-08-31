/* ====================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 * (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied.
 * ==================================================================== */
/*
*   @file  tiimm_osal.c
*   This file contains methods that provides the functionality
*   initializing/deinitializing the osal.
*
*  @path \
*
*/
/* -------------------------------------------------------------------------- */
/* =========================================================================
 *!
 *! Revision History
 *! ===================================
 *! 20-Oct-2008 Maiya ShreeHarsha:Linux specific changes
 *! 0.1: Created the first draft version, ksrini@ti.com
 * ========================================================================= */

/******************************************************************************
* Includes
******************************************************************************/
#include "timm_osal_types.h"
#include "timm_osal_error.h"
#include "timm_osal_memory.h"
/*#include "timm_osal_trace.h"*/


/******************************************************************************
* Function Prototypes
******************************************************************************/

/****************************************************************
*  PRIVATE DECLARATIONS  : only used in this file
****************************************************************/
/*--------data declarations -----------------------------------*/


/* ========================================================================== */
/**
* @fn TIMM_OSAL_Init function initilize the osal with initial settings.
*
* @return  TIMM_OSAL_ERR_NONE if successful
*               !TIMM_OSAL_ERR_NONE if an error occurs
*/
/* ========================================================================== */

TIMM_OSAL_ERRORTYPE TIMM_OSAL_Init(void)
{
	TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_NONE;
	return bReturnStatus;
}



/* ========================================================================== */
/**
* @fn TIMM_OSAL_Init function de-initilize the osal.
*
* @return  TIMM_OSAL_ERR_NONE if successful
*               !TIMM_OSAL_ERR_NONE if an error occurs
*/
/* ========================================================================== */

TIMM_OSAL_ERRORTYPE TIMM_OSAL_Deinit(void)
{
	TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_NONE;
	return bReturnStatus;
}
