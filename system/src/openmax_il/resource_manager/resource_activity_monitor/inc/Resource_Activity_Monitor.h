
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

/* move this to omx/Android.mk */
//#define RAM_ENABLED

#define OPERATING_POINT_1 1
#define OPERATING_POINT_1_MHZ 90

#define OPERATING_POINT_2 2
#define OPERATING_POINT_2_MHZ 180

#define OPERATING_POINT_3 3
#define OPERATING_POINT_3_MHZ 360

#define OPERATING_POINT_4 4
#define OPERATING_POINT_4_MHZ 400

#define OPERATING_POINT_5 5
#define OPERATING_POINT_5_MHZ 430


void omap_pm_set_constraint(int ID, int MHz);

int omap_pm_get_constraint(int ID);
char * ram_itoa(int a);

