/*
 * Copyright (c) 2010, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 *****************************************************************************
 * \file
 *    H264_ILClient.c
 *
 * \brief
 *  This file contains IL Client implementation specific to OMX H264 Decoder
 *
 * \version 1.0
 *
 *****************************************************************************
 */

/****************************************************************
 * INCLUDE FILES
 ****************************************************************/

#define H264_LINUX_CLIENT

#include <pthread.h>
#include <sched.h>

#include <string.h>
#include <sys/time.h>

#ifndef H264_LINUX_CLIENT
#include <ti/sysbios/knl/Task.h>
#include <ti/sdo/codecs/h264dec/ih264vdec.h>
#endif
//#include "ih264vdec.h"

#ifndef H264_LINUX_CLIENT
#include <WTSD_DucatiMMSW/omx/omx_il_1_x/omx_h264_dec/test/H264_Decoder_ILClient.h>
#include <WTSD_DucatiMMSW/platform/osal/timm_osal_events.h>
#include <WTSD_DucatiMMSW/platform/osal/timm_osal_pipes.h>
#include <WTSD_DucatiMMSW/platform/osal/timm_osal_semaphores.h>
#include <WTSD_DucatiMMSW/platform/osal/timm_osal_error.h>
#include <WTSD_DucatiMMSW/platform/osal/timm_osal_task.h>
#include <WTSD_DucatiMMSW/platform/osal/timm_osal_memory.h>
#include <ti/sysbios/hal/unicache/Cache.h>
#else
#include <H264_Playback_ILClient.h>
#include <timm_osal_events.h>
#include <timm_osal_pipes.h>
#include <timm_osal_semaphores.h>
#include <timm_osal_error.h>
#include <timm_osal_task.h>
#include <timm_osal_memory.h>
#include <debug_utils.h>
#include <v4l2_display.h>
#endif


#ifdef H264_LINUX_CLIENT
#if 0
      //AD - only for linux testing
#define OMX_H264D_SRCHANGES
#define OMX_H264D_BUF_HEAP
#define OMX_H264D_NONTILERTEST
#endif
#include <unistd.h>
#if 1
#define OMX_H264D_LINUX_TILERTEST
#define OMX_H264D_USEBUFFERENABLED
#undef OMX_H264D_ALLOCBUFFERENABLED
#endif
#ifdef OMX_H264D_BUF_HEAP
#include <unistd.h>
#include <RcmClient.h>
#include <HeapBuf.h>
#include <SharedRegion.h>
#endif

#ifdef OMX_H264D_LINUX_TILERTEST
#include <memmgr.h>
#endif
#ifdef OMX_H264D_BUF_HEAP
extern HeapBuf_Handle heapHandle;
#endif
#ifdef OMX_H264D_LINUX_TILERTEST
#define STRIDE_8BIT (4 * 1024)
#define STRIDE_16BIT (4 * 1024)
#endif
#endif

/****************************************************************
 * DEFINES
 ****************************************************************/

/** Event definition to indicate input buffer consumed */
#define H264_DECODER_INPUT_READY           1

/** Event definition to indicate output buffer consumed */
#define H264_DECODER_OUTPUT_READY           2

/** Event definition to indicate error in processing */
#define H264_DECODER_ERROR_EVENT            4

/** Event definition to indicate End of stream */
#define H264_DECODER_END_OF_STREAM          8

#define H264_STATETRANSITION_COMPLETE       16

/** Event definition to indicate Video buffer free for reading stream from MMC */
#define H264_DECODER_VIDEO_INPUT_BUF_FREE      (1<<5)
/** Event definition to indicate Video buffer ready for H264 decoding */
#define H264_DECODER_VIDEO_INPUT_BUF_READY     (1<<6)
/** Event definition to indicate Video buffer ready to be display */
#define H264_DECODER_VIDEO_OUTPUT_BUF_READY    (1<<7)
/** Event definition to indicate Video buffer free by the display */
#define H264_DECODER_VIDEO_OUTPUT_BUF_FREE     (1<<8)

#define     PADX    32
#define     PADY    24

//#define __NO_DISPLAY__
#define __MULTIPLE_INPUT_BUF__
#define __TASK_INPUT__

//#undef __MULTIPLE_INPUT_BUF__
#ifdef __MULTIPLE_INPUT_BUF__
#define BITSTREAM_SIZE_FILE "/patterns/input/test_nobits_framedata.txt"
#define BITSTREAM_BUF_MAX_SIZE 300

FILE *fFrameDataFile;
OMX_U32 maxBufferSize;
char input_char[10] = { "\0" };

OMX_U32 frameSize[BITSTREAM_BUF_MAX_SIZE];
OMX_U32 frameCount;
OMX_U32 maxFrameCount;
OMX_U32 frameReadCount;
OMX_U32 OutputFrameCount;
OMX_U32 InputFrameCount;

#endif

#ifdef __TASK_OUTPUT__
static TIMM_OSAL_PTR myEventOut;
#endif
#ifdef __TASK_INPUT__

static TIMM_OSAL_PTR myEventIn;
void H264_InputDataRead(void *threadsArg);
#endif

/****************************************************************
 * GLOBALS
 ****************************************************************/

static TIMM_OSAL_PTR pSem_Events = NULL;
static TIMM_OSAL_PTR pSem_InputPort = NULL;
static TIMM_OSAL_PTR pSem_OutputPort = NULL;
static TIMM_OSAL_PTR pInputPtr = NULL;
static TIMM_OSAL_PTR myEvent;
static TIMM_OSAL_PTR H264VD_CmdEvent;


static TIMM_OSAL_TRACEGRP nTraceGroup;
static TIMM_OSAL_U32 num_out_buffers;

extern H264_Decoder_TestCaseParamsType H264_Decoder_TestCaseParams[];


#ifndef H264_LINUX_CLIENT
/* ==========================================================================
*
*@func   outputDisplayFrame()
*
*@brief  Sub function to write output data to a file
*
*@param  outArgs                  output arguments
*@param  xoff                     horizontal offset
*@param  yoff                     vertical offset
*
*@ret    none
*
* ============================================================================
*/
static unsigned short outputDisplayFrame(IH264VDEC_OutArgs * outArgs,
    unsigned short xoff, unsigned short yoff, unsigned short ref_width,
    unsigned short ref_height, unsigned short width, unsigned short height,
    unsigned short pic_struct, unsigned short hrd, unsigned int fieldBuf,
    FILE * fout, unsigned int complianceMode)
{
	IVIDEO2_BufDesc *displayBufs =
	    (IVIDEO2_BufDesc *) outArgs->viddec3OutArgs.displayBufs.pBufDesc;

  /*--------------------------------------------------------------------------------
   Here the actual frame data (w/out padding), is extracted and dumped.
   Also UV is de-interleaved.
  --------------------------------------------------------------------------------*/
	char *lumaAddr = (char *)displayBufs->planeDesc[0].buf;
	char *chromaAddr = (char *)displayBufs->planeDesc[1].buf;
	unsigned short retval = 0;
	char *CbBuf, *CrBuf, *YBuf;
	unsigned int pic_size, i, j;
	unsigned short ref_width_c = ref_width;

	if (displayBufs->planeDesc[1].memType)
		ref_width_c *= displayBufs->planeDesc[1].memType;

	pic_size = width * height;

	lumaAddr =
	    (char *)((unsigned int)displayBufs->planeDesc[0].buf +
	    (yoff * ref_width) + xoff);
	chromaAddr =
	    (char *)((unsigned int)displayBufs->planeDesc[1].buf +
	    ((yoff >> 1) * ref_width_c) + xoff);

	YBuf = (char *)fieldBuf;
	for (i = 0; i < height; i++)
	{
		memcpy(YBuf, lumaAddr, width);
		YBuf += width;
		lumaAddr += ref_width;
	}

	CbBuf = (char *)(fieldBuf + pic_size);
	CrBuf = (char *)(fieldBuf + pic_size + (pic_size >> 2));
	for (i = 0; i < (height >> 1); i++)
	{
		for (j = 0; j < (width >> 1); j++)
		{
			CbBuf[j] = chromaAddr[(j * 2)];
			CrBuf[j] = chromaAddr[(j * 2) + 1];
		}
		CbBuf += (width >> 1);
		CrBuf += (width >> 1);
		chromaAddr += ref_width_c;
	}

	if (!complianceMode)
	{
		//Cache_inv(fieldBuf, (pic_size * 3)>>1, Cache_Type_ALL, TRUE);
		fwrite((void *)fieldBuf, sizeof(unsigned char),
		    ((pic_size * 3) >> 1), fout);
		//retval = (pic_size/pic_size) - 1;
	}

	return (retval);
}

/* ==========================================================================
*
*@func   outputDisplayField()
*
*@brief  Sub  Function to interleave 2 fields.
* In case of interlaced or PICAFF streams, the frame is stored as
* 2 separate field. This function is used to interleaved the 2 fields
* and form the frame.The interleaving is done in fieldBuf and is then
* written out
*
*@param  outArgs                  output arguments
*@param  xoff                     horizontal offset
*@param  yoff                     vertical offset
*
*@ret    none
*
* ============================================================================*/
static unsigned short outputDisplayField(IH264VDEC_OutArgs * outArgs,
    unsigned short xoff, unsigned short yoff, unsigned short ref_width,
    unsigned short ref_height, unsigned short width, unsigned short height,
    unsigned short pic_struct, unsigned short hrd, unsigned int fieldBuf,
    unsigned short bottom_field_flag, FILE * fout,
    unsigned int complianceMode)
{
	IVIDEO2_BufDesc *displayBufs =
	    (IVIDEO2_BufDesc *) & outArgs->viddec3OutArgs.displayBufs.
	    bufDesc[0];

  /*--------------------------------------------------------------------------------
   Here the actual frame data (w/out padding), is extracted and dumped.
   Also UV is de-interleaved.
   Top and Bottom fields are interleaved and written
  --------------------------------------------------------------------------------*/
	char *lumaAddr1, *lumaAddr2, *chromaAddr1, *chromaAddr2;
	unsigned short retval = 0;
	char *CbBuf, *CrBuf, *YBuf;
	unsigned int pic_size, i, j;

	unsigned int fieldSizeY;
	unsigned int fieldSizeCbCr;
	unsigned short ref_width_c = ref_width;

	if (displayBufs->planeDesc[1].memType)
		ref_width_c *= displayBufs->planeDesc[1].memType;


	fieldSizeY = (ref_width * ref_height) >> 1;
	fieldSizeCbCr = (ref_width_c * ref_height) >> 2;


	pic_size = width * height;

	lumaAddr1 =
	    (char *)((unsigned int)displayBufs->planeDesc[0].buf +
	    (yoff * ref_width) + xoff);
	chromaAddr1 =
	    (char *)((unsigned int)displayBufs->planeDesc[1].buf +
	    ((yoff >> 1) * ref_width_c) + xoff);

	lumaAddr2 = lumaAddr1 + fieldSizeY;
	chromaAddr2 = chromaAddr1 + fieldSizeCbCr;

	YBuf = (char *)fieldBuf;
	for (i = 0; i < (height >> 1); i++)
	{
		memcpy(YBuf, lumaAddr1, width);
		YBuf += width;
		lumaAddr1 += ref_width;
		memcpy(YBuf, lumaAddr2, width);
		YBuf += width;
		lumaAddr2 += ref_width;
	}

	CbBuf = (char *)(fieldBuf + pic_size);
	CrBuf = (char *)(fieldBuf + pic_size + (pic_size >> 2));
	for (i = 0; i < (height >> 2); i++)
	{
		for (j = 0; j < (width >> 1); j++)
		{
			CbBuf[j] = chromaAddr1[(j * 2)];
			CrBuf[j] = chromaAddr1[(j * 2) + 1];
		}
		CbBuf += (width >> 1);
		CrBuf += (width >> 1);
		chromaAddr1 += ref_width_c;

		for (j = 0; j < (width >> 1); j++)
		{
			CbBuf[j] = chromaAddr2[(j * 2)];
			CrBuf[j] = chromaAddr2[(j * 2) + 1];
		}
		CbBuf += (width >> 1);
		CrBuf += (width >> 1);
		chromaAddr2 += ref_width_c;
	}
	if (!complianceMode)
		fwrite((void *)fieldBuf, sizeof(unsigned char),
		    ((pic_size * 3) >> 1), fout);

	return (retval);
}

