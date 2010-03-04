#include "tiomxplayer.h"

extern FILE* infile;
extern FILE* outfile;

/** Initializes the event at a given value
 *
 * @param event the event to initialize
 * @param val the initial value of the event
 *
 */
void event_init(Event_t* event, unsigned int val) {
  pthread_cond_init(&event->cond, NULL);
  pthread_mutex_init(&event->mutex, NULL);
  event->semval = val;
}

/** Destroy the event
 *
 * @param event the event to destroy
 */
void event_deinit(Event_t* event) {
  pthread_cond_destroy(&event->cond);
  pthread_mutex_destroy(&event->mutex);
}

/** Decreases the value of the event. Blocks if the event
 * value is zero.
 *
 * @param event the event to decrease
 */
void event_block(Event_t* event) {
  pthread_mutex_lock(&event->mutex);
  while (event->semval == 0) {
    pthread_cond_wait(&event->cond, &event->mutex);
  }
  event->semval--;
  pthread_mutex_unlock(&event->mutex);
}

/** Increases the value of the event
 *
 * @param event the event to increase
 */
void event_wakeup(Event_t* event) {
  pthread_mutex_lock(&event->mutex);
  event->semval++;
  pthread_cond_signal(&event->cond);
  pthread_mutex_unlock(&event->mutex);
}

/** Reset the value of the event
 *
 * @param event the event to reset
 */
void event_reset(Event_t* event) {
  pthread_mutex_lock(&event->mutex);
  event->semval=0;
  pthread_mutex_unlock(&event->mutex);
}

/** Wait on the cond.
 *
 * @param event the event to wait
 */
void event_wait(Event_t* event) {
  pthread_mutex_lock(&event->mutex);
  pthread_cond_wait(&event->cond, &event->mutex);
  pthread_mutex_unlock(&event->mutex);
}

/** Signal the cond,if waiting
 *
 * @param event the event to signal
 */
void event_signal(Event_t* event) {
  pthread_mutex_lock(&event->mutex);
  pthread_cond_signal(&event->cond);
  pthread_mutex_unlock(&event->mutex);
}

/** Waits for the OMX component to change to the desired state
 *
 * @param pHandle Handle to the component
 * @param Desiredstate State to be reached
 * @param event the event to hold on
 *
 */
OMX_ERRORTYPE WaitForState(OMX_HANDLETYPE pHandle,
                           OMX_STATETYPE DesiredState,
                           Event_t* pevent){

  OMX_STATETYPE CurState = OMX_StateInvalid;
  OMX_ERRORTYPE eError = OMX_ErrorNone;

  eError = OMX_GetState(pHandle, &CurState);
  if (CurState == OMX_StateInvalid){
    APP_DPRINT("OMX_StateInvalid!\n");
    eError = OMX_ErrorInvalidState;
  }else if(CurState != DesiredState){
    event_wait(pevent);
  }

  return eError;
}

