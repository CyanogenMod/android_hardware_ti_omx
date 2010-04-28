/*=========================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 *  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied.
 * ==========================================================================*/

/**
 *  @file  omx_rpc_utils.c
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
#include <string.h>
#include <stdio.h>

#include "omx_rpc.h"
#include "omx_rpc_internal.h"
#include "omx_rpc_utils.h"
#include <MultiProc.h>

extern char rcmservertable[MAX_PROC][MAX_SERVER_NAME_LENGTH];
extern char Core_Array[MAX_PROC][MAX_CORENAME_LENGTH];

/* ===========================================================================*/
/**
 * @name EMPTY-STUB
 * @brief
 * @param
 * @return
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_UTIL_GetTargetServerName(OMX_STRING ComponentName, OMX_STRING *ServerName)
{
    OMX_U8 count=0;
    OMX_U8 i=0;
    OMX_U8 servertable_idx=0;
    OMX_STRING str;
    char Core[MAX_CORENAME_LENGTH];

    DOMX_DEBUG("\n ENTERED Fucntion %s",__FUNCTION__);
    DOMX_DEBUG("\n CAlling Component Name %s", ComponentName);

    while(*ComponentName != '\0')
    {
        if(*ComponentName == '.')
        {
             count++;
             if(count == 2)
             {
                  ComponentName++;
                  str = ComponentName;

                  while(*str!='.')
                  {
                       Core[i] = *str;
                       i++;
                       str++;
                   }
                   Core[i] = '\0';
                   break;
              }

        }

        ComponentName++;
    }

    DOMX_DEBUG("\n CORE NAME RECOVERED: %s", Core);
    DOMX_DEBUG("\n____________________________________________________________");
    DOMX_DEBUG("\nRecovering Server Table Index");
       for(i=0;i<CORE_MAX;i++)
		{

	   	if(strcmp(Core,Core_Array[i])==0) {
			servertable_idx = i;
			DOMX_DEBUG("\n %s: Recovered Srever table index = %d",__FUNCTION__,i);
			break;
			}
		}
	*ServerName = (OMX_STRING)rcmservertable[servertable_idx];
	DOMX_DEBUG("\n ServerName recovered = %s", *ServerName);

	return RPC_OMX_ErrorNone;
}

/* ===========================================================================*/
/**
 * @name EMPTY-STUB
 * @brief
 * @param
 * @return
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_UTIL_GetLocalServerName(OMX_STRING ComponentName, OMX_STRING * ServerName)
{
/* Implementation returns only current core ID - But this is a place holder to abstract out the
default server and other additional servers available on the current core. This additional servers
should be available in the RPC global that is indexed using the calling component name*/
    OMX_U8 servertable_idx=0;

    servertable_idx = MultiProc_getId(NULL); //This can be replace with the mechanism to obtain new addition rcm servers
    *ServerName = rcmservertable[servertable_idx];
    return RPC_OMX_ErrorNone;
}


/* ===========================================================================*/
/**
 * @name EMPTY-STUB
 * @brief  the Api creates the RCM client on the target core(where the component sits).
This happens in the context of the Default RCM server on the target core.
The RCM server name to connect for this client will be the default RCM server on the calling core.
This can be provided as an option as the name that you pass in the string server is used as the RCM server name
input to the client create call.
@Default_RcmServer - The default rcm server on the target core
@Server - The name of the server on the calling core to connect to
 * @param
 * @return
 *
 */
/* ===========================================================================*/
/*
RPC_OMX_ERRORTYPE RPC_GetTargetClient(OMX_STRING Default_RcmServer, OMX_STRING server, rcmhHndl)
{


}
*/

/* ===========================================================================*/
/**
 * @name EMPTY-STUB
 * @brief
 * @param
 * @return
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_MapBuffer(OMX_U32 mappedBuffer )
{
    DOMX_DEBUG("\nEntering: %s", __FUNCTION__);
    DOMX_DEBUG("\nEmpty implementation ");
    //PlaceHolder for Mapping Buffers - Cuurently no implementation here
	return RPC_OMX_ErrorNone;
}

/* ===========================================================================*/
/**
 * @name EMPTY-STUB
 * @brief
 * @param
 * @return
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_UnMapBuffer(OMX_U32 mappedBuffer )
{
    DOMX_DEBUG("\nEntering: %s", __FUNCTION__);
    DOMX_DEBUG("\nEmpty implementation ");
    //PlaceHolder for UnMapping Buffers - Cuurently no implementation here
	return RPC_OMX_ErrorNone;
}

/* ===========================================================================*/
/**
 * @name EMPTY-STUB
 * @brief
 * @param
 * @return
 *
 */
/* ===========================================================================*/
RPC_OMX_ERRORTYPE RPC_FlushBuffer(OMX_U8 * pBuffer, OMX_U32 size )

{
	DOMX_DEBUG("\nEntering: %s", __FUNCTION__);
	DOMX_DEBUG("\n Empty implementation ");
	//PlaceHolder for Flushing Buffers - Cuurently no implementation here
    return RPC_OMX_ErrorNone;
}
