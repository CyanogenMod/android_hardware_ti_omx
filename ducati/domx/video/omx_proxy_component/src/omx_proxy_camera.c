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
 *  @file  omx_proxy_camera.c
 *         This file contains methods that provides the functionality for
 *         the OpenMAX1.1 DOMX Framework Tunnel Proxy component.
 ******************************************************************************
 This is the proxy specific wrapper that passes the component name to the
 generic proxy init() The proxy wrapper also does some runtime/static time
 config on per proxy basis This is a thin wrapper that is called when
 componentiit() of the proxy is called  static OMX_ERRORTYPE PROXY_Wrapper_init
 (OMX_HANDLETYPE hComponent, OMX_PTR pAppData);
 this layer gets called first whenever a proxy s get handle is called
 ******************************************************************************
 *  @path WTSD_DucatiMMSW\omx\omx_il_1_x\omx_proxy_component\src
 *
 *  @rev 1.0
 */

/*==============================================================
 *! Revision History
 *! ============================
 *! 19-August-2009 B Ravi Kiran ravi.kiran@ti.com: Initial Version
 *================================================================*/

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "omx_proxy_common.h"
#include <timm_osal_interfaces.h>

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include "omx_proxy_common.h"
#include <OMX_TI_IVCommon.h>
#include <OMX_TI_Index.h>
#include <stdlib.h>
/* Tiler Apis */
#include "timm_osal_mutex.h"
#include <pthread.h>
#include <SysLinkMemUtils.h>
#include <ProcMgr.h>
#include <sys/time.h>
#include <memmgr.h>
#define COMPONENT_NAME "OMX.TI.DUCATI1.VIDEO.CAMERA"
/*Needs to be specific for every configuration wrapper*/

#ifdef _Android
#define DCC_PATH  "/system/etc/omapcam/"
#else
#define DCC_PATH  "/usr/share/omapcam/"
#endif

#ifdef _Android
#include <utils/Log.h>
#include <cutils/properties.h>
#undef LOG_TAG
#undef DOMX_DEBUG
#undef DOMX_ERROR
#undef DOMX_ENTER
#undef DOMX_EXIT
#define LOG_TAG "OMX_PROXY_CAM"
#define DOMX_DEBUG LOGE
#define DOMX_ERROR LOGE
#define DOMX_ENTER LOGE
#define DOMX_EXIT LOGE
#endif

/* 4Mb buffer, each Sensor need ~1.2Mb */
#define DCC_BUF_SIZE 4194304

#define _PROXY_OMX_INIT_PARAM(param,type) do {  \
    TIMM_OSAL_Memset((param), 0, sizeof (type));\
    (param)->nSize = sizeof (type);             \
    (param)->nVersion.s.nVersionMajor = 1;      \
    (param)->nVersion.s.nVersionMinor = 1;      \
  } while(0)

/* Incase of multiple instance, making sure DCC is initialized only for	\
   first instance */
static OMX_S16 numofInstance = 0;
TIMM_OSAL_PTR cam_mutex = NULL;

/* Ducati Mapped Addr  */
unsigned int pMappedBuf;
MemAllocBlock *MemReqDescTiler;
OMX_PTR TilerAddr = NULL;

OMX_S16 copy_DCCintobuffer(OMX_PTR, OMX_STRING *, OMX_U16);
OMX_ERRORTYPE DCC_Init(OMX_HANDLETYPE);
OMX_ERRORTYPE send_DCCBufPtr(OMX_HANDLETYPE hComponent);
void DCC_DeInit();
OMX_ERRORTYPE PROXY_ComponentDeInit(OMX_HANDLETYPE);

static OMX_ERRORTYPE ComponentPrivateDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  TIMM_OSAL_ERRORTYPE eOsalError = TIMM_OSAL_ERR_NONE;
  eOsalError = TIMM_OSAL_MutexObtain(cam_mutex, TIMM_OSAL_SUSPEND);
  if (eOsalError != TIMM_OSAL_ERR_NONE)
    {
      TIMM_OSAL_Error("Mutex Obtain failed");
    }
  if (numofInstance == 1)
    {
      DCC_DeInit();
    }
  numofInstance = numofInstance - 1;
  eOsalError = TIMM_OSAL_MutexRelease(cam_mutex);
  PROXY_assert(eOsalError == TIMM_OSAL_ERR_NONE,
    OMX_ErrorInsufficientResources, "Mutex release failed");
  eError = PROXY_ComponentDeInit(hComponent);
 EXIT:
  return eError;
}