OMX_ERRORTYPE EventHandler(OMX_HANDLETYPE hComponent,
                           OMX_PTR pAppData,
                           OMX_EVENTTYPE eEvent,
                           OMX_U32 nData1,
                           OMX_U32 nData2,
                           OMX_PTR pEventData)
{

  appPrivateSt* appPrvt = (appPrivateSt*)pAppData;

  switch (eEvent){
  case OMX_EventCmdComplete:
    if (nData1 == OMX_CommandStateSet){
      APP_DPRINT("OMX_EventCmdComplete %ld\n",nData2);
      event_wakeup(appPrvt->state);
    }
    if (nData1 == OMX_CommandPortDisable){
      APP_DPRINT("PortDisable complete for %ld\n",nData2);
    }
    break;
  case OMX_EventError:
    if(nData1 == OMX_ErrorStreamCorrupt){
      APP_DPRINT("OMX_EventError - %s\n",(char*)pEventData);
    }else{
      APP_DPRINT("OMX_EventError\n");
    }
    break;

  case OMX_EventBufferFlag:
#ifdef OMAP3
    if((nData2 == (int)OMX_BUFFERFLAG_EOS) && (nData1 == OMX_DirOutput)){
      APP_DPRINT("EOS reported!\n");
      appPrvt->done_flag = OMX_TRUE;
      event_wakeup(appPrvt->eos);
    }
#else
    if((nData2 == (int)OMX_BUFFERFLAG_EOS) && (nData1 == OMX_DirInput)){
      APP_DPRINT("EOS reported!\n");
      appPrvt->done_flag = OMX_TRUE;
      event_wakeup(appPrvt->eos);
    }
#endif
    break;
  case OMX_EventMax:
    APP_DPRINT("EventMax %ld %ld\n",nData1,nData2);
    break;
  case OMX_EventMark:
    APP_DPRINT("OMX_EventMark %ld %ld\n",nData1,nData2);
    break;
  case OMX_EventPortSettingsChanged:
#ifdef OMAP3
      /*@TODO: Implement Port reconfiguration (this is a hack) */
      if (OMX_SendCommand(hComponent, OMX_CommandPortEnable, 1,NULL)){
          printf("There was an error on port enable \n");
      }
#endif
    APP_DPRINT("OMX_EventPortSettingsChanged %ld %ld\n",nData1,nData2);
    break;
  case OMX_EventComponentResumed:
    APP_DPRINT("OMX_EventComponentResumed %ld %ld\n",nData1,nData2);
    break;
  case OMX_EventDynamicResourcesAvailable:
    APP_DPRINT("OMX_EventDynamicResourcesAvailable %ld %ld\n",nData1,nData2);
    break;
  case OMX_EventPortFormatDetected:
    APP_DPRINT("OMX_EventPortFormatDetected %ld %ld\n",nData1,nData2);
    break;
  case OMX_EventResourcesAcquired:
    APP_DPRINT("OMX_EventResourcesAcquired %ld %ld\n",nData1,nData2);
    break;
  default:
    break;
  }
   return OMX_ErrorNone;
}

OMX_ERRORTYPE FillBufferDone (OMX_HANDLETYPE hComponent,
                              OMX_PTR ptr,
                              OMX_BUFFERHEADERTYPE* pBuffer)
{

  OMX_ERRORTYPE error = OMX_ErrorNone;
  appPrivateSt* appPrvt = (appPrivateSt*)ptr;
  static long bytes_wrote = 0;
  if(!appPrvt->done_flag){
    if(appPrvt->mode==FILE_MODE){
        bytes_wrote += fwrite(pBuffer->pBuffer,
                              sizeof(OMX_U32),
                              pBuffer->nFilledLen/sizeof(OMX_U32),
                              outfile);
                              //pnothing);
    }
    else if(appPrvt->mode==ALSA_MODE){
      alsa_pcm_write(appPrvt, pBuffer);
    }
    /*APP_DPRINT("Send OUT buffer %p %ld %x\n",pBuffer->pBuffer,
               pBuffer->nFilledLen,
               (unsigned int)pBuffer->nFlags);*/
    /*APP_DPRINT("TS %lld TC %ld\n",pBuffer->nTimeStamp,pBuffer->nTickCount);*/
    error = OMX_FillThisBuffer(hComponent,pBuffer);
    if(error != OMX_ErrorNone){
      APP_DPRINT("Warning: buffer (%p) dropped!\n",pBuffer);
    }
  }else{
    if(appPrvt->mode==FILE_MODE){
        APP_DPRINT(" %ld Bytes wrote\n",sizeof(OMX_U32)*(bytes_wrote - 1));
    }
  }

  return error;
}

OMX_ERRORTYPE EmptyBufferDone(OMX_HANDLETYPE hComponent,
                              OMX_PTR ptr,
                              OMX_BUFFERHEADERTYPE* pBuffer)
{
  OMX_ERRORTYPE error = OMX_ErrorNone;
  appPrivateSt* appPrvt = (appPrivateSt*)ptr;

  error = send_input_buffer(appPrvt,pBuffer);
  if(error != OMX_ErrorNone){
    APP_DPRINT("Error from send_input_buffer: %d\n",error);
  }

  return error;
}

