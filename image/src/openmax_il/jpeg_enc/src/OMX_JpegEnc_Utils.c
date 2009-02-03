
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
/* ====================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
* ==================================================================== */
/**
* @file OMX_JpegEnc_Utils.c
*
* This file implements OMX Component for JPEG encoder that
* is fully compliant with the OMX specification 1.5.
*
* @path  $(CSLPATH)\src
*
* @rev  0.1
*/
/* -------------------------------------------------------------------------------- */
/* ================================================================================
*!
*! Revision History
*! ===================================
*!
*! 22-May-2006 mf: Revisions appear in reverse chronological order;
*! that is, newest first.  The date format is dd-Mon-yyyy.
* ================================================================================= */

/****************************************************************
*  INCLUDE FILES
****************************************************************/

/* ----- System and Platform Files ----------------------------*/

#ifdef UNDER_CE 
#include <windows.h>
#include <oaf_osal.h>
#include <omx_core.h>
#else
#include <wchar.h>
#include <unistd.h>
#include <sys/types.h>
#include <malloc.h>
#include <memory.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <dbapi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <OMX_Index.h>
#include <OMX_Image.h>
#include <OMX_Audio.h>
#include <OMX_Video.h>
#include <OMX_IVCommon.h>
#include <OMX_Other.h>

#include <usn.h>

/*------- Program Header Files ----------------------------------------*/

#include "OMX_JpegEnc_Utils.h"
#include "OMX_JpegEnc_CustomCmd.h"

#ifdef RESOURCE_MANAGER_ENABLED
    #include <ResourceManagerProxyAPI.h>
#endif

#define JPEGENC_TIMEOUT 0xFFFFFFFE
                        
static OMX_ERRORTYPE HandleJpegEncInternalFlush(JPEGENC_COMPONENT_PRIVATE *pComponentPrivate, OMX_U32 nParam1);

#ifdef RESOURCE_MANAGER_ENABLED
void ResourceManagerCallback(RMPROXY_COMMANDDATATYPE cbData);
#endif

#ifdef UNDER_CE
void sleep(DWORD Duration)
{
    Sleep(Duration);
}
#endif

/*--------function prototypes ---------------------------------*/
OMX_ERRORTYPE JpegEncLCML_Callback (TUsnCodecEvent event,
                                    void * args [10]);

/*-------- Function Implementations ---------------------------------*/
OMX_ERRORTYPE GetJpegEncLCMLHandle(OMX_HANDLETYPE pComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
#ifndef UNDER_CE
    OMX_HANDLETYPE LCML_pHandle;
    
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    JPEGENC_COMPONENT_PRIVATE *pComponentPrivate = (JPEGENC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;
    fpo fpGetHandle ;
    void *handle = NULL;
    char *error =NULL;

    JPEGENC_DPRINT("Inside GetLCMLHandle function\n");

    handle = dlopen("libLCML.so", RTLD_LAZY);
    if ( !handle ) {
        fputs(dlerror(), stderr);
        eError = OMX_ErrorComponentNotFound;
        goto EXIT;
    }

    fpGetHandle = dlsym (handle, "GetHandle");
    if ( (error = dlerror()) != NULL ) {
        fputs(error, stderr);
        eError = OMX_ErrorInvalidComponent;
        goto EXIT;
    }

    /*  calling gethandle and passing phandle to b filled   */
    eError = (*fpGetHandle)(&LCML_pHandle);

    if ( eError != OMX_ErrorNone ) {
        eError = OMX_ErrorUndefined;
        JPEGENC_DPRINT("eError != OMX_ErrorNone...\n");
        goto EXIT;
    }

    JPEGENC_DPRINT("Received LCML Handle\n");

    pComponentPrivate->pDllHandle = handle;
    pComponentPrivate->pLCML = (void *)LCML_pHandle;
    pComponentPrivate->pLCML->pComponentPrivate = (JPEGENC_COMPONENT_PRIVATE *)pComponentPrivate;
#else
    typedef OMX_ERRORTYPE (*LPFNDLLFUNC1)(OMX_HANDLETYPE);  
    LPFNDLLFUNC1 fpGetHandle1;
    OMX_HANDLETYPE LCML_pHandle = NULL;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent; 
    JPEGENC_COMPONENT_PRIVATE *pComponentPrivate = (JPEGENC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;
    HINSTANCE hDLL; 
    
    hDLL = LoadLibraryEx(TEXT("OAF_BML.dll"), NULL, 0);
    if (hDLL == NULL)
    {
        eError = OMX_ErrorComponentNotFound;
        goto EXIT;
    }
    
    fpGetHandle1 = (LPFNDLLFUNC1)GetProcAddress(hDLL,TEXT("GetHandle"));
    if (!fpGetHandle1)
    {
        
        FreeLibrary(hDLL);
        eError = OMX_ErrorComponentNotFound;
        goto EXIT;
    }
    
    
    eError = fpGetHandle1(&LCML_pHandle);
    if(eError != OMX_ErrorNone) {

        eError = OMX_ErrorUndefined;
        LCML_pHandle = NULL;
          goto EXIT;
    }
    
    (LCML_DSP_INTERFACE*)pComponentPrivate->pLCML = (LCML_DSP_INTERFACE *)LCML_pHandle;
    pComponentPrivate->pLCML->pComponentPrivate = (JPEGENC_COMPONENT_PRIVATE *)pComponentPrivate;

#endif
    JPEGENC_DPRINT("Exit\n");
    EXIT:
    return eError;
}



/*-----------------------------------------------------------------------------*/
/**
  * Disable Port()
  *
  * Called by component thread, handles commands sent by the app.
  *
  * @param
  *
  * @retval OMX_ErrorNone                  success, ready to roll
  *
  **/
/*-----------------------------------------------------------------------------*/
OMX_ERRORTYPE JpegEncDisablePort (JPEGENC_COMPONENT_PRIVATE* pComponentPrivate, OMX_U32 nParam1)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_CHECK_PARAM(pComponentPrivate);

    JPEGENC_DPRINT("Inside DisablePort function\n");

    JPEGENC_DPRINT("Inside disable port (%d) %d %d %d %d\n", 
            nParam1, 
            pComponentPrivate->nInPortIn, 
            pComponentPrivate->nInPortOut,
            pComponentPrivate->nOutPortIn,
            pComponentPrivate->nOutPortOut);

   if (pComponentPrivate->nCurState == OMX_StateExecuting || pComponentPrivate->nCurState == OMX_StatePause) {
       if ((nParam1 == JPEGENC_INP_PORT) || (nParam1 == JPEGENC_OUT_PORT) || (nParam1 == -1)) {
           eError = HandleJpegEncInternalFlush(pComponentPrivate, nParam1);
       } 
   }
 
EXIT:
    JPEGENC_DPRINT("Exit form JPEGEnc Disable Port eError is = %x\n",eError);
    return eError;
}



/*-----------------------------------------------------------------------------*/
/**
  * Enable Port()
  *
  * Called by component thread, handles commands sent by the app.
  *
  * @param
  *
  * @retval OMX_ErrorNone                  success, ready to roll
  *
  **/
/*-----------------------------------------------------------------------------*/

OMX_ERRORTYPE JpegEncEnablePort (JPEGENC_COMPONENT_PRIVATE* pComponentPrivate, OMX_U32 nParam1)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_CHECK_PARAM(pComponentPrivate);

    JPEGENC_DPRINT("Inside EnablePort function\n");

    JPEGENC_DPRINT("in enable port\n");

    if (nParam1 == 0) {
        if (pComponentPrivate->nCurState != OMX_StateLoaded) {
        pthread_mutex_lock(&pComponentPrivate->jpege_mutex_app);
        while (!pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pPortDef->bPopulated) {
            JPEGENC_DPRINT("%d in cond wait\n", __LINE__);
            pthread_cond_wait(&pComponentPrivate->populate_cond, &pComponentPrivate->jpege_mutex_app);
        }
        }
        pthread_mutex_unlock(&pComponentPrivate->jpege_mutex_app);
        pComponentPrivate->cbInfo.EventHandler (pComponentPrivate->pHandle,
                                                pComponentPrivate->pHandle->pApplicationPrivate,
                                                OMX_EventCmdComplete,
                                                OMX_CommandPortEnable,
                                                JPEGENC_INP_PORT,
                                                NULL);
    } else if (nParam1 == 1) {
        if (pComponentPrivate->nCurState != OMX_StateLoaded) {
        pthread_mutex_lock(&pComponentPrivate->jpege_mutex_app);
        while (!pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pPortDef->bPopulated) {
            JPEGENC_DPRINT("%d in cond wait\n", __LINE__);
            pthread_cond_wait(&pComponentPrivate->populate_cond, &pComponentPrivate->jpege_mutex_app);
        }
        }
        pthread_mutex_unlock(&pComponentPrivate->jpege_mutex_app);
        pComponentPrivate->cbInfo.EventHandler (pComponentPrivate->pHandle,
                                                pComponentPrivate->pHandle->pApplicationPrivate,
                                                OMX_EventCmdComplete,
                                                OMX_CommandPortEnable,
                                                JPEGENC_OUT_PORT,
                                                NULL);
    } else if (nParam1 == -1) {
        if (pComponentPrivate->nCurState != OMX_StateLoaded) {
        pthread_mutex_lock(&pComponentPrivate->jpege_mutex_app);
        while ((!pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pPortDef->bPopulated) ||
               (!pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pPortDef->bPopulated)) {
            JPEGENC_DPRINT("%d in cond wait\n", __LINE__);
            pthread_cond_wait(&pComponentPrivate->populate_cond, &pComponentPrivate->jpege_mutex_app);
        }
        pthread_mutex_unlock(&pComponentPrivate->jpege_mutex_app);
        }
        pComponentPrivate->cbInfo.EventHandler (pComponentPrivate->pHandle,
                                                pComponentPrivate->pHandle->pApplicationPrivate,
                                                OMX_EventCmdComplete,
                                                OMX_CommandPortEnable,
                                                JPEGENC_INP_PORT,
                                                NULL);
                                                
        pComponentPrivate->cbInfo.EventHandler (pComponentPrivate->pHandle,
                                                pComponentPrivate->pHandle->pApplicationPrivate,
                                                OMX_EventCmdComplete,
                                                OMX_CommandPortEnable,
                                                JPEGENC_OUT_PORT,
                                                NULL);
    }
EXIT:
    return eError;
}



/*-------------------------------------------------------------------*/
/**
  *  JPEGEnc_Start_ComponentThread() Starts Component Thread
  *
  *  Creates data pipes, commmand pipes and initializes Component thread
  *
  * @param pComponent    handle for this instance of the component
  *
  * @retval OMX_NoError              Success, ready to roll
  * @retval OMX_ErrorInsufficientResources    Insiffucient Resources
  *
  **/
