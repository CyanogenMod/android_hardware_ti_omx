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
 *  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied.
 * ==========================================================================*/
/*
 *  @file  omx_ti_core.h
 *         This file contains the vendor(TI) specific core extensions
 *
 *  @path domx/system/omx_core/inc
 *
 *  @rev 1.0
 */

/*==============================================================
 *! Revision History
 *! ============================
 *! 19-Jul-2010 admonga@ti.com : Initial version
 *================================================================*/

#ifndef _OMX_TI_CORE_H_
#define _OMX_TI_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************
    INCLUDE FILES
 ******************************************************************/
#include <OMX_Types.h>
#include <OMX_Core.h>

/*******************************************************************
 * EXTERNAL REFERENCE NOTE: only use if not found in header file
 *******************************************************************/
/*----------         function prototypes      ------------------- */
/*----------         data declarations        ------------------- */
/*******************************************************************
 * PUBLIC DECLARATIONS: defined here, used elsewhere
 *******************************************************************/
/*----------         function prototypes      ------------------- */
/*----------         data declarations        ------------------- */



typedef enum OMX_TI_ERRORTYPE
{
    /* Vendor specific area for storing TI custom extended events */

    /*Control attribute is pending - Dio_Dequeue will not work until attribute
      is retreived*/
    OMX_TI_WarningAttributePending = (OMX_S32)((OMX_ERRORTYPE)OMX_ErrorVendorStartUnused + 1),
    /*Attribute buffer size is insufficient - reallocate the attribute buffer*/
    OMX_TI_WarningInsufficientAttributeSize,
    /*EOS buffer has been received*/
    OMX_TI_WarningEosReceived,
    /*Port enable is called on an already enabled port*/
    OMX_TI_ErrorPortIsAlreadyEnabled,
    /*Port disable is called on an already disabled port*/
    OMX_TI_ErrorPortIsAlreadyDisabled
} OMX_TI_ERRORTYPE;



typedef enum OMX_TI_EVENTTYPE
{
    /* Vendor specific area for storing indices */
    /*Reference count for the buffer has changed. In the callback, nData1 will
      pBufferHeader, nData2 will be present count*/
    OMX_TI_EventBufferRefCount = (OMX_S32)((OMX_EVENTTYPE)OMX_EventVendorStartUnused + 1)
}OMX_TI_EVENTTYPE;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OMX_TI_INDEX_H_ */

