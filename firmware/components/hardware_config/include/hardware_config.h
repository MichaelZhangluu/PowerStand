#pragma once

// Hardware Configuration for the LCD module MSP4021
// 1. SPI Ports.
#define LCD_MSP4021_DISPLAY_SPI_PORT (SPI2_HOST)
#define LCD_MSP4021_TOUCH_SPI_PORT (SPI3_HOST)
#define LCD_MSP4021_SD_SPI_PORT (SPI3_HOST)

// 2. LCD Module J2/Header 14
//  Pin#,   Lable,      ESP32 Pin,  Description
// 2.1 ST7796 driver pin connections (SPI2/HSPI)
//  1,      VCC,        V_In,       5V power input, J1 is open, use 5V
//  2,      GND,        GND,        Ground
//  3,      CS#,        IO12,       Chip select, active low, SPI 2 CS0#
#define LCD_MSP4021_CS (GPIO_NUM_12)
//  4,      RST#	    EN#,        Reset signal, active low
#define LCD_MSP4021_RST (GPIO_NUM_NC)
//  5,      DC/RS	    IO27,       Command/data selection, also called D/C. 0 = command, 1 = data, 8.4.2.1 Write Sequence
#define LCD_MSP4021_DC (GPIO_NUM_27)
//  6,      SDI(MOSI)	IO13,       SPI MOSI, SPI 2 MOSI
#define LCD_MSP4021_MOSI (GPIO_NUM_13)
//  7,      SCK         IO14,       SPI clock, SPI2 SCK
#define LCD_MSP4021_SCK (GPIO_NUM_14)
//  8,      LED         IO26,       Backlight control. Constant high or PWM.
#define LCD_MSP4021_BCKL (GPIO_NUM_26)
//  9,      SDO(MISO)	NC,         SPI MISO. SPI2 MISO
#define LCD_MSP4021_MISO (GPIO_NUM_NC)

// 2.2 XPT2046 touch controller pin connections (SPI3/VSPI)
// 10,      T_CLK,      IO25,	    SPI clock, SPI3 SCK
#define LCD_MSP4021_TOUCH_SCK (GPIO_NUM_25)
// 11,      T_CS,       IO33,	    Chip select, active low
#define LCD_MSP4021_TOUCH_CS (GPIO_NUM_33)
// 12,      T_DIN,      IO32,	    SPI MOSI, SPI 3 MOSI
#define LCD_MSP4021_TOUCH_MOSI (GPIO_NUM_32)
// 13,      T_DO,       IO35,	    SPI MISO, SPI 3 MISO
#define LCD_MSP4021_TOUCH_MISO (GPIO_NUM_35)
// 14,      T_IRQ,      IO34,       Interrupt, active low
#define LCD_MSP4021_TOUCH_IRQ (GPIO_NUM_34)

// 3. LCD Module J4/Header 4
// 2.3 SD Card pin connections (SPI3/VSPI)
// 1,       SD_CS,      IO05,       Chip select, active low
#define LCD_MSP4021_SD_CS (GPIO_NUM_5)
// 2,       SD_MOSI     IO32        SPI MOSI
#define LCD_MSP4021_SD_MOSI (GPIO_NUM_32)
// 3,       SD_MISO     IO35        SPI MISO
#define LCD_MSP4021_SD_MISO (GPIO_NUM_35)
// 4,       SD_CLK      IO25        SPI clock
#define LCD_MSP4021_SD_SCK (GPIO_NUM_25)

// Hardware Configuration for Rotary Encoder KY040
#define KY040_ROTARY_ENCODER_CLK_GPIO_NUM   (GPIO_NUM_19)
#define KY040_ROTARY_ENCODER_DT_GPIO_NUM    (GPIO_NUM_23)
#define KY040_ROTARY_ENCODER_SW_GPIO_NUM    (GPIO_NUM_18)

// Hardware Configuration for I2C Master Device
#define I2C_MASTER_DEVICE_PORT_NUM          (I2C_NUM_0)
#define I2C_MASTER_DEVICE_SDA_GPIO_NUM      (GPIO_NUM_21)
#define I2C_MASTER_DEVICE_SCL_GPIO_NUM      (GPIO_NUM_22)

// Hardware Configuration for I2C IO Expander PCF8574
#define I2C_DEVICE_SCL_SPEED_HZ_IO_EXPENDER_PCF8574     (100000) // 100 kHz
#define I2C_DEVICE_SCL_WAIT_US_IO_EXPENDER_PCF8574      (0) // Timeout value in microseconds

// Hardware Configruation for I2C LCD 2004A
#define I2C_DEVICE_PORT_NUM_IO_EXPENDER_LCD_2004A       (I2C_MASTER_DEVICE_PORT_NUM)
#define I2C_DEVICE_ADDRESS_IO_EXPENDER_LCD_2004A        (0x27) // I2C address for the lcd 2004A IO expender
