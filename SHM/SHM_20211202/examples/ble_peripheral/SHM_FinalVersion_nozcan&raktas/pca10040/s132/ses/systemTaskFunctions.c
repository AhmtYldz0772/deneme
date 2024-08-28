#include "ADXL355.h"
#include "sdk_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "adxlRam.h"
#include "semphr.h"
#include "arm_math.h"
#include "shmSignal.h"
#include "loraRam.h"
#include "lora.h"
#include "board.h"
#include "nrfRam.h"
#include "flash.h"
#include "adxlFunctions.h"
#include "epochTime.h"  //nozcan
#include "BLE_Services/UART_Service.h"
#include "device_managment.h"
//#include "loraMessages.h"
#include "eeprom.h"
#include <stdlib.h> 
#include "loraFunctions.h"
#include "Initialize.h"

float32_t xThreshholdValue = 0;
float32_t yThreshholdValue = 0;

void collectedSensorDataFFT(float32_t *collectedDataPtr, uint16_t dataSize, float32_t *FftResultPtr){

  //float32_t sourceBuff[FFT_BUFFER_SIZE] = {0};
  
  if(dataSize > FFT_BUFFER_SIZE){
    dataSize = FFT_BUFFER_SIZE;
  }

 // memcpy(sourceBuff,collectedDataPtr,dataSize); /* between datasize and fifoFFTBufferSize is filled 0. Make 0 padding to complete 2048 sample point */

  arm_rfft_fast_f32(&fftIns, collectedDataPtr, FftResultPtr, 0);

}


//void updateTopThree(float32_t new_val, float32_t top_three[3]) {
    
//    float32_t min_val = top_three[0];
//    int min_index = 0;
//    for (int i = 1; i < 3; i++) {
//        if (top_three[i] < min_val) {
//            min_val = top_three[i];
//            min_index = i;
//        }
//    }

//    // Ahmet en küçük deðeri yeni deðerle deðiþtir
//    if (new_val > min_val) {
//    top_three[min_index] = new_val;
//    if (new_val > X_axisTreshold) {
//        xThreshholdValue = new_val;
//    }
    
//}
//}





bool findMaxThreeData(float32_t *collectedDataPtr, uint16_t dataSize, receivedMaxData_t *maxDataPtr){
  uint16_t index = 0;
  float32_t fftResultBuff[FFT_BUFFER_SIZE] = {0};
  float32_t *amplitudeBuffPtr = NULL;
  float32_t top_three[3] = {0, 0, 0};  

  collectedSensorDataFFT(collectedDataPtr, dataSize, fftResultBuff);

  amplitudeBuffPtr = malloc(numOfSamplePoints*sizeof(float32_t));

  if(amplitudeBuffPtr == NULL){
    //not available area in heap. log it.
    return false;
  }

  /* Divide FFT sample point to normalize amplitude. */
  for (index = 0; index < numOfSamplePoints; index++) {
    amplitudeBuffPtr[index] = ((sqrt(pow(fftResultBuff[(2*index)], 2) + pow(fftResultBuff[(2*index)+1], 2)))/dataSize);
  }

  for(index = 0; index < numOfSamplePoints; index++){
    updateTopThree(amplitudeBuffPtr[index], top_three);
  }

  for(index = 0; index < MAX_VALUE_COUNT; index++){

    arm_max_f32(&amplitudeBuffPtr[0], numOfSamplePoints, &(maxDataPtr->value), &(maxDataPtr->index));
    if(maxDataPtr->value > X_axisTreshold){
      xThreshholdValue = maxDataPtr->value;

    }

   

    amplitudeBuffPtr[maxDataPtr->index] = 0; // make 0 maks value to prevent find again as max.
    maxDataPtr++;
  }

  free(amplitudeBuffPtr);

  //for (int i = 0; i < MAX_VALUE_COUNTT; i++) {
  //  //printf("Max Deðer %d: %d: %f\n", i + 1, maxDataPtr[i].index,maxDataPtr[i].value);         //Ahmet
  //}

  return true;
}
bool  findmax(){

return "hello"; 
}


