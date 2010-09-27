/* =================================================================== *             Texas Instruments OMAP(TM) Platform Software
 * (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied.
 * ==================================================================== */

/* -------------------------------------------------------------------------- *
 *
 * @file:OMX_TI_IVCommon.h
 * This header defines the structures specific to the config indices of msp_VPPM.
 *
 * @path ..\OMAPSW_SysDev\multimedia\omx\khronos1_1\omx_core\inc
 *
 * -------------------------------------------------------------------------- */

/* ======================================================================== *!
 *! Revision History
 *! ==================================================================== */

#ifndef OMX_TI_IVCommon_H
#define OMX_TI_IVCommon_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <OMX_IVCommon.h>
#include <OMX_Image.h>



/*======================================================================= */
/* Enumerated values for operation mode for compressed image
 *
 * ENUMS:
 * Chunk         : Chunk based operation
 * NonChunk    : Non-chunk based operation
 */
 /* ======================================================================= */
typedef enum OMX_JPEG_COMPRESSEDMODETYPE {
    OMX_JPEG_ModeChunk = 0,
    OMX_JPEG_ModeNonChunk
}OMX_JPEG_COMPRESSEDMODETYPE ;


/*======================================================================= */
/* Enumerated values for operation mode for uncompressed image
 *
 * ENUMS:
 * Frame   :  Frame based operation
 * Slice   : Slice based operation
 * Stitch  : For stitching between image frames
 * Burst   :  For stitching between image frames
 */
 /* ======================================================================= */
typedef enum OMX_JPEG_UNCOMPRESSEDMODETYPE {
    OMX_JPEG_UncompressedModeFrame = 0,
    OMX_JPEG_UncompressedModeSlice,
    OMX_JPEG_UncompressedModeStitch,
    OMX_JPEG_UncompressedModeBurst
}OMX_JPEG_UNCOMPRESSEDMODETYPE;



/*======================================================================= */
/* Configuration structure for compressed image
 *
 * STRUCT MEMBERS:
 *  nSize                 : Size of the structure in bytes
 *  nVersion              : OMX specification version information
 *  nPortIndex            : Port that this structure applies to
 *  eCompressedImageMode  : Operating mode enumeration for compressed image
 */
 /*======================================================================= */
typedef struct OMX_JPEG_PARAM_COMPRESSEDMODETYPE{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_JPEG_COMPRESSEDMODETYPE eCompressedImageMode;
}OMX_JPEG_PARAM_COMPRESSEDMODETYPE;



/*======================================================================= */
/* Uncompressed image Operating mode configuration structure
 *
 * STRUCT MEMBERS:
 * nSize                     : Size of the structure in bytes
 * nVersion                  : OMX specification version information
 * nPortIndex                : Port that this structure applies to
 * nBurstLength              : No of frames to be dumped in burst mode
 * eUncompressedImageMode    : uncompressed image mode information
 * eSourceType               : Image encode souce info
 * tRotationInfo             : Rotation related information
 */
 /*======================================================================= */
typedef struct OMX_JPEG_PARAM_UNCOMPRESSEDMODETYPE{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nBurstLength;
    OMX_JPEG_UNCOMPRESSEDMODETYPE eUncompressedImageMode;
}OMX_JPEG_PARAM_UNCOMPRESSEDMODETYPE;


/*======================================================================= */
/* Subregion Decode Parameter configuration structure
 *
 * STRUCT MEMBERS:
 * nSize                     : Size of the structure in bytes
 * nVersion                  : OMX specification version information
 * nXOrg                     : Sectional decoding X origin
 * nYOrg                     : Sectional decoding Y origin
 * nXLength                  : Sectional decoding X length
 * nYLength                  : Sectional decoding Y length
 */
 /*======================================================================= */
typedef struct OMX_IMAGE_PARAM_DECODE_SUBREGION{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nXOrg;
    OMX_U32 nYOrg;
    OMX_U32 nXLength;
    OMX_U32 nYLength;
}OMX_IMAGE_PARAM_DECODE_SUBREGION;


/**
 * sensor select  types
 */
typedef enum OMX_SENSORSELECT{
        OMX_PrimarySensor = 0,
        OMX_SecondarySensor,
        OMX_SensorTypeMax = 0x7fffffff
}OMX_SENSORSELECT;

/**
 *
 * Sensor Select
 */
typedef  struct OMX_CONFIG_SENSORSELECTTYPE {
OMX_U32  nSize; /**< Size of the structure in bytes */
OMX_VERSIONTYPE nVersion; /**< OMX specification version info */
OMX_U32 nPortIndex; /**< Port that this struct applies to */
OMX_SENSORSELECT eSensor; /**< sensor select */
} OMX_CONFIG_SENSORSELECTTYPE;

/**
 * Flicker cancellation types
 */
typedef enum OMX_COMMONFLICKERCANCELTYPE{
        OMX_FlickerCancelOff = 0,
        OMX_FlickerCancelAuto,
        OMX_FlickerCancel50,
        OMX_FlickerCancel60,
        OMX_FlickerCancelMax = 0x7fffffff
}OMX_COMMONFLICKERCANCELTYPE;

typedef struct OMX_CONFIG_FLICKERCANCELTYPE {
   OMX_U32 nSize;
   OMX_VERSIONTYPE nVersion;
   OMX_U32 nPortIndex;
   OMX_COMMONFLICKERCANCELTYPE eFlickerCancel;
} OMX_CONFIG_FLICKERCANCELTYPE;


/**
 * Sensor caleberation types
 */
typedef enum OMX_SENSORCALTYPE{
        OMX_SensorCalFull = 0,
        OMX_SensorCalQuick,
        OMX_SensorCalMax = 0x7fffffff
}OMX_SENSORCALTYPE;

typedef struct OMX_CONFIG_SENSORCALTYPE {
   OMX_U32 nSize;
   OMX_VERSIONTYPE nVersion;
   OMX_U32 nPortIndex;
   OMX_SENSORCALTYPE eSensorCal;
} OMX_CONFIG_SENSORCALTYPE;

/**
 * Scene mode types
 */
typedef enum OMX_SCENEMODETYPE{

        OMX_Manual = 0,
        OMX_Closeup,
        OMX_Portrait,
        OMX_Landscape,
        OMX_Underwater,
        OMX_Sport,
        OMX_SnowBeach,
        OMX_Mood,
        OMX_NightPortrait,
        OMX_NightIndoor,
        OMX_Fireworks,
        OMX_Document, /**< for still image */
        OMX_Barcode, /**< for still image */
        OMX_SuperNight, /**< for video */
        OMX_Cine, /**< for video */
        OMX_OldFilm, /**< for video */

        OMX_SceneModeMax = 0x7fffffff
}OMX_SCENEMODETYPE;

