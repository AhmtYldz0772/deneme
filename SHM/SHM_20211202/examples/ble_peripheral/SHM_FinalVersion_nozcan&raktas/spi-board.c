/*!
 * \file      spi-board.c
 *
 * \brief     Target board SPI driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Marten Lootsma(TWTG) on behalf of Microchip/Atmel (c)2017
 */

#include "spi-board.h"
#include "ADXL355.h"
#include "nrf_delay.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_spi.h"

#define SPI_INSTANCE 1  /**< LoRa SPI instance index. */
#define SPI_ADX355_INSTANCE 2 /* ADXL355 spi instance */

uint8_t SPIDummyBuffer[DUMMYBUFFERSIZE] = {0xFF}; /* Dummy Buffer */
uint8_t SPITxBuf[SPITXSIZE];  /* TX buffer. */
uint8_t SPIRxBuf[SPIRXSIZE];  /* RX buffer. */

uint8_t fifoBuffer[FIFOBUFFERSIZE] = {0}; /* Fifo buffer for receiving charcter of ADXL fifo */
uint8_t fifoCounter = 0U;

/**< SPI instance index. */
const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);                /**< SPI instance. */
const nrf_drv_spi_t spi_adxl355 = NRF_DRV_SPI_INSTANCE(SPI_ADX355_INSTANCE); /**< SPI instance. */
static volatile bool spi_xfer_done;                                          /**< Flag used to indicate that SPI instance completed the transfer. */
uint16_t rxData = 0;


/* Accelerometer write command */
#define ADXL355_WRITE 0x00

/* Accelerometer read command */
#define ADXL355_READ 0x01

void spi_event_handler(nrf_drv_spi_evt_t const *p_event) {
  if (p_event->type == NRF_DRV_SPI_EVENT_DONE) {
    spi_xfer_done = true;
  }
}

void SpiInit(Spi_t *obj, SpiId_t spiId, uint8_t mosi, uint8_t miso, uint8_t sclk, uint8_t nss, nrf_drv_spi_mode_t mode) {
  ret_code_t err_code;
  nrf_drv_spi_config_t spiConfig;
  spiConfig.mode = mode;
  spiConfig.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
  spiConfig.irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY;
  spiConfig.miso_pin = miso;
  spiConfig.mosi_pin = mosi;
  spiConfig.ss_pin = nss;
  spiConfig.sck_pin = sclk;
  obj->Miso.pin = miso;
  obj->Mosi.pin = mosi;
  obj->Nss.pin = nss;
  obj->Sclk.pin = sclk;
  obj->SpiId = spiId;

  switch (spiId) {

  case SPI_1:
    spiConfig.frequency = NRF_DRV_SPI_FREQ_500K;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spiConfig, NULL, NULL));
    break;

  case SPI_2:
    spiConfig.frequency = NRF_DRV_SPI_FREQ_4M;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi_adxl355, &spiConfig, NULL, NULL));
    break;
  }
}

void SpiDeInit(Spi_t *obj) {
  nrf_drv_spi_abort(&spi);
  nrf_drv_spi_uninit(&spi);
}

uint16_t SpiInOut(Spi_t *obj, uint16_t outData, uint8_t mode) {
  rxData = 0;
  spi_xfer_done = false;
  ret_code_t err_code;
  if (mode == 1)
    err_code = nrf_drv_spi_transfer(&spi, (uint8_t *)&outData, 1, (uint8_t *)&rxData, 0);
  else
    err_code = nrf_drv_spi_transfer(&spi, (uint8_t *)&outData, 1, (uint8_t *)&rxData, 1);

  APP_ERROR_CHECK(err_code);

  return (rxData);
}

