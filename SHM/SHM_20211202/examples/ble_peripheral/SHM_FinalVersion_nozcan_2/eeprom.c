#include <stdio.h>
#include <stdlib.h>

#include "nrf_delay.h"
/*---------------------------------------------eeprom Includes-----------------------------------------*/
#include "eeprom.h"
/*-------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------CMSIS Includes-----------------------------------------*/
#include "arm_math.h"
/*-------------------------------------------------------------------------------------------------------*/
//-------------------------------------------- NRF-WDT Includes -------------------------------------//

#include "nrf_drv_wdt.h"

//-------------------------------------------------------------------------------------------------------//

/*---------------------------------------------TWI Includes-----------------------------------------*/
#include "nrf_drv_twi.h"
/*-------------------------------------------------------------------------------------------------------*/
#include "BLE_Services/messageCodes.h"
#include "BLE_Services/UART_Service.h"


/* TWI instance ID. */
#define TWI_INSTANCE_ID     0
#define WRITINGDATALENGTH  10
#define READINGDATALENGTH  2
#define UARTDATALENGTH  240
uint8_t slaveAddress;

uint8_t writingData[WRITINGDATALENGTH] ;
uint8_t readingData[READINGDATALENGTH] ;
#define MAX_UART_LEN      (NRF_SDH_BLE_GATT_MAX_MTU_SIZE) /**< Maximum size of a transmitted Heart Rate Measurement. */
uint8_t bluetoothBuffer[MAX_UART_LEN] = {0};
uint16_t  bluetoothCounter = 0;
uint32_t bluetoothAddress = 0;
uint32_t eepromWriteAddress = 0;
uint16_t EepromDataMID = EEPROM_DATA_MESSAGE;
uint32_t eepromAddressAfterPage = 0;

extern uint8_t fifoBuffer[FIFOBUFFERSIZE];
extern uint8_t maxUartServiceDataLength;

static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

extern nrf_drv_wdt_channel_id m_channel_id;
extern ble_uarts_t m_uart_service;




/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
            if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX)
            {
                
            }
            break;
        default:
            break;
    }
}


/**
 * @brief UART initialization.
 */
void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_lm75b_config = {
       .scl                = EEPROM_SDCL_PIN,
       .sda                = EEPROM_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_lm75b_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
} 

/**
 * @brief Writing FIFO values to EEPROM.
 */
void vInitFIfotoEeprom (void)
{
  ret_code_t err_code;
  uint16_t pageControl = 0;
  

  if( (eepromWriteAddress + 5) > LAST_MEMORY_ADDRESS  || eepromWriteAddress < FIRST_MEMORY_ADDRESS )
  {
      eepromWriteAddress = FIRST_MEMORY_ADDRESS;
  }

  
  pageControl = PAGESIZE - (eepromWriteAddress % PAGESIZE);
  
  memset(&writingData[0], 0, sizeof(writingData));

  if(pageControl > 0 && pageControl < 6)
  {
    slaveAddress = ( SLAVE_ADDRESS << 3) | (DEVICE_ADDRESS << 2)  | ((eepromWriteAddress & 0x10000) >> 16);     /* EEPROM device address according to datatsheet.*/
    writingData[1] = (uint8_t)eepromWriteAddress;
    writingData[0] = (uint8_t)(eepromWriteAddress >> 8) ; 

    
    memcpy(&writingData[2], &fifoBuffer[0], pageControl);
    while(nrf_drv_twi_is_busy(&m_twi));
    nrf_drv_twi_tx(&m_twi, slaveAddress, (uint8_t *) &writingData[0], (pageControl + 2), false);      /*Writing axis values to slave address.*/
    nrf_delay_ms(6);

    eepromAddressAfterPage = eepromWriteAddress + pageControl;  
    
    slaveAddress = ( SLAVE_ADDRESS << 3) | (DEVICE_ADDRESS << 2)  | ((eepromAddressAfterPage & 0x10000) >> 16);     /* EEPROM device address according to datatsheet.*/
    memset(&writingData[0], 0, sizeof(writingData));
    
    writingData[1] = (uint8_t)eepromAddressAfterPage;
    writingData[0] = (uint8_t)(eepromAddressAfterPage >> 8) ; 
    memcpy(&writingData[2], &fifoBuffer[pageControl], (6 - pageControl) );
    while(nrf_drv_twi_is_busy(&m_twi));
    nrf_drv_twi_tx(&m_twi, slaveAddress, (uint8_t *) &writingData[0], (8 - pageControl), false);      /*Writing axis values to slave address.*/
    nrf_delay_ms(6);

    eepromWriteAddress = eepromWriteAddress + 6;      /* Shifting EEPROM write adddress 6 bytes*/
  }

  else
  {
    slaveAddress = ( SLAVE_ADDRESS << 3) | (DEVICE_ADDRESS << 2)  | ((eepromWriteAddress & 0x10000) >> 16);     /* EEPROM device address according to datatsheet.*/

    writingData[1] = (uint8_t)eepromWriteAddress;
    writingData[0] = (uint8_t)(eepromWriteAddress >> 8) ; 
    memcpy(&writingData[2], &fifoBuffer[0], 6);


    while(nrf_drv_twi_is_busy(&m_twi));
    err_code =  nrf_drv_twi_tx(&m_twi, slaveAddress, (uint8_t *) &writingData[0], 8, false);      /*Writing axis values to slave address.*/
    if(err_code != 0)
    {
      nrf_delay_ms(5);
      err_code =  nrf_drv_twi_tx(&m_twi, slaveAddress, (uint8_t *) &writingData[0], 8, false);      /*Writing axis values to slave address.*/
      printf("error\n");
    }

   eepromWriteAddress = eepromWriteAddress + 6;      /* Shifting EEPROM write adddress 6 bytes*/
  }
  

  
}



