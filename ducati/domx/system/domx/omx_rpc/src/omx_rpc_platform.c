/*=========================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 *  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied.
 * ==========================================================================*/

/**
 *  @file  omx_rpc_platform.c
 *         This file contains methods that provides the functionality for
 *         the OpenMAX1.1 DOMX Framework RPC.
 *
 *  @path \WTSD_DucatiMMSW\framework\domx\omx_rpc\src
 *
 *  @rev 1.0
 */

/*==============================================================
 *! Revision History
 *! ============================
 *! 29-Mar-2010 Abhishek Ranka : Revamped DOMX implementation
 *!
 *! 19-August-2009 B Ravi Kiran ravi.kiran@ti.com: Initial Version
 *================================================================*/
  /******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
 /* ----- system and platform files ----------------------------*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <Std.h>

#include <OMX_Types.h>
#include <timm_osal_interfaces.h>
#include <timm_osal_trace.h>

#include <MultiProc.h>
#include <RcmClient.h>
#include <RcmServer.h>

/*-------program files ----------------------------------------*/
#include "omx_rpc.h"
#include "omx_rpc_stub.h"
#include "omx_rpc_skel.h"
#include "omx_rpc_internal.h"
#include "omx_rpc_utils.h"

/*This list needs to be a comprehensive list of all possible communicating RCM servers avalilable across the whole system (core 0 core 1, tesla, chiron)*/
char Core_Array[][MAX_CORENAME_LENGTH] = {"CHIRON","TESLA","DUCATI0","DUCATI1"};

char rcmservertable[][MAX_SERVER_NAME_LENGTH] = {"RSrv_Chiron","RSrv_Tesla","RSrv_Ducati0","RSrv_Ducati1"};

OMX_U32 heapIdArray[MAX_NUMBER_OF_HEAPS] = {1,0,0,1};
