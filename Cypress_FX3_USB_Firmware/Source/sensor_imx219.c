
#include <cyu3system.h>
#include <cyu3os.h>
#include <cyu3dma.h>
#include <cyu3error.h>
#include <cyu3uart.h>
#include <cyu3i2c.h>
#include <cyu3types.h>
#include <cyu3gpio.h>
#include <cyu3utils.h>
#include "sensor_imx219.h"
#include "uvc_settings.h"

imgsensor_mode_t *selected_img_mode;


static const imx219_reg_t mode_default[]={	//default register settings, Resolution and FPS specific settings will be over written
		 {REG_MODE_SEL,			0x00},
		 {0x30EB,				0x05},	//access sequence
		 {0x30EB,				0x0C},
		 {0x300A,				0xFF},
		 {0x300B,				0xFF},
		 {0x30EB,				0x05},
		 {0x30EB,				0x09},
		 {REG_CSI_LANE,			0x01},	//3-> 4Lane 1-> 2Lane
		 {REG_DPHY_CTRL,		0x00},	//DPHY timing 0-> auot 1-> manual
		 {REG_EXCK_FREQ_MSB,	0x18},	//external oscillator frequncy 0x18 -> 24Mhz
		 {REG_EXCK_FREQ_LSB,	0x00},
		 {REG_FRAME_LEN_MSB,	0x06},	//frame length , Raspberry pi sends this commands continously when recording video @60fps ,writes come at interval of 32ms , Data 355 for resolution 1280x720 command 162 also comes along with data 0DE7 also 15A with data 0200
		 {REG_FRAME_LEN_LSB,	0xE3},
		 {REG_LINE_LEN_MSB,		0x0d},	//does not directly affect how many bits on wire in one line does affect how many clock between lines
		 {REG_LINE_LEN_LSB,		0x78},	//appears to be having step in value, not every LSb change will reflect on fps
		 {REG_X_ADD_STA_MSB,	0x02},	//x start
		 {REG_X_ADD_STA_LSB,	0xA8},
		 {REG_X_ADD_END_MSB,	0x0A},	//x end
		 {REG_X_ADD_END_LSB,	0x27},
		 {REG_Y_ADD_STA_MSB,	0x02},	//y start
		 {REG_Y_ADD_STA_LSB,	0xB4},
		 {REG_Y_ADD_END_MSB,	0x06},	//y end
		 {REG_Y_ADD_END_LSB,	0xEB},
		 {REG_X_OUT_SIZE_MSB,	0x07},	//resolution 1280 -> 5 00 , 1920 -> 780 , 2048 -> 0x8 0x00
		 {REG_X_OUT_SIZE_LSB,	0x80},
		 {REG_Y_OUT_SIZE_MSB,	0x04},	// 720 -> 0x02D0 | 1080 -> 0x438  | this setting changes how many line over wire does not affect frame rate
		 {REG_Y_OUT_SIZE_LSB,	0x38},
		 {REG_X_ODD_INC,		0x01},	//increment
		 {REG_Y_ODD_INC,		0x01},	//increment
		 {REG_BINNING_H,		0x00},	//binning H 0 off 1 x2 2 x4 3 x2 analog
		 {REG_BINNING_V,		0x00},	//binning H 0 off 1 x2 2 x4 3 x2 analog
		 {REG_CSI_FORMAT_C,		0x0A},	//CSI Data format A-> 10bit
		 {REG_CSI_FORMAT_D,		0x0A},	//CSI Data format
		 {REG_VTPXCK_DIV,		0x05},	//vtpxclkd_div	5 301
		 {REG_VTSYCK_DIV,		0x01},	//vtsclk _div  1	303
		 {REG_PREPLLCK_VT_DIV,	0x03},	//external oscillator /3
		 {REG_PREPLLCK_OP_DIV,	0x03},	//external oscillator /3
		 {REG_PLL_VT_MPY_MSB,	0x00},	//PLL_VT multiplizer
		 {REG_PLL_VT_MPY_LSB,	0x52},	//Changes Frame rate with , integration register 0x15a
		 {REG_OPPXCK_DIV,		0x0A},	//oppxck_div
		 {REG_OPSYCK_DIV,		0x01},	//opsysck_div
		 {REG_PLL_OP_MPY_MSB,	0x00},	//PLL_OP
		 {REG_PLL_OP_MPY_LSB,	0x32}, 	// 8Mhz x 0x57 ->696Mhz -> 348Mhz |  0x32 -> 200Mhz | 0x40 -> 256Mhz
		 {0x455E,				0x00},	//magic?
		 {0x471E,				0x4B},
		 {0x4767,				0x0F},
		 {0x4750,				0x14},
		 {0x4540,				0x00},
		 {0x47B4,				0x14},
		 {0x4713,				0x30},
		 {0x478B,				0x10},
		 {0x478F,				0x10},
		 {0x4793,				0x10},
		 {0x4797,				0x0E},
		 {0x479B,				0x0E},
		 {REG_TP_RED_MSB,		0x00},
		 {REG_TP_RED_LSB,		0x00},
		 {REG_TP_GREEN_MSB, 	0x00},
		 {REG_TP_GREEN_LSB,		0x00},
		 {REG_TP_BLUE_MSB,		0x00},
		 {REG_TP_BLUE_LSB,		0x00},
		 {REG_TEST_PATTERN_MSB, 0x00},	//test pattern
		 {REG_TEST_PATTERN_LSB, 0x00},
		 {REG_TP_X_OFFSET_MSB,  0x00}, //tp offset x 0
		 {REG_TP_X_OFFSET_LSB,  0x00},
		 {REG_TP_Y_OFFSET_MSB,  0x00}, //tp offset y 0
		 {REG_TP_Y_OFFSET_LSB,  0x00},
		 {REG_TP_WIDTH_MSB,   	0x05}, //TP width 1920 ->780 1280->500
		 {REG_TP_WIDTH_LSB,   	0x00},
		 {REG_TP_HEIGHT_MSB,   	0x02}, //TP height 1080 -> 438 720->2D0
		 {REG_TP_HEIGHT_LSB,   	0xD0},
		 {REG_DIG_GAIN_GLOBAL_MSB, 	0x01},
		 {REG_DIG_GAIN_GLOBAL_LSB, 	0x00},
		 {REG_ANA_GAIN_GLOBAL, 		0x80}, //analog gain , raspberry pi constinouly changes this depending on scense
		 {REG_INTEGRATION_TIME_MSB, 0x03},	//integration time , really important for frame rate
		 {REG_INTEGRATION_TIME_LSB, 0x51},
		 {REG_MODE_SEL,			0x01},

};

