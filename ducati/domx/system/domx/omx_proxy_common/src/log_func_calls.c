#include "log_func_calls.h"
#define sd(format, ...) sprintf(buf)

/// prints integer
#define SPFint(Fld) fprintf (outBuff, #Fld ":: %d\n\t\t",(int) (Fld));

/// prints HEX
#define SPFhex(Fld) fprintf (outBuff, #Fld ":: %x\n\t\t",(int) (Fld));

/// prints pointer
#define SPFptr(Fld) fprintf (outBuff, #Fld ":: %p\n\t\t",(Fld));

/// prints structure member in hexadecimal format
#define SPFmemHex(Fld) fprintf (outBuff, #Fld ":: 0x%x\n\t\t",(ptr->Fld));

/// prints structure member in decimal format
#define SPFmemDec(Fld) fprintf (outBuff, #Fld ":: %d\n\t\t",(ptr->Fld));

/// prints any string - usefull for variable names etc.
#define SPFname(name) fprintf (outBuff, #name ": ");

#if 0 /// use these in the cource file, you'll trace

static funcLog_t Log;

maskToFunc_t omxProxyFuncLogLUT[] = {
  { 1 << 0, "" },
  { 1 << 1, "" },
  { 1 << 3, "" },
  { 1 << 4, "" },
  { 1 << 5, "" },
  { 1 << 6, "" },
  { 1 << 7, "" },
  { 1 << 8, "" },
  { 1 << 9, "" },
  { 1 << 10, "" },
  { 1 << 12, "" },
  { 1 << 13, "" },
  { 1 << 14, "" },
  { 1 << 15, "" },
  { 1 << 16, "" },
  { 1 << 17, "" },
  { 1 << 18, "" },
  { 1 << 19, "" },
  { 1 << 20, "" },
  { 1 << 22, "" },
  { 1 << 23, "" },
  { 1 << 24, "" },
  { 1 << 25, "" },
  { 1 << 26, "" },
  { 1 << 27, "" },
  { 1 << 28, "" },
  { 1 << 29, "" },
  { 1 << 30, "" },
  { 1 << 31, "" },
  { 1 << 32, "" },
};

#endif

void log_init(void* handle, funcLog_t* L, maskToFunc_t* lut)
{

    int i;
    L->handle = handle;
    L->funcLUT = lut;
    char filename[32];
    filename[0] = 0;
    /// create LOG filename
    sprintf(filename, "%s.log", L->fileName);

    /// open the log file
    L->file = fopen(filename, "a");

    /// register enabled logs
//     for( i = 0; L->funcLUT[i].funcName ; i++ )
//         L->enabledFuncMask |= lut[i].mask;
}

void log_RegisterEnterFn( void * handle,
                     funcLog_t* L,
                     unsigned int mask ,
                     int fn,
                     int param_0,
                     unsigned int param_1,
                     int param_2,
                     int param_3,
                     int param_4
                  )
{
  int i = 0;

  if( handle == L->handle && (mask & L->enabledFuncMask))
  {
      for( i = 0; i < 32; i ++)
          if( mask & (1 << i) )
              fprintf( L->file, "\n%s [enter]:\n\t", L->funcLUT[i].funcName );

          dumpToBuffer( param_0, (void*)param_1, (void*)param_2, (void*)param_3, L->file, fn);

  }
}

void log_RegisterExitFn( void * handle, funcLog_t* L, unsigned int mask )
{
  int i;
  if( handle == L->handle && (mask & L->enabledFuncMask))
      for( i = 0; i < 32; i ++)
          if( mask & (1 << i) )
              fprintf( L->file, "\n%s [exit]:\n\t", L->funcLUT[i].funcName );
}

void log_deinit(void* handle,  funcLog_t* L)
{
    if( handle == L->handle )
        L->handle = NULL;

    fclose(L->file);
}

