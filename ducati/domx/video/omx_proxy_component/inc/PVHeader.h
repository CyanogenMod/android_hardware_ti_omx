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
 *  @file  PVHeader.h
 *         This file contains methods that provides the functionality for
 *         the PV core specific headers
 *
 *  @path
 *
 *  @rev 1.0
 */

/*==============================================================
 *! Revision History
 *! ============================
 *! 11-April-2010 Saket dsaket@ti.com: Initial Version
 *================================================================*/

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/

#ifndef __PVHEADER__
#define __PVHEADER__

#define PV_OMX_COMPONENT_CAPABILITY_TYPE_INDEX 0xFF7A347
/** Opencore specific, refer to OpenMax Call Sequences document section 3.2 **/
typedef struct PV_OMXComponentCapabilityFlagsType
{
    ////////////////// OMX COMPONENT CAPABILITY RELATED MEMBERS
    OMX_BOOL iIsOMXComponentMultiThreaded;
    OMX_BOOL iOMXComponentSupportsExternalOutputBufferAlloc;
    OMX_BOOL iOMXComponentSupportsExternalInputBufferAlloc;
    OMX_BOOL iOMXComponentSupportsMovableInputBuffers;
    OMX_BOOL iOMXComponentSupportsPartialFrames;
    OMX_BOOL iOMXComponentUsesNALStartCodes;
    OMX_BOOL iOMXComponentCanHandleIncompleteFrames;
    OMX_BOOL iOMXComponentUsesFullAVCFrames;
} PV_OMXComponentCapabilityFlagsType;

#endif //__PVHEADER__