/* ==========================================================================
*
*@func   TestApp_WriteOutputData()
*
*@brief  Application function to write output data to a file
*
*@param  fOutFile           output file ptr
*
*@param  outArgs            OutArgs from process
*
*@param  fieldBuf            Scratch buffer for interleaving the data
*
*@ret    none
*
* ============================================================================
*/
static XDAS_Void TestApp_WriteOutputData
    (FILE * fOutFile,
    IH264VDEC_OutArgs * outArgs,
    unsigned int fieldBuf, unsigned int complianceMode)
{
	unsigned int width, height, paddedheight, hrd;
	unsigned int xOffset, yOffset;

  /*-------------------------------------------------------------------------*/
	/*  Pointer to Display buffer structure                                    */
  /*-------------------------------------------------------------------------*/
	IVIDEO2_BufDesc *displayBufs =
	    (IVIDEO2_BufDesc *) & outArgs->viddec3OutArgs.displayBufs.
	    bufDesc[0];

	paddedheight =
	    displayBufs->imageRegion.bottomRight.y +
	    displayBufs->imageRegion.topLeft.y;

	height =
	    displayBufs->activeFrameRegion.bottomRight.y -
	    displayBufs->activeFrameRegion.topLeft.y;
	width =
	    displayBufs->activeFrameRegion.bottomRight.x -
	    displayBufs->activeFrameRegion.topLeft.x;

	hrd = (displayBufs->repeatFrame >= 0) ? 1 : 0;

	xOffset = displayBufs->activeFrameRegion.topLeft.x;
	yOffset = displayBufs->activeFrameRegion.topLeft.y;

    /*-------------------------------------------------------------
     contentType indicates if the frame was stored as field or not.
     contentType :0 , frame storage
     otherwise, it is field storage
    ---------------------------------------------------------------*/
	if (displayBufs->contentType == IVIDEO_PROGRESSIVE)
	{
		//Progressive frame
		outputDisplayFrame(outArgs,
		    (unsigned short)xOffset,
		    (unsigned short)yOffset,
		    displayBufs->imagePitch,
		    paddedheight,
		    (unsigned short)width,
		    (unsigned short)height,
		    displayBufs->repeatFrame,
		    hrd, fieldBuf, fOutFile, complianceMode);

	} else
	{
	/*---------------------------------------------------------------
         The frame is stored as 2 fields. So top and bottom fields have to
          be interleaved. The interleaving is done in fieldBuf and is then
          written out
        -----------------------------------------------------------------*/
		/* Double the paddedheight and height dimensions for two fields */
		paddedheight <<= 1;
		height <<= 1;
	/*---------------------------------------------------------------
         Write Top and Bottom field
         -----------------------------------------------------------------*/
		outputDisplayField(outArgs,
		    (unsigned short)xOffset,
		    (unsigned short)yOffset,
		    (unsigned short)displayBufs->imagePitch,
		    paddedheight,
		    (unsigned short)width,
		    (unsigned short)height,
		    displayBufs->repeatFrame, hrd, fieldBuf, 0, fOutFile,
		    complianceMode);

	}
}
#endif

/* ========================================================================== */
/**
* H264DEC_Calculate_TotalRefFrames() : Calculates the total reference frames
* required by the codec without display delay
*
*
* @param pAppData   : Pointer to the application data
*
*  @return
*  OMX_U32 = Number of reference frames required
*
*  0 = Failed
*
*/
/* ========================================================================== */
static OMX_U32 H264DEC_Calculate_TotalRefFrames(H264_Client * pAppData)
{
	OMX_U32 ref_frames = 0;
	OMX_U32 spec_computation;

	if (pAppData->nWidth > 1920 || pAppData->nHeight > 1088)
	{
		return 0;
	}

	/* 12288 is the value for Profile 4.1 */
	spec_computation =
	    ((1024 * 12288) / ((pAppData->nWidth / 16) * (pAppData->nHeight /
		    16) * 384));

	ref_frames = (spec_computation > 16) ? 16 : (spec_computation / 1);

	return ref_frames;

}

/* ========================================================================== */
/**
* H264DEC_AllocateResources() : Allocates the resources required for H264
* Decoder.
*
*
* @param pAppData   : Pointer to the application data
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static OMX_ERRORTYPE H264DEC_AllocateResources(H264_Client * pAppData)
{
	OMX_U32 retval;
	OMX_ERRORTYPE eError = OMX_ErrorNone;

	pAppData->pCb =
	    (OMX_CALLBACKTYPE *) TIMM_OSAL_Malloc(sizeof(OMX_CALLBACKTYPE),
	    TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
	if (!pAppData->pCb)
	{
		eError = OMX_ErrorInsufficientResources;
		goto EXIT;
	}

	pAppData->pInPortDef =
	    (OMX_PARAM_PORTDEFINITIONTYPE *)
	    TIMM_OSAL_Malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE),
	    TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
	if (!pAppData->pInPortDef)
	{
		eError = OMX_ErrorInsufficientResources;
		goto EXIT;
	}

	pAppData->pOutPortDef =
	    (OMX_PARAM_PORTDEFINITIONTYPE *)
	    TIMM_OSAL_Malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE),
	    TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
	if (!pAppData->pOutPortDef)
	{
		eError = OMX_ErrorInsufficientResources;
		goto EXIT;
	}

	if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC)
	{
		pAppData->pH264 =
		    (OMX_VIDEO_PARAM_AVCTYPE *)
		    TIMM_OSAL_Malloc(sizeof(OMX_VIDEO_PARAM_AVCTYPE),
		    TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
		if (!pAppData->pH264)
		{
			eError = OMX_ErrorInsufficientResources;
			goto EXIT;
		}
	} else
	{
		TIMM_OSAL_Error("Invalid compression format value.");
		eError = OMX_ErrorUnsupportedSetting;
		goto EXIT;
	}

	/* Create a pipes for Input and Output Buffers.. used to queue data from the callback. */
	retval =
	    TIMM_OSAL_CreatePipe(&(pAppData->IpBuf_Pipe),
	    sizeof(OMX_BUFFERHEADERTYPE *) * NUM_OF_IN_BUFFERS,
	    sizeof(OMX_BUFFERHEADERTYPE *), OMX_TRUE);
	if (retval != 0)
	{
		TIMM_OSAL_Error("Error: TIMM_OSAL_CreatePipe failed to open");
		eError = OMX_ErrorContentPipeCreationFailed;
		goto EXIT;
	}
#ifdef __TASK_INPUT__
	retval =
	    TIMM_OSAL_CreatePipe(&(pAppData->IpFileBuf_Pipe),
	    sizeof(OMX_BUFFERHEADERTYPE *) * NUM_OF_IN_BUFFERS,
	    sizeof(OMX_BUFFERHEADERTYPE *), OMX_TRUE);
	if (retval != 0)
	{
		TIMM_OSAL_Error("Error: TIMM_OSAL_CreatePipe failed to open");
		eError = OMX_ErrorContentPipeCreationFailed;
		goto EXIT;
	}
	retval =
	    TIMM_OSAL_CreatePipe(&(pAppData->OpFileBuf_Pipe),
	    sizeof(OMX_BUFFERHEADERTYPE *) * NUM_OF_IN_BUFFERS,
	    sizeof(OMX_BUFFERHEADERTYPE *), OMX_TRUE);
	if (retval != 0)
	{
		TIMM_OSAL_Error("Error: TIMM_OSAL_CreatePipe failed to open");
		eError = OMX_ErrorContentPipeCreationFailed;
		goto EXIT;
	}

	/* Create input data read thread */
	retval = TIMM_OSAL_CreateTask(
	    (void *)&pAppData->pThreadFileIn,
	    (void *)H264_InputDataRead,
	    0,
	    pAppData,
	    (10 * 1024), -1, (signed char *)"H264_READ_DATA_IN_TSK");

	if (TIMM_OSAL_ERR_NONE != retval)
	{
		TIMM_OSAL_Error
		    ("Unable to start H264 video decode input data read thread\n");
	}
#endif

#ifdef OMX_TILERTEST
	TIMM_OSAL_Info("Enter the number of output buffers to allocate ");
	scanf("%d", &num_out_buffers);
#else
	num_out_buffers = H264DEC_Calculate_TotalRefFrames(pAppData) + 4;
	if (num_out_buffers == 0)
	{
		TIMM_OSAL_Error("Error computing Total reference frames");
		eError = OMX_ErrorInsufficientResources;
		goto EXIT;
	}
	/* :WORKAROUND: there is not enough memory to allocate 8 1080p buffers */
#ifdef __MULTIPLE_INPUT_BUF__
	num_out_buffers = NUM_OF_OUT_BUFFERS;
#endif
#endif

	retval =
	    TIMM_OSAL_CreatePipe(&(pAppData->OpBuf_Pipe),
	    sizeof(OMX_BUFFERHEADERTYPE *) * num_out_buffers,
	    sizeof(OMX_BUFFERHEADERTYPE *), OMX_TRUE);
	if (retval != 0)
	{
		TIMM_OSAL_Error("Error: TIMM_OSAL_CreatePipe failed to open");
		eError = OMX_ErrorContentPipeCreationFailed;
		goto EXIT;
	}

      EXIT:
	return eError;
}

/* ========================================================================== */
/**
* H264DEC_FreeResources() : Free the resources allocated for H264
* Decoder.
*
*
* @param pAppData   : Pointer to the application data
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static void H264DEC_FreeResources(H264_Client * pAppData)
{
	if (pAppData->pCb)
		TIMM_OSAL_Free(pAppData->pCb);

	if (pAppData->pInPortDef)
		TIMM_OSAL_Free(pAppData->pInPortDef);

	if (pAppData->pOutPortDef)
		TIMM_OSAL_Free(pAppData->pOutPortDef);

	if (pAppData->pH264)
		TIMM_OSAL_Free(pAppData->pH264);

	if (pAppData->IpBuf_Pipe)
		TIMM_OSAL_DeletePipe(pAppData->IpBuf_Pipe);

	if (pAppData->OpBuf_Pipe)
		TIMM_OSAL_DeletePipe(pAppData->OpBuf_Pipe);

#ifdef __TASK_INPUT__
	if (pAppData->IpFileBuf_Pipe)
		TIMM_OSAL_DeletePipe(pAppData->IpFileBuf_Pipe);

	if (pAppData->OpFileBuf_Pipe)
		TIMM_OSAL_DeletePipe(pAppData->OpFileBuf_Pipe);

	if (pAppData->pThreadFileIn)
		TIMM_OSAL_DeleteTask(pAppData->pThreadFileIn);

#endif

	return;
}

/* ========================================================================== */
/**
* H264_GetDecoderErrorString() : Function to map the OMX error enum to string
*
* @param error   : OMX Error type
*
*  @return
*  String conversion of the OMX_ERRORTYPE
*
*/
/* ========================================================================== */
static OMX_STRING H264_GetDecoderErrorString(OMX_ERRORTYPE error)
{
	OMX_STRING errorString;

	switch (error)
	{
	case OMX_ErrorNone:
		errorString = "OMX_ErrorNone";
		break;
	case OMX_ErrorInsufficientResources:
		errorString = "OMX_ErrorInsufficientResources";
		break;
	case OMX_ErrorUndefined:
		errorString = "OMX_ErrorUndefined";
		break;
	case OMX_ErrorInvalidComponentName:
		errorString = "OMX_ErrorInvalidComponentName";
		break;
	case OMX_ErrorComponentNotFound:
		errorString = "OMX_ErrorComponentNotFound";
		break;
	case OMX_ErrorInvalidComponent:
		errorString = "OMX_ErrorInvalidComponent";
		break;
	case OMX_ErrorBadParameter:
		errorString = "OMX_ErrorBadParameter";
		break;
	case OMX_ErrorNotImplemented:
		errorString = "OMX_ErrorNotImplemented";
		break;
	case OMX_ErrorUnderflow:
		errorString = "OMX_ErrorUnderflow";
		break;
	case OMX_ErrorOverflow:
		errorString = "OMX_ErrorOverflow";
		break;
	case OMX_ErrorHardware:
		errorString = "OMX_ErrorHardware";
		break;
	case OMX_ErrorInvalidState:
		errorString = "OMX_ErrorInvalidState";
		break;
	case OMX_ErrorStreamCorrupt:
		errorString = "OMX_ErrorStreamCorrupt";
		break;
	case OMX_ErrorPortsNotCompatible:
		errorString = "OMX_ErrorPortsNotCompatible";
		break;
	case OMX_ErrorResourcesLost:
		errorString = "OMX_ErrorResourcesLost";
		break;
	case OMX_ErrorNoMore:
		errorString = "OMX_ErrorNoMore";
		break;
	case OMX_ErrorVersionMismatch:
		errorString = "OMX_ErrorVersionMismatch";
		break;
	case OMX_ErrorNotReady:
		errorString = "OMX_ErrorNotReady";
		break;
	case OMX_ErrorTimeout:
		errorString = "OMX_ErrorTimeout";
		break;
	default:
		errorString = "<unknown>";
	}

	return errorString;
}