OMX_ERRORTYPE OMX_ComponentInit(OMX_HANDLETYPE hComponent)
{
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  OMX_COMPONENTTYPE *pHandle = NULL;
  PROXY_COMPONENT_PRIVATE *pComponentPrivate;
#ifdef _Android
  char set_dcc[2] = {'0','\0'};
  const char set[2] = {'0','\0'};
  property_get("camera.dcc.enable", set_dcc, "0");
#endif
  pHandle = (OMX_COMPONENTTYPE *) hComponent;
  TIMM_OSAL_ERRORTYPE eOsalError = TIMM_OSAL_ERR_NONE;
  DOMX_ENTER("_____________________INSIDE CAMERA PROXY"
     "WRAPPER__________________________\n");
  pHandle->pComponentPrivate = (PROXY_COMPONENT_PRIVATE *)
    TIMM_OSAL_Malloc(sizeof(PROXY_COMPONENT_PRIVATE),
     TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_INT);
  pComponentPrivate =
    (PROXY_COMPONENT_PRIVATE *) pHandle->pComponentPrivate;
  if (pHandle->pComponentPrivate == NULL)
    {
      DOMX_ERROR(" ERROR IN ALLOCATING PROXY COMPONENT"
       "PRIVATE STRUCTURE");
      eError = OMX_ErrorInsufficientResources;
      goto EXIT;
    }
  pComponentPrivate->cCompName =
    TIMM_OSAL_Malloc(MAX_COMPONENT_NAME_LENGTH * sizeof(OMX_U8),
     TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_INT);
  /*Copying component Name - this will be picked up in the proxy common */
  assert(strlen(COMPONENT_NAME) + 1 < MAX_COMPONENT_NAME_LENGTH);
  TIMM_OSAL_Memcpy(pComponentPrivate->cCompName, COMPONENT_NAME,
   strlen(COMPONENT_NAME) + 1);
  /*Calling Proxy Common Init() */
  eError = OMX_ProxyCommonInit(hComponent);
  if (eError != OMX_ErrorNone)
    {
      DOMX_ERROR("\Error in Initializing Proxy");
      TIMM_OSAL_Free(pComponentPrivate->cCompName);
      TIMM_OSAL_Free(pComponentPrivate);
    }
#ifdef _Android
  if (strcmp(set_dcc, set) != 0)
#endif
    {
      DOMX_DEBUG(" DCC LOADING............ \n");
      pHandle->ComponentDeInit = ComponentPrivateDeInit;
      eOsalError = TIMM_OSAL_MutexObtain(cam_mutex, TIMM_OSAL_SUSPEND);
      PROXY_assert(eOsalError == TIMM_OSAL_ERR_NONE,
        OMX_ErrorInsufficientResources, "Mutex lock failed");
      if (numofInstance == 0)
       {
          eError = DCC_Init(hComponent);
          if (eError != OMX_ErrorNone)
            {
              DOMX_DEBUG(" Error in DCC Init");
            }
       }
      numofInstance = numofInstance + 1;
      eError = send_DCCBufPtr(hComponent);
      if (eError != OMX_ErrorNone)
        {
          DOMX_DEBUG(" Error in Sending DCC Buf ptr");
        }
      eOsalError = TIMM_OSAL_MutexRelease(cam_mutex);
      PROXY_assert(eOsalError == TIMM_OSAL_ERR_NONE,
       OMX_ErrorInsufficientResources, "Mutex release failed");
    }
 EXIT:
  return eError;
}
/* ===========================================================================*/
/**
 * @name DCC_Init()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE DCC_Init(OMX_HANDLETYPE hComponent)
{
  OMX_TI_PARAM_DCCURIINFO param;
  OMX_PTR ptempbuf;
  OMX_U16 nIndex = 0;
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  ProcMgr_MapType mapType;
  SyslinkMemUtils_MpuAddrToMap MpuAddr_list_1D = { 0 };
  OMX_S32 status = 0;
  OMX_STRING dcc_dir[80];
  OMX_U16 i;
  OMX_S16 ret_error = 0;
  _PROXY_OMX_INIT_PARAM(&param, OMX_TI_PARAM_DCCURIINFO);

  DOMX_ENTER("ENTER DCC_Init");
  /* Read the the DCC URI info */
  for (nIndex = 0; eError != OMX_ErrorNoMore; nIndex++)
    {
      param.nIndex = nIndex;
      eError =
       OMX_GetParameter(hComponent,OMX_TI_IndexParamDccUriInfo, &param);
      PROXY_assert((eError == OMX_ErrorNone) ||
        (eError == OMX_ErrorNoMore), eError,
       "Error in GetParam for Dcc URI info");
      if (eError == OMX_ErrorNone)
        {
          DOMX_DEBUG("DCC URI's %s ", param.sDCCURI);
          dcc_dir[nIndex] =
            TIMM_OSAL_Malloc(sizeof(OMX_U8) *
              (strlen(DCC_PATH) + MAX_URI_LENGTH + 1),
              TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_INT);
          PROXY_assert(dcc_dir[nIndex] != NULL,
             OMX_ErrorInsufficientResources, "Malloc failed");
          strcpy(dcc_dir[nIndex], DCC_PATH);
          strcat(dcc_dir[nIndex], (OMX_STRING) param.sDCCURI);
          strcat(dcc_dir[nIndex], "/");
        }
    }
  /* setting  back errortype OMX_ErrorNone */
  if (eError == OMX_ErrorNoMore)
    {
      eError = OMX_ErrorNone;
    }
  MemReqDescTiler =
    (MemAllocBlock *) TIMM_OSAL_Malloc((sizeof(MemAllocBlock) * 2),
     TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
  PROXY_assert(MemReqDescTiler != NULL,
       OMX_ErrorInsufficientResources, "Malloc failed");
  /* Allocate 1D Tiler buffer for 'N'DCC files  */
  MemReqDescTiler[0].pixelFormat = PIXEL_FMT_PAGE;
  MemReqDescTiler[0].dim.len = DCC_BUF_SIZE;
  MemReqDescTiler[0].stride = 0;
  TilerAddr = MemMgr_Alloc(MemReqDescTiler, 1);
  PROXY_assert(TilerAddr != NULL,OMX_ErrorInsufficientResources, "ERROR Allocating 1D TILER BUF");
  ptempbuf = TilerAddr;
  ret_error = copy_DCCintobuffer(ptempbuf, dcc_dir, nIndex);
  PROXY_assert(ret_error >= 0, ret_error,"ERROR in copy DCC files into buffer");
  mapType = ProcMgr_MapType_Tiler;
  MpuAddr_list_1D.mpuAddr = ((OMX_U32 ) TilerAddr);
  MpuAddr_list_1D.size = DCC_BUF_SIZE;
  status = SysLinkMemUtils_map(&MpuAddr_list_1D, 1,&pMappedBuf, mapType, PROC_APPM3);
  PROXY_assert(status >= 0, OMX_ErrorInsufficientResources,"Syslink map failed");
  DOMX_EXIT("EXIT DCC_Init eError: %d", eError);
 EXIT:
  for (i = 0; i < nIndex - 1; i++)
    {
      if (dcc_dir[i])
       TIMM_OSAL_Free(dcc_dir[i]);
    }
  return eError;
}

