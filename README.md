### Opensource USB C industrial camera with Interchangeable C mount Lens, Interchangeable MIPI Sensor with Lattice Crosslink NX FPGA Cypress FX3 USB 3.0 controller



##### ISP Pipline Specifications 
No virtual restrication on Supported Frame Rate or Resolution Tested more than 4K with IMX477 4056 x 3040. Can do 8K with around 30FPS or even higher than that as long as FPGA is fast enough for needed frame rate and FPGA has enough memory to be able to store ~ 6 Line worth of pixels. 

##### Configurability
Selectable RAW pixel width
FPGA Design is configurable with parameters to support pixel depth from RAW10 to RAW14 or Veritually any bit depth even 16bit RAW when it becomes a MIPI Spec, 
Selectable number of MIPI lanes
With just definition of Parameter value number of lane is also configurable between 2 or 4 MIPI lanee, 
Selectable Pipline Size
Pipline is Configurable with a parameter to Process 2,4 or 8 Pixel.  2 Pixel Per Clock is only available with 2 Lane MIPI while 8 is only available with 4 Lanes. 
Selectable MIPI Gear Ratio
User can select weather to operate MIPI/DDR phy in 16x or 8x Gear ratio. Most DDR/MIPI phy supports 8x Gear while few do support 16x gear. 

##### Speed
MIPI Speed can reach upto 900Mbitsps Has been Tested upto 900Mbitps with 8x Gear.
Pixel Processing pipeline with 2,4 or 8 Pixel per clock can reach 110Mhz or more with Lattice Crosslink-NX High Speed, So basically Can process upto 880 MegaPixels per second. With this can reach Around 120FPS with 4K resolution and around 30 FPS with 8K. Or even 3000 FPS with 640 x 480 .  
FPGA Oputput Pipeline that runs on output clock, It feeds into Cypress FX3 32bit GPIF can do Max 160Mhz. Which FX3's specs limits max GPIF clock to 100Mhz. 

##### Tests 
2 Lane 10 bit IMX477
1332x990  60 FPS

2 Lane 10 bit IMX219
3280x2464 7 FPS

1280x720  30 FPS

1280x720  60 FPS

1920x1080 30 FPS

640x480   30 FPS

640x480   200 FPS

640x128   600 FPS

640x80    900 FPS



#### Project Blog post 
https://www.circuitvalley.com/2022/06/pensource-usb-c-industrial-camera-c-mount-fpga-imx-mipi-usb-3-crosslinknx.html


Shield: [![CC BY 4.0][cc-by-shield]][cc-by]

This work is licensed under a [Creative Commons Attribution 4.0 International
License][cc-by].

[![CC BY 4.0][cc-by-image]][cc-by]

[cc-by]: http://creativecommons.org/licenses/by/4.0/
[cc-by-image]: https://i.creativecommons.org/l/by/4.0/88x31.png
[cc-by-shield]: https://img.shields.io/badge/License-CC%20BY%204.0-lightgrey.svg

![alt text](https://github.com/circuitvalley/USB_C_Industrial_Camera_FPGA_USB3/raw/master/Hardware/Images/usb_c_fpga_mipi_camera_c_mount_industrial_lattice_crosslink_fpga_xilinx_zynq%20(4).JPG)

![alt text](https://github.com/circuitvalley/USB_C_Industrial_Camera_FPGA_USB3/raw/master/Hardware/Images/usb_c_fpga_mipi_camera_c_mount_industrial_lattice_crosslink_fpga_xilinx_zynq%20(3).JPG)

![alt text](https://github.com/circuitvalley/USB_C_Industrial_Camera_FPGA_USB3/raw/master/Hardware/Images/usb_c_fpga_mipi_camera_c_mount_industrial_lattice_crosslink_fpga_xilinx_zynq%20(33).JPG)

![alt text](https://github.com/circuitvalley/USB_C_Industrial_Camera_FPGA_USB3/raw/master/Hardware/Images/usb_c_fpga_mipi_camera_c_mount_industrial_lattice_crosslink_fpga_xilinx_zynq%20(31).JPG)

![alt text](https://github.com/circuitvalley/USB_C_Industrial_Camera_FPGA_USB3/raw/master/Hardware/Images/usb_c_fpga_mipi_camera_c_mount_industrial_lattice_crosslink_fpga_xilinx_zynq%20(19).JPG)

![alt text](https://github.com/circuitvalley/USB_C_Industrial_Camera_FPGA_USB3/raw/master/Hardware/Images/usb_c_fpga_mipi_camera_c_mount_industrial_lattice_crosslink_fpga_xilinx_zynq%20(12)24.JPG)


