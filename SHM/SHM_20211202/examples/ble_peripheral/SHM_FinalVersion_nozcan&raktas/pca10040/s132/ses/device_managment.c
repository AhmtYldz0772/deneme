#include "device_managment.h"
#include "shmSignal.h"
#include <stdio.h> // printf için
#include <stdbool.h> // bool için
#include "flash.h"
#include <stdarg.h>

#define THRESHOLD_ADJUSTMENT_STEP 0.0001f


DeviceManager_t deviceManager; 

static uint8_t consecutive_alarm_count = 0;

static bool calibration_needed = true;

/*
 *
 *
 *
 */
void deviceManagerInit(uint8_t clear_timers) {
  deviceManager.status = DEVICE_INIT;
  if(clear_timers != 0)
   {
      deviceManager.device_exec_counter_second = 0;
   }
  deviceManager.is_reset_exists = 0;
  deviceManager.device_exec_last_counter_second = 0;


  for(int i = 0; i < NUM_OF_AXIS; i++)
  {
    deviceManager.axis_condition[i].threshold_acceptance_try_count = 0;
    deviceManager.axis_condition[i].status = AXIS_INIT;
    deviceManager.axis_condition[i].threshold_advance_value = 0;
    deviceManager.axis_condition[i].alarm_exists = 0;
    deviceManager.axis_condition[i].threshold = 0;
    deviceManager.axis_condition[i].calibration_strategy = AUTO_SENSE_STRATEGY;

    switch(deviceManager.axis_condition[i].calibration_strategy)
    {
      case INCREMENTAL_STRATEGY:
        break;
       case AUTO_SENSE_STRATEGY:
        break;
       default:
        break;
    
    }
  
  }

  if(deviceManager.axis_condition[DEV_AXIS_X].calibration_strategy == INCREMENTAL_STRATEGY)
  {
  // X private inits
    deviceManager.axis_condition[DEV_AXIS_X].threshold = X_axisTreshold;
    deviceManager.axis_condition[DEV_AXIS_X].status = AXIS_WAITING_CALIBRATION;
    deviceManager.axis_condition[DEV_AXIS_X].threshold_advance_value = 0.0001;// (deviceManager.axis_condition[DEV_AXIS_X].threshold / 10);
  }

  if(deviceManager.axis_condition[DEV_AXIS_Y].calibration_strategy == INCREMENTAL_STRATEGY)
  {
      // Y private inits
      deviceManager.axis_condition[DEV_AXIS_Y].threshold = Y_axisTreshold;
      deviceManager.axis_condition[DEV_AXIS_Y].status = AXIS_WAITING_CALIBRATION;
      deviceManager.axis_condition[DEV_AXIS_Y].threshold_advance_value = 0.0001; //(deviceManager.axis_condition[DEV_AXIS_Y].threshold / 10);
   }
 }

/*
 *
 *
 *
 */
void setDeviceStatus(DeviceStatus_e status) {
  deviceManager.status = status;
}

/*
 *
 *
 *
 */
DeviceStatus_e getDeviceStatus() {
  return deviceManager.status;
}

/*
 *
 *
 *
 */
void updateThreshold(float32_t new_threshold, uint8_t axis) {
    deviceManager.axis_condition[axis].threshold = new_threshold;

    switch(axis)
    {
      case DEV_AXIS_X:
          memcpy(&X_axisTreshold, (void *)&deviceManager.axis_condition[axis].threshold, sizeof(float32_t)); 
         // vFlashWritetoFlash(deviceManager.axis_condition[axis].threshold, XAXISDATAADDRESS, XAXISDATAPAGE);
          break;
      case DEV_AXIS_Y:
          memcpy(&Y_axisTreshold, (void *)&deviceManager.axis_condition[axis].threshold, sizeof(float32_t)); 
          //vFlashWritetoFlash(deviceManager.axis_condition[axis].threshold, YAXISDATAADDRESS, YAXISDATAPAGE);
          break;
      default:
        // mpdelete shm_printf("Invalid Threshold %f",new_threshold);
        break;
    }

// mpdelete shm_printf("Threshold for Axis %d set\n", axis);
 
  
}

/*
 *
 *
 *
 */
float32_t get_threshold(uint8_t axis){
  return deviceManager.axis_condition[axis].threshold;
}

/*
 *
 *
 *
 */
void checkAlarmStatus(uint8_t alarm_detected, uint8_t axis,float32_t max_amplitude) {
    float32_t new_threshold = 0;
    
    if(deviceManager.axis_condition[axis].status == AXIS_CALIBRATED)
      return;
    
    printf("Check alarm status started\n");

    if(deviceManager.axis_condition[axis].alarm_exists == (uint8_t)alarm_detected )
    {  
      deviceManager.axis_condition[axis].threshold_acceptance_try_count++;

    }else{
      deviceManager.axis_condition[axis].threshold_acceptance_try_count = 0;
    }

    deviceManager.axis_condition[axis].alarm_exists = alarm_detected;

    printf("Calibration Axis %x try %x threshold %.10f alarm %x\n",axis, 
      deviceManager.axis_condition[axis].threshold_acceptance_try_count,
      (float)deviceManager.axis_condition[axis].threshold,
      alarm_detected );

    if(alarm_detected == false && deviceManager.axis_condition[axis].threshold_acceptance_try_count < MAX_CONSECUTIVE_ALARM_COUNT)
    {
        return;
    }

    switch(deviceManager.axis_condition[axis].calibration_strategy)
    {
      case INCREMENTAL_STRATEGY:
        new_threshold = deviceManager.axis_condition[axis].threshold_advance_value + deviceManager.axis_condition[axis].threshold;
        break;
      case AUTO_SENSE_STRATEGY:
        new_threshold = max_amplitude;
        break;
    }

    if(deviceManager.axis_condition[axis].alarm_exists == 1)
    {
      //increase threshold and try again
      updateThreshold(new_threshold, axis);
      /* mpdelete shm_printf("Calibration Updated Axis %x try %x threshold %.10f alarm %x\n",axis, deviceManager.axis_condition[axis].threshold_acceptance_try_count,
      deviceManager.axis_condition[axis].threshold,alarm_detected );*/
    }else
    {
      deviceManager.axis_condition[axis].status = AXIS_CALIBRATED;
      printf("Calibrated Axis %x try %x threshold %.10f alarm %x\n",axis, deviceManager.axis_condition[axis].threshold_acceptance_try_count,
      deviceManager.axis_condition[axis].threshold,alarm_detected );

    }

deviceManager.axis_condition[axis].threshold_acceptance_try_count = 0;
deviceManager.axis_condition[axis].alarm_exists = 0;

}

/*
 *
 *
 *
 */
// mpdelete 
/*
void shm_printf(const char * format,...){
va_list args;

if(DEBUG_MOD_ENABLED == 0)
  return;

va_start(args, format);
printf(format,args);
va_end(args);
   
}*/
