#include "ADXL355.h"
#include "sdk_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "adxlRam.h"
#include "adxlFunctions.h"

int readSensorDataTimerTaskCounter = 0;
int readSensorDataTimerTaskErrorCounter = 0;

void readSensorDataTimerTask(){

  //startReadSensorDataTask();   //nozcan

  uint8_t fifoSize = getSensorFifoEntries();
  uint8_t status = !DATA_RDY;
  uint8_t fifoBuff[MAX_FIFO_SIZE];
  uint8_t index = 0, axisIndex = 0, lsbAxisIndex = 2; // lsb comes on last index of axis
  accelerometerData_t accData = {0};


  readSensorDataTimerTaskCounter = 1;

  memcpy(fifoBuff, 0x00, MAX_FIFO_SIZE); /* read treshold */ 

  if(fifoSize % 3 != 0){
    //log tutulacak
    return;
  }

  if(fifoSize > 0){
    status = getSensorStatus();
    if (status & DATA_RDY == DATA_RDY) {
      
      xSPIRead(FIFO_DATA,fifoBuff,fifoSize);

      //data for x,y & z axis is written to fifo for every 8 ms.
      // fifo size is expected to be multiples of 3 because of X,Y & Z axis.
      // every three bytes is One element in FIFO.
      for(index = 0; index < fifoSize/3; index++){
        
        if((fifoBuff[lsbAxisIndex] & EMPTY_DATA_MASK == X_AXIS_MASK) && 
          ((fifoBuff[lsbAxisIndex+AXIS_DATA_SIZE] | fifoBuff[lsbAxisIndex+(AXIS_DATA_SIZE<<1)]) & X_AXIS_MASK == OTHER_AXIS_MASK)){
            
            memcpy(accData.xAxisData, &fifoBuff[axisIndex], AXIS_DATA_SIZE);
            memcpy(accData.yAxisData, &fifoBuff[axisIndex+AXIS_DATA_SIZE], AXIS_DATA_SIZE);


            readSensorDataTimerTaskCounter++;
            
            if(xQueueSend(xAccDataQueue, &accData, portMAX_DELAY) == errQUEUE_FULL){
             readSensorDataTimerTaskErrorCounter++;
            // log tutulacak. ne yapilacak dusunulmeli
              return;
            }
        }

        lsbAxisIndex += 9;
        axisIndex += 9;
      }

    }
  }
}



void createReadSensorDataTask(){

   readSensorDataTimer = xTimerCreate
                       ( /* Just a text name, not used by the RTOS
                         kernel. */
                         "ReadSensorData",
                         /* The timer period in ticks, must be
                         greater than 0. */
                         READ_SENSOR_DATA_PERIOD,
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
         //if( readSensorDataTimer == NULL )
         //{
         //    /* The timer was not created. */
         //}
         //else
         //{
         //    /* Start the timer.  No block time is specified, and
         //    even if one was it would be ignored because the RTOS
         //    scheduler has not yet been started. */
         //    if( xTimerStart( readSensorDataTimer, 0 ) != pdPASS )
         //    {
         //        /* The timer could not be set into the Active
         //        state. */
         //    }
         //}
        
}