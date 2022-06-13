/*
 * uvc_settings.h
 *
 *  Created on: Jan 25, 2020
 *      Author: gaurav
 */

#ifndef UVC_SETTINGS_H_
#define UVC_SETTINGS_H_


#define WBVAL(x) (x & 0xFF),((x >> 8) & 0xFF)
#define DBVAL(x) (x & 0xFF),((x >> 8) & 0xFF),((x >> 16) & 0xFF),((x >> 24) & 0xFF)

#define UVC_WIDTH                                     (unsigned int)3280
#define UVC_HEIGHT                                    (unsigned int)2462

#define CAM_FPS_1000                                  1000
#define CAM_FPS_682                                   682
#define CAM_FPS_200                                   200
#define CAM_FPS_120                                   120
#define CAM_FPS_60                                    60
#define CAM_FPS_30                                    30
#define CAM_FPS_15                                    15
#define CAM_FPS_7                                     7

#define INTERVAL_1000								(unsigned long)(10000000/CAM_FPS_1000)
#define INTERVAL_682								(unsigned long)(10000000/CAM_FPS_682)
#define INTERVAL_200								(unsigned long)(10000000/CAM_FPS_200)
#define INTERVAL_120								(unsigned long)(10000000/CAM_FPS_120)
#define INTERVAL_60									(unsigned long)(10000000/CAM_FPS_60)
#define INTERVAL_30									(unsigned long)(10000000/CAM_FPS_30)
#define INTERVAL_15									(unsigned long)(10000000/CAM_FPS_15)
#define INTERVAL_7									(unsigned long)(10000000/CAM_FPS_7)

//640x480 supports 30 and 200FPS
#define UVC_WIDTH_640								(unsigned int)640
#define UVC_HEIGHT_126								(unsigned int)126
#define MIN_BIT_RATE_640x126						(unsigned long)(UVC_WIDTH_640*UVC_HEIGHT_126*16*CAM_FPS_682) //YUY2 4byte per 2 pixel
#define MAX_BIT_RATE_640x126						(unsigned long)(UVC_WIDTH_640*UVC_HEIGHT_126*16*CAM_FPS_682)
#define MAX_FRAME_SIZE_640x126						(unsigned long)(UVC_WIDTH_640*UVC_HEIGHT_126*2)//YUY2 4byte per 2 pixel

#define UVC_WIDTH_640								(unsigned int)640
#define UVC_HEIGHT_78								(unsigned int)78
#define MIN_BIT_RATE_640x78							(unsigned long)(UVC_WIDTH_640*UVC_HEIGHT_78*16*CAM_FPS_1000) //YUY2 4byte per 2 pixel
#define MAX_BIT_RATE_640x78							(unsigned long)(UVC_WIDTH_640*UVC_HEIGHT_78*16*CAM_FPS_1000)
#define MAX_FRAME_SIZE_640x78						(unsigned long)(UVC_WIDTH_640*UVC_HEIGHT_78*2)//YUY2 4byte per 2 pixel

#define UVC_WIDTH_640								(unsigned int)650
#define UVC_HEIGHT_480								(unsigned int)480
#define MIN_BIT_RATE_640x480						(unsigned long)(UVC_WIDTH_640*UVC_HEIGHT_480*16*CAM_FPS_30) //YUY2 4byte per 2 pixel
#define MAX_BIT_RATE_640x480						(unsigned long)(UVC_WIDTH_640*UVC_HEIGHT_480*16*CAM_FPS_200)
#define MAX_FRAME_SIZE_640x480						(unsigned long)(UVC_WIDTH_640*UVC_HEIGHT_480*2)//YUY2 4byte per 2 pixel

#define UVC_WIDTH_1280								(unsigned int)1290
#define UVC_HEIGHT_720								(unsigned int)720
#define MIN_BIT_RATE_1280x720						(unsigned long)(UVC_WIDTH_1280*UVC_HEIGHT_720*16*CAM_FPS_30)
#define MAX_BIT_RATE_1280x720						(unsigned long)(UVC_WIDTH_1280*UVC_HEIGHT_720*16*CAM_FPS_120)
#define MAX_FRAME_SIZE_1280x720						(unsigned long)(UVC_WIDTH_1280*UVC_HEIGHT_720*2)

#define UVC_WIDTH_1920								(unsigned int)1930
#define UVC_HEIGHT_1080								(unsigned int)1080
#define MIN_BIT_RATE_1920x1080						(unsigned long)(UVC_WIDTH_1920*UVC_HEIGHT_1080*16*CAM_FPS_30)
#define MAX_BIT_RATE_1920x1080						(unsigned long)(UVC_WIDTH_1920*UVC_HEIGHT_1080*16*CAM_FPS_60)
#define MAX_FRAME_SIZE_1920x1080					(unsigned long)(UVC_WIDTH_1920*UVC_HEIGHT_1080*2)

#define UVC_WIDTH_3280								(unsigned int)3290
#define UVC_HEIGHT_2462								(unsigned int)2464
#define MIN_BIT_RATE_3280x2462						(unsigned long)(UVC_WIDTH_3280*UVC_HEIGHT_2462*16*CAM_FPS_15)
#define MAX_BIT_RATE_3280x2462						(unsigned long)(UVC_WIDTH_3280*UVC_HEIGHT_2462*16*CAM_FPS_15)
#define MAX_FRAME_SIZE_3280x2462					(unsigned long)(UVC_WIDTH_3280*UVC_HEIGHT_2462*2)


#define MAX_FRAME_SIZE                              (unsigned long)(UVC_WIDTH_3280*UVC_HEIGHT_2462*2)//yuy2


typedef enum
{
	FRAME_640x480 = 1,
	FRAME_1280x720,
	FRAME_1920x1080,
	FRAME_3280x2462,
	FRMAE_640x128,
	FRMAE_640x80,
}frame_t;

#endif /* UVC_SETTINGS_H_ */