void vInitTwiWrite (uint32_t address , uint8_t *data, uint8_t size )
{

  slaveAddress = ( SLAVE_ADDRESS << 3) | (DEVICE_ADDRESS << 2)  | ((address & 0x10000) >> 16);      /* EEPROM device address according to datatsheet.*/


  writingData[1] = (uint8_t)address;
  writingData[0] = (uint8_t)(address >> 8) ; 
  memcpy(&writingData[2], data, size );

  nrf_drv_twi_tx(&m_twi, slaveAddress, (uint8_t *) &writingData[0], (size + 2), false);

  nrf_delay_ms(5);

}


void vInitTwiRead (uint32_t address, uint8_t size, uint8_t * dataPointer)
{
  uint16_t  eepromAddress = (uint16_t)address;

  slaveAddress = ( SLAVE_ADDRESS << 3) | (DEVICE_ADDRESS << 2)  | ((address & 0x10000) >> 16) ;     /* EEPROM device address according to datatsheet.*/
  
  readingData[1] = (uint8_t)eepromAddress ;  
  readingData[0] = (uint8_t)(eepromAddress >> 8);

  nrf_drv_twi_tx(&m_twi, slaveAddress, &(readingData[0]), 2, true);
  nrf_delay_ms(1);

  nrf_drv_twi_rx(&m_twi, slaveAddress, dataPointer, size);
  nrf_delay_ms(1);
}

void prvSendEepromData(void)
{
 uint8_t UartServiceDataLength = UARTDATALENGTH;    /* Length of EEPROM data to read*/ 
 bluetoothAddress = eepromWriteAddress + (UartServiceDataLength * bluetoothCounter);    /*Packet start address which send*/ 

  if(bluetoothAddress >= LAST_MEMORY_ADDRESS || bluetoothAddress < FIRST_MEMORY_ADDRESS )
  {
    bluetoothAddress = (UartServiceDataLength*bluetoothCounter) - (LAST_MEMORY_ADDRESS - eepromWriteAddress);
  }
  if ((LAST_MEMORY_ADDRESS - bluetoothAddress) < UartServiceDataLength)
  {
    uint8_t firstPartofDataLength = LAST_MEMORY_ADDRESS - bluetoothAddress;
    uint8_t secondPartofDataLength = UartServiceDataLength - firstPartofDataLength;
    EepromDataMID = bluetoothCounter + EEPROM_DATA_MESSAGE;
    memset(&bluetoothBuffer[0], 0, sizeof(bluetoothBuffer));
    memcpy(&bluetoothBuffer[0], &EepromDataMID, sizeof(EepromDataMID));
    vInitTwiRead(bluetoothAddress, firstPartofDataLength, (uint8_t *) &bluetoothBuffer[2]);
    vInitTwiRead(FIRST_MEMORY_ADDRESS, secondPartofDataLength, (uint8_t *) &bluetoothBuffer[2 + firstPartofDataLength]);
  }
  else
  {
    EepromDataMID = bluetoothCounter + EEPROM_DATA_MESSAGE;    
    memset(&bluetoothBuffer[0], 0, sizeof(bluetoothBuffer));  
    memcpy(&bluetoothBuffer[0], &EepromDataMID, sizeof(EepromDataMID));
    vInitTwiRead(bluetoothAddress, UartServiceDataLength, (uint8_t *) &bluetoothBuffer[2]);
  }

  nrf_drv_wdt_channel_feed(m_channel_id);   
  nrf_delay_ms(20);

  uart_tx_characteristic_update(&m_uart_service, (uint8_t *)&bluetoothBuffer[0]);
                         
}


