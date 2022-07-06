
#include <cyu3system.h>
#include <cyu3os.h>
#include <cyu3dma.h>
#include <cyu3error.h>
#include <cyu3usb.h>
#include <cyu3uart.h>
#include <cyu3gpif.h>
#include <cyu3i2c.h>
#include <cyu3gpio.h>
#include <cyu3pib.h>
#include <cyu3utils.h>
#include <cyu3socket.h>

#include "uvc.h"
//#include "sensor_imx219.h"
#include "sensor_imx477.h"
#include "camera_ptzcontrol.h"
#include "cyfxgpif2config.h"
#include "uvc_settings.h"
/*************************************************************************************************
                                         Global Variables
 *************************************************************************************************/
static CyU3PThread   uvcAppThread;                      /* UVC video streaming thread. */
static CyU3PThread   uvcAppEP0Thread;                   /* UVC control request handling thread. */
static CyU3PEvent    glFxUVCEvent;                      /* Event group used to signal threads. */
CyU3PDmaMultiChannel glChHandleUVCStream;               /* DMA multi-channel handle. */

/* Current UVC control request fields. See USB specification for definition. */
uint8_t  bmReqType, bRequest;                           /* bmReqType and bRequest fields. */
uint16_t wValue, wIndex, wLength;                       /* wValue, wIndex and wLength fields. */

CyU3PUSBSpeed_t usbSpeed = CY_U3P_NOT_CONNECTED;        /* Current USB connection speed. */
CyBool_t        streamingStarted = CyFalse;             /* Whether USB host has started streaming data */
CyBool_t        glIsApplnActive  = CyFalse;             /* When CLEAR_FEATURE (stop streaming) request is sent this variable is reset and set when start
                                                           streaming request is sent by Host. This is used during commit buffer failure event. */
static CyBool_t glIsConfigured = CyFalse;               /* Whether Application is in configured state or not */

/* Mac OS does not send EP Clear feature when the app is closed. It just stops issuing IN tokens. So buffer commit failures
 * can be counted and if it reaches beyond a limit, streaming can be stopped. Buffer commit failure code can be cleared
 * on DMA Consumer event so that the limit is not reached under streaming conditions.  */
static uint8_t glCommitBufferFailureCount = 0;

/*Variable to track whether the reason for DMA Reset is Frame Timer overflow or Commit Buffer Failure*/
static uint8_t glDmaResetFlag = CY_FX_UVC_DMA_RESET_EVENT_NOT_ACTIVE;

static uint8_t glUvcVcErrorCode = CY_FX_UVC_VC_ERROR_CODE_NO_ERROR;

#ifdef FRAME_TIMER_ENABLE

/* Maximum frame transfer time in milli-seconds. The value is updated for every resolution and frame rate.
 * Note: The value should always be greater than the frame blanking period */
uint16_t glFrameTimerPeriod = CY_FX_UVC_FRAME_TIMER_VAL_200MS;

/* Timer used to track frame transfer time. */
static CyU3PTimer UvcTimer;

/* Frame timer overflow call back function */
static void CyFxUvcAppProgressTimer(uint32_t arg)
{
    if(glDmaResetFlag == CY_FX_UVC_DMA_RESET_EVENT_NOT_ACTIVE)
    {
        glDmaResetFlag = CY_FX_UVC_DMA_RESET_FRAME_TIMER_OVERFLOW;
        CyU3PEventSet(&glFxUVCEvent, CY_FX_UVC_DMA_RESET_EVENT, CYU3P_EVENT_OR);
    }
}
#endif

#ifdef BACKFLOW_DETECT
uint8_t back_flow_detected = 0;                         /* Whether buffer overflow error is detected. */
#endif

#ifdef USB_DEBUG_INTERFACE
CyU3PDmaChannel  glDebugCmdChannel;                     /* Channel to receive debug commands on. */
CyU3PDmaChannel  glDebugRspChannel;                     /* Channel to send debug responses on. */
uint8_t         *glDebugRspBuffer;                      /* Buffer used to send debug responses. */
#endif

/* UVC Probe Control Settings for a USB 3.0 connection. */
uint8_t glProbeCtrl[CY_FX_UVC_MAX_PROBE_SETTING] = {
    0x00, 0x00,                 /* bmHint : no hit */
    0x01,                       /* Use 1st Video format index */
    0x01,                       /* Use 1st Video frame index */
    DBVAL(INTERVAL_30),     /* Desired frame interval in the unit of 100ns: 30 fps */
    0x00, 0x00,                 /* Key frame rate in key frame/video frame units: only applicable
                                   to video streaming with adjustable compression parameters */
    0x00, 0x00,                 /* PFrame rate in PFrame / key frame units: only applicable to
                                   video streaming with adjustable compression parameters */
    0x00, 0x00,                 /* Compression quality control: only applicable to video streaming
                                   with adjustable compression parameters */
    0x00, 0x00,                 /* Window size for average bit rate: only applicable to video
                                   streaming with adjustable compression parameters */
    0x00, 0x00,                 /* Internal video streaming i/f latency in ms */
    DBVAL(MAX_FRAME_SIZE),     /* Max video frame size in bytes */
    0x00, 0x80, 0x00, 0x00,      /* No. of bytes device can rx in single payload = 32 KB */

#ifndef FX3_UVC_1_0_SUPPORT
    /* UVC 1.1 Probe Control has additional fields from UVC 1.0 */
    0x00, 0x60, 0xE3, 0x16,             /* Device Clock */
    0x00,                               /* Framing Information - Ignored for uncompressed format*/
    0x00,                               /* Preferred payload format version */
    0x00,                               /* Minimum payload format version */
    0x00                                /* Maximum payload format version */
#endif
};

typedef struct __attribute__((packed)) uvc_control_s
{
	uint16_t hint;
	uint8_t format_index;
	uint8_t frame_index;
	uint32_t frame_interval;
	uint16_t key_frame_rate;
	uint16_t PFrame_rate;
	uint16_t comp_quality;
	uint16_t comp_win_size;
	uint16_t delay;
	uint32_t max_video_frame_size;
	uint32_t max_payload_transfer_size;
#ifndef FX3_UVC_1_0_SUPPORT
	uint32_t device_clock;
	uint8_t	frame_info;
	uint8_t prefered_version;
	uint8_t min_version;
	uint8_t max_version;
#endif
	uint8_t align_16[14];
}uvc_control_t;

/* UVC Probe Control Setting for a USB 2.0 connection. */
uint8_t glProbeCtrl20[CY_FX_UVC_MAX_PROBE_SETTING] = {
    0x00, 0x00,                 /* bmHint : no hit */
    0x01,                       /* Use 1st Video format index */
    0x01,                       /* Use 1st Video frame index */
    DBVAL(INTERVAL_30),    /* Desired frame interval in the unit of 100ns: 15 fps */
    0x00, 0x00,                 /* Key frame rate in key frame/video frame units: only applicable
                                   to video streaming with adjustable compression parameters */
    0x00, 0x00,                 /* PFrame rate in PFrame / key frame units: only applicable to
                                   video streaming with adjustable compression parameters */
    0x00, 0x00,                 /* Compression quality control: only applicable to video streaming
                                   with adjustable compression parameters */
    0x00, 0x00,                 /* Window size for average bit rate: only applicable to video
                                   streaming with adjustable compression parameters */
    0x00, 0x00,                 /* Internal video streaming i/f latency in ms */
    DBVAL(MAX_FRAME_SIZE),     /* Max video frame size in bytes */
    0x00, 0x80, 0x00, 0x00,      /* No. of bytes device can rx in single payload = 32 KB */

#ifndef FX3_UVC_1_0_SUPPORT
    /* UVC 1.1 Probe Control has additional fields from UVC 1.0 */
    0x00, 0x60, 0xE3, 0x16,             /* Device Clock */
    0x00,                               /* Framing Information - Ignored for uncompressed format*/
    0x00,                               /* Preferred payload format version */
    0x00,                               /* Minimum payload format version */
    0x00                                /* Maximum payload format version */
#endif
};

/* Video Probe Commit Control. This array is filled out when the host sends down the SET_CUR request. */
static uint8_t glCommitCtrl[CY_FX_UVC_MAX_PROBE_SETTING_ALIGNED];

/* Scratch buffer used for handling UVC class requests with a data phase. */
static uint8_t glEp0Buffer[32];

/* UVC Header to be prefixed at the top of each 16 KB video data buffer. */
uint8_t volatile glUVCHeader[CY_FX_UVC_MAX_HEADER] =
{
    0x0C,                               /* Header Length */
    0x8C,                               /* Bit field header field */
    0x00, 0x00, 0x00, 0x00,             /* Presentation time stamp field */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00  /* Source clock reference field */
};

#ifdef UVC_EXTENSION_UNIT

/* Format is: Version 1.0 (Major.Minor) Build date: 9/10/17 (MM/DD/YY) */
static uint8_t glFxUvcFirmwareVersion[5] = {
    1,       /* Major version */
    0,       /* Minor version */
    9,       /* Build month */
    22,      /* Build day */
    17       /* Build Year */
};
#endif

#ifdef DEBUG_PRINT_FRAME_COUNT
volatile static uint32_t glFrameCount = 0;              /* Number of video frames transferred so far. */
volatile static uint32_t glDmaDone = 1;                 /* Number of buffers transferred in the current frame. */
#endif
volatile static uint32_t gTotal_bytes = 0;

/* Add the UVC packet header to the top of the specified DMA buffer. */
void
CyFxUVCAddHeader (
        uint8_t *buffer_p,              /* Buffer pointer */
        uint8_t frameInd                /* EOF or normal frame indication */
        )
{
    /* Copy header to buffer */
    CyU3PMemCopy (buffer_p, (uint8_t *)glUVCHeader, CY_FX_UVC_MAX_HEADER);

    /* The EOF flag needs to be set if this is the last packet for this video frame. */
    if (frameInd & CY_FX_UVC_HEADER_EOF)
    {
        buffer_p[1] |= CY_FX_UVC_HEADER_EOF;
        glUVCHeader[1] ^= CY_FX_UVC_HEADER_FRAME_ID;
    }
}


/* Application Error Handler */
void
CyFxAppErrorHandler (
        CyU3PReturnStatus_t apiRetStatus    /* API return status */
        )
{
    /* This function is hit when we have hit a critical application error. This is not
       expected to happen, and the current implementation of this function does nothing
       except stay in a loop printing error messages through the UART port.

       This function can be modified to take additional error handling actions such
       as cycling the USB connection or performing a warm reset.
     */
    for (;;)
    {
        CyU3PDebugPrint (4, "Error handler...\r\n");
        CyU3PThreadSleep (1000);
    }
}

/* This function performs the operations for a Video Streaming Abort.
   This is called every time there is a USB reset, suspend or disconnect event.
 */
static void
CyFxUVCApplnAbortHandler (
        void)
{
    /* Set Video Stream Abort Event */
    CyU3PEventSet (&glFxUVCEvent, CY_FX_UVC_STREAM_ABORT_EVENT, CYU3P_EVENT_OR);
}

