/*
*  Lora Tx and Rx Functions
*
*
*/
#include "loraRam.h"
#include "adxlRam.h"
#include "ADXL355.h"
#include "shmSignal.h"
#include "flash.h"
#include "queue.h"
#include "pca10040\s132\ses\RTC_RT.h"
#include "pca10040/s132/ses/device_managment.h"
#include "pca10040/s132/ses/loraFunctions.h"
#include "pca10040\s132\ses\loraMessages.h"
#include "pca10040/s132/ses/loraFunctions.h"
/*
*  Function   : createTxBuffQueue() 
*  Description: Creates a queue to keep transmit data buffer
*/
bool createTxBuffQueue(){
  txDataQueue = xQueueCreate(100, sizeof(buffInfo_t));
  //txDataQueue = xQueueCreate(TX_BUFF_SIZE, sizeof(buffInfo_t));
  //queueExp = xQueueCreate(TX_BUFF_SIZE, sizeof(buffInfo_t));
  if(txDataQueue == NULL){
    return false;
  }
  return true;
}
/*
*  Function   : sendLoraDataMessage() 
*  Description: this function pushes the data to transmit queue.  
*/
void sendLoraDataMessage(uint8_t *data, uint8_t dataSize){
  buffInfo_t buffInfo;
  uint32_t size;
  if(dataSize > LORAWAN_DATA_MAX_SIZE){
    dataSize = LORAWAN_DATA_MAX_SIZE;
  }
  buffInfo.buffAddrPtr = malloc(dataSize);
  if(buffInfo.buffAddrPtr != NULL){
    memcpy(buffInfo.buffAddrPtr, data, dataSize);
    buffInfo.buffSize = dataSize ;
    
    if(xQueueSend(txDataQueue, &buffInfo, 3) == errQUEUE_FULL){
    // log tutulacak. ne yapilacak dusunulmeli
      return;
    }
     printf("timer func is running\n");
     
  }
}
//
//void exampleData(){
//
// uint32_t size = uxQueueMessagesWaiting(queueExp);
// printf("queue size is %d\n", size);
//}
/*
*  Function   : LoraTxData() 
*  Description: this function pull the data from transmit queue to send loraWan.  
*/
void LoraTxData(AppData_t *AppData, uint8_t *IsTxConfirmed){
  buffInfo_t buffInfo;
  
  uint32_t size = uxQueueMessagesWaiting(txDataQueue);
    
  if(size == 0)
  {
    AppData-> Buff = NULL;
    return; //queue is empty
  }
  if(xQueueReceive(txDataQueue, &buffInfo, 3) != pdTRUE){
  // log tutulacak. ne yapilacak dusunulmeli
    return;
  }
  
  *IsTxConfirmed = lora_config.IsTxConfirmed;
  AppData->Port = lora_config.AppPort;
  AppData->Buff = buffInfo.buffAddrPtr;
  AppData->BuffSize = buffInfo.buffSize;
  
}


 float32_t receivedDataX = 0.0001;
 float32_t receivedDataY = 0.0001;

