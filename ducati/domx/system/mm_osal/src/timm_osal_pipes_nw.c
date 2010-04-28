/* ====================================================================
 *   Texas Instruments OMAP(TM) Platform Software
 * (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied.
 * ==================================================================== */
/*
*   @file  timm_osal_pipes.c
*   This file contains methods that provides the functionality
*   for creating/using Nucleus pipes.
*
*  @path \
*
*/
/* -------------------------------------------------------------------------- */
/* =========================================================================
 *!
 *! Revision History
 *! ===================================
 *! 05- Nov -2008 Shree Harsha Maiya updated version
 *! 0.1: Created the first draft version, ksrini@ti.com
 * ========================================================================= */

/******************************************************************************
* Includes
******************************************************************************/


#include "timm_osal_types.h"
#include "timm_osal_trace.h"
#include "timm_osal_error.h"
#include "timm_osal_memory.h"


#include <stdio.h>

/*For Message Queue*/
#include <mqueue.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>

#define _XOPEN_SOURCE 600
/*
* TIMM_OSAL_PIPE structure define the OSAL pipe
*/
typedef struct TIMM_OSAL_PIPE
{
	mqd_t mqDes;
	TIMM_OSAL_U8 name[8];
	TIMM_OSAL_U32  pipeSize;
	TIMM_OSAL_U32  messageSize;
	TIMM_OSAL_U8   isFixedMessage;
	
} TIMM_OSAL_PIPE;

TIMM_OSAL_U32 queue_count = 0;


/******************************************************************************
* Function Prototypes
******************************************************************************/

/* ========================================================================== */
/**
* @fn TIMM_OSAL_CreatePipe function
* 
* 
* DESCRIPTION         
* 
*     This function create a new Message Queue. The Message Queue length is determined by the caller.
* 
* CALLED BY
*
* 
* CALLS    
* 
*     msgget         				Create a Message Queue
*     TIMM_OSAL_Malloc			Allocate memory      
*     TIMM_OSAL_Memset         			Set memory to particular value
*     TIMM_OSAL_Error  			Print Error Statements
*     TIMM_OSAL_Free     			Free Allocated Memory
*
* INPUTS
* 
*     pPipe        			Pipe control block pointer   
*     pipeSize       			Size of the pipe to be created
*     messageSize     				Size of the message in case it is a fixed size message pipe  
*     isFixedMessage    			Whether pipe has fixed or variable sized messages
* 
* OUTPUTS   
* 
*     TIMM_OSAL_ERR_ALLOC        	If memory allocation failed    
*     TIMM_OSAL_ERR_PARAMETER		If bad parameter passed to function
*     TIMM_OSAL_ERR_NONE         	If functionality behaves properly
* 
*
* CONSTRAINTS
*       
*  	MSGMNI: 	System wide maximum number of message queues: policy dependent(on Linux, this 
*				limit can be read and modified via /proc/sys/kernel/msgmni).
* 	MSGMNB:	System limit to message queue size. This can be changed by msgct() call.
*
/* ========================================================================== */