/*-------------------------------------------------------------------*/
OMX_ERRORTYPE JPEGEnc_Start_ComponentThread(OMX_HANDLETYPE pComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = NULL;
    JPEGENC_COMPONENT_PRIVATE *pComponentPrivate = NULL;

    JPEGENC_DPRINT("Inside JPEGEnc_Start_ComponentThread function\n");

    OMX_CHECK_PARAM(pComponent);
    pHandle = (OMX_COMPONENTTYPE *)pComponent;
    pComponentPrivate = (JPEGENC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;

    
    /* create the pipe used to maintain free input buffers*/

    /* create the pipe used to maintain free output buffers*/
    eError = pipe (pComponentPrivate->free_outBuf_Q);
    if ( eError ) {
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    /* create the pipe used to maintain filled input buffers*/
    eError = pipe (pComponentPrivate->filled_inpBuf_Q);
    if ( eError ) {
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    /* create the pipe used to maintain dsp output/encoded buffers*/

    /* create the pipe used to send commands to the thread */
    eError = pipe (pComponentPrivate->nCmdPipe);
    if ( eError ) {
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    /* create the pipe used to send commands to the thread */
    eError = pipe (pComponentPrivate->nCmdDataPipe);
    if ( eError ) {
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    /*  No buffers have been sent to dsp    */
    pComponentPrivate->nNum_dspBuf = 0;

    /* Create the Component Thread */
    eError = pthread_create (&(pComponentPrivate->ComponentThread), NULL,
                             OMX_JpegEnc_Thread, pComponent);


    if ( eError || !pComponentPrivate->ComponentThread ) {
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

#ifdef __PERF_INSTRUMENTATION__
        PERF_ThreadCreated(pComponentPrivate->pPERF,
                           pComponentPrivate->ComponentThread,
                           PERF_FOURCC('J','P','E','T'));

#endif

    EXIT:
    return eError;
}



/* -------------------------------------------------------------------------- */
/**
* @JPEGEnc_Free_ComponentResources() This function is called by the component during
* de-init to close component thread, Command pipe, data pipe & LCML pipe.
*
* @param pComponent  handle for this instance of the component
*
* @pre
*
* @post
*
* @return none
*/
/* -------------------------------------------------------------------------- */
OMX_ERRORTYPE JPEGEnc_Free_ComponentResources(JPEGENC_COMPONENT_PRIVATE *pComponentPrivate)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_ERRORTYPE threadError = OMX_ErrorNone;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMX_ERRORTYPE pipeError = OMX_ErrorNone;
    int pthreadError = 0;
    int nCount = 0;
    OMX_COMMANDTYPE eCmd = OMX_CustomCommandStopThread;
    OMX_U32 nParam = 0;
    OMX_U8 *p;

#ifdef __PERF_INSTRUMENTATION__
        PERF_Boundary(pComponentPrivate->pPERF,
                      PERF_BoundaryStart | PERF_BoundaryCleanup);
#endif

    JPEGENC_DPRINT ("Inside JPEGEnc_Free_ComponentResources function\n");
    OMX_CHECK_PARAM(pComponentPrivate);

    if ( pComponentPrivate && 
         pComponentPrivate->isLCMLActive ==1 ) {
        LCML_ControlCodec(((LCML_DSP_INTERFACE*)pComponentPrivate->pLCML)->pCodecinterfacehandle,EMMCodecControlDestroy,NULL);
        dlclose(pComponentPrivate->pDllHandle);
        pComponentPrivate->isLCMLActive = 0;
    }

    pipeError = write(pComponentPrivate->nCmdPipe[1], &eCmd, sizeof(eCmd));
    if (pipeError == -1) {
        eError = OMX_ErrorHardware;
        JPEGENC_DPRINT("Error while writing to nCmdPipe\n");
    }
    
    pipeError = write(pComponentPrivate->nCmdDataPipe[1], &nParam, sizeof(eCmd));
    if (pipeError == -1) {
        eError = OMX_ErrorHardware;
        JPEGENC_DPRINT("Error while writing to nCmdPipe\n");
    }
    
    pthreadError = pthread_join (pComponentPrivate->ComponentThread,
                                 (void*)&threadError);
    if ( 0 != pthreadError ) {
        eError = OMX_ErrorHardware;
        JPEGENC_DPRINT ("Error while closing Component Thread\n");
    }
    
    if ( OMX_ErrorNone != threadError && OMX_ErrorNone != eError ) {
        eError = OMX_ErrorInsufficientResources;
        JPEGENC_DPRINT ("Error while closing Component Thread\n");
    }
    
    /*  close the data pipe handles */
    
    err = close (pComponentPrivate->free_outBuf_Q[0]);
    if ( 0 != err && OMX_ErrorNone == eError ) {
        eError = OMX_ErrorHardware;
        JPEGENC_DPRINT ("Error while closing data pipe\n");
    }
    
    err = close (pComponentPrivate->filled_inpBuf_Q[0]);
    if ( 0 != err && OMX_ErrorNone == eError ) {
        eError = OMX_ErrorHardware;
        JPEGENC_DPRINT ("Error while closing data pipe\n");
    }

    err = close (pComponentPrivate->free_outBuf_Q[1]);
    if ( 0 != err && OMX_ErrorNone == eError ) {
        eError = OMX_ErrorHardware;
        JPEGENC_DPRINT ("Error while closing data pipe\n");
    }

    err = close (pComponentPrivate->filled_inpBuf_Q[1]);
    if ( 0 != err && OMX_ErrorNone == eError ) {
        eError = OMX_ErrorHardware;
        JPEGENC_DPRINT ("Error while closing data pipe\n");
    }

    /*  Close the command pipe handles  */
    err = close (pComponentPrivate->nCmdPipe[0]);
    if ( 0 != err && OMX_ErrorNone == eError ) {
        eError = OMX_ErrorHardware;
        JPEGENC_DPRINT ("Error while closing cmd pipe\n");
    }

    err = close (pComponentPrivate->nCmdPipe[1]);
    if ( 0 != err && OMX_ErrorNone == eError ) {
        eError = OMX_ErrorHardware;
        JPEGENC_DPRINT ("Error while closing cmd pipe\n");
    }

    /*  Close the command data pipe handles */
    err = close (pComponentPrivate->nCmdDataPipe[0]);
    if ( 0 != err && OMX_ErrorNone == eError ) {
        eError = OMX_ErrorHardware;
        JPEGENC_DPRINT ("Error while closing cmd pipe\n");
    }

    err = close (pComponentPrivate->nCmdDataPipe[1]);
    if ( 0 != err && OMX_ErrorNone == eError ) {
        eError = OMX_ErrorHardware;
        JPEGENC_DPRINT ("Error while closing cmd pipe\n");
    }

    FREE(pComponentPrivate->pPortParamType);
    FREE(pComponentPrivate->pPortParamTypeAudio);
    FREE(pComponentPrivate->pPortParamTypeVideo);
    FREE(pComponentPrivate->pPortParamTypeOthers);
    FREE(pComponentPrivate->pCustomLumaQuantTable);
    FREE(pComponentPrivate->pCustomChromaQuantTable);    
    FREE(pComponentPrivate->pHuffmanTable);        
    FREE(pComponentPrivate->pDynParams);
    FREE(pComponentPrivate->cComponentName);
    FREE(pComponentPrivate->pString_Comment);

    if (pComponentPrivate->InParams.pInParams) {
        p = (OMX_U8 *)pComponentPrivate->InParams.pInParams;
        p -= 128;
        FREE(p);
        pComponentPrivate->InParams.pInParams = NULL;
        pComponentPrivate->InParams.size = 0;
    }

    for (nCount = 0; nCount < NUM_OF_BUFFERSJPEG; nCount++) {
        FREE(pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pBufferPrivate[nCount]);
        FREE(pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pBufferPrivate[nCount]);
    }

    FREE(pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pPortDef);
    FREE(pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pPortDef);
    FREE(pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pPortFormat);
    FREE(pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pPortFormat);
    FREE(pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pParamBufSupplier);
    FREE(pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pParamBufSupplier);
    FREE(pComponentPrivate->pCompPort[JPEGENC_INP_PORT]);
    FREE(pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]);
    FREE (pComponentPrivate->pPriorityMgmt);
    FREE( pComponentPrivate->pQualityfactor );

    pthread_mutex_destroy(&pComponentPrivate->jpege_mutex);
    pthread_cond_destroy(&pComponentPrivate->stop_cond);
    pthread_cond_destroy(&pComponentPrivate->flush_cond);
    /* pthread_cond_destroy(&pComponentPrivate->control_cond); */

    pthread_mutex_destroy(&pComponentPrivate->jpege_mutex_app);
    pthread_cond_destroy(&pComponentPrivate->populate_cond);
    pthread_cond_destroy(&pComponentPrivate->unpopulate_cond);
#ifdef __PERF_INSTRUMENTATION__
        PERF_Boundary(pComponentPrivate->pPERF,
                      PERF_BoundaryComplete | PERF_BoundaryCleanup);
        PERF_Done(pComponentPrivate->pPERF);
#endif

    FREE(pComponentPrivate );
    
    EXIT:

    JPEGENC_DPRINT("Exiting JPEG FreeComponentresources\n");
    return eError;
}



OMX_ERRORTYPE Fill_JpegEncLCMLInitParams(LCML_DSP *lcml_dsp, OMX_U16 arr[], OMX_HANDLETYPE pComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = NULL;
    JPEGENC_COMPONENT_PRIVATE *pComponentPrivate = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDefOut = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDefIn = NULL;
    int outbufsize = 0; 


    JPEGENC_DPRINT("Initialize Params\n");
    OMX_CHECK_PARAM(pComponent);
    pHandle = (OMX_COMPONENTTYPE *)pComponent;
    pComponentPrivate = (JPEGENC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;
    pPortDefIn = pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pPortDef;
    pPortDefOut = pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pPortDef;
    outbufsize = pPortDefOut->nBufferSize;

    lcml_dsp->In_BufInfo.nBuffers = NUM_OF_BUFFERSJPEG;
    lcml_dsp->In_BufInfo.nSize = pPortDefIn->nBufferSize;
    lcml_dsp->In_BufInfo.DataTrMethod = DMM_METHOD;

    lcml_dsp->Out_BufInfo.nBuffers = NUM_OF_BUFFERSJPEG;
    lcml_dsp->Out_BufInfo.nSize = outbufsize;
    lcml_dsp->Out_BufInfo.DataTrMethod = DMM_METHOD;

    lcml_dsp->NodeInfo.nNumOfDLLs = OMX_JPEGENC_NUM_DLLS;
    lcml_dsp->NodeInfo.AllUUIDs[0].uuid = (struct DSP_UUID * )&JPEGESOCKET_TI_UUID;
    strcpy ((char *)lcml_dsp->NodeInfo.AllUUIDs[0].DllName,JPEG_ENC_NODE_DLL);
    lcml_dsp->NodeInfo.AllUUIDs[0].eDllType = DLL_NODEOBJECT;

    lcml_dsp->NodeInfo.AllUUIDs[1].uuid = (struct DSP_UUID * )&JPEGESOCKET_TI_UUID;
    strcpy ((char *)lcml_dsp->NodeInfo.AllUUIDs[1].DllName,JPEG_ENC_NODE_DLL);
    lcml_dsp->NodeInfo.AllUUIDs[1].eDllType = DLL_DEPENDENT;

    lcml_dsp->NodeInfo.AllUUIDs[2].uuid =(struct DSP_UUID * ) &USN_UUID;
    strcpy ((char *)lcml_dsp->NodeInfo.AllUUIDs[2].DllName,USN_DLL);
    lcml_dsp->NodeInfo.AllUUIDs[2].eDllType = DLL_DEPENDENT;

    lcml_dsp->SegID = 0;
    lcml_dsp->Timeout = -1;
    lcml_dsp->Alignment = 0;
    lcml_dsp->Priority = 5;

    lcml_dsp->ProfileID = -1;
    
    JPEGENC_DPRINT("Setting DSP variables.....\n");
    /* CrPhArgs for JpegEnc */
    arr[0] = JPGENC_SNTEST_STRMCNT;
    arr[1] = JPGENC_SNTEST_INSTRMID; /* Stream ID   */
    arr[2] = 0;                      /* Stream based input stream   */
    arr[3] = JPGENC_SNTEST_INBUFCNT; /* Number of buffers on input stream   */
    arr[4] = JPGENC_SNTEST_OUTSTRMID;/* Stream ID   */
    arr[5] = 0;                      /* Stream based input stream   */
    arr[6] = JPGENC_SNTEST_OUTBUFCNT;/* Number of buffers on input stream   */

	if(pPortDefOut->format.image.nFrameWidth > 0) {
    		arr[7] = (OMX_U16)pPortDefOut->format.image.nFrameWidth;
	}
	else {
		arr[7] = (OMX_U16)JPGENC_SNTEST_MAX_WIDTH;
	}
 
    	if(pPortDefOut->format.image.nFrameHeight > 0) {
    		arr[8] = (OMX_U16)pPortDefOut->format.image.nFrameHeight;
	}
	else {
		arr[8] = (OMX_U16)JPGENC_SNTEST_MAX_HEIGHT;
	}
	
        /* arr[9] = 1; */
    
    /*
    if ( pPortDefIn->format.image.eColorFormat == OMX_COLOR_FormatYUV420PackedPlanar ) {
        arr[9] = 1;
    } else if ( pPortDefIn->format.image.eColorFormat ==  OMX_COLOR_FormatCbYCrY ) {
        arr[9] = 4;  
    }  else {
        arr[9] = 4;
    }
    */
    arr[9] = 1;

    arr[10] = 320; /* Maximum Horizontal Size of the Thumbnail for App0 marker */
    arr[11] = 240; /* Maximum Vertical Size of the Thumbnail for App0 marker */
    arr[12] = 320; /* Maximum Horizontal Size of the Thumbnail for App1 marker */
    arr[13] = 240; /* Maximum Vertical Size of the Thumbnail for App1 marker */
    arr[14] = 320; /* Maximum Horizontal Size of the Thumbnail for App13 marker */
    arr[15] = 240; /* Maximum Vertical Size of the Thumbnail for App13 marker */
    arr[16] = END_OF_CR_PHASE_ARGS;
    

    lcml_dsp->pCrPhArgs = arr;
    EXIT:
    return eError;
}




static OMX_ERRORTYPE HandleJpegEncInternalFlush(JPEGENC_COMPONENT_PRIVATE *pComponentPrivate, OMX_U32 nParam1)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 aParam[4];
    LCML_DSP_INTERFACE *pLcmlHandle = NULL;
    
    OMX_CHECK_PARAM(pComponentPrivate);

    if ( nParam1 == 0x0 || 
         nParam1 == -1 ) {

        pComponentPrivate->bFlushComplete = OMX_FALSE;
        aParam[0] = USN_STRMCMD_FLUSH;
        aParam[1] = 0;
        aParam[2] = 0;
        pLcmlHandle = (LCML_DSP_INTERFACE*)pComponentPrivate->pLCML;
        eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,EMMCodecControlStrmCtrl, (void*)aParam);
        if (eError != OMX_ErrorNone) {
            goto EXIT;
        }

        pthread_mutex_lock(&pComponentPrivate->jpege_mutex);
        while (pComponentPrivate->bFlushComplete == OMX_FALSE) {
            JPEGENC_DPRINT("%d in cond wait\n", __LINE__);
            pthread_cond_wait(&pComponentPrivate->flush_cond, &pComponentPrivate->jpege_mutex);
        }
        pthread_mutex_unlock(&pComponentPrivate->jpege_mutex);

        pComponentPrivate->bFlushComplete = OMX_FALSE;
    }
    if ( nParam1 == 0x1 || 
        nParam1 == -1 ) {

        pComponentPrivate->bFlushComplete = OMX_FALSE;
        aParam[0] = USN_STRMCMD_FLUSH;
        aParam[1] = 1;
        aParam[2] = 0;
        pLcmlHandle = (LCML_DSP_INTERFACE*)pComponentPrivate->pLCML;
        eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,EMMCodecControlStrmCtrl, (void*)aParam);
        if (eError != OMX_ErrorNone) {
            goto EXIT;
        }

        pthread_mutex_lock(&pComponentPrivate->jpege_mutex);
        while (pComponentPrivate->bFlushComplete == OMX_FALSE) {
            JPEGENC_DPRINT("%d in cond wait\n", __LINE__);
            pthread_cond_wait(&pComponentPrivate->flush_cond, &pComponentPrivate->jpege_mutex);
        }
        pthread_mutex_unlock(&pComponentPrivate->jpege_mutex);
        pComponentPrivate->bFlushComplete = OMX_FALSE;

        pComponentPrivate->bFlushComplete = OMX_FALSE;
    }

    EXIT:
    JPEGENC_DPRINT ("Exiting HandleCommand FLush Function JEPG Encoder\n");
    return eError;

}


OMX_ERRORTYPE HandleJpegEncCommandFlush(JPEGENC_COMPONENT_PRIVATE *pComponentPrivate, OMX_U32 nParam1)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 aParam[4];
    LCML_DSP_INTERFACE *pLcmlHandle = NULL;
    
    OMX_CHECK_PARAM(pComponentPrivate);

    if ( nParam1 == 0x0 || 
         nParam1 == -1 ) {

        pComponentPrivate->bFlushComplete = OMX_FALSE;

        aParam[0] = USN_STRMCMD_FLUSH;
        aParam[1] = 0;
        aParam[2] = 0;
        pLcmlHandle = (LCML_DSP_INTERFACE*)pComponentPrivate->pLCML;
        eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,EMMCodecControlStrmCtrl, (void*)aParam);
        if (eError != OMX_ErrorNone) {
            goto EXIT;
        }
        JPEGENC_DPRINT("sent EMMCodecControlStrmCtrl command\n");

        pthread_mutex_lock(&pComponentPrivate->jpege_mutex);
        while (pComponentPrivate->bFlushComplete == OMX_FALSE) {
            JPEGENC_DPRINT("%d in cond wait\n", __LINE__);
            pthread_cond_wait(&pComponentPrivate->flush_cond, &pComponentPrivate->jpege_mutex);
        }
        pthread_mutex_unlock(&pComponentPrivate->jpege_mutex);

        pComponentPrivate->bFlushComplete = OMX_FALSE;

        while (pComponentPrivate->nInPortIn > pComponentPrivate->nInPortOut) {

            OMX_BUFFERHEADERTYPE* pBuffHead = NULL;
            JPEGENC_BUFFER_PRIVATE* pBuffPrivate = NULL;
            int ret;

            ret = read(pComponentPrivate->filled_inpBuf_Q[0], &(pBuffHead), sizeof(pBuffHead));
            if ( ret == -1 ) {
                JPEGENC_DPRINT("Error while reading from the pipe\n");
            }

            if (pBuffHead != NULL) {
                pBuffPrivate = pBuffHead->pInputPortPrivate;
            }

            pComponentPrivate->nInPortOut ++;
            pBuffPrivate->eBufferOwner = JPEGENC_BUFFER_CLIENT;
            JPEGENC_DPRINT("buffer summary (return empty output buffer) %d %d %d %d\n",
                    pComponentPrivate->nInPortIn,
                    pComponentPrivate->nInPortOut,
                    pComponentPrivate->nOutPortIn,
                    pComponentPrivate->nOutPortOut);       
            JPEGENC_DPRINT("before EmptyBufferDone\n");
            pComponentPrivate->cbInfo.EmptyBufferDone(
                           pComponentPrivate->pHandle,
                           pComponentPrivate->pHandle->pApplicationPrivate,
                           pBuffHead);   
             JPEGENC_DPRINT("after EmptyBufferDone\n");
        }
#if 0    
        for ( i=0; i < pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pPortDef->nBufferCountActual; i++ ) {

#ifdef __PERF_INSTRUMENTATION__
            PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                              PREF(pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pBufferPrivate[i]->pBufferHdr,pBuffer),
                              0,
                              PERF_ModuleHLMM);
#endif
          pBuffPrivate = (JPEGENC_BUFFER_PRIVATE*) pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pBufferPrivate[i]->pBufferHdr->pInputPortPrivate;
          JPEGENC_DPRINT("flush input port. buffer owner (%d) %d\n", i, pBuffPrivate->eBufferOwner);
        }
#endif      

        JPEGENC_DPRINT("buffer summary (flush input) %d %d %d %d\n",
                    pComponentPrivate->nInPortIn,
                    pComponentPrivate->nInPortOut,
                    pComponentPrivate->nOutPortIn,
                    pComponentPrivate->nOutPortOut);    

        /* returned all input buffers */
        pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle, 
                                      pComponentPrivate->pHandle->pApplicationPrivate,
                      OMX_EventCmdComplete, 
                                      OMX_CommandFlush,
                                      JPEGENC_INP_PORT, 
                                      NULL); 

    }
    if ( nParam1 == 0x1 || 
         nParam1 == -1 ) {

        pComponentPrivate->bFlushComplete = OMX_FALSE;

        aParam[0] = USN_STRMCMD_FLUSH;
        aParam[1] = 1;
        aParam[2] = 0;
        pLcmlHandle = (LCML_DSP_INTERFACE*)pComponentPrivate->pLCML;
        eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,EMMCodecControlStrmCtrl, (void*)aParam);
        if (eError != OMX_ErrorNone) {
            goto EXIT;
        }
        JPEGENC_DPRINT("(1) sent EMMCodecControlStrmCtrl command\n");

        pthread_mutex_lock(&pComponentPrivate->jpege_mutex);
        while (pComponentPrivate->bFlushComplete == OMX_FALSE) {
            JPEGENC_DPRINT("%d in cond wait\n", __LINE__);
            pthread_cond_wait(&pComponentPrivate->flush_cond, &pComponentPrivate->jpege_mutex);
        }
        pthread_mutex_unlock(&pComponentPrivate->jpege_mutex);
        pComponentPrivate->bFlushComplete = OMX_FALSE;

        /* return all output buffers */
        
