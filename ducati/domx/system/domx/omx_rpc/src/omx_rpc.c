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
 *  @file  omx_rpc.c
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
#include <SysMgr.h>
#include <ProcMgr.h>

/*-------program files ----------------------------------------*/
#include "omx_rpc.h"
#include "omx_rpc_stub.h"
#include "omx_rpc_skel.h"
#include "omx_rpc_internal.h"
#include "omx_rpc_utils.h"
 
/* **************************** MACROS DEFINES *************************** */

/*For ipc setup*/
#define SYSM3_PROC_NAME             "SysM3"
    /*Shared Memory Area for MPU - SysM3 */
#define SHAREDMEM                   0xA0000000
#define SHAREDMEMSIZE               0x54000
    /*Shared Memory Area for MPU - AppM3 */
#define SHAREDMEM1                  0xA0055000
#define SHAREDMEMSIZE1              0x54000

/*The version nos. start with 1 and keep on incrementing every time there is a
protocol change in DOMX. This is just a marker to ensure that A9-Ducati DOMX
versions are in sync and does not indicate anything else*/
#define DOMX_VERSION 1
/* ******************************* EXTERNS ********************************* */
extern char rpcFxns[][MAX_FUNCTION_NAME_LENGTH];
extern rpcSkelArr rpcSkelFxns[];
extern char Core_Array[][MAX_CORENAME_LENGTH];
extern char rcmservertable[][MAX_SERVER_NAME_LENGTH];
extern OMX_U32 heapIdArray[MAX_NUMBER_OF_HEAPS];

extern TIMM_OSAL_PTR testSem;
extern TIMM_OSAL_PTR testSemSys;

/* ******************************* GLOBALS ********************************* */
RPC_Object rpcHndl[CORE_MAX];

RcmClient_Handle rcmHndl = NULL;
RcmServer_Handle rcmSrvHndl = NULL;

char *RCM_SERVER_NAME;
char *RCM_SERVER_NAME_LOCAL;

COREID TARGET_CORE_ID = CORE_MAX; //Should be configured in the CFG or header file for SYS APP split header.   
COREID LOCAL_CORE_ID = CORE_MAX; 
OMX_U32 PACKET_SIZE;// different packet sizes required for INTER-M3 case and MPU-APPM3 

//Counter to reflect no. of users
static OMX_U32 nInstanceCount = 0;
OMX_U8 CHIRON_IPC_FLAG = 1;

/*Mutex used to ensure that setup is done only once. It is created when library
  is loaded.*/
OMX_PTR pCreateMutex = NULL;

/*Used in ipc setup/destroy*/
ProcMgr_Handle procMgrHandle = NULL;

/* ************************* EXTERNS, FUNCTION DECLARATIONS ***************************** */
RPC_INDEX fxnExitidx, getFxnIndexFromRemote_skelIdx, nGetDOMXVersionIdx;
static Int32 fxnExit(UInt32 size, UInt32 *data);
RPC_OMX_ERRORTYPE fxn_exit_caller(void);

static Int32 getFxnIndexFromRemote_skel(UInt32 size, UInt32 *data);
static void getFxnIndexFromRemote_stub(void);

RPC_OMX_ERRORTYPE _RPC_IpcSetup();
RPC_OMX_ERRORTYPE _RPC_IpcDestroy();
RPC_OMX_ERRORTYPE _RPC_ClientCreate(OMX_STRING cComponentName);
RPC_OMX_ERRORTYPE _RPC_ClientDestroy();