TIMM_OSAL_ERRORTYPE TIMM_OSAL_CreatePipe (TIMM_OSAL_PTR *pPipe,
  TIMM_OSAL_U32  pipeSize,
  TIMM_OSAL_U32  messageSize,
  TIMM_OSAL_U8   isFixedMessage)
{
    TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;
    TIMM_OSAL_PIPE *pHandle = TIMM_OSAL_NULL;
	struct mq_attr attr;
		
    /*Allocate memory for the OSAL Message Queue Structure*/
    pHandle = (TIMM_OSAL_PIPE *)TIMM_OSAL_Malloc(sizeof(TIMM_OSAL_PIPE), 0, 0, 0);

	/*Check for allocation failure*/
    if (TIMM_OSAL_NULL == pHandle) {
        bReturnStatus = TIMM_OSAL_ERR_ALLOC;
        goto EXIT;
    }

	/*Initialize the allocated memory*/
    TIMM_OSAL_Memset(pHandle, 0x0, sizeof(TIMM_OSAL_PIPE));

	sprintf(pHandle->name,"TIP%2d", queue_count);

	/* Flags: 0 or O_NONBLOCK, */
	attr.mq_flags = 0;

	/* Max. number of messages on queue */
	attr.mq_maxmsg = (pipeSize / messageSize) + 10;

	/* Max. message size (bytes) */
	attr.mq_msgsize = messageSize;	 

    /*Debugging*/
	TIMM_OSAL_Trace("msgsize: %d",attr.mq_msgsize);
	TIMM_OSAL_Trace("maxmsg : %d",attr.mq_maxmsg);
	TIMM_OSAL_Trace("pipesize: %d",pipeSize);
	TIMM_OSAL_Trace("Message Size : %d", messageSize);

	/*Open a queue with the attribute structure*/
	pHandle->mqDes = mq_open(pHandle->name, O_RDWR | O_CREAT, 0666, &attr);

	/*mq_open returns -1 on error with errno indicating the error*/	
	if (NO_SUCCESS == pHandle->mqDes) {
		TIMM_OSAL_Trace("errno : %d",errno);
		switch(errno){
			SWITCH_CASE(EACCES, TIMM_OSAL_ERR_NO_PERMISSIONS,
			"CreatePipe: Caller does not have permission to open it in the specified mode") 
			SWITCH_CASE(EINVAL, TIMM_OSAL_ERR_PARAMETER,
			"CreatePipe: struct mq_attr attr->mq_maxmsg or attr->mq_msqsize was invalid") 
			SWITCH_CASE(EMFILE, TIMM_OSAL_ERR_SYSTEM_LIMIT_EXCEEDED,
			"CreatePipe: The process already has the maximum number of files and message queues open") 
			SWITCH_CASE(ENAMETOOLONG, TIMM_OSAL_ERR_SYSTEM_LIMIT_EXCEEDED,
			"CreatePipe: name was too long") 
			SWITCH_CASE(ENFILE, TIMM_OSAL_ERR_SYSTEM_LIMIT_EXCEEDED,
			"CreatePipe: System limit on the total no. of open files and message queues has been reached") 
			SWITCH_CASE(ENOMEM, TIMM_OSAL_ERR_OUT_OF_RESOURCE,
			"CreatePipe: Insufficient memory") 
			SWITCH_CASE(ENOSPC, TIMM_OSAL_ERR_OUT_OF_RESOURCE,
			"CreatePipe: Caller does not have permission to open it in the specified mode") 
			SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
			"CreatePipe: Error") 
		}	
		goto EXIT;
	}

    ++queue_count;

	/*Store the other parameters supplied in the OSAL Message Queue Structure*/
    pHandle->pipeSize = pipeSize;
    pHandle->messageSize = messageSize;
	pHandle->isFixedMessage = isFixedMessage;

	/*Pass the 'malloced' handle to the handle passed by application*/
    *pPipe = (TIMM_OSAL_PTR ) pHandle ;

	/*All went well*/
	bReturnStatus = TIMM_OSAL_ERR_NONE;

EXIT:
    if ((bReturnStatus != TIMM_OSAL_ERR_NONE) && (TIMM_OSAL_NULL != pHandle)) {
       TIMM_OSAL_Free(pHandle);       
    }
    return bReturnStatus;
}



/* ========================================================================== */
/**
* @fn TIMM_OSAL_DeletePipe function
*
* 
*/
/* ========================================================================== */

TIMM_OSAL_ERRORTYPE TIMM_OSAL_DeletePipe (TIMM_OSAL_PTR pPipe)
{
    TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_NONE;
    mqd_t status = -1;
    TIMM_OSAL_PIPE *pHandle = (TIMM_OSAL_PIPE *)pPipe;

	/*Check for bad pointer*/
	if(TIMM_OSAL_NULL == pHandle) {
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}

	status =  mq_close(pHandle->mqDes);

	/*On error, -1 is returned with errno indicating the error*/
    if (NO_SUCCESS == status) {
		switch(errno){		
			SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
			"DeletePipe: The descriptor specified in mqdes is invalid") 
			SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
			"DeletePipe: Error") 
		}	
	}
	else
		queue_count--;
 
    TIMM_OSAL_Free(pHandle);
EXIT:
    return bReturnStatus;
}



/* ========================================================================== */
/**
* @fn TIMM_OSAL_WriteToPipe function
*pMessage: 	The mtext field is an array (or other structure) whose size is specified by the third parameter, 
*			(a non-negative integer value), in msgsnd()
*/
/* ========================================================================== */

