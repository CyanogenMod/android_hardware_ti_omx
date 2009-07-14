
/*
 *  Copyright 2001-2008 Texas Instruments - http://www.ti.com/
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* =============================================================================
*             Texas Instruments OMAP (TM) Platform Software
*  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
*
*  Use of this software is controlled by the terms and conditions found
*  in the license agreement under which this software has been supplied.
* =========================================================================== */
/**
* @file OMX_G723DecTest.c
*
* This file contains the test application code that invokes the component.
*
* @path  $(CSLPATH)\OMAPSW_MPU\linux\audio\src\openmax_il\g723_dec\tests
*
* @rev  1.0
*/
/* ----------------------------------------------------------------------------
*!
*! Revision History
*! ===================================
*! This is newest file
* =========================================================================== */
/* ------compilation control switches -------------------------*/
/****************************************************************
*  INCLUDE FILES
****************************************************************/
/* ----- system and platform files ----------------------------*/
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/vt.h>
#include <signal.h>
#include <sys/stat.h>
#include <OMX_Index.h>
#include <OMX_Types.h>
#include <OMX_Core.h>
#include <OMX_Audio.h>
#include <TIDspOmx.h>
#include <pthread.h>
#include <stdio.h>
#ifdef DSP_RENDERING_ON    
#include <AudioManagerAPI.h>
#endif
#include <linux/soundcard.h>
#include <OMX_G723Dec_Utils.h>

#ifdef OMX_GETTIME
#include <OMX_Common_Utils.h>
#include <OMX_GetTime.h>     /*Headers for Performance & measuremet    */
#endif

#undef MTRACE
#ifdef MTRACE
#include <mcheck.h>
#endif


#undef APP_DEBUG
#undef APP_STATEDETAILS
#undef USE_BUFFER

#ifdef APP_DEBUG
    #define APP_DPRINT(...)    fprintf(stderr,__VA_ARGS__)
#else
    #define APP_DPRINT(...)
#endif


#ifdef APP_STATEDETAILS
    #define APP_STATEPRINT(...)    fprintf(stderr,__VA_ARGS__)
#else
    #define APP_STATEPRINT(...)
#endif

#define APP_OUTPUT_FILE "g723output.pcm"
#define SLEEP_TIME 2
#define VERSION_MINOR 1
#define VERSION_MAJOR 1
#define MONO_CHANNEL 1
#define BIT_63 6300
#define BIT_53 5300
#define G723_APP_ID  100 /* Defines G723 Dec App ID, App must use this value */
#define INVALID_SAMPLING_FREQ  51 /*Do not change; fixed for component!*/
#define MAX_NUM_OF_BUFS 10         /*Do not change; fixed for component!*/
#define EOS_MIN_SIZE 2
#define BITS_PER_SAMPLE 16
#define SAMPLE_RATE 8000
#define EXTRA_BYTES 256
#define CACHE_ALIGN 128

#define FIFO1 "/dev/fifo.1"
#define FIFO2 "/dev/fifo.2"


#ifdef OMX_GETTIME
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  int GT_FlagE = 0;  /* Fill Buffer 1 = First Buffer,  0 = Not First Buffer  */
  int GT_FlagF = 0;  /*Empty Buffer  1 = First Buffer,  0 = Not First Buffer  */
  static OMX_NODE* pListHead = NULL;
#endif

int nIpBuffs = 1;
int nOpBuffs = 1;
int packetsPerBuffer = 1;

int nIpBufSize = 24; /*default value */
int nOpBufSize = 480; /* default value */
int currentFrameSize = 24;

int gDasfMode = 0;
int playcompleted = 0;
int sendlastbuffer = 0;
int fdwrite = 0;
int fdread = 0;
OMX_STATETYPE gState = OMX_StateExecuting;
int timeToExit=0;
int preempted=0;
static OMX_BOOL bInvalidState = OMX_FALSE;



/* FILE* fCheckFile= NULL; */

int maxint(int a, int b);

int fill_data (OMX_BUFFERHEADERTYPE *pBuf, FILE *fIn);

OMX_STRING strG723Decoder = "OMX.TI.G723.decode";

int IpBuf_Pipe[2] = {0};
int OpBuf_Pipe[2] = {0};
int g729Event_Pipe[2] = {0};

OMX_ERRORTYPE send_input_buffer (OMX_HANDLETYPE pHandle, OMX_BUFFERHEADERTYPE* pBuffer, FILE *fIn);

fd_set rfds;


/* ================================================================================= * */
/**
* @fn maxint() gives the maximum of two integers.
*
* @param a intetger a
*
* @param b integer b
*
* @pre          None
*
* @post         None
*
*  @return      bigger number
*
*  @see         None
*/
/* ================================================================================ * */
int maxint(int a, int b)
{
    return (a>b) ? a : b;
}

/* ================================================================================= * */
/**
* @fn WaitForState() Waits for the state to change.
*
* @param pHandle This is component handle allocated by the OMX core.
*
* @param DesiredState This is state which the app is waiting for.
*
* @pre          None
*
* @post         None
*
*  @return      Appropriate OMX Error.
*
*  @see         None
*/
/* ================================================================================ * */
static OMX_ERRORTYPE WaitForState(OMX_HANDLETYPE pHandle,
                                  OMX_STATETYPE DesiredState)
{
    OMX_STATETYPE CurState = OMX_StateInvalid;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    int nCnt = 0;

    eError = OMX_GetState(pHandle, &CurState);
    while( (eError == OMX_ErrorNone) &&
            (CurState != DesiredState) && (eError == OMX_ErrorNone) ) {
        sched_yield();
        if(nCnt++ == 10) {
            APP_DPRINT( "Still Waiting, press CTL-C to continue\n");
        }
        eError = OMX_GetState(pHandle, &CurState);
    }

    return eError;
}


/* ================================================================================= * */
/**
* @fn EventHandler() This is event handle which is called by the component on
* any event change.
*
* @param hComponent This is component handle allocated by the OMX core.
*
* @param pAppData This is application private data.
*
* @param eEvent  This is event generated from the component.
*
* @param nData1  Data1 associated with the event.
*
* @param nData2  Data2 associated with the event.
*
* @param pEventData Any other string event data from component.
*
* @pre          None
*
* @post         None
*
*  @return      Appropriate OMX error.
*
*  @see         None
*/
/* ================================================================================ * */
OMX_ERRORTYPE EventHandler(OMX_HANDLETYPE hComponent,OMX_PTR pAppData,OMX_EVENTTYPE eEvent,
                           OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData)
{
    OMX_U8 writeValue = 0;
    
    switch (eEvent)
    {
        case OMX_EventResourcesAcquired:
            writeValue = 1;
            write(g729Event_Pipe[1], &writeValue, sizeof(OMX_U8));
            preempted=0;
            break;
            
        case OMX_EventCmdComplete:
            gState = (OMX_STATETYPE)nData2;
            break;
        case OMX_EventError:
            if (nData1 == OMX_ErrorInvalidState) {
                bInvalidState =OMX_TRUE;
            }            
            else if(nData1 == OMX_ErrorResourcesPreempted) {
                preempted=1;
                writeValue = 0;  
                write(g729Event_Pipe[1], &writeValue, sizeof(OMX_U8));
            }
           break;
        case OMX_EventMax:
        case OMX_EventMark:
        case OMX_EventPortSettingsChanged:
        case OMX_EventBufferFlag:
            writeValue = 2;  
            write(g729Event_Pipe[1], &writeValue, sizeof(OMX_U8));
            playcompleted = 1;
        break;

        case OMX_EventComponentResumed:
        case OMX_EventDynamicResourcesAvailable:
        case OMX_EventPortFormatDetected:
        default:
            break;
    }

    return OMX_ErrorNone;
}

