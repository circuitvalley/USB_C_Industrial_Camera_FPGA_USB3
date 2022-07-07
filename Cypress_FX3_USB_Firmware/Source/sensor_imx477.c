/*
 * sensor_imx477.c
 *
 *  Created on: Jun 24, 2022
 *      Author: gaurav
 */


#include <cyu3system.h>
#include <cyu3os.h>
#include <cyu3dma.h>
#include <cyu3error.h>
#include <cyu3uart.h>
#include <cyu3i2c.h>
#include <cyu3types.h>
#include <cyu3gpio.h>
#include <cyu3utils.h>
#include "sensor_imx477.h"
#include "uvc_settings.h"

imgsensor_mode_t *sensor_config;
imgsensor_mode_t *selected_img_mode;

static const imx477_reg_t mode_default[]={	//default register settings, Resolution and FPS specific settings will be over written
			{REG_EXCK_FREQ_MSB, 0x18},
			{REG_EXCK_FREQ_LSB, 0x00},
			{REG_TEMP_SENS_CTL, 0x01},		//temprature sensor control
			{REG_FRAME_BLANKSTOP_CTRL, 0x01}, //whether to go out of HS and into LP mode while frame blank happen
			{0xe07a, 0x01},
			{REG_DPHY_CTRL, 0x02},
			{0x4ae9, 0x18},
			{0x4aea, 0x08},
			{0xf61c, 0x04},
			{0xf61e, 0x04},
			{0x4ae9, 0x21},
			{0x4aea, 0x80},
			{REG_PD_AREA_WIDTH_MSB, 0x1f},
			{REG_PD_AREA_WIDTH_LSB, 0xff},
			{REG_PD_AREA_HEIGHT_MSB, 0x1f},
			{REG_PD_AREA_HEIGHT_LSB, 0xff},
			{0x55d4, 0x00},	//unkown? Maybe calibration or quality control
			{0x55d5, 0x00},
			{0x55d6, 0x07},
			{0x55d7, 0xff},
			{0x55e8, 0x07},
			{0x55e9, 0xff},
			{0x55ea, 0x00},
			{0x55eb, 0x00},
			{0x574c, 0x07},
			{0x574d, 0xff},
			{0x574e, 0x00},
			{0x574f, 0x00},
			{0x5754, 0x00},
			{0x5755, 0x00},
			{0x5756, 0x07},
			{0x5757, 0xff},
			{0x5973, 0x04},
			{0x5974, 0x01},
			{0x5d13, 0xc3},
			{0x5d14, 0x58},
			{0x5d15, 0xa3},
			{0x5d16, 0x1d},
			{0x5d17, 0x65},
			{0x5d18, 0x8c},
			{0x5d1a, 0x06},
			{0x5d1b, 0xa9},
			{0x5d1c, 0x45},
			{0x5d1d, 0x3a},
			{0x5d1e, 0xab},
			{0x5d1f, 0x15},
			{0x5d21, 0x0e},
			{0x5d22, 0x52},
			{0x5d23, 0xaa},
			{0x5d24, 0x7d},
			{0x5d25, 0x57},
			{0x5d26, 0xa8},
			{0x5d37, 0x5a},
			{0x5d38, 0x5a},
			{0x5d77, 0x7f},
			{0x7b75, 0x0e},
			{0x7b76, 0x0b},
			{0x7b77, 0x08},
			{0x7b78, 0x0a},
			{0x7b79, 0x47},
			{0x7b7c, 0x00},
			{0x7b7d, 0x00},
			{0x8d1f, 0x00},
			{0x8d27, 0x00},
			{0x9004, 0x03},
			{0x9200, 0x50},
			{0x9201, 0x6c},
			{0x9202, 0x71},
			{0x9203, 0x00},
			{0x9204, 0x71},
			{0x9205, 0x01},
			{0x9371, 0x6a},
			{0x9373, 0x6a},
			{0x9375, 0x64},
			{0x991a, 0x00},
			{0x996b, 0x8c},
			{0x996c, 0x64},
			{0x996d, 0x50},
			{0x9a4c, 0x0d},
			{0x9a4d, 0x0d},
			{0xa001, 0x0a},
			{0xa003, 0x0a},
			{0xa005, 0x0a},
			{0xa006, 0x01},
			{0xa007, 0xc0},
			{0xa009, 0xc0},
			{0x3d8a, 0x01},
			{0x4421, 0x04},
			{0x7b3b, 0x01},
			{0x7b4c, 0x00},
			{0x9905, 0x00},
			{0x9907, 0x00},
			{0x9909, 0x00},
			{0x990b, 0x00},
			{0x9944, 0x3c},
			{0x9947, 0x3c},
			{0x994a, 0x8c},
			{0x994b, 0x50},
			{0x994c, 0x1b},
			{0x994d, 0x8c},
			{0x994e, 0x50},
			{0x994f, 0x1b},
			{0x9950, 0x8c},
			{0x9951, 0x1b},
			{0x9952, 0x0a},
			{0x9953, 0x8c},
			{0x9954, 0x1b},
			{0x9955, 0x0a},
			{0x9a13, 0x04},
			{0x9a14, 0x04},
			{0x9a19, 0x00},
			{0x9a1c, 0x04},
			{0x9a1d, 0x04},
			{0x9a26, 0x05},
			{0x9a27, 0x05},
			{0x9a2c, 0x01},
			{0x9a2d, 0x03},
			{0x9a2f, 0x05},
			{0x9a30, 0x05},
			{0x9a41, 0x00},
			{0x9a46, 0x00},
			{0x9a47, 0x00},
			{0x9c17, 0x35},
			{0x9c1d, 0x31},
			{0x9c29, 0x50},
			{0x9c3b, 0x2f},
			{0x9c41, 0x6b},
			{0x9c47, 0x2d},
			{0x9c4d, 0x40},
			{0x9c6b, 0x00},
			{0x9c71, 0xc8},
			{0x9c73, 0x32},
			{0x9c75, 0x04},
			{0x9c7d, 0x2d},
			{0x9c83, 0x40},
			{0x9c94, 0x3f},
			{0x9c95, 0x3f},
			{0x9c96, 0x3f},
			{0x9c97, 0x00},
			{0x9c98, 0x00},
			{0x9c99, 0x00},
			{0x9c9a, 0x3f},
			{0x9c9b, 0x3f},
			{0x9c9c, 0x3f},
			{0x9ca0, 0x0f},
			{0x9ca1, 0x0f},
			{0x9ca2, 0x0f},
			{0x9ca3, 0x00},
			{0x9ca4, 0x00},
			{0x9ca5, 0x00},
			{0x9ca6, 0x1e},
			{0x9ca7, 0x1e},
			{0x9ca8, 0x1e},
			{0x9ca9, 0x00},
			{0x9caa, 0x00},
			{0x9cab, 0x00},
			{0x9cac, 0x09},
			{0x9cad, 0x09},
			{0x9cae, 0x09},
			{0x9cbd, 0x50},
			{0x9cbf, 0x50},
			{0x9cc1, 0x50},
			{0x9cc3, 0x40},
			{0x9cc5, 0x40},
			{0x9cc7, 0x40},
			{0x9cc9, 0x0a},
			{0x9ccb, 0x0a},
			{0x9ccd, 0x0a},
			{0x9d17, 0x35},
			{0x9d1d, 0x31},
			{0x9d29, 0x50},
			{0x9d3b, 0x2f},
			{0x9d41, 0x6b},
			{0x9d47, 0x42},
			{0x9d4d, 0x5a},
			{0x9d6b, 0x00},
			{0x9d71, 0xc8},
			{0x9d73, 0x32},
			{0x9d75, 0x04},
			{0x9d7d, 0x42},
			{0x9d83, 0x5a},
			{0x9d94, 0x3f},
			{0x9d95, 0x3f},
			{0x9d96, 0x3f},
			{0x9d97, 0x00},
			{0x9d98, 0x00},
			{0x9d99, 0x00},
			{0x9d9a, 0x3f},
			{0x9d9b, 0x3f},
			{0x9d9c, 0x3f},
			{0x9d9d, 0x1f},
			{0x9d9e, 0x1f},
			{0x9d9f, 0x1f},
			{0x9da0, 0x0f},
			{0x9da1, 0x0f},
			{0x9da2, 0x0f},
			{0x9da3, 0x00},
			{0x9da4, 0x00},
			{0x9da5, 0x00},
			{0x9da6, 0x1e},
			{0x9da7, 0x1e},
			{0x9da8, 0x1e},
			{0x9da9, 0x00},
			{0x9daa, 0x00},
			{0x9dab, 0x00},
			{0x9dac, 0x09},
			{0x9dad, 0x09},
			{0x9dae, 0x09},
			{0x9dc9, 0x0a},
			{0x9dcb, 0x0a},
			{0x9dcd, 0x0a},
			{0x9e17, 0x35},
			{0x9e1d, 0x31},
			{0x9e29, 0x50},
			{0x9e3b, 0x2f},
			{0x9e41, 0x6b},
			{0x9e47, 0x2d},
			{0x9e4d, 0x40},
			{0x9e6b, 0x00},
			{0x9e71, 0xc8},
			{0x9e73, 0x32},
			{0x9e75, 0x04},
			{0x9e94, 0x0f},
			{0x9e95, 0x0f},
			{0x9e96, 0x0f},
			{0x9e97, 0x00},
			{0x9e98, 0x00},
			{0x9e99, 0x00},
			{0x9ea0, 0x0f},
			{0x9ea1, 0x0f},
			{0x9ea2, 0x0f},
			{0x9ea3, 0x00},
			{0x9ea4, 0x00},
			{0x9ea5, 0x00},
			{0x9ea6, 0x3f},
			{0x9ea7, 0x3f},
			{0x9ea8, 0x3f},
			{0x9ea9, 0x00},
			{0x9eaa, 0x00},
			{0x9eab, 0x00},
			{0x9eac, 0x09},
			{0x9ead, 0x09},
			{0x9eae, 0x09},
			{0x9ec9, 0x0a},
			{0x9ecb, 0x0a},
			{0x9ecd, 0x0a},
			{0x9f17, 0x35},
			{0x9f1d, 0x31},
			{0x9f29, 0x50},
			{0x9f3b, 0x2f},
			{0x9f41, 0x6b},
			{0x9f47, 0x42},
			{0x9f4d, 0x5a},
			{0x9f6b, 0x00},
			{0x9f71, 0xc8},
			{0x9f73, 0x32},
			{0x9f75, 0x04},
			{0x9f94, 0x0f},
			{0x9f95, 0x0f},
			{0x9f96, 0x0f},
			{0x9f97, 0x00},
			{0x9f98, 0x00},
			{0x9f99, 0x00},
			{0x9f9a, 0x2f},
			{0x9f9b, 0x2f},
			{0x9f9c, 0x2f},
			{0x9f9d, 0x00},
			{0x9f9e, 0x00},
			{0x9f9f, 0x00},
			{0x9fa0, 0x0f},
			{0x9fa1, 0x0f},
			{0x9fa2, 0x0f},
			{0x9fa3, 0x00},
			{0x9fa4, 0x00},
			{0x9fa5, 0x00},
			{0x9fa6, 0x1e},
			{0x9fa7, 0x1e},
			{0x9fa8, 0x1e},
			{0x9fa9, 0x00},
			{0x9faa, 0x00},
			{0x9fab, 0x00},
			{0x9fac, 0x09},
			{0x9fad, 0x09},
			{0x9fae, 0x09},
			{0x9fc9, 0x0a},
			{0x9fcb, 0x0a},
			{0x9fcd, 0x0a},
			{0xa14b, 0xff},
			{0xa151, 0x0c},
			{0xa153, 0x50},
			{0xa155, 0x02},
			{0xa157, 0x00},
			{0xa1ad, 0xff},
			{0xa1b3, 0x0c},
			{0xa1b5, 0x50},
			{0xa1b9, 0x00},
			{0xa24b, 0xff},
			{0xa257, 0x00},
			{0xa2ad, 0xff},
			{0xa2b9, 0x00},
			{0xb21f, 0x04},
			{0xb35c, 0x00},
			{0xb35e, 0x08},
			{REG_CSI_FORMAT_C, 0x0c},
			{REG_CSI_FORMAT_D, 0x0c},
			{REG_CSI_LANE, 0x01},
			{REG_FRAME_LENGTH_CTRL, 0x00},
			{REG_EBD_SIZE_V, 0x02},
			{REG_DPGA_GLOBEL_GAIN, 0x01}
};


