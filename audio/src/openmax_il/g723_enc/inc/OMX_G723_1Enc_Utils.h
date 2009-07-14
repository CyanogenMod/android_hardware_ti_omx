
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
/* =============================================================================
*             Texas Instruments OMAP(TM) Platform Software
*  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
*
*  Use of this software is controlled by the terms and conditions found
*  in the license agreement under which this software has been supplied.
* ============================================================================ */
/**
* @file OMX_G723_1Enc_Utils.h
*
* This is an header file for an G723_1 Encoder that is fully
* compliant with the OMX Audio specification 1.5.
* This the file that the application that uses OMX would include in its code.
*
* @path $(CSLPATH)\OMAPSW_MPU\linux\audio\src\openmax_il\g723_enc\inc
*
* @rev 1.0
*/
/* --------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------
*!
*! Revision History
*! ===================================
*! Gyancarlo Garcia: Initial Verision
*! 05-Oct-2007
*!
* =========================================================================== */
#ifndef OMX_G723_1ENC_UTILS__H
#define OMX_G723_1ENC_UTILS__H

#include <pthread.h>
#include "LCML_DspCodec.h"
#include <OMX_Component.h>
#include <TIDspOmx.h>
#ifdef __PERF_INSTRUMENTATION__
#include "perf.h"
#endif

#ifdef DSP_RENDERING_ON
    #include <AudioManagerAPI.h>
#endif

#ifndef UNDER_CE
#ifdef RESOURCE_MANAGER_ENABLED
     #include <ResourceManagerProxyAPI.h> 
#endif
#endif

#ifdef UNDER_CE
    #define sleep Sleep
#endif


/* ======================================================================= */
/**
 * @def    G723_1ENC_DEBUG   Turns debug messaging on and off
 */
/* ======================================================================= */
#undef G723_1ENC_DEBUG
/*#define G723_1ENC_DEBUG*/
/* ======================================================================= */
/**
 * @def    G723_1ENC_MEMCHECK   Turns memory messaging on and off
 */
/* ======================================================================= */
#undef G723_1ENC_MEMCHECK

#ifndef UNDER_CE
/* ======================================================================= */
/**
 * @def    G723_1ENC_DEBUG   Debug print macro
 */
/* ======================================================================= */
#ifdef  G723_1ENC_DEBUG
        #define G723_1ENC_DPRINT(...)    fprintf(stderr,__VA_ARGS__)
#else
        #define G723_1ENC_DPRINT(...)
#endif
/* ======================================================================= */
/**
 * @def    G723_1ENC_MEMCHECK   Memory print macro
 */
/* ======================================================================= */
#ifdef  G723_1ENC_MEMCHECK
        #define G723_1ENC_MEMPRINT(...)    fprintf(stderr,__VA_ARGS__)
#else
        #define G723_1ENC_MEMPRINT(...)
#endif

#else   /*UNDER_CE*/
/* ======================================================================= */
/**
 * @def    G723_1ENC_DEBUG   Debug print macro
 */
/* ======================================================================= */
#ifdef  G723_1ENC_DEBUG
        #define G723_1ENC_DPRINT(STR, ARG...) printf()
#else

#endif

/* ======================================================================= */
/**
 * @def    G723_1ENC_MEMCHECK   Memory print macro
 */
/* ======================================================================= */
#ifdef  G723_1ENC_MEMCHECK
        #define G723_1ENC_MEMPRINT(STR, ARG...) printf()
#else

#endif

#endif

#ifdef DEBUG
        #define G723_1ENC_DPRINT(...)    fprintf(stderr,__VA_ARGS__)
        #define G723_1ENC_MEMPRINT(...)    fprintf(stderr,__VA_ARGS__)
        #define G723_1ENC_EPRINT(...)    fprintf(stderr,__VA_ARGS__)
#else
        #define G723_1ENC_DPRINT(...)
        #define G723_1ENC_MEMPRINT(...)
        #define G723_1ENC_EPRINT(...)
#endif


/* ======================================================================= */
/**
 * @def    G723_1ENC_DEBUGMEM   Turns memory leaks messaging on and off.
 *         APP_DEBUGMEM must be defined in Test App in order to get
 *         this functionality On.
 */
/* ======================================================================= */
#undef G723_1ENC_DEBUGMEM
/*#define G723_1ENC_DEBUGMEM*/

/* ======================================================================= */
/**
 *  M A C R O S FOR MALLOC and MEMORY FREE and CLOSING PIPES
 */
/* ======================================================================= */

