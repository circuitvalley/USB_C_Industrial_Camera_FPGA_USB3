/*
 * sensor_imx290.h
 *
 *  Created on: Jun 24, 2022
 *      Author: gaurav
 */


#ifndef _IMX290MIPI_SENSOR_H
#define _IMX290MIPI_SENSOR_H

#include <cyu3types.h>
#include <stdbool.h>


#define SENSOR_ADDR_WR 0x34             /* Slave address used to write sensor registers. */
#define SENSOR_ADDR_RD 0x35             /* Slave address used to read from sensor registers. */
#define I2C_SLAVEADDR_MASK 0xFE         /* Mask to get actual I2C slave address value without direction bit. */

/* GPIO 22 on FX3 is used to reset the Image sensor. */
#define DEBUG1_GPIO 		23
#define DEBUG_GPIO 			27
#define SENSOR_RESET_GPIO 	22

#define _countof(array) (sizeof(array) / sizeof(array[0]))

#define GET_WORD_MSB(x)  ((x >> 8) & 0xFF)
#define GET_WORD_LSB(x)  (		 x & 0xFF)

#define IMX290_SENSOR_ID			0x0290

#define CAMERA_ID 					IMX290_SENSOR_ID


#define REG_MODEL_ID_MSB			0x0016 //also 0x0000
#define REG_MODEL_ID_LSB			0x0017 //also 0x0001



#define IMX290_HMAX_MIN 	2200 /* Min of 2200 pixels = 60fps */
#define IMX290_HMAX_MAX 	0xffff


#define IMX290_PGCTRL_REGEN 	BIT(0)
#define IMX290_PGCTRL_THRU 		BIT(1)
#define IMX290_PGCTRL_MODE(n) 	((n) << 4)

#define IMX290_NATIVE_WIDTH			1945U
#define IMX290_NATIVE_HEIGHT		1109U
#define IMX290_PIXEL_ARRAY_LEFT		4U
#define IMX290_PIXEL_ARRAY_TOP		12U
#define IMX290_PIXEL_ARRAY_WIDTH	1937U
#define IMX290_PIXEL_ARRAY_HEIGHT	1097U


#define REG_FLIP_WINMODE 			0x3007
#define REG_PGCTRL 					0x308c
#define REG_VMAX_LOW 				0x3018
#define REG_VMAX_MAX 				0x3fff
#define REG_HMAX_LOW 				0x301c
#define REG_HMAX_HIGH 				0x301d
#define REG_EXPOSURE_LOW 			0x3020
#define REG_STANDBY					0x3000
#define REG_REGHOLD 				0x3001
#define REG_XMSTA 					0x3002
#define REG_SW_RESET 				0x3003
#define REG_GAIN 					0x3014
#define REG_WINMODE 				0x3007
#define REG_VMAX_LSB 				0x301A
#define REG_WINWV_OB 				0x303A
#define REG_WINPH_MSB				0x3040
#define REG_WINPH_LSB 				0x3041
#define REG_WINPV_MSB 				0x303c
#define REG_WINPV_LSB 				0x303d
#define REG_WINWH_MSB 				0x3042
#define REG_WINWH_LSB 				0x3043
#define REG_WINWV_MSB 				0x303e
#define REG_WINWV_LSB 				0x303f
#define REG_XVSHSOUTSEL 			0x304b
#define REG_INCKSEL1 				0x305c
#define REG_INCKSEL2 				0x305d
#define REG_INCKSEL3 				0x305e
#define REG_INCKSEL4 				0x305f
#define REG_INCKSEL5 				0x315e
#define REG_INCKSEL6 				0x3164
#define REG_EXTCK_FREQ_MSB 			0x3444
#define REG_EXTCK_FREQ_LSB 			0x3445
#define REG_FR_FDG_SEL 				0x3009
#define REG_OPB_SIZE_V 				0x3414
#define REG_X_OUT_SIZE_MSB 			0x3472
#define REG_X_OUT_SIZE_LSB 			0x3473
#define REG_Y_OUT_SIZE_MSB 			0x3418
#define REG_Y_OUT_SIZE_LSB 			0x3419
#define REG_PHY_LANE_NUM 			0x3407
#define REG_CSI_LANE_MODE 			0x3443
#define REG_REPETITION				0x3405

#define REG_TCLKPOST_MSB 			0x3446
#define REG_TCLKPOST_LSB 			0x3447
#define REG_THSZERO_MSB 			0x3448
#define REG_THSZERO_LSB				0x3449
#define REG_THSPREPARE_MSB 			0x344a
#define REG_THSPREPARE_LSB 			0x344b
#define REG_TCLKTRAIL_MSB 			0x344c
#define REG_TCLKTRAIL_LSB 			0x344d
#define REG_THSTRAIL_MSB 			0x344e
#define REG_THSTRAIL_LSB 			0x344f
#define REG_TCLKZERO_MSB 			0x3450
#define REG_TCLKZERO_LSB 			0x3451
#define REG_TCLKPREPARE_MSB 		0x3452
#define REG_TCLKPREPARE_LSB 		0x3453
#define REG_TLPX_MSB				0x3454
#define REG_TLPX_LSB 				0x3455
#define REG_ADBIT 					0x3005
#define REG_PORTBIT_SEL 			0x3046
#define REG_ADBIT1 					0x3129
#define REG_ADBIT2 					0x317c
#define REG_ADBIT3 					0x31ec
#define REG_CSI_DT_FMT_MSB 			0x3441
#define REG_CSI_DT_FMT_LSB 			0x3442
#define REG_BLACK_LEVEL_MSB 		0x300a
#define REG_BLACK_LEVEL_LSB 		0x300b

