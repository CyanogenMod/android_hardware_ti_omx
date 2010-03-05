
/*
 *  Copyright 2001-2008 Texas Instruments - http://www.ti.com/
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* =============================================================================
*             Texas Instruments OMAP (TM) Platform Software
*  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
*
*  Use of this software is controlled by the terms and conditions found
*  in the license agreement under which this software has been supplied.
* =========================================================================== */
/**
* @file AacDecTest.c
*
* This file contains the test application code that invokes the component.
*
* @path  $(CSLPATH)\OMAPSW_MPU\linux\audio\src\openmax_il\aac_dec\tests
*
* @rev  1.0
*/
/* ----------------------------------------------------------------------------
*!
*! Revision History
*! ===================================
*! 21-sept-2006 bk: updated some review findings for alpha release
*! 24-Aug-2006 bk: Khronos OpenMAX (TM) 1.0 Conformance tests some more
*! 18-July-2006 bk: Khronos OpenMAX (TM) 1.0 Conformance tests validated for few cases
*! This is newest file
* =========================================================================== */
/* ------compilation control switches -------------------------*/
/****************************************************************
*  INCLUDE FILES
****************************************************************/
/* ----- system and platform files ----------------------------*/


#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/vt.h>
#include <signal.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdio.h>

#include <Resource_Activity_Monitor.h>


#undef APP_DEBUG

#ifdef APP_DEBUG
#define APP_DPRINT(...)    fprintf(stderr,__VA_ARGS__)
#else
#define APP_DPRINT(...)
#endif

#ifdef APP_MEMCHECK
#define APP_MEMPRINT(...)    fprintf(stderr,__VA_ARGS__)
#else
#define APP_MEMPRINT(...)
#endif


#define APP_OUTPUT_FILE "rm_test_results.txt"
#define SLEEP_TIME 5


/* safe routine to get the maximum of 2 integers */
/* inline int maxint(int a, int b) */

int maxint(int a, int b) {
    return (a > b) ? a : b;
}


int main(int argc, char* argv[]) {
    int error = 0;
    int j =0, tcID =0, testcnt1 = 0;
    int numTestCases = 3;
    int currentOpp = 0, maxDSPfreq = 0;
    int omapVersion = OMAP_NOT_SUPPORTED;
    char* omapVersionStr = "";
    FILE* fOut = NULL;
        
    if (!strcmp(argv[1], "all")) {
        tcID = 0;
    } 
    else {
        printf("Invalid Test Case ID: exiting..\n");
        goto EXIT;
    }

    for (j = 0; j < numTestCases; j++) {
        //printf("main loop counter = %d\n", j);

        if (j == 0) {
            /* do these things first time only */
            /* 1. open the results file */
            fOut = fopen(APP_OUTPUT_FILE, "a");

            if ( fOut == NULL ) {
                printf("Error:  failed to open the file %s for readonly\access\n");
                goto EXIT;
            }

        }

        switch (tcID) {

            case 0:
                printf ("-------------------------------------\n");
                printf ("verify get_omap_version API \n\n");
                printf ("-------------------------------------\n");
                omapVersion = get_omap_version();
                if (omapVersion == OMAP3420_CPU) {
                    omapVersionStr = "omap 3420";
                }
                else if (omapVersion == OMAP3430_CPU) {
                    omapVersionStr = "omap 3430";
                }
                else if (omapVersion == OMAP3440_CPU) {
                    omapVersionStr = "omap 3440";
                }
                else if (omapVersion == OMAP3630_CPU) {
                    omapVersionStr = "omap 3630";
                }
                else {
                    omapVersionStr = "not supported or ERROR";
                }
                printf("\tThe detected OMAP version is %s. Please verify to pass this test\n\n", omapVersionStr);
                if (!strcmp(argv[1], "all")) {
                    tcID ++;
                } 
                break;

            case 1:
                printf ("-------------------------------------\n");
                printf ("verify get/set vdd1 constraint interfaces \n\n");
                printf ("-------------------------------------\n");
                printf ("\tBasic get/set at 430MHz\n");
                currentOpp = rm_get_vdd1_constraint();
                printf ("\tget current OPP = %d\n", currentOpp);
                maxDSPfreq =  get_dsp_max_freq();
                printf ("\tget max dsp freq = %d\n\t\t then set the opp accordingly\n", maxDSPfreq);
                rm_set_vdd1_constraint(maxDSPfreq);
                if (rm_get_vdd1_constraint() == dsp_mhz_to_vdd1_opp(maxDSPfreq)+1){
                    /* +1 is required to adjust the returned value from array index to actual opp value */
                    printf ("\tsuccessfully set OPP for = %d for max dsp frequency\n\n", maxDSPfreq);
                } else {
                    printf ("\tfailed to correctly set OPP = %d for max dsp frequency\n\n", maxDSPfreq);
                }
                if (!strcmp(argv[1], "all")) {
                    tcID ++;
                } 
                break;

            case 2:
                printf ("-------------------------------------\n");
                printf ("Testing new API test \n");
                printf ("-------------------------------------\n");
                if (!strcmp(argv[1], "all")) {
                    tcID ++;
                } 
                break;

            case 3:
                printf ("-------------------------------------\n");
                printf ("Testing new API test\n");
                printf ("-------------------------------------\n");
                if (!strcmp(argv[1], "all")) {
                    tcID ++;
                } 
                break;
        }

    }


EXIT:
   
    return error;
}