/* ================================================================================= * */
/**
* @fn FillBufferDone() Component sens the output buffer to app using this
* callback.
*
* @param hComponent This is component handle allocated by the OMX core.
*
* @param ptr This is another pointer.
*
* @param pBuffer This is output buffer.
*
* @pre          None
*
* @post         None
*
*  @return      None
*
*  @see         None
*/
/* ================================================================================ * */
void FillBufferDone (OMX_HANDLETYPE hComponent, OMX_PTR ptr, OMX_BUFFERHEADERTYPE* pBuffer)
{
    write(OpBuf_Pipe[1], &pBuffer, sizeof(pBuffer));
#ifdef OMX_GETTIME
    if (GT_FlagF == 1 ) /* First Buffer Reply*/  /* 1 = First Buffer,  0 = Not First Buffer  */
    {
        GT_END("Call to FillBufferDone  <First: FillBufferDone>");
        GT_FlagF = 0 ;   /* 1 = First Buffer,  0 = Not First Buffer  */
    }
#endif

}


/* ================================================================================= * */
/**
* @fn EmptyBufferDone() Component sends the input buffer to app using this
* callback.
*
* @param hComponent This is component handle allocated by the OMX core.
*
* @param ptr This is another pointer.
*
* @param pBuffer This is input buffer.
*
* @pre          None
*
* @post         None
*
*  @return      None
*
*  @see         None
*/
/* ================================================================================ * */
void EmptyBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR ptr, OMX_BUFFERHEADERTYPE* pBuffer)
{
    if (!preempted) {
        write(IpBuf_Pipe[1], &pBuffer, sizeof(pBuffer));
#ifdef OMX_GETTIME
        if (GT_FlagE == 1 ) /* First Buffer Reply*/  /* 1 = First Buffer,  0 = Not First Buffer  */
        {
            GT_END("Call to EmptyBufferDone <First: EmptyBufferDone>");
            GT_FlagE = 0;   /* 1 = First Buffer,  0 = Not First Buffer  */
        }
#endif
    }
}

