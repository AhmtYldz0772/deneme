#ifndef EEPROM_H
#define EEPROM_H

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


#define EEPROM_POINTER_ADDRESS 0

#define SLAVE_ADDRESS       0x0A
#define DEVICE_ADDRESS      0x00
#define TWI_READ            1
#define TWI_WRITE           0
#define LAST_MEMORY_ADDRESS 0x1FFFE
#define FIRST_MEMORY_ADDRESS 0x0000
#define PAGESIZE            256

struct FrequencyRange_s
{
  uint32_t xHighFrequency;
  uint32_t xLowFrequency;

  uint32_t yHighFrequency;
  uint32_t yLowFrequency;

};

void vInitTwiWrite (uint32_t address , uint8_t *data, uint8_t size );
void twi_init (void);
void vInitFIfotoEeprom (void);
//void vInitTwiRead (uint32_t address ,  uint8_t size );
void vInitTwiRead (uint32_t address, uint8_t size, uint8_t * dataPointer);
void prvSendEepromData(void);
void eepromWriteff(void);
#endif /* EEPROM_H */