typedef struct OMX_CONFIG_SCENEMODETYPE {
   OMX_U32 nSize;
   OMX_VERSIONTYPE nVersion;
   OMX_U32 nPortIndex;
   OMX_SCENEMODETYPE eSceneMode;
} OMX_CONFIG_SCENEMODETYPE;

 /**
 * Port specific capture trigger
 * useful for the usecases with multiple capture ports.
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  bExtCapturing : Start Captre at the specified port. Can be queried to know the status of a specific port.
 */
typedef struct OMX_CONFIG_EXTCAPTURING {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bExtCapturing;
} OMX_CONFIG_EXTCAPTURING;


 /**
 * Digital Zoom Speed
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  nDigitalZoomSpeed      :  Optical zoom speed level. Special values:
 *      0 – stop current movement
 *      values from 1 to 254 are mapped proportionally to supported zoom speeds inside optical zoom driver.
 *      So 1 is slowest available optical zoom speed and 254 is fastest available optical zoom speed
 *      255 – default optical zoom speed value
 */
typedef struct OMX_CONFIG_DIGITALZOOMSPEEDTYPE {
   OMX_U32 nSize;
   OMX_VERSIONTYPE nVersion;
   OMX_U32 nPortIndex;
   OMX_U8 nDigitalZoomSpeed;
} OMX_CONFIG_DIGITALZOOMSPEEDTYPE;


 /**
 * Digital Zoom Target
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  nDigitalZoomTarget      :  Default and minimum is 0. Maximum is determined by the current supported range
 */

typedef struct OMX_CONFIG_DIGITALZOOMTARGETTYPE {
   OMX_U32 nSize;
   OMX_VERSIONTYPE nVersion;
   OMX_U32 nPortIndex;
   OMX_U32 nDigitalZoomTarget;
} OMX_CONFIG_DIGITALZOOMTARGETTYPE;


/**
* Scale quality enums
*/
typedef enum OMX_SCALEQUALITY{
        OMX_DefaultScaling = 0, /** <default scaling if nothing is specified > */
        OMX_BetterScaling,   /** <better scaling> */
        OMX_BestScaling,  /** <best  scaling> */
        OMX_AutoScalingQuality,  /** <auto scaling quality> */
        OMX_FastScaling,   /** <fast scaling, prioritizes speed> */
        OMX_ScaleQualityMax = 0x7fffffff
}OMX_SCALEQUALITY;

/**
* Scaling Quality Mode
*/
typedef enum OMX_SCALEQUALITYMODE{
        OMX_SingleFrameScalingMode = 0, /** <default > */
        OMX_MultiFrameScalingMode,   /** <better scaling> */
        OMX_AutoScalingMode,  /** <best  scaling> */
        OMX_ScaleModeMax = 0x7fffffff
}OMX_SCALEQUALITYMODE;

 /**
 * Rescale quality control type
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  eScaleQuality : controls the quality level.
 *  eScaleQualityMode      :  controls the scaling algo types
 */
typedef struct OMX_CONFIG_SCALEQUALITYTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_SCALEQUALITY eScaleQuality;
    OMX_SCALEQUALITYMODE eScaleQualityMode;
} OMX_CONFIG_SCALEQUALITYTYPE;

/**
* Smooth Zoom mode enum
* Starts or stops the Smooth Zoom.  Selecting INCREASE will cause an increasing digital zoom factor (increased cropping),
* with a shrinking viewable area and crop height percentage.  Selecting DECREASE will cause a decreasing digital zoom (decreased cropping),
* with a growing viewable area and crop height percentage.  The CaptureCropHeight will continue to update based on the SmoothZoomRate until
* the SmoothZoomMin or SmoothZoomMax zoom step is reached, the framework minimum zoom step is reached, the SmoothZoomRate becomes 0,
* or the SmoothZoomMode is set to OFF.
* NOTE: The message payload includes all parts of the message that is NOT part of the message header as listed for the CAM_SEND_DATA message.
*/
typedef enum OMX_SMOOTHZOOMMODE{
    OMX_Off=0, /**< default OFF */
    OMX_Increase,
    OMX_Decrease
}OMX_SMOOTHZOOMMODE;


 /**
 * Rescale quality control type
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  eSmoothZoomMode : controls the smooth zoom feature.
 *  nSmoothZoomRate      :  Values from 0 to 65535 which represents percentage to increase per second, where 65535 = 100%, and 0 = 0%.
 *  nSmoothZoomQuantize:
 *  nSmoothZoomThresh
 *  nSmoothZoomMin
 *  nSmoothZoomMax
 */
typedef struct OMX_CONFIG_SMOOTHZOOMTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_SMOOTHZOOMMODE eSmoothZoomMode;
    OMX_U32 nSmoothZoomRate;
    OMX_U32 nSmoothZoomQuantize;
    OMX_U32 nSmoothZoomThresh;
    OMX_U32 nSmoothZoomMin;
    OMX_U32 nSmoothZoomMax;
} OMX_CONFIG_SMOOTHZOOMTYPE;

/**
 * Enumeration of possible Extended image filter types for OMX_CONFIG_IMAGEFILTERTYPE
 */
typedef enum OMX_EXTIMAGEFILTERTYPE {
    OMX_ImageFilterSepia = 0x7F000001,
    OMX_ImageFilterGrayScale,
    OMX_ImageFilterNatural,
    OMX_ImageFilterVivid,
    OMX_ImageFilterColourSwap,
    OMX_ImageFilterOutOfFocus,
    OMX_ImageFilterWaterColour,
    OMX_ImageFilterPastel,
    OMX_ImageFilterFilm
} OMX_EXTIMAGEFILTERTYPE;


/**
 * Image filter configuration extended
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bBlemish : Enable/Disable Blemish correction
 */
typedef struct OMX_CONFIG_BLEMISHTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bBlemish;
} OMX_CONFIG_BLEMISHTYPE;

/**
 * Enumeration of Bracket types
 * OMX_BracketExposureRelativeInEV:
 *      Exposure value is changed relative to the value set by automatic exposure.
 *      nBracketStartValue and nBracketStep are in Q16. Increment is additive.
 * OMX_BracketExposureAbsoluteMs:
 *      Exposure value is changed in absolute value in ms.
 *      nBracketStartValue and nBracketStep are in Q16. Increment is multiplicative.
 * OMX_BracketFocusRelative:
 *      Focus is adjusted relative to the focus set by auto focus.
 *      The value is S32 integer, and is the same as adjusting nFocusSteps of OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE relatively.
 *      Increment is additive.
 * OMX_BracketFocusAbsolute:
 *      Focus position is adjusted absolutely. It is the same as setting nFocusSteps of
 *      OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE relatively for each captures.
 *      The value should be interpreted as U32 value.  Increment is additive.
 * OMX_BracketFlashPower:
 *      Power of flash is adjusted relative to the automatic level. Increment is multiplicative.
 * OMX_BracketAperture:
 *      Aperture number relative to the automatic setting. Data in Q16 format. Increment is multiplicative.
 * OMX_BracketTemporal:
 *      To suppport temporal bracketing.
 */
