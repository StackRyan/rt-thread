/*
 * Copyright (c) 2019, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_display.h"
#include "fsl_rm69330.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define RM69330_DelayMs rt_thread_mdelay

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const uint8_t rm69330InitSetting[][2] = {

    {0xFE, 0x07},
    {0x15, 0x04},
    {0xFE, 0x00},
    /* enable TE. */
    {0x35, 0x00},

    {0x3A, 0x77},
    {0x51, 0xff},
};

const display_operations_t rm69330_ops = {
    .init   = RM69330_Init,
    .deinit = RM69330_Deinit,
    .start  = RM69330_Start,
    .stop   = RM69330_Stop,
};

/*******************************************************************************
 * Code
 ******************************************************************************/

status_t RM69330_Init(display_handle_t *handle, const display_config_t *config)
{
    uint32_t i;
    status_t status = kStatus_Success;
    mipi_dsc_pixel_format_t dscPixelFormat;
    rm69330_resource_t *resource = (rm69330_resource_t *)(handle->resource);
    mipi_dsi_device_t *dsiDevice = resource->dsiDevice;

    if (config->resolution != FSL_VIDEO_RESOLUTION(454, 454))
    {
        return kStatus_InvalidArgument;
    }

    handle->height = FSL_VIDEO_EXTRACT_HEIGHT(config->resolution);
    handle->width  = FSL_VIDEO_EXTRACT_WIDTH(config->resolution);

    /* Only support RGB888 and RGB565. */
    if (kVIDEO_PixelFormatRGB565 == config->pixelFormat)
    {
        dscPixelFormat = kMIPI_DCS_Pixel16Bits;
    }
    else if (kVIDEO_PixelFormatRGB888 == config->pixelFormat)
    {
        dscPixelFormat = kMIPI_DCS_Pixel24Bits;
    }
    else
    {
        return kStatus_InvalidArgument;
    }

    handle->pixelFormat = config->pixelFormat;

    /* Power on. */
    resource->pullPowerPin(true);
    RM69330_DelayMs(1);

    /* Perform reset. */
    resource->pullResetPin(false);
    RM69330_DelayMs(1);
    resource->pullResetPin(true);
    RM69330_DelayMs(150);

    /* Set the init settings. */
    for (i = 0; i < ARRAY_SIZE(rm69330InitSetting); i++)
    {
        status = MIPI_DSI_GenericWrite(dsiDevice, rm69330InitSetting[i], 2);

        if (kStatus_Success != status)
        {
            return status;
        }
    }

    status = MIPI_DSI_DCS_SetPixelFormat(dsiDevice, dscPixelFormat, kMIPI_DCS_Pixel24Bits);
    if (kStatus_Success != status)
    {
        return status;
    }

    RM69330_DelayMs(50);

    /* Sleep out. */
    status = MIPI_DSI_DCS_EnterSleepMode(dsiDevice, false);
    if (kStatus_Success != status)
    {
        return status;
    }

    RM69330_DelayMs(150);

    return MIPI_DSI_DCS_SetDisplayOn(dsiDevice, true);
}

status_t RM69330_Deinit(display_handle_t *handle)
{
    rm69330_resource_t *resource = (rm69330_resource_t *)(handle->resource);
    mipi_dsi_device_t *dsiDevice = resource->dsiDevice;

    MIPI_DSI_DCS_EnterSleepMode(dsiDevice, true);

    resource->pullResetPin(false);
    resource->pullPowerPin(false);

    return kStatus_Success;
}

status_t RM69330_Start(display_handle_t *handle)
{
    rm69330_resource_t *resource = (rm69330_resource_t *)(handle->resource);
    mipi_dsi_device_t *dsiDevice = resource->dsiDevice;

    return MIPI_DSI_DCS_SetDisplayOn(dsiDevice, true);
}

status_t RM69330_Stop(display_handle_t *handle)
{
    rm69330_resource_t *resource = (rm69330_resource_t *)(handle->resource);
    mipi_dsi_device_t *dsiDevice = resource->dsiDevice;

    return MIPI_DSI_DCS_SetDisplayOn(dsiDevice, false);
}