bool findMaxFiveData(float32_t *collectedDataPtr, uint16_t dataSize, receivedMaxData_t *maxDataPtr, float32_t data_threshold){

  bool alarmExist = false;
  uint16_t  index = 0;
  float32_t fftResultBuff[FFT_BUFFER_SIZE] = {0};
  float32_t *amplitudeBuffPtr = NULL;

  collectedSensorDataFFT(collectedDataPtr, dataSize, fftResultBuff);
  
  amplitudeBuffPtr = malloc(numOfSamplePoints*sizeof(float32_t));

  if(amplitudeBuffPtr == NULL){
    //not available area in heap. log it.
    return false;
  }

  //Divide FFT sample point to normalize amplitude. 
  for (index = 0; index < numOfSamplePoints; index++) {
    amplitudeBuffPtr[index] = ((sqrt(pow(fftResultBuff[(2*index)], 2) + pow(fftResultBuff[(2*index)+1], 2)))/dataSize);
    //amplitudeBuffPtr[index] = sqrt(pow(3, 2) + pow(4, 2));
  }

  //remmove noises
  memset(&amplitudeBuffPtr[0],0, sizeof(float32_t) * 10);

  for(index = 0; index < MAX_VALUE_COUNT; index++){

    arm_max_f32(&amplitudeBuffPtr[0], numOfSamplePoints, &(maxDataPtr->value), &(maxDataPtr->index));
    if(maxDataPtr->value > data_threshold){
      alarmExist = true;
    }
    amplitudeBuffPtr[maxDataPtr->index] = 0; // make 0 maks value to prevent find again as max.
    maxDataPtr++;
  }

  //for (int i = 0; i < MAX_VALUE_COUNT; i++) {
  //  printf("Max Deðer %d: %d: %f\n", i + 1, maxDataPtr[i].index,maxDataPtr[i].value);         
  //}

  free(amplitudeBuffPtr);
  return alarmExist;
 
}

/*
*
*
*
*
*
*
*/
void updateMax5Values(receivedMaxData_t *xAxisMaxData, receivedMaxData_t *yAxisMaxData)
{
  uint8_t i,j = 0;
  int8_t lastUpdIndex = -1;
  
  /* if stored Higher data is lower than received Lower data, replace all stored data with received data. raktas*/
  if(X_Max5Data[0].value <= xAxisMaxData[MAX_VALUE_COUNT-1].value)
  {
    memcpy(X_Max5Data, xAxisMaxData, (sizeof(receivedMaxData_t)*MAX_VALUE_COUNT));
  }
  else if(X_Max5Data[MAX_VALUE_COUNT-1].value < xAxisMaxData[0].value) //otherwise it is not necessary to compare
  {

    /* 
    Example; 
          '-' j index
          '+' i index
          '/' last updated index
       
       X 32 -18 10 8 5
         32 /20 -18 10 8
         32  20   18 /15 -10
         32  20   18 15  /13 -

       Z için
         32 20 18 /15 -10 

       Y 20 15 13 +7 2
       Z 20 15 +9 7 2
    
    */
    for(i = 0; i < MAX_VALUE_COUNT; i++)
    {
      for(j = lastUpdIndex+1; j < MAX_VALUE_COUNT; j++)
      {
        if(xAxisMaxData[i].value > X_Max5Data[j].value)
        {    
          //shift old data to right.
          /* 0 1 2 3 4 -> '' 0 1 2 3 */
          for(uint8_t k = (MAX_VALUE_COUNT-1); k>j; k--)
          {
            X_Max5Data[k].value = X_Max5Data[k-1].value;
            X_Max5Data[k].index = X_Max5Data[k-1].index;
          }

          X_Max5Data[j].value = xAxisMaxData[i].value;
          X_Max5Data[j].index = xAxisMaxData[i].index;
          
          lastUpdIndex = j;
          break;
        }
      }

      if(j == MAX_VALUE_COUNT)
      {
        /* if i. value lower than all X Max5Data, then i+1. value is already lower, dont check for other i values.*/
        break;
      }
    }
  }

  lastUpdIndex = -1;

  if(Y_Max5Data[0].value <= yAxisMaxData[MAX_VALUE_COUNT-1].value)
  {
    memcpy(Y_Max5Data, yAxisMaxData, (sizeof(receivedMaxData_t)*MAX_VALUE_COUNT));
  }
  else if(Y_Max5Data[MAX_VALUE_COUNT-1].value < yAxisMaxData[0].value)
  {

    for(i = 0; i < MAX_VALUE_COUNT; i++)
    {
      for(j = lastUpdIndex+1; j < MAX_VALUE_COUNT; j++)
      {
        if(yAxisMaxData[i].value > Y_Max5Data[j].value)
        {    
          //shift old data to right.
          /* 0 1 2 3 4 -> '' 0 1 2 3 */
          for(uint8_t k = (MAX_VALUE_COUNT-1); k>j; k--)
          {
            Y_Max5Data[k].value = Y_Max5Data[k-1].value;
            Y_Max5Data[k].index = Y_Max5Data[k-1].index;
          }

          Y_Max5Data[j].value = yAxisMaxData[i].value;
          Y_Max5Data[j].index = yAxisMaxData[i].index;
          
          lastUpdIndex = j;
          break;
        }
      }

      if(j == MAX_VALUE_COUNT)
      {
        /* if i. value lower than all X Max5Data, then i+1. value is already lower, dont check for other i values.*/
        break;
      }
    }
  }

}

