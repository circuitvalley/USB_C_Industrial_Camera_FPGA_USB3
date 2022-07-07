/*
 * sensor_imx477.h
 *
 *  Created on: Jun 24, 2022
 *      Author: gaurav
 */


#ifndef _IMX477MIPI_SENSOR_H
#define _IMX477MIPI_SENSOR_H

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

#define IMX477_SENSOR_ID			0x0477

#define CAMERA_ID 					IMX477_SENSOR_ID

#define REG_SW_RESET 				0x0103
#define REG_MODEL_ID_MSB			0x0016 //also 0x0000
#define REG_MODEL_ID_LSB			0x0017 //also 0x0001
#define REG_MODE_SEL 				0x0100
#define REG_CSI_LANE				0x0114
#define REG_DPHY_CTRL				0x0808
#define REG_EXCK_FREQ_MSB			0x0136
#define REG_EXCK_FREQ_LSB			0x0137
#define REG_FRAME_LEN_MSB			0x0340
#define REG_FRAME_LEN_LSB			0x0341
#define REG_LINE_LEN_MSB			0x0342
#define REG_LINE_LEN_LSB			0x0343
#define REG_X_ADD_STA_MSB			0x0344
#define REG_X_ADD_STA_LSB			0x0345
#define REG_X_ADD_END_MSB			0x0348
#define REG_X_ADD_END_LSB			0x0349
#define REG_Y_ADD_STA_MSB			0x0346
#define REG_Y_ADD_STA_LSB			0x0347
#define REG_Y_ADD_END_MSB			0x034A
#define REG_Y_ADD_END_LSB			0x034B

#define REG_X_OUT_SIZE_MSB			0x034C
#define REG_X_OUT_SIZE_LSB			0x034D
#define REG_Y_OUT_SIZE_MSB			0x034E
#define REG_Y_OUT_SIZE_LSB			0x034F


#define REG_IMG_ORIENT				0x0101
#define REG_BINNING_MODE			0x0900
#define REG_BINNING_HV				0x0901 //Very Diff

#define REG_BINNING_WEIGHTING		0x0902
//#define REG_BIN_CALC_MOD_H			0x0176
//#define REG_BIN_CALC_MOD_V			0x0177

#define REG_CSI_FORMAT_C			0x0112		//0x08-> 8 bit 0x0A-> 10bit 0xC ->12bit
#define REG_CSI_FORMAT_D			0x0113

#define REG_ANA_GAIN_GLOBAL_MSB 		0x0204
#define REG_ANA_GAIN_GLOBAL_LSB 		0x0205

#define REG_ANA_GAIN_GLOBAL1_MSB		0x00F0
#define REG_ANA_GAIN_GLOBAL1_LSB		0x00F1
#define REG_ANA_GAIN_GLOBAL2_MSB		0x00F2
#define REG_ANA_GAIN_GLOBAL2_LSB		0x00F3
#define REG_ANA_GAIN_GLOBAL3_MSB		0x00F4
#define REG_ANA_GAIN_GLOBAL3_LSB		0x00F5

#define REG_DIG_GAIN_GLOBAL1_MSB		0x00F6
#define REG_DIG_GAIN_GLOBAL1_LSB		0x00F7
#define REG_DIG_GAIN_GLOBAL2_MSB		0x00F8
#define REG_DIG_GAIN_GLOBAL2_LSB		0x00F9
#define REG_DIG_GAIN_GLOBAL3_MSB		0x00FA
#define REG_DIG_GAIN_GLOBAL3_LSB		0x00FB

#define REG_FINE_INTEGRATION_TIME_MSB	0x0200
#define REG_FINE_INTEGRATION_TIME_LSB 	0x0201
#define REG_COARSE_INTEGRATION_TIME_MSB	0x0202
#define REG_COARSE_INTEGRATION_TIME_LSB 0x0203

#define REG_IVTPXCK_DIV				0x0301
#define REG_IVTSYCK_DIV				0x0303

