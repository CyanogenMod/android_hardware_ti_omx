
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
/* ==============================================================================
*             Texas Instruments OMAP (TM) Platform Software
*  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
*
*  Use of this software is controlled by the terms and conditions found 
*  in the license agreement under which this software has been supplied.
* ============================================================================ */
/**
* @file ResourceManager.c
*
* This file implements resource manager for Linux 23.x that 
* is fully compliant with the Khronos OpenMax specification 1.0.
*
* @path  $(CSLPATH)\
*
* @rev  1.0
*/
/* ---------------------------------------------------------------------------- 
*! 
*! Revision History 
*! ===================================
*! 24-Apr-2005 rg:  Initial Version. 
*!
* ============================================================================= */
#include <unistd.h>     // for sleep
#include <stdlib.h>     // for calloc
#include <sys/time.h>   // time is part of the select logic
#include <sys/types.h>  // for opening files
#include <sys/ioctl.h>  // for ioctl support
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <string.h>     // for memset
#include <stdio.h>      // for buffered io
#include <fcntl.h>      // for opening files.
#include <errno.h>      // for error handling support
#include <linux/soundcard.h>
#include <qosregistry.h>
#include <qosti.h>
#include <qos_ti_uuid.h>
#include <dbapi.h>
#include <DSPManager.h>
#include <DSPProcessor.h>
#include <DSPProcessor_OEM.h>


#ifdef __PERF_INSTRUMENTATION__
#include "perf.h"
PERF_OBJHANDLE pPERF = NULL;
#endif
#include <ResourceManager.h>
#include <ResourceManagerProxyAPI.h>
#include <PolicyManagerAPI.h>
#include <Resource_Activity_Monitor.h>
#include <pthread.h>
#include <signal.h>

RM_ComponentList componentList;
RM_ComponentList pendingComponentList;
int fdread, fdwrite;
int pmfdread, pmfdwrite;
int eErrno;
unsigned int totalCpu=0;
unsigned int imageTotalCpu=0;
unsigned int videoTotalCpu=0;
unsigned int audioTotalCpu=0;
unsigned int lcdTotalCpu=0;
unsigned int cameraTotalCpu=0;

RESOURCEMANAGER_COMMANDDATATYPE cmd_data;
RESOURCEMANAGER_COMMANDDATATYPE globalrequest_cmd_data;
POLICYMANAGER_COMMANDDATATYPE policy_data;
POLICYMANAGER_RESPONSEDATATYPE policyresponse_data;


/*------------------------------------------------------------------------------------*
  * main() 
  *
  *                     This is the thread of resource manager
  *
  * @param 
  *                     None
  *
  * @retval 
  *                     None
  */
