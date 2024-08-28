#ifndef ADXL355_H
#define ADXL355_H

#include "Communication.h"
#include "System/gpio.h"
#include "System/spi.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "BLE_Services/messageCodes.h"
#include <string.h>

//#define SPI_SEND_REG 1

#define  xAxis 0xAB
#define  yAxis 0xCD

#define SPI_SEND_REG 1

#define ADXL_RANGE 2

/**
   @brief  ADXL355 registers addresses
   @return none
**/
#define DEVID_AD 0x00     /* This register contains the Analog Devices ID, 0xAD */
#define DEVID_MST 0x01    /* This register contains the Analog Devices MEMS ID, 0x1D. */
#define PARTID 0x02       /* This register contains the device ID, 0xED (355 octal). */
#define REVID 0x03        /* This register contains the product revision ID, beginning with 0x00 and incrementing for each subsequent revision. */
#define STATUS 0x04       /* This register includes bits that describe the various conditions of the ADXL355. */
#define FIFO_ENTRIES 0x05 /* This register indicates the number of valid data samples present in the FIFO buffer. This number ranges from 0 to 96. */
#define TEMP2 0x06        /* These two registers contain the uncalibrated temperature data. The nominal intercept is 1852 LSB at 25°C and the nominal slope is \
                             -9.05 LSB/°C. TEMP2 contains the four most significant bits, and TEMP1 contains the eight least significant bits of the 12-bit value.  */
#define TEMP1 0x07        /* These two registers contain the uncalibrated temperature data. The nominal intercept is 1852 LSB at 25°C and the nominal slope is \
                             -9.05 LSB/°C. TEMP2 contains the four most significant bits, and TEMP1 contains the eight least significant bits of the 12-bit value. */
#define XDATA3 0x08       /* These three registers contain the x-axis acceleration data. Data is left justified and formatted as twos complement. */
#define XDATA2 0x09       /* These three registers contain the x-axis acceleration data. Data is left justified and formatted as twos complement. */
#define XDATA1 0x0A       /* These three registers contain the x-axis acceleration data. Data is left justified and formatted as twos complement. */
#define YDATA3 0x0B       /* These three registers contain the y-axis acceleration data. Data is left justified and formatted as twos complement. */
#define YDATA2 0x0C       /* These three registers contain the y-axis acceleration data. Data is left justified and formatted as twos complement. */
#define YDATA1 0x0D       /* These three registers contain the y-axis acceleration data. Data is left justified and formatted as twos complement. */
#define ZDATA3 0x0E       /* These three registers contain the z-axis acceleration data. Data is left justified and formatted as twos complement. */
#define ZDATA2 0x0F       /* These three registers contain the z-axis acceleration data. Data is left justified and formatted as twos complement. */
#define ZDATA1 0x10       /* These three registers contain the z-axis acceleration data. Data is left justified and formatted as twos complement. */
#define FIFO_DATA 0x11    /* Read this register to access data stored in the FIFO. */
#define OFFSET_X_H 0x1E   /* X-AXIS OFFSET TRIM REGISTERS OFFSET_X_H */
#define OFFSET_X_L 0x1F   /* X-AXIS OFFSET TRIM REGISTERS OFFSET_X_L */
#define OFFSET_Y_H 0x20   /* Y-AXIS OFFSET TRIM REGISTERS  OFFSET_Y_H */
#define OFFSET_Y_L 0x21   /* Y-AXIS OFFSET TRIM REGISTERS  OFFSET_Y_L */
#define OFFSET_Z_H 0x22   /* Z-AXIS OFFSET TRIM REGISTERS OFFSET_Z_H  */
#define OFFSET_Z_L 0x23   /* Z-AXIS OFFSET TRIM REGISTERS OFFSET_Z_L */
#define ACT_EN 0x24       /* ACTIVITY ENABLE REGISTER */
#define ACT_THRESH_H 0x25 /* ACTIVITY THRESHOLD REGISTERS ACT_THRESH_H  */
#define ACT_THRESH_L 0x26 /* ACTIVITY THRESHOLD REGISTERS ACT_THRESH_L */
#define ACT_COUNT 0x27    /* ACTIVITY COUNT REGISTER */
#define FILTER 0x28       /* FILTER SETTINGS REGISTER  */
#define FIFO_SAMPLES 0x29 /* FIFO SAMPLES REGISTER */
#define INT_MAP 0x2A      /* INTERRUPT PIN (INTx) FUNCTION MAP REGISTER  */
#define SYNC 0x2B         /* DATA SYNCHRONIZATION */
#define POWER_CTL 0x2D    /* POWER CONTROL REGISTER */
#define SELF_TEST 0x2E    /* SELF TEST REGISTER */
#define RESET 0x2F        /* RESET REGISTER  */

#define ODR_125_Hz 0x05            /* ADR 125 hz set  */
#define HPF_CORNER_ODR_125_Hz 0x25 /* ADR 125 hz set  */
#define HPF_CORNER_ODR_62_5_Hz 0x06 /* ADR 62,5 hz set  */
#define HPF_CORNER_ODR_4_Hz 0x2A /* ADR 3,906 hz set  */


#define FFT_BUFFER_SIZE 2048
#define blocksize 1024

#define LORAWAN_DATA_MAX_SIZE 242


typedef struct ADXL355_s {
  Gpio_t CSPin;
  Gpio_t Int1Pin;
  Gpio_t Int2Pin;
  Gpio_t DRDYPin;
  Spi_t Spi;
} ADXL355_t;


/**
   @brief  ADXL355 functions
   @return none
**/
void vAdxlAccelerometerInit(void);
int32_t xAdxlAccelerationDataConversion(void);
void vAdxlSensorDataScan(void);
void vAdxlAccelerometerStop(void);
void vAdxlAccelerometerStart(void);

void vAdxlGetDeviceID(void);
void vAdxlGetDeviceMemsID(void);
void vAdxlGetID(void);
void vAdxlGetProcductVersionID(void);
void vAdxlGetStatus(void);
uint8_t uxAdxlGetFIFOEntriesRegister(void);
void vAdxlGetTemperatureData(void);
void vAdxlGetFIFOAccess(void);
void vAdxlXOffsetTrim(uint8_t, uint8_t);
void vAdxlYOffsetTrim(uint8_t, uint8_t);
void vAdxlZOffsetTrim(uint8_t, uint8_t);
void vAdxlActivityEnable(uint8_t, uint8_t);
void vAdxlActivityTreshold(uint8_t, uint8_t);
void vAdxlActivityCount(uint8_t, uint8_t);
void vAdxlFilterSetting(uint8_t, uint8_t);
void vAdxlInterruptEnableMap(uint8_t, uint8_t);
void vAdxlInterruptEnableMap(uint8_t, uint8_t);
void vAdxlDataSynchronization(uint8_t, uint8_t);
void vAdxlPowerControl(uint8_t, uint8_t);
void vAdxlSelfTest(uint8_t, uint8_t);
void vAdxlReset(void);
void vAdxlWriteFIFORegisters(void);

extern ADXL355_t Adxl355;

#endif /* NRF52_H */