/*
*
*
*
*
*
*
*/

uint32_t rcvQueueCnt = 0;
uint32_t startTime = 0;
uint32_t endTime = 0;

void collectSensorDataTask(){

//int i;
//  float32_t sourceBuff[fifoFFTBufferSize] = {0};
//  float32_t destBuff[fifoFFTBufferSize] = {0};
//
//  for(i=0; i<2048; i++){
//    sourceBuff[i] = i*12;
//  }
//  
//  arm_rfft_fast_f32(&fftIns, sourceBuff, destBuff, 0);
//
//
//  printf("islem tamam\n");
//
//  i = i*5;

  uxHighWaterMarkGlb = uxTaskGetStackHighWaterMark(NULL);
  accelerometerData_t accData = {0};
  receivedMaxData_t   xAxisMaxData[MAX_VALUE_COUNT] = {0};
  receivedMaxData_t   yAxisMaxData[MAX_VALUE_COUNT] = {0};
  int32_t i32sensorData = 0u;
  //printf("int32_t i32sensorData %d:\n",  i32sensorData);// Ahmet Bakacak
  //float32_t sensorData[X_Y_AXIS_NUM][TEN_SEC_DATA_COUNT] = {0};

  float32_t sensorData[X_Y_AXIS_NUM][FFT_BUFFER_SIZE] = {0};
  uint16_t  dataCount = 0;
  uint32_t sizeQ = 0;
  uint8_t      alarmExist = 0,xAlarmExists = 0, yAlarmExists = 0;

  while(true){

    sizeQ = uxQueueMessagesWaiting(xAccDataQueue);
    //printf("Acc data queue size is= %d\n", size);
  
    //uxHighWaterMarkGlb = uxTaskGetStackHighWaterMark(NULL);
  
    if(sizeQ == 0)
    {
      vTaskDelay(2);
      continue; //queue is empty
    }
    if(xQueueReceive(xAccDataQueue, &accData, 1) != pdTRUE){
  
      continue;
    }

    rcvQueueCnt++;
    i32sensorData = 0u;
    
    i32sensorData = (((uint32_t)accData.xAxisData[0]<<12)&0x000FF000 | ((uint32_t)accData.xAxisData[1]<<4)&0x00000FF0 | ((uint32_t)accData.xAxisData[2]&0x000000F0)>>4);
    
    if((i32sensorData & 0x00080000) == 0x00080000){
      i32sensorData = (i32sensorData|0xFFF00000);
    }

    sensorData[AXIS_X][dataCount] = (((float32_t)i32sensorData)/adxl355Scale);

    i32sensorData = 0u;

    i32sensorData = (((uint32_t)accData.yAxisData[0]<<12)&0x000FF000 | ((uint32_t)accData.yAxisData[1]<<4)&0x00000FF0 | ((uint32_t)accData.yAxisData[2]&0x000000F0)>>4);
    
    if((i32sensorData & 0x00080000) == 0x00080000){
      i32sensorData = (i32sensorData|0xFFF00000);
    }

    sensorData[AXIS_Y][dataCount] = (((float32_t)i32sensorData)/adxl355Scale);
    
    dataCount++;

    if(dataCount == TEN_SEC_DATA_COUNT){
      
      xAlarmExists = findMaxFiveData(&sensorData[AXIS_X][0], dataCount, &xAxisMaxData[0], deviceManager.axis_condition[AXIS_X].threshold);
      alarmExist = xAlarmExists;
      checkAlarmStatus(xAlarmExists,AXIS_X,xAxisMaxData[0].value);
      yAlarmExists = findMaxFiveData(&sensorData[AXIS_Y][0], dataCount, &yAxisMaxData[0], deviceManager.axis_condition[AXIS_Y].threshold);
      alarmExist |= yAlarmExists;
      checkAlarmStatus(yAlarmExists,AXIS_Y,yAxisMaxData[0].value);


     // bool maxThreeExistX = findMaxThreeData(&sensorData[AXIS_X][0], dataCount, &xAxisMaxData[0]);  //Ahmet
      //bool maxThreeExistY = findMaxThreeData(&sensorData[AXIS_Y][0], dataCount, &yAxisMaxData[0]);  

      //if(maxThreeExistX)
      //{  
      //    printf("X eksenindeki en büyük üç deðer:\n");
      //    for(int i = 0; i < MAX_VALUE_COUNT; i++)
      //    {
      //        printf("%f\n", xAxisMaxData[i]);
      //    }
      //}

      //if(maxThreeExistY)
      //{  
      //    printf("Y eksenindeki en büyük üç deðer:\n");
      //    for(int i = 0; i < MAX_VALUE_COUNT; i++)
      //    {
      //        printf("%f\n", yAxisMaxData[i]);
      //    }
      //}

      if(alarmExist)//
      {  
          prepareAlarmMessage(&xAxisMaxData[0], &yAxisMaxData[0], MAX_VALUE_COUNT);
      }

      updateMax5Values(&xAxisMaxData[0], &yAxisMaxData[0]);
      //printf("bu kýsýmda calisiyoruz");
      GetTime();

      dataCount = 0;
      memset(&xAxisMaxData[0],0,(sizeof(receivedMaxData_t)*MAX_VALUE_COUNT));
      memset(&yAxisMaxData[0],0,(sizeof(receivedMaxData_t)*MAX_VALUE_COUNT));
      memset(&sensorData[AXIS_X][0],0,(sizeof(float32_t)*FFT_BUFFER_SIZE));
      memset(&sensorData[AXIS_Y][0],0,(sizeof(float32_t)*FFT_BUFFER_SIZE));
    }

    vTaskDelay(2);   
    //uxHighWaterMarkGlb = uxTaskGetStackHighWaterMark(NULL);   
  }
}