#define OMX_NBCONF_INIT_STRUCT(_s_, _name_) \
    memset((_s_), 0x0, sizeof(_name_)); \
    (_s_)->nSize = sizeof(_name_);      \
    (_s_)->nVersion.s.nVersionMajor = 0x1;  \
    (_s_)->nVersion.s.nVersionMinor = 0x0;  \
    (_s_)->nVersion.s.nRevision = 0x0;      \
    (_s_)->nVersion.s.nStep = 0x0

#define OMX_NBMEMFREE_STRUCT(_pStruct_)\
    G723_1ENC_MEMPRINT("%d :: [FREE] %p\n",__LINE__,_pStruct_);\
    if(_pStruct_ != NULL){\
        SafeFree(_pStruct_);\
        _pStruct_ = NULL;\
    }

#define OMX_NBCLOSE_PIPE(_pStruct_,err)\
    G723_1ENC_DPRINT("%d :: CLOSING PIPE \n",__LINE__);\
    err = close (_pStruct_);\
    if(0 != err && OMX_ErrorNone == eError){\
        eError = OMX_ErrorHardware;\
        printf("%d :: Error while closing pipe\n",__LINE__);\
        goto EXIT;\
    }

#define OMX_NBMALLOC_STRUCT(_pStruct_, _sName_)   \
    _pStruct_ = (_sName_*)SafeMalloc(sizeof(_sName_));      \
    if(_pStruct_ == NULL){      \
        printf("***********************************\n"); \
        printf("%d :: Malloc Failed\n",__LINE__); \
        printf("***********************************\n"); \
        eError = OMX_ErrorInsufficientResources; \
        goto EXIT;      \
    } \
    G723_1ENC_MEMPRINT("%d :: [ALLOC] %p\n",__LINE__,_pStruct_);


/* ======================================================================= */
/**
 * @def G723_1ENC_NUM_INPUT_BUFFERS   Default number of input buffers
 */
/* ======================================================================= */
#define G723_1ENC_NUM_INPUT_BUFFERS 1
/* ======================================================================= */
/**
 * @def G723_1ENC_NUM_INPUT_BUFFERS_DASF  Default No.of input buffers DASF
 */
/* ======================================================================= */
#define G723_1ENC_NUM_INPUT_BUFFERS_DASF 2
/* ======================================================================= */
/**
 * @def G723_1ENC_NUM_OUTPUT_BUFFERS   Default number of output buffers
 */
/* ======================================================================= */
#define G723_1ENC_NUM_OUTPUT_BUFFERS 1
/* ======================================================================= */
/**
 * @def G723_1ENC_INPUT_BUFFER_SIZE_DASF  Default input buffer size DASF
 */
/* ======================================================================= */
#define G723_1ENC_INPUT_BUFFER_SIZE_DASF 480
/* ======================================================================= */
/**
 * @def G723_1ENC_INPUT_BUFFER_SIZE  Default input buffer size DASF
 */
/* ======================================================================= */
#define G723_1ENC_INPUT_BUFFER_SIZE 480
/* ======================================================================= */
/**
 * @def G723_1ENC_OUTPUT_BUFFER_SIZE   Default output buffer size
 */
/* ======================================================================= */
#define G723_1ENC_OUTPUT_BUFFER_SIZE 24

/* ======================================================================= */
/**
 * @def G723_1ENC_APP_ID  App ID Value setting
 */
/* ======================================================================= */
#define G723_1ENC_APP_ID 100

/* ======================================================================= */
/**
 * @def    G723_1ENC_SAMPLING_FREQUENCY   Sampling frequency
 */
/* ======================================================================= */
#define G723_1ENC_SAMPLING_FREQUENCY 8000
/* ======================================================================= */
/**
 * @def    G723_1ENC_MAX_NUM_OF_BUFS   Maximum number of buffers
 */
/* ======================================================================= */
#define G723_1ENC_MAX_NUM_OF_BUFS 10
/* ======================================================================= */
/**
 * @def    G723_1ENC_NUM_OF_PORTS   Number of ports
 */
/* ======================================================================= */
#define G723_1ENC_NUM_OF_PORTS 2
/* ======================================================================= */
/**
 * @def    G723_1ENC_XXX_VER    Component version
 */
/* ======================================================================= */
#define G723_1ENC_MAJOR_VER 0xF1
#define G723_1ENC_MINOR_VER 0xF2
/* ======================================================================= */
/**
 * @def    G723_1ENC_NOT_USED    Defines a value for "don't care" parameters
 */
/* ======================================================================= */
#define G723_1ENC_NOT_USED 10
/* ======================================================================= */
/**
 * @def    G723_1ENC_NORMAL_BUFFER  Defines flag value with all flags off
 */