TIMM_OSAL_ERRORTYPE TIMM_OSAL_WriteToPipe (TIMM_OSAL_PTR pPipe,
   void *pMessage,
   TIMM_OSAL_U32 size,
   TIMM_OSAL_S32 timeout)
{
    TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;
    mqd_t status = -1;
	struct mq_attr attr, old_attr;
	TIMM_OSAL_U32 prio;
	struct timespec abstimeout;
	struct timeval now;
	TIMM_OSAL_U32 timeout_us;
	
    TIMM_OSAL_PIPE *pHandle = (TIMM_OSAL_PIPE *)pPipe;

	/*Check if bad pointer*/
	if(TIMM_OSAL_NULL == pHandle) {
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}

	if(TIMM_OSAL_NULL == pMessage) {
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}

	status = mq_getattr(pHandle->mqDes, &attr);

	/*On error, -1 is returned with errno indicating the error*/	
	if (NO_SUCCESS == pHandle->mqDes) {
		switch(errno){
			SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
			"WriteToPipe: The descriptor specified in mqdes is invalid") 
			SWITCH_CASE(EINVAL, TIMM_OSAL_ERR_PARAMETER,
			"WriteToPipe: newattr->mq_flags contained set bits other than O_NONBLOCK") 
			SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
			"WriteToPipe: Error") 
		}	
		goto EXIT;
	}	

	/*Set the priority to indicate the normal message*/
	prio = NORMAL_MESSAGE;

	switch(timeout){
		case TIMM_OSAL_NO_SUSPEND:
			
			attr.mq_flags = O_NONBLOCK ;			
			status = mq_setattr (pHandle->mqDes, &attr, &old_attr);	

			/*On error, -1 is returned with errno indicating the error*/	
			if (NO_SUCCESS == pHandle->mqDes) {
				switch(errno){
					SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
					"WriteToPipe: The descriptor specified in mqdes is invalid") 
					SWITCH_CASE(EINVAL, TIMM_OSAL_ERR_PARAMETER,
					"WriteToPipe: newattr->mq_flags contained set bits other than O_NONBLOCK") 
					SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
					"WriteToPipe: Error") 
				}	
				goto EXIT;
			}			   

			/*Send the message to the queue*/
			status = mq_send(pHandle->mqDes, (const char *)pMessage, size, prio);

			/*mq_send returns -1 on error with errno indicating the error*/ 
			if (NO_SUCCESS == pHandle->mqDes) {
				switch(errno){
					SWITCH_CASE(EAGAIN, TIMM_OSAL_ERR_PIPE_EMPTY,
					"WriteToPipe: The queue was empty, and the O_NONBLOCK flag was set") 
					SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
					"WriteToPipe: The descriptor specified in mqdes was invalid") 
					SWITCH_CASE(EMSGSIZE, TIMM_OSAL_ERR_SYSTEM_LIMIT_EXCEEDED,
					"WriteToPipe: msg_len was greater than the mq_msgsize attribute of the message queue") 
					SWITCH_CASE(EINTR, TIMM_OSAL_ERR_SIGNAL_CAUGHT,
					"WriteToPipe: The call was interrupted by a signal handler") 
					SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
					"WriteToPipe: Error") 
				}	
				goto EXIT;
			}

			/*Reset the old blocking attributes*/
			status = mq_setattr (pHandle->mqDes, &old_attr, 0);	

			/*On error, -1 is returned with errno indicating the error*/	
			if (NO_SUCCESS == pHandle->mqDes) {
				switch(errno){
					SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
					"WriteToPipe: The descriptor specified in mqdes is invalid") 
					SWITCH_CASE(EINVAL, TIMM_OSAL_ERR_PARAMETER,
					"WriteToPipe: newattr->mq_flags contained set bits other than O_NONBLOCK") 
					SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
					"WriteToPipe: Error") 
				}	
				goto EXIT;
			}			
			break;				
		case TIMM_OSAL_SUSPEND:

			/*Send the message to the queue*/
			status = mq_send(pHandle->mqDes, (const char *)pMessage, size, prio);

			/*mq_send returns -1 on error with errno indicating the error*/ 
			if (NO_SUCCESS == pHandle->mqDes) {
				switch(errno){
					SWITCH_CASE(EAGAIN, TIMM_OSAL_ERR_PIPE_EMPTY,
					"WriteToPipe: The queue was empty, and the O_NONBLOCK flag was set") 
					SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
					"WriteToPipe: The descriptor specified in mqdes was invalid") 
					SWITCH_CASE(EMSGSIZE, TIMM_OSAL_ERR_SYSTEM_LIMIT_EXCEEDED,
					"WriteToPipe: msg_len was greater than the mq_msgsize attribute of the message queue") 
					SWITCH_CASE(EINTR, TIMM_OSAL_ERR_SIGNAL_CAUGHT,
					"WriteToPipe: The call was interrupted by a signal handler") 
					SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
					"WriteToPipe: Error") 
				}	
				goto EXIT;
			}

			break;		
		default:

			/* Calculate uTimeOutMsec in terms of the absolute time. uTimeOutMsec is in milliseconds*/
			gettimeofday(&now, NULL);
			timeout_us = now.tv_usec + 1000 * timeout;
			abstimeout.tv_sec = now.tv_sec + timeout_us / 1000000;
			abstimeout.tv_nsec = (timeout_us % 1000000) * 1000;

			status = mq_timedsend(pHandle->mqDes, (const char *)pMessage, size, prio, &abstimeout);

			/*mq_send returns -1 on error with errno indicating the error*/ 
			if (NO_SUCCESS == pHandle->mqDes) {
				switch(errno){
					SWITCH_CASE(EAGAIN, TIMM_OSAL_ERR_PIPE_EMPTY,
					"WriteToPipe: The queue was empty, and the O_NONBLOCK flag was set") 
					SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
					"WriteToPipe: The descriptor specified in mqdes was invalid") 
					SWITCH_CASE(EMSGSIZE, TIMM_OSAL_ERR_SYSTEM_LIMIT_EXCEEDED,
					"WriteToPipe: msg_len was greater than the mq_msgsize attribute of the message queue") 
					SWITCH_CASE(EINTR, TIMM_OSAL_ERR_SIGNAL_CAUGHT,
					"WriteToPipe: The call was interrupted by a signal handler") 
					SWITCH_CASE(EINVAL, TIMM_OSAL_ERR_PARAMETER,
					"WriteToPipe: abs_timeout was invalid") 
					SWITCH_CASE(ETIMEDOUT, TIMM_OSAL_ERR_TIMEOUT,
					"WriteToPipe: The call timed out before a message could be transferred") 					
					SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
					"WriteToPipe: Error") 
				}	
				goto EXIT;
			}			
		break;			
	}	
		
	/*All is well*/
	bReturnStatus = TIMM_OSAL_ERR_NONE;
	