typedef enum OMX_BRACKETMODETYPE {
    OMX_BracketExposureRelativeInEV = 0, 
    OMX_BracketExposureAbsoluteMs,
    OMX_BracketFocusRelative,
    OMX_BracketFocusAbsolute,
    OMX_BracketFlashPower,
    OMX_BracketAperture,
    OMX_BracketTemporal,    
    OMX_BrackerTypeKhronosExtensions = 0x6f000000,
    OMX_BrackerTypeVendorStartUnused = 0x7f000000, 
    OMX_BracketTypeMax = 0x7FFFFFFF
} OMX_BRACKETMODETYPE;

typedef struct OMX_CONFIG_BRACKETINGTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BRACKETMODETYPE eBracketMode;
    OMX_U32 nNbrBracketingValues;
    OMX_U32 nBracketValues[10]; /**< 10 can be assumed */ 
} OMX_CONFIG_BRACKETINGTYPE;


/** 
 * Capture mode types
 * Note: this list could get extended modified based on the type of interenal use-case pipelines implemented within the camera component.
 *  
 *       OMX_CaptureImageHighSpeedBurst = 0,
 *       OMX_CaptureImageHighSpeedTemporalBracketing,
 *       OMX_CaptureImageProfileBase(Base):
 *       	Base one almost same as Highspeed one. 
 *       OMX_CaptureImageProfileLowLight1(LL1):
 *       	Includes NSF2 in addition to Base processing
 *       OMX_CaptureImageProfileLowLight2(LL2):
 *       	Includes NSF2 and LBCE in addition to Base processing.
 *       OMX_CaptureImageProfileOpticalCorr1(OC1):
 *       	Includes LDC in addition to Base processing.
 *       OMX_CaptureImageProfileOpticalCorr2(OC2):
 *       	Includes LDC and CAC in addition to Base processing.
 *       OMX_CaptureImageProfileExtended1(Ext1):
 *       	Includes NSF2, LBCE, LDC, and CAC in addition to Base
 *       OMX_CaptureStereoImageCapture:
 *       	Stereo image capture use-case. 
 *       OMX_CaptureImageMemoryInput:
 *       	need to take sensor input from INPUT port.
 *       OMX_CaptureVideo:
 *       OMX_CaptureHighSpeedVideo:
 *       OMX_CaptureVideoMemoryInput:
 * 
 */
typedef enum OMX_CAMOPERATINGMODETYPE {
        OMX_CaptureImageHighSpeedBurst = 0,
        OMX_CaptureImageHighSpeedTemporalBracketing,
        OMX_CaptureImageProfileBase,
        OMX_CaptureImageProfileLowLight1,
        OMX_CaptureImageProfileLowLight2,
        OMX_CaptureImageProfileOpticalCorr1,
        OMX_CaptureImageProfileOpticalCorr2,
        OMX_CaptureImageProfileExtended1,
	OMX_CaptureStereoImageCapture,
        OMX_CaptureImageMemoryInput,
        OMX_CaptureVideo,
        OMX_CaptureHighSpeedVideo,
        OMX_CaptureVideoMemoryInput,
        OMX_CamOperatingModeMax = 0x7fffffff
} OMX_CAMOPERATINGMODETYPE;
/** 
 * Capture mode setting: applicable to multi shot capture also including bracketing. 
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes       
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to 
 *  eCamOperatingMode : specifies the camera operating mode. 
 */
typedef struct OMX_CONFIG_CAMOPERATINGMODETYPE {
   OMX_U32 nSize;
   OMX_VERSIONTYPE nVersion;
   OMX_CAMOPERATINGMODETYPE eCamOperatingMode;
} OMX_CONFIG_CAMOPERATINGMODETYPE;


/** 
 * Capture mode setting: applicable to multi shot capture also including bracketing. 
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes       
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to 
 *  nFrameRate   : when bContinuous is FALSE, need to define the frame rate of the muti-shot scenario. Since this would be applicable to IMAGE domain port, there is no port specific frame rate.   
 *  nFrameBefore : 
 * 	is specifying how many frames before the capture trigger shall be used. 
 * 	It is implementation dependent how many is supported. This shall only be supported for images and not for video frames.
 * bPrepareCapture : 
 *	should be set to true when nFrameBefore is greater than zero and before capturing of before-frames should start. 
 *	The component is not allowed to deliver buffers until capturing starts. This shall only be supported for images and not for video frames. 
 * bEnableBracketing : 
 *	should be enabled when bracketing is used. In bracketing mode, one parameter can be changed per each capture.
 * tBracketConfigType :
 *	specifies bracket mode to use. Valid only when bEnableBracketing is set.
 */
typedef struct OMX_CONFIG_EXTCAPTUREMODETYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nFrameRate;
    OMX_U32 nFrameBefore;
    OMX_BOOL bPrepareCapture;
    OMX_BOOL bEnableBracketing;
    OMX_CONFIG_BRACKETINGTYPE tBracketConfigType;
} OMX_CONFIG_EXTCAPTUREMODETYPE;

/**
 * For Extended Focus region Type -
 */
typedef struct OMX_CONFIG_EXTFOCUSREGIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nRefPortIndex;
    OMX_S32 nLeft;
    OMX_S32 nTop;
    OMX_U32 nWidth;
    OMX_U32 nHeight;
} OMX_CONFIG_EXTFOCUSREGIONTYPE;

/**
 * Digital Flash Control
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bDigitalFlash : Digital flash type Enable/Disable -
 * Specifies whether the ‘digital flash’ algorithm is enabled or disabled. This overrides the contrast and brightness settings.
 */
typedef struct OMX_CONFIG_DIGITALFLASHTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bDigitalFlash;
} OMX_CONFIG_DIGITALFLASHTYPE;



/**
 * Privacy Indicator Enable/Disable
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bPrivacyIndicator :
 *        Specifies whether the flash should be used to indicate image or video capture. When flash is not used for exposure,
 *        flash will be activated after exposure to indicate image capture.
 *        If video light is not used, the flash can be blinking or constant at low intensity to indicate capture but not affect exposure.
 *        Specifies whether the ‘digital flash’ algorithm is enabled or disabled. This overrides the contrast and brightness settings.
 */
typedef struct OMX_CONFIG_PRIVACYINDICATOR {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bPrivacyIndicator;
} OMX_CONFIG_PRIVACYINDICATOR;


/**
 * Privacy Indicator Enable/Disable
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bTorchMode :
 *        Enable/Disable
 *      nIntensityLevel : relative intensity from 0 - 100
 *      nDuration : duration in msec
 */
typedef struct OMX_CONFIG_TORCHMODETYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bTorchMode;
    OMX_U32 nIntensityLevel;
    OMX_U32 nDuration;
} OMX_CONFIG_TORCHMODETYPE;



