#ifndef ADXL355_H
#define ADXL355_H

#include "nrf_drv_spi.h"



#define SPI_SEND_REG             1

#define CSACC_PIN                5
#define INT1ACC_PIN              8
#define INT2ACC_PIN              7
#define DATARDYACC_PIN           6
#define ADXL_RANGE               2 

/**
   @brief  ADXL355 registers addresses
   @return none
**/
#define DEVID_AD                 0x00   /* This register contains the Analog Devices ID, 0xAD */
#define DEVID_MST                0x01   /* This register contains the Analog Devices MEMS ID, 0x1D. */
#define PARTID                   0x02   /* This register contains the device ID, 0xED (355 octal). */
#define REVID                    0x03   /* This register contains the product revision ID, beginning with 0x00 and incrementing for each subsequent revision. */
#define STATUS                   0x04   /* This register includes bits that describe the various conditions of the ADXL355. */
#define FIFO_ENTRIES             0x05   /* This register indicates the number of valid data samples present in the FIFO buffer. This number ranges from 0 to 96. */
#define TEMP2                    0x06   /* These two registers contain the uncalibrated temperature data. The nominal intercept is 1852 LSB at 25°C and the nominal slope is
                                           -9.05 LSB/°C. TEMP2 contains the four most significant bits, and TEMP1 contains the eight least significant bits of the 12-bit value.  */
#define TEMP1                    0x07   /* These two registers contain the uncalibrated temperature data. The nominal intercept is 1852 LSB at 25°C and the nominal slope is
                                           -9.05 LSB/°C. TEMP2 contains the four most significant bits, and TEMP1 contains the eight least significant bits of the 12-bit value. */
#define XDATA3                   0x08   /* These three registers contain the x-axis acceleration data. Data is left justified and formatted as twos complement. */
#define XDATA2                   0x09   /* These three registers contain the x-axis acceleration data. Data is left justified and formatted as twos complement. */
#define XDATA1                   0x0A   /* These three registers contain the x-axis acceleration data. Data is left justified and formatted as twos complement. */
#define YDATA3                   0x0B   /* These three registers contain the y-axis acceleration data. Data is left justified and formatted as twos complement. */
#define YDATA2                   0x0C   /* These three registers contain the y-axis acceleration data. Data is left justified and formatted as twos complement. */
#define YDATA1                   0x0D   /* These three registers contain the y-axis acceleration data. Data is left justified and formatted as twos complement. */
#define ZDATA3                   0x0E   /* These three registers contain the z-axis acceleration data. Data is left justified and formatted as twos complement. */
#define ZDATA2                   0x0F   /* These three registers contain the z-axis acceleration data. Data is left justified and formatted as twos complement. */
#define ZDATA1                   0x10   /* These three registers contain the z-axis acceleration data. Data is left justified and formatted as twos complement. */
#define FIFO_DATA                0x11   /* Read this register to access data stored in the FIFO. */
#define OFFSET_X_H               0x1E   /* X-AXIS OFFSET TRIM REGISTERS OFFSET_X_H */
#define OFFSET_X_L               0x1F   /* X-AXIS OFFSET TRIM REGISTERS OFFSET_X_L */
#define OFFSET_Y_H               0x20   /* Y-AXIS OFFSET TRIM REGISTERS  OFFSET_Y_H */
#define OFFSET_Y_L               0x21   /* Y-AXIS OFFSET TRIM REGISTERS  OFFSET_Y_L */
#define OFFSET_Z_H               0x22   /* Z-AXIS OFFSET TRIM REGISTERS OFFSET_Z_H  */
#define OFFSET_Z_L               0x23   /* Z-AXIS OFFSET TRIM REGISTERS OFFSET_Z_L */
#define ACT_EN                   0x24   /* ACTIVITY ENABLE REGISTER */
#define ACT_THRESH_H             0x25   /* ACTIVITY THRESHOLD REGISTERS ACT_THRESH_H  */
#define ACT_THRESH_L             0x26   /* ACTIVITY THRESHOLD REGISTERS ACT_THRESH_L */
#define ACT_COUNT                0x27   /* ACTIVITY COUNT REGISTER */
#define FILTER                   0x28   /* FILTER SETTINGS REGISTER  */
#define FIFO_SAMPLES             0x29   /* FIFO SAMPLES REGISTER */
#define INT_MAP                  0x2A   /* INTERRUPT PIN (INTx) FUNCTION MAP REGISTER  */
#define SYNC                     0x2B   /* DATA SYNCHRONIZATION */
#define POWER_CTL                0x2D   /* POWER CONTROL REGISTER */
#define SELF_TEST                0x2E   /* SELF TEST REGISTER */
#define RESET                    0x2F   /* RESET REGISTER  */


/**
   @brief  ADXL355 functions
   @return none
**/
void ADXL355_Init(void);
void ADXL355_Start_Sensor(void);
int32_t ADXL355_Acceleration_Data_Conversion (uint32_t);
void ADXL355_Data_Scan(void);
void ADXL355_Stop_Sensor(void);
void ADXL355_Start_Sensor(void);



void GetDeviceID (void);
void GetDeviceMemsID (void);
void GetID (void);
void GetProcductVersionID (void);
void GetStatus (void);
void GetFIFOEntriesRegister (void);
void GetTemperatureData (void);
void GetFIFOAccess (void);
void XOffsetTrim (uint8_t, uint8_t);
void YOffsetTrim (uint8_t, uint8_t);
void ZOffsetTrim (uint8_t, uint8_t);
void ActivityEnable (uint8_t, uint8_t);
void ActivityTreshold (uint8_t, uint8_t);
void ActivityCount (uint8_t, uint8_t);
void FilterSetting (uint8_t, uint8_t);
void InterruptEnableMap (uint8_t, uint8_t);
void InterruptEnableMap (uint8_t, uint8_t);
void DataSynchronization (uint8_t, uint8_t);
void PowerControl (uint8_t, uint8_t);
void SelfTest (uint8_t, uint8_t);
void Reset (void);




#endif /* NRF52_H */