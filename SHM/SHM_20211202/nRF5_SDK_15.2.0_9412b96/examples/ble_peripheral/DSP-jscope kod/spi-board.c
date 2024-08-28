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
#include "nrf_drv_gpiote.h"
#include "nrf_drv_spi.h"

#define SPI_INSTANCE 1                                        /**< SPI instance index. */
const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE); /**< SPI instance. */
static volatile bool spi_xfer_done;                           /**< Flag used to indicate that SPI instance completed the transfer. */
uint16_t rxData = 0;

void spi_event_handler(nrf_drv_spi_evt_t const *p_event)
{
    if (p_event->type == NRF_DRV_SPI_EVENT_DONE)
    {
        spi_xfer_done = true;
    }
}

void SpiInit(Spi_t *obj, SpiId_t spiId, uint8_t mosi, uint8_t miso, uint8_t sclk, uint8_t nss, nrf_drv_spi_mode_t mode)
{
    ret_code_t err_code;
    nrf_drv_spi_config_t spiConfig;
    spiConfig.mode = mode;
    spiConfig.frequency = NRF_DRV_SPI_FREQ_500K;
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
    err_code = nrf_drv_spi_init(&spi, &spiConfig, NULL, NULL);
    APP_ERROR_CHECK(err_code);
}

void SpiDeInit(Spi_t *obj)
{
    nrf_drv_spi_abort(&spi);
    nrf_drv_spi_uninit(&spi);
}

uint16_t SpiInOut(Spi_t *obj, uint16_t outData, uint8_t mode)
{
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