EXIT:
    return bReturnStatus;
}



/* ========================================================================== */
/**
* @fn TIMM_OSAL_WriteToFrontOfPipe function
*
* 
*/
/* ========================================================================== */

TIMM_OSAL_ERRORTYPE TIMM_OSAL_WriteToFrontOfPipe (TIMM_OSAL_PTR pPipe,
void *pMessage,
TIMM_OSAL_U32 size,
TIMM_OSAL_S32 timeout)
{
	TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;
	mqd_t status = -1;
	struct mq_attr attr, old_attr;
	TIMM_OSAL_U32 prio;
	struct timespec abstimeout;
	struct timeval now;
	TIMM_OSAL_U32 timeout_us;
	
	TIMM_OSAL_PIPE *pHandle = (TIMM_OSAL_PIPE *)pPipe;

	/*Check if bad pointer*/
	if(TIMM_OSAL_NULL == pHandle) {
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}

	if(TIMM_OSAL_NULL == pMessage) {
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}

	status = mq_getattr(pHandle->mqDes, &attr);

	/*On error, -1 is returned with errno indicating the error*/	
	if (NO_SUCCESS == pHandle->mqDes) {
		switch(errno){
			SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
			"WriteToPipe: The descriptor specified in mqdes is invalid") 
			SWITCH_CASE(EINVAL, TIMM_OSAL_ERR_PARAMETER,
			"WriteToPipe: newattr->mq_flags contained set bits other than O_NONBLOCK") 
			SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
			"WriteToPipe: Error") 
		}	
		goto EXIT;
	}	

	/*Set the priority to indicate the normal message*/
	prio = URGENT_MESSAGE;

	switch(timeout){
		case TIMM_OSAL_NO_SUSPEND:
			
			attr.mq_flags = O_NONBLOCK ;			
			status = mq_setattr (pHandle->mqDes, &attr, &old_attr); 

			/*On error, -1 is returned with errno indicating the error*/	
			if (NO_SUCCESS == pHandle->mqDes) {
				switch(errno){
					SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
					"WriteToPipe: The descriptor specified in mqdes is invalid") 
					SWITCH_CASE(EINVAL, TIMM_OSAL_ERR_PARAMETER,
					"WriteToPipe: newattr->mq_flags contained set bits other than O_NONBLOCK") 
					SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
					"WriteToPipe: Error") 
				}	
				goto EXIT;
			}			   

			/*Send the message to the queue*/
			status = mq_send(pHandle->mqDes, (const char *)pMessage, size, prio);

			/*mq_send returns -1 on error with errno indicating the error*/ 
			if (NO_SUCCESS == pHandle->mqDes) {
				switch(errno){
					SWITCH_CASE(EAGAIN, TIMM_OSAL_ERR_PIPE_EMPTY,
					"WriteToPipe: The queue was empty, and the O_NONBLOCK flag was set") 
					SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
					"WriteToPipe: The descriptor specified in mqdes was invalid") 
					SWITCH_CASE(EMSGSIZE, TIMM_OSAL_ERR_SYSTEM_LIMIT_EXCEEDED,
					"WriteToPipe: msg_len was greater than the mq_msgsize attribute of the message queue") 
					SWITCH_CASE(EINTR, TIMM_OSAL_ERR_SIGNAL_CAUGHT,
					"WriteToPipe: The call was interrupted by a signal handler") 
					SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
					"WriteToPipe: Error") 
				}	
				goto EXIT;
			}

			/*Reset the old blocking attributes*/
			status = mq_setattr (pHandle->mqDes, &old_attr, 0);

			/*On error, -1 is returned with errno indicating the error*/	
			if (NO_SUCCESS == pHandle->mqDes) {
				switch(errno){
					SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
					"WriteToPipe: The descriptor specified in mqdes is invalid") 
					SWITCH_CASE(EINVAL, TIMM_OSAL_ERR_PARAMETER,
					"WriteToPipe: newattr->mq_flags contained set bits other than O_NONBLOCK") 
					SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
					"WriteToPipe: Error") 
				}	
				goto EXIT;
			}			
			break;			
		case TIMM_OSAL_SUSPEND:

			/*Send the message to the queue*/
			status = mq_send(pHandle->mqDes, (const char *)pMessage, size, prio);

			/*mq_send returns -1 on error with errno indicating the error*/ 
			if (NO_SUCCESS == pHandle->mqDes) {
				switch(errno){
					SWITCH_CASE(EAGAIN, TIMM_OSAL_ERR_PIPE_EMPTY,
					"WriteToPipe: The queue was empty, and the O_NONBLOCK flag was set") 
					SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
					"WriteToPipe: The descriptor specified in mqdes was invalid") 
					SWITCH_CASE(EMSGSIZE, TIMM_OSAL_ERR_SYSTEM_LIMIT_EXCEEDED,
					"WriteToPipe: msg_len was greater than the mq_msgsize attribute of the message queue") 
					SWITCH_CASE(EINTR, TIMM_OSAL_ERR_SIGNAL_CAUGHT,
					"WriteToPipe: The call was interrupted by a signal handler") 
					SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
					"WriteToPipe: Error") 
				}	
				goto EXIT;
			}
		
			break;		
		default:

			/* Calculate uTimeOutMsec in terms of the absolute time. uTimeOutMsec is in milliseconds*/
			gettimeofday(&now, NULL);
			timeout_us = now.tv_usec + 1000 * timeout;
			abstimeout.tv_sec = now.tv_sec + timeout_us / 1000000;
			abstimeout.tv_nsec = (timeout_us % 1000000) * 1000;

			status = mq_timedsend(pHandle->mqDes, (const char *)pMessage, size, prio, &abstimeout);

			/*mq_send returns -1 on error with errno indicating the error*/ 
			if (NO_SUCCESS == pHandle->mqDes) {
				switch(errno){
					SWITCH_CASE(EAGAIN, TIMM_OSAL_ERR_PIPE_EMPTY,
					"WriteToPipe: The queue was empty, and the O_NONBLOCK flag was set") 
					SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
					"WriteToPipe: The descriptor specified in mqdes was invalid") 
					SWITCH_CASE(EMSGSIZE, TIMM_OSAL_ERR_SYSTEM_LIMIT_EXCEEDED,
					"WriteToPipe: msg_len was greater than the mq_msgsize attribute of the message queue") 
					SWITCH_CASE(EINTR, TIMM_OSAL_ERR_SIGNAL_CAUGHT,
					"WriteToPipe: The call was interrupted by a signal handler") 
					SWITCH_CASE(EINVAL, TIMM_OSAL_ERR_PARAMETER,
					"WriteToPipe: abs_timeout was invalid") 
					SWITCH_CASE(ETIMEDOUT, TIMM_OSAL_ERR_TIMEOUT,
					"WriteToPipe: The call timed out before a message could be transferred") 					
					SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
					"WriteToPipe: Error") 
				}	
				goto EXIT;
			}			
		break;			
	}	
		
	/*All is well*/
	bReturnStatus = TIMM_OSAL_ERR_NONE;
	
