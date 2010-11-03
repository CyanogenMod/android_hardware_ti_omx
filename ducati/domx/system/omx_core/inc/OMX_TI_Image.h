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
/*=========================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 * (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied.
 * ==========================================================================*/

/*
 * @file:OMX_TI_Image.h
 * This header defines the structures specific to the param or config
 * indices of Openmax Image Component.
 *
 *  @path domx/system/omx_core/inc
 *
 *  @rev 1.0
 */

/*==============================================================
 *! Revision History
 *! ============================
 *! 11-Oct-2010  KMotov 	x0106653@ti.com  	Initial Version
 *================================================================*/


#ifndef OMX_TI_IMAGE_H
#define OMX_TI_IMAGE_H



/**
 * The OMX_TI_IMAGE_CODINGTYPE enumeration is used to define the
 * extended image coding types.
 */
typedef enum OMX_TI_IMAGE_CODINGTYPE
{
    OMX_TI_IMAGE_CodingJPS = OMX_IMAGE_CodingVendorStartUnused + 1, /**< JPS format */
    OMX_TI_IMAGE_CodingMPO               /**< MPO format */
} OMX_TI_IMAGE_CODINGTYPE;


#endif /* OMX_TI_IMAGE_H */

