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
    buffInfo.buffSize = dataSize;

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


/*
*  Function   : receiveLoraMessage() 
*  Description:  
*/
void receiveLoraMessage(uint8_t *buffPtr, uint8_t buffSize){
  float32_t receivedData = 0;
  uint8_t messageType;

  messageType = buffPtr[0]; /* get message type */

  switch (messageType) {

    case SET_THRESHOLD: /* if the message set threshold, the case will run */

      /* firt four bytes x axis treshold */
      receivedData = (buffPtr[1] << 24) | (buffPtr[2] << 16) | (buffPtr[3] << 8) | buffPtr[4] ; 

      memcpy(&X_axisTreshold, &receivedData, sizeof(float32_t)); /* set new treshold */
      vFlashWritetoFlash(receivedData, XAXISDATAADDRESS, XAXISDATAPAGE); 
      receivedData = 0;  /* after set axis, receivedData set to 0 */

      /* second four bytes y axis treshold */
      receivedData = (buffPtr[5] << 24) | (buffPtr[6] << 16) | (buffPtr[7] << 8) | buffPtr[8] ; 


      memcpy(&Y_axisTreshold, &receivedData, sizeof(float32_t)); /* set new treshold */
      vFlashWritetoFlash(receivedData, YAXISDATAADDRESS, YAXISDATAPAGE);  

      break;

    case SET_FREQUENCY:
  
      receivedData = (buffPtr[1] << 24) | (buffPtr[2] << 16) | (buffPtr[3] << 8) | buffPtr[4] ; 
      memcpy(&numOfSamplePoints, &receivedData, sizeof(float32_t)); /* set new frequency */

      if(numOfSamplePoints > 1024)
      {
        numOfSamplePoints = FREQUENCYINITVALUE;
      }

      vFlashWritetoFlash(receivedData, FREQUENCYADDRESS, FREQUENCYPAGE);  
     
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
 