/* 12 mpix */
static  imx477_reg_t mode_4056x3040_regs[] = {
	{REG_LINE_LEN_MSB, 0x5D},
	{REG_LINE_LEN_LSB, 0xC0},
	{REG_X_ADD_STA_MSB, 0x00},
	{REG_X_ADD_STA_LSB, 0x00},
	{REG_Y_ADD_STA_MSB, 0x00},
	{REG_Y_ADD_STA_LSB, 0x00},
	{REG_X_ADD_END_MSB, 0x0f},
	{REG_X_ADD_END_LSB, 0xd7},
	{REG_Y_ADD_END_MSB, 0x0B},
	{REG_Y_ADD_END_LSB, 0xDF},
    {REG_DOL_HDR_EN, 0x00},
    {REG_DOL_HDR_NUM, 0x00},
    {REG_DOL_CSI_DT_FMT_H_2ND, 0x0a},
    {REG_DOL_CSI_DT_FMT_L_2ND, 0x0a},
    {REG_DOL_CSI_DT_FMT_H_3ND, 0x0a},
    {REG_DOL_CSI_DT_FMT_L_3ND, 0x0a},
	{0x0220, 0x00},
	{0x0221, 0x11},
	{REG_X_ENV_INC_CONST, 0x01},
	{REG_X_ODD_INC_CONST, 0x01},
	{REG_Y_ENV_INC_CONST, 0x01},
	{REG_Y_ODD_INC, 0x01},
	{REG_BINNING_MODE, 0x00},
	{REG_BINNING_HV, 0x11},
	{REG_BINNING_WEIGHTING, 0x02},
	{0x3140, 0x02},
	{0x3c00, 0x00},
	{0x3c01, 0x03},
	{0x3c02, 0xa2},
	{REG_ADC_BIT_SETTING, 0x01},
	{0x5748, 0x07},
	{0x5749, 0xff},
	{0x574a, 0x00},
	{0x574b, 0x00},
    {0x7b75, 0x0a},
    {0x7b76, 0x0c},
    {0x7b77, 0x07},
    {0x7b78, 0x06},
    {0x7b79, 0x3c},
	{0x7b53, 0x01},
	{0x9369, 0x5A},
	{0x936b, 0x55},
	{0x936d, 0x28},
	{0x9304, 0x00},
	{0x9305, 0x00},
	{0x9e9a, 0x2f},
	{0x9e9b, 0x2f},
	{0x9e9c, 0x2f},
	{0x9e9d, 0x00},
	{0x9e9e, 0x00},
	{0x9e9f, 0x00},
	{0xa2a9, 0x60},
	{0xa2b7, 0x00},
    {REG_SCALE_MODE, 0x00},
	{REG_SCALE_M_MSbit, 0x00},
	{REG_SCALE_M_LSB, 0x10},
	{REG_DIG_CROP_X_OFFSET_MSB, 0x00},
	{REG_DIG_CROP_X_OFFSET_LSB, 0x00},
	{REG_DIG_CROP_Y_OFFSET_MSB, 0x00},
	{REG_DIG_CROP_Y_OFFSET_LSB, 0x00},
	{REG_DIG_CROP_WIDTH_MSB, 0x0f},
	{REG_DIG_CROP_WIDTH_LSB, 0xd8},
	{REG_DIG_CROP_HEIGHT_MSB, 0x0b},
	{REG_DIG_CROP_HEIGHT_LSB, 0xE0},
	{REG_X_OUT_SIZE_MSB, 0x0F},     //change to 1920
	{REG_X_OUT_SIZE_LSB, 0xd8},
	{REG_Y_OUT_SIZE_MSB, 0x0B},
	{REG_Y_OUT_SIZE_LSB, 0xE0},
	{REG_IVTPXCK_DIV, 0x05},
	{REG_IVTSYCK_DIV, 0x02},
	{REG_IVT_PREPLLCK_DIV, 0x02},
	{REG_PLL_IVT_MPY_MSB, 0x00},
	{REG_PLL_IVT_MPY_LSB, 0x9B},
	{REG_IOPPXCK_DIV, 0x0c}, //decided by output bit width
	{REG_IOPSYCK_DIV, 0x02},
	{REG_IOP_PREPLLCK_DIV, 0x02},
	{REG_IOP_MPY_MSB, 0x00},
	{REG_IOP_MPY_LSB, 0x85},
	{REG_PLL_MULTI_DRV, 0x01},
	{REG_REQ_LINK_BIT_RATE_MSB, 0x07},
	{REG_REQ_LINK_BIT_RATE_LMSB, 0x08},
	{REG_REQ_LINK_BIT_RATE_MLSB, 0x00},
	{REG_REQ_LINK_BIT_RATE_LSB, 0x00},
	{REG_TCLK_POST_EX_MSB, 0x00},
	{REG_TCLK_POST_EX_LSB, 0x7f},
	{REG_THS_PRE_EX_MSB, 0x00},
	{REG_THS_PRE_EX_LSB, 0x4f},
	{REG_THS_ZERO_MIN_MSB, 0x00},
	{REG_THS_ZERO_MIN_LSB, 0x77},
	{REG_THS_TRAIL_EX_MSB, 0x00},
	{REG_THS_TRAIL_EX_LSB, 0x5f},
	{REG_TCLK_TRAIL_MIN_MSB, 0x00},
	{REG_TCLK_TRAIL_MIN_LSB, 0x57},
	{REG_TCLK_PREP_EX_MSB, 0x00},
	{REG_TCLK_PREP_EX_LSB, 0x4f},
	{REG_TCLK_ZERO_EX_MSB, 0x01},
	{REG_TCLK_ZERO_EX_LSB, 0x27},
	{REG_TLPX_EX_MSB, 0x00},
	{REG_TLPX_EX_LSB, 0x3f},
	{0xe04c, 0x00},
	{0xe04d, 0x7f},
	{0xe04e, 0x00},
	{0xe04f, 0x1f},
	{0x3e20, 0x01},
	{REG_PDAF_CTRL1_0, 0x00},
	{REG_POWER_SAVE_ENABLE, 0x00},
	{REG_LINE_LEN_INCLK_MSB, 0x02},
	{REG_LINE_LEN_INCLK_LSB, 0xAE},
};