image_sensor_config_t *sensor_config;

static image_sensor_config_t sensor_config_2LANE = {
	.sensor_mode = 0x01,

    .mode_640x480_30 = {  //200Mhz 2Lane 311040 312000
		.pix_clk_mul = 0x2E,
		.pix_clk_div = 0x4, //only 4 or 5 or 8 or 10
		.integration = 1725 - 4,	//must be < (linelength- 4) to maintain frame rate by framelength or integration time will slow frame rate
		.gain = 0x70,
		.linelength = 3448, 	//Warning! This value need to be either 0xD78 or 0xDE7 regardless of frame size and FPS, other values will result undefined and ununderstanable issues in image
		.framelength = 1725,  //decided how long is frame, basically frame rate with pix clock, it has second priority to integration time. absolute minimum is 255 for imx219
		.startx = 1000,
		.starty = 750,
		.endx = 2280,
		.endy = 1715,		//this has to odd or bayer oder will change
		.width = 640,
		.height = 480,	//each frame will have two extra line to compensate for debayer crop
		.binning = 2,
		.test_pattern = 0
	},

	.mode_640x480_200 = {  //200fps with 2 lane and 200MHz mipi
		.pix_clk_mul = 0x2E,
		.pix_clk_div = 0x4,
		.integration = 258 - 4,
		.gain = 0x70,
		.linelength = 3448,
		.framelength = 258,
		.startx = 1000,
		.starty = 750,
		.endx = 2280,
		.endy = 1715,
		.width = 640,
		.height = 480,
		.binning = 2,
		.test_pattern = 0
	},

    .mode_1280x720_30 = {  //200Mhz 2 lane
		.pix_clk_mul = 0x2E,
		.pix_clk_div = 0x4,
		.integration = 1720 - 4,
		.gain = 0x80,
		.linelength = 3448,
		.framelength = 1720,
		.startx = 4,
		.starty = 4,
		.endx = 0xA27,
		.endy = 0x6EB,
		.width = 1280,
		.height = 720,
		.binning = 0,
		.test_pattern = 0
	},

    .mode_1280x720_60 = { //200Mhz mipi 2 lane 927360
		.pix_clk_mul = 0x2E,
		.pix_clk_div = 0x4,
		.integration = 862 - 4,
		.gain = 0x80,
		.linelength = 0xD78,
		.framelength = 862,
		.startx = 0x2A8,
		.starty = 0x2B4,
		.endx = 0xA27,
		.endy = 0x6EB,
		.width = 1280,
		.height = 720,
		.binning = 0,
		.test_pattern = 0
	},

    .mode_1920x1080_30 = {		//200Mhz 2Lane 2082240
		.pix_clk_mul = 0x20,
		.pix_clk_div = 0x4,
		.integration = 1200 - 4,
		.gain = 0x80,
		.linelength = 0xD78,
		.framelength = 1200,
		.startx = 0x2A8,
		.starty = 0x2B4,
		.endx = 0xA27,
		.endy = 0x6EB,
		.width = 1920,
		.height = 1080,
		.binning = 0,
		.test_pattern = 0
	},

	.mode_640x128_600 = {	//200Mhz 2LANE  82944
		.pix_clk_mul = 0x2D,
		.pix_clk_div = 0x4,
		.integration = 84 - 4,
		.gain = 200,
		.linelength = 0xD78,
		.framelength = 84,
		.startx = 1320,
		.starty = 990,
		.endx = 2600,
		.endy = 1609,
		.width = 640,
		.height = 128,
		.binning = 2,
		.test_pattern = 0
	},

    .mode_640x80_900 = {	//200Mhz 2Lane 51840
		.pix_clk_mul = 0x2D,
		.pix_clk_div = 0x4,
		.integration = 56 - 4,
		.gain = 200,
		.linelength = 0xD78,
		.framelength = 56,
		.startx = 1320,
		.starty = 990,
		.endx = 2600,
		.endy = 1561,
		.width = 640,
		.height = 80,
		.binning = 2,
		.test_pattern = 0
	},

	.mode_3280x2464_7 = {	//200Mhz 2 Lane 3055360
		.pix_clk_mul = 0x12,
		.pix_clk_div = 0x4,
		.integration = 2670 - 4,
		.gain = 200,
		.linelength = 0xD78, //3448
		.framelength = 2670,
		.startx = 0,
		.starty = 0,
		.endx = 3279,
		.endy = 2463,
		.width = 3280,
		.height = 2464,
		.binning = 0,
		.test_pattern = 0
	},

};


