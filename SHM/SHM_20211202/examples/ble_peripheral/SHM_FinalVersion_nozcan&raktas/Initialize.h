#ifndef INITIALIZE_H
#define INITIALIZE_H

#include <stdio.h>
#include <stdlib.h>
/*---------------------------------------------FreeRTOS Includes-----------------------------------------*/
#include "FreeRTOS.h"
#include "SEGGER_RTT.h"
#include "fds.h"
#include "nrf_drv_gpiote.h"
#include "semphr.h"
#include "spi-board.h"
#include "task.h"
#include "timers.h"
/*-------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------CMSIS Includes-----------------------------------------*/
#include "arm_math.h"
/*-------------------------------------------------------------------------------------------------------*/

void vInitTresholdValuesSet(void);
void vInitFirstTimeInit (void);

#endif /* INITIALIZE_H */