/**
 * Privacy Indicator Enable/Disable
 * DISABLE – Fire the xenon flash in the usual manner
 * ENABLE – Reduce the light intensity of the main flash (ex 1EV)
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bSlowSync :
 *        Enable - OMX_TRUE/Disable - OMX_FALSE
 */
typedef struct OMX_CONFIG_SLOWSYNCTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bSlowSync;
} OMX_CONFIG_SLOWSYNCTYPE;


/**
 * Focus control extended enums. use this along with OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE
 */
typedef enum OMX_IMAGE_EXTFOCUSCONTROLTYPE {
    OMX_IMAGE_FocusControAutoMacro = 0x7F000001, /**< Reserved region for introducing Vendor Extensions */
    OMX_IMAGE_FocusControlAutoInfinity,
    OMX_IMAGE_FocusFacePriorityMode,
    OMX_IMAGE_FocusRegionPriorityMode,
    OMX_IMAGE_FocusControlHyperfocal,
    OMX_IMAGE_FocusControlPortrait, /**< from Xena */
    OMX_IMAGE_FocusControlExtended, /**< from Xena */
    OMX_IMAGE_FocusControlContinousNormal, /**< from Xena */
    OMX_IMAGE_FocusControlContinousExtended, /**< from Xena */
    OMX_IMAGE_FocusFacePriorityContinousMode,
    OMX_IMAGE_FocusRegionPriorityContinousMode,
} OMX_IMAGE_EXTFOCUSCONTROLTYPE;



/**
 * Specifies whether the LED can be used to assist in autofocus, due to low lighting conditions.
 * ‘ENABLE’ means use as determined by the auto exposure algorithm.
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bFocusAssist :
 *        Enable - OMX_TRUE/Disable - OMX_FALSE
 */
typedef struct OMX_CONFIG_FOCUSASSISTTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bFocusAssist;
} OMX_CONFIG_FOCUSASSISTTYPE;



/**
 *for locking the focus
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bFocusLock :
 *        Enable - OMX_TRUE/Disable - OMX_FALSE
 */
typedef struct OMX_CONFIG_FOCUSLOCKTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bFocusLock;
} OMX_CONFIG_FOCUSLOCKTYPE;


/**
 *for locking the White balance
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bWhiteBalanceLock :
 *        Enable - OMX_TRUE/Disable - OMX_FALSE
 */
typedef struct OMX_CONFIG_WHITEBALANCELOCKTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bWhiteBalanceLock;
} OMX_CONFIG_WHITEBALANCELOCKTYPE;

/**
 *for locking the Exposure
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bExposureLock :
 *        Enable - OMX_TRUE/Disable - OMX_FALSE
 */
typedef struct OMX_CONFIG_EXPOSURELOCKTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bExposureLock;
} OMX_CONFIG_EXPOSURELOCKTYPE;

/**
 *for locking the Exposure
 *  Simultaneously lock focus, white balance and exposure (and relevant other settings).
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bAllLock :
 *        Enable - OMX_TRUE/Disable - OMX_FALSE
 */
typedef struct OMX_CONFIG_ALLLOCKTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bAllLock;
} OMX_CONFIG_ALLLOCKTYPE;

/**
 *for locking
 *  Simultaneously lock focus, white balance and exposure (and relevant other settings).
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bLock :
 *        Enable - OMX_TRUE/Disable - OMX_FALSE
 *  bAtCapture:
 *
 */
typedef struct OMX_IMAGE_CONFIG_LOCKTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bLock;
    OMX_BOOL bAtCapture;
} OMX_IMAGE_CONFIG_LOCKTYPE;

/**
 * processig level types enum
 */
typedef enum OMX_PROCESSINGLEVEL{
        OMX_Min = 0,
        OMX_Low,
        OMX_Medium,
        OMX_High,
        OMX_Max,
        OMX_ProcessingLevelMax = 0x7fffffff
}OMX_PROCESSINGLEVEL;

/** 
 *processing level type  
 *  Simultaneously lock focus, white balance and exposure (and relevant other settings).
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes       
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to 
 *  nLevel : 
 *               nLevel hinting processing amount. Range of values is -100 to 100.  
 *               0 causes no change to the image.  Increased values cause increased processing to occur, with 100 applying maximum processing. 
 *               Negative values have the opposite effect of positive values.
 *  bAuto:
 *		sets if the processing should be applied according to input data. 
 		It is allowed to combine the hint level with the auto setting,
 *		i.e. to give a bias to the automatic setting. When set to false, the processing should not take input data into account.
 */

typedef struct OMX_IMAGE_CONFIG_PROCESSINGLEVELTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_S32 nLevel;
OMX_BOOL bAuto;
} OMX_IMAGE_CONFIG_PROCESSINGLEVELTYPE;


/**
 * White Balance control type extended enums - to be used along with the structure @OMX_CONFIG_WHITEBALCONTROLTYPE
 *
 *
 *
 */
typedef enum OMX_EXTWHITEBALCONTROLTYPE {
    OMX_WhiteBalControlFacePriorityMode = 0x7F000001 /**<  */

} OMX_EXTWHITEBALCONTROLTYPE;

/**
 *white balance gain type
 *  xWhiteBalanceGain and xWhiteBalanceOffset represents gain and offset for R, Gr, Gb, B channels respectively in Q16 format. \
 *  For example, new red pixel value = xWhiteBalanceGain[1]* the current pixel value + xWhiteBalanceOffset[1].
 *  All values assume that maximum value is 255. If internal implementation uses higher dynamic range, this value should be adjusted internally.
 *  nWhiteThreshhold  represents thresholds for "white" area measurments in Q16 format.
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *
 */
typedef struct OMX_CONFIG_WHITEBALGAINTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_S32 xWhiteBalanceGain[4];
    OMX_S32 xWhiteBalanceOffset[4];
    OMX_S32 nWhiteThreshhold[4];
} OMX_CONFIG_WHITEBALGAINTYPE; 

/**
 *  This structure represents linear color conversion from one space to another.  For example, to conversion from one RGB color into another RGB color space can be represented as
 *  R' =  xColorMatrix[1][1]*R + xColorMatrix[1][2]*G + xColorMatrix[1][3]*B + xColorOffset[1]
 *  G' = xColorMatrix[2][1]*R + xColorMatrix[2][2]*G + xColorMatrix[2][3]*B + xColorOffset[2]
 *  B' = xColorMatrix[3][1]*R + xColorMatrix[3][2]*G + xColorMatrix[3][3]*B + xColorOffset[3]
 *  Both xColorMatrix and xColorOffset are represented as Q16 value.
 *  bFullColorRange represents represents whether valid range of color is 0 to 255 (when set to TRUE) or 16 to 235 (for FALSE).
 *  Again all values assume that maximum value is 255. If internal implementation uses higher dynamic range, this value should be adjusted internally.
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *
 */