/* This is the Callback function to handle the USB Events */
static void
CyFxUVCApplnUSBEventCB (
        CyU3PUsbEventType_t evtype,  /* Event type */
        uint16_t             evdata  /* Event data */
        )
{
    switch (evtype)
    {
        case CY_U3P_USB_EVENT_SUSPEND:
            CyU3PDebugPrint (4, "UsbEventCB: SUSPEND encountered...\r\n");
            /* Set USB suspend Event */
            CyU3PEventSet (&glFxUVCEvent, CY_FX_USB_SUSPEND_EVENT_HANDLER, CYU3P_EVENT_OR);
            break;

        case CY_U3P_USB_EVENT_EP_UNDERRUN:
            CyU3PDebugPrint (4, "UsbEventCB: CY_U3P_USB_EVENT_EP_UNDERRUN encountered...\r\n");
            break;

        /* Intentional Fall-through all cases */
        case CY_U3P_USB_EVENT_SETCONF:
            if (CyU3PUsbGetSpeed() == CY_U3P_SUPER_SPEED)
            {
                CyU3PDebugPrint(4, "UsbEventCB: Detected SS USB Connection\r\n");
            }
            else if (CyU3PUsbGetSpeed() == CY_U3P_HIGH_SPEED)
            {
                CyU3PDebugPrint(4, "UsbEventCB: Detected HS USB Connection\r\n");
            }
        case CY_U3P_USB_EVENT_RESET:
        case CY_U3P_USB_EVENT_DISCONNECT:
        case CY_U3P_USB_EVENT_CONNECT:
            if (evtype == CY_U3P_USB_EVENT_SETCONF)
              glIsConfigured = CyTrue;
            else
              glIsConfigured = CyFalse;

            /* Stop the video streamer application and enable LPM. */
            CyU3PUsbLPMEnable();
            if (glIsApplnActive)
            {
              CyU3PDebugPrint(4, "UsbEventCB: Call App Stop\r\n");
              CyFxUVCApplnAbortHandler();
            }
            break;

        default:
            break;
    }
}

/* Callback to handle the USB Setup Requests and UVC Class events */
static CyBool_t
CyFxUVCApplnUSBSetupCB (
        uint32_t setupdat0, /* SETUP Data 0 */
        uint32_t setupdat1  /* SETUP Data 1 */
        )
{
    CyBool_t uvcHandleReq = CyFalse;
    uint32_t status;

    /* Obtain Request Type and Request */
    bmReqType = (uint8_t)(setupdat0 & CY_FX_USB_SETUP_REQ_TYPE_MASK);
    bRequest  = (uint8_t)((setupdat0 & CY_FX_USB_SETUP_REQ_MASK) >> 8);
    wValue    = (uint16_t)((setupdat0 & CY_FX_USB_SETUP_VALUE_MASK) >> 16);
    wIndex    = (uint16_t)(setupdat1 & CY_FX_USB_SETUP_INDEX_MASK);
    wLength   = (uint16_t)((setupdat1 & CY_FX_USB_SETUP_LENGTH_MASK) >> 16);

    /* Check for UVC Class Requests */
    switch (bmReqType)
    {
        case CY_FX_USB_UVC_GET_REQ_TYPE:
        case CY_FX_USB_UVC_SET_REQ_TYPE:
            /* UVC Specific requests are handled in the EP0 thread. */
            switch (wIndex & 0xFF)
            {
                case CY_FX_UVC_CONTROL_INTERFACE:
                    {
                        uvcHandleReq = CyTrue;
                        status = CyU3PEventSet (&glFxUVCEvent, CY_FX_UVC_VIDEO_CONTROL_REQUEST_EVENT, CYU3P_EVENT_OR);
                        if (status != CY_U3P_SUCCESS)
                        {
                            CyU3PDebugPrint (4, "Set CY_FX_UVC_VIDEO_CONTROL_REQUEST_EVENT Failed %x\r\n", status);
                            CyU3PUsbStall (0, CyTrue, CyFalse);
                        }
                    }
                    break;

                case CY_FX_UVC_STREAM_INTERFACE:
                    {
                        uvcHandleReq = CyTrue;
                        status = CyU3PEventSet (&glFxUVCEvent, CY_FX_UVC_VIDEO_STREAM_REQUEST_EVENT, CYU3P_EVENT_OR);
                        if (status != CY_U3P_SUCCESS)
                        {
                            /* Error handling */
                            CyU3PDebugPrint (4, "Set CY_FX_UVC_VIDEO_STREAM_REQUEST_EVENT Failed %x\r\n", status);
                            CyU3PUsbStall (0, CyTrue, CyFalse);
                        }
                    }
                    break;

                default:
                    break;
            }
            break;

        case CY_FX_USB_SET_INTF_REQ_TYPE:
            if (bRequest == CY_FX_USB_SET_INTERFACE_REQ)
            {
                /* Some hosts send Set Interface Alternate Setting 0 command while stopping the video
                 * stream. The application uses this event to stop streaming. */
                if ((wIndex == CY_FX_UVC_STREAM_INTERFACE) && (wValue == 0))
                {
                    /* Stop GPIF state machine to stop data transfers through FX3 */
                    CyU3PDebugPrint (4, "Alternate setting 0..\r\n");

                    /* Clear the stall condition and sequence numbers. */
                    CyU3PUsbStall (CY_FX_EP_BULK_VIDEO, CyFalse, CyTrue);
                    CyFxUVCApplnAbortHandler ();

                    uvcHandleReq = CyTrue;
                    /* Complete Control request handshake */
                    CyU3PUsbAckSetup ();
                }
            }
            else if ((bRequest == CY_U3P_USB_SC_SET_FEATURE) || (bRequest == CY_U3P_USB_SC_CLEAR_FEATURE))
            {
                CyU3PDebugPrint(4, "USBSetupCB:In SET_FTR %d::%d\r\n", glIsApplnActive, glIsConfigured);
                if (glIsConfigured)
                {
                    uvcHandleReq = CyTrue;
                    CyU3PUsbAckSetup ();
                }
            }

            break;

        case CY_U3P_USB_TARGET_ENDPT:
            if (bRequest == CY_U3P_USB_SC_CLEAR_FEATURE)
            {
                if (wIndex == CY_FX_EP_BULK_VIDEO)
                {
                    /* Windows OS sends Clear Feature Request after it stops streaming,
                     * however MAC OS sends clear feature request right after it sends a
                     * Commit -> SET_CUR request. Hence, stop the video streaming and clear
                     * the stall condition and sequence numbers */
                    CyU3PDebugPrint (4, "Clear feature request detected...\r\n");

                    /* Clear the stall condition and sequence numbers. */
                    CyU3PUsbStall (CY_FX_EP_BULK_VIDEO, CyFalse, CyTrue);
                    CyFxUVCApplnAbortHandler();

                    uvcHandleReq = CyTrue;
                    /* Complete Control request handshake */
                    CyU3PUsbAckSetup ();
                }
            }
            break;

        default:
            break;
    }

    /* Return status of request handling to the USB driver */
    return uvcHandleReq;
}

/* DMA callback providing notification when data buffers are received from the sensor and when they have
 * been drained by the USB host.
 *
 * The UVC headers are attached to the data, and forwarded to the USB host in this callback function.
 */
void
CyFxUvcApplnDmaCallback (
        CyU3PDmaMultiChannel *chHandle,
        CyU3PDmaCbType_t      type,
        CyU3PDmaCBInput_t    *input
        )
{
    CyU3PDmaBuffer_t    dmaBuffer;
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;





    if (type == CY_U3P_DMA_CB_PROD_EVENT)
    {
        /* This is a produce event notification to the CPU. This notification is received upon reception of
         * every buffer. The buffer will not be sent out unless it is explicitly committed. The call shall fail
         * if there is a bus reset / usb disconnect or if there is any application error.
         */

#ifdef FRAME_TIMER_ENABLE
        /* Received data from the sensor so stop the frame timer */
        CyU3PTimerStop(&UvcTimer);

        /* Restart the frame timer so that we receive the next buffer before timer overflows */
        CyU3PTimerModify(&UvcTimer, glFrameTimerPeriod, 0);
        CyU3PTimerStart(&UvcTimer);
#endif

        /* There is a possibility that CyU3PDmaMultiChannelGetBuffer will return CY_U3P_ERROR_INVALID_SEQUENCE here.
         * In such a case, do nothing. We make up for this missed produce event by making repeated commit actions
         * in subsequent produce event callbacks.
         */
        status = CyU3PDmaMultiChannelGetBuffer (chHandle, &dmaBuffer, CYU3P_NO_WAIT);
        while (status == CY_U3P_SUCCESS)
        {
        	CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
        	gTotal_bytes = gTotal_bytes + dmaBuffer.count;

        	apiRetStatus = CyU3PGpioSetValue (DEBUG1_GPIO, CyFalse);
        	if (apiRetStatus != CY_U3P_SUCCESS)
        	{
        		CyU3PDebugPrint (4, "GPIO Set Value Error, Error Code = %d\n", apiRetStatus);
        		return;
        	}

        	//Wait for some time to allow proper reset.

        	// Drive the GPIO high to bring the sensor out of reset.
        	apiRetStatus = CyU3PGpioSetValue (DEBUG1_GPIO, CyTrue);
        	if (apiRetStatus != CY_U3P_SUCCESS)
        	{
        		CyU3PDebugPrint (4, "GPIO Set Value Error, Error Code = %d\n", apiRetStatus);
        		return;
        	}

            /* Add Headers*/
            if (dmaBuffer.count == CY_FX_UVC_BUF_FULL_SIZE)
            {
                /* A full buffer indicates there is more data to go in this video frame. */
                CyFxUVCAddHeader (dmaBuffer.buffer - CY_FX_UVC_MAX_HEADER, CY_FX_UVC_HEADER_FRAME);
            }
            else
            {
                /* A partially filled buffer indicates the end of the ongoing video frame. */
                CyFxUVCAddHeader (dmaBuffer.buffer - CY_FX_UVC_MAX_HEADER, CY_FX_UVC_HEADER_EOF);

               	//CyU3PDebugPrint (4, "t %d\n", gTotal_bytes);
                  //              	gTotal_bytes =0;

#ifdef DEBUG_PRINT_FRAME_COUNT
                glFrameCount++;
            //    glDmaDone = 0;
#endif



            }

            /* Commit Buffer to USB*/
            status = CyU3PDmaMultiChannelCommitBuffer (chHandle, (dmaBuffer.count + CY_FX_UVC_MAX_HEADER), 0);
            if (status == CY_U3P_SUCCESS)
            {
#ifdef DEBUG_PRINT_FRAME_COUNT
                glDmaDone++;
#endif
            }
            else
            {
                if(glDmaResetFlag == CY_FX_UVC_DMA_RESET_EVENT_NOT_ACTIVE)
                {
                    glDmaResetFlag = CY_FX_UVC_DMA_RESET_COMMIT_BUFFER_FAILURE;
                    CyU3PEventSet(&glFxUVCEvent, CY_FX_UVC_DMA_RESET_EVENT, CYU3P_EVENT_OR);
                }
                break;
            }

            /* Check if any more buffers are ready to go, and commit them here. */
            status = CyU3PDmaMultiChannelGetBuffer (chHandle, &dmaBuffer, CYU3P_NO_WAIT);
        }
    }
    else if (type == CY_U3P_DMA_CB_CONS_EVENT)
    {
        streamingStarted = CyTrue;
        glCommitBufferFailureCount = 0;        /* Reset the counter after data is consumed by USB */
    }
}

