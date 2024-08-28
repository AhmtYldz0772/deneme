
/*---------------------------------------------FreeRTOS Includes-----------------------------------------*/
#include "FreeRTOS.h"
#include "SEGGER_RTT.h"
#include "fds.h"
#include "nrf_drv_gpiote.h"
#include "semphr.h"
#include "spi-board.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
/*-------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------CMSIS Includes-----------------------------------------*/
#include "arm_math.h"
/*-------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------gpio-board Includes-----------------------------------------*/
#include "gpio-board.h"
/*-------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------ADX355 Includes-----------------------------------------*/
#include "ADXL355.h"
/*-------------------------------------------------------------------------------------------------------*/

//-------------------------------------------- LoRaWAN Includes ------------------------------------------//
#include "Commissioning.h"
#include "Mac/LoRaMac.h"
#include "System/gpio.h"
#include "board.h"
#include "utilities.h"

#include "System/spi.h"
#include "board-config.h"
//-------------------------------------------------------------------------------------------------------//

/*---------------------------------------------Initialize Includes-----------------------------------------*/
#include "Initialize.h"
/*-------------------------------------------------------------------------------------------------------*/


#include "custom_board.h"
#include "eeprom.h"

//---------------------------------------------- ADXL355 --------------------------------------//
//float32_t XFifoBuffer[fifoFFTBufferSize] = {0};
//float32_t YFifoBuffer[fifoFFTBufferSize] = {0};
//float32_t FFTFifoBuffer[fifoFFTBufferSize] = {0};

extern uint8_t fifoBuffer[FIFOBUFFERSIZE];
int32_t volatile i32SensorX;
int32_t volatile i32SensorY;
int32_t volatile i32SensorZ;
int32_t volatile i32SensorT;

uint32_t Xfifo = 0U;
int32_t signedXfifo = 0U;
int32_t signedYfifo = 0U;
uint32_t Yfifo = 0U;

uint8_t counterTwi = 0;

uint32_t sensorDataCounter = 0U;


//-------------------------------------------------------------------------------------------------------//


//---------------------------------------------- Binary Semaphore Parameters --------------------------------------//
/* this is the queue which WriteFIFORegisters uses to put the plot signal flag */
extern xQueueHandle xPlotSignalQueue;

//-------------------------------------------------------------------------------------------------------//

extern float32_t adxl355Scale;
extern uint8_t SPIRxBuf[SPIRXSIZE]; /* RX buffer. */

//extern arm_rfft_fast_instance_f32 fft_instance;
extern uint32_t addressWrite;

ADXL355_t Adxl355;

/**
   @brief Interrupt 1 Handler 
   @return none
**/
void vAdxlInt1Handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
}

/**
   @brief Interrupt 2 Handler 
   @return none
**/
void vAdxlInt2Handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
}

/**
   @brief DRDY  Handler 
   @return none
**/
void vAdxlDRDYHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {

}

/**
   @brief Initialization the accelerometer sensor
   @return none
**/
void vAdxlAccelerometerInit(void) {

  CheckGpioInitialized();

  /* CS Pin Gpio Init */
  GpioInit(&Adxl355.CSPin, SPIADXL_SS_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);

  /* Int1 Pin Gpio Init */
  Adxl355.Int1Pin.pin = INT1ACC_PIN;
  Adxl355.Int1Pin.pull = PIN_PULL_UP;
  GpioSetInterrupt(&Adxl355.Int1Pin, IRQ_RISING_FALLING_EDGE, IRQ_RISING_FALLING_EDGE, &vAdxlInt1Handler);

  /* Int2 Pin Gpio Init */
  Adxl355.Int2Pin.pin = INT2ACC_PIN;
  Adxl355.Int2Pin.pull = PIN_PULL_UP;
  GpioSetInterrupt(&Adxl355.Int2Pin, IRQ_RISING_FALLING_EDGE, IRQ_RISING_FALLING_EDGE, &vAdxlInt2Handler);

  /* DRDY Pin Gpio Init */
 /* Adxl355.DRDYPin.pin = DATARDYACC_PIN;
  Adxl355.DRDYPin.pull = PIN_PULL_UP;
  GpioSetInterrupt(&Adxl355.DRDYPin, IRQ_RISING_FALLING_EDGE, IRQ_RISING_FALLING_EDGE, &DRDYHandler); */
}