int allocate_buffer(appPrivateSt* appPrvt){

  OMX_ERRORTYPE error = OMX_ErrorNone;
  int i;

  for (i=0; i < appPrvt->in_port->nBufferCountActual; i++){
    error = OMX_AllocateBuffer (appPrvt->phandle,
                                &appPrvt->in_buffers[i],
                                OMX_DirInput,
                                appPrvt,
                                appPrvt->in_port->nBufferSize);
    if(error != OMX_ErrorNone){
      return 1;
    }
    APP_DPRINT("allocate IN buffers %p\n",appPrvt->in_buffers[i]);
  }
  if((appPrvt->mode==FILE_MODE) || (appPrvt->mode==ALSA_MODE) ){
    for (i=0; i < appPrvt->out_port->nBufferCountActual; i++){
      error = OMX_AllocateBuffer (appPrvt->phandle,
                                  &appPrvt->out_buffers[i],
                                  OMX_DirOutput,
                                  appPrvt,
                                  appPrvt->out_port->nBufferSize);
      if(error != OMX_ErrorNone){
        return 1;
      }
      APP_DPRINT("allocate OUT buffers %p\n",appPrvt->out_buffers[i]);
    }
  }
  return 0;
}

int use_buffer(appPrivateSt *appPrvt){

  OMX_ERRORTYPE error = OMX_ErrorNone;
  int i;

  for (i=0; i < appPrvt->in_port->nBufferCountActual; i++){
    appPrvt->in_buffers[i] = malloc(sizeof(appPrvt->in_port->nBufferSize) + EXTRA_BYTES);
    if(!appPrvt->in_buffers[i]){
      perror("malloc-UseBUffer");
    }
    appPrvt->in_buffers[i] += CACHE_ALIGNMENT;

    APP_DPRINT("use IN buffers %p\n",&appPrvt->in_buffers[i]);
    error = OMX_UseBuffer (appPrvt->phandle,
                           &appPrvt->in_buffers[i],
                           OMX_DirInput,
                           appPrvt,
                           appPrvt->in_port->nBufferSize,
                           NULL); //Setting it to null temporary until needed
    if(error != OMX_ErrorNone){
      return 1;
    }
  }
  if((appPrvt->mode==FILE_MODE) || (appPrvt->mode==ALSA_MODE)){
    for (i=0; i < appPrvt->out_port->nBufferCountActual; i++){
      appPrvt->out_buffers[i] = malloc(sizeof(appPrvt->out_port->nBufferSize) +
                                          EXTRA_BYTES);
      if(!appPrvt->out_buffers[i]){
        perror("malloc-UseBuffer");
      }
      appPrvt->out_buffers[i] += CACHE_ALIGNMENT;
      APP_DPRINT("use OUT buffers %p\n",&appPrvt->out_buffers[i]);
      error = OMX_UseBuffer (appPrvt->phandle,
                             &appPrvt->out_buffers[i],
                             OMX_DirOutput,
                             appPrvt,
                             appPrvt->out_port->nBufferSize,
                             NULL);//Setting it to null temporary until needed
      if(error != OMX_ErrorNone){
        return 1;
      }
    }
  }
  return 0;
}

/** Call to OMX_EmptyThisBuffer
 *
 * @param handle Handle to the component.
 * @param buffer IN buffer header pointer
 *
 */