/* 1080p cropped mode */
static imx477_reg_t mode_2028x1080_regs[] = {
	{REG_LINE_LEN_MSB, 0x31},
	{REG_LINE_LEN_LSB, 0xc4},
	{REG_X_ADD_STA_MSB, 0x00},
	{REG_X_ADD_STA_LSB, 0x00},
	{REG_Y_ADD_STA_MSB, 0x01},
	{REG_Y_ADD_STA_LSB, 0xb8},
	{REG_X_ADD_END_MSB, 0x0f},
	{REG_X_ADD_END_LSB, 0xd7},
	{REG_Y_ADD_END_MSB, 0x0a},
	{REG_Y_ADD_END_LSB, 0x27},
	{0x0220, 0x00},
	{0x0221, 0x11},
	{REG_X_ENV_INC_CONST, 0x01},
	{REG_X_ODD_INC_CONST, 0x01},
	{REG_Y_ENV_INC_CONST, 0x01},
	{REG_Y_ODD_INC, 0x01},
	{REG_BINNING_MODE, 0x01},
	{REG_BINNING_HV, 0x12},
	{REG_BINNING_WEIGHTING, 0x02},
	{0x3140, 0x02},
	{0x3c00, 0x00},
	{0x3c01, 0x03},
	{0x3c02, 0xa2},
	{REG_ADC_BIT_SETTING, 0x01},
	{0x5748, 0x07},
	{0x5749, 0xff},
	{0x574a, 0x00},
	{0x574b, 0x00},
	{0x7b53, 0x01},
	{0x9369, 0x73},
	{0x936b, 0x64},
	{0x936d, 0x5f},
	{0x9304, 0x00},
	{0x9305, 0x00},
	{0x9e9a, 0x2f},
	{0x9e9b, 0x2f},
	{0x9e9c, 0x2f},
	{0x9e9d, 0x00},
	{0x9e9e, 0x00},
	{0x9e9f, 0x00},
	{0xa2a9, 0x60},
	{0xa2b7, 0x00},
	{REG_SCALE_MODE, 0x00},
	{REG_SCALE_M_MSbit, 0x00},
	{REG_SCALE_M_LSB, 0x20},
	{REG_DIG_CROP_X_OFFSET_MSB, 0x00},
	{REG_DIG_CROP_X_OFFSET_LSB, 0x00},
	{REG_DIG_CROP_Y_OFFSET_MSB, 0x00},
	{REG_DIG_CROP_Y_OFFSET_LSB, 0x00},
	{REG_DIG_CROP_WIDTH_MSB, 0x0f},
	{REG_DIG_CROP_WIDTH_LSB, 0xd8},
	{REG_DIG_CROP_HEIGHT_MSB, 0x04},
	{REG_DIG_CROP_HEIGHT_LSB, 0x38},
	{REG_X_OUT_SIZE_MSB, 0x07},     //change to 1920
	{REG_X_OUT_SIZE_LSB, 0xEC},
	{REG_Y_OUT_SIZE_MSB, 0x04},
	{REG_Y_OUT_SIZE_LSB, 0x38},
	{REG_IVTPXCK_DIV, 0x05},
	{REG_IVTSYCK_DIV, 0x02},
	{REG_IVT_PREPLLCK_DIV, 0x02},
	{REG_PLL_IVT_MPY_MSB, 0x00},
	{REG_PLL_IVT_MPY_LSB, 0x9B},
	{REG_IOPPXCK_DIV, 0x0c}, //decided by output bit width
	{REG_IOPSYCK_DIV, 0x02},
	{REG_IOP_PREPLLCK_DIV, 0x02},
	{REG_IOP_MPY_MSB, 0x00},
	{REG_IOP_MPY_LSB, 0x85},
	{REG_PLL_MULTI_DRV, 0x01},
	{REG_REQ_LINK_BIT_RATE_MSB, 0x07},
	{REG_REQ_LINK_BIT_RATE_LMSB, 0x08},
	{REG_REQ_LINK_BIT_RATE_MLSB, 0x00},
	{REG_REQ_LINK_BIT_RATE_LSB, 0x00},
	{REG_TCLK_POST_EX_MSB, 0x00},
	{REG_TCLK_POST_EX_LSB, 0x7f},
	{REG_THS_PRE_EX_MSB, 0x00},
	{REG_THS_PRE_EX_LSB, 0x4f},
	{REG_THS_ZERO_MIN_MSB, 0x00},
	{REG_THS_ZERO_MIN_LSB, 0x77},
	{REG_THS_TRAIL_EX_MSB, 0x00},
	{REG_THS_TRAIL_EX_LSB, 0x5f},
	{REG_TCLK_TRAIL_MIN_MSB, 0x00},
	{REG_TCLK_TRAIL_MIN_LSB, 0x57},
	{REG_TCLK_PREP_EX_MSB, 0x00},
	{REG_TCLK_PREP_EX_LSB, 0x4f},
	{REG_TCLK_ZERO_EX_MSB, 0x01},
	{REG_TCLK_ZERO_EX_LSB, 0x27},
	{REG_TLPX_EX_MSB, 0x00},
	{REG_TLPX_EX_LSB, 0x3f},
	{0xe04c, 0x00},
	{0xe04d, 0x7f},
	{0xe04e, 0x00},
	{0xe04f, 0x1f},
	{0x3e20, 0x01},
	{REG_PDAF_CTRL1_0, 0x00},
	{REG_POWER_SAVE_ENABLE, 0x00},
	{REG_LINE_LEN_INCLK_MSB, 0x01},
	{REG_LINE_LEN_INCLK_LSB, 0x6C},
};

