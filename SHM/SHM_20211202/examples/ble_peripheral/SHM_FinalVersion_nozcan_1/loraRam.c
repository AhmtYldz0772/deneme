#include "loraRam.h"
#include "shmSignal.h"

TaskHandle_t _LoraStateMachineTask;

xSemaphoreHandle xDeviceState;
xSemaphoreHandle xNextTx;

lora_config_t lora_config = 
{
  .AppPort = SHM_DATA_PORT,
  .IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON,
};

deviceStateEnum_t   DeviceState; //globale tasi. raktas
ComplianceTest_t ComplianceTest;
bool NextTx = false;
volatile bool isBLEConnected = false;

uint8_t AppEui[] = LORAWAN_APPLICATION_EUI;
uint8_t AppKey[] = LORAWAN_APPLICATION_KEY;