/* ===========================================================================*/
/**
 * @name RPC_InstanceInit() 
 * @brief RPC instance init is used to bring up a instance of a client - this should be ideally invokable from any core
 *        For this the parameters it would require are
 *        Heap ID - this needs to be configured at startup (CFG) and indicates the heaps available for a RCM client to pick from 
 *        Server - this contains the RCM server name that the client should connect to 
 *        rcmHndl - Contains the Client once the call is completed
 *        rcmParams - 
 *        These values can be picked up from the RPC handle. But an unique identifier is required -Server
 * @param cComponentName  : Pointer to the Components Name that is requires the RCM client to be initialized
 * @return RPC_OMX_ErrorNone = Successful 
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_InstanceInit(OMX_STRING cComponentName, RPC_OMX_HANDLE* phRPCCtx)
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    RPC_OMX_CONTEXT *pRPCCtx = NULL;
    TIMM_OSAL_ERRORTYPE eError = TIMM_OSAL_ERR_NONE;
    OMX_BOOL bMutex = OMX_FALSE;

    pRPCCtx = (RPC_OMX_CONTEXT *)TIMM_OSAL_Malloc(sizeof(RPC_OMX_CONTEXT),
                                   TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_INT);
    RPC_assert(pRPCCtx != NULL, RPC_OMX_ErrorInsufficientResources,
               "Malloc failed");

    *(RPC_OMX_CONTEXT **)phRPCCtx = pRPCCtx;

    eError = TIMM_OSAL_MutexObtain(pCreateMutex, TIMM_OSAL_SUSPEND);
    RPC_assert(eError == TIMM_OSAL_ERR_NONE, RPC_OMX_ErrorInsufficientResources,
               "Mutex lock failed");
    bMutex = OMX_TRUE;

    nInstanceCount++;
    /*For 1st instance, do all the setup and create client*/
    if(nInstanceCount == 1)
    {
        eRPCError = _RPC_IpcSetup();
        RPC_assert(eRPCError == RPC_OMX_ErrorNone, eRPCError,
                   "Basic ipc setup failed");

        eRPCError = RPC_ModInit();
        RPC_assert(eRPCError == RPC_OMX_ErrorNone, eRPCError,
                   "ModInit failed");

        /*This will fill in the global rcmHndl*/
        eRPCError = _RPC_ClientCreate(cComponentName);
        RPC_assert(eRPCError == RPC_OMX_ErrorNone, eRPCError,
                   "Client create failed");
    }

    /* updating the RCM client handle within rpc context */
    pRPCCtx->ClientHndl[RCM_DEFAULT_CLIENT] = rcmHndl;
    pRPCCtx->ClientHndl[RCM_ADDITIONAL_CLIENT] = NULL;   
EXIT:
    if(eRPCError != RPC_OMX_ErrorNone)
    {
        if(bMutex)
        {
            nInstanceCount--;
            TIMM_OSAL_MutexRelease(pCreateMutex);
        }
        if(pRPCCtx)
        {
            TIMM_OSAL_Free(pRPCCtx);
            pRPCCtx = NULL;
        }
    }
    else
    {
        TIMM_OSAL_MutexRelease(pCreateMutex);
    }
    return eRPCError;
}



/* ===========================================================================*/
/**
 * @name RPC_InstanceDeInit() 
 * @brief This function Removes or deinitializes RCM client instances. This also manages the number of active users    
 *        of a given RCM client
 * @param cComponentName  : Pointer to the Components Name that is active user of the RCM client to be deinitialized
 * @return RPC_OMX_ErrorNone = Successful 
 * @sa TBD
 *
 */
/* ===========================================================================*/

RPC_OMX_ERRORTYPE RPC_InstanceDeInit(RPC_OMX_HANDLE hRPCCtx)
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone, 
                      eTmpError = RPC_OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE eError = TIMM_OSAL_ERR_NONE;
    OMX_BOOL bMutex = OMX_FALSE;

    eError = TIMM_OSAL_MutexObtain(pCreateMutex, TIMM_OSAL_SUSPEND);
    RPC_assert(eError == TIMM_OSAL_ERR_NONE, RPC_OMX_ErrorInsufficientResources,
               "Mutex lock failed. InstanceDeInit failed completely");
    bMutex = OMX_TRUE;

    nInstanceCount--;
    /*For last instance, shut down everything*/
    if(nInstanceCount == 0)
    {
        eTmpError = _RPC_ClientDestroy();
        if(eTmpError != RPC_OMX_ErrorNone)
        {
            TIMM_OSAL_Error("RPC ClientDestroy failed");
            eRPCError = eTmpError;
        }
        
        eTmpError = RPC_ModDeInit();
        if(eTmpError != RPC_OMX_ErrorNone)
        {
            TIMM_OSAL_Error("RPC ModDeInit failed");
            eRPCError = eTmpError;
        }
        
        eTmpError = _RPC_IpcDestroy();
        if(eTmpError != RPC_OMX_ErrorNone)
        {
            TIMM_OSAL_Error("ipc destroy failed");
            eRPCError = eTmpError;
        }
    }

EXIT:
    if(bMutex)
        TIMM_OSAL_MutexRelease(pCreateMutex);
    TIMM_OSAL_Free(hRPCCtx);

    DOMX_DEBUG("\n Leaving %s",__FUNCTION__);
    return eRPCError;
}