/**
   @brief  SPI Read, 
  * @param address    ADXL355 register address
  * @param regSize    Sending message size
   @return none
**/
void SPIRead(uint8_t address, uint8_t regSize) {

  SPITxBuf[0] = ((address << 1) | ADXL355_READ);
  GpioWrite(&Adxl355.CSPin, 0);

  switch (regSize) {

  case SPI_READ_ONE_REG:
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, SPITxBuf, SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf[0], SPI_READ_ONE_REG));
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, SPIDummyBuffer, SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf[0], SPI_READ_ONE_REG));

    break;

  case SPI_READ_TWO_REG:
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, SPITxBuf, SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf[0], SPI_READ_ONE_REG));
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, SPIDummyBuffer, SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf[0], SPI_READ_ONE_REG));
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, SPIDummyBuffer, SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf[1], SPI_READ_ONE_REG));

    break;

  case SPI_READ_THREE_REG:
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, SPITxBuf, SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf[0], SPI_READ_ONE_REG));
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, SPIDummyBuffer, SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf[0], SPI_READ_ONE_REG));
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, SPIDummyBuffer, SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf[1], SPI_READ_ONE_REG));
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, SPIDummyBuffer, SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf[2], SPI_READ_ONE_REG));
    break;

  default:
    return;
    break;
  }


  spi_xfer_done = false;
  GpioWrite(&Adxl355.CSPin, 1);
}


/**
   @brief  SPI Write, 
  * @param address    ADXL355 register address
  * @param data1      first data to write that register address
  * @param data2      second data to write that register address
  * @param regSize    Sending message size
   @return none
**/
void SPIWrite(uint8_t address, uint8_t data1, uint8_t data2, uint8_t regSize) {

  SPITxBuf[0] = ((address << 1) | ADXL355_WRITE);
  GpioWrite(&Adxl355.CSPin, 0);

  switch (regSize) {

  //case SPI_READ_ONE_REG:
  case SPI_WRITE_ONE_REG:
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, SPITxBuf, SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf[0], SPI_READ_ONE_REG));
    SPITxBuf[0] = data1;
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, SPITxBuf, SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf[0], SPI_READ_ONE_REG));
    break;

  case SPI_WRITE_TWO_REG:
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, SPITxBuf, SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf[0], SPI_READ_ONE_REG));
    SPITxBuf[0] = data1;
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, SPITxBuf, SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf[0], SPI_READ_ONE_REG));
    SPITxBuf[0] = data2;
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, SPITxBuf, SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf[0], SPI_READ_ONE_REG));
    break;

  default:
    return;
    break;
  }

  spi_xfer_done = false;
  GpioWrite(&Adxl355.CSPin, 1);
}


/**
   @brief  SPI Read FIFO,  the function reads adxl355 fifo buffer
  * @param void
   @return none
**/
void SPIReadFIFO(void) {

  memset(&fifoBuffer[0], 0, 9);
  SPITxBuf[0] = ((FIFO_DATA << 1) | ADXL355_READ);

  GpioWrite(&Adxl355.CSPin, 0);

  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, SPITxBuf, SPI_SEND_ONE_REG, (uint8_t *)&SPIRxBuf[0], SPI_READ_ONE_REG));

  for (fifoCounter = 0; fifoCounter < 9; fifoCounter++) {
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, SPIDummyBuffer, SPI_SEND_ONE_REG, (uint8_t *)&fifoBuffer[fifoCounter], SPI_READ_ONE_REG));
  }

  spi_xfer_done = false;
  GpioWrite(&Adxl355.CSPin, 1);
}



//raktas&nozcan
void xSPIRead(uint8_t address, uint8_t *rxBuff, uint8_t rxLength) {

  uint8_t SpiTxBuff = ((address << 1) | ADXL355_READ);
  
  GpioWrite(&Adxl355.CSPin, 0);

  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, (uint8_t *) &SpiTxBuff, 1, NULL, 0));
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_adxl355, NULL, 0, rxBuff, rxLength));

  GpioWrite(&Adxl355.CSPin, 1);


  /*
  rf_drv_spi_t const * const p_instance,
  uint8_t const * p_tx_buffer,
  uint8_t         tx_buffer_length,
  uint8_t       * p_rx_buffer,
  uint8_t         rx_buffer_length

  [in]	p_instance	Pointer to the driver instance structure.
  [in]	p_tx_buffer	Pointer to the transmit buffer. Can be NULL if there is nothing to send.
  tx_buffer_length	Length of the transmit buffer.
  [in]	p_rx_buffer	Pointer to the receive buffer. Can be NULL if there is nothing to receive.
  rx_buffer_length	Length of the receive buffer.
  */

}