typedef struct OMX_CONFIG_EXT_COLORCONVERSIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_S32 xColorMatrix[3][3];
    OMX_S32 xColorOffset[3];
    OMX_BOOL bFullColorRange;
}OMX_CONFIG_EXT_COLORCONVERSIONTYPE;


/**
 * xGamma represents lool-up table for gamma correction in Q16 format.
 * All values assume that maximum value is 255. If internal implementation uses higher dynamic range, this value should be adjusted internally.
 *
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *
 */
typedef struct OMX_CONFIG_GAMMATABLETYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 xGamma[3][256];
}OMX_CONFIG_GAMMATABLETYPE;



/**
 * processig types
 */
typedef enum OMX_PROCESSINGTYPE{
        OMX_BloomingReduction = 0,
        OMX_Denoise,
        OMX_Sharpening,
        OMX_Deblurring,
        OMX_Demosaicing,
        OMX_ContrastEnhancement,
        OMX_ProcessingTypeMax = 0x7fffffff
}OMX_PROCESSINGTYPE;


typedef  struct OMX_CONFIGPROCESSINGORDERTYPE {
OMX_U32  nSize; /**< Size of the structure in bytes */
OMX_VERSIONTYPE nVersion; /**< OMX specification version info */
OMX_U32 nPortIndex; /**< Port that this struct applies to */
OMX_U32 nIndex;
OMX_PROCESSINGTYPE eProc;
} OMX_CONFIGPROCESSINGORDERTYPE;

/**
 * HIST TYPE
 */
typedef enum OMX_HISTTYPE{
        OMX_HistControlLuminance = 0, /**< Luminance histogram is calculated (Y)*/
        OMX_HistControlColorComponents, /**< A histogram per color component (R, G, B) is calculated*/
        OMX_HistControlChrominanceComponents /**< A histogram per chrominance component (Cb, Cr) is calculated.*/
}OMX_HISTTYPE;

/**
 * Histogram Setting
 *  nPortIndex is an output port. The port index decides on which port the extra data structur is sent on.
 *  bFrameLimited is a Boolean used to indicate if measurement shall be terminated after the specified number of
 *  frames if true frame limited measurement is enabled; otherwise the port does not terminate measurement until
 *  instructed to do so by the client.
 *  nFrameLimit is the limit on number of frames measured, this parameter is only valid if bFrameLimited is enabled.
 *  bMeasure is a Boolean that should be set to true when measurement shall begin, otherwise set to false. Query will give status information on if measurement is ongoing or not.
 *  nBins specifies the number of histogram bins. When queried with set to zero, the respons gives the maximum number of bins allowed.
 *  nLeft is the leftmost coordinate of the measurement area rectangle.
 *  nTop is the topmost coordinate of the measurement area rectangle.
 *  nWidth is the width of the measurement area rectangle in pixels.
 *  nHeight is the height of the measurement area rectangle in pixels.
 *  eHistType is an enumeration specifying the histogram type
 *
 *
 */

typedef struct OMX_CONFIG_HISTOGRAMTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bFrameLimited;
    OMX_U32 nFrameLimit;
    OMX_BOOL bMeasure;
    OMX_U32 nBins;
    OMX_S32 nLeft;
    OMX_S32 nTop;
    OMX_U32 nWidth;
    OMX_U32 nHeight;
    OMX_HISTTYPE eHistType;
} OMX_CONFIG_HISTOGRAMTYPE;

/**
 * Enums for HIST component type.
 */
typedef enum OMX_HISTCOMPONENTTYPE{
        OMX_HISTCOMP_Y = 0, /**<    Luminance histogram (Y) */
        OMX_HISTCOMP_YLOG,  /**< Logarithmic luminance histogram (Y)*/
        OMX_HISTCOMP_R, /**< Red histogram component (R)*/
        OMX_HISTCOMP_G, /**< Green histogram component (G)*/
        OMX_HISTCOMP_B, /**< Blue histogram component (B)*/
        OMX_HISTCOMP_Cb,    /**< Chroma blue histogram component (Cb)*/
        OMX_HISTCOMP_Cr /**< Chroma red histogram component (Cr) */
}OMX_HISTCOMPONENTTYPE;

/**
 *  nSize is the size of the structure including the length of data field containing
 *  the histogram data.
 *  nBins is the number of bins in the histogram.
 *  eComponentType specifies the type of the histogram bins according to enum.
 *  It can be selected to generate multiple component types, then the extradata struct
 *  is repeated for each component type.
 *  data[1] first byte of the histogram data
 */
typedef struct OMX_HISTOGRAMTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nBins;
    OMX_HISTCOMPONENTTYPE eComponentType;
    OMX_U8  data[1];
} OMX_HISTOGRAMTYPE;

/**
 * OMX_BARCODETYPE
 */
typedef enum OMX_BARCODETYPE{
        OMX_Barcode1D = 0, /**< 1D barcode */
        OMX_Barcode2D,	/**< 2D barcode */
}OMX_BARCODETYPE;
/**
 * Brcode detection data 
 *	nLeft is the leftmost coordinate of the detected area rectangle.
 *	nTop is the topmost coordinate of the detected area rectangle.
 *	nWidth is the width of the detected area rectangle in pixels.
 *	nHeight is the height of the detected area rectangle in pixels.
 *	nOrientation is the orientation of the axis of the detected object. This refers to the angle between the vertical axis of barcode and the horizontal axis.
 *	eBarcodetype is an enumeration specifying the barcode type, as listed in the given table.
 */ 
typedef struct OMX_BARCODEDETECTIONTYPE {
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex; 
	OMX_S32 nLeft; 
	OMX_S32 nTop;
	OMX_U32 nWidth; 
	OMX_U32 nHeight;
	OMX_S32 nOrientation;
	OMX_BARCODETYPE eBarcodetype; 
 } OMX_BARCODEDETECTIONTYPE;

/**
 * Front object detection data
 *	nLeft is the leftmost coordinate of the detected area rectangle.
 *	nTop is the topmost coordinate of the detected area rectangle.
 *	nWidth is the width of the detected area rectangle in pixels.
 *	nHeight is the height of the detected area rectangle in pixels.
 */ 
typedef struct OMX_FRONTOBJDETECTIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex; 
    OMX_S32 nLeft; 
    OMX_S32 nTop; 
    OMX_U32 nWidth; 
    OMX_U32 nHeight;
} OMX_FRONTOBJDETECTIONTYPE;

/**
 * Distance estimation data
 * nDistance is the estimated distance to the object in millimeters.
 * nLargestDiscrepancy is the estimated largest discrepancy of the distance to the object in millimeters. When equal to MAX_INT the discrepancy is unknown.
 */ 
typedef struct OMX_DISTANCEESTIMATIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex; 
    OMX_U32 nDistance;
    OMX_U32 nLargestDiscrepancy;
} OMX_DISTANCEESTIMATIONTYPE;