/* ===========================================================================*/
/**
* @name send_DCCBufPtr()
* @brief : Sending the DCC uri buff addr to ducati
* @param void
* @return return = 0 is successful
* @sa TBD
*
*/
/* ===========================================================================*/
OMX_ERRORTYPE send_DCCBufPtr(OMX_HANDLETYPE hComponent)
{
  OMX_TI_PARAM_DCCURIBUFFER uribufparam;
  OMX_ERRORTYPE eError = OMX_ErrorNone;

  DOMX_ENTER("ENTER send_DCCBufPtr");
  _PROXY_OMX_INIT_PARAM(&uribufparam, OMX_TI_PARAM_DCCURIBUFFER);
  eError =OMX_GetParameter(hComponent, OMX_TI_IndexParamDccUriBuffer,
      &uribufparam);
  PROXY_assert(eError == OMX_ErrorNone, eError,"Error in GetParam for Dcc URI Buffer");
  uribufparam.nDCCURIBuffSize = DCC_BUF_SIZE;
  uribufparam.pDCCURIBuff = (OMX_U8 *)pMappedBuf;
  DOMX_DEBUG("SYSLINK MAPPED ADDR:  0x%x sizeof buffer %d",
     uribufparam.pDCCURIBuff, uribufparam.nDCCURIBuffSize);
  eError =
    OMX_SetParameter(hComponent, OMX_TI_IndexParamDccUriBuffer,
     &uribufparam);
  if (eError != OMX_ErrorNone)
  {
    DOMX_ERROR(" Error in SetParam for DCC Uri Buffer");
  }
  DOMX_EXIT("EXIT send_DCCBufPtr eError: %d", eError);
 EXIT:
  return eError;
}
/* ===========================================================================*/
/**
 * @name copy_DCCintobuffer()
 * @brief : copies all the dcc profiles into the allocated 1D-Tiler buffer
 * @param void
 * @return return = 0 is successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_S16 copy_DCCintobuffer(OMX_PTR buffer, OMX_STRING * dir_path,
                             OMX_U16 numofURI)
{
  FILE *pFile;
  OMX_S32 lSize;
  size_t result;
  OMX_STRING filename;
  char temp[200];
  OMX_STRING dotdot = "..";
  DIR *d;
  struct dirent *dir;
  OMX_U16 i = 0;
  OMX_S16 ret = 0;

  DOMX_ENTER("ENTER copy_DCCintobuffer");
  for (i = 0; i < numofURI - 1; i++)
    {
      d = opendir(dir_path[i]);
      if (d)
        {
          while ((dir = readdir(d)) != NULL)     /* read each filename */
            {
               filename = dir->d_name;
               strcpy(temp, dir_path[i]);
               strcat(temp, filename);
               if ((*filename != *dotdot))
                 {
                    pFile = fopen(temp, "rb");
                    if (pFile == NULL)
                      {
                         DOMX_ERROR("File open Error");
                         ret = -1;
                      } else
                      {
                        fseek(pFile, 0, SEEK_END);
                        lSize = ftell(pFile);
                        rewind(pFile);
                        // copy the file into the buffer:
                        result =
                          fread(buffer, 1, lSize,pFile);
                        if (result != lSize)
                          {
                            DOMX_ERROR
                              ("fread: Reading error");
                                ret = -1;
                          }
                        buffer = buffer + lSize;
                      }
                      // terminate
                      fclose(pFile);
                 }
            }
            closedir(d);
        }
    }
  DOMX_EXIT("EXIT copy_DCCintobuffer return %d", ret);
  return ret;
}
/* ===========================================================================*/
/**
 * @name DCC_Deinit()
 * @brief
 * @param void
 * @return void
 * @sa TBD
 *
 */
