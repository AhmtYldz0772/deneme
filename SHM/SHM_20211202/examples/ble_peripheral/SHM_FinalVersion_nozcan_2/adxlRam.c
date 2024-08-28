#include <math.h>
#include "adxlRam.h"


//Timer Handles
TimerHandle_t readSensorDataTimer;



//Task Handles
TaskHandle_t xCollectSensorDataTask;



//Queue Handles
xQueueHandle  xAccDataQueue;

