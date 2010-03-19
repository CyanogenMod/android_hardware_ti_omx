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
* @file tiomxplayer.c
*
* This file contains the test application code that invokes the component.
*
* @path  $(CSLPATH)\OMAPSW_MPU\linux\audio\src\openmax_il\
*
* @rev  1.0
*/
/* ----------------------------------------------------------------------------
*!
*! Revision History
*! ===================================
*! xx-July-2009 mg-ad: Initial version
*! xx-December-2009 mg: Added WMA and G729 support
*! xx-March-2010 mg: ported to Android
* =========================================================================== */
/* ------compilation control switches -------------------------*/
/****************************************************************
*  INCLUDE FILES
****************************************************************/
/* ----- system and platform files ----------------------------*/
#include "tiomxplayer.h"
/* ----- globarl variables  -----------------------------------*/

static OMX_BOOL internal_allocation = OMX_TRUE;

FILE * infile  = NULL;
FILE * outfile = NULL;
char * nothing = "/dev/null";
FILE * pnothing;
/* ----- function definitions----------------------------------*/
static int unget_buffers(appPrivateSt* appPrvt){

  OMX_ERRORTYPE error = OMX_ErrorNone;
  unsigned int i;

  APP_DPRINT("Change state to Idle\n");
  error = OMX_SendCommand(appPrvt->phandle,
                          OMX_CommandStateSet,
                          OMX_StateIdle,
                          NULL);
  if(error != OMX_ErrorNone){
    return 1;
  }

  error = WaitForState(appPrvt->phandle,OMX_StateIdle,appPrvt->state);
  if(error != OMX_ErrorNone){
    return 1;
  }

  APP_DPRINT("Change state to Loaded\n");
  error = OMX_SendCommand(appPrvt->phandle,
                          OMX_CommandStateSet,
                          OMX_StateLoaded,
                          NULL);
  if(error != OMX_ErrorNone){
    return 1;
  }

  for (i = 0; i < appPrvt->in_port->nBufferCountActual; i++){
    if(!internal_allocation){
      appPrvt->in_buffers[i] -= CACHE_ALIGNMENT;
    }
    APP_DPRINT("Free IN buffer %p\n",appPrvt->in_buffers[i]);
    error = OMX_FreeBuffer (appPrvt->phandle,
                            OMX_DirInput,
                            appPrvt->in_buffers[i]);
    if(error != OMX_ErrorNone){
      APP_DPRINT("Error in Free IN buffer %p\n",appPrvt->in_buffers[i]);
      return 1;
    }
  }
  if ((appPrvt->mode==FILE_MODE) || (appPrvt->mode==ALSA_MODE)){
    for (i = 0; i < appPrvt->out_port->nBufferCountActual; i++){
      if(!internal_allocation){
        appPrvt->in_buffers[i] -= CACHE_ALIGNMENT;
      }
      APP_DPRINT("Free OUT buffer %p\n",appPrvt->out_buffers[i]);
      error = OMX_FreeBuffer(appPrvt->phandle,
                             OMX_DirOutput,
                             appPrvt->out_buffers[i]);
      if(error != OMX_ErrorNone){
        APP_DPRINT("Error in Free OUT buffer %p\n",appPrvt->out_buffers[i]);
        return 1;
      }
    }
  }

  error = WaitForState(appPrvt->phandle,OMX_StateLoaded,appPrvt->state);
  if(error != OMX_ErrorNone){
    return 1;
  }
  return 0;
}

static int get_buffers(appPrivateSt *appPrvt)
{
  int error = 0;

  /* internal_allocation = TRUE means use OMX_AllocateBuffer *
   * internal_allocation = FALSE means use OMX_UseBuffer     */

  APP_DPRINT("Change state to Idle\n");
  error = OMX_SendCommand(appPrvt->phandle,
                          OMX_CommandStateSet,
                          OMX_StateIdle,
                          NULL);
  if(error != OMX_ErrorNone){
    return 1;
  }
  if(internal_allocation){
    APP_DPRINT("Allocate buffer\n");
    error = allocate_buffer(appPrvt);
    if(error){
      return error;
    }
  }else{
    APP_DPRINT("Use buffer\n");
    error = use_buffer(appPrvt);
    if(error){
      return error;
    }
  }
  error = WaitForState(appPrvt->phandle,OMX_StateIdle,appPrvt->state);
  if(error != OMX_ErrorNone){
    return 1;
  }
  return error;
}