/* ======================================================================= */
#define G723_1ENC_NORMAL_BUFFER 0
/* ======================================================================= */
/**
 * @def    OMX_G723_1ENC_DEFAULT_SEGMENT    Default segment ID for the LCML
 */
/* ======================================================================= */
#define G723_1ENC_DEFAULT_SEGMENT (0)
/* ======================================================================= */
/**
 * @def    OMX_G723_1ENC_SN_TIMEOUT    Timeout value for the socket node
 */
/* ======================================================================= */
#define G723_1ENC_SN_TIMEOUT (-1)
/* ======================================================================= */
/**
 * @def    OMX_G723_1ENC_SN_PRIORITY   Priority for the socket node
 */
/* ======================================================================= */
#define G723_1ENC_SN_PRIORITY (10)
/* ======================================================================= */
/**
 * @def    G723_1ENC_CPU   TBD, 50MHz for the moment
 */
/* ======================================================================= */
#define G723_1ENC_CPU (50)
/* ======================================================================= */
/**
 * @def    OMX_G723_1ENC_NUM_DLLS   number of DLL's
 */
/* ======================================================================= */
#define G723_1ENC_NUM_DLLS (2)
/* ======================================================================= */
/**
 * @def    G723_1ENC_USN_DLL_NAME   USN DLL name
 */
/* ======================================================================= */
#ifdef UNDER_CE
    #define G723_1ENC_USN_DLL_NAME "\\windows\\usn.dll64P"
#else
    #define G723_1ENC_USN_DLL_NAME "/usn.dll64P"
#endif

/* ======================================================================= */
/**
 * @def    G723_1ENC_DLL_NAME   G723_1 Encoder socket node dll name
 */
/* ======================================================================= */
#ifdef UNDER_CE
    #define G723_1ENC_DLL_NAME "\\windows\\g723_1enc_sn.dll64P"
#else
    #define G723_1ENC_DLL_NAME "/g723enc_sn.dll64P"
#endif

/* ======================================================================= */
/** G723_1ENC_StreamType  Stream types
*
*  @param  G723_1ENC_DMM                    DMM
*
*  @param  G723_1ENC_INSTRM             Input stream
*
*  @param  G723_1ENC_OUTSTRM                Output stream
*/
/* ======================================================================= */
enum G723_1ENC_StreamType {
    G723_1ENC_DMM = 0,
    G723_1ENC_INSTRM,
    G723_1ENC_OUTSTRM
};

/* ======================================================================= */
/**
 * @def G723_1ENC_TIMEOUT Default timeout used to come out of blocking calls
 */
/* ======================================================================= */
#define G723_1ENC_TIMEOUT 1000
/* ======================================================================= */
/*
 * @def G723_1ENC_OMX_MAX_TIMEOUTS   Max Time Outs
 * @def G723_1ENC_DONT_CARE             Dont Care Condition
 * @def G723_1ENC_NUM_CHANNELS      Number of Channels
 * @def G723_1ENC_APP_ID            App ID Value setting
 */
/* ======================================================================= */
#define G723_1ENC_OMX_MAX_TIMEOUTS 20
#define G723_1ENC_DONT_CARE 0
#define G723_1ENC_NUM_CHANNELS 1
/* ======================================================================= */
/**
 * @def    G723_1ENC_STREAM_COUNT    Number of streams
 *         G723_1ENC_INPUT_STREAM_ID Stream ID for Input Buffer
 */
/* ======================================================================= */
#define G723_1ENC_STREAM_COUNT 2
#define G723_1ENC_INPUT_STREAM_ID 0

/* ======================================================================= */
/** G723_1ENC_COMP_PORT_TYPE  Port types
 *
 *  @param  G723_1ENC_INPUT_PORT                Input port
 *
 *  @param  G723_1ENC_OUTPUT_PORT           Output port
 */
/*  ====================================================================== */
/*This enum must not be changed. */
typedef enum G723_1ENC_COMP_PORT_TYPE {
    G723_1ENC_INPUT_PORT = 0,
    G723_1ENC_OUTPUT_PORT
}G723_1ENC_COMP_PORT_TYPE;

/* ======================================================================= */
/** G723_1ENC_BUFFER_Dir  Buffer Direction
*
*  @param  G723_1ENC_DIRECTION_INPUT        Input direction
*
*  @param  G723_1ENC_DIRECTION_OUTPUT   Output direction
*
*/
/* ======================================================================= */
typedef enum {
    G723_1ENC_DIRECTION_INPUT,
    G723_1ENC_DIRECTION_OUTPUT
}G723_1ENC_BUFFER_Dir;



