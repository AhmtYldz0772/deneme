
#include "adxlRam.h"
#include "ADXL355.h"
#include "shmSignal.h"
#include "loraRam.h"

void prepareAlarmMessage(receivedMaxData_t *xAxisSignal, receivedMaxData_t *yAxisSignal){
  uint8_t alarmMessage[LORAWAN_DATA_MAX_SIZE];
  uint8_t index = 0, counter = 0;
  float32_t freq;

  alarmMessage[index] = ABOVE_THRESHOLD_SEND;
  index++;
  
  for(counter = 0; counter < MAX_VALUE_COUNT; counter++){
    if(xAxisSignal[counter].value > X_axisTreshold){
      
      alarmMessage[index] = xAxis; 
      index++; 
      
      memcpy(&(alarmMessage[index]), &(xAxisSignal[counter].value), sizeof(float32_t));
      index += sizeof(float32_t);

      freq = (xAxisSignal[counter].index * FREQ_ACCURACY);
      memcpy(&alarmMessage[index], &freq, sizeof(float32_t));
      index += sizeof(float32_t);
    }
  
    if(yAxisSignal[counter].value > Y_axisTreshold){
      
      alarmMessage[index] = yAxis; 
      index++; 
      
      memcpy(&(alarmMessage[index]), &(yAxisSignal[counter].value), sizeof(float32_t));
      index += sizeof(float32_t);

      freq = (yAxisSignal[counter].index * FREQ_ACCURACY);
      memcpy(&alarmMessage[index], &freq, sizeof(float32_t));
      index += sizeof(float32_t);
    }
  }
 
  if(index > 1){
    sendLoraDataMessage(alarmMessage, index);
  }
}


void prepareJoinMessage(){
  uint8_t joinMess[JOIN_MESS_LEN] = {0};
  
  joinMess[0] = JOIN_REQUEST;
  memcpy(&joinMess[1], &DevEui[0], JOIN_MESS_LEN-1);

  sendLoraDataMessage(joinMess, JOIN_MESS_LEN);
}


void prepareMaxValMessage(){

  uint8_t maxValMessage[LORAWAN_DATA_MAX_SIZE];
  uint8_t index = 0, counter = 0;
  uint8_t max5Size = sizeof(float32_t)*MAX_VALUE_COUNT;
  float32_t freq = 0;

  maxValMessage[index] = MAX_VALUES_SEND;
  index++;
  /* mutex giris */

  for(counter = 0; counter < MAX_VALUE_COUNT; counter++){
    freq = (X_Max5Data[counter].index * FREQ_ACCURACY);
    memcpy(&maxValMessage[index], &X_Max5Data[counter].value, sizeof(float32_t));
    memcpy(&maxValMessage[index + max5Size], &freq, sizeof(float32_t));
    index += sizeof(float32_t);
  }

  index += max5Size;
  for(counter = 0; counter < MAX_VALUE_COUNT; counter++){
    freq = (Y_Max5Data[counter].index * FREQ_ACCURACY);
    memcpy(&maxValMessage[index], &Y_Max5Data[counter].value, sizeof(float32_t));
    memcpy(&maxValMessage[index + max5Size], &freq, sizeof(float32_t));
    index += sizeof(float32_t);
  }

  index += max5Size;
  memset(&X_Max5Data[0], 0, MAX_VALUE_COUNT * sizeof(receivedMaxData_t)); 
  memset(&Y_Max5Data[0], 0, MAX_VALUE_COUNT * sizeof(receivedMaxData_t)); 

  /*mutex cikis */


  
  
  sendLoraDataMessage(maxValMessage, index);
}