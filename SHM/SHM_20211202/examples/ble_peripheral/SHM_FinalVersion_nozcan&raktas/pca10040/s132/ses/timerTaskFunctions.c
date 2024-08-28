#include "ADXL355.h"
#include "sdk_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "adxlRam.h"
#include "adxlFunctions.h"
#include "loraRam.h"
#include "device_managment.h"
#include "RTC_RT.h"
#include "spi-board.h"
#include "timerTaskFunction.h"


 uint32_t start = 0, end, duration, time = 0, totFifoCount = 0, rdyCnt = 0, errCnt = 0, mode = 0, fatErrCnt = 0, fat2ErrCnt = 0, fifoFullErrCnt = 0, fifoOvrloadErrCnt = 0, runTimeErrCnt = 0, queuefull = 0, queueCnt = 0;
 uint8_t fifoSize;
  
uint32_t taskStart = 0, taskEnd, taskDuration;
uint8_t status = 0;
uint8_t idx = 0;
uint8_t f1 = 0;
uint8_t f2 = 0;
uint8_t f3 = 0;
uint8_t fifoBuff[MAX_FIFO_SIZE] = {0};
uint32_t ten_seconds_counter = 0;
uint32_t one_second_counter = 0;

void readSensorDataTimerTask(){

  ten_seconds_counter++;
  one_second_counter++;

  if((one_second_counter * READ_SENSOR_DATA_PERIOD) >= 1000){

    //deviceManager.device_exec_counter_second++;
    one_second_counter = 0;
  }


  if(ten_seconds_counter * READ_SENSOR_DATA_PERIOD >= 10000) //send message every 10 secons
  {
      send_device_info_message();
      ten_seconds_counter = 0;
  }


 
 
  update_system_time();//metehanpataci

  if(start == 0){
    start = xTaskGetTickCountFromISR();
      SPIWrite(POWER_CTL, 0x00, 0x00, SPI_WRITE_ONE_REG);
  }

  taskStart = xTaskGetTickCountFromISR();

  //uint8_t 
  //fifoSize = getSensorFifoEntries();
  //uint8_t status = !DATA_RDY;
  uint8_t index = 0, axisIndex = 0;
  uint8_t lsbAxisIndex = 2; // lsb comes on last index of axis
  accelerometerData_t accData = {0};

  //if(totFifoCount < 11250){
   //if(duration < 30000){ 
    status = getSensorStatus();
    if ((status & DATA_RDY) == DATA_RDY) {
      
      fifoSize = getSensorFifoEntries();

     
      
      
      
//      mode = fifoSize % 3;
//      
//      if(fifoSize >= 3){
//          xSPIRead(FIFO_DATA,&fifoBuff[idx],((fifoSize - mode)));
//          errCnt++;
//          idx += ((fifoSize - mode)*3);
//          
//          if(f3 == 0 && f2 != 0 && f1 != 0)
//            f3 = fifoSize;
//          
//          if(f2 == 0 && f1 != 0)
//            f2 = fifoSize;
//
//
//          if(f1 == 0)
//            f1 = fifoSize;
//
//          if(errCnt < 3) return;
//
//           mode = fifoSize % 9;
//      }


      mode = fifoSize % 3;
      
      if(fifoSize < 3){
        fatErrCnt++;
      
      }

      if(fifoSize > 5){
        fat2ErrCnt++;
      }


      if((status & 0x02) == 0x02){
        fifoFullErrCnt++;
      }

      if((status & 0x04) == 0x04){
        fifoOvrloadErrCnt++;
      }

      if(fifoSize % 3 > 0){
        errCnt++;
      }

    if(fifoSize > (MAX_FIFO_SIZE/3)){
      fifoSize = (MAX_FIFO_SIZE/3);
    }

      xSPIRead(FIFO_DATA,fifoBuff,((fifoSize - mode)*3));
      totFifoCount += (fifoSize-mode);

      //data for x,y & z axis is written to fifo for every 8 ms.
      // fifo size is expected to be multiples of 3 because of X,Y & Z axis.
      // every three bytes is One element in FIFO.
      for(index = 0; index < fifoSize/3; index++){
        
        if(((fifoBuff[lsbAxisIndex] & EMPTY_DATA_MASK) == X_AXIS_MASK) && 
          (((fifoBuff[lsbAxisIndex+AXIS_DATA_SIZE] | fifoBuff[lsbAxisIndex+(AXIS_DATA_SIZE<<1)]) & X_AXIS_MASK) == OTHER_AXIS_MASK)){
            
            //raktas
            memcpy(accData.xAxisData, &fifoBuff[axisIndex], AXIS_DATA_SIZE);
            memcpy(accData.yAxisData, &fifoBuff[axisIndex+AXIS_DATA_SIZE], AXIS_DATA_SIZE);
            //memcpy(accData.xAxisData, &fifoBuff[axisIndex+(AXIS_DATA_SIZE<<1)], AXIS_DATA_SIZE); // z axis
            
            //if(xQueueSend(xAccDataQueue, &accData, portMAX_DELAY) == errQUEUE_FULL){
            if(xQueueSend(xAccDataQueue, &accData, 1) == errQUEUE_FULL){
            // log tutulacak. ne yapilacak dusunulmeli
              queuefull++;
              break;
            }
             queueCnt++;
           
          }

        lsbAxisIndex += 9;
        axisIndex += 9;
      }

      rdyCnt++;
    }
    time++;
    end = xTaskGetTickCountFromISR();
    duration = end - start;
    taskDuration = end - taskStart;

    if(taskDuration > READ_SENSOR_DATA_PERIOD){
        runTimeErrCnt++;
    }

 // }




//    status = getSensorStatus();
//    if ((status & DATA_RDY) == DATA_RDY) {
//      
//      xSPIRead(FIFO_DATA,fifoBuff,fifoSize);
//      
//    }


//  memcpy(fifoBuff, 0x00, MAX_FIFO_SIZE); /* read treshold */
//
//  if(fifoSize % 3 != 0){
//    //log tutulacak
//    return;
//  }
//  
//  if(fifoSize > 0){
//    status = getSensorStatus();
//    if ((status & DATA_RDY) == DATA_RDY) {
//      
//      xSPIRead(FIFO_DATA,fifoBuff,fifoSize);
//
//      //data for x,y & z axis is written to fifo for every 8 ms.
//      // fifo size is expected to be multiples of 3 because of X,Y & Z axis.
//      // every three bytes is One element in FIFO.
//      for(index = 0; index < fifoSize/3; index++){
//        
//        if(((fifoBuff[lsbAxisIndex] & EMPTY_DATA_MASK) == X_AXIS_MASK) && 
//          (((fifoBuff[lsbAxisIndex+AXIS_DATA_SIZE] | fifoBuff[lsbAxisIndex+(AXIS_DATA_SIZE<<1)]) & X_AXIS_MASK) == OTHER_AXIS_MASK)){
//
//            memcpy(accData.xAxisData, &fifoBuff[axisIndex], AXIS_DATA_SIZE);
//            memcpy(accData.yAxisData, &fifoBuff[axisIndex+AXIS_DATA_SIZE], AXIS_DATA_SIZE);
//            
//            if(xQueueSend(xAccDataQueue, &accData, portMAX_DELAY) == errQUEUE_FULL){
//            // log tutulacak. ne yapilacak dusunulmeli
//              return;
//            }
//          }
//
//        lsbAxisIndex += 9;
//        axisIndex += 9;
//      }
//
//    }
//  }

//  uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
//
//  printf("Time func. Kalan Bellek: %d\n",uxHighWaterMark);

//    end = xTaskGetTickCountFromISR();
//    if(duration < (end - start)){
//    
//      duration = end - start;
//    }
}