#if 0
        for ( i=0; i < pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pPortDef->nBufferCountActual ; i++ ) {
            JPEGENC_DPRINT("BEFORE  FillBufferDone in OMX_CommandFlush\n");

#ifdef __PERF_INSTRUMENTATION__
                        PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                          PREF(pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pBufferPrivate[i]->pBufferHdr,pBuffer),
                                          PREF(pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pBufferPrivate[i]->pBufferHdr,nFilledLen),
                                          PERF_ModuleHLMM);
#endif
          pBuffPrivate = (JPEGENC_BUFFER_PRIVATE*) pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pBufferPrivate[i]->pBufferHdr->pOutputPortPrivate;
          JPEGENC_DPRINT("flush output port. buffer owner (%d) %d\n", i, pBuffPrivate->eBufferOwner);

          JPEGENC_DPRINT("in flush 1: buffer %d owner %d\n", i, pBuffPrivate->eBufferOwner);
          if (pBuffPrivate->eBufferOwner == JPEGENC_BUFFER_COMPONENT_IN) {
                  JPEGENC_DPRINT("return output buffer %p from free_in_pipe (flush)\n", 
                     pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pBufferPrivate[i]->pBufferHdr);
                  pComponentPrivate->nOutPortOut ++;
                  pBuffPrivate->eBufferOwner = JPEGENC_BUFFER_CLIENT;
                  pComponentPrivate->cbInfo.FillBufferDone (pComponentPrivate->pHandle,
                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                   pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pBufferPrivate[i]->pBufferHdr);
          }

        }
#endif
        while (pComponentPrivate->nOutPortIn > pComponentPrivate->nOutPortOut) {
            OMX_BUFFERHEADERTYPE* pBuffHead = NULL;
            JPEGENC_BUFFER_PRIVATE* pBuffPrivate = NULL;
            int ret;

            JPEGENC_DPRINT("in while loop %d %d )\n", pComponentPrivate->nOutPortIn, pComponentPrivate->nOutPortOut);
            ret = read(pComponentPrivate->free_outBuf_Q[0], &pBuffHead, sizeof(pBuffHead));
            if ( ret == -1 ) {
                JPEGENC_DPRINT ("Error while reading from the pipe\n");
                goto EXIT;
            }
            JPEGENC_DPRINT("after read\n");
            if (pBuffHead != NULL) {
               pBuffPrivate = pBuffHead->pOutputPortPrivate;
            }

            pComponentPrivate->nOutPortOut ++;
            pBuffPrivate->eBufferOwner = JPEGENC_BUFFER_CLIENT;
            JPEGENC_DPRINT("buffer summary (return empty output buffer) %d %d %d %d\n",
                    pComponentPrivate->nInPortIn,
                    pComponentPrivate->nInPortOut,
                    pComponentPrivate->nOutPortIn,
                    pComponentPrivate->nOutPortOut);       
             JPEGENC_DPRINT("before FillBufferDone\n");
             pComponentPrivate->cbInfo.FillBufferDone(pComponentPrivate->pHandle,
                    pComponentPrivate->pHandle->pApplicationPrivate,
                    pBuffHead); 
             JPEGENC_DPRINT("after FillBufferDone\n");
        }

        JPEGENC_DPRINT("buffer summary (flush input) %d %d %d %d\n",
                    pComponentPrivate->nInPortIn,
                    pComponentPrivate->nInPortOut,
                    pComponentPrivate->nOutPortIn,
                    pComponentPrivate->nOutPortOut);   
        
        pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle, 
                                               pComponentPrivate->pHandle->pApplicationPrivate,
                                               OMX_EventCmdComplete, 
                                               OMX_CommandFlush,
                                               JPEGENC_OUT_PORT, 
                                               NULL); 
    }

    EXIT:
    JPEGENC_DPRINT ("Exiting HandleCommand FLush Function JEPG Encoder\n");
    return eError;

}

OMX_ERRORTYPE SendDynamicParam(JPEGENC_COMPONENT_PRIVATE *pComponentPrivate)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    IDMJPGE_TIGEM_DynamicParams* pTmpDynParams;
    OMX_HANDLETYPE pLcmlHandle = NULL;
    char* pTmp = NULL;
    OMX_U32 cmdValues[3] = {0, 0, 0};
    IIMGENC_DynamicParams ptParam;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDefIn = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDefOut = NULL;

    OMX_CHECK_PARAM(pComponentPrivate);

    pPortDefIn = pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pPortDef;
    pPortDefOut = pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pPortDef;

    ptParam.nNumAU = 0; /*  XDM_DEFAULT */
    //ptParam.nSize   =   sizeof(IIMGENC_DynamicParams);
    ptParam.nSize   =   sizeof(IDMJPGE_TIGEM_DynamicParams) ;

    if ( pPortDefOut->format.image.eColorFormat == OMX_COLOR_FormatYUV420PackedPlanar ) {
        ptParam.nInputChromaFormat = 1;

    } 
    else if ( pPortDefOut->format.image.eColorFormat ==  OMX_COLOR_FormatCbYCrY ) {
        ptParam.nInputChromaFormat = 4;

    } 
    else {
        ptParam.nInputChromaFormat = 1;
    }

    if (pComponentPrivate->pCrop->nWidth == 0)
    {
        ptParam.nInputWidth     = pPortDefIn->format.image.nFrameWidth;    
    }
    else
    {
        ptParam.nInputWidth     = pComponentPrivate->pCrop->nWidth;    
    }
    
    if (pComponentPrivate->pCrop->nHeight == 0)
    {
        ptParam.nInputHeight     = pPortDefIn->format.image.nFrameHeight;    
    }
    else
    {
        ptParam.nInputHeight     = pComponentPrivate->pCrop->nHeight;    
    }

    ptParam.nCaptureWidth   =  pPortDefIn->format.image.nFrameWidth;
    ptParam.nGenerateHeader =   0; /*XDM_ENCODE_AU*/
    ptParam.qValue          =   pComponentPrivate->pQualityfactor->nQFactor;

    JPEGENC_DPRINT("ptParam.qValue %d\n", ptParam.qValue);


    pTmp = (char*)pComponentPrivate->pDynParams;
    pTmp += 128;
    pTmpDynParams = (IDMJPGE_TIGEM_DynamicParams*)pTmp;

    pTmpDynParams->params         = ptParam;
    pTmpDynParams->captureHeight = pPortDefIn->format.image.nFrameHeight;
    pTmpDynParams->DRI_Interval  = pComponentPrivate->nDRI_Interval;
    pTmpDynParams->huffmanTable = NULL;
    pTmpDynParams->quantTable     = NULL;
    
    cmdValues[0] = IUALG_CMD_SETSTATUS; 
    cmdValues[1] = (OMX_U32)(pTmpDynParams);
    cmdValues[2] = sizeof(IDMJPGE_TIGEM_DynamicParams) + 128;

    pComponentPrivate->bAckFromSetStatus = 0;
    pLcmlHandle =(LCML_DSP_INTERFACE*)pComponentPrivate->pLCML;
    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               EMMCodecControlAlgCtrl, 
                                               (void*)&cmdValues);

EXIT:
    return eError;
}

/*-------------------------------------------------------------------*/
/**
  *  HandleCommand() Handle State type commands
  *
  *  Depending on the State Command received it executes the corresponding code.
  *
  * @param phandle    handle for this instance of the component
  *
  * @retval OMX_NoError              Success, ready to roll
  * @retval OMX_ErrorInsufficientResources    Insiffucient Resources
  * @retval OMX_ErrorInvalidState    Invalid State Change
  *
  **/