/* ======================================================================= */
/** G723_1ENC_BUFFS  Buffer details
*
*  @param  BufHeader Buffer header
*
*  @param  Buffer   Buffer
*
*/
/* ======================================================================= */
typedef struct G723_1ENC_BUFFS {
    char BufHeader;
    char Buffer;
}G723_1ENC_BUFFS;

/* ======================================================================= */
/** G723_1ENC_BUFFERHEADERTYPE_INFO
*
*  @param  pBufHeader
*
*  @param  bBufOwner
*
*/
/* ======================================================================= */
typedef struct G723_1ENC_BUFFERHEADERTYPE_INFO {
    OMX_BUFFERHEADERTYPE* pBufHeader[G723_1ENC_MAX_NUM_OF_BUFS];
    G723_1ENC_BUFFS bBufOwner[G723_1ENC_MAX_NUM_OF_BUFS];
}G723_1ENC_BUFFERHEADERTYPE_INFO;


typedef OMX_ERRORTYPE (*G723_1ENC_fpo)(OMX_HANDLETYPE);

/* =================================================================================== */
/**
* Socket node Audio Codec Configuration parameters.
*/
/* =================================================================================== */
typedef struct G723_1ENC_AudioCodecParams {
    unsigned long  iSamplingRate;
    unsigned long  iStrmId;
    unsigned short iAudioFormat;
}G723_1ENC_AudioCodecParams; /*Especified as USN_AudioCodecParams on SN Guide*/

/* =================================================================================== */
/**
* G723_1ENC_TALGCtrl                Socket Node Alg Control parameters.
* G723_1ENC_UAlgInBufParamStruct        Input Buffer Param Structure
* G723_1ENC_UAlgOutBufParamStruct   Output Buffer Param Structure
*/
/* =================================================================================== */
/* Algorithm specific command parameters */
typedef struct {
    unsigned int size;                 /*< @sizeField >*/
    unsigned int frameSize;            /*< Input frame size in bytes for sample based codecs>*/
    unsigned int bitRate;              /*< Optional, codec-specific bit rate> */
    unsigned int mode;                 /*< Optional, codec-specific mode > */
    unsigned int vadFlag;              /*< @copydoc ISPEECH1_VADFlag> */
    unsigned int noiseSuppressionMode; /*< Optional, codec-specific noise > */
    unsigned int ttyTddMode;           /*< Optional, codec-specific TTY mode> */
    unsigned int dtmfMode;             /*< Optional, codec-specific DTMF mode>*/
    unsigned int dataTransmit;         /*< Optional, codec-specific data transmit>*/
    unsigned int reserved;             /*< Reserved - serves to pad this structure> */
}G723_1ENC_TALGCtrl;

typedef enum 
{
    ALGCMD_BITRATE = 100,   /*set rate*/
    ALGCMD_DTX,             /*Voice Activity Detection flag*/
    ALGCMD_NOISE_SUPP_MODE  /* Hight Filter  Noise suppression*/
} eSPEECHENCODE_AlgCmd;


/* =================================================================================== */
/**
* G723ENC_FrameStruct       Input Buffer Frame Structure
* bLastBuffer                       To Send Last Buufer Flag
*/
/* =================================================================================== */

typedef struct G723ENC_FrameStruct {
    LgUns usLastFrame;
} G723ENC_FrameStruct;

/* =================================================================================== */
/**
* G723ENC_ParamStruct       Input Buffer Param Structure
* usNbFrames                        number of frames
* pParamElem                        pointer to G723ENC_FrameStruct
*/
/* =================================================================================== */
typedef struct G723ENC_ParamStruct {

    LgUns usNbFrames;
    G723ENC_FrameStruct *pParamElem;    
} G723ENC_ParamStruct; 

/* =================================================================================== */
/**
* G723_1ENC_UAlgOutBufParamStruct   Output Buffer Param Structure
* ulFrameCount No.of Frames Encoded
*/
/* =================================================================================== */
typedef struct {
    unsigned long ulFrameCount;
}G723_1ENC_UAlgOutBufParamStruct;

/* =================================================================================== */
/**
* G723_1ENC_LCML_BUFHEADERTYPE Buffer Header Type
*/
/* =================================================================================== */
typedef struct G723_1ENC_LCML_BUFHEADERTYPE {
      G723_1ENC_BUFFER_Dir eDir;
      G723ENC_ParamStruct *pIpParam;
      G723ENC_FrameStruct *pFrameParam;
      G723ENC_ParamStruct *pOpParam; /*According SN guide, this should not be necessary*/
      DMM_BUFFER_OBJ* pDmmBuf;
      OMX_BUFFERHEADERTYPE* buffer;
}G723_1ENC_LCML_BUFHEADERTYPE;