/* ========================================================================== */
/**
* H264DEC_FillData() : Function to fill the input buffer with data.
* This function currently reads the entire file into one single memory chunk.
* May require modification to support bigger file sizes.
*
*
* @param pAppData   : Pointer to the application data
* @param pBuf       : Pointer to the input buffer
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static OMX_U32 H264DEC_FillData(H264_Client * pAppData,
    OMX_BUFFERHEADERTYPE * pBuf)
{
	OMX_U32 nRead = 0;
	OMX_U32 framesizetoread = 0;

	if (!feof(pAppData->fIn))
	{
#ifndef __MULTIPLE_INPUT_BUF__
		fseek(pAppData->fIn, 0, SEEK_END);
		framesizetoread = ftell(pAppData->fIn);
		fseek(pAppData->fIn, 0, SEEK_SET);
		nRead =
		    fread(pBuf->pBuffer, sizeof(OMX_U8), framesizetoread,
		    pAppData->fIn);
		//nRead = framesizetoread;
#if 0
		if (nRead & 0x01)
		{
			pBuf->pBuffer[nRead++] = 0x00;
		}
#endif
		pBuf->nFilledLen = nRead;
		pBuf->nAllocLen = nRead;
		pBuf->nOffset = 0;

		pBuf->nFlags = OMX_BUFFERFLAG_EOS;
		pInputPtr = pBuf->pBuffer;

#ifdef H264D_DEBUG
		TIMM_OSAL_Info("%d bytes read into the input buffer",
		    (int)framesizetoread);
		TIMM_OSAL_Info("OMX Input buff = 0x%x", pBuf->pBuffer);
#endif
	} else
	{
		nRead = 0;
		pBuf->pBuffer = NULL;
	}
#else
		/* Read from I/P file and Fill data in i/p Buffer */
		nRead = fread(pBuf->pBuffer,
		    sizeof(char), frameSize[frameReadCount], pAppData->fIn);

		/* Update the buffer header with buffer filled length and alloc length */
		pBuf->nFilledLen = nRead;
		pBuf->nAllocLen = maxBufferSize;
		pBuf->nOffset = 0;

		/* Fetch the character from file to check whether EOS is reached */
		fgetc(pAppData->fIn);
		if (nRead < frameSize[frameReadCount] || feof(pAppData->fIn))
		{
			/* Udate the EOS flag in buffer header */
			//pBuf->nFlags = OMX_BUFFERFLAG_EOS;
			printf("%s: End of File reached %d %d\n", __func__,
			    frameReadCount, nRead);
		} else
		{
			pBuf->nFlags = 0;
			/* Seek back if EOS is not reached */
			fseek(pAppData->fIn, -1, SEEK_CUR);
		}
		/*printf("Read File %d %d %x\n",frameReadCount, nRead, pBuf); */
		frameReadCount =
		    (frameReadCount + 1) % BITSTREAM_BUF_MAX_SIZE;
	} else
	{
		/* Set the EOS flag and buffer size to 0 if EOS is reached - This should
		 * not occur !!! */
		nRead = 0;
		//pBuf->pBuffer = NULL;
		pBuf->nFlags = OMX_BUFFERFLAG_EOS;
		printf("End of File reached with pBuffer: %x\n",
		    (unsigned int)pBuf->pBuffer);
	}
#endif

	return nRead;
}

/* ========================================================================== */
/**
* H264DEC_SetParamPortDefinition() : Function to fill the port definition
* structures and call the Set_Parameter function on to the H264 Decoder
* Component
*
* @param pAppData   : Pointer to the application data
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static OMX_ERRORTYPE H264DEC_SetParamPortDefinition(H264_Client * pAppData)
{
	OMX_ERRORTYPE eError = OMX_ErrorUndefined;
	OMX_HANDLETYPE pHandle = pAppData->pHandle;
	OMX_PORT_PARAM_TYPE portInit;

	if (!pHandle)
	{
		eError = OMX_ErrorBadParameter;
		goto EXIT;
	}

	OMX_H264VD_TEST_INIT_STRUCT_PTR(&portInit, OMX_PORT_PARAM_TYPE);

	portInit.nPorts = 2;
	portInit.nStartPortNumber = 0;
	eError =
	    OMX_SetParameter(pAppData->pHandle, OMX_IndexParamVideoInit,
	    &portInit);
	if (eError != OMX_ErrorNone)
	{
		TIMM_OSAL_Error("OMX_SetParameter Resulted an Error %s ",
		    H264_GetDecoderErrorString(eError));
		goto EXIT;
	}

	/* Set the component's OMX_PARAM_PORTDEFINITIONTYPE structure (input) */

	OMX_H264VD_TEST_INIT_STRUCT_PTR(pAppData->pInPortDef,
	    OMX_PARAM_PORTDEFINITIONTYPE);

	pAppData->pInPortDef->nPortIndex = 0x0;	// Need to put input port index macro from spurthi's interface file
	pAppData->pInPortDef->eDir = OMX_DirInput;
	pAppData->pInPortDef->nBufferCountActual = NUM_OF_IN_BUFFERS;
	pAppData->pInPortDef->nBufferCountMin = 1;

	fseek(pAppData->fIn, 0, SEEK_END);
#ifdef __MULTIPLE_INPUT_BUF__
	pAppData->pInPortDef->nBufferSize = maxBufferSize;
#else
	pAppData->pInPortDef->nBufferSize = ftell(pAppData->fIn);	// + sizeof(OMX_U32);
#endif
	fseek(pAppData->fIn, 0, SEEK_SET);

	//pAppData->pInPortDef->nBufferSize = (pAppData->nWidth * pAppData->nHeight); // approx
	pAppData->pInPortDef->bEnabled = OMX_TRUE;
	pAppData->pInPortDef->bPopulated = OMX_FALSE;
	pAppData->pInPortDef->eDomain = OMX_PortDomainVideo;
	pAppData->pInPortDef->bBuffersContiguous = OMX_FALSE;
	pAppData->pInPortDef->nBufferAlignment = 0x0;	// Mention memory alignment if any


	/* OMX_VIDEO_PORTDEFINITION values for input port */
	pAppData->pInPortDef->format.video.cMIMEType = "H264";	//"YUV420";
	pAppData->pInPortDef->format.video.pNativeRender = NULL;
	pAppData->pInPortDef->format.video.nFrameWidth = pAppData->nWidth;	// It is not necessary to mention width and hight according to comment written in omx_video.h
	pAppData->pInPortDef->format.video.nFrameHeight = pAppData->nHeight;
	pAppData->pInPortDef->format.video.nStride = -1;	// h264Deocoder doesn't need this info
	pAppData->pInPortDef->format.video.nSliceHeight = 0;	// h264Deocoder doesn't need this info
	pAppData->pInPortDef->format.video.nBitrate = 10485760;	// pAppData->nBitRate; // Confirm this
	pAppData->pInPortDef->format.video.xFramerate = 30;	// h264Deocoder doesn't need this info
	pAppData->pInPortDef->format.video.eCompressionFormat =
	    OMX_VIDEO_CodingAVC;
	pAppData->pInPortDef->format.video.bFlagErrorConcealment = OMX_FALSE;
	pAppData->pInPortDef->format.video.eColorFormat =
	    pAppData->ColorFormat;

	eError =
	    OMX_SetParameter(pAppData->pHandle, OMX_IndexParamPortDefinition,
	    pAppData->pInPortDef);
	if (eError != OMX_ErrorNone)
	{
		TIMM_OSAL_Error("OMX_SetParameter Resulted an Error %s ",
		    H264_GetDecoderErrorString(eError));
		goto EXIT;
	}

	/* Set the component's OMX_PARAM_PORTDEFINITIONTYPE structure (output) */
	OMX_H264VD_TEST_INIT_STRUCT_PTR(pAppData->pOutPortDef,
	    OMX_PARAM_PORTDEFINITIONTYPE);

	pAppData->pOutPortDef->nPortIndex = 0x1;
	pAppData->pOutPortDef->eDir = OMX_DirOutput;
	pAppData->pOutPortDef->nBufferCountActual = num_out_buffers;
	pAppData->pOutPortDef->nBufferCountMin = 1;
	pAppData->pOutPortDef->nBufferSize = ((pAppData->nWidth + (2 * PADX) + 127) & 0xFFFFFF80) * ((pAppData->nHeight + (4 * PADY))) * 3 / 2;	//(65280 + 48000);//(pAppData->nWidth * pAppData->nHeight) * 5;//3 / 2; // Different if padding is included
	pAppData->pOutPortDef->bEnabled = OMX_TRUE;
	pAppData->pOutPortDef->bPopulated = OMX_FALSE;
	pAppData->pOutPortDef->eDomain = OMX_PortDomainVideo;
	pAppData->pOutPortDef->bBuffersContiguous = OMX_FALSE;
	pAppData->pOutPortDef->nBufferAlignment = 128;	// Mention memory alignment if any

	/* OMX_VIDEO_PORTDEFINITION values for output port */
	pAppData->pOutPortDef->format.video.cMIMEType = "H264";	//"YUV420";
	pAppData->pOutPortDef->format.video.pNativeRender = NULL;
	pAppData->pOutPortDef->format.video.nFrameWidth = ((pAppData->nWidth + (2 * PADX) + 127) & 0xFFFFFF80);	// 256; //pAppData->nWidth; //256
	pAppData->pOutPortDef->format.video.nFrameHeight = ((pAppData->nHeight + (4 * PADY)));	//pAppData->nHeight; //240
	pAppData->pOutPortDef->format.video.nStride = 0;
	pAppData->pOutPortDef->format.video.nSliceHeight = 0;
	pAppData->pOutPortDef->format.video.nBitrate = 10485760;
	pAppData->pOutPortDef->format.video.xFramerate = 30;
	pAppData->pOutPortDef->format.video.bFlagErrorConcealment = OMX_FALSE;
	pAppData->pOutPortDef->format.video.eCompressionFormat =
	    OMX_VIDEO_CodingAVC;
	pAppData->pOutPortDef->format.video.eColorFormat =
	    pAppData->ColorFormat;

	eError =
	    OMX_SetParameter(pHandle, OMX_IndexParamPortDefinition,
	    pAppData->pOutPortDef);
	if (eError != OMX_ErrorNone)
	{
		eError = OMX_ErrorBadParameter;
		goto EXIT;
	}

      EXIT:
	return eError;
}

/* ========================================================================== */
/**
* H264DEC_WaitForState() : This method will wait for the component to get
* to the correct state.
*
* @param pHandle        : Handle to the component
* @param DesiredState   : Desired
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static OMX_ERRORTYPE H264DEC_WaitForState(OMX_HANDLETYPE * pHandle,
    OMX_STATETYPE DesiredState)
{
	OMX_STATETYPE CurState = OMX_StateInvalid;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_U32 nCnt = 0;
	OMX_COMPONENTTYPE *pComponent = (OMX_COMPONENTTYPE *) pHandle;
	TIMM_OSAL_U32 uRequestedEvents, pRetrievedEvents;
	TIMM_OSAL_ERRORTYPE retval;

	/* Wait for an event */
	uRequestedEvents =
	    (H264_STATETRANSITION_COMPLETE | H264_DECODER_ERROR_EVENT);
	retval =
	    TIMM_OSAL_EventRetrieve(H264VD_CmdEvent, uRequestedEvents,
	    TIMM_OSAL_EVENT_OR_CONSUME, &pRetrievedEvents, TIMM_OSAL_SUSPEND);

	if (TIMM_OSAL_ERR_NONE != retval)
	{
		TIMM_OSAL_Debug("\nError in EventRetrieve !\n");
		eError = OMX_ErrorInsufficientResources;
		goto EXIT;
	}

	if (pRetrievedEvents & H264_DECODER_ERROR_EVENT)
	{
		eError = OMX_ErrorUndefined;	//TODO: Needs to be changed
	} else
	{
		eError = OMX_ErrorNone;
	}


#if 0
	eError = pComponent->GetState(pHandle, &CurState);
	while ((eError == OMX_ErrorNone) && (CurState != DesiredState))
	{
		Task_sleep(1);
		if (nCnt++ == 10)
		{
			TIMM_OSAL_Error("%d :: Still Waiting.... ", __LINE__);
		}
		eError = pComponent->GetState(pHandle, &CurState);
		if (CurState == OMX_StateInvalid)
		{
			eError = OMX_ErrorInvalidState;
		}
	}			//end while


	if (eError != OMX_ErrorNone)
		return eError;
#endif
      EXIT:
	return eError;
}

