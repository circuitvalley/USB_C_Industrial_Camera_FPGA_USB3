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
#include "sensor_imx290.h"
#include "uvc_settings.h"

imgsensor_mode_t *sensor_config;
imgsensor_mode_t *selected_img_mode;

static imx290_reg_t mode_default[] = {
	{ REG_WINMODE, 0x00 },
	{ REG_VMAX_LSB, 0x00 },
	{ REG_WINWV_OB, 0x0c },
	{ REG_WINPH_MSB, 0x00 },
	{ REG_WINPH_LSB, 0x00 },
	{ REG_WINPV_MSB, 0x00 },
	{ REG_WINPV_LSB, 0x00 },
	{ REG_WINWH_MSB, 0x9c },
	{ REG_WINWH_LSB, 0x07 },
	{ REG_WINWV_MSB, 0x49 },
	{ REG_WINWV_LSB, 0x04 },
	{ REG_XVSHSOUTSEL, 0x0a },
	{ 0x300f, 0x00 }, //magic
	{ 0x3010, 0x21 },
	{ 0x3012, 0x64 },
	{ 0x3016, 0x09 },
	{ 0x3070, 0x02 },
	{ 0x3071, 0x11 },
	{ 0x309b, 0x10 },
	{ 0x309c, 0x22 },
	{ 0x30a2, 0x02 },
	{ 0x30a6, 0x20 },
	{ 0x30a8, 0x20 },
	{ 0x30aa, 0x20 },
	{ 0x30ac, 0x20 },
	{ 0x30b0, 0x43 },
	{ 0x3119, 0x9e },
	{ 0x311c, 0x1e },
	{ 0x311e, 0x08 },
	{ 0x3128, 0x05 },
	{ 0x313d, 0x83 },
	{ 0x3150, 0x03 },
	{ 0x317e, 0x00 },
	{ 0x32b8, 0x50 },
	{ 0x32b9, 0x10 },
	{ 0x32ba, 0x00 },
	{ 0x32bb, 0x04 },
	{ 0x32c8, 0x50 },
	{ 0x32c9, 0x10 },
	{ 0x32ca, 0x00 },
	{ 0x32cb, 0x04 },
	{ 0x332c, 0xd3 },
	{ 0x332d, 0x10 },
	{ 0x332e, 0x0d },
	{ 0x3358, 0x06 },
	{ 0x3359, 0xe1 },
	{ 0x335a, 0x11 },
	{ 0x3360, 0x1e },
	{ 0x3361, 0x61 },
	{ 0x3362, 0x10 },
	{ 0x33b0, 0x50 },
	{ 0x33b2, 0x1a },
	{ 0x33b3, 0x04 },
};

static imx290_reg_t imx290_37_125mhz_clock_1080p[] = {
	{REG_INCKSEL1, 0x18 },
	{REG_INCKSEL2, 0x03 },
	{REG_INCKSEL3, 0x20 },
	{REG_INCKSEL4, 0x01 },
	{REG_INCKSEL5, 0x1a },
	{REG_INCKSEL6, 0x1a },
	{REG_EXTCK_FREQ_MSB, 0x20 },
	{REG_EXTCK_FREQ_LSB, 0x25 },
	{0x3480, 0x49 }, //magic
};

static imx290_reg_t imx290_1080p_common_settings[] = {
	/* mode settings */
	{ REG_FR_FDG_SEL, 0x01 },
	{ REG_WINMODE, 0x00 },
	{ REG_WINWV_OB, 0x0c },
	{ REG_OPB_SIZE_V , 0x0a },
	{ REG_X_OUT_SIZE_MSB , 0x80 },
	{ REG_X_OUT_SIZE_LSB , 0x07 },
	{ REG_Y_OUT_SIZE_MSB , 0x38 },
	{ REG_Y_OUT_SIZE_LSB , 0x04 },
	{ 0x3012, 0x64 },	//magic
	{ 0x3013, 0x00 },
};

static imx290_reg_t imx290_1080p_4lane_settings[] = {
	{ 0x3405, 0x10 },
	/* data rate settings */
	{ REG_PHY_LANE_NUM, 0x03 },
	{ REG_CSI_LANE_MODE, 0x03 },
	{ REG_TCLKPOST_MSB, 0x57 },
	{ REG_TCLKPOST_LSB, 0x00 },
	{ REG_THSZERO_MSB, 0x37 },
	{ REG_THSZERO_LSB, 0x00 },
	{ REG_THSPREPARE_MSB, 0x1f },
	{ REG_THSPREPARE_LSB, 0x00 },
	{ REG_TCLKTRAIL_MSB, 0x1f },
	{ REG_TCLKTRAIL_LSB, 0x00 },
	{ REG_THSTRAIL_MSB, 0x1f },
	{ REG_THSTRAIL_LSB, 0x00 },
	{ REG_TCLKZERO_MSB, 0x77 },
	{ REG_TCLKZERO_LSB, 0x00 },
	{ REG_TCLKPREPARE_MSB, 0x1f },
	{ REG_TCLKPREPARE_LSB, 0x00 },
	{ REG_TLPX_MSB, 0x17 },
	{ REG_TLPX_LSB, 0x00 },
};


