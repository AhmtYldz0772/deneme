
#include "flash.h"
#include "nrf_delay.h"
#include "arm_math.h"
#include "BLE_Services/messageCodes.h"
#include "BLE_Services/UART_Service.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define SENDTRESHOLDBUFFERLENGTH 20

float32_t readX_axisThreshold = 0;
float32_t readY_axisThreshold = 0;

uint8_t SendThresholdBuffer[SENDTRESHOLDBUFFERLENGTH] = {0};
uint16_t SendThresholdValue = SEND_THRESHOLD_VALUE_MESSAGE;
extern ble_uarts_t m_uart_service;

/**@brief Function for writing datas to flash.
 *
 *@param[in] data           Data to be written to flash.
 *@param[in] flashAddress   Flash address where the data is written.
 */
void vFlashWritetoFlash(uint32_t data, uint32_t flashAddress, uint32_t flashPAGE) {
  uint32_t *address;
  uint32_t *flashCheck = 0;
  address = (uint32_t *)(flashAddress);
  vFlashEraseDataFromFlash(flashPAGE);
  nrf_delay_ms(6);
  sd_flash_write(address, (uint32_t *)&data, 1);
  nrf_delay_ms(20);

  flashCheck = (uint32_t *)(flashAddress);
  if(*flashCheck != data)
  {
    vFlashEraseDataFromFlash(flashPAGE);
    nrf_delay_ms(6);
    sd_flash_write(address, (uint32_t *)&data, 1);
    nrf_delay_ms(20);
  }
}

/**@brief Function for erasing flash page.
  *
  *@details This function erases the all page which is defined.
  *          
  *@param[in]   flashPageAddress   Flash page address which is erased.
 */
void vFlashEraseDataFromFlash(uint32_t flashPageAddress) {
  sd_flash_page_erase(flashPageAddress);
  nrf_delay_ms(20);
}

/**@brief Function for reading  from flash.
 */
uint32_t vFlashReadFromFlash(uint32_t flashAddress) {

  uint32_t *flashAddress_32, flashAddressValue;
  flashAddress_32 = (uint32_t *)(flashAddress); 
  memcpy(&flashAddressValue, flashAddress_32, sizeof(float32_t)); /* set new treshold */
  return flashAddressValue;
}

/**@brief Function for reading axises from flash.
 */
void vFlashReadAxisFromFlash(void) {

  uint32_t *axisFlashAddress_32;
  axisFlashAddress_32 = (uint32_t *)(XAXISDATAADDRESS); 
  memcpy(&readX_axisThreshold, axisFlashAddress_32, sizeof(float32_t)); /* set new treshold */
  axisFlashAddress_32 = (uint32_t *)(YAXISDATAADDRESS); 
  memcpy(&readY_axisThreshold, axisFlashAddress_32, sizeof(float32_t)); /* set new treshold */
  memcpy(&SendThresholdBuffer[0], &SendThresholdValue, sizeof(SendThresholdValue));
  memcpy(&SendThresholdBuffer[2], &readX_axisThreshold, sizeof(readX_axisThreshold));
  memcpy(&SendThresholdBuffer[6], &readY_axisThreshold, sizeof(readY_axisThreshold));
  uart_tx_characteristic_update(&m_uart_service, (uint8_t *)&SendThresholdBuffer);
  memset(&SendThresholdBuffer[0], 0, 20); 
}
