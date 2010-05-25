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
/*=========================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 *  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied.
 * ==========================================================================*/

/**
 *  @file  omx_rpc_internal.h
 *         This file contains methods that provides the functionality for
 *         the OpenMAX1.1 DOMX Framework RPC.
 *
 *  @path \WTSD_DucatiMMSW\framework\domx\omx_rpc\inc 
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
 

#ifndef OMXRPC_INTERNAL_H
#define OMXRPC_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
/* ----- system and platform files ----------------------------*/ 
#include <RcmClient.h>
#include <HeapBuf.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <OMX_Audio.h>
#include <OMX_Video.h>
#include <OMX_Types.h>
#include <OMX_Index.h>
#include <OMX_TI_Index.h>
#include <OMX_TI_Common.h>

/*-------program files ----------------------------------------*/
#include "omx_rpc.h"
#include "omx_rpc_utils.h"

/******************************************************************
 *   DEFINES - CONSTANTS
 ******************************************************************/
/* *********************** OMX RPC DEFINES******************************************/
#define COMPONENT_NAME_LENGTH_MAX 128

/*This defines the maximum length the processor name can take - The is based on the component
name based Target core retireval*/
#define MAX_CORENAME_LENGTH 20 

#define MAX_SERVER_NAME_LENGTH 40

#define MAX_NUM_OF_RPC_USERS 5

/*This defines the maximum number of remote functions that can be registered*/
#define MAX_FUNCTION_LIST 19

/*This defines the maximum number of characters a remote function name can take. 
This is used to define the length of maximum string length the symbol can be*/
#define MAX_FUNCTION_NAME_LENGTH 80

/* ************************* MESSAGE BUFFER DEFINES *********************************/
#define DEFAULTBUFSIZE 1024
#define MAX_MSG_FRAME_LENGTH DEFAULTBUFSIZE
#define MESSAGE_BODY_LENGTH (MAX_MSG_FRAME_LENGTH - sizeof(RPC_MSG_HEADER))

/* **************************PACKET SIZES-TEMPORARY FIX*****************************/
/*Used to define the length of the Heap Id array which contains all
 the statically registered heaps used for RCM buffers*/
#define MAX_NUMBER_OF_HEAPS 4

#define CHIRON_PACKET_SIZE 0x90
#define DUCATI_PACKET_SIZE 0x100

/**NEW**/
#define MAX_PROC 4
#define APPM3_PROC 3
#define SYSM3_PROC 2
#define TESLA_PROC 1
#define CHIRON_PROC 0

/*******************************************************************************
* Enumerated Types
*******************************************************************************/
typedef enum COREID
{
        CORE_CHIRON = 0,
        CORE_TESLA = 1,
        CORE_SYSM3 = 2,
        CORE_APPM3 = 3,
        CORE_MAX = 4
}COREID; 

/************************************* OMX RPC MESSAGE STRUCTURE ******************************/
typedef struct RPC_MSG_HEADER
{ 
        RPC_OMX_ERRORTYPE     nRPCCmdStatus; // command status is for reflecting successful remote OMX return of call
        OMX_ERRORTYPE         nOMXReturn; // omx error      
}RPC_MSG_HEADER;

typedef struct RPC_OMX_MESSAGE
{
        RPC_MSG_HEADER msgHeader;
        RPC_OMX_BYTE   msgBody[MESSAGE_BODY_LENGTH];
}RPC_OMX_MESSAGE;


/**************************** RPC FUNCTION INDEX STRUCTURES FOR STUB AND SKEL **********************/
typedef struct rpcFxnArr {
        RPC_INDEX rpcFxnIdx;
        char *FxnName;
} rpcFxnArr;

typedef struct rpcSkelArr {
        OMX_PTR FxnPtr;
} rpcSkelArr;

/** 
  *  @brief           Index for Remote Function Index Array. 
  */