/* ========================================================================== */
/**
* H264DEC_ChangePortSettings() : This method will
*
* @param pHandle        : Handle to the component
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static OMX_ERRORTYPE H264DEC_ChangePortSettings(H264_Client * pAppData)
{

	TIMM_OSAL_ERRORTYPE retval;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_U32 i;

	eError =
	    OMX_SendCommand(pAppData->pHandle, OMX_CommandPortDisable,
	    pAppData->pOutPortDef->nPortIndex, NULL);
	if (eError != OMX_ErrorNone)
	{
		TIMM_OSAL_Error
		    ("Error from SendCommand OMX_CommandPortDisable ");
		goto EXIT;
	}

	for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++)
	{
		eError =
		    OMX_FreeBuffer(pAppData->pHandle,
		    pAppData->pOutPortDef->nPortIndex, pAppData->pOutBuff[i]);
		if (eError != OMX_ErrorNone)
		{
			TIMM_OSAL_Error("Error in OMX_FreeBuffer");
			goto EXIT;
		}
	}

	eError =
	    OMX_GetParameter(pAppData->pHandle, OMX_IndexParamPortDefinition,
	    pAppData->pOutPortDef);
	if (eError != OMX_ErrorNone)
	{
		TIMM_OSAL_Error("Error in OMX_GetParameter");
		goto EXIT;
	}

	eError =
	    OMX_SendCommand(pAppData->pHandle, OMX_CommandPortEnable,
	    pAppData->pOutPortDef->nPortIndex, NULL);
	if (eError != OMX_ErrorNone)
	{
		TIMM_OSAL_Error
		    ("Error in OMX_SendCommand:OMX_CommandPortEnable");
		goto EXIT;
	}

	retval = TIMM_OSAL_ClearPipe(pAppData->OpBuf_Pipe);
	if (retval != TIMM_OSAL_ERR_NONE)
	{
		TIMM_OSAL_Error("Error in clearing Output Pipe!");
		eError = OMX_ErrorNotReady;
		return eError;
	}

	for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++)
	{
		eError =
		    OMX_AllocateBuffer(pAppData->pHandle,
		    &pAppData->pOutBuff[i], pAppData->pOutPortDef->nPortIndex,
		    pAppData, pAppData->pOutPortDef->nBufferSize);
		if (eError != OMX_ErrorNone)
		{
			TIMM_OSAL_Error("Error in Allocating buffers");
			goto EXIT;
		}

		retval =
		    TIMM_OSAL_WriteToPipe(pAppData->OpBuf_Pipe,
		    &pAppData->pOutBuff[i], sizeof(pAppData->pOutBuff[i]),
		    TIMM_OSAL_SUSPEND);
		if (retval != TIMM_OSAL_ERR_NONE)
		{
			TIMM_OSAL_Error("Error in writing to out pipe!");
			eError = OMX_ErrorNotReady;
			return eError;
		}
	}

      EXIT:
	return eError;

}

/* ========================================================================== */
/**
* H264DEC_EventHandler() : This method is the event handler implementation to
* handle events from the OMX H264 Derived component
*
* @param hComponent        : Handle to the component
* @param ptrAppData        :
* @param eEvent            :
* @param nData1            :
* @param nData2            :
* @param pEventData        :
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static OMX_ERRORTYPE H264DEC_EventHandler(OMX_HANDLETYPE hComponent,
    OMX_PTR ptrAppData, OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2,
    OMX_PTR pEventData)
{
	H264_Client *pAppData = ptrAppData;
/*    OMX_STATETYPE state;*/
	TIMM_OSAL_ERRORTYPE retval;
	OMX_ERRORTYPE eError = OMX_ErrorNone;

//    eError = pComponent->GetState(hComponent, &state);

	switch (eEvent)
	{
	case OMX_EventCmdComplete:
		TIMM_OSAL_SemaphoreRelease(pSem_Events);
		retval =
		    TIMM_OSAL_EventSet(H264VD_CmdEvent,
		    H264_STATETRANSITION_COMPLETE, TIMM_OSAL_EVENT_OR);
		if (retval != TIMM_OSAL_ERR_NONE)
		{
			TIMM_OSAL_Debug("\nError in setting the event!\n");
			eError = OMX_ErrorNotReady;
			return eError;
		}
		break;
	case OMX_EventError:
		TIMM_OSAL_SemaphoreRelease(pSem_Events);
		retval =
		    TIMM_OSAL_EventSet(H264VD_CmdEvent,
		    H264_DECODER_ERROR_EVENT, TIMM_OSAL_EVENT_OR);
		if (retval != TIMM_OSAL_ERR_NONE)
		{
			TIMM_OSAL_Debug("\nError in setting the event!\n");
			eError = OMX_ErrorNotReady;
			return eError;
		}
		break;
	case OMX_EventMark:

		break;
	case OMX_EventPortSettingsChanged:

		/* In case of change in output buffer sizes re-allocate the buffers */
		eError = H264DEC_ChangePortSettings(pAppData);

		break;
	case OMX_EventBufferFlag:
		retval =
		    TIMM_OSAL_EventSet(myEvent, H264_DECODER_END_OF_STREAM,
		    TIMM_OSAL_EVENT_OR);
		if (retval != TIMM_OSAL_ERR_NONE)
		{
			TIMM_OSAL_Error("Error in setting the event!");
			eError = OMX_ErrorNotReady;
			return eError;
		}
		/* EOS here nData1-> port....  nData2->OMX_BUFFERFLAG_EOS */
		break;
	case OMX_EventResourcesAcquired:

		break;
	case OMX_EventComponentResumed:

		break;
	case OMX_EventDynamicResourcesAvailable:

		break;
	case OMX_EventPortFormatDetected:

		break;
	case OMX_EventMax:

		break;

	}			// end of switch

	return eError;
}

/* ========================================================================== */
/**
* H264DEC_FillBufferDone() : This method handles the fill buffer done event
* got from the derived component
*
* @param hComponent        : Handle to the component
* @param ptrAppData        : Pointer to the app data
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static OMX_ERRORTYPE H264DEC_FillBufferDone(OMX_HANDLETYPE hComponent,
    OMX_PTR ptrAppData, OMX_BUFFERHEADERTYPE * pBuffer)
{
	H264_Client *pAppData = ptrAppData;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	TIMM_OSAL_ERRORTYPE retval;

	{
#ifdef H264_LINUX_CLIENT
#ifdef SRCHANGES
		TIMM_OSAL_Debug("\npBuffer SR after FBD = %x\n",
		    pBuffer->pBuffer);
		pBuffer->pBuffer = SharedRegion_getPtr(pBuffer->pBuffer);
		TIMM_OSAL_Debug("\npBuffer after FBD = %x\n",
		    pBuffer->pBuffer);
#endif
#endif


		retval = TIMM_OSAL_WriteToPipe(pAppData->OpBuf_Pipe, &pBuffer, sizeof(pBuffer), TIMM_OSAL_SUSPEND);	//timeout - TIMM_OSAL_SUSPEND ??
		if (retval != TIMM_OSAL_ERR_NONE)
		{
			TIMM_OSAL_Error
			    ("Error writing to Output buffer Pipe!");
			eError = OMX_ErrorNotReady;
			return eError;
		}

		retval =
		    TIMM_OSAL_EventSet(myEvent, H264_DECODER_OUTPUT_READY,
		    TIMM_OSAL_EVENT_OR);
		if (retval != TIMM_OSAL_ERR_NONE)
		{
			TIMM_OSAL_Error("Error in setting the o/p event!");
			eError = OMX_ErrorNotReady;
			return eError;
		}
	}

	return eError;
}

/* ========================================================================== */
/**
* H264DEC_FillBufferDone() : This method handles the Empty buffer done event
* got from the derived component
*
* @param hComponent        : Handle to the component
* @param ptrAppData        : Pointer to the app data
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static OMX_ERRORTYPE H264DEC_EmptyBufferDone(OMX_HANDLETYPE hComponent,
    OMX_PTR ptrAppData, OMX_BUFFERHEADERTYPE * pBuffer)
{
	H264_Client *pAppData = ptrAppData;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	TIMM_OSAL_ERRORTYPE retval;

	retval =
	    TIMM_OSAL_WriteToPipe(pAppData->IpBuf_Pipe, &pBuffer,
	    sizeof(pBuffer), TIMM_OSAL_SUSPEND);
	if (retval != TIMM_OSAL_ERR_NONE)
	{
		TIMM_OSAL_Error("Error writing to Input buffer i/p Pipe!");
		eError = OMX_ErrorNotReady;
		return eError;
	}

	retval =
	    TIMM_OSAL_EventSet(myEvent, H264_DECODER_INPUT_READY,
	    TIMM_OSAL_EVENT_OR);
	if (retval != TIMM_OSAL_ERR_NONE)
	{
		TIMM_OSAL_Error("Error in setting the event!");
		eError = OMX_ErrorNotReady;
		return eError;
	}

	return eError;
}

#ifdef H264_LINUX_CLIENT

#define GOTO_EXIT_IF(_CONDITION,_ERROR) { \
            if((_CONDITION)) { \
                TIMM_OSAL_Error("%s : %s : %d :: ", __FILE__, __FUNCTION__,__LINE__);\
                TIMM_OSAL_Info(" Exiting because: %s \n", #_CONDITION); \
                eError = (_ERROR); \
                goto EXIT; \
            } \
}


/*
Copies 2D buffer to 1D buffer. All heights, widths etc. should be in bytes.
The function copies the lower no. of bytes i.e. if nSize1D < (nHeight2D * nWidth2D)
then nSize1D bytes is copied else (nHeight2D * nWidth2D) bytes is copied.
This function does not return any leftover no. of bytes, the calling function
needs to take care of leftover bytes on its own
*/
static OMX_ERRORTYPE OMXH264_Util_Memcpy_2Dto1D(OMX_PTR pDst1D,
    OMX_PTR pSrc2D, OMX_U32 nSize1D, OMX_U32 nHeight2D, OMX_U32 nWidth2D,
    OMX_U32 nStride2D)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_U32 retval = TIMM_OSAL_ERR_UNKNOWN;
	OMX_U8 *pInBuffer;
	OMX_U8 *pOutBuffer;
	OMX_U32 nSizeLeft, i;

	nSizeLeft = nSize1D;
	pInBuffer = (OMX_U8 *) pSrc2D;
	pOutBuffer = (OMX_U8 *) pDst1D;
	//The lower limit is copied. If nSize1D < H*W then 1Dsize is copied else H*W is copied
	TIMM_OSAL_Debug("\nStarting the 2D to 1D memcpy\n");
	for (i = 0; i < nHeight2D; i++)
	{
		//TIMM_OSAL_Debug("\nCopying row %d\n", i);
		if (nSizeLeft >= nWidth2D)
		{
			retval =
			    TIMM_OSAL_Memcpy(pOutBuffer, pInBuffer, nWidth2D);
			GOTO_EXIT_IF((retval != TIMM_OSAL_ERR_NONE),
			    OMX_ErrorUndefined);
		} else
		{
			retval =
			    TIMM_OSAL_Memcpy(pOutBuffer, pInBuffer,
			    nSizeLeft);
			GOTO_EXIT_IF((retval != TIMM_OSAL_ERR_NONE),
			    OMX_ErrorUndefined);
			break;
		}
		//TIMM_OSAL_Debug("\nCopied row %d, updating pointers\n", i);
		nSizeLeft -= nWidth2D;
		pInBuffer =
		    (OMX_U8 *) ((TIMM_OSAL_U32) pInBuffer + nStride2D);
		pOutBuffer =
		    (OMX_U8 *) ((TIMM_OSAL_U32) pOutBuffer + nWidth2D);
	}
	TIMM_OSAL_Debug("\nDone copying\n");
      EXIT:
	return eError;

}

#endif