/* 2x2 binned.  */
static imx477_reg_t mode_2028x1520_regs[] = {
	{REG_LINE_LEN_MSB, 0x31},
	{REG_LINE_LEN_LSB, 0xc4},
	{REG_X_ADD_STA_MSB, 0x00},
	{REG_X_ADD_STA_LSB, 0x00},
	{REG_Y_ADD_STA_MSB, 0x00},
	{REG_Y_ADD_STA_LSB, 0x00},
	{REG_X_ADD_END_MSB, 0x0f},
	{REG_X_ADD_END_LSB, 0xd7},
	{REG_Y_ADD_END_MSB, 0x0b},
	{REG_Y_ADD_END_LSB, 0xdf},
	{0x0220, 0x00},
	{0x0221, 0x11},
	{REG_X_ENV_INC_CONST, 0x01},
	{REG_X_ODD_INC_CONST, 0x01},
	{REG_Y_ENV_INC_CONST, 0x01},
	{REG_Y_ODD_INC, 0x01},
	{REG_BINNING_MODE, 0x01},
	{REG_BINNING_HV, 0x12},
	{REG_BINNING_WEIGHTING, 0x02},
	{0x3140, 0x02},
	{0x3c00, 0x00},
	{0x3c01, 0x03},
	{0x3c02, 0xa2},
	{REG_ADC_BIT_SETTING, 0x01},
	{0x5748, 0x07},
	{0x5749, 0xff},
	{0x574a, 0x00},
	{0x574b, 0x00},
	{0x7b53, 0x01},
	{0x9369, 0x73},
	{0x936b, 0x64},
	{0x936d, 0x5f},
	{0x9304, 0x00},
	{0x9305, 0x00},
	{0x9e9a, 0x2f},
	{0x9e9b, 0x2f},
	{0x9e9c, 0x2f},
	{0x9e9d, 0x00},
	{0x9e9e, 0x00},
	{0x9e9f, 0x00},
	{0xa2a9, 0x60},
	{0xa2b7, 0x00},
	{REG_SCALE_MODE, 0x01},
	{REG_SCALE_M_MSbit, 0x00},
	{REG_SCALE_M_LSB, 0x20},
	{REG_DIG_CROP_X_OFFSET_MSB, 0x00},
	{REG_DIG_CROP_X_OFFSET_LSB, 0x00},
	{REG_DIG_CROP_Y_OFFSET_MSB, 0x00},
	{REG_DIG_CROP_Y_OFFSET_LSB, 0x00},
	{REG_DIG_CROP_WIDTH_MSB, 0x0f},
	{REG_DIG_CROP_WIDTH_LSB, 0xd8},
	{REG_DIG_CROP_HEIGHT_MSB, 0x0b},
	{REG_DIG_CROP_HEIGHT_LSB, 0xE0},
	{REG_X_OUT_SIZE_MSB, 0x07},
	{REG_X_OUT_SIZE_LSB, 0xec},
	{REG_Y_OUT_SIZE_MSB, 0x05},
	{REG_Y_OUT_SIZE_LSB, 0xF0},
	{REG_IVTPXCK_DIV, 0x05},
	{REG_IVTSYCK_DIV, 0x02},
	{REG_IVT_PREPLLCK_DIV, 0x02},
	{REG_PLL_IVT_MPY_MSB, 0x00},
	{REG_PLL_IVT_MPY_LSB, 0x9B},
	{REG_IOPPXCK_DIV, 0x0c}, //decided by output bit width
	{REG_IOPSYCK_DIV, 0x02},
	{REG_IOP_PREPLLCK_DIV, 0x02},
	{REG_IOP_MPY_MSB, 0x00},
	{REG_IOP_MPY_LSB, 0x85},
	{REG_PLL_MULTI_DRV, 0x01},
	{REG_REQ_LINK_BIT_RATE_MSB, 0x07},
	{REG_REQ_LINK_BIT_RATE_LMSB, 0x08},
	{REG_REQ_LINK_BIT_RATE_MLSB, 0x00},
	{REG_REQ_LINK_BIT_RATE_LSB, 0x00},
	{REG_TCLK_POST_EX_MSB, 0x00},
	{REG_TCLK_POST_EX_LSB, 0x7f},
	{REG_THS_PRE_EX_MSB, 0x00},
	{REG_THS_PRE_EX_LSB, 0x4f},
	{REG_THS_ZERO_MIN_MSB, 0x00},
	{REG_THS_ZERO_MIN_LSB, 0x77},
	{REG_THS_TRAIL_EX_MSB, 0x00},
	{REG_THS_TRAIL_EX_LSB, 0x5f},
	{REG_TCLK_TRAIL_MIN_MSB, 0x00},
	{REG_TCLK_TRAIL_MIN_LSB, 0x57},
	{REG_TCLK_PREP_EX_MSB, 0x00},
	{REG_TCLK_PREP_EX_LSB, 0x4f},
	{REG_TCLK_ZERO_EX_MSB, 0x01},
	{REG_TCLK_ZERO_EX_LSB, 0x27},
	{REG_TLPX_EX_MSB, 0x00},
	{REG_TLPX_EX_LSB, 0x3f},
	{0xe04c, 0x00},
	{0xe04d, 0x7f},
	{0xe04e, 0x00},
	{0xe04f, 0x1f},
	{0x3e20, 0x01},
	{REG_PDAF_CTRL1_0, 0x00},
	{REG_POWER_SAVE_ENABLE, 0x00},
	{REG_LINE_LEN_INCLK_MSB, 0x01},
	{REG_LINE_LEN_INCLK_LSB, 0x6c},
};