static imx290_reg_t imx290_37_125mhz_clock_720p[] = {
	{ REG_INCKSEL1, 0x20 },
	{ REG_INCKSEL2, 0x00 },
	{ REG_INCKSEL3, 0x20 },
	{ REG_INCKSEL4, 0x01 },
	{ REG_INCKSEL5, 0x1a },
	{ REG_INCKSEL6, 0x1a },
	{ REG_EXTCK_FREQ_MSB, 0x20 },
	{ REG_EXTCK_FREQ_LSB, 0x25 },
	{ 0x3480, 0x49 },
};


static imx290_reg_t imx290_720p_common_settings[] = {
	/* mode settings */
	{ REG_FR_FDG_SEL, 0x01 },
	{ REG_WINMODE, 0x10 },
	{ REG_WINWV_OB, 0x06 },
	{ REG_OPB_SIZE_V, 0x04 },
	{ REG_X_OUT_SIZE_MSB, 0x00 },
	{ REG_X_OUT_SIZE_LSB, 0x05 },
	{ REG_Y_OUT_SIZE_MSB, 0xd0 },
	{ REG_Y_OUT_SIZE_LSB, 0x02 },
	{ 0x3012, 0x64 },
	{ 0x3013, 0x00 },
};


static imx290_reg_t imx290_720p_4lane_settings[] = {
	{ REG_REPETITION, 0x10 },
	{ REG_PHY_LANE_NUM, 0x03 },
	{ REG_CSI_LANE_MODE, 0x03 },
	/* data rate settings */
	{ REG_TCLKPOST_MSB, 0x4f },
	{ REG_TCLKPOST_LSB, 0x00 },
	{ REG_THSZERO_MSB, 0x2f },
	{ REG_THSZERO_LSB, 0x00 },
	{ REG_THSPREPARE_MSB, 0x17 },
	{ REG_THSPREPARE_LSB, 0x00 },
	{ REG_TCLKTRAIL_MSB, 0x17 },
	{ REG_TCLKTRAIL_LSB, 0x00 },
	{ REG_THSTRAIL_MSB, 0x17 },
	{ REG_THSTRAIL_LSB, 0x00 },
	{ REG_TCLKZERO_MSB, 0x57 },
	{ REG_TCLKZERO_LSB, 0x00 },
	{ REG_TCLKPREPARE_MSB, 0x17 },
	{ REG_TCLKPREPARE_MSB, 0x00 },
	{ REG_TLPX_MSB, 0x17 },
	{ REG_TLPX_LSB, 0x00 },
};

static imx290_reg_t imx290_10bit_settings[] = {
	{REG_ADBIT, 0x00},
	{REG_PORTBIT_SEL, 0x00},
	{REG_ADBIT1, 0x1d},
	{REG_ADBIT2, 0x12},
	{REG_ADBIT3, 0x37},
	{REG_CSI_DT_FMT_MSB, 0x0a},
	{REG_CSI_DT_FMT_LSB, 0x0a},
	{REG_BLACK_LEVEL_MSB, 0x3c},
	{REG_BLACK_LEVEL_LSB, 0x00},
};

static imx290_reg_t imx290_12bit_settings[] = {
	{ REG_ADBIT, 0x01 },
	{ REG_PORTBIT_SEL, 0x01 },
	{ REG_ADBIT1, 0x00 },
	{ REG_ADBIT2, 0x00 },
	{ REG_ADBIT3, 0x0e },
	{ REG_CSI_DT_FMT_MSB, 0x0c },
	{ REG_CSI_DT_FMT_LSB, 0x0c },
	{ REG_BLACK_LEVEL_MSB, 0xf0 },
	{ REG_BLACK_LEVEL_LSB, 0x00 },
};