/*------------------------------------------------------------------------------------*/
int main()
{
#ifdef __ENABLE_RMPM_STUB__
    fprintf(stderr, "Warning - using stub version of Resource Manager!!\n");
#endif
    RM_DPRINT("[Resource Manager] - start resource manager main function\n");
        
#ifdef __PERF_INSTRUMENTATION__
    pPERF = PERF_Create(PERF_FOURCC('R','M',' ',' '),
                        PERF_ModuleAudioDecode | PERF_ModuleAudioEncode |
                        PERF_ModuleVideoDecode | PERF_ModuleVideoEncode |
                        PERF_ModuleImageDecode | PERF_ModuleImageEncode |
                        PERF_ModuleSystem);
#endif
    RM_DPRINT("[Resource Manager] - start resource manager main function\n");
    
    int size = 0;
    int ret;
    char rmsideNamedPipeName[100];
    char rmsideHandleString[100];
    OMX_S16 fdmax;
    DSP_STATUS status = DSP_SOK;
    fd_set watchset;
    OMX_BOOL Exitflag = OMX_FALSE;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    

#ifndef __ENABLE_RMPM_STUB__
    int MemoryIsPresent = FALSE;
#endif    

    RM_DPRINT("[Resource Manager] - going to create the read & write pipe\n");
    unlink(RM_SERVER_IN);
        
    if((mknod(RM_SERVER_IN,S_IFIFO|PERMS,0)<0)&&(eErrno!=EEXIST)) 
        RM_DPRINT("[Resource Manager] - mknod failure to create the read pipe, error=%d\n", eErrno);


    sleep(1); /* since the policy manager needs to create pipes that this process will 
                        read we will wait to give time for the policy manager time to establish pipes */
    componentList.numRegisteredComponents = 0;

#ifndef __ENABLE_RMPM_STUB__
    // initialize Qos
    eError = InitializeQos();
    if (eError != OMX_ErrorNone)
    {
        RM_DPRINT ("InitializeQos failed\n");
        exit (1);
    }

    // create pipe for read
    if((pmfdwrite=open(PM_SERVER_IN,O_WRONLY))<0)
        RM_DPRINT("[Policy Manager] - failure to open the WRITE pipe\n");

     if((pmfdread=open(PM_SERVER_OUT,O_RDONLY))<0)
        RM_DPRINT("[Policy Manager] - failure to open the READ pipe\n");

#endif    // create pipe for read

    if((fdread=open(RM_SERVER_IN,O_RDONLY))<0)
        RM_DPRINT("[Resource Manager] - failure to open the READ pipe\n");

                
    FD_ZERO(&watchset);
    size=sizeof(cmd_data);
        
    RM_DPRINT("[Resource Manager] - going enter while loop\n");

    while(!Exitflag) {       
        FD_SET(fdread, &watchset);
        fdmax = fdread;
#ifndef __ENABLE_RMPM_STUB__        
        FD_SET(pmfdread, &watchset);    
        if (pmfdread > fdmax) {
            fdmax = pmfdread;
        }
#endif
        sigset_t set;
        sigemptyset (&set);
        sigaddset (&set, SIGALRM);
        status = pselect(fdmax+1, &watchset, NULL, NULL, NULL, &set);
                 
        if(FD_ISSET(fdread, &watchset)) {
            ret = read(fdread, &cmd_data, size);
            if((size>0)&&(ret>0)) {
            
#ifdef __PERF_INSTRUMENTATION__
                PERF_ReceivedCommand(pPERF, cmd_data.RM_Cmd, cmd_data.param1,
                                     PERF_ModuleLLMM);
#endif
                // actually get data 
                RM_DPRINT("[Resource Manager] - get data\n");


                switch (cmd_data.RM_Cmd) {  
                    case RM_RequestResource:
                        HandleRequestResource(cmd_data);                                    
                        break;

                    case RM_WaitForResource:
                        HandleWaitForResource(cmd_data);
                        break;

                    case RM_FreeResource:
                        HandleFreeResource(cmd_data);
                        RM_RemoveComponentFromList(cmd_data.hComponent,cmd_data.nPid);
                        break;
                        
                    case RM_FreeAndCloseResource:
                        HandleFreeResource(cmd_data);
                        RM_ClosePipe(cmd_data);
                        RM_RemoveComponentFromList(cmd_data.hComponent,cmd_data.nPid);
                        break;


                    case RM_CancelWaitForResource:
                        HandleCancelWaitForResource(cmd_data);
                        break;
                                                
                    case RM_StateSet:
                        HandleStateSet(cmd_data);
                        break;
                                                
                    case RM_OpenPipe:
                        // create pipe for write
                        RM_itoa((int)cmd_data.nPid,rmsideHandleString);
                        strcpy(rmsideNamedPipeName,RM_SERVER_OUT);
                        strcat(rmsideNamedPipeName,"_");
                        strcat(rmsideNamedPipeName,rmsideHandleString);

#if 1 
                       // try to create the pipe, don't fail it already exists (reuse the pipe instead)
                        RM_DPRINT("[Resource Manager] - Create and open the write (out) pipe\n");
                        if((mknod(rmsideNamedPipeName,S_IFIFO|PERMS,0)<0) && (errno!=EEXIST))
                            RM_DPRINT("[Resource Manager] - mknod failure to create the write pipe, error=%d\n", errno);
                        //wait for the pipe to be established before opening it.??

                        RM_DPRINT("[Resource Manager] - Try opening the write out pipe for PID %d\n", (int)cmd_data.nPid);
#endif
                        if((fdwrite=open(rmsideNamedPipeName,O_WRONLY))<0) {
                            RM_DPRINT("[Resource Manager] - failure to open the WRITE pipe, errno=%d\n", errno);
                        }
                        else {
                            RM_DPRINT("[Resource Manager] - WRITE pipe opened successfully, Add pipe to the table\n");
                            RM_AddPipe(cmd_data,fdwrite);
                        }
                        break;
                        
                    case RM_ReusePipe:
                        RM_AddPipe(cmd_data,RM_GetPipe((OMX_HANDLETYPE)cmd_data.param4,cmd_data.nPid));
                        break;


                        case RM_Exit:
                        case RM_Init:
                        break;

                        case RM_ExitTI:
                            Exitflag = OMX_TRUE;
                            break;
                }  
            }
                else {
                close(fdread);
                if((fdread=open(RM_SERVER_IN,O_RDONLY))<0)                                
                    RM_DPRINT("[Audio Manager] - failure to re-open the Read pipe\n");
                RM_DPRINT("[Audio Manager] - re-opened Read pipe\n");
            }
        }
#ifndef __ENABLE_RMPM_STUB__        
        else if (FD_ISSET(pmfdread,&watchset)) {
            read(pmfdread,&policyresponse_data,sizeof(policyresponse_data));

            switch(policyresponse_data.PM_Cmd) {
                case PM_PREEMPTED:
                cmd_data.rm_status = RM_PREEMPT;
                cmd_data.hComponent = policyresponse_data.hComponent;
                RM_SetStatus(cmd_data.hComponent, cmd_data.nPid,RM_WaitingForClient);                
                int preemptpipe = RM_GetPipe(policyresponse_data.hComponent,policyresponse_data.nPid);
                if (write(preemptpipe,&cmd_data,sizeof(cmd_data)) < 0)
                    RM_DPRINT("Didn't write pipe\n");
                else
                    RM_DPRINT("Wrote RMProxy pipe\n");
                break;

                case PM_DENYPOLICY:
                    globalrequest_cmd_data.rm_status = RM_DENY;
                    RM_SetStatus(globalrequest_cmd_data.hComponent,globalrequest_cmd_data.nPid,RM_WaitingForClient);        
                    if(write(RM_GetPipe(globalrequest_cmd_data.hComponent,globalrequest_cmd_data.nPid), &globalrequest_cmd_data, sizeof(globalrequest_cmd_data)) < 0)
                        RM_DPRINT ("[Resource Manager] - failure write data back to component\n");
                    else
                        RM_DPRINT ("[Resource Manager] - Denied by policy, ok to write data back to component\n");
                    
                break;

                case PM_GRANTPOLICY:
                    /* if policy request is granted then check to see if resource is available */
                    m= (struct QOSRESOURCE_MEMORY *)DSPData_Create(QOSDataType_Memory_DynAlloc);
                    if (m) {
                        m->align = 4;
                        m->heapId = KAllHeaps;
                        m->size = cmd_data.param3;
                        MemoryIsPresent = DSPQos_TypeSpecific((struct QOSDATA *)registry,
							QOS_FN_HasAvailableResource, (unsigned int)(struct QOSDATA *)m);
                        status = DSPData_Delete((struct QOSDATA *) m);
                    }

                    /* if memory is available and DSP cycles are available grant request */
                    if (MemoryIsPresent && (cpuStruct.cyclesAvailable >= cmd_data.param2)) {
                        globalrequest_cmd_data.rm_status = RM_GRANT;
                        RM_SetStatus(globalrequest_cmd_data.hComponent,globalrequest_cmd_data.nPid,RM_ComponentActive);
                        if(write(RM_GetPipe(globalrequest_cmd_data.hComponent,globalrequest_cmd_data.nPid), &globalrequest_cmd_data, sizeof(globalrequest_cmd_data)) < 0)
                            RM_DPRINT ("[Resource Manager] - failure write data back to component\n");
                        else
                            RM_DPRINT ("[Resource Manager] - Granted by policy, ok to write data back to component\n");
                            
                    }
                    else {
                        policy_data.PM_Cmd = PM_FreeResources;
                        policy_data.param1 = globalrequest_cmd_data.param1;
                        policy_data.hComponent=globalrequest_cmd_data.hComponent;
                        policy_data.nPid = globalrequest_cmd_data.nPid;
                    
                        if (write(pmfdwrite,&policy_data,sizeof(policy_data)) < 0)
                            RM_DPRINT ("[Resource Manager] - failure write data to the policy manager\n");
                        else
                            RM_DPRINT ("[Resource Manager] - wrote the data to the policy manager\n");
                            
                    }

#ifdef __PERF_INSTRUMENTATION__
                PERF_SendingCommand(pPERF, cmd_data.RM_Cmd, cmd_data.param1, PERF_ModuleLLMM);
#endif     

                break;
                default: 
                break;
            }
        }
#endif        
        else {
            RM_DPRINT("[Resource Manager] fdread not ready\n"); 
        }
    }
        
    FreeQos();

    close(fdread);
    close(fdwrite);

    if(unlink(RM_SERVER_IN)<0)
        RM_DPRINT("[Resource Manager] - unlink RM_SERVER_IN error\n");


#ifdef __PERF_INSTRUMENTATION__
    PERF_Done(pPERF);
#endif
    exit(0);
}