/* ===========================================================================*/
/**
 * @name RPC_ModDeInit() 
 * @brief This function Removes or deinitializes RCM client instances. This also manages the number of active users    
 *        of a given RCM client
 * @param cComponentName  : Pointer to the Components Name that is active user of the RCM client to be deinitialized
 * @return RPC_OMX_ErrorNone = Successful 
 * @sa TBD
 *
 */
 /* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_ModDeInit(void)
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    OMX_S32 status = 0;
    
    DOMX_DEBUG("\nEntered %s",__FUNCTION__);

    if(rcmSrvHndl)
    {
        status = RcmServer_delete(&rcmSrvHndl);
        if(status < 0)
        {
            TIMM_OSAL_Error("RCM Server delete failed, status = %d", status);
            eRPCError = RPC_OMX_RCM_ServerFail;
        }
        rcmSrvHndl = NULL;
    }

    status = RcmServer_destroy(); 
    if(status < 0 ) 
    {
        TIMM_OSAL_Error("RCM Server destroy failed, status = %d", status);
        eRPCError = RPC_OMX_RCM_ServerFail;
    }

    DOMX_DEBUG("\nLeaving %s",__FUNCTION__);
    return eRPCError;
}


/* ===========================================================================*/
/**
 * @name RPC_ModInit() 
 * @brief This function Creates the Default RCM servers on current processor       
 * @param Void 
 * @return RPC_OMX_ErrorNone = Successful 
 * @sa TBD
 *
 */
 /* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_ModInit(void)
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone,
                      eTmpError = RPC_OMX_ErrorNone;
    OMX_U32 i = 0, j = 0, fxIndx = 0;
    OMX_S32 status = 0;
    RcmServer_Config cfgParams;
    RcmServer_Params rcmSrvParams;
    OMX_BOOL bCallDestroyIfErr = OMX_FALSE;

    DOMX_DEBUG("\nEntered: RPC_ModInit()");

    /*Fetching RCM server name - This needs to be fetched from the default
    RCM server table*/
    RCM_SERVER_NAME_LOCAL = rcmservertable[MultiProc_getId(NULL)];

    DOMX_DEBUG("\nIn ModInit, MP getId = %d\n", MultiProc_getId(NULL));
    DOMX_DEBUG("\nrcmservertable[0] = %s\n", rcmservertable[0]);
    DOMX_DEBUG("\nrcmservertable[MultiProc_getId(NULL)] = %s\n", 
               rcmservertable[MultiProc_getId(NULL)]);
    DOMX_DEBUG("\nRCM_SERVER_NAME_LOCAL = %s\n", RCM_SERVER_NAME_LOCAL);

    if(CHIRON_IPC_FLAG)
    {
        if(MultiProc_getId(NULL) == APPM3_PROC) 
        {
        // This Value needs to be parsed from the Component Name
            TARGET_CORE_ID = CORE_CHIRON;
            LOCAL_CORE_ID = CORE_APPM3;
        }
        else
        {
        // This Value needs to be parsed from the Component Name
            TARGET_CORE_ID = CORE_APPM3;
            LOCAL_CORE_ID = CORE_CHIRON;
        }
        PACKET_SIZE = CHIRON_PACKET_SIZE;
    }
    else
    {
        if(MultiProc_getId(NULL) == SYSM3_PROC) 
        {
        // This Value needs to be parsed from the Component Name
            TARGET_CORE_ID = CORE_APPM3;
            LOCAL_CORE_ID = CORE_SYSM3;
        }
        else
        {
        // This Value needs to be parsed from the Component Name
            TARGET_CORE_ID = CORE_SYSM3;
            LOCAL_CORE_ID = CORE_APPM3;
        }
        PACKET_SIZE = DUCATI_PACKET_SIZE;
    }

    for(i = 0; i < CORE_MAX; i++)
    {
        rpcHndl[i].rcmHndl[LOCAL_CORE_ID] = NULL;
        rpcHndl[i].heapId[LOCAL_CORE_ID] = heapIdArray[LOCAL_CORE_ID];

        for(j = 0; j < MAX_FUNCTION_LIST; j++)
        {
            rpcHndl[i].rpcFxns[j].rpcFxnIdx = 0;
            rpcHndl[i].rpcFxns[j].FxnName = rpcFxns[j];
        }
        rpcHndl[i].NumOfTXUsers = 0;
    }

