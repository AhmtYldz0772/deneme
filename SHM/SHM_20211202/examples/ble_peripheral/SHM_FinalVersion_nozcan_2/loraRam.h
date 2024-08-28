#ifndef _LORA_RAM_H
#define _LORA_RAM_H

#include "stdint.h"
#include "stdbool.h"
#include "Commissioning.h"
#include <math.h>
#include "adxlRam.h"


/*
 * Device states
 */
typedef enum deviceStateEnum{
  DEVICE_STATE_INIT,
  DEVICE_STATE_JOIN,
  DEVICE_STATE_JOINED,
  DEVICE_STATE_SEND,
  DEVICE_STATE_CYCLE,
  DEVICE_STATE_SLEEP,
}deviceStateEnum_t;

typedef struct buffInfo_s{
  uint8_t *buffAddrPtr;
  uint8_t buffSize;
}buffInfo_t;


/*!! used port cannot be higher than 255!*/
typedef struct AppData_s{
  uint8_t   *Buff;
  uint8_t   BuffSize;
  uint8_t   Port;
}AppData_t;


typedef struct lora_config_s{
  uint16_t AppPort;
  uint8_t  IsTxConfirmed;
}lora_config_t;

#define LORAWAN_DEFAULT_DATARATE DR_5
#define LORAWAN_CONFIRMED_MSG_ON false
#define LORAWAN_ADR_ON 1
#define ACTIVE_REGION LORAMAC_REGION_EU868

#if defined(REGION_EU868)

#include "Mac/LoRaMacTest.h"

/*!
 * LoRaWAN ETSI duty cycle control enable/disable
 *
 * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
 */
#define LORAWAN_DUTYCYCLE_ON false

#endif

/* Externs */

extern TaskHandle_t _LoraStateMachineTask;

extern lora_config_t lora_config;
extern deviceStateEnum_t   DeviceState;
extern bool NextTx;
extern volatile bool isBLEConnected;
extern uint8_t AppEui[];
extern uint8_t AppKey[];

#endif