/*
   Description : This function will initialize Qos
   
   Parameter   : 
   
   Return      : 
   
*/
OMX_ERRORTYPE InitializeQos()
{
    OMX_ERRORTYPE eError= OMX_ErrorNone;
#ifndef __ENABLE_RMPM_STUB__
    DSP_STATUS status = DSP_SOK;
    unsigned int uProcId = 0;	/* default proc ID is 0. */
    unsigned int index = 0;
    struct DSP_PROCESSORINFO dspInfo;
    unsigned int numProcs;
    DSP_HPROCESSOR hProc;
    char *qosdllname;
    
    
    qosdllname = getenv ("QOSDYN_FILE");
    if (qosdllname == NULL)
    {
        eError= OMX_ErrorHardware;
        fprintf (stderr, "[Resource Manager] - No QosDyn DLL file name\n");
        return eError;
    }
    fprintf (stderr, "[Resource Manager] Read QOSDYN file at: %s\n", qosdllname);

    status = DspManager_Open(0, NULL);
    if (DSP_FAILED(status)) {
        fprintf(stderr, "DSPManager_Open failed \n");
        return -1;
    } 
    while (DSP_SUCCEEDED(DSPManager_EnumProcessorInfo(index,&dspInfo,
        (unsigned int)sizeof(struct DSP_PROCESSORINFO),&numProcs))) {
        if ((dspInfo.uProcessorType == DSPTYPE_55) || 
            (dspInfo.uProcessorType == DSPTYPE_64)) {
            uProcId = index;
            status = DSP_SOK;
            break;
        }
        index++;
    }
    status = DSPProcessor_Attach(uProcId, NULL, &hProc);
    if (DSP_SUCCEEDED(status)) {
        status = DSPManager_RegisterObject(
                (struct DSP_UUID *)&QOS_TI_UUID,
                DSP_DCDNODETYPE, qosdllname);
        if (DSP_SUCCEEDED(status)) {
            /*  Register the node DLL. */
            status = DSPManager_RegisterObject(
                    &QOS_TI_UUID, DSP_DCDLIBRARYTYPE,
                    qosdllname);
            if (DSP_FAILED(status)) {
                fprintf (stderr, "[Resource Manager] - InitializeQos() -- DSPManager_RegisterObject() object fail=0x%x\n", (unsigned int)status);
                eError= OMX_ErrorHardware;
            }
        }
        else
        {
            fprintf (stderr, "[Resource Manager] - InitializeQos() -- DSPManager_RegisterObject() object fail=0x%x\n", (unsigned int)status);
            eError= OMX_ErrorHardware;
        }
    } 
    else
    {
        fprintf (stderr, "[Resource Manager] - InitializeQos() -- DSPProcessor_Attach() object fail=0x%x\n", (unsigned int)status);
        eError= OMX_ErrorHardware;

    }
    registry = DSPRegistry_Create();
    if ( !registry) {
        fprintf(stderr, "DSP RegistryCreate FAILED\n");
        eError= OMX_ErrorHardware;
    }

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, (void*)RM_CPULoadThread, NULL);
#endif

    return eError;
}
 

