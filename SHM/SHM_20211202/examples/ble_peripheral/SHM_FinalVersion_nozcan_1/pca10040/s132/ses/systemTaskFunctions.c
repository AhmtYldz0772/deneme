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

//-------------------------------------------- LoRaWAN Includes ------------------------------------------//
#include "board.h"


#include "nrfRam.h"
//asagiyi duzenle. raktas

#include "flash.h"

extern arm_rfft_fast_instance_f32 fftIns;


void collectedSensorDataFFT(float32_t *collectedDataPtr, uint16_t dataSize, float32_t *FftResultPtr){

  float32_t sourceBuff[fifoFFTBufferSize] = {0};
  
  if(dataSize > fifoFFTBufferSize){
    dataSize = fifoFFTBufferSize;
  }

  memcpy(sourceBuff,collectedDataPtr,dataSize); /* between datasize and fifoFFTBufferSize is filled 0. Make 0 padding to complete 2048 sample point */

  arm_rfft_fast_f32(&fftIns, sourceBuff, FftResultPtr, 0);

}


bool findMaxFiveData(float32_t *collectedDataPtr, uint16_t dataSize, receivedMaxData_t *maxDataPtr){
  bool alarmExist = false;
  uint16_t  index = 0;
  float32_t fftResultBuff[fifoFFTBufferSize] = {0};
  float32_t *amplitudeBuffPtr = NULL;

  collectedSensorDataFFT(collectedDataPtr, dataSize, fftResultBuff);
  
  amplitudeBuffPtr = malloc(numOfSamplePoints*sizeof(float32_t));

  if(amplitudeBuffPtr == NULL){
    //not available area in heap. log it.
    return;
  }

  for (index = 0; index < numOfSamplePoints; index++) {
    amplitudeBuffPtr[index] = sqrt(pow(fftResultBuff[(2*index)], 2) + pow(fftResultBuff[(2*index)+1], 2));
  }

  for(index = 0; index < MAX_VALUE_COUNT; index++){

    arm_max_f32(&amplitudeBuffPtr[0], numOfSamplePoints, &(maxDataPtr->value), &(maxDataPtr->index));
    if(maxDataPtr->value > X_axisTreshold){
      alarmExist = true;
    }
    amplitudeBuffPtr[maxDataPtr->index] = 0; // make 0 maks value to prevent find again as max.
    maxDataPtr++;
  }

  free(amplitudeBuffPtr);
  return alarmExist;
}


int collectSensorDataTaskCounter = 0;
int collectSensorDataTaskNoDataCounter = 0;

void collectSensorDataTask(){

  accelerometerData_t accData = {0};
  receivedMaxData_t   xAxisMaxData[MAX_VALUE_COUNT] = {0};
  receivedMaxData_t   yAxisMaxData[MAX_VALUE_COUNT] = {0};
  float32_t xAxis_f = 0.0f;
  float32_t yAxis_f = 0.0f;
  float32_t sensorData[X_Y_AXIS_NUM][TEN_SEC_DATA_COUNT] = {0};
  uint16_t  dataCount = 0;


  while(true){

    if(xQueueReceive(xAccDataQueue, &accData, portMAX_DELAY) != pdTRUE){ //portMAX_DELAY
      collectSensorDataTaskNoDataCounter++;
      continue;
    }

    #if 0 //mpataci

    xAxis_f = (accData.xAxisData[0]<<12 | accData.xAxisData[1]<<4 | (accData.xAxisData[2]&0xF0)>>4); 
  
    if(((uint32_t)xAxis_f & 0x80000) != 0){
      xAxis_f = (float32_t)((uint32_t)xAxis_f|0xFFF00000);
    }

    yAxis_f = (accData.yAxisData[0]<<12 | accData.yAxisData[1]<<4 | (accData.yAxisData[2]&0xF0)>>4);

    if(((uint32_t)yAxis_f & 0x80000) != 0){
      yAxis_f = (float32_t)((uint32_t)yAxis_f|0xFFF00000);
    }

    sensorData[AXIS_X][dataCount] = xAxis_f;
    sensorData[AXIS_Y][dataCount] = yAxis_f;
    dataCount++;

    //AccDataQueue includes data which is written at every 8ms. At 10 sec, 1250 data is collected from queue.
    if(dataCount == TEN_SEC_DATA_COUNT){
      dataCount = 0;
      if(findMaxFiveData(&sensorData[AXIS_X][0], dataCount, &xAxisMaxData[0]) ||
        findMaxFiveData(&sensorData[AXIS_Y][0], dataCount, &yAxisMaxData[0])){
        
          prepareAlarmMessage(&xAxisMaxData[0], &yAxisMaxData[0]);
        }

    }
    #endif
    collectSensorDataTaskCounter++;
    vTaskDelay(4);
  }
}


int basic_status = 0xAA;
int basic_task_exec_cnt = 0;

void basic_task_handler(void * pvParameter)
{
basic_status = 0xBB;

while(true){
  vTaskDelay(10);
  printf("Basic Task Exec %d \n",basic_task_exec_cnt++);
}

basic_status = 0xCC;

}


int createCollectSensorDataTaskStatus = 0;

void createCollectSensorDataTask(){
 
  //mpataci
  uint8_t xReturned = xTaskCreate(collectSensorDataTask, "CollectSensorData", 1000, NULL, tskIDLE_PRIORITY + 1, &xCollectSensorDataTask);
  //uint8_t xReturned = xTaskCreate(basic_task_handler, "CollectSensorData", 100, NULL, tskIDLE_PRIORITY + 1, NULL);
  if (xReturned != pdPASS) {
    NRF_LOG_ERROR("CollectSensorData task not created.");
   // APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    printf("Collect Sensor Data Task is not Created!");
    createCollectSensorDataTaskStatus = -1;
  }

  createCollectSensorDataTaskStatus = 1;
}



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

  setDeviceState(DEVICE_STATE_INIT);
  
  while (1) 
  {

    nrf_drv_wdt_channel_feed(m_channel_id);
    if (isBLEConnected == false) {
  
  
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

      
        static MulticastParams_t channelParam;
        channelParam.Address = CHANNEL_ADDR;

        uint8_t key[] = SESSION_KEY;

        memcpy(channelParam.AppSKey, key, sizeof(key));
        memcpy(channelParam.NwkSKey, key, sizeof(key));
      
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

          mlmeReq.Type = MLME_JOIN;

          mlmeReq.Req.Join.DevEui = DevEui;
          mlmeReq.Req.Join.AppEui = AppEui;
          mlmeReq.Req.Join.AppKey = AppKey;
          mlmeReq.Req.Join.Datarate = LORAWAN_DEFAULT_DATARATE;

          LoRaMacMlmeRequest(&mlmeReq);

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
        
        
          //burasi incelenecek, raktas
  //        if (ComplianceTest.Running == true) {
  //          // Schedule next packet transmission
  //          TxDutyCycleTime = 5000; // 5000 ms
  //        } else {
  //          // Schedule next packet transmission
  //          TxDutyCycleTime = APP_TX_DUTYCYCLE + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
  //        }
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

      vTaskDelay(100); //wait for 10 ms
    }
}



void createLoraStateMachineTask(){

  uint8_t xReturned = xTaskCreate(LoraStateMachineTask, "LoraStateMachine", 100, NULL, tskIDLE_PRIORITY + 1, &_LoraStateMachineTask);
  if (xReturned != pdPASS) {
    NRF_LOG_ERROR("LoraStateMachineTask task not created.");
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    printf("LoraStateMachineTask task not created.");
  }
}
