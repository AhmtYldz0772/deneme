
#include "adxlRam.h"
#include "ADXL355.h"
#include "shmSignal.h"
#include "loraRam.h"
#include "pca10040/s132/ses/device_managment.h"
#include "pca10040/s132/ses/RTC_RT.h"
#include "pca10040/s132/ses/loraFunctions.h"
#include "pca10040/s132/ses/loraMessages.h"


//#define TEST_ENABLED

#define THRESHOLD_MSG_AXIS_DATA_MARKER_LEN            sizeof(uint8_t)
#define THRESHOLD_MSG_AXIS_DATA_MAGNITUDE_LEN         sizeof(float32_t)
#define THRESHOLD_MSG_AXIS_DATA_FREQ_LEN              sizeof(float32_t)
#define THRESHOLD_ONE_MSG_DATA_LEN (THRESHOLD_MSG_AXIS_DATA_MARKER_LEN + THRESHOLD_MSG_AXIS_DATA_MAGNITUDE_LEN + THRESHOLD_MSG_AXIS_DATA_FREQ_LEN)

void prepareAlarmMessage(receivedMaxData_t *xAxisSignal, receivedMaxData_t *yAxisSignal, uint32_t data_size){

  uint8_t alarmMessage[LORAWAN_DATA_MAX_SIZE];
  uint8_t index = 0, counter = 0;
  float32_t freq_x, freq_y;
  uint8_t message_len = 0,message_len_index = 0;
  uint8_t isAlarmExists = 0;
  //float32_t max_valueX = 0;
  //float32_t max_valueY = 0;

  memset(alarmMessage,0,LORAWAN_DATA_MAX_SIZE);

  alarmMessage[index] = ABOVE_THRESHOLD_SEND;
  index++;
  
  // BYTE 1: MESSAGE TYPE -> 0X12
  // [
  // bYTE 2: axýs - x or y 0Xab -> x   0xcd ->Y
  // byte 3-4-5-6 -> ALARM PEAk
  // freq -> 0x3a
  // BYTE 7-8-9-10 -> FREQ

  // 0x12 0xab 0x12 0x3a 0x43 0x3a 0x43 0x41 0x45 0x67n 0xab

  message_len_index = index;
  alarmMessage[message_len_index] = message_len;
  index++;

// For X
  for(counter = 0; counter < data_size; counter++) //MAX_VALUE_COUNT
  {
    if(xAxisSignal[counter].value > get_threshold(AXIS_X))
    {   
      alarmMessage[index] = xAxis; 
      index++; 
      //sprintf(&alarmMessage[index],"%d",xAxis);
      
      memcpy(&(alarmMessage[index]), &(xAxisSignal[counter].value), sizeof(float32_t));
      index += sizeof(float32_t);
      //sprintf(&alarmMessage[index],"%d",xAxis);

      freq_x = (((float32_t)xAxisSignal[counter].index) * FREQ_ACCURACY);
      memcpy(&alarmMessage[index], &freq_x, sizeof(float32_t));
      index += sizeof(float32_t);
      
      message_len += (uint8_t)THRESHOLD_ONE_MSG_DATA_LEN;

      isAlarmExists = 1;
      //if(xAxisSignal[counter].value > max_valueX)
      //  {
      //      max_valueX = xAxisSignal[counter].value;
      //  }
    }    
  }

  // For Y
  for(counter = 0; counter < data_size; counter++) //MAX_VALUE_COUNT
  {
      if(yAxisSignal[counter].value > get_threshold(AXIS_Y))
      {
      
        alarmMessage[index] = yAxis; 
        index++; 
      
        memcpy(&(alarmMessage[index]), &(yAxisSignal[counter].value), sizeof(float32_t));
        index += sizeof(float32_t);

        freq_y = (((float32_t)yAxisSignal[counter].index) * FREQ_ACCURACY);
        memcpy(&alarmMessage[index], &freq_y, sizeof(float32_t));
        index += sizeof(float32_t);

        message_len+= (uint8_t)THRESHOLD_ONE_MSG_DATA_LEN;
        isAlarmExists = 1;
      //if(xAxisSignal[counter].value > max_valueY)
      //  {
      //      max_valueY = xAxisSignal[counter].value;
      //  }
      }
  }

  alarmMessage[message_len_index] = message_len;

/*
printf("#### ALARM ####\n");

printf("X: Index: %d %f Freq: %f \n",((int)xAxisSignal[0].index),(float)xAxisSignal[0].value,(float)freq_x);

printf("Y: Index: %d %f Freq: %f\n",((int)yAxisSignal[0].index),(float)yAxisSignal[0].value,(float)freq_y);
*/

    // >> TEST CODE START
#ifdef TEST_ENABLED   
    float32_t value_temp = 789.456;//0x4ABBCCDD;
    

  
    index = 0;
    alarmMessage[index++] = ABOVE_THRESHOLD_SEND; 
  
    alarmMessage[index++] = (THRESHOLD_ONE_MSG_DATA_LEN * 2);                     // length

    alarmMessage[index++] = 0xAB;                     // Axis type
    memcpy(&alarmMessage[index],&value_temp,4);       // threshold
    index+=4;
    float32_t f_val = 123.456;
    memcpy(&alarmMessage[index],&f_val,4);            // FREQ
    index+=4;
     
    alarmMessage[index++] = 0xCD;// AXÝS
    memcpy(&alarmMessage[index],&value_temp,4);       // THRESHOLD
    index+=4;
    float32_t f_valu = 123.456;                        // FREQ
    memcpy(&alarmMessage[index],&f_valu,4);
    index+=4;
#endif

    // >> TEST CODE END

  if(isAlarmExists == 1)
  {
    
    sendLoraDataMessage(alarmMessage, index);
  }

}
/*
*
*
*
*
*/

