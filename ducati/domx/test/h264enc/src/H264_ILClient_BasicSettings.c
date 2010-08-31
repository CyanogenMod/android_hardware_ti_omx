/*
*   @file  H264_ILClien_BasicSettings.c
*   This file contains the structures and macros that are used in the Application which tests the OpenMAX component
*
*  @path \WTSD_DucatiMMSW\omx\khronos1.1\omx_h264_dec\test
*
*  @rev 1.0
*/

/****************************************************************
 * INCLUDE FILES
 ****************************************************************/
/**----- system and platform files ----------------------------**/

/**-------program files ----------------------------------------**/
#include "H264_ILClient.h"

H264E_TestCaseParams H264_TestCaseTable[1] = {
	{
		    0,
		    "input/foreman_p176x144_30fps_420pl_300fr_nv12.yuv",	/*input file */
		    "output/foreman_p176x144_20input_1output.264",	/*output file */
		    176,
		    144,
		    OMX_VIDEO_AVCProfileHigh,
		    OMX_VIDEO_AVCLevel41,
		    OMX_COLOR_FormatYUV420PackedSemiPlanar,
		    0,		/*progressive or interlace */
		    0,		/*interlace type */
		    OMX_FALSE,	/*bloopfillter */
		    OMX_FALSE,	/*bCABAC */
		    OMX_FALSE,	/*bFMO */
		    OMX_FALSE,	/*bconstIntrapred */
		    0,		/*slicegrps */
		    0,		/*slicegrpmap */
		    OMX_VIDEO_SLICEMODE_AVCDefault,	/*slice mode */
		    OMX_Video_ControlRateDisable,	/*rate cntrl */
		    OMX_Video_Transform_Block_Size_4x4,	/*transform block size */
		    OMX_Video_Enc_User_Defined,	/*Encoder preset */
		    OMX_Video_RC_User_Defined,	/*RC Preset */
		    OMX_Video_BitStreamFormatByte,	/*Bit Stream Select */
		    1,		/*max interframe interval */
		    0,		/*bit enable_advanced */
		    0,		/*bit enable_dynamic */
		    1,		/*NumInputBuf */
		    1,		/*NumOutputBuf */
		    OMX_TRUE,	/*flag_Input_allocate buffer */
		    OMX_TRUE,	/*flag_Output_allocate buffer */
		    PARTIAL_RECORD,	/*test type */
		    10		/*stop frame number in case of partial encode */
	    }
};