/**
 * Distance estimation data
 * nDistance is the estimated distance to the object in millimeters.
 * nLargestDiscrepancy is the estimated largest discrepancy of the distance to the object in millimeters. When equal to MAX_INT the discrepancy is unknown.
 */ 

typedef struct OMX_MOTIONESTIMATIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex; 
    OMX_S32 nPanX; 
    OMX_S32 nPanY; 
} OMX_MOTIONESTIMATIONTYPE;


/**
 * Focus region data
 *	nRefPortIndex is the port the image frame size is defined on. This image frame size is used as reference for the focus region rectangle.
 *	nLeft is the leftmost coordinate of the focus region rectangle.
 *	nTop is the topmost coordinate of the focus region rectangle.
 *	nWidth is the width of the focus region rectangle in pixels.
 *	nHeight is the height of the focus region rectangle in pixels.
 *
 */ 
typedef struct OMX_FOCUSREGIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex; 
    OMX_U32 nRefPortIndex; 
    OMX_S32 nLeft; 
    OMX_S32 nTop; 
    OMX_U32 nWidth; 
    OMX_U32 nHeight;
} OMX_FOCUSREGIONTYPE;

/**
 * OMX_ISOSETTINGTYPE: specifies its auto or manual setting
 *
 */
typedef enum OMX_ISOSETTINGTYPE{
        OMX_Auto = 0, /**<	*/
        OMX_IsoManual	/**< */
}OMX_ISOSETTINGTYPE;

/**
 *  nSize is the size of the structure including the length of data field containing 
 *  the histogram data.
 *  eISOMode: 
 *  	specifies the ISO seetting mode - auto/manual 
 *  nISOSetting:
 *  	for manual mode client can specify the ISO setting. 
 */

typedef struct OMX_CONFIG_ISOSETTINGTYPE{
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex; 
	OMX_ISOSETTINGTYPE eISOMode;
	OMX_U32 nISOSetting; 
}OMX_CONFIG_ISOSETTINGTYPE;

/**
 * custom RAW format 
 */
typedef struct OMX_CONFIG_RAWFORMATTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex; 
    OMX_VERSIONTYPE nFormatVersion;
    OMX_STRING cVendorName;
} OMX_CONFIG_RAWFORMATTYPE;

/**
 * Sensor type 
 */
typedef struct OMX_CONFIG_SENSORTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex; 
    OMX_VERSIONTYPE nSensorVersion;
    OMX_STRING cModelName;
} OMX_CONFIG_SENSORTYPE;

/**
 * Sensor custom data type
 */
typedef struct OMX_CONFIG_SENSORCUSTOMDATATYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex; 
    OMX_U32 nDataLength;
    OMX_U8 xSensorData[1];
} OMX_CONFIG_SENSORCUSTOMDATATYPE;

/**
 * OMX_OBJDETECTQUALITY
 *
 */
typedef enum OMX_OBJDETECTQUALITY{
        OMX_FastDetection = 0, /**< A detection that prioritizes speed*/
        OMX_Default,    /**< The default detection, should be used when no control of the detection quality is given.*/
        OMX_BetterDetection,    /**< A detection that levels correct detection with speed*/
        OMX_BestDtection,   /**< A detection that prioritizes correct detection*/
        OMX_AUTODETECTION   /**< Automatically decide which object detection quality is best.*/
}OMX_OBJDETECTQUALITY;

/**
 * OBJECT DETECTION Type
 *      nPortIndex: is an output port. The port index decides on which port the extra data structur of detected object is sent on.
 *      bEnable : this controls ON/OFF for this object detection algirithm.
 *      bFrameLimited: is a Boolean used to indicate if detection shall be terminated after the specified number of frames if
 *          true frame limited detection is enabled; otherwise the port does not terminate detection until instructed to do so by the client.
 *      nFrameLimit: is the limit on number of frames detection is executed for, this parameter is only valid if bFrameLimited is enabled.
 *      nMaxNbrObjects: specifies the maximum number of objects that should be found in each frame. It is implementation dependent which objects are found.
 *      nLeft: is the leftmost coordinate of the detection area rectangle.
 *      nTop: is the topmost coordinate of the detection area rectangle.
 *      nWidth: is the width of the detection area rectangle in pixels.
 *      nHeight: is the height of the detection area rectangle in pixels.
 *      eObjDetectQuality: is an enumeration specifying the quality desired by the detection.
 *      nPriority: represents priority of each object when there are multiple objects detected.
 */

typedef struct OMX_CONFIG_OBJDETECTIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bEnable;
    OMX_BOOL bFrameLimited;
    OMX_U32 nFrameLimit;
    OMX_U32 nMaxNbrObjects;
    OMX_S32 nLeft;
    OMX_S32 nTop;
    OMX_U32 nWidth;
    OMX_U32 nHeight;
    OMX_OBJDETECTQUALITY eObjDetectQuality;
    OMX_U32 nPriority;
 } OMX_CONFIG_OBJDETECTIONTYPE;


/**
 * OMX_OBJDETECTQUALITY
 *
 */
typedef enum OMX_DISTTYPE{
        OMX_DistanceControlFocus = 0, /**< focus objects distance type*/
        OMX_DISTANCECONTROL_RECT	/**< Evaluated distance to the object found in the rectangelar area indicated as input region.  */
}OMX_DISTTYPE;


/**
 * Distance mesurement 
 *	bStarted is a Boolean. The IL client sets it to true to start the measurement . 
 *		the IL client sets to false to stop the measurement. The IL client can query it to check if the measurement is ongoing.
 *	nLeft : is the leftmost coordinate of the rectangle. 
 *	nTop : is the topmost coordinate of the rectangle. 
 *	nWidth:  is the width of the rectangle in pixels. 
 *	nHeight:  is the height of the rectangle in pixels. 
 *	eDistType:  is an enumeration specifying the distance measurement type, as shown in 
 */ 
typedef struct OMX_CONFIG_DISTANCETYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bStarted;
    OMX_S32 nLeft; 
    OMX_S32 nTop; 
    OMX_U32 nWidth; 
    OMX_U32 nHeight;
    OMX_DISTTYPE eDistType;
} OMX_CONFIG_DISTANCETYPE;