/*
   Description : This function will free Qos
   
   Parameter   : 
   
   Return      : 
   
*/
void FreeQos()
{
        RM_DPRINT ("[Resource Manager] - FreeQos() function call\n");
}

/*
   Description : This function will register Qos
   
   Parameter   : 
   
   Return      : 
   
*/
void RegisterQos()
{
        RM_DPRINT ("[Resource Manager] - RegisterQos() function call\n");
}

/*
   Description : This function will handle request resource
   
   Parameter   : 
   
   Return      : 
   
*/
void HandleRequestResource(RESOURCEMANAGER_COMMANDDATATYPE cmd)
{
#ifndef __ENABLE_RMPM_STUB__
    RM_DPRINT ("[Resource Manager] - HandleRequestResource() function call\n");

    policy_data.PM_Cmd = PM_RequestPolicy;
    policy_data.param1 = cmd.param1;
    policy_data.hComponent=cmd.hComponent;
    policy_data.nPid = cmd.nPid;
    globalrequest_cmd_data.hComponent = cmd.hComponent;
    globalrequest_cmd_data.nPid = cmd.nPid;
    globalrequest_cmd_data.RM_Cmd = RM_RequestResource;
    globalrequest_cmd_data.param1 = cmd.param1;
    if (write(pmfdwrite,&policy_data,sizeof(policy_data)) < 0)
         RM_DPRINT ("[Resource Manager] - failure write data to the policy manager\n");
    else
        RM_DPRINT ("[Resource Manager] - wrote the data to the policy manager\n");

#else
    /* if using stubbed implementation we won't check policy manager but will set the component to active */
    RM_SetStatus(cmd.hComponent,cmd.nPid,RM_ComponentActive);
#endif
}

/*
   Description : This fucntion will handle wait for resource
   
   Parameter   : 
   
   Return      : 
   
*/
void HandleWaitForResource(RESOURCEMANAGER_COMMANDDATATYPE cmd)
{
    int index;
    RM_DPRINT ("[Resource Manager] - HandleWaitForResource() function call\n");
    index = RM_GetListIndex(cmd.hComponent,cmd.nPid);

    if (componentList.component[index].reason == RM_ReasonPolicy) {
        RM_SetStatus(cmd.hComponent,cmd.nPid,RM_WaitingForPolicy);
    }
    else if (componentList.component[index].reason == RM_ReasonResource) {
        RM_SetStatus(cmd.hComponent,cmd.nPid,RM_WaitingForResource);
    }
    
#ifdef __PERF_INSTRUMENTATION__
    PERF_SendingCommand(pPERF, cmd_data.RM_Cmd, cmd_data.param1, PERF_ModuleLLMM);
#endif    // 

    if(write(RM_GetPipe(cmd_data.hComponent,cmd_data.nPid), &cmd_data, sizeof(cmd_data)) < 0)
        RM_DPRINT ("[Resource Manager] - failure write data back to component\n");
    else
        RM_DPRINT ("[Resource Manager] -sending wait for resources\n");

}


/*
   Description : This fucntion will free resource
   
   Parameter   : 
   
   Return      : 
   
*/
void HandleFreeResource(RESOURCEMANAGER_COMMANDDATATYPE cmd)
{
    int i;
    
    RM_DPRINT ("[Resource Manager] - RM_FreeResource() function call\n");
#ifndef __ENABLE_RMPM_STUB__
    policy_data.PM_Cmd = PM_FreePolicy;
    policy_data.hComponent = cmd.hComponent;
    policy_data.nPid = cmd.nPid;
    if (write(pmfdwrite,&policy_data,sizeof(policy_data)) < 0)
         RM_DPRINT ("[Resource Manager] - failure write data to the policy manager\n");
    else
        RM_DPRINT ("[Resource Manager] - wrote the data to the policy manager\n");
#endif        

    RM_DPRINT("componentList.numRegisteredComponents = %d\n",componentList.numRegisteredComponents);
    /* Now if there are pending components they might be able to run. */
    if (componentList.numRegisteredComponents-1 > 0) {
        for (i=0; i < componentList.numRegisteredComponents-1; i++) {
            RM_DPRINT("HandleFreeResource %d\n",__LINE__);
            if (componentList.component[i].componentState== OMX_StateWaitForResources) {
                /* temporarily now assume policy is available */
                RM_DPRINT("HandleFreeResource %d\n",__LINE__);
                cmd_data.rm_status = RM_RESOURCEACQUIRED;
                cmd_data.hComponent = componentList.component[i].componentHandle;
            
                RM_DPRINT("HandleFreeResource %d\n",__LINE__);
                write(RM_GetPipe(componentList.component[i].componentHandle,componentList.component[i].nPid), &cmd_data, sizeof(cmd_data));
            }
            else if (componentList.component[i].status == RM_WaitingForResource) {
            }
        }
    }
}

