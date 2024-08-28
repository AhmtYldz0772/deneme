#include "shmSignal.h"



arm_rfft_fast_instance_f32 fftIns;

xQueueHandle  txDataQueue;

uint16_t numOfSamplePoints = 82; /* for requested Freq. ((10*125)/1024) */
                                 /* Samples between 0-10Hz is required. 125Hz sampling freq, 1024 Point sampling rate. */

float32_t X_axisTreshold = 0.9000;    /* x axis threshold, default 0.5000 raktas  */
float32_t Y_axisTreshold = 0.9000;    /* y axis threshold, default 0.5000 raktas  */

uint8_t DevEui[8];

//xQueueHandle  queueExp;