EXIT:
	return bReturnStatus;
}



/* ========================================================================== */
/**
* @fn TIMM_OSAL_ReadFromPipe function
*
* 
*/
/* ========================================================================== */

TIMM_OSAL_ERRORTYPE TIMM_OSAL_ReadFromPipe (TIMM_OSAL_PTR pPipe,
    void *pMessage,
    TIMM_OSAL_U32 size,
    TIMM_OSAL_U32 *actualSize,
    TIMM_OSAL_S32 timeout)
{
	TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;
	mqd_t readbytes, status = -1;
	struct mq_attr attr, old_attr;
	struct timespec abstimeout;
	struct timeval now;
	TIMM_OSAL_U32 timeout_us;
	
	TIMM_OSAL_PIPE *pHandle = (TIMM_OSAL_PIPE *)pPipe;

	/*Check if bad pointer*/
	if(TIMM_OSAL_NULL == pHandle) {
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}

	if(TIMM_OSAL_NULL == pMessage) {
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}

	status = mq_getattr(pHandle->mqDes, &attr);

	/*On error, -1 is returned with errno indicating the error*/	
	if (NO_SUCCESS == pHandle->mqDes) {
		switch(errno){
			SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
			"ReadFromPipe: The descriptor specified in mqdes is invalid") 
			SWITCH_CASE(EINVAL, TIMM_OSAL_ERR_PARAMETER,
			"ReadFromPipe: newattr->mq_flags contained set bits other than O_NONBLOCK") 
			SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
			"ReadFromPipe: Error") 
		}	
		goto EXIT;
	}	

	switch(timeout){
		case TIMM_OSAL_NO_SUSPEND:

			/*Set the message queue as non blocking*/
			attr.mq_flags = O_NONBLOCK ;
			
			status = mq_setattr (pHandle->mqDes, &attr, &old_attr); 

			/*On error, -1 is returned with errno indicating the error*/	
			if (NO_SUCCESS == pHandle->mqDes) {
				switch(errno){
					SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
					"ReadFromPipe: The descriptor specified in mqdes is invalid") 
					SWITCH_CASE(EINVAL, TIMM_OSAL_ERR_PARAMETER,
					"ReadFromPipe: newattr->mq_flags contained set bits other than O_NONBLOCK") 
					SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
					"ReadFromPipe: Error") 
				}	
				goto EXIT;
			}			   

			/*Send the message to the queue*/
			readbytes = mq_receive (pHandle->mqDes, (const char *)pMessage, size, 0);

			/*mq_send returns -1 on error with errno indicating the error*/ 
			if (NO_SUCCESS == pHandle->mqDes) {
				switch(errno){
					SWITCH_CASE(EAGAIN, TIMM_OSAL_ERR_PIPE_EMPTY,
					"ReadFromPipe: The queue was empty, and the O_NONBLOCK flag was set") 
					SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
					"ReadFromPipe: The descriptor specified in mqdes was invalid") 
					SWITCH_CASE(EMSGSIZE, TIMM_OSAL_ERR_SYSTEM_LIMIT_EXCEEDED,
					"ReadFromPipe: msg_len was less than the mq_msgsize attribute of the message queue") 
					SWITCH_CASE(EINTR, TIMM_OSAL_ERR_SIGNAL_CAUGHT,
					"ReadFromPipe: The call was interrupted by a signal handler") 
					SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
					"ReadFromPipe: Error") 
				}	
				goto EXIT;
			}

			*actualSize = readbytes;

			/*Set the message queue to blocking(default)*/
			status = mq_setattr (pHandle->mqDes, &old_attr, 0); 

			/*On error, -1 is returned with errno indicating the error*/	
			if (NO_SUCCESS == pHandle->mqDes) {
				switch(errno){
					SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
					"ReadFromPipe: The descriptor specified in mqdes is invalid") 
					SWITCH_CASE(EINVAL, TIMM_OSAL_ERR_PARAMETER,
					"ReadFromPipe: newattr->mq_flags contained set bits other than O_NONBLOCK") 
					SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
					"ReadFromPipe: Error") 
				}	
				goto EXIT;
			}				
			break;
		case TIMM_OSAL_SUSPEND:
			
			/*Send the message to the queue*/
			readbytes = mq_receive (pHandle->mqDes, (const char *)pMessage, size, 0);

			/*mq_send returns -1 on error with errno indicating the error*/ 
			if (NO_SUCCESS == pHandle->mqDes) {
				switch(errno){
					SWITCH_CASE(EAGAIN, TIMM_OSAL_ERR_PIPE_EMPTY,
					"ReadFromPipe: The queue was empty, and the O_NONBLOCK flag was set") 
					SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
					"ReadFromPipe: The descriptor specified in mqdes was invalid") 
					SWITCH_CASE(EMSGSIZE, TIMM_OSAL_ERR_SYSTEM_LIMIT_EXCEEDED,
					"ReadFromPipe: msg_len was less than the mq_msgsize attribute of the message queue") 
					SWITCH_CASE(EINTR, TIMM_OSAL_ERR_SIGNAL_CAUGHT,
					"ReadFromPipe: The call was interrupted by a signal handler") 
					SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
					"ReadFromPipe: Error") 
				}	
				goto EXIT;
			}

			*actualSize = readbytes;
			break;		
		default:

			/* Calculate uTimeOutMsec in terms of the absolute time. uTimeOutMsec is in milliseconds*/
			gettimeofday(&now, NULL);
			timeout_us = now.tv_usec + 1000 * timeout;
			abstimeout.tv_sec = now.tv_sec + timeout_us / 1000000;
			abstimeout.tv_nsec = (timeout_us % 1000000) * 1000;

			readbytes = mq_timedreceive(pHandle->mqDes, (const char *)pMessage,	size, 0, &abstimeout);

			/*mq_send returns -1 on error with errno indicating the error*/ 
			if (NO_SUCCESS == pHandle->mqDes) {
				switch(errno){
					SWITCH_CASE(EAGAIN, TIMM_OSAL_ERR_PIPE_EMPTY,
					"ReadFromPipe: The queue was empty, and the O_NONBLOCK flag was set") 
					SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
					"ReadFromPipe: The descriptor specified in mqdes was invalid") 
					SWITCH_CASE(EMSGSIZE, TIMM_OSAL_ERR_SYSTEM_LIMIT_EXCEEDED,
					"ReadFromPipe: msg_len was less than the mq_msgsize attribute of the message queue") 
					SWITCH_CASE(EINTR, TIMM_OSAL_ERR_SIGNAL_CAUGHT,
					"ReadFromPipe: The call was interrupted by a signal handler") 
					SWITCH_CASE(EINVAL, TIMM_OSAL_ERR_PARAMETER,
					"ReadFromPipe: abs_timeout was invalid") 
					SWITCH_CASE(ETIMEDOUT, TIMM_OSAL_ERR_TIMEOUT,
					"ReadFromPipe: The call timed out before a message could be transferred") 					
					SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
					"ReadFromPipe: Error") 
				}	
				goto EXIT;
			}

			*actualSize = readbytes;
		break;			
	}	

	/*All is well*/
	bReturnStatus = TIMM_OSAL_ERR_NONE;
	