/** test: Switch on different test cases
 *
 * @param appPrvt Handle to the app component structure.
 *
 */
int test(appPrivateSt *appPrvt){

  int error = 0;

  switch(appPrvt->tc){
  case 1:
    APP_DPRINT("********TEST PLAY BEGIN********\n");
    if(!(error = test_repeat(appPrvt))){
      APP_DPRINT("********TEST PLAY FINISHED********\n");
    }else{
      APP_DPRINT("********TEST PLAY EXITED********\n");
    }
    break;
  case 2:
    APP_DPRINT("********TEST PAUSE-RESUME BEGIN********\n");
    if(!(error = test_pause_resume(appPrvt))){
      APP_DPRINT("********TEST PAUSE-RESUME FINISHED********\n");
    }else{
      APP_DPRINT("********TEST PAUSE-RESUME EXITED********\n");
    }
    break;
    //TODO: ADD MORE TEST CASES
  default:
    break;
  }
  return error;
}

void display_help()
{
    printf("\n");
    printf("Usage: ./tiomxplayer in-file [-o outfile]  [-c num_channels] [-r frequency] [ -t test_case] [-h] ");
    printf("\n");
    printf("       -o outfile: If this option is specified, the decoded stream is written to outfile if not it will be rendered in ALSA\n");
    printf("       -d : If this option is specified, the decoded stream is rendered in DASF, if not it will be rendered in ALSA\n");
    printf("       -c num_channels: To decode in a specific channel number (MONO/STEREO). Default value is STEREO \n");
    printf("       -r sampling rate:  To decode in a specific frequency. Default value is 44100 \n");
    printf("       -x input buffer amount:  Amount of input buffers, default is 1 \n");
    printf("       -X input buffer size:  Input buffer size in bytes \n");
    printf("       -y output buffer amount:  Amount of output buffers, default is 1 \n");
    printf("       -Y output buffer size:  Output buffer size in bytes \n");
    printf("       -t test_case:  Specify test case number. Default value is 1 (play until EOF) \n");
    printf("       -D Device:  Specify the Device to render audio through ALSA\n");
    printf("       -h : Display help \n");
    printf("\n");

    exit(1);
}
static int parameter_check(int argc,char **argv, appPrivateSt *appPrvt)
{
    //check if parameters are correct, if not display this help
    char *outputFile = NULL;
    char *inputFile =NULL;
    int index;
    int fn_index=0;
    int c;

    opterr = 0;
    if(argc < 2){
        display_help();
    }
    else{
        while ((c = getopt (argc, argv, "o:c:r:t:x:X:y:Y:i:D:dRh")) != -1)
        switch (c)
        {
        case 'o':
            appPrvt->mode=FILE_MODE;
            outputFile = optarg;
            break;
        case 'd':
            appPrvt->mode=DASF_MODE;
            outputFile = optarg;
            break;
        case 'c':
            appPrvt->channels = atoi(optarg);
            break;
        case 'r':
            appPrvt->samplerate = atoi(optarg);
            break;
        case 't':
            appPrvt->tc = atoi(optarg);
            break;
        case 'x':
            appPrvt->nIpBuf = atoi(optarg);
            break;
        case 'X':
            appPrvt->IpBufSize = atoi(optarg);
            break;
        case 'y':
            appPrvt->nOpBuf = atoi(optarg);
            break;
        case 'Y':
            appPrvt->OpBufSize = atoi(optarg);
            break;
        case 'i':
            appPrvt->iterations = atoi(optarg);
            break;
        case 'R':
            appPrvt->raw = OMX_TRUE;
            break;
        case 'D':
            appPrvt->Device = atoi(optarg);
            break;
        case 'h':
            display_help();
            break;
        case '?':
            if (optopt == 'o')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (optopt == 'c')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (optopt == 'r')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (optopt == 't')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (optopt == 'x')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (optopt == 'X')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (optopt == 'y')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (optopt == 'Y')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (optopt == 'i')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (optopt == 'D')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
            fprintf (stderr,
                     "Unknown option character `\\x%x'.\n",
                     optopt);
            printf("Use -h for help\n");
            exit(1);
        default:
            abort ();
        }
        for (index = optind; index < argc; index++){
            inputFile=argv[index];
            printf ("\nNon-option argument %s\n", argv[index]);
        }
        //must validate input file existance here
        if(!inputFile){
            printf("inputFile doesn't exist!\n");
            exit(1);
        }
        fn_index = strlen(inputFile);
        if(fn_index < 4){
            printf("Invalid filename \n");
            exit(1);
        }
        if(strcasecmp((inputFile+fn_index -4),".mp3")==0){
            appPrvt->eEncoding=OMX_AUDIO_CodingMP3;
            printf("MP3 Format is selected\n");
        }
        else if(strcasecmp((inputFile+fn_index -4),".aac")==0){
            appPrvt->eEncoding=OMX_AUDIO_CodingAAC;
            printf("AAC Format is selected\n");
        }
        else if((strcasecmp((inputFile+fn_index -4),".rca")==0) ){
            appPrvt->eEncoding= OMX_AUDIO_CodingWMA;
            printf("WMA Format is selected\n");
        }
        else if((strcasecmp((inputFile+fn_index -4),".amr")==0) ||
                (strcasecmp((inputFile+fn_index -4),".cod")==0)){
            appPrvt->eEncoding=OMX_AUDIO_CodingAMR;
            appPrvt->amr_mode=OMX_FALSE;
            printf("NBAMR Format is selected\n");
        }
        else if((strcasecmp((inputFile+fn_index -6),".amrwb")==0) ||
                (strcasecmp((inputFile+fn_index -4),".awb")==0)){
            appPrvt->eEncoding=OMX_AUDIO_CodingAMR;
            appPrvt->amr_mode=OMX_TRUE;
            printf("WBAMR Format is selected\n");
        }
        else if((strcasecmp((inputFile+fn_index -5),".g729")==0) ){
            appPrvt->eEncoding= OMX_AUDIO_CodingG729;
            printf("G729 Format is selected\n");
        }
        else{
            printf("Format not recognized\n");
            exit(1);
        }

        if((infile = fopen(inputFile,"r")) == NULL){
            perror("fopen-infile\n");
            exit(1);
        }

        if(appPrvt->mode==DASF_MODE){
            printf( "Mode              :DASF Mode \n");
        }
        else if(appPrvt->mode==FILE_MODE){
            if(!outputFile){
              printf("outputFile doesn't exist!\n");
              exit(1);
            }
            printf( "Mode              :File Mode \n");
            printf( "Output File       :%s \n",outputFile);
            if((outfile = fopen(outputFile,"w")) == NULL){
              perror("fopen-outfile\n");
              exit(1);
            }
            if((pnothing = fopen(nothing,"w")) == NULL){
              perror("fopen-/dev/null\n");
              exit(1);
            }
        }
        else{
          printf( "Mode              :Alsa Mode \n");
        }
        printf( "Input File        :%s \n",inputFile);
        printf ("Channels          :%d \n", appPrvt->channels);
        printf("Sampling Rate     :%d \n", (int)appPrvt->samplerate);
        printf("Test case         :%d \n",appPrvt->tc);
    }
    return 0;
}