/* ========================================================================== */
/**
* H264_Decoder_main() : This method is the function that needs to be invoked
* from any test framework. This will execute all the tests that are defined in
* testcase list. This will mimic the functioning of IL Client and test the OMX
* H264 Decoder implementation
*
* @param param1        : Currently used
* @param param2        : Currently used
*
*  @return
*  None
*/
/* ========================================================================== */
#ifndef H264_LINUX_CLIENT
void H264_Decoder_main(UArg param1, UArg param2)
#else
void main()
#endif
{
	H264_Client *pAppData = TIMM_OSAL_NULL;
	OMX_HANDLETYPE pHandle;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_BUFFERHEADERTYPE *pBufferIn = NULL;
	OMX_BUFFERHEADERTYPE *pBufferOut = NULL;
	TIMM_OSAL_ERRORTYPE tTIMMSemStatus = TIMM_OSAL_ERR_NONE;
	OMX_U32 i;
	OMX_U32 actualSize;
	OMX_U32 nRead;
	OMX_U32 test_index;
	OMX_CALLBACKTYPE appCallbacks;

	TIMM_OSAL_U32 uRequestedEvents, pRetrievedEvents;
	OMX_U32 ii = 0;
	OMX_U32 mem_count_start = 0;
	OMX_U32 mem_size_start = 0;
	OMX_U32 mem_count_end = 0;
	OMX_U32 mem_size_end = 0;
	OMX_U32 frames_decoded;
	int input;
	int main_input;
#ifndef H264_LINUX_CLIENT
	Memory_Stats stats;
#endif
	char configFile[] = "Decoder_input.cfg";
	FILE *fconfigFile = NULL;
	char line[256];
	char file_input[256];
	int file_parameter;

#ifdef H264_LINUX_CLIENT
	OMX_U32 setup;
	OMX_U8 *pTmpBuffer = NULL;
	OMX_U8 *p1DWriteBuffer = NULL;
#endif
	struct timeval h264d_t0, h264d_t1, h264d_t2, h264d_t3;

#ifdef H264_LINUX_CLIENT
	//#ifdef OMX_H264D_LINUX_TILERTEST
	MemAllocBlock *MemReqDescTiler = NULL;
	OMX_PTR TilerAddr = NULL;
	TIMM_OSAL_Debug
	    ("\nWait until RCM Server is created on other side. Press any key after that\n");
	sleep(3);
#endif

#ifndef H264_LINUX_CLIENT
	/* Set the Trace Group */
	nTraceGroup = TIMM_OSAL_GetTraceGrp();
#endif
#if 0
	TIMM_OSAL_SetTraceGrp(0x10);	/* 0x10 - indicates OMX H264 Video Decoder */

	nTraceGroup = TIMM_OSAL_GetTraceGrp();
#endif
	TIMM_OSAL_Debug("Trace Group = %x", nTraceGroup);

	appCallbacks.EventHandler = H264DEC_EventHandler;
	appCallbacks.EmptyBufferDone = H264DEC_EmptyBufferDone;
	appCallbacks.FillBufferDone = H264DEC_FillBufferDone;

	TIMM_OSAL_Info("Choose 0/1: ");
	TIMM_OSAL_Info("0: Normal test vectors ");
	TIMM_OSAL_Info("1: Additional test vectors ");
	//scanf("%d", &main_input);
	main_input = 1;


	if (main_input == 0)
	{
		for (test_index = 0; test_index < 7; test_index++)
		{
			TIMM_OSAL_Info("Decode test vector %d: %s",
			    (int)test_index,
			    H264_Decoder_TestCaseParams[test_index].InFile);
		}
		TIMM_OSAL_Info
		    ("Enter the test case number to execute (0-6, 100 to execute all testcases) : ");

#ifdef SCANF_WORKAROUND
		/* Already scanned earlier, just replace the value */
		input = param1;
#else
		scanf("%d", &input);
		//input = 5;
#endif
	} else
	{
		/* Check for the input file and open the input file */
		TIMM_OSAL_Info("Trying to open file Decoder_input.cfg ");
		fconfigFile = fopen(configFile, "r");

		if (!fconfigFile)
		{
			TIMM_OSAL_Error
			    ("%s - File not found. Place the file in the location of the base_image",
			    configFile);
			return;
		}

		/* HACK - TO ALTER THE FLOW OF EXECUTION BELOW */
		input = 100;
	}


	for (ii = 0; ii < 7; ii++)
	{

		if (input != 100)
		{
			if (input < 0 || input > 7)
			{
				TIMM_OSAL_Error
				    ("The input is not in range. Enter a valid value");
				return;
			} else
			{
				test_index = input;
			}
		} else
		{
			test_index = ii;
		}

		frames_decoded = 0;

		TIMM_OSAL_Info("");
		TIMM_OSAL_Info
		    ("-----------------------------------------------");
		TIMM_OSAL_Info("H264 Decoder Test case begin");

		if (main_input == 0)
		{
			TIMM_OSAL_Info("Decode test vector %s",
			    H264_Decoder_TestCaseParams[test_index].InFile);
		}
#ifndef H264_LINUX_CLIENT
		mem_count_start = TIMM_OSAL_GetMemCounter();
		mem_size_start = TIMM_OSAL_GetMemUsage();

		TIMM_OSAL_Info("Value from GetMemCounter = %d",
		    mem_count_start);
		TIMM_OSAL_Info("Value from GetMemUsage = %d", mem_size_start);

		Memory_getStats(NULL, &stats);
		TIMM_OSAL_Info("Total size = %d", stats.totalSize);
		TIMM_OSAL_Info("Total free size = %d", stats.totalFreeSize);
		TIMM_OSAL_Info("Largest Free size = %d",
		    stats.largestFreeSize);
		TIMM_OSAL_Info("");
#endif

		tTIMMSemStatus = TIMM_OSAL_EventCreate(&myEvent);
		if (TIMM_OSAL_ERR_NONE != tTIMMSemStatus)
		{
			TIMM_OSAL_Error("Error in creating event!");
			eError = OMX_ErrorInsufficientResources;
			goto EXIT;
		}
#ifdef __TASK_OUTPUT__
		tTIMMSemStatus = TIMM_OSAL_EventCreate(&myEventout);
		if (TIMM_OSAL_ERR_NONE != tTIMMSemStatus)
		{
			TIMM_OSAL_Error("Error in creating event!");
			eError = OMX_ErrorInsufficientResources;
			goto EXIT;
		}
#endif
#ifdef __TASK_INPUT__
		tTIMMSemStatus = TIMM_OSAL_EventCreate(&myEventIn);
		if (TIMM_OSAL_ERR_NONE != tTIMMSemStatus)
		{
			TIMM_OSAL_Error("Error in creating event!");
			eError = OMX_ErrorInsufficientResources;
			goto EXIT;
		}
#endif
		tTIMMSemStatus = TIMM_OSAL_EventCreate(&H264VD_CmdEvent);
		if (TIMM_OSAL_ERR_NONE != tTIMMSemStatus)
		{
			TIMM_OSAL_Debug("Error in creating event!\n");
			eError = OMX_ErrorInsufficientResources;
			goto EXIT;
		}

		pAppData =
		    (H264_Client *) TIMM_OSAL_Malloc(sizeof(H264_Client),
		    TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
		if (!pAppData)
		{
			TIMM_OSAL_Error("Error allocating pAppData!");
			eError = OMX_ErrorInsufficientResources;
			goto EXIT;
		}
		memset(pAppData, 0x00, sizeof(H264_Client));

		/* Open the file of data to be rendered.  */
		if (main_input == 0)
		{
			pAppData->fIn =
			    fopen(H264_Decoder_TestCaseParams[test_index].
			    InFile, "rb");
		} else
		{
			/* Get the file name from the file */
			if (fgets(line, 254, fconfigFile))
			{
				sscanf(line, "%s", file_input);
				TIMM_OSAL_Info("Input file = %s", file_input);
				pAppData->fIn = fopen(file_input, "rb");
			} else
			{
				TIMM_OSAL_Error
				    ("Could not get the input file. Check the file syntax");
				goto EXIT;
			}
		}

		if (pAppData->fIn == NULL)
		{
			TIMM_OSAL_Error
			    ("Error: failed to open the file <%s> for reading",
			    H264_Decoder_TestCaseParams[test_index].InFile);
			goto EXIT;
		}
#ifdef H264D_WRITEOUTPUT
		if (main_input == 0)
		{
			pAppData->fOut =
			    fopen(H264_Decoder_TestCaseParams[test_index].
			    OutFile, "wb");
		} else
		{
			/* Get the file name from the file */
			if (fgets(line, 254, fconfigFile))
			{
				sscanf(line, "%s", file_input);
				TIMM_OSAL_Info("Output file = %s",
				    file_input);
				pAppData->fOut = fopen(file_input, "wb");
			} else
			{
				TIMM_OSAL_Error
				    ("Could not get the output file. Check the file syntax");
				goto EXIT;
			}
		}

		if (pAppData->fOut == NULL)
		{
			TIMM_OSAL_Error
			    ("Error: failed to open the file <%s> for writing",
			    H264_Decoder_TestCaseParams[test_index].OutFile);
			goto EXIT;
		}
#endif

		pAppData->eCompressionFormat = OMX_VIDEO_CodingAVC;
		if (main_input == 0)
		{
			pAppData->nWidth =
			    H264_Decoder_TestCaseParams[test_index].width;
			pAppData->nHeight =
			    H264_Decoder_TestCaseParams[test_index].height;
			pAppData->ColorFormat =
			    H264_Decoder_TestCaseParams[test_index].
			    ColorFormat;
		} else
		{
			/* Get the Width */
			if (fgets(line, 254, fconfigFile))
			{
				sscanf(line, "%d", &file_parameter);
				TIMM_OSAL_Info("Width = %d", file_parameter);
				pAppData->nWidth = file_parameter;
			} else
			{
				TIMM_OSAL_Error
				    ("Could not get the width. Check the file syntax");
				goto EXIT;
			}

			/* Get the Height */
			if (fgets(line, 254, fconfigFile))
			{
				sscanf(line, "%d", &file_parameter);
				TIMM_OSAL_Info(" Height = %d",
				    file_parameter);
				pAppData->nHeight = file_parameter;
			} else
			{
				TIMM_OSAL_Error
				    ("Could not get the Height. Check the file syntax");
				goto EXIT;
			}

			/* Get the ColorFormat */
			if (fgets(line, 254, fconfigFile))
			{
				sscanf(line, "%d", &file_parameter);
				TIMM_OSAL_Info("ColorFormat = %d",
				    file_parameter);
				pAppData->ColorFormat = file_parameter;
			} else
			{
				TIMM_OSAL_Error
				    ("Could not get the ColorFormat. Check the file syntax");
				goto EXIT;
			}
		}

		eError = H264DEC_AllocateResources(pAppData);	// Allocate memory for the structure fields present in the pAppData(H264_Client)
		if (eError != OMX_ErrorNone)
		{
			TIMM_OSAL_Error
			    ("Error allocating resources in main!");
			eError = OMX_ErrorInsufficientResources;
			goto EXIT;
		}


		pAppData->eState = OMX_StateInvalid;
		*pAppData->pCb = appCallbacks;

		tTIMMSemStatus =
		    TIMM_OSAL_SemaphoreCreate(&pSem_InputPort, 0);
		if (tTIMMSemStatus != TIMM_OSAL_ERR_NONE)
		{
			TIMM_OSAL_Error("Semaphore Create failed!");
			goto EXIT;
		}
		tTIMMSemStatus =
		    TIMM_OSAL_SemaphoreCreate(&pSem_OutputPort, 0);
		if (tTIMMSemStatus != TIMM_OSAL_ERR_NONE)
		{
			TIMM_OSAL_Error("Semaphore Create failed!");
			goto EXIT;
		}
		tTIMMSemStatus = TIMM_OSAL_SemaphoreCreate(&pSem_Events, 0);
		if (tTIMMSemStatus != TIMM_OSAL_ERR_NONE)
		{
			TIMM_OSAL_Error("Semaphore Create failed!");
			goto EXIT;
		}


		eError = OMX_Init();

		/* Load the H264Decoder Component */
		eError =
		    OMX_GetHandle(&pHandle,
		    (OMX_STRING) "OMX.TI.DUCATI1.VIDEO.H264D"
		    /*StrH264Decoder */ , pAppData, pAppData->pCb);
		if ((eError != OMX_ErrorNone) || (pHandle == NULL))
		{
			TIMM_OSAL_Error("Error in Get Handle function : %s ",
			    H264_GetDecoderErrorString(eError));
			goto EXIT;
		}

		pAppData->pHandle = pHandle;
		pAppData->pComponent = (OMX_COMPONENTTYPE *) pHandle;

#ifdef __MULTIPLE_INPUT_BUF__
		/* Open the Frame Size File
		 * Read all the file in order to get the maximun input frame size for TILER allocation */
		fFrameDataFile = fopen(BITSTREAM_SIZE_FILE, "rb");
		if (NULL == fFrameDataFile)
		{
			printf("Error in opening frame size data file %s\n",
			    BITSTREAM_SIZE_FILE);
			goto EXIT;
		}

		/* output buffer size calculation based on input dimension speculation. If Frame Size txt
		 * is used, it is later set to max frame Size */
		maxBufferSize = 0;
		frameCount = 0;
		frameReadCount = 0;
		maxFrameCount = 0;

		/* Read the frame Size from the file */
		while (NULL != fgets((char *)input_char, 10, fFrameDataFile))
		{
			frameSize[frameCount] = atoi((char *)input_char);
			/* Initialising bufferSize to Max */
			if (frameSize[frameCount] > maxBufferSize)
				maxBufferSize = frameSize[frameCount];
			maxFrameCount++;
		}
		printf("\n Max size =  %d\n", maxBufferSize);
		frameReadCount = 0;
		/* Update InPortDef frame size */
		pAppData->pInPortDef->nBufferSize = maxBufferSize;

		OutputFrameCount = 0;
		InputFrameCount = 0;
		fclose(fFrameDataFile);

		fFrameDataFile = fopen(BITSTREAM_SIZE_FILE, "rb");
#endif

		H264DEC_SetParamPortDefinition(pAppData);

		/* OMX_SendCommand expecting OMX_StateIdle */
		eError =
		    OMX_SendCommand(pHandle, OMX_CommandStateSet,
		    OMX_StateIdle, NULL);
		if (eError != OMX_ErrorNone)
		{
			TIMM_OSAL_Error
			    ("Error in SendCommand()-OMX_StateIdle State set : %s ",
			    H264_GetDecoderErrorString(eError));
			goto EXIT;
		}
		TIMM_OSAL_Debug
		    ("\nCame back from send command without error\n");

#ifdef H264_LINUX_CLIENT
#ifdef OMX_H264D_LINUX_TILERTEST
		MemReqDescTiler =
		    (MemAllocBlock *) TIMM_OSAL_Malloc((sizeof(MemAllocBlock)
			* 2), TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
		if (MemReqDescTiler == TIMM_OSAL_NULL)
		{
			TIMM_OSAL_Debug
			    ("Error during memory allocation : OMX_ErrorInsufficientResources \n");
			goto EXIT;
		}
#endif
#endif

#ifdef H264_LINUX_CLIENT
		/*use buffer calls */
#ifdef OMX_H264D_USEBUFFERENABLED
		for (i = 0; i < pAppData->pInPortDef->nBufferCountActual; i++)
		{
			TIMM_OSAL_Debug("\nAllocating buffer no. %d\n", i);
#ifdef OMX_H264D_BUF_HEAP
			pTmpBuffer =
			    HeapBuf_alloc(heapHandle,
			    pAppData->pInPortDef->nBufferSize, 0);
#elif defined (OMX_H264D_LINUX_TILERTEST)
			MemReqDescTiler[0].pixelFormat = PIXEL_FMT_PAGE;
			MemReqDescTiler[0].dim.len =
			    pAppData->pInPortDef->nBufferSize;
			MemReqDescTiler[0].stride = 0;
			TIMM_OSAL_Debug
			    ("\nBefore tiler alloc for the UseBuffer \n");
			TilerAddr = MemMgr_Alloc(MemReqDescTiler, 1);
			TIMM_OSAL_Debug("\nTiler buffer allocated is %x\n",
			    TilerAddr);
			pTmpBuffer = (OMX_U8 *) TilerAddr;
#else
			pTmpBuffer =
			    TIMM_OSAL_Malloc(((pAppData->pInPortDef->
				    nBufferSize)), TIMM_OSAL_TRUE, 0,
			    TIMMOSAL_MEM_SEGMENT_EXT);
#endif

			if (pTmpBuffer == TIMM_OSAL_NULL)
			{
				TIMM_OSAL_Debug
				    ("OMX_ErrorInsufficientResources\n");
				goto EXIT;
			}
			TIMM_OSAL_Debug("\ncall to use buffer\n");
			eError =
			    OMX_UseBuffer(pHandle, &(pAppData->pInBuff[i]),
			    pAppData->pInPortDef->nPortIndex, pAppData,
			    (pAppData->pInPortDef->nBufferSize), pTmpBuffer);
			if (eError != OMX_ErrorNone)
			{
				TIMM_OSAL_Debug
				    ("Error %s:    after Usebuffer\n",
				    H264_GetDecoderErrorString(eError));
				goto EXIT;
			}
		}

		/* initialize v4l2 */
		if (NOT_I(omx_v4l2_init(pAppData->pOutPortDef->
			    nBufferCountActual,
			    pAppData->pOutPortDef->format.video.nFrameWidth,
			    pAppData->pOutPortDef->format.video.nFrameHeight,
			    pAppData->pOutPortDef->format.video.eColorFormat,
			    NULL), ==, 0))
			goto EXIT;

		A_I(v4l2_set_crop(36, 24, pAppData->nWidth,
			pAppData->nHeight), ==, 0);

		for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual;
		    i++)
		{
			TIMM_OSAL_Debug("\nAllocating buffer no. %d\n", i);
#ifdef OMX_H264D_BUF_HEAP
			pTmpBuffer =
			    HeapBuf_alloc(heapHandle,
			    pAppData->pOutPortDef->nBufferSize, 0);
#elif defined (OMX_H264D_LINUX_TILERTEST)
			pTmpBuffer = v4l2_get_start(i);
#else
			pTmpBuffer =
			    TIMM_OSAL_Malloc((pAppData->pOutPortDef->
				nBufferSize), TIMM_OSAL_TRUE, 0,
			    TIMMOSAL_MEM_SEGMENT_EXT);
#endif
			if (pTmpBuffer == TIMM_OSAL_NULL)
			{
				TIMM_OSAL_Debug
				    ("OMX_ErrorInsufficientResources\n");
				goto EXIT;
			}
			if (pTmpBuffer == TIMM_OSAL_NULL)
			{
				TIMM_OSAL_Debug
				    ("OMX_ErrorInsufficientResources\n");
				goto EXIT;
			}
			eError =
			    OMX_UseBuffer(pHandle, &(pAppData->pOutBuff[i]),
			    pAppData->pOutPortDef->nPortIndex, pAppData,
			    (pAppData->pOutPortDef->nBufferSize), pTmpBuffer);
			if (eError != OMX_ErrorNone)
			{
				TIMM_OSAL_Debug
				    ("Error %s:    after Usebuffer\n",
				    H264_GetDecoderErrorString(eError));
				goto EXIT;
			}
			/* save output buffer - display buffer correspondence */
			omx_v4l2_associate(i, pAppData->pOutBuff[i]);
		}

#endif
#endif

#if 0
		/* Allocate I/O Buffers */
		for (i = 0; i < pAppData->pInPortDef->nBufferCountActual; i++)
		{
			eError =
			    OMX_AllocateBuffer(pHandle, /*&pBufferIn */
			    &pAppData->pInBuff[i],
			    pAppData->pInPortDef->nPortIndex, pAppData,
			    pAppData->pInPortDef->nBufferSize);
		}
		for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual;
		    i++)
		{
			eError =
			    OMX_AllocateBuffer(pHandle, /*&pBufferOut */
			    &pAppData->pOutBuff[i],
			    pAppData->pOutPortDef->nPortIndex, pAppData,
			    pAppData->pOutPortDef->nBufferSize);
		}
#endif

		/* Wait for initialization to complete.. Wait for Idle stete of component  */
		eError = H264DEC_WaitForState(pHandle, OMX_StateIdle);
		if (eError != OMX_ErrorNone)
		{
			TIMM_OSAL_Error
			    ("Error %s:    WaitForState has timed out ",
			    H264_GetDecoderErrorString(eError));
			goto EXIT;
		}

		eError =
		    OMX_SendCommand(pHandle, OMX_CommandStateSet,
		    OMX_StateExecuting, NULL);
		if (eError != OMX_ErrorNone)
		{
			TIMM_OSAL_Error
			    ("Error from SendCommand-Executing State set :%s ",
			    H264_GetDecoderErrorString(eError));
			goto EXIT;
		}

		eError = H264DEC_WaitForState(pHandle, OMX_StateExecuting);
		if (eError != OMX_ErrorNone)
		{
			TIMM_OSAL_Error
			    ("Error %s:    WaitForState has timed out ",
			    H264_GetDecoderErrorString(eError));
			goto EXIT;
		}

		for (i = 0; i < pAppData->pInPortDef->nBufferCountActual; i++)
		{
#ifdef  __TASK_INPUT__
			printf("Input buffer [%d/%d]= %x %x\n", i,
			    pAppData->pInPortDef->nBufferCountActual,
			    &pAppData->pInBuff[i], pAppData->pInBuff[i]);
			/* Write inside File In pipe the avaliable buffer */
			tTIMMSemStatus =
			    TIMM_OSAL_WriteToPipe(pAppData->IpFileBuf_Pipe,
			    &pAppData->pInBuff[i],
			    sizeof(pAppData->pInBuff[i]), TIMM_OSAL_SUSPEND);
#else
			nRead =
			    H264DEC_FillData(pAppData, pAppData->pInBuff[i]);
			if (nRead <= 0)
			{
				break;
			}
			eError =
			    pAppData->pComponent->EmptyThisBuffer(pHandle,
			    pAppData->pInBuff[i]);
			printf("ETB %x %d %d %d %d\n", pAppData->pInBuff[i],
			    pAppData->pInBuff[i]->nFilledLen,
			    pAppData->pInBuff[i]->nAllocLen,
			    pAppData->pInBuff[i]->nOffset,
			    pAppData->pInBuff[i]->nFlags);

			if (eError != OMX_ErrorNone)
			{
				TIMM_OSAL_Error
				    ("Error from Empty this buffer : %s ",
				    H264_GetDecoderErrorString(eError));
				goto EXIT;
			}
#endif
		}

#ifdef  __TASK_INPUT__
		/* Send an Event in order to Wake-up the reading Thread */
		tTIMMSemStatus =
		    TIMM_OSAL_EventSet(myEventIn,
		    H264_DECODER_VIDEO_INPUT_BUF_FREE, TIMM_OSAL_EVENT_OR);
		/* sleep in order to start Input stream reading */
		sleep(3);

		/* Send ETB to Ducati */
		for (i = 0; i < pAppData->pInPortDef->nBufferCountActual - 4;
		    i++)
		{
			tTIMMSemStatus =
			    TIMM_OSAL_ReadFromPipe(pAppData->OpFileBuf_Pipe,
			    &pBufferIn, sizeof(pBufferIn), &actualSize,
			    TIMM_OSAL_SUSPEND);
			if (tTIMMSemStatus != TIMM_OSAL_ERR_NONE)
			{
				TIMM_OSAL_Debug
				    ("\nRead from pipe unsuccessful, going back to wait for event\n");
				break;
			}
			InputFrameCount++;
			eError =
			    pAppData->pComponent->EmptyThisBuffer(pHandle,
			    pBufferIn);
			printf("ETB %d %x\n", pBufferIn->nFilledLen,
			    pBufferIn);
		}
#endif
		/* Send FTB to Ducati */
		for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual - 0;
		    i++)
		{
			eError =
			    pAppData->pComponent->FillThisBuffer(pHandle,
			    pAppData->pOutBuff[i]);
			if (eError != OMX_ErrorNone)
			{
				TIMM_OSAL_Error
				    ("Error from Fill this buffer : %s ",
				    H264_GetDecoderErrorString(eError));
				goto EXIT;
			}
			printf("FTB [%d]=%x\n", i, pAppData->pOutBuff[i]);
		}

		TIMM_OSAL_Debug("\n Done with ETB/FTB, calling get state \n");
		eError = OMX_GetState(pHandle, &pAppData->eState);
		TIMM_OSAL_Debug("\n Returned from GetState, state = %d\n",
		    pAppData->eState);
		printf("\n Returned from GetState, state = %d\n",
		    pAppData->eState);

		/* Initialize the number of encoded frames to zero */
		pAppData->nEncodedFrms = 0;

		//while ((eError == OMX_ErrorNone) && (pAppData->eState != OMX_StateIdle)) {
		while ((eError == OMX_ErrorNone))
		{
			TIMM_OSAL_U32 numRemaining = 0;
			TIMM_OSAL_U32 numRemainingOut = 0;

			TIMM_OSAL_Debug
			    ("\n Wait for an event (input/output/error) ");
			uRequestedEvents =
			    (H264_DECODER_INPUT_READY |
			    H264_DECODER_OUTPUT_READY |
			    H264_DECODER_ERROR_EVENT |
			    H264_DECODER_END_OF_STREAM |
			    H264_DECODER_VIDEO_INPUT_BUF_READY);
			tTIMMSemStatus =
			    TIMM_OSAL_EventRetrieve(myEvent, uRequestedEvents,
			    TIMM_OSAL_EVENT_OR_CONSUME, &pRetrievedEvents,
			    TIMM_OSAL_SUSPEND);
			if (TIMM_OSAL_ERR_NONE != tTIMMSemStatus)
			{
				TIMM_OSAL_Error("Error in creating event!");
				eError = OMX_ErrorUndefined;
				goto EXIT;
			}

			TIMM_OSAL_Debug("\n Received event");
#if 0
			if (pRetrievedEvents & H264_DECODER_END_OF_STREAM)
			{
				//TODO: Check if you need to copy the pInputPtr to pBufferIn
				//pBufferIn->pBuffer = pInputPtr;

				TIMM_OSAL_Info("End of stream processed");
				TIMM_OSAL_Info("");
				break;
			}
#endif

			if (pRetrievedEvents &
			    H264_DECODER_VIDEO_INPUT_BUF_READY)
			{
				TIMM_OSAL_Debug
				    ("\n Input Buffer ready (Send ETB to Ducati)");

				TIMM_OSAL_GetPipeReadyMessageCount(pAppData->
				    OpFileBuf_Pipe, (void *)&numRemaining);
				while (numRemaining)
				{
					/* Read inside File Out pipe the avaliable buffers (Input Video Stream) */
					tTIMMSemStatus =
					    TIMM_OSAL_ReadFromPipe(pAppData->
					    OpFileBuf_Pipe, &pBufferIn,
					    sizeof(pBufferIn), &actualSize,
					    TIMM_OSAL_SUSPEND);
					if (tTIMMSemStatus !=
					    TIMM_OSAL_ERR_NONE)
					{
						TIMM_OSAL_Debug
						    ("\nRead from pipe unsuccessful, going back to wait for event\n");
						break;
					}

					if (pBufferIn == TIMM_OSAL_NULL)
					{
						TIMM_OSAL_Debug
						    ("\n Null received from pipe");
					} else
					{
						TIMM_OSAL_Debug
						    ("\n Header received from pipe = 0x%x",
						    pBufferIn);
					}
					/* Get buffer Index */
					for (i = 0;
					    i <
					    pAppData->pInPortDef->
					    nBufferCountActual; i++)
					{
						if (pBufferIn ==
						    pAppData->pInBuff[i])
							break;
					}
					if (i ==
					    pAppData->pInPortDef->
					    nBufferCountActual)
						printf("Error\n");

					if (tTIMMSemStatus !=
					    TIMM_OSAL_ERR_NONE)
					{
						printf
						    ("\nTASK INPUT OpFileBuf_Pipe read error %d %d\n",
						    InputFrameCount,
						    maxFrameCount);
						break;
					}
					pBufferIn->nTickCount = 0;
#ifdef H264_LINUX_CLIENT
					if (pBufferIn->pBuffer == NULL)
					{
						printf("\nBuffer is NULL\n");
						break;
					}
#endif
					eError =
					    pAppData->pComponent->
					    EmptyThisBuffer(pHandle,
					    pBufferIn);

					if (eError != OMX_ErrorNone)
					{
						TIMM_OSAL_Error
						    ("Error from Empty this buffer : %s ",
						    H264_GetDecoderErrorString
						    (eError));
						goto EXIT;
					}
					TIMM_OSAL_GetPipeReadyMessageCount
					    (pAppData->OpFileBuf_Pipe,
					    &numRemaining);
				}
			}

			if (pRetrievedEvents & H264_DECODER_OUTPUT_READY)
			{
				gettimeofday(&h264d_t3, NULL);

				TIMM_OSAL_GetPipeReadyMessageCount(pAppData->
				    OpBuf_Pipe, (void *)&numRemainingOut);

				while (numRemainingOut)
				{
					void *fieldBuf;
					/*read from the pipe */
					tTIMMSemStatus =
					    TIMM_OSAL_ReadFromPipe(pAppData->
					    OpBuf_Pipe, &pBufferOut,
					    sizeof(pBufferOut), &actualSize,
					    TIMM_OSAL_SUSPEND);
					if (tTIMMSemStatus !=
					    TIMM_OSAL_ERR_NONE)
					{
						TIMM_OSAL_Debug
						    ("\nRead from pipe unsuccessful, going back to wait for event\n");
						printf
						    ("\nRead from OpBuf_Pipe unsuccessful, going back to wait for event\n");
						break;
					}

					/* Make sure this is a format that V4l2 understands */
					for (i = 0;
					    i <
					    pAppData->pOutPortDef->
					    nBufferCountActual; i++)
					{
						if (pBufferOut ==
						    pAppData->pOutBuff[i])
							break;
					}
					if (i ==
					    pAppData->pOutPortDef->
					    nBufferCountActual)
					{
						printf("Error\n");
					} else
					{
						if (pAppData->pOutBuff[i] !=
						    NULL)
						{
							if (pAppData->
							    pOutBuff[i]->
							    nFlags &
							    OMX_BUFFERFLAG_EOS)
							{
								printf
								    ("EOS check %d\n",
								    pBufferOut->
								    nFlags);
								pBufferOut->
								    nFlags =
								    OMX_BUFFERFLAG_EOS;
							}
						}
					}

					if (pBufferOut != NULL)
					{
						if (pBufferOut->
						    nFlags &
						    OMX_BUFFERFLAG_EOS)
						{
							eError =
							    OMX_ErrorUndefined;
							printf
							    ("Out end of File\n");
							goto END_OF_FILE;
						}
					}
#ifdef __NO_DISPLAY__
//                    gettimeofday(&h264d_t0, NULL);
//                    printf("T=%d ms\n", ((h264d_t0.tv_sec&0xF)*1000+h264d_t0.tv_usec/1000));
					eError =
					    pAppData->pComponent->
					    FillThisBuffer(pHandle,
					    pBufferOut);
#else
					pBufferOut =
					    omx_v4l2_display_buffer
					    (pBufferOut);
#if 0
					printf("T=%d %d %d %d ms\n",
					    ((h264d_t0.tv_sec & 0xF) * 1000 +
						h264d_t0.tv_usec / 1000),
					    /*pBufferOut->nFlags */
					    pAppData->nEncodedFrms,
					    /*((h264d_t1.tv_sec-h264d_t0.tv_sec)*1000+(h264d_t1.tv_usec-h264d_t0.tv_usec)/1000), */
					    ((h264d_t2.tv_sec & 0xF) * 1000 +
						h264d_t2.tv_usec / 1000),
					    ((h264d_t3.tv_sec & 0xF) * 1000 +
						h264d_t3.tv_usec / 1000));
#endif
					if ((pAppData->nEncodedFrms % 100) ==
					    0)
					{
						float fps;
						fps =
						    (h264d_t3.tv_sec -
						    h264d_t1.tv_sec) +
						    0.000001 *
						    (h264d_t3.tv_usec -
						    h264d_t1.tv_usec);
						fps /= 100;
						if (fps)
							fps = 1 / fps;
						printf("%d fps = %4.2f\n",
						    pAppData->nEncodedFrms,
						    fps);
						gettimeofday(&h264d_t1, NULL);
					}


/*                while (pBufferOut) {*/
					if (pBufferOut)
					{
						pBufferOut->nFilledLen = 0;

						TIMM_OSAL_Debug
						    ("\n pAppData = 0x%x",
						    pAppData);
						TIMM_OSAL_Debug
						    ("\n pComponent = 0x%x",
						    pAppData->pComponent);
						TIMM_OSAL_Debug
						    ("\n Fill This buffer fn ptr = 0x%x",
						    pAppData->pComponent->
						    FillThisBuffer);
						TIMM_OSAL_Debug
						    ("\n pHandle = 0x%x",
						    pHandle);
						TIMM_OSAL_Debug
						    ("\n pBufferOut = 0x%x",
						    pBufferOut);

						eError =
						    pAppData->pComponent->
						    FillThisBuffer(pHandle,
						    pBufferOut);
						TIMM_OSAL_Debug
						    ("\nReturnrd from FTB call\n");
						if (eError != OMX_ErrorNone)
						{
							TIMM_OSAL_Error
							    ("Error from Fill this buffer : %s ",
							    H264_GetDecoderErrorString
							    (eError));
							goto EXIT;
						}
						/*printf("FTB %x\n",pBufferOut); */
/*                        pBufferOut = omx_v4l2_get_displayed_buffer(1);*/
					}
#endif
					pAppData->nEncodedFrms++;

					TIMM_OSAL_GetPipeReadyMessageCount
					    (pAppData->OpBuf_Pipe,
					    &numRemainingOut);
				}
			}

			if (pRetrievedEvents & H264_DECODER_INPUT_READY)
			{
				TIMM_OSAL_Debug
				    ("\n Input ready received (received EBD from Ducati)");
				gettimeofday(&h264d_t2, NULL);

				TIMM_OSAL_GetPipeReadyMessageCount(pAppData->
				    IpBuf_Pipe, (void *)&numRemaining);
				while (numRemaining)
				{
					/* Read from the pipe */
					tTIMMSemStatus =
					    TIMM_OSAL_ReadFromPipe(pAppData->
					    IpBuf_Pipe, &pBufferIn,
					    sizeof(pBufferIn), &actualSize,
					    TIMM_OSAL_SUSPEND);
					if (tTIMMSemStatus !=
					    TIMM_OSAL_ERR_NONE)
					{
						TIMM_OSAL_Debug
						    ("\nRead from pipe unsuccessful, going back to wait for event\n");
						printf
						    ("\nRead from IpBuf_Pipe unsuccessful, going back to wait for event\n");
						break;
					}

					if (pBufferIn == TIMM_OSAL_NULL)
					{
						TIMM_OSAL_Debug
						    ("\n Null received from pipe");
					} else
					{
						TIMM_OSAL_Debug
						    ("\n Header received from pipe = 0x%x",
						    pBufferIn);
					}
#ifdef __MULTIPLE_INPUT_BUF__
					gettimeofday(&h264d_t0, NULL);
#ifdef  __TASK_INPUT__
					/* Write inside File In pipe the avaliable buffer */
					for (i = 0;
					    i <
					    pAppData->pInPortDef->
					    nBufferCountActual; i++)
					{
						if (pBufferIn ==
						    pAppData->pInBuff[i])
							break;
					}
					if (i ==
					    pAppData->pInPortDef->
					    nBufferCountActual)
						printf("Error\n");

					InputFrameCount++;
#else
					nRead =
					    H264DEC_FillData(pAppData,
					    pBufferIn);
					if (nRead <= 0)
					{
						break;
					}
					pBufferIn->nOffset = 0;
					gettimeofday(&h264d_t1, NULL);
#endif
#else
					if (pBufferIn->nFilledLen > 3)
					{
						pBufferIn->nOffset =
						    (pBufferIn->nAllocLen -
						    pBufferIn->nFilledLen);
						//pBufferIn->nFilledLen  = pBufferIn->nAllocLen;
					} else
					{
						//pBufferIn->nFilledLen = pBufferIn->nAllocLen;
						pBufferIn->nOffset =
						    (pBufferIn->nAllocLen -
						    pBufferIn->nFilledLen);
						pBufferIn->nFilledLen = 0;
						pBufferIn->nFlags =
						    OMX_BUFFERFLAG_EOS;
					}
#endif

#ifdef  __TASK_INPUT__
					/*printf("EBD [%d]=%x %d \n", i, pBufferIn, pBufferIn->nFilledLen); */
					tTIMMSemStatus =
					    TIMM_OSAL_WriteToPipe(pAppData->
					    IpFileBuf_Pipe,
					    &pAppData->
					    pInBuff[i] /*&pBufferIn */ ,
					    sizeof(pBufferIn),
					    TIMM_OSAL_SUSPEND);
					tTIMMSemStatus =
					    TIMM_OSAL_EventSet(myEventIn,
					    H264_DECODER_VIDEO_INPUT_BUF_FREE,
					    TIMM_OSAL_EVENT_OR);
#endif
					if (eError != OMX_ErrorNone)
					{
						TIMM_OSAL_Error
						    ("Error from Empty this buffer : %s ",
						    H264_GetDecoderErrorString
						    (eError));
						goto EXIT;
					}
					TIMM_OSAL_GetPipeReadyMessageCount
					    (pAppData->IpBuf_Pipe,
					    &numRemaining);
				}
			}

			if (pRetrievedEvents & H264_DECODER_ERROR_EVENT)
			{
				eError = OMX_ErrorUndefined;
				printf("Decoder Error\n");
			}

			TIMM_OSAL_Debug("\n SUCCESSFULLY DECODED FRAME :  %d",
			    pAppData->nEncodedFrms);

			//eError = OMX_GetState(pHandle, &pAppData->eState);
		}

	      END_OF_FILE:

		printf("End 0001\n");
		eError =
		    OMX_SendCommand(pHandle, OMX_CommandStateSet,
		    OMX_StateIdle, NULL);
		if (eError != OMX_ErrorNone)
		{
			TIMM_OSAL_Error
			    ("Error from SendCommand-Idle State set : %s ",
			    H264_GetDecoderErrorString(eError));
			printf("Error from SendCommand-Idle State set : %s ",
			    H264_GetDecoderErrorString(eError));
			goto EXIT;
		}

		printf("End 0002\n");
		eError = H264DEC_WaitForState(pHandle, OMX_StateIdle);
		if (eError != OMX_ErrorNone)
		{
			TIMM_OSAL_Error
			    ("Error %s:    WaitForState has timed out ",
			    H264_GetDecoderErrorString(eError));
			printf("Error %s:    WaitForState has timed out ",
			    H264_GetDecoderErrorString(eError));
			goto EXIT;
		}


		printf("End 0003\n");
		eError =
		    OMX_SendCommand(pHandle, OMX_CommandStateSet,
		    OMX_StateLoaded, NULL);
		if (eError != OMX_ErrorNone)
		{
			TIMM_OSAL_Error
			    ("Error from SendCommand-Loaded State set : %s ",
			    H264_GetDecoderErrorString(eError));
			printf
			    ("Error from SendCommand-Loaded State set : %s ",
			    H264_GetDecoderErrorString(eError));
			goto EXIT;
		}

		printf("End 0004\n");
		for (i = 0; i < pAppData->pInPortDef->nBufferCountActual; i++)
		{
#ifdef H264_LINUX_CLIENT
#ifdef OMX_H264D_USEBUFFERENABLED
#ifdef OMX_H264E_BUF_HEAP
			HeapBuf_free(heapHandle,
			    pAppData->pInBuff[i]->pBuffer,
			    pAppData->pInBuff[i]->nAllocLen);
#elif defined (OMX_H264D_LINUX_TILERTEST)
			MemMgr_Free(pAppData->pInBuff[i]->pBuffer);
#else
			TIMM_OSAL_Free(pAppData->pInBuff[i]->pBuffer);
#endif
#endif
#endif
			eError =
			    OMX_FreeBuffer(pHandle,
			    pAppData->pInPortDef->nPortIndex,
			    pAppData->pInBuff[i]);
			if (eError != OMX_ErrorNone)
			{
				TIMM_OSAL_Error
				    ("Error in OMX_FreeBuffer : %s ",
				    H264_GetDecoderErrorString(eError));
				printf("Error in OMX_FreeBuffer : %s ",
				    H264_GetDecoderErrorString(eError));
				goto EXIT;
			}
		}

		printf("End 0005\n");
		for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual;
		    i++)
		{
#ifdef H264_LINUX_CLIENT
#ifdef OMX_H264D_USEBUFFERENABLED
#ifdef OMX_H264E_BUF_HEAP
			HeapBuf_free(heapHandle,
			    pAppData->pOutBuff[i]->pBuffer,
			    pAppData->pOutBuff[i]->nAllocLen);
#elif defined (OMX_H264D_LINUX_TILERTEST)
			MemMgr_Free(pAppData->pOutBuff[i]->pBuffer);
#else
			TIMM_OSAL_Free(pAppData->pOutBuff[i]->pBuffer);
#endif
#endif
#endif
			eError =
			    OMX_FreeBuffer(pHandle,
			    pAppData->pOutPortDef->nPortIndex,
			    pAppData->pOutBuff[i]);
			if (eError != OMX_ErrorNone)
			{
				TIMM_OSAL_Error
				    ("Error in OMX_FreeBuffer : %s ",
				    H264_GetDecoderErrorString(eError));
				goto EXIT;
			}
		}

		printf("End 0006\n");
		eError = H264DEC_WaitForState(pHandle, OMX_StateLoaded);
		if (eError != OMX_ErrorNone)
		{
			TIMM_OSAL_Error
			    ("Error %s:    WaitForState has timed out ",
			    H264_GetDecoderErrorString(eError));
			goto EXIT;
		}

		printf("End 0007\n");
		/* UnLoad the Decoder Component */
		eError = OMX_FreeHandle(pHandle);
		if ((eError != OMX_ErrorNone))
		{
			TIMM_OSAL_Error("Error in Free Handle function : %s ",
			    H264_GetDecoderErrorString(eError));
			goto EXIT;
		}
		printf("\nFree handle done\n");