/*-------------------------------------------------------------------*/
OMX_ERRORTYPE HandleJpegEncCommand (JPEGENC_COMPONENT_PRIVATE *pComponentPrivate, OMX_U32 nParam1)
{
    
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDefIn = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDefOut = NULL;
    OMX_HANDLETYPE pLcmlHandle = NULL;
    LCML_DSP *lcml_dsp;
    OMX_U16 arr[100];
    OMX_U32 lImageResolution = 0;
    LCML_CALLBACKTYPE cb;
#ifdef RESOURCE_MANAGER_ENABLED
    OMX_U8 nMHzRM = 0;
#endif

    
    JPEGENC_DPRINT("JPEGEnc Handlecommand\n");

    OMX_CHECK_PARAM(pComponentPrivate);
    pHandle = (OMX_COMPONENTTYPE *) pComponentPrivate->pHandle;
    pPortDefIn = pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pPortDef;
    pPortDefOut = pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pPortDef;
    
    switch ( (OMX_STATETYPE)(nParam1) ) {
    case OMX_StateIdle:
        JPEGENC_DPRINT("HandleCommand: Cmd OMX_StateIdle\n");
        JPEGENC_DPRINT("CHP 1 pComponentPrivate->nCurState  = %d\n",pComponentPrivate->nCurState );
        JPEGENC_DPRINT("In idle in %d out %d\n", pComponentPrivate->nInPortIn, pComponentPrivate->nOutPortOut);

        if ( pComponentPrivate->nCurState == OMX_StateIdle ) {
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventError, 
                                                   OMX_ErrorSameState, 
                                                   OMX_TI_ErrorMinor, 
                                                   NULL);
            break;
        } 
        else if ( pComponentPrivate->nCurState == OMX_StateLoaded || 
                  pComponentPrivate->nCurState == OMX_StateWaitForResources) {
                 
                      JPEGENC_DPRINT("state tranc from loaded to idle\n");
#ifdef __PERF_INSTRUMENTATION__
                  PERF_Boundary(pComponentPrivate->pPERFcomp,
                                PERF_BoundaryStart | PERF_BoundarySetup);
#endif
    
#if 0
            nTimeout=0x0;
            if ( pPortDefIn->bEnabled == OMX_TRUE && 
                pPortDefOut->bEnabled == OMX_TRUE ) 
            {           

            nTimeout = 0x0;
                while ( 1 )
                {
        
                    if (pPortDefIn->bPopulated) {
                        JPEGENC_DPRINT("Entrando al break\n");
                   break;
                    } 
                    else if ( nTimeout++ > JPEGENC_TIMEOUT ) {
                        JPEGENC_DPRINT("Timeout ...\n");
                    pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle, 
                                                               pComponentPrivate->pHandle->pApplicationPrivate,
                                                               OMX_EventError, 
                                                               OMX_ErrorPortUnresponsiveDuringAllocation, 
                                                               0x0,
                                                               "Not response Port -Idle");         
                        break;
                    }
                    
#ifndef UNDER_CE
            sched_yield();
#else 
            Sleep(0);
#endif          
                /* sleep(1); */
                }
            }
#endif

#ifdef RESOURCE_MANAGER_ENABLED /* Resource Manager Proxy Calls */
            pComponentPrivate->rmproxyCallback.RMPROXY_Callback = (void *)ResourceManagerCallback;
            lImageResolution = pPortDefIn->format.image.nFrameWidth * pPortDefIn->format.image.nFrameHeight;
            OMX_GET_RM_VALUE(lImageResolution, nMHzRM);
            JPEGENC_DPRINT("Value sent to RM = %d\n", nMHzRM);
            if (pComponentPrivate->nCurState != OMX_StateWaitForResources) {

                eError = RMProxy_NewSendCommand(pHandle, RMProxy_RequestResource, OMX_JPEG_Encoder_COMPONENT, nMHzRM, 3456, &(pComponentPrivate->rmproxyCallback));

                if (eError != OMX_ErrorNone) {
                    /* resource is not available, need set state to OMX_StateWaitForResources*/
                    JPEGENC_DPRINT("Resource is not available\n");

                    pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                           pHandle->pApplicationPrivate,
                                                           OMX_EventError,
                                                           OMX_ErrorInsufficientResources,
                                                           OMX_TI_ErrorSevere,
                                                           NULL);
                    eError = OMX_ErrorNone;
                    break;
                }
            }
#endif

            if ( pPortDefIn->bEnabled == OMX_TRUE && pPortDefOut->bEnabled == OMX_TRUE ) {
                pthread_mutex_lock(&pComponentPrivate->jpege_mutex_app);
                while ( (!pPortDefIn->bPopulated) || (!pPortDefOut->bPopulated)) {
                    JPEGENC_DPRINT("%d in cond wait\n", __LINE__);
                    pthread_cond_wait(&pComponentPrivate->populate_cond, &pComponentPrivate->jpege_mutex_app);
                }
                pthread_mutex_unlock(&pComponentPrivate->jpege_mutex_app);
            }

             eError =  GetJpegEncLCMLHandle(pHandle);
            
            if ( eError != OMX_ErrorNone ) {
                JPEGENC_DPRINT("GetLCMLHandle failed...\n");
                goto EXIT;
            }
            pLcmlHandle =(LCML_DSP_INTERFACE*)pComponentPrivate->pLCML;
            lcml_dsp = (((LCML_DSP_INTERFACE*)pLcmlHandle)->dspCodec);
            Fill_JpegEncLCMLInitParams(lcml_dsp,arr, pHandle);
            JPEGENC_DPRINT("Start Callback funtion JPEG .....\n");
            cb.LCML_Callback = (void *) JpegEncLCML_Callback;
            JPEGENC_DPRINT("Start LCML_InitMMCodec JPEG Phase in JPEG.....\n");
            
            /*  calling initMMCodec to init codec with details filled earlier   */
            eError = LCML_InitMMCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle, NULL, &pLcmlHandle, NULL, &cb);

            if ( eError != OMX_ErrorNone ) {
                JPEGENC_DPRINT("InitMMCodec failed...  %x\n", eError);
                goto EXIT;
            }
            pComponentPrivate->isLCMLActive = 1;
            JPEGENC_DPRINT("End LCML_InitMMCodec Phase\n");


            pComponentPrivate->bFlushComplete = OMX_FALSE;
            JPEGENC_DPRINT("State has been Set to Idle\n");
            pComponentPrivate->nCurState = OMX_StateIdle;

            pComponentPrivate->nInPortIn   = pComponentPrivate->nInPortOut   = 0;
            pComponentPrivate->nOutPortIn = pComponentPrivate->nOutPortOut = 0;
            

#ifdef RESOURCE_MANAGER_ENABLED
            eError= RMProxy_NewSendCommand(pHandle, RMProxy_StateSet, OMX_JPEG_Encoder_COMPONENT, OMX_StateIdle,  3456, NULL);
            if (eError != OMX_ErrorNone) {
                JPEGENC_DPRINT("Resources not available Loaded ->Idle\n");

                pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                       pHandle->pApplicationPrivate,
                                                       OMX_EventError,
                                                       OMX_ErrorInsufficientResources,
                                                       OMX_TI_ErrorSevere,
                                                       NULL);
                break;
            }
#endif
            
#ifdef __PERF_INSTRUMENTATION__
            PERF_Boundary(pComponentPrivate->pPERFcomp,
                          PERF_BoundaryComplete | PERF_BoundarySetup);
#endif

            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle, 
                              pComponentPrivate->pHandle->pApplicationPrivate,
                          OMX_EventCmdComplete, 
                          OMX_CommandStateSet,
                          pComponentPrivate->nCurState, 
                          NULL);
            break;
            
        } 
        else if ( pComponentPrivate->nCurState == OMX_StateExecuting ||  
                  pComponentPrivate->nCurState == OMX_StatePause ) {
#ifdef __PERF_INSTRUMENTATION__
            PERF_Boundary(pComponentPrivate->pPERFcomp,
                          PERF_BoundaryComplete | PERF_BoundarySteadyState);
#endif
            
          pLcmlHandle =(LCML_DSP_INTERFACE*)pComponentPrivate->pLCML;
          pComponentPrivate->bDSPStopAck = OMX_FALSE;
          JPEGENC_DPRINT("bDSPStopAck is %d\n", pComponentPrivate->bDSPStopAck);
          eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,MMCodecControlStop,NULL);
          pComponentPrivate->nApp_nBuf= 1; 
         /* HandleJpegEncCommandFlush(pComponentPrivate, -1); */
          /*
          if ( pComponentPrivate->isLCMLActive ==1 ) {
              LCML_ControlCodec(((LCML_DSP_INTERFACE*)pComponentPrivate->pLCML)->pCodecinterfacehandle,EMMCodecControlDestroy,NULL);
              dlclose(pComponentPrivate->pDllHandle);
              pComponentPrivate->isLCMLActive = 0;
          }
          */

#ifdef RESOURCE_MANAGER_ENABLED

    eError= RMProxy_NewSendCommand(pHandle, RMProxy_StateSet, OMX_JPEG_Encoder_COMPONENT, OMX_StateIdle, 3456, NULL);
    if (eError != OMX_ErrorNone) {
        JPEGENC_DPRINT("Resources not available Executing ->Idle\n");
        pComponentPrivate->nCurState = OMX_StateWaitForResources;
        pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                               pHandle->pApplicationPrivate,
                                               OMX_EventCmdComplete, 
                                               OMX_CommandStateSet, 
                                               pComponentPrivate->nCurState, 
                                               NULL);
        break;
        }
#endif
          pComponentPrivate->ExeToIdleFlag |= JPEGE_BUFFERBACK;

        JPEGENC_DPRINT("JPEG enc: before stop lock\n");
        pthread_mutex_lock(&pComponentPrivate->jpege_mutex);
        /*
        while ((pComponentPrivate->ExeToIdleFlag & 0x3) != JPEGE_IDLEREADY) {
            JPEGENC_DPRINT("%d in cond wait\n", __LINE__);
            pthread_cond_wait(&pComponentPrivate->stop_cond, &pComponentPrivate->jpege_mutex);
        }
        */
        while (pComponentPrivate->bDSPStopAck == OMX_FALSE) {
            JPEGENC_DPRINT("%d in cond wait\n", __LINE__);
            pthread_cond_wait(&pComponentPrivate->stop_cond, &pComponentPrivate->jpege_mutex);
        }
        pthread_mutex_unlock(&pComponentPrivate->jpege_mutex);

        JPEGENC_DPRINT("JPEG enc:got STOP ack from DSP\n");

        int i;
        for (i = 0; i < pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pPortDef->nBufferCountActual; i ++) {
            JPEGENC_BUFFER_PRIVATE    *pBuffPrivate = NULL;
            OMX_BUFFERHEADERTYPE* pBuffHead = NULL;

            pBuffHead = pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pBufferPrivate[i]->pBufferHdr;
            pBuffPrivate = pBuffHead->pInputPortPrivate;

            JPEGENC_DPRINT("JPEG enc:: owner %d \n", pBuffPrivate->eBufferOwner);
            if (pBuffPrivate->eBufferOwner != JPEGENC_BUFFER_CLIENT) {
                if (pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pBufSupplier != OMX_BufferSupplyInput) {
                    if(pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->hTunnelComponent == NULL){
                        JPEGENC_DPRINT("Sending buffer to app\n");
                        JPEGENC_DPRINT("Handle error from DSP/bridge\n");
                        pComponentPrivate->nInPortOut ++;
                        pBuffPrivate->eBufferOwner = JPEGENC_BUFFER_CLIENT;
                        pComponentPrivate->cbInfo.EmptyBufferDone(
                                   pComponentPrivate->pHandle,
                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                   pBuffHead); 
                }
                else{
                    JPEGENC_DPRINT("JPEG enc:: Sending beffer to tunnel, pHandle=%p\n", pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->hTunnelComponent);
                    pBuffHead->nFilledLen = 0;
                    pBuffPrivate->eBufferOwner = JPEGENC_BUFFER_CLIENT;
                    eError = OMX_FillThisBuffer(pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->hTunnelComponent, 
                                                pBuffHead);
                }
            }
        }
    }
  
        JPEGENC_DPRINT("returned all input buffers\n");

        for (i = 0; i < pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pPortDef->nBufferCountActual; i ++) {
            JPEGENC_BUFFER_PRIVATE    *pBuffPrivate = NULL;
            OMX_BUFFERHEADERTYPE* pBuffHead = NULL;

            pBuffHead = pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pBufferPrivate[i]->pBufferHdr;
            pBuffPrivate = pBuffHead->pOutputPortPrivate;

            JPEGENC_DPRINT("buffer %p owner %d \n", pBuffHead, pBuffPrivate->eBufferOwner);
            if (pBuffPrivate->eBufferOwner != JPEGENC_BUFFER_CLIENT) {
                if (pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pBufSupplier != OMX_BufferSupplyOutput) {
                    if(pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->hTunnelComponent == NULL){
                        JPEGENC_DPRINT("JPEG enc:: Sending OUTPUT buffer to app\n");
                        JPEGENC_DPRINT("Handle error from DSP/bridge\n");
                        pComponentPrivate->nOutPortOut ++;
                        pBuffPrivate->eBufferOwner = JPEGENC_BUFFER_CLIENT;
                        pComponentPrivate->cbInfo.FillBufferDone(
                                   pComponentPrivate->pHandle,
                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                   pBuffHead);
                    }
                    else{
                        JPEGENC_DPRINT("JPEG enc:: Sending OUTPUT buffer to Tunnel component\n");
                        pBuffPrivate->eBufferOwner = JPEGENC_BUFFER_CLIENT;
                    eError = OMX_EmptyThisBuffer(pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->hTunnelComponent, 
                                                pBuffHead);
                    }
                }
            }
        }
        JPEGENC_DPRINT("returned all output buffers\n");

         pComponentPrivate->nCurState = OMX_StateIdle;
         pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
               pComponentPrivate->pHandle->pApplicationPrivate,
               OMX_EventCmdComplete, 
               OMX_CommandStateSet, 
               pComponentPrivate->nCurState, 
               NULL);
             pComponentPrivate->ExeToIdleFlag = 0;
    } 
        else {
         JPEGENC_DPRINT("Error: Invalid State Given by Application\n");
         pComponentPrivate->cbInfo.EventHandler (pComponentPrivate->pHandle, 
                                                    pComponentPrivate->pHandle->pApplicationPrivate,
                            OMX_EventError, 
                                                    OMX_ErrorIncorrectStateTransition, 
                                                    OMX_TI_ErrorMinor, 
                                                    "Invalid State");
        }
    break;

    case OMX_StateExecuting:
        
        JPEGENC_DPRINT("HandleCommand: Cmd OMX_StateExecuting \n");
        JPEGENC_DPRINT("In exec in %d out %d\n", pComponentPrivate->nInPortIn, pComponentPrivate->nOutPortOut);
        if ( pComponentPrivate->nCurState == OMX_StateExecuting ) {
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventError, 
                                                   OMX_ErrorSameState, 
                                                   OMX_TI_ErrorMinor, 
                                                   NULL);
        } 
        else if ( pComponentPrivate->nCurState == OMX_StateIdle || pComponentPrivate->nCurState == OMX_StatePause ) {

#ifdef __PERF_INSTRUMENTATION__
            PERF_Boundary(pComponentPrivate->pPERFcomp,
                          PERF_BoundaryStart | PERF_BoundarySteadyState);
#endif  

#if 1       
        eError = SendDynamicParam(pComponentPrivate);
            if (eError != OMX_ErrorNone ) {
                JPEGENC_DPRINT("SETSTATUS failed...  %x\n", eError);
                goto EXIT;
        }
#endif
    



        JPEGENC_DPRINT("after SendDynamicParam\n");
        pLcmlHandle =(LCML_DSP_INTERFACE*)pComponentPrivate->pLCML;
        eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,EMMCodecControlStart,NULL);
        
