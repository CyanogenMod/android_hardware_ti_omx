
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
* @file OMX_PcmDecoder.h
*
* This is an header file for an audio PCM decoder that is fully
* compliant with the OMX Audio specification.
* This the file that the application that uses OMX would include
* in its code.
*
* @path  $(CSLPATH)\OMAPSW_MPU\linux\audio\src\openmax_il\pcm_dec\inc\
*
* @rev 1.0
*/
/* --------------------------------------------------------------------------- */


#ifndef OMX_PCMDECODER_H
#define OMX_PCMDECODER_H


#ifdef UNDER_CE
#include <windows.h>
#include <oaf_osal.h>
#include <omx_core.h>
#include <stdlib.h>
#else
#include <pthread.h>
#endif

#include <OMX_Component.h>

#ifndef UNDER_CE
#define RM_MANAGER /* Enable to use Resource Manager functionality */
#else
#undef RM_MANAGER /* Enable to use Resource Manager functionality */
#endif

/* #define PCMDEC_DEBUG */    /* See all debug statement of the component */
/* #define PCMDEC_MEMDETAILS */ /* See memory details of the component */
/* #define PCMDEC_BUFDETAILS */  /* See buffers details of the component */
/* #define PCMDEC_STATEDETAILS */ /* See all state transitions of the component */
/* #define PCMDEC_SWATDETAILS */  /* See SWAT debug statement of the component */


#define MAX_NUM_OF_BUFS 10 /* Max number of buffers used */
#define PCMD_NUM_INPUT_BUFFERS 1  /* Default number of input buffers */
#define PCMD_NUM_OUTPUT_BUFFERS 1 /* Default number of output buffers */
/*#define PCMD_INPUT_BUFFER_SIZE 4096*/
#define PCMD_INPUT_BUFFER_SIZE  1000 /* Default size of input buffer */
#define PCMD_OUTPUT_BUFFER_SIZE 320 /* Default size of output buffer */

#define PCMD_MONO_STREAM  1 /* Mono stream index */
#define PCMD_STEREO_INTERLEAVED_STREAM  2 /* Stereo Interleaved stream index */
#define PCMD_STEREO_NONINTERLEAVED_STREAM  3 /* Stereo Non-Interleaved stream index */

#define NUM_OF_PORTS 2 /* Number of ports of component */

#define INVALID_SAMPLING_FREQ  325

/* ======================================================================= */
/** OMX_PCMDEC_INDEXAUDIOTYPE  Defines the custom configuration settings
*                              for the component
*
*  @param  OMX_IndexCustomPcmDecModeDasfConfig      Sets the DASF mode
*
*/
/*  ==================================================================== */
typedef enum OMX_PCMDEC_INDEXAUDIOTYPE {
	OMX_IndexCustomPcmDecModeDasfConfig = OMX_IndexIndexVendorStartUnused + 1,
	OMX_IndexCustomPcmDecHeaderInfoConfig,
	OMX_IndexCustomPcmDecLowLatencyConfig,
	OMX_IndexCustomPcmDecStreamIDConfig,
    OMX_IndexCustomPcmDecDataPath
}OMX_PCMDEC_INDEXAUDIOTYPE;


/* ============================================================================== * */
/** PCMD_COMP_PORT_TYPE  describes the input and output port of indices of the
* component.
*
* @param  PCMD_INPUT_PORT  Input port index
*
* @param  PCMD_OUTPUT_PORT Output port index
*/
/* ============================================================================ * */
typedef enum PCMD_COMP_PORT_TYPE {
    PCMD_INPUT_PORT = 0,
    PCMD_OUTPUT_PORT
}PCMD_COMP_PORT_TYPE;

#endif /* OMX_PCMDECODER_H */
