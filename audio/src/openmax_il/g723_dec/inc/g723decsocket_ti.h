
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
* @file g723decsocket_ti.h
*
* This is an header file for an audio G723 decoder that is fully
* compliant with the OMX Audio specification.
* This the file is used internally by the component
* in its code and it contains UUID of G723 decoder socket node.
*
* @path  $(CSLPATH)\OMAPSW_MPU\linux\audio\src\openmax_il\g723_dec\inc\
*
* @rev 1.0
*/
/* --------------------------------------------------------------------------- */

#ifndef G723DECSOCKET_TI_H
#define G723DECSOCKET_TI_H
/* ======================================================================= */
/** G723DECSOCKET_TI_UUID: This struct contains the UUID of G723 decoder socket
 * node on DSP.
*/
/* ==================================================================== */
/* G723DEC_SN_UUID= 6E9F0416_4504_49A1_AFBE_09B7141CE87D */
struct DSP_UUID G723DECSOCKET_TI_UUID = {
	0x6e9f0416, 0x4504, 0x49a1, 0xaf, 0xbe, {
	0x09, 0xb7, 0x14, 0x1c, 0xe8, 0x7d
    }
};

#endif /* G723DECSOCKET_TI_H */

