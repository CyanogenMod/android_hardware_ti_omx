/* =============================================================================
*             Texas Instruments OMAP(TM) Platform Software
*  (c) Copyright 2009 Texas Instruments Incorporated.  All Rights Reserved.
*
*  Use of this software is controlled by the terms and conditions found 
*  in the license agreement under which this software has been supplied.
* =========================================================================== */
/*!  
 *****************************************************************************
 * \file
 *	H264_ILClien_Testcases.c														 
 *
 * \brief  
 * This File contains functions the details of the input test streams to be
 * used for the verification of the H264 Video Decode component
 *	
 * \version 1.0
 *
 *****************************************************************************
 */

/****************************************************************
 * INCLUDE FILES
 ****************************************************************/
#include <H264_Decoder_ILClient.h>


/* ========================================================================== */
/** H264_TestCaseParams is the structure definition to define the input
* test vector properties as defined by the structure H264_TestCaseParamsType
*
* @param SR_Id          SR ID that this testcase validates
* @param InFile         Input file location
* @param OutFile        Output file location
* @param width          Width of the input stream
* @param height         Height of the input stream
* @param ColorFormat    Format of the input data
*/
/* ========================================================================== */
H264_Decoder_TestCaseParamsType H264_Decoder_TestCaseParams[] = 
{

/*SR Id*/	/* Input File */	/* Input FrameSize File*/	/* Output File */	/* Width */	/* Height*/ /* Color Format*/

{"SR10503",		"../patterns/input/test_bits_norc_1.264"	,"../patterns/output/test_bits_noRC_1.yuv"	,	176	,	144	,	OMX_COLOR_FormatYUV420Planar},
{"SR11301",		"../patterns/input/SVA_BA2_D.264"	,"../patterns/output/SVA_BA2_D.yuv"	,	176	,	144	,	OMX_COLOR_FormatYUV420Planar},
{"SR10714",		"../patterns/input/CI_MW_D.264"	,"../patterns/output/CI_MW_D.yuv"	,	176	,	144	,	OMX_COLOR_FormatYUV420Planar},
{"SR11966",		"../patterns/input/SVA_FM1_E.264"	,"../patterns/output/SVA_FM1_E.yuv"	,	176	,	144	,	OMX_COLOR_FormatYUV420Planar},
{"SR14062",		"../patterns/input/BA1_Sony_D.264"	,"../patterns/output/BA1_Sony_D.yuv"	,	176	,	144	,	OMX_COLOR_FormatYUV420Planar},
{"SR18838",		"../patterns/input/CVPCMNL1_SVA_C.264"	,"../patterns/output/CVPCMNL1_SVA_C.yuv"	,	352	,	288	,	OMX_COLOR_FormatYUV420Planar},
{"SR11603",		"../patterns/input/1088p25_pedestrian_area_BP01.264"	,"../patterns/output/1088p25_pedestrian_area_BP01.yuv"	,	1920	,	1088	,	OMX_COLOR_FormatYUV420Planar},
{"SR19620",		NULL	,	NULL	,	0	,	0	,	0}

};


