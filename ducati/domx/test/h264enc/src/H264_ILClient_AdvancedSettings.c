/*
*   @file  H264_ILClien_AdvancedSettings.c
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

H264E_TestCaseParamsAdvanced H264_TestCaseAdvTable[1] = {
	{
		    {
				0x2180,
				0x2040,
				0,
				0x2040,
			0}
		    ,
		    {
				0,
				0,
				0,
				0,
				0,
				{0, 0}
			}
		    ,
		    {
				1,
			1}
		    ,
		    {
				0,
				0,
				0,
				0,
			0}
		    ,
		    {
				1,
				1,
			1}
		    ,
		    {
				0x1,
				0x1,
				0x1,
				0x1,
			1}
		    ,
		    {
				0,
				0,
				0,
			0}
	    }
};