//RCM Server config    
/*
    cfgParams.maxTables = 8;
    cfgParams.maxNameLen = 20;
*/    
    status = RcmServer_getConfig(&cfgParams);
    RPC_assert(status >= 0, RPC_OMX_RCM_ServerFail, 
               "Server_getConfig failed");
    DOMX_DEBUG("Config Parameters:\n");
    DOMX_DEBUG("\nMaxNameLen = %d\nMaxTables = %d\n",
               cfgParams.maxNameLen, cfgParams.maxTables);

    //Create an rcm server instance
    DOMX_DEBUG("\nCalling Server setup\n");
    status = RcmServer_setup(&cfgParams);
    RPC_assert(status >= 0, RPC_OMX_RCM_ServerFail, 
               "Server_setup failed");
    bCallDestroyIfErr = OMX_TRUE;

    DOMX_DEBUG("\nCalling Server params init\n");
    status = RcmServer_Params_init(NULL, &rcmSrvParams);
    RPC_assert(status >= 0, RPC_OMX_RCM_ServerFail, 
               "Server_setup failed");

    DOMX_DEBUG("RCM Server Name: = %s", RCM_SERVER_NAME_LOCAL);
    status = RcmServer_create(RCM_SERVER_NAME_LOCAL, &rcmSrvParams, 
                              &rcmSrvHndl);
    RPC_assert(status >= 0, RPC_OMX_RCM_ServerFail, 
               "Server_create failed");
    DOMX_DEBUG("\nServer created\n");

    status = RcmServer_addSymbol(rcmSrvHndl, "fxnExit", fxnExit,
                                 (UInt32 *)&fxIndx);
    RPC_assert((status >= 0 && fxIndx != 0xFFFFFFFF), RPC_OMX_RCM_ServerFail,
               "Server_addSymbol failed");

    status = RcmServer_addSymbol(rcmSrvHndl, "getFxnIndexFromRemote_skel",
                                 getFxnIndexFromRemote_skel,
                                 (UInt32 *)&getFxnIndexFromRemote_skelIdx);
    RPC_assert((status >= 0 && fxIndx != 0xFFFFFFFF), RPC_OMX_RCM_ServerFail,
               "Server_addSymbol failed");

    for(i = 0; i < MAX_FUNCTION_LIST; i++)
    {
        status = RcmServer_addSymbol(rcmSrvHndl, rpcFxns[i], rpcSkelFxns[i].
                                     FxnPtr, (UInt32 *)(&rpcHndl
                                     [LOCAL_CORE_ID].rpcFxns[i].rpcFxnIdx));

        RPC_assert((status >= 0 && fxIndx != 0xFFFFFFFF),
                   RPC_OMX_RCM_ServerFail, "Server_addSymbol failed");
        DOMX_DEBUG("\n%d. Function %s registered", i+1, rpcFxns[i]);
    }

    //Start the RCM server thread
    status = RcmServer_start(rcmSrvHndl);
    RPC_assert(status >= 0, RPC_OMX_RCM_ServerFail, "Server_start failed");
    DOMX_DEBUG("\nRunning RcmServer\n");

EXIT:
    DOMX_DEBUG("\nLeaving %s",__FUNCTION__);
    if (eRPCError != RPC_OMX_ErrorNone && bCallDestroyIfErr)
    {
        eTmpError = RPC_ModDeInit();
        if(eTmpError != RPC_OMX_ErrorNone)
        {
            TIMM_OSAL_Error("ModDeInit failed");
        }
    }
    return eRPCError;
}



/* ===========================================================================*/
/**
 * @name fxnExit() 
 * @brief 
 * @param size   : Size of the incoming RCM message (parameter used in the RCM alloc call)
 * @param *data  : Pointer to the RCM message/buffer received
 * @return RPC_OMX_ErrorNone = Successful 
 * @sa TBD
 *
 */
/* ===========================================================================*/

static
Int32 fxnExit(UInt32 size, UInt32 *data)
{
    OMX_S16 status = 0;  /* success */

    DOMX_DEBUG("Executing fxnExit \n");
    DOMX_DEBUG("\nReleasing testcase semaphore:");
/*
    if(MultiProc_getId(NULL) == APPM3_PROC)	
        TIMM_OSAL_SemaphoreRelease (testSem);
    else
        TIMM_OSAL_SemaphoreRelease (testSemSys);
*/	
    return status;
}

/* ===========================================================================*/
/**
 * @name fxn_exit_caller() 
 * @brief 
 * @param void 
 * @return RPC_OMX_ErrorNone = Successful 
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE fxn_exit_caller(void)
{
    RcmClient_Message *rcmMsg = NULL;
    OMX_S32 status;
    RPC_OMX_ERRORTYPE rpcError = RPC_OMX_ErrorNone;
    
    DOMX_DEBUG("\n Entered %s",__FUNCTION__);	
    rcmMsg = RcmClient_alloc(rcmHndl, sizeof(RcmClient_Message));

    if (rcmMsg == NULL) {
        DOMX_DEBUG("\n Error in allocating RCM msg");
        rpcError = RPC_OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    rcmMsg->fxnIdx = fxnExitidx;
    //Sending Terminate messsage to remote processor
    status = RcmClient_execDpc(rcmHndl, rcmMsg); 
    
    if (status < 0) {
        DOMX_DEBUG( "\n Error RcmClient_execDpc failed \n");
        rpcError = RPC_OMX_RCM_ErrorExecFail;
        goto EXIT;        
        }	

EXIT:
    DOMX_DEBUG("\n Exitting %s",__FUNCTION__);
    return rpcError;

}

/* ===========================================================================*/
/**
 * @name getFxnIndexFromRemote_stub() 
 * @brief 
 * @param void 
 * @return RPC_OMX_ErrorNone = Successful 
 * @sa TBD
 *
 */