//4x4 binning 10bit
static imx477_reg_t mode_1332x990_regs[] = {
		{0x420b, 0x01},
		{0x990c, 0x00},
		{0x990d, 0x08},
		{0x9956, 0x8c},
		{0x9957, 0x64},
		{0x9958, 0x50},
		{0x9a48, 0x06},
		{0x9a49, 0x06},
		{0x9a4a, 0x06},
		{0x9a4b, 0x06},
		{0x9a4c, 0x06},
		{0x9a4d, 0x06},
		{REG_CSI_FORMAT_C, 0x0a},
		{REG_CSI_FORMAT_D, 0x0a},
		{REG_CSI_LANE, 0x01},
		{REG_LINE_LEN_MSB, 0x1a},
		{REG_LINE_LEN_LSB, 0x08},
		{REG_FRAME_LEN_MSB, 0x04},
		{REG_FRAME_LEN_LSB, 0x1a},
		{REG_X_ADD_STA_MSB, 0x00},
		{REG_X_ADD_STA_LSB, 0x00},
		{REG_Y_ADD_STA_MSB, 0x02},
		{REG_Y_ADD_STA_LSB, 0x10},
		{REG_X_ADD_END_MSB, 0x0f},
		{REG_X_ADD_END_LSB, 0xd7},
		{REG_Y_ADD_END_MSB, 0x09},
		{REG_Y_ADD_END_LSB, 0xcf},
		{REG_DOL_HDR_EN, 0x00},
		{REG_DOL_HDR_NUM, 0x00},
		{REG_DOL_CSI_DT_FMT_H_2ND, 0x0a},
		{REG_DOL_CSI_DT_FMT_L_2ND, 0x0a},
		{REG_DOL_CSI_DT_FMT_H_3ND, 0x0a},
		{REG_DOL_CSI_DT_FMT_L_3ND, 0x0a},
		{REG_DOL_CONST, 0x00},
		{0x0220, 0x00},
		{0x0221, 0x11},
		{REG_X_ENV_INC_CONST, 0x01},
		{REG_X_ODD_INC_CONST, 0x01},
		{REG_Y_ENV_INC_CONST, 0x01},
		{REG_Y_ODD_INC, 0x01},
		{REG_BINNING_MODE, 0x01},
		{REG_BINNING_HV, 0x22},
		{REG_BINNING_WEIGHTING, 0x02},
		{0x3140, 0x02},
		{0x3c00, 0x00},
		{0x3c01, 0x01},
		{0x3c02, 0x9c},
		{REG_ADC_BIT_SETTING, 0x00},
		{0x5748, 0x00},
		{0x5749, 0x00},
		{0x574a, 0x00},
		{0x574b, 0xa4},
		{0x7b75, 0x0e},
		{0x7b76, 0x09},
		{0x7b77, 0x08},
		{0x7b78, 0x06},
		{0x7b79, 0x34},
		{0x7b53, 0x00},
		{0x9369, 0x73},
		{0x936b, 0x64},
		{0x936d, 0x5f},
		{0x9304, 0x03},
		{0x9305, 0x80},
		{0x9e9a, 0x2f},
		{0x9e9b, 0x2f},
		{0x9e9c, 0x2f},
		{0x9e9d, 0x00},
		{0x9e9e, 0x00},
		{0x9e9f, 0x00},
		{0xa2a9, 0x27},
		{0xa2b7, 0x03},
		{REG_SCALE_MODE, 0x00},
		{REG_SCALE_M_MSbit, 0x00},
		{REG_SCALE_M_LSB, 0x10},
		{REG_DIG_CROP_X_OFFSET_MSB, 0x01},
		{REG_DIG_CROP_X_OFFSET_LSB, 0x5c},
		{REG_DIG_CROP_Y_OFFSET_MSB, 0x00},
		{REG_DIG_CROP_Y_OFFSET_LSB, 0x00},
		{REG_DIG_CROP_WIDTH_MSB, 0x05},
		{REG_DIG_CROP_WIDTH_LSB, 0x34},
		{REG_DIG_CROP_HEIGHT_MSB, 0x03},
		{REG_DIG_CROP_HEIGHT_LSB, 0xde},
		{REG_X_OUT_SIZE_MSB, 0x05},
		{REG_X_OUT_SIZE_LSB, 0x34},
		{REG_Y_OUT_SIZE_MSB, 0x03},
		{REG_Y_OUT_SIZE_LSB, 0xde},
		{REG_IVTPXCK_DIV, 0x05},
		{REG_IVTSYCK_DIV, 0x02},
		{REG_IVT_PREPLLCK_DIV, 0x02},
		{REG_PLL_IVT_MPY_MSB, 0x00},
		{REG_PLL_IVT_MPY_LSB, 0x9B},
		{REG_IOPPXCK_DIV, 0x0a},
		{REG_IOPSYCK_DIV, 0x02},
		{REG_IOP_PREPLLCK_DIV, 0x02},
		{REG_IOP_MPY_MSB, 0x00},
		{REG_IOP_MPY_LSB, 0x85},
		{REG_PLL_MULTI_DRV, 0x01},
		{REG_REQ_LINK_BIT_RATE_MSB, 0x07},      //does not seems to be actually affecting link rate
		{REG_REQ_LINK_BIT_RATE_LMSB, 0x08},
		{REG_REQ_LINK_BIT_RATE_MLSB, 0x00},
		{REG_REQ_LINK_BIT_RATE_LSB, 0x00},
		{REG_TCLK_POST_EX_MSB, 0x00},
		{REG_TCLK_POST_EX_LSB, 0x7f},
		{REG_THS_PRE_EX_MSB, 0x00},
		{REG_THS_PRE_EX_LSB, 0x4f},
		{REG_THS_ZERO_MIN_MSB, 0x00},
		{REG_THS_ZERO_MIN_LSB, 0x77},
		{REG_THS_TRAIL_EX_MSB, 0x00},
		{REG_THS_TRAIL_EX_LSB, 0x5f},
		{REG_TCLK_TRAIL_MIN_MSB, 0x00},
		{REG_TCLK_TRAIL_MIN_LSB, 0x57},
		{REG_TCLK_PREP_EX_MSB, 0x00},
		{REG_TCLK_PREP_EX_LSB, 0x4f},
		{REG_TCLK_ZERO_EX_MSB, 0x01},
		{REG_TCLK_ZERO_EX_LSB, 0x27},
		{REG_TLPX_EX_MSB, 0x00},
		{REG_TLPX_EX_LSB, 0x3f},
		{0xe04c, 0x00},
		{0xe04d, 0x5f},
		{0xe04e, 0x00},
		{0xe04f, 0x1f},
		{0x3e20, 0x01},
		{REG_PDAF_CTRL1_0, 0x00},
		{REG_POWER_SAVE_ENABLE, 0x00},
		{REG_LINE_LEN_INCLK_MSB, 0x00},
		{REG_LINE_LEN_INCLK_LSB, 0xbf},
};