#define LANES 4

#if LANES == 2


#define SENSOR_MODE0_WIDTH							(unsigned int)640
#define SENSOR_MODE0_HEIGHT							(unsigned int)78
#define SENSOR_MODE0_FPS							(unsigned int)1000

#define SENSOR_MODE1_WIDTH							(unsigned int)1332
#define SENSOR_MODE1_HEIGHT							(unsigned int)990
#define SENSOR_MODE1_FPS							(unsigned int)200

#define SENSOR_MODE2_WIDTH							(unsigned int)2028
#define SENSOR_MODE2_HEIGHT							(unsigned int)1080
#define SENSOR_MODE2_FPS							(unsigned int)100

#define SENSOR_MODE3_WIDTH							(unsigned int)2028
#define SENSOR_MODE3_HEIGHT							(unsigned int)1520
#define SENSOR_MODE3_FPS							(unsigned int)70

#define SENSOR_MODE4_WIDTH							(unsigned int)4056
#define SENSOR_MODE4_HEIGHT							(unsigned int)3040
#define SENSOR_MODE4_FPS_MIN						(unsigned int)10
#define SENSOR_MODE4_FPS							(unsigned int)20

#else

#define SENSOR_MODE0_WIDTH							(unsigned int)640
#define SENSOR_MODE0_HEIGHT							(unsigned int)480
#define SENSOR_MODE0_FPS							(unsigned int)200

#define SENSOR_MODE1_WIDTH							(unsigned int)1280
#define SENSOR_MODE1_HEIGHT							(unsigned int)720
#define SENSOR_MODE1_FPS							(unsigned int)100

#define SENSOR_MODE2_WIDTH							(unsigned int)1920
#define SENSOR_MODE2_HEIGHT							(unsigned int)1080
#define SENSOR_MODE2_FPS							(unsigned int)50

#define SENSOR_MODE3_WIDTH							(unsigned int)1920
#define SENSOR_MODE3_HEIGHT							(unsigned int)1080
#define SENSOR_MODE3_FPS							(unsigned int)35

#define SENSOR_MODE4_WIDTH							(unsigned int)1920
#define SENSOR_MODE4_HEIGHT							(unsigned int)1080
#define SENSOR_MODE4_FPS_MIN						(unsigned int)5
#define SENSOR_MODE4_FPS							(unsigned int)10

#endif

typedef enum{
	IMGSENSOR_MODE_INIT,
	IMGSENSOR_MODE_PREVIEW,
	IMGSENSOR_MODE_CAPTURE,
	IMGSENSOR_MODE_VIDEO,
	IMGSENSOR_MODE_HIGH_SPEED_VIDEO,
	IMGSENSOR_MODE_SLIM_VIDEO,
} IMGSENSOR_MODE;

enum
{
    IMAGE_NORMAL=0,
    IMAGE_H_MIRROR,
    IMAGE_V_MIRROR,
    IMAGE_HV_MIRROR
};

typedef struct imx477_reg_s {
	uint16_t address;
	uint8_t val;
}imx290_reg_t;

typedef struct imx477_reg_list_struct_s {
	unsigned int num_of_regs;
	imx290_reg_t *regs;
}imx290_reg_list_t;

typedef struct imgsensor_mode_struct_s {
	uint16_t 	integration_def;
	uint16_t 	integration;
	uint16_t 	integration_max;
	uint16_t	integration_min;
	uint8_t 	mirror;
	uint8_t 	sensor_mode;
	uint16_t 	width;
	uint16_t	height;
	uint16_t	fps;
	uint16_t 	gain;
	uint16_t	gain_max;
	uint8_t 	bits;
	uint16_t	hmax;
	uint16_t	vmax;
	uint8_t		test_pattern;
	imx290_reg_list_t mode_reg_list;
	imx290_reg_list_t lane_reg_list;
	imx290_reg_list_t clk_reg_list;
	imx290_reg_list_t bit_reg_list;
} imgsensor_mode_t;


void SensorInit (void);
void SensorReset (void);
uint8_t SensorI2cBusTest (void);
uint8_t SensorGetBrightness (void);
uint16_t getMaxBrightness(void);
void SensorSetBrightness (uint16_t input);
uint16_t sensor_get_exposure (void);
uint16_t sensor_get_max_exposure();
uint16_t sensor_get_min_exposure();
uint16_t sensor_get_def_exposure();
void sensor_set_exposure (uint16_t input);
uint8_t sensor_get_test_pattern (void);
void sensor_set_test_pattern (uint8_t input);
void sensor_configure_mode(imgsensor_mode_t * mode);
void sensor_handle_uvc_control(uint8_t frame_index, uint32_t interval);
static CyU3PReturnStatus_t sensor_write_buffered (uint16_t reg_addr, uint8_t n_regs, uint32_t data);

#endif