/**
   @brief Turns on accelerometer measurement mode.
   @return none
**/
void vAdxlAccelerometerStart(void) {
  uint8_t ui8temp;

  SPIRead(POWER_CTL, SPI_READ_ONE_REG); /* Read POWER_CTL register, before modifying it */

  ui8temp = 0x00; // (uint8_t)SPIRxBuf[0] & 0xFE;                            /* Modify POWER_CTL register */

  SPIWrite(POWER_CTL, 0x00, 0x00, SPI_WRITE_ONE_REG); /* Set measurement bit in POWER_CTL register */

  SPIRead(POWER_CTL, SPI_READ_ONE_REG); /* Read POWER_CTL register, before modifying it */

  SPIRead(POWER_CTL, SPI_READ_ONE_REG); /* Read POWER_CTL register, before modifying it */
}

/**
   @brief Puts the accelerometer into standby mode.
   @return none
**/
void vAdxlAccelerometerStop(void) {
  uint8_t ui8temp;

  SPIRead(POWER_CTL, SPI_READ_ONE_REG); /*Read POWER_CTL register, before modifying it */

  ui8temp = SPIRxBuf[0] | 0x01; /* Clear measurement bit in POWER_CTL register */

  SPIWrite(POWER_CTL, ui8temp, 0x00, SPI_WRITE_ONE_REG); /* Write the new value to POWER_CTL register */
}

/**
   @brief This register contains the Analog Devices ID, 0xAD
   @return none
**/
void vAdxlGetDeviceID(void) {
  SPIRead(DEVID_AD, SPI_READ_ONE_REG);
}

/**
   @brief This register contains the Analog Devices MEMS ID, 0x1D. 
   @return none
**/
void vAdxlGetDeviceMemsID(void) {
  SPIRead(DEVID_MST, SPI_READ_ONE_REG);
}

/**
   @brief This register contains the device ID, 0xED.
   @return none
**/
void vAdxlGetID(void) {
  SPIRead(PARTID, SPI_READ_ONE_REG);
}

/**
   @brief This register contains the product revision ID, beginning with 0x00 and incrementing for each subsequent revision.
   @return none
**/
void vAdxlGetProcductVersionID(void) {
  SPIRead(REVID, SPI_READ_ONE_REG);
}

/**
   @brief This register includes bits that describe the various conditions of the ADXL355. 
   [7:5] Reserved Reserved. 0x0 R
   4 NVM_BUSY NVM controller is busy with either refresh, programming, or built-in, self test (BIST). 0x0 R
   3 Activity Activity, as defined in the THRESH_ACT and COUNT_ACT registers, is detected. 0x0 R
   2 FIFO_OVR FIFO has overrun, and the oldest data is lost. 0x0 R
   1 FIFO_FULL FIFO watermark is reached. 0x0 R
   0 DATA_RDY A complete x-axis, y-axis, and z-axis measurement was made and results can be read. 0x0 R

   @return none
**/
void vAdxlGetStatus(void) {
  SPIRead(STATUS, SPI_READ_ONE_REG);
}

/**
   @brief This register indicates the number of valid data samples present in the FIFO buffer. This number ranges from 0 to 96
   7 Reserved Reserved 0x0 R
   [6:0] FIFO_ENTRIES Number of data samples stored in the FIFO 0x0 R

   @return none
**/
uint8_t uxAdxlGetFIFOEntriesRegister(void) {
  SPIRead(FIFO_ENTRIES, SPI_READ_ONE_REG);
  return SPIRxBuf[0];
}

/**
   @brief  These two registers contain the uncalibrated temperature data. The nominal intercept is 1852 LSB at 25Â°C and the nominal slope is
   -9.05 LSB/Â°C. TEMP2 contains the four most significant bits, and TEMP1 contains the eight least significant bits of the 12-bit value.
   [7:0] Temperature, Bits[7:0] Uncalibrated temperature data 0x0 R 
   @return none
**/
void vAdxlGetTemperatureData(void) {
  SPIRead(TEMP2, SPI_READ_TWO_REG);
}