typedef struct _G723_1ENC_BUFFERLIST G723_1ENC_BUFFERLIST;

/* =================================================================================== */
/**
* _G723_1ENC_BUFFERLIST Structure for buffer list
*/
/* ================================================================================== */
struct _G723_1ENC_BUFFERLIST{
    OMX_BUFFERHEADERTYPE sBufHdr;
    OMX_BUFFERHEADERTYPE *pBufHdr[G723_1ENC_MAX_NUM_OF_BUFS];
    OMX_U32 bufferOwner[G723_1ENC_MAX_NUM_OF_BUFS];
    OMX_U32 bBufferPending[G723_1ENC_MAX_NUM_OF_BUFS];
    OMX_U32 numBuffers;
    G723_1ENC_BUFFERLIST *pNextBuf;
    G723_1ENC_BUFFERLIST *pPrevBuf;
};

/* =================================================================================== */
/**
* G723_1ENC_PORT_TYPE Structure for PortFormat details
*/
/* =================================================================================== */
typedef struct G723_1ENC_PORT_TYPE {
    OMX_HANDLETYPE hTunnelComponent;
    OMX_U32 nTunnelPort;
    OMX_BUFFERSUPPLIERTYPE eSupplierSetting;
    OMX_U8 nBufferCnt;
    OMX_AUDIO_PARAM_PORTFORMATTYPE* pPortFormat;
} G723_1ENC_PORT_TYPE;

