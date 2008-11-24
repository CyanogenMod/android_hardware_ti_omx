
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
* @file OMX_AacDecoder.h
*
* This is an header file for an AAC Decoder that is fully
* compliant with the OMX Audio specification 1.5.
* This the file that the application that uses OMX would include
* in its code.
*
* @path $(CSLPATH)\
*
* @rev 0.1
*/
/* --------------------------------------------------------------------------- */

#ifndef OMX_AACDECODER_H
#define OMX_AACDECODER_H

#include <pthread.h>
#ifdef UNDER_CE
#include <windows.h>
#include <oaf_osal.h>
#include <omx_core.h>
#include <stdlib.h>
#else
#endif

#include <OMX_Component.h>
#include "LCML_DspCodec.h"

#ifndef UNDER_CE
#define AUDIO_MANAGER
#else
#undef AUDIO_MANAGER
#endif

 /* ======================================================================= */
 /**
  * @def    AACDEC_BUFDETAILS   Turns buffer messaging on and off
  */
 /* ======================================================================= */
 #undef AACDEC_BUFDETAILS
 /* ======================================================================= */
 /**
  * @def    AACDEC_STATEDETAILS   Turns state messaging on and off
  */
 /* ======================================================================= */
 #undef AACDEC_STATEDETAILS
 /* ======================================================================= */
 /**
  * @def    AACDEC_MEMDETAILS   Turns memory messaging on and off
  */
 /* ======================================================================= */
 #undef AACDEC_MEMDETAILS
/* #define AACDEC_SWATDETAILS*/

#define AACDEC_APP_ID  100
#define MAX_NUM_OF_BUFS_AACDEC 15
#define PARAMETRIC_STEREO_AACDEC 1
#define NON_PARAMETRIC_STEREO_AACDEC 0
/* ======================================================================= */
/**
 * @def    NUM_OF_PORTS_AACDEC   Number of ports
 */
/* ======================================================================= */
#define NUM_OF_PORTS_AACDEC 2
/* ======================================================================= */
/**
 * @def    STREAM_COUNT_AACDEC   Number of streams
 */
/* ======================================================================= */
#define STREAM_COUNT_AACDEC 2

/** Default timeout used to come out of blocking calls*/

/* ======================================================================= */
/**
 * @def    AACD_NUM_INPUT_BUFFERS   Default number of input buffers
 *
 */
/* ======================================================================= */
#define AACD_NUM_INPUT_BUFFERS 4
/* ======================================================================= */
/**
 * @def    AACD_NUM_OUTPUT_BUFFERS   Default number of output buffers
 *
 */
/* ======================================================================= */
#define AACD_NUM_OUTPUT_BUFFERS 2

/* ======================================================================= */
/**
 * @def    AACD_INPUT_BUFFER_SIZE   Default input buffer size
 *
 */
/* ======================================================================= */
#define AACD_INPUT_BUFFER_SIZE 1536
/* ======================================================================= */
/**
 * @def    AACD_OUTPUT_BUFFER_SIZE   Default output buffer size
 *
 */
/* ======================================================================= */
#define AACD_OUTPUT_BUFFER_SIZE 8192
/* ======================================================================= */
/**
 * @def    AACD_SAMPLING_FREQUENCY   Sampling frequency
 */
/* ======================================================================= */
#define AACD_SAMPLING_FREQUENCY 44100


/* ======================================================================= */
/**
 * @def    AACDec macros for MONO,STEREO,STEREO_NONINTERLEAVED
 */
/* ======================================================================= */
#define AACD_MONO_STREAM    1
#define AACD_STEREO_STREAM  2
#define AACD_STEREO_NONINTERLEAVED_STREAM  3

/* ======================================================================= */
/**
 * @def    Mem test application
 */
/* ======================================================================= */
#undef AACDEC_DEBUGMEM 

#ifdef AACDEC_DEBUGMEM
#define newmalloc(x) mymalloc(__LINE__,__FILE__,x)
#define newfree(z) myfree(z,__LINE__,__FILE__) 
#else
#define newmalloc(x) malloc(x)
#define newfree(z) free(z)
#endif

/* ======================================================================= */
/**
 * @def    AACDec macros for MONO,STEREO_INTERLEAVED,STEREO_NONINTERLEAVED
 */
/* ======================================================================= */

#define AACD_STEREO_INTERLEAVED_STREAM     2
#define AACD_STEREO_NONINTERLEAVED_STREAM  3
/* ======================================================================= */
/**
 * @def    AACDec macros for MONO,STEREO_INTERLEAVED,STEREO_NONINTERLEAVED
 */
/* ======================================================================= */
/* Stream types supported*/
#define MONO_STREAM_AACDEC                   1
#define STEREO_INTERLEAVED_STREAM_AACDEC     2
#define STEREO_NONINTERLEAVED_STREAM_AACDEC  3

/**
 *
 * AAC Decoder Profile:0 - MAIN, 1 - LC, 2 - SSR, 3 - LTP.
 */
typedef enum
{
    EProfileMain,
    EProfileLC,
    EProfileSSR,
    EProfileLTP
}AACProfile;
/* ======================================================================= */
/** COMP_PORT_TYPE_AACDEC  Port types
*
*  @param  INPUT_PORT_AACDEC					Input port
*
*  @param  OUTPUT_PORT_AACDEC				Output port
*/
/*  ==================================================================== */
/*This enum must not be changed. */
typedef enum COMP_PORT_TYPE_AACDEC {
    INPUT_PORT_AACDEC = 0,
    OUTPUT_PORT_AACDEC
}COMP_PORT_TYPE_AACDEC;
/* ======================================================================= */
/** OMX_INDEXAUDIOTYPE_AACDEC  Defines the custom configuration settings
*                              for the component
*
*  @param  OMX_IndexCustomMode16_24bit_AACDEC  Sets the 16/24 mode
*
*  @param  OMX_IndexCustomModeProfile_AACDEC  Sets the Profile mode
*
*  @param  OMX_IndexCustomModeSBR_AACDEC  Sets the SBR mode
*
*  @param  OMX_IndexCustomModeDasfConfig_AACDEC  Sets the DASF mode
*
*  @param  OMX_IndexCustomModeRAW_AACDEC  Sets the RAW mode
*
*  @param  OMX_IndexCustomModePS_AACDEC  Sets the ParametricStereo mode
*
*/
/*  ==================================================================== */
typedef enum OMX_INDEXAUDIOTYPE_AACDEC {
	OMX_IndexCustomAacDecHeaderInfoConfig = 0xFF000001,
	OMX_IndexCustomAacDecStreamIDConfig,
    OMX_IndexCustomAacDecDataPath
}OMX_INDEXAUDIOTYPE_AACDEC;

/* TEMPORARY DEFINES FOR DEBUGGING */
#define NUM_DOMAINS 0x4
#endif /* OMX_AACDECODER_H */