/* GpifCB callback function is invoked when FV triggers GPIF interrupt */
void
CyFxGpifCB (
        uint8_t currentState            /* GPIF state which triggered the interrupt. */
        )
{
    /* The ongoing video frame has ended. If we have a partial buffer sitting on the socket, we need to forcibly
     * wrap it up. We also need to toggle the FW_TRG a couple of times to get the state machine ready for the
     * next frame.
     *
     * Note: DMA channel APIs cannot be used here as this is ISR context. We are making use of the raw socket
     * APIs.
     */

/*
	CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;

  	apiRetStatus = CyU3PGpioSetValue (DEBUG1_GPIO, CyFalse);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
	   CyU3PDebugPrint (4, "GPIO Set Value Error, Error Code = %d\n", apiRetStatus);
	   return;
	}

	// Wait for some time to allow proper reset.

	// Drive the GPIO high to bring the sensor out of reset.
	apiRetStatus = CyU3PGpioSetValue (DEBUG1_GPIO, CyTrue);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
	   CyU3PDebugPrint (4, "GPIO Set Value Error, Error Code = %d\n", apiRetStatus);
	   return;
	}
*/

	 //CyU3PDebugPrint (4, "FSYNC Callback (\n");

    switch (currentState)
    {
        case PARTIAL_BUF_IN_SCK0:
            CyU3PDmaSocketSetWrapUp (CY_U3P_PIB_SOCKET_0);
            break;
        case FULL_BUF_IN_SCK0:
            break;
        case PARTIAL_BUF_IN_SCK1:
            CyU3PDmaSocketSetWrapUp (CY_U3P_PIB_SOCKET_1);
            break;
        case FULL_BUF_IN_SCK1:
            break;

        default:
            /* This should not happen. Do nothing. */
            return;
    }

    CyU3PGpifControlSWInput (CyTrue);
    CyU3PGpifControlSWInput (CyFalse);
}

/* This function initializes the Debug Module for the UVC Application */
static void
CyFxUVCApplnDebugInit (
        void)
{
    CyU3PUartConfig_t uartConfig;
    CyU3PReturnStatus_t apiRetStatus;

    /* Initialize the UART for printing debug messages */
    apiRetStatus = CyU3PUartInit ();
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "UART initialization failed!\n");
        CyFxAppErrorHandler (apiRetStatus);
    }

    /* Set UART Configuration */
    uartConfig.baudRate = CY_U3P_UART_BAUDRATE_115200;
    uartConfig.stopBit  = CY_U3P_UART_ONE_STOP_BIT;
    uartConfig.parity   = CY_U3P_UART_NO_PARITY;
    uartConfig.txEnable = CyTrue;
    uartConfig.rxEnable = CyFalse;
    uartConfig.flowCtrl = CyFalse;
    uartConfig.isDma    = CyTrue;

    /* Set the UART configuration */
    apiRetStatus = CyU3PUartSetConfig (&uartConfig, NULL);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyFxAppErrorHandler (apiRetStatus);
    }

    /* Set the UART transfer */
    apiRetStatus = CyU3PUartTxSetBlockXfer (0xFFFFFFFF);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyFxAppErrorHandler (apiRetStatus);
    }

    /* Initialize the Debug logger module. */
    apiRetStatus = CyU3PDebugInit (CY_U3P_LPP_SOCKET_UART_CONS, 4);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyFxAppErrorHandler (apiRetStatus);
    }

    /* Disable log message headers. */
    CyU3PDebugPreamble (CyFalse);
}

/* I2C initialization. */
static void
CyFxUVCApplnI2CInit (void)
{
    CyU3PI2cConfig_t i2cConfig;;
    CyU3PReturnStatus_t status;

    status = CyU3PI2cInit ();
    if (status != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "I2C initialization failed!\n");
        CyFxAppErrorHandler (status);
    }

    /*  Set I2C Configuration */
    i2cConfig.bitRate    = 100000;      /*  100 KHz */
    i2cConfig.isDma      = CyFalse;
    i2cConfig.busTimeout = 0xffffffffU;
    i2cConfig.dmaTimeout = 0xffff;

    status = CyU3PI2cSetConfig (&i2cConfig, 0);
    if (CY_U3P_SUCCESS != status)
    {
        CyU3PDebugPrint (4, "I2C configuration failed!\n");
        CyFxAppErrorHandler (status);
    }
}

#ifdef BACKFLOW_DETECT
static void CyFxUvcAppPibCallback (
        CyU3PPibIntrType cbType,
        uint16_t cbArg)
{
    if ((cbType == CYU3P_PIB_INTR_ERROR) && ((cbArg == 0x1005) || (cbArg == 0x1006)))
    {
        if (!back_flow_detected)
        {
            CyU3PDebugPrint (4, "Backflow detected...\r\n");
            back_flow_detected = 1;
        }
    }
}
#endif

#ifdef USB_DEBUG_INTERFACE
static void
CyFxUvcAppDebugCallback (
        CyU3PDmaChannel   *handle,
        CyU3PDmaCbType_t   type,
        CyU3PDmaCBInput_t *input)
{
    if (type == CY_U3P_DMA_CB_PROD_EVENT)
    {
        /* Data has been received. Notify the EP0 thread which handles the debug commands as well. */
        CyU3PEventSet (&glFxUVCEvent, CY_FX_USB_DEBUG_CMD_EVENT, CYU3P_EVENT_OR);
    }
}
#endif

/*
 * Load the GPIF configuration on the GPIF-II engine. This operation is performed at start-up.
 */
