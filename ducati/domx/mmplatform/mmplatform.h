
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef MMPLATFORM_H
#define MMPLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
setup = 1 for full ipc setup(including procmgr_start for loading the base image)
setup = 2 for ipc setup without loading the base image
*/
int mmplatform_init(int setup);

int mmplatform_deinit();
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif 