void dumpToBuffer(OMX_U32 iIndex, void* pStructure, void* pOther, void* pOther2, FILE* outBuff, int fn)
{

        int rCount=0,i;

        if (fn == 13){
          switch(iIndex){
              case OMX_CommandStateSet:
              {
                      OMX_STATETYPE *st = (OMX_STATETYPE*)pStructure;
                      SPFname(OMX_CommandStateSet);
                      SPFint (st)
                      break;
              }
              case OMX_CommandFlush:
              {
                      SPFname(OMX_CommandFlush);
                      break;
              }
              case OMX_CommandPortDisable:
              {
                      int *st = (int*)pStructure;
                      SPFname(OMX_CommandPortDisable)
                      SPFint (st)
                      break;
              }
              case OMX_CommandPortEnable:
              {
                      int *st = (int*)pStructure;
                      SPFname(OMX_CommandPortEnable)
                      SPFint (st)
                      break;
              }
              case OMX_CommandMarkBuffer:
              {
                      OMX_MARKTYPE * ptr = (OMX_MARKTYPE *)pOther;
                      SPFname(OMX_CommandPortDisable)
                      SPFmemHex(hMarkTargetComponent)
                      SPFmemHex(pMarkData)
                      break;
              }
              default:
              {
                      fprintf(outBuff, "Unrecognized Command: %x", iIndex);
              }
          }
        }else if (fn == 0) {
          switch(iIndex){
            case OMX_EventCmdComplete:{
              SPFname(OMX_EventCmdComplete)
              break;
            }
            case OMX_EventError: {
              SPFname(OMX_EventError)
              break;
            }
            case OMX_EventMark: {
              SPFname(OMX_EventMark)
              break;
            }
            case OMX_EventPortSettingsChanged: {
              SPFname(OMX_EventPortSettingsChanged)
              break;
            }
            case OMX_EventBufferFlag: {
              SPFname(OMX_EventBufferFlag)
              break;
            }
            case OMX_EventResourcesAcquired: {
              SPFname(OMX_EventResourcesAcquired)
              break;
            }
            case OMX_EventComponentResumed: {
              SPFname(OMX_EventComponentResumed)
              break;
            }
            case OMX_EventDynamicResourcesAvailable: {
              SPFname(OMX_EventDynamicResourcesAvailable)
              break;
            }
            case OMX_EventPortFormatDetected: {
              SPFname(OMX_EventPortFormatDetected)
              break;
            }
          }
        }else if (fn == 2) {
          OMX_U32 remoteBufHdr = iIndex;
          OMX_U32 nOffset = (OMX_U32)pStructure;
          OMX_U32 nFlags =  (OMX_U32)pOther;
          OMX_PTR pMarkData = (OMX_PTR)pOther2;

          SPFint(remoteBufHdr)
          SPFint(nOffset)
          SPFint(nFlags)
          SPFhex(pMarkData)

        }else{
          switch(iIndex) {

              case OMX_IndexCameraOperatingMode: {
                      OMX_CONFIG_CAMOPERATINGMODETYPE *ptr = (OMX_CONFIG_CAMOPERATINGMODETYPE *)pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemHex (eCamOperatingMode);
                      break;

              }
              case OMX_IndexConfigCaptureMode: {
                      OMX_CONFIG_CAPTUREMODETYPE *ptr = (OMX_CONFIG_CAPTUREMODETYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (bContinuous);
                      SPFmemDec (bFrameLimited);
                      SPFmemDec (nFrameLimit);
                      break;
              }

              case OMX_IndexConfigCapturing: {
                      OMX_CONFIG_BOOLEANTYPE *ptr = (OMX_CONFIG_BOOLEANTYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (bEnabled);
                      break;
              }

              case OMX_IndexConfigChromaticAberrationCorrection: {
                      OMX_CONFIG_BOOLEANTYPE *ptr = (OMX_CONFIG_BOOLEANTYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (bEnabled);
                      break;
              }

              case OMX_IndexConfigCommonBrightness: {
                      OMX_CONFIG_BRIGHTNESSTYPE *ptr = (OMX_CONFIG_BRIGHTNESSTYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (nBrightness);
                      break;
              }

              case OMX_IndexConfigCommonContrast: {
                      OMX_CONFIG_CONTRASTTYPE *ptr = (OMX_CONFIG_CONTRASTTYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (nContrast);
                      break;
              }

              case OMX_IndexConfigCommonDigitalZoom: {
                      OMX_CONFIG_SCALEFACTORTYPE *ptr = (OMX_CONFIG_SCALEFACTORTYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (xWidth);
                      SPFmemDec (xHeight);
                      break;
              }

              case OMX_IndexConfigCommonExposure: {
                      OMX_CONFIG_EXPOSURECONTROLTYPE *ptr = (OMX_CONFIG_EXPOSURECONTROLTYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemHex (eExposureControl);
                      break;
              }

              case OMX_IndexConfigCommonExposureValue: {
                      OMX_CONFIG_EXPOSUREVALUETYPE *ptr = (OMX_CONFIG_EXPOSUREVALUETYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemHex (eMetering);
                      SPFmemDec (xEVCompensation);
                      SPFmemDec (nApertureFNumber);
                      SPFmemDec (bAutoAperture);
                      SPFmemDec (nShutterSpeedMsec);
                      SPFmemDec (bAutoShutterSpeed);
                      SPFmemDec (nSensitivity);
                      SPFmemDec (bAutoSensitivity);
                      break;
              }

              case OMX_IndexConfigCommonFocusStatus: {
                      OMX_PARAM_FOCUSSTATUSTYPE *ptr = (OMX_PARAM_FOCUSSTATUSTYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemHex (eFocusStatus);
                      SPFmemDec (bCenterStatus);
                      SPFmemDec (bLeftStatus);
                      SPFmemDec (bRightStatus);
                      SPFmemDec (bTopStatus);
                      SPFmemDec (bBottomStatus);
                      SPFmemDec (bTopLeftStatus);
                      SPFmemDec (bTopRightStatus);
                      SPFmemDec (bBottomLeftStatus);
                      SPFmemDec (bBottomRightStatus);
                      break;
              }

              case OMX_IndexConfigCommonFrameStabilisation: {
                      OMX_CONFIG_FRAMESTABTYPE *ptr = (OMX_CONFIG_FRAMESTABTYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (bStab);
                      break;
              }

              case OMX_IndexConfigCommonImageFilter: {
                      OMX_CONFIG_IMAGEFILTERTYPE *ptr = (OMX_CONFIG_IMAGEFILTERTYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemHex (eImageFilter);
                      break;
              }

              case OMX_IndexConfigCommonRotate: {
                      OMX_CONFIG_ROTATIONTYPE *ptr = (OMX_CONFIG_ROTATIONTYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (nRotation);
                      break;
              }

              case OMX_IndexConfigCommonSaturation: {
                      OMX_CONFIG_SATURATIONTYPE *ptr = (OMX_CONFIG_SATURATIONTYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (nSaturation);
                      break;
              }

              case OMX_IndexConfigCommonWhiteBalance: {
                      OMX_CONFIG_WHITEBALCONTROLTYPE *ptr = (OMX_CONFIG_WHITEBALCONTROLTYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemHex (eWhiteBalControl);
                      break;
              }

              case OMX_IndexConfigExtCaptureMode: {
                      OMX_CONFIG_EXTCAPTUREMODETYPE *ptr = (OMX_CONFIG_EXTCAPTUREMODETYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (nFrameRate);
                      SPFmemDec (nFrameBefore);
                      SPFmemDec (bPrepareCapture);
                      SPFmemDec (bEnableBracketing);
                      SPFmemHex (tBracketConfigType.eBracketMode);
                      SPFmemDec (tBracketConfigType.nNbrBracketingValues);
                      for (i=0; i < ptr->tBracketConfigType.nNbrBracketingValues; ++i)
                              SPFmemDec (tBracketConfigType.nBracketValues[i]);
                      break;
              }

              case OMX_IndexConfigExtFocusRegion: {
                      OMX_CONFIG_EXTFOCUSREGIONTYPE *ptr = (OMX_CONFIG_EXTFOCUSREGIONTYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (nRefPortIndex);
                      SPFmemDec (nLeft);
                      SPFmemDec (nTop);
                      SPFmemDec (nWidth);
                      SPFmemHex (nHeight);
                      break;
              }

              case OMX_IndexConfigFlashControl: {
                      OMX_IMAGE_PARAM_FLASHCONTROLTYPE *ptr = (OMX_IMAGE_PARAM_FLASHCONTROLTYPE *)pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemHex (eFlashControl);
                      break;
              }

              case OMX_IndexConfigFlickerCancel: {
                      OMX_CONFIG_FLICKERCANCELTYPE *ptr = (OMX_CONFIG_FLICKERCANCELTYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemHex (eFlickerCancel);
                      break;
              }

              case OMX_IndexConfigFocusControl: {
                      OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE *ptr = (OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE *)pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemHex (eFocusControl);
                      SPFmemDec (nFocusSteps);
                      SPFmemDec (nFocusStepIndex);
                      break;
              }

              case OMX_IndexConfigImageFaceDetection: {
                      OMX_CONFIG_OBJDETECTIONTYPE *ptr = (OMX_CONFIG_OBJDETECTIONTYPE *) pStructure;
                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (bEnable);
                      SPFmemDec (bFrameLimited);
                      SPFmemDec (nFrameLimit);
                      SPFmemDec (nMaxNbrObjects);
                      SPFmemDec (nLeft);
                      SPFmemDec (nTop);
                      SPFmemDec (nWidth);
                      SPFmemDec (nHeight);
                      SPFmemHex (eObjDetectQuality);
                      SPFmemDec (nPriority);
                      break;
              }
              case OMX_IndexConfigOtherExtraDataControl : {

                      OMX_CONFIG_EXTRADATATYPE *ptr = (OMX_CONFIG_EXTRADATATYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemHex (eExtraDataType);
                      SPFmemHex (eCameraView);
                      SPFmemDec (bEnable);

                      break;
              }
              case OMX_IndexConfigSharpeningLevel : {

                      OMX_IMAGE_CONFIG_PROCESSINGLEVELTYPE *ptr = (OMX_IMAGE_CONFIG_PROCESSINGLEVELTYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (nLevel);
                      SPFmemDec (bAuto);

                      break;
              }
              case OMX_IndexParamFlashControl : {

                      OMX_IMAGE_PARAM_FLASHCONTROLTYPE *ptr = (OMX_IMAGE_PARAM_FLASHCONTROLTYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemHex (eFlashControl);

                      break;
              }
              case OMX_IndexParamFrameStabilisation : {

                      OMX_CONFIG_BOOLEANTYPE *ptr = (OMX_CONFIG_BOOLEANTYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (bEnabled);

                      break;
              }
              case OMX_IndexParamHighISONoiseFiler : {

                      OMX_CONFIG_BOOLEANTYPE *ptr = (OMX_CONFIG_BOOLEANTYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (bEnabled);

                      break;
              }
              case OMX_IndexParamLensDistortionCorrection : {

                      OMX_CONFIG_BOOLEANTYPE *ptr = (OMX_CONFIG_BOOLEANTYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (bEnabled);

                      break;
              }
              case OMX_IndexParamPortDefinition : {

                      OMX_PARAM_PORTDEFINITIONTYPE *ptr = (OMX_PARAM_PORTDEFINITIONTYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemHex (eDir);
                      SPFmemDec (nBufferCountActual);
                      SPFmemDec (nBufferCountMin);
                      SPFmemDec (nBufferSize);
                      SPFmemDec (bEnabled);
                      SPFmemDec (bPopulated);
                      SPFmemHex (eDomain);
                      SPFmemHex (format);
                      SPFmemDec (bBuffersContiguous);
                      SPFmemDec (nBufferAlignment);

                      break;
              }
              case OMX_IndexParamQFactor : {

                      OMX_IMAGE_PARAM_QFACTORTYPE *ptr = (OMX_IMAGE_PARAM_QFACTORTYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (nQFactor);

                      break;
              }
              case OMX_IndexParamThumbnail : {

                      OMX_PARAM_THUMBNAILTYPE *ptr = (OMX_PARAM_THUMBNAILTYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (nHeight);
                      SPFmemDec (nWidth);
                      SPFmemHex (eCompressionFormat);
                      SPFmemHex (eColorFormat);
                      SPFmemDec (nQuality);
                      SPFmemDec (nMaxSize);

                      break;
              }
              case OMX_IndexParamVideoNoiseFilter : {

                      OMX_PARAM_VIDEONOISEFILTERTYPE *ptr = (OMX_PARAM_VIDEONOISEFILTERTYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemHex (eMode);

                      break;
              }
              case OMX_TI_IndexConfigAutoConvergence : {

                      OMX_TI_CONFIG_CONVERGENCETYPE *ptr = (OMX_TI_CONFIG_CONVERGENCETYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemHex (eACMode);
                      SPFmemDec (nManualConverence);
                      SPFmemDec (nACProcWinStartX);
                      SPFmemDec (nACProcWinStartY);
                      SPFmemDec (nACProcWinWidth);
                      SPFmemDec (nACProcWinHeight);
                      SPFmemDec (bACStatus);

                      break;
              }
              case OMX_TI_IndexConfigExifTags : {

                      OMX_TI_CONFIG_SHAREDBUFFER *ptr = (OMX_TI_CONFIG_SHAREDBUFFER *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (nSharedBuffSize);
                      SPFmemHex (pSharedBuff);

                      break;
              }
              case OMX_TI_IndexConfigFacePriority3a : {

                      OMX_TI_CONFIG_3A_FACE_PRIORITY *ptr = (OMX_TI_CONFIG_3A_FACE_PRIORITY *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (bAwbFaceEnable);
                      SPFmemDec (bAeFaceEnable);
                      SPFmemDec (bAfFaceEnable);

                      break;
              }
              case OMX_TI_IndexConfigGlobalBrightnessContrastEnhance : {

                      OMX_TI_CONFIG_LOCAL_AND_GLOBAL_BRIGHTNESSCONTRASTTYPE *ptr = (OMX_TI_CONFIG_LOCAL_AND_GLOBAL_BRIGHTNESSCONTRASTTYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (eControl);

                      break;
              }
              case OMX_TI_IndexConfigLocalBrightnessContrastEnhance : {

                      OMX_TI_CONFIG_LOCAL_AND_GLOBAL_BRIGHTNESSCONTRASTTYPE *ptr = (OMX_TI_CONFIG_LOCAL_AND_GLOBAL_BRIGHTNESSCONTRASTTYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (eControl);

                      break;
              }
              case OMX_TI_IndexConfigRegionPriority3a : {

                      OMX_TI_CONFIG_3A_REGION_PRIORITY *ptr = (OMX_TI_CONFIG_3A_REGION_PRIORITY *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (bAwbRegionEnable);
                      SPFmemDec (bAeRegionEnable);
                      SPFmemDec (bAfRegionEnable);

                      break;
              }
              case OMX_TI_IndexConfigSceneMode : {

                      OMX_CONFIG_SCENEMODETYPE *ptr = (OMX_CONFIG_SCENEMODETYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemHex (eSceneMode);

                      break;
              }
              case OMX_TI_IndexConfigSensorSelect : {

                      OMX_CONFIG_SENSORSELECTTYPE *ptr = (OMX_CONFIG_SENSORSELECTTYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemHex (eSensor);

                      break;
              }
              case OMX_TI_IndexConfigVarFrmRange : {

                      OMX_TI_CONFIG_VARFRMRANGETYPE *ptr = (OMX_TI_CONFIG_VARFRMRANGETYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (xMin);
                      SPFmemDec (xMax);

                      break;
              }
              case OMX_TI_IndexParam2DBufferAllocDimension : {

                      OMX_CONFIG_RECTTYPE *ptr = (OMX_CONFIG_RECTTYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (nPortIndex);
                      SPFmemDec (nLeft);
                      SPFmemDec (nTop);
                      SPFmemDec (nWidth);
                      SPFmemDec (nHeight);

                      break;
              }
              case OMX_TI_IndexParamSensorOverClockMode:
              {

                      OMX_CONFIG_BOOLEANTYPE *ptr = (OMX_CONFIG_BOOLEANTYPE *)pStructure;

                      SPFmemDec (nSize);
                      SPFmemDec (nVersion);
                      SPFmemDec (bEnabled);

                      break;
              }
              case OMX_log_Event_Handling:
              {
                      break;
              }
              case OMX_log_Empty_Buffer:
              {
                      OMX_BUFFERHEADERTYPE *ptr = (OMX_BUFFERHEADERTYPE *)pStructure;
                      SPFmemDec(nSize)
                      SPFmemDec(nVersion)
                      SPFmemHex(pBuffer)
                      SPFmemDec(nAllocLen)
                      SPFmemDec(nFilledLen)
                      SPFmemDec(nOffset)
                      SPFmemHex(pAppPrivate)
                      SPFmemHex(pPlatformPrivate)
                      SPFmemHex(pInputPortPrivate)
                      SPFmemHex(pOutputPortPrivate)
                      SPFmemHex(pMarkData)
                      SPFmemDec(nTickCount)
                      SPFmemHex(hMarkTargetComponent)
                      SPFmemDec(pMarkData)
                      SPFmemDec(nFlags)
                      SPFmemDec(nOutputPortIndex)
                      SPFmemDec(nInputPortIndex)
                      break;
              }
              case OMX_log_Fill_Buffer:
              {
                      OMX_BUFFERHEADERTYPE *ptr = (OMX_BUFFERHEADERTYPE *)pStructure;
                      SPFmemDec(nSize)
                      SPFmemDec(nVersion)
                      SPFmemHex(pBuffer)
                      SPFmemDec(nAllocLen)
                      SPFmemDec(nFilledLen)
                      SPFmemDec(nOffset)
                      SPFmemHex(pAppPrivate)
                      SPFmemHex(pPlatformPrivate)
                      SPFmemHex(pInputPortPrivate)
                      SPFmemHex(pOutputPortPrivate)
                      SPFmemHex(pMarkData)
                      SPFmemDec(nTickCount)
                      SPFmemHex(hMarkTargetComponent)
                      SPFmemDec(pMarkData)
                      SPFmemDec(nFlags)
                      SPFmemDec(nOutputPortIndex)
                      SPFmemDec(nInputPortIndex)
                      break;
              }
              case OMX_log_Empty_This_Buff:
              {
                      OMX_BUFFERHEADERTYPE *ptr = (OMX_BUFFERHEADERTYPE *)pStructure;
                      SPFmemDec(nSize)
                      SPFmemDec(nVersion)
                      SPFmemHex(pBuffer)
                      SPFmemDec(nAllocLen)
                      SPFmemDec(nFilledLen)
                      SPFmemDec(nOffset)
                      SPFmemHex(pAppPrivate)
                      SPFmemHex(pPlatformPrivate)
                      SPFmemHex(pInputPortPrivate)
                      SPFmemHex(pOutputPortPrivate)
                      SPFmemHex(pMarkData)
                      SPFmemDec(nTickCount)
                      SPFmemHex(hMarkTargetComponent)
                      SPFmemDec(pMarkData)
                      SPFmemDec(nFlags)
                      SPFmemDec(nOutputPortIndex)
                      SPFmemDec(nInputPortIndex)
                      break;
              }
              case OMX_log_Fill_This_Buffer:
              {
                      OMX_BUFFERHEADERTYPE *ptr = (OMX_BUFFERHEADERTYPE *)pStructure;
                      SPFmemDec(nSize)
                      SPFmemDec(nVersion)
                      SPFmemHex(pBuffer)
                      SPFmemDec(nAllocLen)
                      SPFmemDec(nFilledLen)
                      SPFmemDec(nOffset)
                      SPFmemHex(pAppPrivate)
                      SPFmemHex(pPlatformPrivate)
                      SPFmemHex(pInputPortPrivate)
                      SPFmemHex(pOutputPortPrivate)
                      SPFmemHex(pMarkData)
                      SPFmemDec(nTickCount)
                      SPFmemHex(hMarkTargetComponent)
                      SPFmemDec(pMarkData)
                      SPFmemDec(nFlags)
                      SPFmemDec(nOutputPortIndex)
                      SPFmemDec(nInputPortIndex)
                      break;
              }
              case OMX_log_Allocate_Buffer:
              {
                      OMX_BUFFERHEADERTYPE *ptr = (OMX_BUFFERHEADERTYPE *)pStructure;
                      SPFmemDec(nSize)
                      SPFmemDec(nVersion)
                      SPFmemHex(pBuffer)
                      SPFmemDec(nAllocLen)
                      SPFmemDec(nFilledLen)
                      SPFmemDec(nOffset)
                      SPFmemHex(pAppPrivate)
                      SPFmemHex(pPlatformPrivate)
                      SPFmemHex(pInputPortPrivate)
                      SPFmemHex(pOutputPortPrivate)
                      SPFmemHex(pMarkData)
                      SPFmemDec(nTickCount)
                      SPFmemHex(hMarkTargetComponent)
                      SPFmemDec(pMarkData)
                      SPFmemDec(nFlags)
                      SPFmemDec(nOutputPortIndex)
                      SPFmemDec(nInputPortIndex)
                      break;
              }
              case OMX_log_Use_Buffer:
              {
                      OMX_BUFFERHEADERTYPE *ptr = (OMX_BUFFERHEADERTYPE *)pStructure;
                      SPFmemDec(nSize)
                      SPFmemDec(nVersion)
                      SPFmemHex(pBuffer)
                      SPFmemDec(nAllocLen)
                      SPFmemDec(nFilledLen)
                      SPFmemDec(nOffset)
                      SPFmemHex(pAppPrivate)
                      SPFmemHex(pPlatformPrivate)
                      SPFmemHex(pInputPortPrivate)
                      SPFmemHex(pOutputPortPrivate)
                      SPFmemHex(pMarkData)
                      SPFmemDec(nTickCount)
                      SPFmemHex(hMarkTargetComponent)
                      SPFmemDec(pMarkData)
                      SPFmemDec(nFlags)
                      SPFmemDec(nOutputPortIndex)
                      SPFmemDec(nInputPortIndex)
                      break;
              }
              case OMX_log_Free_Buffer:
              {
                      OMX_BUFFERHEADERTYPE *ptr = (OMX_BUFFERHEADERTYPE *)pStructure;
                      SPFmemDec(nSize)
                      SPFmemDec(nVersion)
                      SPFmemHex(pBuffer)
                      SPFmemDec(nAllocLen)
                      SPFmemDec(nFilledLen)
                      SPFmemDec(nOffset)
                      SPFmemHex(pAppPrivate)
                      SPFmemHex(pPlatformPrivate)
                      SPFmemHex(pInputPortPrivate)
                      SPFmemHex(pOutputPortPrivate)
                      SPFmemHex(pMarkData)
                      SPFmemDec(nTickCount)
                      SPFmemHex(hMarkTargetComponent)
                      SPFmemDec(pMarkData)
                      SPFmemDec(nFlags)
                      SPFmemDec(nOutputPortIndex)
                      SPFmemDec(nInputPortIndex)
                      break;
              }
              case OMX_log_Send_Command:
              {
                      break;
              }
              case OMX_log_Get_State:
              {
                      SPFint(*(OMX_STATETYPE*)pStructure);
                      break;
              }
              case OMX_log_Callback_set:
              {
                      break;
              }
              case OMX_log_Un_Map_Buffer_Ducati:
              {
                      break;
              }
              case OMX_log_Meta_Data_Host:
              {
                      break;
              }
              case OMX_log_Image_video_Lines:
              {
                      break;
              }
              default : {
                      rCount = fprintf(outBuff, "Unrecognized structure index: %x", iIndex);
                      break;
              }
          }
      }
  }
