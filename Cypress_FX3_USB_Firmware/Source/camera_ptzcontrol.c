/*
 ## Cypress FX3 Camera Kit source file (camera_ptzcontrol.c)
 ## ===========================
 ##
 ##  Copyright Cypress Semiconductor Corporation, 2010-2012,
 ##  All Rights Reserved
 ##  UNPUBLISHED, LICENSED SOFTWARE.
 ##
 ##  CONFIDENTIAL AND PROPRIETARY INFORMATION
 ##  WHICH IS THE PROPERTY OF CYPRESS.
 ##
 ##  Use of this file is governed
 ##  by the license agreement included in the file
 ##
 ##     <install>/license/license.txt
 ##
 ##  where <install> is the Cypress software
 ##  installation root directory path.
 ##
 ## ===========================
 */

/*
 * This file contains function stubs that can be filled in to implement Pan, Tilt
 * and Zoom controls for the UVC camera application. These functions have not been
 * implemented for the image sensor as of now.
 */

#include "uvc.h"
#include "camera_ptzcontrol.h"

#ifdef UVC_PTZ_SUPPORT

int32_t pan_cur;                        /* Current pan value. */
int32_t tilt_cur;                       /* Current tilt value. */
uint16_t zoom_cur;                      /* Current zoom value. */

void
CyFxUvcAppPTZInit (
        void)
{
    /* Initialize the Pan, Tilt and Zoom control values. Code can be added here to configure these
     * values on the sensor as well.
     */
    zoom_cur = ZOOM_DEFAULT;
    pan_cur = 0;
    tilt_cur = 0;
}

uint16_t
CyFxUvcAppGetCurrentZoom (
        void)
{
    return zoom_cur;
}

int32_t
CyFxUvcAppGetCurrentPan (
        void)
{
    return pan_cur;
}

int32_t
CyFxUvcAppGetCurrentTilt (
        void)
{
    return tilt_cur;
}

void
CyFxUvcAppModifyPan (
        int32_t panValue)
{
    /* Place holder for the pan modification function */
    pan_cur = panValue;
}

void
CyFxUvcAppModifyTilt (
        int32_t tiltValue)
{
    /* Place holder for the tilt modification function */
    tilt_cur = tiltValue;
}

void
CyFxUvcAppModifyZoom (
        uint16_t zoomValue)
{
    /* Place holder for the zoom modification function */
    zoom_cur = zoomValue;
}

#endif