static int config_params(appPrivateSt* appPrvt)
{
  OMX_ERRORTYPE error = OMX_ErrorNone;

  config_pcm(appPrvt);

  switch(appPrvt->in_port->format.audio.eEncoding){
  case OMX_AUDIO_CodingMP3:
    config_mp3(appPrvt);
    break;
  case OMX_AUDIO_CodingAAC:
    config_aac(appPrvt);
    break;
  case OMX_AUDIO_CodingWMA:
      config_wma(appPrvt);
      break;
  case OMX_AUDIO_CodingAMR:
      if(!appPrvt->amr_mode)
        config_nbamr(appPrvt);
      else
        config_wbamr(appPrvt);
    break;
  case OMX_AUDIO_CodingG729:
      config_g729(appPrvt);
      break;
    /*TODO: add other components */
  default:
    APP_DPRINT("OMX_AUDIO_CodingXXX not found\n");
    break;
  }

  return error;

}

appPrivateSt* app_core_new(void){
  appPrivateSt* me = (appPrivateSt *) malloc (sizeof (appPrivateSt));
  if (NULL == me) {
    perror("malloc-app_core_new");
    return me;
  }
  me->phandle = NULL;
  me->in_port = NULL;
  me->out_port = NULL;
  me->in_buffers = NULL;
  me->out_buffers = NULL;
  me->pcm = NULL;
  me->mp3 = NULL;
  me->aac = NULL;
  me->wma = NULL;
  me->nbamr = NULL;
  me->wbamr = NULL;
  me->g729 = NULL;
  me->mode=ALSA_MODE;
  me->iterations=1;
  me->Device=0;
  me->channels = 2;
  me->samplerate = 44100;
  me->nIpBuf = 1;
  me->IpBufSize = IN_BUFFER_SIZE;
  me->nOpBuf = 1;
  me->OpBufSize = OUT_BUFFER_SIZE;
  me->done_flag = OMX_FALSE;
  me->raw = OMX_FALSE;
  me->amr_mode = OMX_FALSE;
  me->tc=1; /*play until eof*/
  me->eos = NULL;
  me->flush = NULL;
  me->state = NULL;
  me->pause = NULL;
  me->processed_buffers = 0;
  return me;
}