void send_device_info_message(){
    uint8_t index = 0;
    uint8_t get_Aktif_Message[64];

    get_Aktif_Message[index] = DEVICE_INFO_MESSAGE_CODE;
    index++;

    //THRASHOLDX DEÐERÝ
    float32_t GET_AXIS_X = get_threshold(AXIS_X);
    memcpy(&get_Aktif_Message[index], &GET_AXIS_X, sizeof(float32_t));
    index += sizeof(float32_t);

    //THRESHOLDY DEÐERÝ
    float32_t GET_AXIS_Y = get_threshold(AXIS_Y);
    memcpy(&get_Aktif_Message[index], &GET_AXIS_Y, sizeof(float32_t));
    index += sizeof(float32_t);

    // TIME DEGERI 
    uint32_t curr_time = system_time_ready == 1 ? GET_TIME_IN_SECONDS : 0;
    memcpy(&get_Aktif_Message[index], &curr_time, sizeof(uint32_t));
    index += sizeof(uint32_t);

    // device_exec_counter_second
    uint32_t exec_counter = deviceManager.device_exec_counter_second;
    memcpy(&get_Aktif_Message[index], &exec_counter, sizeof(uint32_t));
    index += sizeof(uint32_t);

    sendLoraDataMessage(get_Aktif_Message, index);

}



/*
*
*
*
*
*/
void Get_Show_Threshold() {
    uint8_t index = 0;
    uint8_t threshold_mess_len = 9;
    uint8_t getThreshold[threshold_mess_len];

    getThreshold[index] = GET_SHOW_THRESHOLD;
    index++;

    float32_t GET_AXIS_X = get_threshold(AXIS_X);
    memcpy(&getThreshold[index], &GET_AXIS_X, sizeof(float32_t));
    index += sizeof(float32_t);
   // printf("GET_AXIS_X: %f\n", GET_AXIS_X); 

    float32_t GET_AXIS_Y = get_threshold(AXIS_Y);
    memcpy(&getThreshold[index], &GET_AXIS_Y, sizeof(float32_t));
    index += sizeof(float32_t);
    // printf("GET_AXIS_Y: %f\n", GET_AXIS_Y); 

    sendLoraDataMessage(getThreshold, threshold_mess_len);
}

/*
*
*
*
*
*/

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

  /*
    message_id : 0x11
    len = MAX_VALUE_COUNT;
    X
    [
      peak: 4  byte
      freq: 4  byte
    ]
    Y
    [
      peak: 4  byte
      freq: 4  byte
    ] 

  */

  maxValMessage[index] = MAX_VALUES_SEND;
  index++;
  maxValMessage[index] = MAX_VALUE_COUNT;
  index++;
  /* mutex giris */

  for(counter = 0; counter < MAX_VALUE_COUNT; counter++){
    freq = (X_Max5Data[counter].index * FREQ_ACCURACY);
    memcpy(&maxValMessage[index], &X_Max5Data[counter].value, sizeof(float32_t));
    index += sizeof(float32_t);
    memcpy(&maxValMessage[index], &freq, sizeof(float32_t));
    index += sizeof(float32_t);
  }

  //index += max5Size;
  for(counter = 0; counter < MAX_VALUE_COUNT; counter++){
    freq = (Y_Max5Data[counter].index * FREQ_ACCURACY);
    memcpy(&maxValMessage[index], &Y_Max5Data[counter].value, sizeof(float32_t));
    index += sizeof(float32_t);
    memcpy(&maxValMessage[index], &freq, sizeof(float32_t));
    index += sizeof(float32_t);
  }

  //index += max5Size;
  memset(&X_Max5Data[0], 0, MAX_VALUE_COUNT * sizeof(receivedMaxData_t)); 
  memset(&Y_Max5Data[0], 0, MAX_VALUE_COUNT * sizeof(receivedMaxData_t)); 

  /*mutex cikis */

  sendLoraDataMessage(maxValMessage, index);
}