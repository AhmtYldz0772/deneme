#ifndef _DEVICE_MANAGMENT_H
#define _DEVICE_MANAGMENT_H
#include "shmSignal.h"
#include "sdk_common.h" // float32_t, DeviceManager_t, DeviceStatus_e tanýmlamalarýný içerir
#include <stdbool.h>

#define THRESHOLD_MAX_ACCEPTANCE_TRY_COUNT  3
#define THRESHOLD_ADJUSTMENT_STEP 0.0001f
#define MAX_CONSECUTIVE_ALARM_COUNT 5
#define DEBUG_MOD_ENABLED           0


typedef enum {
  DEVICE_INIT,
  DEVICE_INIT_REGISTERS,
  DEVICE_INIT_LORA,
  DEVICE_CALIBRATION_REQUEST_RECEIVED,
  DEVICE_CALIBRATION,
  DEVICE_CALIBRATED,
  DEVICE_READY
} DeviceStatus_e;

typedef enum {
  AXIS_INIT,
  AXIS_WAITING_CALIBRATION,
  AXIS_CALIBRATION,
  AXIS_CALIBRATED
} AxisStatus_e;

enum{
  DEV_AXIS_X,
  DEV_AXIS_Y,
  DEV_AXIS_Z,
  NUM_OF_AXIS
};

enum{
  INCREMENTAL_STRATEGY,
  AUTO_SENSE_STRATEGY,
  NUM_OF_CALIBRATION_STRATEGIES
};

typedef struct {
  uint8_t status; // AxisStatus_e
  uint8_t calibration_strategy; // AxisStatus_e
  float32_t threshold_advance_value;
  float32_t threshold;
  uint8_t threshold_acceptance_try_count;
  uint8_t alarm_exists;
} AxisCondition_t;

typedef struct {
  uint8_t           status; // DeviceStatus_e
  uint32_t          device_exec_counter_second;
  uint32_t          device_exec_last_counter_second;
  uint32_t          is_reset_exists;
  AxisCondition_t   axis_condition[NUM_OF_AXIS];
} DeviceManager_t;

extern DeviceManager_t deviceManager;

void deviceManagerInit(uint8_t clr_timers);
void setDeviceStatus(DeviceStatus_e status);
DeviceStatus_e getDeviceStatus(void);
extern void updateThreshold(float32_t new_threshold, uint8_t axis);
void checkAlarmStatus(uint8_t alarm_detected,uint8_t axis,float32_t max_amplitude);
void resetConsecutiveAlarmCount(void);
float32_t get_threshold(uint8_t);

uint8_t isDeviceCalibrated(void);

void updateDeviceStatus(void); // if x,  y and z axises on calibrated status device must be at calibrated status.


// mpdelete void shm_printf(const char * format,...);

#endif // _DEVICE_MANAGMENT_H