/* =================================================================================== */
/**
* G723_1ENC_COMPONENT_PRIVATE Component private data Structure
*/
/* =================================================================================== */
typedef struct G723_1ENC_COMPONENT_PRIVATE
{
    /** callback Info */           
    OMX_CALLBACKTYPE cbInfo;

    /** port parameters Info */     
    OMX_PORT_PARAM_TYPE* sPortParam; /*Needed??*/
    
    /** priority management */ 
    OMX_PRIORITYMGMTTYPE* sPriorityMgmt;
    
    /** port definition structure */ 
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDef[G723_1ENC_NUM_OF_PORTS];
    
    /** port parameter structure */ 
    OMX_AUDIO_PARAM_G723TYPE* G723_1Params[G723_1ENC_NUM_OF_PORTS];

    /** Buffer info */     
    G723_1ENC_BUFFERHEADERTYPE_INFO BufInfo[G723_1ENC_NUM_OF_PORTS];
    
    /** Component ports */ 
    G723_1ENC_PORT_TYPE *pCompPort[G723_1ENC_NUM_OF_PORTS];
    
    /** LCML headers */     
    G723_1ENC_LCML_BUFHEADERTYPE *pLcmlBufHeader[G723_1ENC_NUM_OF_PORTS];
    
    /** This is component handle */
    OMX_COMPONENTTYPE* pHandle;
    /** Current state of this component */
    
    /** current state variable */     
    OMX_STATETYPE curState;
    
    /** The component thread handle */
    pthread_t ComponentThread;
    
    /** The pipes for sending buffers to the thread */
    int dataPipe[2];
    
    /** The pipes for sending command to the thread */
    int cmdPipe[2];
    
    /** The pipes for sending cmd data to the thread */
    int cmdDataPipe[2];
    
    /** dasf mode flag */ 
    OMX_U32 dasfMode;

    /** acdn mode flag */ 
    OMX_U32 acdnMode;

    /** Set to indicate component is stopping */
    OMX_U32 bIsStopping;

    /** stream ID */ 
    OMX_U32 streamID;

    /** port defaults allocated */ 
    OMX_U32 bPortDefsAllocated;

    /** thread started flag */ 
    OMX_U32 bCompThreadStarted;

    /** version number */     
    OMX_U32 nVersion;

    /** FillThisBufferCount */ 
    OMX_U32 nFillThisBufferCount;

    /** FillBufferDoneCount */
    OMX_U32 nFillBufferDoneCount;

    /** EmptyThisBufferCount */
    OMX_U32 nEmptyThisBufferCount;

    /** EmptyBufferDoneCount */
    OMX_U32 nEmptyBufferDoneCount;

    /** InitParamsInitialized */
    OMX_U32 bInitParamsInitialized;

    /** NumInputBufPending */
    OMX_U32 nNumInputBufPending;

    /** NumOutputBufPending */
    OMX_U32 nNumOutputBufPending;

    /** DisableCommandPending */
    OMX_U32 bDisableCommandPending;

    /** DisableCommandParam */
    OMX_U32 bDisableCommandParam;

    /** Lcml Handle */
    OMX_HANDLETYPE pLcmlHandle;

    /** Mark Data pointer */
    OMX_PTR pMarkData;

    /** Mark Buffer pointer */
    OMX_MARKTYPE *pMarkBuf;

    /** Mark Target component */
    OMX_HANDLETYPE hMarkTargetComponent;

    /** pointer to Input Buffer List */
    G723_1ENC_BUFFERLIST *pInputBufferList;

    /** pointer to Output Buffer List */
    G723_1ENC_BUFFERLIST *pOutputBufferList;

    /** LCML stream attributes */
    LCML_STRMATTR *strmAttr;

    /** pointer to algorithm parameters */
    G723_1ENC_TALGCtrl *pAlgParam;

    /** pointer to audio codec parameters */
    G723_1ENC_AudioCodecParams *pParams;

    /** component name */
    OMX_STRING cComponentName;

    /** component version */
    OMX_VERSIONTYPE ComponentVersion;

    /** pending input buffer headers */
    OMX_BUFFERHEADERTYPE *pInputBufHdrPending[G723_1ENC_MAX_NUM_OF_BUFS];

    /** pending output buffer headers */
    OMX_BUFFERHEADERTYPE *pOutputBufHdrPending[G723_1ENC_MAX_NUM_OF_BUFS];

    /** Flag to set when socket node stop callback should not transition
        component to OMX_StateIdle */
    OMX_U32 bNoIdleOnStop;

    /** Flag set when socket node is stopped */
    OMX_U32 bDspStoppedWhileExecuting;

    /** Number of outstanding FillBufferDone() calls */
    OMX_U32 nOutStandingFillDones;

#ifndef UNDER_CE
    /** sync mutexes and signals */
    pthread_mutex_t AlloBuf_mutex;
    pthread_cond_t AlloBuf_threshold;
    OMX_U8 AlloBuf_waitingsignal;

    pthread_mutex_t InLoaded_mutex;
    pthread_cond_t InLoaded_threshold;
    OMX_U8 InLoaded_readytoidle;

    pthread_mutex_t InIdle_mutex;
    pthread_cond_t InIdle_threshold;
    OMX_U8 InIdle_goingtoloaded;
#endif
    /** pointer to LCML lib */
    void* ptrLibLCML;
   
    /** frame size array */
    OMX_U8 G723_1FrameSize[4];
    
    /** component role */
    OMX_PARAM_COMPONENTROLETYPE componentRole;
    
    /** device string */
    OMX_STRING* sDeviceString;
    
    /** runtime input buffers */
    OMX_U8 nRuntimeInputBuffers;
    
    /** runtime output buffers */
    OMX_U8 nRuntimeOutputBuffers;
   
    /** hold buffer */
    OMX_U8 *pHoldBuffer;

    /** hold length */
    OMX_U16 nHoldLength;

    /** temp buffer */
    OMX_U8 *ptempBuffer;

    /** last out buffer arrived */
    OMX_BUFFERHEADERTYPE *lastOutBufArrived;    

    /** last buffer sent */
    OMX_U8 LastBufSent;

#ifdef __PERF_INSTRUMENTATION__
    PERF_OBJHANDLE pPERF, pPERFcomp;
#endif

    /** Keep buffer timestamps **/
    OMX_S64 arrTimestamp[G723_1ENC_MAX_NUM_OF_BUFS];
    /** Keep buffer nTickCounts **/
    OMX_S64 arrTickCount[G723_1ENC_MAX_NUM_OF_BUFS];
    /** Index to arrTimestamp[], used for input buffer timestamps */
    OMX_U8 IpBufindex;
    /** Index to arrTimestamp[], used for output buffer timestamps */
    OMX_U8 OpBufindex;
    
    /** RM proxy callback */
#ifdef RESOURCE_MANAGER_ENABLED
    RMPROXY_CALLBACKTYPE rmproxyCallback;
#endif
    /** preempted flag */
    OMX_BOOL bPreempted;    

} G723_1ENC_COMPONENT_PRIVATE;


#ifndef UNDER_CE
    OMX_ERRORTYPE OMX_ComponentInit (OMX_HANDLETYPE hComp);
#else
/*  WinCE Implicit Export Syntax */
#define OMX_EXPORT __declspec(dllexport)
/* =================================================================================== */
/**
*  OMX_ComponentInit()  Initializes component
*
*
*  @param hComp         OMX Handle
*
*  @return OMX_ErrorNone = Successful
*          Other error code = fail
*
*/
/* =================================================================================== */
OMX_EXPORT OMX_ERRORTYPE OMX_ComponentInit (OMX_HANDLETYPE hComp);
#endif