int send_input_buffer(appPrivateSt* appPrvt,OMX_BUFFERHEADERTYPE *buffer){

  OMX_ERRORTYPE error = OMX_ErrorNone;
  static OMX_BOOL eos_flag = OMX_FALSE;
  int nread;
  static int drop_count = 0;
  static long file_size = 0;
  static OMX_BOOL first_buff= OMX_TRUE;
  OMX_U8 temp;
  if(!eos_flag){
      switch(appPrvt->in_port->format.audio.eEncoding){
      case OMX_AUDIO_CodingAMR:
          if(!appPrvt->amr_mode){
              nread = process_nbamr(appPrvt,buffer->pBuffer);
          }else{
              nread = process_wbamr(appPrvt,buffer->pBuffer);
          }
          break;
      case OMX_AUDIO_CodingG729:
          nread = process_g729(appPrvt,buffer->pBuffer);
          break;
      case OMX_AUDIO_CodingWMA:
          if(first_buff){
              buffer->nFlags = OMX_BUFFERFLAG_CODECCONFIG;
              first_buff =  OMX_FALSE;
          }
          else
              buffer->nFlags= 0;
          nread = process_wma(appPrvt,buffer->pBuffer);
          break;
      default:
          nread = fread(buffer->pBuffer,
                        1,
                        buffer->nAllocLen,
                        infile);
          file_size += nread;
          break;
      }
      buffer->nFilledLen = nread;
      buffer->nFlags |= 0;
      if(nread <= 0 ) {
          /*set the buffer flag*/
          buffer->nFlags |= OMX_BUFFERFLAG_EOS;
          eos_flag = OMX_TRUE;
          APP_DPRINT("EOS marked!\n");
          APP_DPRINT("End of file reached (%ld)!\n",file_size);
	  if(appPrvt->in_port->format.audio.eEncoding == OMX_AUDIO_CodingAMR){
	    if(!appPrvt->amr_mode){
	      appPrvt->done_flag = OMX_TRUE;
	      event_wakeup(appPrvt->eos);
	      return 0;
	    }
	  }
      }
      appPrvt->processed_buffers++;
      if((appPrvt->tc == 2) && (appPrvt->processed_buffers == 50)){
          event_wakeup(appPrvt->pause);
          return 0;
      }
      /*APP_DPRINT("Send %ld IN buffer %p %ld %x\n",appPrvt->processed_buffers,
        buffer->pBuffer,
        buffer->nFilledLen,
        (unsigned int)buffer->nFlags);*/

      buffer->nTimeStamp = rand() % 100;
      buffer->nTickCount = rand() % 70;
      error = OMX_EmptyThisBuffer(appPrvt->phandle,buffer);
      if(error != OMX_ErrorNone){
          APP_DPRINT("Error on EmptyThisBuffer\n");
          return 1;
      }
  }else{
    drop_count++;
    if(drop_count == appPrvt->nIpBuf){
      /*APP_DPRINT("Resetting eos_flag\n");*/
      drop_count = 0;
      eos_flag = OMX_FALSE;
    }
  }
  return 0;
}

/** Configure pcm output params
 *
 * @param appPrvt appPrvt Handle to the app component structure.
 *
 */
int config_pcm(appPrivateSt* appPrvt){

  OMX_ERRORTYPE error = OMX_ErrorNone;

  OMX_INIT_STRUCT(appPrvt->pcm,OMX_AUDIO_PARAM_PCMMODETYPE);

  APP_DPRINT("call get_parameter for OMX_IndexParamAudioPcm\n");
  appPrvt->pcm->nPortIndex = OMX_DirOutput;
  error = OMX_GetParameter (appPrvt->phandle,
                            OMX_IndexParamAudioPcm,
                            appPrvt->pcm);
  if (error != OMX_ErrorNone) {
    APP_DPRINT("OMX_ErrorBadParameter in SetParameter\n");
    return 1;
  }

  appPrvt->pcm->nSize = sizeof(OMX_AUDIO_PARAM_PCMMODETYPE);
  appPrvt->pcm->nSamplingRate = appPrvt->samplerate;
  appPrvt->pcm->nChannels = appPrvt->channels;
  /* TODO configure other parameters */
#if 0 /*set this to test 24-bit or non-inteleaved modes*/
  appPrvt->pcm->bInterleaved = OMX_FALSE; /*false => non-interleaved mode */
  appPrvt->pcm->nBitPerSample = 24;
#endif


  APP_DPRINT("call set_parameter for OMX_IndexParamAudioPcm\n");
  error = OMX_SetParameter (appPrvt->phandle,
                            OMX_IndexParamAudioPcm,
                            appPrvt->pcm);
  if (error != OMX_ErrorNone) {
    APP_DPRINT("OMX_ErrorBadParameter in SetParameter\n");
    return 1;
  }

  return 0;
}

/** test_play: Plays till end of file
 *
 * @param appPrvt Handle to the app component structure.
 *
 */