static void
CyFxUvcAppGpifInit (
        void)
{
    CyU3PReturnStatus_t apiRetStatus;

    apiRetStatus =  CyU3PGpifLoad ((CyU3PGpifConfig_t *) &CyFxGpifConfig);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        /* Error Handling */
        CyU3PDebugPrint (4, "Loading GPIF Configuration failed, Error Code = %d\r\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }
}

/* Callback for LPM requests. Always return true to allow host to transition device
 * into required LPM state U1/U2/U3. When data transmission is active LPM management
 * is explicitly disabled to prevent data transmission errors.
 */
static CyBool_t
CyFxUVCAppLPMRqtCB (
        CyU3PUsbLinkPowerMode link_mode         /*USB 3.0 linkmode requested by Host */
        )
{
    return CyTrue;
}

/* This function initializes the USB Module, creates event group,
   sets the enumeration descriptors, configures the Endpoints and
   configures the DMA module for the UVC Application */
static void
CyFxUVCApplnInit (void)
{
    CyU3PDmaMultiChannelConfig_t dmaMultiConfig;
    CyU3PEpConfig_t              endPointConfig;
    CyU3PReturnStatus_t          apiRetStatus;
    CyU3PGpioClock_t             gpioClock;
    CyU3PGpioSimpleConfig_t      gpioConfig;
    CyU3PPibClock_t              pibclock;

#ifdef USB_DEBUG_INTERFACE
    CyU3PDmaChannelConfig_t channelConfig;
#endif

    /* Create UVC event group */
    apiRetStatus = CyU3PEventCreate (&glFxUVCEvent);
    if (apiRetStatus != 0)
    {
        CyU3PDebugPrint (4, "UVC Create Event failed, Error Code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

#ifdef UVC_PTZ_SUPPORT
    CyFxUvcAppPTZInit ();
#endif

    /* Init the GPIO module */
    gpioClock.fastClkDiv = 2;
    gpioClock.slowClkDiv = 2;
    gpioClock.simpleDiv  = CY_U3P_GPIO_SIMPLE_DIV_BY_2;
    gpioClock.clkSrc     = CY_U3P_SYS_CLK;
    gpioClock.halfDiv    = 0;

    /* Initialize Gpio interface */
    apiRetStatus = CyU3PGpioInit (&gpioClock, NULL);
    if (apiRetStatus != 0)
    {
        CyU3PDebugPrint (4, "GPIO Init failed, Error Code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }


    apiRetStatus = CyU3PDeviceGpioOverride (DEBUG1_GPIO, CyTrue);
     if (apiRetStatus != 0)
     {
         CyU3PDebugPrint (4, "GPIO Override failed, Error Code = %d\n", apiRetStatus);
         CyFxAppErrorHandler (apiRetStatus);
     }

     gpioConfig.outValue    = CyTrue;
     gpioConfig.driveLowEn  = CyTrue;
     gpioConfig.driveHighEn = CyTrue;
     gpioConfig.inputEn     = CyFalse;
     gpioConfig.intrMode    = CY_U3P_GPIO_NO_INTR;
     apiRetStatus           = CyU3PGpioSetSimpleConfig (DEBUG1_GPIO, &gpioConfig);
     if (apiRetStatus != CY_U3P_SUCCESS)
     {
         CyU3PDebugPrint (4, "GPIO Set Config Error, Error Code = %d\n", apiRetStatus);
         CyFxAppErrorHandler (apiRetStatus);
     }


    /* CTL pins are restricted and cannot be configured using I/O matrix configuration function,
     * must use GpioOverride to configure it */
    apiRetStatus = CyU3PDeviceGpioOverride (SENSOR_RESET_GPIO, CyTrue);
    if (apiRetStatus != 0)
    {
        CyU3PDebugPrint (4, "GPIO Override failed, Error Code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    /* SENSOR_RESET_GPIO is the Sensor reset pin */
    gpioConfig.outValue    = CyTrue;
    gpioConfig.driveLowEn  = CyTrue;
    gpioConfig.driveHighEn = CyTrue;
    gpioConfig.inputEn     = CyFalse;
    gpioConfig.intrMode    = CY_U3P_GPIO_NO_INTR;
    apiRetStatus           = CyU3PGpioSetSimpleConfig (SENSOR_RESET_GPIO, &gpioConfig);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "GPIO Set Config Error, Error Code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }
	CyU3PGpioSetValue (FX3_MUXSEL_GPIO, CyTrue);




    /* FX3_MUXSEL_GPIO is the Sensor reset pin */
     /* CTL pins are restricted and cannot be configured using I/O matrix configuration function,
     * must use GpioOverride to configure it */
    apiRetStatus = CyU3PDeviceGpioOverride (FX3_MUXSEL_GPIO, CyTrue);
    if (apiRetStatus != 0)
    {
        CyU3PDebugPrint (4, "GPIO Override failed, Error Code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    /* SENSOR_RESET_GPIO is the Sensor reset pin */
    gpioConfig.outValue    = CyTrue;
    gpioConfig.driveLowEn  = CyTrue;
    gpioConfig.driveHighEn = CyTrue;
    gpioConfig.inputEn     = CyFalse;
    gpioConfig.intrMode    = CY_U3P_GPIO_NO_INTR;
    apiRetStatus           = CyU3PGpioSetSimpleConfig (FX3_MUXSEL_GPIO, &gpioConfig);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "GPIO Set Config Error, Error Code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }







    /* Initialize the P-port. */
    pibclock.clkDiv      = 2;
    pibclock.clkSrc      = CY_U3P_SYS_CLK;
    pibclock.isDllEnable = CyFalse;
    pibclock.isHalfDiv   = CyFalse;

    apiRetStatus = CyU3PPibInit (CyTrue, &pibclock);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "PIB Function Failed to Start, Error Code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    CyFxUvcAppGpifInit ();

    /* Register the GPIF State Machine callback used to get frame end notifications.
     * We use the fast callback version which is triggered from ISR context.
     */
    CyU3PGpifRegisterSMIntrCallback (CyFxGpifCB);

#ifdef BACKFLOW_DETECT
    back_flow_detected = 0;
    CyU3PPibRegisterCallback (CyFxUvcAppPibCallback, CYU3P_PIB_INTR_ERROR);
#endif

    /* Image sensor initialization. Reset and then initialize with appropriate configuration. */
    CyU3PThreadSleep(100);
    SensorReset ();
    SensorInit ();

    /* USB initialization. */
    apiRetStatus = CyU3PUsbStart ();
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "USB Function Failed to Start, Error Code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    /* Setup the Callback to Handle the USB Setup Requests */
    CyU3PUsbRegisterSetupCallback (CyFxUVCApplnUSBSetupCB, CyFalse);

    /* Setup the Callback to Handle the USB Events */
    CyU3PUsbRegisterEventCallback (CyFxUVCApplnUSBEventCB);

    /* Register a callback to handle LPM requests from the USB 3.0 host. */
    CyU3PUsbRegisterLPMRequestCallback (CyFxUVCAppLPMRqtCB);

    /* Register the USB device descriptors with the driver. */
    CyU3PUsbSetDesc (CY_U3P_USB_SET_HS_DEVICE_DESCR, 0, (uint8_t *)CyFxUSBDeviceDscr);
    CyU3PUsbSetDesc (CY_U3P_USB_SET_SS_DEVICE_DESCR, 0, (uint8_t *)CyFxUSBDeviceDscrSS);

    /* BOS and Device qualifier descriptors. */
    CyU3PUsbSetDesc (CY_U3P_USB_SET_DEVQUAL_DESCR, 0, (uint8_t *)CyFxUSBDeviceQualDscr);
    CyU3PUsbSetDesc (CY_U3P_USB_SET_SS_BOS_DESCR, 0, (uint8_t *)CyFxUSBBOSDscr);

    /* Configuration descriptors. */
    CyU3PUsbSetDesc (CY_U3P_USB_SET_HS_CONFIG_DESCR, 0, (uint8_t *)CyFxUSBHSConfigDscr);
    CyU3PUsbSetDesc (CY_U3P_USB_SET_FS_CONFIG_DESCR, 0, (uint8_t *)CyFxUSBFSConfigDscr);
    CyU3PUsbSetDesc (CY_U3P_USB_SET_SS_CONFIG_DESCR, 0, (uint8_t *)CyFxUSBSSConfigDscr);

    /* String Descriptors */
    CyU3PUsbSetDesc (CY_U3P_USB_SET_STRING_DESCR, 0, (uint8_t *)CyFxUSBStringLangIDDscr);
    CyU3PUsbSetDesc (CY_U3P_USB_SET_STRING_DESCR, 1, (uint8_t *)CyFxUSBManufactureDscr);
    CyU3PUsbSetDesc (CY_U3P_USB_SET_STRING_DESCR, 2, (uint8_t *)CyFxUSBProductDscr);

    /* Configure the video streaming endpoint. */
    endPointConfig.enable   = 1;
    endPointConfig.epType   = CY_U3P_USB_EP_BULK;
    endPointConfig.pcktSize = CY_FX_EP_BULK_VIDEO_PKT_SIZE;
    endPointConfig.isoPkts  = 1;
    endPointConfig.burstLen = 16;
    endPointConfig.streams  = 0;
    apiRetStatus = CyU3PSetEpConfig (CY_FX_EP_BULK_VIDEO, &endPointConfig);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        /* Error Handling */
        CyU3PDebugPrint (4, "USB Set Endpoint config failed, Error Code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    /* Configure the status interrupt endpoint.
       Note: This endpoint is not being used by the application as of now. This can be used in case
       UVC device needs to notify the host about any error conditions. A MANUAL_OUT DMA channel
       can be associated with this endpoint and used to send these data packets.
     */
    endPointConfig.enable   = 1;
    endPointConfig.epType   = CY_U3P_USB_EP_INTR;
    endPointConfig.pcktSize = 64;
    endPointConfig.isoPkts  = 0;
    endPointConfig.streams  = 0;
    endPointConfig.burstLen = 1;
    apiRetStatus = CyU3PSetEpConfig (CY_FX_EP_CONTROL_STATUS, &endPointConfig);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        /* Error Handling */
        CyU3PDebugPrint (4, "USB Set Endpoint config failed, Error Code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    /* Create a DMA Manual channel for sending the video data to the USB host. */
    dmaMultiConfig.size           = CY_FX_UVC_STREAM_BUF_SIZE;
    dmaMultiConfig.count          = CY_FX_UVC_STREAM_BUF_COUNT;
    dmaMultiConfig.validSckCount  = 2;
    dmaMultiConfig.prodSckId [0]  = (CyU3PDmaSocketId_t)CY_U3P_PIB_SOCKET_0;
    dmaMultiConfig.prodSckId [1]  = (CyU3PDmaSocketId_t)CY_U3P_PIB_SOCKET_1;
    dmaMultiConfig.consSckId [0]  = (CyU3PDmaSocketId_t)(CY_U3P_UIB_SOCKET_CONS_0 | CY_FX_EP_VIDEO_CONS_SOCKET);
    dmaMultiConfig.prodAvailCount = 0;
    dmaMultiConfig.prodHeader     = 12;                 /* 12 byte UVC header to be added. */
    dmaMultiConfig.prodFooter     = 4;                  /* 4 byte footer to compensate for the 12 byte header. */
    dmaMultiConfig.consHeader     = 0;
    dmaMultiConfig.dmaMode        = CY_U3P_DMA_MODE_BYTE;
    dmaMultiConfig.notification   = CY_U3P_DMA_CB_PROD_EVENT | CY_U3P_DMA_CB_CONS_EVENT;
    dmaMultiConfig.cb             = CyFxUvcApplnDmaCallback;
    apiRetStatus = CyU3PDmaMultiChannelCreate (&glChHandleUVCStream, CY_U3P_DMA_TYPE_MANUAL_MANY_TO_ONE,
            &dmaMultiConfig);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        /* Error handling */
        CyU3PDebugPrint (4, "DMA Channel Creation Failed, Error Code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

#ifdef USB_DEBUG_INTERFACE
    /* Configure the endpoints and create DMA channels used by the USB debug interface.
       The command (OUT) endpoint is configured in packet mode and enabled to receive data.
       Once the CY_U3P_DMA_CB_PROD_EVENT callback is received, the received data packet is
       processed and the data is returned through the CyU3PDmaChannelSetupSendBuffer API call.
     */

    endPointConfig.enable   = 1;
    endPointConfig.epType   = CY_U3P_USB_EP_BULK;
    endPointConfig.pcktSize = 1024;                     /* Use SuperSpeed settings here. */
    endPointConfig.isoPkts  = 0;
    endPointConfig.streams  = 0;
    endPointConfig.burstLen = 1;

    apiRetStatus = CyU3PSetEpConfig (CY_FX_EP_DEBUG_CMD, &endPointConfig);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "Debug Command endpoint config failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    CyU3PUsbSetEpPktMode (CY_FX_EP_DEBUG_CMD, CyTrue);

    apiRetStatus = CyU3PSetEpConfig (CY_FX_EP_DEBUG_RSP, &endPointConfig);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "Debug Response endpoint config failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    channelConfig.size           = 1024;
    channelConfig.count          = 1;
    channelConfig.prodSckId      = CY_U3P_UIB_SOCKET_PROD_0 | CY_FX_EP_DEBUG_CMD_SOCKET;
    channelConfig.consSckId      = CY_U3P_CPU_SOCKET_CONS;
    channelConfig.prodAvailCount = 0;
    channelConfig.prodHeader     = 0;
    channelConfig.prodFooter     = 0;
    channelConfig.consHeader     = 0;
    channelConfig.dmaMode        = CY_U3P_DMA_MODE_BYTE;
    channelConfig.notification   = CY_U3P_DMA_CB_PROD_EVENT;
    channelConfig.cb             = CyFxUvcAppDebugCallback;

    apiRetStatus = CyU3PDmaChannelCreate (&glDebugCmdChannel, CY_U3P_DMA_TYPE_MANUAL_IN, &channelConfig);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "Debug Command channel create failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    apiRetStatus = CyU3PDmaChannelSetXfer (&glDebugCmdChannel, 0);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "Debug channel SetXfer failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    channelConfig.size           = 1024;
    channelConfig.count          = 0;           /* No buffers allocated. We will only use the SetupSend API. */
    channelConfig.prodSckId      = CY_U3P_CPU_SOCKET_PROD;
    channelConfig.consSckId      = CY_U3P_UIB_SOCKET_CONS_0 | CY_FX_EP_DEBUG_RSP_SOCKET;
    channelConfig.prodAvailCount = 0;
    channelConfig.prodHeader     = 0;
    channelConfig.prodFooter     = 0;
    channelConfig.consHeader     = 0;
    channelConfig.dmaMode        = CY_U3P_DMA_MODE_BYTE;
    channelConfig.notification   = 0;
    channelConfig.cb             = 0;

    apiRetStatus = CyU3PDmaChannelCreate (&glDebugRspChannel, CY_U3P_DMA_TYPE_MANUAL_OUT, &channelConfig);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "Debug Response channel create failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    glDebugRspBuffer = (uint8_t *)CyU3PDmaBufferAlloc (1024);
    if (glDebugRspBuffer == 0)
    {
        CyU3PDebugPrint (4, "Failed to allocate memory for debug buffer\r\n");
        CyFxAppErrorHandler (CY_U3P_ERROR_MEMORY_ERROR);
    }
#endif

#ifdef FRAME_TIMER_ENABLE
  CyU3PTimerCreate(&UvcTimer, CyFxUvcAppProgressTimer, 0x00, glFrameTimerPeriod, 0, CYU3P_NO_ACTIVATE);
#endif


  /* Disable USB2.0 connection */
  apiRetStatus = CyU3PUsbControlUsb2Support (CyFalse);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
      	 CyU3PDebugPrint(4, "USB2.0 host disable failed = %d\n", apiRetStatus);
    }
  /* Check the enumeration MuxControl_GPIO to High */
  apiRetStatus = CyU3PConnectState(CyTrue, CyTrue);
    if(CyU3PUsbGetSpeed() != CY_U3P_SUPER_SPEED)
     {
        apiRetStatus = CyU3PConnectState(CyFalse, CyFalse);
        CyU3PDebugPrint(4, "First attempt failed = %d\n", apiRetStatus);

          /* Check in other orientation */
        //CyU3PGpioSetValue(0, CyFalse);
        CyU3PGpioSetValue (FX3_MUXSEL_GPIO, CyFalse);
        apiRetStatus = CyU3PUsbControlUsb2Support (CyTrue);
        apiRetStatus = CyU3PConnectState(CyTrue, CyTrue);
        if (apiRetStatus != CY_U3P_SUCCESS)
          {
          	CyU3PDebugPrint(4, "USB Connect failed, Error code = %d\n", apiRetStatus);
          }
      }



//    /* Enable USB connection from the FX3 device, preferably at USB 3.0 speed. */
//    apiRetStatus = CyU3PConnectState (CyTrue, CyTrue);
//    if (apiRetStatus != CY_U3P_SUCCESS)
//    {
//        CyU3PDebugPrint (4, "USB Connect failed, Error Code = %d\n", apiRetStatus);
//        CyFxAppErrorHandler (apiRetStatus);
//    }
}

void
CyFxUvcApplnStop()
{
#ifdef DEBUG_PRINT_FRAME_COUNT
    /* Clear state variables. */
    glDmaDone    = 1;
    glFrameCount = 0;
#endif /* DEBUG_PRINT_FRAME_COUNT */

#ifdef FRAME_TIMER_ENABLE
    /* Stop the frame timer during an application stop */
    CyU3PTimerStop(&UvcTimer);
#endif

    /* Disable the GPIF state machine. */
    CyU3PGpifDisable (CyFalse);
    streamingStarted = CyFalse;
    glDmaResetFlag = CY_FX_UVC_DMA_RESET_EVENT_NOT_ACTIVE;

    /* Place the EP in NAK mode before cleaning up the pipe. */
    CyU3PUsbSetEpNak (CY_FX_EP_BULK_VIDEO, CyTrue);
    CyU3PBusyWait (125);

    /* Reset and flush the endpoint pipe. */
    CyU3PDmaMultiChannelReset (&glChHandleUVCStream);
    CyU3PUsbFlushEp (CY_FX_EP_BULK_VIDEO);
    CyU3PUsbSetEpNak (CY_FX_EP_BULK_VIDEO, CyFalse);
    CyU3PBusyWait (125);

    /* Allow USB low power link transitions at this stage. */
    CyU3PUsbLPMEnable ();
	CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;

    CyU3PDebugPrint (4, "Application Stopped\r\n");
}

void
CyFxUvcApplnStart()
{
    CyU3PReturnStatus_t   apiRetStatus;

#ifdef DEBUG_PRINT_FRAME_COUNT
    /* Clear state variables. */
    glDmaDone    = 1;
    glFrameCount = 0;
#endif /* DEBUG_PRINT_FRAME_COUNT */

    /* Start with frame ID 0. */
    glUVCHeader[1] &= ~CY_FX_UVC_HEADER_FRAME_ID;

    /* Make sure we return to an active USB link state and stay there. */
    CyU3PUsbLPMDisable ();
    if (CyU3PUsbGetSpeed () == CY_U3P_SUPER_SPEED)
    {
        CyU3PUsbSetLinkPowerState (CyU3PUsbLPM_U0);
        CyU3PBusyWait (200);
    }
    else
    {
        CyU3PUsb2Resume ();
    }

    /* Place the EP in NAK mode before cleaning up the pipe. */
    CyU3PUsbSetEpNak (CY_FX_EP_BULK_VIDEO, CyTrue);
    CyU3PBusyWait (125);

    /* Reset and flush the endpoint pipe. */
    CyU3PUsbFlushEp (CY_FX_EP_BULK_VIDEO);
    CyU3PDmaMultiChannelReset (&glChHandleUVCStream);

    /* Set DMA Channel transfer size, first producer socket */
    apiRetStatus = CyU3PDmaMultiChannelSetXfer (&glChHandleUVCStream, 0, 0);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        /* Error handling */
        CyU3PDebugPrint (4, "DMA Channel Set Transfer Failed, Error Code = %d\r\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    CyU3PUsbSetEpNak (CY_FX_EP_BULK_VIDEO, CyFalse);
    CyU3PBusyWait (125);

#ifdef FRAME_TIMER_ENABLE
    /* Start the frame timer so that we receive first buffer on time */
    CyU3PTimerModify(&UvcTimer, glFrameTimerPeriod, 0);
    CyU3PTimerStart(&UvcTimer);
#endif
    glDmaResetFlag = CY_FX_UVC_DMA_RESET_EVENT_NOT_ACTIVE;

    /* Start the state machine from the designated start state. */
    apiRetStatus = CyU3PGpifSMSwitch(CY_FX_UVC_INVALID_GPIF_STATE, START_SCK0,
            CY_FX_UVC_INVALID_GPIF_STATE, ALPHA_START_SCK0, CY_FX_UVC_GPIF_SWITCH_TIMEOUT);

    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        /* Error Handling */
        CyU3PDebugPrint (4, "Switching GPIF state machine failed, Error Code = %d\r\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    CyU3PDebugPrint (4, "Application Started\r\n");
}

/*
 * Entry function for the UVC Application Thread
 */
void
UVCAppThread_Entry (
        uint32_t input)
{
    CyU3PUsbLinkPowerMode usb3mode;
    uint16_t              wakeReason;
    CyU3PReturnStatus_t   apiRetStatus;
    uint32_t              flag;

    /* Initialize the Uart Debug Module */
    CyFxUVCApplnDebugInit ();

    /* Initialize the I2C interface */
    CyFxUVCApplnI2CInit ();

    /* Initialize the UVC Application */
    CyFxUVCApplnInit ();

    /*
       The actual data forwarding from sensor to USB host is done from the DMA and GPIF callback
       functions. The thread is only responsible for checking for streaming start/stop conditions.
      
       The CY_FX_UVC_STREAM_EVENT event flag will indicate that the UVC video stream should be started.

       The CY_FX_UVC_STREAM_ABORT_EVENT event indicates that we need to abort the video streaming. This
       only happens when we receive a CLEAR_FEATURE request indicating that streaming is to be stopped,
       or when we have a critical error in the data path.

       The CY_FX_UVC_DMA_RESET_EVENT indicates that we need to reset the DMA and endpoint buffers and
       disable GPIF. We restarting the GPIF state machine from first state and device will start streaming
       video after it receives the next frame. FOr camera applications, if we discard few frames and resatrt
       video stream, it shouldn't be a problem. It may be a bad user experience if we abruptly stop video stream.
       Note that we will not restart video stream after few commit buffer failures as this is required for MAC OS.

       The CY_FX_USB_SUSPEND_EVENT_HANDLER indicates that device must enter low power USB suspend mode.
       There is a provision for users to reset and/or turn OFF power to the sensor/Image signal processor (ISP).
     */
    for (;;)
    {
        apiRetStatus = CyU3PEventGet (&glFxUVCEvent, CY_FX_UVC_STREAM_ABORT_EVENT | CY_FX_UVC_STREAM_EVENT |
                CY_FX_UVC_DMA_RESET_EVENT | CY_FX_USB_SUSPEND_EVENT_HANDLER, CYU3P_EVENT_OR_CLEAR, &flag, LOOP_TIMEOUT);

        if (apiRetStatus == CY_U3P_SUCCESS)
        {
            /* Request to start video stream. */
            if ((flag & CY_FX_UVC_STREAM_EVENT) != 0)
            {
                glIsApplnActive = CyTrue;
                CyFxUvcApplnStart();
            }

            /* Video stream abort requested. */
            if ((flag & CY_FX_UVC_STREAM_ABORT_EVENT) != 0)
            {
                glIsApplnActive = CyFalse;
                CyFxUvcApplnStop();
            }

            if (((flag & CY_FX_UVC_DMA_RESET_EVENT) != 0) && glIsApplnActive)
            {
                if(glDmaResetFlag == CY_FX_UVC_DMA_RESET_COMMIT_BUFFER_FAILURE)
                    CyU3PDebugPrint (4, "DMA Reset Event: Commit buffer failure\r\n");

#ifdef FRAME_TIMER_ENABLE
                else if(glDmaResetFlag == CY_FX_UVC_DMA_RESET_FRAME_TIMER_OVERFLOW)
                    CyU3PDebugPrint (4, "DMA Reset Event: Frame timer overflow, time period = %d\r\n", glFrameTimerPeriod);
#endif

                CyFxUvcApplnStop();

                if ((glIsApplnActive) && (++glCommitBufferFailureCount < CY_FX_UVC_MAX_COMMIT_BUF_FAILURE_CNT))
                {
                    CyFxUvcApplnStart();
                }

                /* Reset the video streaming flags for a MAC OS */
                if(glCommitBufferFailureCount == CY_FX_UVC_MAX_COMMIT_BUF_FAILURE_CNT)
                {
                    glIsApplnActive = CyFalse;
                    CyU3PDebugPrint (4, "Application Stopped after %d Commit buffer failures\r\n", glCommitBufferFailureCount);
                    glCommitBufferFailureCount = 0;
                }
            }

            /* Handle USB suspend event by putting device into low power mode */
            if ((flag & CY_FX_USB_SUSPEND_EVENT_HANDLER) != 0)
            {
                /* Include your code here... to reset and/or shutdown power to the sensor/ISP
                 * This will help to reduce the overall power consumption by the kit */

                /* Place FX3 in Low Power Suspend mode */
                CyU3PDebugPrint(4, "Entering USB Suspend Mode\r\n");
                CyU3PThreadSleep(5);

                /* As per the USB3 specs, link layer takes 10ms (Max.) to leave U3 state. If the device sees some spurious (unwanted)
                 * USB activity it will leave suspend mode (even though USB is in U3 state). The device can wait for 10ms (U3 exit LFPS duration)
                 * and check the Link Layer State. If the link layer is not in U3 state, CX3 device will wake up else it will enter suspend mode */
                do
                {
                    apiRetStatus = CyU3PSysEnterSuspendMode(CY_U3P_SYS_USB_BUS_ACTVTY_WAKEUP_SRC, 0, &wakeReason);
                    if ((apiRetStatus != CY_U3P_SUCCESS) || (CyU3PUsbGetSpeed() != CY_U3P_SUPER_SPEED))
                        break;

                    /* Wait for the maximum U3 exit LFPS duration. */
                    CyU3PThreadSleep(10);

                    /* If the link is still in U3, we can continue to attempt Suspend mode entry. */
                    apiRetStatus = CyU3PUsbGetLinkPowerState(&usb3mode);
                    if ((apiRetStatus != CY_U3P_SUCCESS) || (usb3mode != CyU3PUsbLPM_U3))
                        break;
                }while(1);

                /* Leaving Low Power Suspend mode */
                CyU3PDebugPrint(4, "Leaving Suspend Mode\r\n");

                /* Include your code here... to bring sensor/ISP out of reset and/or switch ON the power to the sensor/ISP */
            }
        }

#ifdef DEBUG_PRINT_FRAME_COUNT
        CyU3PDebugPrint (4, "UVC: Completed %d frames and %d buffers\r\n", glFrameCount,
                (glDmaDone != 0) ? (glDmaDone - 1) : 0);
#endif
    }
}

/*
 * Handler for control requests addressed to the Processing Unit.
 */
static void
UVCHandleProcessingUnitRqts (
        void)
{
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
    uint16_t readCount, brightnessVal;
    uint16_t exposure_val, gain_val;
    switch (wValue)
    {
        case CY_FX_UVC_PU_BRIGHTNESS_CONTROL:
            switch (bRequest)
            {
                case CY_FX_USB_UVC_GET_LEN_REQ: /* Length of brightness data = 2 byte. */
                    glEp0Buffer[0] = 2;
                    glEp0Buffer[1] = 0;
                    CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
                    break;
                case CY_FX_USB_UVC_GET_CUR_REQ: /* Current brightness value. */
                    glEp0Buffer[0] = SensorGetBrightness ();
                    glEp0Buffer[1] = 0;
                    CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
                    break;
                case CY_FX_USB_UVC_GET_MIN_REQ: /* Minimum brightness = 0. */
                    glEp0Buffer[0] = 0;
                    glEp0Buffer[1] = 0;
                    CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
                    break;
                case CY_FX_USB_UVC_GET_MAX_REQ: /* Maximum brightness = 255. */
                    glEp0Buffer[0] = 255;
                    glEp0Buffer[1] = 0;
                    CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
                    break;
                case CY_FX_USB_UVC_GET_RES_REQ: /* Resolution = 1. */
                    glEp0Buffer[0] = 1;
                    glEp0Buffer[1] = 0;
                    CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
                    break;
                case CY_FX_USB_UVC_GET_INFO_REQ: /* Both GET and SET requests are supported, auto modes not supported */
                    glEp0Buffer[0] = 3;
                    glEp0Buffer[1] = 0;
                    CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
                    break;
                case CY_FX_USB_UVC_GET_DEF_REQ: /* Default brightness value = 55. */
                    glEp0Buffer[0] = 55;
                    glEp0Buffer[1] = 0;
                    CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
                    break;
                case CY_FX_USB_UVC_SET_CUR_REQ: /* Update brightness value. */
                    apiRetStatus = CyU3PUsbGetEP0Data (CY_FX_UVC_MAX_PROBE_SETTING_ALIGNED,
                            glEp0Buffer, &readCount);
                    if (apiRetStatus == CY_U3P_SUCCESS)
                    {
                        brightnessVal = CY_U3P_MAKEWORD(glEp0Buffer[1], glEp0Buffer[0]);
                        /* Update the brightness value only if the value is within the range */
                        if(brightnessVal >= 0 && brightnessVal <= 255)
                        {
                            SensorSetBrightness (glEp0Buffer[0]);
                        }
                    }
                    break;
                default:
                    glUvcVcErrorCode = CY_FX_UVC_VC_ERROR_CODE_INVALID_REQUEST;
                    CyU3PUsbStall (0, CyTrue, CyFalse);
                    break;
            }
            break;
        case CY_FX_UVC_PU_SATURATION_CONTROL:
        {

        	switch (bRequest)
        	{
        	case CY_FX_USB_UVC_GET_LEN_REQ: /* Length of brightness data = 2 byte. */
        		glEp0Buffer[0] = 2;
        		CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
        		break;
        	case CY_FX_USB_UVC_GET_CUR_REQ: /* Current brightness value. */
        		exposure_val = sensor_get_exposure ();
        		glEp0Buffer[0] = exposure_val & 0xFF;
        		glEp0Buffer[1] = (exposure_val>>8) & 0xFF;
        		CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
        		break;
        	case CY_FX_USB_UVC_GET_MIN_REQ: /* Minimum brightness = 0. */
        		glEp0Buffer[0] = 0;
        		glEp0Buffer[1] = 0;
        		CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
        		break;
        	case CY_FX_USB_UVC_GET_MAX_REQ: /* Maximum brightness = 255. */
        		exposure_val = sensor_get_max_exposure ();
        		glEp0Buffer[0] = exposure_val & 0xFF;
        		glEp0Buffer[1] = (exposure_val>>8) & 0xFF;
        		CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
        		break;
        	case CY_FX_USB_UVC_GET_RES_REQ: /* Resolution = 1. */
        		glEp0Buffer[0] = 1;
        		glEp0Buffer[1] = 0;
        		CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
        		break;
        	case CY_FX_USB_UVC_GET_INFO_REQ: /* Both GET and SET requests are supported, auto modes not supported */
        		glEp0Buffer[0] = 3;
        		CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
        		break;
        	case CY_FX_USB_UVC_GET_DEF_REQ: /* Default brightness */
        		exposure_val = sensor_get_def_exposure ();
        		glEp0Buffer[0] = exposure_val & 0xFF;
        		glEp0Buffer[1] = (exposure_val>>8) & 0xFF;
        		CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
        		break;
        	case CY_FX_USB_UVC_SET_CUR_REQ: /* Update brightness value. */
        		apiRetStatus = CyU3PUsbGetEP0Data (CY_FX_UVC_MAX_PROBE_SETTING_ALIGNED,
        				glEp0Buffer, &readCount);
        		if (apiRetStatus == CY_U3P_SUCCESS)
        		{
        			exposure_val = CY_U3P_MAKEWORD(glEp0Buffer[1], glEp0Buffer[0]);

       				//sensor_set_exposure (exposure_val);
        		}
        		break;
        	default:
        		glUvcVcErrorCode = CY_FX_UVC_VC_ERROR_CODE_INVALID_REQUEST;
        		CyU3PUsbStall (0, CyTrue, CyFalse);
        		break;
        	}
        	break;
        }
        case  CY_FX_UVC_PU_GAMMA_CONTROL: //used as test Pattern
        {
        	switch (bRequest)
        	{
        	case CY_FX_USB_UVC_GET_LEN_REQ: /* Length of TestPattern  data = 2 byte. */
        		glEp0Buffer[0] = 2;
        		glEp0Buffer[1] = 0;
        		CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
        		break;
        	case CY_FX_USB_UVC_GET_CUR_REQ: /* Current TestPattern  value. */
        		glEp0Buffer[0] = sensor_get_test_pattern ();
        		glEp0Buffer[1] = 0;
        		CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
        		break;
        	case CY_FX_USB_UVC_GET_MIN_REQ: /* Minimum TestPattern  = 0. */
        		glEp0Buffer[0] = 0;
        		glEp0Buffer[1] = 0;
        		CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
        		break;
        	case CY_FX_USB_UVC_GET_MAX_REQ: /* Maximum TestPattern  = 8. */
        		glEp0Buffer[0] = 8;
        		glEp0Buffer[1] = 0;
        		CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
        		break;
        	case CY_FX_USB_UVC_GET_RES_REQ: /* Resolution = 1. */
        		glEp0Buffer[0] = 1;
        		glEp0Buffer[1] = 0;
        		CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
        		break;
        	case CY_FX_USB_UVC_GET_INFO_REQ: /* Both GET and SET requests are supported, auto modes not supported */
        		glEp0Buffer[0] = 3;
        		glEp0Buffer[1] = 0;
        		CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
        		break;
        	case CY_FX_USB_UVC_GET_DEF_REQ: /* Default TestPattern 0 No test pattern */
        		glEp0Buffer[0] = 0;
        		glEp0Buffer[1] = 0;
        		CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
        		break;
        	case CY_FX_USB_UVC_SET_CUR_REQ: /* Update TestPattern  value. */
        		apiRetStatus = CyU3PUsbGetEP0Data (CY_FX_UVC_MAX_PROBE_SETTING_ALIGNED,
        				glEp0Buffer, &readCount);
        		if (apiRetStatus == CY_U3P_SUCCESS)
        		{
        			gain_val = CY_U3P_MAKEWORD(glEp0Buffer[1], glEp0Buffer[0]);
        			/* Update the gain value only if the value is within the range */
        			if(gain_val >= 0 && gain_val <= 8)
        			{
        				sensor_set_test_pattern (glEp0Buffer[0]);
        			}
        		}
        		break;
        	default:
        		glUvcVcErrorCode = CY_FX_UVC_VC_ERROR_CODE_INVALID_REQUEST;
        		CyU3PUsbStall (0, CyTrue, CyFalse);
        		break;
        	}
        	break;
        }
        default:
            /*
             * Add additional code here to support
             * other controls.
             */
            glUvcVcErrorCode = CY_FX_UVC_VC_ERROR_CODE_INVALID_CONTROL;
            CyU3PUsbStall (0, CyTrue, CyFalse);
            break;
    }
}

/*
 * Handler for control requests addressed to the UVC Camera Terminal unit.
 */
static void
UVCHandleCameraTerminalRqts (
        void)
{
#ifdef UVC_PTZ_SUPPORT
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
    uint16_t readCount;
    uint16_t zoomVal;
    int32_t  panVal, tiltVal;
    CyBool_t sendData = CyFalse;
#endif

    switch (wValue)
    {
#ifdef UVC_PTZ_SUPPORT
        case CY_FX_UVC_CT_ZOOM_ABSOLUTE_CONTROL:
            switch (bRequest)
            {
                case CY_FX_USB_UVC_GET_INFO_REQ:
                    glEp0Buffer[0] = 3;                /* Support GET/SET queries. */
                    CyU3PUsbSendEP0Data (1, (uint8_t *)glEp0Buffer);
                    break;
                case CY_FX_USB_UVC_GET_CUR_REQ: /* Current zoom control value. */
                    zoomVal  = CyFxUvcAppGetCurrentZoom ();
                    sendData = CyTrue;
                    break;
                case CY_FX_USB_UVC_GET_MIN_REQ: /* Minimum zoom control value. */
                    zoomVal  = CyFxUvcAppGetMinimumZoom ();
                    sendData = CyTrue;
                    break;
                case CY_FX_USB_UVC_GET_MAX_REQ: /* Maximum zoom control value. */
                    zoomVal  = CyFxUvcAppGetMaximumZoom ();
                    sendData = CyTrue;
                    break;
                case CY_FX_USB_UVC_GET_RES_REQ: /* Resolution is one unit. */
                    zoomVal  = CyFxUvcAppGetZoomResolution ();
                    sendData = CyTrue;
                    break;
                case CY_FX_USB_UVC_GET_DEF_REQ: /* Default zoom setting. */
                    zoomVal  = CyFxUvcAppGetDefaultZoom ();
                    sendData = CyTrue;
                    break;
                case CY_FX_USB_UVC_SET_CUR_REQ:
                    apiRetStatus = CyU3PUsbGetEP0Data (CY_FX_UVC_MAX_PROBE_SETTING_ALIGNED,
                            glEp0Buffer, &readCount);
                    if (apiRetStatus == CY_U3P_SUCCESS)
                    {
                        zoomVal = (glEp0Buffer[0]) | (glEp0Buffer[1] << 8);
                        CyFxUvcAppModifyZoom (zoomVal);
                    }
                    break;
                default:
                    glUvcVcErrorCode = CY_FX_UVC_VC_ERROR_CODE_INVALID_REQUEST;
                    CyU3PUsbStall (0, CyTrue, CyFalse);
                    break;
            }

            if (sendData)
            {
                /* Send the 2-byte data in zoomVal back to the USB host. */
                glEp0Buffer[0] = CY_U3P_GET_LSB (zoomVal);
                glEp0Buffer[1] = CY_U3P_GET_MSB (zoomVal);
                CyU3PUsbSendEP0Data (wLength, (uint8_t *)glEp0Buffer);
            }
            break;

        case CY_FX_UVC_CT_PANTILT_ABSOLUTE_CONTROL:
            switch (bRequest)
            {
                case CY_FX_USB_UVC_GET_INFO_REQ:
                    glEp0Buffer[0] = 3;                /* GET/SET requests supported for this control */
                    CyU3PUsbSendEP0Data (1, (uint8_t *)glEp0Buffer);
                    break;
                case CY_FX_USB_UVC_GET_CUR_REQ:
                    panVal   = CyFxUvcAppGetCurrentPan ();
                    tiltVal  = CyFxUvcAppGetCurrentTilt ();
                    sendData = CyTrue;
                    break;
                case CY_FX_USB_UVC_GET_MIN_REQ:
                    panVal   = CyFxUvcAppGetMinimumPan ();
                    tiltVal  = CyFxUvcAppGetMinimumTilt ();
                    sendData = CyTrue;
                    break;
                case CY_FX_USB_UVC_GET_MAX_REQ:
                    panVal   = CyFxUvcAppGetMaximumPan ();
                    tiltVal  = CyFxUvcAppGetMaximumTilt ();
                    sendData = CyTrue;
                    break;
                case CY_FX_USB_UVC_GET_RES_REQ:
                    panVal   = CyFxUvcAppGetPanResolution ();
                    tiltVal  = CyFxUvcAppGetTiltResolution ();
                    sendData = CyTrue;
                    break;
                case CY_FX_USB_UVC_GET_DEF_REQ:
                    panVal   = CyFxUvcAppGetDefaultPan ();
                    tiltVal  = CyFxUvcAppGetDefaultTilt ();
                    sendData = CyTrue;
                    break;
                case CY_FX_USB_UVC_SET_CUR_REQ:
                    apiRetStatus = CyU3PUsbGetEP0Data (CY_FX_UVC_MAX_PROBE_SETTING_ALIGNED,
                            glEp0Buffer, &readCount);
                    if (apiRetStatus == CY_U3P_SUCCESS)
                    {
                        panVal = (glEp0Buffer[0]) | (glEp0Buffer[1]<<8) |
                            (glEp0Buffer[2]<<16) | (glEp0Buffer[2]<<24);
                        tiltVal = (glEp0Buffer[4]) | (glEp0Buffer[5]<<8) |
                            (glEp0Buffer[6]<<16) | (glEp0Buffer[7]<<24);

                        CyFxUvcAppModifyPan (panVal);
                        CyFxUvcAppModifyTilt (tiltVal);
                    }
                    break;
                default:
                    CyU3PUsbStall (0, CyTrue, CyFalse);
                    glUvcVcErrorCode = CY_FX_UVC_VC_ERROR_CODE_INVALID_REQUEST;
                    break;
            }

            if (sendData)
            {
                /* Send the 8-byte PAN and TILT values back to the USB host. */
                glEp0Buffer[0] = CY_U3P_DWORD_GET_BYTE0 (panVal);
                glEp0Buffer[1] = CY_U3P_DWORD_GET_BYTE1 (panVal);
                glEp0Buffer[2] = CY_U3P_DWORD_GET_BYTE2 (panVal);
                glEp0Buffer[3] = CY_U3P_DWORD_GET_BYTE3 (panVal);
                glEp0Buffer[4] = CY_U3P_DWORD_GET_BYTE0 (tiltVal);
                glEp0Buffer[5] = CY_U3P_DWORD_GET_BYTE1 (tiltVal);
                glEp0Buffer[6] = CY_U3P_DWORD_GET_BYTE2 (tiltVal);
                glEp0Buffer[7] = CY_U3P_DWORD_GET_BYTE3 (tiltVal);
                CyU3PUsbSendEP0Data (wLength, (uint8_t *)glEp0Buffer);
            }
            break;
#endif

        default:
            glUvcVcErrorCode = CY_FX_UVC_VC_ERROR_CODE_INVALID_CONTROL;
            CyU3PUsbStall (0, CyTrue, CyFalse);
            break;
    }
}

/*
 * Handler for UVC Interface control requests.
 */
static void
UVCHandleInterfaceCtrlRqts (
        void)
{
    switch (wValue)
    {
        /* Control to send video control errors to the Host. When device stalls a video
         * control request, Windows host gets the error through this control */
        case CY_FX_UVC_VC_REQUEST_ERROR_CODE_CONTROL:
            switch (bRequest)
            {
                case CY_FX_USB_UVC_GET_CUR_REQ:
                    CyU3PUsbSendEP0Data(1, &glUvcVcErrorCode);
                    glUvcVcErrorCode = CY_FX_UVC_VC_ERROR_CODE_NO_ERROR;
                    break;
            }
            break;
    }
}

/*
 * Handler for control requests addressed to the Extension Unit.
 */
static void
UVCHandleExtensionUnitRqts (
        void)
{
#ifdef UVC_EXTENSION_UNIT
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
    uint16_t readCount;
    CyBool_t sendData = CyFalse;
#endif

    switch (wValue)
    {
#ifdef UVC_EXTENSION_UNIT
        case CY_FX_UVC_XU_GET_FIRMWARE_VERSION_CONTROL:
            switch (bRequest)
            {
                case CY_FX_USB_UVC_GET_INFO_REQ:
                    glEp0Buffer[0] = 3;                /* Support GET/SET queries. */
                    CyU3PUsbSendEP0Data (1, (uint8_t *)glEp0Buffer);
                    break;
                case CY_FX_USB_UVC_GET_CUR_REQ: /* Current firmware version control value. */
                    CyU3PMemCopy(glEp0Buffer, glFxUvcFirmwareVersion, 5);
                    sendData = CyTrue;
                    break;
                case CY_FX_USB_UVC_GET_MIN_REQ: /* Minimum firmware version control value. */
                    /* Min value is version 0.1 Build date: 1/1/17 */
                    glEp0Buffer[0] = 0;
                    CyU3PMemSet(&glEp0Buffer[1], 1, 3);
                    glEp0Buffer[4] = 17;
                    sendData = CyTrue;
                    break;
                case CY_FX_USB_UVC_GET_MAX_REQ: /* Maximum firmware version control value. */
                    /* Max value is version 255.255 Build date: 12/31/99 */
                    CyU3PMemSet(glEp0Buffer, 0xFF, 2);
                    glEp0Buffer[2] = 12;
                    glEp0Buffer[3] = 31;
                    glEp0Buffer[4] = 99;
                    sendData = CyTrue;
                    break;
                case CY_FX_USB_UVC_GET_RES_REQ: /* Resolution is one unit for all the fields */
                    CyU3PMemSet(glEp0Buffer, 1, 5);
                    sendData = CyTrue;
                    break;
                case CY_FX_USB_UVC_GET_LEN_REQ: /* Length of the control */
                    /* As per UVC spec, we send 2 bytes of data. Firmware version control length is 5 bytes */
                    glEp0Buffer[0] = 5;
                    glEp0Buffer[1] = 0;
                    CyU3PUsbSendEP0Data (2, (uint8_t *)glEp0Buffer);
                    break;
                case CY_FX_USB_UVC_GET_DEF_REQ: /* Default firmware version setting, is the current value. But it can be changed */
                    CyU3PMemCopy(glEp0Buffer, glFxUvcFirmwareVersion, 5);
                    sendData = CyTrue;
                    break;
                case CY_FX_USB_UVC_SET_CUR_REQ:
                    apiRetStatus = CyU3PUsbGetEP0Data (CY_FX_UVC_MAX_PROBE_SETTING_ALIGNED,
                            glEp0Buffer, &readCount);
                    if (apiRetStatus == CY_U3P_SUCCESS)
                    {
                        /* Copy firmware version sent by Host application */
                        CyU3PMemCopy(glFxUvcFirmwareVersion, glEp0Buffer, 5);
                    }
                    break;
                default:
                    glUvcVcErrorCode = CY_FX_UVC_VC_ERROR_CODE_INVALID_REQUEST;
                    CyU3PUsbStall (0, CyTrue, CyFalse);
                    break;
            }

            if (sendData)
            {
                /* Send the data to the USB host. */
                CyU3PUsbSendEP0Data (5, (uint8_t *)glEp0Buffer);
            }
            break;
#endif

        default:
            glUvcVcErrorCode = CY_FX_UVC_VC_ERROR_CODE_INVALID_CONTROL;
            CyU3PUsbStall (0, CyTrue, CyFalse);
            break;
    }
}

/*
 * Handler for the video streaming control requests.
 */
static void UVCHandleVideoStreamingRqts (void)
{
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
    uint16_t readCount;

    switch (wValue)
    {
        case CY_FX_UVC_PROBE_CTRL:
            switch (bRequest)
            {
                case CY_FX_USB_UVC_GET_INFO_REQ:
                    glEp0Buffer[0] = 3;                /* GET/SET requests are supported. */
                    CyU3PUsbSendEP0Data (1, (uint8_t *)glEp0Buffer);
                    break;
                case CY_FX_USB_UVC_GET_LEN_REQ:
                    glEp0Buffer[0] = CY_FX_UVC_MAX_PROBE_SETTING;
                    CyU3PUsbSendEP0Data (1, (uint8_t *)glEp0Buffer);
                    break;
                case CY_FX_USB_UVC_GET_CUR_REQ:
                case CY_FX_USB_UVC_GET_MIN_REQ:
                case CY_FX_USB_UVC_GET_MAX_REQ:
                case CY_FX_USB_UVC_GET_DEF_REQ: /* There is only one setting per USB speed. */
                    if (usbSpeed == CY_U3P_SUPER_SPEED)
                    {
                        CyU3PUsbSendEP0Data (CY_FX_UVC_MAX_PROBE_SETTING, (uint8_t *)glProbeCtrl);
                    }
                    else
                    {
                        CyU3PUsbSendEP0Data (CY_FX_UVC_MAX_PROBE_SETTING, (uint8_t *)glProbeCtrl20);
                    }
                    break;
                case CY_FX_USB_UVC_SET_CUR_REQ:
                    apiRetStatus = CyU3PUsbGetEP0Data (CY_FX_UVC_MAX_PROBE_SETTING_ALIGNED, glCommitCtrl, &readCount);
                    if (apiRetStatus == CY_U3P_SUCCESS)
                    {

                        /* Copy the relevant settings from the host provided data into the
                           active data structure. */
                        glProbeCtrl[2] = glCommitCtrl[2];
                        glProbeCtrl[3] = glCommitCtrl[3];
                        glProbeCtrl[4] = glCommitCtrl[4];
                        glProbeCtrl[5] = glCommitCtrl[5];
                        glProbeCtrl[6] = glCommitCtrl[6];
                        glProbeCtrl[7] = glCommitCtrl[7];
                    }
                    break;
                default:
                    CyU3PUsbStall (0, CyTrue, CyFalse);
                    break;
            }
            break;

        case CY_FX_UVC_COMMIT_CTRL:
            switch (bRequest)
            {
                case CY_FX_USB_UVC_GET_INFO_REQ:
                    glEp0Buffer[0] = 3;                        /* GET/SET requests are supported. */
                    CyU3PUsbSendEP0Data (1, (uint8_t *)glEp0Buffer);
                    break;
                case CY_FX_USB_UVC_GET_LEN_REQ:
                    glEp0Buffer[0] = CY_FX_UVC_MAX_PROBE_SETTING;
                    CyU3PUsbSendEP0Data (1, (uint8_t *)glEp0Buffer);
                    break;
                case CY_FX_USB_UVC_GET_CUR_REQ:
                    if (usbSpeed == CY_U3P_SUPER_SPEED)
                    {
                        CyU3PUsbSendEP0Data (CY_FX_UVC_MAX_PROBE_SETTING, (uint8_t *)glProbeCtrl);
                    }
                    else
                    {
                        CyU3PUsbSendEP0Data (CY_FX_UVC_MAX_PROBE_SETTING, (uint8_t *)glProbeCtrl20);
                    }
                    break;
                case CY_FX_USB_UVC_SET_CUR_REQ:
                    /* The host has selected the parameters for the video stream. Check the desired
                       resolution settings, configure the sensor and start the video stream.
                       */
                    apiRetStatus = CyU3PUsbGetEP0Data (CY_FX_UVC_MAX_PROBE_SETTING_ALIGNED, glCommitCtrl, &readCount);
                    if (apiRetStatus == CY_U3P_SUCCESS)
                    {
                    	uvc_control_t *uvc_control = (uvc_control_t *)glCommitCtrl;

                        sensor_handle_uvc_control(uvc_control->frame_index, uvc_control->frame_interval);

                        #ifdef FRAME_TIMER_ENABLE
                        	/* We are using frame timer value of 400ms as the frame time is upto 66ms.
 	 	 	 	 	 	 	 * Having more margin so that DMA reset doen't happen every now and then */
                        	glFrameTimerPeriod = CY_FX_UVC_FRAME_TIMER_VAL_400MS;
						#endif

                        /* We can start streaming video now. */
                        apiRetStatus = CyU3PEventSet (&glFxUVCEvent, CY_FX_UVC_STREAM_EVENT, CYU3P_EVENT_OR);
                        if (apiRetStatus != CY_U3P_SUCCESS)
                        {
                            CyU3PDebugPrint (4, "Set CY_FX_UVC_STREAM_EVENT failed %x\r\n", apiRetStatus);
                        }
                    }
                    break;

                default:
                    CyU3PUsbStall (0, CyTrue, CyFalse);
                    break;
            }
            break;

        default:
            CyU3PUsbStall (0, CyTrue, CyFalse);
            break;
    }
}

/*
 * Entry function for the UVC control request processing thread.
 */
void
UVCAppEP0Thread_Entry (
        uint32_t input)
{
    uint32_t eventMask = (CY_FX_UVC_VIDEO_CONTROL_REQUEST_EVENT | CY_FX_UVC_VIDEO_STREAM_REQUEST_EVENT);
    uint32_t eventFlag;

#ifdef USB_DEBUG_INTERFACE
    CyU3PReturnStatus_t apiRetStatus;
    CyU3PDmaBuffer_t    dmaInfo;

    eventMask |= CY_FX_USB_DEBUG_CMD_EVENT;
#endif

    for (;;)
    {
        /* Wait for a Video control or streaming related request on the control endpoint. */
        if (CyU3PEventGet (&glFxUVCEvent, eventMask, CYU3P_EVENT_OR_CLEAR, &eventFlag,
                    CYU3P_WAIT_FOREVER) == CY_U3P_SUCCESS)
        {
            usbSpeed = CyU3PUsbGetSpeed ();

            if (eventFlag & CY_FX_UVC_VIDEO_CONTROL_REQUEST_EVENT)
            {
                switch ((wIndex >> 8))
                {
                    case CY_FX_UVC_PROCESSING_UNIT_ID:
                        UVCHandleProcessingUnitRqts ();
                        break;

                    case CY_FX_UVC_CAMERA_TERMINAL_ID:
                        UVCHandleCameraTerminalRqts ();
                        break;

                    case CY_FX_UVC_INTERFACE_CTRL:
                        UVCHandleInterfaceCtrlRqts ();
                        break;

                    case CY_FX_UVC_EXTENSION_UNIT_ID:
                        UVCHandleExtensionUnitRqts ();
                        break;

                    default:
                        /* Unsupported request. Fail by stalling the control endpoint. */
                        CyU3PUsbStall (0, CyTrue, CyFalse);
                        break;
                }
            }

            if (eventFlag & CY_FX_UVC_VIDEO_STREAM_REQUEST_EVENT)
            {
                if (wIndex != CY_FX_UVC_STREAM_INTERFACE)
                {
                    CyU3PUsbStall (0, CyTrue, CyFalse);
                }
                else
                {
                    UVCHandleVideoStreamingRqts ();
                }
            }

#ifdef USB_DEBUG_INTERFACE
            if (eventFlag & CY_FX_USB_DEBUG_CMD_EVENT)
            {
                /* Get the command buffer */
                apiRetStatus = CyU3PDmaChannelGetBuffer (&glDebugCmdChannel, &dmaInfo, CYU3P_WAIT_FOREVER);
                if (apiRetStatus != CY_U3P_SUCCESS)
                {
                    CyU3PDebugPrint (4, "Failed to receive debug command, Error code = %d\r\n", apiRetStatus);
                    CyFxAppErrorHandler (apiRetStatus);
                }

                /* Decode the command from the command buffer, error checking is not implemented,
                 * so the command is expected to be correctly sent from the host application. First byte indicates
                 * read (0x00) or write (0x01) command. Second and third bytes are register address high byte and
                 * register address low byte. For read commands the fourth byte (optional) can be N>0, to read N
                 * registers in sequence. Response first byte is status (0=Pass, !0=Fail) followed by N pairs of
                 * register value high byte and register value low byte.
                 */
                if (dmaInfo.buffer[0] == 0)
                {
                    if (dmaInfo.count == 3)
                    {
                        glDebugRspBuffer[0] = SensorRead2B (SENSOR_ADDR_RD, dmaInfo.buffer[1], dmaInfo.buffer[2],
                        		(glDebugRspBuffer+1));
                        dmaInfo.count = 3;
                    }
                    else if (dmaInfo.count == 4)
                    {
                        if (dmaInfo.buffer[3] > 0)
                        {
                                glDebugRspBuffer[0] = SensorRead (SENSOR_ADDR_RD, dmaInfo.buffer[1], dmaInfo.buffer[2],
                                		(dmaInfo.buffer[3]*2), (glDebugRspBuffer+1));
                        }
                        dmaInfo.count = dmaInfo.buffer[3]*2+1;
                    }
                }
                /*  For write commands, the register address is followed by N pairs (N>0) of register value high byte
                 *  and register value low byte to write in sequence. Response first byte is status (0=Pass, !0=Fail)
                 *  followed by N pairs of register value high byte and register value low byte after modification.
                 */
                else if (dmaInfo.buffer[0] == 1)
                {
                        glDebugRspBuffer[0] = SensorWrite (SENSOR_ADDR_WR, dmaInfo.buffer[1], dmaInfo.buffer[2],
                        		(dmaInfo.count-3), (dmaInfo.buffer+3));
                        if (glDebugRspBuffer[0] != CY_U3P_SUCCESS)
                        	break;
                        glDebugRspBuffer[0] = SensorRead (SENSOR_ADDR_RD, dmaInfo.buffer[1], dmaInfo.buffer[2],
                        		(dmaInfo.count-3), (glDebugRspBuffer+1));
                        if (glDebugRspBuffer[0] != CY_U3P_SUCCESS)
                        	break;
                    dmaInfo.count -= 2;
                }
                /* Default case, prepare buffer for loop back command in response */
                else
                {
                   /* For now, we just copy the command into the response buffer; and send it back to the
                      USB host. This can be expanded to include I2C transfers. */
                    CyU3PMemCopy (glDebugRspBuffer, dmaInfo.buffer, dmaInfo.count);
                }

                dmaInfo.buffer = glDebugRspBuffer;
                dmaInfo.size   = 1024;
                dmaInfo.status = 0;

                /* Free the command buffer to receive the next command. */
                apiRetStatus = CyU3PDmaChannelDiscardBuffer (&glDebugCmdChannel);
                if (apiRetStatus != CY_U3P_SUCCESS)
                {
                    CyU3PDebugPrint (4, "Failed to free up command OUT EP buffer, Error code = %d\r\n", apiRetStatus);
                    CyFxAppErrorHandler (apiRetStatus);
                }

                /* Wait until the response has gone out. */
                CyU3PDmaChannelWaitForCompletion (&glDebugRspChannel, CYU3P_WAIT_FOREVER);

                apiRetStatus = CyU3PDmaChannelSetupSendBuffer (&glDebugRspChannel, &dmaInfo);
                if (apiRetStatus != CY_U3P_SUCCESS)
                {
                    CyU3PDebugPrint (4, "Failed to send debug response, Error code = %d\r\n", apiRetStatus);
                    CyFxAppErrorHandler (apiRetStatus);
                }
            }
#endif
        }

        /* Allow other ready threads to run. */
        CyU3PThreadRelinquish ();
    }
}

/*
 * This function is called by the FX3 framework once the ThreadX RTOS has started up.
 * The application specific threads and other OS resources are created and initialized here.
 */
void
CyFxApplicationDefine (
        void)
{
    void *ptr1, *ptr2;
    uint32_t retThrdCreate;

    /* Allocate the memory for the thread stacks. */
    ptr1 = CyU3PMemAlloc (UVC_APP_THREAD_STACK);
    ptr2 = CyU3PMemAlloc (UVC_APP_THREAD_STACK);
    if ((ptr1 == 0) || (ptr2 == 0))
        goto fatalErrorHandler;

    /* Create the UVC application thread. */
    retThrdCreate = CyU3PThreadCreate (&uvcAppThread,   /* UVC Thread structure */
            "30:UVC App Thread",                        /* Thread Id and name */
            UVCAppThread_Entry,                         /* UVC Application Thread Entry function */
            0,                                          /* No input parameter to thread */
            ptr1,                                       /* Pointer to the allocated thread stack */
            UVC_APP_THREAD_STACK,                       /* UVC Application Thread stack size */
            UVC_APP_THREAD_PRIORITY,                    /* UVC Application Thread priority */
            UVC_APP_THREAD_PRIORITY,                    /* Threshold value for thread pre-emption. */
            CYU3P_NO_TIME_SLICE,                        /* No time slice for the application thread */
            CYU3P_AUTO_START                            /* Start the Thread immediately */
            );
    if (retThrdCreate != 0)
    {
        goto fatalErrorHandler;
    }

    /* Create the control request handling thread. */
    retThrdCreate = CyU3PThreadCreate (&uvcAppEP0Thread,        /* UVC Thread structure */
            "31:UVC App EP0 Thread",                            /* Thread Id and name */
            UVCAppEP0Thread_Entry,                              /* UVC Application EP0 Thread Entry function */
            0,                                                  /* No input parameter to thread */
            ptr2,                                               /* Pointer to the allocated thread stack */
            UVC_APP_EP0_THREAD_STACK,                           /* UVC Application Thread stack size */
            UVC_APP_EP0_THREAD_PRIORITY,                        /* UVC Application Thread priority */
            UVC_APP_EP0_THREAD_PRIORITY,                        /* Threshold value for thread pre-emption. */
            CYU3P_NO_TIME_SLICE,                                /* No time slice for the application thread */
            CYU3P_AUTO_START                                    /* Start the Thread immediately */
            );
    if (retThrdCreate != 0)
    {
        goto fatalErrorHandler;
    }

    return;

fatalErrorHandler:
    /* Add custom recovery or debug actions here */
    /* Loop indefinitely */
    while (1);
}

/* Main entry point for the C code. We perform device initialization and start
 * the ThreadX RTOS here.
 */
int
main (
        void)
{
    CyU3PReturnStatus_t apiRetStatus;
    CyU3PIoMatrixConfig_t io_cfg;

    CyU3PSysClockConfig_t clk_cfg;
    /* Initialize the device */
    clk_cfg.setSysClk400  = CyTrue;
    clk_cfg.cpuClkDiv     = 2;
    clk_cfg.dmaClkDiv     = 2;
    clk_cfg.mmioClkDiv    = 2;
    clk_cfg.useStandbyClk = CyFalse;
    clk_cfg.clkSrc        = CY_U3P_SYS_CLK;
    apiRetStatus = CyU3PDeviceInit (&clk_cfg);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        goto handle_fatal_error;
    }

    /* Turn on instruction cache to improve firmware performance. Use Release build to improve it further */
    apiRetStatus = CyU3PDeviceCacheControl (CyTrue, CyFalse, CyFalse);

    /* Configure the IO matrix for the device. */
    io_cfg.isDQ32Bit        = CyTrue;
    io_cfg.s0Mode       	= CyFalse;
    io_cfg.s1Mode	        = CyFalse;
    io_cfg.lppMode          = CY_U3P_IO_MATRIX_LPP_DEFAULT;
    io_cfg.gpioSimpleEn[0]  = 0;
    io_cfg.gpioSimpleEn[1]  = 0;
    io_cfg.gpioComplexEn[0] = 0;
    io_cfg.gpioComplexEn[1] = 0;
    io_cfg.useUart          = CyTrue;   /* Uart is enabled for logging. */
    io_cfg.useI2C           = CyTrue;   /* I2C is used for the sensor interface. */
    io_cfg.useI2S           = CyFalse;
    io_cfg.useSpi           = CyFalse;

    apiRetStatus = CyU3PDeviceConfigureIOMatrix (&io_cfg);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        goto handle_fatal_error;
    }

    /* This is a non returnable call for initializing the RTOS kernel */
    CyU3PKernelEntry ();

    /* Dummy return to make the compiler happy */
    return 0;

handle_fatal_error:
    /* Cannot recover from this error. */
    while (1);
}