#ifdef H264_LINUX_CLIENT
#ifdef OMX_H264D_LINUX_TILERTEST
		if (MemReqDescTiler)
			TIMM_OSAL_Free(MemReqDescTiler);
#endif
#endif
		printf("\nDeleting semaphores\n");
		tTIMMSemStatus = TIMM_OSAL_SemaphoreDelete(pSem_InputPort);
		if (tTIMMSemStatus != TIMM_OSAL_ERR_NONE)
		{
			TIMM_OSAL_Error("Semaphore Delete failed!");
			goto EXIT;
		}
		tTIMMSemStatus = TIMM_OSAL_SemaphoreDelete(pSem_OutputPort);
		if (tTIMMSemStatus != TIMM_OSAL_ERR_NONE)
		{
			TIMM_OSAL_Error("Semaphore Delete failed!");
			goto EXIT;
		}
		tTIMMSemStatus = TIMM_OSAL_SemaphoreDelete(pSem_Events);
		if (tTIMMSemStatus != TIMM_OSAL_ERR_NONE)
		{
			TIMM_OSAL_Error("Semaphore Delete failed!");
			goto EXIT;
		}
		printf("\nSemaphores deleted\n");
		/* De-Initialize OMX Core */
		eError = OMX_Deinit();

	      EXIT:
		printf("\nExit\n");

		/* Think about moving to end of stream case in the loop but think about memory leaks
		   due to improper v4l2 termination */
		omx_v4l2_display_done();

		if (pAppData)
		{
			printf("\nClosing files\n");
			if (pAppData->fIn)
				fclose(pAppData->fIn);

#ifdef H264D_WRITEOUTPUT
			if (pAppData->fOut)
				fclose(pAppData->fOut);
#endif
			TIMM_OSAL_Debug("\nFreeing resources\n");
			H264DEC_FreeResources(pAppData);
			TIMM_OSAL_Debug("\nFreeing app data\n");
			TIMM_OSAL_Free(pAppData);
		}
		TIMM_OSAL_Debug("\nDeleting events\n");
		tTIMMSemStatus = TIMM_OSAL_EventDelete(myEvent);
		if (TIMM_OSAL_ERR_NONE != tTIMMSemStatus)
		{
			TIMM_OSAL_Error("Error in deleting event!");
			eError = OMX_ErrorInsufficientResources;
			goto EXIT;
		}
