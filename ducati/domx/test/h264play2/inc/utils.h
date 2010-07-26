/*
 * utils.h
 *
 * Utility definitions for the Memory Interface for TI OMAP processors.
 *
 * Copyright (C) 2008-2010 Texas Instruments, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _UTILS_H_
#define _UTILS_H_

/* ---------- Generic Macros Used in Macros ---------- */

/* statement begin */
#define S_ do
/* statement end */
#define _S while (0)
/* expression begin */
#define E_ (
/* expression end */
#define _E )

/* fourcc macros */
#define FOURCC(a,b,c,d)                   \
    ( ((((unsigned long) (d)) & 0xFF) << 24) | \
      ((((unsigned long) (c)) & 0xFF) << 16) | \
      ((((unsigned long) (b)) & 0xFF) << 8)  | \
       (((unsigned long) (a)) & 0xFF) )

#define FOURS(id) \
      FOURCC((id)[0], (id)[1], (id)[2], (id)[3])

#define FOUR_CHARS(i) \
      (char) ((i) & 0xFF), (char) (((i) >> 8)  & 0xFF), \
      (char) (((i) >> 16) & 0xFF), (char) (((i) >> 24) & 0xFF)

/* safer pointer dereference, must enclose in parentheses */
#define MAGIC(a) (a) == NULL ? NULL : (a)

/* allocation macro */
#define NEW(type)    (type*)calloc(1, sizeof(type))
#define NEWN(type,n) (type*)calloc(n, sizeof(type))
#define ALLOC(var)    var = calloc(1, sizeof(*var))
#define ALLOCN(var,n) var = calloc(n, sizeof(*var))

/* free variable and set it to NULL */
#define FREE(var)    S_ { free(var); var = NULL; } _S

/* clear variable */
#define ZERO(var)    memset(&(var), 0, sizeof(var))

/* binary round methods */
#define ROUND_DOWN_TO2POW(x, N) ((x) & ~((N)-1))
#define ROUND_UP_TO2POW(x, N) ROUND_DOWN_TO2POW((x) + (N) - 1, N)

/* regulare round methods */
#define ROUND_DOWN_TO(x, N) ((x) / (N) * (N))
#define ROUND_UP_TO(x, N) ROUND_DOWN_TO((x) + (N) - 1, N)

#endif