EXIT:
	return bReturnStatus;

}



/* ========================================================================== */
/**
* @fn TIMM_OSAL_ClearPipe function
*
* 
*/
/* ========================================================================== */

TIMM_OSAL_ERRORTYPE TIMM_OSAL_ClearPipe (TIMM_OSAL_PTR pPipe)
{
#if 0
	TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;
	/*Caller-defined structure for message queue. msgType can be used to identify Priority messages*/
	TIMM_OSAL_MSGQ msgStruct;
	/*To hold the actual size read by msgrcv*/
	TIMM_OSAL_U32 retSize = -1;

	TIMM_OSAL_PIPE *pHandle = (TIMM_OSAL_PIPE *)pPipe;
	/*Check if bad pointer*/
	if(TIMM_OSAL_NULL == pHandle) {
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}
	
	/*Needs to be completed*/
	while(ENOMSG  != errno){
		retSize = msgrcv(pHandle->msgQid, pMessage, size, 0, IPC_NOWAIT);
	}	

EXIT:
    return bReturnStatus;
#endif
	return TIMM_OSAL_ERR_NOT_SUPPORTED;
}



/* ========================================================================== */
/**
* @fn TIMM_OSAL_IsPipeReady function
*
* 
*/
/* ========================================================================== */

TIMM_OSAL_ERRORTYPE TIMM_OSAL_IsPipeReady (TIMM_OSAL_PTR pPipe)
{
    TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;
	TIMM_OSAL_PIPE *pHandle = (TIMM_OSAL_PIPE *)pPipe;
	mqd_t status = -1;
	struct mq_attr attr;

	/*Check for bad pointer*/
	if(TIMM_OSAL_NULL == pHandle) {
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}

	status = mq_getattr(pHandle->mqDes, &attr);

	/*On error, -1 is returned with errno indicating the error*/	
	if (NO_SUCCESS == pHandle->mqDes) {
		switch(errno){
			SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
			"WriteToPipe: The descriptor specified in mqdes is invalid") 
			SWITCH_CASE(EINVAL, TIMM_OSAL_ERR_PARAMETER,
			"WriteToPipe: newattr->mq_flags contained set bits other than O_NONBLOCK") 
			SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
			"WriteToPipe: Error") 
		}	
		goto EXIT;
	}

    if (0 != attr.mq_curmsgs) {
        bReturnStatus =  TIMM_OSAL_ERR_NONE;
    } else {
        bReturnStatus =  TIMM_OSAL_ERR_NOT_READY;
    }


