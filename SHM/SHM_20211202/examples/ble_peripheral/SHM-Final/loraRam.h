#ifndef _LORA_RAM_H
#define _LORA_RAM_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "stdint.h"
#include "stdbool.h"
#include "Commissioning.h"
#include <math.h>
#include "adxlRam.h"
#include "Mac/LoRaMac.h"

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


/*!
 * LoRaWAN compliance tests support data
 */
typedef struct ComplianceTest_s {
  bool Running;
  uint8_t State;
  bool IsTxConfirmed;
  uint8_t AppPort;
  uint8_t AppDataSize;
  uint8_t *AppDataBuffer;
  uint16_t DownLinkCounter;
  bool LinkCheck;
  uint8_t DemodMargin;
  uint8_t NbGateways;
} ComplianceTest_t;





/* LoRaMAC Multicast Channel Address */
#define CHANNEL_ADDR 0x11223344

/* LoRaMAC Multicast Channel Application & Network Session Key */
#define SESSION_KEY { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C }

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


#define SEND_LORA_DATA_PERIOD (180*1024) //Send Sensor data to Lora evary 3 min.


/* Externs */

extern TimerHandle_t readSensorDataTimer;
extern TimerHandle_t sendPeriodicDataTimer;

extern TaskHandle_t _LoraStateMachineTask;
extern TaskHandle_t xCollectSensorDataTask;

extern xSemaphoreHandle xDeviceState;
extern xSemaphoreHandle xNextTx;

extern lora_config_t lora_config;
extern deviceStateEnum_t   DeviceState;
extern bool NextTx;
extern volatile bool isBLEConnected;
extern uint8_t AppEui[];
extern uint8_t AppKey[];
extern ComplianceTest_t ComplianceTest;
extern MulticastParams_t channelParam;

extern uint32_t uxHighWaterMarkGlb;

#endif