/* ===========================================================================*/
static void getFxnIndexFromRemote_stub(void)
{
    //RPC_INDEX FxnIdxArr[MAX_FUNCTION_LIST];
    OMX_U32 packetSize = 0x100;
    RPC_INDEX *FxnIdxArr;
    RcmClient_Message *rcmMsg;
    OMX_S16 status;
    RPC_INDEX fxnIdx;
    OMX_U8 i;
    FxnList FxnIdxList;
    
    DOMX_DEBUG("\nENTERED %s",__FUNCTION__);
    
    rcmMsg = RcmClient_alloc(rcmHndl, packetSize);
    
    if(rcmMsg ==NULL) {
        DOMX_DEBUG("\nError in Allocting rcm message");        
        goto EXIT;
    }
    
    rcmMsg->fxnIdx = getFxnIndexFromRemote_skelIdx;
       FxnIdxArr = (RPC_INDEX *)(&rcmMsg->data);
    TIMM_OSAL_Memcpy(FxnIdxArr,0,sizeof(RPC_INDEX)*MAX_FUNCTION_LIST);

    status = RcmClient_exec(rcmHndl, rcmMsg);
    
    if (status < 0) {
        DOMX_DEBUG( "\n Error RcmClient_exec failed \n");
        goto EXIT;        
        }
        
    for(i=0;i<MAX_FUNCTION_LIST;i++)
    {
    
        DOMX_DEBUG("\n function index from remote side %x",  (OMX_U32)(*(FxnIdxArr+i)));
        //rpcHndl[TARGET_CORE_ID].rpcFxns[i].rpcFxnIdx =  (RPC_INDEX *)(FxnIdxArr+i);
        
    }
EXIT:
    return;
}

/* ===========================================================================*/
/**
 * @name getFxnIndexFromRemote_skel() 
 * @brief 
 * @param void 
 * @return RPC_OMX_ErrorNone = Successful 
 * @sa TBD
 *
 */
/* ===========================================================================*/

static
Int32 getFxnIndexFromRemote_skel(UInt32 size, UInt32 *data)
{
    Int status = 0;  /* success */
    RPC_INDEX *FxnIdxArr;
    OMX_U32 i;
    
    FxnIdxArr = (RPC_INDEX *)(data);
    
    DOMX_DEBUG("\nENTERED %s",__FUNCTION__);
    
    for(i=0;i<MAX_FUNCTION_LIST;i++)
    //TIMM_OSAL_Memcpy(FxnIdxArr+i,rpcHndl[LOCAL_CORE_ID].rpcFxns[i].rpcFxnIdx,sizeof(RPC_INDEX));
	TIMM_OSAL_Memcpy((TIMM_OSAL_PTR)(FxnIdxArr+i),(TIMM_OSAL_PTR)i,sizeof(RPC_INDEX));
    //    (RPC_INDEX )(*(FxnIdxArr+i)) = rpcHndl[LOCAL_CORE_ID].rpcFxns[i].rpcFxnIdx;

    return status;
}