int test_play(appPrivateSt *appPrvt){

  int i;
  OMX_ERRORTYPE error = OMX_ErrorNone;

  APP_DPRINT("Change state to Executing\n");
  error = OMX_SendCommand(appPrvt->phandle,
                          OMX_CommandStateSet,
                          OMX_StateExecuting,
                          NULL);
  if(error != OMX_ErrorNone){
    return 1;
  }
  error = WaitForState(appPrvt->phandle,
                       OMX_StateExecuting,
                       appPrvt->state);
  if(error != OMX_ErrorNone){
    return 1;
  }

  for (i=0; i < appPrvt->out_port->nBufferCountActual; i++) {
    APP_DPRINT( "Send OUT buffer %p\n",appPrvt->out_buffers[i]);
    error = OMX_FillThisBuffer(appPrvt->phandle,appPrvt->out_buffers[i]);
    if(error != OMX_ErrorNone) {
      APP_DPRINT("Warning: buffer (%p) dropped!\n",appPrvt->in_buffers[i]);
    }
  }
  for (i=0; i < appPrvt->in_port->nBufferCountActual; i++) {
    APP_DPRINT( "Send IN buffer %p\n",appPrvt->in_buffers[i]);
    error = send_input_buffer(appPrvt,appPrvt->in_buffers[i]);
    if(error) {
      APP_DPRINT("Warning: buffer (%p) not sent!\n",appPrvt->in_buffers[i]);
    }
  }

  /* Now wait for EOS.... */
  APP_DPRINT("Now wait for EOS to finish....\n");
  event_block(appPrvt->eos);

  return 0;
}

/** test_repeat: Multiple repetition till end of file
 *
 * @param appPrvt Handle to the app component structure.
 *
 */
int test_repeat(appPrivateSt *appPrvt){

  int i;
  int error = 0;

  for(i = 0;i < appPrvt->iterations;i++){
    sleep(1);
    APP_DPRINT("********PLAY FOR %d TIME********\n",(i+1));

    if((error = test_play(appPrvt))){
      APP_DPRINT("FAILED!!\n");
      return 1;
    }


    APP_DPRINT("Change state to Idle\n");
    error = OMX_SendCommand(appPrvt->phandle,
                            OMX_CommandStateSet,
                            OMX_StateIdle,
                            NULL);
    if(error != OMX_ErrorNone){
      return 1;
    }
    error = WaitForState(appPrvt->phandle,
                         OMX_StateIdle,
                         appPrvt->state);
    if(error != OMX_ErrorNone){
      return 1;
    }

    appPrvt->done_flag = OMX_FALSE;
    rewind(infile);

  }

  APP_DPRINT("Change state to Pause\n");
  error = OMX_SendCommand(appPrvt->phandle,
                          OMX_CommandStateSet,
                          OMX_StatePause,
                          NULL);
  if(error != OMX_ErrorNone){
    return 1;
  }
  error = WaitForState(appPrvt->phandle,
                       OMX_StatePause,
                       appPrvt->state);
  if(error != OMX_ErrorNone){
    return 1;
  }

  return 0;
}

/** test_pause_resume: Pause-resume
 *
 * @param appPrvt Handle to the app component structure.
 *
 */
