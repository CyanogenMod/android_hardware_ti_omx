/*=========================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 *  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied.
 * ==========================================================================*/
/**
 *  @file  omx_ti_common.h
 *
 *
 *  @path \OMAPSW_SysDev\multimedia\omx\khronos1_1\omx_core\inc
 *
 *  @rev 1.0
 */

/*==============================================================
 *! Revision History
 *! ============================
 *! 20-Dec-2008 x0052661@ti.com, initial version
 *================================================================*/

#ifndef _OMX_TI_COMMON_H_
#define _OMX_TI_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
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
#define OMX_BUFFERHEADERFLAG_MODIFIED 0x00000100

#define OMX_TI_BUFFERFLAG_READONLY 0x00000200

typedef struct OMX_CONFIG_CHANNELNAME {

	OMX_U32 nSize; /**< Size of the structure in bytes */
	OMX_VERSIONTYPE nVersion; /**< OMX specification version information */
	OMX_U32 nPortIndex; /**< Index of the port */
	OMX_STRING cChannelName; /**< Channel name  */

} OMX_CONFIG_CHANNELNAME;

typedef struct OMX_TI_PLATFORMPRIVATE
{
	OMX_U32 nSize;
	OMX_PTR pExtendedPlatformPrivate;
	OMX_BOOL bReadViaCPU;
	OMX_BOOL bWriteViaCPU;
	OMX_PTR pMetaDataBuffer;
	OMX_U32 nMetaDataSize;
	OMX_PTR pAuxBuf1;
	OMX_U32 pAuxBufSize1;
}OMX_TI_PLATFORMPRIVATE;

/*===============================================================*/
/** OMX_TI_PARAM_BUFFERPREANNOUNCE    : This parameter is used to enable/disable
 *                                      buffer pre announcement. Buffer pre
 *                                      announcement is enabled by default i.e.
 *                                      if buffer is being allocated by client 
 *                                      then the buffer ptrs will be made known 
 *                                      to the component at the time of 
 *                                      UseBuffer and these will not change in
 *                                      lifetime of the component. If pre
 *                                      announcement is disabled then new 
 *                                      buffers can be allocated by the client 
 *                                      at runtime and passed in ETB/FTB. This
 *                                      parameter is valid only in cases where 
 *                                      client allocates the buffer 
 *                                      (i.e. UseBuffer cases).
 *
 *  @ param nSize                     : Size of the structure.
 *  @ param nVersion                  : Version.
 *  @ param nPortIndex                : Port index on which the parameter will
 *                                      be applied.
 *  @ param bEnabled                  : Whether buffer pre announcement is 
 *                                      enabled or not. Set to TRUE (enabled)
 *                                      by default.
 */
/*===============================================================*/
typedef struct OMX_TI_PARAM_BUFFERPREANNOUNCE
{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bEnabled;
}OMX_TI_PARAM_BUFFERPREANNOUNCE;



/*===============================================================*/
/** OMX_TI_CONFIG_BUFFERREFCOUNTNOTIFYTYPE : This config is used to 
 *                                           enable/disable notification when
 *                                           reference count of a buffer changes
 *                                           This happens usually when buffers 
 *                                           are locked/unlocked by codecs. By
 *                                           DEFAULT all notifications are
 *                                           DISABLED.
 *
 *  @ param nSize                          : Size of the structure.
 *  @ param nVersion                       : Version.
 *  @ param nPortIndex                     : Port index on which the config will
 *                                           be applied.
 *  @param bNotifyOnIncrease              : Enable notification when reference 
 *                                           count is increased.
 *  @ param bNotifyOnDecrease              : Enable notification when reference 
 *                                           count is decreased.
 *  @ param nCountForNotification          : Count at which to trigger 
 *                                           notification. Count indicates no.
 *                                           of copies of the buffer in 
 *                                           circulation e.g.
 *                                           1 - Only the original buffer is in 
 *                                               circulation i.e. no buffers are
 *                                               currently locked.
 *                                           2 - There are two copies of the 
 *                                               buffer i.e. one original and 
 *                                               one copy which has been locked
 *                                               by the codec.
 *                                           And so on
 *                                           SPECIAL CASE
 *                                           0 - Indicates notify always 
 *                                               irrespective of count value.
 */
/*===============================================================*/
typedef struct OMX_TI_CONFIG_BUFFERREFCOUNTNOTIFYTYPE
{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bNotifyOnIncrease;
    OMX_BOOL bNotifyOnDecrease;
    OMX_U32 nCountForNotification;
}OMX_TI_CONFIG_BUFFERREFCOUNTNOTIFYTYPE;





/* OMX_TI_SEVERITYTYPE enumeration is used to indicate severity level of errors returned by TI OpenMax components. 
   Critcal      Requires reboot/reset DSP
   Severe       Have to unload components and free memory and try again
   Major        Can be handled without unloading the component
   Minor        Essentially informational 
*/
typedef enum OMX_TI_SEVERITYTYPE {
    OMX_TI_ErrorCritical=1,
    OMX_TI_ErrorSevere,
    OMX_TI_ErrorMajor,
    OMX_TI_ErrorMinor
} OMX_TI_SEVERITYTYPE;



/*******************************************************************
 * PRIVATE DECLARATIONS: defined here, used only here
 *******************************************************************/
/*----------          data declarations        ------------------- */
/*----------          function prototypes      ------------------- */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OMX_TI_COMMON_H_ */
