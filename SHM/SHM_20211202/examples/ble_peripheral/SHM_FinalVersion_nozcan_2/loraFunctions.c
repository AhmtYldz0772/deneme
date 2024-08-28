/*
*  Lora Tx and Rx Functions
*
*
*/

#include "loraRam.h"
#include "adxlRam.h"
#include "ADXL355.h"
#include "shmSignal.h"


/*
*  Function   : createTxBuffQueue() 
*  Description: Creates a queue to keep transmit data buffer
*/
bool createTxBuffQueue(){
  txDataQueue = xQueueCreate(TX_BUFF_SIZE, sizeof(buffInfo_t));

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

  if(dataSize > LORAWAN_DATA_MAX_SIZE){
    dataSize = LORAWAN_DATA_MAX_SIZE;
  }

  buffInfo.buffAddrPtr = malloc(dataSize);

  if(buffInfo.buffAddrPtr != NULL){
    memcpy(buffInfo.buffAddrPtr, data, dataSize);
    buffInfo.buffSize = dataSize;

    if(xQueueSend(txDataQueue, &buffInfo, portMAX_DELAY) == errQUEUE_FULL){
    // log tutulacak. ne yapilacak dusunulmeli
      return;
    }
  }
}

/*
*  Function   : LoraTxData() 
*  Description: this function pull the data from transmit queue to send loraWan.  
*/
void LoraTxData(AppData_t *AppData, uint8_t *IsTxConfirmed){
  buffInfo_t buffInfo;
  
  if(xQueueReceive(txDataQueue, &buffInfo, portMAX_DELAY) != pdTRUE){
    // log tutulacak. ne yapilacak dusunulmeli
      return;
    }

  IsTxConfirmed = lora_config.IsTxConfirmed;
  AppData->Port = lora_config.AppPort;
  AppData->Buff = buffInfo.buffAddrPtr;
  AppData->BuffSize = buffInfo.buffSize;

}