/**
 * face detect data - face attribute
 *  nARGBEyeColor: is the indicates a 32-bit eye color of the person, where bits 0-7 are blue,
 *      bits 15-8 are green, bits 24-16 are red, and bits 31-24 are for alpha.
 *  nARGBSkinColor: is the indicates a 32-bit skin color of the person, where bits 0-7 are blue,
 *      bits 15-8 are green, bits 24-16 are red, and bits 31-24 are for alpha.
 *  nARGBHairColor: is the indicates a 32-bit hair color of the person, where bits 0-7 are blue,
 *      bits 15-8 are green, bits 24-16 are red, and bits 31-24 are for alpha.
 *  nSmileScore: a smile detection score between 0 and 100, where 0 means not detecting,
 *      1 means least certain and 100 means most certain a smile is detected.
 *  nBlinkScore: a eye-blink detection score between 0 and 100, where 0 means not detecting,
 *      1 means least certain and 100 means most certain an eye-blink is detected.
 *  xIdentity: represents the identity of the face. With identity equal to zero this is not supported.
 *      This can be used by a face recognition application. The component shall not reuse an identity value unless the same face.
 *      Can be used to track detected faces when it moves between frames. Specific usage of this field is implementation dependent.
 *      It can be some kind of ID.
 *
 */
typedef struct OMX_FACEATTRIBUTE {
        OMX_U32 nARGBEyeColor;
    OMX_U32 nARGBSkinColor;
    OMX_U32 nARGBHairColor;
    OMX_U32 nSmileScore;
    OMX_U32 nBlinkScore;
    OMX_U32 xIdentity[4];
} OMX_FACEATTRIBUTE;

/**
 * xGamma represents lool-up table for gamma correction in Q16 format.
 * All values assume that maximum value is 255. If internal implementation uses higher dynamic range, this value should be adjusted internally.
 *
 *
 *
 * STRUCT MEMBERS:
 *  nScore: is a detection score between 0 and 100, where 0 means unknown score, 1 means least certain and 100 means most certain the detection is correct.
 *  nLeft: is the leftmost coordinate of the detected area rectangle.
 *  nTop: is the topmost coordinate of the detected area rectangle.
 *  nWidth: is the width of the detected area rectangle in pixels.
 *  nHeight: is the height of the detected area rectangle in pixels.
 *  nOrientationRoll/Yaw/Pitch is the orientation of the axis of the detected object. Here roll angle is defined as the angle between the vertical axis of face and the horizontal axis. All angles can have the value of -180 to 180 degree in Q16 format. Some face detection algorithm may not be able to fill in the angles, this is denoted by the use of MAX_INT value.
 *  nPriority represents priority of each object when there are multiple objects detected.
 *  nFaceAttr describe the attributes of the detected face object with the following structure:
 *
 *
 */
typedef struct OMX_FACEDETECTIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nScore;
    OMX_S32 nLeft;
    OMX_S32 nTop;
OMX_U32 nWidth;
OMX_U32 nHeight;
OMX_S32 nOrientationRoll;
OMX_S32 nOrientationYaw;
OMX_S32 nOrientationPitch;
OMX_U32 nPriority;
OMX_FACEATTRIBUTE nFaceAttr;
} OMX_FACEDETECTIONTYPE;


/**
 * The OMX_EXTRADATATYPE enumeration is used to define the 
 * possible extra data payload types.
 */
typedef enum OMX_EXT_EXTRADATATYPE
{
   OMX_ExifAttributes = 0x7F000001, /**< Reserved region for introducing Vendor Extensions */
   OMX_FaceDetection, /**< face detect data */
   OMX_BarcodeDetection, /**< bar-code detct data */
   OMX_FrontObjectDetection, /**< Front object detection data */
   OMX_MotionEstimation, /**< motion Estimation data */
   OMX_DistanceEstimation, /**< disctance estimation */
   OMX_Histogram, /**< histogram */
   OMX_FocusRegion, /**< focus region data */
   OMX_RawFormat, /**< custom RAW data format */
   OMX_SensorType, /**< vendor & model of the sensor being used */
   OMX_SensorCustomDataLength, /**< vendor specific custom data length */
   OMX_SensorCustomData /**< vendor specific data */
} OMX_EXT_EXTRADATATYPE;


/** 
 * Enable Extra-data on a specific port. 
 *
 *
 * 
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes       
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port on which this extra data to be assosiated 
 *  eExtraDataType :  Extra data type
 *  bEnable      : Eneble/Disable this extra-data through port. 
 * 
 */
typedef struct OMX_CONFIG_EXTRADATATYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_EXT_EXTRADATATYPE eExtraDataType;
    OMX_BOOL bEnable;
} OMX_CONFIG_EXTRADATATYPE;

/**
 * JPEG header type 
 * */

typedef enum OMX_JPEGHEADERTYPE{
	OMX_NoHeader = 0,
	OMX_JFIF,
	OMX_EXIF
}OMX_JPEGHEADERTYPE;
/** 
 * Re-start marker configuration 
 *
 *
 * 
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes       
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port on which this extra data to be assosiated 
 *  eJpegHeaderType : JPEG header type EXIF, JFIF, or No heeader.  
 */

typedef struct OMX_CONFIG_JPEGHEEADERTYPE{
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex;	
	OMX_JPEGHEADERTYPE eJpegHeaderType;
}OMX_CONFIG_JPEGHEEADERTYPE;

/** 
 * Re-start marker configuration 
 *
 *
 * 
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes       
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port on which this extra data to be assosiated 
 *  nRstInterval :  interval at which RST markers are to be inserted. 
 *  bEnable      : Eneble/Disable this RST marker insertion feature. 
 * 
 */

typedef struct OMX_CONFIG_RSTMARKER{
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex;	
	OMX_U32 nRstInterval;
	OMX_BOOL nEnable;
}OMX_CONFIG_RSTMARKER;

/** 
 * Enable Extra-data on a specific port.
 *
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 * nMaxSize : max size
 *
 *
 */
typedef struct OMX_IMAGE_JPEGMAXSIZE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nMaxSize;
} OMX_IMAGE_JPEGMAXSIZE;


typedef enum OMX_IMAGESTAMPOPERATION{
    OMX_NewImageStamp = 0,
    OMX_Continuation
}OMX_IMAGESTAMPOPERATION;


/**
 * Enable Extra-data on a specific port.
 *
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 * nMaxSize : max size
 *
 *
 */
typedef struct OMX_PARAM_IMAGESTAMPOVERLAYTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_IMAGESTAMPOPERATION nOp;
    OMX_U32 nLeft;
    OMX_U32 nTop;
    OMX_U32 nHeight;
    OMX_U32 nWidth;
    OMX_COLOR_FORMATTYPE eFormat;
    OMX_U8 * pBitMap;
} OMX_PARAM_IMAGESTAMPOVERLAYTYPE;


/**
 * Enable Extra-data on a specific port.
 *
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 * nMaxSize : max size
 *
 *
 */
typedef struct OMX_PARAM_THUMBNAILTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nHeight;
    OMX_U32 nWidth;
    OMX_IMAGE_CODINGTYPE eCompressionFormat;
    OMX_COLOR_FORMATTYPE eColorFormat;
    OMX_U32 nQuality;
    OMX_U32 nMaxSize;
} OMX_PARAM_THUMBNAILTYPE;

/**
 * Red-Eye Removal Enum
 */