EXIT:
	return bReturnStatus;
}



/* ========================================================================== */
/**
* @fn TIMM_OSAL_GetPipeReadyMessageCount function
*
* 
*/
/* ========================================================================== */

TIMM_OSAL_ERRORTYPE TIMM_OSAL_GetPipeReadyMessageCount (TIMM_OSAL_PTR pPipe,
      TIMM_OSAL_U32 *count)
{
	TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;
	TIMM_OSAL_PIPE *pHandle = (TIMM_OSAL_PIPE *)pPipe;
	mqd_t status = -1;
	struct mq_attr attr;

	/*Check for bad pointer*/
	if(TIMM_OSAL_NULL == pHandle) {
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}

	status = mq_getattr(pHandle->mqDes, &attr);

	/*On error, -1 is returned with errno indicating the error*/	
	if (NO_SUCCESS == pHandle->mqDes) {
		switch(errno){
			SWITCH_CASE(EBADF, TIMM_OSAL_ERR_PARAMETER,
			"WriteToPipe: The descriptor specified in mqdes is invalid") 
			SWITCH_CASE(EINVAL, TIMM_OSAL_ERR_PARAMETER,
			"WriteToPipe: newattr->mq_flags contained set bits other than O_NONBLOCK") 
			SWITCH_DEFAULT_CASE(TIMM_OSAL_ERR_UNKNOWN,
			"WriteToPipe: Error") 
		}	
		goto EXIT;
	}


	/*Put the message count into the parameter passed from application*/
	*count = attr.mq_curmsgs;
	
	/*All is well*/
	bReturnStatus = TIMM_OSAL_ERR_NONE;

EXIT:
	return bReturnStatus;

}