//void eepromWriteff(void)
//{  uint32_t countertestr = 0;
//  
//  uint8_t slaveAddresstest = 0;
//  uint8_t writingDatatest[10] = {0};
//    uint16_t pageNumber = 0;
//  uint32_t eepromAddressAfterPagetest = 0;
//  uint8_t pageControl = 0;
//
//// for( countertestr = 0; countertestr < 32768; countertestr ++)
//// {
////
//// slaveAddresstest = ( SLAVE_ADDRESS << 3) | (DEVICE_ADDRESS << 2)  | ((eepromWriteAddresstest & 0x10000) >> 16);     /* EEPROM device address according to datatsheet.*/
////
//// writingDatatest[1] = (uint8_t)eepromWriteAddresstest;
//// writingDatatest[0] = (uint8_t)(eepromWriteAddresstest >> 8) ; 
//// writingDatatest[3] = 0xff;
//// writingDatatest[4] = 0xff;
//// writingDatatest[5] = 0xff;
//// writingDatatest[6] = 0xff;
//// writingDatatest[7] = 0xff;
//// writingDatatest[8] = 0xff;
//// writingDatatest[9] = 0xff;
////
//// while(nrf_drv_twi_is_busy(&m_twi));
//// nrf_drv_twi_tx(&m_twi, slaveAddresstest, (uint8_t *) &writingDatatest[0], 10, false);      /*Writing axis values to slave address.*/
////
//// nrf_delay_ms(5);
//// eepromWriteAddresstest = eepromWriteAddresstest ++;      /* Shifting EEPROM write adddress 6 bytes*/
////
//// }
////  printf("eeprom full");
//
//
//
//
//
//  pageNumber = eepromWriteAddresstest / PAGESIZE;
//  pageControl = PAGESIZE - (eepromWriteAddresstest - PAGESIZE*pageNumber) + 1;
//
//
//for (countertestr = 0; countertestr < 5; countertestr ++)
//{
//  slaveAddresstest = ( SLAVE_ADDRESS << 3) | (DEVICE_ADDRESS << 2)  | ((eepromWriteAddresstest & 0x10000) >> 16);     /* EEPROM device address according to datatsheet.*/
//  
//  writingDatatest[1] = (uint8_t)eepromWriteAddresstest;
//  writingDatatest[0] = (uint8_t)(eepromWriteAddresstest >> 8) ; 
// memcpy(&writingDatatest[2], &fifoBuffer[0], 6);
//  
//  while(nrf_drv_twi_is_busy(&m_twi));
//  nrf_drv_twi_tx(&m_twi, slaveAddresstest, (uint8_t *) &writingDatatest[0], 8, false); 
//  nrf_delay_ms(5);
//
////   slaveAddresstest = ( SLAVE_ADDRESS << 3) | (DEVICE_ADDRESS << 2)  | ((eepromWriteAddresstest & 0x10000) >> 16);     /* EEPROM device address according to datatsheet.*/
////
////  writingDatatest[1] = (uint8_t)eepromWriteAddresstest;
////  writingDatatest[0] = (uint8_t)(eepromWriteAddresstest >> 8) ; 
////
////  if(pageControl > 0 && pageControl < 6)
////  {
////    memcpy(&writingDatatest[2], &testBuffer[0], pageControl);
////    nrf_drv_twi_tx(&m_twi, slaveAddresstest, (uint8_t *) &writingDatatest[0], pageControl + 2, false);      /*Writing axis values to slave address.*/
////    nrf_delay_ms(5);
////    eepromAddressAfterPagetest = eepromWriteAddresstest + pageControl;  // 1 olmasi gerekebilir
////    
////    slaveAddresstest = ( SLAVE_ADDRESS << 3) | (DEVICE_ADDRESS << 2)  | ((eepromAddressAfterPagetest & 0x10000) >> 16);     /* EEPROM device address according to datatsheet.*/
////    writingDatatest[1] = (uint8_t)eepromAddressAfterPagetest;
////    writingDatatest[0] = (uint8_t)(eepromAddressAfterPagetest >> 8) ; 
////    memcpy(&writingDatatest[2], &testBuffer[pageControl], 6 - pageControl);
////    nrf_drv_twi_tx(&m_twi, slaveAddresstest, (uint8_t *) &writingDatatest[0], (6 - pageControl) + 2, false);      /*Writing axis values to slave address.*/
////  }
////  else
////  {
////    slaveAddresstest = ( SLAVE_ADDRESS << 3) | (DEVICE_ADDRESS << 2)  | ((eepromWriteAddresstest & 0x10000) >> 16);     /* EEPROM device address according to datatsheet.*/
////
////  writingDatatest[1] = (uint8_t)eepromWriteAddresstest;
////  writingDatatest[0] = (uint8_t)(eepromWriteAddresstest >> 8) ; 
////  memcpy(&writingDatatest[2], &testBuffer[0], 6);
////
////
////  while(nrf_drv_twi_is_busy(&m_twi));
////  nrf_drv_twi_tx(&m_twi, slaveAddresstest, (uint8_t *) &writingDatatest[0], 8, false);      /*Writing axis values to slave address.*/
////  }
////
////    memset(&bluetoothBuffer[0], 0, sizeof(bluetoothBuffer));  
////    vInitTwiRead(eepromWriteAddresstest, 6, (uint8_t *) &bluetoothBuffer[0]);
////    nrf_delay_ms(5);
////
//    if(!(memcmp(&writingDatatest[2],&bluetoothBuffer[0],6)))
//    {
//    printf("same\n");
//    break;
//    }
//
//    else
//    printf("%d %d %d %d %d %d\n",bluetoothBuffer[0],bluetoothBuffer[1],bluetoothBuffer[2],bluetoothBuffer[3],
//    bluetoothBuffer[4],bluetoothBuffer[5]);
//}
//
//  if( countertestr > 3)
//  {
//     slaveAddresstest = ( SLAVE_ADDRESS << 3) | (DEVICE_ADDRESS << 2)  | ((eepromWriteAddresstest & 0x10000) >> 16);     /* EEPROM device address according to datatsheet.*/
//  
//    writingDatatest[1] = (uint8_t)eepromWriteAddresstest;
//    writingDatatest[0] = (uint8_t)(eepromWriteAddresstest >> 8) ; 
//    writingDatatest[2] = 0xF1;
//    writingDatatest[3] = 0xF1;
//    writingDatatest[4] = 0xF1;
//    writingDatatest[5] = 0xF2;
//    writingDatatest[6] = 0xF2;
//    writingDatatest[7] = 0xF2;
//
//   nrf_drv_twi_tx(&m_twi, slaveAddresstest, (uint8_t *) &writingDatatest[0], 8, false); 
//   nrf_delay_ms(5);
//
//     memset(&bluetoothBuffer[0], 0, sizeof(bluetoothBuffer));  
//    vInitTwiRead(eepromWriteAddresstest, 6, (uint8_t *) &bluetoothBuffer[0]);
//    if(!(memcmp(&writingDatatest[2],&bluetoothBuffer[0],6)))
//    {
//    printf("same second try\n");
//    }
//
//    else
//    printf("%d %d %d %d %d %d\n",bluetoothBuffer[0],bluetoothBuffer[1],bluetoothBuffer[2],bluetoothBuffer[3],
//    bluetoothBuffer[4],bluetoothBuffer[5]);
//  }
//}
//


