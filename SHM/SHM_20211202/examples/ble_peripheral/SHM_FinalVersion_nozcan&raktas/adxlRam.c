#include <math.h>
#include "adxlRam.h"



//Queue Handles
xQueueHandle  xAccDataQueue;

float32_t adxl355Scale = 0.0f;

receivedMaxData_t X_Max5Data[MAX_VALUE_COUNT];
receivedMaxData_t Y_Max5Data[MAX_VALUE_COUNT];