//4x4 binning 10bit
static imx477_reg_t mode_640x480_regs[] = {
		{0x420b, 0x01},
		{0x990c, 0x00},
		{0x990d, 0x08},
		{0x9956, 0x8c},
		{0x9957, 0x64},
		{0x9958, 0x50},
		{0x9a48, 0x06},
		{0x9a49, 0x06},
		{0x9a4a, 0x06},
		{0x9a4b, 0x06},
		{0x9a4c, 0x06},
		{0x9a4d, 0x06},
		{REG_CSI_FORMAT_C, 0x0a},
		{REG_CSI_FORMAT_D, 0x0a},
		{REG_CSI_LANE, 0x01},
		{REG_LINE_LEN_MSB, 0x1a},
		{REG_LINE_LEN_LSB, 0x08},
		{REG_FRAME_LEN_MSB, 0x04},
		{REG_FRAME_LEN_LSB, 0x1a},
		{REG_X_ADD_STA_MSB, 0x00},
		{REG_X_ADD_STA_LSB, 0x00},
		{REG_Y_ADD_STA_MSB, 0x02},
		{REG_Y_ADD_STA_LSB, 0x10},
		{REG_X_ADD_END_MSB, 0x0f},
		{REG_X_ADD_END_LSB, 0xd7},
		{REG_Y_ADD_END_MSB, 0x09},
		{REG_Y_ADD_END_LSB, 0xcf},
		{REG_DOL_HDR_EN, 0x00},
		{REG_DOL_HDR_NUM, 0x00},
		{REG_DOL_CSI_DT_FMT_H_2ND, 0x0a},
		{REG_DOL_CSI_DT_FMT_L_2ND, 0x0a},
		{REG_DOL_CSI_DT_FMT_H_3ND, 0x0a},
		{REG_DOL_CSI_DT_FMT_L_3ND, 0x0a},
		{REG_DOL_CONST, 0x00},
		{0x0220, 0x00},
		{0x0221, 0x11},
		{REG_X_ENV_INC_CONST, 0x01},
		{REG_X_ODD_INC_CONST, 0x01},
		{REG_Y_ENV_INC_CONST, 0x01},
		{REG_Y_ODD_INC, 0x01},
		{REG_BINNING_MODE, 0x01},
		{REG_BINNING_HV, 0x22},
		{REG_BINNING_WEIGHTING, 0x02},
		{0x3140, 0x02},
		{0x3c00, 0x00},
		{0x3c01, 0x01},
		{0x3c02, 0x9c},
		{REG_ADC_BIT_SETTING, 0x00},
		{0x5748, 0x00},
		{0x5749, 0x00},
		{0x574a, 0x00},
		{0x574b, 0xa4},
		{0x7b75, 0x0e},
		{0x7b76, 0x09},
		{0x7b77, 0x08},
		{0x7b78, 0x06},
		{0x7b79, 0x34},
		{0x7b53, 0x00},
		{0x9369, 0x73},
		{0x936b, 0x64},
		{0x936d, 0x5f},
		{0x9304, 0x03},
		{0x9305, 0x80},
		{0x9e9a, 0x2f},
		{0x9e9b, 0x2f},
		{0x9e9c, 0x2f},
		{0x9e9d, 0x00},
		{0x9e9e, 0x00},
		{0x9e9f, 0x00},
		{0xa2a9, 0x27},
		{0xa2b7, 0x03},
		{REG_SCALE_MODE, 0x00},
		{REG_SCALE_M_MSbit, 0x00},
		{REG_SCALE_M_LSB, 0x10},
		{REG_DIG_CROP_X_OFFSET_MSB, 0x02},
		{REG_DIG_CROP_X_OFFSET_LSB, 0x5c},
		{REG_DIG_CROP_Y_OFFSET_MSB, 0x00},
		{REG_DIG_CROP_Y_OFFSET_LSB, 0x00},
		{REG_DIG_CROP_WIDTH_MSB, 0x02},
		{REG_DIG_CROP_WIDTH_LSB, 0x80},
		{REG_DIG_CROP_HEIGHT_MSB, 0x01},
		{REG_DIG_CROP_HEIGHT_LSB, 0xE0},
		{REG_X_OUT_SIZE_MSB, 0x02},
		{REG_X_OUT_SIZE_LSB, 0x80},
		{REG_Y_OUT_SIZE_MSB, 0x01},
		{REG_Y_OUT_SIZE_LSB, 0xE0},
		{REG_IVTPXCK_DIV, 0x05},
		{REG_IVTSYCK_DIV, 0x02},
		{REG_IVT_PREPLLCK_DIV, 0x02},
		{REG_PLL_IVT_MPY_MSB, 0x00},
		{REG_PLL_IVT_MPY_LSB, 0x9B},
		{REG_IOPPXCK_DIV, 0x0a},
		{REG_IOPSYCK_DIV, 0x02},
		{REG_IOP_PREPLLCK_DIV, 0x02},
		{REG_IOP_MPY_MSB, 0x00},
		{REG_IOP_MPY_LSB, 0x85},
		{REG_PLL_MULTI_DRV, 0x01},
		{REG_REQ_LINK_BIT_RATE_MSB, 0x07},      //does not seems to be actually affecting link rate
		{REG_REQ_LINK_BIT_RATE_LMSB, 0x08},
		{REG_REQ_LINK_BIT_RATE_MLSB, 0x00},
		{REG_REQ_LINK_BIT_RATE_LSB, 0x00},
		{REG_TCLK_POST_EX_MSB, 0x00},
		{REG_TCLK_POST_EX_LSB, 0x7f},
		{REG_THS_PRE_EX_MSB, 0x00},
		{REG_THS_PRE_EX_LSB, 0x4f},
		{REG_THS_ZERO_MIN_MSB, 0x00},
		{REG_THS_ZERO_MIN_LSB, 0x77},
		{REG_THS_TRAIL_EX_MSB, 0x00},
		{REG_THS_TRAIL_EX_LSB, 0x5f},
		{REG_TCLK_TRAIL_MIN_MSB, 0x00},
		{REG_TCLK_TRAIL_MIN_LSB, 0x57},
		{REG_TCLK_PREP_EX_MSB, 0x00},
		{REG_TCLK_PREP_EX_LSB, 0x4f},
		{REG_TCLK_ZERO_EX_MSB, 0x01},
		{REG_TCLK_ZERO_EX_LSB, 0x27},
		{REG_TLPX_EX_MSB, 0x00},
		{REG_TLPX_EX_LSB, 0x3f},
		{0xe04c, 0x00},
		{0xe04d, 0x5f},
		{0xe04e, 0x00},
		{0xe04f, 0x1f},
		{0x3e20, 0x01},
		{REG_PDAF_CTRL1_0, 0x00},
		{REG_POWER_SAVE_ENABLE, 0x00},
		{REG_LINE_LEN_INCLK_MSB, 0x00},
		{REG_LINE_LEN_INCLK_LSB, 0xbf},
};




