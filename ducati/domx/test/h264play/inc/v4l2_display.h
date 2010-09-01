/* =============================================================================
*             Texas Instruments OMAP(TM) Platform Software
*  (c) Copyright 2009 Texas Instruments Incorporated.  All Rights Reserved.
*
*  Use of this software is controlled by the terms and conditions found
*  in the license agreement under which this software has been supplied.
* =========================================================================== */

#ifndef _V4L2_DISPLAY_H_
#define _V4L2_DISPLAY_H_

/* V4L2 related methods */

struct viddev
{
	char *path;
};

int v4l2_init(int num_buffers, int width, int height, int pixfmt,
    struct viddev *viddev);
void *v4l2_get_displayed_buffer(int timeout_ms);
void *v4l2_display_buffer(void *pBuffer);
int v4l2_display_done();
void *v4l2_get_start(int index);
size_t v4l2_get_length(int index);
int v4l2_get_stride();
int v4l2_set_crop(int x0, int y0, int width, int height);
#ifndef __NO_OMX__

#include <OMX_Core.h>
#include <OMX_IVCommon.h>

int omx_v4l2_init(int num_buffers, int width, int height,
    OMX_COLOR_FORMATTYPE eFormat, struct viddev *viddev);
int omx_v4l2_associate(int index, OMX_BUFFERHEADERTYPE * pBufHead);
OMX_BUFFERHEADERTYPE *omx_v4l2_display_buffer(OMX_BUFFERHEADERTYPE *
    pBufferOut);
OMX_BUFFERHEADERTYPE *omx_v4l2_get_displayed_buffer(int timeout_ms);
#define omx_v4l2_display_done() v4l2_display_done()
OMX_BUFFERHEADERTYPE *fomx_v4l2_display_buffer(OMX_BUFFERHEADERTYPE *
    pBufferOut, int w, int h, int s);

#endif

#endif