/* ================================================================================= * */
/**
* @fn main() This is the main function of application which gets called.
*
* @param argc This is the number of commandline arguments..
*
* @param argv[] This is an array of pointers to command line arguments..
*
* @pre          None
*
* @post         None
*
*  @return      An integer value.
*
*  @see         None
*/
/* ================================================================================ * */
int main(int argc, char* argv[])
{
    OMX_CALLBACKTYPE G723CaBa = {(void *)EventHandler,
                                                          (void*)EmptyBufferDone,
                                                          (void*)FillBufferDone};
    OMX_HANDLETYPE pHandle = NULL;
    OMX_ERRORTYPE error = OMX_ErrorNone;
    OMX_U32 AppData = G723_APP_ID;
    OMX_PARAM_PORTDEFINITIONTYPE* pCompPrivateStruct = NULL;
    OMX_AUDIO_CONFIG_MUTETYPE* pCompPrivateStructMute = NULL;
    OMX_AUDIO_CONFIG_VOLUMETYPE* pCompPrivateStructVolume = NULL;
    OMX_AUDIO_PARAM_PCMMODETYPE *pPcmParam = NULL;
    OMX_AUDIO_PARAM_G723TYPE *pG723Param = NULL;
    TI_OMX_DSP_DEFINITION  *pAppPrivate = NULL;
    OMX_INDEXTYPE index = 0;
    OMX_BUFFERHEADERTYPE* pInputBufferHeader[MAX_NUM_OF_BUFS] = {NULL};
    OMX_BUFFERHEADERTYPE* pOutputBufferHeader[MAX_NUM_OF_BUFS] = {NULL};
#ifdef DSP_RENDERING_ON    
    AM_COMMANDDATATYPE cmd_data;
#endif
    TI_OMX_DATAPATH dataPath;
    
#ifdef USE_BUFFER    
    OMX_U8* pInputBuffer[MAX_NUM_OF_BUFS] ={NULL};
    OMX_U8* pOutputBuffer[MAX_NUM_OF_BUFS]={NULL};
#endif    

    int i=0;
    static int totalFilled = 0;
    int k=0;
    struct timeval tv;
    int retval = 0, j = 0;
    int frmCount = 0;
    int frmCnt = 1;
    int testcnt = 1;
    int testcnt1 = 1;
    char fname[100] = APP_OUTPUT_FILE;
    FILE* fIn = NULL;
    FILE* fOut = NULL;
    int tcID = 0;
    int fdmax = 0;
    OMX_BUFFERHEADERTYPE* pBuffer = NULL;
     
    APP_DPRINT("------------------------------------------------------\n");
    APP_DPRINT("This is Main Thread In G723 DECODER Test Application:\n");
    APP_DPRINT("Test Core 1.5 - " __DATE__ ":" __TIME__ "\n");
    APP_DPRINT("------------------------------------------------------\n");

#ifdef MTRACE
    mtrace();
#endif

#ifdef OMX_GETTIME
    GTeError = OMX_ListCreate(&pListHead);
    printf("eError = %d\n",GTeError);
    GT_START();
#endif  

    /* check the input parameters */
    if(argc != 10) {
        printf("Wrong Arguments: See Below:\n\n");
        printf("%d :: Usage: [TestApp] [Input File] [Input File Bit Rate] [TC ID]", __LINE__);
        printf("[FM/DM] [NB INPUT BUF] [INPUT BUF SIZE] [NB OUTPUT BUF]");
        printf("[OUTPUT BUF SIZE] [PACKETS PER BUFFER\n");
        goto EXIT;
    }

    /* check to see that the input file exists */
    struct stat sb = {0};
    int status = stat(argv[1], &sb);
    if( status != 0 )
    {
        printf( "Cannot find file %s. (%u)\n", argv[1], errno);
        goto EXIT;
    }

    error = TIOMX_Init();
    if(error != OMX_ErrorNone) {
        APP_DPRINT("%d :: Error returned by TIOMX_Init()\n",__LINE__);
        goto EXIT;
    }

    if(!strcmp(argv[4],"DM")) {
        gDasfMode = 1;
    }
    else if(!strcmp(argv[4],"FM")) 
    {
        gDasfMode = 0;
    }
    else
    {
        error = OMX_ErrorBadParameter;
        printf("\n%d :: App: Sending Bad Parameter for dasf mode\n",__LINE__);
        printf("%d :: App: Should Be One of these Modes FM or DM\n",__LINE__);
        goto EXIT;
    }

    nIpBuffs = atoi(argv[5]);
    APP_DPRINT("%d :: App: nIpBuf = %d\n",__LINE__, nIpBuffs);
    nIpBufSize = atoi(argv[6]);
    APP_DPRINT("%d :: App: nIpBufSize = %d\n",__LINE__, nIpBufSize);
    nOpBuffs = atoi(argv[7]);
    APP_DPRINT("%d :: App: nOpBuf = %d\n",__LINE__, nOpBuffs);
    nOpBufSize = atoi(argv[8]);
    APP_DPRINT("%d :: App: nOpBufSize = %d\n",__LINE__, nOpBufSize);
    packetsPerBuffer = atoi(argv[9]);
    APP_DPRINT("%d :: App: nPacetsPerBuffer = %d\n",__LINE__, packetsPerBuffer);
    
    if(!strcmp(argv[3],"TC_1")) {
        tcID = 1;
    } else if (!strcmp(argv[3],"TC_2")) {
        tcID = 2;
    } else if (!strcmp(argv[3],"TC_3")) {
        tcID = 3;
    } else if (!strcmp(argv[3],"TC_4")) {
        tcID = 4;
    } else if (!strcmp(argv[3],"TC_5")) {
        tcID = 5;
    } else if (!strcmp(argv[3],"TC_6")) {
        tcID = 6;
    } else if (!strcmp(argv[3],"TC_7")) {
        tcID = 7;
    } else if (!strcmp(argv[3],"TC_8")) {
        tcID = 8;
    } else {
        printf("Invalid Test Case ID: exiting...\n");
        exit(1);
    }

    switch (tcID) {
        case 1:
        case 7:
        case 8:
            printf ("-------------------------------------\n");
            printf ("Testing Simple PLAY till EOF \n");
            printf ("-------------------------------------\n");
            break;
        case 2:
            printf ("-------------------------------------\n");
            printf ("Testing Basic Stop \n");
            printf ("-------------------------------------\n");
            strcat(fname,"_tc2.pcm");
            break;
        case 3:
            printf ("-------------------------------------\n");
            printf ("Testing PAUSE & RESUME Command\n");
            printf ("-------------------------------------\n");
            break;
        case 4:
            printf ("---------------------------------------------\n");
            printf ("Testing STOP Command by Stopping In-Between\n");
            printf ("---------------------------------------------\n");
            break;
        case 5:
            printf ("-------------------------------------------------\n");
            printf ("Testing Repeated PLAY without Deleting Component\n");
            printf ("-------------------------------------------------\n");
            strcat(fname,"_tc5.pcm");
            testcnt = 20;
            break;
        case 6:
            printf ("------------------------------------------------\n");
            printf ("Testing Repeated PLAY with Deleting Component\n");
            printf ("------------------------------------------------\n");
            strcat(fname,"_tc6.pcm");
            testcnt1 = 20;
            break;
    }

/****************************************************
                    OUTER LOOP STARTS
****************************************************/

    for(j = 0; j < testcnt1; j++) {
        /* reset flags and counters*/
        sendlastbuffer = 0;
        playcompleted = 0;
        totalFilled = 0;           
        
#ifdef DSP_RENDERING_ON    
        if((fdwrite=open(FIFO1,O_WRONLY))<0) 
        {
            printf("[G723TEST] - failure to open WRITE pipe\n");
        }
        else
        {
            printf("[G723TEST] - opened WRITE pipe\n");
        }
        if((fdread=open(FIFO2,O_RDONLY))<0) 
        {
            printf("[G723TEST] - failure to open READ pipe\n");
            goto EXIT;
        }
        else 
        {
            printf("[G723TEST] - opened READ pipe\n");
        }
#endif   

        /* Create a pipe used to queue data from the callback. */
        retval = pipe(IpBuf_Pipe);
        if( retval != 0) 
        {
            printf( "Error:Fill Data Pipe failed to open\n");
            goto EXIT;
        }
        retval = pipe(OpBuf_Pipe);
        if( retval != 0) 
        {
            printf( "Error:Empty Data Pipe failed to open\n");
            goto EXIT;
        }
        retval = pipe(g729Event_Pipe);
        if( retval != 0)
        {
            printf( "%d :: G723DecTest.c :: Error:Empty Data Pipe failed to open\n",__LINE__);
            goto EXIT;
        }

         /* save off the "max" of the handles for the selct statement */
        fdmax = maxint(IpBuf_Pipe[0], OpBuf_Pipe[0]);
        fdmax = maxint(fdmax, g729Event_Pipe[0]);

#ifdef OMX_GETTIME
        GT_START();
        error = OMX_GetHandle(&pHandle, strG723Decoder, &AppData, &G723CaBa);
        GT_END("Call to GetHandle");
#else 
        error = TIOMX_GetHandle(&pHandle, strG723Decoder, &AppData, &G723CaBa);
#endif

        if((error != OMX_ErrorNone) || (pHandle == NULL)) 
        {
             APP_DPRINT ("Error in Get Handle function\n");
             goto EXIT;
        }
        gState = OMX_StateLoaded;
        
        if (tcID == 6) 
        {
            printf("*********************************************************\n");
            printf("%d :: App: Processing %d of %d: TC6\n",__LINE__, j+1, testcnt1);
            printf("*********************************************************\n");
        }

        pCompPrivateStruct = malloc (sizeof (OMX_PARAM_PORTDEFINITIONTYPE));
        if(NULL == pCompPrivateStruct) 
        {
            printf("%d :: App: Malloc Failed\n",__LINE__);
            goto EXIT;
        }

        /* set playback stream mute/unmute */
        pCompPrivateStructMute = malloc (sizeof(OMX_AUDIO_CONFIG_MUTETYPE));
        if(pCompPrivateStructMute == NULL) 
        {
            printf("%d :: App: Malloc Failed\n",__LINE__);
            goto EXIT;
        }

        pCompPrivateStructVolume = malloc (sizeof(OMX_AUDIO_CONFIG_VOLUMETYPE));
        if(pCompPrivateStructVolume == NULL) 
        {
            printf("%d :: App: Malloc Failed\n",__LINE__);
            goto EXIT;
        }

        pCompPrivateStruct->nSize = sizeof (OMX_PARAM_PORTDEFINITIONTYPE);

        pAppPrivate = malloc(sizeof(TI_OMX_DSP_DEFINITION));
        if (pAppPrivate == NULL) 
        {
           printf("%d :: App: Error: Malloc Failed\n",__LINE__);
           goto EXIT;
        }
        pAppPrivate->dasfMode = gDasfMode;


        /* Send  G723 config for input */
        pG723Param = malloc (sizeof(OMX_AUDIO_PARAM_G723TYPE));
        if (pG723Param == NULL) 
        {
            printf("%d :: App: Error: Malloc Failed\n",__LINE__);
            goto EXIT;
        }
        pG723Param->nSize                    = sizeof(OMX_AUDIO_PARAM_G723TYPE);
        pG723Param->nVersion.s.nVersionMajor = VERSION_MAJOR;
        pG723Param->nVersion.s.nVersionMinor = VERSION_MINOR;
        pG723Param->nPortIndex               = OMX_DirInput;
        pG723Param->nChannels                = MONO_CHANNEL; /* mono */
        pG723Param->bDTX = OMX_FALSE;
        pG723Param->bHiPassFilter = OMX_FALSE;
        pG723Param->bPostFilter = OMX_FALSE;

        if (atoi(argv[2]) == BIT_63) 
        {
             pG723Param->eBitRate = BIT_63;
             currentFrameSize = SPEECH63_FRAME_SIZE;
        } else if(atoi(argv[2]) == BIT_53) 
        {
            pG723Param->eBitRate = BIT_53;
             currentFrameSize = SPEECH53_FRAME_SIZE;
        }
        else if (atoi(argv[2]) == INVALID_SAMPLING_FREQ) 
        {
            printf("***********************************************************\n");
            printf("%d :: App: Note: Invalid Frequency has been specified by App\n",__LINE__);
            printf("***********************************************************\n");
            pG723Param->eBitRate = INVALID_SAMPLING_FREQ;
        } else 
        {
            printf("Sample Frequency Not Supported: \n");
            printf("Supported frequencies are: 6300 or 5300.\n");
            printf("If you want to specify invalid sampling frequency, ");
            printf("specify %d and test the component for error.\n", INVALID_SAMPLING_FREQ);
            printf("%d :: Exiting...........\n",__LINE__);
            exit(1);
        }
        
#ifdef OMX_GETTIME
        GT_START();
        error = OMX_SetParameter (pHandle, OMX_IndexParamAudioG723, pG723Param);
        GT_END("Set Parameter Test-SetParameter");
#else
        error = OMX_SetParameter (pHandle, OMX_IndexParamAudioG723, pG723Param);
#endif

        if (error != OMX_ErrorNone) 
        {
            error = OMX_ErrorBadParameter;
            goto EXIT;
        }

        /* Send  G723 config for output */
        pPcmParam = malloc (sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
        if (pPcmParam == NULL) 
        {
           printf("%d :: App: Error: Malloc Failed\n",__LINE__);
           goto EXIT;
        }
        pPcmParam->nSize = sizeof(OMX_AUDIO_PARAM_PCMMODETYPE);
        pPcmParam->nVersion.s.nVersionMajor = 1;
        pPcmParam->nVersion.s.nVersionMinor = 1;
        pPcmParam->nPortIndex = OMX_DirOutput;
        pPcmParam->nChannels = MONO_CHANNEL; /* mono */
        pPcmParam->nBitPerSample = BITS_PER_SAMPLE;
        pPcmParam->nSamplingRate = SAMPLE_RATE;
        pPcmParam->eNumData = OMX_NumericalDataUnsigned;
        pPcmParam->eEndian = OMX_EndianLittle;
        pPcmParam->bInterleaved = OMX_FALSE;
        pPcmParam->ePCMMode = OMX_AUDIO_PCMModeLinear;

#ifdef OMX_GETTIME
        GT_START();
        error = OMX_SetParameter (pHandle, OMX_IndexParamAudioPcm, pPcmParam);
        GT_END("Set Parameter Test-SetParameter");
#else
        error = OMX_SetParameter (pHandle, OMX_IndexParamAudioPcm, pPcmParam);
#endif

        if (error != OMX_ErrorNone)
        {
            error = OMX_ErrorBadParameter;
            goto EXIT;
        }
        
        /* Send input port config */
        pCompPrivateStruct->eDir = OMX_DirInput;
        pCompPrivateStruct->nBufferCountActual = nIpBuffs;
        pCompPrivateStruct->nBufferSize = nIpBufSize*packetsPerBuffer;
        pCompPrivateStruct->format.audio.eEncoding = OMX_AUDIO_CodingG723;
        pCompPrivateStruct->bEnabled = OMX_TRUE;
        pCompPrivateStruct->bPopulated = OMX_FALSE;
        pCompPrivateStruct->nPortIndex = OMX_DirInput;
        
#ifdef OMX_GETTIME
        GT_START();
        error = OMX_SetParameter(pHandle,OMX_IndexParamPortDefinition, pCompPrivateStruct);
        GT_END("Set Parameter Test-SetParameter");
#else
        error = OMX_SetParameter(pHandle,OMX_IndexParamPortDefinition, pCompPrivateStruct);
#endif

        if (error != OMX_ErrorNone) 
        {
            error = OMX_ErrorBadParameter;
            printf ("%d :: OMX_ErrorBadParameter\n",__LINE__);
            goto EXIT;
        }

        /* Send output port config */
        pCompPrivateStruct->eDir = OMX_DirOutput;
        pCompPrivateStruct->nBufferCountActual = nOpBuffs;
        pCompPrivateStruct->nBufferSize = nOpBufSize*packetsPerBuffer;
        pCompPrivateStruct->format.audio.eEncoding = OMX_AUDIO_CodingPCM;
        pCompPrivateStruct->bEnabled = OMX_TRUE;
        pCompPrivateStruct->bPopulated = OMX_FALSE;
        pCompPrivateStruct->nPortIndex = OMX_DirOutput;

        if(gDasfMode == 1) 
        {
            pCompPrivateStruct->bPopulated = OMX_TRUE;
        }
        
#ifdef OMX_GETTIME
        GT_START();
        error = OMX_SetParameter(pHandle, OMX_IndexParamPortDefinition, pCompPrivateStruct);
        GT_END("Set Parameter Test-SetParameter");
#else
        error = OMX_SetParameter(pHandle, OMX_IndexParamPortDefinition, pCompPrivateStruct);
#endif

        if (error != OMX_ErrorNone) 
        {
            error = OMX_ErrorBadParameter;
            printf ("%d :: OMX_ErrorBadParameter\n",__LINE__);
            goto EXIT;
        }

        error = OMX_GetExtensionIndex(pHandle, "OMX.TI.index.config.g723headerinfo",&index);
        if (error != OMX_ErrorNone)
        {
            APP_DPRINT("Error getting extension index\n");
            goto EXIT;
        }

#ifdef DSP_RENDERING_ON
        cmd_data.hComponent = pHandle;
        cmd_data.AM_Cmd = AM_CommandIsOutputStreamAvailable;
        
        /* for decoder, using AM_CommandIsInputStreamAvailable */
        cmd_data.param1 = 0;
        if((write(fdwrite, &cmd_data, sizeof(cmd_data)))<0) 
        {
            printf("%d ::G723DecTest.c ::[G723 Dec Component] - send command to audio manager\n", __LINE__);
        }
        if((read(fdread, &cmd_data, sizeof(cmd_data)))<0) 
        {
            printf("%d ::G723DecTest.c ::[G723 Dec Component] - failure to get data from the audio manager\n", __LINE__);
            goto EXIT;
        }
        pAppPrivate->streamId = cmd_data.streamID;
#endif

        OMX_SetConfig(pHandle, index, pAppPrivate);
        if(error != OMX_ErrorNone) 
        {
            error = OMX_ErrorBadParameter;
            APP_DPRINT("%d :: Error from OMX_SetConfig() function\n",__LINE__);
            goto EXIT;
        }
        
#ifdef OMX_GETTIME
        GT_START();
#endif

        error = OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
        if(error != OMX_ErrorNone) {
            APP_DPRINT ("Error from SendCommand-Idle(Init) State function - error = %d\n",error);
            goto EXIT;
        }

#ifndef USE_BUFFER
        for(i=0; i<nIpBuffs; i++) 
        {
            APP_DPRINT("%d :: About to call OMX_AllocateBuffer On Input\n",__LINE__);
            error = OMX_AllocateBuffer(pHandle, &pInputBufferHeader[i], 0, NULL, nIpBufSize*packetsPerBuffer);
            APP_DPRINT("%d :: called OMX_AllocateBuffer\n",__LINE__);
            if(error != OMX_ErrorNone) 
            {
                APP_DPRINT("%d :: Error returned by OMX_AllocateBuffer()\n",__LINE__);
                goto EXIT;
            }
        }

        if (pAppPrivate->dasfMode == 0)
        {
            for(i=0; i<nOpBuffs; i++) 
            {
                error = OMX_AllocateBuffer(pHandle,&pOutputBufferHeader[i],1,NULL,nOpBufSize*packetsPerBuffer);
                if(error != OMX_ErrorNone) 
                {
                    APP_DPRINT("%d :: Error returned by OMX_AllocateBuffer()\n",__LINE__);
                    goto EXIT;
                }
            }
        }
#else

        for(i=0; i<nIpBuffs; i++) 
        {
            pInputBuffer[i] = (OMX_U8*)malloc(nIpBufSize);
            APP_DPRINT("%d :: About to call OMX_UseBuffer On Input\n",__LINE__);
            error = OMX_UseBuffer(pHandle,&pInputBufferHeader[i],0,NULL,nIpBufSize*packetsPerBuffer,pInputBuffer[i]);
            if(error != OMX_ErrorNone) 
            {
                APP_DPRINT("%d :: Error returned by OMX_UseBuffer()\n",__LINE__);
                goto EXIT;
            }
        }
        if (pAppPrivate->dasfMode == 0) 
        {
            for(i=0; i<nOpBuffs; i++) {
                pOutputBuffer[i] = malloc (nOpBufSize + EXTRA_BYTES);
                pOutputBuffer[i] = pOutputBuffer[i] + CACHE_ALIGN;
                /* allocate output buffer */
                APP_DPRINT("%d :: About to call OMX_UseBuffer On Output\n",__LINE__);
                error = OMX_UseBuffer(pHandle,&pOutputBufferHeader[i],1,NULL,nOpBufSize*packetsPerBuffer,pOutputBuffer[i]);
                if(error != OMX_ErrorNone) 
                {
                    APP_DPRINT("%d :: Error returned by OMX_UseBuffer()\n",__LINE__);
                    goto EXIT;
                }
                pOutputBufferHeader[i]->nFilledLen = 0;
            }
        }
#endif

        /* default setting for Mute/Unmute */
        pCompPrivateStructMute->nSize                    = sizeof (OMX_AUDIO_CONFIG_MUTETYPE);
        pCompPrivateStructMute->nVersion.s.nVersionMajor = VERSION_MAJOR;
        pCompPrivateStructMute->nVersion.s.nVersionMinor = VERSION_MINOR;
        pCompPrivateStructMute->nPortIndex               = OMX_DirInput;
        pCompPrivateStructMute->bMute                    = OMX_FALSE;

        /* default setting for volume */
        pCompPrivateStructVolume->nSize                    = sizeof(OMX_AUDIO_CONFIG_VOLUMETYPE);
        pCompPrivateStructVolume->nVersion.s.nVersionMajor = VERSION_MAJOR;
        pCompPrivateStructVolume->nVersion.s.nVersionMinor = VERSION_MINOR;
        pCompPrivateStructVolume->nPortIndex               = OMX_DirInput;
        pCompPrivateStructVolume->bLinear                  = OMX_FALSE;
        pCompPrivateStructVolume->sVolume.nValue           = 30;  /* actual volume */
        pCompPrivateStructVolume->sVolume.nMin             = 0;   /* min volume */
        pCompPrivateStructVolume->sVolume.nMax             = 100; /* max volume */

        if(pAppPrivate->dasfMode == 1) 
        {
            APP_DPRINT("%d :: G723 DECODER RUNNING UNDER DASF MODE\n",__LINE__);
        }
        else if(pAppPrivate->dasfMode == 0) 
        {
            APP_DPRINT("%d :: G723 DECODER RUNNING UNDER FILE MODE\n",__LINE__);
        }
        else
        {
            printf("%d :: IMPROPER SETTING OF DASF/FILE MODE \n",__LINE__);
        }

        if (pAppPrivate->dasfMode) 
        {
            APP_DPRINT("***************StreamId=%d******************\n", (int)pAppPrivate->streamId);

#ifdef RTM_PATH    
            dataPath = DATAPATH_APPLICATION_RTMIXER;
#endif

#ifdef ETEEDN_PATH
            dataPath = DATAPATH_APPLICATION;
#endif        
            error = OMX_GetExtensionIndex(pHandle, "OMX.TI.index.config.g723dec.datapath",&index);
            if (error != OMX_ErrorNone) 
            {
                printf("Error getting extension index\n");
                goto EXIT;
            }
            error = OMX_SetConfig (pHandle, index, &dataPath);
            if(error != OMX_ErrorNone) 
            {
                error = OMX_ErrorBadParameter;
                APP_DPRINT("%d :: G723DecTest.c :: Error from OMX_SetConfig() function\n",__LINE__);
                goto EXIT;
            }
        }

        APP_DPRINT("%d :: App :: About to call WaitForState to change to Idle\n",__LINE__);
        error = WaitForState(pHandle, OMX_StateIdle);

#ifdef OMX_GETTIME
        GT_END("Call to Wait for State  after init <OMX_StateIdle>");
#endif

        APP_DPRINT("%d :: App :: state changed to Idle\n",__LINE__);
        if(error != OMX_ErrorNone) 
        {
            APP_DPRINT( "Error:  hAmrEncoder->WaitForState reports an error %X\n", error);
            goto EXIT;
        }
   
        for(i = 0; i < testcnt; i++) 
        {
             /* reset flags */
            gState = OMX_StateIdle;
            sendlastbuffer = 0;
            playcompleted = 0;  

            /* open the specified input file */
            fIn = fopen(argv[1], "r");
            if(fIn == NULL)
            {
                fprintf(stderr, "Error:  failed to open the file %s for readonly access\n", argv[1]);
                goto EXIT;
            }

            if(gDasfMode == 0) 
            {
                /* open the specified output file if not in render mode */
                fOut = fopen(fname, "w");
                if(fOut == NULL) 
                {
                    fprintf(stderr, "Error:  failed to create the output file \n");
                    goto EXIT;
                }
               APP_DPRINT("%d :: Op File has created\n",__LINE__);
            }
            if (tcID == 5) 
            {
                printf("*********************************************************\n");
                printf ("App: Processing %d of %d: TC 5\n",i+1,testcnt);
                printf("*********************************************************\n");
            }

#ifdef OMX_GETTIME
            GT_START();
#endif

            error = OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
            if(error != OMX_ErrorNone) 
            {
                APP_DPRINT ("Error from SendCommand-Executing State function\n");
                goto EXIT;
            }
            APP_DPRINT("%d :: App :: About to call WaitForState to change to Executing\n",__LINE__);
            error = WaitForState(pHandle, OMX_StateExecuting);
            gState = OMX_StateExecuting;
            
#ifdef OMX_GETTIME
            GT_END("Call to SendCommand <OMX_StateExecuting>");
#endif

            APP_DPRINT("%d :: App :: state changed to Executing\n",__LINE__);
            if(error != OMX_ErrorNone) 
            {
                APP_DPRINT( "Error:  WaitForState reports an error %X\n", error);
                goto EXIT;
            }

            for (k=0; k < nIpBuffs; k++) 
            {
            
#ifdef OMX_GETTIME
                if (k==0)
                { 
                    GT_FlagE=1;  /* 1 = First Buffer,  0 = Not First Buffer  */
                    GT_START(); /* Empty Bufffer */
                }
#endif

                error = send_input_buffer (pHandle, pInputBufferHeader[k], fIn);
                if (error != OMX_ErrorNone) 
                {
                    goto EXIT;
                }

            }

            if (gDasfMode == 0) 
            {
                for (k=0; k < nOpBuffs; k++) {
#ifdef OMX_GETTIME
                    if (k==0)
                    { 
                        GT_FlagF=1;  /* 1 = First Buffer,  0 = Not First Buffer  */
                        GT_START(); /* Fill Buffer */
                    }
#endif
                    OMX_FillThisBuffer(pHandle, pOutputBufferHeader[k]);
                }
            }

            while((error == OMX_ErrorNone) && (gState != OMX_StateIdle))
            {
                FD_ZERO(&rfds);
                FD_SET(IpBuf_Pipe[0], &rfds);
                FD_SET(OpBuf_Pipe[0], &rfds);
                FD_SET(g729Event_Pipe[0], &rfds);
                tv.tv_sec = 1;
                tv.tv_usec = 0;
                frmCount++;

                retval = select(fdmax+1, &rfds, NULL, NULL, &tv);
                if(retval == -1) 
                {
                    perror("select()");
                    APP_DPRINT ( " : Error \n");
                    break;
                }

                if(retval == 0)
                {
                    printf ("%d :: BasicFn App Timeout !!!!!!!!!!! \n",__LINE__);
                }

                switch (tcID) {
                    case 1:
                    case 5:
                    case 6:
                        if(FD_ISSET(IpBuf_Pipe[0], &rfds)) 
                        {
                            read(IpBuf_Pipe[0], &pBuffer, sizeof(pBuffer));
                            error = send_input_buffer (pHandle, pBuffer, fIn);

                            if (error != OMX_ErrorNone) 
                            {
                                goto EXIT;
                            }
                            frmCnt++;
                        }
                        break;
                    case 2:
                    case 4:
                        if(FD_ISSET(IpBuf_Pipe[0], &rfds)) 
                        {
                            read(IpBuf_Pipe[0], &pBuffer, sizeof(pBuffer));
                            if(frmCnt == 25) 
                            {
                                printf("Shutting down ---------- \n");
#ifdef OMX_GETTIME
                                GT_START();
#endif
                                error = OMX_SendCommand(pHandle,OMX_CommandStateSet, OMX_StateIdle, NULL);
                                if(error != OMX_ErrorNone) 
                                {
                                    fprintf (stderr,"Error from SendCommand-Idle(Stop) State function\n");
                                    goto EXIT;
                                }
                                
                            }
                            else 
                            {
                                error = send_input_buffer (pHandle, pBuffer, fIn);
                                if (error != OMX_ErrorNone) 
                                {
                                    printf ("Error While reading input pipe\n");
                                    goto EXIT;
                                }
                            }
                            if(frmCnt == 25 && tcID == 4) 
                            {
                                APP_STATEPRINT ("*********** Waiting for state to change to Idle ************\n\n");
                                error = WaitForState(pHandle, OMX_StateIdle);
#ifdef OMX_GETTIME
                                GT_END("Call to SendCommand <OMX_StateIdle>");
#endif
                                if(error != OMX_ErrorNone) 
                                {
                                    fprintf(stderr, "Error:  WaitForState reports an error %X\n", error);
                                    goto EXIT;
                                }
                                APP_STATEPRINT("*********** State Changed to Idle ************\n\n");
                                gState = OMX_StateIdle;
                                printf("wait %d seconds before playing further\n",SLEEP_TIME);
                                sleep(SLEEP_TIME);
                                if (gDasfMode == 0) 
                                {
                                    /*rewind input and output files*/
                                    fseek(fIn, 0L, SEEK_SET);
                                    fseek(fOut, 0L, SEEK_SET);
                                }
                                else
                                {
                                    fseek(fIn, 0L, SEEK_SET);
                                }
                                APP_STATEPRINT("*************** Execute command to Component *******************\n");
                                
#ifdef OMX_GETTIME
                                GT_START();
#endif
                                error = OMX_SendCommand(pHandle, OMX_CommandStateSet,OMX_StateExecuting, NULL);
                                if(error != OMX_ErrorNone) 
                                {
                                    fprintf (stderr,"Error from SendCommand-Executing State function\n");
                                    goto EXIT;
                                }
                                error = WaitForState(pHandle, OMX_StateExecuting);
                                
#ifdef OMX_GETTIME
                                GT_END("Call to SendCommand <OMX_StateExecuting>");
#endif

                                if(error != OMX_ErrorNone) 
                                {
                                    fprintf(stderr, "Error:  WaitForState reports an error %X\n", error);
                                    goto EXIT;
                                }
                                gState = OMX_StateExecuting;
                                APP_STATEPRINT("*********** State Changed to Executing ************\n\n");
                                for (k=0; k < nIpBuffs; k++) 
                                {
                                    error = send_input_buffer (pHandle, pInputBufferHeader[k], fIn);
                                }
                            }
                            frmCnt++;
                        }
                        break;
                    case 3:
                        if(frmCount == 30 || frmCount == 60) 
                        {
                            APP_STATEPRINT("\n\n*************** Pause command to Component *******************\n");
#ifdef OMX_GETTIME
                            GT_START();
#endif
                            error = OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StatePause, NULL);
                            if(error != OMX_ErrorNone) 
                            {
                                fprintf (stderr,"Error from SendCommand-Pasue State function\n");
                                goto EXIT;
                            }
                            APP_STATEPRINT("*********** Waiting for state to change to Pause ************\n");
                            error = WaitForState(pHandle, OMX_StatePause);
                            
#ifdef OMX_GETTIME
                            GT_END("Call to SendCommand <OMX_StatePause>");
#endif

                            if(error != OMX_ErrorNone) 
                            {
                                fprintf(stderr, "Error:  WaitForState reports an error %X\n", error);
                                goto EXIT;
                            }
                            gState = OMX_StatePause;
                            APP_STATEPRINT("*********** State Changed to Pause ************\n\n");
                            printf("Sleeping for %d secs....\n\n",SLEEP_TIME);
                            sleep(SLEEP_TIME);
                            APP_STATEPRINT("*************** Resume command to Component *******************\n");
                            
#ifdef OMX_GETTIME
                            GT_START();
#endif
                            error = OMX_SendCommand(pHandle, OMX_CommandStateSet,OMX_StateExecuting, NULL);
                            if(error != OMX_ErrorNone) 
                            {
                                fprintf (stderr,"Error from SendCommand-Executing State function\n");
                                goto EXIT;
                            }
                            APP_STATEPRINT("******** Waiting for state to change to Resume ************\n");

#ifdef OMX_GETTIME
                            GT_END("Call to SendCommand <OMX_StateExecuting>");
#endif

                            error = WaitForState(pHandle, OMX_StateExecuting);
                            if(error != OMX_ErrorNone) {
                                fprintf(stderr, "Error:  WaitForState reports an error %X\n", error);
                                goto EXIT;
                            }
                            gState = OMX_StateExecuting;
                            APP_STATEPRINT("*********** State Changed to Resume ************\n\n");
                        }

                        if(FD_ISSET(IpBuf_Pipe[0], &rfds)) 
                        {
                            OMX_BUFFERHEADERTYPE* pBuffer = NULL;
                            read(IpBuf_Pipe[0], &pBuffer, sizeof(pBuffer));
                            error = send_input_buffer (pHandle, pBuffer, fIn);
                            if (error != OMX_ErrorNone) 
                            {
                                printf ("Error While reading input pipe\n");
                                goto EXIT;
                            }
                            frmCnt++;
                        }
                        break;
                    case 7:
                        /* test mute/unmute playback stream */
                        if(FD_ISSET(IpBuf_Pipe[0], &rfds)) 
                        {
                            OMX_BUFFERHEADERTYPE* pBuffer = NULL;
                            read(IpBuf_Pipe[0], &pBuffer, sizeof(pBuffer));
                            error = send_input_buffer (pHandle, pBuffer, fIn);
                            if (error != OMX_ErrorNone) 
                            {
                                goto EXIT;
                            }
                            frmCnt++;
                        }
                        if(frmCnt == 30) 
                        {
                            printf("************Mute the playback stream*****************\n");
                            pCompPrivateStructMute->bMute = OMX_TRUE;
                            error = OMX_SetConfig(pHandle, OMX_IndexConfigAudioMute, pCompPrivateStructMute);
                            if (error != OMX_ErrorNone) 
                            {
                                error = OMX_ErrorBadParameter;
                                goto EXIT;
                            }
                        }
                        if(frmCnt == 60) 
                        {
                            printf("************Unmute the playback stream*****************\n");
                            pCompPrivateStructMute->bMute = OMX_FALSE;
                            error = OMX_SetConfig(pHandle, OMX_IndexConfigAudioMute, pCompPrivateStructMute);
                            if (error != OMX_ErrorNone) 
                            {
                                error = OMX_ErrorBadParameter;
                                goto EXIT;
                            }
                        }
                        break;
                    case 8:
                        /* test set volume for playback stream */
                        if(FD_ISSET(IpBuf_Pipe[0], &rfds)) 
                        {
                            OMX_BUFFERHEADERTYPE* pBuffer = NULL;
                            read(IpBuf_Pipe[0], &pBuffer, sizeof(pBuffer));
                            error = send_input_buffer (pHandle, pBuffer, fIn);
                            if (error != OMX_ErrorNone) 
                            {
                                goto EXIT;
                            }
                            frmCnt++;
                        }
                        if(frmCnt == 30) 
                        {
                            printf("************Set stream volume to high*****************\n");
                            pCompPrivateStructVolume->sVolume.nValue = 0x8000;
                            error = OMX_SetConfig(pHandle, OMX_IndexConfigAudioVolume, pCompPrivateStructVolume);
                            if (error != OMX_ErrorNone) 
                            {
                                error = OMX_ErrorBadParameter;
                                goto EXIT;
                            }
                        }
                        if(frmCnt == 60) 
                        {
                            printf("************Set stream volume to low*****************\n");
                            pCompPrivateStructVolume->sVolume.nValue = 0x1000;
                            error = OMX_SetConfig(pHandle, OMX_IndexConfigAudioVolume, pCompPrivateStructVolume);
                            if (error != OMX_ErrorNone) 
                            {
                                error = OMX_ErrorBadParameter;
                                goto EXIT;
                            }
                        }
                        break;
                    default:
                        APP_DPRINT("%d :: ### Running Simple DEFAULT Case Here ###\n",__LINE__);
                }

                if( FD_ISSET(OpBuf_Pipe[0], &rfds) ) 
                {
                    OMX_BUFFERHEADERTYPE* pBuf = NULL;
                    read(OpBuf_Pipe[0], &pBuf, sizeof(pBuf));
                    if(pBuf->nFilledLen == 0)
                    {
                        APP_DPRINT("APP: output buffer received, filled length = %d, \
totalfilled = %d\n", (int)pBuf->nFilledLen,totalFilled);
                    }
                    else 
                    {
                        APP_DPRINT("%d :: APP: output buffer received, filled length = %d,\
totalfilled = %d\n",__LINE__,(int)pBuf->nFilledLen,totalFilled);
                        fwrite(pBuf->pBuffer, 1, pBuf->nFilledLen, fOut);
                    }
                    totalFilled += pBuf->nFilledLen;
                    fflush(fOut);
                    OMX_FillThisBuffer(pHandle, pBuf);
                }
                if(playcompleted)
                {
                
#ifdef OMX_GETTIME
                    GT_START();
#endif
                    error = OMX_SendCommand(pHandle,OMX_CommandStateSet, OMX_StateIdle, NULL);
                    if(error != OMX_ErrorNone) 
                    {
                        fprintf (stderr,"Error from SendCommand-Idle(Stop) State function\n");
                        goto EXIT;
                    }
                    error = WaitForState(pHandle, OMX_StateIdle);
                    
#ifdef OMX_GETTIME
                    GT_END("Call to SendCommand <OMX_StateIdle>");
#endif
                    if(error != OMX_ErrorNone) 
                    {
                        fprintf(stderr, "Error:  G723Decoder->WaitForState reports an error %X\n", error);
                        goto EXIT;
                    }
                    gState = OMX_StateIdle;
                    playcompleted = 0; 
                }
            } /* While Loop Ending Here */
            
            if(0 == gDasfMode) 
            {
                fclose(fOut);
            }
            fclose(fIn);
            if(i != (testcnt-1)) 
            {
                if((tcID == 5) || (tcID == 2)) 
                {
                    printf("%d :: sleeping here for %d secs\n",__LINE__,SLEEP_TIME);
                    sleep (SLEEP_TIME);
                }
            }
        }/* inner process loop end */

#ifdef DSP_RENDERING_ON
        cmd_data.hComponent = pHandle;
        cmd_data.AM_Cmd = AM_Exit;
        if((write(fdwrite, &cmd_data, sizeof(cmd_data)))<0)
        {
            printf("%d :: [G723 Dec Component] - send command to audio manager\n",__LINE__);
        }
        close(fdwrite);
        close(fdread);
#endif        

        for(i=0; i<nIpBuffs; i++) 
        {
            APP_DPRINT("%d :: App: Freeing %p IP BufHeader\n",__LINE__,pInputBufferHeader[i]);
            error = OMX_FreeBuffer(pHandle,OMX_DirInput,pInputBufferHeader[i]);
            if((error != OMX_ErrorNone)) 
            {
                APP_DPRINT ("%d:: Error in Free Buffer function\n",__LINE__);
                goto EXIT;
            }
        }

        if(pAppPrivate->dasfMode == 0) 
        {
            for(i=0; i<nOpBuffs; i++) 
            {
                APP_DPRINT("%d :: App: Freeing %p OP BufHeader\n",__LINE__,pOutputBufferHeader[i]);
                error = OMX_FreeBuffer(pHandle,OMX_DirOutput,pOutputBufferHeader[i]);
                if((error != OMX_ErrorNone)) 
                {
                    APP_DPRINT ("%d:: Error in Free Buffer function\n",__LINE__);
                    goto EXIT;
                }
            }
        }
        APP_STATEPRINT ("Sending the StateLoaded Command\n");
        
#ifdef OMX_GETTIME
        GT_START();
#endif

        error = OMX_SendCommand(pHandle,OMX_CommandStateSet, OMX_StateLoaded, NULL);
        error = WaitForState(pHandle, OMX_StateLoaded);
        
#ifdef OMX_GETTIME
        GT_END("Call to SendCommand <OMX_StateLoaded>");
#endif

        if(error != OMX_ErrorNone) 
        {
            printf ("%d:: Error from SendCommand-Idle State function\n",__LINE__);
            goto EXIT;
        }
        gState = OMX_StateLoaded;
        APP_STATEPRINT ("state changed to Loaded\n");
        APP_DPRINT("Free the component handle\n");
        error = TIOMX_FreeHandle(pHandle);
        if( (error != OMX_ErrorNone)) 
        {
            printf ("%d:: Error in Free Handle function\n",__LINE__);
            goto EXIT;
        }
        APP_DPRINT ("%d:: Free Handle returned Successfully \n\n\n\n",__LINE__);
        if(pCompPrivateStruct != NULL)
        {
            free(pCompPrivateStruct);
            pCompPrivateStruct = NULL;
        }
        if(pG723Param != NULL)
        {
            free(pG723Param);
            pG723Param = NULL;
        }
        if(pPcmParam != NULL)
        {
            free(pPcmParam);
            pPcmParam = NULL;
        }
        if(pCompPrivateStructMute != NULL)
        {
            free(pCompPrivateStructMute);
            pCompPrivateStructMute = NULL;
        }
        if(pCompPrivateStructVolume != NULL)
        {
            free(pCompPrivateStructVolume);
            pCompPrivateStructVolume = NULL;
        }
        if(pAppPrivate != NULL)
        {
            free(pAppPrivate);
            pAppPrivate = NULL;
        }
        close(IpBuf_Pipe[0]);
        close(IpBuf_Pipe[1]);
        close(OpBuf_Pipe[0]);
        close(OpBuf_Pipe[1]);
        close(g729Event_Pipe[0]);
        close(g729Event_Pipe[1]);

       if (tcID == 6){
            printf("sleeping 2 seconds\n");
            sleep(2);
        }
        
    }/* outer process loop end */

    error = TIOMX_Deinit();
    if(error != OMX_ErrorNone) {
        APP_DPRINT("%d :: Error returned by TIOMX_Deinit()\n",__LINE__);
        goto EXIT;
    }

    if (gDasfMode == 0) 
    {
        printf("**********************************************************\n");
        printf("APP: done decoding, totalfilled = %d\n", totalFilled);
        printf("NOTE: An output file %s has been created in file system\n",APP_OUTPUT_FILE);
        printf("**********************************************************\n");
    }
    
EXIT:

#ifdef OMX_GETTIME
    GT_END("G723_DEC test <End>");
    OMX_ListDestroy(pListHead); 
#endif

#ifdef MTRACE
    muntrace();
#endif

    return error;
}