#ifdef __TASK_OUTPUT__
		tTIMMSemStatus = TIMM_OSAL_EventDelete(myEventOut);
		if (TIMM_OSAL_ERR_NONE != tTIMMSemStatus)
		{
			TIMM_OSAL_Error("Error in deleting event!");
			eError = OMX_ErrorInsufficientResources;
			goto EXIT;
		}
#endif
#ifdef __TASK_INPUT__
		tTIMMSemStatus = TIMM_OSAL_EventDelete(myEventIn);
		if (TIMM_OSAL_ERR_NONE != tTIMMSemStatus)
		{
			TIMM_OSAL_Error("Error in deleting event!");
			eError = OMX_ErrorInsufficientResources;
			goto EXIT;
		}
#endif
		tTIMMSemStatus = TIMM_OSAL_EventDelete(H264VD_CmdEvent);
		if (TIMM_OSAL_ERR_NONE != tTIMMSemStatus)
		{
			TIMM_OSAL_Debug("Error in creating event!\n");
			eError = OMX_ErrorInsufficientResources;
			goto EXIT;
		}

		TIMM_OSAL_Info("");
		TIMM_OSAL_Info("H264 Decoder Test End");

#ifndef H264_LINUX_CLIENT
		mem_count_end = TIMM_OSAL_GetMemCounter();
		mem_size_end = TIMM_OSAL_GetMemUsage();
		TIMM_OSAL_Info(" Value from GetMemCounter = %d",
		    mem_count_end);
		TIMM_OSAL_Info(" Value from GetMemUsage = %d", mem_size_end);