#ifdef RESOURCE_MANAGER_ENABLED

    eError= RMProxy_NewSendCommand(pHandle, RMProxy_StateSet, OMX_JPEG_Encoder_COMPONENT, OMX_StateExecuting, 3456, NULL);
    if (eError != OMX_ErrorNone) {
        JPEGENC_DPRINT("Resources not available\n");
        pComponentPrivate->nCurState = OMX_StateWaitForResources;
        pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                               pHandle->pApplicationPrivate,
                                               OMX_EventCmdComplete, 
                                               OMX_CommandStateSet, 
                                               pComponentPrivate->nCurState, 
                                               NULL);
        break;
        }
#endif

        pComponentPrivate->nCurState = OMX_StateExecuting;
        JPEGENC_DPRINT("State has been set to Executing\n"); 
#ifdef __PERF_INSTRUMENTATION__
            PERF_Boundary(pComponentPrivate->pPERFcomp,
                          PERF_BoundarySteadyState| PERF_BoundaryComplete);
#endif

        pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                               pComponentPrivate->pHandle->pApplicationPrivate,
                                               OMX_EventCmdComplete, 
                                               OMX_CommandStateSet, 
                                               pComponentPrivate->nCurState, 
                                               NULL);
        } else {
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventError, 
                                                   OMX_ErrorIncorrectStateTransition, 
                                                   OMX_TI_ErrorMinor, 
                                                   NULL);
        }
        break;

    case OMX_StatePause:
        JPEGENC_DPRINT("HandleCommand: Cmd OMX_StatePause\n");

        pComponentPrivate->nToState = OMX_StatePause;
#ifdef __PERF_INSTRUMENTATION__
                PERF_Boundary(pComponentPrivate->pPERFcomp,
                              PERF_BoundaryComplete | PERF_BoundarySteadyState);
#endif

        if ( pComponentPrivate->nCurState == OMX_StatePause ) {
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventError, OMX_ErrorSameState, OMX_TI_ErrorMinor , NULL);
        } else if ( pComponentPrivate->nCurState == OMX_StateExecuting || pComponentPrivate->nCurState == OMX_StateIdle ) {

            pLcmlHandle =(LCML_DSP_INTERFACE*)pComponentPrivate->pLCML;
            eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,EMMCodecControlPause,NULL);
            if ( eError != OMX_ErrorNone ) {
                JPEGENC_DPRINT("Error during EMMCodecControlPause. Error: %d.\n", eError );
                goto EXIT;
            }
            /*
            pComponentPrivate->nCurState = OMX_StatePause;
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventCmdComplete, OMX_CommandStateSet, pComponentPrivate->nCurState, NULL);
                                                   */
        } else {
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventError, OMX_ErrorIncorrectStateTransition, OMX_TI_ErrorMinor , NULL);
            JPEGENC_DPRINT ("Error: Invalid State Given by Application\n");
        }
        break;

    
    case OMX_StateInvalid:
        JPEGENC_DPRINT("HandleCommand: Cmd OMX_StateInvalid::\n");
        if ( pComponentPrivate->nCurState == OMX_StateInvalid ) {
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle, pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventError, OMX_ErrorSameState, OMX_TI_ErrorMinor , NULL);
        }
        if ( pComponentPrivate->nCurState != OMX_StateLoaded ) {
            JPEGENC_DPRINT("HandleJpegEncInternalFlush \n");
            eError = HandleJpegEncInternalFlush(pComponentPrivate, nParam1);
            }

            pComponentPrivate->nCurState = OMX_StateInvalid;

            if(pComponentPrivate->nToState == OMX_StateInvalid){ /*if the IL client call directly send to invalid state*/
                pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                       OMX_EventCmdComplete, 
                                       OMX_CommandStateSet, 
                                       pComponentPrivate->nCurState, 
                                       NULL);
            }
            else{ /*When the component go to invalid state by it self*/
                pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle, 
                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                       OMX_EventError, 
                                       OMX_ErrorInvalidState, 
                                       OMX_TI_ErrorSevere, 
                                       NULL);
            }
        break;

    case OMX_StateLoaded:
        if ( pComponentPrivate->nCurState == OMX_StateLoaded ) {
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventError, 
                                                   OMX_ErrorSameState, 
                                                   OMX_TI_ErrorMinor , 
                                                   NULL);
            
        } else if ( pComponentPrivate->nCurState == OMX_StateIdle ||
                    pComponentPrivate->nCurState == OMX_StateWaitForResources ) {
            /* Ports have to be unpopulated before transition completes */
            JPEGENC_DPRINT("from idle to loaded\n");
            
            pLcmlHandle = (LCML_DSP_INTERFACE*)pComponentPrivate->pLCML;
            if ( pComponentPrivate->pLCML != NULL && pComponentPrivate->isLCMLActive) {
                pLcmlHandle =(LCML_DSP_INTERFACE*)pComponentPrivate->pLCML;
                JPEGENC_DPRINT("try to close library again %p\n", pComponentPrivate->pLCML);
                LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,EMMCodecControlDestroy,NULL);
                JPEGENC_DPRINT("after close library again %p\n", pComponentPrivate->pLCML);
                pComponentPrivate->pLCML = NULL;
                dlclose(pComponentPrivate->pDllHandle);
                pComponentPrivate->isLCMLActive = 0;

            }
            JPEGENC_DPRINT("after release LCML\n");
#ifdef __PERF_INSTRUMENTATION__
            PERF_Boundary(pComponentPrivate->pPERFcomp,
                          PERF_BoundaryStart | PERF_BoundaryCleanup);
#endif

#ifdef UNDER_CE
            nTimeout=0x0;
        
                while ( 1 )
                {
                    if ( (pPortDefOut->bPopulated == OMX_FALSE) )
                    {
                        JPEGENC_DPRINT("Thread Sending Cmd EMMCodecControlDestroy\n");
                    
                        
                        break;
                    } else if ( nTimeout++ > JPEGENC_TIMEOUT )
                    
                    {
                        JPEGENC_DPRINT("Timeout ...\n");
                        pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle, 
                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                       OMX_EventError, 
                                       OMX_ErrorPortUnresponsiveDuringAllocation, 
                                       OMX_TI_ErrorMajor,
                                       "Not response Port -Loaded");  
                        JPEGENC_ERRPRINT("Not all ports are unpopulated!\n");
                        break;
                    }
                    /* Sleep for a while, so the application thread can allocate buffers */
            sched_yield();
            }
#else

            pthread_mutex_lock(&pComponentPrivate->jpege_mutex_app);
            while ( pPortDefIn->bPopulated || pPortDefOut->bPopulated) {
                JPEGENC_DPRINT("%d in cond wait\n", __LINE__);
                pthread_cond_wait(&pComponentPrivate->unpopulate_cond, &pComponentPrivate->jpege_mutex_app);
            }
            pthread_mutex_unlock(&pComponentPrivate->jpege_mutex_app);
#endif

#ifdef __PERF_INSTRUMENTATION__
                        PERF_Boundary(pComponentPrivate->pPERFcomp,
                                      PERF_BoundaryComplete | PERF_BoundaryCleanup);
#endif

#ifdef RESOURCE_MANAGER_ENABLED
            if (pComponentPrivate->nCurState != OMX_StateWaitForResources) {
                eError= RMProxy_NewSendCommand(pHandle,  RMProxy_FreeResource, OMX_JPEG_Encoder_COMPONENT, 0, 3456, NULL);
                if (eError != OMX_ErrorNone) {
                    JPEGENC_DPRINT("Cannot Free Resources\n");                    
                    pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                           pHandle->pApplicationPrivate,
                                                           OMX_EventError,
                                                           OMX_ErrorUndefined,
                                                           OMX_TI_ErrorMajor,
                                                           NULL);
                    break;
                }
            }
#endif

            pComponentPrivate->nCurState = OMX_StateLoaded;            

            if ((pComponentPrivate->nCurState == OMX_StateIdle) &&
                 (pComponentPrivate->bPreempted == 1 )){
                
                pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                                       OMX_EventError,
                                                       OMX_ErrorResourcesLost,
                                                       OMX_TI_ErrorSevere,
                                                       NULL);
                pComponentPrivate->bPreempted = 0;
                
            }
            else {
                pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete,
                                                       OMX_CommandStateSet,
                                                       OMX_StateLoaded,
                                                       NULL);
            }
            
        } 
        else {
        
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventError, 
                                                   OMX_ErrorIncorrectStateTransition, 
                                                   OMX_TI_ErrorMinor, 
                                                   NULL);
        }
        break;

    case OMX_StateWaitForResources:
        if ( pComponentPrivate->nCurState == OMX_StateWaitForResources ) {
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                                            pComponentPrivate->pHandle->pApplicationPrivate,
                                                                            OMX_EventError, 
                                                                            OMX_ErrorSameState, 
                                                                            OMX_TI_ErrorMinor, 
                                                                            NULL);
        } else if ( pComponentPrivate->nCurState == OMX_StateLoaded ) {

#ifdef RESOURCE_MANAGER_ENABLED
            eError= RMProxy_NewSendCommand(pHandle, RMProxy_StateSet, OMX_JPEG_Encoder_COMPONENT, OMX_StateWaitForResources, 3456, NULL);
            if (eError != OMX_ErrorNone) {
                JPEGENC_DPRINT("RMProxy_NewSendCommand(OMX_StateWaitForResources) failed\n");
                pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                       pHandle->pApplicationPrivate,
                                                       OMX_EventError,
                                                       OMX_ErrorUndefined,
                                                       OMX_TI_ErrorSevere,
                                                       NULL);
                break;
            }
#endif
        
            pComponentPrivate->nCurState = OMX_StateWaitForResources;
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                                            pComponentPrivate->pHandle->pApplicationPrivate,
                                                                            OMX_EventCmdComplete, 
                                                                            OMX_CommandStateSet, 
                                                                            pComponentPrivate->nCurState, 
                                                                            NULL);
        } else {
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                                            pComponentPrivate->pHandle->pApplicationPrivate,
                                                                            OMX_EventError, 
                                                                            OMX_ErrorIncorrectStateTransition, 
                                                                            OMX_TI_ErrorMinor, 
                                                                            NULL);
        }
        break;

    case OMX_StateMax:
        JPEGENC_DPRINT("HandleCommand: Cmd OMX_StateMax::\n");
        break;
    } /* End of Switch */


    EXIT:
    return eError;
}


