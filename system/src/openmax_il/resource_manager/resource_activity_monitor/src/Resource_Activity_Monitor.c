
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


int currentMHzConstraint = 0;

int rm_set_vdd1_constraint(int MHz)
{
    int vdd1_opp = 0;
    
#ifdef DVFS_ENABLED
    
    char command[100];
    int vdd2_opp = 0;
    /* for any MM case, set c-state to 2, unless no mm is active */
    int c_state = 2;
    
    currentMHzConstraint = MHz;
    if(MHz == 0)
    {
        /* clear constraints for idle MM case */
        vdd1_opp = OPERATING_POINT_1;
        vdd2_opp = OPERATING_POINT_2;
        
        /* set c-state back if no active MM */
        c_state = 6;
    }
    else
    {
        vdd1_opp = dsp_mhz_to_vdd1_opp(MHz);
    }
    
    /* actually set the sysfs for vdd2_opp */
    if(vdd1_opp > OPERATING_POINT_2)
    {
        vdd2_opp = OPERATING_POINT_3;
    }

    /* plus one to convert zero based array indeces above */
    vdd1_opp++;
    vdd2_opp++;
    if (vdd1_opp != rm_get_vdd1_constraint())
    {
        /* actually set the sysfs for vdd1_opp */
        RAM_DPRINT("[setting operating point] MHz = %d vdd1 = %d\n",MHz,vdd1_opp);
        strcpy(command,"echo ");
        strcat(command,ram_itoa(vdd1_opp));
        strcat(command," > /sys/power/vdd1_opp");
        system(command);

        RAM_DPRINT("[setting operating point] MHz = %d vdd2 = %d\n",MHz,vdd2_opp);
        strcpy(command,"echo ");
        strcat(command,ram_itoa(vdd2_opp));
        strcat(command," > /sys/power/vdd2_opp");
        system(command);

        /* actually set the sysfs for cpuidle/max_state */
        RAM_DPRINT("[setting c-state] c-state %d\n",c_state);
        strcpy(command,"echo ");
        strcat(command,ram_itoa(c_state));
        strcat(command," > /sys/devices/system/cpu/cpu0/cpuidle/max_state");
        system(command);
    }

#endif

    return vdd1_opp;
}

int dsp_mhz_to_vdd1_opp(int MHz)
{
    /* initialize both vdd1 & vdd2 at 2
     idea is to prohobit vdd1=1 during MM use cases */
    unsigned int vdd1_opp = OPERATING_POINT_2;
    unsigned int vdd2_opp = OPERATING_POINT_2;
    int cpu_variant = get_omap_version();
    
    switch (cpu_variant) {
    case OMAP3420_CPU:
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
        break;
        
        case OMAP3440_CPU:
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
        break;
        
        case OMAP3430_CPU:
        default:
        /* intentionally fall through */
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
        break;
    } /* end switch */
    return vdd1_opp;
}

int get_omap_version()
{
    int cpu_variant = 0;
    int dsp_max_freq = 0;
    
    FILE *fp = fopen("/sys/power/max_dsp_frequency","r");
    if (fp == NULL) RAM_DPRINT("open file failed\n");
    fscanf(fp, "%d",&dsp_max_freq);
    fclose(fp);
    dsp_max_freq /= 1000000;
    if (dsp_max_freq == vdd1_dsp_mhz_3420[OPERATING_POINT_5]){
        cpu_variant = OMAP3420_CPU;
    }
    else if (dsp_max_freq == vdd1_dsp_mhz_3430[OPERATING_POINT_5]){
        cpu_variant = OMAP3430_CPU;
    }
    else if (dsp_max_freq == vdd1_dsp_mhz_3440[OPERATING_POINT_6]){
        /* 3440 has 6 OPPs */
        cpu_variant = OMAP3440_CPU;
    }
    
    return cpu_variant;
    
}

int rm_get_vdd1_constraint()
{
    int vdd1_opp = 0;
    FILE *fp = fopen("/sys/power/vdd1_opp","r");
    if (fp == NULL) RAM_DPRINT("open file failed\n");
    fscanf(fp, "%d",&vdd1_opp);
    fclose(fp);
    RAM_DPRINT("[rm_get_vdd1_constraint] vdd1 OPP = %d \n",vdd1_opp);
    return vdd1_opp;
    
}

void rm_request_boost(int level)
{
    int vdd1_opp = rm_get_vdd1_constraint();
    int boostConstraintMHz = 0;
    int cpu_variant =  get_omap_version();
    
    if (level == MAX_BOOST)
    {
        switch (cpu_variant)
        {
            case OMAP3420_CPU:
                boostConstraintMHz = vdd1_dsp_mhz_3420[ sizeof(vdd1_dsp_mhz_3420)/sizeof(vdd1_dsp_mhz_3420[0]) ];
                break;
                
            case OMAP3440_CPU:
                boostConstraintMHz = vdd1_dsp_mhz_3440[ sizeof(vdd1_dsp_mhz_3440)/sizeof(vdd1_dsp_mhz_3420[0]) ];
                break;
                
            case OMAP3430_CPU:
            default:
            /* fall through intentional */
                boostConstraintMHz = vdd1_dsp_mhz_3430[ sizeof(vdd1_dsp_mhz_3430)/sizeof(vdd1_dsp_mhz_3420[0])];
                break;
        }
    }
    else if (level == NOMINAL_BOOST)
    {
        switch (cpu_variant)
        {
            case OMAP3420_CPU:
                boostConstraintMHz = vdd1_dsp_mhz_3420[sizeof(vdd1_dsp_mhz_3420)/sizeof(vdd1_dsp_mhz_3420[0]) - 1];
                break;
                
            case OMAP3440_CPU:
                boostConstraintMHz = vdd1_dsp_mhz_3440[sizeof(vdd1_dsp_mhz_3440)/sizeof(vdd1_dsp_mhz_3440[0]) -1];
                break;
                
            case OMAP3430_CPU:
            default:
                /* fall through intentional */
                boostConstraintMHz = vdd1_dsp_mhz_3430[sizeof(vdd1_dsp_mhz_3430)/sizeof(vdd1_dsp_mhz_3430[0]) -1];
                break;
        }
    }
    if (dsp_mhz_to_vdd1_opp(boostConstraintMHz) != vdd1_opp)
    {
        rm_set_vdd1_constraint(boostConstraintMHz);
    }
}


void rm_release_boost()
{
    int vdd1_opp = rm_get_vdd1_constraint();
    int currentConstraint = dsp_mhz_to_vdd1_opp(currentMHzConstraint);
    
    if (currentConstraint != vdd1_opp)
    {
         rm_set_vdd1_constraint(currentMHzConstraint);
    }
   
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