/*
*
*
*
*
*
*
*/
void createCollectSensorDataTask(){

  uint8_t xReturned = xTaskCreate(collectSensorDataTask, "CollectSensorData", 6500, NULL, tskIDLE_PRIORITY + 1, &xCollectSensorDataTask);
  if (xReturned != pdPASS) {
    NRF_LOG_ERROR("CollectSensorData task not created.");
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
  }
}

/**@brief Function BLE data task function.
   @param[in] *arg
 */

void prvBLEData()
{

    uint32_t newTresholdValue;
    uint16_t bluetoothCounter;
    uint32_t receivedTreshold;
    uint8_t bluetoothBuffer[MAX_UART_LEN];	

//
//    while (1)
//    {
//        
        nrf_drv_wdt_channel_feed(m_channel_id);
        ret_code_t err_code;
        newTresholdValue = 0;
        bluetoothCounter = 0;
        memset(bluetoothBuffer, 0, MAX_UART_LEN);
        receivedTreshold = 0;

        if (RxDataBuffer[0] == SET_XAXISTHRESHOLD_VALUE_MESSAGE)
        {
            RxDataBuffer[0] = 0xFF;
            newTresholdValue = RxDataBuffer[1] << 24 | RxDataBuffer[2] << 16 | RxDataBuffer[3] << 8 | RxDataBuffer[4];
            memcpy((void* )&X_axisTreshold,(void* )&newTresholdValue,sizeof(float32_t));
            // mpdelete shm_printf("xaxis value = %x\n",newTresholdValue);
            vFlashWritetoFlash(newTresholdValue, XAXISDATAADDRESS, XAXISDATAPAGE);
            nrf_delay_ms(100);
            newTresholdValue = 0;
        }

        if (RxDataBuffer[0] == SET_YAXISTHRESHOLD_VALUE_MESSAGE)
        {
            RxDataBuffer[0] = 0xFF;
            newTresholdValue = RxDataBuffer[1] << 24 | RxDataBuffer[2] << 16 | RxDataBuffer[3] << 8 | RxDataBuffer[4];
            memcpy((void* )&Y_axisTreshold,(void* )&newTresholdValue,sizeof(float32_t));
            // mpdelete shm_printf("yaxis value = %x\n",newTresholdValue);
            vFlashWritetoFlash(newTresholdValue, YAXISDATAADDRESS, YAXISDATAPAGE);
            nrf_delay_ms(100);
            newTresholdValue = 0;
        }

        if (RxDataBuffer[0] == READ_AXISTHRESHOLD_VALUE_MESSAGE)
        {
            RxDataBuffer[0] = 0xFF;
            vFlashReadAxisFromFlash();
        }

        if (RxDataBuffer[0] == SEND_EEPROM_DATA_MESSAGE)
        {
            RxDataBuffer[0] = 0xFF;
//            TimerStop(&LoRaSendPacketTimer);
            bluetoothCounter = RxDataBuffer[1] << 8 | RxDataBuffer[2];
            prvSendEepromData(bluetoothCounter);
//            TimerSetValue(&LoRaSendPacketTimer, 180000);
//            TimerStart(&LoRaSendPacketTimer);
        }
        if (RxDataBuffer[0] == READLORAPARAMETERS)
        {
            RxDataBuffer[0] = 0xFF;
            memcpy(&bluetoothBuffer[0], DevEui, 8);
            uart_tx_characteristic_update(&m_uart_service, &bluetoothBuffer[0]);
        }

        if (RxDataBuffer[0] == SET_FREQUENCY)
        {

            receivedTreshold = (RxDataBuffer[1] << 24) | (RxDataBuffer[2] << 16) | (RxDataBuffer[3] << 8) | RxDataBuffer[4];

            if(receivedTreshold > 0 && receivedTreshold < 1024)
            {
              numOfSamplePoints = (uint16_t)receivedTreshold;
              vFlashWritetoFlash(receivedTreshold, FREQUENCYADDRESS, FREQUENCYPAGE);
            }
  
            
            receivedTreshold = 0;
        }

        if (RxDataBuffer[0] == READ_FREQUENCY)
        {

            receivedTreshold = vFlashReadFromFlash(FREQUENCYADDRESS);
            memcpy(&bluetoothBuffer[0], &receivedTreshold, sizeof(uint32_t));
            uart_tx_characteristic_update(&m_uart_service, &bluetoothBuffer[0]);
        }
//
//        vTaskDelay(100); //wait for 10 ms 
//    }

}