/**
   @brief Read this register to access data stored in the FIFO. 
   [7:0] FIFO_DATA FIFO data is formatted to 24 bits, 3 bytes, most significant byte first. A read to this
   address pops an effective three equal byte words of axis data from the FIFO. Two
   subsequent reads or a multibyte read completes the transaction of this data onto the
   interface. Continued reading or a sustained multibyte read of this field continues to
   pop the FIFO every third byte. Multibyte reads to this address do not increment the
   address pointer. If this address is read due to an autoincrement from the previous
   address, it does not pop the FIFO. Instead, it returns zeros and increments on to the
   next address 0x0 R
   @return none
**/
void vAdxlGetFIFOAccess(void) {
  //SPIRead(FIFO_DATA, SPI_READ_THREE_REG);
  SPIReadFIFO();  
				   
  vInitFIfotoEeprom();
   
}

/**
   @brief 
   [7:0] OFFSET_X,
   Bits[15:8]  Offset added to x-axis data after all other signal processing. Data is in twos complement
   format. The significance of OFFSET_X[15:0] matches the significance of XDATA[19:4].  0x0 R/W 
   @return none
**/
void vAdxlXOffsetTrim(uint8_t offsetTrimData, uint8_t type) {

  switch (type) {
  case SPI_WRITE_TYPE:
    SPIWrite(OFFSET_X_H, offsetTrimData, 0x00, SPI_WRITE_TWO_REG);
    break;

  case SPI_READ_TYPE:
    SPIRead(OFFSET_X_H, SPI_READ_TWO_REG);
    break;

  default:
    break;
  }
}

/**
   @brief 
   [7:0] OFFSET_Y,
   Bits[15:8]  Offset added to x-axis data after all other signal processing. Data is in twos complement
   format. The significance of OFFSET_Y[15:0] matches the significance of YDATA[19:4].  0x0 R/W 
   @return none
**/
void vAdxlYOffsetTrim(uint8_t offsetTrimData, uint8_t type) {
  switch (type) {
  case SPI_WRITE_TYPE:
    SPIWrite(OFFSET_Y_H, offsetTrimData, 0x00, SPI_WRITE_TWO_REG);
    break;

  case SPI_READ_TYPE:
    SPIRead(OFFSET_Y_H, SPI_READ_TWO_REG);
    break;

  default:
    break;
  }
}

/**
   @brief 
   [7:0] OFFSET_Z,
   Bits[15:8]  Offset added to x-axis data after all other signal processing. Data is in twos complement
   format. The significance of OFFSET_Z[15:0] matches the significance of ZDATA[19:4].  0x0 R/W 
   @return none
**/
void vAdxlZOffsetTrim(uint8_t offsetTrimData, uint8_t type) {
  switch (type) {
  case SPI_WRITE_TYPE:
    SPIWrite(OFFSET_Z_H, offsetTrimData, 0x00, SPI_WRITE_TWO_REG);
    break;

  case SPI_READ_TYPE:
    SPIRead(OFFSET_Z_H, SPI_READ_TWO_REG);
    break;

  default:
    break;
  }
}

/**
   @brief 
   [7:3] Reserved Reserved. 0x0 R
   ACT_Z Z-axis data is a component of the activity detection algorithm. 0x0 R/W
   1 ACT_Y Y-axis data is a component of the activity detection algorithm. 0x0 R/W
   0 ACT_X X-axis data is a component of the activity detection algorithm. 0x0 R/W 
   @return none
**/
void vAdxlActivityEnable(uint8_t data, uint8_t type) {

  switch (type) {
  case SPI_WRITE_TYPE:
    SPIWrite(ACT_EN, data, 0x00, SPI_WRITE_ONE_REG);
    break;

  case SPI_READ_TYPE:
    SPIRead(ACT_EN, SPI_READ_ONE_REG);
    break;

  default:
    break;
  }
}

/**
   @brief 
   [7:0] ACT_THRESH[15:8] Threshold for activity detection. Acceleration magnitude must be above
   ACT_THRESH to trigger the activity counter. ACT_THRESH is an unsigned
   magnitude. The significance of ACT_TRESH[15:0] matches the significance of
   XDATA, YDATA, and ZDATA[18:3].  0x0 R/W 
   @return none
**/
void vAdxlActivityTreshold(uint8_t data, uint8_t type) {

  switch (type) {
  case SPI_WRITE_TYPE:
    SPIWrite(ACT_THRESH_H, data, 0x00, SPI_WRITE_TWO_REG);
    break;

  case SPI_READ_TYPE:
    SPIRead(ACT_THRESH_H, SPI_READ_TWO_REG);
    break;

  default:
    break;
  }
}