/* ===========================================================================*/
/**
 * @name _RPC_GetRemoteDOMXVersion() 
 * @brief This function is used by DOMX to communicate with its remote
 *        counterpart on Ducati and ensure that they are in sync. This function
 *        does not have any OMX counterpart and is used only internally by DOMX.
 * @param hRcmHandle  : The RCM client handle.
 * @param nFxnIdx     : Function index of the remote function that will give the
 *                      version
 * @param *nVer       : The version no. returned by the remote side.
 * @return RPC_OMX_ErrorNone = Successful 
 * @sa TBD
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE _RPC_GetRemoteDOMXVersion(RPC_OMX_HANDLE hRcmHandle,
                                           OMX_U32 nFxnIdx, OMX_U32 *nVer)
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    RcmClient_Message *pPacket = NULL;
    RcmClient_Handle hRcmClient = (RcmClient_Handle)hRcmHandle;
    RPC_OMX_MESSAGE *pRPCMsg = NULL;
    RPC_OMX_BYTE *pMsgBody = NULL;
    OMX_U32 nPos = 0, nPacketSize = PACKET_SIZE;
    OMX_S32 status = 0;

    pPacket = RcmClient_alloc(hRcmClient, nPacketSize);
    RPC_assert(pPacket != NULL, RPC_OMX_ErrorInsufficientResources,
               "Error Allocating RCM Message Frame");
    pRPCMsg = (RPC_OMX_MESSAGE*)(&pPacket->data);
    pMsgBody = &pRPCMsg->msgBody[0];
    pPacket->fxnIdx = nFxnIdx;
    status = RcmClient_exec(hRcmClient, pPacket);
    if(status < 0)
    {
        RcmClient_free(hRcmClient, pPacket);
        pPacket = NULL;
        RPC_assert(0, RPC_OMX_RCM_ErrorExecFail,
                   "RcmClient_exec failed");
    }
    /*Get the DOMX version*/
    RPC_GETFIELDVALUE(pMsgBody, nPos, *nVer, OMX_U32);
    RcmClient_free(rcmHndl, pPacket);

EXIT:
    return eRPCError;
}



/*===============================================================*/
/** @fn _RPC_ClientCreate : This function creates RCM Client and gets symbol
 *                          indices for all functions.
 *
 */
/*===============================================================*/
RPC_OMX_ERRORTYPE _RPC_ClientCreate(OMX_STRING cComponentName)
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone,
                      eTmpError = RPC_OMX_ErrorNone;
    RcmClient_Config cfgParams;
    RcmClient_Params rcmParams;
    OMX_STRING rcmServerName = NULL;
    OMX_S32 status = 0;
    OMX_U32 i = 0;
    OMX_BOOL bCallDestroyIfErr = OMX_FALSE;
    OMX_U32 nVer = 0;

    eRPCError = RPC_UTIL_GetTargetServerName(cComponentName, &rcmServerName);
    RPC_assert(eRPCError == RPC_OMX_ErrorNone, eRPCError,
               "Get target server name failed");
    DOMX_DEBUG("\n RCM Server Name To connected to: %s", rcmServerName);
    
    /* RCM client configuration*/
    /*
    cfgParams.maxNameLen = MAX_FUNCTION_LIST+2;
    cfgParams.defaultHeapIdArrayLength = MAX_NUMBER_OF_HEAPS;
    */

    status = RcmClient_getConfig(&cfgParams);
    RPC_assert(status >= 0, RPC_OMX_RCM_ClientFail, "Client_getConfig failed");
    DOMX_DEBUG("\nConfig Parameters:\n");
    DOMX_DEBUG("\nMaxNameLen = %d\nHeapIdArrayLength = %d\n", 
               cfgParams.maxNameLen, cfgParams.defaultHeapIdArrayLength);

    DOMX_DEBUG( "\nCalling RCM Setup\n");
    status = RcmClient_setup(&cfgParams);
    RPC_assert(status >= 0, RPC_OMX_RCM_ClientFail, "Client_setup failed");
    bCallDestroyIfErr = OMX_TRUE;

    status = RcmClient_Params_init(NULL, &rcmParams); 
    RPC_assert(status >= 0, RPC_OMX_RCM_ClientFail, 
               "Client_Params_init failed");
    rcmParams.heapId = heapIdArray[LOCAL_CORE_ID];
    DOMX_DEBUG("\n Heap ID configured : %d\n", rcmParams.heapId);
    if(CHIRON_IPC_FLAG && (LOCAL_CORE_ID==CORE_APPM3))
    {
        /*Hardcoded as heapId 0 when running on APPM3. This is done only for 
        WHEN RUNNING ON MPU-APPM3 - "Both use HeapId 0" Work around will be
        fixed when Independent Heaps are available across MPU and APPM3*/
        rcmParams.heapId = 1; 
    }
    DOMX_DEBUG("\n Heap ID configured : %d\n", rcmParams.heapId);

    /* Component Name based Server Name*/
    RCM_SERVER_NAME = rcmServerName;

    DOMX_DEBUG("\nCalling client create with server name = %s\n", 
               RCM_SERVER_NAME);
    status = RcmClient_create(RCM_SERVER_NAME, &rcmParams, &rcmHndl);
    RPC_assert(status >= 0, RPC_OMX_RCM_ClientFail,
               "RCM ClientCreate failed. Cannot Establish the connection");
    DOMX_DEBUG("\nClient created. Connected to Server\n");

    /*Checking DOMX version*/
    DOMX_DEBUG("Checking DOMX version");
    status = RcmClient_getSymbolIndex(rcmHndl, "getDOMXVersion",
                                      (UInt32 *)(&nGetDOMXVersionIdx));
    RPC_assert(status >= 0, RPC_OMX_RCM_ClientFail,
               "GetSymbolIndex failed");
    eRPCError = _RPC_GetRemoteDOMXVersion(rcmHndl, nGetDOMXVersionIdx, &nVer);
    RPC_assert(eRPCError == RPC_OMX_ErrorNone, eRPCError,
               "Failed to get remote DOMX version");
    RPC_assert(nVer == DOMX_VERSION, RPC_OMX_ErrorUndefined,
         "Version mismatch detected - A9 and Ducati DOMX versions not in sync");

    DOMX_DEBUG("\nCalling RCM_getSymbolIndex(rpcFxns array)\n");
    status = RcmClient_getSymbolIndex(rcmHndl, "getFxnIndexFromRemote_skel",
                                    (UInt32 *)(&getFxnIndexFromRemote_skelIdx));
    
    RPC_assert(status >= 0, RPC_OMX_RCM_ClientFail,
               "GetSymbolIndex failed");

    DOMX_DEBUG("\nGetting Symbols\n");
    for(i = 0; i < MAX_FUNCTION_LIST; i++)
    {
        status = RcmClient_getSymbolIndex(rcmHndl,
                        rpcHndl[TARGET_CORE_ID].rpcFxns[i].FxnName,
                        (UInt32*)&rpcHndl[TARGET_CORE_ID].rpcFxns[i].rpcFxnIdx);
        RPC_assert(status >= 0, RPC_OMX_RCM_ClientFail,
               "GetSymbolIndex failed");

        DOMX_DEBUG("\n%d. Function Index Obtained: %d for %s", i+1,
                   rpcHndl[TARGET_CORE_ID].rpcFxns[i].rpcFxnIdx,
                   rpcHndl[TARGET_CORE_ID].rpcFxns[i].FxnName);
    }

    DOMX_DEBUG("\nCalling RCM_getSymbolIndex(fxnExit)");
    status = RcmClient_getSymbolIndex(rcmHndl, "fxnExit",
                                      (UInt32 *)(&fxnExitidx));
    RPC_assert(status >= 0, RPC_OMX_RCM_ClientFail,
               "GetSymbolIndex failed");