#define REG_IOP_PREPLLCK_DIV		0x030D
#define	REG_IVT_PREPLLCK_DIV		0x0305
#define	REG_PLL_IVT_MPY_MSB			0x0306
#define	REG_PLL_IVT_MPY_LSB			0x0307
#define REG_IOPPXCK_DIV				0x0309
#define REG_IOPSYCK_DIV				0x030B

#define REG_IOP_MPY_MSB				0x030E
#define REG_IOP_MPY_LSB				0x030F
#define REG_PLL_MULTI_DRV			0x0310

#define REG_TEST_PATTERN_MSB		0x0600
#define REG_TEST_PATTERN_LSB		0x0601
#define REG_TP_RED_MSB				0x0602
#define REG_TP_RED_LSB				0x0603
#define REG_TP_GREENR_MSB			0x0604
#define REG_TP_GREENR_LSB			0x0605
#define REG_TP_BLUE_MSB				0x0606
#define REG_TP_BLUE_LSB				0x0607
#define REG_TP_GREENB_MSB			0x0608
#define REG_TP_GREENB_LSB			0x0609
//#define	REG_TP_X_OFFSET_MSB			0x0620
//#define	REG_TP_X_OFFSET_LSB			0x0621
//#define	REG_TP_Y_OFFSET_MSB			0x0622
//#define	REG_TP_Y_OFFSET_LSB			0x0623
//#define	REG_TP_WIDTH_MSB			0x0624
//#define	REG_TP_WIDTH_LSB			0x0625
//#define	REG_TP_HEIGHT_MSB			0x0626
//#define	REG_TP_HEIGHT_LSB			0x0627
#define REG_FRAME_BLANKSTOP_CTRL	0xE000 //whether to go out of HS and into LP mode while frame blank happen

#define REG_PD_AREA_WIDTH_MSB		0x38A8
#define REG_PD_AREA_WIDTH_LSB		0x38A9
#define REG_PD_AREA_HEIGHT_MSB		0x38AA
#define REG_PD_AREA_HEIGHT_LSB		0x38AB

#define REG_FRAME_LENGTH_CTRL		0x0350
#define REG_EBD_SIZE_V				0xBCF1
#define REG_DPGA_GLOBEL_GAIN		0x3FF9

#define REG_X_ENV_INC_CONST			0x0381
#define REG_X_ODD_INC_CONST			0x0383
#define REG_Y_ENV_INC_CONST			0x0385
#define REG_Y_ODD_INC				0x0387


#define REG_MULTI_CAM_MODE			0x3F0B
#define REG_ADC_BIT_SETTING			0x3F0D

#define REG_SCALE_MODE				0x0401
#define REG_SCALE_M_MSbit			0x0404
#define REG_SCALE_M_LSB				0x0405
#define REG_SCALE_N_MSbit			0x0406
#define REG_SCALE_N_LSB				0x0407

#define REG_DIG_CROP_X_OFFSET_MSB	0x0408
#define REG_DIG_CROP_X_OFFSET_LSB	0x0409
#define REG_DIG_CROP_Y_OFFSET_MSB	0x040A
#define REG_DIG_CROP_Y_OFFSET_LSB	0x040B
#define REG_DIG_CROP_WIDTH_MSB		0x040C
#define REG_DIG_CROP_WIDTH_LSB		0x040D
#define REG_DIG_CROP_HEIGHT_MSB		0x040E
#define REG_DIG_CROP_HEIGHT_LSB		0x040F

#define REG_REQ_LINK_BIT_RATE_MSB	0x0820
#define REG_REQ_LINK_BIT_RATE_LMSB	0x0821
#define REG_REQ_LINK_BIT_RATE_MLSB	0x0822
#define REG_REQ_LINK_BIT_RATE_LSB	0x0823

