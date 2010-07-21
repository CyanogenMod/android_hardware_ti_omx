
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

#ifndef OMX_DEBUGMEM_H
#define OMX_DEBUGMEM_H

#include <stdlib.h> // for size_t

#if OMX_DEBUG
void* OMX_DebugMem_calloc(size_t num, size_t size, 
                           const char* file, const char* func, int line);
void* OMX_DebugMem_malloc(size_t size,
                           const char* file, const char* func, int line);
void* OMX_DebugMem_realloc(void *ptr, size_t size,
                           const char* file, const char* func, int line);
void  OMX_DebugMem_free(void* ptr,
                         const char* file, const char* func, int line);
int   OMX_DebugMem_validate(void *ptr,
                         const char* file, const char* func, int line);

void OMX_DebugMem_dump(const char *file, const char *func, int line);
#endif

#define OMX_MASK_MEMORY    0x00010000 /** One of the reserved bits from OMX_MASK_USERMASK */

#define MEM_DEBUG_HANDLE_DESCR "DebugMemory"

#if OMX_DEBUG
    #define malloc(x)    OMX_DebugMem_malloc(x,__FILE__,__FUNCTION__,__LINE__)
    #define calloc(n,s)  OMX_DebugMem_calloc(n,s,__FILE__,__FUNCTION__,__LINE__)
    #define realloc(x,s) OMX_DebugMem_realloc(x,s,__FILE__,__FUNCTION__,__LINE__)
    #define free(x)      OMX_DebugMem_free(x,__FILE__,__FUNCTION__ ,__LINE__)
    #define validate(x)  OMX_DebugMem_validate(x, __FILE__,__FUNCTION__ ,__LINE__)
    #define dump()      OMX_DebugMem_dump( __FILE__,__FUNCTION__ ,__LINE__)
#else
    #define validate(x)
    #define dump()
#endif    


#endif