/**
   @brief 
   [7:0] ACT_COUNT Number of consecutive events above threshold required to detect activity 0x1 R/W 
   @return none
**/
void vAdxlActivityCount(uint8_t data, uint8_t type) {
  switch (type) {
  case SPI_WRITE_TYPE:
    SPIWrite(ACT_COUNT, data, 0x00, SPI_WRITE_ONE_REG);
    break;

  case SPI_READ_TYPE:
    SPIRead(ACT_COUNT, SPI_READ_ONE_REG);
    break;

  default:
    break;
  }
}

/**
   @brief  Use this register to specify parameters for the internal high-pass and low-pass filters. 
   [6:4] HPF_CORNER -3 dB filter corner for the first-order, high-pass filter relative to the ODR 0x0 R/W
   000 Not applicable, no high-pass filter enabled
   001 247 Ã? 10-3 Ã? ODR
   010 62.084 Ã? 10-3 Ã? ODR
   011 15.545 Ã? 10-3 Ã? ODR
   100 3.862 Ã? 10-3 Ã? ODR
   101 0.954 Ã? 10-3 Ã? ODR
   110 0.238 Ã? 10-3 Ã? ODR
   [3:0] ODR_LPF ODR and low-pass filter corner 0x0 R/W
   0000 4000 Hz and 1000 Hz
   0001 2000 Hz and 500 Hz
   0010 1000 Hz and 250 Hz
   0011 500 Hz and 125 Hz
   0100 250 Hz and 62.5 Hz
   0101 125 Hz and 31.25 Hz
   0110 62.5 Hz and 15.625 Hz
   0111 31.25 Hz and 7.813 Hz
   1000 15.625 Hz and 3.906 Hz
   1001 7.813 Hz and 1.953 Hz
   1010 3.906 Hz and 0.977 Hz 
   @return none
**/
void vAdxlFilterSetting(uint8_t data, uint8_t type) {
  switch (type) {
  case SPI_WRITE_TYPE:
    SPIWrite(FILTER, data, 0x00, SPI_WRITE_ONE_REG);
    break;

  case SPI_READ_TYPE:
    SPIRead(FILTER, SPI_READ_ONE_REG);
    break;

  default:
    break;
  }
}

/**
   @brief The INT_MAP register configures the interrupt pins. Bits[7:0] select which function(s) generate an interrupt on the INT1 and INT2 pins.
   Multiple events can be configured. If the corresponding bit is set to 1, the function generates an interrupt on the interrupt pins. 
   7 ACT_EN2 Activity interrupt enable on INT2 0x0 R/W
   6 OVR_EN2 FIFO_OVR interrupt enable on INT2 0x0 R/W
   5 FULL_EN2 FIFO_FULL interrupt enable on INT2 0x0 R/W
   4 RDY_EN2 DATA_RDY interrupt enable on INT2 0x0 R/W
   3 ACT_EN1 Activity interrupt enable on INT1 0x0 R/W
   2 OVR_EN1 FIFO_OVR interrupt enable on INT1 0x0 R/W
   1 FULL_EN1 FIFO_FULL interrupt enable on INT1 0x0 R/W
   0 RDY_EN1 DATA_RDY interrupt enable on INT1 0x0 R/W  
   @return none
**/
void vAdxlInterruptEnableMap(uint8_t data, uint8_t type) /* test edildi */
{
  switch (type) {
  case SPI_WRITE_TYPE:
    SPIWrite(INT_MAP, data, 0x00, SPI_WRITE_ONE_REG);
    break;

  case SPI_READ_TYPE:
    SPIRead(INT_MAP, SPI_READ_ONE_REG);
    break;

  default:
    break;
  }
}

/**
   @brief Use this register to control the external timing triggers.
   [7:3] Reserved Reserved. 0x0 R
   2 EXT_CLK Enable external clock. 0x0 R/W
   [1:0] EXT_SYNC Enable external sync control. 0x0 R/W
          00 Internal sync.
          01 External sync, no interpolation filter. After synchronization, and for EXT_SYNC within
             specification, DATA_RDY occurs on EXT_SYNC.
          10 External sync, interpolation filter, next available data indicated by DATA_RDY 14 to
             8204 oscillator cycles later (longer delay for higher ODR_LPF setting), data represents
             a sample point group delay earlier in time.
          11 Reserved. 
   @return none
**/
void vAdxlDataSynchronization(uint8_t data, uint8_t type) /* test edildi */
{
  switch (type) {
  case SPI_WRITE_TYPE:
    SPIWrite(SYNC, data, 0x00, SPI_WRITE_ONE_REG);
    break;

  case SPI_READ_TYPE:
    SPIRead(SYNC, SPI_READ_ONE_REG);
    break;

  default:
    break;
  }
}