static image_sensor_config_t sensor_config_4LANE = {
	.sensor_mode = 0x01,

	.mode_640x480_30 = {
		.pix_clk_mul = 0x53,
		.pix_clk_div = 0x4, //only 4 or 5 or 8 or 10
		.integration = 3209 - 4,	//must be < (linelength- 4) to maintain frame rate by framelength or integration time will slow frame rate
		.gain = 0x70,
		.linelength = 3448, 	//Warning! This value need to be either 0xD78 or 0xDE7 regardless of frame size and FPS, other values will result undefined and ununderstanable issues in image
		.framelength = 3209,
		.startx = 1000,
		.starty = 750,
		.endx = 2280,
		.endy = 1715,		//this has to be odd or bayer oder will change
		.width = 640,
		.height = 482,	//each frame will have two extra line to compensate for debayer crop
		.binning = 2,
		.test_pattern = 0
	},
	.mode_640x480_200 = {
		.pix_clk_mul = 0x53,
		.pix_clk_div = 0x4,
		.integration = 481 - 4,
		.gain = 0x70,
		.linelength = 3448,
		.framelength = 481,
		.startx = 1000,
		.starty = 750,
		.endx = 2280,
		.endy = 1715,
		.width = 640,
		.height = 482,
		.binning = 2,
		.test_pattern = 0
	},

	.mode_1280x720_30 = {
		.pix_clk_mul = 0x58,
		.pix_clk_div = 0x4,
		.integration = 3402 - 10,
		.gain = 0x80,
		.linelength = 3448,
		.framelength = 3402,
		.startx = 4,
		.starty = 4,
		.endx = 0xA27,
		.endy = 0x6EB,
		.width = 1280,
		.height = 722,
		.binning = 0,
		.test_pattern = 0
	},
	.mode_1280x720_60 = {
		.pix_clk_mul = 0x58,
		.pix_clk_div = 0x4,
		.integration = 1701 - 4,
		.gain = 0x80,
		.linelength = 0xD78,
		.framelength = 1701,
		.startx = 0x2A8,
		.starty = 0x2B4,
		.endx = 0xA27,
		.endy = 0x6EB,
		.width = 1280,
		.height = 722,
		.binning = 0,
		.test_pattern = 0
	},
	.mode_1280x720_120 = {		//Camera output @120
		.pix_clk_mul = 0x58,
		.pix_clk_div = 0x4,
		.integration = 850 - 4,
		.gain = 0x80,
		.linelength = 0xD78,
		.framelength = 850,
		.startx = 0x2A8,
		.starty = 0x2B4,
		.endx = 0xA27,
		.endy = 0x6EB,
		.width = 1280,
		.height = 722,
		.binning = 0,
		.test_pattern = 0
	},
	.mode_1920x1080_30 = {		//camera output 1920x1080 @30FPS
		.pix_clk_mul = 0x50,
		.pix_clk_div = 0x5,
		.integration = 2474 - 4,
		.gain = 0x80,
		.linelength = 0xD78,
		.framelength = 2474,
		.startx = 0x2A8,
		.starty = 0x2B4,
		.endx = 0xA27,
		.endy = 0x6EB,
		.width = 1920,
		.height = 1082,
		.binning = 0,
		.test_pattern = 0
	},
	.mode_1920x1080_60 = {		//camera output 1920x1080 @60FPS
		.pix_clk_mul = 0x50,
		.pix_clk_div = 0x5,
		.integration = 1237 - 4,
		.gain = 0x80,
		.linelength = 0xD78,
		.framelength = 1237,
		.startx = 0x2A8,
		.starty = 0x2B4,
		.endx = 0xA27,
		.endy = 0x6EB,
		.width = 1920,
		.height = 1082,
		.binning = 0,
		.test_pattern = 0
	},
	.mode_640x128_682 = {	//camera output 640x128 pixel @682 FPS
		.pix_clk_mul = 0x53,
		.pix_clk_div = 0x4,
		.integration = 141 - 4,
		.gain = 200,
		.linelength = 0xD78,
		.framelength = 141,
		.startx = 1320,
		.starty = 990,
		.endx = 2600,
		.endy = 1609,
		.width = 640,
		.height = 128,
		.binning = 2,
		.test_pattern = 0
	},

	.mode_640x80_1000 = {	//camera output 640x80 pixel @1000 FPS
		.pix_clk_mul = 0x53,
		.pix_clk_div = 0x4,
		.integration = 96 - 4,
		.gain = 200,
		.linelength = 0xD78,
		.framelength = 96,
		.startx = 1320,
		.starty = 990,
		.endx = 2600,
		.endy = 1561,
		.width = 640,
		.height = 80,
		.binning = 2,
		.test_pattern = 0
	},
	.mode_3280x2464_15 = {	//full frame 3280x2464 @15FPS
		.pix_clk_mul = 0x31,
		.pix_clk_div = 0x5,
		.integration = 3031 - 4,
		.gain = 200,
		.linelength = 0xD78,
		.framelength = 3031,
		.startx = 0,
		.starty = 0,
		.endx = 3279,
		.endy = 2463,
		.width = 3280,
		.height = 2464,
		.binning = 0,
		.test_pattern = 0
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
//	CyU3PDebugPrint(4,"image_mirror = %d\n", image_mirror);

	/********************************************************
	   *
	   *   0x3820[2] ISP Vertical flip
	   *   0x3820[1] Sensor Vertical flip
	   *
	   *   0x3821[2] ISP Horizontal mirror
	   *   0x3821[1] Sensor Horizontal mirror
	   *
	   *   ISP and Sensor flip or mirror register bit should be the same!!
	   *
	   ********************************************************/
	uint8_t  iTemp;

	image_mirror = IMAGE_NORMAL;
    sensor_i2c_read(REG_IMG_ORIENT ,  iTemp);
    iTemp = iTemp & 0x03;
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
		case FRAME_640x480:
		{
			if (interval == INTERVAL_30)
			{
				selected_img_mode = &sensor_config->mode_640x480_30;
			}
			else if(interval == INTERVAL_200)
			{
				selected_img_mode = &sensor_config->mode_640x480_200;
			}

		}
		break;
		case FRAME_1280x720:
		{
			if (interval == INTERVAL_30)
			{
				selected_img_mode = &sensor_config->mode_1280x720_30;
			}
			else if(interval == INTERVAL_60)
			{
				selected_img_mode = &sensor_config->mode_1280x720_60;
			}
			else if(interval == INTERVAL_120)
			{
				//selected_img_mode = &sensor_config->mode_1280x720_120;
				selected_img_mode = &sensor_config->mode_1280x720_60;
			}
		}
		break;
		case FRAME_1920x1080:
		{
			if (interval == INTERVAL_30)
			{
				selected_img_mode = &sensor_config->mode_1920x1080_30;
			}
			else if(interval == INTERVAL_60)
			{
				//selected_img_mode = &sensor_config->mode_1920x1080_60;
				selected_img_mode = &sensor_config->mode_1920x1080_30;

			}
		}
		break;
		case FRAME_3280x2464:
		{
			if (interval == INTERVAL_7)
			{
				//selected_img_mode = &sensor_config->mode_3280x2464_15;
				selected_img_mode = &sensor_config->mode_3280x2464_7;
			}
		}
		break;
		case FRMAE_640x128:
		{
			if (interval == INTERVAL_682)
			{
				selected_img_mode = &sensor_config->mode_640x128_682;
			}
		}
		break;
		case FRMAE_640x80:
		{
			if (interval == INTERVAL_1000)
			{
				selected_img_mode = &sensor_config->mode_640x80_1000;
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
	set_mirror_flip(mode->mirror);
	camera_stream_on(false);
	sensor_i2c_write(REG_PLL_VT_MPY_MSB, GET_WORD_MSB(mode->pix_clk_mul));
	sensor_i2c_write(REG_PLL_VT_MPY_LSB, GET_WORD_LSB(mode->pix_clk_mul));

	sensor_i2c_write(REG_VTPXCK_DIV, GET_WORD_LSB(mode->pix_clk_div));

	sensor_i2c_write(REG_INTEGRATION_TIME_MSB, GET_WORD_MSB(mode->integration));
	sensor_i2c_write(REG_INTEGRATION_TIME_LSB, GET_WORD_LSB(mode->integration));

	sensor_i2c_write(REG_ANALOG_GAIN, 	GET_WORD_LSB(mode->gain));
	sensor_i2c_write(REG_LINE_LEN_MSB, 	GET_WORD_MSB(mode->linelength));
	sensor_i2c_write(REG_LINE_LEN_LSB, 	GET_WORD_LSB(mode->linelength));

	sensor_i2c_write(REG_FRAME_LEN_MSB, GET_WORD_MSB(mode->framelength));
	sensor_i2c_write(REG_FRAME_LEN_LSB, GET_WORD_LSB(mode->framelength));

	sensor_i2c_write(REG_X_ADD_STA_MSB, GET_WORD_MSB(mode->startx));
	sensor_i2c_write(REG_X_ADD_STA_LSB, GET_WORD_LSB(mode->startx));

	sensor_i2c_write(REG_Y_ADD_STA_MSB, GET_WORD_MSB(mode->starty));
	sensor_i2c_write(REG_Y_ADD_STA_LSB, GET_WORD_LSB(mode->starty));

	sensor_i2c_write(REG_X_ADD_END_MSB, GET_WORD_MSB(mode->endx));
	sensor_i2c_write(REG_X_ADD_END_LSB, GET_WORD_LSB(mode->endx));

	sensor_i2c_write(REG_Y_ADD_END_MSB, GET_WORD_MSB(mode->endy));
	sensor_i2c_write(REG_Y_ADD_END_LSB, GET_WORD_LSB(mode->endy));

	sensor_i2c_write(REG_X_OUT_SIZE_MSB, GET_WORD_MSB(mode->width));
	sensor_i2c_write(REG_X_OUT_SIZE_LSB, GET_WORD_LSB(mode->width));

	sensor_i2c_write(REG_Y_OUT_SIZE_MSB, GET_WORD_MSB(mode->height));
	sensor_i2c_write(REG_Y_OUT_SIZE_LSB, GET_WORD_LSB(mode->height));

	sensor_i2c_write(REG_TEST_PATTERN_LSB, (mode->test_pattern < 8)? mode->test_pattern : 0);

	sensor_i2c_write(REG_TP_WIDTH_MSB, GET_WORD_MSB(mode->width));
	sensor_i2c_write(REG_TP_WIDTH_LSB, GET_WORD_LSB(mode->width));
	sensor_i2c_write(REG_TP_HEIGHT_MSB, GET_WORD_MSB(mode->height));
	sensor_i2c_write(REG_TP_HEIGHT_LSB, GET_WORD_LSB(mode->height));

	if ( mode->binning == 2)
	{
	sensor_i2c_write(REG_BINNING_H, 0x03);
	sensor_i2c_write(REG_BINNING_V, 0x03);
	}
	else
	{
		sensor_i2c_write(REG_BINNING_H, 0x00);
		sensor_i2c_write(REG_BINNING_V, 0x00);
	}
	camera_stream_on(sensor_config->sensor_mode);
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
	sensor_config = &sensor_config_2LANE;
	sensor_configure_mode(&sensor_config->mode_1280x720_30);
}

uint8_t SensorGetBrightness (void)
{
    return selected_img_mode->gain;
}

void SensorSetBrightness (uint8_t input)
{
	selected_img_mode->gain = input;
	sensor_i2c_write (REG_ANALOG_GAIN, input);
}

uint16_t sensor_get_min_exposure (void)
{
	return 0;
}

uint16_t sensor_get_max_exposure (void)
{
	return selected_img_mode->integration;
}

uint16_t sensor_get_def_exposure (void)
{
	return selected_img_mode->integration;
}

uint16_t sensor_get_exposure (void)
{
	return selected_img_mode->integration;
}

void sensor_set_exposure (uint16_t integration)
{
	if (integration > selected_img_mode->integration)
	{
		integration = selected_img_mode->integration;
	}
	sensor_i2c_write (REG_INTEGRATION_TIME_MSB, (integration >> 8) & 0xFF);
	sensor_i2c_write (REG_INTEGRATION_TIME_LSB, integration & 0xFF);
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