static imgsensor_mode_t sensor_config_2Lane[] = {
		{
			.sensor_mode = 1,
			.integration_def = 500,
			.integration = 500,
			.integration_max = 557-22, // From RPI IMX477 driver exp offset is 22
			.integration_min = 7,
			.width = 640,
			.height = 480,
			.frame_length= 557, //decided frame rate along with mode regs
			.fps = 200,
			.gain = 200,
			.gain_max = 978,
			.test_pattern =0,
			.bits = 10,
			.reg_list  = {
					.num_of_regs = _countof(mode_640x480_regs),
					.regs = mode_640x480_regs,
			}
		},
		{
				.sensor_mode = 1,
				.integration_def = 500,
				.integration = 500,
				.integration_max = 1115 -22,
				.integration_min = 7,
				.width = 1332,
				.height = 990,
				.frame_length= 1115, //decided frame rate along with mode regs
				.fps = 100,
				.gain = 200,
				.gain_max = 978,
				.test_pattern =0,
				.bits = 10,
				.reg_list  = {
						.num_of_regs = _countof(mode_1332x990_regs),
						.regs = mode_1332x990_regs,
				}
		},
		{
				.sensor_mode = 1,
				.integration_def = 500,
				.integration = 500,
				.integration_max = 1167-22,
				.integration_min = 7,
				.width = 1920,
				.height = 1080,
				.frame_length= 1167, //decided frame rate along with mode regs
				.fps = 60,
				.gain = 200,
				.gain_max = 978,
				.bits = 12,
				.test_pattern =0,
				.reg_list  = {
						.num_of_regs = _countof(mode_2028x1080_regs),
						.regs = mode_2028x1080_regs,
				}
		},
		{
				.sensor_mode = 1,
				.integration_def = 500,
				.integration = 500,
				.integration_max = 1666-22,
				.integration_min = 7,
				.width = 2028,
				.height = 1520,
				.frame_length= 1666, //decided frame rate along with mode regs
				.fps = 35,
				.gain = 200,
				.gain_max = 978,
				.bits = 12,
				.test_pattern =0,
				.reg_list  = {
						.num_of_regs = _countof(mode_2028x1520_regs),
						.regs = mode_2028x1520_regs,
				}
		},
		{
				.sensor_mode = 1,
				.integration_def = 500,
				.integration = 500,
				.integration_max = 9312-22,
				.integration_min = 7,
				.width = 4056,
				.height = 3040,
				.frame_length= 9312, //decided frame rate along with mode regs
				.fps = 5,
				.gain = 200,
				.gain_max = 978,
				.bits = 12,
				.test_pattern =0,
				.reg_list  = {
						.num_of_regs = _countof(mode_4056x3040_regs),
						.regs = mode_4056x3040_regs,
				}
		},

		{
				.sensor_mode = 1,
				.integration_def = 500,
				.integration = 500,
				.integration_max = 3104-22,
				.integration_min = 7,
				.width = 4056,
				.height = 3040,
				.frame_length= 3104, //decided frame rate along with mode regs
				.fps = 10,
				.gain = 200,
				.gain_max = 978,
				.bits = 12,
				.test_pattern =0,
				.reg_list  = {
						.num_of_regs = _countof(mode_4056x3040_regs),
						.regs = mode_4056x3040_regs,
				}
		},


};


static void SensorI2CAccessDelay (CyU3PReturnStatus_t status)
{
    /* Add a 10us delay if the I2C operation that preceded this call was successful. */
    if (status == CY_U3P_SUCCESS)
        CyU3PBusyWait (50);
}

CyU3PReturnStatus_t sensor_i2c_write(uint16_t reg_addr, uint8_t data)
{
	CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
	CyU3PI2cPreamble_t  preamble;
	uint8_t buf[2];

	/* Set the parameters for the I2C API access and then call the write API. */
	preamble.buffer[0] = SENSOR_ADDR_WR;
	preamble.buffer[1] = (reg_addr >> 8) & 0xFF;
	preamble.buffer[2] = (reg_addr) & 0xFF;
	preamble.length    = 3;             /*  Three byte preamble. */
	preamble.ctrlMask  = 0x0000;        /*  No additional start and stop bits. */
	buf[0] = data;
	apiRetStatus = CyU3PI2cTransmitBytes (&preamble, buf, 1, 0);
	SensorI2CAccessDelay (apiRetStatus);

	return apiRetStatus;
}

CyU3PReturnStatus_t sensor_i2c_read(uint16_t reg_addr , uint8_t *buffer)
{
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
    CyU3PI2cPreamble_t preamble;

	preamble.length    = 4;
    preamble.buffer[0] = SENSOR_ADDR_RD & I2C_SLAVEADDR_MASK;        /*  Mask out the transfer type bit. */
    preamble.buffer[1] = (reg_addr >> 8) & 0xFF;
    preamble.buffer[2] = reg_addr & 0xFF;
    preamble.buffer[3] = SENSOR_ADDR_RD ;
    preamble.ctrlMask  = 1<<2;                                /*  Send start bit after third byte of preamble. */

    apiRetStatus = CyU3PI2cReceiveBytes (&preamble, buffer, 1, 0);

    SensorI2CAccessDelay (apiRetStatus);

    return apiRetStatus;
}



static CyU3PReturnStatus_t camera_stream_on (uint8_t on)
{
   return sensor_i2c_write(REG_MODE_SEL , on);
}


void SensorReset (void)
{
    sensor_i2c_write(REG_SW_RESET , 0x01);
    /* Wait for some time to allow proper reset. */
    CyU3PThreadSleep (10);
    /* Delay the allow the sensor to power up. */
    sensor_i2c_write(REG_SW_RESET , 0x00);
    CyU3PThreadSleep (10);
    return;
}