//void createBluetoothDataTask(){
//
//  uint8_t xReturned = xTaskCreate(prvBLEData, "BluetoothData", 150, NULL, tskIDLE_PRIORITY + 1, &xBluetoothDataTask);
//  if (xReturned != pdPASS) {
//    NRF_LOG_ERROR("BluetoothDataTask task not created.");
//    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
//  }
//}


/* STATE MACHINE TASK FUNCTIONS */

void LoraStateMachineTask() {

  LoRaMacPrimitives_t LoRaMacPrimitives;
  LoRaMacCallback_t LoRaMacCallbacks;
  MibRequestConfirm_t mibReq;
  MlmeReq_t mlmeReq;
  AppData_t AppData;
  uint8_t   IsTxConfirmed;
  bool      isBusy = false;
  bool      isJoined = false;
  //UBaseType_t     uxHighWaterMark;
  
  uint32_t size;
  setDeviceState(DEVICE_STATE_INIT);


  //uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);

  
  while (1) 
  {

    nrf_drv_wdt_channel_feed(m_channel_id);
    if (isBLEConnected == false) {
  
    //uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);


    switch (getDeviceState()) {

      case DEVICE_STATE_INIT:

        LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
        LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
        LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
        LoRaMacPrimitives.MacMlmeIndication = MlmeIndication;
        LoRaMacCallbacks.GetBatteryLevel = BoardGetBatteryLevel;
        LoRaMacInitialization(&LoRaMacPrimitives, &LoRaMacCallbacks, ACTIVE_REGION);

        mibReq.Type = MIB_ADR;
        mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
        LoRaMacMibSetRequestConfirm(&mibReq);

        mibReq.Type = MIB_SYSTEM_MAX_RX_ERROR;
        mibReq.Param.SystemMaxRxError = 20;
        LoRaMacMibSetRequestConfirm(&mibReq);

        mibReq.Type = MIB_PUBLIC_NETWORK;
        mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
        LoRaMacMibSetRequestConfirm(&mibReq);

        #if defined(REGION_EU868)
                LoRaMacTestSetDutyCycleOn(LORAWAN_DUTYCYCLE_ON);
        #endif

        #if (USE_TTN_NETWORK == 1)
                mibReq.Type = MIB_RX2_DEFAULT_CHANNEL;
                mibReq.Param.Rx2DefaultChannel = (Rx2ChannelParams_t){869525000, DR_3};
                LoRaMacMibSetRequestConfirm(&mibReq);

                mibReq.Type = MIB_RX2_CHANNEL;
                mibReq.Param.Rx2Channel = (Rx2ChannelParams_t){869525000, DR_3};
                LoRaMacMibSetRequestConfirm(&mibReq);
        #endif

       
        channelParam.Address = (uint32_t)CHANNEL_ADDR;

        uint8_t key[] = SESSION_KEY;
//        size = uxQueueMessagesWaiting(txDataQueue);
//  
//        printf("Init2 queue size is= %d\n", size);
      
        memcpy(channelParam.AppSKey, key, sizeof(key));
        memcpy(channelParam.NwkSKey, key, sizeof(key));
       
//        size = uxQueueMessagesWaiting(txDataQueue);
//  
//        printf("Init3 queue size is= %d\n", size);

        channelParam.DownLinkCounter = 0;

        channelParam.Next = 0;

        mibReq.Type = MIB_MULTICAST_CHANNEL;
        mibReq.Param.MulticastList = &channelParam;
        LoRaMacMibSetRequestConfirm(&mibReq);

        mibReq.Type = MIB_DEVICE_CLASS;
        mibReq.Param.Class = CLASS_C;
        LoRaMacMibSetRequestConfirm(&mibReq);

        setDeviceState(DEVICE_STATE_JOIN);
        break;

      case DEVICE_STATE_JOIN: 
      
        isJoined = false;
//      size = uxQueueMessagesWaiting(txDataQueue);
//  
//        printf("Join1 queue size is= %d\n", size);

        #if (OVER_THE_AIR_ACTIVATION != 0)
        
          if(INITADDRESS != 0xAB)
          {
            vInitFirstTimeInit();
          }
          else
          {
            vInitTresholdValuesSet();
          }

          // Initialize LoRaMac device unique ID
          BoardGetUniqueId(DevEui);
//      size = uxQueueMessagesWaiting(txDataQueue);
//  
//        printf("Join2 queue size is= %d\n", size);
          mlmeReq.Type = MLME_JOIN;

          mlmeReq.Req.Join.DevEui = DevEui;
          mlmeReq.Req.Join.AppEui = AppEui;
          mlmeReq.Req.Join.AppKey = AppKey;
          mlmeReq.Req.Join.Datarate = LORAWAN_DEFAULT_DATARATE;
//        printf("Join2.5 queue size is= %d\n", size);
          LoRaMacMlmeRequest(&mlmeReq);
//      size = uxQueueMessagesWaiting(txDataQueue);
//  
//        printf("Join3 queue size is= %d\n", size);
          setDeviceState(DEVICE_STATE_SLEEP);
      
        #else
          // Choose a random device address if not already defined in Commissioning.h
          if (DevAddr == 0) {
            // Random seed initialization
            srand1(BoardGetRandomSeed());

            // Choose a random device address
            DevAddr = randr(0, 0x01FFFFFF);
          }

          mibReq.Type = MIB_NET_ID;
          mibReq.Param.NetID = LORAWAN_NETWORK_ID;
          LoRaMacMibSetRequestConfirm(&mibReq);

          mibReq.Type = MIB_DEV_ADDR;
          mibReq.Param.DevAddr = DevAddr;
          LoRaMacMibSetRequestConfirm(&mibReq);

          mibReq.Type = MIB_NWK_SKEY;
          mibReq.Param.NwkSKey = NwkSKey;
          LoRaMacMibSetRequestConfirm(&mibReq);

          mibReq.Type = MIB_APP_SKEY;
          mibReq.Param.AppSKey = AppSKey;
          LoRaMacMibSetRequestConfirm(&mibReq);

          mibReq.Type = MIB_NETWORK_JOINED;
          mibReq.Param.IsNetworkJoined = true;
          LoRaMacMibSetRequestConfirm(&mibReq);

          setDeviceState(DEVICE_STATE_SEND);
        #endif
        break;

        case DEVICE_STATE_JOINED:
          
          startReadSensorDataTask();
          startSendLoraPeriodicDataTask();
          isJoined = true;
          setDeviceState(DEVICE_STATE_SEND);
          break;
      
        case DEVICE_STATE_SEND:


            if(isBusy == false)
            {
              AppData.Buff = NULL;
              AppData.BuffSize = 0;
              AppData.Port = 0;
              IsTxConfirmed = 0;

              PrepareTxFrame(&AppData, &IsTxConfirmed); //gets new buffer from queue
            }
            
            NextTx = SendFrame(&AppData, &IsTxConfirmed, &isBusy);
            
           
          if(NextTx == false)
          {
            setDeviceState(DEVICE_STATE_SLEEP);
          }


          break;

        case DEVICE_STATE_SLEEP:
          // Wake up through events

          //if transmit ready, change state to send state
          //else check network state, if it is joined change state to send state

          if(isJoined)
          {
            if(NextTx == true){
              setDeviceState(DEVICE_STATE_SEND);
            }
            else{
              OnSendEvent();
            }
          }

          break;

        default:
          setDeviceState(DEVICE_STATE_INIT);
          break;

        }
      }

      vTaskDelay(1000); //wait for 10 ms 
    }
}



void createLoraStateMachineTask(){

  uint8_t xReturned = xTaskCreate(LoraStateMachineTask, "LoraStateMachine", 250, NULL, tskIDLE_PRIORITY + 1, &_LoraStateMachineTask);
  if (xReturned != pdPASS) {
    NRF_LOG_ERROR("LoraStateMachineTask task not created.");
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
  }
}