EXIT:
    if(eRPCError != RPC_OMX_ErrorNone && bCallDestroyIfErr)
    {
        eTmpError = _RPC_ClientDestroy();
        if(eTmpError != RPC_OMX_ErrorNone)
        {
            TIMM_OSAL_Error("Client destruction failed");
        }
    }
    return eRPCError;
}



/*===============================================================*/
/** @fn _RPC_ClientDestroy : This function destroys RCM Client.
 *
 */
/*===============================================================*/
RPC_OMX_ERRORTYPE _RPC_ClientDestroy()
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    OMX_S32 status = 0;

    if(rcmHndl)
    {
        status = RcmClient_delete(&rcmHndl);
        if(status < 0)
        {
            TIMM_OSAL_Error("\nError in RcmClient_delete. Error Code: %d\n",
                            status);
            eRPCError = RPC_OMX_RCM_ClientFail;
        }
        rcmHndl = NULL;
    }

    status = RcmClient_destroy();
    if(status < 0)
    {
        TIMM_OSAL_Error("\nError in RcmClient_destroy. Error Code: %d\n",
                        status);
        eRPCError = RPC_OMX_RCM_ClientFail;
    }

    return eRPCError;
}



/*===============================================================*/
/** @fn _RPC_IpcSetup : This function performs basic ipc setup.
 *
 */