/* ================================================================================= * */
/**
* @fn send_input_buffer() Sends the input buffer to the component.
*
* @param pHandle This is component handle allocated by the OMX core.
*
* @param pBuffer This is the buffer pointer.
*
* @fIn This is input file handle.
*
* @pre          None
*
* @post         None
*
*  @return      Appropriate OMX error.
*
*  @see         None
*/
/* ================================================================================ * */
OMX_ERRORTYPE send_input_buffer(OMX_HANDLETYPE pHandle, OMX_BUFFERHEADERTYPE* pBuffer, FILE *fIn)
{
    OMX_ERRORTYPE error = OMX_ErrorNone;
    int nRead =  0;

    nRead = fill_data (pBuffer, fIn);
    if(sendlastbuffer)
    {
        sendlastbuffer = 0;
        pBuffer->nFlags = 0;
        playcompleted = 1; /* this will cause the transition to idle */
        goto EXIT;
    }
    if(nRead == 0) /* this will be the dummy buffer that has no data in it */
    {
       pBuffer->nFlags = OMX_BUFFERFLAG_EOS;       
       sendlastbuffer = 1; /* this will not allow another buffer to be sent */
    } 
  
    OMX_EmptyThisBuffer(pHandle, pBuffer);
    
    EXIT:
    return error;
}

