
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

#undef RAM_DEBUG

#ifdef RAM_DEBUG
  #include <utils/Log.h>
  #undef LOG_TAG
  #define LOG_TAG "OMXRM DVFS MONITOR"
  #define RAM_DPRINT LOGD
#else
  #define RAM_DPRINT(...)
#endif

/* define these for array indexing */
#define OPERATING_POINT_1 0
#define OPERATING_POINT_2 1
#define OPERATING_POINT_3 2
#define OPERATING_POINT_4 3
#define OPERATING_POINT_5 4

/* for 3440 */
#define OPERATING_POINT_6 5

typedef enum _OMAP_CPU
{
    OMAP3420_CPU = 0,
    OMAP3430_CPU,
    OMAP3440_CPU
} OMAP_CPU;

/* for 3420 family */
static const int vdd1_dsp_mhz_3420[5] = {90, 180, 360, 360, 360};
static const int vdd1_mpu_mhz_3420[5] = {125, 250, 500, 550, 600};

/* for 3430 family */
static const int vdd1_dsp_mhz_3430[5] = {90, 180, 360, 430, 430};
static const int vdd1_mpu_mhz_3430[5] = {125, 250, 500, 550, 600};

/* for 3440 family */
static const int vdd1_dsp_mhz_3440[6] = {90, 180, 360, 430, 430, 520};
static const int vdd1_mpu_mhz_3440[6] = {125, 250, 500, 550, 600, 720};


void omap_pm_set_constraint(int ID, int MHz);

int omap_pm_get_constraint(int ID);
char * ram_itoa(int a);

