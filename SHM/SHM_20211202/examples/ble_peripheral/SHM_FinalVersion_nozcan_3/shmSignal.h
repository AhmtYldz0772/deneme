#ifndef _SHM_SIGNAL_H
#define _SHM_SIGNAL_H

#include <stdlib.h>
#include "sdk_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "arm_math.h"

#define FREQ_ACCURACY ((float32_t)125/1024)
#define JOIN_MESS_LEN 9
#define SHM_DATA_PORT 92
#define TX_BUFF_SIZE  30



extern uint16_t numOfSamplePoints;

extern arm_rfft_fast_instance_f32 fftIns;

extern float32_t X_axisTreshold;
extern float32_t Y_axisTreshold;

extern uint8_t DevEui[8];

extern xQueueHandle  txDataQueue;
//extern xQueueHandle  queueExp;

#endif