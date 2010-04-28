#include <stdio.h>
#include <omx_rpc.h>
#include <omx_rpc_utils.h>
#ifdef _Android
#include <utils/Log.h>
#undef LOG_TAG
#define LOG_TAG "MMPLAT"
//#define DOMX_DEBUG LOGD
#define TIMM_OSAL_Error LOGE
#endif
extern int ipc_setup(int);
extern int ipc_finalize();

OMX_U8 CHIRON_IPC_FLAG=1;


//todo: synchronize access
static int cnt = 0;


int mmplatform_init(int setup)
{
    int nRetVal = 0;
	cnt++;
	if (cnt > 1)
		return nRetVal;

	DOMX_DEBUG("Calling ipc setup\n");
    nRetVal = ipc_setup(setup);
    if(nRetVal < 0)
    {
        TIMM_OSAL_Error("\nipc_setup failed\n");
        goto leave;
    }
    else
    {
        DOMX_DEBUG("\nipc_setup successful\n");
        nRetVal = 0;
    }

    DOMX_DEBUG("\nCreating Rcm Server\n");
    nRetVal = (int)RPC_ModInit();
    if(nRetVal != 0)
    {
        TIMM_OSAL_Error("RPC Mod Init failed");
        nRetVal = -1;
    }
    else
        DOMX_DEBUG("\nRCM Server created\n");
leave:
    return nRetVal;
}

int mmplatform_deinit()
{
    int nRetVal = 0;
	cnt--;
	if (cnt > 0)
		return nRetVal;

    DOMX_DEBUG("\nCalling RPC Mod deinit\n");
    nRetVal = (int)RPC_ModDeInit();
    if(nRetVal != 0)
    {
        TIMM_OSAL_Error("RPC_ModDeInit failed");
        nRetVal = -1;
    }
    else
        DOMX_DEBUG("\nRPC mod deinit done\n");
    if(ipc_finalize() < 0)
    {
        TIMM_OSAL_Error("\nIPC finalize failed\n");
        nRetVal = -1;
    }
    else
        TIMM_OSAL_Info("\nIPC Deinitialized\n");
    return nRetVal;
}