int test_pause_resume(appPrivateSt *appPrvt){

  int i;
  int error = 0;

  sleep(1);
  APP_DPRINT("Change state to Executing\n");
  error = OMX_SendCommand(appPrvt->phandle,
                          OMX_CommandStateSet,
                          OMX_StateExecuting,
                          NULL);
  if(error != OMX_ErrorNone){
    return 1;
  }
  error = WaitForState(appPrvt->phandle,
                       OMX_StateExecuting,
                       appPrvt->state);
  if(error != OMX_ErrorNone){
    return 1;
  }

  for (i=0; i < appPrvt->out_port->nBufferCountActual; i++) {
    APP_DPRINT( "Send OUT buffer %p\n",appPrvt->out_buffers[i]);
    error = OMX_FillThisBuffer(appPrvt->phandle,appPrvt->out_buffers[i]);
    if(error != OMX_ErrorNone) {
      APP_DPRINT("Warning: buffer (%p) dropped!\n",appPrvt->in_buffers[i]);
    }
  }
  for (i=0; i < appPrvt->in_port->nBufferCountActual; i++) {
    APP_DPRINT( "Send IN buffer %p\n",appPrvt->in_buffers[i]);
    error = send_input_buffer(appPrvt,appPrvt->in_buffers[i]);
    if(error) {
      APP_DPRINT("Warning: buffer (%p) not sent!\n",appPrvt->in_buffers[i]);
    }
  }

  /* Now wait for pause.... */
  APP_DPRINT("Process some buffers....\n");
  event_block(appPrvt->pause);

  APP_DPRINT("Flush ports\n");
  error = OMX_SendCommand(appPrvt->phandle,
                          OMX_CommandFlush,
                          OMX_ALL,
                          NULL);
  if(error != OMX_ErrorNone){
      return 1;
  }
  APP_DPRINT("Change state to Pause\n");
  error = OMX_SendCommand(appPrvt->phandle,
                          OMX_CommandStateSet,
                          OMX_StatePause,
                          NULL);
  if(error != OMX_ErrorNone){
      return 1;
  }
  error = WaitForState(appPrvt->phandle,
                       OMX_StatePause,
                       appPrvt->state);
  if(error != OMX_ErrorNone){
      return 1;
  }

  sleep(3);

  APP_DPRINT("Resume Playback\n");
  error = OMX_SendCommand(appPrvt->phandle,
                          OMX_CommandStateSet,
                          OMX_StateExecuting,
                          NULL);
  if(error != OMX_ErrorNone){
    return 1;
  }
  error = WaitForState(appPrvt->phandle,
                       OMX_StateExecuting,
                       appPrvt->state);
  if(error != OMX_ErrorNone){
    return 1;
  }

  for (i=0; i < appPrvt->out_port->nBufferCountActual; i++) {
    APP_DPRINT( "Send OUT buffer %p\n",appPrvt->out_buffers[i]);
    error = OMX_FillThisBuffer(appPrvt->phandle,appPrvt->out_buffers[i]);
    if(error != OMX_ErrorNone) {
      APP_DPRINT("Warning: buffer (%p) dropped!\n",appPrvt->in_buffers[i]);
    }
  }
  for (i=0; i < appPrvt->in_port->nBufferCountActual; i++) {
    APP_DPRINT( "Send IN buffer %p\n",appPrvt->in_buffers[i]);
    error = send_input_buffer(appPrvt,appPrvt->in_buffers[i]);
    if(error) {
      APP_DPRINT("Warning: buffer (%p) not sent!\n",appPrvt->in_buffers[i]);
    }
  }

  /* Now wait for EOS.... */
  APP_DPRINT("Now wait for EOS to finish....\n");
  event_block(appPrvt->eos);

  return 0;
}

