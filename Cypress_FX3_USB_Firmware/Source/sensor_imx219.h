
#ifndef _IMX219MIPI_SENSOR_H
#define _IMX219MIPI_SENSOR_H

#include <cyu3types.h>
#include <stdbool.h>


#define SENSOR_ADDR_WR 0x20             /* Slave address used to write sensor registers. */
#define SENSOR_ADDR_RD 0x21             /* Slave address used to read from sensor registers. */
#define I2C_SLAVEADDR_MASK 0xFE         /* Mask to get actual I2C slave address value without direction bit. */

/* GPIO 22 on FX3 is used to reset the Image sensor. */
#define DEBUG1_GPIO 		23
#define DEBUG_GPIO 			27
#define SENSOR_RESET_GPIO 	22

#define _countof(array) (sizeof(array) / sizeof(array[0]))

#define GET_WORD_MSB(x)  ((x >> 8) & 0xFF)
#define GET_WORD_LSB(x)  (		 x & 0xFF)

#define IMX219_SENSOR_ID			0x0219

#define CAMERA_ID 					IMX219_SENSOR_ID

#define REG_SW_RESET 				0x0103
#define REG_MODEL_ID_MSB			0x0000
#define REG_MODEL_ID_LSB			0x0001
#define REG_MODE_SEL 				0x0100
#define REG_CSI_LANE				0x0114
#define REG_DPHY_CTRL				0x0128
#define REG_EXCK_FREQ_MSB			0x012A
#define REG_EXCK_FREQ_LSB			0x012B
#define REG_FRAME_LEN_MSB			0x0160
#define REG_FRAME_LEN_LSB			0x0161
#define REG_LINE_LEN_MSB			0x0162
#define REG_LINE_LEN_LSB			0x0163
#define REG_X_ADD_STA_MSB			0x0164
#define REG_X_ADD_STA_LSB			0x0165
#define REG_X_ADD_END_MSB			0x0166
#define REG_X_ADD_END_LSB			0x0167
#define REG_Y_ADD_STA_MSB			0x0168
#define REG_Y_ADD_STA_LSB			0x0169
#define REG_Y_ADD_END_MSB			0x016A
#define REG_Y_ADD_END_LSB			0x016B

#define REG_X_OUT_SIZE_MSB			0x016C
#define REG_X_OUT_SIZE_LSB			0x016D
#define REG_Y_OUT_SIZE_MSB			0x016E
#define REG_Y_OUT_SIZE_LSB			0x016F

#define REG_X_ODD_INC				0x0170
#define REG_Y_ODD_INC				0x0171
#define REG_IMG_ORIENT				0x0172
#define REG_BINNING_H				0x0174
#define REG_BINNING_V				0x0175
#define REG_BIN_CALC_MOD_H			0x0176
#define REG_BIN_CALC_MOD_V			0x0177

#define REG_CSI_FORMAT_C			0x018C
#define REG_CSI_FORMAT_D			0x018D

#define REG_DIG_GAIN_GLOBAL_MSB		0x0158
#define REG_DIG_GAIN_GLOBAL_LSB		0x0159
#define REG_ANA_GAIN_GLOBAL			0x0157
#define REG_INTEGRATION_TIME_MSB	0x015A
#define REG_INTEGRATION_TIME_LSB 	0x015B
#define REG_ANALOG_GAIN 			0x0157

#define REG_VTPXCK_DIV				0x0301
#define REG_VTSYCK_DIV				0x0303
#define	REG_PREPLLCK_VT_DIV			0x0304
#define	REG_PREPLLCK_OP_DIV			0x0305
#define	REG_PLL_VT_MPY_MSB			0x0306
#define	REG_PLL_VT_MPY_LSB			0x0307
#define REG_OPPXCK_DIV				0x0309
#define REG_OPSYCK_DIV				0x030B
#define REG_PLL_OP_MPY_MSB			0x030C
#define REG_PLL_OP_MPY_LSB			0x030D


#define REG_TEST_PATTERN_MSB		0x0600
#define REG_TEST_PATTERN_LSB		0x0601
#define REG_TP_RED_MSB				0x0602
#define REG_TP_RED_LSB				0x0603
#define REG_TP_GREEN_MSB			0x0604
#define REG_TP_GREEN_LSB			0x0605
#define REG_TP_BLUE_MSB				0x0606
#define REG_TP_BLUE_LSB				0x0607
#define	REG_TP_X_OFFSET_MSB			0x0620
#define	REG_TP_X_OFFSET_LSB			0x0621
#define	REG_TP_Y_OFFSET_MSB			0x0622
#define	REG_TP_Y_OFFSET_LSB			0x0623
#define	REG_TP_WIDTH_MSB			0x0624
#define	REG_TP_WIDTH_LSB			0x0625
#define	REG_TP_HEIGHT_MSB			0x0626
#define	REG_TP_HEIGHT_LSB			0x0627

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

typedef struct imgsensor_mode_struct_s {
	uint16_t 	pix_clk_mul;
	uint16_t 	pix_clk_div;
	uint8_t 	mirror;
	uint16_t 	integration;
	uint16_t 	gain;

	uint16_t 	linelength;
	uint16_t 	framelength;
	uint16_t 	startx;
	uint16_t 	starty;
	uint16_t 	endx;
	uint16_t 	endy;

	uint16_t 	width;
	uint16_t 	height;
	uint16_t 	framerate;
	uint8_t 	binning;
	uint8_t		test_pattern;
} imgsensor_mode_t;


/* SENSOR PRIVATE STRUCT FOR CONSTANT*/
typedef struct image_sensor_config_s {
	uint8_t 	sensor_mode;

	imgsensor_mode_t mode_640x480_30;		//640x480 30fps
	imgsensor_mode_t mode_1280x720_30;		//1280x720 30fps
	imgsensor_mode_t mode_1280x720_60;		//1280x720 60fps
	imgsensor_mode_t mode_1280x720_120;		//1280x720 120fps
	imgsensor_mode_t mode_1920x1080_30;		//1920x1080 30fps
	imgsensor_mode_t mode_1920x1080_60;		//1920x1080 60fps
	imgsensor_mode_t mode_640x480_200;
	imgsensor_mode_t mode_640x128_682;
	imgsensor_mode_t mode_640x128_600;
	imgsensor_mode_t mode_640x80_900;
	imgsensor_mode_t mode_640x80_1000;
	imgsensor_mode_t mode_3280x2464_15;
	imgsensor_mode_t mode_3280x2464_7;
} image_sensor_config_t;




typedef struct imx219_reg_s {
	uint16_t address;
	uint8_t val;
}imx219_reg_t;


void SensorInit (void);
void SensorReset (void);
uint8_t SensorI2cBusTest (void);
uint8_t SensorGetBrightness (void);
void SensorSetBrightness (uint8_t input);
uint16_t sensor_get_exposure (void);
uint16_t sensor_get_max_exposure();
uint16_t sensor_get_min_exposure();
uint16_t sensor_get_def_exposure();
void sensor_set_exposure (uint16_t input);
uint8_t sensor_get_test_pattern (void);
void sensor_set_test_pattern (uint8_t input);
void sensor_configure_mode(imgsensor_mode_t * mode);
void sensor_handle_uvc_control(uint8_t frame_index, uint32_t interval);

#endif 