/**
   @brief 
   [7:3] Reserved Reserved. 0x0 R
   2 DRDY_OFF Set to 1 to force the DRDY output to 0 in modes where it is normally signal data ready. 0x0 R/W
   1 TEMP_OFF Set to 1 to disable temperature processing. Temperature processing is also disabled
   when STANDBY = 1.  0x0 R/W
   0 STANDBY Standby or measurement mode. 0x1 R/W
     1 Standby mode. In standby mode, the device is in a low power state, and the
       temperature and acceleration datapaths are not operating. In addition, digital
       functions, including FIFO pointers, reset. Changes to the configuration setting of the
       device must be made when STANDBY = 1. An exception is a high-pass filter that can
       be changed when the device is operating.
     0 Measurement mode. 
   @return none
**/
void vAdxlPowerControl(uint8_t data, uint8_t type) {
  switch (type) {
  case SPI_WRITE_TYPE:
    SPIWrite(POWER_CTL, data, 0x00, SPI_WRITE_ONE_REG);
    break;

  case SPI_READ_TYPE:
    SPIRead(POWER_CTL, SPI_READ_ONE_REG);
    break;

  default:
    break;
  }
}

/**
   @brief Refer to the Self Test section for more information on the operation of the self test feature. 
   [7:2] Reserved Reserved. 0x0 R
   1 ST2 Set to 1 to enable self test force 0x0 R/W
   0 ST1 Set to 1 to enable self test mode 0x0 R/W 
   @return none
**/
void vAdxlSelfTest(uint8_t data, uint8_t type) {
  switch (type) {
  case SPI_WRITE_TYPE:
    SPIWrite(SELF_TEST, data, 0x00, SPI_WRITE_ONE_REG);
    break;

  case SPI_READ_TYPE:
    SPIRead(SELF_TEST, SPI_READ_ONE_REG);
    break;

  default:
    break;
  }
}

/**
   @brief Refer to the Self Test section for more information on the operation of the self test feature. 
   [7:0] Reset Write Code 0x52 to resets the device, similar to a power-on reset (POR) 0x0 W 
   @return none
**/
void vAdxlReset(void) {
  uint8_t data = 0x52;
  SPIWrite(RESET, data, 0x00, SPI_WRITE_ONE_REG);
}

/**
   @brief Reads the accelerometer data.
   @return none
**/
void vAdxlSensorDataScan(void) {

  SPIRead(XDATA3, SPI_READ_THREE_REG);
  SPIRead(YDATA3, SPI_READ_THREE_REG);
  SPIRead(ZDATA3, SPI_READ_THREE_REG);
  SPIRead(TEMP2, SPI_READ_TWO_REG);

  i32SensorX = xAdxlAccelerationDataConversion();
  i32SensorY = xAdxlAccelerationDataConversion();
  i32SensorZ = xAdxlAccelerationDataConversion();
}

/**
   @brief Convert the two's complement data in X,Y,Z registers to signed integers
   @param ui32SensorData - raw data from register
   @return int32_t - signed integer data
**/
int32_t xAdxlAccelerationDataConversion(void) {
  int32_t volatile i32Conversion = 0, ui32SensorData = 0;

  ui32SensorData = ui32SensorData | SPIRxBuf[0];
  ui32SensorData = (ui32SensorData << 8) | SPIRxBuf[1];
  ui32SensorData = (ui32SensorData << 8) | SPIRxBuf[2];

  ui32SensorData = (ui32SensorData >> 4);
  ui32SensorData = (ui32SensorData & 0x000FFFFF);

  if ((ui32SensorData & 0x00080000) == 0x00080000) {

    i32Conversion = (ui32SensorData | 0xFFF00000);

  } else {
    i32Conversion = ui32SensorData;
  }

  return i32Conversion;
}

/**
   @brief Write Fifo Register Convert the two's complement data in X,Y,Z registers to signed integers
   @param void
   @return void
**/

void vAdxlWriteFIFORegisters(void) {

 
}