static int alloc_app_resources(appPrivateSt* appPrvt)
{
    appPrvt->in_port =  (OMX_PARAM_PORTDEFINITIONTYPE *)malloc (sizeof (OMX_PARAM_PORTDEFINITIONTYPE));
    if (NULL == appPrvt->in_port) {
        perror("malloc-in_port");
        return 1;
    }
    appPrvt->out_port =  (OMX_PARAM_PORTDEFINITIONTYPE *)malloc (sizeof (OMX_PARAM_PORTDEFINITIONTYPE));
    if (NULL == appPrvt->out_port) {
        perror("malloc-out_port");
        return 1;
    }

    appPrvt->in_buffers = (OMX_BUFFERHEADERTYPE **) malloc (sizeof (OMX_BUFFERHEADERTYPE) * 10);
    if (NULL == appPrvt->in_buffers) {
      perror("malloc-in_buffers");
      return 1;
    }
    appPrvt->out_buffers = (OMX_BUFFERHEADERTYPE **) malloc (sizeof (OMX_BUFFERHEADERTYPE) * 10);
    if (NULL == appPrvt->out_buffers) {
      perror("malloc-out_buffers");
      return 1;
    }
    appPrvt->in_port->format.audio.eEncoding = appPrvt->eEncoding;

    switch(appPrvt->in_port->format.audio.eEncoding){
    case OMX_AUDIO_CodingMP3:
        appPrvt->mp3 = malloc (sizeof(OMX_AUDIO_PARAM_MP3TYPE));
        if (NULL == appPrvt->mp3) {
            perror("malloc-mp3");
            return 1;
        }
        break;
    case OMX_AUDIO_CodingAAC:
        appPrvt->aac = malloc (sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));
        if (NULL == appPrvt->aac) {
            perror("malloc-aac");
            return 1;
        }
        break;
    case OMX_AUDIO_CodingWMA:
        appPrvt->wma = malloc (sizeof(OMX_AUDIO_PARAM_WMATYPE));
        if (NULL == appPrvt->wma) {
            perror("malloc-wma");
            return 1;
        }
        break;
    case OMX_AUDIO_CodingAMR:
        if(!appPrvt->amr_mode){
            appPrvt->nbamr = malloc (sizeof(OMX_AUDIO_PARAM_AMRTYPE));
            if (NULL == appPrvt->nbamr) {
                perror("malloc-nbamr");
                return 1;
            }
        }else{
            appPrvt->wbamr = malloc (sizeof(OMX_AUDIO_PARAM_AMRTYPE));
            if (NULL == appPrvt->wbamr) {
                perror("malloc-wbamr");
                return 1;
            }
        }
        break;
    case OMX_AUDIO_CodingG729:
        appPrvt->g729 = malloc (sizeof(OMX_AUDIO_PARAM_G729TYPE));
        if (NULL == appPrvt->g729) {
            perror("malloc-g729");
            return 1;
        }
        break;
        /*TODO: add other components */
    default:
        APP_DPRINT("OMX_AUDIO_CodingXXX not found\n");
        break;
    }
    appPrvt->pcm = malloc (sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
    if (NULL == appPrvt->pcm) {
        perror("malloc-pcm");
        return 1;
    }
    appPrvt->eos = malloc (sizeof(Event_t));
    if (NULL == appPrvt->eos) {
        perror("malloc-eos");
        return 1;
    }
    appPrvt->flush = malloc (sizeof(Event_t));
    if (NULL == appPrvt->flush) {
        perror("malloc-flush");
        return 1;
    }
    appPrvt->state = malloc (sizeof(Event_t));
    if (NULL == appPrvt->state) {
        perror("malloc-state");
        return 1;
    }
    appPrvt->pause = malloc (sizeof(Event_t));
    if (NULL == appPrvt->pause) {
        perror("malloc-pause");
        return 1;
    }
    /* Initialize component role */
    appPrvt->pCompRoleStruct = malloc (sizeof (OMX_PARAM_COMPONENTROLETYPE));
    if(NULL == appPrvt->pCompRoleStruct) {
        perror("malloc-OMX_PARAM_COMPONENTROLETYPE");
        return 1;
    }

    if(appPrvt->mode==ALSA_MODE){
      appPrvt->alsaPrvt = malloc (sizeof (alsaPrvtSt));
      if(NULL == appPrvt->alsaPrvt) {
        perror("malloc-alsaPrvt");
        return 1;
      }
    }

    return 0;
}

