
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

 /* OS-specific headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

//#include <pthread.h>
//#include <semaphore.h>

/* Standard headers */
#include <Std.h>


#include <SysMgr.h>
#include <ProcMgr.h>

//For the new shared heap
//#define BUF_HEAP

#ifdef BUF_HEAP
#include <HeapBuf.h>
#include <GatePeterson.h>

#define GATEPETERSONMEMSIZE         0x1000

#define NUMBLOCKS 22
#define MSGSIZE (120 *1024)
#define SHAREDMEMSIZE2 0xC00000
#endif


/* App defines */
#define MAX_CREATE_ATTEMPTS         0xFFFF
#define LOOP_COUNT                  4
#define JOB_COUNT                   4
#define MAX_NAME_LENGTH             32
#define MSGSIZE                     256
#define NSRN_NOTIFY_EVENTNO         7
#define TRANSPORT_NOTIFY_EVENTNO    8
#define NSRN_NOTIFY_EVENTNO1        22
#define TRANSPORT_NOTIFY_EVENTNO1   23
#define HEAPID_SYSM3                0
#define HEAPNAME_SYSM3              "SysMgrHeap0"
#define HEAPID_APPM3                1
#define HEAPNAME_APPM3              "SysMgrHeap1"
#define RCMSERVER_NAME              "RcmSvr_Mpu:0"
#define SYSM3_SERVER_NAME           "RcmSvr_SysM3:0"
#define APPM3_SERVER_NAME           "RcmSvr_AppM3:0"
#define MPU_PROC_NAME               "MPU"
#define SYSM3_PROC_NAME             "SysM3"
#define APPM3_PROC_NAME             "AppM3"

/*
 *  Definitions for additional application specified HeapBuf.
 */
#define SHAREDMEM2             0x81300000
//#define SHAREDMEMSIZE2         0xC00000
#define APP_HEAP_SHAREDBUF     0x2000
#define APP_HEAP_HEAPNAME      "ApplicationHeap0"
#define APP_HEAP_BLOCKSIZE     256

/*
 *  The shared memory is going to split between
 *  Notify:             0xA0000000 - 0xA0004000
 *  Gatepeterson:       0xA0004000 - 0xA0005000
 *  HEAPBUFMEM:         0xA0005000 - 0xA0009000
 *  NSRN_MEM:           0xA0009000 - 0xA000A000
 *  transport:          0xA000A000 - 0xA000C000
 *  MESSAGEQ_NS_MEM:    0xA000C000 - 0xA000D000
 *  HEAPBUF_NS_MEM:     0xA000D000 - 0xA000E000
 *  HEAPBUFMEM1:        0xA000E000 - 0xA0012000
 *  GATEPETERSONMEM1:   0xA0012000 - 0xA0013000
 *  HEAPMEM:            0xA0013000 - 0xA0014000
 *  HEAPMEM1:           0xA0014000 - 0xA0015000
 *  List:               0xA0015000 - 0xA0016000
 *  List1:              0xA0016000 - 0xA0017000
 *  HEAPMBMEM_CTRL:     0xA0017000 - 0xA0018000
 *  HEAPMBMEM:          0xA0018000 - 0xA001B000
 *  HEAPMBMEM1_CTRL:    0xA001B000 - 0xA001C000
 *  HEAPMBMEM1:         0xA001C000 - 0xA001F000
 */

/* Shared Memory Area for MPU - SysM3 */
#define SHAREDMEM                   0xA0000000
#define SHAREDMEMSIZE               0x54000

/* Shared Memory Area for MPU - AppM3 */
#define SHAREDMEM1                  0xA0055000
#define SHAREDMEMSIZE1              0x54000

/* Memory for the Notify Module */
#define NOTIFYMEM                   0xA0000000
#define NOTIFYMEMSIZE               0x4000

/* Memory a GatePeterson instance */
#define GATEPETERSONMEM             (NOTIFYMEM + NOTIFYMEMSIZE)
#define GATEPETERSONMEMSIZE         0x1000

/* Memory a HeapBuf instance */
#define HEAPBUFMEM                  (GATEPETERSONMEM + GATEPETERSONMEMSIZE)
#define HEAPBUFMEMSIZE              0x4000

/* Memory for NameServerRemoteNotify */
#define NSRN_MEM                    (HEAPBUFMEM + HEAPBUFMEMSIZE)
#define NSRN_MEMSIZE                0x1000

