/*#include "Communication.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_drv_spi.h"

#include "board.h"
#include "System/spi.h"
#include "board-config.h"

#include "rtc-board.h"
#include "sx1272-board.h"
#include "utilities.h"


#define SPI_INSTANCE  2 /*  SPI instance index. */
/*static const nrf_drv_spi_t spi2 = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /* SPI instance. */
/*uint8_t spi_xfer_done;  /* Flag used to indicate that SPI instance completed the transfer. */

/*uint8_t       SPIDummy[1] = {0xFF}; 
/*uint8_t       SPITxBuf[1] ;        /* TX buffer. */
/*uint8_t       SPITxBuf2[2] ;       /* TX buffer. */
/*uint8_t       SPITxBuf3[3] ;       /* TX buffer. */
/*uint8_t       SPIRxBuf[3];         /* RX buffer. */
/*
uint8_t rxData2 = 0;
extern Gpio_t CSPin;
/**
 * @brief SPI user event handler.
 * @param event
 */
/*void spi_adxl355_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    if (p_event->type == NRF_DRV_SPI_EVENT_DONE)
    {
        spi_xfer_done = true;
    }

    //bu interrupti cozmek gerekir ne zaman geldigini. Sus kiz.
}


/**
   @brief  SPI Initialize
   @return none
**/
/*
void SPI_Init (void)
{
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = NRF_DRV_SPI_PIN_NOT_USED;
    spi_config.miso_pin = SPIADXL_MISO_PIN;
    spi_config.mosi_pin = SPIADXL_MOSI_PIN;
    spi_config.sck_pin  = SPIADXL_SCK_PIN;
    spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
    spi_config.irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY;
    spi_config.frequency = NRF_DRV_SPI_FREQ_500K;
    spi_config.mode = NRF_DRV_SPI_MODE_0;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi2, &spi_config, NULL, NULL)); 

   
  //  SpiInit(&spi_config, SPI_2, SPIADXL_MOSI_PIN, SPIADXL_MISO_PIN, SPIADXL_SCK_PIN, SPIADXL_SS_PIN, NRF_DRV_SPI_MODE_0);
}



/**
   @brief  SPI Read, 
  * @param address    ADXL355 register address
  * @param regSize    Sending message size
   @return none
**/
/*
void SPIRead(uint8_t address, uint8_t regSize)
{

  SPITxBuf[0]  = ((address << 1) | ADXL355_READ);
  GpioWrite( &Adxl355.CSPin, 0 );
 
  switch  (regSize)
  {

    case SPI_READ_ONE_REG:
      APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi2, SPITxBuf, SPI_SEND_ONE_REG, (uint8_t *)&rxData2, SPI_READ_ONE_REG));  
      APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi2, SPIDummy , SPI_SEND_ONE_REG, (uint8_t *)&rxData2, SPI_READ_ONE_REG));  

    break;

    case SPI_READ_TWO_REG:
      APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi2, SPITxBuf, SPI_SEND_ONE_REG, SPIRxBuf, SPI_READ_ONE_REG));
      APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi2, SPIDummy , SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf, SPI_READ_ONE_REG));
      APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi2, SPIDummy , SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf, SPI_READ_ONE_REG));
    break;         
                                                                                         
    case SPI_READ_THREE_REG:
      APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi2, SPITxBuf, SPI_SEND_ONE_REG, SPIRxBuf, SPI_READ_ONE_REG));
      APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi2, SPIDummy , SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf, SPI_READ_ONE_REG));
      APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi2, SPIDummy , SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf, SPI_READ_ONE_REG));
      APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi2, SPIDummy , SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf, SPI_READ_ONE_REG));
    break;

    default:
      return;
    break;

  }

 /* while (!spi_xfer_done)
  {
    __WFE();
  }
  */
 // NRF_LOG_FLUSH();
  /*spi_xfer_done = false;
  GpioWrite(&Adxl355.CSPin, 1 );
  printf("Received bytes: %x \r\n ", rxData2);

  
  
}


/**
   @brief  SPI Write, 
  * @param address    ADXL355 register address
  * @param data1      first data to write that register address
  * @param data2      second data to write that register address
  * @param regSize    Sending message size
   @return none
**/
/*void SPIWrite(uint8_t address, uint8_t data1, uint8_t data2, uint8_t regSize)
{
  
  SPITxBuf[0]  = ((address << 1) | ADXL355_WRITE);

  switch  (regSize)
  {

    case SPI_READ_ONE_REG:
      SPITxBuf2[0] = address | ADXL355_WRITE;
      SPITxBuf2[1]  = data1;
      APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi2, SPITxBuf2, SPI_SEND_TWO_REG, SPIRxBuf, SPI_READ_ONE_REG));
    break;

    case SPI_WRITE_TWO_REG:
      SPITxBuf3[0] = address | ADXL355_WRITE;
      SPITxBuf3[1]  = data1;
      SPITxBuf3[2]  = data2;
      APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi2, SPITxBuf3, SPI_SEND_THREE_REG, SPIRxBuf, SPI_READ_ONE_REG));
    break;                                                                                              

    default:
      return;
    break;

  }

 /* while (!spi_xfer_done)
  {
    __WFE();
  }
  */

  
 // NRF_LOG_FLUSH();
  /*spi_xfer_done = false; 
 // printf("Received bytes: %x \r\n ", SPIRxBuf[0]);
  
  
}


void MessageReceive(void)
{
}
*/