static int config_ports(appPrivateSt* appPrvt)
{
  OMX_ERRORTYPE error = OMX_ErrorNone;
  error = OMX_SendCommand(appPrvt->phandle, OMX_CommandPortDisable, OMX_ALL,NULL);
  if(error != OMX_ErrorNone) {
    APP_DPRINT ("Error from SendCommand-port disable function - error= %d\n",
                error);
    return 1;
  }
  /* Get/Set INPUT params for port definition */
  OMX_INIT_STRUCT(appPrvt->in_port,OMX_PARAM_PORTDEFINITIONTYPE);

  appPrvt->in_port->nPortIndex = OMX_DirInput;
  error = OMX_GetParameter (appPrvt->phandle,
                            OMX_IndexParamPortDefinition,
                            appPrvt->in_port);
  if (error != OMX_ErrorNone) {
    error = OMX_ErrorBadParameter;
    APP_DPRINT ("Error in GetParameter\n");
    return 1;
  }

  appPrvt->in_port->eDir = OMX_DirInput;
  appPrvt->in_port->nBufferCountActual = appPrvt->nIpBuf;
  appPrvt->in_port->nBufferSize = appPrvt->IpBufSize;

  error = OMX_SetParameter (appPrvt->phandle,
                            OMX_IndexParamPortDefinition,
                            appPrvt->in_port);
  if (error != OMX_ErrorNone) {
    error = OMX_ErrorBadParameter;
    APP_DPRINT ("Error in SetParameter\n");
    return 1;
  }

  /* Get/Set OUTPUT params for port definition */
  OMX_INIT_STRUCT(appPrvt->out_port,OMX_PARAM_PORTDEFINITIONTYPE);
  appPrvt->out_port->nPortIndex = OMX_DirOutput;
  error = OMX_GetParameter (appPrvt->phandle,
                            OMX_IndexParamPortDefinition,
                            appPrvt->out_port);
  if (error != OMX_ErrorNone) {
    error = OMX_ErrorBadParameter;
    APP_DPRINT ("Error in GetParameter\n");
    return 1;
  }
  appPrvt->out_port->eDir = OMX_DirOutput;
  appPrvt->out_port->nBufferCountActual = appPrvt->nOpBuf;
  appPrvt->out_port->nBufferSize = appPrvt->OpBufSize;

  APP_DPRINT("call set_parameter for OUT OMX_PARAM_PORTDEFINITIONTYPE\n");
  error = OMX_SetParameter (appPrvt->phandle,
                            OMX_IndexParamPortDefinition,
                            appPrvt->out_port);
  if (error != OMX_ErrorNone) {
    error = OMX_ErrorBadParameter;
    APP_DPRINT ("Error in SetParameter\n");
    return 1;
  }
  /*after the get/set the port setting need to enabled */
  error = OMX_SendCommand(appPrvt->phandle, OMX_CommandPortEnable, OMX_ALL,NULL);
  if(error != OMX_ErrorNone) {
    APP_DPRINT ("Error from SendCommand-port enable function - error = %d\n",
                error);
    return 1;
  }
  /*port enable is done*/

  return 0;
}

