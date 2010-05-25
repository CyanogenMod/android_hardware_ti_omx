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

#ifdef _Android

/* #include "OMX_RegLib.h" */
#include "OMX_Component.h"
#include "OMX_Core.h"
#include "OMX_ComponentRegistry.h"

#include "OMX_Core_Wrapper.h"
#include <utils/Log.h>
#undef LOG_TAG
#define LOG_TAG "OMX_CORE"

/** determine capabilities of a component before acually using it */
#if 0
#include "ti_omx_config_parser.h"
#else
extern OMX_BOOL TIOMXConfigParser(
        OMX_PTR aInputParameters,
        OMX_PTR aOutputParameters);
#endif

#endif


#ifdef _Android
OMX_BOOL TIOMXConfigParserRedirect(
    OMX_PTR aInputParameters,
    OMX_PTR aOutputParameters)

{
    LOGD("OMXConfigParserRedirect +\n");
    OMX_BOOL Status = OMX_FALSE;

    Status = TIOMXConfigParser(aInputParameters, aOutputParameters);

    LOGD("OMXConfigParserRedirect -\n");
    return Status;
}

OMX_ERRORTYPE TIComponentTable_EventHandler(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1,
        OMX_IN OMX_U32 nData2,
        OMX_IN OMX_PTR pEventData)
{
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE TIComponentTable_EmptyBufferDone(
        OMX_OUT OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_PTR pAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE TIComponentTable_FillBufferDone(
        OMX_OUT OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_PTR pAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
    return OMX_ErrorNotImplemented;
}


OMX_API OMX_ERRORTYPE TIOMX_Init(void)
{
   LOGD("TIOMX_Init\n");

	return OMX_Init();
}

OMX_API OMX_ERRORTYPE TIOMX_Deinit(void)
{
   LOGD("TIOMX_Deinit\n");

	return OMX_Deinit();
}

OMX_API OMX_ERRORTYPE TIOMX_ComponentNameEnum(
    OMX_OUT OMX_STRING cComponentName,
    OMX_IN  OMX_U32 nNameLength,
    OMX_IN  OMX_U32 nIndex)
{

   LOGD("TIOMX_ComponentNameEnum\n");

	return OMX_ComponentNameEnum(
		cComponentName,
		nNameLength,
		nIndex);
}

OMX_API OMX_ERRORTYPE TIOMX_GetHandle(
    OMX_OUT OMX_HANDLETYPE* pHandle,
    OMX_IN  OMX_STRING cComponentName,
    OMX_IN  OMX_PTR pAppData,
    OMX_IN  OMX_CALLBACKTYPE* pCallBacks)
{

   LOGD("TIOMX_GetHandle\n");

	return OMX_GetHandle(
		pHandle,
		cComponentName,
		pAppData,
		pCallBacks);
}

OMX_API OMX_ERRORTYPE TIOMX_FreeHandle(
    OMX_IN  OMX_HANDLETYPE hComponent)
{
   LOGD("TIOMX_FreeHandle\n");

	return OMX_FreeHandle(hComponent);
}

OMX_API OMX_ERRORTYPE TIOMX_GetComponentsOfRole (
	OMX_IN      OMX_STRING role,
    OMX_INOUT   OMX_U32 *pNumComps,
    OMX_INOUT   OMX_U8  **compNames)
{

   LOGD("TIOMX_GetComponentsOfRole\n");

	return OMX_GetComponentsOfRole (
		role,
		pNumComps,
		compNames);
}

OMX_API OMX_ERRORTYPE TIOMX_GetRolesOfComponent (
	OMX_IN      OMX_STRING compName,
    OMX_INOUT   OMX_U32 *pNumRoles,
    OMX_OUT     OMX_U8 **roles)
{

   LOGD("TIOMX_GetRolesOfComponent\n");

	return OMX_GetRolesOfComponent (
		compName,
		pNumRoles,
		roles);
}

OMX_API OMX_ERRORTYPE TIOMX_SetupTunnel(
    OMX_IN  OMX_HANDLETYPE hOutput,
    OMX_IN  OMX_U32 nPortOutput,
    OMX_IN  OMX_HANDLETYPE hInput,
    OMX_IN  OMX_U32 nPortInput)
{

   LOGD("TIOMX_SetupTunnel\n");

	return OMX_SetupTunnel(
		hOutput,
		nPortOutput,
		hInput,
		nPortInput);
}

OMX_API OMX_ERRORTYPE   TIOMX_GetContentPipe(
    OMX_OUT OMX_HANDLETYPE *hPipe,
    OMX_IN OMX_STRING szURI)
{

   LOGD("TIOMX_GetContentPipe\n");

	//return OMX_GetContentPipe(
	//	hPipe,
	//	szURI);
	return 0;
}
#endif