void createReadSensorDataTask(){

   readSensorDataTimer = xTimerCreate
                       ( /* Just a text name, not used by the RTOS
                         kernel. */
                         "ReadSensorData",
                         /* The timer period in ticks, must be
                         greater than 0. */
                         READ_SENSOR_DATA_PERIOD,  //8 ms
                         /* The timers will auto-reload themselves
                         when they expire. */
                         pdTRUE,
                         /* The ID is used to store a count of the
                         number of times the timer has expired, which
                         is initialised to 0. */
                         ( void * ) 0,
                         /* Each timer calls the same callback when
                         it expires. */
                         readSensorDataTimerTask

                       );
}



/*------------- LORA PERIODIC MESSAGE TASK FUNCTIONS -------------*/

//UBaseType_t uxHighWaterMarkTimer;

void sendLoraPeriodicDataTask(){

printf("timer func tick count = %d\n",xTaskGetTickCount());
  
//  uxHighWaterMarkTimer = uxTaskGetStackHighWaterMark(NULL);
  prepareMaxValMessage();
//  uxHighWaterMarkTimer = uxTaskGetStackHighWaterMark(NULL);

}


void createSendLoraPeriodicDataTask(){

   sendPeriodicDataTimer = xTimerCreate
                         ( /* Just a text name, not used by the RTOS
                           kernel. */
                           "SendPeriodicData",
                           /* The timer period in ticks, must be
                           greater than 0. */
                           SEND_LORA_DATA_PERIOD,  //180000 ms
                           /* The timers will auto-reload themselves
                           when they expire. */
                           pdTRUE,
                           /* The ID is used to store a count of the
                           number of times the timer has expired, which
                           is initialised to 0. */
                           ( void * ) 0,
                           /* Each timer calls the same callback when
                           it expires. */
                           sendLoraPeriodicDataTask
                         );
}