#endif

		if (mem_count_start != mem_count_end)
		{
			TIMM_OSAL_Error
			    ("Memory leak detected. Bytes lost = %d",
			    (mem_size_end - mem_size_start));
		}
		TIMM_OSAL_Info
		    ("-----------------------------------------------");
		TIMM_OSAL_Info("");

		if (input != 100)
			break;

		if (main_input == 1)
		{
			if (fconfigFile)
			{
				if (!feof(fconfigFile))
				{
					ii = 0;
				}
			}
			break;
		} else
		{
			continue;
		}

	}

	if (fconfigFile)
		fclose(fconfigFile);
}

#ifdef __TASK_INPUT__
/* ==========================================================================
*
*@func   H264_InputDataRead()
*
*@brief  Thread function in order to manage the read from the MMC card of the
*        input video stream. When Input video file is read up the the end the
*        Thread is stopped.
*            Read from IpFileBuf_Pipe the free buffer
*            Write to OpFileBuf_Pipe the buffer with input video frame
*            Wake up by event inside myEventIn
*
*@void   threadsArg               Thread args (Pointer on OMX Handle)
*
*@ret    none
*
* ============================================================================
*/
void H264_InputDataRead(void *threadsArg)
{
	unsigned int actualSize = 0, dataRead = 1;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	H264_Client *pAppData = NULL;
	OMX_BUFFERHEADERTYPE *pBufferIn = NULL;
	TIMM_OSAL_U32 uRequestedEvents, pRetrievedEvents;
	unsigned int numRemainingIn = 0;
	unsigned int dIndex;

	pthread_t thread_id;
	int policy;
	int rc = 0;
	struct sched_param param;

	thread_id = pthread_self();
	rc = pthread_getschedparam(thread_id, &policy, &param);
	if (rc != 0)
		printf("<Thread> 1 error %d\n", rc);
	printf("<Thread> %d %d\n", policy, param.sched_priority);
	param.sched_priority = 10;
	rc = pthread_setschedparam(thread_id, SCHED_RR /*policy */ , &param);
	if (rc != 0)
		printf("<Thread> 2 error %d\n", rc);

	rc = pthread_getschedparam(thread_id, &policy, &param);
	if (rc != 0)
		printf("<Thread> 3 error %d\n", rc);
	printf("<Thread> %d %d %d %d\n", policy, param.sched_priority,
	    sched_get_priority_min(policy), sched_get_priority_max(policy));

	pAppData = (H264_Client *) (threadsArg);

	printf("Entering <Thread > : <%s %x %x %d>\n", __func__, &pAppData,
	    pAppData, param.sched_priority);

	while ((OMX_ErrorNone == err) && (dataRead != 0))
	{
		/* Need to associate an event with the PIPE as the function
		 * TIMM_OSAL_ReadFromPipe() is not blocking */
		uRequestedEvents = H264_DECODER_VIDEO_INPUT_BUF_FREE;
		err = TIMM_OSAL_EventRetrieve(myEventIn, uRequestedEvents,
		    TIMM_OSAL_EVENT_OR_CONSUME, &pRetrievedEvents,
		    TIMM_OSAL_SUSPEND);

		if (TIMM_OSAL_ERR_NONE != err)
		{
			TIMM_OSAL_Error("Error in creating event!");
			err = OMX_ErrorUndefined;
		}

		/* Read the number of buffers available in pipe */
		TIMM_OSAL_GetPipeReadyMessageCount(pAppData->IpFileBuf_Pipe,
		    (void *)&numRemainingIn);
		while (numRemainingIn)
		{
			/* Read empty buffer pointer from the pipe */
			err =
			    TIMM_OSAL_ReadFromPipe(pAppData->IpFileBuf_Pipe,
			    &pBufferIn, sizeof(pBufferIn), &actualSize,
			    TIMM_OSAL_SUSPEND);
			if (err != TIMM_OSAL_ERR_NONE)
			{
				printf
				    ("\n<Thread>Read from IpFileBuf_Pipe unsuccessful, going back to wait for event\n");
				break;
			}
			for (dIndex = 0;
			    dIndex < pAppData->pInPortDef->nBufferCountActual;
			    dIndex++)
			{
				if (pBufferIn == pAppData->pInBuff[dIndex])
					break;
			}
			if (dIndex ==
			    pAppData->pInPortDef->nBufferCountActual)
				printf("Error\n");

			/* Read the frame Size from the frame size file */
			if (frameCount == 0)
			{
				int k;
				for (k = 0; k < BITSTREAM_BUF_MAX_SIZE; k++)
				{
					if (NULL != fgets((char *)input_char,
						10, fFrameDataFile))
					{
						frameSize[k] =
						    atoi((char *)input_char);
					} else
					{
						frameSize[k] = 0;
						break;
					}
				}
			}
			frameCount =
			    (frameCount + 1) % BITSTREAM_BUF_MAX_SIZE;

			dataRead = H264DEC_FillData(pAppData, pBufferIn);
			/*printf("<Thread : %d r %d [%d]=%x>\n", numRemainingIn, dataRead, dIndex, pBufferIn); */

			/* Send back message to the PIPE for Test App */
			err =
			    TIMM_OSAL_WriteToPipe(pAppData->OpFileBuf_Pipe,
			    &pAppData->pInBuff[dIndex] /*&pBufferIn */ ,
			    sizeof(pBufferIn), TIMM_OSAL_SUSPEND);
			err =
			    TIMM_OSAL_EventSet(myEvent,
			    H264_DECODER_VIDEO_INPUT_BUF_READY,
			    TIMM_OSAL_EVENT_OR);

			TIMM_OSAL_GetPipeReadyMessageCount(pAppData->
			    IpFileBuf_Pipe, &numRemainingIn);
			if (dataRead == 0)
			{
				/* To be sure to not send several EOS when need to overwrite the number of remaining message in the queue */
				/* TODO:
				 *   - Check if remaining messagea insde the queue nees to be read to avoid memory leak */
				numRemainingIn = 0;
			}
		}
	}
	printf("Leaving <Thread : <%s> %d\n", __func__, frameCount);
	fclose(fFrameDataFile);
}
#endif