#define G723_1ENC_EXIT_COMPONENT_THRD  10
/* =================================================================================== */
/**
*  G723_1ENC_CompThread()  Component thread
*
*
*  @param pThreadData       void*
*
*  @return void
*
*/
/* =================================================================================== */
void* G723_1ENC_CompThread(void* pThreadData);
/* =================================================================================== */
/**
*  G723_1ENC_StartComponentThread()  Starts component thread
*
*
*  @param hComp         OMX Handle
*
*  @return OMX_ErrorNone = Successful
*          Other error code = fail
*
*/
/* =================================================================================== */
OMX_ERRORTYPE G723_1ENC_StartComponentThread(OMX_HANDLETYPE pHandle);
/* =================================================================================== */
/**
*  G723_1ENC_StopComponentThread()  Stops component thread
*
*
*  @param hComp         OMX Handle
*
*  @return OMX_ErrorNone = Successful
*          Other error code = fail
*
*/
/* =================================================================================== */
OMX_ERRORTYPE G723_1ENC_StopComponentThread(OMX_HANDLETYPE pHandle);
/* =================================================================================== */
/**
*  G723_1ENC_FreeCompResources()  Frees allocated memory
*
*
*  @param hComp         OMX Handle
*
*  @return OMX_ErrorNone = Successful
*          Other error code = fail
*
*/
/* =================================================================================== */
OMX_ERRORTYPE G723_1ENC_FreeCompResources(OMX_HANDLETYPE pComponent);
/* =================================================================================== */
/**
*  G723_1ENC_GetCorrespondingLCMLHeader()  Returns LCML header
* that corresponds to the given buffer
*
*  @param pComponentPrivate Component private data
*
*  @return OMX_ErrorNone = Successful
*          Other error code = fail
*/
/* =================================================================================== */
OMX_ERRORTYPE G723_1ENC_GetCorrespondingLCMLHeader( G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate,
                                                    OMX_U8 *pBuffer,
                                                OMX_DIRTYPE eDir,
                                                G723_1ENC_LCML_BUFHEADERTYPE **ppLcmlHdr);
/* =================================================================================== */
/**
*  G723_1ENC_LCMLCallback() Callback from LCML
*
*  @param event     Codec Event
*
*  @param args      Arguments from LCML
*
*  @return OMX_ErrorNone = Successful
*          Other error code = fail
*/
/* =================================================================================== */
OMX_ERRORTYPE G723_1ENC_LCMLCallback(TUsnCodecEvent event,
                                    void * args [10]);
/* =================================================================================== */
/**
*  G723_1ENC_FillLCMLInitParams() Fills the parameters needed
* to initialize the LCML
*
*  @param pHandle OMX Handle
*
*  @param plcml_Init LCML initialization parameters
*
*  @return OMX_ErrorNone = Successful
*          Other error code = fail
*
*/
/* =================================================================================== */
OMX_ERRORTYPE G723_1ENC_FillLCMLInitParams(OMX_HANDLETYPE pHandle,
                                          LCML_DSP *plcml_Init,
                                          OMX_U16 arr[]);
/* =================================================================================== */
/**
*  G723_1ENC_GetBufferDirection() Returns direction of pBufHeader
*
*  @param pBufHeader        Buffer header
*
*  @param eDir              Buffer direction
*
*  @param pComponentPrivate Component private data
*
*  @return OMX_ErrorNone = Successful
*          Other error code = fail
*/
/* =================================================================================== */
OMX_ERRORTYPE G723_1ENC_GetBufferDirection(OMX_BUFFERHEADERTYPE *pBufHeader,
                                          OMX_DIRTYPE *eDir);
/* ===========================================================  */
/**
*  G723_1ENC_HandleCommand()  Handles commands sent via SendCommand()
*
*  @param pComponentPrivate Component private data
*
*  @return OMX_ErrorNone = Successful
*          Other error code = fail
*  @return OMX_ErrorNone = Successful
*          Other error code = fail
*/
/* =================================================================================== */
OMX_U32 G723_1ENC_HandleCommand(G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate);
/* =================================================================================== */
/**
*  G723_1ENC_HandleDataBufFromApp()  Handles data buffers received
* from the IL Client
*
*  @param pComponentPrivate Component private data
*
*  @return OMX_ErrorNone = Successful
*          Other error code = fail
*  @return OMX_ErrorNone = Successful
*          Other error code = fail
*/
/* =================================================================================== */
OMX_ERRORTYPE G723_1ENC_HandleDataBufFromApp(OMX_BUFFERHEADERTYPE *pBufHeader,
                                            G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate);
