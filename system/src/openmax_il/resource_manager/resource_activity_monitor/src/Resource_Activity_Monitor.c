
/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Resource_Activity_Monitor.h"

#undef LOG_TAG
#define LOG_TAG "OMXRM DVFS MONITOR"

void omap_pm_set_constraint(int ID, int MHz)
{
#ifdef DVFS_ENABLED
    char command[100];
    int mpu_max_freq = 0;
    int cpu_variant = 0;
    int dsp_max_freq = 0;
    int c_state = 0;
    /* initialize both vdd1 & vdd2 at 2
       idea is to prohobit vdd1=1 during MM use cases */
    unsigned int vdd1_opp = OPERATING_POINT_2;
    unsigned int vdd2_opp = OPERATING_POINT_2;

    FILE *fp = fopen("/sys/power/max_dsp_frequency","r");
    if (fp == NULL) RAM_DPRINT("open file failed\n");
    fscanf(fp, "%d",&dsp_max_freq);
    fclose(fp);
    dsp_max_freq /= 1000000;
    if (dsp_max_freq == vdd1_dsp_mhz_3420[OPERATING_POINT_5]){
        cpu_variant = OMAP3420_CPU;
    }
    else if (mpu_max_freq == vdd1_mpu_mhz_3430[OPERATING_POINT_5]){
        cpu_variant = OMAP3430_CPU;
    }
    else if (mpu_max_freq == vdd1_mpu_mhz_3440[OPERATING_POINT_6]){
        /* 3440 has 6 OPPs */
        cpu_variant = OMAP3440_CPU;
    }

    /* for any MM case, set c-state to 2, unless no mm is active */
    c_state = 2;

    if(MHz == 0)
    {
        /* clear constraints for idle MM case */
        vdd1_opp = OPERATING_POINT_1;
        vdd2_opp = OPERATING_POINT_2;

        /* set c-state back if no active MM */
        c_state = 6;
    }
    else if (cpu_variant == OMAP3420_CPU){
        if (MHz <= vdd1_dsp_mhz_3420[OPERATING_POINT_2]) {
            /* MM should never use opp1, so skip to opp2 */
            vdd1_opp = OPERATING_POINT_2;
        }
        else if (MHz <= vdd1_dsp_mhz_3420[OPERATING_POINT_3]) {
            vdd1_opp = OPERATING_POINT_3;
        }
        else if (MHz <= vdd1_dsp_mhz_3420[OPERATING_POINT_4]) {
            vdd1_opp = OPERATING_POINT_4;
        }
        else {
            vdd1_opp = OPERATING_POINT_5;
        }
    }
    else if (cpu_variant == OMAP3430_CPU){
        if (MHz <= vdd1_dsp_mhz_3430[OPERATING_POINT_2]) {
            /* MM should never use opp1, so skip to opp2 */
            vdd1_opp = OPERATING_POINT_2;
        }
        else if (MHz <= vdd1_dsp_mhz_3430[OPERATING_POINT_3]) {
            vdd1_opp = OPERATING_POINT_3;
        }
        else if (MHz <= vdd1_dsp_mhz_3430[OPERATING_POINT_4]) {
            vdd1_opp = OPERATING_POINT_4;
        }
        else {
            vdd1_opp = OPERATING_POINT_5;
        }
    }
    else if (cpu_variant == OMAP3440_CPU){
        if (MHz <= vdd1_dsp_mhz_3440[OPERATING_POINT_2]) {
            /* MM should never use opp1, so skip to opp2 */
            vdd1_opp = OPERATING_POINT_2;
        }
        else if (MHz <= vdd1_dsp_mhz_3440[OPERATING_POINT_3]) {
            vdd1_opp = OPERATING_POINT_3;
        }
        else if (MHz <= vdd1_dsp_mhz_3440[OPERATING_POINT_4]) {
            vdd1_opp = OPERATING_POINT_4;
        }
        else if (MHz <= vdd1_dsp_mhz_3440[OPERATING_POINT_5]) {
            vdd1_opp = OPERATING_POINT_5;
        }
        else {
            vdd1_opp = OPERATING_POINT_6;
        }
    }

    /* actually set the sysfs for vdd2_opp */
    if(vdd1_opp > OPERATING_POINT_2)
    {
        vdd2_opp = OPERATING_POINT_3;
    }

    /* plus one to convert zero based array indeces above */
    vdd1_opp++;
    vdd2_opp++;

    /* actually set the sysfs for vdd1_opp */
    RAM_DPRINT("[setting operating point] MHz = %d vdd1 = %d\n",MHz,vdd1_opp);
    strcpy(command,"echo ");
    strcat(command,ram_itoa(vdd1_opp)); /* plus one to convert zero based array indeces above */
    strcat(command," > /sys/power/vdd1_opp");
    system(command);

    RAM_DPRINT("[setting operating point] MHz = %d vdd2 = %d\n",MHz,vdd2_opp);
    strcpy(command,"echo ");
    strcat(command,ram_itoa(vdd2_opp)); /* plus one to convert zero based array indeces above */
    strcat(command," > /sys/power/vdd2_opp");
    system(command);

    /* actually set the sysfs for cpuidle/max_state */
    RAM_DPRINT("[setting c-state] c-state %d\n",c_state);
    strcpy(command,"echo ");
    strcat(command,ram_itoa(c_state));
    strcat(command," > /sys/devices/system/cpu/cpu0/cpuidle/max_state");
    system(command);

#endif
}

int omap_pm_get_constraint(int ID)
{
    RAM_DPRINT("[omap_pm_get_constraint] id = %d \n",ID);
    return 0;
}

char * ram_itoa(int a)
{
    static char str[50];
    int i = sizeof(str) - 1;
    if (i != 0) {
        do {
            str[--i] = '0' + a % 10;
        }
        while ((a = a / 10) && i > 0);
    }
    return &str[i];
}

