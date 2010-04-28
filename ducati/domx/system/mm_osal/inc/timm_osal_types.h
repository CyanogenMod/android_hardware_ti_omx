/* ====================================================================
 *             Texas Instruments OMAP(TM) Platform Software
 * (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied.
 * ==================================================================== */
/*
*  @file timm_osal_types.h
*  The timm_osal_types header file defines the primative osal type definitions.
*  @path
*
*/
/* -------------------------------------------------------------------------- */
/* =========================================================================
 *!
 *! Revision History
 *! ===================================
 *! 0.1: Created the first draft version, ksrini@ti.com
 * ========================================================================= */

#ifndef _TIMM_OSAL_TYPES_H_
#define _TIMM_OSAL_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

typedef int8_t TIMM_OSAL_S8;       /*INT8*/
typedef int16_t TIMM_OSAL_S16;	      /*INT16*/
typedef int32_t TIMM_OSAL_S32;	      /*INT32*/

#define TIMM_OSAL_INT8_MIN 0xFF
#define TIMM_OSAL_INT8_MAX 0x7F

#define TIMM_OSAL_INT16_MIN 0xFFFF
#define TIMM_OSAL_INT16_MAX 0x7FFF

#define TIMM_OSAL_INT32_MIN 0xFFFFFFFF
#define TIMM_OSAL_INT32_MAX 0x7FFFFFFF

typedef   uint8_t   TIMM_OSAL_U8;	/*UINT8*/
typedef   uint16_t  TIMM_OSAL_U16;	/*UINT16*/
typedef   uint32_t  TIMM_OSAL_U32;	/*UINT32*/

#define TIMM_OSAL_UINT8_MIN 0
#define TIMM_OSAL_UINT8_MAX 0xFF

#define TIMM_OSAL_UINT16_MIN 0
#define TIMM_OSAL_UINT16_MAX 0xFFFF

#define TIMM_OSAL_UINT32_MIN 0
#define TIMM_OSAL_UINT32_MAX 0xFFFFFFFF


typedef char    TIMM_OSAL_CHAR;	/*CHAR*/

typedef   void *  TIMM_OSAL_HANDLE;
typedef   void *  TIMM_OSAL_PTR;

typedef enum TIMM_OSAL_BOOL {
    TIMM_OSAL_FALSE = 0,
    TIMM_OSAL_TRUE = !TIMM_OSAL_FALSE,
    TIMM_OSAL_BOOL_MAX = 0x7FFFFFFF
} TIMM_OSAL_BOOL;

#define TIMM_OSAL_SUSPEND     0xFFFFFFFFUL
#define TIMM_OSAL_NO_SUSPEND  0x0
#define TIMM_OSAL_TIMED_OUT   0x7FFFFFFFUL


#define SUCCESS 0
#define NO_SUCCESS -1

#define ERROR 1
/*
#define TRUE 0
#define FALSE 1
*/
#define URGENT_MESSAGE 2
#define NORMAL_MESSAGE 1


#define TIMM_OSAL_NULL 0

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TIMM_OSAL_TYPES_H_ */