static int prepare(appPrivateSt* appPrvt, OMX_CALLBACKTYPE callbacks)
{
    OMX_ERRORTYPE error = OMX_ErrorNone;
    OMX_STRING audio_decode_string = "OMX.TI.AUDIO.DECODE";
    /*Allocating application resources*/
#ifdef OMAP3
    switch(appPrvt->eEncoding){
    case OMX_AUDIO_CodingMP3:
        audio_decode_string = "OMX.TI.MP3.decode";
        break;
    case OMX_AUDIO_CodingAAC:
        audio_decode_string = "OMX.TI.AAC.decode";
        break;
    case OMX_AUDIO_CodingWMA:
        audio_decode_string = "OMX.TI.WMA.decode";
        break;
    case OMX_AUDIO_CodingAMR:
        if(!appPrvt->amr_mode)
            audio_decode_string = "OMX.TI.AMR.decode";
        else
            audio_decode_string = "OMX.TI.WBAMR.decode";
        break;
    case OMX_AUDIO_CodingG729:
        audio_decode_string = "OMX.TI.G729.decode";
        break;
    default:
        APP_DPRINT("Component index not found \n");
        break;
    }
#endif
    alloc_app_resources(appPrvt);

    /*Initializing mutexes*/
    event_init(appPrvt->eos,0);
    event_init(appPrvt->flush,0);
    event_init(appPrvt->state,0);
    event_init(appPrvt->pause,0);

    /* Get the component handle */
    error = OMX_GetHandle(&appPrvt->phandle,
                          audio_decode_string,
                          appPrvt,
                          &callbacks);
    if (error != OMX_ErrorNone) {
        APP_DPRINT("Error in OMX_GetHandle-%d\n",error);
        return 1;
    }

    OMX_INIT_STRUCT(appPrvt->pCompRoleStruct,OMX_PARAM_COMPONENTROLETYPE);

    /* Check for default role */
    /* Commenting for now, as it seems get parameter does not have this index in
       current implementation*/
    /*
      error = OMX_GetParameter (appPrvt->phandle,
                              OMX_IndexParamStandardComponentRole,
                              appPrvt->pCompRoleStruct);
    if (error != OMX_ErrorNone) {
        APP_DPRINT("Error in OMX_GetParameter-%d\n",error);
        return 1;
        }
    */
    /* Now set the role to the appropriate format */
    switch(appPrvt->in_port->format.audio.eEncoding){
    case OMX_AUDIO_CodingMP3:
        strcpy((char*)appPrvt->pCompRoleStruct->cRole,"audio_decode.dsp.mp3");
        break;
    case OMX_AUDIO_CodingAAC:
        strcpy((char*)appPrvt->pCompRoleStruct->cRole,"audio_decode.dsp.aac");
        break;
    case OMX_AUDIO_CodingWMA:
        strcpy((char*)appPrvt->pCompRoleStruct->cRole,"audio_decode.dsp.wma");
        break;
    case OMX_AUDIO_CodingAMR:
        if(!appPrvt->amr_mode)
          strcpy((char*)appPrvt->pCompRoleStruct->cRole,"audio_decode.dsp.amrnb");
        else
          strcpy((char*)appPrvt->pCompRoleStruct->cRole,"audio_decode.dsp.amrwb");

        break;
    case OMX_AUDIO_CodingG729:
        strcpy((char*)appPrvt->pCompRoleStruct->cRole,"audio_decode.dsp.g729");
        break;
    default:
        strcpy((char*)appPrvt->pCompRoleStruct->cRole,"audio_decode.dsp.mp3");
        break;
    }
    error = OMX_SetParameter (appPrvt->phandle,
                              OMX_IndexParamStandardComponentRole,
                              appPrvt->pCompRoleStruct);
    if (error != OMX_ErrorNone) {
        error = OMX_ErrorBadParameter;
        APP_DPRINT("Error in OMX_SetParameter-%d\n",error);
        return 1;
    }

    APP_DPRINT("Role changed to %s\n",appPrvt->pCompRoleStruct->cRole);
    /*role has been set*/

    if(appPrvt->mode==ALSA_MODE){
      alsa_setAudioParams(appPrvt) ;
    }

    return 0;
}