typedef enum rpc_fxn_list{
    RPC_OMX_FXN_IDX_SET_PARAMETER = 0,
    RPC_OMX_FXN_IDX_GET_PARAMETER = 1,
    RPC_OMX_FXN_IDX_GET_HANDLE = 2,
    RPC_OMX_FXN_IDX_USE_BUFFER = 3,
    RPC_OMX_FXN_IDX_FREE_HANDLE = 4,
    RPC_OMX_FXN_IDX_SET_CONFIG    = 5,
    RPC_OMX_FXN_IDX_GET_CONFIG    = 6,
    RPC_OMX_FXN_IDX_GET_STATE     = 7,
    RPC_OMX_FXN_IDX_SEND_CMD      = 8,
    RPC_OMX_FXN_IDX_GET_VERSION   = 9,
    RPC_OMX_FXN_IDX_GET_EXT_INDEX = 10,
    RPC_OMX_FXN_IDX_FILLTHISBUFFER = 11,
    RPC_OMX_FXN_IDX_FILLBUFFERDONE = 12,
    RPC_OMX_FXN_IDX_FREE_BUFFER = 13,
    RPC_OMX_FXN_IDX_EMPTYTHISBUFFER = 14,
    RPC_OMX_FXN_IDX_EMPTYBUFFERDONE = 15,
    RPC_OMX_FXN_IDX_EVENTHANDLER = 16,
    RPC_OMX_FXN_IDX_ALLOCATE_BUFFER = 17,
    RPC_OMX_FXN_IDX_COMP_TUNNEL_REQUEST = 18,
    RPC_OMX_FXN_IDX_MAX = 19
}rpc_fxn_list;

/**************************** MISC ENUM **********************/
typedef enum ProxyType {
    ClientProxy = 0,
    TunnelProxy = 1,
}ProxyType;

typedef enum pRcmClientType{
    RCM_DEFAULT_CLIENT = 0,
    RCM_ADDITIONAL_CLIENT = 1,
    RCM_MAX_CLIENT = 2        
}pRcmClientType;

typedef enum CacheType{
    Cache_SameProc = 0, // Same processor cache (Inter Ducati case)
    Cache_DMA = 1,  // No cache required as data access is through DMA - flag to be filled in by OMX component(BASE)
    //placeholder future types to follow
}CacheType;

/*******************************************************************************
* STRUCTURES
*******************************************************************************/
typedef struct {
    RPC_INDEX FxnIdxArr[MAX_FUNCTION_LIST];
}FxnList;

/* ********************************************* RPC LAYER HANDLE ************************************** */
typedef struct RPC_Object
{
//RCM init params
RcmClient_Handle rcmHndl[CORE_MAX]; //RCM Handle on that respective core
//RcmClient_Params rcmParams[CORE_MAX];
OMX_U32 heapId[CORE_MAX]; //these heap IDs need to be configured during ModInit to the correct values after heaps for TILER and other apps have been allocated
//Functions to be registered and indices
rpcFxnArr rpcFxns[RPC_OMX_FXN_IDX_MAX];
//Number of users per transport or RCM client
OMX_U32 NumOfTXUsers;
}RPC_Object;

/*New*/
typedef struct RPC_OMX_CONTEXT
{
RPC_OMX_HANDLE remoteHandle; //real components handle
RcmClient_Handle ClientHndl[RCM_MAX_CLIENT]; //This is filled in after the ClientCreate() call goes through
OMX_U32 HeapId[2]; //This needs to be filled in before client create() - fetch this head ID from Global table
COREID RealCore; // Target core of component - To be parsed from component name and put in
ProxyType Tprxy; // Tunnel or Client Proxy
CacheType CacheMode; //DMA, same processor etc
OMX_CALLBACKTYPE *CbInfo; // should contain the updated pointers
OMX_PTR pAppData;
}RPC_OMX_CONTEXT;

typedef struct RPC_OMX_SKEL_CONTEXT
{
OMX_PTR hRpcAppData;
RcmClient_Handle ClientHndl[RCM_MAX_CLIENT];
}RPC_OMX_SKEL_CONTEXT;

/*********************TEMP-TO REMOVE*********************/
//NOTE: Need to check if we need this. It should be defined in some OMX header.
/* this is used for GetComponentVersion****/
typedef struct VERSION_INFO
{
char cName[128]; /***like "OMX.TI.Video.encoder" **/
OMX_VERSIONTYPE sVersion;
OMX_VERSIONTYPE sSpecVersion;
OMX_UUIDTYPE sUUID;
} VERSION_INFO; 

#endif
