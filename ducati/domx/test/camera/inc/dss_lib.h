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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
/*=========================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 *  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied.
 * ==========================================================================*/
/**
* @file dss_lib.h
*
* This File contains definitions which are required to render the frames to LCD
*
* @path  domx/test/camera/inc
*
* @rev  0.1
*/
/* ----------------------------------------------------------------------------
*!
*! Revision History
*! =======================================================================
*!  Initial Version
* ========================================================================*/
struct dss_buffers
{
	void *start;
	size_t length;
};

int open_video1();
int SendbufferToDss(int index, int vid1_fd);
int SetFormatforDSSvid(unsigned int width, unsigned int height,
    const char *image_fmt, int vid1_fd);
uint getDSSBuffers(uint count, struct dss_buffers *buffers, int vid1_fd);