static int free_app_resources(appPrivateSt* appPrvt)
{
  OMX_ERRORTYPE error = OMX_ErrorNone;

  /* Free allocated resources */
  APP_DPRINT("Free pcm params\n");
  free(appPrvt->pcm);
  appPrvt->pcm = NULL;

  switch(appPrvt->in_port->format.audio.eEncoding){
  case OMX_AUDIO_CodingMP3:
    APP_DPRINT("Free mp3 params\n");
    free(appPrvt->mp3);
    appPrvt->mp3 = NULL;
    break;
  case OMX_AUDIO_CodingAAC:
    APP_DPRINT("Free aac params\n");
    free(appPrvt->aac);
    appPrvt->aac = NULL;
    break;
  case OMX_AUDIO_CodingWMA:
    APP_DPRINT("Free wma params\n");
    free(appPrvt->wma);
    appPrvt->wma = NULL;
    break;
  case OMX_AUDIO_CodingAMR:
      if(!appPrvt->amr_mode){
          APP_DPRINT("Free nbamr params\n");
          free(appPrvt->nbamr);
          appPrvt->nbamr = NULL;
      }else{
          APP_DPRINT("Free wbamr params\n");
          free(appPrvt->wbamr);
          appPrvt->wbamr = NULL;
      }
    break;
  case OMX_AUDIO_CodingG729:
      APP_DPRINT("Free g729 params\n");
      free(appPrvt->g729);
      appPrvt->g729 = NULL;
      break;
    /*TODO: add other components */
  default:
    APP_DPRINT("OMX_AUDIO_CodingXXX not found\n");
    break;
  }

  APP_DPRINT("Free PORTDEFINITIONTYPE\n");
  free(appPrvt->in_port);
  appPrvt->in_port = NULL;

  free(appPrvt->out_port);
  appPrvt->out_port = NULL;

  free(appPrvt->pCompRoleStruct);
  appPrvt->pCompRoleStruct = NULL;

  free(appPrvt->in_buffers);
  appPrvt->in_buffers = NULL;

  free(appPrvt->out_buffers);
  appPrvt->out_buffers = NULL;

  if(appPrvt->mode==ALSA_MODE){
    snd_pcm_drain(appPrvt->alsaPrvt->playback_handle);
    snd_pcm_close(appPrvt->alsaPrvt->playback_handle);
    free(appPrvt->alsaPrvt);
    appPrvt->alsaPrvt = NULL;
  }

  /* Free handle */
  APP_DPRINT("Free OMX handle\n");
  error = OMX_FreeHandle(appPrvt->phandle);
  if( (error != OMX_ErrorNone)) {
    APP_DPRINT ("Error in Free Handle function\n");
    return 1;
  }
  APP_DPRINT ("OMX_FreeHandle returned Successfully!\n");

  event_deinit(appPrvt->eos);
  free(appPrvt->eos);
  appPrvt->eos = NULL;

  event_deinit(appPrvt->flush);
  free(appPrvt->flush);
  appPrvt->flush = NULL;

  event_deinit(appPrvt->state);
  free(appPrvt->state);
  appPrvt->state = NULL;

  event_deinit(appPrvt->pause);
  free(appPrvt->pause);
  appPrvt->pause = NULL;

  if(fclose(infile)){
    return 1;
  }
  if(appPrvt->mode == FILE_MODE){
    if(fclose(outfile)){
      return 1;
    }
  }

  /* Last but not least */
  free(appPrvt);
  appPrvt = NULL;

  return 0;
}

int main (int argc,char **argv)
{

  appPrivateSt *appPrvt = NULL;
  static OMX_CALLBACKTYPE callbacks =
  {
    EventHandler,
    EmptyBufferDone,
    FillBufferDone
  };

  appPrvt = app_core_new();
  if(!appPrvt){
    APP_DPRINT("ERROR! no resources!\n");
    return 1;
  }

  parameter_check(argc,argv, appPrvt);

  OMX_Init();

  prepare(appPrvt, callbacks);

  config_ports(appPrvt);

  config_params(appPrvt);

  get_buffers(appPrvt);

  test(appPrvt);

  unget_buffers(appPrvt);

  free_app_resources(appPrvt);

  OMX_Deinit ();

  return 0;
}