typedef enum OMX_REDEYEREMOVALTYPE{
    OMX_RedEyeRemovalOff    = 0, /** No red eye removal*/
    OMX_RedEyeRemovalOn, /**    Red eye removal on*/
    OMX_RedEyeRemovalAuto,  /** Red eye removal will be done automatically when detected*/
    OMX_RedEyeRemovalKhronosExtensions = 0x6F000000,    /** Reserved region for introducing Khronos Standard Extensions*/
    OMX_RedEyeRemovalVendorStartUnused = 0x7F000000,    /** Reserved region for introducing Vendor Extensions*/
    OMX_RedEyeRemovalMax = 0x7FFFFFFF
}OMX_REDEYEREMOVALTYPE;

/**
 * Enable Extra-data on a specific port.
 *
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  nLeft: is the leftmost coordinate of the detection area rectangle (such as face region).
 *  nTop: is the topmost coordinate of the detection area rectangle (such as face region).
 *  nWidth: is the width of the detection area rectangle  in pixels.
 *  nHeight: is the height of the detection area rectangle in pixels.
 *  nARGBEyeColor indicates a 32-bit eye color to replace the red-eye, where bits 0-7 are blue, bits 15-8 are green, bits 24-16 are red, and bits 31-24 are for alpha. When all zero indicates automatic choice.

 *
 */
typedef struct OMX_CONFIG_REDEYEREMOVALTYPE {
   OMX_U32 nSize;
   OMX_VERSIONTYPE nVersion;
   OMX_U32 nPortIndex;
   OMX_S32 nLeft;
   OMX_S32 nTop;
   OMX_U32 nWidth;
   OMX_U32 nHeight;
   OMX_U32 nARGBEyeColor;
   OMX_REDEYEREMOVALTYPE eMode;
} OMX_CONFIG_REDEYEREMOVALTYPE;






/**
 * Video capture YUV Range Enum
 */
typedef enum OMX_VIDEOYUVRANGETYPE{
    OMX_ITURBT601 = 0,
    OMX_Full8Bit,
    OMX_VideoYUVRangeKhronosExtensions = 0x6F000000,    /** Reserved region for introducing Khronos Standard Extensions*/
    OMX_VideoYUVRangeVendorStartUnused = 0x7F000000,    /** Reserved region for introducing Vendor Extensions*/
    OMX_VideoYUVRangeMax = 0x7FFFFFFF
}OMX_VIDEOYUVRANGETYPE;

/**
 * Enable Extra-data on a specific port.
 *
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *
 */
typedef struct OMX_PARAM_VIDEOYUVRANGETYPE {
   OMX_U32 nSize;
   OMX_VERSIONTYPE nVersion;
   OMX_U32 nPortIndex;
   OMX_VIDEOYUVRANGETYPE eYUVRange;
} OMX_PARAM_VIDEOYUVRANGETYPE;

/**
 * Video noise filter mode range enum
 */
typedef enum OMX_VIDEONOISEFILTERMODETYPE{
    OMX_VideoNoiseFilterModeOff = 0,
    OMX_VideoNoiseFilterModeOn,
    OMX_VideoNoiseFilterModeAuto,
    OMX_VideoNoiseFilterModeExtensions = 0x6F000000,    /** Reserved region for introducing Khronos Standard Extensions */
    OMX_VideoNoiseFilterModeStartUnused = 0x7F000000,   /** Reserved region for introducing Vendor Extensions */
    OMX_VideoNoiseFilterModeMax = 0x7FFFFFFF
} OMX_VIDEONOISEFILTERMODETYPE;

/**
 * Enable video noise filter.
 *
 * STRUCT MEMBERS:
 *  nSize       : Size of the structure in bytes
 *  nVersion    : OMX specification version information
 *  nPortIndex  : Port that this structure applies to
 *  eMode       : Video noise filter mode (on/off/auto)
 */
typedef struct OMX_PARAM_VIDEONOISEFILTERTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_VIDEONOISEFILTERMODETYPE eMode;
} OMX_PARAM_VIDEONOISEFILTERTYPE;


/**
 * High ISO Noise filter mode range enum
 */
typedef enum OMX_ISONOISEFILTERMODETYPE{
    OMX_ISONoiseFilterModeOff = 0,
    OMX_ISONoiseFilterModeOn,
    OMX_ISONoiseFilterModeAuto,
    OMX_ISONoiseFilterModeExtensions = 0x6F000000,    /** Reserved region for introducing Khronos Standard Extensions */
    OMX_ISONoiseFilterModeStartUnused = 0x7F000000,   /** Reserved region for introducing Vendor Extensions */
    OMX_ISONoiseFilterModeMax = 0x7FFFFFFF
} OMX_ISONOISEFILTERMODETYPE;

/**
 * Enable ISO noise filter.
 *
 * STRUCT MEMBERS:
 *  nSize       : Size of the structure in bytes
 *  nVersion    : OMX specification version information
 *  nPortIndex  : Port that this structure applies to
 *  eMode       : ISO noise filter (NSF2 is used) mode (on/off/auto)
 */
typedef struct OMX_PARAM_ISONOISEFILTERTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_ISONOISEFILTERMODETYPE eMode;
} OMX_PARAM_ISONOISEFILTERTYPE;


/**
 * Manual White Balance color temperature
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  nColorTemperature : Color Temperature in K
 */
typedef struct OMX_TI_CONFIG_WHITEBALANCECOLORTEMPTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nColorTemperature;
} OMX_TI_CONFIG_WHITEBALANCECOLORTEMPTYPE;

/**
 * Focus spot weighting range enum
 */
typedef enum OMX_TI_CONFIG_FOCUSSPOTMODETYPE {
    OMX_FocusSpotDefault = 0,                           /** Makes CommonFocusRegion to be used. */
    OMX_FocusSpotSinglecenter,
    OMX_FocusSpotMultiNormal,
    OMX_FocusSpotMultiAverage,
    OMX_FocusSpotMultiCenter,
    OMX_FocusSpotExtensions = 0x6F000000,               /** Reserved region for introducing Khronos Standard Extensions */
    OMX_FocusSpotModeStartUnused = 0x7F000000,          /** Reserved region for introducing Vendor Extensions */
    OMX_FocusSpotModeMax = 0x7FFFFFFF
} OMX_TI_CONFIG_FOCUSSPOTMODETYPE;

/**
 * Focus Spot Weighting configuration.
 *
 * STRUCT MEMBERS:
 *  nSize       : Size of the structure in bytes
 *  nVersion    : OMX specification version information
 *  nPortIndex  : Port that this structure applies to
 *  eMode       : Spot Weighting mode
 */
typedef struct OMX_TI_CONFIG_FOCUSSPOTWEIGHTINGTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_TI_CONFIG_FOCUSSPOTMODETYPE eMode;
} OMX_TI_CONFIG_FOCUSSPOTWEIGHTINGTYPE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