/* ===========================================================================*/
void DCC_DeInit()
{
  DOMX_ENTER("ENTER DCC_DeInit");
  if (pMappedBuf)
    SysLinkMemUtils_unmap( pMappedBuf, PROC_APPM3);
  if (TilerAddr)
    MemMgr_Free(TilerAddr);
  if (MemReqDescTiler)
    TIMM_OSAL_Free(MemReqDescTiler);
  DOMX_EXIT("EXIT DCC_DeInit");
}
/*===============================================================*/
/** @fn Cam_Setup : This function is called when the the OMX Camera library is
 *                  loaded. It creates a mutex, which is used during DCC_Init()
 */
/*===============================================================*/
void __attribute__ ((constructor)) Cam_Setup(void)
{
  TIMM_OSAL_ERRORTYPE eError = TIMM_OSAL_ERR_NONE;

  eError = TIMM_OSAL_MutexCreate(&cam_mutex);
  if (eError != TIMM_OSAL_ERR_NONE)
    {
      TIMM_OSAL_Error("Creation of default mutex failed");
    }
}
/*===============================================================*/
/** @fn Cam_Destroy : This function is called when the the OMX Camera library is
 *                    unloaded. It destroys the mutex which was created by
 *                    Core_Setup().
 *
 */
/*===============================================================*/
void __attribute__ ((destructor)) Cam_Destroy(void)
{
  TIMM_OSAL_ERRORTYPE eError = TIMM_OSAL_ERR_NONE;

  eError = TIMM_OSAL_MutexDelete(cam_mutex);
  if (eError != TIMM_OSAL_ERR_NONE)
    {
      TIMM_OSAL_Error("Destruction of default mutex failed");
    }
}
