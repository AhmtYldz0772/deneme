#ifndef _ADXL_RAM_H
#define _ADXL_RAM_H

#include "sdk_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "arm_math.h"


#define ACC_DATA_QUEUE_LEN      250 // 2000ms/8ms = 250 count
#define AXIS_DATA_SIZE          3
#define X_AXIS_MASK             0x01
#define EMPTY_DATA_MASK         0x03
#define OTHER_AXIS_MASK         0x00

#define READ_SENSOR_DATA_PERIOD   4         //ms
#define DATA_RDY 0x01
#define MAX_FIFO_SIZE 96

#define TEN_SEC_DATA_COUNT  1250  // 10000ms/8ms = 1250 count
#define X_Y_AXIS_NUM        2
#define X_Y_Z_AXIS_NUM      3
#define AXIS_X              0
#define AXIS_Y              1
#define AXIS_Z              2
#define MAX_VALUE_COUNT     5

typedef struct accelerometerData{
  
  uint8_t xAxisData[AXIS_DATA_SIZE];
  uint8_t yAxisData[AXIS_DATA_SIZE];
  //uint8_t zAxisData[AXIS_DATA_SIZE];
}accelerometerData_t;

typedef struct receivedMaxData{
  
  float32_t value;
  uint16_t  index;
}receivedMaxData_t;

extern  TimerHandle_t readSensorDataTimer;

extern TaskHandle_t xCollectSensorDataTask;

extern  xQueueHandle  xAccDataQueue;

#endif