OMX_ERRORTYPE HandleJpegEncFreeOutputBufferFromApp(JPEGENC_COMPONENT_PRIVATE *pComponentPrivate )
{

    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE* pBuffHead = NULL;
    /* IUALG_Buf *ptParam = NULL; */
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDefOut = NULL;
    LCML_DSP_INTERFACE* pLcmlHandle = NULL;
    JPEGENC_BUFFER_PRIVATE* pBuffPrivate = NULL;
    int ret;

    JPEGENC_DPRINT("Inside HandleFreeOutputBufferFromApp function\n");
    
    pLcmlHandle = (LCML_DSP_INTERFACE *)(pComponentPrivate->pLCML);
    pPortDefOut = pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pPortDef;


    ret = read(pComponentPrivate->free_outBuf_Q[0], &pBuffHead, sizeof(pBuffHead));
    if ( ret == -1 ) {
        JPEGENC_DPRINT ("Error while reading from the pipe\n");
        goto EXIT;
    }

    if (pBuffHead != NULL) {
        pBuffPrivate = pBuffHead->pOutputPortPrivate;
    } else {
        JPEGENC_DPRINT ("Error while reading pBuffHead from the pipe\n");
        goto EXIT;
    }

    if (pComponentPrivate->nCurState != OMX_StatePause || pComponentPrivate->nToState != OMX_StatePause) {

   if ((pComponentPrivate->nCurState != OMX_StateExecuting ) ||
       (pComponentPrivate->nToState == OMX_StateIdle) ||
         (pPortDefOut->bEnabled == OMX_FALSE)) {
         if (pBuffPrivate->eBufferOwner != JPEGENC_BUFFER_CLIENT) {
            pBuffPrivate->eBufferOwner = JPEGENC_BUFFER_CLIENT;
            pComponentPrivate->nOutPortOut ++;
            JPEGENC_DPRINT("buffer summary (return empty output buffer) %d %d %d %d\n",
                    pComponentPrivate->nInPortIn,
                    pComponentPrivate->nInPortOut,
                    pComponentPrivate->nOutPortIn,
                    pComponentPrivate->nOutPortOut);       
            JPEGENC_DPRINT("FillBufferDone (incorrect state %d) %p\n", pComponentPrivate->nCurState, pBuffHead);
            pComponentPrivate->cbInfo.FillBufferDone(pComponentPrivate->pHandle,
                    pComponentPrivate->pHandle->pApplicationPrivate,
                    pBuffHead); 
         }
        goto EXIT;
    }
    }

    JPEGENC_DPRINT("buffer summary (HandleJpegEncFreeOutputBufferFromApp) %d %d %d %d\n",
                    pComponentPrivate->nInPortIn,
                    pComponentPrivate->nInPortOut,
                    pComponentPrivate->nOutPortIn,
                    pComponentPrivate->nOutPortOut); 

#ifdef __PERF_INSTRUMENTATION__
    PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                      pBuffHead->pBuffer,
                      0,
                      PERF_ModuleCommonLayer);
#endif

    /* ptParam =  (IUALG_Buf *)pBuffPrivate->pUALGParams; */
    pBuffPrivate->eBufferOwner = JPEGENC_BUFFER_DSP;

    JPEGENC_DPRINT("before queue output buffer %p\n", pBuffHead);
    eError = LCML_QueueBuffer(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                              EMMCodecOuputBuffer,
                              pBuffHead->pBuffer,
                              pPortDefOut->nBufferSize,
                              0,   
                              NULL, 
                              0,     
                              (OMX_U8 *)  pBuffHead);
    JPEGENC_DPRINT("after queue output buffer %p\n", pBuffHead);

    JPEGENC_DPRINT("Error is %x\n",eError);

    EXIT:
    return eError;
}

static OMX_U32 CalInParamBufSize(JPEGENC_COMPONENT_PRIVATE *pComponentPrivate)
{
    THUMBNAIL_INFO *pThumbnailInfo = &pComponentPrivate->ThumbnailInfo;
    OMX_U32 size = 0;
    int str_size;

    size ++;  /* for the first field: total size of buffer */


    if (!pThumbnailInfo->APP0_THUMB_INDEX) {
        size += 6;
    } else {
        size += 12; /* for INDEX, W and H */
        if (pThumbnailInfo->APP0_BUF.size > 0) {
          size += 2 + pThumbnailInfo->APP0_BUF.size / 4;
          if (pThumbnailInfo->APP0_BUF.size % 4) {
            size ++;
          }
        } else {
          // size += 4; /* for default APP0_BUF */
          size += 3;
        }
    }

    if (!pThumbnailInfo->APP1_THUMB_INDEX) {
        size += 6;
    } else {
        size += 12; /* for INDEX, W and H */
        if (pThumbnailInfo->APP1_BUF.size > 0) {
          size += 2 + pThumbnailInfo->APP1_BUF.size / 4;
          if (pThumbnailInfo->APP1_BUF.size % 4) {
            size ++;
          }
        } else {
          size += 4; /* for default APP1_BUF */
        }
    }
    if (!pThumbnailInfo->APP13_THUMB_INDEX) {
        size += 6;
    } else {
        size += 12; /* for INDEX, W and H */
        if (pThumbnailInfo->APP13_BUF.size > 0) {
          size += 2 + pThumbnailInfo->APP13_BUF.size / 4;
          if (pThumbnailInfo->APP13_BUF.size % 4) {
            size ++;
          }
        } else {
          size += 4; /* for default APP13_BUF */
        }
    }

    size ++; /* for the COMMENT_BUFFER */

    if (pComponentPrivate->nCommentFlag == 0 || pComponentPrivate->pString_Comment == NULL) {
        size += 3;
    } else {
        size ++; /* for storing the length of comment */
        str_size = strlen((char *)pComponentPrivate->pString_Comment);
        size += str_size / 4;
        if (str_size % 4) {
            size ++;
        }
    }

    if (pComponentPrivate->bSetLumaQuantizationTable && pComponentPrivate->bSetChromaQuantizationTable)
    {
        size += 64 /* Table Size 64 * 2 * 2 */ + 2 /* Content Type and Length */;
    }

    if (pComponentPrivate->bSetHuffmanTable)
    {
        size += 2 /* Content Type and Length */;
        if (sizeof(JPEGENC_CUSTOM_HUFFMAN_TABLE) % 4) {
            size += (sizeof(JPEGENC_CUSTOM_HUFFMAN_TABLE) + (4 - (sizeof(JPEGENC_CUSTOM_HUFFMAN_TABLE) % 4))) / 4 ;  
        }
        else {
           size += sizeof(JPEGENC_CUSTOM_HUFFMAN_TABLE) / 4;
        }
    }
    
    return (size * 4);
}

static OMX_ERRORTYPE SetJpegEncInPortParams(JPEGENC_COMPONENT_PRIVATE *pComponentPrivate, OMX_U32* new_params, OMX_U32 buf_size)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    int i = 1;
    OMX_U8 *p;
    int str_size;
    THUMBNAIL_INFO *pThumbnailInfo = &pComponentPrivate->ThumbnailInfo;

    new_params[0] = buf_size;

    /* Set Custom Quantization Table */
    if (pComponentPrivate->bSetLumaQuantizationTable && pComponentPrivate->bSetChromaQuantizationTable)
    {
        new_params[i++] = DYNPARAMS_QUANTTABLE;
        new_params[i++] = 256; /* 2 tables * 64 entries * 2(16bit entries) */
        OMX_U16 *temp = (OMX_U16 *)&new_params[i];
        int j, k;
        for (j = 0; j < 64; j++)
        {
            temp[j] = pComponentPrivate->pCustomLumaQuantTable->nQuantizationMatrix[j];
        }
        for (k = 0; k < 64; k++, j++)
        {
            temp[j] = pComponentPrivate->pCustomChromaQuantTable->nQuantizationMatrix[k];
        }
        i += 64; /* 256 / 4 */
    
    }

    /* Set Custom Huffman Table */
    if (pComponentPrivate->bSetHuffmanTable)
    {
        new_params[i++] = DYNPARAMS_HUFFMANTABLE;
        new_params[i++] = sizeof(JPEGENC_CUSTOM_HUFFMAN_TABLE); /* 2572 % 4 = 0 */

        memcpy((OMX_U8 *)(&new_params[i]), &(pComponentPrivate->pHuffmanTable->sHuffmanTable), sizeof(JPEGENC_CUSTOM_HUFFMAN_TABLE));
        if (sizeof(JPEGENC_CUSTOM_HUFFMAN_TABLE) % 4) {
            i += (sizeof(JPEGENC_CUSTOM_HUFFMAN_TABLE) + (4 - (sizeof(JPEGENC_CUSTOM_HUFFMAN_TABLE) % 4)))/4 ; 
        }
        else {
           i += sizeof(JPEGENC_CUSTOM_HUFFMAN_TABLE)/4;
        }
    }
    

    /* handle thumbnail in APP0 marker (JFIF)*/
    if (pThumbnailInfo->APP0_THUMB_INDEX) {

        new_params[i++] = APP0_NUMBUF;
        new_params[i++] = 4;
        new_params[i++] = 1;

        /* set default APP0 BUFFER */
        new_params[i++] = APP0_BUFFER;

        pThumbnailInfo->APP0_BUF.size = 0;
        if (pThumbnailInfo->APP0_BUF.size > 0) {
          OMX_U8 *t;
          new_params[i++] = pThumbnailInfo->APP0_BUF.size;
          t = (OMX_U8 *) &new_params[i];
          memcpy(t, pThumbnailInfo->APP0_BUF.app, pThumbnailInfo->APP0_BUF.size);
          i += pThumbnailInfo->APP0_BUF.size / 4;
          if (pThumbnailInfo->APP0_BUF.size % 4) {
            i ++;
          }
        } else {
          new_params[i++] = 4;
          new_params[i++] = 0; 
        }

        new_params[i++] = APP0_THUMB_INDEX;
        new_params[i++] = 4;
        new_params[i++] = 1;

        new_params[i++] = APP0_THUMB_W;
        new_params[i++] = 4;
        new_params[i++] = pThumbnailInfo->APP0_THUMB_W;

        new_params[i++] = APP0_THUMB_H;
        new_params[i++] = 4;
        new_params[i++] = pThumbnailInfo->APP0_THUMB_H;




    } else {
        new_params[i++] = APP0_THUMB_INDEX;
        new_params[i++] = 4;
        new_params[i++] = 0;

        new_params[i++] = APP0_NUMBUF;
        new_params[i++] = 4;
        new_params[i++] = 0;
    }



    /* handle thumbnail in APP1 marker (EXIF)*/
    if (pThumbnailInfo->APP1_THUMB_INDEX) {
        new_params[i++] = APP1_THUMB_INDEX;
        new_params[i++] = 4;
        new_params[i++] = 1;

        new_params[i++] = APP1_THUMB_W;
        new_params[i++] = 4;
        new_params[i++] = pThumbnailInfo->APP1_THUMB_W;

        new_params[i++] = APP1_THUMB_H;
        new_params[i++] = 4;
        new_params[i++] = pThumbnailInfo->APP1_THUMB_H;

        new_params[i++] = APP1_NUMBUF;
        new_params[i++] = 4;
        new_params[i++] = 1;

        /* set default APP1 BUFFER */
        
        new_params[i++] = APP1_BUFFER;

        if (pThumbnailInfo->APP1_BUF.size > 0) {
          OMX_U8 *t;
          new_params[i++] = pThumbnailInfo->APP1_BUF.size;
          t = (OMX_U8 *) &new_params[i];
          memcpy(t, pThumbnailInfo->APP1_BUF.app, pThumbnailInfo->APP1_BUF.size);
          i += pThumbnailInfo->APP1_BUF.size / 4;
          if (pThumbnailInfo->APP1_BUF.size % 4) {
            i ++;
          }
        } else {
          new_params[i++] = 8;
          new_params[i++] = 0; 
          new_params[i++] = 'F' | 'F' << 8 | 'F' << 16 | 'F' << 24; 
        }

    } else {
        new_params[i++] = APP1_THUMB_INDEX;
        new_params[i++] = 4;
        new_params[i++] = 0;

        new_params[i++] = APP1_NUMBUF;
        new_params[i++] = 4;
        new_params[i++] = 0;
    }

    /* handle thumbnail in APP13 marker */
    if (pThumbnailInfo->APP13_THUMB_INDEX) {
        new_params[i++] = APP13_THUMB_INDEX;
        new_params[i++] = 4;
        new_params[i++] = 1;

        new_params[i++] = APP13_THUMB_W;
        new_params[i++] = 4;
        new_params[i++] = pThumbnailInfo->APP13_THUMB_W;

        new_params[i++] = APP13_THUMB_H;
        new_params[i++] = 4;
        new_params[i++] = pThumbnailInfo->APP13_THUMB_H;

        new_params[i++] = APP13_NUMBUF;
        new_params[i++] = 4;
        new_params[i++] = 1;

        /* set default APP1 BUFFER */
        new_params[i++] = APP13_BUFFER;

        if (pThumbnailInfo->APP13_BUF.size > 0) {
          OMX_U8 *t;
          new_params[i++] = pThumbnailInfo->APP13_BUF.size;
          t = (OMX_U8 *) &new_params[i];
          memcpy(t, pThumbnailInfo->APP13_BUF.app, pThumbnailInfo->APP13_BUF.size);
          i += pThumbnailInfo->APP13_BUF.size / 4;
          if (pThumbnailInfo->APP13_BUF.size % 4) {
            i ++;
          }
        } else {
          new_params[i++] = 8;
          new_params[i++] = 0; 
          new_params[i++] = 'F' | 'F' << 8 | 'F' << 16 | 'F' << 24; 
        }
    } else {
        new_params[i++] = APP13_THUMB_INDEX;
        new_params[i++] = 4;
        new_params[i++] = 0;

        new_params[i++] = APP13_NUMBUF;
        new_params[i++] = 4;
        new_params[i++] = 0;
    }

    /*if(pComponentPrivate->nCommentFlag){*/
        new_params[i++] = COMMENT_BUFFER;
        if (pComponentPrivate->nCommentFlag == 1 && pComponentPrivate->pString_Comment) {
            str_size = strlen((char *)pComponentPrivate->pString_Comment);
            new_params[i++] = str_size  + 4 ;
            new_params[i++] = 0; /* index ? */
            p = (OMX_U8 *) &new_params[i];
            strncpy((char *)p, (char *)pComponentPrivate->pString_Comment, 255);
            /* new_params[0] = i * 4 + str_size + ((str_size % 4)? 4 - (str_size % 4) : 0); */
        }
        else {
            new_params[i++] = 5;
            new_params[i++] = 0;
            new_params[i++] = ' ';
            /* new_params[0] = i * 4; */
        }
/*    }*/
    return eError;
}