#define REG_TCLK_POST_EX_MSB		0x080A
#define REG_TCLK_POST_EX_LSB		0x080B
#define REG_THS_PRE_EX_MSB			0x080C
#define REG_THS_PRE_EX_LSB			0x080D
#define REG_THS_ZERO_MIN_MSB		0x080E
#define REG_THS_ZERO_MIN_LSB		0x080F
#define REG_THS_TRAIL_EX_MSB		0x0810
#define REG_THS_TRAIL_EX_LSB		0x0811
#define REG_TCLK_TRAIL_MIN_MSB		0x0812
#define REG_TCLK_TRAIL_MIN_LSB		0x0813
#define REG_TCLK_PREP_EX_MSB		0x0814
#define REG_TCLK_PREP_EX_LSB		0x0815
#define REG_TCLK_ZERO_EX_MSB		0x0816
#define REG_TCLK_ZERO_EX_LSB		0x0817
#define REG_TLPX_EX_MSB				0x0818
#define REG_TLPX_EX_LSB				0x0819

#define REG_PDAF_CTRL1_0			0x3E37
#define REG_POWER_SAVE_ENABLE		0x3F50

#define REG_LINE_LEN_INCLK_MSB		0x3F56
#define REG_LINE_LEN_INCLK_LSB		0x3F57

#define REG_MAP_COUPLET_CORR		0x0B05
#define REG_SING_DYNAMIC_CORR		0x0B06
#define REG_CIT_LSHIFT_LONG_EXP		0x3100

#define REG_TEMP_SENS_CTL 			0x0138

#define REG_DOL_HDR_EN				0x00E3
#define REG_DOL_HDR_NUM				0x00E4
#define REG_DOL_CSI_DT_FMT_H_2ND	0x00FC
#define REG_DOL_CSI_DT_FMT_L_2ND	0x00FD
#define REG_DOL_CSI_DT_FMT_H_3ND	0x00FE
#define REG_DOL_CSI_DT_FMT_L_3ND	0x00FF
#define REG_DOL_CONST				0xE013


#define LANES 2

#if LANES == 2
#define SENSOR_MODE0_WIDTH							(unsigned int)640
#define SENSOR_MODE0_HEIGHT							(unsigned int)480
#define SENSOR_MODE0_FPS							(unsigned int)200

#define SENSOR_MODE1_WIDTH							(unsigned int)1332
#define SENSOR_MODE1_HEIGHT							(unsigned int)990
#define SENSOR_MODE1_FPS							(unsigned int)100

#define SENSOR_MODE2_WIDTH							(unsigned int)2028
#define SENSOR_MODE2_HEIGHT							(unsigned int)1080
#define SENSOR_MODE2_FPS							(unsigned int)50

#define SENSOR_MODE3_WIDTH							(unsigned int)2028
#define SENSOR_MODE3_HEIGHT							(unsigned int)1520
#define SENSOR_MODE3_FPS							(unsigned int)35

#define SENSOR_MODE4_WIDTH							(unsigned int)4056
#define SENSOR_MODE4_HEIGHT							(unsigned int)3040
#define SENSOR_MODE4_FPS_MIN						(unsigned int)5
#define SENSOR_MODE4_FPS							(unsigned int)10
#else

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
}imx477_reg_t;

typedef struct imx477_reg_list_struct_s {
	unsigned int num_of_regs;
	imx477_reg_t *regs;
}imx477_reg_list_t;

typedef struct imgsensor_mode_struct_s {
	uint16_t 	integration_def;
	uint16_t 	integration;
	uint16_t 	integration_max;
	uint16_t	integration_min;
	uint8_t 	mirror;
	uint8_t 	sensor_mode;
	uint16_t 	width;
	uint16_t	height;
    uint16_t    frame_length;
	uint16_t	fps;
	uint16_t 	gain;
	uint16_t	gain_max;
	uint8_t 	bits;
	uint8_t		test_pattern;
	imx477_reg_list_t reg_list;
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

#endif
