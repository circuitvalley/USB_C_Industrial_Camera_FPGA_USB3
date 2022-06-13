/*
 ## Cypress FX3 Camera Kit header file (camera_ptzcontrol.h)
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
 * This file defines the variables and functions used to control and query the Pan, Tilt
 * and Zoom controls for this UVC camera function.
 */

#include "uvc.h"

#ifndef _INCLUDED_CAMERA_PTZCONTROL_H_
#define _INCLUDED_CAMERA_PTZCONTROL_H_

#ifdef UVC_PTZ_SUPPORT

#define wObjectiveFocalLengthMin                (uint16_t)(1)       /* Minimum Lobjective value for the sensor lens. */
#define wObjectiveFocalLengthMax                (uint16_t)(10)      /* Maximum Lobjective value for the sensor lens. */
#define wOcularFocalLength                      (uint16_t)(1)       /* Locular value for the sensor lens. */
#define ZOOM_DEFAULT				            (uint16_t)(5)       /* Default zoom setting that we start with. */

#define CyFxUvcAppGetMinimumZoom()              (wObjectiveFocalLengthMin)      /* Minimum supported zoom value. */
#define CyFxUvcAppGetMaximumZoom()              (wObjectiveFocalLengthMax)      /* Maximum supported zoom value. */
#define CyFxUvcAppGetZoomResolution()           ((uint16_t)1)                   /* Zoom resolution is one unit. */
#define CyFxUvcAppGetDefaultZoom()              ((uint16_t)ZOOM_DEFAULT)        /* Default zoom setting. */

#define PANTILT_MIN				                (int32_t)(-648000)  /* Minimum value for Pan and Tilt controls. */
#define PANTILT_MAX				                (int32_t)(648000)   /* Maximum value for Pan and Tilt controls. */

#define CyFxUvcAppGetMinimumPan()               (PANTILT_MIN)       /* Minimum pan value. */
#define CyFxUvcAppGetMaximumPan()               (PANTILT_MAX)       /* Maximum pan value. */
#define CyFxUvcAppGetPanResolution()            ((int32_t)1)        /* Resolution for pan setting. */
#define CyFxUvcAppGetDefaultPan()               ((int32_t)0)        /* Default pan setting. */

#define CyFxUvcAppGetMinimumTilt()              (PANTILT_MIN)       /* Minimum tilt value. */
#define CyFxUvcAppGetMaximumTilt()              (PANTILT_MAX)       /* Maximum tilt value. */
#define CyFxUvcAppGetTiltResolution()           ((int32_t)1)        /* Resolution for tilt setting. */
#define CyFxUvcAppGetDefaultTilt()              ((int32_t)0)        /* Default tilt setting. */

/* Function    : CyFxUvcAppPTZInit
   Description : Initialize the Pan, Tilt and Zoom settings for the camera.
   Parameters  : None
 */
extern void
CyFxUvcAppPTZInit (
        void);

/* Function    : CyFxUvcAppGetCurrentZoom
   Description : Get the current zoom setting for the sensor.
   Parameters  : None
 */
extern uint16_t
CyFxUvcAppGetCurrentZoom (
        void);


/* Function    : CyFxUvcAppGetCurrentPan
   Description : Get the current pan setting for the camera.
   Parameters  : None
 */
extern int32_t
CyFxUvcAppGetCurrentPan (
        void);

/* Function    : CyFxUvcAppGetCurrentTilt
   Description : Get the current tilt setting for the camera.
   Parameters  : None
 */
extern int32_t
CyFxUvcAppGetCurrentTilt (
        void);

/* Function    : CyFxUvcAppModifyPan
   Description : Stub function that can be filled in to implement a camera PAN control.
   Parameters  :
                 panValue - PAN control value selected by the host application.
 */
extern void
CyFxUvcAppModifyPan (
        int32_t panValue);

/* Function    : CyFxUvcAppModifyTilt
   Description : Stub function that can be filled in to implement a camera TILT control.
   Parameters  :
                 tiltValue - TILT control value selected by the host application.
 */
extern void
CyFxUvcAppModifyTilt (
        int32_t tiltValue);

/* Function    : CyFxUvcAppModifyZoom
   Description : Stub function that can be filled in to implement a camera ZOOM control.
   Parameters  :
                 zoomValue - ZOOM control value selected by the host application.
 */
extern void
CyFxUvcAppModifyZoom (
        uint16_t zoomValue);

#endif

#endif /* _INCLUDED_CAMERA_PTZCONTROL_H_ */