/* =================================================================================== */
/**
*  G723_1ENC_GetLCMLHandle()  Get the handle to the LCML
*
*
*  @return OMX_ErrorNone = Successful
*          Other error code = fail
*/
/* =================================================================================== */
OMX_HANDLETYPE G723_1ENC_GetLCMLHandle(G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate);
/* =================================================================================== */
/**
*  G723_1ENC_FreeLCMLHandle()  Frees the handle to the LCML
*
*
*  @return OMX_ErrorNone = Successful
*          Other error code = fail
*/
/* =================================================================================== */
OMX_ERRORTYPE G723_1ENC_FreeLCMLHandle();
/* =================================================================================== */
/**
*  G723_1ENC_CleanupInitParams()  Starts component thread
*
*  @param pComponent        OMX Handle
*
*  @return OMX_ErrorNone = Successful
*          Other error code = fail
*/
/* =================================================================================== */
OMX_ERRORTYPE G723_1ENC_CleanupInitParams(OMX_HANDLETYPE pHandle);
/* =================================================================================== */
/**
*  G723_1ENC_SetPending()  Called when the component queues a buffer
* to the LCML
*
*  @param pComponentPrivate     Component private data
*
*  @param pBufHdr               Buffer header
*
*  @param eDir                  Direction of the buffer
*
*  @return None
*/
/* =================================================================================== */
void G723_1ENC_SetPending(G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate,
                         OMX_BUFFERHEADERTYPE *pBufHdr,
                         OMX_DIRTYPE eDir,
                         OMX_U32 lineNumber);
/* =================================================================================== */
/**
*  G723_1ENC_ClearPending()  Called when a buffer is returned
* from the LCML
*
*  @param pComponentPrivate     Component private data
*
*  @param pBufHdr               Buffer header
*
*  @param eDir                  Direction of the buffer
*
*  @return None
*/
/* =================================================================================== */
void G723_1ENC_ClearPending(G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate,
                           OMX_BUFFERHEADERTYPE *pBufHdr,
                           OMX_DIRTYPE eDir,
                           OMX_U32 lineNumber);
/* =================================================================================== */
/**
*  G723_1ENC_IsPending()
*
*
*  @param pComponentPrivate     Component private data
*
*  @return OMX_ErrorNone = Successful
*          Other error code = fail
*/
/* =================================================================================== */
OMX_U32 G723_1ENC_IsPending(G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate,
                           OMX_BUFFERHEADERTYPE *pBufHdr,
                           OMX_DIRTYPE eDir);
/* =================================================================================== */
/**
*  G723_1ENC_FillLCMLInitParamsEx()  Fills the parameters needed
* to initialize the LCML without recreating the socket node
*
*  @param pComponent            OMX Handle
*
*  @return None
*/
/* =================================================================================== */
OMX_ERRORTYPE G723_1ENC_FillLCMLInitParamsEx(OMX_HANDLETYPE pComponent);
/* =================================================================================== */
/**
*  G723_1ENC_IsValid() Returns whether a buffer is valid
*
*
*  @param pComponentPrivate     Component private data
*
*  @param pBuffer               Data buffer
*
*  @param eDir                  Buffer direction
*
*  @return OMX_True = Valid
*          OMX_False= Invalid
*/
/* =================================================================================== */
OMX_U32 G723_1ENC_IsValid(G723_1ENC_COMPONENT_PRIVATE *pComponentPrivate,
                         OMX_U8 *pBuffer,
                         OMX_DIRTYPE eDir);

/* =================================================================================== */
/**
*  G723_ResourceManagerCallback() Callback from Resource Manager
*
*  @param cbData    RM Proxy command data
*
*  @return None
*/
/* =================================================================================== */
#ifdef RESOURCE_MANAGER_ENABLED
void G723_ResourceManagerCallback(RMPROXY_COMMANDDATATYPE cbData);
#endif

/* ======================================================================= */
/** OMX_G723_1ENC_INDEXAUDIOTYPE  Defines the custom configuration settings
*                              for the component
*
*  @param  OMX_IndexCustomG723_1ENCModeConfig      Sets the DASF mode
*
*  
*/
/*  ==================================================================== */
typedef enum OMX_G723_1ENC_INDEXAUDIOTYPE {
    OMX_IndexCustomG723_1ENCModeConfig = 0xFF000001,
    OMX_IndexCustomG723_1ENCStreamIDConfig, 
    OMX_IndexCustomG723_1ENCDataPath
}OMX_G723_1ENC_INDEXAUDIOTYPE;


#endif  /* OMX_G723_1ENC_UTILS__H */