/* Memory a Transport instance */
#define TRANSPORTMEM                (NSRN_MEM + NSRN_MEMSIZE)
#define TRANSPORTMEMSIZE            0x2000

/* Memory for second gatepeterson */
#define MESSAGEQ_NS_MEM             (TRANSPORTMEM + TRANSPORTMEMSIZE)
#define MESSAGEQ_NS_MEMSIZE         0x1000

/* Memory for HeapBuf's NameServer instance */
#define HEAPBUF_NS_MEM              (MESSAGEQ_NS_MEM + MESSAGEQ_NS_MEMSIZE)
#define HEAPBUF_NS_MEMSIZE          0x1000

/* Memory a HeapBuf instance for RCM TestCase*/
#define HEAPBUFMEM1                 (HEAPBUF_NS_MEM + HEAPBUF_NS_MEMSIZE)
#define HEAPBUFMEMSIZE1             0x4000

#define GATEPETERSONMEM1            (HEAPBUFMEM1 + HEAPBUFMEMSIZE1)
#define GATEPETERSONMEMSIZE1        0x1000

/* Memory for the Notify Module */
#define HEAPMEM                     (GATEPETERSONMEM1 + GATEPETERSONMEMSIZE1)
#define HEAPMEMSIZE                 0x1000

#define HEAPMEM1                    (HEAPMEM + HEAPMEMSIZE)
#define HEAPMEMSIZE1                0x1000

#define List                        (HEAPMEM1 + HEAPMEMSIZE1)
#define ListSIZE                    0x1000

#define List1                       (List + ListSIZE)
#define ListSIZE1                   0x1000

/* Memory a HeapMultiBuf instance */
#define HEAPMBMEM_CTRL              (List1 + ListSIZE1)
#define HEAPMBMEMSIZE_CTRL          0x1000

#define HEAPMBMEM_BUFS              (HEAPMBMEM_CTRL + HEAPMBMEMSIZE_CTRL)
#define HEAPMBMEMSIZE_BUFS          0x3000

#define HEAPMBMEM1_CTRL             (HEAPMBMEM_BUFS + HEAPMBMEMSIZE_BUFS)
#define HEAPMBMEMSIZE1_CTRL         0x1000

#define HEAPMBMEM1_BUFS             (HEAPMBMEM1_CTRL + HEAPMBMEMSIZE1_CTRL)
#define HEAPMBMEMSIZE1_BUFS         0x3000
int ipc_finalize();

ProcMgr_Handle                  procMgrHandle = NULL;
Char *                          remoteServerName = NULL;
UInt16                          remoteId;

//#ifdef BUF_HEAP
HeapBuf_Handle                  heapHandle = NULL;
GatePeterson_Handle             gateHandle_client = NULL;
//#endif

