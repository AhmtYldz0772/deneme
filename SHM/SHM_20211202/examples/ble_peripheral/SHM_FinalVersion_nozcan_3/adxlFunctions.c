#include "ADXL355.h"
#include "sdk_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "adxlRam.h"
#include "loraRam.h"

uint8_t getSensorFifoEntries(){
    uint8_t fifoSize = 0;
    xSPIRead(FIFO_ENTRIES, &fifoSize, sizeof(fifoSize));

    return fifoSize;
}

uint8_t getSensorStatus(){
    uint8_t status = !DATA_RDY;
    xSPIRead(STATUS, &status, sizeof(status));

    return status;
}


void startReadSensorDataTask(){
  //vAdxlAccelerometerStart();   //Start Adxl
  //vAdxlFilterSetting(HPF_CORNER_ODR_125_Hz, SPI_WRITE_TYPE);    //Frequency Conf. Adxl
  xTimerStart(readSensorDataTimer,0);

}


void stopReadSensorDataTask(){
  xTimerStop(readSensorDataTimer,0);

}


bool createFifoBuffQueue(){
  xAccDataQueue = xQueueCreate(ACC_DATA_QUEUE_LEN, sizeof(accelerometerData_t));

  if(xAccDataQueue == NULL){
    return false;
  }

  return true;
}