OMX_ERRORTYPE SetJpegEncInParams(JPEGENC_COMPONENT_PRIVATE *pComponentPrivate)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 buf_size;
    OMX_U8 *p = NULL;

    buf_size = CalInParamBufSize(pComponentPrivate);

    if (pComponentPrivate->InParams.pInParams) {
        p = (OMX_U8 *)pComponentPrivate->InParams.pInParams;
        p -= 128;
        FREE(p);
        pComponentPrivate->InParams.pInParams = NULL;
    }

    OMX_MALLOC_STRUCT_EXTRA(p, OMX_U8, (buf_size + 256 + 256 + 500 - 1));
    p += 128;
    pComponentPrivate->InParams.pInParams = (OMX_U32 *)p;
    p = NULL;
    eError = SetJpegEncInPortParams(pComponentPrivate, pComponentPrivate->InParams.pInParams, buf_size);

EXIT:
    return eError;
}

OMX_ERRORTYPE HandleJpegEncDataBuf_FromApp(JPEGENC_COMPONENT_PRIVATE *pComponentPrivate )
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE* pBuffHead = NULL;
    LCML_DSP_INTERFACE* pLcmlHandle = NULL; 
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDefIn = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDefOut = NULL;
    JPEGENC_BUFFER_PRIVATE* pBuffPrivate = NULL;
    int ret;

    OMX_CHECK_PARAM(pComponentPrivate);
    
    pLcmlHandle = (LCML_DSP_INTERFACE*)pComponentPrivate->pLCML;
    pPortDefIn = pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pPortDef;
    pPortDefOut = pComponentPrivate->pCompPort[JPEGENC_OUT_PORT]->pPortDef;

    JPEGENC_DPRINT ("Inside HandleDataBuf_FromApp function\n");
    ret = read(pComponentPrivate->filled_inpBuf_Q[0], &(pBuffHead), sizeof(pBuffHead));
    if ( ret == -1 ) {
        JPEGENC_DPRINT("Error while reading from the pipe\n");
    }

    if (pBuffHead != NULL) {
        pBuffPrivate = pBuffHead->pInputPortPrivate;
    } else {
        eError = OMX_ErrorInsufficientResources;
          goto EXIT;
        }

   if (pBuffPrivate->eBufferOwner == JPEGENC_BUFFER_CLIENT) { 
        /* already returned to client */
        JPEGENC_DPRINT("this buffer %p already returned to client\n", pBuffHead);
        goto EXIT;
    }
     
   if ((pComponentPrivate->nCurState != OMX_StateExecuting) || 
       (pComponentPrivate->nToState == OMX_StateIdle) ||
        (pPortDefIn->bEnabled == OMX_FALSE)) {
            if (pBuffPrivate->eBufferOwner != JPEGENC_BUFFER_CLIENT) {
                pComponentPrivate->nInPortOut ++;
                pBuffPrivate->eBufferOwner = JPEGENC_BUFFER_CLIENT;
                JPEGENC_DPRINT("buffer summary (return empty input buffer) %d %d %d %d\n",
                    pComponentPrivate->nInPortIn,
                    pComponentPrivate->nInPortOut,
                    pComponentPrivate->nOutPortIn,
                    pComponentPrivate->nOutPortOut);  
                pComponentPrivate->cbInfo.EmptyBufferDone(
                           pComponentPrivate->pHandle,
                           pComponentPrivate->pHandle->pApplicationPrivate,
                           pBuffHead);   
             }
             goto EXIT;
    }

    JPEGENC_DPRINT("buffer summary (HandleJpegEncDataBuf_FromApp) %d %d %d %d\n",
                    pComponentPrivate->nInPortIn,
                    pComponentPrivate->nInPortOut,
                    pComponentPrivate->nOutPortIn,
                    pComponentPrivate->nOutPortOut);  

#ifdef __PERF_INSTRUMENTATION__
    PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                      pBuffHead->pBuffer,
                      pBuffHead->nFilledLen,
                      PERF_ModuleCommonLayer);
#endif

    if ((pBuffHead->nFlags == OMX_BUFFERFLAG_EOS) && (pBuffHead->nAllocLen == 0)) {
            JPEGENC_DPRINT("BufferFlag Set!!\n");
            pComponentPrivate->nFlags = OMX_BUFFERFLAG_EOS;
        pBuffHead->nFlags = 0;
    }
#if 0
    eError = SendDynamicParam(pComponentPrivate);
    if (eError != OMX_ErrorNone ) {
            JPEGENC_DPRINT("SETSTATUS failed...  %x\n", eError);
            goto EXIT;
    }
#endif

    JPEGENC_DPRINT("pBuffHead->nAllocLen = %d\n",(int)pBuffHead->nAllocLen);
    JPEGENC_DPRINT("pBuffHead->pBuffer = %p\n",pBuffHead->pBuffer);
    JPEGENC_DPRINT("pBuffHead->nFilledLen = %d\n",(int)pBuffHead->nFilledLen);
    JPEGENC_DPRINT("pBuffHead = %p\n",pBuffHead);

    pBuffPrivate->eBufferOwner = JPEGENC_BUFFER_DSP;

    JPEGENC_DPRINT("Input: before queue buffer %p\n", pBuffHead);
        eError = LCML_QueueBuffer(
                                  pLcmlHandle->pCodecinterfacehandle,
                                  EMMCodecInputBuffer,
                                  pBuffHead->pBuffer,
                                  pPortDefIn->nBufferSize, 
                                  pBuffHead->nFilledLen,  
                                  (OMX_U8 *) pComponentPrivate->InParams.pInParams,
                                  pComponentPrivate->InParams.pInParams[0],
                                  (OMX_U8 *)pBuffHead); 

    JPEGENC_DPRINT("Input: after queue buffer %p\n", pBuffHead);

    if ( eError ) {
        eError = OMX_ErrorInsufficientResources;
        JPEGENC_DPRINT("ERROR: OMX_ErrorInsufficientResources\n");
        goto EXIT;
    }
    JPEGENC_DPRINT("Error is %x\n",eError);
    EXIT:
    
    return eError;
}


OMX_ERRORTYPE HandleJpegEncDataBuf_FromDsp(JPEGENC_COMPONENT_PRIVATE *pComponentPrivate, OMX_BUFFERHEADERTYPE* pBuffHead)
{

    OMX_ERRORTYPE eError                 = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE* pInpBuf        = NULL;
    JPEGENC_BUFFER_PRIVATE* pBuffPrivate = NULL;

    OMX_CHECK_PARAM(pComponentPrivate);

    pInpBuf = pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pBufferPrivate[0]->pBufferHdr;
    
    pBuffPrivate = pBuffHead->pOutputPortPrivate; 

        if (pBuffPrivate->eBufferOwner == JPEGENC_BUFFER_CLIENT) {
           JPEGENC_DPRINT("buffer %p already at the client side\n", pBuffHead);
           pComponentPrivate->nOutPortOut --;
           JPEGENC_DPRINT("buffer summary (FromDsp escape return output buffer) %d %d %d %d\n",
                                        pComponentPrivate->nInPortIn,
                                        pComponentPrivate->nInPortOut,
                                        pComponentPrivate->nOutPortIn,
                                        pComponentPrivate->nOutPortOut);

           goto EXIT;
        }
    
#ifdef __PERF_INSTRUMENTATION__
        PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                          pBuffHead->pBuffer,
                          pBuffHead->nFilledLen,
                          PERF_ModuleHLMM);
#endif
    pBuffPrivate->eBufferOwner = JPEGENC_BUFFER_CLIENT;

        if (pBuffHead->pMarkData) {
           JPEGENC_DPRINT("get Mark buffer %p %p %p\n", pBuffHead->pMarkData, pBuffHead->hMarkTargetComponent, pComponentPrivate->pHandle);
        }

        if (pBuffHead->pMarkData && pBuffHead->hMarkTargetComponent == pComponentPrivate->pHandle) {
           JPEGENC_DPRINT("send OMX_MarkEvent\n");
           pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                pComponentPrivate->pHandle->pApplicationPrivate,
                                                OMX_EventMark,
                                                JPEGENC_OUT_PORT,
                                                0,
                                                pBuffHead->pMarkData);
        }

    JPEGENC_DPRINT("buffer summary (return empty output buffer) %d %d %d %d\n",
                    pComponentPrivate->nInPortIn,
                    pComponentPrivate->nInPortOut,
                    pComponentPrivate->nOutPortIn,
                    pComponentPrivate->nOutPortOut);   
    JPEGENC_DPRINT("Output: before fillbufferdone %p\n", pBuffHead);
        pComponentPrivate->cbInfo.FillBufferDone(pComponentPrivate->pHandle,
                        pComponentPrivate->pHandle->pApplicationPrivate,
                        pBuffHead);
  
    if ( pComponentPrivate->nFlags & OMX_BUFFERFLAG_EOS )   {

        pBuffHead->nFlags |= OMX_BUFFERFLAG_EOS;
        pComponentPrivate->cbInfo.EventHandler (pComponentPrivate->pHandle, 
                                                pComponentPrivate->pHandle->pApplicationPrivate,
                                                OMX_EventBufferFlag, 
                                                JPEGENC_OUT_PORT, 
                                                OMX_BUFFERFLAG_EOS, 
                                                NULL);

        pComponentPrivate->nFlags = 0;
    }

    EXIT:
    return eError;
}



OMX_ERRORTYPE HandleJpegEncFreeDataBuf( JPEGENC_COMPONENT_PRIVATE *pComponentPrivate, OMX_BUFFERHEADERTYPE* pBuffHead )
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDefIn = NULL;
    JPEGENC_BUFFER_PRIVATE* pBuffPrivate = NULL;
    OMX_HANDLETYPE hTunnelComponent = pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->hTunnelComponent;

    JPEGENC_DPRINT("Inside HandleFreeDataBuf function \n");
    OMX_CHECK_PARAM(pComponentPrivate);
    pPortDefIn = pComponentPrivate->pCompPort[JPEGENC_INP_PORT]->pPortDef;

    /* pBuffHead->nAllocLen = pPortDefIn->nBufferSize; */
    pBuffPrivate = pBuffHead->pInputPortPrivate;
    
    JPEGENC_DPRINT("hTunnelComponent = %p\n" ,hTunnelComponent );
    JPEGENC_DPRINT("pComponentPrivate->pHandle = %p\n",pComponentPrivate->pHandle);

    
    if (pBuffPrivate->eBufferOwner == JPEGENC_BUFFER_CLIENT) {
        JPEGENC_DPRINT("buffer %p already at the client side\n", pBuffHead);
        pComponentPrivate->nInPortOut --;
        JPEGENC_DPRINT("buffer summary (FromDsp escape return input buffer) %d %d %d %d\n",
                                        pComponentPrivate->nInPortIn,
                                        pComponentPrivate->nInPortOut,
                                        pComponentPrivate->nOutPortIn,
                                        pComponentPrivate->nOutPortOut);

           goto EXIT;
    }

    if(hTunnelComponent != NULL)
    {

#ifdef __PERF_INSTRUMENTATION__
            PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                              pBuffHead->pBuffer,
                              0,
                              PERF_ModuleLLMM);
#endif
    pBuffPrivate->eBufferOwner = JPEGENC_BUFFER_TUNNEL_COMPONENT;
        eError = OMX_FillThisBuffer(hTunnelComponent, pBuffHead);

    }
    else {    

#ifdef __PERF_INSTRUMENTATION__
            PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                              pBuffHead->pBuffer,
                              0,
                              PERF_ModuleHLMM);
#endif
        pBuffPrivate->eBufferOwner = JPEGENC_BUFFER_CLIENT;
        JPEGENC_DPRINT("before emptybufferdone in HandleJpegEncFreeDataBuf %p\n", pBuffHead);
        pComponentPrivate->cbInfo.EmptyBufferDone(
                 pComponentPrivate->pHandle,
                 pComponentPrivate->pHandle->pApplicationPrivate,
                 pBuffHead);
    }

    EXIT:
    return eError;

}


/* -------------------------------------------------------------------*/
/**
  *  Callback() function will be called LCML component to write the msg
  *
  * @param msgBuffer                 This buffer will be returned by the LCML
  *
  * @retval OMX_NoError              Success, ready to roll
  *         OMX_Error_BadParameter   The input parameter pointer is null
 **/
