/* =============================================================================
*             Texas Instruments OMAP(TM) Platform Software
*  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
*
*  Use of this software is controlled by the terms and conditions found
*  in the license agreement under which this software has been supplied.
* =========================================================================== */
/**
* @file OMX_Main.h
*
* This File contains functions declaration for Test Case API
* functionality .
*
* @path
*
* @rev
*/

/* ----- system and platform files ----------------------------*/
#include <stdint.h>
#include <string.h>
#include <timm_osal_types.h>


/**
* THC TestCase return values
*/
typedef enum OMX_TestStatus
{
	OMX_RET_FAIL = 0,
	OMX_RET_PASS = 1,
	OMX_RET_SKIP = 2,
	OMX_RET_ABORT = 3,
	OMX_RET_MEM = 4
} OMX_TestStatus;


/**
* Function Types
*  testcase handling procedures
*/
typedef OMX_TestStatus(*OMXTestproc) (uint32_t uMsg, void *pParam,
    uint32_t paramSize);


/**
* Function Table Entry Structure
*/
typedef struct OMX_TEST_CASE_ENTRY
{

	TIMM_OSAL_CHAR *pTestID;	/* uniquely identifies the test - used in loading/saving scripts */
	TIMM_OSAL_CHAR *pTestDescription;	/* description of test */
	TIMM_OSAL_CHAR *pUserData;	/* user defined data that will be passed to TestProc at runtime */
	TIMM_OSAL_CHAR *pDynUserData;	/* user defined data that will be passed to TestProc at runtime */
	uint32_t depth;		/* depth of item in tree hierarchy */
	OMXTestproc pTestProc;	/* pointer to TestProc function to be called for this test */


} OMX_TEST_CASE_ENTRY;


/**
* TESTCASE Engine message values
*/

#define OMX_MSG_EXECUTE     1
