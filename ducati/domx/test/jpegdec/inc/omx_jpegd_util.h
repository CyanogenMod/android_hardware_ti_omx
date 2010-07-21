
/* =============================================================================
*             Texas Instruments OMAP(TM) Platform Software
*  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
*
*  Use of this software is controlled by the terms and conditions found
*  in the license agreement under which this software has been supplied.
* =========================================================================== */
/**
* @file omx_Util.h
*
* This File contains definition for types & functions part of the THC Utility
* functionlaity.
*
*/

#include <stdint.h>

/********************/
/* Files Needed for EXIF reading */
#include <OMX_Types.h>

/**
* Testcase return values
*/
typedef enum OMX_Status {
    OMX_STATUS_OK,
    OMX_ERROR_ALLOC,
    OMX_ERROR_INVALID_PARAM,
    OMX_ERROR_UNKNOWN
} OMX_Status;


/*******************************************************************************
*                   ******* GENERAL DEFINES  *********
*******************************************************************************/

#define MAX_STRING_SIZE        512
#define OMX_MMC_TEST_PATH     "/MMC/"


