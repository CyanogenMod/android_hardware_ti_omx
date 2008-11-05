
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
/* =============================================================================
*             Texas Instruments OMAP(TM) Platform Software
*  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
*
*  Use of this software is controlled by the terms and conditions found
*  in the license agreement under which this software has been supplied.
* ============================================================================ */
/**
* @file OMX_PcmDec_CompThread.h
*
* This is an header file for an audio PCM decoder that is fully
* compliant with the OMX Audio specification.
* This the file is used internally by the component
* in its code and it contains UUID of PCM decoder socket node.
*
* @path  $(CSLPATH)\OMAPSW_MPU\linux\audio\src\openmax_il\pcm_dec\inc\
*
* @rev 1.0
*/
/* --------------------------------------------------------------------------- */

#ifndef PCMDECSOCKET_TI_H
#define PCMDECSOCKET_TI_H
/* ======================================================================= */
/** PCMDECSOCKET_TI_UUID: This struct contains the UUID of PCM decoder socket
 * node on DSP.
*/
/* ==================================================================== */
struct DSP_UUID PCMDECSOCKET_TI_UUID = {
	0xad4465dc, 0x80d4, 0x44d0, 0x81, 0x4a, {
	0xad, 0xb6, 0xe5, 0xc7, 0x72, 0x5d
    }
};

#endif /* PCMDECSOCKET_TI_H */

