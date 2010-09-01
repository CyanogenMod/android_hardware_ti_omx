/* =============================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 *  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied.
 * =========================================================================== */
/**
 *
 * This File contains the test entry function for OMX Jpeg Decoder.
 *
 * @path
 *
 * @rev  0.1
 */
/* ----------------------------------------------------------------------------
 *!
 *! Revision History
 *! =============================================================
 *! Initial Version
 * ============================================================== */

#include <timm_osal_types.h>
#include <timm_osal_error.h>
#include <timm_osal_memory.h>
#include <timm_osal_semaphores.h>
#include <timm_osal_interfaces.h>
#include <timm_osal_trace.h>

#include <stdio.h>
#include "omx_jpegd_test.h"
#include <OMX_Types.h>

#define OMX_PORT_DUCATI


#define OMX_LINUX_TILERTEST

#ifdef OMX_LINUX_TILERTEST
/*Tiler APIs*/
#include <memmgr.h>

#endif

extern OMX_TEST_CASE_ENTRY OMX_JPEGDTestCaseTable[];
OMX_U32 OMX_Jpegd_MemStatPrint(void);
OMX_U32 jpegdec_prev = 0;

void main()
{
	OMX_TEST_CASE_ENTRY *testcaseEntry = NULL;
	OMX_U32 test_case, test_case_start, test_case_end, TraceGrp;
	OMX_U32 mem_size_start = 0, mem_size_end = 0;



	printf
	    ("\n Wait until RCM Server is created on other side. Press any key after that\n");
	getchar();


	TraceGrp = TIMM_OSAL_TRACEGRP_SYSTEM;
	TIMM_OSAL_DebugExt(TIMM_OSAL_TRACEGRP_OMXIMGDEC, "tracegroup=%x",
	    TraceGrp);


	TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXIMGDEC,
	    " Select test case start ID (1-51):");
	fflush(stdout);
	scanf("%d", &test_case_start);


	while (test_case_start < 1 || test_case_start > 51)
	{
		TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXIMGDEC,
		    " Invalid test ID selection.Select test case start ID (1-51):");
		fflush(stdout);
		scanf("%d", &test_case_start);
	}


	TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXIMGDEC,
	    " Do you want to have preview of Decoded Frames on Display?");
	TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXIMGDEC,
	    " Enter 1 for yes, 2 for no\n");

	fflush(stdout);
	scanf("%d", &jpegdec_prev);


	while (jpegdec_prev < 1 || jpegdec_prev > 2)
	{
		TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXIMGDEC,
		    " Invalid selection. Please select again: [1|2]");
		fflush(stdout);
		scanf("%d", &jpegdec_prev);
	}


	if (jpegdec_prev == 1)
		TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXIMGDEC,
		    " Preview On Display selected.\n");

	TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXIMGDEC,
	    " JPEG Decoder Test case begin");
	printf
	    ("\n----------------JPEG Decoder Test Start-------------------------------");


	// Get Memory status before running the test case
	mem_size_start = OMX_Jpegd_MemStatPrint();

#ifdef MEMORY_DEBUG
	TIMM_OSAL_StartMemDebug();
#endif


	test_case = (test_case_start - 1);

	testcaseEntry = &OMX_JPEGDTestCaseTable[test_case];
	testcaseEntry->pTestProc(1,
	    (TIMM_OSAL_U32 *) & OMX_JPEGDTestCaseTable[test_case], NULL);


#ifdef MEMORY_DEBUG
	TIMM_OSAL_EndMemDebug();
#endif

	// Get Memory status after running the test case
	mem_size_end = OMX_Jpegd_MemStatPrint();
	if (mem_size_end != mem_size_start)
		TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXIMGDEC,
		    " Memory leak detected. Bytes lost = %d",
		    (mem_size_end - mem_size_start));
	TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXIMGDEC,
	    " JPEG Decoder Test End");

	printf
	    ("\n----------------JPEG Decoder Test End-------------------------------");


      EXIT:
	return 0;
}


OMX_U32 OMX_Jpegd_MemStatPrint(void)
{

/*
Memory_Stats stats;
    OMX_U32 mem_count= 0, mem_size = 0;

    // Get Memory status before running the test case
    mem_count = TIMM_OSAL_GetMemCounter();
    mem_size = TIMM_OSAL_GetMemUsage();

    TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXIMGDEC," Value from GetMemCounter = %d", mem_count);
    TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXIMGDEC," Value from GetMemUsage = %d", mem_size);

    Memory_getStats(NULL, &stats);
    TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXIMGDEC," TotalSize = %d; TotalFreeSize = %d; LargetFreeSize = %d",
		stats.totalSize,stats.totalFreeSize,stats.largestFreeSize);


    return mem_size;*/
	return 0;
}
