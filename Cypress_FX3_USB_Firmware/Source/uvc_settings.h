/*
 * uvc_settings.h
 *
 *  Created on: Jan 25, 2020
 *      Author: gaurav
 */

#ifndef UVC_SETTINGS_H_
#define UVC_SETTINGS_H_


#include "sensor_imx477.h" 	//Current driver state allow one sensor file to be included at once.
//#include "sensor_imx219.h"

#define WBVAL(x) (x & 0xFF),((x >> 8) & 0xFF)
#define DBVAL(x) (x & 0xFF),((x >> 8) & 0xFF),((x >> 16) & 0xFF),((x >> 24) & 0xFF)

#define UVC_MODE0_WIDTH								(unsigned int)SENSOR_MODE0_WIDTH
#define UVC_MODE0_HEIGHT							(unsigned int)SENSOR_MODE0_HEIGHT
#define UVC_MODE0_FPS								(unsigned int)SENSOR_MODE0_FPS
#define MIN_MODE0_BIT_RATE							(unsigned long)(UVC_MODE0_WIDTH*UVC_MODE0_HEIGHT*16*UVC_MODE0_FPS) //YUY2 4byte per 2 pixel
#define MAX_MODE0_BIT_RATE							(unsigned long)(UVC_MODE0_WIDTH*UVC_MODE0_HEIGHT*16*UVC_MODE0_FPS)
#define MAX_MODE0_FRAME_SIZE						(unsigned long)(UVC_MODE0_WIDTH*UVC_MODE0_HEIGHT*2)//YUY2 4byte per 2 pixel
#define INTERVAL_MODE0								(unsigned long)(10000000/UVC_MODE0_FPS)

#define UVC_MODE1_WIDTH								(unsigned int)SENSOR_MODE1_WIDTH
#define UVC_MODE1_HEIGHT							(unsigned int)SENSOR_MODE1_HEIGHT
#define UVC_MODE1_FPS								(unsigned int)SENSOR_MODE1_FPS
#define MIN_MODE1_BIT_RATE							(unsigned long)(UVC_MODE1_WIDTH*UVC_MODE1_HEIGHT*16*UVC_MODE1_FPS) //YUY2 4byte per 2 pixel
#define MAX_MODE1_BIT_RATE							(unsigned long)(UVC_MODE1_WIDTH*UVC_MODE1_HEIGHT*16*UVC_MODE1_FPS)
#define MAX_MODE1_FRAME_SIZE						(unsigned long)(UVC_MODE1_WIDTH*UVC_MODE1_HEIGHT*2)//YUY2 4byte per 2 pixel
#define INTERVAL_MODE1								(unsigned long)(10000000/UVC_MODE1_FPS)

#define UVC_MODE2_WIDTH								(unsigned int)SENSOR_MODE2_WIDTH
#define UVC_MODE2_HEIGHT							(unsigned int)SENSOR_MODE2_HEIGHT
#define UVC_MODE2_FPS								(unsigned int)SENSOR_MODE2_FPS
#define MIN_MODE2_BIT_RATE							(unsigned long)(UVC_MODE2_WIDTH*UVC_MODE2_HEIGHT*16*UVC_MODE2_FPS) //YUY2 4byte per 2 pixel
#define MAX_MODE2_BIT_RATE							(unsigned long)(UVC_MODE2_WIDTH*UVC_MODE2_HEIGHT*16*UVC_MODE2_FPS)
#define MAX_MODE2_FRAME_SIZE						(unsigned long)(UVC_MODE2_WIDTH*UVC_MODE2_HEIGHT*2)//YUY2 4byte per 2 pixel
#define INTERVAL_MODE2								(unsigned long)(10000000/UVC_MODE2_FPS)

#define UVC_MODE3_WIDTH								(unsigned int)SENSOR_MODE3_WIDTH
#define UVC_MODE3_HEIGHT							(unsigned int)SENSOR_MODE3_HEIGHT
#define UVC_MODE3_FPS								(unsigned int)SENSOR_MODE3_FPS
#define MIN_MODE3_BIT_RATE							(unsigned long)(UVC_MODE3_WIDTH*UVC_MODE3_HEIGHT*16*UVC_MODE3_FPS) //YUY2 4byte per 2 pixel
#define MAX_MODE3_BIT_RATE							(unsigned long)(UVC_MODE3_WIDTH*UVC_MODE3_HEIGHT*16*UVC_MODE3_FPS)
#define MAX_MODE3_FRAME_SIZE						(unsigned long)(UVC_MODE3_WIDTH*UVC_MODE3_HEIGHT*2)//YUY2 4byte per 2 pixel
#define INTERVAL_MODE3								(unsigned long)(10000000/UVC_MODE3_FPS)

#define UVC_MODE4_WIDTH								(unsigned int)SENSOR_MODE4_WIDTH
#define UVC_MODE4_HEIGHT							(unsigned int)SENSOR_MODE4_HEIGHT
#define UVC_MODE4_FPS_MIN							(unsigned int)SENSOR_MODE4_FPS_MIN
#define UVC_MODE4_FPS								(unsigned int)SENSOR_MODE4_FPS
#define MIN_MODE4_BIT_RATE							(unsigned long)(UVC_MODE4_WIDTH*UVC_MODE4_HEIGHT*16*UVC_MODE4_FPS_MIN) //YUY2 4byte per 2 pixel
#define MAX_MODE4_BIT_RATE							(unsigned long)(UVC_MODE4_WIDTH*UVC_MODE4_HEIGHT*16*UVC_MODE4_FPS)
#define MAX_MODE4_FRAME_SIZE						(unsigned long)(UVC_MODE4_WIDTH*UVC_MODE4_HEIGHT*2)//YUY2 4byte per 2 pixel
#define INTERVAL_MODE4_MIN							(unsigned long)(10000000/UVC_MODE4_FPS_MIN)
#define INTERVAL_MODE4								(unsigned long)(10000000/UVC_MODE4_FPS)

#define INTERVAL_30									(unsigned long)(10000000/30)
#define MAX_FRAME_SIZE                              (unsigned long)(UVC_MODE4_WIDTH*UVC_MODE4_HEIGHT*2)//yuy2

typedef enum
{
	FRAME_MODE0 = 1,
	FRAME_MODE1,
	FRAME_MODE2,
	FRAME_MODE3,
	FRAME_MODE4,
}frame_t;

#endif /* UVC_SETTINGS_H_ */