void alsa_setAudioParams(appPrivateSt *appPrvt) {
  int err;
  snd_pcm_uframes_t period_frames = 100;
  if(appPrvt->Device == 1)
      appPrvt->alsaPrvt->device = "plughw:0,3";            /* HDMI device */
  else
      appPrvt->alsaPrvt->device = "default";            /* playback device */
  appPrvt->alsaPrvt->playback_handle = NULL;
  appPrvt->alsaPrvt->hw_params = NULL;
  appPrvt->alsaPrvt->sw_params = NULL;
  appPrvt->alsaPrvt->format = SND_PCM_FORMAT_S16_LE ;   /* sample format */
  appPrvt->alsaPrvt->frames = 0;


  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&appPrvt->alsaPrvt->hw_params);
  /* Open PCM. The last parameter of this function is the mode. */
  if ((err = snd_pcm_open(&appPrvt->alsaPrvt->playback_handle, appPrvt->alsaPrvt->device,
                          SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    printf("Playback open error: %s\n", snd_strerror(err));
    exit(1);
  }
  /* Fill it in with default values. */
  snd_pcm_hw_params_any(appPrvt->alsaPrvt->playback_handle,
                        appPrvt->alsaPrvt->hw_params);
  /* Set the desired hardware parameters. */
  /* Interleaved mode */
  snd_pcm_hw_params_set_access(appPrvt->alsaPrvt->playback_handle,
                               appPrvt->alsaPrvt->hw_params,
                               SND_PCM_ACCESS_RW_INTERLEAVED);
  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(appPrvt->alsaPrvt->playback_handle,
                               appPrvt->alsaPrvt->hw_params,
                               SND_PCM_FORMAT_S16_LE);
  /* 1 channel for mono, 2 channels for stereo */
  snd_pcm_hw_params_set_channels(appPrvt->alsaPrvt->playback_handle,
                                 appPrvt->alsaPrvt->hw_params,
                                 appPrvt->channels);

  /* Sampling rate in bits/second */
  snd_pcm_hw_params_set_rate_near(appPrvt->alsaPrvt->playback_handle,
                                  appPrvt->alsaPrvt->hw_params,
                                  (unsigned int*)&appPrvt->samplerate,
                                  0);

  /*Setting value for period size to avoid underruns*/
  snd_pcm_hw_params_set_period_size_near(appPrvt->alsaPrvt->playback_handle,
                                        appPrvt->alsaPrvt->hw_params,
                                        &period_frames, 0);

  /* Write the parameters to the driver */
  if ((err = snd_pcm_hw_params (appPrvt->alsaPrvt->playback_handle,
                                appPrvt->alsaPrvt->hw_params)) < 0) {
    fprintf (stderr, "cannot set hwparameters (%s)\n", snd_strerror (err));
    exit (1);
  }

  /*Leaving this part for now as could help when we get ALSA working*/
#if 0
  /* Configure sw pcm params */
  if ((err = snd_pcm_sw_params_malloc(&appPrvt->alsaPrvt->sw_params)) < 0) {
    fprintf (stderr, "cannot allocate software parameters structure (%s)\n",
             snd_strerror (err));
    exit (1);
  }
  /* Get current sw configuration for a PCM */
  if ((err = snd_pcm_sw_params_current (appPrvt->alsaPrvt->playback_handle,
                                        appPrvt->alsaPrvt->sw_params)) < 0) {
    fprintf (stderr, "cannot initialize software parameters structure (%s)\n",
             snd_strerror (err));
    exit (1);
  }
  /* Set min avail frames to consider PCM ready */
  if ((err = snd_pcm_sw_params_set_avail_min (appPrvt->alsaPrvt->playback_handle,
                                              appPrvt->alsaPrvt->sw_params,
                                              1024)) < 0) {
    fprintf (stderr, "cannot set minimum available count (%s)\n",
             snd_strerror (err));
    exit (1);
  }
  /* PCM is automatically started when playback frames available
     to PCM are >= threshold */
  if ((err = snd_pcm_sw_params_set_start_threshold (appPrvt->alsaPrvt->playback_handle,
                                                    appPrvt->alsaPrvt->sw_params,
                                                    0)) < 0) {
    fprintf (stderr, "cannot set start mode (%s)\n",
             snd_strerror (err));
    exit (1);
  }
  /* Install PCM sw configuration defined */
  if ((err = snd_pcm_sw_params (appPrvt->alsaPrvt->playback_handle,
                                appPrvt->alsaPrvt->sw_params)) < 0) {
    fprintf (stderr, "cannot set software parameters (%s)\n",
             snd_strerror (err));
    exit (1);
  }
#endif
  snd_pcm_prepare(appPrvt->alsaPrvt->playback_handle);

}

void alsa_pcm_write(appPrivateSt *appPrvt, OMX_BUFFERHEADERTYPE* pBuffer)
{
  int err;
  int frameSize;
  int totalBuffer;

  frameSize = (appPrvt->pcm->nChannels * appPrvt->pcm->nBitPerSample) >> 3;

  if(pBuffer->nFilledLen < frameSize){
    APP_DPRINT("Data not enough! %ld\n",pBuffer->nFilledLen);
    return;
  }

  totalBuffer = pBuffer->nFilledLen/frameSize;

  err = snd_pcm_writei(appPrvt->alsaPrvt->playback_handle,
                       pBuffer->pBuffer, totalBuffer);
  /*err = snd_pcm_writei(appPrvt->alsaPrvt->playback_handle,
                       pBuffer->pBuffer, pBuffer->nFilledLen/(2*appPrvt->pcm->nChannels));*/
  if (err == -EPIPE) {
    APP_DPRINT("UNDERRUN\n");
    snd_pcm_prepare(appPrvt->alsaPrvt->playback_handle);
  } else if (err < 0) {
    APP_DPRINT("Error from writei: %s\n", snd_strerror(err));
    exit(1);
  }  else if (err != (int)(pBuffer->nFilledLen/(2*appPrvt->pcm->nChannels))) {
    /*APP_DPRINT("Short write, write %d frames\n", err);*/
  }
}