/*
   Description : This fucntion will cancel wait for resource
   
   Parameter   : 
   
   Return      : 
   
*/
void HandleCancelWaitForResource(RESOURCEMANAGER_COMMANDDATATYPE cmd)
{
        RM_DPRINT ("[Resource Manager] - RM_CancelWaitForResource() function call\n");
}

/*
   Description : This fucntion will set the resource state
   
   Parameter   : 
   
   Return      : 
   
*/
void HandleStateSet(RESOURCEMANAGER_COMMANDDATATYPE cmd)
{
    int i;
    int index=-1;
    OMX_STATETYPE previousState;
    OMX_STATETYPE newState;
    int componentType = 0;
    RM_DPRINT("HandleStateSet %d\n",__LINE__);

    switch (cmd.param1) {
        case OMX_MP3_Decoder_COMPONENT:
        case OMX_AAC_Decoder_COMPONENT:
        case OMX_AAC_Encoder_COMPONENT:
        case OMX_ARMAAC_Encoder_COMPONENT:
        case OMX_ARMAAC_Decoder_COMPONENT:
        case OMX_PCM_Decoder_COMPONENT:
        case OMX_PCM_Encoder_COMPONENT:
        case OMX_NBAMR_Decoder_COMPONENT:
        case OMX_NBAMR_Encoder_COMPONENT:
        case OMX_WBAMR_Decoder_COMPONENT:
        case OMX_WBAMR_Encoder_COMPONENT:
        case OMX_WMA_Decoder_COMPONENT:
        case OMX_G711_Decoder_COMPONENT:
        case OMX_G711_Encoder_COMPONENT:
        case OMX_G722_Decoder_COMPONENT:
        case OMX_G722_Encoder_COMPONENT:
        case OMX_G723_Decoder_COMPONENT:
        case OMX_G723_Encoder_COMPONENT:
        case OMX_G726_Decoder_COMPONENT:
        case OMX_G726_Encoder_COMPONENT:
        case OMX_G729_Decoder_COMPONENT:
        case OMX_G729_Encoder_COMPONENT:
        case OMX_GSMFR_Decoder_COMPONENT:
        case OMX_GSMHR_Decoder_COMPONENT:
        case OMX_GSMFR_Encoder_COMPONENT:
        case OMX_GSMHR_Encoder_COMPONENT:
        case OMX_ILBC_Decoder_COMPONENT:
        case OMX_ILBC_Encoder_COMPONENT:
        case OMX_RAGECKO_Decoder_COMPONENT:
            componentType = RM_AUDIO;
        break;

    /* video*/
        case OMX_MPEG4_Decode_COMPONENT:
        case OMX_MPEG4_Encode_COMPONENT:
        case OMX_H263_Decode_COMPONENT:
        case OMX_H263_Encode_COMPONENT:
        case OMX_H264_Decode_COMPONENT:
        case OMX_H264_Encode_COMPONENT:
        case OMX_WMV_Decode_COMPONENT:
        case OMX_MPEG2_Decode_COMPONENT:
            componentType = RM_VIDEO;
        break;

    /* image*/
        case OMX_JPEG_Decoder_COMPONENT:
        case OMX_JPEG_Encoder_COMPONENT:
        case OMX_VPP_COMPONENT:
            componentType = RM_IMAGE;
        break;

    /* camera*/
        case OMX_CAMERA_COMPONENT:
            componentType = RM_CAMERA;
        break;

        /* lcd */
        case OMX_DISPLAY_COMPONENT:
            componentType = RM_LCD;
        break;


        default:
            RM_DPRINT("[HandleStateSet] Unknown component type\n");
            componentType = -1;
        break;
        
    }

    
    RM_DPRINT ("[Resource Manager] - HandleStateSet() function call\n");

    for (i=0; i < componentList.numRegisteredComponents; i++) {
        if (componentList.component[i].componentHandle == cmd.hComponent && cmd.nPid == componentList.component[i].nPid) {
            index = i;
            break;
        }        
    }
    
    if (index != -1) {
        previousState = componentList.component[index].componentState;
        newState = cmd.param2;
        componentList.component[index].componentState = newState;
        if ((previousState == OMX_StateIdle || previousState == OMX_StatePause) && newState == OMX_StateExecuting) {
            /* If component is transitioning from Idle to Executing update the 
                 totalCpu usage of all of the components */
            if (componentType == RM_AUDIO) {
                audioTotalCpu += componentList.component[index].componentCPU;
            }
            else if (componentType == RM_VIDEO) {
                videoTotalCpu += componentList.component[index].componentCPU;
            }
            else if (componentType == RM_IMAGE) {
                imageTotalCpu += componentList.component[index].componentCPU;
            }
            else if (componentType == RM_CAMERA) {
                cameraTotalCpu += componentList.component[index].componentCPU;
            }
            else if (componentType == RM_LCD) {
                lcdTotalCpu += componentList.component[index].componentCPU;
            }
            else {
                RM_DPRINT("ERROR - unknown component type\n");
            }

            totalCpu = audioTotalCpu + videoTotalCpu + imageTotalCpu + cameraTotalCpu + lcdTotalCpu;
            /* Inform the Resource Activity Monitor of the new CPU usage */
            omap_pm_set_constraint(componentType,totalCpu);        
        }
        else if (previousState == OMX_StateExecuting && (newState == OMX_StateIdle || newState == OMX_StatePause)) {

            /* If component is transitioning from Executing to Idle update the 
                 totalCpu usage of all of the components */
            if (componentType == RM_AUDIO) {
                audioTotalCpu -= componentList.component[index].componentCPU;
            }
            else if (componentType == RM_VIDEO) {
                videoTotalCpu -= componentList.component[index].componentCPU;
            }
            else if (componentType == RM_IMAGE) {
                imageTotalCpu -= componentList.component[index].componentCPU;
            }
            else if (componentType == RM_CAMERA) {
                cameraTotalCpu -= componentList.component[index].componentCPU;
            }
            else if (componentType == RM_LCD) {
                lcdTotalCpu -= componentList.component[index].componentCPU;
            }
            else {
                RM_DPRINT("ERROR - unknown component type\n");
            }
            
            /* Inform the Resource Activity Monitor of the new CPU usage */
            totalCpu = audioTotalCpu + videoTotalCpu + imageTotalCpu + cameraTotalCpu + lcdTotalCpu;
            
            omap_pm_set_constraint(componentType,totalCpu);
        }
        RM_DPRINT("newState = %d\n",newState);
        if (newState == OMX_StateWaitForResources) {
            HandleWaitForResource(cmd);
        }
    }
    else {
        RM_DPRINT("ERROR: Component is not registered\n");
    }
}