static void set_mirror_flip(uint8_t image_mirror)
{

	uint8_t  iTemp = 0x03;

	image_mirror = IMAGE_NORMAL;
    //sensor_i2c_read(REG_IMG_ORIENT ,  iTemp);
    //iTemp = iTemp & 0x03;
    switch (image_mirror)
    {
        case IMAGE_NORMAL:
            sensor_i2c_write(REG_IMG_ORIENT, iTemp | 0x03);	//Set normal
            break;
        case IMAGE_V_MIRROR:
            sensor_i2c_write(REG_IMG_ORIENT, iTemp | 0x01);	//Set flip
            break;
        case IMAGE_H_MIRROR:
            sensor_i2c_write(REG_IMG_ORIENT, iTemp | 0x02);	//Set mirror
            break;
        case IMAGE_HV_MIRROR:
            sensor_i2c_write(REG_IMG_ORIENT, iTemp);	//Set mirror and flip
            break;
    }
}

void sensor_handle_uvc_control(uint8_t frame_index, uint32_t interval)
{
	switch(frame_index)
	{
		case FRAME_MODE0:
		{
			if (interval == INTERVAL_MODE0)
			{
				selected_img_mode = &sensor_config[0];
			}
		}
		break;
		case FRAME_MODE1:
		{
			if (interval == INTERVAL_MODE1)
			{
				selected_img_mode = &sensor_config[1];
			}
		}
		break;
		case FRAME_MODE2:
		{
			if (interval == INTERVAL_MODE2)
			{
				selected_img_mode = &sensor_config[2];
			}
		}
		break;
		case FRAME_MODE3:
		{
			if (interval == INTERVAL_MODE3)
			{
				selected_img_mode = &sensor_config[3];
			}
		}
		break;
		case FRAME_MODE4:
		{
			if (interval == INTERVAL_MODE4_MIN)
			{
				selected_img_mode = &sensor_config[4];
			}
			else if (interval == INTERVAL_MODE4)
			{
				selected_img_mode = &sensor_config[5];
			}

		}
		break;
		default:
		{

		}
	}

	sensor_configure_mode (selected_img_mode);
}
void sensor_configure_mode(imgsensor_mode_t * mode)
{

	camera_stream_on(false);

	set_mirror_flip(mode->mirror);

	for (uint16_t i = 0; i < mode->reg_list.num_of_regs; i++)
	{
		//CyU3PDebugPrint (4, "Reg 0x%x val 0x%x\n", (mode_default + i)->address, (mode_default + i)->val);
		sensor_i2c_write(((mode->reg_list.regs) + i)->address, ((mode->reg_list.regs) + i)->val);

	}

	sensor_i2c_write(REG_TEST_PATTERN_LSB, (mode->test_pattern < 8)? mode->test_pattern : 0);
	sensor_i2c_write(REG_MAP_COUPLET_CORR, 0x01);
	sensor_i2c_write(REG_SING_DYNAMIC_CORR, 0x01);
	sensor_i2c_write(REG_CIT_LSHIFT_LONG_EXP, 0x00);
	sensor_i2c_write(REG_FRAME_LEN_MSB, (mode->frame_length & 0xFF00)>>8);
	sensor_i2c_write(REG_FRAME_LEN_LSB,  mode->frame_length & 0xFF);

	sensor_i2c_write(REG_TP_RED_MSB, 0x0);
	sensor_i2c_write(REG_TP_RED_LSB, 0x0);
	sensor_i2c_write(REG_TP_GREENR_MSB, 0x00);
	sensor_i2c_write(REG_TP_GREENR_LSB, 0x00);
	sensor_i2c_write(REG_TP_GREENB_MSB, 0x00);
	sensor_i2c_write(REG_TP_GREENB_LSB, 0x0);
	sensor_i2c_write(REG_TP_BLUE_MSB, 0x07);
	sensor_i2c_write(REG_TP_BLUE_LSB, 0xFF);
	sensor_i2c_write(REG_COARSE_INTEGRATION_TIME_MSB, (mode->integration_def >> 8) & 0xFF);
	sensor_i2c_write(REG_COARSE_INTEGRATION_TIME_LSB, mode->integration_def & 0xFF);

	camera_stream_on(selected_img_mode->sensor_mode);
}

uint8_t	SensorI2cBusTest (void)
{
	uint8_t model_lsb;
	uint8_t model_msb;
	sensor_i2c_read (REG_MODEL_ID_MSB, &model_msb);
	sensor_i2c_read (REG_MODEL_ID_LSB, &model_lsb);

	if (((((uint16_t)model_msb & 0x0F) << 8) | model_lsb) == CAMERA_ID )
	{
		CyU3PDebugPrint(4,"I2C Sensor id: 0x%x\n", (((uint16_t)model_msb & 0x0F) << 8) | model_lsb);
		return CY_U3P_SUCCESS;
	}

	return CY_U3P_ERROR_DMA_FAILURE;
}

void SensorInit (void)
{

    if (SensorI2cBusTest() != CY_U3P_SUCCESS)        /* Verify that the sensor is connected. */
    {
        CyU3PDebugPrint (4, "Error: Reading Sensor ID failed!\r\n");
        return;
    }

	for (uint16_t i = 0; i < _countof(mode_default); i++)
	{
		//CyU3PDebugPrint (4, "Reg 0x%x val 0x%x\n", (mode_default + i)->address, (mode_default + i)->val);
		sensor_i2c_write((mode_default + i)->address, (mode_default + i)->val);
	}
	sensor_config = sensor_config_2Lane;
	selected_img_mode = &sensor_config[4];
	sensor_configure_mode(selected_img_mode);
}

uint8_t SensorGetBrightness (void)
{
    return selected_img_mode->gain;
}

uint16_t getMaxBrightness(void)
{
	return selected_img_mode->gain_max;
}

void SensorSetBrightness (uint16_t input)
{
	selected_img_mode->gain = input;
	sensor_i2c_write (REG_ANA_GAIN_GLOBAL_MSB, (input >> 8) & 0xFF);
	sensor_i2c_write (REG_ANA_GAIN_GLOBAL_LSB, input & 0xFF);

}

uint16_t sensor_get_min_exposure (void)
{
	return selected_img_mode->integration_min;
}


uint16_t sensor_get_max_exposure (void)
{
	return selected_img_mode->integration_max;
}

uint16_t sensor_get_def_exposure (void)
{
	return selected_img_mode->integration_def;
}

uint16_t sensor_get_exposure (void)
{
	return selected_img_mode->integration;
}

void sensor_set_exposure (uint16_t integration)
{
	if (integration > selected_img_mode->integration_max)
	{
		integration = selected_img_mode->integration_max;
	}
	selected_img_mode->integration = integration;
	sensor_i2c_write (REG_COARSE_INTEGRATION_TIME_MSB, (integration >> 8) & 0xFF);
	sensor_i2c_write (REG_COARSE_INTEGRATION_TIME_LSB, integration & 0xFF);
}


uint8_t sensor_get_test_pattern (void)
{
	return selected_img_mode->test_pattern;
}

void sensor_set_test_pattern (uint8_t test_pattern)
{
	if (test_pattern > 8)
	{
		test_pattern = 0;
	}
	selected_img_mode->test_pattern = test_pattern;
	sensor_i2c_write (REG_TEST_PATTERN_LSB, test_pattern);
}