/*
*  Function   : receiveLoraMessage() 
*  Description:  
*/

    
void receiveLoraMessage(uint8_t *buffPtr, uint8_t buffSize){
  float32_t receivedData = 0.0001;
  uint8_t dataBuffer[8] ;
  uint8_t messageType;
  messageType = buffPtr[0]; /* get message type */
  switch (messageType) {

    
    case SET_THRESHOLD: 
            printf("threshold deðeri güncellendi");
           // receivedDataX = (float32_t)(((buffPtr[1] >> 0) & 0x000000FF) | ((buffPtr[2] >> 0) && 0x0000FFFF)| ((buffPtr[3] >> 0) & 0x00FFFFFF )| ((buffPtr[4]) & 0xFFFFFFFF));
            receivedDataX = (float32_t)*((float32_t *) &buffPtr[1]);
           // receivedDataY = (float32_t)((buffPtr[5] >> 24) | (buffPtr[6] >> 16) | (buffPtr[7] >> 8) | (buffPtr[8]));
            receivedDataY =  receivedDataX;

            deviceManager.axis_condition[0].threshold = receivedDataX;
            deviceManager.axis_condition[1].threshold = receivedDataX;
            memcpy(&X_axisTreshold, (void *)&receivedDataX, sizeof(float32_t)); 
            memcpy(&Y_axisTreshold, (void *)&receivedDataX, sizeof(float32_t));

            //(float32_t)((buffPtr[5]) | (buffPtr[6] >> 8) | (buffPtr[7] >> 16) | (buffPtr[8] >> 24));
            // First four bytes for x-axis threshold 
            //receivedDataX = *((float32_t*)(buffPtr + 1)); // Convert bytes to float

            // Next four bytes for y-axis threshold
            //receivedDataY = *((float32_t*)(buffPtr + 5)); // Convert bytes to float
            
            // Call updateThreshold function for X and Y axis
           // updateThreshold(receivedDataX, AXIS_X);
            //updateThreshold(receivedDataY, AXIS_Y);
            break;

// GET_SHOW_THRESHOLD returns the last threshold value
    case GET_SHOW_THRESHOLD:
        Get_Show_Threshold();
    
      break;


// Restarts CALIBRATION from the beginning
    case KALIBRASYON:
      printf("bu kisim kalibrasyon icin ayarlanmis görünmesi gerekiyor.");
      deviceManagerInit((uint8_t)0); 

      break;

// Restarts CALIBRATION from the beginning
    case RESET_MANUAL:
      printf("bu kisim reset için ayarlanmistir cihazin exit edildigi görünmesi gerekir");
      exit(0);
      break;

//
    case SET_FREQUENCY:
  
      receivedData = (buffPtr[1] << 24) | (buffPtr[2] << 16) | (buffPtr[3] << 8) | buffPtr[4] ; 
      memcpy(&numOfSamplePoints, &receivedData, sizeof(float32_t)); /* set new frequency */
      if(numOfSamplePoints > 1024)
      {
        numOfSamplePoints = FREQUENCYINITVALUE;
      }
      vFlashWritetoFlash(receivedData, FREQUENCYADDRESS, FREQUENCYPAGE);  
     
      break;


    case SET_TIME:
    
          for (int i = 0; i < buffSize; i++) {
             printf("%02X ", buffPtr[i]);
          }
          printf("\n");

          uint32_t hexToDecimal(uint32_t hex) {
              uint32_t decimal = hex;
              if (hex >= 'A' && hex <= 'F') {
                  decimal = hex - 'A' + 10;
              } else if (hex >= 'a' && hex <= 'f') {
                  decimal = hex - 'a' + 10;
              } else if (hex >= '0' && hex <= '9') {
                  decimal = hex - '0';
              }
              return decimal;
          }

          uint32_t ext_hours = buffPtr[1];
          uint32_t ext_minutes = buffPtr[2];
          uint32_t ext_seconds = buffPtr[3];
          update_rtc_from_external_data(ext_hours, ext_minutes, ext_seconds);

          printf("time data: %02d:%02d:%02d\n", ext_hours, ext_minutes, ext_seconds);

         break;
  }

}



void setDeviceState(deviceStateEnum_t state) 
{
  xSemaphoreTake(xDeviceState, 0);  
  DeviceState = state;
  xSemaphoreGive(xDeviceState);
}

deviceStateEnum_t getDeviceState() 
{
  deviceStateEnum_t tempState;  
  xSemaphoreTake(xDeviceState, 0);
  tempState = DeviceState;
  xSemaphoreGive(xDeviceState);
  return tempState;
}
//nozcan
void setNextTx(uint8_t nextTx) 
{
  xSemaphoreTake(xNextTx, 0);  
  NextTx = nextTx;
  xSemaphoreGive(xNextTx);
}
//nozcan
int getNextTx(void) 
{
  uint8_t tempTxState;  
  xSemaphoreTake(xNextTx, 0);
  tempTxState = NextTx;
  xSemaphoreGive(xDeviceState);
  return tempTxState;
}
void startSendLoraPeriodicDataTask(){
  xTimerStart(sendPeriodicDataTimer,10);
}
void stopSendLoraPeriodicDataTask(){
  xTimerStop(sendPeriodicDataTimer,0);
} 