void RM_AddPipe(RESOURCEMANAGER_COMMANDDATATYPE cmd, int aPipe)
{
    int alreadyRegistered = 0;
    int i;

    for (i=0; i < componentList.numRegisteredComponents; i++) {
        if (cmd.hComponent == componentList.component[i].componentHandle && cmd.nPid == componentList.component[i].nPid) {
            alreadyRegistered = 1;
            break;
        }
    }

    if (!alreadyRegistered) {
        componentList.component[componentList.numRegisteredComponents].componentHandle = cmd.hComponent;
        componentList.component[componentList.numRegisteredComponents].nPid = cmd.nPid;
        componentList.component[componentList.numRegisteredComponents].componentPipe = aPipe;
        componentList.component[componentList.numRegisteredComponents].componentState = OMX_StateIdle;
        componentList.component[componentList.numRegisteredComponents].reason = RM_ReasonNone;
        componentList.component[componentList.numRegisteredComponents++].componentCPU = cmd.param2;
        
    }
}


int RM_GetPipe(OMX_HANDLETYPE hComponent,OMX_U32 aPid)
{
    int returnPipe=-1;
    int listIndex;
    listIndex = RM_GetListIndex(hComponent,aPid);
    if (listIndex != -1) {
        returnPipe = componentList.component[listIndex].componentPipe;
    }
    return returnPipe;
}


void RM_ClosePipe(RESOURCEMANAGER_COMMANDDATATYPE cmd_data)
{
 
    int listIndex;
    listIndex = RM_GetListIndex(cmd_data.hComponent,cmd_data.nPid);
    /* If pipe is found close it */
    if (listIndex != -1) {
        fdwrite = componentList.component[listIndex].componentPipe;
        close(fdwrite);
    }
 }


void RM_itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    RM_reverse(s);
} 