/*===============================================================*/
RPC_OMX_ERRORTYPE _RPC_IpcSetup()
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone,
                      eTmpError = RPC_OMX_ErrorNone;
    OMX_U16 procId = 0;
    OMX_U32 shAddrBase = 0, shAddrBase1 = 0;
    SysMgr_Config config;
    OMX_S32 status = 0;
    OMX_BOOL bCallDestroyIfErr = OMX_FALSE;

    DOMX_DEBUG("\nSetup IPC components\n");

    SysMgr_getConfig(&config);
    status = SysMgr_setup(&config);
    RPC_assert(status >= 0, RPC_OMX_ErrorHardware, 
               "SysMgr Setup failed");
    bCallDestroyIfErr = OMX_TRUE;

    procId = MultiProc_getId(SYSM3_PROC_NAME);

        /* Open a handle to the ProcMgr instance. */
    status = ProcMgr_open(&procMgrHandle,
                          procId);
    RPC_assert(status >= 0, RPC_OMX_ErrorHardware, 
               "ProcMgr open failed");
    DOMX_DEBUG("ProcMgr_open Status [0x%x]\n", status);
    /* Get the address of the shared region in kernel space. */
    status = ProcMgr_translateAddr(procMgrHandle,
                                    (OMX_PTR)&shAddrBase,
                                    ProcMgr_AddrType_MasterUsrVirt,
                                    (OMX_PTR)SHAREDMEM,
                                    ProcMgr_AddrType_SlaveVirt);
    RPC_assert(status >= 0, RPC_OMX_ErrorHardware,
               "Error in ProcMgr_translateAddr");
    DOMX_DEBUG("Virt address of shared address base #1: 0x%x\n", shAddrBase);

    /* Get the address of the shared region in kernel space. */
    status = ProcMgr_translateAddr(procMgrHandle,
                                    (OMX_PTR)&shAddrBase1,
                                    ProcMgr_AddrType_MasterUsrVirt,
                                    (OMX_PTR)SHAREDMEM1,
                                    ProcMgr_AddrType_SlaveVirt);
    RPC_assert(status >= 0, RPC_OMX_ErrorHardware,
               "Error in ProcMgr_translateAddr");
    DOMX_DEBUG("Virt address of shared address base #2: 0x%x\n", shAddrBase1);

    /* Add the region to SharedRegion module. */
    status = SharedRegion_add(0, (OMX_PTR)shAddrBase, SHAREDMEMSIZE);
    RPC_assert(status >= 0, RPC_OMX_ErrorHardware,
               "Error in SharedRegion_add");
    DOMX_DEBUG("SharedRegion_add 0x%x\n", status);

    /* Add the region to SharedRegion module. */
    status = SharedRegion_add(1, (OMX_PTR)shAddrBase1, SHAREDMEMSIZE1);
    RPC_assert(status >= 0, RPC_OMX_ErrorHardware,
               "Error in SharedRegion_add1");
    DOMX_DEBUG("SharedRegion_add1 0x%x\n", status);

EXIT:
    if(eRPCError != RPC_OMX_ErrorNone && bCallDestroyIfErr)
    {
        eTmpError = _RPC_IpcDestroy();
        if(eTmpError != RPC_OMX_ErrorNone)
        {
            TIMM_OSAL_Error("ipc destroy failed");
        }
    }
    DOMX_DEBUG("Leaving _RPC_IpcSetup()\n");
    return eRPCError;
}



/*===============================================================*/
/** @fn _RPC_IpcDestroy : This function destroys the basic ipc modules.
 *
 */
/*===============================================================*/
RPC_OMX_ERRORTYPE _RPC_IpcDestroy()
{
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    OMX_S32 status = 0;

    if(procMgrHandle)
    {
        status = ProcMgr_close(&procMgrHandle);
        procMgrHandle = NULL;
        if(status < 0) 
        {
            eRPCError = RPC_OMX_ErrorHardware;
            TIMM_OSAL_Error("\nError in ProcMgr_close 0x%x\n", status);
        }
    }

    DOMX_DEBUG("\nClosing sysmgr\n");
    status = SysMgr_destroy();
    if (status < 0) 
    {
        eRPCError = RPC_OMX_ErrorHardware;
        TIMM_OSAL_Error("Error in SysMgr_destroy 0x%x\n", status);
    }

    return eRPCError;
}



/*===============================================================*/
/** @fn RPC_Setup : This function is called when the the DOMX library is
 *                  loaded. It creates a mutex, which is used to synchronize
 *                  init/deinit in multi-instance scenarios.
 *
 */
/*===============================================================*/
void __attribute__ ((constructor)) RPC_Setup(void)
{
    TIMM_OSAL_ERRORTYPE eError = TIMM_OSAL_ERR_NONE;

    eError = TIMM_OSAL_MutexCreate(&pCreateMutex);
    if(eError != TIMM_OSAL_ERR_NONE)
    {
        TIMM_OSAL_Error("\nCreation of default mutex failed\n");
    }
}



/*===============================================================*/
/** @fn RPC_Destroy : This function is called when the the DOMX library is
 *                    unloaded. It destroys the mutex which was created by
 *                    RPC_Setup().
 *
 */
/*===============================================================*/
void __attribute__ ((destructor)) RPC_Destroy(void)
{
    TIMM_OSAL_ERRORTYPE eError = TIMM_OSAL_ERR_NONE;

    eError = TIMM_OSAL_MutexDelete(pCreateMutex);
    if(eError != TIMM_OSAL_ERR_NONE)
    {
        TIMM_OSAL_Error("\nDestruction of default mutex failed\n");
    }
}