/*-------------------------------------------------------------------*/
OMX_ERRORTYPE JpegEncLCML_Callback (TUsnCodecEvent event,void * argsCb [10])
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    JPEGENC_BUFFER_PRIVATE *pBuffPrivate = NULL;
    JPEG_PORT_TYPE *pPortType = NULL;
    int i;

    JPEGENC_COMPONENT_PRIVATE *pComponentPrivate = NULL;
    OMX_COMPONENTTYPE *pHandle;

    JPEGENC_DPRINT("Event = %d\n", event);
    if ( ((LCML_DSP_INTERFACE*)argsCb[6] ) != NULL ) {
        pComponentPrivate = (JPEGENC_COMPONENT_PRIVATE*)((LCML_DSP_INTERFACE*)argsCb[6])->pComponentPrivate;
        pHandle = (OMX_COMPONENTTYPE *)pComponentPrivate->pHandle; 
    }
    else {
    JPEGENC_DPRINT("wrong in LCML callback, exit\n");
    goto EXIT;
    }

    if ( event == EMMCodecBufferProcessed ) {
    if ( (int)argsCb [0] == EMMCodecOuputBuffer ) {    
       OMX_BUFFERHEADERTYPE* pBuffHead = (OMX_BUFFERHEADERTYPE*)argsCb[7];
       pBuffPrivate = pBuffHead->pOutputPortPrivate;

       pComponentPrivate->nOutPortOut ++;
#ifdef __PERF_INSTRUMENTATION__
                    PERF_ReceivedFrame(pComponentPrivate->pPERFcomp,
                                       PREF(pBuffHead,pBuffer),
                                       PREF(pBuffHead,nFilledLen),
                                       PERF_ModuleCommonLayer);
#endif
        JPEGENC_DPRINT("argsCb[8] is %d\n", argsCb[8]);
        pBuffHead->nFilledLen = (OMX_U32) argsCb[8];

        JPEGENC_DPRINT("buffer summary (LCML for output buffer %p) %d %d %d %d\n", pBuffHead, 
                    pComponentPrivate->nInPortIn,
                    pComponentPrivate->nInPortOut,
                    pComponentPrivate->nOutPortIn,
                    pComponentPrivate->nOutPortOut);

        pPortType = pComponentPrivate->pCompPort[JPEGENC_INP_PORT];
        for (i = 0; i < pPortType->pPortDef->nBufferCountActual; i ++) {
            if (pComponentPrivate->nOutPortOut > 10) {
                JPEGENC_DPRINT("pPortType->sBufferFlagTrack[i].buffer_id %d\n", pPortType->sBufferFlagTrack[i].buffer_id);
            }
            if (pPortType->sBufferFlagTrack[i].buffer_id == pComponentPrivate->nOutPortOut) {
                JPEGENC_DPRINT("output buffer %d has flag %x\n", 
                           pPortType->sBufferFlagTrack[i].buffer_id, 
                           pPortType->sBufferFlagTrack[i].flag);
                pBuffHead->nFlags = pPortType->sBufferFlagTrack[i].flag;
                pPortType->sBufferFlagTrack[i].flag = 0;
                pPortType->sBufferFlagTrack[i].buffer_id = 0xFFFFFFFF;
                break;
            }
        }
        for (i = 0; i < pPortType->pPortDef->nBufferCountActual; i ++) {
            if (pPortType->sBufferMarkTrack[i].buffer_id == pComponentPrivate->nInPortOut) {
                JPEGENC_DPRINT("buffer ID %d has mark (output port)\n", pPortType->sBufferMarkTrack[i].buffer_id);
                pBuffHead->pMarkData = pPortType->sBufferMarkTrack[i].pMarkData;
                pBuffHead->hMarkTargetComponent = pPortType->sBufferMarkTrack[i].hMarkTargetComponent;
                pPortType->sBufferMarkTrack[i].buffer_id = 0xFFFFFFFF;
                break;
            }
        }
        
        JPEGENC_DPRINT("EMMCodec Args -> %x, %p\n", (int)argsCb[1] , (int)argsCb[5]);
        if (pBuffPrivate->eBufferOwner != JPEGENC_BUFFER_CLIENT) {
            JPEGENC_DPRINT("return output buffer %p from LCML_Callback (%d)\n", 
                           pBuffHead, 
                           pBuffPrivate->eBufferOwner);
            pBuffPrivate->eBufferOwner = JPEGENC_BUFFER_COMPONENT_OUT;
            JPEGENC_DPRINT("LCML_Callback - Filled (output) Data from DSP %p\n", pBuffHead);
            eError = HandleJpegEncDataBuf_FromDsp(pComponentPrivate, pBuffHead);
        }
    }

    if ((int) argsCb [0] == EMMCodecInputBuffer ) {   
        OMX_BUFFERHEADERTYPE* pBuffHead = (OMX_BUFFERHEADERTYPE*)argsCb[7];
        pBuffPrivate = pBuffHead->pInputPortPrivate;

       pComponentPrivate->nInPortOut ++;
        JPEGENC_DPRINT("buffer summary (LCML for InputBuffer %p) %d %d %d %d\n", pBuffHead,
                    pComponentPrivate->nInPortIn,
                    pComponentPrivate->nInPortOut,
                    pComponentPrivate->nOutPortIn,
                    pComponentPrivate->nOutPortOut);
#ifdef __PERF_INSTRUMENTATION__
                        PERF_ReceivedFrame(pComponentPrivate->pPERFcomp,
                                           PREF(pBuffHead,pBuffer),
                                           0,
                                           PERF_ModuleCommonLayer);
#endif
        JPEGENC_DPRINT("EMMCodec Args -> %x, %p\n", (int)argsCb[1] , (int)argsCb[5]);
        if (pBuffPrivate->eBufferOwner != JPEGENC_BUFFER_CLIENT) {
            JPEGENC_DPRINT("return input buffer %p from LCML_Callback (%d)\n", 
                           pBuffHead, 
                           pBuffPrivate->eBufferOwner);
            pBuffPrivate->eBufferOwner = JPEGENC_BUFFER_COMPONENT_OUT;
            JPEGENC_DPRINT("LCML_Callback - Emptied (input) Data from DSP %p\n", pBuffHead);
            eError = HandleJpegEncFreeDataBuf(pComponentPrivate, pBuffHead);
        }
    }
    goto EXIT;
    } /* end     if ( event == EMMCodecBufferProcessed ) */

    if ( event == EMMCodecProcessingStoped ) {
        JPEGENC_DPRINT( "Entering To EMMCodecProcessingStoped \n");
        JPEGENC_DPRINT("buffer summary (Stopped) %d %d %d %d\n",
                    pComponentPrivate->nInPortIn,
                    pComponentPrivate->nInPortOut,
                    pComponentPrivate->nOutPortIn,
                    pComponentPrivate->nOutPortOut);
        pComponentPrivate->bDSPStopAck = OMX_TRUE;
        JPEGENC_DPRINT("to state is %d\n", pComponentPrivate->nToState);

        
        /* if (pComponentPrivate->nToState == OMX_StateIdle) { */
            pComponentPrivate->ExeToIdleFlag |= JPEGE_DSPSTOP;
        /* } */
        
        JPEGENC_DPRINT("before stop signal\n");

        pthread_mutex_lock(&pComponentPrivate->jpege_mutex);
        pthread_cond_signal(&pComponentPrivate->stop_cond);
        pthread_mutex_unlock(&pComponentPrivate->jpege_mutex);

        JPEGENC_DPRINT("after stop signal\n");

        goto EXIT;
    }

    if ( event == EMMCodecDspError ) {
    
       JPEGENC_DPRINT("in EMMCodecDspError EMMCodec Args -> %x, %x\n", (int)argsCb[4] , (int)argsCb[5]);
       if ((int)argsCb[4] != 0x1 || (int)argsCb[5] != 0x500) {
           JPEGENC_DPRINT("DSP Error %x %x\n", argsCb[4], argsCb[5]);
           pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                     pComponentPrivate->pHandle->pApplicationPrivate,
                                     OMX_EventError, 
                                     OMX_ErrorHardware, 
                                     OMX_TI_ErrorCritical,
                                     NULL);
           goto EXIT;
       }
    }
    if (event == EMMCodecInternalError) {
        eError = OMX_ErrorHardware;
        JPEGENC_DPRINT("JPEG-E: EMMCodecInternalError\n");
        pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                               pComponentPrivate->pHandle->pApplicationPrivate,
                                               OMX_EventError,
                                               OMX_ErrorHardware, 
                                               OMX_TI_ErrorCritical,
                                               NULL);
        goto EXIT;
    }
    if ( event == EMMCodecProcessingPaused ) {
        JPEGENC_DPRINT( "ENTERING TO EMMCodecProcessingPaused JPEG Encoder\n");
        if (pComponentPrivate != NULL) {
            pComponentPrivate->bDSPStopAck = OMX_TRUE;
            pComponentPrivate->nCurState = OMX_StatePause;
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventCmdComplete, OMX_CommandStateSet, pComponentPrivate->nCurState, NULL);
        }
    }
    if (event == EMMCodecStrmCtrlAck) {
        JPEGENC_DPRINT("EMMCodecStrmCtrlAck\n");
        if ((int)argsCb [0] == USN_ERR_NONE) {
            JPEGENC_DPRINT("Callback: no error\n");
            pComponentPrivate->bFlushComplete = OMX_TRUE;
            pthread_mutex_lock(&pComponentPrivate->jpege_mutex);
            pthread_cond_signal(&pComponentPrivate->flush_cond);
            pthread_mutex_unlock(&pComponentPrivate->jpege_mutex);        
        }
    }
    if (event == EMMCodecAlgCtrlAck) {
        JPEGENC_DPRINT("jpeg-enc: EMMCodecAlgCtrlAck\n"); 
        pComponentPrivate->bAckFromSetStatus = 1;
        /*
        pthread_mutex_lock(&pComponentPrivate->jpege_mutex);
        pthread_cond_signal(&pComponentPrivate->control_cond);
        pthread_mutex_unlock(&pComponentPrivate->jpege_mutex);
        */
    }

EXIT:
    JPEGENC_DPRINT( "Exiting the LCML_Callback function\n");
    return eError;
}
/*-------------------------------------------------------------------*/
/**
  * IsTIOMXComponent()
  *
  * Check if the component is TI component.
  *
  * @param hTunneledComp Component Tunnel Pipe
  *  
  * @retval OMX_TRUE   Input is a TI component.
  *         OMX_FALSE  Input is a not a TI component. 
  *
  **/
/*-------------------------------------------------------------------*/
OMX_BOOL IsTIOMXComponent(OMX_HANDLETYPE hComp)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_STRING pTunnelcComponentName = NULL;
    OMX_VERSIONTYPE* pTunnelComponentVersion = NULL;
    OMX_VERSIONTYPE* pSpecVersion = NULL;
    OMX_UUIDTYPE* pComponentUUID = NULL;
    char *pSubstring = NULL;
    OMX_BOOL bResult = OMX_TRUE;

    OMX_MALLOC_STRUCT_EXTRA(pTunnelcComponentName, char, 127);
    OMX_MALLOC_STRUCT(pTunnelComponentVersion, OMX_VERSIONTYPE);
    OMX_MALLOC_STRUCT(pSpecVersion, OMX_VERSIONTYPE);    
    OMX_MALLOC_STRUCT(pComponentUUID, OMX_UUIDTYPE);    

    eError = OMX_GetComponentVersion (hComp, pTunnelcComponentName, pTunnelComponentVersion, pSpecVersion, pComponentUUID);

    /* Check if tunneled component is a TI component */
    pSubstring = strstr(pTunnelcComponentName, "OMX.TI.");

    if(pSubstring == NULL) {
        bResult = OMX_FALSE;
    }

EXIT:
    FREE(pTunnelcComponentName);
    FREE(pTunnelComponentVersion);
    FREE(pSpecVersion);
    FREE(pComponentUUID);

    return bResult;
} /* End of IsTIOMXComponent */



#ifdef RESOURCE_MANAGER_ENABLED
/* ========================================================================== */
/**
 *  ResourceManagerCallback() - handle callbacks from Resource Manager
 * @param cbData    Resource Manager Command Data Structure
 * @return: void
  **/
/* ========================================================================== */

void ResourceManagerCallback(RMPROXY_COMMANDDATATYPE cbData)
{
    OMX_COMMANDTYPE Cmd = OMX_CommandStateSet;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)cbData.hComponent;
    JPEGENC_COMPONENT_PRIVATE *pComponentPrivate = NULL;
    OMX_ERRORTYPE RM_Error = *(cbData.RM_Error);
    
    JPEGENC_DPRINT("%s: %d: RM_Error = %x\n", __FUNCTION__, __LINE__, RM_Error);

    pComponentPrivate = (JPEGENC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;

    if (RM_Error == OMX_RmProxyCallback_ResourcesPreempted) {

        pComponentPrivate->bPreempted = 1;
        
        if (pComponentPrivate->nCurState == OMX_StateExecuting || 
            pComponentPrivate->nCurState == OMX_StatePause) {

            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventError,
                                                   OMX_ErrorResourcesPreempted,
                                                   OMX_TI_ErrorSevere,
                                                   NULL);
            
            pComponentPrivate->nToState = OMX_StateIdle;
            JPEGENC_DPRINT("Component Preempted. Going to IDLE State.\n");
        }
        else if (pComponentPrivate->nCurState == OMX_StateIdle){
            pComponentPrivate->nToState = OMX_StateLoaded;
            JPEGENC_DPRINT("Component Preempted. Going to LOADED State.\n");            
        }
        
#ifdef __PERF_INSTRUMENTATION__
        PERF_SendingCommand(pComponentPrivate->pPERF, Cmd, pComponentPrivate->nToState, PERF_ModuleComponent);
#endif
        
        write (pComponentPrivate->nCmdPipe[1], &Cmd, sizeof(Cmd));
        write (pComponentPrivate->nCmdDataPipe[1], &(pComponentPrivate->nToState) ,sizeof(OMX_U32));
        
    }
    else if (RM_Error == OMX_RmProxyCallback_ResourcesAcquired ){

        if (pComponentPrivate->nCurState == OMX_StateWaitForResources) /* Wait for Resource Response */
        {
            pComponentPrivate->cbInfo.EventHandler (
                            pHandle, pHandle->pApplicationPrivate,
                            OMX_EventResourcesAcquired, 0,0,
                            NULL);
            
            pComponentPrivate->nToState = OMX_StateIdle;
            
#ifdef __PERF_INSTRUMENTATION__
            PERF_SendingCommand(pComponentPrivate->pPERF, Cmd, pComponentPrivate->nToState, PERF_ModuleComponent);
#endif
        
            write (pComponentPrivate->nCmdPipe[1], &Cmd, sizeof(Cmd));
            write (pComponentPrivate->nCmdDataPipe[1], &(pComponentPrivate->nToState) ,sizeof(OMX_U32));
            JPEGENC_DPRINT("OMX_RmProxyCallback_ResourcesAcquired.\n");
        }            
        
    }

}
#endif