static imgsensor_mode_t sensor_config_4lane[] = {
		{
			.sensor_mode = 1,
			.integration_def = 10,
			.integration = 10,
			.integration_max = 557-22, // From RPI IMX477 driver exp offset is 22
			.integration_min = 7,
			.width = 1920,
			.height = 1080,
			.fps = 200,
			.gain = 200,
			.gain_max = 978,
			.test_pattern =0,
			.bits = 10,
			.hmax = 0x0898,
			.vmax = 0x0465,
			.mode_reg_list  = {
					.num_of_regs = _countof(imx290_1080p_common_settings),
					.regs = imx290_1080p_common_settings,
			},
			.lane_reg_list  = {
					.num_of_regs = _countof(imx290_1080p_4lane_settings),
					.regs = imx290_1080p_4lane_settings,
			},
			.clk_reg_list  = {
					.num_of_regs = _countof(imx290_37_125mhz_clock_1080p),
					.regs = imx290_37_125mhz_clock_1080p,
			},
			.bit_reg_list = {
					.num_of_regs = _countof(imx290_12bit_settings),
					.regs = imx290_12bit_settings,
			},
		},
		{
				.sensor_mode = 1,
				.integration_def = 500,
				.integration = 500,
				.integration_max = 557-22, // From RPI IMX477 driver exp offset is 22
				.integration_min = 7,
				.width = 1280,
				.height = 720,
				.fps = 200,
				.gain = 200,
				.gain_max = 978,
				.test_pattern =0,
				.bits = 10,
				.hmax = 0x0ce4,
				.vmax = 0x02ee,
				.mode_reg_list  = {
						.num_of_regs = _countof(imx290_720p_common_settings),
						.regs = imx290_1080p_common_settings,
				},
				.lane_reg_list  = {
						.num_of_regs = _countof(imx290_720p_4lane_settings),
						.regs = imx290_720p_4lane_settings,
				},
				.clk_reg_list  = {
						.num_of_regs = _countof(imx290_37_125mhz_clock_720p),
						.regs = imx290_37_125mhz_clock_1080p,
				},
				.bit_reg_list = {
						.num_of_regs = _countof(imx290_12bit_settings),
						.regs = imx290_12bit_settings,
				},
		},
		{
				.sensor_mode = 1,
				.integration_def = 500,
				.integration = 500,
				.integration_max = 557-22, // From RPI IMX477 driver exp offset is 22
				.integration_min = 7,
				.width = 1920,
				.height = 1080,
				.fps = 200,
				.gain = 200,
				.gain_max = 978,
				.test_pattern =0,
				.bits = 10,
				.hmax = 0x0898,
				.vmax = 0x0465,
				.mode_reg_list  = {
						.num_of_regs = _countof(imx290_1080p_common_settings),
						.regs = imx290_1080p_common_settings,
				},
				.lane_reg_list  = {
						.num_of_regs = _countof(imx290_1080p_4lane_settings),
						.regs = imx290_1080p_4lane_settings,
				},
				.clk_reg_list  = {
						.num_of_regs = _countof(imx290_37_125mhz_clock_1080p),
						.regs = imx290_37_125mhz_clock_1080p,
				},
				.bit_reg_list = {
						.num_of_regs = _countof(imx290_12bit_settings),
						.regs = imx290_12bit_settings,
				},
		},
		{
				.sensor_mode = 1,
				.integration_def = 500,
				.integration = 500,
				.integration_max = 557-22, // From RPI IMX477 driver exp offset is 22
				.integration_min = 7,
				.width = 1920,
				.height = 1080,
				.fps = 200,
				.gain = 200,
				.gain_max = 978,
				.test_pattern =0,
				.bits = 10,
				.hmax = 0x0898,
				.vmax = 0x0465,
				.mode_reg_list  = {
						.num_of_regs = _countof(imx290_1080p_common_settings),
						.regs = imx290_1080p_common_settings,
				},
				.lane_reg_list  = {
						.num_of_regs = _countof(imx290_1080p_4lane_settings),
						.regs = imx290_1080p_4lane_settings,
				},
				.clk_reg_list  = {
						.num_of_regs = _countof(imx290_37_125mhz_clock_1080p),
						.regs = imx290_37_125mhz_clock_1080p,
				},
				.bit_reg_list = {
						.num_of_regs = _countof(imx290_12bit_settings),
						.regs = imx290_12bit_settings,
				},
		},
		{
				.sensor_mode = 1,
				.integration_def = 500,
				.integration = 500,
				.integration_max = 557-22, // From RPI IMX477 driver exp offset is 22
				.integration_min = 7,
				.width = 1920,
				.height = 1080,
				.fps = 200,
				.gain = 200,
				.gain_max = 978,
				.test_pattern =0,
				.bits = 10,
				.hmax = 0x0898,
				.vmax = 0x0465,
				.mode_reg_list  = {
						.num_of_regs = _countof(imx290_1080p_common_settings),
						.regs = imx290_1080p_common_settings,
				},
				.lane_reg_list  = {
						.num_of_regs = _countof(imx290_1080p_4lane_settings),
						.regs = imx290_1080p_4lane_settings,
				},
				.clk_reg_list  = {
						.num_of_regs = _countof(imx290_37_125mhz_clock_1080p),
						.regs = imx290_37_125mhz_clock_1080p,
				},
				.bit_reg_list = {
						.num_of_regs = _countof(imx290_12bit_settings),
						.regs = imx290_12bit_settings,
				},
		},
		{
				.sensor_mode = 1,
				.integration_def = 500,
				.integration = 500,
				.integration_max = 557-22, // From RPI IMX477 driver exp offset is 22
				.integration_min = 7,
				.width = 1920,
				.height = 1080,
				.fps = 200,
				.gain = 200,
				.gain_max = 978,
				.test_pattern =0,
				.bits = 10,
				.hmax = 0x0898,
				.vmax = 0x0465,
				.mode_reg_list  = {
						.num_of_regs = _countof(imx290_1080p_common_settings),
						.regs = imx290_1080p_common_settings,
				},
				.lane_reg_list  = {
						.num_of_regs = _countof(imx290_1080p_4lane_settings),
						.regs = imx290_1080p_4lane_settings,
				},
				.clk_reg_list  = {
						.num_of_regs = _countof(imx290_37_125mhz_clock_1080p),
						.regs = imx290_37_125mhz_clock_1080p,
				},
				.bit_reg_list = {
						.num_of_regs = _countof(imx290_12bit_settings),
						.regs = imx290_12bit_settings,
				},
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
	sensor_i2c_write(REG_STANDBY , !on);
   return sensor_i2c_write(REG_XMSTA, !on);
}

static CyU3PReturnStatus_t sensor_write_buffered (uint16_t reg_addr, uint8_t n_regs, uint32_t data)
{
	sensor_i2c_write(REG_REGHOLD, 0x01);

	for (uint8_t i = 0; i < n_regs; i++)
	{
		sensor_i2c_write( reg_addr + i,(uint8_t)(data >> (i * 8)));
	}

	return sensor_i2c_write(REG_REGHOLD, 0x00);
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
            sensor_i2c_write(REG_FLIP_WINMODE, iTemp | 0x03);	//Set normal
            break;
        case IMAGE_V_MIRROR:
            sensor_i2c_write(REG_FLIP_WINMODE, iTemp | 0x01);	//Set flip
            break;
        case IMAGE_H_MIRROR:
            sensor_i2c_write(REG_FLIP_WINMODE, iTemp | 0x02);	//Set mirror
            break;
        case IMAGE_HV_MIRROR:
            sensor_i2c_write(REG_FLIP_WINMODE, iTemp);	//Set mirror and flip
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

	//set_mirror_flip(mode->mirror);


	for (uint16_t i = 0; i < mode->clk_reg_list.num_of_regs; i++)
	{
		sensor_i2c_write(((mode->clk_reg_list.regs) + i)->address, ((mode->clk_reg_list.regs) + i)->val);
	}

	for (uint16_t i = 0; i < mode->bit_reg_list.num_of_regs; i++)
	{
		sensor_i2c_write(((mode->bit_reg_list.regs) + i)->address, ((mode->bit_reg_list.regs) + i)->val);
	}

	for (uint16_t i = 0; i < mode->mode_reg_list.num_of_regs; i++)
	{
		sensor_i2c_write(((mode->mode_reg_list.regs) + i)->address, ((mode->mode_reg_list.regs) + i)->val);
	}

	for (uint16_t i = 0; i < mode->lane_reg_list.num_of_regs; i++)
	{
		sensor_i2c_write(((mode->lane_reg_list.regs) + i)->address, ((mode->lane_reg_list.regs) + i)->val);
	}

	sensor_write_buffered(REG_GAIN, 1, mode->gain);
	sensor_write_buffered(REG_EXPOSURE_LOW, 3, mode->integration_def);
	sensor_write_buffered(REG_HMAX_LOW, 2, mode->hmax);
	sensor_write_buffered(REG_VMAX_LOW, 3,mode->vmax);
	sensor_set_test_pattern(mode->test_pattern);

	camera_stream_on(selected_img_mode->sensor_mode);
}

uint8_t	SensorI2cBusTest (void)
{

	return CY_U3P_SUCCESS;
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
	sensor_config = sensor_config_4lane;
	selected_img_mode = &sensor_config[0];
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
	sensor_write_buffered(REG_GAIN, 1, input & 0xFF);
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
	sensor_write_buffered(REG_EXPOSURE_LOW, 3, integration);
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

	if (test_pattern > 0)
	{
		sensor_i2c_write(REG_BLACK_LEVEL_LSB, 0x00);
		sensor_i2c_write(REG_BLACK_LEVEL_MSB, 0x00);
	}
	else
	{
		sensor_i2c_write(REG_BLACK_LEVEL_LSB, 0xF0);
		sensor_i2c_write(REG_BLACK_LEVEL_MSB, 0x00);
	}

	selected_img_mode->test_pattern = test_pattern;
	sensor_i2c_write(REG_PGCTRL, (test_pattern < 8)? (test_pattern<<4) & 0xF: 0);

}