/* ================================================================================= * */
/**
* @fn fill_data() Reads the data from the input file.
*
* @param pBuffer This is the buffer pointer.
*
* @fIn This is input file handle.
*
* @pre          None
*
* @post         None
*
*  @return      Number of bytes that have been read.
*
*  @see         None
*/
/* ================================================================================ * */
int fill_data (OMX_BUFFERHEADERTYPE *pBuf,FILE *fIn)
{
    OMX_S16 nRead = 0;
    OMX_U8 serial[currentFrameSize * packetsPerBuffer];
    OMX_S16 j = 0;
    OMX_S16 frame_type = 0;
    OMX_U32 nBytes = 0; /* Number of data bytes in packet */
    OMX_U8 offset = 0;    /* Offset in bytes in input buffer */

    pBuf->nFilledLen = nIpBufSize * packetsPerBuffer;
    pBuf->nAllocLen = nIpBufSize * packetsPerBuffer;
    pBuf->pInputPortPrivate = (unsigned long*)packetsPerBuffer;
    pBuf->nFlags = 0;
    memset(pBuf->pBuffer, 0x0, pBuf->nAllocLen);  /* set the whole buffer to zeros */
    memset(&serial[0], 0x0, currentFrameSize * packetsPerBuffer);
    for(j = 0; j < packetsPerBuffer; j++)
    {
        nRead = fread(serial, 1, 1 , fIn);
         if(nRead != 0)
         {
            /* determine number of bytes in packet from frame type*/
            frame_type = serial[0] & 0x0003;
            /* set frame type */
            switch(frame_type)
            {
                case SPEECH63_FRAME_TYPE:
                    nBytes = SPEECH63_FRAME_SIZE;
                    break;
                case SPEECH53_FRAME_TYPE:
                    nBytes = SPEECH53_FRAME_SIZE;
                    break;
                case SID_FRAME_TYPE:
                    nBytes = SID_FRAME_SIZE;
                    break;
                 default:
                    nBytes = NODATA_FRAME_SIZE;
             }
                        
            /* rewind input file 1 byte so when the frame is read, the byte is not lost */
            fseek(fIn, -1, SEEK_CUR);
            /* read the frame according to the frame type determined above */
            nRead = fread(serial, 1, nBytes , fIn);
            /* copy data into pBuffer for this frame*/
            memcpy(&pBuf->pBuffer[0]+offset, &serial[0], nBytes);
            /* move the pointer for the buffer offset */
            offset = offset + FRAME_SIZE;
        }
        else
        {
            j = packetsPerBuffer; 
        }
    }
    return nRead;
}