void RM_reverse(char s[])
{
    int c, i, j;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

int RM_SetStatus(OMX_HANDLETYPE hComponent, OMX_U32 aPid, RM_COMPONENTSTATUS status)
{
    int listIndex;
    listIndex = RM_GetListIndex(hComponent,aPid);

    if (listIndex != -1) {
        componentList.component[listIndex].status = status;
    }

    return 0;
}

 int RM_SetReason(OMX_HANDLETYPE hComponent, RM_DENYREASON reason)
{
    int listIndex;
    listIndex = RM_GetListIndex(hComponent,0);
    
    if (listIndex != -1) {
        componentList.component[listIndex].reason= reason;
    }

    return 0;
}
 
int RM_GetListIndex(OMX_HANDLETYPE hComponent, OMX_U32 aPid) 
{
    int i;
    int match=-1;

    for (i=0; i < componentList.numRegisteredComponents; i++) {
        if (hComponent == componentList.component[i].componentHandle && aPid == componentList.component[i].nPid) {
            match = i;
            break;
        }
    }
    return match;
}


int RM_RemoveComponentFromList(OMX_HANDLETYPE hComponent,OMX_U32 aPid)
{
    int index = -1;
    int i;
    index = RM_GetListIndex(hComponent,aPid);

    if (index != -1) {
        /* Shift all other components in the list up */    
        for(i=index; i < componentList.numRegisteredComponents-1; i++) {
            componentList.component[i].componentCPU = componentList.component[i+1].componentCPU;            
            componentList.component[i].componentHandle = componentList.component[i+1].componentHandle;            
            componentList.component[i].componentState = componentList.component[i+1].componentState;
            componentList.component[i].componentPipe = componentList.component[i+1].componentPipe;
            componentList.component[i].reason = componentList.component[i+1].reason;
            componentList.component[i].status = componentList.component[i+1].status;
            componentList.component[i].nPid = componentList.component[i+1].nPid;
        }

        /* Decrement the count of registered components */
        componentList.numRegisteredComponents--;
    }

    return 0;
}

int Install_Bridge()
{
    int fd;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char *filename = "/tmp/bridgeinstalled";

    system("insmod  /dspbridge/bridgedriver.ko phys_mempool_base=0x87000000  phys_mempool_size=0x600000 shm_size=0x40f000");
    system("mdev -s");

    fd = creat(filename, mode);

    printf("Bridge Installed\n");
    return 0;
}

int Uninstall_Bridge()
{
    system("rmmod bridgedriver");
    system("rm -f /dev/DspBridge");

    printf("Bridge Uninstalled \n");    
    return 0;
}


int LoadBaseimage()
{
    int fd;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char *filename = "/tmp/baseimageloaded";

        struct stat sb = {0};

    while (stat("/tmp/bridgeinstalled",&sb)) {
        sched_yield();
    }
/*    unlink("/tmp/bridgeinstalled");*/
    unsigned int uProcId = 0;	/* default proc ID is 0. */
    unsigned int index = 0;
    struct DSP_PROCESSORINFO dspInfo;
    DSP_HPROCESSOR hProc;
    DSP_STATUS status = DSP_SOK;
    unsigned int numProcs;
    char* argv[2];

    argv[0] = "/lib/dsp/baseimage.dof";
    status = DspManager_Open(0, NULL);
    if (DSP_FAILED(status)) {
        printf("DSPManager_Open failed \n");
        return -1;
    } 
    while (DSP_SUCCEEDED(DSPManager_EnumProcessorInfo(index,&dspInfo,
        (unsigned int)sizeof(struct DSP_PROCESSORINFO),&numProcs))) {
        if ((dspInfo.uProcessorType == DSPTYPE_55) || 
            (dspInfo.uProcessorType == DSPTYPE_64)) {
            uProcId = index;
            status = DSP_SOK;
            break;
        }
        index++;
    }
    status = DSPProcessor_Attach(uProcId, NULL, &hProc);
    if (DSP_SUCCEEDED(status)) {
        status = DSPProcessor_Stop(hProc);
        if (DSP_SUCCEEDED(status)) {
            status = DSPProcessor_Load(hProc,1,(const char **)argv,NULL);
            if (DSP_SUCCEEDED(status)) {
                status = DSPProcessor_Start(hProc);
                if (DSP_SUCCEEDED(status)) {
                } 
                else {
                }
            } 
			else {
            }
            DSPProcessor_Detach(hProc);
        }
    }
    else {
    }
    fd = creat(filename, mode);
    
    printf("Baseimage Loaded\n");
    return 0;		
}

void ReloadBaseimage()
{
}




void *RM_CPULoadThread(int pipeToWatch)
{

#ifndef __ENABLE_RMPM_STUB__
    unsigned long NumFound;
    int i=0;
    int sum=0;
    DSP_STATUS status = DSP_SOK;
    int currentOverallUtilization=0;
    unsigned int dsp_currload=0, dsp_predload=0, dsp_currfreq=0, dsp_predfreq=0;
    cpuStruct.snapshotsCaptured = 0;
    cpuStruct.averageCpuLoad = 0;

    /* initialize array */
    for (i=0; i < RM_CPUAVGDEPTH; i++) {
        cpuStruct.cpuLoadSnapshots[i] = 0;
    }
    
    while (1) {
        results = NULL;
        NumFound = 0;

        /* get the ARM maximum operating point */
        FILE *fp = fopen("/sys/power/vdd1_opp_value","r");
        if (fp == NULL) RM_DPRINT("open file failed\n");
        int op;
        fscanf(fp, "%d",&op);
        fclose(fp);

        int maxMhz=0; 

        switch (op) {
            case RM_OPERATING_POINT_1 : 
                maxMhz = RM_OPERATING_POINT_1_MHZ;
            break;

            case RM_OPERATING_POINT_2 : 
                maxMhz = RM_OPERATING_POINT_2_MHZ;
            break;

            case RM_OPERATING_POINT_3: 
                maxMhz = RM_OPERATING_POINT_3_MHZ;
            break;

            case RM_OPERATING_POINT_4: 
                maxMhz = RM_OPERATING_POINT_4_MHZ;
            break;

            case RM_OPERATING_POINT_5: 
                maxMhz = RM_OPERATING_POINT_5_MHZ;
            break;

            default:
                RM_DPRINT("RM read incorrect operating point\n");
            break;
        }


        status = QosTI_GetProcLoadStat (&dsp_currload, &dsp_predload, &dsp_currfreq, &dsp_predfreq);
        if (DSP_SUCCEEDED(status)) 
        {
            /* get the dsp load from the Qos call without a DSP wake up*/
            currentOverallUtilization = dsp_currload * maxMhz / RM_OPERATING_POINT_5_MHZ;
        } 
        else 
        {
            /* get the dsp load from the Registry call by a DSP wake up*/
            status = DSPRegistry_Find(QOSDataType_Processor_C6X, registry, results, &NumFound);
            if ( !DSP_SUCCEEDED(status) && status != DSP_ESIZE) {
                RM_DPRINT("None.\n\n");
            }

            results = malloc(NumFound * sizeof (struct QOSDATA *));
            if (!results) {
                RM_DPRINT("FAILED (out of memory)\n\n");
            }

            /* Get processor usage */
            status = DSPRegistry_Find(QOSDataType_Processor_C6X, registry, results, &NumFound);
            if (DSP_SUCCEEDED(status)) {
            } 
            else {
                NumFound = 0;
            }
            p = (struct  QOSRESOURCE_PROCESSOR *) results[0];
            currentOverallUtilization = p->currentLoad * maxMhz / RM_OPERATING_POINT_5_MHZ;
        }

        /* If we have not yet captured RM_CPUAVGDEPTH samples add this to the next slot in the array */
        if (cpuStruct.snapshotsCaptured < RM_CPUAVGDEPTH) {
            cpuStruct.cpuLoadSnapshots[cpuStruct.snapshotsCaptured++] = currentOverallUtilization;
        }
        else {
            /* If the array is now full, shift the existing entries of the array */
            for (i=0; i < RM_CPUAVGDEPTH-1; i++) {
                cpuStruct.cpuLoadSnapshots[i] = cpuStruct.cpuLoadSnapshots[i+1];
            }
            /* ...and then put the most recent value in the last slot in the array */            
            cpuStruct.cpuLoadSnapshots[RM_CPUAVGDEPTH-1] = currentOverallUtilization;
        }

        /* Calculate the average */
        sum = 0;
        for (i=0; i < cpuStruct.snapshotsCaptured; i++) {
            sum += cpuStruct.cpuLoadSnapshots[i];
        }
        cpuStruct.averageCpuLoad = sum / cpuStruct.snapshotsCaptured;
        cpuStruct.cyclesInUse = ((cpuStruct.averageCpuLoad * RM_OPERATING_POINT_5_MHZ) / 100);
        cpuStruct.cyclesAvailable = RM_OPERATING_POINT_5_MHZ - cpuStruct.cyclesInUse;

        /* wait for RM_CPUAVERAGEDELAY seconds */
        sleep(RM_CPUAVERAGEDELAY);
        free(results);
        }

#endif /*  __ENABLE_RMPM_STUB__  */

    return NULL;
}


void *RM_FatalErrorWatchThread()
{

    DSP_STATUS status = DSP_SOK;
    unsigned int index=0;
    struct DSP_NOTIFICATION * notificationObjects[2];
    DSP_HPROCESSOR hProc;
    struct DSP_NOTIFICATION* notification_mmufault;
    struct DSP_NOTIFICATION* notification_syserror ;
    int i;

    status = DSPProcessor_Attach(0, NULL, &hProc);
           
    notification_mmufault = (struct DSP_NOTIFICATION*)malloc(sizeof(struct DSP_NOTIFICATION));
    if(notification_mmufault == NULL) {
        RM_DPRINT("%d :: malloc failed....\n",__LINE__);
    }
    memset(notification_mmufault,0,sizeof(struct DSP_NOTIFICATION));

    status = DSPProcessor_RegisterNotify(hProc, DSP_MMUFAULT, DSP_SIGNALEVENT, notification_mmufault);
    DSP_ERROR_EXIT(status, "DSP node register notify DSP_MMUFAULT", EXIT);
    notificationObjects[0] =  notification_mmufault;

    notification_syserror = (struct DSP_NOTIFICATION*)malloc(sizeof(struct DSP_NOTIFICATION));
    if(notification_syserror == NULL) {
        RM_DPRINT("%d :: malloc failed....\n",__LINE__);
    }
    memset(notification_syserror,0,sizeof(struct DSP_NOTIFICATION));
            
    status = DSPProcessor_RegisterNotify(hProc, DSP_SYSERROR, DSP_SIGNALEVENT, notification_syserror);
    DSP_ERROR_EXIT(status, "DSP node register notify DSP_SYSERROR", EXIT);
    notificationObjects[1] =  notification_syserror;

    while (1) {
    status = DSPManager_WaitForEvents(notificationObjects, 2, &index, 1000000);
    if (DSP_SUCCEEDED(status)) {
        if (index == 0 || index == 1){
            /* exception received - start telling all components to close */
            for(i=0; i < componentList.numRegisteredComponents; i++) {
                cmd_data.rm_status = RM_RESOURCEFATALERROR;

                cmd_data.hComponent = componentList.component[i].componentHandle;
                cmd_data.nPid = componentList.component[i].nPid;
                RM_SetStatus(cmd_data.hComponent, cmd_data.nPid,RM_WaitingForClient);  
                int preemptpipe = RM_GetPipe(cmd_data.hComponent,cmd_data.nPid);
                if (write(preemptpipe,&cmd_data,sizeof(cmd_data)) < 0)
                    RM_DPRINT("Didn't write pipe\n");
                else
                    RM_DPRINT("Wrote RMProxy pipe\n");
            }

            /* detach processor from gpp */
            status = DSPProcessor_Detach(hProc);
            DSP_ERROR_EXIT (status, "DeInit: DSP Processor Detach ", EXIT);
            status = DspManager_Close(0, NULL);
            DSP_ERROR_EXIT (status, "DeInit: DSPManager Close ", EXIT);
//            FreeQos();
            while (componentList.numRegisteredComponents != 0) {
                sleep(1);
            }

            /* After all components finish shutting down, uninstall bridge, reinstall bridge, and reload baseimage */
            Uninstall_Bridge();
            Install_Bridge();
            LoadBaseimage();
            
        }
    }
    }
EXIT:    
    return NULL;
}