Int ipc_setup (int setup)
{
    Char *                          procName = NULL;
    UInt16                          procId;
    UInt32                          shAddrBase;
    UInt32                          shAddrBase1;
    UInt32                          curShAddr;
    UInt32                          nsrnEventNo;
    UInt32                          mqtEventNo;
//#if defined(SYSLINK_USE_LOADER) || defined(SYSLINK_USE_SYSMGR)
    UInt32                          entry_point = 0;
    ProcMgr_StartParams             start_params;
//#endif

//#if defined(SYSLINK_USE_SYSMGR)
    SysMgr_Config                   config;
//#else
    /*UInt32                          curShAddr_temp;
    Notify_Config                   notifyConfig;
    NotifyDriverShm_Config          notifyDrvShmConfig;
    NameServerRemoteNotify_Config   nsrConfig;
    SharedRegion_Config             sharedRegConfig;
    ListMPSharedMemory_Config       listMPSharedConfig;
    GatePeterson_Config             gpConfig;
    HeapBuf_Config                  heapbufConfig;
    MessageQTransportShm_Config     msgqTransportConfig;
    MessageQ_Config                 messageqConfig;
    MultiProc_Config                multiProcConfig;
    NotifyDriverShm_Params          notifyShmParams;
    GatePeterson_Params             gateParams;
    NameServerRemoteNotify_Params   nsrParams;
    HeapBuf_Params                  heapbufParams;
    MessageQTransportShm_Params     msgqTransportParams;*/
//#endif /* if defined(SYSLINK_USE_SYSMGR) */
    Int                             status = 0;
    Int                             callFinalIfErr = 0;

#ifdef BUF_HEAP
	UInt32                          shAddrBase2;
	GatePeterson_Params             gateParams;
    HeapBuf_Params                  heapbufParams;
#endif

    printf ("ipc_setup: Setup IPC componnets \n");

    
        printf ("ipc_setup: RCM test with RCM client and server on "
                    "App M3\n\n");
        remoteServerName = APPM3_SERVER_NAME;
        procName = APPM3_PROC_NAME;
        nsrnEventNo = NSRN_NOTIFY_EVENTNO1;
        mqtEventNo = TRANSPORT_NOTIFY_EVENTNO1;
    





    SysMgr_getConfig (&config);
    status = SysMgr_setup (&config);
    if (status < 0) {
        printf ("Error in SysMgr_setup [0x%x]\n", status);
        goto leave;
    }
    else
        callFinalIfErr = 1;


    if (status >= 0) {
        procId = MultiProc_getId (SYSM3_PROC_NAME);
        remoteId = MultiProc_getId (procName);

        /* Open a handle to the ProcMgr instance. */
        status = ProcMgr_open (&procMgrHandle,
                           procId);
        if (status < 0) {
            printf ("Error in ProcMgr_open [0x%x]\n", status);
            goto leave;
        }
        else {
            printf ("ProcMgr_open Status [0x%x]\n", status);
            /* Get the address of the shared region in kernel space. */
            status = ProcMgr_translateAddr (procMgrHandle,
                                            (Ptr) &shAddrBase,
                                            ProcMgr_AddrType_MasterUsrVirt,
                                            (Ptr) SHAREDMEM,
                                            ProcMgr_AddrType_SlaveVirt);
            if (status < 0) {
                printf ("Error in ProcMgr_translateAddr [0x%x]\n",
                             status);
                goto leave;
            }
            else {
                printf ("Virt address of shared address base #1:"
                             " [0x%x]\n", shAddrBase);
            }

            if (status >= 0) {
                /* Get the address of the shared region in kernel space. */
                status = ProcMgr_translateAddr (procMgrHandle,
                                                (Ptr) &shAddrBase1,
                                                ProcMgr_AddrType_MasterUsrVirt,
                                                (Ptr) SHAREDMEM1,
                                                ProcMgr_AddrType_SlaveVirt);
                if (status < 0) {
                    printf ("Error in ProcMgr_translateAddr [0x%x]\n",
                                 status);
                    goto leave;
                }
                else {
                    printf ("Virt address of shared address base #2:"
                                 " [0x%x]\n", shAddrBase1);
                }
            }
#ifdef BUF_HEAP
			if (status >= 0) {
                /* Get the address of the shared region in kernel space. */
                status = ProcMgr_translateAddr (procMgrHandle,
                                                (Ptr) &shAddrBase2,
                                                ProcMgr_AddrType_MasterUsrVirt,
                                                (Ptr) SHAREDMEM2,
                                                ProcMgr_AddrType_SlaveVirt);
                if (status < 0) {
                    printf ("Error in ProcMgr_translateAddr [0x%x]\n",
                                 status);
                    goto leave;
                }
                else {
                    printf ("Virt address of shared address base #3:"
                                 " [0x%x]\n", shAddrBase2);
                }
            }
#endif
        }
    }

    if (status >= 0) {
        curShAddr = shAddrBase;
        /* Add the region to SharedRegion module. */
        status = SharedRegion_add (0,
                                   (Ptr) curShAddr,
                                   SHAREDMEMSIZE);
        if (status < 0) {
            printf ("ipc_setup: Error in SharedRegion_add [0x%x]\n",
                            status);
            goto leave;
        }
        else {
            printf ("SharedRegion_add [0x%x]\n", status);
        }
    }

    if (status >= 0) {
        /* Add the region to SharedRegion module. */
        status = SharedRegion_add (1,
                                   (Ptr) shAddrBase1,
                                   SHAREDMEMSIZE1);
        if (status < 0) {
            printf ("ipc_setup: Error in SharedRegion_add1 [0x%x]\n",
                            status);
            goto leave;
        }
        else {
            printf ("SharedRegion_add1 [0x%x]\n", status);
        }
    }
#ifdef BUF_HEAP
	if (status >= 0) {
        /* Add the region to SharedRegion module. */
        status = SharedRegion_add (2,
                                   (Ptr) shAddrBase2,
                                   SHAREDMEMSIZE2);
        if (status < 0) {
            printf ("ipc_setup: Error in SharedRegion_add2 [0x%x]\n",
                            status);
            goto leave;
        }
        else {
            printf ("SharedRegion_add2 [0x%x]\n", status);
        }
    }
#endif

//If setup = 1 then base images have to be loaded
if(setup == 1)
{
//#if defined(SYSLINK_USE_SYSMGR) ||(SYSLINK_USE_LOADER)
    start_params.proc_id = MultiProc_getId (SYSM3_PROC_NAME);
    status = ProcMgr_start (procMgrHandle, entry_point, &start_params);
    printf ("SYSM3: ProcMgr_start Status [0x%x]\n", status);
//#endif

    /*FIXME: */

//#if defined(SYSLINK_USE_SYSMGR) ||(SYSLINK_USE_LOADER)
        start_params.proc_id = MultiProc_getId (APPM3_PROC_NAME);
        status = ProcMgr_start (procMgrHandle, entry_point, &start_params);
        printf ("APPM3: ProcMgr_start Status [0x%x]\n", status);
//#endif
}

#ifdef BUF_HEAP
GatePeterson_Params_init (gateHandle_client, &gateParams);
    gateParams.sharedAddrSize = GatePeterson_sharedMemReq (&gateParams);
    printf ("ipc_setup: Memory required for GatePeterson instance "
                    "[0x%x] bytes \n",
                    gateParams.sharedAddrSize);

    do {
        gateParams.sharedAddr     = (Ptr)(shAddrBase2);
        status = GatePeterson_open (&gateHandle_client,
                                    &gateParams);
    }
    while ((status == GATEPETERSON_E_NOTFOUND) ||
            (status == GATEPETERSON_E_VERSION));

    if (status < 0) {
        printf ("ipc_setup: Error in GatePeterson_open [0x%x]\n",
                        status);
        goto leave;
    }
    else {
        printf ("ipc_setup: GatePeterson_open Status [0x%x]\n",
                        status);
    }

    if (status >= 0) {
        /* Increment the offset for the next allocation */
        curShAddr += GATEPETERSONMEMSIZE;

        /* Create the heap. */
        HeapBuf_Params_init(NULL, &heapbufParams);
        heapbufParams.name           = "MPU-APP_BufHeap";

        heapbufParams.sharedAddr     = (Ptr)(shAddrBase2 + 0x1000);

        heapbufParams.align          = 128;
        heapbufParams.numBlocks      = NUMBLOCKS;
        heapbufParams.blockSize      = MSGSIZE;
        heapbufParams.gate           = (Gate_Handle) gateHandle_client;
        heapbufParams.sharedAddrSize = HeapBuf_sharedMemReq (&heapbufParams,
                    &heapbufParams.sharedBufSize);
        heapbufParams.sharedBuf      = (Ptr)(shAddrBase2 + 0x2000);


        status = HeapBuf_open (&heapHandle, &heapbufParams);
        if(status < 0) {
            printf ("ipc_setup: Error in HeapBuf_create\n");
            goto leave;
        }
        else {
            printf ("ipc_setup: HeapBuf_create Handle [0x%x]\n",
                            heapHandle);
        }
//#define HEAPID 2
        /* Register this heap with MessageQ */
     //   MessageQ_registerHeap (heapHandle, HEAPID);
    }
#endif
leave:
    if(status < 0 && callFinalIfErr == 1)
        ipc_finalize();
    printf ("ipc_setup: Leaving ipc_setup()\n");
    return status;
}

int ipc_finalize()
{    
    Int status = 0;
    Int retstatus = 0;
    
#ifdef BUF_HEAP
  if(heapHandle)
  {
    status = HeapBuf_close (&heapHandle);  
    heapHandle = NULL; 
    if(status < 0) 
    {
        retstatus = status;
        printf ("\nError in HeapBuf_close [0x%x]\n", status);
    }
  }
  if(gateHandle_client)
  {
    status = GatePeterson_close (&gateHandle_client);   
    gateHandle_client = NULL;
    if(status < 0) 
    {
        retstatus = status;
        printf ("\nError in GatePeterson_close [0x%x]\n", status);
    }
  }    
#endif    
  if(procMgrHandle)
  {
    status = ProcMgr_close(&procMgrHandle);
    procMgrHandle = NULL;
    if (status < 0) 
    {
        retstatus = status;
        printf ("\nError in ProcMgr_close [0x%x]\n", status);
    }
  }
    printf("\nClosing sysmgr\n");
    status = SysMgr_destroy();
    if (status < 0) 
    {
        retstatus = status;
        printf ("Error in SysMgr_destroy [0x%x]\n", status);
    }
    return retstatus;
}


